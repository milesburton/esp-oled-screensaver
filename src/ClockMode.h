#pragma once

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <time.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class ClockMode : public DisplayMode {
 public:
  static const char* DAY_NAMES[7];
  static const char* MONTH_NAMES[12];

 private:
  uint32_t _lastNtpSyncMs = 0;
  bool _ntpSynced = false;
  uint32_t _colonToggleMs = 0;
  bool _colonVisible = true;

  void render(U8G2* u8g2, struct tm* ti) {
    u8g2->clearBuffer();

    // Large HH:MM centred — use fixed colon width to avoid jitter
    u8g2->setFont(u8g2_font_logisoso28_tf);
    char hourBuf[3], minBuf[3];
    snprintf(hourBuf, sizeof(hourBuf), "%02d", ti->tm_hour);
    snprintf(minBuf, sizeof(minBuf), "%02d", ti->tm_min);
    int colonW = u8g2->getStrWidth(":");
    int numW = u8g2->getStrWidth("00");
    int totalW = numW + colonW + numW;
    int startX = (Config::DISPLAY_WIDTH - totalW) / 2;
    DisplayManager::drawStr(u8g2, startX, 38, hourBuf);
    if (_colonVisible)
      DisplayManager::drawStr(u8g2, startX + numW, 38, ":");
    DisplayManager::drawStr(u8g2, startX + numW + colonW, 38, minBuf);

    // Date line centred: "Mon 02 Mar"
    char dateBuf[12];
    snprintf(dateBuf, sizeof(dateBuf), "%s %02d %s", DAY_NAMES[ti->tm_wday], ti->tm_mday,
             MONTH_NAMES[ti->tm_mon]);

    u8g2->setFont(u8g2_font_6x12_tf);
    int dw = u8g2->getStrWidth(dateBuf);
    DisplayManager::drawStr(u8g2, (Config::DISPLAY_WIDTH - dw) / 2, 54, dateBuf);

    // NTP indicator top-right
    if (_ntpSynced) {
      u8g2->setFont(u8g2_font_5x7_tf);
      DisplayManager::drawStr(u8g2, Config::DISPLAY_WIDTH - 16, 7, "NTP");
    }

    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  bool isNtpSynced() const { return _ntpSynced; }
  bool isColonVisible() const { return _colonVisible; }
  uint32_t getLastNtpSyncMs() const { return _lastNtpSyncMs; }
  // Expose colon toggle step for direct testing
  void testUpdate(uint32_t nowMs) {
    if (nowMs - _colonToggleMs >= 500) {
      _colonVisible = !_colonVisible;
      _colonToggleMs = nowMs;
    }
    if (nowMs - _lastNtpSyncMs >= Config::NTP_RESYNC_INTERVAL_MS) {
      _lastNtpSyncMs = nowMs;
    }
  }
  void setLastNtpSyncMs(uint32_t t) { _lastNtpSyncMs = t; }
  void setColonToggleMs(uint32_t t) { _colonToggleMs = t; }
#endif

 public:
  const char* getName() const override { return "clock"; }

  void begin() override {
    configTime(Config::NTP_UTC_OFFSET_HOURS * 3600, 0, "pool.ntp.org", "time.nist.gov");
    _lastNtpSyncMs = millis();
    _ntpSynced = false;
    _colonVisible = true;
    _colonToggleMs = millis();
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    uint32_t now = millis();

    // Re-sync NTP every hour
    if (now - _lastNtpSyncMs >= Config::NTP_RESYNC_INTERVAL_MS) {
      configTime(Config::NTP_UTC_OFFSET_HOURS * 3600, 0, "pool.ntp.org", "time.nist.gov");
      _lastNtpSyncMs = now;
    }

    // Colon blink every 500ms
    if (now - _colonToggleMs >= 500) {
      _colonVisible = !_colonVisible;
      _colonToggleMs = now;
    }

    time_t t = time(nullptr);
    _ntpSynced = (t > 100000UL);

    struct tm ti;
    localtime_r(&t, &ti);
    render(u8g2, &ti);
  }
};

const char* ClockMode::DAY_NAMES[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* ClockMode::MONTH_NAMES[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
