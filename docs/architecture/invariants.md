# Architecture Invariants — MCSOS 260502

## Boot Path Invariants
1. Firmware OVMF menyiapkan platform sebelum bootloader berjalan.
2. Limine memuat kernel ELF64 dari ISO bootable.
3. Kernel entry point adalah kmain pada alamat higher-half 0xffffffff80000000.
4. Kernel tidak kembali setelah kmain; halt_forever dijalankan.
5. Serial console COM1 diinisialisasi sebelum subsistem lain.

## Build Invariants
1. Semua source dikompilasi dengan -ffreestanding dan -mno-red-zone.
2. Tidak ada dependency pada hosted libc.
3. Compiler target adalah x86_64-unknown-none-elf.
4. Linker adalah ld.lld dengan linker script eksplisit.

## ABI Invariants
1. Fungsi C menggunakan x86_64 System V calling convention.
2. Red zone dinonaktifkan (-mno-red-zone).
3. SIMD/FPU tidak digunakan pada M2.
4. Kernel tidak bergantung pada main, libc, exception runtime, unwinder, atau dynamic loader.

## State Transition
OVMF -> Limine -> kernel.elf -> kmain -> serial_init -> serial_write -> halt_forever
