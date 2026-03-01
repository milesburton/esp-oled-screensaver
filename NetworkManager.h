#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include "Config.h"
#include "Logger.h"
#include "DisplayManager.h"

// Forward declarations
class StatusMode;
class BoingMode;
class WeatherMode;

class NetworkManager {
private:
  ESP8266WebServer http;
  DisplayManager* displayManager;
  StatusMode* statusMode;
  BoingMode* boingMode;
  WeatherMode* weatherMode;
  
  static const char* wlStatusName(wl_status_t s) {
    switch (s) {
      case WL_IDLE_STATUS:     return "IDLE";
      case WL_NO_SSID_AVAIL:   return "NO_SSID";
      case WL_SCAN_COMPLETED:  return "SCAN_DONE";
      case WL_CONNECTED:       return "CONNECTED";
      case WL_CONNECT_FAILED:  return "CONNECT_FAILED";
      case WL_CONNECTION_LOST: return "CONNECTION_LOST";
      case WL_DISCONNECTED:    return "DISCONNECTED";
      default:                 return "UNKNOWN";
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
      html += "<li><b>RSSI:</b> " + String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0) + "</li>";
      html += "<li><b>Heap:</b> " + String(ESP.getFreeHeap()) + "</li>";
      
      if (displayManager && displayManager->getCurrentMode()) {
        html += "<li><b>Mode:</b> " + String(displayManager->getCurrentMode()->getName()) + "</li>";
      }
      
      html += "<li><b>OLED:</b> " + String(Config::runtime.oledEnabled ? "on" : "off") + "</li>";
      html += "<li><b>Driver:</b> " + String(Config::runtime.getDriverName()) + "</li>";
      html += "<li><b>X offset:</b> " + String(Config::runtime.xOffset) + "</li>";
      html += "<li><b>I2C:</b> SDA=" + String(Config::OLED_SDA) + " SCL=" + String(Config::OLED_SCL) + " addr=0x" + String(Config::OLED_ADDR, HEX) + "</li>";
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
      html += "<li><a href='/oledcfg?drv=sh1106&xoff=2'>SH1106 xoff=2 (common)</a></li>";
      html += "<li><a href='/oledcfg?drv=sh1106&xoff=0'>SH1106 xoff=0</a></li>";
      html += "<li><a href='/oledcfg?drv=ssd1306&xoff=0'>SSD1306 xoff=0 (common)</a></li>";
      html += "<li><a href='/oledcfg?drv=ssd1306&xoff=2'>SSD1306 xoff=2</a></li>";
      html += "</ul>";
      html += "<p><a href='/oled?on=1'>OLED ON</a> | <a href='/oled?on=0'>OLED OFF</a></p>";
      
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
        mode.toLowerCase();
        
        if (mode == "status" && statusMode) {
          displayManager->setMode(statusMode, 400);  // 2.5 FPS
        } else if (mode == "boing" && boingMode) {
          displayManager->setMode(boingMode, 40);    // 25 FPS
        } else if (mode == "weather" && weatherMode) {
          displayManager->setMode(weatherMode, 5000); // Every 5 seconds
        }
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
        d.toLowerCase();
        Config::runtime.driver = (d == "ssd1306") ? Config::OledDriver::SSD1306 : Config::OledDriver::SH1106;
      }
      
      if (http.hasArg("xoff")) {
        Config::runtime.xOffset = http.arg("xoff").toInt();
      }
      
      Logger::printf("OLED config: drv=%s xoff=%d",
                     Config::runtime.getDriverName(),
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
    
    // 404
    http.onNotFound([this]() {
      http.send(404, "text/plain", "Not found");
    });
  }
  
public:
  NetworkManager() 
    : http(Config::HTTP_PORT)
    , displayManager(nullptr)
    , statusMode(nullptr)
    , boingMode(nullptr)
    , weatherMode(nullptr)
  {}
  
  void setDisplayManager(DisplayManager* dm) {
    displayManager = dm;
  }
  
  void setModes(StatusMode* status, BoingMode* boing, WeatherMode* weather) {
    statusMode = status;
    boingMode = boing;
    weatherMode = weather;
  }
  
  void begin() {
    // WiFi setup
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.hostname(Config::HOSTNAME);
    WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASS);
    
    Logger::printf("WiFi: connecting to '%s'...", Config::WIFI_SSID);
    
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
  }
  
  void logStatus() {
    Logger::printf("WiFi: %s ip=%s rssi=%d heap=%u",
                   wlStatusName(WiFi.status()),
                   (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "(unset)",
                   (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0,
                   ESP.getFreeHeap());
  }
};
