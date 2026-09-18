# AirOasis ESPHome replacement

Unofficial local ESPHome firmware for the ESP32 Wi-Fi controller in selected AirOasis AOIA-2-series purifiers. It replaces the OEM controller firmware and communicates with the purifier over its internal UART; it does not use the AirOasis cloud.

> [!WARNING]
> Initial installation overwrites OEM firmware and can disable official app/cloud control until an OEM backup is restored. This is hardware modification work. Back up flash first, use 3.3 V only, and proceed at your own risk.

## Status

- **AOIA-2M:** supported.
- **AOIA-2L:** basic controls and telemetry tested successfully.
- Other AirOasis appliances using the same AOR-BC-100 Rev 1.0.2 controller are likely compatible, but untested.

## Features

- Local Home Assistant API and OTA updates
- Fan power and Auto / Low / Medium / High / Max modes
- UV, ionizer, and Night/child-lock controls
- Reply-reconciled state: commands are not published as state until the purifier replies with a valid frame
- Local PM2.5 and filter-life sensors on AOIA-2M

## Quick start

1. Read [HARDWARE.md](HARDWARE.md) to access and remove the controller.
2. Read [FLASHING.md](FLASHING.md) completely.
3. Copy and edit the example secrets file:
   ```sh
   cp secrets.yaml.example secrets.yaml
   chmod 600 secrets.yaml
   ```
4. Use `aoia-2.yaml`. If installing more than one controller, duplicate it and give each copy a unique `device_name`.
5. Validate, back up the original flash, then perform the initial serial upload as described in [FLASHING.md](FLASHING.md).
6. Remove the FTDI adapter before reconnecting the purifier connector, then add the ESPHome device to Home Assistant.

## UART protocol implemented

The appliance UART is 9600 baud, 8N1 on ESP32 `GPIO17` (TX) and `GPIO16` (RX). Commands are 12-byte `BB … 44` frames; replies are 16-byte `AC … 44` frames. Both use an 8-bit additive checksum. The component only accepts a fixed-length, checksum-valid reply and preserves the complete returned flag bitmap when sending a command.

Known response fields: flags at byte 1, speed/status at byte 2, PM2.5 at byte 5, and the big-endian filter counter at bytes 9–10. Do not assign meanings to other bytes without independent verification.

## Repository layout

- `aoia-2.yaml` — configuration for a compatible AOIA-2-series controller
- `components/aoia_purifier/` — reply-reconciled ESPHome external component
- `secrets.yaml.example` — safe template; live `secrets.yaml` is ignored
- `HARDWARE.md` — controller access and removal photos
- `FLASHING.md` — programming header, backup, install, OTA, and restoration instructions
