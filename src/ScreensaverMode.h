#pragma once

#include "BoingMode.h"
#include "BreakoutMode.h"
#include "ClockMode.h"
#include "Config.h"
#include "DisplayMode.h"
#include "PacManMode.h"
#include "WeatherMode.h"

class ScreensaverMode : public DisplayMode {
 private:
  DisplayMode* modes[5];
  uint8_t modeCount;
  uint8_t currentIndex;
  uint32_t elapsedMs;

 public:
  ScreensaverMode(ClockMode* clock, BoingMode* boing, WeatherMode* weather,
                  BreakoutMode* breakout, PacManMode* pacman)
      : modeCount(0), currentIndex(0), elapsedMs(0) {
    modes[modeCount++] = clock;
    modes[modeCount++] = boing;
    modes[modeCount++] = weather;
    modes[modeCount++] = breakout;
    modes[modeCount++] = pacman;
  }

  const char* getName() const override { return "screensaver"; }

  void begin() override {
    currentIndex = 0;
    elapsedMs = 0;
    if (modes[currentIndex])
      modes[currentIndex]->begin();
  }

  void end() override {
    if (modes[currentIndex])
      modes[currentIndex]->end();
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    elapsedMs += deltaMs;
    if (elapsedMs >= Config::SCREENSAVER_MODE_INTERVAL_MS) {
      elapsedMs = 0;
      if (modes[currentIndex])
        modes[currentIndex]->end();
      currentIndex = (currentIndex + 1) % modeCount;
      if (modes[currentIndex])
        modes[currentIndex]->begin();
    }
    if (modes[currentIndex])
      modes[currentIndex]->update(u8g2, deltaMs);
  }
};
