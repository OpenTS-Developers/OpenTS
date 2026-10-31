#!/usr/bin/env bash

# Assemble siliconsun.app from a built Game binary, the ui directory beside it, and the icon PNG.
set -euo pipefail

if [[ $# -ne 5 ]]; then
	echo "usage: stage-macos-app.sh <game-binary> <ui-dir> <icon-png> <version> <dest-dir>" >&2
	exit 1
fi

binary=$1
ui=$2
png=$3
version=$4
dest=$5

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)

if [[ ! -f "$binary" ]]; then
	echo "game binary not found: $binary" >&2
	exit 1
fi
if [[ ! -d "$ui" ]]; then
	echo "ui directory not found: $ui" >&2
	exit 1
fi
if [[ ! -f "$png" ]]; then
	echo "icon not found: $png" >&2
	exit 1
fi
if [[ -z "$version" || "$version" == *"/"* ]]; then
	echo "version is empty or contains a slash: $version" >&2
	exit 1
fi

app="$dest/siliconsun.app"
macos="$app/Contents/MacOS"
resources="$app/Contents/Resources"

rm -rf "$app"
mkdir -p "$macos" "$resources"

cp "$binary" "$macos/Game"
chmod +x "$macos/Game"
# codesign rejects files that are not code under Contents/MacOS. The game reads this
# copy from Contents/Resources when it is launched from the bundle.
cp -R "$ui" "$resources/ui"
bash "$root/code/resources/app-icon/build-icns.sh" "$png" "$resources/siliconsun.icns"
sed "s/__VERSION__/${version}/g" "$root/.github/macos/Info.plist.in" > "$app/Contents/Info.plist"
plutil -lint "$app/Contents/Info.plist" >/dev/null

# "-" is an ad-hoc signature. A Developer ID identity also gets a timestamp and the
# hardened runtime, which notarization requires.
identity=${CODESIGN_IDENTITY:--}
sign_args=(--force --sign "$identity")
if [[ "$identity" != "-" ]]; then
	sign_args+=(--options runtime --timestamp)
fi
codesign "${sign_args[@]}" "$macos/Game"
codesign "${sign_args[@]}" "$app"
codesign --verify --strict "$app"

echo "Staged $app"
