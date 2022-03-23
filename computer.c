#include "computer.h"

void
computer_init(Computer *computer)
{
	cpu_init(&computer->cpu, &computer->mem);
}

void
computer_clk(Computer *computer)
{
	cpu_clk(&computer->cpu);
	mem_clk(&computer->mem);
}
