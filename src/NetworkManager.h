#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ElegantOTA.h>

#include "AutoUpdater.h"
#include "BreakoutMode.h"
#include "ClockMode.h"
#include "Config.h"
#include "CredentialsManager.h"
#include "DisplayManager.h"
#include "LifeMode.h"
#include "Logger.h"
#include "MatrixRainMode.h"
#include "ModeHelper.h"
#include "PacManMode.h"
#include "PlasmaMode.h"
#include "PongMode.h"
#include "ScreensaverMode.h"
#include "StarfieldMode.h"
#include "TunnelMode.h"
#include "UpdateChecker.h"
#include "UpdateManager.h"

class NetworkManager {
 private:
  ESP8266WebServer http;
  DNSServer dnsServer;
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
  MatrixRainMode* matrixMode;
  PlasmaMode* plasmaMode;
  TunnelMode* tunnelMode;
  PongMode* pongMode;

  bool apMode = false;
  bool apFallbackActive = false;
  static constexpr const char* AP_SSID = "ESP-OLED-Setup";
  static constexpr const char* AP_PASS = "";  // Open network for captive portal

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

    // Start DNS server for captive portal (redirect all domains to this device)
    dnsServer.start(53, "*", WiFi.softAPIP());

    Logger::printf("WiFi: captive portal active — SSID='%s' (open) ip=%s mode=%s", AP_SSID,
                   WiFi.softAPIP().toString().c_str(),
                   fallbackFromSta ? "AP+STA fallback" : "AP only");
  }

  void stopFallbackAPIfConnected() {
    if (!apFallbackActive) {
      return;
    }

    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    apMode = false;
    apFallbackActive = false;
    disconnectedStartMs = 0;
    Logger::println("WiFi: connected, captive portal disabled");
  }

  void setupRoutes() {
    // Captive portal detection routes (for iOS, Android, Windows)
    http.on("/generate_204", HTTP_GET, [this]() {  // Android
      http.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/wifi");
      http.send(302, "text/plain", "");
    });
    http.on("/hotspot-detect.html", HTTP_GET, [this]() {  // iOS/macOS
      http.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/wifi");
      http.send(302, "text/plain", "");
    });
    http.on("/connecttest.txt", HTTP_GET, [this]() {  // Windows
      http.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/wifi");
      http.send(302, "text/plain", "");
    });
    http.on("/redirect", HTTP_GET, [this]() {  // Windows
      http.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/wifi");
      http.send(302, "text/plain", "");
    });

    // Root page
    http.on("/", HTTP_GET, [this]() {
      if (apMode) {
        http.sendHeader("Location", "/wifi");
        http.send(302, "text/plain", "");
        return;
      }
      http.setContentLength(CONTENT_LENGTH_UNKNOWN);
      http.send(200, F("text/html; charset=utf-8"), "");

      http.sendContent(
          F("<!doctype html><html lang='en'><head>"
            "<meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width,initial-scale=1'>"
            "<title>OLED Screensaver</title>"
            "<script src='https://cdn.tailwindcss.com'></script>"
            "<script>tailwind.config={theme:{extend:{colors:{accent:'#4ade80'}}}}</script>"
            "</head>"
            "<body class='bg-gray-950 text-gray-100 min-h-screen p-4 font-mono'>"
            "<div class='max-w-lg mx-auto space-y-4'>"
            "<div class='flex items-center justify-between'>"
            "<h1 class='text-xl font-bold text-accent'>"));
      http.sendContent(Config::HOSTNAME);
      http.sendContent(F("</h1><span class='text-xs text-gray-500'>"));
      http.sendContent(Config::FW_VERSION);
      http.sendContent(F("</span></div>"));

      // Device card
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-2'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>Device</h2>"
            "<div class='grid grid-cols-2 gap-x-4 gap-y-1 text-sm'>"));
      http.sendContent(F("<span class='text-gray-400'>WiFi</span><span class='text-right'>"));
      http.sendContent(wlStatusName(WiFi.status()));
      http.sendContent(F("</span><span class='text-gray-400'>IP</span><span class='text-right'>"));
      http.sendContent(WiFi.localIP().toString());
      http.sendContent(
          F("</span><span class='text-gray-400'>RSSI</span><span class='text-right'>"));
      http.sendContent(String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0));
      http.sendContent(
          F(" dBm</span><span class='text-gray-400'>Heap</span><span class='text-right'>"));
      http.sendContent(String(ESP.getFreeHeap()));
      http.sendContent(F(" B</span>"));
      if (displayManager && displayManager->getCurrentMode()) {
        http.sendContent(
            F("<span class='text-gray-400'>Mode</span><span class='text-right text-accent'>"));
        http.sendContent(displayManager->getCurrentMode()->getName());
        http.sendContent(F("</span>"));
      }
      http.sendContent(F("</div></div>"));

      // OLED card
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-2'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>OLED</h2>"
            "<div class='grid grid-cols-2 gap-x-4 gap-y-1 text-sm'>"));
      http.sendContent(F("<span class='text-gray-400'>Driver</span><span class='text-right'>"));
      http.sendContent(Config::runtime.getDriverName());
      http.sendContent(
          F("</span><span class='text-gray-400'>Rotation</span><span class='text-right'>"));
      http.sendContent(Config::runtime.getRotationName());
      http.sendContent(
          F("</span><span class='text-gray-400'>X Offset</span><span class='text-right'>"));
      http.sendContent(String(Config::runtime.xOffset));
      http.sendContent(
          F("</span><span class='text-gray-400'>I2C SDA</span><span class='text-right'>GPIO "));
      http.sendContent(String(Config::OLED_SDA));
      http.sendContent(
          F("</span><span class='text-gray-400'>I2C SCL</span><span class='text-right'>GPIO "));
      http.sendContent(String(Config::OLED_SCL));
      http.sendContent(F("</span></div></div>"));

      // Diagnostics card
      bool credLoaded = CredentialsManager::hasValidCredentials();
      uint32_t freeHeap = ESP.getFreeHeap();
      uint32_t sketchSize = ESP.getSketchSize();
      uint8_t flashUsed = static_cast<uint8_t>((sketchSize * 100) / ESP.getFlashChipSize());
      uint8_t heapPct = static_cast<uint8_t>((freeHeap * 100) / 81920);
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-2'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>"
            "Diagnostics</h2>"
            "<div class='grid grid-cols-2 gap-x-4 gap-y-1 text-sm'>"));
      http.sendContent(F("<span class='text-gray-400'>RAM Free</span><span class='text-right'>"));
      http.sendContent(String(freeHeap));
      http.sendContent(F(" B ("));
      http.sendContent(String(heapPct));
      http.sendContent(F("%)</span>"));
      http.sendContent(F("<span class='text-gray-400'>Flash Used</span><span class='text-right'>"));
      http.sendContent(String(flashUsed));
      http.sendContent(F("%</span>"));
      http.sendContent(F("<span class='text-gray-400'>WiFi Creds</span><span class='text-right'>"));
      http.sendContent(credLoaded ? "EEPROM" : "Compiled");
      http.sendContent(F("</span></div></div>"));

      // Auto-Update card
      bool autoUpdateEnabled = UpdateManager::isAutoUpdateEnabled();
      bool updateAvail = UpdateChecker::isUpdateAvailable();
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-3'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>"
            "Auto-Update</h2>"
            "<div class='grid grid-cols-2 gap-x-4 gap-y-1 text-sm'>"));
      http.sendContent(F("<span class='text-gray-400'>Status</span><span class='text-right'>"));
      http.sendContent(autoUpdateEnabled ? "<span class='text-accent'>Enabled</span>"
                                         : "<span class='text-gray-500'>Disabled</span>");
      http.sendContent(
          F("</span><span class='text-gray-400'>Channel</span><span class='text-right'>"));
      http.sendContent(UpdateManager::getUpdateChannel() == UpdateManager::CHANNEL_STABLE ? "Stable"
                                                                                          : "Beta");
      http.sendContent(
          F("</span><span class='text-gray-400'>Update</span><span class='text-right'>"));
      if (updateAvail) {
        http.sendContent(F("<span class='text-yellow-400'>v"));
        http.sendContent(UpdateChecker::getAvailableVersion());
        http.sendContent(F(" available</span>"));
      } else {
        http.sendContent(F("<span class='text-gray-500'>Up to date</span>"));
      }
      http.sendContent(F("</span></div>"));
      http.sendContent(F("<div class='flex flex-wrap gap-2'>"));
      http.sendContent(autoUpdateEnabled
                           ? "<a href='/autoupdate?on=0' class='px-3 py-1 rounded-lg bg-gray-700 "
                             "hover:bg-gray-600 text-sm'>Disable</a>"
                           : "<a href='/autoupdate?on=1' class='px-3 py-1 rounded-lg bg-accent "
                             "text-gray-950 hover:opacity-90 text-sm font-semibold'>Enable</a>");
      http.sendContent(
          F("<a href='/check-update' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Check Now</a>"
            "<a href='/update' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>OTA Flash</a>"
            "</div></div>"));

      // Display modes card
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-3'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>"
            "Display Mode</h2>"
            "<div class='flex flex-wrap gap-2'>"
            "<a href='/mode?m=screensaver' class='px-3 py-1 rounded-lg bg-accent text-gray-950 "
            "text-sm font-semibold'>Screensaver</a>"
            "<a href='/mode?m=clock' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Clock</a>"
            "<a href='/mode?m=boing' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Boing</a>"
            "<a href='/mode?m=weather' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Weather</a>"
            "<a href='/mode?m=breakout' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Breakout</a>"
            "<a href='/mode?m=pacman' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Pac-Man</a>"
            "<a href='/mode?m=starfield' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Starfield</a>"
            "<a href='/mode?m=life' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Life</a>"
            "<a href='/mode?m=matrix' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Matrix</a>"
            "<a href='/mode?m=plasma' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Plasma</a>"
            "<a href='/mode?m=tunnel' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Tunnel</a>"
            "<a href='/mode?m=pong' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Pong</a>"
            "<a href='/mode?m=status' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Status</a>"
            "</div></div>"));

      // OLED config card
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-3'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>"
            "OLED Config</h2>"
            "<div class='flex flex-wrap gap-2'>"
            "<a href='/oledcfg?drv=ssd1306&xoff=0' class='px-3 py-1 rounded-lg bg-gray-700 "
            "hover:bg-gray-600 text-sm'>SSD1306 x0</a>"
            "<a href='/oledcfg?drv=ssd1306&xoff=2' class='px-3 py-1 rounded-lg bg-gray-700 "
            "hover:bg-gray-600 text-sm'>SSD1306 x2</a>"
            "<a href='/oledcfg?drv=sh1106&xoff=0' class='px-3 py-1 rounded-lg bg-gray-700 "
            "hover:bg-gray-600 text-sm'>SH1106 x0</a>"
            "<a href='/oledcfg?drv=sh1106&xoff=2' class='px-3 py-1 rounded-lg bg-gray-700 "
            "hover:bg-gray-600 text-sm'>SH1106 x2</a>"
            "</div>"
            "<div class='flex flex-wrap gap-2'>"
            "<span class='text-xs text-gray-400 w-full'>Rotation</span>"
            "<a href='/rotation?rot=0' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>0&#176;</a>"
            "<a href='/rotation?rot=1' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>90&#176;</a>"
            "<a href='/rotation?rot=2' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>180&#176;</a>"
            "<a href='/rotation?rot=3' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>270&#176;</a>"
            "</div>"
            "<div class='flex gap-2'>"
            "<a href='/oled?on=1' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>OLED On</a>"
            "<a href='/oled?on=0' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>OLED Off</a>"
            "</div></div>"));

      // System card
      http.sendContent(
          F("<div class='bg-gray-900 rounded-xl p-4 space-y-3'>"
            "<h2 class='text-xs font-semibold text-gray-400 uppercase tracking-wider'>System</h2>"
            "<div class='flex flex-wrap gap-2'>"
            "<a href='/wifi' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>WiFi Setup</a>"
            "<a href='/clear-eeprom' class='px-3 py-1 rounded-lg bg-gray-700 hover:bg-gray-600 "
            "text-sm'>Clear WiFi</a>"
            "<a href='/reboot' class='px-3 py-1 rounded-lg bg-red-900 hover:bg-red-800 "
            "text-sm'>Reboot</a>"
            "</div>"
            "<p class='text-xs text-gray-500'>Telnet console: port 23</p>"
            "</div>"
            "</div></body></html>"));
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
        setModeByName(displayManager, mode, statusMode, boingMode, weatherMode, clockMode,
                      breakoutMode, pacManMode, screensaverMode, starfieldMode, lifeMode,
                      matrixMode, plasmaMode, tunnelMode, pongMode);
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
      http.send(200, F("text/html; charset=utf-8"),
                F("<!doctype html><html lang='en'><head>"
                  "<meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<title>WiFi Setup</title>"
                  "<script src='https://cdn.tailwindcss.com'></script>"
                  "</head>"
                  "<body class='bg-gray-950 text-gray-100 min-h-screen p-4 font-mono'>"
                  "<div class='max-w-lg mx-auto space-y-4'>"
                  "<h1 class='text-xl font-bold text-green-400'>WiFi Setup</h1>"
                  "<div class='bg-gray-900 rounded-xl p-4 space-y-4'>"
                  "<form method='POST' action='/save-wifi' class='space-y-4'>"
                  "<div class='space-y-1'>"
                  "<label class='text-xs text-gray-400 uppercase tracking-wider'>SSID</label>"
                  "<input name='ssid' maxlength='31' required autocomplete='off' "
                  "class='w-full bg-gray-800 rounded-lg px-3 py-2 text-sm outline-none "
                  "focus:ring-2 focus:ring-green-400'></div>"
                  "<div class='space-y-1'>"
                  "<label class='text-xs text-gray-400 uppercase tracking-wider'>Password</label>"
                  "<input name='pass' type='password' maxlength='63' autocomplete='off' "
                  "class='w-full bg-gray-800 rounded-lg px-3 py-2 text-sm outline-none "
                  "focus:ring-2 focus:ring-green-400'></div>"
                  "<button type='submit' class='w-full py-2 rounded-lg bg-green-400 "
                  "text-gray-950 font-semibold hover:opacity-90'>Save &amp; Reboot</button>"
                  "</form></div>"
                  "<a href='/' class='text-sm text-gray-500 hover:text-gray-300'>"
                  "&#8592; Back</a>"
                  "</div></body></html>"));
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

      http.send(200, F("text/html; charset=utf-8"),
                F("<!doctype html><html><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<script src='https://cdn.tailwindcss.com'></script></head>"
                  "<body class='bg-gray-950 text-gray-100 min-h-screen p-4 font-mono'>"
                  "<div class='max-w-lg mx-auto pt-12 text-center space-y-2'>"
                  "<p class='text-green-400 text-lg font-semibold'>Saved. Rebooting...</p>"
                  "<p class='text-gray-500 text-sm'>Device will reconnect shortly.</p>"
                  "</div></body></html>"));
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

    http.on("/autoupdate", HTTP_GET, [this]() {
      if (http.hasArg("on")) {
        bool enabled = (http.arg("on") == "1");
        UpdateManager::setAutoUpdateEnabled(enabled);
        Logger::printf("Auto-update: %s", enabled ? "enabled" : "disabled");
        if (enabled) {
          UpdateChecker::reset();
        }
      }
      http.sendHeader("Location", "/");
      http.send(302, "text/plain", "");
    });

    http.on("/check-update", HTTP_GET, [this]() {
      UpdateChecker::forceCheck();
      String json = "{";
      json += "\"auto_update_enabled\":" +
              String(UpdateManager::isAutoUpdateEnabled() ? "true" : "false") + ",";
      auto channel =
          (UpdateManager::getUpdateChannel() == UpdateManager::CHANNEL_STABLE) ? "stable" : "beta";
      json += "\"channel\":\"" + String(channel) + "\",";
      json += "\"fw_version\":\"" + String(Config::FW_VERSION) + "\",";
      json +=
          "\"update_available\":" + String(UpdateChecker::isUpdateAvailable() ? "true" : "false");
      if (UpdateChecker::isUpdateAvailable()) {
        json += ",\"available_version\":\"" + String(UpdateChecker::getAvailableVersion()) + "\"";
      }
      json += "}";
      http.send(200, "application/json", json);
    });

    http.on("/force-check", HTTP_GET, [this]() {
      // Force a manual update check regardless of auto-update setting or interval
      UpdateChecker::forceCheck();
      http.send(200, "application/json", "{\"status\":\"check triggered\"}");
    });

    http.on("/update-status", HTTP_GET, [this]() {
      // Report current auto-update status in JSON format
      String json = "{";
      json += "\"auto_update_enabled\":" +
              String(UpdateManager::isAutoUpdateEnabled() ? "true" : "false") + ",";
      json += "\"fw_version\":\"" + String(Config::FW_VERSION) + "\",";
      json +=
          "\"update_available\":" + String(UpdateChecker::isUpdateAvailable() ? "true" : "false") +
          ",";

      // Map updater state to string
      const char* stateStr = "UNKNOWN";
      switch (AutoUpdater::getState()) {
        case AutoUpdater::UpdateState::IDLE:
          stateStr = "IDLE";
          break;
        case AutoUpdater::UpdateState::DOWNLOADING:
          stateStr = "DOWNLOADING";
          break;
        case AutoUpdater::UpdateState::VALIDATING:
          stateStr = "VALIDATING";
          break;
        case AutoUpdater::UpdateState::FLASHING:
          stateStr = "FLASHING";
          break;
        case AutoUpdater::UpdateState::VERIFYING:
          stateStr = "VERIFYING";
          break;
        case AutoUpdater::UpdateState::SUCCESS:
          stateStr = "SUCCESS";
          break;
        case AutoUpdater::UpdateState::ERROR_DOWNLOAD:
          stateStr = "ERROR_DOWNLOAD";
          break;
        case AutoUpdater::UpdateState::ERROR_UNPACK:
          stateStr = "ERROR_UNPACK";
          break;
        case AutoUpdater::UpdateState::ERROR_FLASH:
          stateStr = "ERROR_FLASH";
          break;
        case AutoUpdater::UpdateState::ERROR_ABORT:
          stateStr = "ERROR_ABORT";
          break;
      }

      json += "\"updater_state\":\"" + String(stateStr) + "\",";
      json += "\"update_progress\":" + String(AutoUpdater::getProgress());

      if (UpdateChecker::isUpdateAvailable()) {
        json += ",\"available_version\":\"" + String(UpdateChecker::getAvailableVersion()) + "\"";
      }

      json += "}";
      http.send(200, "application/json", json);
    });

    http.onNotFound([this]() { http.send(404, "text/plain", "Not found"); });
  }

 public:
  NetworkManager()
      : http(Config::HTTP_PORT),
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
        matrixMode(nullptr),
        plasmaMode(nullptr),
        tunnelMode(nullptr),
        pongMode(nullptr) {}

  void setDisplayManager(DisplayManager* dm) { displayManager = dm; }

  void setModes(StatusMode* status, BoingMode* boing, WeatherMode* weather, ClockMode* clock,
                BreakoutMode* breakout, PacManMode* pacman, ScreensaverMode* screensaver,
                StarfieldMode* starfield, LifeMode* life, MatrixRainMode* matrix,
                PlasmaMode* plasma, TunnelMode* tunnel, PongMode* pong) {
    statusMode = status;
    boingMode = boing;
    weatherMode = weather;
    clockMode = clock;
    breakoutMode = breakout;
    pacManMode = pacman;
    screensaverMode = screensaver;
    starfieldMode = starfield;
    lifeMode = life;
    matrixMode = matrix;
    plasmaMode = plasma;
    tunnelMode = tunnel;
    pongMode = pong;
  }

  void begin() {
    CredentialsManager::initialize();
    UpdateManager::initialize();

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

    // Check for available updates periodically (every 6 hours if enabled)
    UpdateChecker::checkForUpdates();

    // Attempt automatic update if available and enabled
    AutoUpdater::attemptAutoUpdate();

    // Process DNS requests in AP mode for captive portal (wildcard redirect)
    if (apMode) {
      dnsServer.processNextRequest();
    }

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
