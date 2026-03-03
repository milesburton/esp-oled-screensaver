#pragma once

#include <ESP8266WiFi.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class StatusMode : public DisplayMode {
 public:
  const char* getName() const override { return "status"; }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_6x12_tf);

    char buf[32];

    DisplayManager::drawStr(u8g2, 0, 12, Config::HOSTNAME);

    snprintf(buf, sizeof(buf), "FW: %s", Config::FW_VERSION);
    DisplayManager::drawStr(u8g2, 0, 28, buf);

    snprintf(buf, sizeof(buf), "DRV: %s X:%d", Config::runtime.getDriverName(),
             Config::runtime.xOffset);
    DisplayManager::drawStr(u8g2, 0, 42, buf);

    if (WiFi.status() == WL_CONNECTED) {
      snprintf(buf, sizeof(buf), "IP: %s", WiFi.localIP().toString().c_str());
    } else {
      snprintf(buf, sizeof(buf), "IP: (connecting)");
    }
    DisplayManager::drawStr(u8g2, 0, 56, buf);

    DisplayManager::drawFrame(u8g2, 0, 0, Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);

    u8g2->sendBuffer();
  }
};
