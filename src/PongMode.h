#pragma once

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class PongMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr float BALL_SPEED = 60.0f;
  static constexpr float PADDLE_SPEED = 50.0f;
  static constexpr int PADDLE_W = 2;
  static constexpr int PADDLE_H = 14;
  static constexpr int BALL_R = 2;
  static constexpr int PADDLE_MARGIN = 3;

  float _ballX, _ballY;
  float _ballDX, _ballDY;
  float _leftY, _rightY;
  int _scoreLeft, _scoreRight;

  void resetBall() {
    _ballX = W / 2.0f;
    _ballY = H / 2.0f;
    _ballDX = (_scoreLeft > _scoreRight) ? BALL_SPEED : -BALL_SPEED;
    _ballDY = BALL_SPEED * 0.6f;
  }

  void clampPaddle(float* y) {
    if (*y < 0.0f)
      *y = 0.0f;
    if (*y + PADDLE_H > H)
      *y = H - PADDLE_H;
  }

  void movePaddle(float* paddleY, float targetY, float dt) {
    float mid = *paddleY + PADDLE_H / 2.0f;
    float diff = targetY - mid;
    float step = PADDLE_SPEED * dt;
    if (diff > step)
      *paddleY += step;
    else if (diff < -step)
      *paddleY -= step;
    clampPaddle(paddleY);
  }

 public:
  const char* getName() const override { return "pong"; }

  void begin() override {
    _leftY = (H - PADDLE_H) / 2.0f;
    _rightY = (H - PADDLE_H) / 2.0f;
    _scoreLeft = 0;
    _scoreRight = 0;
    resetBall();
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0)
      return;

    float dt = deltaMs / 1000.0f;

    movePaddle(&_leftY, _ballY, dt);
    movePaddle(&_rightY, _ballY, dt);

    _ballX += _ballDX * dt;
    _ballY += _ballDY * dt;

    if (_ballY - BALL_R < 0) {
      _ballY = BALL_R;
      _ballDY = fabsf(_ballDY);
    }
    if (_ballY + BALL_R >= H) {
      _ballY = H - BALL_R - 1;
      _ballDY = -fabsf(_ballDY);
    }

    int lx = PADDLE_MARGIN + PADDLE_W;
    if (_ballDX < 0 && _ballX - BALL_R <= lx &&
        _ballY >= _leftY &&  // NOLINT(whitespace/line_length)
        _ballY <= _leftY + PADDLE_H) {
      _ballX = lx + BALL_R;
      _ballDX = fabsf(_ballDX);
    }

    int rx = W - PADDLE_MARGIN - PADDLE_W;
    if (_ballDX > 0 && _ballX + BALL_R >= rx &&
        _ballY >= _rightY &&  // NOLINT(whitespace/line_length)
        _ballY <= _rightY + PADDLE_H) {
      _ballX = rx - BALL_R;
      _ballDX = -fabsf(_ballDX);
    }

    if (_ballX < 0) {
      _scoreRight++;
      resetBall();
    } else if (_ballX > W) {
      _scoreLeft++;
      resetBall();
    }

    u8g2->clearBuffer();

    u8g2->drawDisc(
        static_cast<int>(_ballX) + Config::runtime.xOffset,  // NOLINT(whitespace/line_length)
        static_cast<int>(_ballY), BALL_R);

    DisplayManager::drawFrame(u8g2, PADDLE_MARGIN,
                              static_cast<int>(_leftY),  // NOLINT(whitespace/line_length)
                              PADDLE_W, PADDLE_H);
    DisplayManager::drawFrame(u8g2, W - PADDLE_MARGIN - PADDLE_W,  // NOLINT(whitespace/line_length)
                              static_cast<int>(_rightY), PADDLE_W, PADDLE_H);

    for (int y = 4; y < H; y += 6) {
      DisplayManager::drawVLine(u8g2, W / 2, y, 3);
    }

    u8g2->setFont(u8g2_font_5x7_tr);
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", _scoreLeft);
    DisplayManager::drawStr(u8g2, W / 2 - 18, 7, buf);
    snprintf(buf, sizeof(buf), "%d", _scoreRight);
    DisplayManager::drawStr(u8g2, W / 2 + 10, 7, buf);

    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  float getBallX() const { return _ballX; }
  float getBallY() const { return _ballY; }
  float getBallDX() const { return _ballDX; }
  float getBallDY() const { return _ballDY; }
  float getLeftY() const { return _leftY; }
  float getRightY() const { return _rightY; }
  int getScoreLeft() const { return _scoreLeft; }
  int getScoreRight() const { return _scoreRight; }
  static float getBallSpeed() { return BALL_SPEED; }
  static int getPaddleH() { return PADDLE_H; }
  static int getBallR() { return BALL_R; }
  static int getScreenW() { return W; }
  static int getScreenH() { return H; }
#endif
};
