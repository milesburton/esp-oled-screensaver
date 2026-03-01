#!/bin/bash
# Build script for creating OTA-compatible firmware binaries
# Usage: ./build.sh [environment]
# Example: ./build.sh esp8266_d1_mini

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
DEFAULT_ENV="esp8266_d1_mini"
TARGET_ENV="${1:-$DEFAULT_ENV}"
BUILD_DIR=".pio/build/$TARGET_ENV"
FIRMWARE_DIR="firmware"
VERSION=$(grep 'FW_VERSION' Config.h | grep -oP '"platform-\K[^"]+')
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo -e "${YELLOW}================================================${NC}"
echo -e "${YELLOW}ESP8266 Weather Clock - Build Script${NC}"
echo -e "${YELLOW}================================================${NC}"
echo "Environment: $TARGET_ENV"
echo "Version: platform-$VERSION"
echo "Timestamp: $TIMESTAMP"
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}ERROR: PlatformIO not found. Install with: pip install platformio${NC}"
    exit 1
fi

# Create firmware output directory
mkdir -p "$FIRMWARE_DIR"

echo -e "${YELLOW}Building firmware...${NC}"
pio run -e "$TARGET_ENV" -d . || {
    echo -e "${RED}Build failed!${NC}"
    exit 1
}

echo -e "${GREEN}Build successful!${NC}"
echo ""

# Check for compiled firmware
if [ ! -f "$BUILD_DIR/firmware.bin" ]; then
    echo -e "${RED}ERROR: firmware.bin not found at $BUILD_DIR/firmware.bin${NC}"
    exit 1
fi

# Copy and rename firmware for OTA
FIRMWARE_NAME="esp8266-weather-clock_${VERSION}_${TARGET_ENV}_${TIMESTAMP}.bin"
FIRMWARE_PATH="$FIRMWARE_DIR/$FIRMWARE_NAME"

cp "$BUILD_DIR/firmware.bin" "$FIRMWARE_PATH"

echo -e "${GREEN}✓ Firmware binary created:${NC}"
echo "  Path: $FIRMWARE_PATH"
echo "  Size: $(ls -lh "$FIRMWARE_PATH" | awk '{print $5}')"
echo ""

# Create a latest symlink for convenience
LATEST_LINK="$FIRMWARE_DIR/latest_${TARGET_ENV}.bin"
ln -sf "$FIRMWARE_NAME" "$LATEST_LINK"
echo -e "${GREEN}✓ Latest symlink created: $LATEST_LINK${NC}"
echo ""

# Generate build info
BUILD_INFO_FILE="$FIRMWARE_DIR/build_info.txt"
cat > "$BUILD_INFO_FILE" << EOF
ESP8266 Weather Clock Build Information
$(date)

Version: platform-$VERSION
Environment: $TARGET_ENV
Firmware: $FIRMWARE_NAME
Size: $(ls -lh "$FIRMWARE_PATH" | awk '{print $5}')

Build Command:
  pio run -e $TARGET_ENV

Upload via OTA:
  Navigate to http://<device-ip>/update
  Select: $FIRMWARE_PATH
  Login with credentials from secrets.h

Upload via Serial (PlatformIO):
  pio run -e $TARGET_ENV -t upload

Upload via Arduino IDE:
  1. Build: pio run -e $TARGET_ENV
  2. Use firmware.bin from $BUILD_DIR
  3. Arduino IDE → Sketch → Export compiled Binary
EOF

echo -e "${GREEN}✓ Build info file created: $BUILD_INFO_FILE${NC}"
echo ""

echo -e "${GREEN}================================================${NC}"
echo -e "${GREEN}Build Complete!${NC}"
echo -e "${GREEN}================================================${NC}"
echo ""
echo "Next steps:"
echo "1. Test upload via OTA: http://<device-ip>/update"
echo "2. Or upload via serial: pio run -e $TARGET_ENV -t upload"
echo ""
