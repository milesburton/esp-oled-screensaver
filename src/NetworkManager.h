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

  // AP Mode
  bool apMode = false;
  static constexpr const char* AP_SSID = "esp-oled-setup";
  static constexpr const char* AP_PASS = "setup1234";

  // WiFi reconnection state
  uint32_t wifiRetryMs = 0;
  uint32_t wifiRetryIntervalMs = 5000;  // Start at 5s
  static constexpr uint32_t WIFI_RETRY_MAX_MS = 5 * 60 * 1000;  // Cap at 5 min

  // Connection failure tracking for auto-clear EEPROM
  uint32_t connectionFailureStartMs = 0;
  uint32_t connectionFailureCount = 0;
  static constexpr uint32_t CONNECTION_FAILURE_THRESHOLD = 10;
  static constexpr uint32_t CONNECTION_FAILURE_WINDOW_MS = 5 * 60 * 1000;  // 5 min

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
    // Root page
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

      html += "<p><i>Telnet console on port 23</i></p>";

      http.send(200, "text/html", html);
    });

    // Mode switching
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

    // OLED on/off
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

    // OLED configuration
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

    // Reboot
    http.on("/reboot", HTTP_GET, [this]() {
      http.send(200, "text/plain", "Rebooting...");
      delay(100);
      ESP.restart();
    });

    // WiFi setup form (AP mode)
    http.on("/wifi", HTTP_GET, [this]() {
      String html;
      html.reserve(1500);
      html += "<!DOCTYPE html><html><head>";
      html += "<title>WiFi Setup</title>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>";
      html += "body { font-family: Arial; margin: 20px; }";
      html += "input { padding: 8px; width: 100%; margin: 5px 0; box-sizing: border-box; }";
      html += "button { padding: 10px; width: 100%; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }";
      html += "button:hover { background: #0056b3; }";
      html += ".error { color: red; margin: 10px 0; }";
      html += "</style></head><body>";
      html += "<h1>WiFi Configuration</h1>";
      html += "<form method='POST' action='/save-wifi'>";
      html += "<label>SSID (Network Name):</label>";
      html += "<input type='text' name='ssid' required><br>";
      html += "<label>Password:</label>";
      html += "<input type='password' name='pass' required><br>";
      html += "<button type='submit'>Save & Connect</button>";
      html += "</form>";
      html += "<p><small>Device will restart after saving.</small></p>";
      html += "</body></html>";

      http.send(200, "text/html", html);
    });

    // Save WiFi credentials (AP mode form submission)
    http.on("/save-wifi", HTTP_POST, [this]() {
      if (!http.hasArg("ssid") || !http.hasArg("pass")) {
        http.send(400, "text/plain", "Missing SSID or password");
        return;
      }

      String ssid = http.arg("ssid");
      String pass = http.arg("pass");

      if (ssid.length() == 0 || ssid.length() > 31) {
        http.send(400, "text/plain", "SSID must be 1-31 characters");
        return;
      }

      if (pass.length() > 63) {
        http.send(400, "text/plain", "Password must be 0-63 characters");
        return;
      }

      // Save to EEPROM
      if (CredentialsManager::saveCredentials(ssid.c_str(), pass.c_str())) {
        http.send(200, "text/html",
                  "<html><body><h1>Saved!</h1><p>Device will restart and connect to your "
                  "network...</p></body></html>");
        delay(500);
        ESP.restart();
      } else {
        http.send(500, "text/plain", "Failed to save credentials");
      }
    });

    // Clear EEPROM credentials
    http.on("/clear-eeprom", HTTP_GET, [this]() {
      CredentialsManager::clearCredentials();
      Logger::println("HTTP: EEPROM cleared via /clear-eeprom");
      http.send(200, "text/html",
                "<html><body><h1>Cleared!</h1><p>EEPROM credentials cleared. Device will "
                "restart...</p></body></html>");
      delay(500);
      ESP.restart();
    });

    // 404
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
    // Initialize EEPROM
    CredentialsManager::initialize();

    WiFi.persistent(false);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    // Step 1: Try to load credentials from EEPROM
    if (CredentialsManager::loadCredentials(Config::runtimeWiFi.ssid,
                                             sizeof(Config::runtimeWiFi.ssid),
                                             Config::runtimeWiFi.password,
                                             sizeof(Config::runtimeWiFi.password))) {
      // Use EEPROM credentials
      Logger::println("WiFi: using EEPROM credentials");
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      WiFi.hostname(Config::HOSTNAME);
      WiFi.begin(Config::runtimeWiFi.ssid, Config::runtimeWiFi.password);
      Logger::printf("WiFi: connecting to '%s'...", Config::runtimeWiFi.ssid);
    } else if (strlen(Config::WIFI_SSID) > 0) {  // Step 2: Check if compiled credentials exist (non-empty)
      Logger::println("WiFi: saving compiled credentials to EEPROM");
      CredentialsManager::saveCredentials(Config::WIFI_SSID, Config::WIFI_PASS);

      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      WiFi.hostname(Config::HOSTNAME);
      WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASS);
      Logger::printf("WiFi: connecting to '%s'...", Config::WIFI_SSID);
    } else {  // Step 3: No credentials - start AP mode for setup
      Logger::println("WiFi: no credentials found, starting AP mode");
      apMode = true;
      WiFi.mode(WIFI_AP);
      WiFi.softAP(AP_SSID, AP_PASS);
      Logger::printf("WiFi: AP mode — SSID='%s' ip=192.168.4.1", AP_SSID);
    }

    // HTTP server setup
    setupRoutes();
    ElegantOTA.begin(&http, Config::OTA_USER, Config::OTA_PASS);
    http.begin();

    Logger::println("HTTP: server started on port 80");
    Logger::println("OTA: /update (basic auth enabled)");
  }

  void update() {
    http.handleClient();
    ElegantOTA.loop();

    // If in AP mode, no WiFi reconnection logic needed
    if (apMode) {
      return;
    }

    wl_status_t status = WiFi.status();

    // Track connection failures for auto-clear
    if (status == WL_CONNECT_FAILED || status == WL_CONNECTION_LOST ||
        status == WL_DISCONNECTED) {
      uint32_t now = millis();

      // Initialize failure window if this is the first failure
      if (connectionFailureCount == 0) {
        connectionFailureStartMs = now;
      }

      // Reset counter if outside the window
      if (now - connectionFailureStartMs > CONNECTION_FAILURE_WINDOW_MS) {
        connectionFailureCount = 0;
        connectionFailureStartMs = now;
      }

      // Standard reconnection logic with exponential backoff
      if (now - wifiRetryMs >= wifiRetryIntervalMs) {
        connectionFailureCount++;
        Logger::printf("WiFi: reconnecting attempt %u (interval=%lus)...",
                       connectionFailureCount, wifiRetryIntervalMs / 1000);

        WiFi.disconnect();
        WiFi.begin(Config::runtimeWiFi.ssid[0] != 0 ? Config::runtimeWiFi.ssid
                                                      : Config::WIFI_SSID,
                   Config::runtimeWiFi.password[0] != 0 ? Config::runtimeWiFi.password
                                                         : Config::WIFI_PASS);
        wifiRetryMs = now;
        wifiRetryIntervalMs = min(wifiRetryIntervalMs * 2, WIFI_RETRY_MAX_MS);

        // Auto-clear EEPROM if too many failures
        if (connectionFailureCount >= CONNECTION_FAILURE_THRESHOLD) {
          Logger::printf("WiFi: too many failures (%u), clearing EEPROM and restarting",
                         connectionFailureCount);
          CredentialsManager::clearCredentials();
          delay(500);
          ESP.restart();
        }
      }
    } else if (status == WL_CONNECTED) {
      // Reset backoff on successful connection
      wifiRetryIntervalMs = 5000;
      connectionFailureCount = 0;
    }
  }

  void logStatus() {
    if (apMode) {
      Logger::printf("WiFi: AP mode ip=192.168.4.1 heap=%u", ESP.getFreeHeap());
      return;
    }

    Logger::printf("WiFi: %s ip=%s rssi=%d heap=%u", wlStatusName(WiFi.status()),
                   (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "(unset)",
                   (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0, ESP.getFreeHeap());
  }
};
