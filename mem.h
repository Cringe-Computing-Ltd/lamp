#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include <stdint.h>

typedef struct mem {
	uint16_t _mem[16*1024];

	uint16_t addr;
	uint16_t in;
	uint16_t out;
	uint8_t we;

	uint8_t wcnt;
	uint8_t rcnt;
} Mem;


void mem_init(Mem *mem);

void mem_load_rom(Mem *mem, uint16_t *rom, size_t size);
void mem_load_rom_file(Mem *mem, FILE *src);

void mem_clk(Mem *mem);

#endif
