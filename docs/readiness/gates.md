# Readiness Gates — MCSOS 260502

## M2 Gate
Status yang diperbolehkan:
- Belum siap uji QEMU: build, image, atau serial log gagal.
- Siap uji QEMU tahap M2: build ELF, image, QEMU/OVMF, dan serial marker lulus dengan evidence lengkap.

Status yang TIDAK diperbolehkan:
- Siap produksi
- Tanpa error
- Siap hardware umum

## Acceptance Criteria M2
1. kernel.elf adalah ELF64 x86_64 dengan entry point 0xffffffff80000000.
2. build/mcsos.iso berhasil dibuat.
3. build/qemu-serial.log memuat tiga marker:
   - MCSOS 260502 M2 boot path entered
   - [M2] early serial online
   - [M2] kernel reached controlled halt loop
4. Semua inspeksi ELF (readelf, objdump, nm) tersedia.
5. Tidak ada warning kompilasi (-Werror aktif).
