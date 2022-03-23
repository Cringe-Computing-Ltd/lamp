#include <stdio.h>
#include <stdint.h>

#include "computer.h"
#include "mem.h"

uint16_t rom[] = {
	21,
	6,
};

int
main()
{
	Computer computer;
	computer_init(&computer);
	mem_load_rom(&computer.mem, rom, sizeof(rom));
	while (1)
		computer_clk(&computer);
}

// SIGNING-OFF: Alejandro, 23/03 22:54, I was implementing the instructions print-debugging
