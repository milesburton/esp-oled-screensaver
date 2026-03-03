#pragma once

#include <math.h>
#include <pgmspace.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

// clang-format off

static const uint8_t SONIC_F0[] PROGMEM = {
  0x00, 0x88, 0x00,
  0x00, 0xCC, 0x00,
  0x00, 0xFE, 0x00,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x6F,
  0x00, 0xFF, 0x63,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x3F,
  0x00, 0xFF, 0x9F,
  0x00, 0xFE, 0x0F,
  0xC0, 0xFF, 0x07,
  0xF0, 0xFF, 0x07,
  0xF0, 0xFF, 0x03,
  0xC0, 0xFF, 0x01,
  0x80, 0xFF, 0x00,
  0x80, 0x6D, 0x00,
  0x00, 0x77, 0x00,
  0x00, 0x77, 0x00,
  0x00, 0x63, 0x00,
  0x80, 0x7F, 0x07,
  0x80, 0x7F, 0x07,
  0x00, 0x3E, 0x06,
};

static const uint8_t SONIC_F1[] PROGMEM = {
  0x00, 0x88, 0x00,
  0x00, 0xCC, 0x00,
  0x00, 0xFE, 0x00,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x6F,
  0x00, 0xFF, 0x63,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x3F,
  0x00, 0xFF, 0x9F,
  0x00, 0xFE, 0x0F,
  0xC0, 0xFF, 0x07,
  0xF0, 0xFF, 0x07,
  0xF0, 0xFF, 0x03,
  0xC0, 0xFF, 0x01,
  0x80, 0xFF, 0x00,
  0x40, 0x01, 0x7C,
  0x20, 0x00, 0x78,
  0x10, 0x00, 0x70,
  0x08, 0x00, 0x60,
  0x1C, 0x00, 0x7C,
  0x3C, 0x00, 0x7C,
  0x38, 0x00, 0x38,
};

static const uint8_t SONIC_F2[] PROGMEM = {
  0x00, 0x88, 0x00,
  0x00, 0xCC, 0x00,
  0x00, 0xFE, 0x00,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x6F,
  0x00, 0xFF, 0x63,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x3F,
  0x00, 0xFF, 0x9F,
  0x00, 0xFE, 0x0F,
  0xC0, 0xFF, 0x07,
  0xF0, 0xFF, 0x07,
  0xF0, 0xFF, 0x03,
  0xC0, 0xFF, 0x01,
  0x80, 0xFF, 0x00,
  0x04, 0x00, 0x7C,
  0x02, 0x00, 0x70,
  0x02, 0x00, 0x60,
  0x02, 0x00, 0x40,
  0x0F, 0x00, 0x78,
  0x0F, 0x00, 0x78,
  0x0E, 0x00, 0x30,
};

static const uint8_t SONIC_F3[] PROGMEM = {
  0x00, 0x88, 0x00,
  0x00, 0xCC, 0x00,
  0x00, 0xFE, 0x00,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x6F,
  0x00, 0xFF, 0x63,
  0x00, 0xFF, 0x7F,
  0x00, 0xFF, 0x3F,
  0x00, 0xFF, 0x9F,
  0x00, 0xFE, 0x0F,
  0xC0, 0xFF, 0x07,
  0xF0, 0xFF, 0x07,
  0xF0, 0xFF, 0x03,
  0xC0, 0xFF, 0x01,
  0x80, 0xFF, 0x00,
  0x40, 0x7C, 0x01,
  0x20, 0x78, 0x00,
  0x10, 0x70, 0x00,
  0x08, 0x60, 0x00,
  0x1C, 0x7C, 0x00,
  0x3C, 0x7C, 0x00,
  0x38, 0x38, 0x00,
};

static const uint8_t SONIC_BALL[] PROGMEM = {
  0x00, 0x00, 0x00,
  0x00, 0x3E, 0x00,
  0x80, 0xFF, 0x01,
  0xC0, 0xFF, 0x03,
  0xE0, 0xFF, 0x07,
  0xF0, 0xFF, 0x0F,
  0xF0, 0xFF, 0x0F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF8, 0xFF, 0x1F,
  0xF0, 0xFF, 0x0F,
  0xF0, 0xFF, 0x0F,
  0xE0, 0xFF, 0x07,
  0xC0, 0xFF, 0x03,
  0x80, 0xFF, 0x01,
  0x00, 0xFF, 0x00,
  0x00, 0x7E, 0x00,
  0x00, 0x1C, 0x00,
  0x00, 0x00, 0x00,
};

// clang-format on

static const uint8_t* const SONIC_FRAMES[4] PROGMEM = {SONIC_F0, SONIC_F1, SONIC_F2, SONIC_F3};

class SonicMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr int GROUND_Y = 54;
  static constexpr int LOOP_R = 17;
  static constexpr int LOOP_CX = 80;
  static constexpr int LOOP_CY = GROUND_Y - LOOP_R;
  static constexpr int SPR_W = 24;
  static constexpr int SPR_H = 24;
  static constexpr int FOOT_X_OFF = SPR_W / 2;
  static constexpr int FOOT_Y_OFF = SPR_H - 2;
  static constexpr int RUN_SPEED_PPS = 95;
  static constexpr int FRAME_COUNT = 4;
  static constexpr uint32_t FRAME_MS = 14000 / RUN_SPEED_PPS;
  static constexpr int GROUND_DASH_LEN = 5;
  static constexpr int GROUND_DASH_GAP = 11;
  static constexpr float PI_F = 3.14159265f;

  int _spriteX;
  int _spriteY;
  uint8_t _frameIdx;
  uint32_t _frameAccMs;
  int32_t _pathPosQ8;
  int _groundScrollX;
  bool _onLoop;

  static int approachLen() { return (LOOP_CX - LOOP_R) + FOOT_X_OFF; }
  static int loopLen() { return static_cast<int>(2.0f * PI_F * LOOP_R); }
  static int exitLen() { return W + FOOT_X_OFF - (LOOP_CX + LOOP_R); }
  static int totalLen() { return approachLen() + loopLen() + exitLen(); }

  void computeFootPosition(int s, int& footX, int& footY, bool& onLoop) {
    onLoop = false;
    if (s < approachLen()) {
      footX = -FOOT_X_OFF + s;
      footY = GROUND_Y;
      return;
    }
    int ls = s - approachLen();
    if (ls < loopLen()) {
      onLoop = true;
      float t = static_cast<float>(ls) / static_cast<float>(loopLen());
      float angle = PI_F - (t * 2.0f * PI_F);
      footX = LOOP_CX + static_cast<int>(LOOP_R * cosf(angle));
      footY = LOOP_CY + static_cast<int>(LOOP_R * sinf(angle));
      return;
    }
    int es = ls - loopLen();
    footX = (LOOP_CX + LOOP_R) + es;
    footY = GROUND_Y;
  }

  void drawGround(U8G2* u8g2) {
    DisplayManager::drawHLine(u8g2, 0, GROUND_Y + 1, W);
    int period = GROUND_DASH_LEN + GROUND_DASH_GAP;
    int offset = ((_groundScrollX % period) + period) % period;
    for (int x = -period + offset; x < W; x += period) {
      int x0 = x < 0 ? 0 : x;
      int x1 = x + GROUND_DASH_LEN;
      if (x1 > W) x1 = W;
      if (x1 > x0) DisplayManager::drawHLine(u8g2, x0, GROUND_Y + 3, x1 - x0);
    }
  }

  void drawLoop(U8G2* u8g2) {
    DisplayManager::drawCircle(u8g2, LOOP_CX, LOOP_CY, LOOP_R);
    DisplayManager::drawCircle(u8g2, LOOP_CX, LOOP_CY, LOOP_R - 2);
  }

  void drawSpeedLines(U8G2* u8g2, int sonicX, int sonicMidY) {
    if (sonicX < 6) return;
    int x0 = sonicX - 2;
    DisplayManager::drawHLine(u8g2, x0 - 10, sonicMidY - 3, 10);
    DisplayManager::drawHLine(u8g2, x0 - 7,  sonicMidY,     7);
    DisplayManager::drawHLine(u8g2, x0 - 5,  sonicMidY + 3, 5);
  }

#ifdef NATIVE_TEST

 public:
  bool isOnLoop() const { return _onLoop; }
  int getGroundScrollX() const { return _groundScrollX; }
  uint8_t getFrameIdx() const { return _frameIdx; }
  int getSpriteX() const { return _spriteX; }
  int getSpriteY() const { return _spriteY; }
  int32_t getPathPosQ8() const { return _pathPosQ8; }
  static int getTotalLen() { return totalLen(); }
  static int getLoopStartLen() { return approachLen(); }
  static int getLoopLen() { return loopLen(); }
#endif

 public:
  const char* getName() const override { return "sonic"; }

  void begin() override {
    _spriteX = -SPR_W;
    _spriteY = GROUND_Y - FOOT_Y_OFF;
    _frameIdx = 0;
    _frameAccMs = 0;
    _pathPosQ8 = 0;
    _groundScrollX = 0;
    _onLoop = false;
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0) return;

    int32_t stepQ8 =
        static_cast<int32_t>(RUN_SPEED_PPS) * static_cast<int32_t>(deltaMs) * 256 / 1000;
    _pathPosQ8 += stepQ8;
    int lenQ8 = totalLen() * 256;
    if (_pathPosQ8 >= lenQ8) _pathPosQ8 -= lenQ8;

    _groundScrollX -= static_cast<int>(stepQ8 >> 8);

    int footX = 0, footY = 0;
    computeFootPosition(_pathPosQ8 >> 8, footX, footY, _onLoop);
    _spriteX = footX - FOOT_X_OFF;
    _spriteY = footY - FOOT_Y_OFF;

    if (!_onLoop) {
      _frameAccMs += deltaMs;
      while (_frameAccMs >= FRAME_MS) {
        _frameAccMs -= FRAME_MS;
        _frameIdx = (_frameIdx + 1) % FRAME_COUNT;
      }
    }

    u8g2->clearBuffer();
    drawGround(u8g2);
    drawLoop(u8g2);

    int sonicMidY = _spriteY + SPR_H / 2;
    if (!_onLoop) drawSpeedLines(u8g2, _spriteX, sonicMidY);

    bool visible = _spriteX + SPR_W > 0 && _spriteX < W && _spriteY + SPR_H > 0 && _spriteY < H;
    if (visible) {
      const uint8_t* frame = _onLoop ? SONIC_BALL : SONIC_FRAMES[_frameIdx];
      u8g2->drawXBMP(_spriteX, _spriteY, SPR_W, SPR_H, frame);
    }

    yield();
    u8g2->sendBuffer();
  }
};
