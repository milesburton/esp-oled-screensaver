# Integration Tests

This directory contains integration tests that test multiple components working together.

## Purpose

Integration tests verify that different modules interact correctly as a system, unlike unit tests which test components in isolation.

## Example Integration Tests (To Be Written)

- Display modes rendering with DisplayManager
- Network manager handling HTTP requests
- Telnet console command execution
- OTA update process
- WiFi reconnection handling

## Running Integration Tests

```bash
pio test
```

## Future

- Test display mode switching
- Test network events (connect/disconnect)
- Test firmware update scenarios
- Test error conditions and recovery
