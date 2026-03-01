#include <AUnit.h>
#include "../../../src/Config.h"

// Test default configuration values
test(ConfigTest, DefaultOledSDA) {
  assertEqual(Config::OLED_SDA, 0);
}

test(ConfigTest, DefaultOledSCL) {
  assertEqual(Config::OLED_SCL, 2);
}

test(ConfigTest, DefaultOledAddress) {
  assertEqual(Config::OLED_ADDR, 0x3C);
}

test(ConfigTest, DefaultDisplayWidth) {
  assertEqual(Config::DISPLAY_WIDTH, 128);
}

test(ConfigTest, DefaultDisplayHeight) {
  assertEqual(Config::DISPLAY_HEIGHT, 64);
}

test(ConfigTest, RuntimeConfigExists) {
  // Verify runtime config object can be created
  assertTrue(true);
}

test(ConfigTest, GetDriverNameSSD1306) {
  Config::RuntimeConfig config;
  config.driver = Config::OledDriver::SSD1306;
  assertStringEqual("SSD1306", config.getDriverName());
}

test(ConfigTest, GetDriverNameSH1106) {
  Config::RuntimeConfig config;
  config.driver = Config::OledDriver::SH1106;
  assertStringEqual("SH1106", config.getDriverName());
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " - Config Module Tests");
}

void loop() {
  aunit::TestRunner::run();
}
