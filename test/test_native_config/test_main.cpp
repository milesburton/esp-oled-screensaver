#include <unity.h>

#include "../../src/Config.h"

#include <cstring>

void test_hostname_is_set() {
  TEST_ASSERT_NOT_NULL(Config::HOSTNAME);
  TEST_ASSERT_GREATER_THAN(0, static_cast<int>(strlen(Config::HOSTNAME)));
}

void test_runtime_defaults() {
  TEST_ASSERT_EQUAL(0, Config::runtime.xOffset);
  TEST_ASSERT_TRUE(Config::runtime.oledEnabled);
  TEST_ASSERT_EQUAL_STRING("SSD1306", Config::runtime.getDriverName());
}

void test_network_defaults() {
  TEST_ASSERT_EQUAL_UINT16(80, Config::HTTP_PORT);
  TEST_ASSERT_EQUAL_UINT16(23, Config::TELNET_PORT);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_hostname_is_set);
  RUN_TEST(test_runtime_defaults);
  RUN_TEST(test_network_defaults);
  return UNITY_END();
}
