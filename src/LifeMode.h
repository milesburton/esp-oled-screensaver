#pragma once

#include "Config.h"
#include "DisplayMode.h"

class LifeMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;

  // Double-buffer — one cell per bit would be tight; use bytes for simplicity
  uint8_t grid[H][W];
  uint8_t next[H][W];

  uint32_t seed;
  uint16_t lastPopulation;
  uint8_t stasisCount;
  static constexpr uint8_t STASIS_LIMIT = 10;

  uint32_t nextRand() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
  }

  // The icon: 3x3 grid, cells marked 1 are filled circles
  // Row 0 (top):    [ ]  [X]  [ ]
  // Row 1 (middle): [ ]  [ ]  [X]
  // Row 2 (bottom): [X]  [X]  [ ]
  static constexpr uint8_t ICON_CELLS[3][3] = {{0, 1, 0}, {0, 0, 1}, {1, 1, 0}};

  void seedFromIcon() {
    memset(grid, 0, sizeof(grid));

    // Each icon cell maps to a ~10x10 block of Life cells, centred on the display
    // 3 cells * 14px each = 42px wide, centred in 128px -> margin = 43
    static constexpr int CELL_SIZE = 14;
    static constexpr int MARGIN_X = (W - 3 * CELL_SIZE) / 2;
    static constexpr int MARGIN_Y = (H - 3 * CELL_SIZE) / 2;
    static constexpr int DOT_RADIUS = 5;

    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        if (!ICON_CELLS[row][col]) continue;
        int cx = MARGIN_X + col * CELL_SIZE + CELL_SIZE / 2;
        int cy = MARGIN_Y + row * CELL_SIZE + CELL_SIZE / 2;
        for (int dy = -DOT_RADIUS; dy <= DOT_RADIUS; dy++) {
          for (int dx = -DOT_RADIUS; dx <= DOT_RADIUS; dx++) {
            if (dx * dx + dy * dy <= DOT_RADIUS * DOT_RADIUS) {
              int px = cx + dx, py = cy + dy;
              if (px >= 0 && px < W && py >= 0 && py < H)
                grid[py][px] = 1;
            }
          }
        }
      }
    }
  }

  void seedRandom() {
    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++)
        grid[y][x] = (nextRand() & 3) == 0 ? 1 : 0;
  }

  uint16_t step() {
    uint16_t population = 0;
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        uint8_t n = 0;
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + W) % W;
            int ny = (y + dy + H) % H;
            n += grid[ny][nx];
          }
        }
        bool alive = grid[y][x];
        bool survives = alive ? (n == 2 || n == 3) : (n == 3);
        next[y][x] = survives ? 1 : 0;
        if (survives) population++;
      }
      yield();
    }
    memcpy(grid, next, sizeof(grid));
    return population;
  }

 public:
  LifeMode() : seed(0xCAFEBABE), lastPopulation(0), stasisCount(0) {}

  const char* getName() const override { return "life"; }

  void begin() override {
    seed = millis() ^ 0xCAFEBABE;
    stasisCount = 0;
    lastPopulation = 0;
    seedFromIcon();
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    uint16_t population = step();

    // Detect death or stasis
    if (population == 0 || population == lastPopulation) {
      stasisCount++;
    } else {
      stasisCount = 0;
    }
    lastPopulation = population;

    if (stasisCount >= STASIS_LIMIT) {
      seedRandom();
      stasisCount = 0;
      lastPopulation = 0;
    }

    u8g2->clearBuffer();
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        if (grid[y][x]) u8g2->drawPixel(x, y);
      }
      yield();
    }
    u8g2->sendBuffer();
  }
};
