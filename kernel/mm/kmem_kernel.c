#include <stddef.h>
#include "mcsos/kmem.h"
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

#define M8_BOOT_HEAP_SIZE (64u * 1024u)

static unsigned char m8_boot_heap[M8_BOOT_HEAP_SIZE]
    __attribute__((aligned(4096)));

void kernel_heap_init(void) {
    int rc = kmem_init(m8_boot_heap, sizeof(m8_boot_heap));
    if (rc != 0) {
        KERNEL_PANIC("M8 kmem_init failed", (uint64_t)(unsigned int)-rc);
    }

    void *probe = kmem_alloc(128);
    if (probe == (void *)0) {
        KERNEL_PANIC("M8 kmem_alloc probe failed", 0x4D38u);
    }

    if (kmem_free_checked(probe) != 0) {
        KERNEL_PANIC("M8 kmem_free_checked probe failed", 0x4D39u);
    }

    kmem_stats_t st;
    kmem_get_stats(&st);

    log_writeln("[MCSOS:M8] kmem initialized");
    log_write("[MCSOS:M8] heap total=");
    log_dec64((uint64_t)st.total_bytes);
    log_write(" free=");
    log_dec64((uint64_t)st.free_bytes);
    log_write(" largest=");
    log_dec64((uint64_t)st.largest_free);
    log_write(" blocks=");
    log_dec64((uint64_t)st.block_count);
    log_writeln("");
    log_writeln("[MCSOS:M8] M8 heap ready");
}
