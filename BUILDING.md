# Building and Deployment

This guide covers building firmware binaries and deploying them to your ESP8266 device via OTA updates.

## Quick Start with Development Container

**Recommended for new developers** - Get started in minutes with zero manual installation.

Prerequisites: Docker and VS Code with Remote Containers extension

```bash
# 1. Open in VS Code
code .

# 2. Click "Reopen in Container"
# 3. All tools are ready!

./build.sh                    # Build firmware
pio test --without-uploading  # Run tests
```

See [.devcontainer/README.md](.devcontainer/README.md) for complete devcontainer guide.

## Manual Local Setup

If you prefer local installation without containers, follow the steps below.

## Build Environments

The project supports multiple ESP8266 boards via environment configurations in `platformio.ini`:

| Environment | Board | Flash | Best For |
|---|---|---|---|
| `esp8266_d1_mini` | Wemos D1 Mini | 4MB | ✅ Recommended |
| `esp8266_nodmcu` | NodeMCU v2 | 4MB | Common choice |
| `esp8266_generic` | Generic ESP8266 | 1MB | Memory-constrained |

## Prerequisites

### Option 1: PlatformIO (Recommended)

Install PlatformIO:

```bash
pip install platformio
```

### Option 2: Arduino IDE

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP8266 board support via Board Manager
3. Install required libraries (U8g2, ElegantOTA)

## Building Firmware

### Using Build Script (Linux/Mac)

```bash
# Default environment (esp8266_d1_mini)
./build.sh

# Specific environment
./build.sh esp8266_nodmcu

# All environments
for env in esp8266_d1_mini esp8266_nodmcu esp8266_generic; do
  ./build.sh $env
done
```

Output binaries go to `firmware/` directory with version and timestamp.

### Using Build Script (Windows)

```bash
# Default environment
build.bat

# Specific environment
build.bat esp8266_nodmcu
```

### Using PlatformIO CLI

```bash
# Build (generates firmware in .pio/build/)
pio run -e esp8266_d1_mini

# Build and upload via serial
pio run -e esp8266_d1_mini -t upload

# Build and upload via OTA
pio run -e esp8266_d1_mini -t upload --upload-port=esp-weather-clock.local

# Monitor serial output
pio device monitor -b 115200
```

### Using Arduino IDE

1. Open `OA_OLED_Display_with_wifi_working.ino`
2. Select Tools → Board → ESP8266 → Wemos D1 Mini
3. Sketch → Export compiled Binary
4. Binary saved as `OA_OLED_Display_with_wifi_working.ino.d1_mini.bin`

## Binary Files

After building, you'll have firmware binaries ready for OTA upload:

```
firmware/
├── esp8266-weather-clock_0.5.0_esp8266_d1_mini_20240301_143022.bin
├── latest_esp8266_d1_mini.bin -> esp8266-weather-clock_0.5.0_esp8266_d1_mini_20240301_143022.bin
└── build_info.txt
```

### File Naming Convention

```
esp8266-weather-clock_<VERSION>_<BOARD>_<TIMESTAMP>.bin
```

- `VERSION`: From `FW_VERSION` in Config.h (e.g., `0.5.0`)
- `BOARD`: Environment/board name
- `TIMESTAMP`: Build date/time for traceability

## Flashing Firmware

### Method 1: OTA Update (Recommended)

Requires device to be connected to WiFi with OTA enabled.

1. **Find device IP**:
   - Check serial monitor for printed IP
   - Or check router DHCP clients for `esp-weather-clock`

2. **Navigate to update page**:
   ```
   http://<device-ip>/update
   ```

3. **Upload firmware**:
   - Click "Choose File"
   - Select `.bin` file from `firmware/` directory
   - Login with credentials from `secrets.h`
   - Click Upload
   - Wait for reboot (30-60 seconds)

4. **Verify**:
   - Check device boots successfully
   - Access web interface at `http://<device-ip>/`

### Method 2: Serial Upload (Initial Flash)

Use for first-time flash when device has no WiFi credentialsyet.

```bash
# PlatformIO
pio run -e esp8266_d1_mini -t upload

# Arduino IDE
Sketch → Upload (with board selected)
```

### Method 3: Esptool (Advanced)

```bash
# Install esptool
pip install esptool

# Flash firmware
esptool.py --port /dev/ttyUSB0 write_flash -fm dout -fs 4MB -ff 40m 0x0 firmware.bin
```

## Automated Builds (GitHub Actions)

### Every Push/PR

CI/CD automatically:
- ✅ Builds firmware for all boards
- ✅ Lints C++ code
- ✅ Runs unit tests
- ✅ Creates build artifacts

Access artifacts from GitHub Actions run:
1. Go to Actions tab
2. Select workflow run
3. Download "firmware-*" artifacts

### Release Builds

Create a release by pushing a tag:

```bash
git tag v1.0.0
git push origin v1.0.0
```

This triggers:
- ✅ Builds firmware for all boards
- ✅ Creates GitHub Release
- ✅ Uploads binaries as release assets
- ✅ Generates checksums for verification

Download from: https://github.com/milesburton/esp8266-weather-clock/releases

## Firmware Updates

### OTA Update Strategy

1. **Test on development device first**
2. **Create backup** (download current firmware)
3. **Perform update** via OTA page
4. **Monitor logs** via telnet after update

### Rolling Back

If update fails or device doesn't boot:

1. **Serial recovery** (erase and reflash):
   ```bash
   esptool.py erase_flash
   esptool.py write_flash -fm dout -fs 4MB -ff 40m 0x0 <previous-firmware.bin>
   ```

2. **Factory reset** via telnet:
   ```
   reboot
   ```

## Debugging

### Serial Monitor

Monitor device output during/after build:

```bash
# With build
pio run -e esp8266_d1_mini -t upload && pio device monitor

# Standalone
pio device monitor -b 115200 -p /dev/ttyUSB0
```

### Telnet Console

Connect to remote console for live debugging:

```bash
telnet <device-ip> 23

# Or from inside Telnet:
help
status
```

### Build Troubleshooting

```bash
# Verbose build output
pio run -e esp8266_d1_mini -v

# Clean before rebuild
pio run -e esp8266_d1_mini --target clean
pio run -e esp8266_d1_mini

# Check environment config
pio run -e esp8266_d1_mini --target envdump
```

## Performance Optimization

For production builds with minimal debug info:

```bash
# Optimize for size
pio run -e esp8266_d1_mini --target release
```

## Verification

Verify binary integrity using checksums:

```bash
# SHA256 checksums generated automatically
sha256sum -c esp8266-weather-clock_0.5.0_esp8266_d1_mini.bin.sha256

# Manual verification
sha256sum esp8266-weather-clock_0.5.0_esp8266_d1_mini.bin
```

## Next Steps

- [Setting up OTA updates](README.md#features)
- [Development workflow](CONTRIBUTING.md)
- [Telnet debugging](README.md#telnet-console)
