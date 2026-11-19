#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <utility>

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

 protected:
  static uint8_t encode_char_(char c);
  void prepare_frame_(const std::string &text);

  TextLambda text_lambda_;
  std::array<uint8_t, 8> segments_{};
  uint8_t digit_index_{0};
  uint32_t last_scan_us_{0};
  uint32_t scan_count_{0};
  bool segment_active_low_{true};
  bool digit_active_high_{true};
};

}  // namespace esphome::hc595_7seg
