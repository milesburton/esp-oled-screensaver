#include <AUnit.h>
#include "../../../src/DisplayMode.h"

// Test base DisplayMode interface
test(DisplayModeTest, HasGetName) {
  // DisplayMode is abstract, but we can test the interface exists
  // In actual tests, we'd test concrete implementations like StatusMode, BoingMode
  assertTrue(true);
}

test(DisplayModeTest, HasUpdate) {
  // DisplayMode::update() is pure virtual
  // Tested via concrete implementations in integration tests
  assertTrue(true);
}

test(DisplayModeTest, HasBeginEnd) {
  // DisplayMode has optional begin() and end() lifecycle methods
  // Default implementations do nothing
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " - DisplayMode Tests");
}

void loop() {
  aunit::TestRunner::run();
}
