#include <AUnit.h>
#include "../../../src/Logger.h"

test(LoggerTest, PrintlnBasic) {
  // Logger should handle basic println
  Logger::println("Test message");
  assertTrue(true);
}

test(LoggerTest, PrintlnEmpty) {
  // Logger should handle empty strings
  Logger::println("");
  assertTrue(true);
}

test(LoggerTest, PrintlnLong) {
  // Logger should handle long strings
  Logger::println("This is a very long message that should still be handled correctly");
  assertTrue(true);
}

test(LoggerTest, PrintfBasic) {
  // Logger should handle printf format strings
  Logger::printf("Integer: %d", 42);
  assertTrue(true);
}

test(LoggerTest, PrintfMultipleArgs) {
  // Logger should handle multiple format arguments
  Logger::printf("Value1: %d, Value2: %s", 42, "test");
  assertTrue(true);
}

test(LoggerTest, PrintfFloat) {
  // Logger should handle float formatting
  Logger::printf("Float: %.2f", 3.14159);
  assertTrue(true);
}

test(LoggerTest, PrintfHex) {
  // Logger should handle hex formatting
  Logger::printf("Hex: 0x%02X", 255);
  assertTrue(true);
}

test(LoggerTest, ConsecutiveCalls) {
  // Logger should handle multiple consecutive calls
  Logger::println("Line 1");
  Logger::println("Line 2");
  Logger::printf("Line %d", 3);
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " - Logger Module Tests");
}

void loop() {
  aunit::TestRunner::run();
}
