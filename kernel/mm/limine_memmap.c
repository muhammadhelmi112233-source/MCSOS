#include <stddef.h>
#include <mcsos/kernel/pmm.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

/* Limine memmap request — harus di file terpisah agar linker
   menempatkan variabel ini di section yang benar */
#include "third_party/limine/limine.h"

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id       = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = (struct limine_memmap_response *)0,
};

static struct pmm_state    kernel_pmm;
static uint8_t             kernel_pmm_bitmap[PMM_BITMAP_BYTES]
    __attribute__((aligned(4096)));

/* Konversi tipe Limine -> BOOT_MEM_* */
static uint32_t limine_type_to_boot_mem(uint64_t ltype) {
    switch (ltype) {
    case LIMINE_MEMMAP_USABLE:                 return (uint32_t)BOOT_MEM_USABLE;
    case LIMINE_MEMMAP_RESERVED:               return (uint32_t)BOOT_MEM_RESERVED;
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:       return (uint32_t)BOOT_MEM_ACPI_RECLAIMABLE;
    case LIMINE_MEMMAP_ACPI_NVS:               return (uint32_t)BOOT_MEM_ACPI_NVS;
    case LIMINE_MEMMAP_BAD_MEMORY:             return (uint32_t)BOOT_MEM_BAD_MEMORY;
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: return (uint32_t)BOOT_MEM_BOOTLOADER_RECLAIMABLE;
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:     return (uint32_t)BOOT_MEM_KERNEL_AND_MODULES;
    case LIMINE_MEMMAP_FRAMEBUFFER:            return (uint32_t)BOOT_MEM_FRAMEBUFFER;
    default:                                   return (uint32_t)BOOT_MEM_RESERVED;
    }
}

struct pmm_state *kernel_pmm_get(void) {
    return &kernel_pmm;
}

void kernel_memory_init(void) {
    struct limine_memmap_response *resp = memmap_request.response;
    if (resp == (struct limine_memmap_response *)0) {
        KERNEL_PANIC("limine memmap response null", 0x6D36u);
    }

    uint64_t entry_count = resp->entry_count;
    if (entry_count == 0u) {
        KERNEL_PANIC("limine memmap empty", 0x6D37u);
    }

    /* Batas maksimum region yang kita proses */
    if (entry_count > 64u) {
        entry_count = 64u;
    }

    /* Salin ke boot_mem_region lokal */
    struct boot_mem_region regions[64];
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *e = resp->entries[i];
        regions[i].base   = e->base;
        regions[i].length = e->length;
        regions[i].type   = limine_type_to_boot_mem(e->type);
    }

    /* Log ringkasan memory map */
    log_writeln("[MCSOS:M6] memory map dari limine:");
    for (uint64_t i = 0; i < entry_count; i++) {
        log_write("  region ");
        log_dec64(i);
        log_write(": base=");
        log_hex64(regions[i].base);
        log_write(" len=");
        log_hex64(regions[i].length);
        log_write(" type=");
        log_dec64((uint64_t)regions[i].type);
        log_writeln("");
    }

    /* Inisialisasi PMM */
    bool ok = pmm_init_from_map(&kernel_pmm,
                                regions,
                                (size_t)entry_count,
                                kernel_pmm_bitmap,
                                sizeof(kernel_pmm_bitmap),
                                PMM_MAX_PHYS_BYTES);
    if (!ok) {
        KERNEL_PANIC("pmm_init_from_map gagal", 0x6D38u);
    }

    log_write("[MCSOS:M6] pmm initialized: frames=");
    log_dec64(pmm_frame_count(&kernel_pmm));
    log_write(" free=");
    log_dec64(pmm_free_count(&kernel_pmm));
    log_write(" used=");
    log_dec64(pmm_used_count(&kernel_pmm));
    log_writeln("");

    /* Smoke test: alokasi satu frame lalu kembalikan */
    uint64_t f = pmm_alloc_frame(&kernel_pmm);
    if (f == PMM_INVALID_FRAME) {
        KERNEL_PANIC("pmm_alloc_frame gagal pada smoke test", 0x6D39u);
    }
    log_write("[MCSOS:M6] sample frame alloc=");
    log_hex64(f);
    if ((f & (PMM_PAGE_SIZE - 1ULL)) != 0u) {
        KERNEL_PANIC("frame tidak aligned", 0x6D3Au);
    }
    if (!pmm_free_frame(&kernel_pmm, f)) {
        KERNEL_PANIC("pmm_free_frame gagal pada smoke test", 0x6D3Bu);
    }
    log_writeln(" -> freed OK");
}
