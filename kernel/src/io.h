#pragma once

#include <stdint.h>

void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);
void io_wait();
uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t data);