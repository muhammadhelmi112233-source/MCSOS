#include <stdint.h>
#include "mcsos/syscall.h"
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>
#include "mcsos_thread.h"

extern mcsos_scheduler_t *g_sched_ptr;

static uint64_t k_get_ticks(void) {
    extern uint64_t timer_ticks(void);
    return timer_ticks();
}

static void k_yield_current(void) {
    if (g_sched_ptr != (mcsos_scheduler_t *)0) {
        mcsos_sched_yield(g_sched_ptr);
    }
}

static void k_exit_current(int code) {
    log_write("[MCSOS:M10] exit_thread code=");
    log_dec64((uint64_t)(unsigned int)code);
    log_writeln("");
    if (g_sched_ptr != (mcsos_scheduler_t *)0) {
        g_sched_ptr->current->exit_code = code;
        g_sched_ptr->current->state = MCSOS_THREAD_ZOMBIE;
    }
    for (;;) {
#if defined(__x86_64__)
        __asm__ volatile("hlt");
#endif
    }
}

static int64_t k_write_serial(const char *buf, size_t len) {
    if (buf == (const char *)0 || len == 0u) return MCSOS_EINVAL;
    size_t limit = len < 256u ? len : 256u;
    for (size_t i = 0u; i < limit; ++i) {
        log_putc(buf[i]);
    }
    return (int64_t)limit;
}

void kernel_syscall_init(void) {
    mcsos_syscall_ops_t ops = {
        .get_ticks     = k_get_ticks,
        .yield_current = k_yield_current,
        .exit_current  = k_exit_current,
        .write_serial  = k_write_serial,
    };
    mcsos_syscall_init(&ops);

    mcsos_syscall_set_user_region((mcsos_user_region_t){
        .base  = 0x0000000000400000ULL,
        .limit = 0x0000800000000000ULL,
    });

    log_writeln("[MCSOS:M10] syscall init");

    int64_t r = mcsos_syscall_dispatch(MCSOS_SYS_PING, 0, 0, 0, 0, 0, 0);
    if (r != 0x2605020AL) {
        KERNEL_PANIC("M10 syscall ping failed", (uint64_t)r);
    }
    log_writeln("[MCSOS:M10] syscall ping ok");

    int64_t ticks = mcsos_syscall_dispatch(MCSOS_SYS_GET_TICKS, 0, 0, 0, 0, 0, 0);
    log_write("[MCSOS:M10] syscall get_ticks=");
    log_dec64((uint64_t)ticks);
    log_writeln("");
    log_writeln("[MCSOS:M10] syscall get_ticks ok");

    log_writeln("[MCSOS:M10] syscall smoke done");
}
