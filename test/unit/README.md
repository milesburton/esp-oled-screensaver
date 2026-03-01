# Unit Testing Framework

This directory contains unit tests for the ESP8266 Weather Clock project.

## Framework

We use [AUnit](https://github.com/bxparks/AUnit) for Arduino-compatible unit testing.

### Installation

Install via Arduino Library Manager:
1. Open Arduino IDE
2. Go to Sketch → Include Library → Manage Libraries
3. Search for "AUnit"
4. Install "AUnit by Brian Park"

Or via PlatformIO:
```ini
lib_deps = 
    bxparks/AUnit@^1.7.1
```

## Running Tests

### Arduino IDE
1. Open a test sketch (e.g., `test_Config/test_Config.ino`)
2. Upload to your ESP8266
3. Open Serial Monitor (115200 baud)
4. View test results

### Command Line (PlatformIO)
```bash
pio test
```

### Pre-commit Hook
Tests run automatically on commit via pre-commit hooks.

## Writing Tests

Create a new test file following this pattern:

```cpp
#include <AUnit.h>
#include "../Config.h"

test(ConfigTest, DefaultValues) {
  assertEqual(Config::OLED_SDA, 0);
  assertEqual(Config::OLED_SCL, 2);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  aunit::TestRunner::run();
}
```

## Test Coverage

- [ ] Config module
- [ ] Logger module
- [ ] DisplayManager
- [ ] DisplayMode implementations
- [ ] NetworkManager
- [ ] TelnetConsole

## TODO

- Set up PlatformIO for automated testing
- Add CI/CD integration
- Increase test coverage to >80%
