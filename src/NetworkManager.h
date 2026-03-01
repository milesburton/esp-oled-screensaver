#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include <ElegantOTA.h>

#include "Config.h"
#include "DisplayManager.h"
#include "Logger.h"
#include "ModeHelper.h"

class NetworkManager {
 private:
  ESP8266WebServer http;
  DisplayManager* displayManager;
  StatusMode* statusMode;
  BoingMode* boingMode;
  WeatherMode* weatherMode;

  uint32_t wifiRetryMs = 0;
  uint32_t wifiRetryIntervalMs = 5000;
  static constexpr uint32_t WIFI_RETRY_MAX_MS = 5 * 60 * 1000;

  static const char* wlStatusName(wl_status_t s) {
    switch (s) {
      case WL_IDLE_STATUS:
        return "IDLE";
      case WL_NO_SSID_AVAIL:
        return "NO_SSID";
      case WL_SCAN_COMPLETED:
        return "SCAN_DONE";
      case WL_CONNECTED:
        return "CONNECTED";
      case WL_CONNECT_FAILED:
        return "CONNECT_FAILED";
      case WL_CONNECTION_LOST:
        return "CONNECTION_LOST";
      case WL_DISCONNECTED:
        return "DISCONNECTED";
      default:
        return "UNKNOWN";
    }
  }

  void setupRoutes() {
    http.on("/", HTTP_GET, [this]() {
      http.setContentLength(CONTENT_LENGTH_UNKNOWN);
      http.send(200, F("text/html; charset=utf-8"), "");

      http.sendContent(F("<h1>"));
      http.sendContent(Config::HOSTNAME);
      http.sendContent(F("</h1><p><b>FW:</b> "));
      http.sendContent(Config::FW_VERSION);
      http.sendContent(F("</p><ul>"));

      http.sendContent(F("<li><b>WiFi:</b> "));
      http.sendContent(wlStatusName(WiFi.status()));
      http.sendContent(F("</li><li><b>IP:</b> "));
      http.sendContent(WiFi.localIP().toString());
      http.sendContent(F("</li><li><b>RSSI:</b> "));
      http.sendContent(String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0));
      http.sendContent(F("</li><li><b>Heap:</b> "));
      http.sendContent(String(ESP.getFreeHeap()));
      http.sendContent(F("</li>"));

      if (displayManager && displayManager->getCurrentMode()) {
        http.sendContent(F("<li><b>Mode:</b> "));
        http.sendContent(displayManager->getCurrentMode()->getName());
        http.sendContent(F("</li>"));
      }

      http.sendContent(F("<li><b>OLED:</b> "));
      http.sendContent(Config::runtime.oledEnabled ? "on" : "off");
      http.sendContent(F("</li><li><b>Driver:</b> "));
      http.sendContent(Config::runtime.getDriverName());
      http.sendContent(F("</li><li><b>Rotation:</b> "));
      http.sendContent(Config::runtime.getRotationName());
      http.sendContent(F("</li><li><b>X offset:</b> "));
      http.sendContent(String(Config::runtime.xOffset));
      http.sendContent(F("</li><li><b>I2C:</b> SDA="));
      http.sendContent(String(Config::OLED_SDA));
      http.sendContent(F(" SCL="));
      http.sendContent(String(Config::OLED_SCL));
      http.sendContent(F(" addr=0x"));
      http.sendContent(String(Config::OLED_ADDR, HEX));
      http.sendContent(F("</li></ul>"));

      http.sendContent(
          F("<p><a href='/update'>OTA Update</a></p>"
            "<h3>Display Mode</h3>"
            "<p>"
            "<a href='/mode?m=status'>Status</a> | "
            "<a href='/mode?m=boing'>Boing</a> | "
            "<a href='/mode?m=weather'>Weather</a>"
            "</p>"
            "<h3>OLED Configuration</h3>"
            "<p>Try these until border + text align perfectly:</p>"
            "<ul>"
            "<li><a href='/oledcfg?drv=sh1106&xoff=2'>SH1106 xoff=2 (common)</a></li>"
            "<li><a href='/oledcfg?drv=sh1106&xoff=0'>SH1106 xoff=0</a></li>"
            "<li><a href='/oledcfg?drv=ssd1306&xoff=0'>SSD1306 xoff=0 (common)</a></li>"
            "<li><a href='/oledcfg?drv=ssd1306&xoff=2'>SSD1306 xoff=2</a></li>"
            "</ul>"
            "<p><a href='/oled?on=1'>OLED ON</a> | <a href='/oled?on=0'>OLED OFF</a></p>"
            "<h3>Display Rotation</h3>"
            "<p>"
            "<a href='/rotation?rot=0'>0&#176;</a> | "
            "<a href='/rotation?rot=1'>90&#176;</a> | "
            "<a href='/rotation?rot=2'>180&#176;</a> | "
            "<a href='/rotation?rot=3'>270&#176;</a>"
            "</p>"
            "<p><i>Telnet console on port 23</i></p>"));
    });

    http.on("/health", HTTP_GET, [this]() {
      String json;
      json.reserve(256);
      json += "{";
      json += "\"fw\":\"" + String(Config::FW_VERSION) + "\",";
      json += "\"uptime_ms\":" + String(millis()) + ",";
      json += "\"heap_free\":" + String(ESP.getFreeHeap()) + ",";
      json += "\"wifi\":\"" + String(wlStatusName(WiFi.status())) + "\",";
      json += "\"rssi\":" + String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0) + ",";
      json += "\"ip\":\"" +
              (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("")) + "\",";
      if (displayManager && displayManager->getCurrentMode()) {
        json += "\"mode\":\"" + String(displayManager->getCurrentMode()->getName()) + "\"";
      } else {
        json += "\"mode\":null";
      }
      json += "}";
      http.send(200, "application/json", json);
    });

    http.on("/mode", HTTP_GET, [this]() {
      if (!displayManager) {
        http.send(500, "text/plain", "Display manager not initialized");
        return;
      }

      if (http.hasArg("m")) {
        String mode = http.arg("m");
        if (mode.length() > 16) {
          http.send(400, "text/plain", "Bad request");
          return;
        }
        mode.toLowerCase();
        setModeByName(displayManager, mode, statusMode, boingMode, weatherMode);
      }

      http.sendHeader("Location", "/");
      http.send(302, "text/plain", "");
    });

    http.on("/oled", HTTP_GET, [this]() {
      if (http.hasArg("on")) {
        Config::runtime.oledEnabled = (http.arg("on") == "1");
        Logger::printf("OLED: %s", Config::runtime.oledEnabled ? "enabled" : "disabled");

        if (!Config::runtime.oledEnabled && displayManager) {
          displayManager->clear();
        } else if (Config::runtime.oledEnabled && displayManager) {
          displayManager->begin();
        }
      }

      http.sendHeader("Location", "/");
      http.send(302, "text/plain", "");
    });

    http.on("/oledcfg", HTTP_GET, [this]() {
      if (http.hasArg("drv")) {
        String d = http.arg("drv");
        if (d.length() > 16) {
          http.send(400, "text/plain", "Bad request");
          return;
        }
        d.toLowerCase();
        Config::runtime.driver =
            (d == "ssd1306") ? Config::OledDriver::SSD1306 : Config::OledDriver::SH1106;
      }

      if (http.hasArg("xoff")) {
        int val = http.arg("xoff").toInt();
        if (val >= -20 && val <= 20) {
          Config::runtime.xOffset = val;
        }
      }

      Logger::printf("OLED config: drv=%s xoff=%d", Config::runtime.getDriverName(),
                     Config::runtime.xOffset);

      if (displayManager) {
        displayManager->selectDriver();
      }

      http.sendHeader("Location", "/mode?m=status");
      http.send(302, "text/plain", "");
    });

    http.on("/rotation", HTTP_GET, [this]() {
      if (http.hasArg("rot")) {
        int val = http.arg("rot").toInt();
        if (val >= 0 && val <= 3 && displayManager) {
          displayManager->setRotation(static_cast<Config::DisplayRotation>(val));
        }
      }
      http.sendHeader("Location", "/");
      http.send(302, "text/plain", "");
    });

    http.on("/reboot", HTTP_GET, [this]() {
      http.send(200, "text/plain", "Rebooting...");
      delay(100);
      ESP.restart();
    });

    http.onNotFound([this]() { http.send(404, "text/plain", "Not found"); });
  }

 public:
  NetworkManager()
      : http(Config::HTTP_PORT),
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
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.hostname(Config::HOSTNAME);
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASS);

    Logger::printf("WiFi: connecting to '%s'...", Config::WIFI_SSID);

    setupRoutes();
    ElegantOTA.begin(&http, Config::OTA_USER, Config::OTA_PASS);
    http.begin();

    Logger::println("HTTP: server started on port 80");
    Logger::println("OTA: /update (basic auth enabled)");
  }

  void update() {
    http.handleClient();
    ElegantOTA.loop();

    wl_status_t status = WiFi.status();
    if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST || status == WL_DISCONNECTED) {
      uint32_t now = millis();
      if (now - wifiRetryMs >= wifiRetryIntervalMs) {
        Logger::printf("WiFi: reconnecting (interval=%lus)...", wifiRetryIntervalMs / 1000);
        WiFi.disconnect();
        WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASS);
        wifiRetryMs = now;
        wifiRetryIntervalMs = min(wifiRetryIntervalMs * 2, WIFI_RETRY_MAX_MS);
      }
    } else if (status == WL_CONNECTED) {
      wifiRetryIntervalMs = 5000;
    }
  }

  void logStatus() {
    Logger::printf("WiFi: %s ip=%s rssi=%d heap=%u", wlStatusName(WiFi.status()),
                   (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "(unset)",
                   (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0, ESP.getFreeHeap());
  }
};
