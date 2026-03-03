# Auto-Update

The device checks GitHub Releases every 6 hours and flashes new firmware automatically. **Disabled by default** — the user must opt in.

## Enable / Disable

**Web UI:** browse to `http://<device-ip>/` → Auto-Update section → Enable.

**API:**

```bash
curl "http://<device-ip>/autoupdate?on=1"   # enable
curl "http://<device-ip>/autoupdate?on=0"   # disable
```

The setting is stored in EEPROM and survives firmware upgrades.

## Status

```bash
curl "http://<device-ip>/update-status" | jq
```

```json
{
  "auto_update_enabled": true,
  "fw_version": "platform-1.0.47",
  "update_available": true,
  "updater_state": "IDLE",
  "update_progress": 0,
  "available_version": "1.0.48"
}
```

| State            | Meaning                                  |
|------------------|------------------------------------------|
| `IDLE`           | Waiting for next check                   |
| `DOWNLOADING`    | Fetching binary (see `update_progress`)  |
| `FLASHING`       | Writing to flash                         |
| `VERIFYING`      | Post-flash validation                    |
| `SUCCESS`        | Rebooting in 5 s                         |
| `ERROR_DOWNLOAD` | Download failed — retries in 1 hour      |
| `ERROR_FLASH`    | Flash write failed — retries in 1 hour   |

## Force a Check Now

```bash
curl "http://<device-ip>/force-check"
```

This bypasses the 6-hour interval and runs immediately, regardless of the enabled flag.

## How It Works

1. `UpdateChecker` polls `api.github.com/repos/milesburton/esp-oled-screensaver/releases/latest` every 6 hours.
2. The `tag_name` from the response is compared against the running firmware version using semantic versioning.
3. If a newer version is found, `AutoUpdater` streams the `firmware.bin` asset directly to the flash partition.
4. After a successful flash the device reboots (5-second delay).

No authentication is required — the repository is public.

## Publishing a Release

```bash
git tag v1.0.48
git push origin v1.0.48
```

GitHub Actions builds the firmware and uploads `firmware.bin` to the release automatically. Devices detect the update within 6 hours.
