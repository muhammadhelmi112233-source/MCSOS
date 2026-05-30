#include <stdint.h>
#include <mcsos/arch/cpu.h>
#include <mcsos/arch/idt.h>
#include <mcsos/arch/pic.h>
#include <mcsos/arch/pit.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>
#include <mcsos/kernel/version.h>
#include <mcsos/kernel/pmm.h>
#include <mcsos/kernel/vmm.h>

extern char __kernel_start[];
extern char __kernel_end[];
extern void kernel_memory_init(void);
extern void kernel_vmm_init(void);
extern void kernel_heap_init(void);
extern void kernel_scheduler_init(void);
extern void kernel_syscall_init(void);

static void m4_selftest(void) {
    KERNEL_ASSERT(__kernel_end > __kernel_start);
    KERNEL_ASSERT(sizeof(uintptr_t) == 8u);
    KERNEL_ASSERT(sizeof(x86_64_idt_entry_t) == 16u);
    KERNEL_ASSERT(x86_64_idt_base_for_test() != 0u);
    KERNEL_ASSERT(x86_64_idt_limit_for_test() == 4095u);
    log_writeln("[M4] selftest: IDT invariants passed");
}

void kmain(void) {
    cpu_cli();
    log_init();

    log_write(MCSOS_NAME);
    log_write(" ");
    log_write(MCSOS_VERSION);
    log_write(" ");
    log_write(MCSOS_MILESTONE);
    log_writeln(" kernel entered");

    log_key_value_hex64("kernel_start", (uint64_t)(uintptr_t)__kernel_start);
    log_key_value_hex64("kernel_end",   (uint64_t)(uintptr_t)__kernel_end);
    log_key_value_hex64("rflags_before_idt", cpu_read_rflags());

    log_writeln("[MCSOS:M5] boot: external interrupt bring-up start");

    x86_64_idt_init();
    m4_selftest();
    log_writeln("[MCSOS:M5] idt: loaded");

    pic_remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);
    pic_mask_all();
    pic_unmask_irq(0u);
    log_write("[MCSOS:M5] pic: remapped; mask master=");
    log_hex64((uint64_t)pic_read_master_mask());
    log_write(" slave=");
    log_hex64((uint64_t)pic_read_slave_mask());
    log_writeln("");

    pit_configure_hz(100u);
    log_writeln("[MCSOS:M5] pit: configured 100Hz");

    log_writeln("[MCSOS:M6] boot: physical memory manager init start");
    kernel_memory_init();
    log_writeln("[MCSOS:M6] pmm: ready");

    log_writeln("[MCSOS:M7] boot: virtual memory manager init start");
    kernel_vmm_init();
    log_writeln("[MCSOS:M7] vmm: ready");

    log_writeln("[MCSOS:M8] boot: kernel heap init start");
    kernel_heap_init();
    log_writeln("[MCSOS:M8] heap: ready");

    log_writeln("[MCSOS:M9] boot: kernel scheduler init start");
    kernel_scheduler_init();
    log_writeln("[MCSOS:M9] scheduler: ready");

    log_writeln("[MCSOS:M10] boot: kernel syscall init start");
    kernel_syscall_init();
    log_writeln("[MCSOS:M10] syscall: ready");

    log_writeln("[MCSOS:M5] sti: enabling interrupts");
    cpu_sti();

#ifdef MCSOS_M4_TRIGGER_BREAKPOINT
    log_writeln("[M4] triggering intentional breakpoint exception");
    x86_64_trigger_breakpoint_for_test();
    log_writeln("[M4] returned from breakpoint handler");
#endif

#ifdef MCSOS_M4_TRIGGER_PANIC
    KERNEL_PANIC("intentional M4 panic test", 0x4D43534F533034u);
#endif

    for (;;) {
        cpu_hlt();
    }
}
