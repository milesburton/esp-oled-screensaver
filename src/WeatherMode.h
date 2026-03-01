#pragma once

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class WeatherMode : public DisplayMode {
 public:
  const char* getName() const override { return "weather"; }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_6x12_tf);

    DisplayManager::drawStr(u8g2, 0, 12, "Weather Forecast");

    u8g2->setCursor(0 + Config::runtime.xOffset, 28);
    u8g2->print("Not implemented");

    DisplayManager::drawFrame(u8g2, 0, 0, Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);

    u8g2->sendBuffer();
  }
};
