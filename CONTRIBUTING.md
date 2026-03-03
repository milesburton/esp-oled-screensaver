# Contributing

## Setup

**Dev container (recommended):** open in VS Code and click "Reopen in Container".

**Local:**

```bash
git clone https://github.com/milesburton/esp-oled-screensaver.git
cd esp-oled-screensaver
cp secrets.h.template secrets.h
pip install pre-commit
pre-commit install --hook-type commit-msg
```

## Code Style

- 2-space indentation, 100-character line limit
- `PascalCase` classes, `camelCase` methods and variables, `UPPER_CASE` constants
- Google C++ style enforced by clang-format and cpplint via pre-commit

## Commits

[Conventional Commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <subject>
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`, `perf`, `ci`

## Adding a Display Mode

1. Create `src/MyMode.h` implementing `DisplayMode` (`getName`, `update`).
2. Register in all four files: `ESP8266-OLED-Experiment.ino`, `ModeHelper.h`, `NetworkManager.h`, `TelnetConsole.h`, and `ScreensaverMode.h`.
3. Add tests in `test/test_DisplayModes/test_main.cpp`.
4. Update the mode table in `README.md`.

## Pull Requests

1. Branch: `git checkout -b feat/name`
2. Implement with tests
3. `pre-commit run --all-files` passes
4. `pio test --without-uploading` passes
5. Submit PR
