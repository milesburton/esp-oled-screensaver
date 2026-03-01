#!/bin/bash
# Recovery and reflash script for ESP8266-OLED-Experiment
# Usage: ./recovery.sh [port] [board]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Defaults
PORT="/dev/ttyUSB0"
BOARD="esp8266_d1_mini"
ERASE_FIRST=false

# Parse arguments
if [ "$#" -gt 0 ]; then
    PORT="$1"
fi

if [ "$#" -gt 1 ]; then
    BOARD="$2"
fi

echo -e "${YELLOW}================================================${NC}"
echo -e "${YELLOW}ESP8266-OLED-Experiment Recovery & Flash Tool${NC}"
echo -e "${YELLOW}================================================${NC}"
echo ""

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"
if ! command -v esptool.py &> /dev/null; then
    echo -e "${RED}ERROR: esptool not found!${NC}"
    echo "Install it with: pip install esptool"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    echo -e "${RED}ERROR: python3 not found!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ esptool.py found${NC}"

# Check serial port
if [ ! -e "$PORT" ]; then
    echo -e "${RED}ERROR: Serial port $PORT not found!${NC}"
    echo ""
    echo "Available ports:"
    ls -1 /dev/tty* | grep -E "USB|ACM|SLAB|CH34" || echo "  (none found)"
    echo ""
    echo "Usage: $0 [port] [board]"
    echo "Example: $0 /dev/ttyUSB0 esp8266_d1_mini"
    exit 1
fi

echo -e "${GREEN}✓ Serial port $PORT is available${NC}"
echo ""

# Show board selection
echo "Selected board: $BOARD"
echo "  4MB boards: esp8266_d1_mini, esp8266_nodmcu"
echo "  1MB board:  esp8266_generic"
echo ""

# Configure credentials
echo -e "${YELLOW}Configuring WiFi credentials...${NC}"
if [ ! -f "src/secrets.h" ]; then
    cp src/secrets.h.template src/secrets.h
    echo -e "${YELLOW}Created src/secrets.h from template${NC}"
    echo -e "${RED}WARNING: Please edit src/secrets.h with your WiFi credentials!${NC}"
    echo ""
    echo "Run: nano src/secrets.h"
    echo "Then run this script again"
    exit 1
else
    echo -e "${GREEN}✓ Using existing src/secrets.h${NC}"
fi

echo ""

# Build firmware
echo -e "${YELLOW}Building firmware for $BOARD...${NC}"
./build.sh "$BOARD" > /dev/null 2>&1 || {
    echo -e "${RED}ERROR: Build failed!${NC}"
    exit 1
}

FIRMWARE_BIN=".pio/build/$BOARD/firmware.bin"
if [ ! -f "$FIRMWARE_BIN" ]; then
    echo -e "${RED}ERROR: Firmware binary not found: $FIRMWARE_BIN${NC}"
    exit 1
fi

FIRMWARE_SIZE=$(stat -f%z "$FIRMWARE_BIN" 2>/dev/null || stat -c%s "$FIRMWARE_BIN" 2>/dev/null)
echo -e "${GREEN}✓ Built: $FIRMWARE_BIN (${FIRMWARE_SIZE} bytes)${NC}"
echo ""

# Ask for erase confirmation
echo -e "${YELLOW}Preparing for flash...${NC}"
echo "Port: $PORT"
echo "Board: $BOARD"
echo "Firmware: $FIRMWARE_BIN"
echo ""
read -p "Erase entire flash before writing? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    ERASE_FIRST=true
fi

echo ""
echo -e "${YELLOW}IMPORTANT: Put device in bootloader mode:${NC}"
echo "  1. Hold GPIO0 to GND"
echo "  2. Press RST button (or cycle power)"
echo "  3. Wait 1 second"
echo "  4. Release GPIO0"
echo ""
read -p "Ready to flash? Press Enter to continue (Ctrl+C to cancel)..."
echo ""

# Erase if requested
if [ "$ERASE_FIRST" = true ]; then
    echo -e "${YELLOW}Erasing flash...${NC}"
    esptool.py -p "$PORT" -b 921600 erase_flash
    echo -e "${GREEN}✓ Flash erased${NC}"
    echo ""
    sleep 2
fi

# Flash firmware
echo -e "${YELLOW}Flashing firmware to $PORT...${NC}"
esptool.py -p "$PORT" -b 921600 \
  write_flash --flash_mode dout --flash_freq 40m --flash_size detect \
  0x0 "$FIRMWARE_BIN"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}================================================${NC}"
    echo -e "${GREEN}✓ FLASH SUCCESSFUL!${NC}"
    echo -e "${GREEN}================================================${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Disconnect programmer and power cycle device"
    echo "  2. Look for WiFi network: 'esp-oled-setup'"
    echo "  3. Password: 'setup1234'"
    echo "  4. Open: http://192.168.4.1/"
    echo "  5. Configure your WiFi credentials"
    echo ""
    echo "The device will reboot and connect to your WiFi!"
else
    echo ""
    echo -e "${RED}FLASH FAILED!${NC}"
    echo "Check:"
    echo "  - GPIO0 is connected to GND during boot"
    echo "  - Serial port is correct"
    echo "  - USB cable is working"
    exit 1
fi
