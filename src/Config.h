#pragma once

#include <Arduino.h>
#include "secrets.h"

namespace Config {
  // ===== Identity =====
  static constexpr const char* HOSTNAME   = "esp-weather-clock";
  static constexpr const char* FW_VERSION = "platform-1.0.0-modular";
  
  // ===== WiFi Credentials =====
  static constexpr const char* WIFI_SSID = secrets::WIFI_SSID;
  static constexpr const char* WIFI_PASS = secrets::WIFI_PASS;
  static constexpr const char* OTA_USER  = secrets::OTA_USER;
  static constexpr const char* OTA_PASS  = secrets::OTA_PASS;
  
  // ===== OLED Hardware =====
  static constexpr uint8_t OLED_SDA  = 0;    // GPIO0
  static constexpr uint8_t OLED_SCL  = 2;    // GPIO2
  static constexpr uint8_t OLED_ADDR = 0x3C; // I2C address
  
  // ===== Display Settings =====
  static constexpr int DISPLAY_WIDTH  = 128;
  static constexpr int DISPLAY_HEIGHT = 64;
  
  // ===== Network Ports =====
  static constexpr uint16_t HTTP_PORT   = 80;
  static constexpr uint16_t TELNET_PORT = 23;
  
  // ===== Runtime OLED Configuration =====
  enum class OledDriver : uint8_t { SSD1306, SH1106 };
  
  struct RuntimeConfig {
    OledDriver driver = OledDriver::SSD1306;  // Standard configuration (fixes border)
    int xOffset = 0;                           // SSD1306 standard: 0 (no offset needed)
    bool oledEnabled = true;
    
    const char* getDriverName() const {
      return (driver == OledDriver::SSD1306) ? "SSD1306" : "SH1106";
    }
  };
  
  extern RuntimeConfig runtime;
}
