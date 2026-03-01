#include <AUnit.h>

#include "../../../src/BoingMode.h"
#include "../../../src/DisplayMode.h"
#include "../../../src/StatusMode.h"
#include "../../../src/WeatherMode.h"

// Test StatusMode
test(StatusModeTest, GetName) {
  StatusMode mode;
  assertStringEqual("status", mode.getName());
}

test(StatusModeTest, NameIsNotNull) {
  StatusMode mode;
  assertNotEqual(mode.getName(), nullptr);
}

// Test BoingMode
test(BoingModeTest, GetName) {
  BoingMode mode;
  assertStringEqual("boing", mode.getName());
}

test(BoingModeTest, NameIsNotNull) {
  BoingMode mode;
  assertNotEqual(mode.getName(), nullptr);
}

// Test WeatherMode
test(WeatherModeTest, GetName) {
  WeatherMode mode;
  assertStringEqual("weather", mode.getName());
}

test(WeatherModeTest, NameIsNotNull) {
  WeatherMode mode;
  assertNotEqual(mode.getName(), nullptr);
}

// Test mode name consistency
test(DisplayModeTest, NamesAreLowercase) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;

  // All mode names should be lowercase for consistency
  assertStringEqual("status", status.getName());
  assertStringEqual("boing", boing.getName());
  assertStringEqual("weather", weather.getName());
}

test(DisplayModeTest, NamesAreUnique) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;

  // Names should be unique
  assertNotEqual(status.getName(), boing.getName());
  assertNotEqual(status.getName(), weather.getName());
  assertNotEqual(boing.getName(), weather.getName());
}

// Test lifecycle methods exist (default empty implementations)
test(DisplayModeTest, BeginMethodExists) {
  StatusMode mode;
  mode.begin();  // Should not crash
  assertTrue(true);
}

test(DisplayModeTest, EndMethodExists) {
  StatusMode mode;
  mode.end();  // Should not crash
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " - DisplayMode Tests");
}

void loop() { aunit::TestRunner::run(); }
