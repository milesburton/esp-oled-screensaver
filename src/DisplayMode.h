#pragma once

#include <U8g2lib.h>

// Abstract base class for display modes
class DisplayMode {
public:
  virtual ~DisplayMode() {}
  
  // Called once when mode is activated
  virtual void begin() {}
  
  // Called every frame to update and render
  virtual void update(U8G2* display, uint32_t deltaMs) = 0;
  
  // Get the name of this mode
  virtual const char* getName() const = 0;
  
  // Optional: called when mode is deactivated
  virtual void end() {}
};
