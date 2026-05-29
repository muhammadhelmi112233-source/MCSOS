.RECIPEPREFIX := >
SHELL := /usr/bin/env bash
BUILD_DIR := build
KERNEL := $(BUILD_DIR)/kernel.elf
BP_KERNEL := $(BUILD_DIR)/kernel.breakpoint.elf
PANIC_KERNEL := $(BUILD_DIR)/kernel.panic.elf
MAP := $(BUILD_DIR)/kernel.map
BP_MAP := $(BUILD_DIR)/kernel.breakpoint.map
PANIC_MAP := $(BUILD_DIR)/kernel.panic.map
DISASM := $(BUILD_DIR)/kernel.disasm.txt
SYMS := $(BUILD_DIR)/kernel.syms.txt
CC := clang
LD := ld.lld
OBJDUMP := objdump
READELF := readelf
NM := nm
COMMON_CFLAGS := --target=x86_64-unknown-none-elf -std=c17 -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check -fno-pic -fno-pie -fno-lto -m64 -march=x86-64 -mabi=sysv -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel -Wall -Wextra -Werror -Ikernel/arch/x86_64/include -Ikernel/include -I.
COMMON_ASFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-pic -fno-pie -m64 -mno-red-zone -Wall -Wextra -Werror -Ikernel/arch/x86_64/include -Ikernel/include -I.
CFLAGS := $(COMMON_CFLAGS)
ASFLAGS := $(COMMON_ASFLAGS)
BP_CFLAGS := $(COMMON_CFLAGS) -DMCSOS_M4_TRIGGER_BREAKPOINT=1
PANIC_CFLAGS := $(COMMON_CFLAGS) -DMCSOS_M4_TRIGGER_PANIC=1
LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T linker.ld
SRC_C := $(shell find kernel -name '*.c' | LC_ALL=C sort)
SRC_S := $(shell find kernel -name '*.S' | LC_ALL=C sort)
OBJ := $(patsubst %.c,$(BUILD_DIR)/normal/%.o,$(SRC_C)) $(patsubst %.S,$(BUILD_DIR)/normal/%.o,$(SRC_S))
BP_OBJ := $(patsubst %.c,$(BUILD_DIR)/breakpoint/%.o,$(SRC_C)) $(patsubst %.S,$(BUILD_DIR)/breakpoint/%.o,$(SRC_S))
PANIC_OBJ := $(patsubst %.c,$(BUILD_DIR)/panic/%.o,$(SRC_C)) $(patsubst %.S,$(BUILD_DIR)/panic/%.o,$(SRC_S))
.PHONY: all build breakpoint panic inspect audit clean distclean
all: build inspect
build: $(KERNEL)
breakpoint: $(BP_KERNEL)
panic: $(PANIC_KERNEL)
$(BUILD_DIR)/normal/%.o: %.c
>mkdir -p $(dir $@)
>$(CC) $(CFLAGS) -c $< -o $@
$(BUILD_DIR)/normal/%.o: %.S
>mkdir -p $(dir $@)
>$(CC) $(ASFLAGS) -c $< -o $@
$(BUILD_DIR)/breakpoint/%.o: %.c
>mkdir -p $(dir $@)
>$(CC) $(BP_CFLAGS) -c $< -o $@
$(BUILD_DIR)/breakpoint/%.o: %.S
>mkdir -p $(dir $@)
>$(CC) $(ASFLAGS) -c $< -o $@
$(BUILD_DIR)/panic/%.o: %.c
>mkdir -p $(dir $@)
>$(CC) $(PANIC_CFLAGS) -c $< -o $@
$(BUILD_DIR)/panic/%.o: %.S
>mkdir -p $(dir $@)
>$(CC) $(ASFLAGS) -c $< -o $@
$(KERNEL): $(OBJ) linker.ld
>mkdir -p $(BUILD_DIR)
>$(LD) $(LDFLAGS) -Map=$(MAP) -o $@ $(OBJ)
$(BP_KERNEL): $(BP_OBJ) linker.ld
>mkdir -p $(BUILD_DIR)
>$(LD) $(LDFLAGS) -Map=$(BP_MAP) -o $@ $(BP_OBJ)
$(PANIC_KERNEL): $(PANIC_OBJ) linker.ld
>mkdir -p $(BUILD_DIR)
>$(LD) $(LDFLAGS) -Map=$(PANIC_MAP) -o $@ $(PANIC_OBJ)
inspect: $(KERNEL)
>$(READELF) -h $(KERNEL) > $(BUILD_DIR)/kernel.readelf.header.txt
>$(READELF) -l $(KERNEL) > $(BUILD_DIR)/kernel.readelf.programs.txt
>$(NM) -n $(KERNEL) > $(SYMS)
>$(OBJDUMP) -d -Mintel $(KERNEL) > $(DISASM)
>grep -q 'ELF64' $(BUILD_DIR)/kernel.readelf.header.txt
>grep -q 'Machine:[[:space:]]*Advanced Micro Devices X86-64' $(BUILD_DIR)/kernel.readelf.header.txt
>grep -q 'kmain' $(SYMS)
>grep -q 'x86_64_idt_init' $(SYMS)
>grep -q 'x86_64_trap_dispatch' $(SYMS)
>grep -q 'iretq' $(DISASM)
>grep -q 'lidt' $(DISASM)
audit: inspect breakpoint panic
>! $(NM) -u $(KERNEL) | grep .
>! $(NM) -u $(BP_KERNEL) | grep .
>! $(NM) -u $(PANIC_KERNEL) | grep .
>grep -q 'isr_stub_14' $(SYMS)
>grep -q 'x86_64_exception_stubs' $(SYMS)
>$(READELF) -S $(KERNEL) | grep -q '.text'
>$(READELF) -S $(KERNEL) | grep -q '.rodata'
clean:
>rm -rf $(BUILD_DIR)
distclean: clean
>rm -rf iso_root limine evidence


# ---- M7 VMM host test targets ----
HOSTCC ?= cc
HOST_CFLAGS_M7 := -std=c17 -Wall -Wextra -Werror -DMCSOS_HOST_TEST -Ikernel/include
VMM_FREESTANDING_CFLAGS := --target=x86_64-unknown-none-elf \
  -std=c17 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-stack-check \
  -fno-pic -fno-pie -fno-lto -m64 -march=x86-64 -mno-red-zone \
  -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel \
  -Ikernel/include

build/vmm.o: kernel/mm/vmm.c kernel/include/mcsos/kernel/vmm.h kernel/include/mcsos/kernel/types.h
>mkdir -p build
>$(CC) $(VMM_FREESTANDING_CFLAGS) -c kernel/mm/vmm.c -o build/vmm.o

build/test_vmm_host: kernel/mm/vmm.c tests/test_vmm_host.c kernel/include/mcsos/kernel/vmm.h
>mkdir -p build
>$(HOSTCC) $(HOST_CFLAGS_M7) kernel/mm/vmm.c tests/test_vmm_host.c -o build/test_vmm_host

check: build/vmm.o build/test_vmm_host
>./build/test_vmm_host
>nm -u build/vmm.o
>objdump -dr build/vmm.o > build/vmm.objdump.txt
>grep -q "invlpg" build/vmm.objdump.txt
>grep -q "cr3"    build/vmm.objdump.txt
>@echo "[PASS] M7 check selesai"
