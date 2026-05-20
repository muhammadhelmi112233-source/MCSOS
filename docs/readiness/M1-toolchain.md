# Readiness Review M1 - Toolchain Reproducible

## Identitas
- Nama mahasiswa/kelompok:
- NIM anggota:
- Kelas:
- Dosen: Muhaemin Sidiq, S.Pd., M.Pd.
- Program Studi: Pendidikan Teknologi Informasi, Institut Pendidikan Indonesia
- Tanggal:
- Commit hash:

## Ringkasan hasil
Tuliskan ringkasan singkat hasil M1. Gunakan istilah terukur: `siap untuk M2` hanya bila semua acceptance criteria M1 terpenuhi.

## Evidence checklist
| Evidence | Path | Status | Catatan |
|---|---|---|---|
| Toolchain versions | `build/meta/toolchain-versions.txt` | | |
| Host readiness | `build/meta/host-readiness.txt` | | |
| QEMU capabilities | `build/meta/qemu-capabilities.txt` | | |
| Freestanding object | `build/proof/freestanding_probe.o` | | |
| Freestanding ELF | `build/proof/freestanding_probe.elf` | | |
| ELF header | `build/proof/readelf-header.txt` | | |
| ELF sections | `build/proof/readelf-sections.txt` | | |
| Disassembly | `build/proof/objdump-disassembly.txt` | | |
| Undefined symbol report | `build/proof/nm-undefined.txt` | | |
| Reproducibility hash | `build/repro/sha256-run1.txt`, `build/repro/sha256-run2.txt` | | |

## Acceptance criteria M1
| Kriteria | Lulus/Gagal | Bu
