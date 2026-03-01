# Contributing

## Setup

**Development Container (Recommended)**:

```bash
code .
# Click "Reopen in Container"
```

See [.devcontainer/README.md](.devcontainer/README.md).

**Local Installation**:

```bash
git clone https://github.com/milesburton/esp8266-weather-clock.git
cd esp8266-weather-clock
cp secrets.h.template secrets.h

pip install pre-commit
pre-commit install --hook-type commit-msg
```

## Code Style

- **Indentation**: 2 spaces
- **Line length**: 100 characters max
- **Classes**: `PascalCase`
- **Methods/variables**: `camelCase`
- **Constants**: `UPPER_CASE` or `kCamelCase`
- **Braces**: Attached (K&R style)

## Commits

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <subject>
<blank line>
<body>
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`, `perf`, `ci`

**Examples**:

```bash
feat(modes): add temperature display
fix(display): correct X-offset for SH1106
docs(readme): update build instructions
test(logger): add printf test with multiple args
```

## Adding Display Modes

1. Create mode in `src/MyMode.h`:

```cpp
#pragma once
#include "DisplayMode.h"

class MyMode : public DisplayMode {
public:
  const char* getName() const override {
    return "mymode";
  }

  void update(U8G2* u8g2, uint32_t deltaMs) override {
    u8g2->clearBuffer();
    u8g2->drawStr(0, 20, "My Mode");
    u8g2->sendBuffer();
  }
};
```

2. Register in `src/ESP8266-OLED-Experiment.ino`:

```cpp
#include "MyMode.h"
MyMode myMode;

// In setup()
networkManager.setModes(&statusMode, &boingMode, &myMode);
telnetConsole.setModes(&statusMode, &boingMode, &myMode);
```

3. Add test in `test/unit/test_MyMode/test_MyMode.ino`

4. Update README.md with features

## Pull Requests

1. Create feature branch: `git checkout -b feat/name`
2. Make changes and write tests
3. Commit with conventional format
4. Push: `git push origin feat/name`
5. Submit PR

## Testing

```bash
# Run all checks
pre-commit run --all-files

# Run tests
pio test --without-uploading
```

Aim for >80% test coverage.

## Code Quality

Pre-commit hooks automatically run:

- clang-format (C++ formatting)
- cpplint (linting)
- markdownlint (docs)
- Conventional commit validation

Enforce before commit.

## Questions

Open an issue or contact maintainers.

## License

By contributing, your work is licensed under the project license.
