#pragma once

#include "BoingMode.h"
#include "BreakoutMode.h"
#include "ClockMode.h"
#include "DisplayManager.h"
#include "LifeMode.h"
#include "MatrixRainMode.h"
#include "PacManMode.h"
#include "PlasmaMode.h"
#include "PongMode.h"
#include "ScreensaverMode.h"
#include "StarfieldMode.h"
#include "StatusMode.h"
#include "TunnelMode.h"
#include "WeatherMode.h"

inline bool setModeByName(DisplayManager* displayManager, const String& name,
                          StatusMode* statusMode, BoingMode* boingMode, WeatherMode* weatherMode,
                          ClockMode* clockMode, BreakoutMode* breakoutMode, PacManMode* pacManMode,
                          ScreensaverMode* screensaverMode, StarfieldMode* starfieldMode,
                          LifeMode* lifeMode, MatrixRainMode* matrixMode, PlasmaMode* plasmaMode,
                          TunnelMode* tunnelMode, PongMode* pongMode) {
  if (name == "status" && statusMode) {
    displayManager->setMode(statusMode, 400);
    return true;
  } else if (name == "boing" && boingMode) {
    displayManager->setMode(boingMode, 40);
    return true;
  } else if (name == "weather" && weatherMode) {
    displayManager->setMode(weatherMode, 5000);
    return true;
  } else if (name == "clock" && clockMode) {
    displayManager->setMode(clockMode, 500);
    return true;
  } else if (name == "breakout" && breakoutMode) {
    displayManager->setMode(breakoutMode, 40);
    return true;
  } else if (name == "pacman" && pacManMode) {
    displayManager->setMode(pacManMode, 40);
    return true;
  } else if (name == "screensaver" && screensaverMode) {
    displayManager->setMode(screensaverMode, 40);
    return true;
  } else if (name == "starfield" && starfieldMode) {
    displayManager->setMode(starfieldMode, 40);
    return true;
  } else if (name == "life" && lifeMode) {
    displayManager->setMode(lifeMode, 100);
    return true;
  } else if (name == "matrix" && matrixMode) {
    displayManager->setMode(matrixMode, 40);
    return true;
  } else if (name == "plasma" && plasmaMode) {
    displayManager->setMode(plasmaMode, 40);
    return true;
  } else if (name == "tunnel" && tunnelMode) {
    displayManager->setMode(tunnelMode, 40);
    return true;
  } else if (name == "pong" && pongMode) {
    displayManager->setMode(pongMode, 40);
    return true;
  }
  return false;
}
