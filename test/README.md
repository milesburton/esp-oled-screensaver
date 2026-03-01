# Unit Tests

This directory contains unit and integration tests for the ESP8266-OLED-Experiment project.

## Directory Structure

```
test/
├── test_Config/          - Config module unit tests
│   └── test_Config.ino
├── test_Logger/          - Logger module unit tests
│   └── test_Logger.ino
├── test_DisplayModes/    - DisplayModes module unit tests
│   └── test_DisplayModes.ino
└── integration/          - Integration tests (coming soon)
    └── README.md
```

## Framework

We use [AUnit](https://github.com/bxparks/AUnit) - Arduino Unit Testing Framework.

### Installation

**Via Arduino Library Manager:**

1. Open Arduino IDE
2. Sketch → Include Library → Manage Libraries
3. Search for "AUnit"
4. Install "AUnit by Brian Park"

**Via PlatformIO:**

```ini
; Already configured in platformio.ini
lib_deps = bxparks/AUnit@^1.7.1
```

## Running Tests

### Via PlatformIO

```bash
# Run all unit tests
pio test

# Run specific test environment
pio test -e esp8266_d1_mini

# With verbose output
pio test -vvv
```

### Via Arduino IDE

1. Open a test sketch (e.g., `test_Config/test_Config.ino`)
2. Upload to your ESP8266
3. Open Serial Monitor (115200 baud)
4. View test results

### Via Pre-commit Hook

Tests automatically run on commit via pre-commit hooks (when configured).

## Writing Tests

### Basic Test Structure

```cpp
#include <AUnit.h>
#include "../../src/Config.h"

test(ConfigTest, DefaultOledSDA) {
  assertEqual(Config::OLED_SDA, 0);
}

test(ConfigTest, DefaultOledSCL) {
  assertEqual(Config::OLED_SCL, 2);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection
}

void loop() {
  aunit::TestRunner::run();
}
```

### Test Naming Convention

- **Test Class**: `ModuleNameTest` (e.g., `ConfigTest`, `LoggerTest`)
- **Test Method**: Descriptive name (e.g., `DefaultOledSDA`, `SerialOutput`)
- **Format**: `test(ClassName, MethodName)`

### Common Assertions

```cpp
// Equality
assertEqual(actual, expected);
assertNotEqual(actual, unexpected);

// Boolean
assertTrue(condition);
assertFalse(condition);

// Comparison
assertLess(actual, limit);
assertMore(actual, limit);
assertLessOrEqual(actual, limit);
assertMoreOrEqual(actual, limit);

// String
assertStringEqual("hello", actual);

// Float/Double (with epsilon)
assertFloatNear(actual, expected, 0.01);
```

## Test Coverage

Current test coverage:

- [ ] **Config** - Configuration module tests
- [ ] **Logger** - Logging utility tests
- [ ] **DisplayManager** - Display and mode management
- [ ] **NetworkManager** - WiFi and HTTP server
- [ ] **DisplayMode** - Base class functionality
- [ ] **StatusMode** - Status display mode
- [ ] **BoingMode** - Boing animation mode
- [ ] **WeatherMode** - Weather display mode
- [ ] **TelnetConsole** - Remote console

## Best Practices

1. **Keep tests focused** - One aspect per test
2. **Use descriptive names** - Test name should indicate what's being tested
3. **Minimize dependencies** - Mock/stub external dependencies when possible
4. **Test edge cases** - Test boundary conditions, null values, etc.
5. **Avoid test interdependence** - Tests should run in any order

## Example Test File

```cpp
#include <AUnit.h>
#include "../../src/Logger.h"

test(LoggerTest, PrintlnWorks) {
  // Logger should not crash when printing
  Logger::println("Test message");
  assertTrue(true); // If we got here, no crash occurred
}

test(LoggerTest, PrintfWorks) {
  // Logger::printf should handle format strings
  Logger::printf("Value: %d", 42);
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\n" __FILE__ " started");
}

void loop() {
  aunit::TestRunner::run();
}
```

## Troubleshooting

### Tests Won't Compile

- Check AUnit library is installed
- Verify include paths are correct (use `../../src/` for src files)
- Check for missing semicolons or syntax errors

### Serial Output Not Showing

- Verify baud rate is 115200
- Check that Serial.begin(115200) is called before TestRunner::run()
- Allow time for serial connection (while (!Serial);)

### Tests Hang or Crash

- Check for infinite loops in test code
- Verify hardware is not being blocked (e.g., I2C operations on test board without hardware)
- Check memory usage on ESP8266

## Continuous Integration

Tests are automatically run on:

- Every push to the repository (GitHub Actions)
- Pre-commit hooks (local)

See [build workflow](./.github/workflows/quality.yml) for CI configuration.

## Next Steps

1. Write tests for Config module
2. Write tests for Logger module
3. Write tests for DisplayManager
4. Increase overall test coverage to >80%
5. Set up coverage reports

## References

- [AUnit Documentation](https://github.com/bxparks/AUnit)
- [PlatformIO Testing](https://docs.platformio.org/en/latest/feature_unit_testing.html)
