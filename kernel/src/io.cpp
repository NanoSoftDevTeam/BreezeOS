#include <io.h>

void outb(uint16_t port, uint8_t data) {
	asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
	return;
}

uint8_t inb(uint16_t port) {
	uint8_t res;
	asm volatile("inb %1, %0" : "=a"(res) : "Nd"(port));
	return res;
}

void io_wait() {
    asm volatile ("outb %%al, $0x80" : : "a"(0));
}

uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %1, %0" : : "dN"(port), "a"(data));
}