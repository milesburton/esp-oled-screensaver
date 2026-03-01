#pragma once

#include <EEPROM.h>

#include "Logger.h"

namespace CredentialsManager {
// EEPROM layout:
// Byte 0: Magic number (0xAA indicates valid credentials)
// Bytes 1-32: SSID (32 bytes, null-terminated)
// Bytes 33-96: Password (64 bytes, null-terminated)

static constexpr uint16_t EEPROM_SIZE = 512;
static constexpr uint8_t MAGIC_NUMBER = 0xAA;
static constexpr uint16_t MAGIC_OFFSET = 0;
static constexpr uint16_t SSID_OFFSET = 1;
static constexpr uint16_t SSID_SIZE = 32;
static constexpr uint16_t PASS_OFFSET = 33;
static constexpr uint16_t PASS_SIZE = 64;

inline void initialize() {
  EEPROM.begin(EEPROM_SIZE);
}

inline bool hasValidCredentials() {
  return EEPROM.read(MAGIC_OFFSET) == MAGIC_NUMBER;
}

inline bool loadCredentials(char* ssid, uint16_t ssid_max, char* password, uint16_t pass_max) {
  if (!hasValidCredentials()) {
    return false;
  }

  // Read SSID
  uint16_t i = 0;
  for (i = 0; i < ssid_max - 1 && i < SSID_SIZE; i++) {
    char c = EEPROM.read(SSID_OFFSET + i);
    if (c == 0)
      break;
    ssid[i] = c;
  }
  ssid[i] = 0;

  // Read Password
  for (i = 0; i < pass_max - 1 && i < PASS_SIZE; i++) {
    char c = EEPROM.read(PASS_OFFSET + i);
    if (c == 0)
      break;
    password[i] = c;
  }
  password[i] = 0;

  Logger::printf("CredMgr: loaded WiFi='%s'", ssid);
  return true;
}

inline bool saveCredentials(const char* ssid, const char* password) {
  if (!ssid || strlen(ssid) == 0) {
    Logger::println("CredMgr: ERROR - SSID cannot be empty");
    return false;
  }

  if (strlen(ssid) > SSID_SIZE - 1) {
    Logger::printf("CredMgr: ERROR - SSID too long (%zu > %u)", strlen(ssid), SSID_SIZE - 1);
    return false;
  }

  if (strlen(password) > PASS_SIZE - 1) {
    Logger::printf("CredMgr: ERROR - Password too long (%zu > %u)", strlen(password),
                   PASS_SIZE - 1);
    return false;
  }

  // Write magic number
  EEPROM.write(MAGIC_OFFSET, MAGIC_NUMBER);

  // Write SSID
  uint16_t i = 0;
  for (i = 0; i < SSID_SIZE && ssid[i] != 0; i++) {
    EEPROM.write(SSID_OFFSET + i, ssid[i]);
  }
  EEPROM.write(SSID_OFFSET + i, 0);  // null terminator

  // Write Password
  for (i = 0; i < PASS_SIZE && password[i] != 0; i++) {
    EEPROM.write(PASS_OFFSET + i, password[i]);
  }
  EEPROM.write(PASS_OFFSET + i, 0);  // null terminator

  EEPROM.commit();

  Logger::printf("CredMgr: saved WiFi='%s' to EEPROM", ssid);
  return true;
}

inline void clearCredentials() {
  EEPROM.write(MAGIC_OFFSET, 0xFF);  // Invalid magic
  EEPROM.commit();
  Logger::println("CredMgr: cleared EEPROM credentials");
}

}  // namespace CredentialsManager
