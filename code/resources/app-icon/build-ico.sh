#!/usr/bin/env bash

set -euo pipefail

icon_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd -- "$icon_dir/../../.." && pwd)"

if command -v magick >/dev/null 2>&1; then
	im=(magick)
elif command -v convert >/dev/null 2>&1; then
	im=(convert)
else
	echo "Error: ImageMagick 'magick' was not found." >&2
	exit 1
fi

src="$icon_dir/siliconsun.png"
# Each size is stored as a bitmap, which the resource compiler accepts.
"${im[@]}" \
	\( "$src" -background none -resize 256x256 -compress none \) \
	\( "$src" -background none -resize 128x128 -compress none \) \
	\( "$src" -background none -resize 64x64 -compress none \) \
	\( "$src" -background none -resize 48x48 -compress none \) \
	\( "$src" -background none -resize 32x32 -compress none \) \
	\( "$src" -background none -resize 24x24 -compress none \) \
	\( "$src" -background none -resize 16x16 -compress none \) \
	"$icon_dir/siliconsun.ico"

cp "$src" "$root/ui/icon.png"

echo "Created $icon_dir/siliconsun.ico and $root/ui/icon.png"
