#pragma once

#include <math.h>

#include "Config.h"
#include "DisplayMode.h"

class TunnelMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr int CX = W / 2;
  static constexpr int CY = H / 2;
  static constexpr int MAX_RINGS = 10;
  static constexpr float RING_SPEED = 18.0f;
  static constexpr float MAX_RADIUS = 80.0f;
  static constexpr float RING_SPACING = MAX_RADIUS / MAX_RINGS;

  float _offset;

 public:
  const char* getName() const override { return "tunnel"; }

  void begin() override { _offset = 0.0f; }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0)
      return;

    _offset += RING_SPEED * deltaMs / 1000.0f;
    if (_offset >= RING_SPACING)
      _offset -= RING_SPACING;

    u8g2->clearBuffer();

    for (int i = 0; i < MAX_RINGS; i++) {
      float r = _offset + i * RING_SPACING;
      if (r < 1.0f)
        continue;

      int rx = static_cast<int>(r * CX / MAX_RADIUS);
      int ry = static_cast<int>(r * CY / MAX_RADIUS);

      if (rx < 1 || ry < 1)
        continue;
      if (rx > CX + 4 && ry > CY + 4)
        continue;

      u8g2->drawEllipse(CX + Config::runtime.xOffset, CY, rx, ry,
                        U8G2_DRAW_ALL);  // NOLINT(whitespace/line_length)
    }

    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  float getOffset() const { return _offset; }
  static float getRingSpacing() { return RING_SPACING; }
  static int getMaxRings() { return MAX_RINGS; }
#endif
};
