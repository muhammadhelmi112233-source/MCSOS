#ifndef MCSOS_ARCH_ISR_H
#define MCSOS_ARCH_ISR_H
#include <stdint.h>

#define MCSOS_IDT_VECTOR_COUNT 48u

typedef void (*x86_64_isr_handler_t)(void);

extern x86_64_isr_handler_t x86_64_exception_stubs[32];
extern x86_64_isr_handler_t isr_stub_table[48];

#endif
