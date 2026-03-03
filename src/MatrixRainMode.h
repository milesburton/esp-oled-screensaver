#pragma once

#include "Config.h"
#include "DisplayMode.h"

class MatrixRainMode : public DisplayMode {
 private:
  static constexpr int W = Config::DISPLAY_WIDTH;
  static constexpr int H = Config::DISPLAY_HEIGHT;
  static constexpr int CHAR_H = 8;
  static constexpr int CHAR_W = 4;
  static constexpr int COLS = W / CHAR_W;
  static constexpr int ROWS = H / CHAR_H;
  static constexpr int DROP_MIN_LEN = 3;
  static constexpr int DROP_MAX_LEN = ROWS;

  struct Column {
    int8_t head;
    uint8_t len;
    uint8_t speed;
    uint8_t tick;
  };

  Column cols[COLS];
  uint8_t grid[COLS][ROWS];
  uint32_t seed;

  uint32_t nextRand() {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return seed;
  }

  char randChar() {
    static const char CHARSET[] = "01";
    return CHARSET[nextRand() % (sizeof(CHARSET) - 1)];
  }

  void initCol(int c) {
    cols[c].head = -(int8_t)(1 + nextRand() % ROWS);
    cols[c].len = DROP_MIN_LEN + nextRand() % (DROP_MAX_LEN - DROP_MIN_LEN + 1);
    cols[c].speed = 1 + nextRand() % 3;
    cols[c].tick = 0;
    for (int r = 0; r < ROWS; r++) {
      grid[c][r] = ' ';
    }
  }

 public:
  const char* getName() const override { return "matrix"; }

  void begin() override {
    seed = millis() ^ 0xCAFEBABE;
    for (int c = 0; c < COLS; c++) {
      initCol(c);
      cols[c].head = -(int8_t)(nextRand() % ROWS);
    }
    for (int c = 0; c < COLS; c++)
      for (int r = 0; r < ROWS; r++)
        grid[c][r] = ' ';
  }

  void end() override {}

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    if (!u8g2 || deltaMs == 0)
      return;

    (void)deltaMs;

    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_4x6_tr);

    for (int c = 0; c < COLS; c++) {
      cols[c].tick++;
      if (cols[c].tick >= cols[c].speed) {
        cols[c].tick = 0;
        cols[c].head++;

        int h = cols[c].head;
        if (h >= 0 && h < ROWS) {
          grid[c][h] = randChar();
        }

        if (h >= ROWS + static_cast<int>(cols[c].len)) {
          initCol(c);
        }
      }

      for (int r = 0; r < ROWS; r++) {
        if (grid[c][r] == ' ')
          continue;
        int h = cols[c].head;
        if (r == h) {
          u8g2->setDrawColor(1);
        } else if (r > h - static_cast<int>(cols[c].len) && r < h) {
          u8g2->setDrawColor(1);
        } else {
          continue;
        }
        char buf[2] = {grid[c][r], '\0'};
        u8g2->drawStr(c * CHAR_W, (r + 1) * CHAR_H, buf);
      }
    }

    u8g2->setDrawColor(1);
    u8g2->sendBuffer();
  }

#ifdef NATIVE_TEST

 public:
  int getColCount() const { return COLS; }
  int getRowCount() const { return ROWS; }
  int8_t getColHead(int c) const { return cols[c].head; }
  uint8_t getColLen(int c) const { return cols[c].len; }
  char getGridCell(int c, int r) const { return grid[c][r]; }
#endif
};
