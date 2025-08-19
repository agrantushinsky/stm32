#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <stdint.h>

// https://gcc.gnu.org/onlinedocs/gcc/Spec-Files.html
// TODO: Add newlib-nano
// TODO: Increase clock

extern uint8_t _sstack, _stext, _etext, _sdata, _edata, _sbss, _ebss;
#define VTABLE_SIZE 113

void default_handler(void) { __asm__("b ."); };

void reset_handler(void);
void nmi_handler(void) __attribute__((weak, alias("default_handler")));
void hard_default_handler(void) __attribute__((weak, alias("default_handler")));
void mem_manage_handler(void) __attribute__((weak, alias("default_handler")));
void bus_fault_handler(void) __attribute__((weak, alias("default_handler")));
void usage_fault_handler(void) __attribute__((weak, alias("default_handler")));
void svc_handler(void) __attribute__((weak, alias("default_handler")));
void debug_monitor_handler(void) __attribute__((weak, alias("default_handler")));
void pendsv_handler(void) __attribute__((weak, alias("default_handler")));

volatile uint32_t ticks;
void systick_handler(void) {
    ticks++;
}

__attribute__((section(".isr_vector")))
uintptr_t isr_vector[VTABLE_SIZE] = {
    (uintptr_t)&_sstack,

    (uintptr_t)&reset_handler,
    (uintptr_t)&nmi_handler,
    (uintptr_t)&hard_default_handler,
    (uintptr_t)&mem_manage_handler,
    (uintptr_t)&bus_fault_handler,
    (uintptr_t)&usage_fault_handler,
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    (uintptr_t)&svc_handler,
    (uintptr_t)&debug_monitor_handler,
    0, // Reserved
    (uintptr_t)&pendsv_handler,
    (uintptr_t)&systick_handler,
    // External interupts...
};

void sleep(uint32_t time) {
    uint32_t start = ticks;
    uint32_t wake = start + time;
    if(wake < start) {
        while(ticks > start);
    }
    while(ticks < wake);
}

uint32_t time(void) {
    return ticks;
}

void entry_point(void);

void reset_handler(void) {
    // copy .data to sram1
    uintptr_t data_size = (uintptr_t)&_edata - (uintptr_t)&_sdata;
    uint8_t* sram = (uint8_t*)&_sdata;
    uint8_t* data_in_flash = (uint8_t*)&_etext;
    for(uintptr_t i = 0; i < data_size; i++) {
        sram[i] = data_in_flash[i];
    }

    // zero .bss section
    uint8_t bss_size = (uintptr_t)&_ebss - (uintptr_t)&_sbss;
    uint8_t* bss = (uint8_t*)&_sbss;
    for(uintptr_t i = 0; i < bss_size; i++) {
        bss[i] = 0;
    }

    entry_point();
}

void entry_point(void) {
    // Configure LED
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);

    // Configure clock
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_180MHZ]);
    systick_set_reload(180000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();

    cm_enable_interrupts();

    while (1) {
        gpio_toggle(GPIOA, GPIO5);
        sleep(1000);
    }
}

