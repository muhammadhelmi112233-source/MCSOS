#include <stddef.h>
#include <mcsos/kernel/vmm.h>
#include <mcsos/kernel/pmm.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

#include "third_party/limine/limine.h"

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id       = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = (struct limine_hhdm_response *)0,
};

static struct vmm_space kernel_space;
static uint64_t         hhdm_offset;

static uint64_t kernel_vmm_alloc(void *ctx) {
    (void)ctx;
    extern struct pmm_state *kernel_pmm_get(void);
    return pmm_alloc_frame(kernel_pmm_get());
}

static void kernel_vmm_free(void *ctx, uint64_t frame_paddr) {
    (void)ctx;
    extern struct pmm_state *kernel_pmm_get(void);
    pmm_free_frame(kernel_pmm_get(), frame_paddr);
}

static void *kernel_phys_to_virt(void *ctx, uint64_t paddr) {
    const uint64_t offset = *(const uint64_t *)ctx;
    return (void *)(offset + paddr);
}

struct vmm_space *kernel_vmm_get(void) {
    return &kernel_space;
}

void kernel_vmm_init(void) {
    struct limine_hhdm_response *resp = hhdm_request.response;
    if (resp == (struct limine_hhdm_response *)0) {
        KERNEL_PANIC("limine HHDM response null", 0x6D40u);
    }

    hhdm_offset = resp->offset;
    log_write("[MCSOS:M7] hhdm_offset=");
    log_hex64(hhdm_offset);
    log_writeln("");

    /* Alokasi root page table dari PMM */
    extern struct pmm_state *kernel_pmm_get(void);
    uint64_t root = pmm_alloc_frame(kernel_pmm_get());
    if (root == PMM_INVALID_FRAME) {
        KERNEL_PANIC("M7: cannot allocate root page table", 0x6D41u);
    }

    /* Zero root table melalui HHDM */
    uint64_t *root_virt = (uint64_t *)(hhdm_offset + root);
    for (size_t i = 0; i < 512; i++) {
        root_virt[i] = 0;
    }

    int rc = vmm_space_init(&kernel_space, root, &hhdm_offset,
                            kernel_vmm_alloc, kernel_vmm_free,
                            kernel_phys_to_virt);
    if (rc != VMM_MAP_OK) {
        KERNEL_PANIC("M7: vmm_space_init failed", 0x6D42u);
    }

    log_write("[MCSOS:M7] vmm initialized: root_paddr=");
    log_hex64(root);
    log_writeln("");
    log_writeln("[MCSOS:M7] VMM core initialized");

    /* Tugas wajib berhenti di sini.
       Jangan write_cr3 sebelum mapping kernel lengkap. */
}
