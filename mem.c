#include "mem.h"

#include <string.h>

void
mem_init(Mem *mem)
{
	*mem = (Mem) {
	};
}

void
mem_load_rom_file(Mem *mem, FILE *src)
{
	fseek(src, 0L, SEEK_END);
	size_t sz = ftell(src);
	fseek(src, 0L, SEEK_SET);
	
	int fd = fileno(src);
	read(fd, mem->_mem, sz);
}

void
mem_load_rom(Mem *mem, uint16_t *rom, size_t size)
{
	memcpy(mem->_mem, rom, size);
}

void
mem_clk(Mem *mem)
{
	if (mem->we) {
		mem->rcnt = 0;
		mem->wcnt++;
	} else {
		mem->rcnt++;
		mem->wcnt = 0;
	}

	if (mem->wcnt >= 2) {
		mem->_mem[mem->addr] = mem->in;
		mem->wcnt = 0;
	} else if (mem->rcnt >= 2) {
		mem->out = mem->_mem[mem->addr];
		mem->rcnt = 0;
	}
}
