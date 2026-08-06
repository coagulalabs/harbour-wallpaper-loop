#!/usr/bin/env bash
# Deploy harbour-wallpaper-loop to a Sailfish device over SSH.
# Usage: DEVICE=defaultuser@192.168.2.15 ./scripts/deploy.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEVICE="${DEVICE:-defaultuser@192.168.2.15}"
TARGET="${TARGET:-SailfishOS-5.0.0.62-aarch64}"

cd "$ROOT"
sfdk config target="$TARGET"
sfdk build

RPM=$(find "$ROOT" -path '*/RPMS/*' -name 'harbour-wallpaper-loop-*.rpm' ! -name '*.src.rpm' | sort | tail -n1)
if [[ -z "${RPM}" ]]; then
  echo "No RPM found after build" >&2
  exit 1
fi

echo "Deploying $RPM -> $DEVICE"
scp "$RPM" "$DEVICE:/tmp/"
ssh "$DEVICE" "devel-su rpm -Uvh --force /tmp/$(basename "$RPM")"
echo "Done. Launch Wallpaper Loop from the app grid."
