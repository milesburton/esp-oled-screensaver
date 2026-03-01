# Development Container Setup

This project includes a [Development Container](https://containers.dev/) configuration for consistent development environments across Windows, macOS, and Linux.

## Quick Start

### Prerequisites
- **Docker** ([Install Docker Desktop](https://www.docker.com/products/docker-desktop))
- **VS Code** ([Download](https://code.visualstudio.com/))
- **Remote - Containers extension** - Install from VS Code Extensions marketplace

### Opening in Container

1. Open the project folder in VS Code
2. When prompted, click **"Reopen in Container"** (or use Command Palette: `Remote-Containers: Open Folder in Container`)
3. VS Code will build the container (~3-5 minutes first time, then cached)
4. Once ready, you have a fully configured development environment

## What's Included

The devcontainer provides:

✅ **PlatformIO** - Latest version with ESP8266 platform  
✅ **Build Tools** - Compiler, linker, and firmware generation  
✅ **Code Quality** - clang-format, cpplint, pre-commit hooks  
✅ **Testing** - AUnit framework pre-configured  
✅ **Git Integration** - Pre-commit hooks, GitLens, Git Graph  
✅ **Extensions** - C/C++ IntelliSense, language support, formatters  

## Usage

### Building Firmware

```bash
# Build for default target (D1 Mini)
./build.sh

# Build for specific target
./build.sh esp8266_nodmcu

# Available targets: esp8266_d1_mini, esp8266_nodmcu, esp8266_generic
```

Output binaries are in the `firmware/` directory.

### Running Tests

```bash
# Run all tests
pio test --without-uploading

# Run specific test
pio test --without-uploading -e debug
```

### Pre-commit Hooks

Pre-commit is automatically installed when the container starts. It will:
- Format code with clang-format
- Lint C++ with cpplint
- Check markdown formatting
- Validate conventional commit messages

Hooks run automatically on `git commit`. To run manually:

```bash
pre-commit run --all-files
```

### Serial Monitor / Telnet

To interact with the device:

```bash
# Via Telnet (if device is on network)
telnet <device-ip> 23

# Via Serial (if your machine can pass through USB)
pio device monitor -p /dev/ttyUSB0  # Linux/Mac
pio device monitor -p COM3           # Windows host (use WSL2)
```

Note: OTA updates eliminate the need for serial access in most cases.

### Stopping the Container

- **Keep changes**: Just close VS Code; container stays running (fast restart)
- **Full stop**: Command Palette → `Remote-Containers: Restart Container in Local Folder`
- **Remove**: Delete container image from Docker Desktop

## File Structure

```
.devcontainer/
├── Dockerfile          # Container build specification
├── devcontainer.json   # VS Code Remote Containers configuration
└── docker-compose.yml  # Optional Docker Compose setup
```

## Troubleshooting

### "Docker daemon is not running"
Start Docker Desktop and try again.

### "Port 80 already in use"
The devcontainer exposes ports 80 (HTTP) and 23 (Telnet). If these conflict:
- Stop other containers using `docker ps` and `docker stop <id>`
- Or modify port mappings in `devcontainer.json`

### "Permission denied" errors on Linux
Ensure your user is in the docker group:
```bash
sudo usermod -aG docker $USER
newgrp docker
```

### Build takes too long
The first build pulls base image (~500MB) and installs tools (~1min). Subsequent builds are instant (cached).

### PlatformIO claims unknown board
Ensure container fully started, then:
```bash
platformio update
platformio platform install espressif8266
```

## Comparison: Container vs Local Installation

| Aspect | Container | Local |
|--------|-----------|-------|
| Setup Time | 5 min | 15-30 min |
| Dependencies | Automatic | Manual install |
| Platform Consistency | ✅ Yes | ❌ Varies |
| Python Conflicts | ✅ Isolated | ❌ Possible |
| CI/CD Alignment | ✅ Identical | ⚠️ Similar |
| Disk Space | ~2GB | ~1GB |
| Container Usage | Faster after first build | N/A |

## CI/CD Integration

GitHub Actions can also build using this container (future enhancement):

```yaml
# In .github/workflows/build.yml
container:
  image: platformio/platformio-core:latest
```

This ensures your CI environment matches local development exactly.

## Additional Resources

- [Development Containers Specification](https://containers.dev/)
- [VS Code Remote Container Docs](https://code.visualstudio.com/docs/remote/containers)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Docker Desktop](https://www.docker.com/products/docker-desktop)
