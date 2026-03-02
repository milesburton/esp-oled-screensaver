#pragma once

#include <Arduino.h>

#include <math.h>

#include "Config.h"
#include "DisplayManager.h"
#include "DisplayMode.h"

class PacManMode : public DisplayMode {
 public:
  struct Entity {
    float x, y;
    float dx, dy;
    float speed;
  };

 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;

  static constexpr int COLS = 21;
  static constexpr int ROWS = 11;
  static constexpr int TILE = 6;
  static constexpr int GRID_X = 1;
  static constexpr int GRID_Y = 10;

  static const uint8_t MAZE[ROWS][COLS];

  bool dots[ROWS][COLS];

  Entity pac;

  struct Ghost : public Entity {
    uint8_t scatter;
  };
  Ghost ghosts[4];

  int score;
  uint32_t frameCount;
  uint8_t mouthPhase;

  static bool isWall(int col, int row) {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS)
      return true;
    return MAZE[row][col] != 0;
  }

  static bool isCorridor(int col, int row) { return !isWall(col, row); }

  static int tilePixX(float col) { return GRID_X + (int)(col * TILE) + TILE / 2; }
  static int tilePixY(float row) { return GRID_Y + (int)(row * TILE) + TILE / 2; }

  void moveEntity(Entity& e, float dt) {
    float nx = e.x + e.dx * e.speed * dt;
    float ny = e.y + e.dy * e.speed * dt;

    int col = (int)(nx + 0.5f);
    int row = (int)(ny + 0.5f);

    if (!isWall(col, row)) {
      e.x = nx;
      e.y = ny;
    } else {
      e.x = roundf(e.x);
      e.y = roundf(e.y);
      e.dx = 0;
      e.dy = 0;
    }
    if (e.x < 0)
      e.x = COLS - 1;
    if (e.x >= COLS)
      e.x = 0;
  }

  void turnPacMan() {
    int col = (int)roundf(pac.x);
    int row = (int)roundf(pac.y);
    if (fabsf(pac.x - col) > 0.1f || fabsf(pac.y - row) > 0.1f)
      return;

    static const int8_t dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    int bestDir = -1;
    int bestDots = -1;

    for (int d = 0; d < 4; d++) {
      if (dirs[d][0] == -(int)pac.dx && dirs[d][1] == -(int)pac.dy)
        continue;
      int nc = col + dirs[d][0];
      int nr = row + dirs[d][1];
      if (isWall(nc, nr))
        continue;

      int dotCount = 0;
      int lc = nc, lr = nr;
      for (int step = 0; step < 3; step++) {
        if (isCorridor(lc, lr) && dots[lr][lc])
          dotCount++;
        lc += dirs[d][0];
        lr += dirs[d][1];
      }
      if (dotCount > bestDots) {
        bestDots = dotCount;
        bestDir = d;
      }
    }

    if (bestDir >= 0) {
      pac.dx = dirs[bestDir][0];
      pac.dy = dirs[bestDir][1];
    }
  }

  void turnGhost(Ghost& g) {
    int col = (int)roundf(g.x);
    int row = (int)roundf(g.y);
    if (fabsf(g.x - col) > 0.1f || fabsf(g.y - row) > 0.1f)
      return;

    static const int8_t dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int pcol = (int)roundf(pac.x);
    int prow = (int)roundf(pac.y);

    int bestDir = -1;
    int bestDist = 9999;
    int worstDist = -1;
    bool fleeing = (g.scatter > 0);

    for (int d = 0; d < 4; d++) {
      if (dirs[d][0] == -(int)g.dx && dirs[d][1] == -(int)g.dy)
        continue;
      int nc = col + dirs[d][0];
      int nr = row + dirs[d][1];
      if (isWall(nc, nr))
        continue;

      int dist = abs(nc - pcol) + abs(nr - prow);

      if (!fleeing) {
        if (dist < bestDist) {
          bestDist = dist;
          bestDir = d;
        }
      } else {
        if (dist > worstDist) {
          worstDist = dist;
          bestDir = d;
        }
      }
    }

    if (bestDir >= 0) {
      g.dx = dirs[bestDir][0];
      g.dy = dirs[bestDir][1];
    }

    if (g.scatter > 0)
      g.scatter--;
  }

  void resetDots() {
    for (int r = 0; r < ROWS; r++)
      for (int c = 0; c < COLS; c++)
        dots[r][c] = isCorridor(c, r);
  }

  int eatDot() {
    int col = (int)roundf(pac.x);
    int row = (int)roundf(pac.y);
    if (isCorridor(col, row) && dots[row][col]) {
      dots[row][col] = false;
      return 1;
    }
    return 0;
  }

  bool allDotsEaten() {
    for (int r = 0; r < ROWS; r++)
      for (int c = 0; c < COLS; c++)
        if (dots[r][c])
          return false;
    return true;
  }

  bool ghostCatchesPac() {
    for (int i = 0; i < 4; i++) {
      float dx = ghosts[i].x - pac.x;
      float dy = ghosts[i].y - pac.y;
      if (sqrtf(dx * dx + dy * dy) < 0.9f)
        return true;
    }
    return false;
  }

  void render(U8G2* u8g2) {
    u8g2->clearBuffer();

    u8g2->setFont(u8g2_font_5x7_tf);
    char buf[12];
    snprintf(buf, sizeof(buf), "PAC %d", score);
    DisplayManager::drawStr(u8g2, 0, 7, buf);

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        yield();
        if (!MAZE[r][c])
          continue;
        int px = GRID_X + c * TILE;
        int py = GRID_Y + r * TILE;
        DisplayManager::drawFrame(u8g2, px, py, TILE, TILE);
      }
    }

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < COLS; c++) {
        if (!dots[r][c])
          continue;
        DisplayManager::drawPixel(u8g2, tilePixX(c), tilePixY(r));
      }
    }

    for (int i = 0; i < 4; i++) {
      int gx = tilePixX(ghosts[i].x) - 2;
      int gy = tilePixY(ghosts[i].y) - 2;
      DisplayManager::drawFrame(u8g2, gx, gy, 5, 5);
      DisplayManager::drawPixel(u8g2, gx + 1, gy + 1);
      DisplayManager::drawPixel(u8g2, gx + 3, gy + 1);
    }

    int px = tilePixX(pac.x);
    int py = tilePixY(pac.y);
    static constexpr int pr = 3;

    for (int dy2 = -pr; dy2 <= pr; dy2++) {
      int half = (int)sqrtf((float)(pr * pr - dy2 * dy2));
      for (int dx2 = -half; dx2 <= half; dx2++) {
        float angle = atan2f((float)dy2, (float)dx2);
        float mouthAngle = (mouthPhase < 4) ? (mouthPhase / 4.0f) * (3.14159f / 3.0f)
                                            : ((8 - mouthPhase) / 4.0f) * (3.14159f / 3.0f);

        bool inMouth = false;
        if (mouthAngle > 0.05f) {
          float faceAngle = atan2f(pac.dy, pac.dx != 0 ? pac.dx : 1.0f);
          float diff = angle - faceAngle;
          while (diff > 3.14159f)
            diff -= 2.0f * 3.14159f;
          while (diff < -3.14159f)
            diff += 2.0f * 3.14159f;
          inMouth = fabsf(diff) < mouthAngle;
        }

        if (!inMouth)
          DisplayManager::drawPixel(u8g2, px + dx2, py + dy2);
      }
    }

    u8g2->sendBuffer();
  }

 public:
  const char* getName() const override { return "pacman"; }

  void begin() override {
    score = 0;
    frameCount = 0;
    mouthPhase = 0;
    resetDots();
    pac = {10.0f, 5.0f, 1.0f, 0.0f, 3.5f};
    float gs = 2.8f;
    ghosts[0] = {{9.0f, 4.0f, -1.0f, 0.0f, gs}, 0};
    ghosts[1] = {{11.0f, 4.0f, 1.0f, 0.0f, gs}, 0};
    ghosts[2] = {{9.0f, 6.0f, -1.0f, 0.0f, gs}, 0};
    ghosts[3] = {{11.0f, 6.0f, 1.0f, 0.0f, gs}, 0};
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    float dt = deltaMs / 1000.0f;
    if (dt > 0.05f)
      dt = 0.05f;

    frameCount++;
    mouthPhase = (mouthPhase + 1) % 8;

    turnPacMan();
    moveEntity(pac, dt);
    score += eatDot();

    for (int i = 0; i < 4; i++) {
      if (frameCount % (3 + i) == 0) {
        turnGhost(ghosts[i]);
        if (frameCount % (180 + i * 47) == 0)
          ghosts[i].scatter = 40;
      }
      moveEntity(ghosts[i], dt);
      yield();
    }

    if (ghostCatchesPac()) {
      pac.x = 10.0f;
      pac.y = 5.0f;
      pac.dx = 1.0f;
      pac.dy = 0.0f;
      ghosts[0].x = 9.0f;
      ghosts[0].y = 4.0f;
      ghosts[1].x = 11.0f;
      ghosts[1].y = 4.0f;
      ghosts[2].x = 9.0f;
      ghosts[2].y = 6.0f;
      ghosts[3].x = 11.0f;
      ghosts[3].y = 6.0f;
    }

    if (allDotsEaten())
      begin();

    render(u8g2);
  }

#ifdef NATIVE_TEST

 public:
  float getPacX() const { return pac.x; }
  float getPacY() const { return pac.y; }
  float getPacDX() const { return pac.dx; }
  float getPacDY() const { return pac.dy; }
  int getScore() const { return score; }
  bool isDotAlive(int r, int c) const { return dots[r][c]; }
  bool testIsWall(int c, int r) const { return isWall(c, r); }
  void testMoveEntity(Entity& e, float dt) { moveEntity(e, dt); }
  int getRows() const { return ROWS; }
  int getCols() const { return COLS; }
#endif
};

// clang-format off
const uint8_t PacManMode::MAZE[PacManMode::ROWS][PacManMode::COLS] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1},
  {1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
  {1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};
// clang-format on
