#pragma once

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

#include "Config.h"
#include "Logger.h"
#include "UpdateManager.h"

namespace UpdateChecker {
// Simple version comparison: returns true if remote > local
// Versions are "major.minor.patch" format
inline bool isNewerVersion(const char* remoteVersionStr, const char* localVersionStr) {
  int rMajor = 0, rMinor = 0, rPatch = 0;
  int lMajor = 0, lMinor = 0, lPatch = 0;

  sscanf(remoteVersionStr, "%d.%d.%d", &rMajor, &rMinor, &rPatch);
  sscanf(localVersionStr, "%d.%d.%d", &lMajor, &lMinor, &lPatch);

  if (rMajor != lMajor) return rMajor > lMajor;
  if (rMinor != lMinor) return rMinor > lMinor;
  return rPatch > lPatch;
}

struct ManifestEntry {
  char version[16];
  char board[32];
  char download_url[256];
  char sha256[65];
};

static constexpr uint32_t CHECK_INTERVAL_MS = 6 * 60 * 60 * 1000;  // 6 hours
static constexpr uint32_t HTTP_TIMEOUT_MS = 10000;                  // 10 seconds
static constexpr const char* MANIFEST_URL =
    "https://milesburton.github.io/esp-oled-screensaver/manifest.json";

// Will be set by caller; indicates if update is available
static bool updateAvailable = false;
static char availableVersionStr[16] = {0};
static char downloadUrlStr[256] = {0};
static uint32_t lastCheckMs = 0;

inline void reset() {
  updateAvailable = false;
  availableVersionStr[0] = 0;
  downloadUrlStr[0] = 0;
  lastCheckMs = 0;
}

inline bool shouldCheck(uint32_t nowMs) {
  if (!UpdateManager::isAutoUpdateEnabled()) {
    return false;
  }
  if (lastCheckMs == 0) {
    return true;  // First check
  }
  return (nowMs - lastCheckMs >= CHECK_INTERVAL_MS);
}

// Minimal JSON parser for manifest: expects board-specific entry only
// Format: {"fw_version":"1.0.45","board":"d1_mini","download_url":"...","sha256":"..."}
inline bool parseManifest(const String& json, ManifestEntry& entry) {
  // Simple string-based extraction (no JSON library to save RAM)
  int pos = 0;

  // Extract fw_version
  pos = json.indexOf("\"fw_version\":\"");
  if (pos < 0) return false;
  pos += 14;
  int end = json.indexOf("\"", pos);
  if (end < 0 || end - pos >= 16) return false;
  json.substring(pos, end).toCharArray(entry.version, 16);

  // Extract board
  pos = json.indexOf("\"board\":\"");
  if (pos < 0) return false;
  pos += 9;
  end = json.indexOf("\"", pos);
  if (end < 0 || end - pos >= 32) return false;
  json.substring(pos, end).toCharArray(entry.board, 32);

  // Extract download_url
  pos = json.indexOf("\"download_url\":\"");
  if (pos < 0) return false;
  pos += 16;
  end = json.indexOf("\"", pos);
  if (end < 0 || end - pos >= 256) return false;
  json.substring(pos, end).toCharArray(entry.download_url, 256);

  // Extract sha256
  pos = json.indexOf("\"sha256\":\"");
  if (pos < 0) return false;
  pos += 10;
  end = json.indexOf("\"", pos);
  if (end < 0 || end - pos >= 65) return false;
  json.substring(pos, end).toCharArray(entry.sha256, 65);

  return true;
}

// Check for updates: fetch manifest, compare version, set flag
inline void checkForUpdates() {
  uint32_t now = millis();

  if (!shouldCheck(now)) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Logger::println("UpdateChecker: WiFi not connected, skipping check");
    lastCheckMs = now;  // Don't retry immediately
    return;
  }

  Logger::printf("UpdateChecker: checking for updates (interval=6h)...");

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);

  if (!http.begin(MANIFEST_URL)) {
    Logger::println("UpdateChecker: ERROR - failed to begin HTTP request");
    lastCheckMs = now;
    return;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Logger::printf("UpdateChecker: HTTP error %d", httpCode);
    http.end();
    lastCheckMs = now;
    return;
  }

  String payload = http.getString();
  http.end();

  if (payload.length() == 0) {
    Logger::println("UpdateChecker: ERROR - empty response");
    lastCheckMs = now;
    return;
  }

  ManifestEntry entry;
  if (!parseManifest(payload, entry)) {
    Logger::println("UpdateChecker: ERROR - failed to parse manifest JSON");
    lastCheckMs = now;
    return;
  }

  // Check if this is for our board
  const char* expectedBoard = "d1_mini";  // TODO: derive from Config
  if (strcmp(entry.board, expectedBoard) != 0) {
    Logger::printf("UpdateChecker: board mismatch (expected %s, got %s)", expectedBoard,
                   entry.board);
    lastCheckMs = now;
    return;
  }

  // Check version
  if (isNewerVersion(entry.version, Config::FW_VERSION)) {
    updateAvailable = true;
    strncpy(availableVersionStr, entry.version, sizeof(availableVersionStr) - 1);
    strncpy(downloadUrlStr, entry.download_url, sizeof(downloadUrlStr) - 1);
    Logger::printf("UpdateChecker: UPDATE AVAILABLE v%s (current v%s)", entry.version,
                   Config::FW_VERSION);
  } else {
    updateAvailable = false;
    Logger::printf("UpdateChecker: already on latest v%s", Config::FW_VERSION);
  }

  lastCheckMs = now;
}

// Force a manual check regardless of interval (for /force-check route)
inline void forceCheck() {
  if (WiFi.status() != WL_CONNECTED) {
    Logger::println("UpdateChecker: WiFi not connected, cannot check");
    return;
  }

  Logger::println("UpdateChecker: FORCED CHECK triggered manually");
  lastCheckMs = 0;  // Reset interval timer
  checkForUpdates();
}

inline bool isUpdateAvailable() { return updateAvailable; }

inline const char* getAvailableVersion() { return availableVersionStr; }

inline const char* getDownloadUrl() { return downloadUrlStr; }

inline void printStatus() {
  Logger::printf("UpdateChecker: available=%s version=%s", updateAvailable ? "YES" : "NO",
                 availableVersionStr[0] ? availableVersionStr : "(none)");
}

}  // namespace UpdateChecker
