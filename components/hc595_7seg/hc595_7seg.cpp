#include "hc595_7seg.h"

#include <algorithm>
#include <cctype>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::hc595_7seg {

static const char *const TAG = "hc595_7seg";

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
  this->last_scan_us_ = micros();
}

void HC5957Segment::update() {
  if (!this->text_lambda_)
    return;

  const auto value = this->text_lambda_();
  if (!value.has_value())
    return;

  this->publish_state(*value);
  this->prepare_frame_(*value);
}

void HC5957Segment::loop() {
  // Scan one digit every 2 ms: a complete 8-digit frame is about 62 Hz.
  const uint32_t now = micros();
  if (now - this->last_scan_us_ < 2000)
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

  this->digit_index_ = (this->digit_index_ + 1) & 0x07;
}

void HC5957Segment::prepare_frame_(const std::string &text) {
  const uint8_t blank = this->segment_active_low_ ? 0xFF : 0x00;
  this->segments_.fill(blank);

  // Right-align up to eight characters. A decimal point attaches to the
  // preceding digit, so it does not consume a digit position.
  size_t output_pos = 8;
  for (size_t i = text.size(); i > 0; i--) {
    const char c = text[i - 1];
    if (c == '.' || c == ',') {
      if (output_pos < 8) {
        if (this->segment_active_low_)
          this->segments_[output_pos] &= static_cast<uint8_t>(~0x80);
        else
          this->segments_[output_pos] |= 0x80;
      }
      continue;
    }
    if (output_pos == 0)
      break;
    --output_pos;
    uint8_t pattern = encode_char_(c);
    if (this->segment_active_low_)
      pattern = static_cast<uint8_t>(~pattern);
    this->segments_[output_pos] = pattern;
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
  LOG_PIN("  Latch/RCK: ", this->cs_);
  ESP_LOGCONFIG(TAG, "  Segment active low: %s", this->segment_active_low_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Digit active high: %s", this->digit_active_high_ ? "YES" : "NO");
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace esphome::hc595_7seg
