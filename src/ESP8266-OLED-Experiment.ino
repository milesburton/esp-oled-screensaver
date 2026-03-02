// Hardware: ESP8266 + 128x64 OLED (SSD1306/SH1106)
// Wiring: SDA=GPIO0, SCL=GPIO2, I2C addr=0x3C

#include "BoingMode.h"
#include "BreakoutMode.h"
#include "ClockMode.h"
#include "Config.h"
#include "DisplayManager.h"
#include "LifeMode.h"
#include "Logger.h"
#include "NetworkManager.h"
#include "PacManMode.h"
#include "ScreensaverMode.h"
#include "SonicMode.h"
#include "StarfieldMode.h"
#include "StatusMode.h"
#include "TelnetConsole.h"
#include "WeatherMode.h"

DisplayManager displayManager;
NetworkManager networkManager;
TelnetConsole telnetConsole;

StatusMode statusMode;
BoingMode boingMode;
WeatherMode weatherMode;
ClockMode clockMode;
BreakoutMode breakoutMode;
PacManMode pacManMode;
StarfieldMode starfieldMode;
LifeMode lifeMode;
SonicMode sonicMode;
ScreensaverMode screensaverMode(&clockMode, &boingMode, &weatherMode, &breakoutMode, &pacManMode,
                                &starfieldMode, &lifeMode, &sonicMode);

uint32_t lastStatusLog = 0;

void setup() {
  Serial.begin(115'200);
  delay(50);

  Logger::println("");
  Logger::printf("Boot: %s fw=%s", Config::HOSTNAME, Config::FW_VERSION);

  displayManager.begin();

  networkManager.setDisplayManager(&displayManager);
  networkManager.setModes(&statusMode, &boingMode, &weatherMode, &clockMode, &breakoutMode,
                          &pacManMode, &screensaverMode, &starfieldMode, &lifeMode, &sonicMode);

  telnetConsole.setDisplayManager(&displayManager);
  telnetConsole.setModes(&statusMode, &boingMode, &weatherMode, &clockMode, &breakoutMode,
                         &pacManMode, &screensaverMode, &starfieldMode, &lifeMode, &sonicMode);

  networkManager.begin();
  telnetConsole.begin();

  displayManager.setMode(&screensaverMode, 40);  // 25 FPS

  Logger::println("System ready!");
}

void loop() {
  networkManager.update();
  telnetConsole.update();
  displayManager.update();

  uint32_t now = millis();
  if (now - lastStatusLog > 1000) {
    lastStatusLog = now;
    networkManager.logStatus();
  }

  delay(1);
  yield();
}
