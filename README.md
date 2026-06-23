<div align="center" markdown="1">

<img src=".github/meshtastic_logo.png" alt="Meshtastic Logo" width="80"/>
<h1>Meshtastic Firmware</h1>

![GitHub release downloads](https://img.shields.io/github/downloads/meshtastic/firmware/total)
[![CI](https://img.shields.io/github/actions/workflow/status/meshtastic/firmware/main_matrix.yml?branch=master&label=actions&logo=github&color=yellow)](https://github.com/meshtastic/firmware/actions/workflows/ci.yml)
[![CLA assistant](https://cla-assistant.io/readme/badge/meshtastic/firmware)](https://cla-assistant.io/meshtastic/firmware)
[![Fiscal Contributors](https://opencollective.com/meshtastic/tiers/badge.svg?label=Fiscal%20Contributors&color=deeppink)](https://opencollective.com/meshtastic/)
[![Vercel](https://img.shields.io/static/v1?label=Powered%20by&message=Vercel&style=flat&logo=vercel&color=000000)](https://vercel.com?utm_source=meshtastic&utm_campaign=oss)

<a href="https://trendshift.io/repositories/5524" target="_blank"><img src="https://trendshift.io/api/badge/repositories/5524" alt="meshtastic%2Ffirmware | Trendshift" style="width: 250px; height: 55px;" width="250" height="55"/></a>

</div>

</div>

<div align="center">
	<a href="https://meshtastic.org">Website</a>
	-
	<a href="https://meshtastic.org/docs/">Documentation</a>
</div>

## DS18B20 Temperature Commands for Heltec Wireless Tracker

This branch adds DS18B20 temperature support for the Meshtastic `heltec-wireless-tracker` target. It is intended for a Heltec Wireless Tracker detected by Meshtastic as `Heltec Wireless Tracker V1.1`, with one or more DS18B20 sensors connected on GPIO 5.

### Tested Hardware

- Heltec Wireless Tracker
- PCB marking: `HTIT-Tracker V1.2`
- Meshtastic target: `heltec-wireless-tracker`
- Detected by Meshtastic as `Heltec Wireless Tracker V1.1`
- ESP32-S3
- DS18B20 temperature sensor, 3-wire wiring

### Wiring

| DS18B20 | Heltec Wireless Tracker |
| --- | --- |
| VDD | 3.3V |
| GND | GND |
| DATA | GPIO 5 |

Add a 4.7 kOhm pull-up resistor between DATA and 3.3V.

### Install From This Branch

```bash
git clone https://github.com/Loanb/meshtastic-heltec-wireless-tracker-ds18b20.git
cd meshtastic-heltec-wireless-tracker-ds18b20
git submodule update --init --recursive
pio run -e heltec-wireless-tracker
```

Flash the generated firmware image:

```text
.pio/build/heltec-wireless-tracker/firmware.bin
```

Use `firmware.bin` for an update flash. Do not use `firmware.factory.bin` unless you intentionally want a full factory image.

### Apply As A Patch

The patch file is generated against Meshtastic `v2.7.15.567b8ea`:

```bash
git clone https://github.com/meshtastic/firmware.git
cd firmware
git checkout v2.7.15.567b8ea
git submodule update --init --recursive
git apply ds18b20-multi-sensor-text-commands.patch
pio run -e heltec-wireless-tracker
```

### Text Commands

Send these commands as normal Meshtastic text messages. The node answers only when a command is received; it does not send temperature automatically.

```text
/list
```

Scans the 1-Wire bus and returns each detected sensor with a temporary index, its ROM address, and the saved name if one exists.

```text
1) 28FF641D9716035C -> 1m
2) 28FFA27B96160391 -> unassigned
```

```text
/register <index|address> <name>
```

Saves a DS18B20 ROM address to a user name in ESP32 NVS/Preferences. Names are limited to 16 characters and may contain letters, numbers, `-`, and `_`.

```text
/register 1 1m
/register 28FF641D9716035C surface
```

```text
/temp
```

Reads all registered sensors:

```text
1m : 12.4 °C
surface : 11.8 °C
```

If a registered sensor is missing, the response is:

```text
1m : not detected
```

```text
/temp <name|address|index>
```

Reads one sensor by saved name, full ROM address, or the temporary index from the latest `/list`.

```text
/temp 1m
/temp surface
/temp 28FF641D9716035C
/temp 1
```

```text
/clear_memory
```

Clears all saved sensor names from NVS/Preferences.

### Notes

- DS18B20 DATA is currently hardcoded to GPIO 5.
- The feature is limited to `HELTEC_TRACKER_V1_1`.
- Responses are sent as Meshtastic text messages on the same channel as the request, or directly back to the requester when the incoming packet is direct/private.
- Responses are sent to the mesh without the extra local `toPhone` queue status notification used by normal local sends.
- A simple 5-second throttle prevents command spam.

## Overview

This repository contains the official device firmware for Meshtastic, an open-source LoRa mesh networking project designed for long-range, low-power communication without relying on internet or cellular infrastructure. The firmware supports various hardware platforms, including ESP32, nRF52, RP2040/RP2350, and Linux-based devices.

Meshtastic enables text messaging, location sharing, and telemetry over a decentralized mesh network, making it ideal for outdoor adventures, emergency preparedness, and remote operations.

### Get Started

- 🔧 **[Building Instructions](https://meshtastic.org/docs/development/firmware/build)** – Learn how to compile the firmware from source.
- ⚡ **[Flashing Instructions](https://meshtastic.org/docs/getting-started/flashing-firmware/)** – Install or update the firmware on your device.

Join our community and help improve Meshtastic! 🚀

## Stats

![Alt](https://repobeats.axiom.co/api/embed/8025e56c482ec63541593cc5bd322c19d5c0bdcf.svg "Repobeats analytics image")
