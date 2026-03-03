#pragma once

#include <math.h>
#include <pgmspace.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

// Sonic the Hedgehog running screensaver.
// Sonic runs left-to-right across the screen and the ground scrolls to give a
// sense of speed. Artwork is 24×24 pixel bitmaps drawn with U8G2 drawXBMP.
// Bytes are stored in flash (PROGMEM) to save RAM.

// clang-format off

// Each sprite: 24 cols × 24 rows = 72 bytes (3 bytes/row, XBM LSB-first order)

// Frame 0 — stride centre (both feet near ground)
static const uint8_t SONIC_F0[] PROGMEM = {
  0xF0, 0x01, 0x00,  // ....####........
  0xF8, 0x03, 0x00,  // ...######.......
  0xFC, 0x07, 0x00,  // ..########......
  0xFE, 0x07, 0x00,  // .#########......
  0xFE, 0x07, 0x00,  // .#########......
  0xFF, 0x07, 0x00,  // ##########......
  0xFF, 0x07, 0x00,  // ##########......
  0xFE, 0x07, 0x00,  // .#########......
  0xFC, 0x07, 0x00,  // ..########......
  0xF8, 0x07, 0x00,  // ...#######......
  0xF0, 0x07, 0x00,  // ....######......
  0xE0, 0x07, 0x00,  // .....#####......
  0xC0, 0x07, 0x00,  // ......####......
  0xC0, 0x07, 0x00,  // ......####......
  0x80, 0x07, 0x00,  // .......###......
  0x8E, 0x07, 0x00,  // .###...###......  legs
  0x8E, 0x07, 0x00,
  0x8E, 0x07, 0x00,
  0x8E, 0x07, 0x00,
  0x8E, 0x07, 0x00,
  0xFF, 0x07, 0x00,  // ##########......  shoes
  0xFF, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0x00, 0x00, 0x00,
};

// Frame 1 — right leg forward
static const uint8_t SONIC_F1[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xF8, 0x03, 0x00,
  0xFC, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFC, 0x07, 0x00,
  0xF8, 0x07, 0x00,
  0xF0, 0x07, 0x00,
  0xE0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0x80, 0x07, 0x00,
  0x06, 0x0E, 0x00,  // right fwd, left back
  0x06, 0x0C, 0x00,
  0x06, 0x08, 0x00,
  0x86, 0x01, 0x00,
  0xC6, 0x00, 0x00,
  0xFC, 0x01, 0x00,  // shoes
  0xFC, 0x01, 0x00,
  0xF8, 0x01, 0x00,
  0x00, 0x00, 0x00,
};

// Frame 2 — airborne (legs tucked)
static const uint8_t SONIC_F2[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xF8, 0x03, 0x00,
  0xFC, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFC, 0x07, 0x00,
  0xF8, 0x07, 0x00,
  0xF0, 0x07, 0x00,
  0xE0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0xC0, 0x0F, 0x00,  // body + legs tucked
  0xE0, 0x1F, 0x00,
  0xF0, 0x3F, 0x00,
  0xF8, 0x1F, 0x00,
  0xF8, 0x0F, 0x00,
  0xF0, 0x07, 0x00,
  0xE0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0x80, 0x03, 0x00,
  0x00, 0x00, 0x00,
};

// Frame 3 — left leg forward
static const uint8_t SONIC_F3[] PROGMEM = {
  0xF0, 0x01, 0x00,
  0xF8, 0x03, 0x00,
  0xFC, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFF, 0x07, 0x00,
  0xFE, 0x07, 0x00,
  0xFC, 0x07, 0x00,
  0xF8, 0x07, 0x00,
  0xF0, 0x07, 0x00,
  0xE0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0xC0, 0x07, 0x00,
  0x80, 0x07, 0x00,
  0x60, 0x07, 0x00,  // left fwd, right back
  0x30, 0x06, 0x00,
  0x18, 0x06, 0x00,
  0x0C, 0x46, 0x00,
  0x06, 0x66, 0x00,
  0x0F, 0x7E, 0x00,  // shoes
  0x0F, 0x7E, 0x00,
  0x0E, 0x3E, 0x00,
  0x00, 0x00, 0x00,
};

// clang-format on

static const uint8_t* const SONIC_FRAMES[4] = {SONIC_F0, SONIC_F1, SONIC_F2, SONIC_F3};

class SonicMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr int GROUND_Y = 52;
  static constexpr int LOOP_R = 18;
  static constexpr int LOOP_CX = 84;
  static constexpr int LOOP_CY = GROUND_Y - LOOP_R;
  static constexpr int SPR_W = 24;
  static constexpr int SPR_H = 24;
  static constexpr int FEET_X_OFFSET = 12;
  static constexpr int FEET_OFFSET = SPR_H - 4;
  static constexpr int RUN_SPEED_PPS = 90;
  static constexpr int FRAME_COUNT = 4;
  static constexpr uint32_t FRAME_MS = 80;
  static constexpr int START_FOOT_X = -FEET_X_OFFSET;
  static constexpr float PI_F = 3.14159265f;

  int _spriteX;
  int _spriteY;
  uint8_t _frameIdx;
  uint32_t _frameAccMs;

  int32_t _pathPosQ8;

  static int approachLen() { return (LOOP_CX - LOOP_R) - START_FOOT_X; }
  static int loopLen() { return static_cast<int>(LOOP_R * PI_F * 3.0f); }
  static int exitLen() { return (W + FEET_X_OFFSET) - (LOOP_CX + LOOP_R); }
  static int totalLen() { return approachLen() + loopLen() + exitLen(); }

  void computeFootPosition(int s, int& footX, int& footY) {
    if (s < approachLen()) {
      footX = START_FOOT_X + s;
      footY = GROUND_Y;
      return;
    }

    int ls = s - approachLen();
    if (ls < loopLen()) {
      float t = static_cast<float>(ls) / static_cast<float>(loopLen());
      float angle = PI_F - (t * (3.0f * PI_F));
      footX = LOOP_CX + static_cast<int>(LOOP_R * cosf(angle));
      footY = LOOP_CY + static_cast<int>(LOOP_R * sinf(angle));
      return;
    }

    int es = ls - loopLen();
    footX = (LOOP_CX + LOOP_R) + es;
    footY = GROUND_Y;
  }

  void drawScene(U8G2* u8g2) {
    DisplayManager::drawHLine(u8g2, 0, GROUND_Y, W);
    DisplayManager::drawCircle(u8g2, LOOP_CX, LOOP_CY, LOOP_R);
    DisplayManager::drawCircle(u8g2, LOOP_CX, LOOP_CY, LOOP_R - 2);

    for (int x = 0; x < W; x += 8) {
      DisplayManager::drawVLine(u8g2, x, GROUND_Y + 2, H - (GROUND_Y + 2));
    }
  }

 public:
  SonicMode() : _spriteX(0), _spriteY(0), _frameIdx(0), _frameAccMs(0), _pathPosQ8(0) {}

  const char* getName() const override { return "sonic"; }

  void begin() override {
    _spriteX = -SPR_W;
    _spriteY = GROUND_Y - FEET_OFFSET;
    _frameIdx = 0;
    _frameAccMs = 0;
    _pathPosQ8 = 0;
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0)
      return;

    _pathPosQ8 += static_cast<int32_t>(RUN_SPEED_PPS) * static_cast<int32_t>(deltaMs) * 256 / 1000;
    int lenQ8 = totalLen() * 256;
    if (_pathPosQ8 >= lenQ8) {
      _pathPosQ8 -= lenQ8;
    }

    int footX = 0;
    int footY = 0;
    computeFootPosition(_pathPosQ8 >> 8, footX, footY);
    _spriteX = footX - FEET_X_OFFSET;
    _spriteY = footY - FEET_OFFSET;

    _frameAccMs += deltaMs;
    while (_frameAccMs >= FRAME_MS) {
      _frameAccMs -= FRAME_MS;
      _frameIdx = (_frameIdx + 1) % FRAME_COUNT;
    }

    u8g2->clearBuffer();
    drawScene(u8g2);

    if (_frameIdx < FRAME_COUNT && SONIC_FRAMES[_frameIdx] != nullptr && _spriteX + SPR_W > 0 &&
        _spriteX < W && _spriteY + SPR_H > 0 && _spriteY < H) {
      u8g2->drawXBMP(_spriteX, _spriteY, SPR_W, SPR_H, SONIC_FRAMES[_frameIdx]);
    }

    yield();
    u8g2->sendBuffer();
  }
};
