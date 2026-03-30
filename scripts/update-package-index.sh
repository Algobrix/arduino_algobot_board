#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 4 ]]; then
  echo "Usage: $0 <package-index.json> <template.json> <archive.zip> <download-url>" >&2
  exit 1
fi

PACKAGE_INDEX_FILE="$1"
TEMPLATE_FILE="$2"
ARCHIVE_FILE="$3"
DOWNLOAD_URL="$4"

if [[ ! -f "$PACKAGE_INDEX_FILE" ]]; then
  echo "Package index file not found: $PACKAGE_INDEX_FILE" >&2
  exit 1
fi

if [[ ! -f "$TEMPLATE_FILE" ]]; then
  echo "Template file not found: $TEMPLATE_FILE" >&2
  exit 1
fi

if [[ ! -f "$ARCHIVE_FILE" ]]; then
  echo "Archive file not found: $ARCHIVE_FILE" >&2
  exit 1
fi

VERSION="$(unzip -Z -1 "$ARCHIVE_FILE" | head -n 1 | cut -d '/' -f 1)"
SIZE="$(stat -c '%s' "$ARCHIVE_FILE")"
SHA256="$(sha256sum "$ARCHIVE_FILE" | cut -d ' ' -f 1)"
ARCHIVE_NAME="$(basename "$ARCHIVE_FILE")"

if [[ -z "$VERSION" ]]; then
  echo "Failed to detect version from $ARCHIVE_FILE" >&2
  exit 1
fi

PLATFORM_JSON="$(
  sed '/^#/d' "$TEMPLATE_FILE" \
    | jq \
        --arg version "$VERSION" \
        --arg size "$SIZE" \
        --arg checksum "SHA-256:$SHA256" \
        --arg url "$DOWNLOAD_URL" \
        --arg archiveFileName "$ARCHIVE_NAME" \
        '.version = $version
        | .size = $size
        | .checksum = $checksum
        | .url = $url
        | .archiveFileName = $archiveFileName'
)"

if jq --exit-status --arg version "$VERSION" \
  'any(.packages[0].platforms[]; .version == $version)' \
  "$PACKAGE_INDEX_FILE" > /dev/null; then
  echo "Version $VERSION already exists in $PACKAGE_INDEX_FILE" >&2
  exit 1
fi

TMP_FILE="$(mktemp)"

jq --argjson platform "$PLATFORM_JSON" \
  '.packages[0].platforms = [$platform] + .packages[0].platforms' \
  "$PACKAGE_INDEX_FILE" > "$TMP_FILE"

mv "$TMP_FILE" "$PACKAGE_INDEX_FILE"

echo "Updated $PACKAGE_INDEX_FILE with version $VERSION"
