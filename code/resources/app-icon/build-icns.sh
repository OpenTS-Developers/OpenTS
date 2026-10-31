#!/usr/bin/env bash

# Build a macOS .icns from a square PNG. sips and iconutil ship with macOS.
set -euo pipefail

if [[ $# -ne 2 ]]; then
	echo "usage: build-icns.sh <source.png> <output.icns>" >&2
	exit 1
fi

src=$1
out=$2

if ! command -v sips >/dev/null 2>&1 || ! command -v iconutil >/dev/null 2>&1; then
	echo "sips and iconutil are required." >&2
	exit 1
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
iconset="$work/siliconsun.iconset"
mkdir -p "$iconset"

# slot pixel-size. @2x slots are the doubled bitmap Apple expects.
sizes=(
	"16x16 16"
	"16x16@2x 32"
	"32x32 32"
	"32x32@2x 64"
	"128x128 128"
	"128x128@2x 256"
	"256x256 256"
	"256x256@2x 512"
	"512x512 512"
	"512x512@2x 1024"
)

for entry in "${sizes[@]}"; do
	name=${entry%% *}
	px=${entry##* }
	sips -z "$px" "$px" "$src" --out "$iconset/icon_${name}.png" >/dev/null
done

mkdir -p "$(dirname -- "$out")"
iconutil -c icns "$iconset" -o "$out"
echo "Created $out"
