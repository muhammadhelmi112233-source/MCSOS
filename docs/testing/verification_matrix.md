# Verification Matrix — MCSOS 260502

## M2 Verification

| ID | Komponen | Metode Verifikasi | Artefak Bukti | Status |
|---|---|---|---|---|
| V-M2-01 | Filesystem WSL | m2_preflight.sh | m2-preflight.txt | - |
| V-M2-02 | Toolchain tersedia | m2_preflight.sh | m2-preflight.txt | - |
| V-M2-03 | Dokumen M0 ada | m2_preflight.sh | m2-preflight.txt | - |
| V-M2-04 | Object M1 ELF64 x86_64 | readelf | m2-check-m1-object-readelf.txt | - |
| V-M2-05 | OVMF tersedia | m2_preflight.sh | m2-preflight.txt | - |
| V-M2-06 | kernel.elf terbentuk | make build | build/kernel.elf | - |
| V-M2-07 | kernel.elf ELF64 x86_64 | readelf -hW | readelf-header.txt | - |
| V-M2-08 | Entry point 0xffffffff80000000 | readelf -hW | readelf-header.txt | - |
| V-M2-09 | Symbol kmain ada | nm | nm-symbols.txt | - |
| V-M2-10 | ISO bootable terbentuk | make image | build/mcsos.iso | - |
| V-M2-11 | ISO checksum valid | sha256sum | mcsos.iso.sha256 | - |
| V-M2-12 | Serial marker M2 muncul | make run | qemu-serial.log | - |
| V-M2-13 | Script shell lulus bash -n | make check-scripts | output shell lint | - |

## Readiness Gate
M2 dinyatakan siap uji QEMU tahap boot awal hanya jika semua baris di atas berstatus PASS.
