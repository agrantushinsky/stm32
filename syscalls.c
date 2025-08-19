#include <stdint.h>
#include <sys/types.h>
#include <errno.h>
#include <libopencm3/stm32/usart.h>

extern uint8_t __bss_end__;

caddr_t _sbrk(int incr) {
    static uint8_t* heap = &__bss_end__;
    uint8_t* prev_heap = heap;

    heap += incr;
    return (caddr_t)prev_heap;
}

int _write(int fd, char* buf, int len) {
    for (int i = 0; i < len; i++) {
        usart_send_blocking(USART2, buf[i]);
    }
    return len;
}

int _close(int fd) {
    return -1;
}

int _lseek(int fd, int offset, int whence) {
    return -1;
}

int _read(int fd, char* buf, int len) {
    return -1;
}

int _fstat(int fd, void* buf) {
    return -1;
}

int _isatty(int fd) {
    return -1;
}

