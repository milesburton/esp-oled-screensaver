#pragma once

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#include "Config.h"
#include "DisplayManager.h"
#include "Logger.h"

// Forward declarations of mode classes
class StatusMode;
class BoingMode;
class WeatherMode;

class TelnetConsole {
 private:
  WiFiServer server;
  WiFiClient client;
  DisplayManager* displayManager;
  StatusMode* statusMode;
  BoingMode* boingMode;
  WeatherMode* weatherMode;

  void sendHelp() {
    client.println("Commands:");
    client.println("  help                   - Show this help");
    client.println("  status                 - Show device status");
    client.println("  drv ssd1306|sh1106     - Set OLED driver");
    client.println(
        "  xoff <int>             - Set X offset (e.g., xoff 0 or xoff 2)");
    client.println("  mode status|boing|weather - Switch display mode");
    client.println("  oled on|off            - Enable/disable OLED");
    client.println("  reboot                 - Restart device");
  }

  void sendStatus() {
    client.printf("fw=%s ip=%s rssi=%d heap=%u\n", Config::FW_VERSION,
                  WiFi.localIP().toString().c_str(),
                  (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0,
                  ESP.getFreeHeap());
    client.printf("oled=%d drv=%s xoff=%d sda=%u scl=%u addr=0x%02X\n",
                  Config::runtime.oledEnabled ? 1 : 0,
                  Config::runtime.getDriverName(), Config::runtime.xOffset,
                  Config::OLED_SDA, Config::OLED_SCL, Config::OLED_ADDR);
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
        weatherMode(nullptr) {}

  void setDisplayManager(DisplayManager* dm) { displayManager = dm; }

  void setModes(StatusMode* status, BoingMode* boing, WeatherMode* weather) {
    statusMode = status;
    boingMode = boing;
    weatherMode = weather;
  }

  void begin() {
    server.begin();
    server.setNoDelay(true);
    Logger::println("Telnet: listening on port 23");
  }

  void update() {
    // Check for new connections
    if (server.hasClient()) {
      WiFiClient incoming = server.available();
      if (!client || !client.connected()) {
        client = incoming;
        client.setNoDelay(true);
        Logger::setTelnetClient(&client);

        client.println();
        client.println("ESP8266 Remote Console");
        client.println("Type 'help' for commands");
        client.println();

        Logger::println("Telnet: client connected");
      } else {
        incoming.println("busy");
        incoming.stop();
      }
    }

    // Process commands
    if (client && client.connected() && client.available()) {
      String cmd = client.readStringUntil('\n');
      cmd.trim();
      cmd.toLowerCase();

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

    } else if (cmd.startsWith("xoff ")) {
      String v = cmd.substring(5);
      Config::runtime.xOffset = v.toInt();
      client.println("ok");

    } else if (cmd.startsWith("mode ")) {
      if (!displayManager) {
        client.println("error: no display manager");
        return;
      }

      if (cmd.endsWith("status") && statusMode) {
        displayManager->setMode(statusMode, 400);  // 2.5 FPS
      } else if (cmd.endsWith("boing") && boingMode) {
        displayManager->setMode(boingMode, 40);  // 25 FPS
      } else if (cmd.endsWith("weather") && weatherMode) {
        displayManager->setMode(weatherMode, 5000);  // Every 5 seconds
      } else {
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
