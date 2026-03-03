# Building

## Environments

| Environment        | Board         | Flash |
|--------------------|---------------|-------|
| `esp8266_d1_mini`  | Wemos D1 Mini | 4 MB  |
| `esp8266_nodmcu`   | NodeMCU v2    | 4 MB  |
| `esp8266_generic`  | Generic       | 1 MB  |

## Build

```bash
./build.sh esp8266_d1_mini          # Build (outputs to firmware/)
pio run -e esp8266_d1_mini -t upload # Build and upload via serial
pio device monitor -b 115200         # Serial monitor
pio test --without-uploading         # Run tests
```

## Flashing

**OTA (device already on network):**

1. Browse to `http://<device-ip>/update`
2. Upload the `.bin` from `firmware/`

**Serial (initial or recovery):**

```bash
pio run -e esp8266_d1_mini -t upload
```

**esptool (recovery):**

```bash
pip install esptool
esptool.py -p /dev/ttyUSB0 -b 921600 write_flash \
  --flash_mode dout --flash_freq 40m --flash_size detect \
  0x0 .pio/build/esp8266_d1_mini/firmware.bin
```

See [RECOVERY.md](RECOVERY.md) for full recovery procedures.

## CI/CD

Pushes to `main` run linting, tests, and a build check. Tagging `v*` triggers a release build that uploads `firmware.bin` to GitHub Releases.

```bash
git tag v1.0.0
git push origin v1.0.0
```
