#include <Arduino.h>

#include <AUnit.h>

#include "../../src/BoingMode.h"
#include "../../src/BreakoutMode.h"
#include "../../src/ClockMode.h"
#include "../../src/DisplayMode.h"
#include "../../src/PacManMode.h"
#include "../../src/StatusMode.h"
#include "../../src/WeatherMode.h"

#include <cmath>
#include <cstring>

test(StatusModeTest, GetName) {
  StatusMode mode;
  assertEqual(strcmp("status", mode.getName()), 0);
}

test(StatusModeTest, NameIsNotNull) {
  StatusMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(BoingModeTest, GetName) {
  BoingMode mode;
  assertEqual(strcmp("boing", mode.getName()), 0);
}

test(BoingModeTest, NameIsNotNull) {
  BoingMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(WeatherModeTest, GetName) {
  WeatherMode mode;
  assertEqual(strcmp("weather", mode.getName()), 0);
}

test(WeatherModeTest, NameIsNotNull) {
  WeatherMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(ClockModeTest, GetName) {
  ClockMode mode;
  assertEqual(strcmp("clock", mode.getName()), 0);
}

test(ClockModeTest, NameIsNotNull) {
  ClockMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(BreakoutModeTest, GetName) {
  BreakoutMode mode;
  assertEqual(strcmp("breakout", mode.getName()), 0);
}

test(BreakoutModeTest, NameIsNotNull) {
  BreakoutMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(PacManModeTest, GetName) {
  PacManMode mode;
  assertEqual(strcmp("pacman", mode.getName()), 0);
}

test(PacManModeTest, NameIsNotNull) {
  PacManMode mode;
  assertTrue(mode.getName() != nullptr);
}

test(DisplayModeTest, NamesAreLowercase) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;
  ClockMode clock;
  BreakoutMode breakout;
  PacManMode pacman;

  assertEqual(strcmp("status", status.getName()), 0);
  assertEqual(strcmp("boing", boing.getName()), 0);
  assertEqual(strcmp("weather", weather.getName()), 0);
  assertEqual(strcmp("clock", clock.getName()), 0);
  assertEqual(strcmp("breakout", breakout.getName()), 0);
  assertEqual(strcmp("pacman", pacman.getName()), 0);
}

test(DisplayModeTest, NamesAreUnique) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;
  ClockMode clock;
  BreakoutMode breakout;
  PacManMode pacman;

  assertNotEqual(strcmp(status.getName(), boing.getName()), 0);
  assertNotEqual(strcmp(status.getName(), weather.getName()), 0);
  assertNotEqual(strcmp(status.getName(), clock.getName()), 0);
  assertNotEqual(strcmp(status.getName(), breakout.getName()), 0);
  assertNotEqual(strcmp(status.getName(), pacman.getName()), 0);
  assertNotEqual(strcmp(boing.getName(), weather.getName()), 0);
  assertNotEqual(strcmp(boing.getName(), clock.getName()), 0);
  assertNotEqual(strcmp(boing.getName(), breakout.getName()), 0);
  assertNotEqual(strcmp(boing.getName(), pacman.getName()), 0);
  assertNotEqual(strcmp(weather.getName(), clock.getName()), 0);
  assertNotEqual(strcmp(weather.getName(), breakout.getName()), 0);
  assertNotEqual(strcmp(weather.getName(), pacman.getName()), 0);
  assertNotEqual(strcmp(clock.getName(), breakout.getName()), 0);
  assertNotEqual(strcmp(clock.getName(), pacman.getName()), 0);
  assertNotEqual(strcmp(breakout.getName(), pacman.getName()), 0);
}

test(DisplayModeTest, BeginMethodExists) {
  StatusMode mode;
  mode.begin();
  assertTrue(true);
}

test(DisplayModeTest, EndMethodExists) {
  StatusMode mode;
  mode.end();
  assertTrue(true);
}

test(BreakoutModeTest, BeginInitialisesAllBricks) {
  BreakoutMode mode;
  mode.begin();
  for (int r = 0; r < mode.getBrickRows(); r++)
    for (int c = 0; c < mode.getBrickCols(); c++)
      assertTrue(mode.isBrickAlive(r, c));
}

test(BreakoutModeTest, BeginResetsScore) {
  BreakoutMode mode;
  mode.begin();
  assertEqual(mode.getScore(), 0);
}

test(BreakoutModeTest, BeginCentresPaddle) {
  BreakoutMode mode;
  mode.begin();
  float centre = mode.getPaddleX() + mode.getPaddleW() / 2.0f;
  float screenCentre = mode.getW() / 2.0f;
  assertTrue(centre > screenCentre - 5.0f);
  assertTrue(centre < screenCentre + 5.0f);
}

test(BreakoutModeTest, BeginPlacesBallAtScreenCentre) {
  BreakoutMode mode;
  mode.begin();
  float cx = mode.getW() / 2.0f;
  assertTrue(mode.getBallX() > cx - 2.0f);
  assertTrue(mode.getBallX() < cx + 2.0f);
}

test(BreakoutModeTest, ResetBallPreservesSpeed) {
  BreakoutMode mode;
  mode.begin();
  mode.testResetBall();
  float spd = sqrtf(mode.getBallDX() * mode.getBallDX() + mode.getBallDY() * mode.getBallDY());
  assertTrue(spd > mode.getBallSpeed() - 1.0f);
  assertTrue(spd < mode.getBallSpeed() + 1.0f);
}

test(BreakoutModeTest, ResetBallAlwaysMovesUpward) {
  BreakoutMode mode;
  mode.begin();
  mode.testResetBall();
  assertTrue(mode.getBallDY() < 0.0f);
}

test(BreakoutModeTest, BallClampedAtLeftWall) {
  BreakoutMode mode;
  mode.begin();
  mode.testResetBall();
  for (int i = 0; i < 200; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    assertTrue(mode.getBallX() >= 0.0f);
    assertTrue(mode.getBallX() <= (float)mode.getW());
  }
}

test(BreakoutModeTest, BallNeverExceedsRightWall) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 200; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    assertTrue(mode.getBallX() <= (float)mode.getW());
  }
}

test(BreakoutModeTest, BallNeverExceedsTopWall) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 200; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    assertTrue(mode.getBallY() >= 0.0f);
  }
}

test(BreakoutModeTest, PaddleNeverExceedsLeftBoundary) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 200; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    assertTrue(mode.getPaddleX() >= 0.0f);
  }
}

test(BreakoutModeTest, PaddleNeverExceedsRightBoundary) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 200; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    assertTrue(mode.getPaddleX() + mode.getPaddleW() <= (float)mode.getW());
  }
}

test(BreakoutModeTest, BrickHitIncrementsScore) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 500; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    if (mode.getScore() > 0)
      break;
  }
  assertTrue(mode.getScore() > 0);
}

test(BreakoutModeTest, BrickHitRemovesBrick) {
  BreakoutMode mode;
  mode.begin();
  for (int i = 0; i < 500; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
    if (mode.getScore() > 0)
      break;
  }
  int alive = 0;
  for (int r = 0; r < mode.getBrickRows(); r++)
    for (int c = 0; c < mode.getBrickCols(); c++)
      if (mode.isBrickAlive(r, c))
        alive++;
  assertTrue(alive < mode.getBrickRows() * mode.getBrickCols());
}

test(BreakoutModeTest, ScoreEqualsDestroyedBricksWithinFirstClear) {
  BreakoutMode mode;
  mode.begin();
  int total = mode.getBrickRows() * mode.getBrickCols();
  for (int i = 0; i < 500 && mode.getScore() < total; i++) {
    mode.testMoveBall(0.04f);
    mode.testMovePaddle(0.04f);
  }
  int alive = 0;
  for (int r = 0; r < mode.getBrickRows(); r++)
    for (int c = 0; c < mode.getBrickCols(); c++)
      if (mode.isBrickAlive(r, c))
        alive++;
  int destroyed = total - alive;
  assertEqual(mode.getScore(), destroyed);
}

test(ClockModeTest, DayNamesCount) {
  for (int i = 0; i < 7; i++)
    assertTrue(ClockMode::DAY_NAMES[i] != nullptr);
}

test(ClockModeTest, DayNamesAreThreeChars) {
  for (int i = 0; i < 7; i++)
    assertEqual((int)strlen(ClockMode::DAY_NAMES[i]), 3);
}

test(ClockModeTest, DayNamesCorrect) {
  assertEqual(strcmp("Sun", ClockMode::DAY_NAMES[0]), 0);
  assertEqual(strcmp("Mon", ClockMode::DAY_NAMES[1]), 0);
  assertEqual(strcmp("Tue", ClockMode::DAY_NAMES[2]), 0);
  assertEqual(strcmp("Wed", ClockMode::DAY_NAMES[3]), 0);
  assertEqual(strcmp("Thu", ClockMode::DAY_NAMES[4]), 0);
  assertEqual(strcmp("Fri", ClockMode::DAY_NAMES[5]), 0);
  assertEqual(strcmp("Sat", ClockMode::DAY_NAMES[6]), 0);
}

test(ClockModeTest, MonthNamesCount) {
  for (int i = 0; i < 12; i++)
    assertTrue(ClockMode::MONTH_NAMES[i] != nullptr);
}

test(ClockModeTest, MonthNamesAreThreeChars) {
  for (int i = 0; i < 12; i++)
    assertEqual((int)strlen(ClockMode::MONTH_NAMES[i]), 3);
}

test(ClockModeTest, MonthNamesCorrect) {
  assertEqual(strcmp("Jan", ClockMode::MONTH_NAMES[0]), 0);
  assertEqual(strcmp("Feb", ClockMode::MONTH_NAMES[1]), 0);
  assertEqual(strcmp("Mar", ClockMode::MONTH_NAMES[2]), 0);
  assertEqual(strcmp("Apr", ClockMode::MONTH_NAMES[3]), 0);
  assertEqual(strcmp("May", ClockMode::MONTH_NAMES[4]), 0);
  assertEqual(strcmp("Jun", ClockMode::MONTH_NAMES[5]), 0);
  assertEqual(strcmp("Jul", ClockMode::MONTH_NAMES[6]), 0);
  assertEqual(strcmp("Aug", ClockMode::MONTH_NAMES[7]), 0);
  assertEqual(strcmp("Sep", ClockMode::MONTH_NAMES[8]), 0);
  assertEqual(strcmp("Oct", ClockMode::MONTH_NAMES[9]), 0);
  assertEqual(strcmp("Nov", ClockMode::MONTH_NAMES[10]), 0);
  assertEqual(strcmp("Dec", ClockMode::MONTH_NAMES[11]), 0);
}

test(ClockModeTest, ColonVisibleOnBegin) {
  ClockMode mode;
  mode.begin();
  assertTrue(mode.isColonVisible());
}

test(ClockModeTest, ColonDoesNotToggleBeforeInterval) {
  ClockMode mode;
  mode.begin();
  mode.setColonToggleMs(1000);
  mode.testUpdate(1499);
  assertTrue(mode.isColonVisible());
}

test(ClockModeTest, ColonTogglesAfterInterval) {
  ClockMode mode;
  mode.begin();
  mode.setColonToggleMs(1000);
  mode.testUpdate(1500);
  assertFalse(mode.isColonVisible());
}

test(ClockModeTest, ColonTogglesBackAfterSecondInterval) {
  ClockMode mode;
  mode.begin();
  mode.setColonToggleMs(1000);
  mode.testUpdate(1500);
  mode.testUpdate(2000);
  assertTrue(mode.isColonVisible());
}

test(ClockModeTest, NtpResyncTracksLastSyncTime) {
  ClockMode mode;
  mode.begin();
  uint32_t start = 1000;
  mode.setLastNtpSyncMs(start);
  mode.setColonToggleMs(start);
  mode.testUpdate(start + Config::NTP_RESYNC_INTERVAL_MS - 1);
  assertEqual(mode.getLastNtpSyncMs(), start);
}

test(ClockModeTest, NtpResyncUpdatesTimestampAfterInterval) {
  ClockMode mode;
  mode.begin();
  uint32_t start = 1000;
  mode.setLastNtpSyncMs(start);
  mode.setColonToggleMs(start);
  uint32_t atResync = start + Config::NTP_RESYNC_INTERVAL_MS;
  mode.testUpdate(atResync);
  assertEqual(mode.getLastNtpSyncMs(), atResync);
}

test(PacManModeTest, BeginResetsScore) {
  PacManMode mode;
  mode.begin();
  assertEqual(mode.getScore(), 0);
}

test(PacManModeTest, BeginPlacesPacManInGrid) {
  PacManMode mode;
  mode.begin();
  assertTrue(mode.getPacX() >= 0.0f);
  assertTrue(mode.getPacX() < (float)mode.getCols());
  assertTrue(mode.getPacY() >= 0.0f);
  assertTrue(mode.getPacY() < (float)mode.getRows());
}

test(PacManModeTest, BeginPacManStartsOnCorridor) {
  PacManMode mode;
  mode.begin();
  int col = (int)roundf(mode.getPacX());
  int row = (int)roundf(mode.getPacY());
  assertFalse(mode.testIsWall(col, row));
}

test(PacManModeTest, BeginPopulatesDots) {
  PacManMode mode;
  mode.begin();
  int dotCount = 0;
  for (int r = 0; r < mode.getRows(); r++)
    for (int c = 0; c < mode.getCols(); c++)
      if (mode.isDotAlive(r, c))
        dotCount++;
  assertTrue(dotCount > 10);
}

test(PacManModeTest, WallTilesHaveNoDots) {
  PacManMode mode;
  mode.begin();
  for (int r = 0; r < mode.getRows(); r++)
    for (int c = 0; c < mode.getCols(); c++)
      if (mode.testIsWall(c, r))
        assertFalse(mode.isDotAlive(r, c));
}

test(PacManModeTest, MazeHasBorderWalls) {
  PacManMode mode;
  mode.begin();
  int topWalls = 0, botWalls = 0;
  for (int c = 0; c < mode.getCols(); c++) {
    if (mode.testIsWall(c, 0))
      topWalls++;
    if (mode.testIsWall(c, mode.getRows() - 1))
      botWalls++;
  }
  assertTrue(topWalls >= mode.getCols() - 2);
  assertTrue(botWalls >= mode.getCols() - 2);
}

test(PacManModeTest, EntityWallClampZerosVelocity) {
  PacManMode mode;
  mode.begin();
  PacManMode::Entity e;
  e.x = 19.0f;
  e.y = 4.0f;
  e.dx = 1.0f;
  e.dy = 0.0f;
  e.speed = 10.0f;
  mode.testMoveEntity(e, 0.2f);
  assertEqual(e.dx, 0.0f);
  assertEqual(e.dy, 0.0f);
}

test(PacManModeTest, EntityMovesThroughCorridor) {
  PacManMode mode;
  mode.begin();
  PacManMode::Entity e;
  e.x = 1.0f;
  e.y = 4.0f;
  e.dx = 1.0f;
  e.dy = 0.0f;
  e.speed = 3.5f;
  float startX = e.x;
  mode.testMoveEntity(e, 0.04f);
  assertTrue(e.x > startX);
}

test(PacManModeTest, EntityWrapsLeftEdge) {
  PacManMode mode;
  mode.begin();
  PacManMode::Entity e;
  e.x = 0.0f;
  e.y = 8.0f;
  e.dx = -1.0f;
  e.dy = 0.0f;
  e.speed = 10.0f;
  mode.testMoveEntity(e, 0.15f);
  assertTrue(e.x > (float)(mode.getCols() / 2));
}

test(PacManModeTest, MazeHasAtLeastOneCorridor) {
  PacManMode mode;
  mode.begin();
  int corridors = 0;
  for (int r = 0; r < mode.getRows(); r++)
    for (int c = 0; c < mode.getCols(); c++)
      if (!mode.testIsWall(c, r))
        corridors++;
  assertTrue(corridors > 20);
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(1000);
}

void loop() {
  aunit::TestRunner::run();
}
