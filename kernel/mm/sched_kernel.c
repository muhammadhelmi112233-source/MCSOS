#include <stdint.h>
#include "mcsos_thread.h"
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

static mcsos_scheduler_t g_sched;
mcsos_scheduler_t *g_sched_ptr = &g_sched;
static mcsos_thread_t    g_boot_thread;
static mcsos_thread_t    g_thread_a;
static mcsos_thread_t    g_thread_b;

static unsigned char g_stack_a[8192] __attribute__((aligned(16)));
static unsigned char g_stack_b[8192] __attribute__((aligned(16)));

static void demo_thread_a(void *arg) {
    (void)arg;
    log_writeln("[MCSOS:M9] thread A tick");
    mcsos_sched_yield(&g_sched);
    log_writeln("[MCSOS:M9] thread A tick 2");
    mcsos_sched_yield(&g_sched);
}

static void demo_thread_b(void *arg) {
    (void)arg;
    log_writeln("[MCSOS:M9] thread B tick");
    mcsos_sched_yield(&g_sched);
    log_writeln("[MCSOS:M9] thread B tick 2");
    mcsos_sched_yield(&g_sched);
}

void kernel_scheduler_init(void) {
    int rc = mcsos_scheduler_init(&g_sched, &g_boot_thread);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 mcsos_scheduler_init failed", (uint64_t)(unsigned int)-rc);
    }

    rc = mcsos_thread_prepare(&g_thread_a, "demo-a", demo_thread_a,
                              (void *)0, g_stack_a, sizeof(g_stack_a),
                              g_sched.next_id++);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 thread_prepare A failed", (uint64_t)(unsigned int)-rc);
    }

    rc = mcsos_thread_prepare(&g_thread_b, "demo-b", demo_thread_b,
                              (void *)0, g_stack_b, sizeof(g_stack_b),
                              g_sched.next_id++);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 thread_prepare B failed", (uint64_t)(unsigned int)-rc);
    }

    rc = mcsos_sched_enqueue(&g_sched, &g_thread_a);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 enqueue A failed", (uint64_t)(unsigned int)-rc);
    }

    rc = mcsos_sched_enqueue(&g_sched, &g_thread_b);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 enqueue B failed", (uint64_t)(unsigned int)-rc);
    }

    log_writeln("[MCSOS:M9] scheduler initialized");
    log_write("[MCSOS:M9] runnable_count=");
    log_dec64(g_sched.runnable_count);
    log_writeln("");

    rc = mcsos_sched_yield(&g_sched);
    if (rc != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 first yield failed", (uint64_t)(unsigned int)-rc);
    }

    log_write("[MCSOS:M9] context_switches=");
    log_dec64(g_sched.context_switches);
    log_writeln("");
    log_writeln("[MCSOS:M9] M9 scheduler checkpoint reached");
}
