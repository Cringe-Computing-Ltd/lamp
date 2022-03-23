#ifndef COMPUTER_H
#define COMPUTER_H

#include "mem.h"
#include "cpu.h"

typedef struct computer {
	Cpu cpu;
	Mem mem;
} Computer;

void computer_init(Computer *computer);

void
computer_clk(Computer *computer);

#endif
