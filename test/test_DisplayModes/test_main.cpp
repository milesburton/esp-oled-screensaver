#include <Arduino.h>

#include <AUnit.h>

#include "../../src/BoingMode.h"
#include "../../src/DisplayMode.h"
#include "../../src/StatusMode.h"
#include "../../src/WeatherMode.h"

#include <cstring>

// Test StatusMode
test(StatusModeTest, GetName) {
  StatusMode mode;
  assertEqual(strcmp("status", mode.getName()), 0);
}

test(StatusModeTest, NameIsNotNull) {
  StatusMode mode;
  assertTrue(mode.getName() != nullptr);
}

// Test BoingMode
test(BoingModeTest, GetName) {
  BoingMode mode;
  assertEqual(strcmp("boing", mode.getName()), 0);
}

test(BoingModeTest, NameIsNotNull) {
  BoingMode mode;
  assertTrue(mode.getName() != nullptr);
}

// Test WeatherMode
test(WeatherModeTest, GetName) {
  WeatherMode mode;
  assertEqual(strcmp("weather", mode.getName()), 0);
}

test(WeatherModeTest, NameIsNotNull) {
  WeatherMode mode;
  assertTrue(mode.getName() != nullptr);
}

// Test mode name consistency
test(DisplayModeTest, NamesAreLowercase) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;

  // All mode names should be lowercase for consistency
  assertEqual(strcmp("status", status.getName()), 0);
  assertEqual(strcmp("boing", boing.getName()), 0);
  assertEqual(strcmp("weather", weather.getName()), 0);
}

test(DisplayModeTest, NamesAreUnique) {
  StatusMode status;
  BoingMode boing;
  WeatherMode weather;

  assertNotEqual(strcmp(status.getName(), boing.getName()), 0);
  assertNotEqual(strcmp(status.getName(), weather.getName()), 0);
  assertNotEqual(strcmp(boing.getName(), weather.getName()), 0);
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
  while (!Serial)
    ;
  delay(1000);
}

void loop() {
  aunit::TestRunner::run();
}
