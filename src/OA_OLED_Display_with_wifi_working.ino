// ESP8266 Weather Clock - Modular Architecture
//
// A flexible platform for ESP8266-based OLED displays with:
// - Pluggable display modes (Status, Boing animation, Weather forecast, etc.)
// - WiFi with OTA updates
// - Telnet console for remote debugging
// - Web interface for configuration
//
// Hardware: ESP8266 + 128x64 OLED (SSD1306/SH1106)
// Wiring: SDA=GPIO0, SCL=GPIO2, I2C addr=0x3C

#include "Config.h"
#include "DisplayManager.h"
#include "Logger.h"
#include "NetworkManager.h"
#include "TelnetConsole.h"

// Display modes
#include "BoingMode.h"
#include "StatusMode.h"
#include "WeatherMode.h"

// ===== Global Components =====
DisplayManager displayManager;
NetworkManager networkManager;
TelnetConsole telnetConsole;

// ===== Display Modes =====
StatusMode statusMode;
BoingMode boingMode;
WeatherMode weatherMode;

// ===== Periodic Logging =====
uint32_t lastStatusLog = 0;

void setup() {
  Serial.begin(115200);
  delay(50);

  Logger::println("");
  Logger::printf("Boot: %s fw=%s", Config::HOSTNAME, Config::FW_VERSION);

  // Initialize display
  displayManager.begin();

  // Link components
  networkManager.setDisplayManager(&displayManager);
  networkManager.setModes(&statusMode, &boingMode, &weatherMode);

  telnetConsole.setDisplayManager(&displayManager);
  telnetConsole.setModes(&statusMode, &boingMode, &weatherMode);

  // Start network services
  networkManager.begin();
  telnetConsole.begin();

  // Start with status mode so user can tune OLED driver/offset first
  displayManager.setMode(&statusMode, 400);  // 2.5 FPS

  Logger::println("System ready!");
}

void loop() {
  // Update network services
  networkManager.update();
  telnetConsole.update();

  // Update display
  displayManager.update();

  // Periodic status logging (every 1 second)
  uint32_t now = millis();
  if (now - lastStatusLog > 1000) {
    lastStatusLog = now;
    networkManager.logStatus();
  }

  delay(1);
  yield();
}