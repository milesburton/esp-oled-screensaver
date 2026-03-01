#pragma once

#include <Arduino.h>
#include <WiFiClient.h>

class Logger {
private:
  static WiFiClient* telnetClient;
  
public:
  static void setTelnetClient(WiFiClient* client) {
    telnetClient = client;
  }
  
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
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    println(String(buf));
  }
};

// Static member definition
WiFiClient* Logger::telnetClient = nullptr;
