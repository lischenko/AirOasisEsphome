# Accessing the controller

These photos show the controller-access sequence for a compatible AOIA-2-series purifier. Cabinet details can vary by model.

> [!WARNING]
> Unplug the purifier before opening it. The controller is powered from J1; do not connect an FTDI adapter until J1 is disconnected. Continue with [FLASHING.md](FLASHING.md) only after the controller is removed.

## Open the upper panel

Remove the upper panel to reach the Wi-Fi access area.

| | |
|---|---|
| ![Upper control panel](docs/images/01-control-panel.jpg) | ![Lifted upper panel](docs/images/02-lift-panel.jpg) |

## Expose the controller

Remove the Wi-Fi access cover, then disconnect J1 and remove the controller.

| | | |
|---|---|---|
| ![Removing Wi-Fi cover](docs/images/03-remove-wifi-cover.jpg) | ![Wi-Fi access opening](docs/images/04-lift-wifi-cover.jpg) | ![Controller removal](docs/images/05-remove-controller.jpg) |

## Identify the programming connection

The controller has the appliance connector J1 and a separate unpopulated 2×3 programming header. The component-side and reverse-side views below help locate and populate the header.

| | |
|---|---|
| ![Controller component side](docs/images/06-controller-front.jpg) | ![Populate programming header](docs/images/07-populate-programming-header.jpg) |

The pinout, FTDI wiring, backup, and flashing procedure are in [FLASHING.md](FLASHING.md).
