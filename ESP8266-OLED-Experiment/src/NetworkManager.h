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
      String html;
      html.reserve(2000);

      html += "<h1>" + String(Config::HOSTNAME) + "</h1>";
      html += "<p><b>FW:</b> " + String(Config::FW_VERSION) + "</p>";
      html += "<ul>";
      html += "<li><b>WiFi:</b> " + String(wlStatusName(WiFi.status())) + "</li>";
      html += "<li><b>IP:</b> " + WiFi.localIP().toString() + "</li>";
      html +=
          "<li><b>RSSI:</b> " + String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0) + "</li>";
      html += "<li><b>Heap:</b> " + String(ESP.getFreeHeap()) + "</li>";

      if (displayManager && displayManager->getCurrentMode()) {
        html += "<li><b>Mode:</b> " + String(displayManager->getCurrentMode()->getName()) + "</li>";
      }

      html += "<li><b>OLED:</b> " + String(Config::runtime.oledEnabled ? "on" : "off") + "</li>";
      html += "<li><b>Driver:</b> " + String(Config::runtime.getDriverName()) + "</li>";
      html += "<li><b>Rotation:</b> " + String(Config::runtime.getRotationName()) + "</li>";
      html += "<li><b>X offset:</b> " + String(Config::runtime.xOffset) + "</li>";
      html += "<li><b>I2C:</b> SDA=" + String(Config::OLED_SDA) +
              " SCL=" + String(Config::OLED_SCL) + " addr=0x" + String(Config::OLED_ADDR, HEX) +
              "</li>";
      html += "</ul>";

      html += "<p><a href='/update'>OTA Update</a></p>";

      html += "<h3>Display Mode</h3>";
      html += "<p>";
      html += "<a href='/mode?m=status'>Status</a> | ";
      html += "<a href='/mode?m=boing'>Boing</a> | ";
      html += "<a href='/mode?m=weather'>Weather</a>";
      html += "</p>";

      html += "<h3>OLED Configuration</h3>";
      html += "<p>Try these until border + text align perfectly:</p>";
      html += "<ul>";
      html +=
          "<li><a href='/oledcfg?drv=sh1106&xoff=2'>SH1106 xoff=2 "
          "(common)</a></li>";
      html += "<li><a href='/oledcfg?drv=sh1106&xoff=0'>SH1106 xoff=0</a></li>";
      html +=
          "<li><a href='/oledcfg?drv=ssd1306&xoff=0'>SSD1306 xoff=0 "
          "(common)</a></li>";
      html += "<li><a href='/oledcfg?drv=ssd1306&xoff=2'>SSD1306 xoff=2</a></li>";
      html += "</ul>";
      html +=
          "<p><a href='/oled?on=1'>OLED ON</a> | <a href='/oled?on=0'>OLED "
          "OFF</a></p>";

      html += "<h3>Display Rotation</h3>";
      html += "<p>";
      html += "<a href='/rotation?rot=0'>0°</a> | ";
      html += "<a href='/rotation?rot=1'>90°</a> | ";
      html += "<a href='/rotation?rot=2'>180°</a> | ";
      html += "<a href='/rotation?rot=3'>270°</a>";
      html += "</p>";

      html += "<p><i>Telnet console on port 23</i></p>";

      http.send(200, "text/html", html);
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
