#pragma once

#include <Arduino.h>

#include <WiFiClient.h>

class Logger {
 private:
  static WiFiClient* telnetClient;

 public:
  static void setTelnetClient(WiFiClient* client) { telnetClient = client; }

  static void println(const String& message) {
    Serial.println(message);
    if (telnetClient && telnetClient->connected()) {
      telnetClient->println(message);
    }
  }

  static void printf(const char* fmt, ...) {
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
    println(String(buf));
  }
};

// Static member definition
WiFiClient* Logger::telnetClient = nullptr;
