#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/usart.h>
#include <stdint.h>
#include <stdio.h>

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

void _init(void) { }

void __libc_init_array();
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

    __libc_init_array();
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

    // Configure USART
    rcc_periph_clock_enable(RCC_USART2);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
    
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_stopbits(USART2, USART_CR2_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_enable(USART2);

    // Enable interrupts
    cm_enable_interrupts();

    setbuf(stdout, NULL);

    while (1) {
        gpio_toggle(GPIOA, GPIO5);
        printf("Ticks: %d\r\n", ticks);
        sleep(1000);
    }
}

