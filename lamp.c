#include <stdio.h>
#include <stdint.h>

#include "computer.h"
#include "mem.h"

uint8_t rom[] = {
	0 | (1 << 6), 0, 69, 0,
	21, 0,
	16 | (1 << 6), 0,
	0 | (2 << 6), 0, 2, 0,
	6 | (2 << 6), 0
};

int
main()
{
	FILE *fp = fopen("out.bin", "r+");

	Computer computer;
	computer_init(&computer);

	mem_load_rom_file(&computer.mem, fp);

	fclose(fp);

	while (1)
		computer_clk(&computer);
}
