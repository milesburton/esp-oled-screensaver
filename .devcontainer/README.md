# Development Container

Provides a consistent, reproducible development environment across Windows, macOS, and Linux.

## Prerequisites

- Docker Desktop
- VS Code with Remote Containers extension

## Setup

1. Open the project in VS Code
2. Click "Reopen in Container" (or use Command Palette: `Remote-Containers: Open Folder in Container`)
3. Container builds on first launch (~3-5 minutes), then cached for fast restarts

## Included

- PlatformIO with ESP8266 platform
- Build tools (compiler, linker, firmware generation)
- Code quality tools (clang-format, cpplint, pre-commit)
- Testing framework (AUnit)
- GitHub CLI (`gh`) for repository operations
- VS Code extensions (C/C++, git tools, formatters)

## Credentials

SSH keys, git config, and GitHub CLI credentials are automatically mounted from your host machine. No reauthentication required for git operations or `gh` commands.

Mounted from host:

- `~/.ssh` → SSH keys for git push/pull
- `~/.gitconfig` → Git user identity and preferences
- `~/.config/gh` → GitHub CLI authentication

If `gh` commands fail, authenticate once on the host: `gh auth login`

## Serial Programmer Access (USB devices)

The container is configured to access USB serial devices (e.g., CH340, CP2102 programmers) for flashing ESP8266 boards.

**Automatic Setup:**

- Device container mounts `/dev` directory with `--privileged`
- USB devices appear at `/dev/ttyUSB0`, `/dev/ttyACM0`, etc.
- No additional configuration needed

**Recovery and Flashing:**

```bash
# Inside container, use the recovery script
./recovery.sh /dev/ttyUSB0 esp8266_d1_mini

# Or manually with esptool
esptool.py -p /dev/ttyUSB0 version
```

**If USB devices don't appear:**

1. Restart Docker Desktop
2. Unplug and reconnect USB device
3. Rebuild container: `Ctrl+Shift+P` → "Dev Containers: Rebuild Container"

## Usage

```bash
# Build firmware
./build.sh esp8266_d1_mini

# Run tests
pio test --without-uploading

# Run code checks manually
pre-commit run --all-files

# Monitor serial output (if USB passthrough is available)
pio device monitor -b 115200
```

Pre-commit hooks run automatically on `git commit`.

## Troubleshooting

**"Docker daemon is not running"**
Start Docker Desktop.

**"Port 80 already in use"**
Stop conflicting containers:

```bash
docker ps
docker stop <container-id>
```

Or modify port mappings in `devcontainer.json`.

**"Permission denied" on Linux**
Add user to docker group:

```bash
sudo usermod -aG docker $USER
newgrp docker
```

**"PlatformIO claims unknown board"**
Update inside the container:

```bash
platformio update
platformio platform install espressif8266
```

## Configuration

`Dockerfile` - Base image and tool installation
`devcontainer.json` - VS Code settings and extensions
`docker-compose.yml` - Optional Docker Compose configuration

See [Development Containers spec](https://containers.dev/) for details.
