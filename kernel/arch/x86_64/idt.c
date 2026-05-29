#include <stdint.h>
#include <mcsos/arch/idt.h>
#include <mcsos/arch/isr.h>
#include <mcsos/arch/pic.h>
#include <mcsos/arch/pit.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>

static x86_64_idt_entry_t idt[X86_64_IDT_VECTOR_COUNT];
static x86_64_idtr_t idtr;

static inline void lidt(const x86_64_idtr_t *descriptor) {
    __asm__ volatile ("lidt (%0)" :: "r"(descriptor) : "memory");
}

void x86_64_idt_set_gate(uint8_t vector, uint64_t handler, uint8_t type_attributes) {
    idt[vector].offset_low      = (uint16_t)(handler & 0xFFFFu);
    idt[vector].selector        = (uint16_t)X86_64_KERNEL_CODE_SELECTOR;
    idt[vector].ist             = 0u;
    idt[vector].type_attributes = type_attributes;
    idt[vector].offset_mid      = (uint16_t)((handler >> 16u) & 0xFFFFu);
    idt[vector].offset_high     = (uint32_t)((handler >> 32u) & 0xFFFFFFFFu);
    idt[vector].reserved        = 0u;
}

uint64_t x86_64_idt_base_for_test(void) {
    return idtr.base;
}

uint16_t x86_64_idt_limit_for_test(void) {
    return idtr.limit;
}

void x86_64_trigger_breakpoint_for_test(void) {
    __asm__ volatile ("int3");
}

void x86_64_idt_init(void) {
    for (uint8_t i = 0u; i < (uint8_t)MCSOS_IDT_VECTOR_COUNT; ++i) {
        x86_64_idt_set_gate(i,
            (uint64_t)(uintptr_t)isr_stub_table[i],
            X86_64_IDT_GATE_INTERRUPT);
    }
    idtr.limit = (uint16_t)(sizeof(idt) - 1u);
    idtr.base  = (uint64_t)(uintptr_t)&idt[0];
    lidt(&idtr);
}

void x86_64_trap_dispatch(x86_64_trap_frame_t *frame) {
    if (frame == (x86_64_trap_frame_t *)0) {
        KERNEL_PANIC("null trap frame", 0u);
    }

    if (frame->vector >= PIC_MASTER_OFFSET &&
        frame->vector < (uint64_t)(PIC_SLAVE_OFFSET + 8u)) {
        uint8_t irq = (uint8_t)(frame->vector - PIC_MASTER_OFFSET);
        if (irq == 0u) {
            timer_on_irq0();
        } else {
            log_write("[M5:IRQ] unexpected irq=");
            log_dec64((uint64_t)irq);
            log_writeln("");
        }
        pic_send_eoi(irq);
        return;
    }

    if (frame->vector == 3u) {
        log_write("[M4] trap dispatch: #BP Breakpoint\n");
        log_key_value_hex64("trap_vector", frame->vector);
        log_key_value_hex64("trap_error",  frame->error_code);
        log_key_value_hex64("trap_rip",    frame->rip);
        log_key_value_hex64("trap_cs",     frame->cs);
        log_key_value_hex64("trap_rflags", frame->rflags);
        log_key_value_hex64("trap_rax",    frame->rax);
        log_key_value_hex64("trap_rbx",    frame->rbx);
        log_key_value_hex64("trap_rcx",    frame->rcx);
        log_key_value_hex64("trap_rdx",    frame->rdx);
        log_writeln("[M4] breakpoint handled; returning with iretq");
        return;
    }

    log_write("[M5:EXCEPTION] vector=");
    log_dec64(frame->vector);
    log_write(" error=");
    log_hex64(frame->error_code);
    log_write(" rip=");
    log_hex64(frame->rip);
    log_writeln("");
    KERNEL_PANIC("unhandled CPU exception", frame->vector);
}
