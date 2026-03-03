#pragma once

#include <EEPROM.h>

#include "Logger.h"

namespace UpdateManager {
// EEPROM layout for auto-update preferences:
// Byte 128: Magic number (0xBB indicates valid auto-update settings)
// Byte 129: Schema version (1)
// Byte 130: Auto-update enabled flag (1=true/enabled, 0=false/disabled)
// Byte 131: Update channel (0=stable, 1=beta)
// Bytes 132-191: Reserved for future use (60 bytes for phase 2/3/4 expansion)

static constexpr uint16_t EEPROM_SIZE = 512;
static constexpr uint8_t MAGIC_NUMBER = 0xBB;
static constexpr uint8_t SCHEMA_VERSION = 1;
static constexpr uint16_t MAGIC_OFFSET = 128;
static constexpr uint16_t SCHEMA_OFFSET = 129;
static constexpr uint16_t ENABLED_OFFSET = 130;
static constexpr uint16_t CHANNEL_OFFSET = 131;

// Update channel constants
static constexpr uint8_t CHANNEL_STABLE = 0;
static constexpr uint8_t CHANNEL_BETA = 1;

// Default settings: auto-update OFF, stable channel
static constexpr bool DEFAULT_AUTO_UPDATE_ENABLED = false;
static constexpr uint8_t DEFAULT_CHANNEL = CHANNEL_STABLE;

inline void initialize() {
  // EEPROM already initialized by CredentialsManager, but safe to call multiple times
  EEPROM.begin(EEPROM_SIZE);
}

inline bool hasValidSettings() {
  return EEPROM.read(MAGIC_OFFSET) == MAGIC_NUMBER && 
         EEPROM.read(SCHEMA_OFFSET) == SCHEMA_VERSION;
}

inline bool isAutoUpdateEnabled() {
  if (!hasValidSettings()) {
    return DEFAULT_AUTO_UPDATE_ENABLED;
  }
  return EEPROM.read(ENABLED_OFFSET) != 0;
}

inline uint8_t getUpdateChannel() {
  if (!hasValidSettings()) {
    return DEFAULT_CHANNEL;
  }
  uint8_t ch = EEPROM.read(CHANNEL_OFFSET);
  return (ch <= CHANNEL_BETA) ? ch : DEFAULT_CHANNEL;
}

inline bool setAutoUpdateEnabled(bool enabled) {
  // Initialize valid settings block if not present
  if (!hasValidSettings()) {
    EEPROM.write(MAGIC_OFFSET, MAGIC_NUMBER);
    EEPROM.write(SCHEMA_OFFSET, SCHEMA_VERSION);
  }

  EEPROM.write(ENABLED_OFFSET, enabled ? 1 : 0);
  EEPROM.commit();

  Logger::printf("UpdateMgr: auto-update %s", enabled ? "ENABLED" : "DISABLED");
  return true;
}

inline bool setUpdateChannel(uint8_t channel) {
  if (channel > CHANNEL_BETA) {
    Logger::printf("UpdateMgr: ERROR - invalid channel %u", channel);
    return false;
  }

  // Initialize valid settings block if not present
  if (!hasValidSettings()) {
    EEPROM.write(MAGIC_OFFSET, MAGIC_NUMBER);
    EEPROM.write(SCHEMA_OFFSET, SCHEMA_VERSION);
  }

  EEPROM.write(CHANNEL_OFFSET, channel);
  EEPROM.commit();

  Logger::printf("UpdateMgr: channel set to %s", channel == CHANNEL_STABLE ? "stable" : "beta");
  return true;
}

inline void clearSettings() {
  // Mark block as invalid by clearing magic
  EEPROM.write(MAGIC_OFFSET, 0xFF);
  EEPROM.commit();
  Logger::println("UpdateMgr: cleared EEPROM settings");
}

inline void printSettings() {
  Logger::printf("UpdateMgr: valid=%d, enabled=%d, channel=%u",
                 hasValidSettings(),
                 isAutoUpdateEnabled(),
                 getUpdateChannel());
}

}  // namespace UpdateManager
