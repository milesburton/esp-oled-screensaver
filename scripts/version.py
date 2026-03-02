#!/usr/bin/env python3

import argparse
import re
import sys
from pathlib import Path


DEFAULT_CONFIG_PATH = Path("src/Config.h")
VERSION_PATTERN = re.compile(r'FW_VERSION\s*=\s*"platform-(\d+)\.(\d+)\.(\d+)"')


def read_file(path: Path) -> str:
  return path.read_text(encoding="utf-8")


def write_file(path: Path, content: str) -> None:
  path.write_text(content, encoding="utf-8")


def parse_version(content: str) -> tuple[int, int, int]:
  match = VERSION_PATTERN.search(content)
  if not match:
    raise ValueError("Could not find FW_VERSION in Config.h")
  return int(match.group(1)), int(match.group(2)), int(match.group(3))


def format_version(version: tuple[int, int, int]) -> str:
  return f"{version[0]}.{version[1]}.{version[2]}"


def bump_version(version: tuple[int, int, int], bump_type: str) -> tuple[int, int, int]:
  major, minor, patch = version
  if bump_type == "major":
    return major + 1, 0, 0
  if bump_type == "minor":
    return major, minor + 1, 0
  return major, minor, patch + 1


def replace_version(content: str, new_version: tuple[int, int, int]) -> str:
  new_literal = f'FW_VERSION = "platform-{format_version(new_version)}"'
  return VERSION_PATTERN.sub(new_literal, content, count=1)


def main() -> int:
  parser = argparse.ArgumentParser(description="Read and bump firmware version in src/Config.h")
  parser.add_argument("command", choices=["current", "bump"])
  parser.add_argument("bump_type", nargs="?", choices=["major", "minor", "patch"], default="patch")
  parser.add_argument("--write", action="store_true", help="Write bumped version back to Config.h")
  parser.add_argument("--config", default=str(DEFAULT_CONFIG_PATH), help="Path to Config.h")
  args = parser.parse_args()

  config_path = Path(args.config)
  content = read_file(config_path)
  current = parse_version(content)

  if args.command == "current":
    print(format_version(current))
    return 0

  new_version = bump_version(current, args.bump_type)
  if args.write:
    updated = replace_version(content, new_version)
    write_file(config_path, updated)

  print(format_version(new_version))
  return 0


if __name__ == "__main__":
  try:
    raise SystemExit(main())
  except Exception as exc:
    print(str(exc), file=sys.stderr)
    raise SystemExit(1)
