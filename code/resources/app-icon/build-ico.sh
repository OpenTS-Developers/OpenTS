#!/usr/bin/env bash

set -euo pipefail

icon_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v magick >/dev/null 2>&1; then
	echo "Error: ImageMagick 'magick' was not found." >&2
	exit 1
fi

magick \
	\( -background none "$icon_dir/opents.svg" -resize 256x256 \) \
	\( -background none "$icon_dir/opents.svg" -resize 128x128 \) \
	\( -background none "$icon_dir/opents.svg" -resize 64x64 \) \
	\( -background none "$icon_dir/opents.svg" -resize 48x48 \) \
	\( -background none "$icon_dir/opents-flat.svg" -resize 32x32 \) \
	\( -background none "$icon_dir/opents-flat.svg" -resize 24x24 \) \
	\( -background none "$icon_dir/opents-flat.svg" -resize 16x16 \) \
	"$icon_dir/opents.ico"

echo "Created $icon_dir/opents.ico"
