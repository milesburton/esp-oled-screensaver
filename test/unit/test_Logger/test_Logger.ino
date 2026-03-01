#include <AUnit.h>
#include "../../../src/Logger.h"

test(LoggerTest, PrintlnBasic) {
  // Logger should handle basic println
  Logger::println("Test message");
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

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " - Logger Module Tests");
}

void loop() {
  aunit::TestRunner::run();
}
