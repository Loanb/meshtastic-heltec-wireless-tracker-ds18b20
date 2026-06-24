# DS18B20 Text Commands for Meshtastic Heltec Wireless Tracker

## Overview

This repository provides a small documentation and patch package that adds DS18B20 temperature sensor support to Meshtastic on the Heltec Wireless Tracker.

The integration uses normal Meshtastic text messages. The node does not send temperature automatically. It answers only when it receives a command such as `/temp`.

This is not a full Meshtastic firmware fork. It only contains the files needed to reproduce the integration on top of the official Meshtastic firmware release.

## Tested Hardware

- Heltec Wireless Tracker
- PCB marking: `HTIT-Tracker V1.2`
- Meshtastic target: `heltec-wireless-tracker`
- Detected by Meshtastic as `Heltec Wireless Tracker V1.1`
- ESP32-S3
- One or more DS18B20 temperature sensors, 3-wire mode

## Wiring

| DS18B20 / component | Heltec Wireless Tracker |
| --- | --- |
| DS18B20 VDD | 3.3V |
| DS18B20 GND | GND |
| DS18B20 DATA | GPIO 5 |
| 4.7 kOhm resistor | Between DATA and 3.3V |

Multiple DS18B20 sensors can share the same 1-Wire DATA line on GPIO 5.

## Meshtastic Version

This patch was developed and tested with:

```text
v2.7.15.567b8ea
```

The patch is intended to be applied on top of the official Meshtastic firmware tag above.

## Repository Contents

```text
README.md
ds18b20-multi-sensor-text-commands.patch
logs/
  serial-log-example.png
```

## Applying the Patch

Clone the official Meshtastic firmware repository, check out the tested tag, initialize submodules, and apply the patch:

```bash
git clone https://github.com/meshtastic/firmware.git
cd firmware
git checkout v2.7.15.567b8ea
git submodule update --init --recursive
wget https://raw.githubusercontent.com/Loanb/meshtastic-heltec-wireless-tracker-ds18b20/ds18b20-working/ds18b20-multi-sensor-text-commands.patch
git apply ds18b20-multi-sensor-text-commands.patch
```

The patch modifies:

- `src/mesh/MeshService.cpp`
- `src/mesh/MeshService.h`
- `src/modules/TextMessageModule.cpp`
- `src/modules/TextMessageModule.h`
- `variants/esp32s3/heltec_wireless_tracker/platformio.ini`

## Compiling

Build the Heltec Wireless Tracker PlatformIO environment:

```bash
pio run -e heltec-wireless-tracker
```

## Flashing

Flash this file:

```text
.pio/build/heltec-wireless-tracker/firmware.bin
```

Do not flash `firmware.factory.bin` for this use case. Use `firmware.bin`.

## Text Commands

Send these commands as normal Meshtastic text messages. The node answers on demand only.

### /list

Scans the 1-Wire bus and returns each detected DS18B20 sensor with:

- a temporary index;
- the full 64-bit ROM address in hexadecimal;
- the saved name, or `unassigned`.

Example:

```text
1) 28FF641D9716035C -> 1m
2) 28FFA27B96160391 -> 2m
3) 28FF0C88951603A7 -> unassigned
```

The index is temporary and comes from the latest `/list` result.

### /register <index|address> <name>

Saves a sensor name in ESP32 NVS/Preferences.

The sensor identifier can be either:

- the temporary index from the latest `/list`;
- the full ROM address.

Examples:

```text
/register 1 1m
/register 28FF641D9716035C surface
```

Name rules:

- maximum 16 characters;
- no spaces;
- allowed characters: letters, numbers, `-`, `_`.

### /temp

Reads all registered sensors.

Example:

```text
1m : 12.4 °C
2m : 11.8 °C
```

If a registered sensor is not currently detected:

```text
1m : not detected
```

### /temp <name|address|index>

Reads only one sensor.

Examples:

```text
/temp 1m
/temp surface
/temp 28FF641D9716035C
/temp 1
```

If the argument is an index, it refers to the latest `/list` result.

### /clear_memory

Clears all saved sensor names from ESP32 NVS/Preferences.

Response:

```text
Sensor memory cleared.
Use /list then /register <index> <name>.
```

## Notes / Limitations

- GPIO 5 is currently hardcoded.
- The integration is limited to `HELTEC_TRACKER_V1_1`.
- This is not yet a generic implementation ready for official upstream inclusion in Meshtastic.
- The feature uses normal Meshtastic text messages, not Environment Telemetry.
- Temperature is never sent automatically.
- Replies are sent through the mesh without the extra local `toPhone` queue status notification used by normal local sends.
- A simple 5-second throttle prevents repeated command spam.
