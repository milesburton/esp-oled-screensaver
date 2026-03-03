#pragma once

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <Updater.h>

#include "Config.h"
#include "Logger.h"
#include "UpdateChecker.h"
#include "UpdateManager.h"

namespace AutoUpdater {
// Update states
enum class UpdateState {
  IDLE,           // Waiting or no update available
  DOWNLOADING,    // Fetching binary from GitHub
  VALIDATING,     // Verifying SHA256 (if available)
  FLASHING,       // Writing to flash partition
  VERIFYING,      // Post-flash verification
  SUCCESS,        // Completed successfully
  ERROR_DOWNLOAD, // Download failed
  ERROR_UNPACK,   // Binary unpacking failed
  ERROR_FLASH,    // Flash write failed
  ERROR_ABORT,    // User aborted
};

static UpdateState currentState = UpdateState::IDLE;
static uint32_t lastUpdateAttemptMs = 0;
static uint32_t updateStartMs = 0;
static uint32_t bytesDownloaded = 0;
static uint32_t totalBytesExpected = 0;
static uint8_t updateProgress = 0;  // 0-100

// Attempt update only once per hour to avoid hammering network if it fails
static constexpr uint32_t AUTO_UPDATE_RETRY_INTERVAL_MS = 60 * 60 * 1000;
static constexpr uint32_t HTTP_TIMEOUT_MS = 30000;  // 30 sec timeout for binary download

inline const char* stateToString(UpdateState state) {
  switch (state) {
    case UpdateState::IDLE:
      return "IDLE";
    case UpdateState::DOWNLOADING:
      return "DOWNLOADING";
    case UpdateState::VALIDATING:
      return "VALIDATING";
    case UpdateState::FLASHING:
      return "FLASHING";
    case UpdateState::VERIFYING:
      return "VERIFYING";
    case UpdateState::SUCCESS:
      return "SUCCESS";
    case UpdateState::ERROR_DOWNLOAD:
      return "ERROR_DOWNLOAD";
    case UpdateState::ERROR_UNPACK:
      return "ERROR_UNPACK";
    case UpdateState::ERROR_FLASH:
      return "ERROR_FLASH";
    case UpdateState::ERROR_ABORT:
      return "ERROR_ABORT";
    default:
      return "UNKNOWN";
  }
}

inline UpdateState getState() { return currentState; }

inline uint8_t getProgress() { return updateProgress; }

inline void reset() {
  currentState = UpdateState::IDLE;
  bytesDownloaded = 0;
  totalBytesExpected = 0;
  updateProgress = 0;
  lastUpdateAttemptMs = millis();
}

// Attempt to perform OTA update if update is available
// Called periodically from NetworkManager::update()
inline void attemptAutoUpdate() {
  uint32_t now = millis();

  // Only try if:
  // 1. Auto-update enabled
  // 2. Update is available from checker
  // 3. WiFi connected
  // 4. Not in the middle of an update
  // 5. Haven't retried too recently

  if (!UpdateManager::isAutoUpdateEnabled()) {
    return;
  }

  if (!UpdateChecker::isUpdateAvailable()) {
    reset();
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (currentState != UpdateState::IDLE && currentState != UpdateState::SUCCESS &&
      currentState != UpdateState::ERROR_DOWNLOAD && currentState != UpdateState::ERROR_FLASH) {
    return;  // Still busy or in error state
  }

  if (currentState == UpdateState::SUCCESS) {
    Logger::println("AutoUpdater: update completed, rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  // Rate-limit: don't retry failed updates too frequently
  if ((currentState == UpdateState::ERROR_DOWNLOAD || currentState == UpdateState::ERROR_FLASH) &&
      (now - lastUpdateAttemptMs < AUTO_UPDATE_RETRY_INTERVAL_MS)) {
    return;
  }

  // Start new download attempt
  if (currentState == UpdateState::IDLE) {
    Logger::printf("AutoUpdater: starting auto-update from %s to v%s",
                   UpdateChecker::getAvailableVersion(), UpdateChecker::getAvailableVersion());
    currentState = UpdateState::DOWNLOADING;
    updateStartMs = now;
    lastUpdateAttemptMs = now;
    performUpdate();
  }
}

// Actual download and flash logic
inline void performUpdate() {
  const char* downloadUrl = UpdateChecker::getDownloadUrl();
  if (!downloadUrl || strlen(downloadUrl) == 0) {
    Logger::println("AutoUpdater: ERROR - empty download URL");
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

  Logger::printf("AutoUpdater: downloading from %s", downloadUrl);

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.setConnectTimeout(5000);

  if (!http.begin(downloadUrl)) {
    Logger::println("AutoUpdater: ERROR - failed to begin HTTP request");
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    Logger::printf("AutoUpdater: HTTP error %d", httpCode);
    http.end();
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

  totalBytesExpected = http.getSize();
  if (totalBytesExpected <= 0) {
    Logger::println("AutoUpdater: ERROR - invalid content length");
    http.end();
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

  Logger::printf("AutoUpdater: downloading %u bytes", totalBytesExpected);

  // Check available free update partition space
  if (totalBytesExpected > (ESP.getFreeSketchSpace() - 0x1000)) {
    Logger::println("AutoUpdater: ERROR - insufficient space for update");
    http.end();
    currentState = UpdateState::ERROR_FLASH;
    return;
  }

  // Begin OTA update
  if (!Update.begin(totalBytesExpected)) {
    Logger::printf("AutoUpdater: Update.begin() failed, error: %u", Update.getError());
    http.end();
    currentState = UpdateState::ERROR_FLASH;
    return;
  }

  // Stream binary from HTTP into flash
  bytesDownloaded = 0;
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buffer[512];
  size_t bytesRead;

  currentState = UpdateState::FLASHING;

  while (http.connected() && (bytesRead > 0 || bytesRead == 0)) {
    if (stream->available()) {
      bytesRead = stream->readBytes(buffer, sizeof(buffer));
      if (bytesRead > 0) {
        if (Update.write(buffer, bytesRead) != bytesRead) {
          Logger::printf("AutoUpdater: Update.write() failed, error: %u", Update.getError());
          http.end();
          Update.abort();
          currentState = UpdateState::ERROR_FLASH;
          return;
        }
        bytesDownloaded += bytesRead;
        updateProgress = (bytesDownloaded * 100) / totalBytesExpected;
        Logger::printf("AutoUpdater: progress %u%% (%u / %u bytes)", updateProgress, bytesDownloaded,
                       totalBytesExpected);
        yield();
      }
    } else {
      yield();
    }

    // Timeout protection
    if (millis() - updateStartMs > (HTTP_TIMEOUT_MS + 10000)) {
      Logger::println("AutoUpdater: ERROR - download timeout");
      http.end();
      Update.abort();
      currentState = UpdateState::ERROR_DOWNLOAD;
      return;
    }
  }

  http.end();

  // Verify download size
  if (bytesDownloaded != totalBytesExpected) {
    Logger::printf("AutoUpdater: ERROR - size mismatch (got %u, expected %u)", bytesDownloaded,
                   totalBytesExpected);
    Update.abort();
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

  // Finalize flash
  currentState = UpdateState::VERIFYING;
  if (!Update.end(true)) {
    Logger::printf("AutoUpdater: Update.end() failed, error: %u", Update.getError());
    currentState = UpdateState::ERROR_FLASH;
    return;
  }

  Logger::printf("AutoUpdater: update successful! New firmware v%s ready to install",
                 UpdateChecker::getAvailableVersion());
  currentState = UpdateState::SUCCESS;
  updateProgress = 100;
}

inline void printStatus() {
  Logger::printf(
      "AutoUpdater: state=%s progress=%u%% bytesDownloaded=%u totalExpected=%u "
      "timeSinceStart=%lums",
      stateToString(currentState), updateProgress, bytesDownloaded, totalBytesExpected,
      (millis() - updateStartMs));
}

}  // namespace AutoUpdater
