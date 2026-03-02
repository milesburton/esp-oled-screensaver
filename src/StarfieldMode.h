#pragma once

#include "Config.h"
#include "DisplayMode.h"

class StarfieldMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr int NUM_STARS = 80;
  static constexpr int MAX_DEPTH = 64;

  struct Star {
    int16_t x;  // -MAX_DEPTH..MAX_DEPTH
    int16_t y;  // -MAX_DEPTH..MAX_DEPTH
    uint8_t z;  // 1..MAX_DEPTH (depth: 1=close, MAX_DEPTH=far)
  };

  Star stars[NUM_STARS];
  uint32_t seed;

  uint32_t nextRand() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
  }

  void initStar(Star& s) {
    s.x = (int16_t)((nextRand() % (2 * MAX_DEPTH)) - MAX_DEPTH);
    s.y = (int16_t)((nextRand() % (2 * MAX_DEPTH)) - MAX_DEPTH);
    s.z = MAX_DEPTH;
  }

 public:
  StarfieldMode() : seed(12345) {}

  const char* getName() const override { return "starfield"; }

  void begin() override {
    seed = millis() ^ 0xDEADBEEF;
    for (int i = 0; i < NUM_STARS; i++) {
      stars[i].x = (int16_t)((nextRand() % (2 * MAX_DEPTH)) - MAX_DEPTH);
      stars[i].y = (int16_t)((nextRand() % (2 * MAX_DEPTH)) - MAX_DEPTH);
      stars[i].z = (uint8_t)(1 + nextRand() % MAX_DEPTH);
    }
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    uint8_t speed = (deltaMs > 50) ? 4 : (uint8_t)((deltaMs * 4 + 25) / 50);
    if (speed < 1) speed = 1;

    u8g2->clearBuffer();

    for (int i = 0; i < NUM_STARS; i++) {
      stars[i].z -= speed;
      if (stars[i].z < 1) {
        initStar(stars[i]);
      }

      // Project from 3D to 2D
      int sx = (stars[i].x * MAX_DEPTH) / stars[i].z + W / 2;
      int sy = (stars[i].y * MAX_DEPTH) / stars[i].z + H / 2;

      if (sx < 0 || sx >= W || sy < 0 || sy >= H) {
        initStar(stars[i]);
        continue;
      }

      // Closer stars are bigger/brighter
      if (stars[i].z < MAX_DEPTH / 4) {
        u8g2->drawPixel(sx, sy);
        if (sx + 1 < W) u8g2->drawPixel(sx + 1, sy);
        if (sy + 1 < H) u8g2->drawPixel(sx, sy + 1);
      } else {
        u8g2->drawPixel(sx, sy);
      }
    }

    u8g2->sendBuffer();
  }
};
