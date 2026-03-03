#include <Arduino.h>

#include <AUnit.h>

#include "../../src/UpdateChecker.h"

// Test version comparison logic
test(UpdateCheckerTest, VersionComparisonMajor) {
  assertEqual(UpdateChecker::isNewerVersion("2.0.0", "1.9.9"), true);
}

test(UpdateCheckerTest, VersionComparisonMinor) {
  assertEqual(UpdateChecker::isNewerVersion("1.1.0", "1.0.9"), true);
}

test(UpdateCheckerTest, VersionComparisonPatch) {
  assertEqual(UpdateChecker::isNewerVersion("1.0.5", "1.0.4"), true);
}

test(UpdateCheckerTest, VersionComparisonEqual) {
  assertEqual(UpdateChecker::isNewerVersion("1.0.0", "1.0.0"), false);
}

test(UpdateCheckerTest, VersionComparisonOlder) {
  assertEqual(UpdateChecker::isNewerVersion("1.0.0", "1.0.1"), false);
}

test(UpdateCheckerTest, VersionComparisonMajorOlder) {
  assertEqual(UpdateChecker::isNewerVersion("0.9.9", "1.0.0"), false);
}

// Test GitHub JSON parsing
test(UpdateCheckerTest, ParseGitHubReleaseValid) {
  UpdateChecker::ManifestEntry entry;

  // Simplified GitHub API response snippet
  String json =
      R"({"tag_name":"v1.0.45","browser_download_url":"https://github.com/user/repo/releases/download/v1.0.45/firmware.bin"})";

  bool result = UpdateChecker::parseGitHubRelease(json, entry);

  assertEqual(result, true);
  assertEqual(String(entry.version), "1.0.45");
  assertEqual(String(entry.board), "d1_mini");
  assertTrue(String(entry.download_url).startsWith("https://"));
}

test(UpdateCheckerTest, ParseGitHubReleaseMissingTagName) {
  UpdateChecker::ManifestEntry entry;
  String json = R"({"browser_download_url":"https://example.com/firmware.bin"})";

  bool result = UpdateChecker::parseGitHubRelease(json, entry);

  assertEqual(result, false);
}

test(UpdateCheckerTest, ParseGitHubReleaseMissingDownloadUrl) {
  UpdateChecker::ManifestEntry entry;
  String json = R"({"tag_name":"v1.0.45"})";

  bool result = UpdateChecker::parseGitHubRelease(json, entry);

  assertEqual(result, false);
}

test(UpdateCheckerTest, ParseGitHubReleaseVersionWithoutPrefix) {
  UpdateChecker::ManifestEntry entry;

  // Some releases may not have 'v' prefix
  String json =
      R"({"tag_name":"1.0.45","browser_download_url":"https://github.com/user/repo/releases/download/1.0.45/firmware.bin"})";

  bool result = UpdateChecker::parseGitHubRelease(json, entry);

  assertEqual(result, true);
  assertEqual(String(entry.version), "1.0.45");
}

void setup() {
#ifdef EPOXY_DUINO
  Serial.begin(115200);
  while (!Serial)
    delay(100);
#endif
  delay(1000);
}

void loop() {
  aunit::TestRunner::run();
}
