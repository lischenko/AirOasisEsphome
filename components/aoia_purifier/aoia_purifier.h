#pragma once

#include "esphome/components/fan/fan.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome::aoia_purifier {

class AOIAPurifier : public Component, public fan::Fan {
 public:
  void setup() override;
  void dump_config() override;
  fan::FanTraits get_traits() override;

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }

  // Called exclusively after the YAML UART callback has accepted a complete,
  // checksum-valid appliance response. This is the only state-publication path.
  void update_from_appliance(uint8_t flags, uint8_t speed_status);

  // Change one feature flag while preserving all remaining flags and the
  // current mapped Auto/manual mode. It intentionally does not publish state.
  void set_option(uint8_t flag_mask, bool enabled);

  // Select a named mode while preserving all feature flags. It intentionally
  // does not publish state; the next appliance response is authoritative.
  void set_mode(const char *mode);

 protected:
  void control(const fan::FanCall &call) override;

  bool mode_is_known_(uint8_t speed_status) const;
  uint8_t command_for_status_(uint8_t speed_status) const;
  void send_command_(uint8_t flags, uint8_t speed_code);

  uart::UARTComponent *uart_{nullptr};
  uint8_t flags_{0};
  uint8_t speed_status_{0};
  bool appliance_state_known_{false};
};

}  // namespace esphome::aoia_purifier
