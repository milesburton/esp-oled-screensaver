#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdint>
#endif

#include "secrets.h"

namespace Config {
// ===== Identity =====
static constexpr const char* HOSTNAME = "esp-weather-clock";
static constexpr const char* FW_VERSION = "platform-1.0.7";

// ===== WiFi Credentials =====
static constexpr const char* WIFI_SSID = secrets::WIFI_SSID;
static constexpr const char* WIFI_PASS = secrets::WIFI_PASS;
static constexpr const char* OTA_USER = secrets::OTA_USER;
static constexpr const char* OTA_PASS = secrets::OTA_PASS;

// Runtime WiFi credentials (loaded from EEPROM)
struct RuntimeWiFiConfig {
  char ssid[32] = {0};
  char password[64] = {0};
};

extern RuntimeWiFiConfig runtimeWiFi;

// ===== OLED Hardware =====
static constexpr uint8_t OLED_SDA = 0;      // GPIO0
static constexpr uint8_t OLED_SCL = 2;      // GPIO2
static constexpr uint8_t OLED_ADDR = 0x3C;  // I2C address

// ===== Display Settings =====
static constexpr int DISPLAY_WIDTH = 128;
static constexpr int DISPLAY_HEIGHT = 64;

// ===== Network Ports =====
static constexpr uint16_t HTTP_PORT = 80;
static constexpr uint16_t TELNET_PORT = 23;

// ===== Boing Animation =====
static constexpr int BOING_BALL_RADIUS = 12;      // Ball radius in pixels
static constexpr float BOING_X_SPEED = 52.0f;     // Horizontal speed (px/s)
static constexpr float BOING_BOUNCE_FREQ = 0.75f; // Bounces per second
static constexpr float BOING_BOUNCE_HEIGHT = 34.0f; // Max bounce height in pixels

// ===== Runtime OLED Configuration =====
enum class OledDriver : uint8_t { SSD1306, SH1106 };
enum class DisplayRotation : uint8_t { None = 0, _90CW = 1, _180 = 2, _90CCW = 3 };

struct RuntimeConfig {
  OledDriver driver = OledDriver::SSD1306;  // Standard configuration (fixes border)
  int xOffset = 0;                          // SSD1306 standard: 0 (no offset needed)
  bool oledEnabled = true;
  DisplayRotation rotation = DisplayRotation::None;

  const char* getDriverName() const {
    return (driver == OledDriver::SSD1306) ? "SSD1306" : "SH1106";
  }

  uint8_t getU8G2Rotation() const {
    return static_cast<uint8_t>(rotation);
  }

  const char* getRotationName() const {
    switch (rotation) {
      case DisplayRotation::None: return "0°";
      case DisplayRotation::_90CW: return "90° CW";
      case DisplayRotation::_180: return "180°";
      case DisplayRotation::_90CCW: return "90° CCW";
      default: return "unknown";
    }
  }
};

extern RuntimeConfig runtime;
}  // namespace Config
