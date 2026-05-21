# Threat Model — MCSOS 260502

## Scope
Threat model ini mencakup jalur pengembangan dan boot awal M2.

## Aset Utama
- Source code kernel
- Toolchain (Clang, LLD)
- kernel.elf dan image ISO
- Konfigurasi Limine
- Log evidence

## Ancaman yang Relevan pada M2
1. Dependency bootloader tidak terverifikasi (supply-chain Limine).
2. Path repository tidak deterministik (bekerja di /mnt/c).
3. Script shell rusak karena CRLF Windows.
4. QEMU command tidak terdokumentasi.
5. Klaim readiness yang melebihi bukti.

## Mitigasi M2
1. Pencatatan revision dan branch Limine.
2. Checksum ISO (SHA-256).
3. Preflight script untuk deteksi dini.
4. Generated artifact policy (build/ tidak dikomit).
5. Git commit evidence.
6. Status readiness terbatas: hanya "siap uji QEMU tahap M2".

## Luar Scope M2
- User/kernel isolation
- Credential dan capability
- MAC/DAC
- Secure boot dan measured boot
- Update integrity
