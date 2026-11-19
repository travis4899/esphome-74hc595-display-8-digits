#include "hc595_7seg.h"

#include <algorithm>
#include <cctype>

#include "esp_timer.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::hc595_7seg {

static const char *const TAG = "hc595_7seg";
static constexpr uint64_t SCAN_INTERVAL_US = 1000;

// Bit order: dp, g, f, e, d, c, b, a. A set bit means the segment is on
// before applying the module's active-low/active-high polarity.
static const uint8_t DIGIT_GLYPHS[10] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F,  // 9
};

float HC5957Segment::get_setup_priority() const { return setup_priority::PROCESSOR; }

void HC5957Segment::setup() {
  this->spi_setup();
  this->segments_.fill(this->segment_active_low_ ? 0xFF : 0x00);
  this->digit_index_ = 0;
  esp_timer_create_args_t timer_args = {};
  timer_args.callback = &HC5957Segment::timer_callback_;
  timer_args.arg = this;
  timer_args.dispatch_method = ESP_TIMER_TASK;
  timer_args.name = "hc595_scan";
  esp_err_t err = esp_timer_create(&timer_args, &this->scan_timer_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create scan timer: %s", esp_err_to_name(err));
    this->mark_failed();
    return;
  }
  err = esp_timer_start_periodic(this->scan_timer_, SCAN_INTERVAL_US);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start scan timer: %s", esp_err_to_name(err));
    this->mark_failed();
  }
}

void HC5957Segment::update() {
  const uint32_t scans = this->scan_count_.exchange(0, std::memory_order_relaxed);
  ESP_LOGI(TAG, "Observed scan rate: %u digit writes/s (~%u full frames/s)", scans, scans / 8);

  if (!this->text_lambda_)
    return;

  const auto value = this->text_lambda_();
  if (!value.has_value())
    return;

  this->publish_state(*value);
  this->prepare_frame_(*value);
}

void HC5957Segment::loop() {
  // Scan one digit every 100 us: a complete 8-digit frame is about 1.25 kHz.
  const uint32_t now = micros();
  if (now - this->last_scan_us_ < 100)
    return;
  this->last_scan_us_ = now;

  const uint8_t logical_digit = this->digit_index_;
  // These boards wire their physical digits in the order 4,5,6,7,0,1,2,3.
  const uint8_t physical_digit = (logical_digit + 4) & 0x07;
  uint8_t digit_mask = static_cast<uint8_t>(1U << physical_digit);
  if (!this->digit_active_high_)
    digit_mask = static_cast<uint8_t>(~digit_mask);

  this->enable();
  // The AceSegment Hc595Module uses segment byte first, then digit-select byte.
  this->write_byte(this->segments_[logical_digit]);
  this->write_byte(digit_mask);
  this->disable();
  this->scan_count_.fetch_add(1, std::memory_order_relaxed);

  this->digit_index_ = (this->digit_index_ + 1) & 0x07;
}

void HC5957Segment::prepare_frame_(const std::string &text) {
  const uint8_t blank = this->segment_active_low_ ? 0xFF : 0x00;
  this->segments_.fill(blank);

  // Count visible characters first so shorter strings are right-aligned.
  size_t visible = 0;
  for (char c : text) {
    if (c != '.' && c != ',')
      ++visible;
  }
  const size_t skip = visible > 8 ? visible - 8 : 0;
  size_t skipped = 0;
  size_t output_pos = 8 - std::min<size_t>(visible, 8);
  size_t previous_pos = 8;

  for (char c : text) {
    if (c == '.' || c == ',') {
      if (previous_pos < 8) {
        if (this->segment_active_low_)
          this->segments_[previous_pos] &= static_cast<uint8_t>(~0x80);
        else
          this->segments_[previous_pos] |= 0x80;
      }
      continue;
    }
    if (skipped < skip) {
      ++skipped;
      continue;
    }
    if (output_pos >= 8)
      break;

    uint8_t pattern = encode_char_(c);
    if (this->segment_active_low_)
      pattern = static_cast<uint8_t>(~pattern);
    this->segments_[output_pos] = pattern;
    previous_pos = output_pos;
    ++output_pos;
  }
}

uint8_t HC5957Segment::encode_char_(char c) {
  if (c >= '0' && c <= '9')
    return DIGIT_GLYPHS[c - '0'];
  switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
    case '-':
      return 0x40;  // g
    case ' ':
      return 0x00;
    default:
      return 0x00;
  }
}

void HC5957Segment::dump_config() {
  ESP_LOGCONFIG(TAG, "74HC595 8-digit 7-segment display");
  ESP_LOGCONFIG(TAG, "  Scan interval: 1000 us/digit (125 Hz full frame target)");\n  ESP_LOGCONFIG(TAG, "  Scan timer: ESP-IDF task dispatch");
  LOG_PIN("  Latch/RCK: ", this->cs_);
  ESP_LOGCONFIG(TAG, "  Segment active low: %s", this->segment_active_low_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Digit active high: %s", this->digit_active_high_ ? "YES" : "NO");
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace esphome::hc595_7seg
