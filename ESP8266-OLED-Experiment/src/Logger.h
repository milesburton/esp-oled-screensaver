#pragma once

#include <Arduino.h>

#include <WiFiClient.h>

class Logger {
 public:
  enum class Level : uint8_t { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

  static Level minLevel;

  static void setTelnetClient(WiFiClient* client) { telnetClient = client; }

  static void log(Level level, const String& message) {
    if (level < minLevel)
      return;
    String line = prefix(level) + message;
    Serial.println(line);
    if (telnetClient && telnetClient->connected()) {
      telnetClient->println(line);
    }
  }

  static void logf(Level level, const char* fmt, ...) {
    if (level < minLevel)
      return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (written >= (int)sizeof(buf)) {
      buf[sizeof(buf) - 4] = '.';
      buf[sizeof(buf) - 3] = '.';
      buf[sizeof(buf) - 2] = '.';
      buf[sizeof(buf) - 1] = '\0';
    }
    log(level, String(buf));
  }

  // Convenience shims preserved for backwards compatibility — emit at INFO level
  static void println(const String& message) { log(Level::INFO, message); }

  static void printf(const char* fmt, ...) {
    if (Level::INFO < minLevel)
      return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (written >= (int)sizeof(buf)) {
      buf[sizeof(buf) - 4] = '.';
      buf[sizeof(buf) - 3] = '.';
      buf[sizeof(buf) - 2] = '.';
      buf[sizeof(buf) - 1] = '\0';
    }
    log(Level::INFO, String(buf));
  }

  static void debug(const char* fmt, ...) {
    if (Level::DEBUG < minLevel)
      return;
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(Level::DEBUG, String(buf));
  }

  static void warn(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(Level::WARN, String(buf));
  }

  static void error(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(Level::ERROR, String(buf));
  }

 private:
  static WiFiClient* telnetClient;

  static String prefix(Level level) {
    const char* tag;
    switch (level) {
      case Level::DEBUG:
        tag = "DBG";
        break;
      case Level::WARN:
        tag = "WRN";
        break;
      case Level::ERROR:
        tag = "ERR";
        break;
      default:
        tag = "INF";
        break;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "[%s] %lu ", tag, millis());
    return String(buf);
  }
};

WiFiClient* Logger::telnetClient = nullptr;
Logger::Level Logger::minLevel = Logger::Level::INFO;
