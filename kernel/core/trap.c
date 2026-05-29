/* M5: x86_64_trap_dispatch dipindah ke kernel/arch/x86_64/idt.c */
#include <stdint.h>
#include <mcsos/arch/idt.h>

static uint64_t trap_count;

uint64_t m4_trap_count_for_test(void) {
    return trap_count;
}
