#pragma once

#include <Arduino.h>

#ifdef OTA_SUPPORTED
#include <ESP8266HTTPClient.h>

#include <Updater.h>
#include <WiFiClientSecure.h>
#endif

#include "Config.h"
#include "Logger.h"
#include "UpdateChecker.h"
#include "UpdateManager.h"

namespace AutoUpdater {

#ifndef OTA_SUPPORTED

inline void attemptAutoUpdate() {}
inline void printStatus() {}

#else

enum class UpdateState {
  IDLE,
  DOWNLOADING,
  VALIDATING,
  FLASHING,
  VERIFYING,
  SUCCESS,
  ERROR_DOWNLOAD,
  ERROR_UNPACK,
  ERROR_FLASH,
  ERROR_ABORT,
};

static UpdateState currentState = UpdateState::IDLE;
static uint32_t lastUpdateAttemptMs = 0;
static uint32_t updateStartMs = 0;
static uint32_t bytesDownloaded = 0;
static uint32_t totalBytesExpected = 0;
static uint8_t updateProgress = 0;

static constexpr uint32_t AUTO_UPDATE_RETRY_INTERVAL_MS = 60 * 60 * 1000;
static constexpr uint32_t HTTP_TIMEOUT_MS = 30000;

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

inline UpdateState getState() {
  return currentState;
}

inline uint8_t getProgress() {
  return updateProgress;
}

inline void reset() {
  currentState = UpdateState::IDLE;
  bytesDownloaded = 0;
  totalBytesExpected = 0;
  updateProgress = 0;
  lastUpdateAttemptMs = millis();
}

inline void performUpdate();

inline void attemptAutoUpdate() {
  uint32_t now = millis();

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
    return;
  }

  if (currentState == UpdateState::SUCCESS) {
    Logger::println("AutoUpdater: update completed, rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  if ((currentState == UpdateState::ERROR_DOWNLOAD || currentState == UpdateState::ERROR_FLASH) &&
      (now - lastUpdateAttemptMs < AUTO_UPDATE_RETRY_INTERVAL_MS)) {
    return;
  }

  if (currentState == UpdateState::IDLE) {
    Logger::printf("AutoUpdater: starting auto-update from v%s to v%s", Config::FW_VERSION,
                   UpdateChecker::getAvailableVersion());
    currentState = UpdateState::DOWNLOADING;
    updateStartMs = now;
    lastUpdateAttemptMs = now;
    performUpdate();
  }
}

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
  WiFiClientSecure wifiClient;
  wifiClient.setInsecure();

  if (!http.begin(wifiClient, downloadUrl)) {
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

  if (totalBytesExpected > (ESP.getFreeSketchSpace() - 0x1000)) {
    Logger::println("AutoUpdater: ERROR - insufficient space for update");
    http.end();
    currentState = UpdateState::ERROR_FLASH;
    return;
  }

  if (!Update.begin(totalBytesExpected)) {
    Logger::printf("AutoUpdater: Update.begin() failed, error: %u", Update.getError());
    http.end();
    currentState = UpdateState::ERROR_FLASH;
    return;
  }

  bytesDownloaded = 0;
  WiFiClient* stream = http.getStreamPtr();
  uint8_t buffer[512];

  currentState = UpdateState::FLASHING;

  while (http.connected() && bytesDownloaded < totalBytesExpected) {
    if (stream->available()) {
      size_t bytesRead = stream->readBytes(buffer, sizeof(buffer));
      if (bytesRead > 0) {
        if (Update.write(buffer, bytesRead) != bytesRead) {
          Logger::printf("AutoUpdater: Update.write() failed, error: %u", Update.getError());
          http.end();
          Update.end();
          currentState = UpdateState::ERROR_FLASH;
          return;
        }
        bytesDownloaded += bytesRead;
        updateProgress = (bytesDownloaded * 100) / totalBytesExpected;
        Logger::printf("AutoUpdater: progress %u%% (%u / %u bytes)", updateProgress,
                       bytesDownloaded, totalBytesExpected);
        yield();
      }
    } else {
      yield();
    }

    if (millis() - updateStartMs > (HTTP_TIMEOUT_MS + 10000)) {
      Logger::println("AutoUpdater: ERROR - download timeout");
      http.end();
      Update.end();
      currentState = UpdateState::ERROR_DOWNLOAD;
      return;
    }
  }

  http.end();

  if (bytesDownloaded != totalBytesExpected) {
    Logger::printf("AutoUpdater: ERROR - size mismatch (got %u, expected %u)", bytesDownloaded,
                   totalBytesExpected);
    Update.end();
    currentState = UpdateState::ERROR_DOWNLOAD;
    return;
  }

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

#endif  // OTA_SUPPORTED

}  // namespace AutoUpdater
