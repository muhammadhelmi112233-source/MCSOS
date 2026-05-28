#!/usr/bin/env bash
set -Eeuo pipefail

ISO="${1:-build/mcsos.iso}"
LOG="${2:-build/m3_serial.log}"
TIMEOUT_SEC="${MCSOS_QEMU_TIMEOUT:-30}"
OVMF_CODE="${OVMF_CODE:-/usr/share/OVMF/OVMF_CODE_4M.fd}"
OVMF_VARS_TEMPLATE="${OVMF_VARS_TEMPLATE:-/usr/share/OVMF/OVMF_VARS_4M.fd}"
OVMF_VARS="build/OVMF_VARS.fd"

fail() { echo "FAIL: $*" >&2; exit 1; }

test -f "$ISO" || fail "ISO tidak ditemukan: $ISO"
command -v qemu-system-x86_64 >/dev/null 2>&1 || fail "qemu-system-x86_64 tidak ditemukan"
test -f "$OVMF_CODE" || fail "OVMF_CODE tidak ditemukan: $OVMF_CODE"
test -f "$OVMF_VARS_TEMPLATE" || fail "OVMF_VARS_TEMPLATE tidak ditemukan: $OVMF_VARS_TEMPLATE"

mkdir -p build "$(dirname "$LOG")"
cp "$OVMF_VARS_TEMPLATE" "$OVMF_VARS"
rm -f "$LOG"

timeout "$TIMEOUT_SEC" qemu-system-x86_64 \
  -machine q35 \
  -cpu qemu64 \
  -m 512M \
  -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
  -drive if=pflash,format=raw,file="$OVMF_VARS" \
  -cdrom "$ISO" \
  -serial file:"$LOG" \
  -display none \
  -monitor none \
  -no-reboot \
  -no-shutdown || true

cat "$LOG"
grep -q 'MCSOS 260502 M3 kernel entered' "$LOG" || fail "log boot M3 tidak ditemukan"
grep -q '\[M3\] selftest: basic invariants passed' "$LOG" || fail "selftest M3 tidak lulus"
echo "PASS: QEMU smoke test M3 selesai"
