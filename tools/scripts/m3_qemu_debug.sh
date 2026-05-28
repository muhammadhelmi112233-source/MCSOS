#!/usr/bin/env bash
set -Eeuo pipefail

ISO="${1:-build/mcsos.iso}"
OVMF_CODE="${OVMF_CODE:-/usr/share/OVMF/OVMF_CODE_4M.fd}"
OVMF_VARS_TEMPLATE="${OVMF_VARS_TEMPLATE:-/usr/share/OVMF/OVMF_VARS_4M.fd}"
OVMF_VARS="build/OVMF_VARS.fd"

test -f "$ISO" || { echo "FAIL: ISO tidak ditemukan: $ISO" >&2; exit 1; }
cp "$OVMF_VARS_TEMPLATE" "$OVMF_VARS"

exec qemu-system-x86_64 \
  -machine q35 \
  -cpu qemu64 \
  -m 512M \
  -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
  -drive if=pflash,format=raw,file="$OVMF_VARS" \
  -cdrom "$ISO" \
  -serial mon:stdio \
  -display none \
  -no-reboot \
  -no-shutdown \
  -s -S
