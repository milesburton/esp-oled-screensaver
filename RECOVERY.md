# Recovery

## Quick Reference

| Situation              | Action                                            |
|------------------------|---------------------------------------------------|
| New device / lost WiFi | Connect to `ESP-OLED-Setup` AP, go to 192.168.4.1 |
| AP not visible         | Serial flash (Option 2)                           |
| Stuck bootloop         | `erase_flash` with esptool then reflash           |

## Option 1: AP Setup Mode

1. Connect to the `ESP-OLED-Setup` WiFi network (no password).
2. Browse to `http://192.168.4.1/` and enter your WiFi credentials.
3. Device reboots and joins your network.

## Option 2: Serial Flash

**Prerequisites:** USB-to-serial adapter (3.3 V logic — CH340, CP2102, or FTDI).

**Wiring:**

| Programmer | ESP8266 |
|------------|---------|
| GND        | GND     |
| 3.3 V      | 3.3 V   |
| TX         | RX (GPIO3) |
| RX         | TX (GPIO1) |
| —          | GPIO0 → GND (hold during boot to enter flash mode) |

**Flash:**

```bash
git clone https://github.com/milesburton/esp-oled-screensaver.git
cd esp-oled-screensaver
cp secrets.h.template secrets.h   # add WiFi credentials
./build.sh esp8266_d1_mini

# Enter flash mode: hold GPIO0 to GND, press RST, release GPIO0
pio run -e esp8266_d1_mini -t upload
```

Or with esptool directly:

```bash
pip install esptool
esptool.py -p /dev/ttyUSB0 -b 921600 write_flash \
  --flash_mode dout --flash_freq 40m --flash_size detect \
  0x0 .pio/build/esp8266_d1_mini/firmware.bin
```

**Serial port locations:** `/dev/ttyUSB0` (Linux), `/dev/tty.SLAB_*` (macOS), `COM3` (Windows).

**Linux permission error:**

```bash
sudo usermod -a -G dialout $USER   # then log out and back in
```

## Dev Container USB Access

Add to `.devcontainer/devcontainer.json`:

```json
"runArgs": ["--privileged", "-v", "/dev:/dev"]
```

Then rebuild the container (`Ctrl+Shift+P` → "Dev Containers: Rebuild Container").
