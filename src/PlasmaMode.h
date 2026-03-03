#pragma once

#include <math.h>

#include "Config.h"
#include "DisplayMode.h"

class PlasmaMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr float TWO_PI_F = 6.28318530f;

  float _t;

 public:
  const char* getName() const override { return "plasma"; }

  void begin() override { _t = 0.0f; }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0)
      return;

    _t += deltaMs * 0.002f;

    u8g2->clearBuffer();

    for (int y = 0; y < H; y += 2) {
      for (int x = 0; x < W; x += 2) {
        float fx = static_cast<float>(x) / W;
        float fy = static_cast<float>(y) / H;

        float v = sinf(fx * TWO_PI_F * 2.0f + _t) +
                  sinf(fy * TWO_PI_F * 2.0f + _t * 0.7f) +  // NOLINT(whitespace/line_length)
                  sinf((fx + fy) * TWO_PI_F * 1.5f + _t * 0.5f) +
                  sinf(sqrtf(fx * fx + fy * fy) * TWO_PI_F * 3.0f + _t);

        if (v > 0.0f) {
          u8g2->drawPixel(x, y);
        }
      }
    }

    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  float getT() const { return _t; }
#endif
};
