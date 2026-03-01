# ESP8266-OLED-Experiment

Modular ESP8266 firmware for 128x64 OLED displays with extensible display modes, WiFi connectivity, OTA updates, and remote debugging via Telnet.

## Features

**OLED Display**

- Runtime-switchable drivers (SSD1306 / SH1106)
- Configurable X-offset for alignment
- Pluggable display mode system

**Networking**

- WiFi connectivity with auto-reconnect
- Web interface for configuration and OTA updates
- Telnet console (port 23) for remote debugging
- Multi-board support: Wemos D1 Mini, NodeMCU, Generic ESP8266

**Display Modes** (Extensible)

- Status: Device info, IP address, firmware version
- Boing: Animated bouncing ball with rotation and physics
- Weather: Placeholder for weather API integration

**Development**

- Modular architecture with single responsibility principle
- Pre-commit hooks (clang-format, cpplint, conventional commits)
- Automated tests and CI/CD
- Development container for zero-config setup

## Hardware

**Required**

- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- 128x64 OLED display (SSD1306 or SH1106)

**Wiring**

| OLED Pin | ESP8266 Pin |
|----------|-------------|
| SDA      | GPIO0       |
| SCL      | GPIO2       |
| VCC      | 3.3V        |
| GND      | GND         |

I2C Address: 0x3C (configurable at compile time)

## Quick Start

### Download Pre-built Firmware (Easiest)

1. **Check your ESP8266 flash size**:
   - Most NodeMCU and Wemos D1 Mini boards have **4MB** flash
   - Some generic ESP8266 modules have **1MB** flash

2. **Download the correct binary** from [Releases](https://github.com/milesburton/esp8266-oled-experiment/releases):
   - `esp8266-oled-experiment_vX.X.X_esp8266_d1_mini.bin` - For Wemos D1 Mini (4MB)
   - `esp8266-oled-experiment_vX.X.X_esp8266_nodmcu.bin` - For NodeMCU (4MB)
   - `esp8266-oled-experiment_vX.X.X_esp8266_generic.bin` - For generic ESP8266 (1MB)

3. **Flash via OTA** or use `esptool.py` for initial upload

4. **First Boot - Setup Mode**:
   After flashing, the device starts in **AP (Access Point) setup mode** by default:
   - Look for WiFi network: `esp-oled-setup` (password: `setup1234`)
   - Open your browser to: `http://192.168.4.1/`
   - Configure your WiFi credentials
   - Device will reboot and connect to your network

**Important**: If you see `ERROR[9]: new Flash config wrong, real size: 1048576` when uploading, your device has 1MB flash. Use the **generic** binary instead.

**Device won't connect?** See [RECOVERY.md](RECOVERY.md) for troubleshooting and recovery options.

### With Development Container (Recommended)

```bash
# Prerequisites: Docker and VS Code with Remote Containers extension
code .
# Click "Reopen in Container"
./build.sh esp8266_d1_mini
```

See [.devcontainer/README.md](.devcontainer/README.md) for full details.

### Local Installation

```bash
pip install platformio pre-commit
pre-commit install --hook-type commit-msg

# Configure credentials
cp secrets.h.template secrets.h
# Edit secrets.h with WiFi credentials

# Build
./build.sh esp8266_d1_mini

# Upload via OTA
# Navigate to http://<device-ip>/update
```

## Building

Supported environments in `platformio.ini`:

- `esp8266_d1_mini` (4MB) - Recommended
- `esp8266_nodmcu` (4MB)
- `esp8266_generic` (1MB)

```bash
# Build for specific board
./build.sh esp8266_nodmcu

# Build and test
pio run -e esp8266_d1_mini
pio test --without-uploading

# Monitor serial output
pio device monitor -b 115200
```

Output binaries are written to `firmware/` directory.

See [BUILDING.md](BUILDING.md) for complete build guide.

## Usage

**Web Interface**: `http://<device-ip>/`

- Device status
- Mode switching
- OLED configuration
- OTA firmware updates

**Telnet Console**: `telnet <device-ip> 23`

- `help` - Command list
- `status` - Device status
- `mode status|boing|weather` - Switch display mode
- `drv ssd1306|sh1106` - Set OLED driver
- `xoff <int>` - Set X offset (e.g., `xoff 0`)
- `oled on|off` - Enable/disable display
- `reboot` - Restart device

**OTA Updates**: `http://<device-ip>/update`

- Login with credentials from `secrets.h`
- Upload new `.bin` firmware file

## Architecture

```
src/
├── ESP8266-OLED-Experiment.ino            # Main sketch (~60 lines)
├── Config.h / Config.cpp                  # Configuration & constants
├── Logger.h                               # Unified logging
├── DisplayManager.h                       # OLED management
├── NetworkManager.h                       # WiFi & HTTP server
├── TelnetConsole.h                        # Remote console
├── DisplayMode.h                          # Mode base class
├── StatusMode.h                           # Status display
├── BoingMode.h                            # Animation demo
└── WeatherMode.h                          # Weather placeholder

test/
├── unit/                                  # AUnit tests
│   ├── test_Config/
│   ├── test_Logger/
│   └── test_DisplayModes/
└── integration/                           # Coming soon
```

### Core Components

- **Config**: Hardware pins, runtime settings, OLED driver selection
- **Logger**: Serial + Telnet logging
- **DisplayManager**: OLED driver lifecycle and mode switching
- **NetworkManager**: WiFi, HTTP server, OTA
- **TelnetConsole**: Remote debugging interface
- **DisplayMode**: Abstract base for all display implementations

## Adding Display Modes

Create a new header file inheriting from `DisplayMode`:

```cpp
#pragma once
#include "DisplayMode.h"

class MyMode : public DisplayMode {
public:
  const char* getName() const override {
    return "mymode";
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_10x20_tf);
    u8g2->drawStr(10, 30, "Hello!");
    u8g2->sendBuffer();
  }
};
```

Register in the main `.ino` file and expose via web/Telnet interfaces.

## Development

Code standards enforced by pre-commit hooks:

- **C++ Formatting**: clang-format (Google style, 100 char lines)
- **Linting**: cpplint
- **Commits**: Conventional commits format
- **Testing**: AUnit framework

Run checks manually:

```bash
pre-commit run --all-files
pio test --without-uploading
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## CI/CD Trigger Policy

- Pushes to `main` run quality checks (linting, native tests, embedded build verification)
- Releases are created from tags (`v*` / `release-*`) or manual deployment workflow dispatch

## Configuration

Edit compile-time constants in `src/Config.h`:

- OLED SDA/SCL pins (GPIO0, GPIO2)
- WiFi hostname
- HTTP/Telnet ports
- Default driver and X-offset

Runtime configuration via web interface or Telnet console:

- OLED driver type
- X-offset for alignment
- Display enable/disable
- Active display mode

## Performance

- Main loop: ~1ms cycle time
- Boing mode: ~25 FPS
- Status mode: ~2.5 FPS
- WiFi status: logged every second

## References

- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [U8g2 Graphics Library](https://github.com/olikraus/u8g2)
- [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- [Conventional Commits](https://www.conventionalcommits.org/)
