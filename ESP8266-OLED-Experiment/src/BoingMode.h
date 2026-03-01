#pragma once

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class BoingMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;

  static constexpr int R0 = Config::BOING_BALL_RADIUS;
  static constexpr int FLOOR_LINE_Y = H - 1;
  static constexpr int FLOOR_Y = H - 2;
  static constexpr int LEFT_X = R0 + 2;
  static constexpr int RIGHT_X = W - 1 - (R0 + 2);

  static constexpr float X_SPEED = Config::BOING_X_SPEED;
  static constexpr float BOUNCE_FREQ = Config::BOING_BOUNCE_FREQ;
  static constexpr float BOUNCE_HEIGHT = Config::BOING_BOUNCE_HEIGHT;

  float xPos;
  int xDir;
  float rotPhase;
  uint32_t startTime;

  static inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

  void drawShadow(U8G2* u8g2, int cx, int cy, int rx, int ry) {
    for (int y = -ry; y <= ry; y++) {
      float yy = (float)y / (float)ry;
      float inside = 1.0f - yy * yy;
      if (inside <= 0)
        continue;
      int half = (int)(rx * sqrtf(inside));
      DisplayManager::drawHLine(u8g2, cx - half, cy + y, 2 * half + 1);
      yield();
    }
  }

  void drawBoingBall(U8G2* u8g2, int cx, int cy, int rx, int ry, float rot) {
    DisplayManager::drawCircle(u8g2, cx, cy, max(rx, ry));

    int minX = cx - rx, maxX = cx + rx;
    int minY = cy - ry, maxY = cy + ry;

    for (int py = minY; py <= maxY; py++) {
      for (int px = minX; px <= maxX; px++) {
        float nx = (px - cx) / (float)rx;
        float ny = (py - cy) / (float)ry;
        float rr = nx * nx + ny * ny;
        if (rr > 1.0f)
          continue;

        // lon/lat-ish mapping
        float lon = atan2f(nx, sqrtf(1.0f - rr));
        float lat = asinf(clampf(ny, -1.0f, 1.0f));

        // Checker pattern
        int u = (int)floorf((lon + rot) * 3.0f);
        int v = (int)floorf(lat * 3.0f);

        bool on = ((u + v) & 1) == 0;

        // Specular highlight
        bool highlight = (ny < -0.35f && ny > -0.45f);

        if (on || highlight) {
          DisplayManager::drawPixel(u8g2, px, py);
        }
      }
      yield();
    }
  }

 public:
  BoingMode() : xPos(30.0f), xDir(1), rotPhase(0.0f), startTime(0) {}

  const char* getName() const override { return "boing"; }

  void begin() override {
    startTime = millis();
    xPos = 30.0f;
    xDir = 1;
    rotPhase = 0.0f;
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    float dt = deltaMs / 1000.0f;
    if (dt > 0.05f)
      dt = 0.05f;

    // X ping-pong
    xPos += xDir * X_SPEED * dt;
    if (xPos <= LEFT_X) {
      xPos = LEFT_X;
      xDir = +1;
    }
    if (xPos >= RIGHT_X) {
      xPos = RIGHT_X;
      xDir = -1;
    }

    // Bounce curve
    uint32_t now = millis();
    float t = (now - startTime) / 1000.0f;
    float phase = t * (2.0f * 3.1415926f) * BOUNCE_FREQ;
    float bounce01 = 0.5f * (1.0f - cosf(phase));

    float yPos = (float)FLOOR_Y - bounce01 * BOUNCE_HEIGHT;

    // Squash/stretch
    float squash = 1.0f - (1.0f - bounce01) * 0.35f;
    float stretch = 1.0f + (1.0f - bounce01) * 0.20f;

    int rx = (int)(R0 * (1.0f / squash));
    int ry = (int)(R0 * squash * stretch);

    // Rotate pattern
    rotPhase += (xDir * 1.8f) * dt;

    // Render
    u8g2->clearBuffer();

    // Border
    DisplayManager::drawFrame(u8g2, 0, 0, W, H);

    // Floor
    DisplayManager::drawHLine(u8g2, 0, FLOOR_LINE_Y, W);

    // Shadow
    int sx = (int)xPos;
    int shadowY = FLOOR_Y;
    int srx = (int)(rx * (1.2f + (1.0f - bounce01) * 0.6f));
    int sry = (int)(2 + (1.0f - bounce01) * 2);
    drawShadow(u8g2, sx, shadowY, srx, sry);

    // Ball
    drawBoingBall(u8g2, (int)xPos, (int)yPos, rx, ry, rotPhase);

    // Label
    u8g2->setFont(u8g2_font_5x7_tf);
    DisplayManager::drawStr(u8g2, 0, 7, "BOING");

    u8g2->sendBuffer();
  }
};
