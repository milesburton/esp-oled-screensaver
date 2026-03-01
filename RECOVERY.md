# Device Recovery Guide

## Device Won't Connect to WiFi

If your device appears offline and you can't access the web interface, follow these recovery steps.

### Option 1: Use Access Point (AP) Setup Mode (Recommended)

If the device was flashed with a pre-built release binary without WiFi credentials:

1. **Find the device's access point**:
   - Look for a WiFi network named: `esp-oled-setup`
   - Password: `setup1234`

2. **Connect to the AP**:

   ```bash
   # macOS/Linux
   wifi connect esp-oled-setup

   # Windows: Use WiFi settings to connect to esp-oled-setup
   ```

3. **Open web interface**:
   - Navigate to: `http://192.168.4.1/`
   - Configure your WiFi network credentials

4. **Restart device**:
   - The device will reboot and connect to your WiFi

### Option 2: Serial Flash with Custom Credentials (Advanced)

If AP mode doesn't work or you need to flash again:

1. **Clone or download the repository**:

   ```bash
   git clone https://github.com/milesburton/esp8266-oled-experiment.git
   cd esp8266-oled-experiment
   ```

2. **Configure credentials**:

   ```bash
   cp secrets.h.template src/secrets.h
   # Edit src/secrets.h with your actual WiFi credentials
   nano src/secrets.h
   ```

3. **Build firmware for your board**:

   ```bash
   # For Wemos D1 Mini (4MB)
   ./build.sh esp8266_d1_mini

   # For Generic ESP8266 (1MB)
   ./build.sh esp8266_generic
   ```

4. **Flash via serial connection**:

   ```bash
   pio run -e esp8266_d1_mini -t upload
   ```

### Option 3: Recover with esptool

For direct control over the flashing process:

```bash
# Erase entire flash
esptool.py -p /dev/ttyUSB0 erase_flash

# Flash with pre-built binary or custom build
esptool.py -p /dev/ttyUSB0 write_flash 0x00000 .pio/build/esp8266_d1_mini/firmware.bin
```

## Serial Connection Details

| Settings | Value |
|---|---|
| Port | /dev/ttyUSB0 (linux), COM3 (windows), /dev/tty.SILAB(mac) |
| Baud Rate | 115200 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| Flow Control | None |

## Telnet Access (if WiFi works)

If the device successfully connects to WiFi but web interface fails:

```bash
telnet <device-ip> 23
# Commands: help, status, mode, drv, xoff, oled, reboot
```

## Check Device Status

From a computer on the same network:

```bash
# Ping test
ping esp-oled-experiment.local

# nmap to find device IP
nmap -sn 192.168.1.0/24

# Check device logs via Telnet
telnet 192.168.1.X 23
telnet> status
```

## Still Having Issues?

1. **Device not found in AP list**:
   - Check if device power is stable
   - Look for LED indicators on the board
   - Try a different USB cable or power source

2. **Can't connect to AP**:
   - Verify WiFi is on and SSID visible
   - Try forgetting/re-adding the network
   - Restart your WiFi radio

3. **Web interface shows error**:
   - Check network connectivity: `ping 192.168.4.1`
   - Try clearing browser cache
   - Access from a different device

4. **GET MORE HELP**:
   - Check [BUILDING.md](BUILDING.md) for build issues
   - Review [README.md](README.md) for feature docs
