# Contributing to ESP8266 Weather Clock

Thank you for your interest in contributing! This document outlines the development workflow and code standards.

## Development Setup

### Prerequisites

- Arduino IDE or PlatformIO
- Python 3.7+ (for pre-commit hooks)
- Git

### Initial Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/milesburton/esp8266-weather-clock.git
   cd esp8266-weather-clock
   ```

2. **Set up secrets**
   ```bash
   cp secrets.h.template secrets.h
   # Edit secrets.h with your WiFi credentials
   ```

3. **Install pre-commit hooks**
   ```bash
   pip install pre-commit
   pre-commit install
   pre-commit install --hook-type commit-msg
   ```

4. **Verify setup**
   ```bash
   pre-commit run --all-files
   ```

## Code Quality Standards

### Pre-commit Hooks

We use pre-commit hooks to maintain code quality:

- ✅ **C++ Formatting** (clang-format) - Auto-formats code
- ✅ **C++ Linting** (cpplint) - Checks code style
- ✅ **Conventional Commits** - Enforces commit message format
- ✅ **Security** - Prevents committing secrets.h
- ✅ **Tests** - Runs unit tests when available
- ✅ **Markdown** - Lints documentation

### C++ Style Guide

- **Indentation**: 2 spaces (no tabs)
- **Line length**: 100 characters max
- **Naming**:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Constants: `UPPER_CASE` or `kCamelCase`
  - Variables: `camelCase`
- **Braces**: Attached (K&R style)
- **Includes**: Sorted and grouped

Example:
```cpp
class MyClass {
public:
  void doSomething() {
    if (condition) {
      // code here
    }
  }

private:
  int myVariable;
};
```

## Commit Message Format

We follow [Conventional Commits](https://www.conventionalcommits.org/) specification.

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `perf`: Performance improvements
- `ci`: CI/CD changes

### Examples

```bash
# Feature
feat(weather): add OpenWeatherMap API integration

# Bug fix
fix(display): correct X-offset calculation for SH1106

# Breaking change
feat(config)!: move secrets to environment variables

BREAKING CHANGE: secrets.h no longer used. Set WiFi credentials via environment variables.

# Multiple types
feat(modes): add ClockMode display
- Shows current time from NTP
- 12/24 hour format support
- Timezone configuration
```

### Scope (Optional)

Common scopes:
- `config`
- `display`
- `network`
- `telnet`
- `modes`
- `status`
- `boing`
- `weather`

## Adding New Display Modes

1. **Create mode header file in `src/`**
   ```cpp
   // src/MyMode.h
   #pragma once
   #include "DisplayMode.h"
   
   class MyMode : public DisplayMode {
   public:
     const char* getName() const override { return "mymode"; }
     void update(U8G2* u8g2, uint32_t deltaMs) override {
       // Your rendering code
     }
   };
   ```

2. **Register in main sketch** (`src/OA_OLED_Display_with_wifi_working.ino`)
   ```cpp
   #include "MyMode.h"
   MyMode myMode;
   
   // In setup()
   networkManager.setModes(&statusMode, &boingMode, &myMode);
   telnetConsole.setModes(&statusMode, &boingMode, &myMode);
   ```

3. **Add unit test** in `test/unit/test_MyMode/test_MyMode.ino`
   ```cpp
   #include <AUnit.h>
   #include "../../../src/MyMode.h"
   
   test(MyModeTest, GetName) {
     MyMode mode;
     assertStringEqual("mymode", mode.getName());
   }
   
   void setup() {
     Serial.begin(115200);
     while (!Serial);
   }
   
   void loop() {
     aunit::TestRunner::run();
   }
   ```

4. **Update documentation**
   - Add mode description to README.md
   - Document configuration options
   - Add telnet command documentation

## Pull Request Process

1. **Create a feature branch**
   ```bash
   git checkout -b feat/my-new-feature
   ```

2. **Make your changes**
   - Follow code style guidelines
   - Add tests for new features
   - Update documentation

3. **Commit with conventional format**
   ```bash
   git add .
   git commit -m "feat(modes): add temperature display mode"
   ```

4. **Push and create PR**
   ```bash
   git push origin feat/my-new-feature
   ```

5. **PR will be reviewed for**:
   - Code quality and style
   - Test coverage
   - Documentation updates
   - Conventional commit format

## Testing

### Running Tests Locally

```bash
# With pre-commit
pre-commit run --all-files

# Manual test run
./run_tests.sh

# PlatformIO (when configured)
pio test
```

### Writing Tests

- Use AUnit framework
- Aim for >80% coverage
- Test both success and failure cases
- Mock hardware dependencies when possible

## Questions?

Open an issue or reach out to the maintainers.

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.
