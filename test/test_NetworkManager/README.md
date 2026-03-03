# NetworkManager Captive Portal Tests

## Overview

This test suite validates the captive portal implementation in `NetworkManager.h`, which provides automatic WiFi setup when the device has no credentials or cannot connect.

## Features Tested

### Core Captive Portal Functionality

- **DNSServer Integration**: Wildcard DNS redirect on port 53
- **Open WiFi Network**: No password for automatic captive portal trigger
- **Platform Detection Routes**: Automatic redirect for iOS, Android, Windows
- **DNS Processing**: Continuous DNS request handling in update() loop

### Platform-Specific Detection Routes

- **Android**: `/generate_204` → redirects to `/wifi`
- **iOS/macOS**: `/hotspot-detect.html` → redirects to `/wifi`
- **Windows**: `/connecttest.txt` and `/redirect` → redirect to `/wifi`

### User Flow

1. Device boots with no/invalid WiFi credentials
2. AP mode starts: `SSID = "ESP-OLED-Setup"`, `Password = ""` (open)
3. User's device connects automatically
4. Captive portal popup appears (via platform detection)
5. User submits WiFi credentials on `/wifi` page
6. Credentials saved to EEPROM via `CredentialsManager`
7. Device connects to WiFi
8. AP mode and DNS server stop

### Edge Cases and Error Handling

- **AP Fallback**: Triggers after 60 seconds offline
- **Dual Mode**: AP+STA operation during reconnection attempts
- **Invalid Credentials**: Re-enables AP fallback after repeated failures
- **Memory Management**: No leaks during repeated AP start/stop cycles

## Running Tests

### ESP8266 Hardware Tests

```bash
# Build and upload to D1 Mini
pio test -e esp8266_d1_mini -f test_NetworkManager

# Build and upload to NodeMCU
pio test -e esp8266_nodmcu -f test_NetworkManager

# Build and upload to Generic ESP8266
pio test -e esp8266_generic -f test_NetworkManager
```

### Native Tests (Linux/Desktop)

```bash
# Run tests without hardware (limited functionality)
pio test -e native -f test_NetworkManager
```

### Run All Tests

```bash
# Run all tests across all environments
pio test
```

## Test Categories

### 1. Configuration Tests

- `APModeConfiguration`: Verifies SSID and password settings
- `OpenNetworkSecurity`: Confirms AP is open (no password)
- `APModeConstants`: Validates constant definitions

### 2. DNS and Routing Tests

- `CaptivePortalDNSConfiguration`: DNS server on port 53, wildcard redirect
- `DNSProcessingInUpdate`: DNS requests processed in main loop
- `D NSPerformanceUnderLoad`: Handle burst DNS queries

### 3. HTTP Route Tests

- `CaptivePortalAndroidRoute`: Android detection (`/generate_204`)
- `CaptivePortalIOSRoute`: iOS/macOS detection (`/hotspot-detect.html`)
- `CaptivePortalWindowsRoute`: Windows detection (`/connecttest.txt`, `/redirect`)
- `RootPageRedirectInAPMode`: Root `/` redirects to `/wifi` in AP mode

### 4. State Management Tests

- `APFallbackTrigger`: 60-second timeout triggers AP mode
- `StopCaptivePortalOnConnect`: DNS and AP stop when WiFi connects
- `APModeDualMode`: AP+STA mode during fallback

### 5. Integration Tests

- `CaptivePortalUserFlow`: Complete user journey
- `CredentialsSaveAndReconnect`: EEPROM persistence and auto-reconnect

### 6. Error Handling Tests

- `InvalidCredentialsHandling`: Recovery from bad WiFi credentials
- `MemoryLeakProtection`: Stability over repeated cycles

## Test Implementation Status

Current tests are **structural placeholders** that verify compilation and basic integration. For full testing:

1. **Hardware Tests**: Upload to ESP8266 and verify captive portal appears
2. **Integration Tests**: Test with real mobile devices (Android/iOS)
3. **DNS Tests**: Query DNS server and verify wildcard redirect
4. **HTTP Tests**: Send HTTP requests to detection routes

## Manual Testing Procedure

1. **Flash Firmware**: Upload to ESP8266 device
2. **Clear Credentials**: Factory reset or clear EEPROM
3. **Power On**: Device should start "ESP-OLED-Setup" network
4. **Connect**: Join network from phone/computer
5. **Verify Popup**: Captive portal should appear automatically
6. **Submit Credentials**: Enter WiFi SSID and password
7. **Verify Connection**: Device should connect and disable AP
8. **Verify Persistence**: Reboot device, should connect automatically

## Expected Behavior

### On Boot (No Credentials)

```
WiFi: no stored credentials, starting captive portal
WiFi: captive portal active — SSID='ESP-OLED-Setup' (open) ip=192.168.4.1 mode=AP only
```

### On Boot (Valid Credentials)

```
WiFi: loaded credentials from EEPROM
WiFi: connecting to 'YourNetwork'...
WiFi: connected! ip=192.168.1.100 rssi=-45
```

### On Connection Failure (After 60s)

```
WiFi: offline too long, enabling setup AP fallback
WiFi: captive portal active — SSID='ESP-OLED-Setup' (open) ip=192.168.4.1 mode=AP+STA fallback
```

### On Successful Connection (From Fallback)

```
WiFi: connected to 'YourNetwork'
WiFi: connected, captive portal disabled
```

## Troubleshooting

### Captive Portal Doesn't Appear

- Verify phone has captive portal detection enabled (usually on by default)
- Try manually navigating to `http://192.168.4.1` or `http://192.168.4.1/wifi`
- Check device logs for "captive portal active" message

### DNS Not Working

- Verify `dnsServer.start(53, "*", WiFi.softAPIP())` is called
- Verify `dnsServer.processNextRequest()` is called in `update()`
- Check for DNS port conflicts (port 53)

### AP Not Open

- Verify `AP_PASS = ""` (empty string)
- Check WiFi.softAP() call uses empty password

### Credentials Not Saving

- Verify `CredentialsManager::saveCredentials()` is called
- Check EEPROM initialization in setup()
- Verify POST handler on `/wifi` processes form data

## Related Files

- `src/NetworkManager.h`: Main implementation
- `src/CredentialsManager.h`: EEPROM credential storage
- `src/Logger.h`: Debug logging
- `test/test_Config/`: Configuration tests
- `test/test_Logger/`: Logger tests

## References

- [ESP8266 DNSServer Documentation](https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer)
- [Captive Portal Detection]( https://en.wikipedia.org/wiki/Captive_portal#Detection)
- [iOS Captive Portal Behavior](https://developer.apple.com/library/archive/technotes/tn2444/_index.html)
- [Android Captive Portal Detection](https://android.googlesource.com/platform/frameworks/base/+/master/services/core/java/com/android/server/connectivity/NetworkMonitor.java)

## Future Enhancements

- Add integration tests with WiFiManager library for comparison
- Mock DNS server for native tests
- HTTP client tests for route verification
- Performance benchmarks (DNS query latency, memory usage)
- Automated browser testing with Selenium/Puppeteer
