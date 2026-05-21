# Readiness Review M2 - Boot Image dan Early Serial Console

## Identitas
- Proyek: MCSOS 260502
- Praktikum: M2
- Target: x86_64, QEMU, OVMF, Limine
- Nama/Kelompok: Sihab
- Commit hash: (diisi setelah commit)
- Tanggal: 2026-05-21

## Ringkasan Status
Status yang diajukan: siap uji QEMU tahap M2.
Alasan ringkas: Build ELF, image ISO, QEMU/OVMF, dan serial marker seluruhnya lulus. Evidence matrix lengkap.

## Evidence Matrix
| Evidence | Lokasi | Status | Catatan |
|---|---|---|---|
| Preflight M2 | `build/meta/m2-preflight.txt` | PASS | Semua tool OK |
| Kernel ELF | `build/kernel.elf` | PASS | ELF64 x86_64 |
| Kernel map | `build/kernel.map` | PASS | Symbol boundary tersedia |
| readelf header | `build/inspect/readelf-header.txt` | PASS | Class ELF64, entry 0xffffffff80000000 |
| readelf PHDR | `build/inspect/readelf-program-headers.txt` | PASS | 2 program headers |
| objdump | `build/inspect/objdump-disassembly.txt` | PASS | Disassembly tersedia |
| ISO | `build/mcsos.iso` | PASS | 4.0M bootable |
| ISO checksum | `build/mcsos.iso.sha256` | PASS | SHA-256 tercatat |
| Serial log | `build/qemu-serial.log` | PASS | Ketiga marker M2 muncul |
| Git commit | `build/meta/m2-commit.txt` | PASS | |

## Invariants yang Diperiksa
1. Kernel adalah ELF64 x86_64.
2. Entry point sesuai linker script: 0xffffffff80000000.
3. Kernel tidak memakai hosted libc.
4. Source dikompilasi dengan `-ffreestanding` dan `-mno-red-zone`.
5. Serial console tersedia sebelum subsistem kompleks.
6. Kernel tidak kembali setelah `kmain`.
7. Output QEMU disimpan sebagai log file.

## Failure Modes yang Diuji atau Dianalisis
| Failure mode | Pernah terjadi? | Diagnosis | Perbaikan |
|---|---|---|---|
| Toolchain salah | Tidak | - | - |
| OVMF tidak ditemukan | Ya | OVMF_CODE_4M.secboot.fd terdeteksi duluan | Update find_first di run_qemu.sh |
| Limine gagal fetch | Tidak | - | - |
| ISO gagal dibuat | Tidak | - | - |
| QEMU log kosong | Ya | OVMF path salah | Perbaiki urutan pencarian OVMF |
| Entry point salah | Tidak | - | - |
| Reboot loop | Tidak | - | - |
| CRLF script | Tidak | - | - |

## Keputusan Readiness
- [x] Lulus M2: siap uji QEMU tahap M2.
- [ ] Belum lulus M2: perlu perbaikan.

## Catatan Reviewer
Seluruh checkpoint CP-M2.1 sampai CP-M2.8 lulus. Serial log memuat ketiga marker wajib M2.
OVMF yang digunakan: /usr/share/OVMF/OVMF_CODE_4M.fd
Limine branch: v8.x-binary
