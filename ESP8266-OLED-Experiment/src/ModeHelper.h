#pragma once

#include "BoingMode.h"
#include "BreakoutMode.h"
#include "ClockMode.h"
#include "DisplayManager.h"
#include "PacManMode.h"
#include "StatusMode.h"
#include "WeatherMode.h"

// Switch to a named mode. Returns true if the name was recognised and the mode
// pointer was non-null.
inline bool setModeByName(DisplayManager* displayManager, const String& name,
                          StatusMode* statusMode, BoingMode* boingMode, WeatherMode* weatherMode,
                          ClockMode* clockMode, BreakoutMode* breakoutMode,
                          PacManMode* pacManMode) {
  if (name == "status" && statusMode) {
    displayManager->setMode(statusMode, 400);  // 2.5 FPS
    return true;
  } else if (name == "boing" && boingMode) {
    displayManager->setMode(boingMode, 40);  // 25 FPS
    return true;
  } else if (name == "weather" && weatherMode) {
    displayManager->setMode(weatherMode, 5000);  // 0.2 FPS, fetch-driven
    return true;
  } else if (name == "clock" && clockMode) {
    displayManager->setMode(clockMode, 500);  // 2 FPS
    return true;
  } else if (name == "breakout" && breakoutMode) {
    displayManager->setMode(breakoutMode, 40);  // 25 FPS
    return true;
  } else if (name == "pacman" && pacManMode) {
    displayManager->setMode(pacManMode, 40);  // 25 FPS
    return true;
  }
  return false;
}
