#pragma once

#include <U8g2lib.h>
#include <Wire.h>

#include "Config.h"
#include "DisplayMode.h"
#include "Logger.h"

class DisplayManager {
 private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_ssd;
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2_sh;
  U8G2* u8g2;

  DisplayMode* currentMode;
  uint32_t lastFrameMs;
  uint32_t frameIntervalMs;
  uint8_t currentRotation;  // Track current rotation for change detection

 public:
  DisplayManager()
      : u8g2_ssd(U8G2_R0, U8X8_PIN_NONE),
        u8g2_sh(U8G2_R0, U8X8_PIN_NONE),
        u8g2(nullptr),
        currentMode(nullptr),
        lastFrameMs(0),
        frameIntervalMs(40),  // Default: ~25 FPS
        currentRotation(0) {}

  void begin() {
    if (!Config::runtime.oledEnabled)
      return;

    Wire.begin(Config::OLED_SDA, Config::OLED_SCL);
    Wire.setClock(400'000);
    delay(5);

    selectDriver();

    Logger::printf("Display: initialized drv=%s addr=0x%02X xoff=%d",
                   Config::runtime.getDriverName(), Config::OLED_ADDR, Config::runtime.xOffset);
  }

  void selectDriver() {
    if (!Config::runtime.oledEnabled)
      return;

    u8g2 = (Config::runtime.driver == Config::OledDriver::SSD1306) ? (U8G2*)&u8g2_ssd
                                                                   : (U8G2*)&u8g2_sh;

    u8g2->setI2CAddress(Config::OLED_ADDR << 1);
    u8g2->begin();
    applyRotation();
  }

  void applyRotation() {
    if (!u8g2)
      return;

    uint8_t newRotation = Config::runtime.getU8G2Rotation();
    if (currentRotation != newRotation) {
      // Map DisplayRotation enum to U8G2 rotation callbacks
      const u8g2_cb_t* rotation_cb[] = {U8G2_R0, U8G2_R1, U8G2_R2, U8G2_R3};
      u8g2->setDisplayRotation(rotation_cb[newRotation]);
      currentRotation = newRotation;
      Logger::printf("Display: rotation set to %s", Config::runtime.getRotationName());
    }
  }

  void setMode(DisplayMode* mode, uint32_t frameIntervalMs = 40) {
    if (currentMode) {
      currentMode->end();
    }

    currentMode = mode;
    this->frameIntervalMs = frameIntervalMs;
    lastFrameMs = 0;  // Force immediate render

    if (currentMode) {
      currentMode->begin();
      Logger::printf("Display: mode changed to '%s'", currentMode->getName());
    }
  }

  void update() {
    if (!Config::runtime.oledEnabled || !u8g2 || !currentMode)
      return;

    // Check for rotation changes
    applyRotation();

    uint32_t now = millis();
    if (now - lastFrameMs >= frameIntervalMs) {
      uint32_t deltaMs = (lastFrameMs == 0) ? 16 : (now - lastFrameMs);
      lastFrameMs = now;

      currentMode->update(u8g2, deltaMs);
    }
  }

  void clear() {
    if (u8g2) {
      u8g2->clearBuffer();
      u8g2->sendBuffer();
    }
  }

  U8G2* getDisplay() { return u8g2; }
  DisplayMode* getCurrentMode() { return currentMode; }

  void setRotation(Config::DisplayRotation rot) {
    Config::runtime.rotation = rot;
    applyRotation();
  }

  Config::DisplayRotation getRotation() const { return Config::runtime.rotation; }

  // Drawing helpers with X offset
  static void drawPixel(U8G2* u8g2, int x, int y) {
    u8g2->drawPixel(x + Config::runtime.xOffset, y);
  }

  static void drawHLine(U8G2* u8g2, int x, int y, int w) {
    u8g2->drawHLine(x + Config::runtime.xOffset, y, w);
  }

  static void drawVLine(U8G2* u8g2, int x, int y, int h) {
    u8g2->drawVLine(x + Config::runtime.xOffset, y, h);
  }

  static void drawFrame(U8G2* u8g2, int x, int y, int w, int h) {
    u8g2->drawFrame(x + Config::runtime.xOffset, y, w, h);
  }

  static void drawCircle(U8G2* u8g2, int x, int y, int r) {
    u8g2->drawCircle(x + Config::runtime.xOffset, y, r, U8G2_DRAW_ALL);
  }

  static void drawStr(U8G2* u8g2, int x, int y, const char* s) {
    u8g2->drawStr(x + Config::runtime.xOffset, y, s);
  }
};
