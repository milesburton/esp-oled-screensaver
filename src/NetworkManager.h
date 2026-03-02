#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include <ElegantOTA.h>

#include "Config.h"
#include "CredentialsManager.h"
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

  bool apMode = false;
  bool apFallbackActive = false;
  static constexpr const char* AP_SSID = "esp-oled-setup";
  static constexpr const char* AP_PASS = "setup1234";

  uint32_t wifiRetryMs = 0;
  uint32_t wifiRetryIntervalMs = 5000;
  static constexpr uint32_t WIFI_RETRY_MAX_MS = 5 * 60 * 1000;
  uint32_t disconnectedStartMs = 0;
  static constexpr uint32_t AP_FALLBACK_TIMEOUT_MS = 60 * 1000;

  char activeSsid[32] = {0};
  char activePass[64] = {0};

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

  void startSetupAP(bool fallbackFromSta) {
    if (apMode) {
      return;
    }

    if (fallbackFromSta) {
      WiFi.mode(WIFI_AP_STA);
      apFallbackActive = true;
    } else {
      WiFi.mode(WIFI_AP);
      apFallbackActive = false;
    }

    apMode = true;
    WiFi.softAP(AP_SSID, AP_PASS);
    Logger::printf("WiFi: setup AP active ssid='%s' ip=%s", AP_SSID,
                   WiFi.softAPIP().toString().c_str());
  }

  void stopFallbackAPIfConnected() {
    if (!apFallbackActive) {
      return;
    }

    WiFi.softAPdisconnect(true);
    apMode = false;
    apFallbackActive = false;
    disconnectedStartMs = 0;
    Logger::println("WiFi: connected, fallback AP disabled");
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
            "<p><a href='/wifi'>WiFi Setup</a> | <a href='/clear-eeprom'>Clear Saved WiFi</a></p>"
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

    http.on("/wifi", HTTP_GET, [this]() {
      String html;
      html.reserve(900);
      html += "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
      html += "<title>WiFi Setup</title></head><body>";
      html += "<h2>WiFi Setup</h2>";
      html += "<form method='POST' action='/save-wifi'>";
      html += "SSID:<br><input name='ssid' maxlength='31' required><br><br>";
      html += "Password:<br><input name='pass' type='password' maxlength='63'><br><br>";
      html += "<button type='submit'>Save & Reboot</button></form>";
      html += "</body></html>";
      http.send(200, "text/html", html);
    });

    http.on("/save-wifi", HTTP_POST, [this]() {
      if (!http.hasArg("ssid") || !http.hasArg("pass")) {
        http.send(400, "text/plain", "Missing SSID/password");
        return;
      }

      String ssid = http.arg("ssid");
      String pass = http.arg("pass");

      if (ssid.length() == 0 || ssid.length() > 31 || pass.length() > 63) {
        http.send(400, "text/plain", "Invalid SSID/password length");
        return;
      }

      if (!CredentialsManager::saveCredentials(ssid.c_str(), pass.c_str())) {
        http.send(500, "text/plain", "Failed to save credentials");
        return;
      }

      http.send(200, "text/html", "<html><body><h3>Saved. Rebooting...</h3></body></html>");
      delay(500);
      ESP.restart();
    });

    http.on("/clear-eeprom", HTTP_GET, [this]() {
      CredentialsManager::clearCredentials();
      http.send(200, "text/plain", "EEPROM credentials cleared, rebooting...");
      delay(500);
      ESP.restart();
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
    CredentialsManager::initialize();

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.hostname(Config::HOSTNAME);

    if (CredentialsManager::loadCredentials(activeSsid, sizeof(activeSsid), activePass,
                                            sizeof(activePass))) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(activeSsid, activePass);
      Logger::printf("WiFi: using EEPROM credentials '%s'", activeSsid);
    } else if (strlen(Config::WIFI_SSID) > 0) {
      strncpy(activeSsid, Config::WIFI_SSID, sizeof(activeSsid) - 1);
      strncpy(activePass, Config::WIFI_PASS, sizeof(activePass) - 1);
      CredentialsManager::saveCredentials(activeSsid, activePass);
      WiFi.mode(WIFI_STA);
      WiFi.begin(activeSsid, activePass);
      Logger::printf("WiFi: using compiled credentials '%s'", activeSsid);
    } else {
      Logger::println("WiFi: no credentials found, starting setup AP");
      startSetupAP(false);
    }

    setupRoutes();
    ElegantOTA.begin(&http, Config::OTA_USER, Config::OTA_PASS);
    http.begin();

    Logger::println("HTTP: server started on port 80");
    Logger::println("OTA: /update (basic auth enabled)");
  }

  void update() {
    http.handleClient();
    ElegantOTA.loop();

    if (apMode && !apFallbackActive) {
      return;
    }

    wl_status_t status = WiFi.status();
    if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST || status == WL_DISCONNECTED ||
        status == WL_NO_SSID_AVAIL) {
      uint32_t now = millis();

      if (disconnectedStartMs == 0) {
        disconnectedStartMs = now;
      }

      if (!apMode && now - disconnectedStartMs >= AP_FALLBACK_TIMEOUT_MS) {
        Logger::println("WiFi: offline too long, enabling fallback setup AP");
        startSetupAP(true);
      }

      if (now - wifiRetryMs >= wifiRetryIntervalMs) {
        Logger::printf("WiFi: reconnecting (interval=%lus)...", wifiRetryIntervalMs / 1000);
        WiFi.disconnect();
        WiFi.begin(activeSsid[0] ? activeSsid : Config::WIFI_SSID,
                   activePass[0] ? activePass : Config::WIFI_PASS);
        wifiRetryMs = now;
        wifiRetryIntervalMs = min(wifiRetryIntervalMs * 2, WIFI_RETRY_MAX_MS);
      }
    } else if (status == WL_CONNECTED) {
      wifiRetryIntervalMs = 5000;
      disconnectedStartMs = 0;
      stopFallbackAPIfConnected();
    }
  }

  void logStatus() {
    if (apMode) {
      Logger::printf("WiFi: AP mode ip=%s heap=%u", WiFi.softAPIP().toString().c_str(),
                     ESP.getFreeHeap());
      return;
    }

    Logger::printf("WiFi: %s ip=%s rssi=%d heap=%u", wlStatusName(WiFi.status()),
                   (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "(unset)",
                   (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0, ESP.getFreeHeap());
  }
};
