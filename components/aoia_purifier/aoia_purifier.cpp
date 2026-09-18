#include "aoia_purifier.h"

#include <cstring>

#include "esphome/core/log.h"

namespace esphome::aoia_purifier {

static const char *const TAG = "aoia_purifier";

static constexpr uint8_t FLAG_POWER = 0x01;
static constexpr uint8_t STATUS_AUTO = 0x11;
static constexpr uint8_t SPEED_LOW = 0x13;
static constexpr uint8_t SPEED_MEDIUM = 0x23;
static constexpr uint8_t SPEED_HIGH = 0x33;
static constexpr uint8_t SPEED_MAX = 0x43;
static constexpr uint8_t COMMAND_AUTO = 0x41;

void AOIAPurifier::setup() {
  this->set_supported_preset_modes({"Auto"});
}

void AOIAPurifier::dump_config() { LOG_FAN("", "AOIA Purifier", this); }

fan::FanTraits AOIAPurifier::get_traits() {
  fan::FanTraits traits(false, true, false, 4);
  this->wire_preset_modes_(traits);
  return traits;
}

bool AOIAPurifier::mode_is_known_(uint8_t speed_status) const {
  return speed_status == STATUS_AUTO || speed_status == SPEED_LOW || speed_status == SPEED_MEDIUM ||
         speed_status == SPEED_HIGH || speed_status == SPEED_MAX;
}

uint8_t AOIAPurifier::command_for_status_(uint8_t speed_status) const {
  return speed_status == STATUS_AUTO ? COMMAND_AUTO : speed_status;
}

void AOIAPurifier::send_command_(uint8_t flags, uint8_t speed_code) {
  if (this->uart_ == nullptr) {
    ESP_LOGE(TAG, "Cannot send AOIA command: UART is not configured");
    return;
  }

  const uint8_t prefix[] = {0xBB, 0x02, flags, speed_code, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t checksum = 0;
  for (size_t i = 1; i < sizeof(prefix); i++) {
    checksum = static_cast<uint8_t>(checksum + prefix[i]);
  }
  const uint8_t frame[] = {0xBB, 0x02, flags, speed_code, 0x03, 0x00, 0x00,
                           0x00, 0x00, 0x00, static_cast<uint8_t>(0U - checksum), 0x44};
  this->uart_->write_array(frame, sizeof(frame));
}

void AOIAPurifier::set_option(uint8_t flag_mask, bool enabled) {
  if (!this->appliance_state_known_ || !this->mode_is_known_(this->speed_status_)) {
    ESP_LOGW(TAG, "Ignoring option command until a valid response establishes flags and operating mode");
    return;
  }

  uint8_t flags = this->flags_;
  if (enabled) {
    flags |= flag_mask;
  } else {
    flags &= static_cast<uint8_t>(~flag_mask);
  }
  this->send_command_(flags, this->command_for_status_(this->speed_status_));
}

void AOIAPurifier::set_mode(const char *mode) {
  if (!this->appliance_state_known_) {
    ESP_LOGW(TAG, "Ignoring mode command until a checksum-valid appliance response is received");
    return;
  }

  uint8_t speed_code;
  if (strcmp(mode, "Auto") == 0) {
    speed_code = COMMAND_AUTO;
  } else if (strcmp(mode, "Low") == 0) {
    speed_code = SPEED_LOW;
  } else if (strcmp(mode, "Medium") == 0) {
    speed_code = SPEED_MEDIUM;
  } else if (strcmp(mode, "High") == 0) {
    speed_code = SPEED_HIGH;
  } else if (strcmp(mode, "Max") == 0) {
    speed_code = SPEED_MAX;
  } else {
    ESP_LOGW(TAG, "Ignoring unsupported mode selection");
    return;
  }

  this->send_command_(this->flags_ | FLAG_POWER, speed_code);
}

void AOIAPurifier::control(const fan::FanCall &call) {
  if (!this->appliance_state_known_) {
    ESP_LOGW(TAG, "Ignoring purifier command until a checksum-valid appliance response is received");
    return;
  }

  uint8_t flags = this->flags_;
  uint8_t speed_code = this->command_for_status_(this->speed_status_);

  // Power-off deliberately wins over any
  // accompanying mode/preset field in the Home Assistant service call.
  if (call.get_state().has_value() && !*call.get_state()) {
    flags &= static_cast<uint8_t>(~FLAG_POWER);
    this->send_command_(flags, SPEED_LOW);
    return;
  }

  bool mode_requested = false;
  if (call.has_preset_mode()) {
    // "Auto" is the sole advertised preset and is validated by FanCall.
    flags |= FLAG_POWER;
    speed_code = COMMAND_AUTO;
    mode_requested = true;
  }
  if (call.get_speed().has_value()) {
    flags |= FLAG_POWER;
    switch (*call.get_speed()) {
      case 1:
        speed_code = SPEED_LOW;
        break;
      case 2:
        speed_code = SPEED_MEDIUM;
        break;
      case 3:
        speed_code = SPEED_HIGH;
        break;
      default:
        speed_code = SPEED_MAX;
        break;
    }
    mode_requested = true;
  }

  if (call.get_state().has_value() && *call.get_state()) {
    flags |= FLAG_POWER;
  }

  // A pure turn-on preserves Auto/manual mode. Never substitute a mode when
  // the valid reply contained an operating mode this component does not map.
  if (!mode_requested && !this->mode_is_known_(this->speed_status_)) {
    ESP_LOGW(TAG, "Ignoring purifier command: appliance reported an unknown speed status 0x%02X", this->speed_status_);
    return;
  }

  this->send_command_(flags, speed_code);
}

void AOIAPurifier::update_from_appliance(uint8_t flags, uint8_t speed_status) {
  this->flags_ = flags;
  this->speed_status_ = speed_status;
  this->appliance_state_known_ = true;

  this->state = (flags & FLAG_POWER) != 0;
  if (!this->state) {
    this->speed = 0;
    this->clear_preset_mode_();
  } else if (speed_status == STATUS_AUTO) {
    this->speed = 0;
    this->set_preset_mode_("Auto");
  } else {
    this->clear_preset_mode_();
    switch (speed_status) {
      case SPEED_LOW:
        this->speed = 1;
        break;
      case SPEED_MEDIUM:
        this->speed = 2;
        break;
      case SPEED_HIGH:
        this->speed = 3;
        break;
      case SPEED_MAX:
        this->speed = 4;
        break;
      default:
        // The power state is still known, but do not mislabel an unmapped mode.
        this->speed = 0;
        ESP_LOGW(TAG, "Appliance reported an unmapped speed status 0x%02X", speed_status);
        break;
    }
  }

  this->publish_state();
}

}  // namespace esphome::aoia_purifier
