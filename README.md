# ESP8266 Weather Clock - OLED Display with WiFi

ESP8266-based modular platform firmware featuring extensible OLED display modes, WiFi connectivity, OTA updates, and remote debugging capabilities.

## Features

- 🖥️ **OLED Display Support**
  - Runtime-switchable drivers: SSD1306 / SH1106
  - Configurable X-offset for proper alignment
  - Modular, pluggable display modes
  
- 📡 **WiFi & Network**
  - WiFi connectivity with auto-reconnect
  - Web interface for configuration
  - OTA (Over-The-Air) firmware updates via ElegantOTA
  - Telnet console on port 23
  
- 🎨 **Display Modes** (Extensible!)
  - **Status Mode**: Shows device info, IP, firmware version
  - **Boing Mode**: Animated bouncing ball with rotation and squash/stretch physics
  - **Weather Mode**: Placeholder for weather forecast display (TODO)
  - Easy to add new custom modes!
  
- 🛠️ **Remote Management**
  - Web dashboard at `http://[device-ip]/`
  - OTA updates at `http://[device-ip]/update`
  - Telnet console for live debugging

- 🏗️ **Clean Architecture**
  - Modular design following single responsibility principle
  - Easy to understand, maintain, and extend
  - Pluggable display mode system

## Hardware

### Required Components
- ESP8266 board (NodeMCU, Wemos D1 Mini, etc.)
- 128x64 OLED display (SSD1306 or SH1106)
- Connecting wires

### Wiring

| OLED Pin | ESP8266 Pin |
|----------|-------------|
| SDA      | GPIO0       |
| SCL      | GPIO2       |
| VCC      | 3.3V        |
| GND      | GND         |

**I2C Address**: 0x3C (configurable at compile time)

## Setup

### 1. Install Dependencies

Install the following libraries via Arduino IDE Library Manager:

- ESP8266WiFi (included with ESP8266 board package)
- ESP8266WebServer (included with ESP8266 board package)
- ElegantOTA
- U8g2lib

### 2. Configure Secrets

Copy the template and add your credentials:

```bash
cp secrets.h.template secrets.h
```

Edit `secrets.h` with your WiFi credentials and OTA password:

```cpp
namespace secrets {
  static constexpr const char* WIFI_SSID = "YourWiFiSSID";
  static constexpr const char* WIFI_PASS = "YourWiFiPassword";
  static constexpr const char* OTA_USER  = "admin";
  static constexpr const char* OTA_PASS  = "SecurePassword123";
}
```

**⚠️ Important**: Never commit `secrets.h` to version control!

### 3. Upload Firmware

1. Open `OA_OLED_Display_with_wifi_working.ino` in Arduino IDE
2. Select your ESP8266 board and port
3. Click Upload

### 4. Find Your Device

Check Serial Monitor (115200 baud) for the assigned IP address, or check your router's DHCP client list for device hostname: `esp-weather-clock`

## Usage

### Web Interface

Navigate to `http://[device-ip]/` to access:
- Device status and information
- Mode switching (Boing / Status)
- OLED configuration (driver type, X offset)
- OTA update interface
- OLED on/off toggle

### Telnet Console

Connect via telnet for live debugging:

```bash
telnet [device-ip] 23
```

Available commands:
- `help` - Show command list
- `status` - Display device status
- `drv ssd1306|sh1106` - Switch OLED driver
- `xoff <int>` - Set X offset (e.g., `xoff 0` or `xoff 2`)
- `mode status|boing|weather` - Switch display mode
- `oled on|off` - Enable/disable OLED
- `reboot` - Restart the device

### OTA Updates

1. Navigate to `http://[device-ip]/update`
2. Login with credentials from `secrets.h`
3. Upload new `.bin` firmware file

## Configuration

### OLED Driver & Alignment

If your display shows misaligned content:

1. Try different driver/offset combinations via web interface
2. Common configurations:
   - **SH1106**: Usually needs `xoff=2`
   - **SSD1306**: Usually needs `xoff=0`
3. Use Status mode to verify alignment (border should be perfect)

### Customization

Edit these constants in the `.ino` file:

```cpp
static constexpr const char* HOSTNAME   = "esp-weather-clock";
static constexpr const char* FW_VERSION = "platform-0.5.0-boing-auto";
```

## Troubleshooting

**Display not working?**
- Check I2C wiring (SDA, SCL)
- Try both drivers: SSD1306 and SH1106
- Adjust X offset via web interface or Telnet

**WiFi not connecting?**
- Verify credentials in `secrets.h`
- Check Serial Monitor for connection status
- Ensure 2.4GHz WiFi (ESP8266 doesn't support 5GHz)

**Can't access web interface?**
- Check Serial Monitor for IP address
- Verify device is on same network
- Try pinging the device

## Architecture

The codebase follows a modular architecture with clear separation of concerns:

```
OA_OLED_Display_with_wifi_working/
├── OA_OLED_Display_with_wifi_working.ino  # Main entry point (setup/loop)
├── Config.h / Config.cpp                   # Configuration and constants
├── Logger.h                                # Logging utility
├── DisplayManager.h                        # OLED display management
├── NetworkManager.h                        # WiFi and HTTP server
├── TelnetConsole.h                         # Telnet remote console
├── DisplayMode.h                           # Abstract base class for modes
├── StatusMode.h                            # Status display implementation
├── BoingMode.h                             # Boing animation implementation
├── WeatherMode.h                           # Weather forecast (placeholder)
├── secrets.h.template                      # Credentials template
└── secrets.h                               # Your actual credentials (gitignored)
```

### Core Components

- **Config**: Centralized configuration (hardware pins, network settings, runtime config)
- **Logger**: Unified logging to Serial and Telnet
- **DisplayManager**: Manages OLED hardware and display mode lifecycle
- **NetworkManager**: Handles WiFi connection, HTTP server, and OTA updates
- **TelnetConsole**: Remote command-line interface
- **DisplayMode**: Abstract interface that all display modes implement

### Adding New Display Modes

Creating a new display mode is simple:

1. **Create a new header file** (e.g., `ClockMode.h`)
2. **Inherit from `DisplayMode`**
3. **Implement required methods**:
   - `getName()` - Return mode name
   - `update(U8G2* display, uint32_t deltaMs)` - Render your content
   - Optional: `begin()` and `end()` for mode lifecycle

Example:

```cpp
#pragma once
#include "DisplayMode.h"
#include "DisplayManager.h"

class ClockMode : public DisplayMode {
public:
  const char* getName() const override {
    return "clock";
  }
  
  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_10x20_tf);
    
    // Draw your content here
    DisplayManager::drawStr(u8g2, 10, 30, "12:34:56");
    
    u8g2->sendBuffer();
  }
};
```

4. **Register your mode** in the main `.ino` file:
   - Instantiate it
   - Add it to `NetworkManager` and `TelnetConsole`
   - Users can now switch to it via web or telnet!

### Performance

- Main loop: ~1ms cycle time
- Boing mode: ~25 FPS (40ms per frame)
- Status mode: ~2.5 FPS (400ms refresh)
- Weather mode: 5 second refresh (configurable)
- WiFi status logged every second

### Firmware Version
Current version: `platform-1.0.0-modular`

## License

MIT License - Feel free to modify and use for your projects!

## Credits

Built with:
- [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- [U8g2](https://github.com/olikraus/u8g2)
- ESP8266 Arduino Core
