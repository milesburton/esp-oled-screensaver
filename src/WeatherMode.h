#pragma once

#include <Arduino.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include <WiFiClient.h>
#include <time.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"
#include "Logger.h"

class WeatherMode : public DisplayMode {
 private:
  enum class WxState : uint8_t { IDLE, DONE, ERROR };

  WxState _state = WxState::IDLE;
  uint32_t _lastFetchMs = 0;
  bool _synced = false;

  float _tempC = 0.0f;
  int _wmoCode = 0;
  char _lastUpdate[6] = {0};  // "HH:MM\0"

  WiFiClient _wifiClient;
  HTTPClient _http;

  static const char* wmoDescription(int code) {
    if (code == 0)
      return "Clear sky";
    if (code <= 3)
      return "Cloudy";
    if (code >= 45 && code <= 48)
      return "Fog";
    if (code >= 51 && code <= 67)
      return "Rain";
    if (code >= 71 && code <= 77)
      return "Snow";
    if (code >= 80 && code <= 82)
      return "Showers";
    if (code == 95)
      return "Thunderstorm";
    return "Unknown";
  }

  void parseResponse(const String& body) {
    // The response contains "current_units" before "current" — skip to the
    // "current":{...} object so we parse numeric values, not unit strings.
    const char* base = strstr(body.c_str(), "\"current\":{");
    if (!base)
      base = body.c_str();

    const char* t = strstr(base, "\"temperature_2m\":");
    if (t)
      _tempC = atof(t + 17);

    const char* w = strstr(base, "\"weather_code\":");
    if (w)
      _wmoCode = atoi(w + 15);

    time_t now = time(nullptr);
    if (now > 100000UL) {
      struct tm ti;
      localtime_r(&now, &ti);
      snprintf(_lastUpdate, sizeof(_lastUpdate), "%02d:%02d", ti.tm_hour, ti.tm_min);
    } else {
      snprintf(_lastUpdate, sizeof(_lastUpdate), "--:--");
    }
  }

  void startFetch() {
    if (WiFi.status() != WL_CONNECTED) {
      _state = WxState::ERROR;
      _lastFetchMs = millis();
      return;
    }

    char url[192];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code",
             Config::WEATHER_LATITUDE, Config::WEATHER_LONGITUDE);

    _http.begin(_wifiClient, url);
    _http.setTimeout(8000);
    int code = _http.GET();
    if (code == HTTP_CODE_OK) {
      parseResponse(_http.getString());
      _state = WxState::DONE;
      _synced = true;
      Logger::printf("Weather: %.1fC code=%d", _tempC, _wmoCode);
    } else {
      _state = WxState::ERROR;
      Logger::printf("Weather: fetch failed http=%d", code);
    }
    _http.end();
    _lastFetchMs = millis();
  }

  void render(U8G2* u8g2) {
    u8g2->clearBuffer();

    if (!_synced) {
      u8g2->setFont(u8g2_font_6x12_tf);
      const char* msg = (_state == WxState::ERROR) ? "Weather error" : "Fetching...";
      int mw = u8g2->getStrWidth(msg);
      DisplayManager::drawStr(u8g2, (Config::DISPLAY_WIDTH - mw) / 2, 30, msg);
    } else {
      // Large temperature centred
      char tempStr[10];
      snprintf(tempStr, sizeof(tempStr), "%.1fC", _tempC);
      u8g2->setFont(u8g2_font_logisoso24_tf);
      int tw = u8g2->getStrWidth(tempStr);
      DisplayManager::drawStr(u8g2, (Config::DISPLAY_WIDTH - tw) / 2, 40, tempStr);

      // Weather description below temperature
      u8g2->setFont(u8g2_font_6x12_tf);
      const char* desc = wmoDescription(_wmoCode);
      int dw = u8g2->getStrWidth(desc);
      DisplayManager::drawStr(u8g2, (Config::DISPLAY_WIDTH - dw) / 2, 56, desc);

      // Last-updated top-right
      u8g2->setFont(u8g2_font_5x7_tf);
      int luw = u8g2->getStrWidth(_lastUpdate);
      DisplayManager::drawStr(u8g2, Config::DISPLAY_WIDTH - luw - 2, 7, _lastUpdate);
    }

    DisplayManager::drawFrame(u8g2, 0, 0, Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);
    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  bool isSynced() const { return _synced; }
  uint32_t getLastFetchMs() const { return _lastFetchMs; }
  float getTempC() const { return _tempC; }
  int getWmoCode() const { return _wmoCode; }
  void setSynced(bool s) { _synced = s; }
  void setLastFetchMs(uint32_t t) { _lastFetchMs = t; }
  void testParseResponse(const String& body) { parseResponse(body); }
#endif

 public:
  const char* getName() const override { return "weather"; }

  void begin() override {
    _state = WxState::IDLE;
    if (!_synced) {
      _lastFetchMs = 0;
    }
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    uint32_t now = millis();
    if (_lastFetchMs == 0 || (now - _lastFetchMs >= Config::WEATHER_FETCH_INTERVAL_MS)) {
      startFetch();
    }
    render(u8g2);
  }
};
