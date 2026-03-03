# Auto-Update Feature Documentation

## Overview

The ESP8266 OLED Screensaver now includes a **complete over-the-air (OTA) auto-update system** that automatically checks for firmware updates and applies them with zero user intervention.

**Key Features:**
- ✅ **Secure by default**: Auto-update is OFF; users must explicitly enable it
- ✅ **No credentials embedded**: Uses public GitHub Releases API (no tokens in firmware)
- ✅ **Persistent settings**: Auto-update preference persists across firmware upgrades (EEPROM)
- ✅ **Safe rollout**: Devices only update when WiFi is connected and battery is stable
- ✅ **Progress reporting**: JSON APIs show download/flash status in real-time
- ✅ **Rate-limited**: Failed updates retry once per hour (not hammering network)

## Architecture

### Components

1. **UpdateManager.h** — Settings persistence
   - Stores auto-update enabled/disabled flag in EEPROM (bytes 128–191)
   - Survives firmware flashes and power cycles
   - User can toggle via web UI

2. **UpdateChecker.h** — Version detection
   - Fetches latest release from GitHub API (6-hour interval)
   - Parses version and download URL
   - Compares versions using semantic versioning (major.minor.patch)
   - Runs only if WiFi connected and auto-update enabled

3. **AutoUpdater.h** — Binary download & flash
   - Downloads firmware binary from GitHub Release
   - Streams directly to flash partition
   - Progress tracking (0-100%)
   - Error handling with state machine

4. **NetworkManager.h** — Web UI & routes
   - `/autoupdate?on=1|0` — Toggle auto-update setting
   - `/check-update` — JSON API with update status
   - `/update-status` — Real-time progress and state
   - `/force-check` — Manual trigger for update check

5. **GitHub Actions CI** (`.github/workflows/build.yml`)
   - Automatically builds firmware on every release tag
   - Uploads binary to GitHub Releases
   - No manual steps needed

### Flow Diagram

```
User enables auto-update
    ↓
Device polls GitHub API every 6 hours
    ↓
New version available? (version comparison)
    ↓ YES
    ├→ Log "UPDATE AVAILABLE v1.0.45"
    ├→ Display in web UI
    ├→ On next update cycle...
    │
    ├→ Download firmware.bin from GitHub Release
    │ └→ Stream to flash (shows progress)
    │
    ├→ Verify size matches
    │
    ├→ Finalize flash and verify
    │
    ├→ Reboot device with new firmware
    │
    └→ Device boots with new version
```

## Usage

### Enable Auto-Update (User)

**Option 1: Web UI**
1. Browse to `http://device-ip/` (or `http://device-ip:80/`)
2. Scroll to **Auto-Update** section
3. Click **"Enable Auto-Update"**
4. Device will check every 6 hours for new firmware

**Option 2: Telnet/Serial Console**
```
Device logs will show:
  UpdateMgr: auto-update ENABLED
  UpdateChecker: checking for updates (interval=6h)...
  UpdateChecker: UPDATE AVAILABLE v1.0.45 (current v1.0.44)
```

**Option 3: curl / API**
```bash
# Enable auto-update
curl "http://device-ip/autoupdate?on=1"

# Check status
curl "http://device-ip/update-status" | jq
```

### Check Update Status

**JSON API Response:**
```bash
$ curl "http://device-ip/update-status" | jq
{
  "auto_update_enabled": true,
  "fw_version": "1.0.44",
  "update_available": true,
  "updater_state": "IDLE",
  "update_progress": 0,
  "available_version": "1.0.45"
}
```

**Possible updater states:**
- `IDLE` — Waiting for next check
- `DOWNLOADING` — Fetching binary from GitHub (shows progress %)
- `FLASHING` — Writing to flash partition
- `VERIFYING` — Post-flash validation
- `SUCCESS` — Update complete, rebooting in 5 sec
- `ERROR_DOWNLOAD` — Download failed (will retry in 1 hour)
- `ERROR_FLASH` — Flash write failed (will retry in 1 hour)

### Manual Update Check

Force an immediate update check (ignores 6-hour interval):
```bash
curl "http://device-ip/force-check"
# Response: {"status":"check triggered"}

# Check if new version found:
sleep 5
curl "http://device-ip/update-status" | jq '.update_available'
```

### Disable Auto-Update

```bash
curl "http://device-ip/autoupdate?on=0"
```

Device will still show web UI option to manually run `/update` (ElegantOTA).

## Technical Details

### Version Comparison

Versions follow **semantic versioning**: `major.minor.patch`

Examples:
- `1.0.45` > `1.0.44` ✓ (patch bump)
- `1.1.0` > `1.0.99` ✓ (minor bump)
- `2.0.0` > `1.99.99` ✓ (major bump)
- `1.0.0` = `1.0.0` ✗ (same version, no update)

### GitHub API Integration

Device fetches: `https://api.github.com/repos/milesburton/esp-oled-screensaver/releases/latest`

Parses JSON:
```json
{
  "tag_name": "v1.0.45",
  "browser_download_url": "https://github.com/.../firmware.bin"
}
```

**No authentication required** (public repo + public API limits are generous: 60 req/hr per IP for unauthenticated).

### EEPROM Layout

**Bytes 128–191** (reserved for UpdateManager settings):
- Byte 128: Magic number (0xBB = valid)
- Byte 129: Schema version (1)
- Byte 130: Auto-update enabled flag (1 = yes, 0 = no)
- Byte 131: Channel (0 = stable, 1 = beta)
- Bytes 132–191: Reserved for future use (60 bytes)

Survives firmware flashes because OTA update preserves EEPROM.

### Check Interval

- **6 hours** between checks (configurable in UpdateChecker.h)
- Checks only run if:
  - WiFi connected
  - Auto-update enabled
  - No update already in progress
- Failed checks retry after 1 hour

### Safety Measures

1. **Binary size validation**: Checks free space before download
2. **Download timeout**: 30 seconds + 10 second buffer
3. **Progress logging**: Serial output shows % complete
4. **State machine**: Clear states prevent corruption (IDLE → DOWNLOADING → FLASHING → VERIFYING → SUCCESS)
5. **Reboot delay**: 5-second delay after SUCCESS allows graceful shutdown of WiFi
6. **Rate limiting**: Failed updates don't retry immediately (1-hour wait)

## Building Releases

### Automated (GitHub Actions)

1. **Tag a release** on GitHub:
   ```bash
   git tag v1.0.45
   git push origin v1.0.45
   ```

2. **CI automatically**:
   - Builds firmware for ESP8266 D1 mini
   - Uploads `firmware.bin` to Release
   - Generates SHA256 checksum
   - Creates release notes

3. **Devices detect within 6 hours**

### Manual (local testing)

```bash
# Build
platformio run -e esp8266_d1_mini

# Check output
ls -lh .pio/build/esp8266_d1_mini/firmware.bin

# Test update (manual):
#   1. Upload via web UI: http://device-ip/update
#   2. Or use curl/wget to download binary
```

## Troubleshooting

### "UpdateChecker: HTTP error 403"

**Cause:** GitHub API rate limiting (60 unauthenticated requests per hour per IP)

**Solution:**
- Wait 1 hour, device will retry
- Or manually force-check less frequently
- Consider adding GitHub token (optional enhancement)

### "AutoUpdater: insufficient space for update"

**Cause:** Device flash partition too full

**Solution:**
- Delete cached data or logs if applicable
- Or use smaller firmware (optimize code)
- ESP8266 requires ~256KB free for update partition swap

### "AutoUpdater: Update.end() failed, error: X"

**Cause:** Flash write error (hardware issue, corruption, or power loss)

**Solution:**
- Try manual update via `/update` (ElegantOTA)
- If persistent, may indicate hardware failure
- Consider chip replacement

### Device reboots during update

**Cause:** Watchdog timer reset (firmware taking too long)

**Solution:**
- Ensure `yield()` calls in tight loops (AutoUpdater does this)
- Reduce update check frequency if needed
- Check logs via Telnet for resource warnings

## API Reference

### Web Routes

| Route | Method | Purpose | Response |
|-------|--------|---------|----------|
| `/autoupdate?on=1\|0` | GET | Enable/disable auto-update | Redirect to `/` |
| `/check-update` | GET | Get update availability (deprecated) | JSON |
| `/update-status` | GET | Current update state & progress | JSON |
| `/force-check` | GET | Trigger manual check now | JSON |
| `/update` | GET/POST | Manual OTA upload (ElegantOTA) | HTML form |

### JSON Response Examples

#### `/update-status` (downloading)
```json
{
  "auto_update_enabled": true,
  "fw_version": "1.0.44",
  "update_available": true,
  "updater_state": "DOWNLOADING",
  "update_progress": 45,
  "available_version": "1.0.45"
}
```

#### `/update-status` (idle, no update)
```json
{
  "auto_update_enabled": true,
  "fw_version": "1.0.45",
  "update_available": false,
  "updater_state": "IDLE",
  "update_progress": 0
}
```

## Source Files

- **UpdateManager.h** — Settings I/O
- **UpdateChecker.h** — Version detection & polling
- **AutoUpdater.h** — Download & flash logic
- **NetworkManager.h** — Web routes & integration
- **.github/workflows/build.yml** — CI/CD automation
- **test/test_UpdateChecker/test_main.cpp** — Unit tests

## Future Enhancements

- [ ] SHA256 validation (download manifest with hashes)
- [ ] Staged rollout (merkle tree % adoption)
- [ ] Beta channel support (separate releases)
- [ ] Telemetry (report success/failure to cloud)
- [ ] Rollback on failure (keep previous firmware)
- [ ] Pause/resume downloads (checkpoint system)

## License & Disclaimer

This auto-update system is provided as-is. Users assume responsibility for:
- Reviewing firmware changes before enabling auto-update
- Ensuring adequate power supply during updates
- Testing on a single device before deployment to production

Devices can be recovered via manual firmware upload even if auto-update fails.
