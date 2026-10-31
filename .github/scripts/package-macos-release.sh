#!/usr/bin/env bash

# Stage siliconsun.app, sign it, and zip it for a GitHub release.
# With no Apple secrets the signature is ad-hoc. With the Developer ID certificate
# and the App Store Connect API key, the app is notarized and the ticket is stapled
# before the zip is written.
set -euo pipefail

if [[ $# -ne 6 ]]; then
	echo "usage: package-macos-release.sh <game-binary> <ui-dir> <icon-png> <version> <dest-dir> <zip-path>" >&2
	exit 1
fi

binary=$1
ui=$2
png=$3
version=$4
dest=$5
zip_path=$6

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)

keychain=""
p12=""
keyfile=""
notarize_zip=""
previous=()
while IFS= read -r line; do
	line="${line#"${line%%[![:space:]]*}"}"
	line="${line%\"}"
	line="${line#\"}"
	[[ -n "$line" ]] && previous+=("$line")
done < <(security list-keychains -d user)

cleanup() {
	if [[ -n "$keychain" ]]; then
		security delete-keychain "$keychain" >/dev/null 2>&1 || true
	fi
	if [[ ${#previous[@]} -gt 0 ]]; then
		security list-keychains -d user -s "${previous[@]}" >/dev/null 2>&1 || true
	fi
	rm -f "$p12" "$keyfile" "$notarize_zip"
}
trap cleanup EXIT

identity="-"
if [[ -n "${APPLE_CERTIFICATE_P12:-}" ]]; then
	missing=()
	[[ -n "${APPLE_CERTIFICATE_PASSWORD:-}" ]] || missing+=(APPLE_CERTIFICATE_PASSWORD)
	[[ -n "${APPLE_API_KEY:-}" ]] || missing+=(APPLE_API_KEY)
	[[ -n "${APPLE_API_KEY_ID:-}" ]] || missing+=(APPLE_API_KEY_ID)
	[[ -n "${APPLE_API_ISSUER:-}" ]] || missing+=(APPLE_API_ISSUER)
	if [[ ${#missing[@]} -gt 0 ]]; then
		echo "Developer ID signing needs ${missing[*]}." >&2
		exit 1
	fi

	p12=$(mktemp)
	keychain=$(mktemp)
	rm -f "$keychain"
	keychain="${keychain}.keychain-db"
	keychain_password=$(openssl rand -base64 24)
	printf '%s' "$APPLE_CERTIFICATE_P12" | base64 -D > "$p12"
	security create-keychain -p "$keychain_password" "$keychain"
	security set-keychain-settings -lut 21600 "$keychain"
	security unlock-keychain -p "$keychain_password" "$keychain"
	security import "$p12" -k "$keychain" -P "$APPLE_CERTIFICATE_PASSWORD" -T /usr/bin/codesign -T /usr/bin/security
	security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$keychain_password" "$keychain"
	if [[ ${#previous[@]} -gt 0 ]]; then
		security list-keychains -d user -s "$keychain" "${previous[@]}"
	else
		security list-keychains -d user -s "$keychain"
	fi

	identity=$(security find-identity -v -p codesigning | sed -n 's/.*"\(Developer ID Application:[^"]*\)"/\1/p' | head -1)
	if [[ -z "$identity" ]]; then
		echo "The imported certificate has no Developer ID Application identity." >&2
		exit 1
	fi
	echo "Signing with $identity"
fi

CODESIGN_IDENTITY="$identity" bash "$root/.github/scripts/stage-macos-app.sh" \
	"$binary" "$ui" "$png" "$version" "$dest"

app="$dest/siliconsun.app"
if [[ "$identity" != "-" ]]; then
	notarize_zip=$(mktemp)
	rm -f "$notarize_zip"
	notarize_zip="${notarize_zip}.zip"
	ditto -c -k --keepParent "$app" "$notarize_zip"
	keyfile=$(mktemp)
	chmod 600 "$keyfile"
	printf '%s' "$APPLE_API_KEY" > "$keyfile"
	[[ "$APPLE_API_KEY" == *$'\n' ]] || printf '\n' >> "$keyfile"
	xcrun notarytool submit "$notarize_zip" \
		--key "$keyfile" \
		--key-id "$APPLE_API_KEY_ID" \
		--issuer "$APPLE_API_ISSUER" \
		--wait
	xcrun stapler staple "$app"
	xcrun stapler validate "$app"
fi

bash "$root/.github/scripts/stage-notices.sh" "$dest"
rm -f "$zip_path"
ditto -c -k "$dest" "$zip_path"
echo "Wrote $zip_path"
