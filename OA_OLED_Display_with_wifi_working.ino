// OTAupgrade.ino
// ESP8266 platform firmware: ElegantOTA (/update) + Telnet console (23) + OLED + Boing ball demo.
// Controller unknown? Switch SSD1306/SH1106 at runtime + adjust X offset at runtime.
//
// Known wiring from probe: SDA=GPIO0, SCL=GPIO2, addr=0x3C.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ElegantOTA.h>
#include <WiFiServer.h>

#include <Wire.h>
#include <U8g2lib.h>

#include "secrets.h"

// ===== identity =====
static constexpr const char* HOSTNAME   = "esp-weather-clock";
static constexpr const char* FW_VERSION = "platform-0.5.0-boing-auto";
// ====================

// ===== secrets (local-only) =====
static constexpr const char* WIFI_SSID = secrets::WIFI_SSID;
static constexpr const char* WIFI_PASS = secrets::WIFI_PASS;
static constexpr const char* OTA_USER  = secrets::OTA_USER;
static constexpr const char* OTA_PASS  = secrets::OTA_PASS;
// ===============================

// ===== OLED wiring (from your probe) =====
static constexpr uint8_t OLED_SDA  = 0;    // GPIO0
static constexpr uint8_t OLED_SCL  = 2;    // GPIO2
static constexpr uint8_t OLED_ADDR = 0x3C; // 0x3C
// =======================================

// ===== runtime OLED config =====
enum class OledDrv : uint8_t { SSD1306, SH1106 };
static OledDrv g_drv = OledDrv::SH1106;  // default guess (common on cheap modules)
static int g_xOffset = 2;                // SH1106 often needs +2; SSD1306 usually 0
static bool g_oledEnabled = true;
// ======================================

// ===== modes =====
enum class DemoMode : uint8_t { Status, Boing };
static DemoMode g_mode = DemoMode::Boing;
// ==============

// HTTP + Telnet
ESP8266WebServer http(80);
WiFiServer telnetServer(23);
WiFiClient telnetClient;

// -------- logging --------
static void logln(const String& s) {
  Serial.println(s);
  if (telnetClient && telnetClient.connected()) telnetClient.println(s);
}
static void logf(const char* fmt, ...) {
  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  logln(String(buf));
}

// -------- telnet handling --------
static void handleTelnet() {
  if (telnetServer.hasClient()) {
    WiFiClient incoming = telnetServer.available();
    if (!telnetClient || !telnetClient.connected()) {
      telnetClient = incoming;
      telnetClient.setNoDelay(true);
      telnetClient.println();
      telnetClient.println("ESP8266 remote console connected");
      telnetClient.println("Type 'help' for commands.");
      telnetClient.println();
      logln("[telnet] client connected");
    } else {
      incoming.println("busy");
      incoming.stop();
    }
  }

  if (telnetClient && telnetClient.connected() && telnetClient.available()) {
    String cmd = telnetClient.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "help") {
      telnetClient.println("commands:");
      telnetClient.println("  help");
      telnetClient.println("  status");
      telnetClient.println("  drv ssd1306|sh1106");
      telnetClient.println("  xoff <int>         (e.g. xoff 0 or xoff 2)");
      telnetClient.println("  mode status|boing");
      telnetClient.println("  oled on|off");
      telnetClient.println("  reboot");
    } else if (cmd == "status") {
      telnetClient.printf(
        "fw=%s ip=%s rssi=%d heap=%u oled=%d drv=%s xoff=%d sda=%u scl=%u addr=0x%02X mode=%s\n",
        FW_VERSION,
        WiFi.localIP().toString().c_str(),
        (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0,
        ESP.getFreeHeap(),
        g_oledEnabled ? 1 : 0,
        (g_drv == OledDrv::SSD1306) ? "ssd1306" : "sh1106",
        g_xOffset,
        OLED_SDA, OLED_SCL, OLED_ADDR,
        (g_mode == DemoMode::Boing) ? "boing" : "status"
      );
    } else if (cmd.startsWith("drv ")) {
      if (cmd.endsWith("ssd1306")) { g_drv = OledDrv::SSD1306; }
      else if (cmd.endsWith("sh1106")) { g_drv = OledDrv::SH1106; }
      telnetClient.println("ok");
    } else if (cmd.startsWith("xoff ")) {
      String v = cmd.substring(5);
      g_xOffset = v.toInt();
      telnetClient.println("ok");
    } else if (cmd.startsWith("mode ")) {
      if (cmd.endsWith("status")) g_mode = DemoMode::Status;
      else if (cmd.endsWith("boing")) g_mode = DemoMode::Boing;
      telnetClient.println("ok");
    } else if (cmd == "oled on") {
      g_oledEnabled = true;
      telnetClient.println("ok");
    } else if (cmd == "oled off") {
      g_oledEnabled = false;
      telnetClient.println("ok");
    } else if (cmd == "reboot") {
      telnetClient.println("rebooting...");
      telnetClient.flush();
      delay(50);
      ESP.restart();
    } else if (cmd.length()) {
      telnetClient.println("unknown");
    }
  }
}

static const char* wlStatusName(wl_status_t s) {
  switch (s) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID";
    case WL_SCAN_COMPLETED:  return "SCAN_DONE";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}

// ===== OLED objects =====
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_ssd(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C  u8g2_sh (U8G2_R0, U8X8_PIN_NONE);
U8G2* u8g2 = nullptr;

static void oledSelectAndBegin() {
  if (!g_oledEnabled) return;

  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(400000);
  delay(5);

  u8g2 = (g_drv == OledDrv::SSD1306) ? (U8G2*)&u8g2_ssd : (U8G2*)&u8g2_sh;
  u8g2->setI2CAddress(OLED_ADDR << 1);
  u8g2->begin();

  logf("oled: begin drv=%s addr=0x%02X (8bit=0x%02X) xoff=%d",
       (g_drv == OledDrv::SSD1306) ? "ssd1306" : "sh1106",
       OLED_ADDR, (OLED_ADDR << 1), g_xOffset);
}

// Drawing helpers with X offset
static inline void dxPixel(int x, int y) {
  u8g2->drawPixel(x + g_xOffset, y);
}
static inline void dxHLine(int x, int y, int w) {
  u8g2->drawHLine(x + g_xOffset, y, w);
}
static inline void dxVLine(int x, int y, int h) {
  u8g2->drawVLine(x + g_xOffset, y, h);
}
static inline void dxFrame(int x, int y, int w, int h) {
  u8g2->drawFrame(x + g_xOffset, y, w, h);
}
static inline void dxCircle(int x, int y, int r) {
  u8g2->drawCircle(x + g_xOffset, y, r, U8G2_DRAW_ALL);
}
static inline void dxStr(int x, int y, const char* s) {
  u8g2->drawStr(x + g_xOffset, y, s);
}

// ===== status render =====
static void renderStatus() {
  if (!g_oledEnabled || !u8g2) return;

  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_6x12_tf);

  dxStr(0, 12, "esp-weather-clock");

  u8g2->setCursor(0 + g_xOffset, 28);
  u8g2->print("FW: ");
  u8g2->print(FW_VERSION);

  u8g2->setCursor(0 + g_xOffset, 42);
  u8g2->print("DRV: ");
  u8g2->print((g_drv == OledDrv::SSD1306) ? "SSD1306" : "SH1106");
  u8g2->print(" XOFF:");
  u8g2->print(g_xOffset);

  u8g2->setCursor(0 + g_xOffset, 56);
  u8g2->print("IP: ");
  if (WiFi.status() == WL_CONNECTED) u8g2->print(WiFi.localIP());
  else u8g2->print("(unset)");

  // border to test alignment
  dxFrame(0, 0, 128, 64);

  u8g2->sendBuffer();
}

// ===== Boing-ish demo =====
static constexpr int W = 128;
static constexpr int H = 64;

static constexpr int R0 = 12;
static constexpr int FLOOR_LINE_Y = H - 1;
static constexpr int FLOOR_Y = H - 2;
static constexpr int LEFT_X  = R0 + 2;
static constexpr int RIGHT_X = W - 1 - (R0 + 2);

static constexpr float X_SPEED = 52.0f;      // px/s
static constexpr float BOUNCE_FREQ = 0.75f;  // bounces per second
static constexpr float BOUNCE_HEIGHT = 34.0f;

static uint32_t lastFrameMs = 0;
static float xPos = 30.0f;
static int xDir = +1;
static float rotPhase = 0.0f;

static inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

static void drawShadow(int cx, int cy, int rx, int ry) {
  for (int y = -ry; y <= ry; y++) {
    float yy = (float)y / (float)ry;
    float inside = 1.0f - yy * yy;
    if (inside <= 0) continue;
    int half = (int)(rx * sqrtf(inside));
    dxHLine(cx - half, cy + y, 2 * half + 1);
    yield();
  }
}

// Procedural rotating checker on a sphere-ish mapping.
static void drawBoingBall(int cx, int cy, int rx, int ry, float rot) {
  dxCircle(cx, cy, max(rx, ry));

  int minX = cx - rx, maxX = cx + rx;
  int minY = cy - ry, maxY = cy + ry;

  for (int py = minY; py <= maxY; py++) {
    for (int px = minX; px <= maxX; px++) {
      float nx = (px - cx) / (float)rx;
      float ny = (py - cy) / (float)ry;
      float rr = nx * nx + ny * ny;
      if (rr > 1.0f) continue;

      // lon/lat-ish mapping
      float lon = atan2f(nx, sqrtf(1.0f - rr));
      float lat = asinf(clampf(ny, -1.0f, 1.0f));

      // tweak these for checker size
      int u = (int)floorf((lon + rot) * 3.0f);
      int v = (int)floorf(lat * 3.0f);

      bool on = ((u + v) & 1) == 0;

      // specular-ish stripe
      bool highlight = (ny < -0.35f && ny > -0.45f);

      if (on || highlight) dxPixel(px, py);
    }
    yield();
  }
}

static void stepAndRenderBoing() {
  if (!g_oledEnabled || !u8g2) return;

  uint32_t now = millis();
  float dt = (lastFrameMs == 0) ? 0.016f : (now - lastFrameMs) / 1000.0f;
  if (dt > 0.05f) dt = 0.05f;
  lastFrameMs = now;

  // X ping-pong
  xPos += xDir * X_SPEED * dt;
  if (xPos <= LEFT_X)  { xPos = LEFT_X;  xDir = +1; }
  if (xPos >= RIGHT_X) { xPos = RIGHT_X; xDir = -1; }

  // Bounce curve (0 at floor, 1 at apex)
  float t = now / 1000.0f;
  float phase = t * (2.0f * 3.1415926f) * BOUNCE_FREQ;
  float bounce01 = 0.5f * (1.0f - cosf(phase)); // 0..1

  float yPos = (float)FLOOR_Y - bounce01 * BOUNCE_HEIGHT;

  // squash/stretch near impact
  float squash = 1.0f - (1.0f - bounce01) * 0.35f;   // floor: 0.65
  float stretch = 1.0f + (1.0f - bounce01) * 0.20f;  // floor: 1.20

  int rx = (int)(R0 * (1.0f / squash));
  int ry = (int)(R0 * squash * stretch);

  // rotate the checker pattern continuously, slightly biased by direction
  rotPhase += (xDir * 1.8f) * dt;

  // render
  u8g2->clearBuffer();

  // Border for alignment check
  dxFrame(0, 0, 128, 64);

  // floor
  dxHLine(0, FLOOR_LINE_Y, W);

  // shadow: larger near floor
  int sx = (int)xPos;
  int shadowY = FLOOR_Y;
  int srx = (int)(rx * (1.2f + (1.0f - bounce01) * 0.6f));
  int sry = (int)(2 + (1.0f - bounce01) * 2);
  drawShadow(sx, shadowY, srx, sry);

  // ball
  drawBoingBall((int)xPos, (int)yPos, rx, ry, rotPhase);

  // tiny label
  u8g2->setFont(u8g2_font_5x7_tf);
  dxStr(0, 7, "BOING");

  u8g2->sendBuffer();
}

// ===== WiFi + HTTP =====
static void setupWifi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

static void setupHttpRoutes() {
  http.on("/", HTTP_GET, []() {
    String html;
    html.reserve(2000);

    html += "<h1>esp-weather-clock</h1>";
    html += "<p><b>FW:</b> " + String(FW_VERSION) + "</p>";
    html += "<ul>";
    html += "<li><b>WiFi:</b> " + String(wlStatusName(WiFi.status())) + "</li>";
    html += "<li><b>IP:</b> " + WiFi.localIP().toString() + "</li>";
    html += "<li><b>RSSI:</b> " + String((WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0) + "</li>";
    html += "<li><b>Heap:</b> " + String(ESP.getFreeHeap()) + "</li>";
    html += "<li><b>Mode:</b> " + String((g_mode == DemoMode::Boing) ? "boing" : "status") + "</li>";
    html += "<li><b>OLED:</b> " + String(g_oledEnabled ? "on" : "off") + "</li>";
    html += "<li><b>Driver:</b> " + String((g_drv == OledDrv::SSD1306) ? "SSD1306" : "SH1106") + "</li>";
    html += "<li><b>X offset:</b> " + String(g_xOffset) + "</li>";
    html += "<li><b>I2C:</b> SDA GPIO" + String(OLED_SDA) + " / SCL GPIO" + String(OLED_SCL) + " / addr 0x" + String(OLED_ADDR, HEX) + "</li>";
    html += "</ul>";

    html += "<p><a href='/update'>OTA update</a></p>";

    html += "<h3>Mode</h3>";
    html += "<p><a href='/mode?demo=boing'>boing</a> | <a href='/mode?demo=status'>status</a></p>";

    html += "<h3>OLED config</h3>";
    html += "<p>Try these until border + text align perfectly:</p>";
    html += "<ul>";
    html += "<li><a href='/oledcfg?drv=sh1106&xoff=2'>SH1106 xoff=2 (common)</a></li>";
    html += "<li><a href='/oledcfg?drv=sh1106&xoff=0'>SH1106 xoff=0</a></li>";
    html += "<li><a href='/oledcfg?drv=ssd1306&xoff=0'>SSD1306 xoff=0 (common)</a></li>";
    html += "<li><a href='/oledcfg?drv=ssd1306&xoff=2'>SSD1306 xoff=2</a></li>";
    html += "</ul>";
    html += "<p><a href='/oled?on=1'>oled on</a> | <a href='/oled?on=0'>oled off</a></p>";

    html += "<p>Telnet: port 23</p>";

    http.send(200, "text/html", html);
  });

  http.on("/mode", HTTP_GET, []() {
    if (http.hasArg("demo")) {
      String d = http.arg("demo");
      d.toLowerCase();
      g_mode = (d == "status") ? DemoMode::Status : DemoMode::Boing;
      logf("mode: %s", (g_mode == DemoMode::Boing) ? "boing" : "status");
    }
    http.sendHeader("Location", "/");
    http.send(302, "text/plain", "");
  });

  http.on("/oled", HTTP_GET, []() {
    if (http.hasArg("on")) {
      g_oledEnabled = (http.arg("on") == "1");
      logf("oled: %s", g_oledEnabled ? "enabled" : "disabled");
      if (!g_oledEnabled && u8g2) {
        u8g2->clearBuffer();
        u8g2->sendBuffer();
      } else if (g_oledEnabled) {
        oledSelectAndBegin();
      }
    }
    http.sendHeader("Location", "/");
    http.send(302, "text/plain", "");
  });

  http.on("/oledcfg", HTTP_GET, []() {
    if (http.hasArg("drv")) {
      String d = http.arg("drv");
      d.toLowerCase();
      g_drv = (d == "ssd1306") ? OledDrv::SSD1306 : OledDrv::SH1106;
    }
    if (http.hasArg("xoff")) {
      g_xOffset = http.arg("xoff").toInt();
    }

    logf("oledcfg: drv=%s xoff=%d",
         (g_drv == OledDrv::SSD1306) ? "ssd1306" : "sh1106",
         g_xOffset);

    oledSelectAndBegin(); // re-init with new driver
    http.sendHeader("Location", "/mode?demo=status");
    http.send(302, "text/plain", "");
  });

  http.on("/reboot", HTTP_GET, []() {
    http.send(200, "text/plain", "rebooting");
    delay(100);
    ESP.restart();
  });

  http.onNotFound([]() { http.send(404, "text/plain", "not found"); });
}

void setup() {
  Serial.begin(115200);
  delay(50);

  logln("");
  logf("boot: fw=%s", FW_VERSION);

  telnetServer.begin();
  telnetServer.setNoDelay(true);

  setupWifi();

  setupHttpRoutes();
  ElegantOTA.begin(&http, OTA_USER, OTA_PASS);
  http.begin();

  logln("http: started (port 80)");
  logln("ota:  /update (basic auth enabled)");
  logln("telnet: port 23");

  oledSelectAndBegin();
  g_mode = DemoMode::Status; // start with status so you can tune drv/xoff first
}

void loop() {
  handleTelnet();
  http.handleClient();
  ElegantOTA.loop();

  uint32_t now = millis();

  // render budget: keep OTA/telnet responsive
  static uint32_t lastDraw = 0;
  if (g_mode == DemoMode::Boing) {
    if (now - lastDraw >= 40) { // ~25fps (safer for OTA responsiveness)
      lastDraw = now;
      stepAndRenderBoing();
    }
  } else {
    if (now - lastDraw >= 400) { // 2.5fps
      lastDraw = now;
      renderStatus();
    }
  }

  static uint32_t lastLog = 0;
  if (now - lastLog > 1000) {
    lastLog = now;
    logf("wifi: %s ip=%s rssi=%d heap=%u mode=%s drv=%s xoff=%d",
      wlStatusName(WiFi.status()),
      (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString().c_str() : "(unset)",
      (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0,
      ESP.getFreeHeap(),
      (g_mode == DemoMode::Boing) ? "boing" : "status",
      (g_drv == OledDrv::SSD1306) ? "ssd1306" : "sh1106",
      g_xOffset
    );
  }

  delay(1);
  yield();
}