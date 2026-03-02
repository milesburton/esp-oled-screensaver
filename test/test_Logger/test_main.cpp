#include <Arduino.h>

#include <AUnit.h>

#include "../../src/Logger.h"

test(LoggerTest, DefaultLevelIsInfo) {
  assertEqual((uint8_t)Logger::minLevel, (uint8_t)Logger::Level::INFO);
}

test(LoggerTest, LevelOrderDebugLessThanInfo) {
  assertTrue((uint8_t)Logger::Level::DEBUG < (uint8_t)Logger::Level::INFO);
}

test(LoggerTest, LevelOrderInfoLessThanWarn) {
  assertTrue((uint8_t)Logger::Level::INFO < (uint8_t)Logger::Level::WARN);
}

test(LoggerTest, LevelOrderWarnLessThanError) {
  assertTrue((uint8_t)Logger::Level::WARN < (uint8_t)Logger::Level::ERROR);
}

test(LoggerTest, MinLevelFiltersDebugWhenSetToInfo) {
  Logger::minLevel = Logger::Level::INFO;
  assertTrue(Logger::Level::DEBUG < Logger::minLevel);
}

test(LoggerTest, MinLevelAllowsInfoWhenSetToInfo) {
  Logger::minLevel = Logger::Level::INFO;
  assertFalse(Logger::Level::INFO < Logger::minLevel);
}

test(LoggerTest, MinLevelCanBeChangedToDebug) {
  Logger::minLevel = Logger::Level::DEBUG;
  assertEqual((uint8_t)Logger::minLevel, (uint8_t)Logger::Level::DEBUG);
  Logger::minLevel = Logger::Level::INFO;
}

test(LoggerTest, PrintlnAndPrintfDoNotCrash) {
  Logger::println("ping");
  Logger::printf("val=%d", 1);
  Logger::debug("dbg");
  Logger::warn("wrn");
  Logger::error("err");
  assertTrue(true);
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
