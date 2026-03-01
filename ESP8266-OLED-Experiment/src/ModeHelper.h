#pragma once

#include "BoingMode.h"
#include "DisplayManager.h"
#include "StatusMode.h"
#include "WeatherMode.h"

// Switch to a named mode. Returns true if the name was recognised and the mode
// pointer was non-null.
inline bool setModeByName(DisplayManager* displayManager, const String& name,
                          StatusMode* statusMode, BoingMode* boingMode, WeatherMode* weatherMode) {
  if (name == "status" && statusMode) {
    displayManager->setMode(statusMode, 400);  // 2.5 FPS
    return true;
  } else if (name == "boing" && boingMode) {
    displayManager->setMode(boingMode, 40);  // 25 FPS
    return true;
  } else if (name == "weather" && weatherMode) {
    displayManager->setMode(weatherMode, 5000);  // Every 5 seconds
    return true;
  }
  return false;
}
