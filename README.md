# ESP8266 Weather Clock - OLED Display with WiFi

ESP8266-based platform firmware featuring OLED display control, WiFi connectivity, OTA updates, and a Telnet console for remote debugging.

## Features

- 🖥️ **OLED Display Support**
  - Runtime-switchable drivers: SSD1306 / SH1106
  - Configurable X-offset for proper alignment
  - Two display modes: Status display & Boing ball demo
  
- 📡 **WiFi & Network**
  - WiFi connectivity with auto-reconnect
  - Web interface for configuration
  - OTA (Over-The-Air) firmware updates via ElegantOTA
  - Telnet console on port 23
  
- 🎨 **Display Modes**
  - **Status Mode**: Shows device info, IP, firmware version
  - **Boing Mode**: Animated bouncing ball with rotation and squash/stretch physics
  
- 🛠️ **Remote Management**
  - Web dashboard at `http://[device-ip]/`
  - OTA updates at `http://[device-ip]/update`
  - Telnet console for live debugging

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
- `mode status|boing` - Switch display mode
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

## Development

### Firmware Version
Current version: `platform-0.5.0-boing-auto`

### Architecture
- Main loop runs at ~1ms cycle time
- Boing mode: ~25 FPS (40ms per frame)
- Status mode: ~2.5 FPS (400ms refresh)
- WiFi status logged every second

## License

MIT License - Feel free to modify and use for your projects!

## Credits

Built with:
- [ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
- [U8g2](https://github.com/olikraus/u8g2)
- ESP8266 Arduino Core
