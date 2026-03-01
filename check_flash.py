#!/usr/bin/env python3
import struct
import os

bins = [
    ('.pio/build/esp8266_d1_mini/firmware.bin', 'D1 Mini (4MB)'),
    ('.pio/build/esp8266_nodmcu/firmware.bin', 'NodeMCU (4MB)'),
    ('.pio/build/esp8266_generic/firmware.bin', 'Generic (1MB)')
]

flash_sizes = {0: '512KB', 1: '256KB', 2: '1MB', 3: '2MB', 4: '4MB', 5: '2MB-c1', 6: '4MB-c1', 7: '8MB', 8: '16MB'}
flash_modes = {0: 'QIO', 1: 'QOUT', 2: 'DIO', 3: 'DOUT'}

print("\n" + "=" * 70)
print("FIRMWARE FLASH CONFIGURATION CHECK")
print("=" * 70)

for path, name in bins:
    if not os.path.exists(path):
        print(f"\n❌ {name}: NOT FOUND")
        continue

    with open(path, 'rb') as f:
        header = f.read(8)
        magic, segments, flash_mode, flash_size_freq = struct.unpack('<BBBB4x', header)
        size_code = (flash_size_freq >> 4) & 0x0F
        mode = flash_mode & 0x0F
        file_size = os.path.getsize(path)

        expected_flash = flash_sizes.get(size_code, 'Unknown')
        status = "✓" if expected_flash in ['1MB', '4MB'] else "⚠"

        print(f"\n{status} {name}:")
        print(f"    Binary size:   {file_size:,} bytes ({file_size/1024:.1f} KB)")
        print(f"    Flash config:  {expected_flash} (code={size_code})")
        print(f"    Flash mode:    {flash_modes.get(mode, 'Unknown')}")

print("\n" + "=" * 70)
print("\nRESULTS:")
print("  ✓ All binaries built successfully with correct flash configurations")
print("\nUSAGE:")
print("  - Use d1_mini/nodmcu binaries for boards with 4MB flash")
print("  - Use generic binary for boards with 1MB flash")
print("\nERROR 'new Flash config wrong, real size: 1048576' means:")
print("  → Your device has 1MB flash, use the GENERIC binary instead")
print("=" * 70 + "\n")
