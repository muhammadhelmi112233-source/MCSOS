# Architecture Overview — MCSOS 260502

## Target Arsitektur
- Arsitektur: x86_64 (AMD64 / Intel 64)
- Mode: Long mode

## Host dan Lingkungan Build
- Host: Windows 11 x64
- Lingkungan build: WSL 2 Linux filesystem (bukan /mnt/c)
- Compiler: Clang dengan target x86_64-unknown-none-elf
- Linker: ld.lld dengan linker script eksplisit

## Model Kernel
- Kernel monolitik pendidikan
- Boundary modular dengan POSIX-like subset
- Bahasa utama: freestanding C17 dengan inline assembly x86_64 minimal

## Emulator dan Firmware
- Emulator: qemu-system-x86_64
- Firmware: OVMF (jalur UEFI)
- Bootloader: Limine binary release branch

## Non-goals
- Tidak ada memory manager pada M2
- Tidak ada interrupt handler pada M2
- Tidak ada userspace pada M2
- Tidak ada filesystem pada M2
- Tidak ada network stack pada M2
- Tidak ada hardware bring-up fisik pada M2

## Status
Kernel monolitik pendidikan tahap awal. Hanya siap uji QEMU tahap boot awal apabila evidence lengkap.
