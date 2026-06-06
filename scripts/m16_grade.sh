#!/usr/bin/env bash
set -euo pipefail
mkdir -p logs/m16 evidence/m16
{
  echo "== M16 grade =="
  date -Iseconds
  echo "== C2 host test =="
  make -C tests/m16 clean host 2>&1
  echo "== C4 nm undefined =="
  cat evidence/m16/nm_undefined.txt && echo "(kosong - OK)" || echo "FAIL: ada undefined symbol"
  echo "== C5 readelf =="
  grep -E 'Class|Type|Machine' evidence/m16/readelf_header.txt
  echo "== C7 sha256sum =="
  cat evidence/m16/sha256sum.txt
  echo "== git log =="
  git log --oneline -5
} | tee logs/m16/m16_grade.log
