#include <Arduino.h>

#include <AUnit.h>

#include "../../src/Config.h"

#include <cstring>

// Test default OLED hardware configuration
test(ConfigTest, DefaultOledSDA) {
  assertEqual(Config::OLED_SDA, 0);
}

test(ConfigTest, DefaultOledSCL) {
  assertEqual(Config::OLED_SCL, 2);
}

test(ConfigTest, DefaultOledAddress) {
  assertEqual(Config::OLED_ADDR, 0x3C);
}

// Test display dimensions
test(ConfigTest, DefaultDisplayWidth) {
  assertEqual(Config::DISPLAY_WIDTH, 128);
}

test(ConfigTest, DefaultDisplayHeight) {
  assertEqual(Config::DISPLAY_HEIGHT, 64);
}

// Test network port configuration
test(ConfigTest, DefaultHTTPPort) {
  assertEqual(Config::HTTP_PORT, 80);
}

test(ConfigTest, DefaultTelnetPort) {
  assertEqual(Config::TELNET_PORT, 23);
}

// Test runtime configuration
test(ConfigTest, RuntimeConfigDefaultDriver) {
  Config::RuntimeConfig config;
  assertEqual((uint8_t)config.driver, (uint8_t)Config::OledDriver::SSD1306);
}

test(ConfigTest, RuntimeConfigDefaultXOffset) {
  Config::RuntimeConfig config;
  assertEqual(config.xOffset, 0);
}

test(ConfigTest, RuntimeConfigDefaultOledEnabled) {
  Config::RuntimeConfig config;
  assertTrue(config.oledEnabled);
}

// Test driver name methods
test(ConfigTest, GetDriverNameSSD1306) {
  Config::RuntimeConfig config;
  config.driver = Config::OledDriver::SSD1306;
  assertEqual(strcmp("SSD1306", config.getDriverName()), 0);
}

test(ConfigTest, GetDriverNameSH1106) {
  Config::RuntimeConfig config;
  config.driver = Config::OledDriver::SH1106;
  assertEqual(strcmp("SH1106", config.getDriverName()), 0);
}

// Test driver switching
test(ConfigTest, DriverSwitching) {
  Config::RuntimeConfig config;
  config.driver = Config::OledDriver::SSD1306;
  assertEqual(strcmp("SSD1306", config.getDriverName()), 0);

  config.driver = Config::OledDriver::SH1106;
  assertEqual(strcmp("SH1106", config.getDriverName()), 0);
}

// Test X offset configuration
test(ConfigTest, XOffsetConfiguration) {
  Config::RuntimeConfig config;
  config.xOffset = 0;
  assertEqual(config.xOffset, 0);

  config.xOffset = 2;
  assertEqual(config.xOffset, 2);

  config.xOffset = -2;
  assertEqual(config.xOffset, -2);
}

// Test OLED enable/disable
test(ConfigTest, OledToggle) {
  Config::RuntimeConfig config;
  assertTrue(config.oledEnabled);

  config.oledEnabled = false;
  assertFalse(config.oledEnabled);

  config.oledEnabled = true;
  assertTrue(config.oledEnabled);
}

test(ConfigTest, HostnameIsDefined) {
  assertTrue(Config::HOSTNAME != nullptr);
  assertTrue(strlen(Config::HOSTNAME) > 0);
}

test(ConfigTest, VersionIsDefined) {
  assertTrue(Config::FW_VERSION != nullptr);
  assertTrue(strlen(Config::FW_VERSION) > 0);
}

// Test rotation defaults and names
test(ConfigTest, RuntimeConfigDefaultRotation) {
  Config::RuntimeConfig config;
  assertEqual((uint8_t)config.rotation, (uint8_t)Config::DisplayRotation::R0);
}

test(ConfigTest, GetRotationNameR0) {
  Config::RuntimeConfig config;
  config.rotation = Config::DisplayRotation::R0;
  assertEqual(strcmp("0\xC2\xB0", config.getRotationName()), 0);
}

test(ConfigTest, GetRotationNameR2) {
  Config::RuntimeConfig config;
  config.rotation = Config::DisplayRotation::R2;
  assertEqual(strcmp("180\xC2\xB0", config.getRotationName()), 0);
}

test(ConfigTest, GetU8G2Rotation) {
  Config::RuntimeConfig config;
  config.rotation = Config::DisplayRotation::R3;
  assertEqual(config.getU8G2Rotation(), (uint8_t)3);
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
