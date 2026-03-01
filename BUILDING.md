# Building and Deployment

## Quick Start

### Development Container (Recommended)

```bash
code .
# Click "Reopen in Container"
./build.sh esp8266_d1_mini
```

See [.devcontainer/README.md](.devcontainer/README.md) for details.

### Local Installation

```bash
pip install platformio
cp secrets.h.template secrets.h
# Edit secrets.h with WiFi credentials
./build.sh esp8266_d1_mini
```

## Build Environments

| Environment | Board | Flash | Notes |
|---|---|---|---|
| `esp8266_d1_mini` | Wemos D1 Mini | 4MB | Recommended |
| `esp8266_nodmcu` | NodeMCU v2 | 4MB | Common |
| `esp8266_generic` | Generic ESP8266 | 1MB | Small flash |

## Building Firmware

```bash
# Build with script (all platforms)
./build.sh esp8266_d1_mini

# Build with PlatformIO
pio run -e esp8266_d1_mini

# Build all environments
for env in esp8266_d1_mini esp8266_nodmcu esp8266_generic; do
  ./build.sh $env
done
```

Output binaries are in `firmware/` with version and timestamp.

### PlatformIO Commands

```bash
# Build only
pio run -e esp8266_d1_mini

# Build and upload via serial
pio run -e esp8266_d1_mini -t upload

# Build and upload via OTA
pio run -e esp8266_d1_mini -t upload --upload-port=esp-weather-clock.local

# Monitor serial
pio device monitor -b 115200

# Run tests
pio test --without-uploading

# Verbose output
pio run -e esp8266_d1_mini -v

# Clean
pio run -e esp8266_d1_mini -t clean
```

## Flashing Firmware

### Method 1: OTA Update (Recommended)

1. Navigate to `http://<device-ip>/update`
2. Login with credentials from `secrets.h`
3. Select `.bin` file from `firmware/` directory
4. Wait for reboot (30-60 seconds)

### Method 2: Serial Upload (Initial Flash)

```bash
# PlatformIO
pio run -e esp8266_d1_mini -t upload

# Arduino IDE
1. Open OA_OLED_Display_with_wifi_working.ino
2. Select board and port
3. Sketch → Upload
```

### Method 3: Esptool

```bash
pip install esptool
esptool.py --port /dev/ttyUSB0 write_flash -fm dout -fs 4MB -ff 40m 0x0 firmware.bin
```

## Debugging

**Serial Monitor**:

```bash
pio run -e esp8266_d1_mini -t upload && pio device monitor
```

**Telnet Console** (if device is on network):

```bash
telnet <device-ip> 23
help
status
```

**Build Troubleshooting**:

```bash
# Check environment
pio run -e esp8266_d1_mini --target envdump

# Verbose output
pio run -e esp8266_d1_mini -v
```

## GitHub Actions CI/CD

Pushes to `main` run the quality pipeline (linting, native tests, and embedded build verification).

Releases are tag-driven:

- Push tag `v*` or `release-*` to run release build/publish workflow
- Or run `Continuous Deployment` manually via `workflow_dispatch`

For releases, create a git tag:

```bash
git tag v1.0.0
git push origin v1.0.0
```

This creates a GitHub Release with binaries.
