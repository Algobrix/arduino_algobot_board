#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIR="${1:-}"

if [[ -z "$SOURCE_DIR" ]]; then
  if [[ -f "$ROOT_DIR/platform.txt" ]]; then
    SOURCE_DIR="$ROOT_DIR"
  elif [[ -f "$ROOT_DIR/src/platform.txt" ]]; then
    SOURCE_DIR="$ROOT_DIR/src"
  else
    echo "platform.txt not found in repository root or src/" >&2
    exit 1
  fi
fi

if [[ ! -f "$SOURCE_DIR/platform.txt" ]]; then
  echo "platform.txt not found in $SOURCE_DIR" >&2
  exit 1
fi

VERSION="${VERSION_OVERRIDE:-}"
PLATFORM_NAME_OVERRIDE="${PLATFORM_NAME_OVERRIDE:-}"
BOARD_NAME_OVERRIDE="${BOARD_NAME_OVERRIDE:-}"
BOARD_KEY_OVERRIDE="${BOARD_KEY_OVERRIDE:-}"

if [[ -z "$VERSION" ]]; then
  VERSION="$(grep '^version=' "$SOURCE_DIR/platform.txt" | head -n 1 | cut -d '=' -f 2-)"
fi

if [[ -z "$VERSION" ]]; then
  echo "Failed to read version from $SOURCE_DIR/platform.txt" >&2
  exit 1
fi

BUILD_DIR="$(mktemp -d)"
PACKAGE_DIR="$BUILD_DIR/$VERSION"
ARCHIVE_NAME="algobot_pack_${VERSION}.zip"
ARCHIVE_PATH="$ROOT_DIR/$ARCHIVE_NAME"

cleanup() {
  rm -rf "$BUILD_DIR"
}
trap cleanup EXIT

mkdir -p "$PACKAGE_DIR"

if [[ "$SOURCE_DIR" == "$ROOT_DIR" ]]; then
  for entry in platform.txt boards.txt programmers.txt bootloaders variants libraries cores tools; do
    if [[ -e "$SOURCE_DIR/$entry" ]]; then
      cp -a "$SOURCE_DIR/$entry" "$PACKAGE_DIR/"
    fi
  done
else
  cp -a "$SOURCE_DIR"/. "$PACKAGE_DIR"/
fi

if grep -q '^version=' "$PACKAGE_DIR/platform.txt"; then
  sed -i "s/^version=.*/version=$VERSION/" "$PACKAGE_DIR/platform.txt"
else
  echo "version entry not found in $PACKAGE_DIR/platform.txt" >&2
  exit 1
fi

if [[ -n "$PLATFORM_NAME_OVERRIDE" ]]; then
  if grep -q '^name=' "$PACKAGE_DIR/platform.txt"; then
    sed -i "s/^name=.*/name=$PLATFORM_NAME_OVERRIDE/" "$PACKAGE_DIR/platform.txt"
  else
    echo "name entry not found in $PACKAGE_DIR/platform.txt" >&2
    exit 1
  fi
fi

BOARDS_FILE="$PACKAGE_DIR/boards.txt"
if [[ -f "$BOARDS_FILE" ]]; then
  BOARD_KEY="algobotpb"
  if [[ -n "$BOARD_KEY_OVERRIDE" && "$BOARD_KEY_OVERRIDE" != "algobotpb" ]]; then
    if grep -q '^algobotpb\.' "$BOARDS_FILE"; then
      sed -i "s/^algobotpb\./${BOARD_KEY_OVERRIDE}./" "$BOARDS_FILE"
      BOARD_KEY="$BOARD_KEY_OVERRIDE"
    else
      echo "algobotpb.* entries not found in $BOARDS_FILE" >&2
      exit 1
    fi
  fi

  if [[ -n "$BOARD_NAME_OVERRIDE" ]]; then
    if grep -q "^${BOARD_KEY}\.name=" "$BOARDS_FILE"; then
      sed -i "s/^${BOARD_KEY}\.name=.*/${BOARD_KEY}.name=$BOARD_NAME_OVERRIDE/" "$BOARDS_FILE"
    else
      echo "${BOARD_KEY}.name entry not found in $BOARDS_FILE" >&2
      exit 1
    fi
  fi
fi

FWVER_FILE="$PACKAGE_DIR/libraries/Tangible_and_GoAlgo/examples/Tangible_and_GoAlgo_Firmware/fwver.h"
if [[ -f "$FWVER_FILE" ]]; then
  IFS='.' read -r VERSION_MAJOR VERSION_MINOR VERSION_PATCH <<< "$VERSION"

  if [[ -z "${VERSION_MAJOR:-}" || -z "${VERSION_MINOR:-}" ]]; then
    echo "Expected semantic version x.y.z, got '$VERSION'" >&2
    exit 1
  fi

  sed -i \
    -e "s/^#define FIRMWARE_VERSION_MAJOR.*/#define FIRMWARE_VERSION_MAJOR        $VERSION_MAJOR/" \
    -e "s/^#define FIRMWARE_VERSION_MINOR.*/#define FIRMWARE_VERSION_MINOR        $VERSION_MINOR/" \
    -e "s/^#define FIRMWARE_VERSION_STRING.*/#define FIRMWARE_VERSION_STRING       \"$VERSION_MAJOR.$VERSION_MINOR\"/" \
    "$FWVER_FILE"
fi

rm -f "$ARCHIVE_PATH"

(
  cd "$BUILD_DIR"
  zip -qr "$ARCHIVE_PATH" "$VERSION"
)

echo "VERSION=$VERSION"
echo "ARCHIVE_NAME=$ARCHIVE_NAME"
echo "ARCHIVE_PATH=$ARCHIVE_PATH"
