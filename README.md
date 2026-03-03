# ESP8266 OLED Screensaver

Modular ESP8266 firmware for 128×64 OLED displays with WiFi, OTA updates, and a Telnet console.

## Hardware

| OLED Pin | ESP8266 Pin |
|----------|-------------|
| SDA      | GPIO0       |
| SCL      | GPIO2       |
| VCC      | 3.3V        |
| GND      | GND         |

Supported boards: Wemos D1 Mini, NodeMCU, Generic ESP8266. Supported drivers: SSD1306, SH1106 (runtime-switchable).

## Quick Start

**Dev container (recommended):**

```bash
code .  # Click "Reopen in Container"
./build.sh esp8266_d1_mini
```

**Local:**

```bash
pip install platformio pre-commit
pre-commit install --hook-type commit-msg
cp secrets.h.template secrets.h  # Add WiFi credentials
./build.sh esp8266_d1_mini
```

## Display Modes

| Mode        | Description                               |
|-------------|-------------------------------------------|
| `status`    | Device info, IP address, firmware version |
| `boing`     | Bouncing ball with physics and spin       |
| `weather`   | Live temperature via Open-Meteo API       |
| `clock`     | NTP-synced digital clock with date        |
| `breakout`  | Self-playing brick breaker with AI paddle |
| `pacman`    | Self-playing Pac-Man with ghost AI        |
| `starfield` | Parallax star field animation             |
| `life`      | Conway's Game of Life                     |
| `sonic`     | Sonic the Hedgehog running animation      |

The `screensaver` mode cycles through all of the above automatically.

## Web Interface

Browse to `http://<device-ip>/` to switch modes, configure the OLED driver and X-offset, trigger OTA updates, and manage auto-update settings.

OTA upload: `http://<device-ip>/update` — credentials in `secrets.h`.

## Telnet Console

```
telnet <device-ip> 23
```

| Command | Description |
|---------|-------------|
| `help` | Command list |
| `status` | Device status |
| `mode <name>` | Switch mode (`status`, `boing`, `weather`, `clock`, `breakout`, `pacman`, `starfield`, `life`, `sonic`, `screensaver`) |
| `drv ssd1306\|sh1106` | Set OLED driver |
| `xoff <n>` | Set X offset |
| `oled on\|off` | Enable/disable display |
| `reboot` | Restart device |

## Architecture

```
src/
├── ESP8266-OLED-Experiment.ino  # Main sketch
├── Config.h / Config.cpp        # Hardware pins and runtime settings
├── Logger.h                     # Serial + Telnet logging
├── DisplayManager.h             # OLED driver lifecycle
├── NetworkManager.h             # WiFi, HTTP server, OTA
├── TelnetConsole.h              # Remote console
├── ModeHelper.h                 # Mode name → instance routing
├── DisplayMode.h                # Abstract base class
├── ScreensaverMode.h            # Cycles through all modes
├── StatusMode.h
├── BoingMode.h
├── WeatherMode.h
├── ClockMode.h
├── BreakoutMode.h
├── PacManMode.h
├── StarfieldMode.h
├── LifeMode.h
├── SonicMode.h
├── UpdateManager.h              # EEPROM-backed auto-update settings
├── UpdateChecker.h              # GitHub API version polling
└── AutoUpdater.h                # OTA download and flash
```

## Adding a Display Mode

1. Create `src/MyMode.h` implementing `DisplayMode`:

```cpp
#pragma once
#include "DisplayMode.h"

class MyMode : public DisplayMode {
 public:
  const char* getName() const override { return "mymode"; }
  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->drawStr(10, 30, "Hello!");
    u8g2->sendBuffer();
  }
};
```

2. Register in `ESP8266-OLED-Experiment.ino`, `ModeHelper.h`, `NetworkManager.h`, `TelnetConsole.h`, and `ScreensaverMode.h`.
3. Add tests in `test/test_DisplayModes/test_main.cpp`.

See [CONTRIBUTING.md](CONTRIBUTING.md) for full details.

## Auto-Update

The device can check GitHub Releases every 6 hours and flash new firmware automatically. Disabled by default — enable via the web interface or `curl http://<device-ip>/autoupdate?on=1`.

See [AUTO_UPDATE.md](AUTO_UPDATE.md).

## Recovery

See [RECOVERY.md](RECOVERY.md) if the device is unreachable.

## CI/CD

Pushes to `main` run linting, tests, and an embedded build check. Tagging `v*` triggers a release build that uploads `firmware.bin` to GitHub Releases.

## References

- [PlatformIO](https://docs.platformio.org/)
- [U8g2](https://github.com/olikraus/u8g2)
- [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- [Open-Meteo](https://open-meteo.com/)
