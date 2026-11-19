#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "esp_timer.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome::hc595_7seg {

using TextLambda = std::function<std::optional<std::string>()>;

class HC5957Segment final
    : public text_sensor::TextSensor,
      public PollingComponent,
      public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                            spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_text_lambda(TextLambda lambda) { this->text_lambda_ = std::move(lambda); }
  void set_segment_active_low(bool value) { this->segment_active_low_ = value; }
  void set_digit_active_high(bool value) { this->digit_active_high_ = value; }
  void set_chain_length(size_t value) { this->chain_length_ = value; }

 protected:
  static void timer_callback_(void *arg);
  void scan_digit_();
  static uint8_t encode_char_(char c);
  void prepare_frame_(size_t module_index, const std::string &text);

  TextLambda text_lambda_;
  std::vector<std::array<uint8_t, 8>> segments_;
  size_t chain_length_{1};
  uint8_t digit_index_{0};
  esp_timer_handle_t scan_timer_{nullptr};
  std::atomic<uint32_t> scan_count_{0};
  bool segment_active_low_{true};
  bool digit_active_high_{true};
};

}  // namespace esphome::hc595_7seg
