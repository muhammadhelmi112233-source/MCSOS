#!/usr/bin/env bash
set -euo pipefail

mkdir -p build

: "${CC:=clang}"
: "${HOSTCC:=clang}"

echo "[M6] Step 1: compile pmm.o freestanding..."
${CC} --target=x86_64-unknown-none-elf \
  -std=c17 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check \
  -fno-pic -fno-pie -fno-lto -m64 -march=x86-64 -mno-red-zone \
  -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
  -Ikernel/include \
  -c kernel/mm/pmm.c -o build/pmm.o
echo "[M6] pmm.o built OK"

echo "[M6] Step 2: build host unit test..."
${HOSTCC} -std=c17 -Wall -Wextra -Werror \
  -Ikernel/include \
  kernel/mm/pmm.c tests/test_pmm_host.c -o build/test_pmm_host
echo "[M6] test_pmm_host built OK"

echo "[M6] Step 3: run host unit test..."
./build/test_pmm_host

echo "[M6] Step 4: freestanding symbol audit..."
nm -u build/pmm.o | tee build/pmm.undefined.txt
if grep -q . build/pmm.undefined.txt; then
  echo "[FAIL] pmm.o masih memiliki unresolved symbol" >&2
  exit 1
fi
echo "[M6] nm -u audit: OK (kosong)"

echo "[M6] Step 5: disassembly..."
objdump -dr build/pmm.o > build/pmm.objdump.txt
echo "[M6] disassembly saved to build/pmm.objdump.txt"

echo "[PASS] M6 static check selesai"
