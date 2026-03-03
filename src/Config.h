#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <cstdint>
#endif

#include "secrets.h"

namespace Config {
// ===== Identity =====
static constexpr const char* HOSTNAME = "esp-oled-screensaver";
static constexpr const char* FW_VERSION = "platform-1.0.59";

// ===== WiFi Credentials =====
static constexpr const char* WIFI_SSID = secrets::WIFI_SSID;
static constexpr const char* WIFI_PASS = secrets::WIFI_PASS;
static constexpr const char* OTA_USER = secrets::OTA_USER;
static constexpr const char* OTA_PASS = secrets::OTA_PASS;

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
static constexpr int BOING_BALL_RADIUS = 12;         // Ball radius in pixels
static constexpr float BOING_X_SPEED = 52.0f;        // Horizontal speed (px/s)
static constexpr float BOING_BOUNCE_FREQ = 0.75f;    // Bounces per second
static constexpr float BOING_BOUNCE_HEIGHT = 34.0f;  // Max bounce height in pixels

// ===== Weather API (Open-Meteo) =====
static constexpr float WEATHER_LATITUDE = 51.5f;                             // London default
static constexpr float WEATHER_LONGITUDE = -0.1f;                            // London default
static constexpr uint32_t WEATHER_FETCH_INTERVAL_MS = 10UL * 60UL * 1000UL;  // 10 minutes

// ===== Clock / NTP =====
static constexpr int NTP_UTC_OFFSET_HOURS = 0;  // UTC; change for local tz
static constexpr uint32_t NTP_RESYNC_INTERVAL_MS = 60UL * 60UL * 1000UL;  // Re-sync every hour

// ===== Screensaver =====
static constexpr uint32_t SCREENSAVER_MODE_INTERVAL_MS = 30UL * 1000UL;  // 30s per mode

// ===== Runtime OLED Configuration =====
enum class OledDriver : uint8_t { SSD1306, SH1106 };

// Display rotation: 0=normal, 1=90deg CW, 2=180deg, 3=270deg CW
enum class DisplayRotation : uint8_t { R0 = 0, R1 = 1, R2 = 2, R3 = 3 };

struct RuntimeConfig {
  OledDriver driver = OledDriver::SSD1306;         // Standard configuration (fixes border)
  DisplayRotation rotation = DisplayRotation::R2;  // Display rotation (0/90/180/270)
  int xOffset = 0;                                 // SSD1306 standard: 0 (no offset needed)
  bool oledEnabled = true;

  const char* getDriverName() const {
    return (driver == OledDriver::SSD1306) ? "SSD1306" : "SH1106";
  }

  const char* getRotationName() const {
    switch (rotation) {
      case DisplayRotation::R0:
        return "0°";
      case DisplayRotation::R1:
        return "90°";
      case DisplayRotation::R2:
        return "180°";
      case DisplayRotation::R3:
        return "270°";
      default:
        return "0°";
    }
  }

  uint8_t getU8G2Rotation() const { return static_cast<uint8_t>(rotation); }
};

inline RuntimeConfig runtime;
}  // namespace Config
