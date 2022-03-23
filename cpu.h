#ifndef CPU_H
#define CPU_H

#include "mem.h"
#include <stdint.h>

enum ExecState {
	FETCH,
	IDLE,
	EXEC,
	CONTD
};

typedef struct cpu {
	Mem *mem;

	enum ExecState state;
	enum ExecState state_after_idle;
	
	uint16_t ip;
	uint16_t regs[8];
	
	uint8_t zf;
	uint8_t sf;
	uint8_t cf;

	uint8_t opcode_longlive;
	
	uint8_t dst_longlive;
	uint8_t src_longlive;

	uint16_t dst_content_longlive;
	uint16_t src_content_longlive;

	uint32_t multmp;

#define CARRY(flags) ((flags & (1 << 2)) >> 2)
#define ZERO(flags) ((flags & (1 << 0)) >> 0)
#define SIGN(flags) ((flags & (1 << 1)) >> 1)	
} Cpu;

void cpu_init(Cpu *cpu, Mem *mem);

void cpu_clk(Cpu *cpu);
void cpu_fetch(Cpu *cpu);
void cpu_idle(Cpu *cpu);
void cpu_exec(Cpu *cpu);
void cpu_contd(Cpu *cpu);

#endif
