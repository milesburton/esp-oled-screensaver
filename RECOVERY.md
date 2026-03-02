# Device Recovery Guide

**TL;DR - Quick Recovery Steps**

| Situation | Action |
|-----------|--------|
| **New device** | Connect to `esp-oled-setup` AP (pwd: `setup1234`), go to `http://192.168.4.1/` |
| **Lost WiFi** | Same as above OR reflash with `./build.sh esp8266_d1_mini` |
| **No AP visible** | Use serial programmer (see Option 3) |
| **Stuck in bootloop** | Erase flash with esptool: `esptool.py -p /dev/ttyUSB0 erase_flash` |

---

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

5. **Update firmware via OTA (Optional)**:
   Once your device is connected to WiFi, you can update firmware wirelessly:
   - Navigate to: `http://<device-ip>/update`
   - Username: `admin`
   - Password: `test-ota-password`
   - Select new `.bin` firmware file and upload

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

### Option 3: Serial Programmer Flash (When AP Mode Unavailable)

If you cannot access AP mode (e.g., device flashed with broken firmware), use a serial programmer for direct flash access.

#### Automated Recovery Script (Easiest)

We provide an automated recovery script that handles everything:

**If using VS Code Dev Container:**

```bash
# From inside the container terminal:
./recovery.sh /dev/ttyUSB0 esp8266_d1_mini

# Or explicitly specify port if using different mapping:
./recovery.sh /dev/ttyUSB0 esp8266_d1_mini
```

**If using system Python (not in container):**

```bash
# Clone repository
git clone https://github.com/milesburton/esp8266-oled-experiment.git
cd esp8266-oled-experiment

# Run recovery script
./recovery.sh [port] [board]

# Examples:
./recovery.sh /dev/ttyUSB0 esp8266_d1_mini
./recovery.sh COM3 esp8266_generic  # Windows
```

The script will:

1. Verify esptool is installed
2. Check serial port availability
3. Prompt you to configure WiFi credentials (if not already done)
4. Build firmware for your board
5. Guide you through bootloader mode
6. Flash the firmware
7. Provide next steps

**If script has issues**, follow the manual steps below.

#### Dev Container USB Serial Port Access

If you're using VS Code Remote Containers, you need to expose the USB device to the container.

**Option A: Automatic (Easiest - VS Code only)**

Update `.devcontainer/devcontainer.json`:

```json
{
  "runArgs": [
    "--privileged",
    "-v", "/dev:/dev"
  ]
}
```

Then rebuild container: `Ctrl+Shift+P` → "Dev Containers: Rebuild Container"

**Option B: Manual Device Mapping (Docker/Linux)**

If using Docker directly:

```bash
# Find your USB device
ls -la /dev/ttyUSB*  # or /dev/ttyACM*

# Run container with device access
docker run -it --privileged \
  -v /dev/ttyUSB0:/dev/ttyUSB0 \
  your-container-name /bin/bash
```

**Option C: User Group Access (Linux only, no privileged needed)**

```bash
# On host, add your user to dialout group
sudo usermod -a -G dialout $USER

# Then in devcontainer.json (simpler):
{
  "runArgs": [
    "-v", "/dev/ttyUSB0:/dev/ttyUSB0",
    "--device-cgroup-rule=c 188:* rmw"
  ]
}
```

**Finding your serial port in container:**

```bash
# Inside container
ls -la /dev/tty*
# Should see: /dev/ttyUSB0, /dev/ttyACM0, etc.

# Verify esptool can find it
esptool.py version  # Will list available ports
```

#### Manual Flash Instructions

1. **USB-to-Serial Programmer**:
   - CH340, CP2102, or FTDI-based
   - 3.3V logic (NOT 5V)

2. **Install esptool**:

   ```bash
   pip install esptool
   ```

3. **Identify the serial port**:

   ```bash
   # List available serial ports
   esptool.py version  # This will show your port in the output

   # Or use system commands:
   # Linux: ls /dev/ttyUSB* or /dev/ttyACM*
   # macOS: ls /dev/tty.* | grep -E "SLAB|CH34"
   # Windows: Check Device Manager for COM ports
   ```

#### Wiring Connections

| Programmer Pin | ESP8266 Pin | Purpose |
|---|---|---|
| GND | GND | Ground |
| VCC/3.3V | 3.3V | Power |
| TX | RX (GPIO3) | Serial TX |
| RX | TX (GPIO1) | Serial RX |
| - | GPIO0 | **Hold to GND during boot to enter flash mode** |
| - | GPIO2 | Can leave floating |

#### Step-by-Step Flash Instructions

1. **Clone repository and setup**:

   ```bash
   git clone https://github.com/milesburton/esp8266-oled-experiment.git
   cd esp8266-oled-experiment
   ```

2. **Configure WiFi credentials**:

   ```bash
   cp secrets.h.template src/secrets.h
   # Edit with your actual credentials
   nano src/secrets.h
   # Change:
   #   WIFI_SSID = "your-wifi-name"
   #   WIFI_PASS = "your-wifi-password"
   ```

3. **Build firmware**:

   ```bash
   # For Wemos D1 Mini or NodeMCU (4MB)
   ./build.sh esp8266_d1_mini

   # OR for Generic ESP8266 (1MB)
   ./build.sh esp8266_generic
   ```

4. **Enter Flash Mode**:
   - Hold GPIO0 to GND
   - Press RST (or cycle power)
   - Release GPIO0
   - Device should be in bootloader mode

5. **Flash the firmware**:

   ```bash
   # Replace /dev/ttyUSB0 with your serial port
   esptool.py -p /dev/ttyUSB0 -b 921600 \
     write_flash --flash_mode dout --flash_freq 40m --flash_size detect \
     0x0 .pio/build/esp8266_d1_mini/firmware.bin

   # For Windows (use COM port instead):
   esptool.py -p COM3 -b 921600 ^
     write_flash --flash_mode dout --flash_freq 40m --flash_size detect ^
     0x0 .pio\build\esp8266_d1_mini\firmware.bin
   ```

6. **Verify flash success**:

   ```bash
   # You should see:
   # Wrote 358608 bytes at address 0x00000000 in X.X seconds
   # Hash of data verified.
   ```

7. **Reset device**:
   - Press RST button or cycle power
   - Device will boot with new firmware
   - Should start AP mode: `esp-oled-setup`

#### Complete Flash Erase (if needed)

If you need to erase everything first:

```bash
esptool.py -p /dev/ttyUSB0 erase_flash
# Then proceed with write_flash command above
```

#### Troubleshooting Serial Flash

**"Device not found" error**:

- Check USB cable is connected
- Verify correct COM port (use `esptool.py version`)
- Try different USB port on your computer
- Check programmer is 3.3V not 5V

**"Failed to enter ROM bootloader"**:

- Make sure GPIO0 is held to GND during boot
- Try holding GPIO0 longer (full 2+ seconds)
- Check EN/RST button press timing

**"Hash of data verified" but device won't boot**:

- Try pressing RST button
- Check power supply voltage (must be 3.3V stable)
- Device may need 5-10 seconds to boot

**Serial port permission denied (Linux)**:

```bash
sudo usermod -a -G dialout $USER
# Then logout and login, or:
sudo chmod 666 /dev/ttyUSB0
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
