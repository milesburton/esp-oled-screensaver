#include <Arduino.h>

#include <AUnit.h>

#include "../../src/UpdateChecker.h"

// Version comparison — basic ordering
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

// Version comparison — local version has "platform-" prefix as shipped on device
test(UpdateCheckerTest, VersionComparisonPlatformPrefix) {
  assertEqual(UpdateChecker::isNewerVersion("1.0.53", "platform-1.0.51"), true);
}

test(UpdateCheckerTest, VersionComparisonPlatformPrefixEqual) {
  assertEqual(UpdateChecker::isNewerVersion("1.0.53", "platform-1.0.53"), false);
}

test(UpdateCheckerTest, VersionComparisonBothVPrefix) {
  assertEqual(UpdateChecker::isNewerVersion("v1.0.5", "v1.0.4"), true);
}

// JSON parsing — fixture mirrors the real GitHub API response format:
// pretty-printed with newlines and "key": "value" (space after colon).
// tag_name appears after several other fields; browser_download_url is
// deeply nested inside the assets array.
static const char REAL_FORMAT_JSON[] =
    "{\n"
    "  \"url\": \"https://api.github.com/repos/owner/repo/releases/1\",\n"
    "  \"assets_url\": \"https://api.github.com/repos/owner/repo/releases/1/assets\",\n"
    "  \"tag_name\": \"v1.0.45\",\n"
    "  \"name\": \"Firmware v1.0.45\",\n"
    "  \"draft\": false,\n"
    "  \"prerelease\": false,\n"
    "  \"assets\": [{\n"
    "    \"name\": \"firmware_esp8266_d1_mini.bin\",\n"
    "    \"browser_download_url\": "
    "\"https://github.com/owner/repo/releases/download/v1.0.45/firmware.bin\"\n"
    "  }]\n"
    "}";

test(UpdateCheckerTest, ParseRealFormatJson) {
  UpdateChecker::ManifestEntry entry;
  bool result = UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  assertEqual(result, true);
  assertEqual(String(entry.version), String("1.0.45"));
  assertTrue(String(entry.download_url).startsWith("https://"));
}

test(UpdateCheckerTest, ParseRealFormatJsonStripsVPrefix) {
  UpdateChecker::ManifestEntry entry;
  UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  assertFalse(String(entry.version).startsWith("v"));
  assertEqual(String(entry.version), String("1.0.45"));
}

test(UpdateCheckerTest, ParseRealFormatJsonDownloadUrl) {
  UpdateChecker::ManifestEntry entry;
  UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  assertTrue(String(entry.download_url).endsWith(".bin"));
}

test(UpdateCheckerTest, ParseRealFormatJsonBoardIsSet) {
  UpdateChecker::ManifestEntry entry;
  UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  assertEqual(String(entry.board), String("d1_mini"));
}

// JSON parsing — version without v prefix
test(UpdateCheckerTest, ParseVersionWithoutVPrefix) {
  UpdateChecker::ManifestEntry entry;
  String json =
      "{\n"
      "  \"tag_name\": \"1.0.45\",\n"
      "  \"assets\": [{\n"
      "    \"browser_download_url\": \"https://example.com/firmware.bin\"\n"
      "  }]\n"
      "}";

  bool result = UpdateChecker::parseGitHubRelease(json, entry);

  assertEqual(result, true);
  assertEqual(String(entry.version), String("1.0.45"));
}

// JSON parsing — missing fields
test(UpdateCheckerTest, ParseMissingTagName) {
  UpdateChecker::ManifestEntry entry;
  String json =
      "{\n"
      "  \"name\": \"Firmware v1.0.45\",\n"
      "  \"assets\": [{\n"
      "    \"browser_download_url\": \"https://example.com/firmware.bin\"\n"
      "  }]\n"
      "}";

  assertEqual(UpdateChecker::parseGitHubRelease(json, entry), false);
}

test(UpdateCheckerTest, ParseMissingDownloadUrl) {
  UpdateChecker::ManifestEntry entry;
  String json =
      "{\n"
      "  \"tag_name\": \"v1.0.45\",\n"
      "  \"assets\": []\n"
      "}";

  assertEqual(UpdateChecker::parseGitHubRelease(json, entry), false);
}

// End-to-end: parse then compare against a platform-prefixed local version
test(UpdateCheckerTest, ParsedVersionNewerThanPlatformLocal) {
  UpdateChecker::ManifestEntry entry;
  UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  // entry.version = "1.0.45", local = "platform-1.0.44"
  assertEqual(UpdateChecker::isNewerVersion(entry.version, "platform-1.0.44"), true);
}

test(UpdateCheckerTest, ParsedVersionNotNewerThanPlatformLocal) {
  UpdateChecker::ManifestEntry entry;
  UpdateChecker::parseGitHubRelease(String(REAL_FORMAT_JSON), entry);

  // entry.version = "1.0.45", local = "platform-1.0.46"
  assertEqual(UpdateChecker::isNewerVersion(entry.version, "platform-1.0.46"), false);
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
