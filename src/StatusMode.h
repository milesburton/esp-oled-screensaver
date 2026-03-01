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

    DisplayManager::drawStr(u8g2, 0, 12, Config::HOSTNAME);

    u8g2->setCursor(0 + Config::runtime.xOffset, 28);
    u8g2->print("FW: ");
    u8g2->print(Config::FW_VERSION);

    u8g2->setCursor(0 + Config::runtime.xOffset, 42);
    u8g2->print("DRV: ");
    u8g2->print(Config::runtime.getDriverName());
    u8g2->print(" X:");
    u8g2->print(Config::runtime.xOffset);

    u8g2->setCursor(0 + Config::runtime.xOffset, 56);
    u8g2->print("IP: ");
    if (WiFi.status() == WL_CONNECTED) {
      u8g2->print(WiFi.localIP());
    } else {
      u8g2->print("(connecting)");
    }

    // Border to test alignment
    DisplayManager::drawFrame(u8g2, 0, 0, Config::DISPLAY_WIDTH,
                              Config::DISPLAY_HEIGHT);

    u8g2->sendBuffer();
  }
};
