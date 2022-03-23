#include "cpu.h"

#include <stdio.h>

#define LAST(k,n) ((k) & ((1<<(n))-1))
#define DOWNTO(k,n,m) LAST((k)>>(m),((n+1)-(m)))

void
cpu_init(Cpu *cpu, Mem *mem)
{
	*cpu = (Cpu) {
		.mem = mem,
		.state = FETCH,   
	};
}

void
cpu_clk(Cpu *cpu)
{
	
	switch (cpu->state) {
	case FETCH:
		cpu_fetch(cpu);
		break;	       
	case IDLE:
		cpu_idle(cpu);
		break;
	case EXEC:
		cpu_exec(cpu);
		break;
	case CONTD:
		cpu_contd(cpu);
		break;		
	}
}

void
cpu_fetch(Cpu *cpu)
{
	printf("[CPU] fetching at ip(%d)\n", cpu->ip);
	cpu->mem->addr = cpu->ip;
	cpu->mem->we = 0;

	cpu->state = IDLE;
	cpu->state_after_idle = EXEC;
}

void
cpu_idle(Cpu *cpu)
{
	printf("[CPU] idling\n");
	cpu->state = cpu->state_after_idle;
}

void
cpu_exec(Cpu *cpu)
{
	printf("[CPU] inst: %d\n", cpu->mem->out);
	uint16_t opcode;
	uint8_t src;
	uint8_t dst;
	uint16_t src_content;
	uint16_t dst_content;

	uint16_t tmp_content;
	uint32_t carrier_tmp;
	uint8_t jmp_cond_ok;
	
	opcode = DOWNTO(cpu->mem->out, 5, 0);
	cpu->opcode_longlive = opcode;

	dst = DOWNTO(cpu->mem->out, 10, 6);
	src = DOWNTO(cpu->mem->out, 15, 11);

	cpu->dst_longlive = dst;
	cpu->src_longlive = src;

	dst_content = cpu->regs[dst];
	src_content = cpu->regs[src];

	cpu->dst_content_longlive = dst_content;
	cpu->src_content_longlive = src_content;

	printf("opcode: %d\n", opcode);

	switch (opcode) {
	case 0: // ldi: load imm into dst.
		printf("[CPU] ldi %d, %d\n", dst, src);
		cpu->mem->addr = cpu->ip + 1;

		cpu->state = IDLE;
		cpu->state_after_idle = CONTD;
		break;
	case 1: // st: store src into [dst]
		printf("[CPU] st %d, %d\n", dst, src);
		cpu->mem->addr = dst_content;
		cpu->mem->in = src_content;
		cpu->mem->we = 1;

		cpu->ip = cpu->ip + 1;

		cpu->state = IDLE;
		cpu->state_after_idle = FETCH;
		break;
	case 2: // ld: load [src] into dst
		printf("[CPU] ld %d, %d\n", dst, src);
		cpu->mem->addr = src_content;

		cpu->state = IDLE;
		cpu->state_after_idle = CONTD;
		// ldi: CONT'D
		break;
	case 3: // add: put dst+src into dst
		printf("[CPU] add %d, %d\n", dst, src);
		carrier_tmp = dst_content + src_content;

		dst_content = carrier_tmp;

		cpu->cf = carrier_tmp >> 16;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 4: // sub: put dst-src into dst
		printf("[CPU] sub %d, %d\n", dst, src);
		carrier_tmp = dst_content - src_content;

		dst_content = carrier_tmp;

		cpu->cf = carrier_tmp >> 16;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 5: // mul: put dst*src into d:dst
		printf("[CPU] mul %d, %d\n", dst, src);
		cpu->multmp = dst_content * src_content;

		cpu->cf = 0;

		cpu->state = CONTD;
		// mul: cont'd
		break;
	case 6: // jmp: jump to dst
		printf("[CPU] jmp %d", dst);
		switch (DOWNTO(src, 3, 0)) {
		case 0: // unconditional
			printf("\t(unconditional)");
			jmp_cond_ok = 1;
			break;
		case 14: // ==
			printf("\t(==)");
			jmp_cond_ok = cpu->zf;
			break;
		case 15: // !=
			printf("\t(!=)");
			jmp_cond_ok = !cpu->zf;
			break;
		case 8: // >
			printf("\t(>)");
			jmp_cond_ok = !cpu->sf && !cpu->zf;
			break;
		case 9: // >=
			printf("\t(>=)");
			jmp_cond_ok = !cpu->sf;
			break;
		case 4: // <
			printf("\t(<)");
			jmp_cond_ok = cpu->sf;
			break;
		case 5: // <=
			printf("\t(<=)");
			jmp_cond_ok = cpu->sf || cpu->zf;
			break;
		case 1: // carry
			printf("\t(on carry)");
			jmp_cond_ok = cpu->cf;
		default:
			// NOTHING
			(void)0;
		}

		if (jmp_cond_ok) {
			if (src & (1 << 4)) {
				printf("\timm [ OK ]\n");
				cpu->mem->addr = cpu->ip + 1;

				cpu->state_after_idle = CONTD;
				cpu->state = IDLE;
			} else {
				printf("\tto dst [ OK ]\n");
				cpu->ip = dst_content;
				cpu->state = FETCH;
			}
		} else if (src & (1 << 4)) {
			printf("\timm [ FAILED ]\n");
			cpu->ip = cpu->ip + 2;
			cpu->state = FETCH;
		} else {
			printf("\tto dst [ FAILED ]\n");
			cpu->ip = cpu->ip + 1;
			cpu->state = FETCH;
		}
		
		break;
	case 7: // xchg: exchange src and dst
		printf("[CPU] xchg %d, %d\n", dst, src);
		tmp_content = src_content;
		src_content = dst_content;
		dst_content = tmp_content;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 8: // xor: dst = dst xor src
		printf("[CPU] xor %d, %d\n", dst, src);
		dst_content = dst_content ^ src_content;

		cpu->ip = cpu->ip + 1;

		cpu->state = FETCH;
		break;
	case 9: // and: dst = dst and src
		printf("[CPU] and %d, %d\n", dst, src);
		dst_content = dst_content & src_content;

		cpu->ip = cpu->ip + 1;

		cpu->state = FETCH;
		break;
	case 10: // or: dst = dst or src
		printf("[CPU] or %d, %d\n", dst, src);
		dst_content = dst_content | src_content;

		cpu->ip = cpu->ip + 1;

		cpu->state = FETCH;
		break;
	case 11: // cmp compares dst and src
		printf("[CPU] cmp %d, %d\n", dst, src);
		carrier_tmp = dst_content + src_content;

		if (DOWNTO(carrier_tmp, 15, 0) == 0)
			cpu->zf = 1;
		else
			cpu->zf = 0;

		cpu->sf = carrier_tmp & (1 << 15);
		cpu->cf = carrier_tmp >> 16;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 12: // sti: store dst into imm
		cpu->mem->addr = cpu->ip + 1;

		cpu->state_after_idle = CONTD;
		cpu->state = IDLE;
		break;
	case 13: // shl: left shift
		dst_content = dst_content << src;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 14: // shr: right shift
		dst_content = dst_content >> src;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 15: // mov: move src into dst
		dst_content = src_content;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 16: // inc: increment dst
		carrier_tmp = dst_content + 1;

		dst_content = carrier_tmp;

		cpu->cf = carrier_tmp >> 16;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 17: // dec: decrement dst
		carrier_tmp = dst_content - 1;

		dst_content = carrier_tmp;

		cpu->cf = carrier_tmp >> 16;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 18: // pshi: push immediate
		cpu->mem->addr = cpu->ip + 1;
		cpu->regs[7] = cpu->regs[7] - 1;

		cpu->state_after_idle = CONTD;
		cpu->state = IDLE;
		break;
	case 19: // psh: push dst
		cpu->mem->addr = cpu->regs[7] - 1;
		cpu->mem->in = dst_content;
		cpu->mem->we = 1;

		cpu->regs[7] = cpu->regs[7] - 1;

		cpu->ip = cpu->ip + 1;
		cpu->state_after_idle = FETCH;
		cpu->state = IDLE;
		break;
	case 20: // pop: pop do dst
		cpu->mem->addr = cpu->regs[7];

		cpu->regs[7] = cpu->regs[7] + 1;
		cpu->state_after_idle = CONTD;
		cpu->state = IDLE;
		break;
        case 21: // dbg
		printf("debug!\n");

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	default:
		// invalid instruction
		(void)0;
	}

	// reload
	cpu->regs[dst] = dst_content;
	cpu->regs[src] = src_content;

	if (dst_content == 0)
		cpu->zf = 1;
	else
		cpu->zf = 0;

	cpu->sf = dst_content >> 15;
}

void
cpu_contd(Cpu *cpu)
{
	uint16_t opcode = cpu->opcode_longlive;
	uint8_t src = cpu->src_longlive;
	uint8_t dst = cpu->dst_longlive;
	uint16_t src_content = cpu->src_content_longlive;
	uint16_t dst_content = cpu->dst_content_longlive;
	
	switch (opcode) {
	case 0:
		dst_content = cpu->mem->out;

		cpu->ip = cpu->ip + 2;

		cpu->state = FETCH;
		break;
	case 2:
		dst_content = cpu->mem->out;

		cpu->ip = cpu->ip + 1;

		cpu->state = FETCH;
		break;
	case 3:
		dst_content = DOWNTO(cpu->multmp, 15, 0);
		cpu->regs[3] = DOWNTO(cpu->multmp, 31, 16);

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	case 6:
		cpu->ip = cpu->mem->out;
		cpu->state = FETCH;
		break;
	case 18:
		cpu->mem->addr = cpu->regs[7];
		cpu->mem->in = cpu->mem->out;
		cpu->mem->we = 1;

		cpu->ip = cpu->ip + 2;
		cpu->state_after_idle = FETCH;
		cpu->state = IDLE;
		break;
	case 20:
		dst_content = cpu->mem->out;

		cpu->ip = cpu->ip + 1;
		cpu->state = FETCH;
		break;
	default:
		// invalid state
		(void)0;
	}

	cpu->regs[dst] = dst_content;
	cpu->regs[src] = src_content;

	if (dst_content == 0)
		cpu->zf = 1;
	else
		cpu->zf = 0;

	cpu->sf = dst_content >> 15;
}
