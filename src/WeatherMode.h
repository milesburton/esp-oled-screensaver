#pragma once

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

// Placeholder for weather forecast display
// TODO: Integrate with weather API (OpenWeatherMap, Weather.gov, etc.)
class WeatherMode : public DisplayMode {
 private:
  // TODO: Add weather data structure
  // struct WeatherData {
  //   float temperature;
  //   float humidity;
  //   String condition;
  //   String forecast;
  // };
  // WeatherData currentWeather;

 public:
  const char* getName() const override { return "weather"; }

  void begin() override {
    // TODO: Fetch initial weather data
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_6x12_tf);

    // Placeholder display
    DisplayManager::drawStr(u8g2, 0, 12, "Weather Forecast");

    u8g2->setCursor(0 + Config::runtime.xOffset, 28);
    u8g2->print("Coming Soon!");

    u8g2->setCursor(0 + Config::runtime.xOffset, 44);
    u8g2->print("TODO: API integration");

    // Border
    DisplayManager::drawFrame(u8g2, 0, 0, Config::DISPLAY_WIDTH, Config::DISPLAY_HEIGHT);

    u8g2->sendBuffer();
  }

  // TODO: Add methods for:
  // - Fetching weather data from API
  // - Parsing JSON response
  // - Displaying temperature, conditions, forecast
  // - Icons for different weather conditions
  // - Auto-refresh at intervals
};
