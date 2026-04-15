# AlgoBrix Arduino Package

This repository contains the Arduino Boards Manager package for the AlgoBrix AVR platform and the bundled firmware example used by the board.

## Install

Add this URL to Arduino IDE `File > Preferences > Additional Boards Manager URLs`:

`https://raw.githubusercontent.com/Algobrix/arduino_algobot_board/master/package_Algobot_index.json`

Then open `Tools > Board > Boards Manager`, search for `algobot`, and install the package.

## Repository Layout

The repository root is the Arduino platform package content.

Important files and folders:

- `platform.txt`
- `boards.txt`
- `bootloaders/`
- `variants/`
- `tools/`
- `libraries/`
- `package_Algobot_index.json`
- `package_Algobot_dev_index.json`
- `.github/workflows/publish-releases.yml`
- `.github/workflows/publish-dev-package.yml`
- `scripts/build-package.sh`
- `scripts/update-package-index.sh`

## Development Workflow

For normal development:

1. Make changes directly in the repository root.
2. Update board support files, libraries, examples, or firmware as needed.
3. Test locally in Arduino IDE or with your normal firmware workflow.
4. Commit and push changes to the default branch.

Notes:

- The repository is both the source for development and the source used to build release ZIP packages.
- You do not need to manually build `algobot_pack_<version>.zip` during normal development.
- `package_Algobot_index.json` currently keeps `0.3.16` as the in-repo package entry. Newer versions are intended to come from GitHub Releases.

## Firmware Versioning

The firmware example version is defined in:

`libraries/Tangible_and_GoAlgo/examples/Tangible_and_GoAlgo_Firmware/fwver.h`

The firmware uses:

- `FIRMWARE_VERSION_MAJOR`
- `FIRMWARE_VERSION_MINOR`
- `FIRMWARE_VERSION_STRING`
- `FIRMWARE_VERSION`

`FIRMWARE_VERSION` is kept as a single byte for BLE compatibility:

- high nibble = major
- low nibble = minor

The firmware startup log prints the full firmware version as:

`major.minor`

## Release Workflow

Releases are automated through:

`/.github/workflows/publish-releases.yml`

Versioning is linked to the GitHub release tag. When a release is published, the workflow automatically derives the version from the tag, updates the packaged platform files, updates the firmware version header, builds the ZIP, and publishes the new package entry.

When a GitHub Release is published:

1. The workflow reads the release tag.
2. The version is derived from the tag by removing the leading `v`.
3. A package ZIP named `algobot_pack_<version>.zip` is created.
4. `platform.txt` inside the packaged ZIP is updated to the release version.
5. `fwver.h` inside the packaged ZIP is updated from the release version:
   - package version uses full `major.minor.patch`
   - firmware version uses only `major.minor`
6. The ZIP is uploaded as a GitHub Release asset.
7. `package_Algobot_index.json` is updated to prepend the new release entry using the GitHub Release asset URL.
8. The updated package index is committed back to the repository default branch.

## How To Create A New Release

Use this process for every public package release:

1. Make sure your development changes are committed and pushed.
2. Create a git tag using semantic versioning, for example:
   `v0.3.17`
3. Publish a GitHub Release for that tag.
4. GitHub Actions will:
   - build `algobot_pack_0.3.17.zip`
   - upload it to the release
   - update `package_Algobot_index.json`

## Version Rules

Package version:

- comes from the release tag
- uses `major.minor.patch`

Firmware version:

- also comes from the release tag
- uses only `major.minor`
- example: release tag `v0.3.17` becomes firmware version `0.3`

## Important Behavior

- The release workflow updates version values inside the generated package ZIP automatically.
- It does not need manual version edits before the release for packaging.
- The checked-in source files do not have to be manually changed just to produce a release package.

## Scripts

### `scripts/build-package.sh`

Builds the Arduino package ZIP.

Behavior:

- supports both repository-root layout and legacy `src/` layout
- updates packaged `platform.txt`
- updates packaged `fwver.h`

### `scripts/update-package-index.sh`

Updates `package_Algobot_index.json` with:

- version
- size
- checksum
- archive file name
- release asset URL

## Summary

Development happens in the repo.

Releases happen from a GitHub Release tag.

The workflow builds the ZIP, updates package metadata, updates firmware/package version values inside the artifact, uploads the ZIP, and publishes the new package entry automatically.

## Dev Package Channel (Team Sharing)

If you want to share in-progress builds with your team, use the separate dev package index:

`https://raw.githubusercontent.com/Algobrix/arduino_algobot_board/dev-package-index/package_Algobot_dev_index.json`

This channel is independent from stable releases and does not change `package_Algobot_index.json`.

### How to publish a dev package

1. Open GitHub `Actions`.
2. Run workflow: `Publish Dev Package`.
3. Optionally provide a custom version (`x.y.z`), or leave empty to auto-generate `0.0.<run_number>`.
4. The workflow:
   - builds `algobot_pack_<version>.zip`
   - updates `package_Algobot_dev_index.json`
   - pushes ZIP + index to branch `dev-package-index`

Team members can add the dev URL above to Additional Boards Manager URLs to install the latest shared dev build.
