#pragma once

#include <Arduino.h>

#include <math.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class BreakoutMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;

  static constexpr int BRICK_COLS = 8;
  static constexpr int BRICK_ROWS = 4;
  static constexpr int BRICK_W = 14;
  static constexpr int BRICK_H = 4;
  static constexpr int BRICK_GAP = 1;
  static constexpr int BRICK_TOP = 4;
  static constexpr int BRICK_LEFT = 4;

  static constexpr int PADDLE_W = 20;
  static constexpr int PADDLE_H = 3;
  static constexpr int PADDLE_Y = H - PADDLE_H - 2;
  static constexpr float PADDLE_SPD = 60.0f;

  static constexpr float BALL_SPD = 70.0f;
  static constexpr int BALL_R = 2;

  bool bricks[BRICK_ROWS][BRICK_COLS];
  float ballX, ballY;
  float ballDX, ballDY;
  float paddleX;
  int score;

  void resetBall() {
    ballX = W / 2.0f;
    ballY = H / 2.0f;
    float angle = (score % 2 == 0) ? 0.785f : 2.356f;
    ballDX = cosf(angle) * BALL_SPD;
    ballDY = -fabsf(sinf(angle) * BALL_SPD);
  }

  void moveBall(float dt) {
    ballX += ballDX * dt;
    ballY += ballDY * dt;

    if (ballX - BALL_R <= 0) {
      ballX = BALL_R;
      ballDX = fabsf(ballDX);
    }
    if (ballX + BALL_R >= W - 1) {
      ballX = W - 1 - BALL_R;
      ballDX = -fabsf(ballDX);
    }

    if (ballY - BALL_R <= 0) {
      ballY = BALL_R;
      ballDY = fabsf(ballDY);
    }

    if (ballDY > 0 && ballY + BALL_R >= PADDLE_Y && ballY - BALL_R <= PADDLE_Y + PADDLE_H &&
        ballX + BALL_R >= paddleX && ballX - BALL_R <= paddleX + PADDLE_W) {
      ballDY = -fabsf(ballDY);
      float rel = (ballX - (paddleX + PADDLE_W / 2.0f)) / (PADDLE_W / 2.0f);
      ballDX += rel * 12.0f;
      float spd = sqrtf(ballDX * ballDX + ballDY * ballDY);
      if (spd > 0.0f) {
        ballDX = ballDX / spd * BALL_SPD;
        ballDY = ballDY / spd * BALL_SPD;
      }
    }

    bool hit = false;
    for (int r = 0; r < BRICK_ROWS && !hit; r++) {
      for (int c = 0; c < BRICK_COLS && !hit; c++) {
        yield();
        if (!bricks[r][c])
          continue;
        int bx = BRICK_LEFT + c * (BRICK_W + BRICK_GAP);
        int by = BRICK_TOP + r * (BRICK_H + BRICK_GAP);
        if (ballX + BALL_R >= bx && ballX - BALL_R <= bx + BRICK_W && ballY + BALL_R >= by &&
            ballY - BALL_R <= by + BRICK_H) {
          bricks[r][c] = false;
          score++;
          ballDY = -ballDY;
          hit = true;
        }
      }
    }

    if (ballY - BALL_R > H) {
      begin();
      return;
    }

    bool anyLeft = false;
    for (int r = 0; r < BRICK_ROWS && !anyLeft; r++)
      for (int c = 0; c < BRICK_COLS; c++)
        if (bricks[r][c]) {
          anyLeft = true;
          break;
        }
    if (!anyLeft) {
      for (int r = 0; r < BRICK_ROWS; r++)
        for (int c = 0; c < BRICK_COLS; c++)
          bricks[r][c] = true;
      resetBall();
    }
  }

  void movePaddle(float dt) {
    float centre = paddleX + PADDLE_W / 2.0f;
    float jitter = sinf(millis() / 2700.0f) * 7.0f;
    float target = ballX + jitter;

    float diff = target - centre;
    float maxMove = PADDLE_SPD * dt;
    if (diff > maxMove)
      diff = maxMove;
    if (diff < -maxMove)
      diff = -maxMove;

    paddleX += diff;
    if (paddleX < 0)
      paddleX = 0;
    if (paddleX + PADDLE_W > W)
      paddleX = (float)(W - PADDLE_W);
  }

  void render(U8G2* u8g2) {
    u8g2->clearBuffer();

    for (int r = 0; r < BRICK_ROWS; r++) {
      for (int c = 0; c < BRICK_COLS; c++) {
        yield();
        if (!bricks[r][c])
          continue;
        int bx = BRICK_LEFT + c * (BRICK_W + BRICK_GAP);
        int by = BRICK_TOP + r * (BRICK_H + BRICK_GAP);
        DisplayManager::drawFrame(u8g2, bx, by, BRICK_W, BRICK_H);
      }
    }

    DisplayManager::drawFrame(u8g2, (int)paddleX, PADDLE_Y, PADDLE_W, PADDLE_H);
    DisplayManager::drawCircle(u8g2, (int)ballX, (int)ballY, BALL_R);

    char scoreBuf[8];
    snprintf(scoreBuf, sizeof(scoreBuf), "%d", score);
    u8g2->setFont(u8g2_font_5x7_tf);
    DisplayManager::drawStr(u8g2, 2, 7, scoreBuf);

    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  float getBallX() const { return ballX; }
  float getBallY() const { return ballY; }
  float getBallDX() const { return ballDX; }
  float getBallDY() const { return ballDY; }
  float getPaddleX() const { return paddleX; }
  int getScore() const { return score; }
  bool isBrickAlive(int r, int c) const { return bricks[r][c]; }
  int getBrickRows() const { return BRICK_ROWS; }
  int getBrickCols() const { return BRICK_COLS; }
  float getBallSpeed() const { return BALL_SPD; }
  int getPaddleW() const { return PADDLE_W; }
  int getW() const { return W; }
  void testMoveBall(float dt) { moveBall(dt); }
  void testMovePaddle(float dt) { movePaddle(dt); }
  void testResetBall() { resetBall(); }
#endif

 public:
  const char* getName() const override { return "breakout"; }

  void begin() override {
    for (int r = 0; r < BRICK_ROWS; r++)
      for (int c = 0; c < BRICK_COLS; c++)
        bricks[r][c] = true;
    score = 0;
    paddleX = (W - PADDLE_W) / 2.0f;
    resetBall();
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    float dt = deltaMs / 1000.0f;
    if (dt > 0.05f)
      dt = 0.05f;

    moveBall(dt);
    movePaddle(dt);
    render(u8g2);
  }
};
