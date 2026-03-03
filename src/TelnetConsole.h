#pragma once

#include <ESP8266WiFi.h>

#include <WiFiClient.h>
#include <WiFiServer.h>

#include "BreakoutMode.h"
#include "ClockMode.h"
#include "Config.h"
#include "DisplayManager.h"
#include "LifeMode.h"
#include "Logger.h"
#include "ModeHelper.h"
#include "PacManMode.h"
#include "ScreensaverMode.h"
#include "SonicMode.h"
#include "StarfieldMode.h"

class TelnetConsole {
 private:
  WiFiServer server;
  WiFiClient client;
  DisplayManager* displayManager;
  StatusMode* statusMode;
  BoingMode* boingMode;
  WeatherMode* weatherMode;
  ClockMode* clockMode;
  BreakoutMode* breakoutMode;
  PacManMode* pacManMode;
  ScreensaverMode* screensaverMode;
  StarfieldMode* starfieldMode;
  LifeMode* lifeMode;
  SonicMode* sonicMode;

  uint32_t lastActivityMs = 0;
  static constexpr uint32_t IDLE_TIMEOUT_MS = 5 * 60 * 1000;  // 5 minutes

  void sendHelp() {
    client.println("Commands:");
    client.println("  help                      - Show this help");
    client.println("  status                    - Show device status");
    client.println("  drv ssd1306|sh1106        - Set OLED driver");
    client.println("  xoff <int>                - Set X offset (-20..20)");
    client.println("  rot 0|1|2|3               - Set rotation (0/90/180/270 deg)");
    client.println(
        "  mode screensaver|status|boing|weather|clock|breakout|pacman|starfield|life|sonic"
        " - Switch display mode");
    client.println("  oled on|off               - Enable/disable OLED");
    client.println("  reboot                    - Restart device");
  }

  void sendStatus() {
    client.printf("fw=%s ip=%s rssi=%d heap=%u\n", Config::FW_VERSION,
                  WiFi.localIP().toString().c_str(),
                  (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0, ESP.getFreeHeap());
    client.printf("oled=%d drv=%s rot=%s xoff=%d sda=%u scl=%u addr=0x%02X\n",
                  Config::runtime.oledEnabled ? 1 : 0, Config::runtime.getDriverName(),
                  Config::runtime.getRotationName(), Config::runtime.xOffset, Config::OLED_SDA,
                  Config::OLED_SCL, Config::OLED_ADDR);
    if (displayManager && displayManager->getCurrentMode()) {
      client.printf("mode=%s\n", displayManager->getCurrentMode()->getName());
    }
  }

 public:
  TelnetConsole()
      : server(Config::TELNET_PORT),
        displayManager(nullptr),
        statusMode(nullptr),
        boingMode(nullptr),
        weatherMode(nullptr),
        clockMode(nullptr),
        breakoutMode(nullptr),
        pacManMode(nullptr),
        screensaverMode(nullptr),
        starfieldMode(nullptr),
        lifeMode(nullptr),
        sonicMode(nullptr) {}

  void setDisplayManager(DisplayManager* dm) { displayManager = dm; }

  void setModes(StatusMode* status, BoingMode* boing, WeatherMode* weather, ClockMode* clock,
                BreakoutMode* breakout, PacManMode* pacman, ScreensaverMode* screensaver,
                StarfieldMode* starfield, LifeMode* life, SonicMode* sonic) {
    statusMode = status;
    boingMode = boing;
    weatherMode = weather;
    clockMode = clock;
    breakoutMode = breakout;
    pacManMode = pacman;
    screensaverMode = screensaver;
    starfieldMode = starfield;
    lifeMode = life;
    sonicMode = sonic;
  }

  void begin() {
    server.begin();
    server.setNoDelay(true);
    Logger::println("Telnet: listening on port 23");
  }

  void update() {
    if (server.hasClient()) {
      WiFiClient incoming = server.accept();
      if (!client || !client.connected()) {
        client = incoming;
        client.setNoDelay(true);
        Logger::setTelnetClient(&client);

        client.println();
        client.println("ESP8266 Remote Console");
        client.println("Type 'help' for commands");
        client.println();

        lastActivityMs = millis();
        Logger::println("Telnet: client connected");
      } else {
        incoming.println("busy");
        incoming.stop();
      }
    }

    if (client && client.connected()) {
      if (millis() - lastActivityMs >= IDLE_TIMEOUT_MS) {
        Logger::println("Telnet: idle timeout, disconnecting");
        client.println("Idle timeout. Disconnecting.");
        client.flush();
        client.stop();
        Logger::setTelnetClient(nullptr);
        return;
      }
    }

    if (client && client.connected() && client.available()) {
      String cmd = client.readStringUntil('\n');
      cmd.trim();
      cmd.toLowerCase();

      lastActivityMs = millis();
      handleCommand(cmd);
    }
  }

  void handleCommand(const String& cmd) {
    if (cmd == "help") {
      sendHelp();

    } else if (cmd == "status") {
      sendStatus();

    } else if (cmd.startsWith("drv ")) {
      if (cmd.endsWith("ssd1306")) {
        Config::runtime.driver = Config::OledDriver::SSD1306;
      } else if (cmd.endsWith("sh1106")) {
        Config::runtime.driver = Config::OledDriver::SH1106;
      }
      if (displayManager) {
        displayManager->selectDriver();
      }
      client.println("ok");

    } else if (cmd.startsWith("rot ")) {
      int val = cmd.substring(4).toInt();
      if (val < 0 || val > 3) {
        client.println("error: rot must be 0, 1, 2, or 3");
        return;
      }
      if (displayManager) {
        displayManager->setRotation(static_cast<Config::DisplayRotation>(val));
      }
      client.println("ok");

    } else if (cmd.startsWith("xoff ")) {
      String v = cmd.substring(5);
      int val = v.toInt();
      if (val < -20 || val > 20) {
        client.println("error: xoff must be between -20 and 20");
        return;
      }
      Config::runtime.xOffset = val;
      client.println("ok");

    } else if (cmd.startsWith("mode ")) {
      if (!displayManager) {
        client.println("error: no display manager");
        return;
      }

      String modeName = cmd.substring(5);
      if (!setModeByName(displayManager, modeName, statusMode, boingMode, weatherMode, clockMode,
                         breakoutMode, pacManMode, screensaverMode, starfieldMode, lifeMode,
                         sonicMode)) {
        client.println("unknown mode");
        return;
      }
      client.println("ok");

    } else if (cmd == "oled on") {
      Config::runtime.oledEnabled = true;
      if (displayManager) {
        displayManager->begin();
      }
      client.println("ok");

    } else if (cmd == "oled off") {
      Config::runtime.oledEnabled = false;
      if (displayManager) {
        displayManager->clear();
      }
      client.println("ok");

    } else if (cmd == "reboot") {
      client.println("Rebooting...");
      client.flush();
      delay(50);
      ESP.restart();

    } else if (cmd.length() > 0) {
      client.println("unknown command (type 'help')");
    }
  }
};
