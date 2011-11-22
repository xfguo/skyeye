#include "arm_dyncom_mmu.h"
#include <map>
#include <cstdio>
using namespace std;

#include "stat.h"
#include "armdefs.h"
#include "armmmu.h"
#include "bank_defs.h"

#define CHECK_RS 	if(RS == 15) rs += 8
#define CHECK_RM 	if(RM == 15) rm += 8

#define BITS(s, a, b) (((s) >> (a)) & ((1 << (1 + (b) - (a))) - 1))
#define BIT(s, n) ((s >> (n)) & 1)
#define RM	BITS(sht_oper, 0, 3)
#define RS	BITS(sht_oper, 8, 11)

#define glue(x, y)		x ## y
#define DPO(s)			glue(DataProcessingOperands, s)
#define ROTATE_RIGHT(n, i, l)	(n << (l - i)) | (n >> i)
#define ROTATE_LEFT(n, i, l)	(n >> (l - i)) | (n << i)
#define ROTATE_RIGHT_32(n, i)	ROTATE_RIGHT(n, i, 32)
#define ROTATE_LEFT_32(n, i)	ROTATE_LEFT(n, i, 32)

//#define rotr(x,n) ((((x)>>(n))&((1<<(sizeof(x) * 8)-1))|(x<<(sizeof(x)*8-n))))
//#define rotl(x,n) ((((x)<<(n))&(-(1<<(n))))|(((x)>>(sizeof(x)*8-n))&((1<<(n))-1)))
#define rotr(x,n) ( (x >> n) | ((x & ((1 << (n + 1)) - 1)) << (32 - n)) )

extern void switch_mode(arm_core_t *core, uint32_t mode);
//extern bool InAPrivilegedMode(arm_core_t *core);

typedef arm_core_t arm_processor;
typedef unsigned int (*shtop_fp_t)(arm_processor *cpu, unsigned int sht_oper);

unsigned int DPO(Immediate)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int immed_8 = BITS(sht_oper, 0, 7);
	unsigned int rotate_imm = BITS(sht_oper, 8, 11);
//	printf("immed_8 is %x\n", immed_8);
//	printf("rotate_imm is %x\n", rotate_imm);
	unsigned int shifter_operand = rotr(immed_8, rotate_imm * 2);//ROTATE_RIGHT_32(immed_8, rotate_imm * 2);
//	printf("shifter_operand : %x\n", shifter_operand);
	/* set c flag */
	if (rotate_imm == 0) 
		cpu->shifter_carry_out = cpu->CFlag;
	else
		cpu->shifter_carry_out = BIT(shifter_operand, 31);
	return shifter_operand;
}

unsigned int DPO(Register)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	unsigned int shifter_operand = rm;
	cpu->shifter_carry_out = cpu->CFlag;
	return shifter_operand;
}

unsigned int DPO(LogicalShiftLeftByImmediate)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	int shift_imm = BITS(sht_oper, 7, 11);
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	unsigned int shifter_operand;
	if (shift_imm == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = cpu->CFlag;
	} else {
		shifter_operand = rm << shift_imm;
		cpu->shifter_carry_out = BIT(rm, 32 - shift_imm);
	}
	return shifter_operand;
}

unsigned int DPO(LogicalShiftLeftByRegister)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	int shifter_operand;
	unsigned int rm = cpu->Reg[RM];
	unsigned int rs = cpu->Reg[RS];
	if (RM == 15) rm += 8;
	if (RS == 15) rs += 8;
	if (BITS(rs, 0, 7) == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = cpu->CFlag;
	} else if (BITS(rs, 0, 7) < 32) {
		shifter_operand = rm << BITS(rs, 0, 7);
		cpu->shifter_carry_out = BIT(rm, 32 - BITS(rs, 0, 7));
	} else if (BITS(rs, 0, 7) == 32) {
		shifter_operand = 0;
		cpu->shifter_carry_out = BIT(rm, 0);
	} else {
		shifter_operand = 0;
		cpu->shifter_carry_out = 0;
	}
	return shifter_operand;
}

unsigned int DPO(LogicalShiftRightByImmediate)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	unsigned int shifter_operand;
	int shift_imm = BITS(sht_oper, 7, 11);
	if (shift_imm == 0) {
		shifter_operand = 0;
		cpu->shifter_carry_out = BIT(rm, 31);
	} else {
		shifter_operand = rm >> shift_imm;
		cpu->shifter_carry_out = BIT(rm, shift_imm - 1);
	}
	return shifter_operand;
}

unsigned int DPO(LogicalShiftRightByRegister)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rs = cpu->Reg[RS];
	unsigned int rm = cpu->Reg[RM];
	if (RS == 15) rs += 8;
	if (RM == 15) rm += 8;
	unsigned int shifter_operand;
	if (BITS(rs, 0, 7) == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = cpu->CFlag;
	} else if (BITS(rs, 0, 7) < 32) {
		shifter_operand = rm >> BITS(rs, 0, 7);
		cpu->shifter_carry_out = BIT(rm, BITS(rs, 0, 7) - 1);
	} else if (BITS(rs, 0, 7) == 32) {
		shifter_operand = 0;
		cpu->shifter_carry_out = BIT(rm, 31);
	} else {
		shifter_operand = 0;
		cpu->shifter_carry_out = 0;
	}
	return shifter_operand;
}

unsigned int DPO(ArithmeticShiftRightByImmediate)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	unsigned int shifter_operand;
	int shift_imm = BITS(sht_oper, 7, 11);
	if (shift_imm == 0) {
		if (BIT(rm, 31)) {
			shifter_operand = 0;
			cpu->shifter_carry_out = BIT(rm, 31);
		} else {
			shifter_operand = 0xFFFFFFFF;
			cpu->shifter_carry_out = BIT(rm, 31);
		}
	} else {
		shifter_operand = static_cast<int>(rm) >> shift_imm;
		cpu->shifter_carry_out = BIT(rm, shift_imm - 1);
	}
	return shifter_operand;
}

unsigned int DPO(ArithmeticShiftRightByRegister)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rs = cpu->Reg[RS];
	unsigned int rm = cpu->Reg[RM];
	if (RS == 15) rs += 8;
	if (RM == 15) rm += 8;
	unsigned int shifter_operand;
	if (BITS(rs, 0, 7) == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = cpu->CFlag;
	} else if (BITS(rs, 0, 7) < 32) {
		shifter_operand = static_cast<int>(rm) >> BITS(rs, 0, 7);
		cpu->shifter_carry_out = BIT(rm, BITS(rs, 0, 7) - 1);
	} else {
		if (BITS(rs, 0, 7) == 32) {
			shifter_operand = 0;
			cpu->shifter_carry_out = BIT(rm, 31);
		} else {
			shifter_operand = 0xFFFFFFFF;
			cpu->shifter_carry_out = BIT(rm, 31);
		}
	}
	return shifter_operand;
}

unsigned int DPO(RotateRightByImmediate)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int shifter_operand;
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	int shift_imm = BITS(sht_oper, 7, 11);
	if (shift_imm == 0) {
		shifter_operand = (cpu->CFlag << 31) | 
					(rm >> 1);
		cpu->shifter_carry_out = BIT(rm, 0);
	} else {
		shifter_operand = ROTATE_RIGHT_32(rm, shift_imm);
		cpu->shifter_carry_out = BIT(rm, shift_imm - 1);
	}
	return shifter_operand;
}

unsigned int DPO(RotateRightByRegister)(arm_processor *cpu, unsigned int sht_oper)
{
//	printf("in %s\n", __FUNCTION__);
	unsigned int rm = cpu->Reg[RM];
	if (RM == 15) rm += 8;
	unsigned int rs = cpu->Reg[RS];
	if (RS == 15) rs += 8;
	unsigned int shifter_operand;
	if (BITS(rs, 0, 7) == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = cpu->CFlag;
	} else if (BITS(rs, 0, 4) == 0) {
		shifter_operand = rm;
		cpu->shifter_carry_out = BIT(rm, 31);
	} else {
		shifter_operand = ROTATE_RIGHT_32(rm, BITS(rs, 0, 4));
		cpu->shifter_carry_out = BIT(rm, BITS(rs, 0, 4) - 1);
	}
	#if 0
	if (cpu->icounter >= 20371544) {
		printf("in %s\n", __FUNCTION__);
		printf("RM:%d\nRS:%d\n", RM, RS);
		printf("rm:0x%08x\nrs:0x%08x\n", cpu->Reg[RM], cpu->Reg[RS]);
	}
	#endif
	return shifter_operand;
}

//typedef unsigned int (*get_addr_fp_t)(arm_processor *cpu);
typedef struct _MiscImmeData {
	unsigned int U;
	unsigned int Rn;
	unsigned int offset_8;
} MiscLSData;

typedef struct _MiscRegData {
	unsigned int U;
	unsigned int Rn;
	unsigned int Rm;
} MiscRegData;

typedef struct _MiscImmePreIdx {
	unsigned int offset_8;
	unsigned int U;
	unsigned int Rn;
} MiscImmePreIdx;

typedef struct _MiscRegPreIdx {
	unsigned int U;
	unsigned int Rn;
	unsigned int Rm;
} MiscRegPreIdx;

typedef struct _MiscImmePstIdx {
	unsigned int offset_8;
	unsigned int U;
	unsigned int Rn;
} MIscImmePstIdx;

typedef struct _MiscRegPstIdx {
	unsigned int Rn;
	unsigned int Rm;
	unsigned int U;
} MiscRegPstIdx;

typedef struct _LSWordorUnsignedByte {
} LDnST;

fault_t interpreter_read_memory(cpu_t *cpu, addr_t virt_addr, addr_t phys_addr, uint32_t &value, uint32_t size);
fault_t interpreter_write_memory(cpu_t *cpu, addr_t virt_addr, addr_t phys_addr, uint32_t value, uint32_t size);
fault_t interpreter_fetch(cpu_t *cpu, addr_t virt_addr, uint32_t &value, uint32_t size);
fault_t check_address_validity(arm_core_t *core, addr_t virt_addr, addr_t *phys_addr, uint32_t rw);

typedef fault_t (*get_addr_fp_t)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw);

typedef struct _ldst_inst {
	unsigned int inst;
	get_addr_fp_t get_addr;
} ldst_inst;

int CondPassed(arm_processor *cpu, unsigned int cond);
#define LnSWoUB(s)	glue(LnSWoUB, s)
#define MLnS(s)		glue(MLnS, s)
#define LdnStM(s)	glue(LdnStM, s)

#define U_BIT		BIT(inst, 23)
#define I_BIT		BIT(inst, 25)
#define OFFSET_12	BITS(inst, 0, 11)
fault_t LnSWoUB(ImmediateOffset)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int addr;
	fault_t fault;
	if (U_BIT) {
		addr = cpu->Reg[Rn] + OFFSET_12;
	} else {
		addr = cpu->Reg[Rn] - OFFSET_12;
	}
	if (Rn == 15) addr += 8;
	if (cpu->icounter == 249111596) {
		printf("in %s\n", __FUNCTION__);
		printf("RN:%d\n", Rn);
		printf("Rn:%x\n", cpu->Reg[Rn]);
		printf("OFFSET12:%d\n", OFFSET_12);
	}
	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	return fault;
//	return addr;
}

fault_t LnSWoUB(RegisterOffset)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int Rm = BITS(inst, 0, 3);
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	unsigned int rm = cpu->Reg[Rm];
	if (Rm == 15) rm += 8;
	unsigned int addr;
	if (U_BIT) {
		addr = rn + rm;
	} else {
		addr = rn - rm;
	}
	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	return fault;
}

fault_t LnSWoUB(ImmediatePostIndexed)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int addr = cpu->Reg[Rn];
	if (Rn == 15) addr += 8;

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	if (fault) return fault;

	if (U_BIT) {
		cpu->Reg[Rn] += OFFSET_12;
	} else {
		cpu->Reg[Rn] -= OFFSET_12;
	}
	return fault;
}

fault_t LnSWoUB(ImmediatePreIndexed)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int addr;
	if (U_BIT) {
		addr = cpu->Reg[Rn] + OFFSET_12;
	} else {
		addr = cpu->Reg[Rn] - OFFSET_12;
	}
	if (Rn == 15) {
		addr += 8;
	}

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31))) {
		cpu->Reg[Rn] = addr;
	}
	return fault;
}

fault_t MLnS(ImmediateOffset)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int immedL = BITS(inst, 0, 3);
	unsigned int immedH = BITS(inst, 8, 11);

	unsigned int Rn     = BITS(inst, 16, 19);
	unsigned int addr;

	unsigned int offset_8 = (immedH << 4) | immedL;
	if (U_BIT) {
		addr = cpu->Reg[Rn] + offset_8;
	} else
		addr = cpu->Reg[Rn] - offset_8;

	if (Rn == 15) {
		addr += 8;
	}

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	return fault;
}

fault_t MLnS(RegisterOffset)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int addr;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int Rm = BITS(inst,  0,  3);
	unsigned int rn = cpu->Reg[Rn];
	unsigned int rm = cpu->Reg[Rm];
	if (Rn == 15) rn += 8;
	if (Rm == 15) rm += 8;
	if (U_BIT) {
		addr = rn + rm;
	} else
		addr = rn - rm;

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	return fault;
}

fault_t MLnS(ImmediatePreIndexed)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn     = BITS(inst, 16, 19);
	unsigned int immedH = BITS(inst,  8, 11);
	unsigned int immedL = BITS(inst,  0,  3);
	unsigned int addr;
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;

//	printf("in %s\n", __FUNCTION__);
	unsigned int offset_8 = (immedH << 4) | immedL;
	if (U_BIT) {
		addr = rn + offset_8;
	} else 
		addr = rn - offset_8;

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31))) {
		cpu->Reg[Rn] = addr;
	}
	return fault;
}

fault_t MLnS(ImmediatePostIndexed)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn     = BITS(inst, 16, 19);
	unsigned int immedH = BITS(inst,  8, 11);
	unsigned int immedL = BITS(inst,  0,  3);
	unsigned int addr;
	unsigned int rn = cpu->Reg[Rn];
	/* FIXME */
	if (Rn == 15) {rn += 8; exit(-1);}
	addr = rn;

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31))) {
		unsigned int offset_8 = (immedH << 4) | immedL;
		if (U_BIT) {
			rn += offset_8;
		} else {
			rn -= offset_8;
		}
		cpu->Reg[Rn] = rn;
	}

	return fault;
}

fault_t LdnStM(DecrementBefore)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int i = BITS(inst, 0, 15);
	int count = 0;
	while(i) {
		if(i & 1) count ++;
		i = i >> 1;
	}
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	unsigned int start_addr = rn - count * 4;

	virt_addr = start_addr;
	fault = check_address_validity(cpu, start_addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31)) && BIT(inst, 21)) {
		cpu->Reg[Rn] = start_addr;
	}

	return fault;
}

fault_t LdnStM(IncrementBefore)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int i = BITS(inst, 0, 15);
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	unsigned int start_addr = rn + 4;

	virt_addr = start_addr;
	fault = check_address_validity(cpu, start_addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31)) && BIT(inst, 21)) {
		int count = 0;
		while(i) {
			if(i & 1) count ++;
			i = i >> 1;
		}
		cpu->Reg[Rn] += count * 4;
	}
	return fault;
}

fault_t LdnStM(IncrementAfter)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int i = BITS(inst, 0, 15);
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	unsigned int start_addr = rn;

	virt_addr = start_addr;
	fault = check_address_validity(cpu, start_addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31)) && BIT(inst, 21)) {
		int count = 0;
		while(i) {
			if(i & 1) count ++;
			i = i >> 1;
		}
		cpu->Reg[Rn] += count * 4;
	}
	return fault;
}

fault_t LdnStM(DecrementAfter)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int i = BITS(inst, 0, 15);
	int count = 0;
	while(i) {
		if(i & 1) count ++;
		i = i >> 1;
	}
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	unsigned int start_addr = rn - count * 4 + 4;

	virt_addr = start_addr;
	fault = check_address_validity(cpu, start_addr, &phys_addr, rw);
	if (fault) return fault;

	if (CondPassed(cpu, BITS(inst, 28, 31)) && BIT(inst, 21)) {
		cpu->Reg[Rn] -= count * 4;
	}
	return fault;
}

#define DEBUG_MSG	printf("in %s %d\n", __FUNCTION__, __LINE__);		\
			printf("inst is %x\n", inst);				\
			exit(0)
fault_t LnSWoUB(ScaledRegisterOffset)(arm_processor *cpu, unsigned int inst, unsigned int &virt_addr, unsigned int &phys_addr, unsigned int rw)
{
	fault_t fault;
	unsigned int shift = BITS(inst, 5, 6);
	unsigned int shift_imm = BITS(inst, 7, 11);
	unsigned int Rn = BITS(inst, 16, 19);
	unsigned int Rm = BITS(inst, 0, 3);
	unsigned int index;
	unsigned int addr;

	unsigned int rm = cpu->Reg[Rm];
	if (Rm == 15) rm += 8;
	unsigned int rn = cpu->Reg[Rn];
	if (Rn == 15) rn += 8;
	switch (shift) {
	case 0:
		//DEBUG_MSG;
		index = rm << shift_imm;
		break;
	case 1:
//		DEBUG_MSG;
		if (shift_imm == 0) {
			index = 0;
		} else {
			index = rm >> shift_imm;
		}
		break;
	case 2:
		DEBUG_MSG;
		break;
	case 3:
		DEBUG_MSG;
		break;
	}
	if (U_BIT) {
		addr = rn + index;
	} else
		addr = rn - index;

	virt_addr = addr;
	fault = check_address_validity(cpu, addr, &phys_addr, rw);
	return fault;
}

#define ISNEG(n)	(n < 0)
#define ISPOS(n)	(n >= 0)

enum {
	NON_BRANCH,
	DIRECT_BRANCH,
	INDIRECT_BRANCH,
	END_OF_PAGE
};

typedef struct _arm_inst {
	unsigned int idx;
	unsigned int cond;
	int br;
	int load_r15;
	char component[0];
} arm_inst;

typedef struct _adc_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} adc_inst;

typedef struct _add_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} add_inst;

typedef struct _orr_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} orr_inst;

typedef struct _and_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} and_inst;

typedef struct _eor_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} eor_inst;

typedef struct _bbl_inst {
	unsigned int L;
	int signed_immed_24;
	unsigned int next_addr;
	unsigned int jmp_addr;
} bbl_inst;

typedef struct _bx_inst {
	unsigned int Rm;
} bx_inst;

typedef struct _blx_inst {
	union {
		int32_t signed_immed_24;
		uint32_t Rm;
	} val;
	unsigned int inst;
} blx_inst;

typedef struct _clz_inst {
	unsigned int Rm;
	unsigned int Rd;
} clz_inst;

typedef struct _cps_inst {
	unsigned int imod0;
	unsigned int imod1;
	unsigned int mmod;
	unsigned int A, I, F;
	unsigned int mode;
} cps_inst;

typedef struct _clrex_inst {
} clrex_inst;

typedef struct _cpy_inst {
	unsigned int Rm;
	unsigned int Rd;
} cpy_inst;

typedef struct _bic_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} bic_inst;

typedef struct _sub_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} sub_inst;

typedef struct _tst_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} tst_inst;

typedef struct _cmn_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} cmn_inst;

typedef struct _teq_inst {
	unsigned int I;
	unsigned int Rn;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} teq_inst;

typedef struct _stm_inst {
	unsigned int inst;
} stm_inst;

struct bkpt_inst {
};

struct blx1_inst {
	unsigned int addr;
};

struct blx2_inst {
	unsigned int Rm;
};

typedef struct _stc_inst {
} stc_inst;

typedef struct _ldc_inst {
} ldc_inst;

typedef struct _swi_inst {
	unsigned int num;
} swi_inst;

typedef struct _cmp_inst {
	unsigned int I;
	unsigned int Rn;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} cmp_inst;

typedef struct _mov_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} mov_inst;

typedef struct _mvn_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} mvn_inst;

typedef struct _rev_inst {
	unsigned int Rd;
	unsigned int Rm;
} rev_inst;

typedef struct _rsb_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} rsb_inst;

typedef struct _rsc_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} rsc_inst;

typedef struct _sbc_inst {
	unsigned int I;
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int shifter_operand;
	shtop_fp_t shtop_func;
} sbc_inst;

typedef struct _mul_inst {
	unsigned int S;
	unsigned int Rd;
	unsigned int Rs;
	unsigned int Rm;
} mul_inst;

typedef struct _umull_inst {
	unsigned int S;
	unsigned int RdHi;
	unsigned int RdLo;
	unsigned int Rs;
	unsigned int Rm;
} umull_inst;

typedef struct _umlal_inst {
	unsigned int S;
	unsigned int Rm;
	unsigned int Rs;
	unsigned int RdHi;
	unsigned int RdLo;
} umlal_inst;

typedef struct _smlal_inst {
	unsigned int S;
	unsigned int Rm;
	unsigned int Rs;
	unsigned int RdHi;
	unsigned int RdLo;
} smlal_inst;

typedef struct _mla_inst {
	unsigned int S;
	unsigned int Rn;
	unsigned int Rd;
	unsigned int Rs;
	unsigned int Rm;
} mla_inst;

typedef struct _mrc_inst {
	unsigned int opcode_1;
	unsigned int opcode_2;
	unsigned int cp_num;
	unsigned int crn;
	unsigned int crm;
	unsigned int Rd;
	unsigned int inst;
} mrc_inst;

typedef struct _mcr_inst {
	unsigned int opcode_1;
	unsigned int opcode_2;
	unsigned int cp_num;
	unsigned int crn;
	unsigned int crm;
	unsigned int Rd;
	unsigned int inst;
} mcr_inst;

typedef struct _mrs_inst {
	unsigned int R;
	unsigned int Rd;
} mrs_inst;

typedef struct _msr_inst {
	unsigned int field_mask;
	unsigned int R;
	unsigned int inst;
} msr_inst;

typedef struct _pld_inst {
} pld_inst;

typedef struct _sxtb_inst {
	unsigned int Rd;
	unsigned int Rm;
	unsigned int rotate;
} sxtb_inst;

typedef struct _sxth_inst {
	unsigned int Rd;
	unsigned int Rm;
	unsigned int rotate;
} sxth_inst;

typedef struct _uxtah_inst {
	unsigned int Rn;
	unsigned int Rd;
	unsigned int rotate;
	unsigned int Rm;
} uxtah_inst;

typedef struct _uxth_inst {
	unsigned int Rd;
	unsigned int Rm;
	unsigned int rotate;
} uxth_inst;

typedef struct _uxtb_inst {
	unsigned int Rd;
	unsigned int Rm;
	unsigned int rotate;
} uxtb_inst;

typedef arm_inst * ARM_INST_PTR;

#define CACHE_BUFFER_SIZE	(64 * 1024 * 100)
char inst_buf[CACHE_BUFFER_SIZE];
int top = 0;
inline void *AllocBuffer(unsigned int size)
{
	int start = top;
	top += size;
	if (top > CACHE_BUFFER_SIZE) {
		fprintf(stderr, "inst_buf is full\n");
		exit(-1);
	}
	return (void *)&inst_buf[start];
}

int CondPassed(arm_processor *cpu, unsigned int cond)
{
	#define NFLAG		cpu->NFlag
	#define ZFLAG		cpu->ZFlag
	#define CFLAG		cpu->CFlag
	#define VFLAG		cpu->VFlag
	int temp;
	switch (cond) {
	case 0x0:
		temp = ZFLAG;
		break;
	case 0x1:   /* NE */
		temp = !ZFLAG;
		break;
	case 0x6:  /* VS */
		temp = VFLAG;
		break;
	case 0x7:        /* VC */
		temp = !VFLAG;
		break;
	case 0x4:         /* MI */
		temp = NFLAG;
		break;
	case 0x5:        /* PL */
		temp = !NFLAG;
		break;
	case 0x2:       /* CS */
		temp = CFLAG;
		break;
	case 0x3:      /* CC */
		temp = !CFLAG;
		break;
	case 0x8:     /* HI */
		temp = (CFLAG && !ZFLAG);
		break;
	case 0x9:    /* LS */
		temp = (!CFLAG || ZFLAG);
		break;
	case 0xa:   /* GE */
		temp = ((!NFLAG && !VFLAG) || (NFLAG && VFLAG));
		break;
	case 0xb:  /* LT */
		temp = ((NFLAG && !VFLAG) || (!NFLAG && VFLAG));
		break;
	case 0xc:  /* GT */
		temp = ((!NFLAG && !VFLAG && !ZFLAG)
			|| (NFLAG && VFLAG && !ZFLAG));
		break;
	case 0xd:  /* LE */
		temp = ((NFLAG && !VFLAG) || (!NFLAG && VFLAG))
			|| ZFLAG;
		break;
	case 0xe: /* AL */
		temp = 1;
		break;
	case 0xf:
//		printf("inst is %x\n");
//		printf("icounter is %lld\n", cpu->icounter);
//		exit(-1);
		temp = 1;
		break;
	}
	return temp;
}

enum DECODE_STATUS {
	DECODE_SUCCESS,
	DECODE_FAILURE
};

#if 0
struct instruction_set_encoding_item {
        const char *name;
        int attribute_value;
        int version;
        unsigned int content[12];
};

typedef struct instruction_set_encoding_item ISEITEM;

enum ARMVER {
        ARMALL,
        ARMV4,
        ARMV4T,
        ARMV5T,
        ARMV5TE,
        ARMV5TEJ,
        ARMV6,
	ARM1176JZF_S
};


static const ISEITEM arm_instruction[] = {
        {"adc",         2,      ARMALL,         26, 27, 0,      21, 24, 5},
        {"add",         2,      ARMALL,         26, 27, 0,      21, 24, 4},
        {"and",         2,      ARMALL,         26, 27, 0,      21, 24, 0},
        {"b,bl",        1,      ARMALL,         25, 27, 5},
        {"bic",         2,      ARMALL,         26, 27, 0,      21, 24, 14},
        {"bkpt",        2,      ARMV5T,         20, 31, 0xe12,  4, 7, 7},
        {"blx(1)",      1,      ARMV5T,         25, 31, 0x7d},
        {"blx(2)",      2,      ARMV5T,         20, 27, 0x12,   4, 7, 3},
        {"bx",          2,      ARMV4T,         20, 27, 0x12,   4, 7, 1},
        {"bxj",         2,      ARMV5TEJ,       20, 27, 0x12,   4, 7, 2},
        {"cdp",         2,      ARMALL,         24, 27, 0xe,    4, 4, 0},
	{"clrex",	1,	ARMV6,		0, 31, 0xf57ff01f},
        {"clz",         2,      ARMV5T,         20, 27, 0x16,   4, 7, 1},
        {"cmn",         2,      ARMALL,         26, 27, 0,      20, 24, 0x17},
        {"cmp",         2,      ARMALL,         26, 27, 0,      20, 24, 0x15},
        {"cps",         3,      ARMV6,          20, 31, 0xf10,  16, 16, 0,      5, 5, 0},
        {"cpy",         2,      ARMV6,          20, 27, 0x1a,   4, 11, 0},
        {"eor",         2,      ARMALL,         26, 27, 0,      21, 24, 1},
        {"ldc",         2,      ARMALL,         25, 27, 6,      20, 20, 1},
        {"ldm(1)",      3,      ARMALL,         25, 27, 4,      22, 22, 0,      20, 20, 1},
        {"ldm(2)",      3,      ARMALL,         25, 27, 4,      20, 22, 5,      15, 15, 0},
        {"ldm(3)",      4,      ARMALL,         25, 27, 4,      22, 22, 1,      20 ,20, 1,      15, 15, 1},
        {"sxth",        2,      ARMV6,          16, 27, 0x6bf,  4, 7, 7},
        {"ldr",         3,      ARMALL,         26, 27, 1,      22, 22, 0,      20, 20, 1},
        {"uxth",        2,      ARMV6,          16, 27, 0x6ff,  4, 7, 7},
        {"uxtah",       2,      ARMV6,          20, 27, 0x6f,   4, 7, 7},
        {"ldrb",        3,      ARMALL,         26, 27, 1,      22, 22, 1,      20, 20, 1},
        {"ldrbt",       3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 7},
        {"ldrd",        3,      ARMV5TE,        25, 27, 0,      20, 20, 0,      4, 7, 0xd},
        {"ldrex",       2,      ARMALL,         20, 27, 0x19,   4, 7, 9},
        {"ldrexb",	2,	ARM1176JZF_S,	20, 27, 0x1d,	4, 7, 9},
        {"ldrh",        3,      ARMALL,         25, 27, 0,      20, 20, 1,      4, 7, 0xb},
        {"ldrsb",       3,      ARMV4T,         25, 27, 0,      20, 20, 1,      4, 7, 0xd},
        {"ldrsh",       3,      ARMV4T,         25, 27, 0,      20, 20, 1,      4, 7, 0xf},
        {"ldrt",        3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 3},
        {"mcr",         3,      ARMALL,         24, 27, 0xe,    20, 20, 0,      4, 4, 1},
        {"mcrr",        1,      ARMV6,          20, 27, 0xc4},
        {"mla",         2,      ARMALL,         21, 27, 1,      4, 7, 9},
        {"mov",         2,      ARMALL,         26, 27, 0,      21, 24, 0xd},
        {"mrc",         3,      ARMV6,          24, 27, 0xe,    20, 20, 1,      4, 4, 1},
        {"mrrc",        1,      ARMV6,          20, 27, 0xc5},
        {"mrs",         4,      ARMALL,         23, 27, 2,      20, 21, 0,	16, 19, 0xf,	0, 11, 0},
        {"msr",         2,      ARMALL,         23, 27, 6,      20, 21, 2},
        {"msr",         3,      ARMALL,         23, 27, 2,      20, 21, 2,	4, 7, 0},
        {"mul",         2,      ARMALL,         21, 27, 0,      4, 7, 9},
        {"mvn",         2,      ARMALL,         26, 27, 0,      21, 24, 0xf},
        {"orr",         2,      ARMALL,         26, 27, 0,      21, 24, 0xc},
        {"pkhbt",       2,      ARMV6,          20, 27, 0x68,   4, 6, 1},
        {"pkhtb",       2,      ARMV6,          20, 27, 0x68,   4, 6, 5},
        {"pld",         4,      ARMV5TE,        26, 31, 0x3d,   24, 24, 1,      20, 22, 5,      12, 15, 0xf},
        {"qadd",        2,      ARMV5TE,        20, 27, 0x10,   4, 7, 5},
        {"qadd16",      2,      ARMV6,          20, 27, 0x62,   4, 7, 1},
        {"qadd8",       2,      ARMV6,          20, 27, 0x62,   4, 7, 9},
        {"qaddsubx",    2,      ARMV6,          20, 27, 0x62,   4, 7, 3},
        {"qdadd",       2,      ARMV5TE,        20, 27, 0x14,   4, 7, 5},
        {"qdsub",       2,      ARMV5TE,        20, 27, 0x16,   4, 7, 5},
        {"qsub",        2,      ARMV5TE,        20, 27, 0x12,   4, 7, 5},
        {"qsub16",      2,      ARMV6,          20, 27, 0x62,   4, 7, 7},
        {"qsub8",       2,      ARMV6,          20, 27, 0x62,   4, 7, 0xf},
        {"qsubaddx",    2,      ARMV6,          20, 27, 0x62,   4, 7, 5},
        {"rev",         2,      ARMV6,          20, 27, 0x6b,   4, 7, 3},
        {"revsh",       2,      ARMV6,          20, 27, 0x6f,   4, 7, 0xb},
        {"rfe",         4,      ARMV6,          25, 31, 0x7c,   22, 22, 0,      20, 20, 1,      8, 11, 0xa},
        {"rsb",         2,      ARMALL,         26, 27, 0,      21, 24, 3},
        {"rsc",         2,      ARMALL,         26, 27, 0,      21, 24, 7},
        {"sadd16",      2,      ARMV6,          20, 27, 0x61,   4, 7, 1},
        {"sadd8",       2,      ARMV6,          20, 27, 0x61,   4, 7, 9},
        {"saddsubx",    2,      ARMV6,          20, 27, 0x61,   4, 7, 3},
        {"sbc",         2,      ARMALL,         26, 27, 0,      21, 24, 6},
        {"sel",         2,      ARMV6,          20, 27, 0x68,   4, 7, 0xb},
        {"setend",      2,      ARMV6,          16, 31, 0xf101, 4, 7, 0},
        {"shadd16",     2,      ARMV6,          20, 27, 0x63,   4, 7, 1},
        {"shadd8",      2,      ARMV6,          20, 27, 0x63,   4, 7, 9},
        {"shaddsubx",   2,      ARMV6,          20, 27, 0x63,   4, 7, 3},
        {"shsub16",     2,      ARMV6,          20, 27, 0x63,   4, 7, 7},
        {"shsub8",      2,      ARMV6,          20, 27, 0x63,   4, 7, 0xf},
        {"shsubaddx",   2,      ARMV6,          20, 27, 0x63,   4, 7, 5},
        {"smla<x><y>",  3,      ARMV5TE,        20, 27, 0x10,   7, 7, 1,        4, 4, 0},
        {"smlad",       3,      ARMV6,          20, 27, 0x70,   6, 7, 0,        4, 4, 0},
        {"smlal",       2,      ARMALL,         21, 27, 0x7,    4, 7, 9},
        {"smlal<x><y>", 3,      ARMV5TE,        20, 27, 0x14,   7, 7, 1,        4, 4, 0},
        {"smlald",      3,      ARMV6,          20, 27, 0x74,   6, 7, 0,        4, 4, 1},
        {"smlaw<y>",    3,      ARMV5TE,        20, 27, 0x12,   7, 7, 0,        4, 5, 0},
        {"smlsd",       3,      ARMV6,          20, 27, 0x70,   6, 7, 1,        4, 4, 1},
        {"smlsld",      3,      ARMV6,          20, 27, 0x74,   6, 7, 1,        4, 4, 1},
        {"smmla",       3,      ARMV6,          20, 27, 0x75,   6, 7, 0,        4, 4, 1},
        {"smmls",       3,      ARMV6,          20, 27, 0x75,   6, 7, 3,        4, 4, 1},
        {"smmul",       4,      ARMV6,          20, 27, 0x75,   12, 15, 0xf,    6, 7, 0,        4, 4, 1},
        {"smuad",       4,      ARMV6,          20, 27, 0x70,   12, 15, 0xf,    6, 7, 0,        4, 4, 1},
        {"smul<x><y>",  3,      ARMV5TE,        20, 27, 0x16,   7, 7, 1,        4, 4, 0},
        {"smull",       2,      ARMALL,         21, 27, 6,      4, 7, 9},
        {"smulw<y>",    3,      ARMV5TE,        20, 27, 0x12,   7, 7, 1,        4, 5, 2},
        {"smusd",       4,      ARMV6,          20, 27, 0x70,   12, 15, 0xf,    6, 7, 1,        4, 4, 1},
        {"srs",         4,      ARMV6,          25, 31, 0x3c,   22, 22, 1,      16, 20, 0xd,    8, 11, 5},
        {"ssat",        2,      ARMV6,          21, 27, 0x35,   4, 5, 1},
        {"ssat16",      2,      ARMV6,          20, 27, 0x6a,   4, 7, 3},
        {"ssub16",      2,      ARMV6,          20, 27, 0x61,   4, 7, 7},
        {"ssub8",       2,      ARMV6,          20, 27, 0x61,   4, 7, 0xf},
        {"ssubaddx",    2,      ARMV6,          20, 27, 0x61,   4, 7, 5},
        {"stc",         2,      ARMALL,         25, 27, 6,      20, 20, 0},
        {"stm(1)",      3,      ARMALL,         25, 27, 4,      22, 22, 0,      20, 20, 0},
        {"stm(2)",      2,      ARMALL,         25, 27, 4,      20, 22, 4},
        {"sxtb",        2,      ARMV6,          16, 27, 0x6af,  4, 7, 7},
        {"str",         3,      ARMALL,         26, 27, 1,      22, 22, 0,      20, 20, 0},
        {"uxtb",        2,      ARMV6,          16, 27, 0x6ef,  4, 7, 7},
        {"uxtab",       2,      ARMV6,          20, 27, 0x6e,   4, 9, 7},
        {"strb",        3,      ARMALL,         26, 27, 1,      22, 22, 1,      20, 20, 0},
        {"strbt",       3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 6},
        {"strd",        3,      ARMV5TE,        25, 27, 0,      20, 20, 0,      4, 7, 0xf},//ARMv5TE and above, excluding ARMv5TExP
        {"strex",       2,      ARMV6,          20, 27, 0x18,   4, 7, 9},
	{"strexb",	2,	ARM1176JZF_S,	20, 27, 0x1c,	4, 7, 9},
        {"strh",        3,      ARMALL,         25, 27, 0,      20, 20, 0,      4, 7, 0xb},
        {"strt",        3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 2},
        {"sub",         2,      ARMALL,         26, 27, 0,      21, 24, 2},
        {"swi",         1,      ARMALL,         24, 27, 0xf},
        {"swp",         2,      ARMALL,         20, 27, 0x10,   4, 7, 9},
        {"swpb",        2,      ARMALL,         20, 27, 0x14,   4, 7, 9},
        {"sxtab",       2,      ARMV6,          20, 27, 0x6a,   4, 7, 7},
        {"sxtab16",     2,      ARMV6,          20, 27, 0x68,   4, 7, 7},
        {"sxtah",       2,      ARMV6,          20, 27, 0x6b,   4, 7, 7},
        {"sxtb16",      2,      ARMV6,          16, 27, 0x68f,  4, 7, 7},
        {"teq",         2,      ARMALL,         26, 27, 0,      20, 24, 0x13},
        {"tst",         2,      ARMALL,         26, 27, 0,      20, 24, 0x11},
        {"uadd16",      2,      ARMV6,          20, 27, 0x65,   4, 7, 1},
        {"uadd8",       2,      ARMV6,          20, 27, 0x65,   4, 7, 9},
        {"uaddsubx",    2,      ARMV6,          20, 27, 0x65,   4, 7, 3},
        {"uhadd16",     2,      ARMV6,          20, 27, 0x67,   4, 7, 1},
        {"uhadd8",      2,      ARMV6,          20, 27, 0x67,   4, 7, 9},
        {"uhaddsubx",   2,      ARMV6,          20, 27, 0x67,   4, 7, 3},
        {"uhsub16",     2,      ARMV6,          20, 27, 0x67,   4, 7, 7},
        {"uhsub8",      2,      ARMV6,          20, 27, 0x67,   4, 7, 0xf},
        {"uhsubaddx",   2,      ARMV6,          20, 27, 0x67,   4, 7, 5},
        {"umaal",       2,      ARMV6,          20, 27, 4,      4, 7, 9},
        {"umlal",       2,      ARMALL,         21, 27, 5,      4, 7, 9},
        {"umull",       2,      ARMALL,         21, 27, 4,      4, 7, 9},
        {"uqadd16",     2,      ARMV6,          20, 27, 0x66,   4, 7, 1},
        {"uqadd8",      2,      ARMV6,          20, 27, 0x66,   4, 7, 9},
        {"uqaddsubx",   2,      ARMV6,          20, 27, 0x66,   4, 7, 3},
        {"uqsub16",     2,      ARMV6,          20, 27, 0x66,   4, 7, 7},
        {"uqsub8",      2,      ARMV6,          20, 27, 0x66,   4, 7, 0xf},
        {"uqsubaddx",   2,      ARMV6,          20, 27, 0x66,   4, 7, 5},
        {"usad8",       3,      ARMV6,          20, 27, 0x78,   12, 15, 0xf,    4, 7, 1},
        {"usada8",      2,      ARMV6,          20, 27, 0x78,   4, 7, 1},
        {"usat",        2,      ARMV6,          21, 27, 0x37,   4, 5, 1},
        {"usat16",      2,      ARMV6,          20, 27, 0x6e,   4, 7, 3},
        {"usub16",      2,      ARMV6,          20, 27, 0x65,   4, 7, 7},
        {"usub8",       2,      ARMV6,          20, 27, 0x65,   4, 7, 0xf},
        {"usubaddx",    2,      ARMV6,          20, 27, 0x65,   4, 7, 5},
        {"uxtab16",     2,      ARMV6,          20, 27, 0x6c,   4, 7, 7},
        {"uxtb16",      2,      ARMV6,          16, 27, 0x6cf,  4, 7, 7}
};


static const ISEITEM arm_exclusion_code[] = {
        {"adc",         3,      ARMALL,		4, 4, 1,	7, 7, 1,	25, 25, 0}, 
        {"add",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0}, 
        {"and",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"b,bl",        0,      ARMALL, 	0},        
        {"bic",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"bkpt",        0,      ARMV5T, 0},        
        {"blx(1)",      0,      ARMV5T, 0},        
        {"blx(2)",      0,      ARMV5T, 0},        
        {"bx",          0,      ARMV4T, 0},        
        {"bxj",         0,      ARMV5TEJ, 0},      
        {"cdp",         0,      ARMALL, 0},        
	{"clrex",	0,	ARMV6,	0},
        {"clz",         0,      ARMV5T, 0},        
        {"cmn",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"cmp",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"cps",         0,      ARMV6, 0},         
        {"cpy",         0,      ARMV6, 0},         
        {"eor",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"ldc",         0,      ARMALL, 0},        
        {"ldm(1)",      0,      ARMALL, 0},        
        {"ldm(2)",      0,      ARMALL, 0},        
        {"ldm(3)",      0,      ARMALL, 0},        
        {"sxth",        0,      ARMV6, 0},         
        {"ldr",         0,      ARMALL, 0},        
        {"uxth",        0,      ARMV6, 0},
        {"uxtah",       0,      ARMV6, 0},         
        {"ldrb",        0,      ARMALL, 0},        
        {"ldrbt",       0,      ARMALL, 0},        
        {"ldrd",        0,      ARMV5TE, 0},       
        {"ldrex",       0,      ARMALL, 0},        
	{"ldrexb",	0,	ARM1176JZF_S, 0},	
        {"ldrh",        0,      ARMALL, 0},        
        {"ldrsb",       0,      ARMV4T, 0},        
        {"ldrsh",       0,      ARMV4T, 0},        
        {"ldrt",        0,      ARMALL, 0},        
        {"mcr",         0,      ARMALL, 0},        
        {"mcrr",        0,      ARMV6, 0},         
        {"mla",         0,      ARMALL, 0},        
        {"mov",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"mrc",         0,      ARMV6, 0},         
        {"mrrc",        0,      ARMV6, 0},         
        {"mrs",         0,      ARMALL, 0},        
        {"msr",         0,      ARMALL, 0},        
        {"msr",         0,      ARMALL, 0},        
        {"mul",         0,      ARMALL, 0},        
        {"mvn",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"orr",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"pkhbt",       0,      ARMV6, 0},         
        {"pkhtb",       0,      ARMV6, 0},         
        {"pld",         0,      ARMV5TE, 0},       
        {"qadd",        0,      ARMV5TE, 0},       
        {"qadd16",      0,      ARMV6, 0},         
        {"qadd8",       0,      ARMV6, 0},         
        {"qaddsubx",    0,      ARMV6, 0},         
        {"qdadd",       0,      ARMV5TE, 0},       
        {"qdsub",       0,      ARMV5TE, 0},       
        {"qsub",        0,      ARMV5TE, 0},       
        {"qsub16",      0,      ARMV6, 0},         
        {"qsub8",       0,      ARMV6, 0},         
        {"qsubaddx",    0,      ARMV6, 0},         
        {"rev",         0,      ARMV6, 0},         
        {"revsh",       0,      ARMV6, 0},         
        {"rfe",         0,      ARMV6, 0},         
        {"rsb",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"rsc",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"sadd16",      0,      ARMV6, 0},         
        {"sadd8",       0,      ARMV6, 0},         
        {"saddsubx",    0,      ARMV6, 0},         
        {"sbc",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"sel",         0,      ARMV6, 0},         
        {"setend",      0,      ARMV6, 0},         
        {"shadd16",     0,      ARMV6, 0},         
        {"shadd8",      0,      ARMV6, 0},         
        {"shaddsubx",   0,      ARMV6, 0},         
        {"shsub16",     0,      ARMV6, 0},         
        {"shsub8",      0,      ARMV6, 0},         
        {"shsubaddx",   0,      ARMV6, 0},         
        {"smla<x><y>",  0,      ARMV5TE, 0},       
        {"smlad",       0,      ARMV6, 0},         
        {"smlal",       0,      ARMALL, 0},        
        {"smlal<x><y>", 0,      ARMV5TE, 0},       
        {"smlald",      0,      ARMV6, 0},         
        {"smlaw<y>",    0,      ARMV5TE, 0},       
        {"smlsd",       0,      ARMV6, 0},         
        {"smlsld",      0,      ARMV6, 0},         
        {"smmla",       0,      ARMV6, 0},         
        {"smmls",       0,      ARMV6, 0},         
        {"smmul",       0,      ARMV6, 0},         
        {"smuad",       0,      ARMV6, 0},         
        {"smul<x><y>",  0,      ARMV5TE, 0},       
        {"smull",       0,      ARMALL, 0},        
        {"smulw<y>",    0,      ARMV5TE, 0},       
        {"smusd",       0,      ARMV6, 0},         
        {"srs",         0,      ARMV6, 0},         
        {"ssat",        0,      ARMV6, 0},         
        {"ssat16",      0,      ARMV6, 0},         
        {"ssub16",      0,      ARMV6, 0},         
        {"ssub8",       0,      ARMV6, 0},         
        {"ssubaddx",    0,      ARMV6, 0},         
        {"stc",         0,      ARMALL, 0},        
        {"stm(1)",      0,      ARMALL, 0},        
        {"stm(2)",      0,      ARMALL, 0},        
        {"sxtb",        0,      ARMV6, 0},         
        {"str",         0,      ARMALL, 0},        
        {"uxtb",        0,      ARMV6, 0},         
        {"uxtab",       0,      ARMV6, 0},         
        {"strb",        0,      ARMALL, 0},        
        {"strbt",       0,      ARMALL, 0},        
        {"strd",        0,      ARMV5TE, 0},       
        {"strex",       0,      ARMV6, 0},         
	{"strexb",	0,	ARM1176JZF_S, 0},	
        {"strh",        0,      ARMALL, 0},        
        {"strt",        0,      ARMALL, 0},        
        {"sub",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"swi",         0,      ARMALL, 0},        
        {"swp",         0,      ARMALL, 0},        
        {"swpb",        0,      ARMALL, 0},        
        {"sxtab",       0,      ARMV6, 0},         
        {"sxtab16",     0,      ARMV6, 0},         
        {"sxtah",       0,      ARMV6, 0},         
        {"sxtb16",      0,      ARMV6, 0},         
        {"teq",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"tst",         3,      ARMALL, 	4, 4, 1,	7, 7, 1,	25, 25, 0},        
        {"uadd16",      0,      ARMV6, 0},         
        {"uadd8",       0,      ARMV6, 0},         
        {"uaddsubx",    0,      ARMV6, 0},         
        {"uhadd16",     0,      ARMV6, 0},         
        {"uhadd8",      0,      ARMV6, 0},         
        {"uhaddsubx",   0,      ARMV6, 0},         
        {"uhsub16",     0,      ARMV6, 0},         
        {"uhsub8",      0,      ARMV6, 0},         
        {"uhsubaddx",   0,      ARMV6, 0},         
        {"umaal",       0,      ARMV6, 0},         
        {"umlal",       0,      ARMALL, 0},        
        {"umull",       0,      ARMALL, 0},        
        {"uqadd16",     0,      ARMV6, 0},         
        {"uqadd8",      0,      ARMV6, 0},         
        {"uqaddsubx",   0,      ARMV6, 0},         
        {"uqsub16",     0,      ARMV6, 0},         
        {"uqsub8",      0,      ARMV6, 0},         
        {"uqsubaddx",   0,      ARMV6, 0},         
        {"usad8",       0,      ARMV6, 0},         
        {"usada8",      0,      ARMV6, 0},         
        {"usat",        0,      ARMV6, 0},         
        {"usat16",      0,      ARMV6, 0},         
        {"usub16",      0,      ARMV6, 0},         
        {"usub8",       0,      ARMV6, 0},         
        {"usubaddx",    0,      ARMV6, 0},         
        {"uxtab16",     0,      ARMV6, 0},         
        {"uxtb16",      0,      ARMV6, 0}         
};


static int decode_arm_instr(uint32_t instr, int32_t *idx)
{
	int n = 0;
	int base = 0;
	int ret = DECODE_FAILURE;
	int i = 0;
	int instr_slots = sizeof(arm_instruction)/sizeof(ISEITEM);
	for (i = 0; i < instr_slots; i++)
	{
//		ret = DECODE_SUCCESS;
		n = arm_instruction[i].attribute_value;
		base = 0;
		while (n) {
			if (arm_instruction[i].content[base + 1] == 31 && arm_instruction[i].content[base] == 0) {
				/* clrex */
				if (instr != arm_instruction[i].content[base + 2]) {
					break;
				}
			} else if (BITS(instr, arm_instruction[i].content[base], arm_instruction[i].content[base + 1]) != arm_instruction[i].content[base + 2]) {
				break;
			}
			base += 3;
			n --;
		}
		//All conditions are satisfied.
		if (n == 0)
			ret = DECODE_SUCCESS;

		if (ret == DECODE_SUCCESS) {
			n = arm_exclusion_code[i].attribute_value;
			if (n != 0) {
				base = 0;
				while (n) {
					if (BITS(instr, arm_exclusion_code[i].content[base], arm_exclusion_code[i].content[base + 1]) != arm_exclusion_code[i].content[base + 2]) {
						break;					}
					base += 3;
					n --;
				}
				//All conditions is satisfied.
				if (n == 0)
					ret = DECODE_FAILURE;
			}
		}

		if (ret == DECODE_SUCCESS) {
			*idx = i;
			return ret;
		}
	}
	return ret;
}
#endif

int decode_arm_instr(uint32_t instr, int32_t *idx);

shtop_fp_t get_shtop(unsigned int inst)
{
	if (BIT(inst, 25)) {
		return DPO(Immediate);
	} else if (BITS(inst, 4, 11) == 0) {
		return DPO(Register);
	} else if (BITS(inst, 4, 6) == 0) {
		return DPO(LogicalShiftLeftByImmediate);
	} else if (BITS(inst, 4, 7) == 1) {
		return DPO(LogicalShiftLeftByRegister);
	} else if (BITS(inst, 4, 6) == 2) {
		return DPO(LogicalShiftRightByImmediate);
	} else if (BITS(inst, 4, 7) == 3) {
		return DPO(LogicalShiftRightByRegister);
	} else if (BITS(inst, 4, 6) == 4) {
		return DPO(ArithmeticShiftRightByImmediate);
	} else if (BITS(inst, 4, 7) == 5) {
		return DPO(ArithmeticShiftRightByRegister);
	} else if (BITS(inst, 4, 6) == 6) {
		return DPO(RotateRightByImmediate);
	} else if (BITS(inst, 4, 7) == 7) {
		return DPO(RotateRightByRegister);
	}
}

get_addr_fp_t get_calc_addr_op(unsigned int inst)
{
	/* 1 */
	if (BITS(inst, 24, 27) == 5 && BIT(inst, 21) == 0) {
//		printf("line is %d", __LINE__);
		return LnSWoUB(ImmediateOffset);
	} else if (BITS(inst, 24, 27) == 7 && BIT(inst, 21) == 0 && BITS(inst, 4, 11) == 0) {
//		DEBUG_MSG;
//		printf("line is %d", __LINE__);
		return LnSWoUB(RegisterOffset);
	} else if (BITS(inst, 24, 27) == 7 && BIT(inst, 21) == 0 && BIT(inst, 4) == 0) {
//		DEBUG_MSG;
//		printf("line is %d", __LINE__);
		return LnSWoUB(ScaledRegisterOffset);
	} else if (BITS(inst, 24, 27) == 5 && BIT(inst, 21) == 1) {
//		printf("line is %d", __LINE__);
		return LnSWoUB(ImmediatePreIndexed);
	} else if (BITS(inst, 24, 27) == 7 && BIT(inst, 21) == 1 && BITS(inst, 4, 11) == 0) {
		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 7 && BIT(inst, 21) == 1 && BIT(inst, 4) == 0) {
		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 4 && BIT(inst, 21) == 0) {
		return LnSWoUB(ImmediatePostIndexed);
	} else if (BITS(inst, 24, 27) == 6 && BIT(inst, 21) == 0 && BITS(inst, 4, 11) == 0) {
		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 6 && BIT(inst, 21) == 0 && BIT(inst, 4) == 0) {
		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 1 && BITS(inst, 21, 22) == 2 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
	/* 2 */
//		printf("line is %d", __LINE__);
		return MLnS(ImmediateOffset);
//		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 1 && BITS(inst, 21, 22) == 0 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
//		printf("line is %d\n", __LINE__);
		return MLnS(RegisterOffset);
//		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 1 && BITS(inst, 21, 22) == 3 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
//		printf("line is %d\n", __LINE__);
		return MLnS(ImmediatePreIndexed);
//		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 1 && BITS(inst, 21, 22) == 1 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
		DEBUG_MSG;
	} else if (BITS(inst, 24, 27) == 0 && BITS(inst, 21, 22) == 2 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
//		DEBUG_MSG;
		return MLnS(ImmediatePostIndexed);
	} else if (BITS(inst, 24, 27) == 0 && BITS(inst, 21, 22) == 0 && BIT(inst, 7) == 1 && BIT(inst, 4) == 1) {
		DEBUG_MSG;
	} else if (BITS(inst, 23, 27) == 0x11) {
	/* 3 */
//		DEBUG_MSG;
//		printf("line is %d", __LINE__);
		return LdnStM(IncrementAfter);
	} else if (BITS(inst, 23, 27) == 0x13) {
//		printf("line is %d", __LINE__);
		return LdnStM(IncrementBefore);
//		DEBUG_MSG;
	} else if (BITS(inst, 23, 27) == 0x10) {
//		DEBUG_MSG;
//		printf("line is %d", __LINE__);
		return LdnStM(DecrementAfter);
	} else if (BITS(inst, 23, 27) == 0x12) {
//		DEBUG_MSG;
//		printf("line is %d", __LINE__);
		return LdnStM(DecrementBefore);
	}
	#if 0
	printf("In %s Unknown addressing mode\n", __FUNCTION__);
	printf("inst:%x\n", inst);
	exit(-1);
	#endif
	return NULL;
}

#define INTERPRETER_TRANSLATE(s) glue(InterpreterTranslate_, s)

#define CHECK_RN			(inst_cream->Rn == 15)
#define CHECK_RM			(inst_cream->Rm == 15)
#define CHECK_RS			(inst_cream->Rs == 15)


ARM_INST_PTR INTERPRETER_TRANSLATE(adc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(adc_inst));
	adc_inst *inst_cream = (adc_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(add)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(add_inst));
	add_inst *inst_cream = (add_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(and)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(and_inst));
	and_inst *inst_cream = (and_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) 
		inst_base->br = INDIRECT_BRANCH;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(bbl)(unsigned int inst, int index)
{
	#define POSBRANCH ((inst & 0x7fffff) << 2)
	#define NEGBRANCH ((0xff000000 |(inst & 0xffffff)) << 2)

	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(bbl_inst));
	bbl_inst *inst_cream = (bbl_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = DIRECT_BRANCH;

	inst_cream->L 	 = BIT(inst, 24);
	inst_cream->signed_immed_24 = BIT(inst, 23) ? NEGBRANCH : POSBRANCH;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(bic)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(bic_inst));
	bic_inst *inst_cream = (bic_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	if (inst_cream->Rd == 15) 
		inst_base->br = INDIRECT_BRANCH;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(bkpt)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(blx)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(blx_inst));
	blx_inst *inst_cream = (blx_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = INDIRECT_BRANCH;

	inst_cream->inst = inst;
	if (BITS(inst, 20, 27) == 0x12 && BITS(inst, 4, 7) == 0x3) {
		inst_cream->val.Rm = BITS(inst, 0, 3);
	} else {
		printf("inst is %x\n", inst);
		exit(-1);
//		DEBUG_MSG;
	}

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(bx)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(bx_inst));
	bx_inst *inst_cream = (bx_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = INDIRECT_BRANCH;

	inst_cream->Rm	 = BITS(inst, 0, 3);

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(bxj)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(cdp)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(clrex)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(clrex_inst));
	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(clz)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(clz_inst));
	clz_inst *inst_cream = (clz_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rm   = BITS(inst,  0,  3);
	inst_cream->Rd   = BITS(inst, 12, 15);
	if (CHECK_RM) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(cmn)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(cmn_inst));
	cmn_inst *inst_cream = (cmn_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(cmp)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(cmp_inst));
	cmp_inst *inst_cream = (cmp_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(cps)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(cps_inst));
	cps_inst *inst_cream = (cps_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->imod0 = BIT(inst, 18);
	inst_cream->imod1 = BIT(inst, 19);
	inst_cream->mmod  = BIT(inst, 17);
	inst_cream->A	  = BIT(inst, 8);
	inst_cream->I	  = BIT(inst, 7);
	inst_cream->F	  = BIT(inst, 6);
	inst_cream->mode  = BITS(inst, 0, 4);

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(cpy)(unsigned int inst, int index)
{
	#if 1
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mov_inst));
	mov_inst *inst_cream = (mov_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	#endif
	#if 0
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(cpy_inst));
	cpy_inst *inst_cream = (cpy_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->Rd = BITS(inst, 12, 15);
	inst_cream->Rm = BITS(inst,  0,  3);
	#endif
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(eor)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(eor_inst));
	eor_inst *inst_cream = (eor_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldc_inst));
	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldm)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BIT(inst, 15)) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxth)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(sxtb_inst));
	sxtb_inst *inst_cream = (sxtb_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rd     = BITS(inst, 12, 15);
	inst_cream->Rm     = BITS(inst,  0,  3);
	inst_cream->rotate = BITS(inst, 10, 11);
	if (CHECK_RM) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldr)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxth)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(uxth_inst));
	uxth_inst *inst_cream = (uxth_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rd     = BITS(inst, 12, 15);
	inst_cream->rotate = BITS(inst, 10, 11);
	inst_cream->Rm     = BITS(inst,  0,  3);
	if (CHECK_RM) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxtah)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(uxtah_inst));
	uxtah_inst *inst_cream = (uxtah_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rn     = BITS(inst, 16, 19);
	inst_cream->Rd     = BITS(inst, 12, 15);
	inst_cream->rotate = BITS(inst, 10, 11);
	inst_cream->Rm     = BITS(inst,  0,  3);
	if (CHECK_RM || CHECK_RN) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrbt)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	if (I_BIT == 0) {
		inst_cream->get_addr = LnSWoUB(ImmediatePostIndexed);
	} else {
		DEBUG_MSG;
	}
	#if 0
	inst_cream->get_addr = get_calc_addr_op(inst);
	if(inst == 0x54f13001) {
		printf("get_calc_addr_op:%llx\n", inst_cream->get_addr);
	}
	#endif

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrex)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrexb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrh)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrsb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrsh)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(ldrt)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	if (I_BIT == 0) {
		inst_cream->get_addr = LnSWoUB(ImmediatePostIndexed);
	} else {
		DEBUG_MSG;
	}
//	printf("get_calc_addr_op:%llx\n", inst_cream->get_addr);
	#if 0
	inst_cream->get_addr = get_calc_addr_op(inst);
	if(inst == 0x54f13001) {
	}
	#endif

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mcr)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mcr_inst));
	mcr_inst *inst_cream = (mcr_inst *)inst_base->component;
	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->crn      = BITS(inst, 16, 19);
	inst_cream->crm      = BITS(inst,  0,  3);
	inst_cream->opcode_1 = BITS(inst, 21, 23);
	inst_cream->opcode_2 = BITS(inst,  5,  7);
	inst_cream->Rd       = BITS(inst, 12, 15);
	inst_cream->cp_num   = BITS(inst,  8, 11);
	inst_cream->inst     = inst;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mcrr)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(mla)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mla_inst));
	mla_inst *inst_cream = (mla_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 12, 15);
	inst_cream->Rd	 = BITS(inst, 16, 19);
	inst_cream->Rs	 = BITS(inst,  8, 11);
	inst_cream->Rm	 = BITS(inst,  0,  3);

	if (CHECK_RM || CHECK_RN || CHECK_RS) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mov)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mov_inst));
	mov_inst *inst_cream = (mov_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mrc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mrc_inst));
	mrc_inst *inst_cream = (mrc_inst *)inst_base->component;
	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->crn      = BITS(inst, 16, 19);
	inst_cream->crm      = BITS(inst,  0,  3);
	inst_cream->opcode_1 = BITS(inst, 21, 23);
	inst_cream->opcode_2 = BITS(inst,  5,  7);
	inst_cream->Rd       = BITS(inst, 12, 15);
	inst_cream->cp_num   = BITS(inst,  8, 11);
	inst_cream->inst     = inst;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mrrc)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(mrs)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mrs_inst));
	mrs_inst *inst_cream = (mrs_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->Rd   = BITS(inst, 12, 15);
	inst_cream->R    = BIT(inst, 22);

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(msr)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(msr_inst));
	msr_inst *inst_cream = (msr_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->field_mask = BITS(inst, 16, 19);
	inst_cream->R          = BIT(inst, 22);
	inst_cream->inst       = inst;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mul)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mul_inst));
	mul_inst *inst_cream = (mul_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rm	 = BITS(inst, 0, 3);
	inst_cream->Rs	 = BITS(inst, 8, 11);
	inst_cream->Rd	 = BITS(inst, 16, 19);

	if (CHECK_RM || CHECK_RS) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(mvn)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(mvn_inst));
	mvn_inst *inst_cream = (mvn_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;

}
ARM_INST_PTR INTERPRETER_TRANSLATE(orr)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(orr_inst));
	orr_inst *inst_cream = (orr_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(pkhbt)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(pkhtb)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(pld)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(pld_inst));

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(qadd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qaddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qdadd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qdsub)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qsub)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qsub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qsub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(qsubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(rev)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(rev_inst));
	rev_inst *inst_cream = (rev_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rm   = BITS(inst,  0,  3);
	inst_cream->Rd   = BITS(inst, 12, 15);

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(revsh)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(rfe)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(rsb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(rsb_inst));
	rsb_inst *inst_cream = (rsb_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (CHECK_RN) 
		inst_base->load_r15 = 1;

	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(rsc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(rsc_inst));
	rsc_inst *inst_cream = (rsc_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (CHECK_RN)
		inst_base->load_r15 = 1;

	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(sadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(saddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sbc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(sbc_inst));
	sbc_inst *inst_cream = (sbc_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (CHECK_RN)
		inst_base->load_r15 = 1;

	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(sel)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(setend)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shaddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shsub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shsub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(shsubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smla)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlad)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlal)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(umlal_inst));
	umlal_inst *inst_cream = (umlal_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rm	 = BITS(inst, 0, 3);
	inst_cream->Rs	 = BITS(inst, 8, 11);
	inst_cream->RdHi = BITS(inst, 16, 19);
	inst_cream->RdLo = BITS(inst, 12, 15);

	if (CHECK_RM || CHECK_RS) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlalxy)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlald)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlaw)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlsd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smlsld)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smmla)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smmls)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smmul)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smuad)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smul)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smull)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(umull_inst));
	umull_inst *inst_cream = (umull_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rm	 = BITS(inst, 0, 3);
	inst_cream->Rs	 = BITS(inst, 8, 11);
	inst_cream->RdHi = BITS(inst, 16, 19);
	inst_cream->RdLo = BITS(inst, 12, 15);

	if (CHECK_RM || CHECK_RS) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(smulw)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(smusd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(srs)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ssat)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ssat16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ssub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ssub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(ssubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(stc)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(stc_inst));
	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(stm)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);
#if 0
	if (BIT(inst, 15)) {
		inst_base->br = INDIRECT_BRANCH;
	}
#endif
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxtb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(sxtb_inst));
	sxtb_inst *inst_cream = (sxtb_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rd     = BITS(inst, 12, 15);
	inst_cream->Rm     = BITS(inst,  0,  3);
	inst_cream->rotate = BITS(inst, 10, 11);

	if (CHECK_RM) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(str)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxtb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(uxth_inst));
	uxth_inst *inst_cream = (uxth_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->Rd     = BITS(inst, 12, 15);
	inst_cream->rotate = BITS(inst, 10, 11);
	inst_cream->Rm     = BITS(inst,  0,  3);

	if (CHECK_RM) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxtab)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(strb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(strbt)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
//	inst_cream->get_addr = get_calc_addr_op(inst);
	if (I_BIT == 0) {
		inst_cream->get_addr = LnSWoUB(ImmediatePostIndexed);
	} else {
		DEBUG_MSG;
	}

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(strd)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(strex)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(strexb)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(strh)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	inst_cream->get_addr = get_calc_addr_op(inst);

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(strt)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(ldst_inst));
	ldst_inst *inst_cream = (ldst_inst *)inst_base->component;

	inst_base->cond = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->inst = inst;
	if (I_BIT == 0) {
		inst_cream->get_addr = LnSWoUB(ImmediatePostIndexed);
	} else {
		DEBUG_MSG;
	}

	if (BITS(inst, 12, 15) == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(sub)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(sub_inst));
	sub_inst *inst_cream = (sub_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}
	if (CHECK_RN) 
		inst_base->load_r15 = 1;

	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(swi)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(swi_inst));
	swi_inst *inst_cream = (swi_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;

	inst_cream->num  = BITS(inst, 0, 23);
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(swp)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(swpb)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxtab)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxtab16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxtah)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(sxtb16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(teq)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(teq_inst));
	teq_inst *inst_cream = (teq_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);

	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(tst)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(tst_inst));
	tst_inst *inst_cream = (tst_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->I	 = BIT(inst, 25);
	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rn	 = BITS(inst, 16, 19);
	inst_cream->Rd	 = BITS(inst, 12, 15);
	inst_cream->shifter_operand = BITS(inst, 0, 11);
	inst_cream->shtop_func = get_shtop(inst);
	if (inst_cream->Rd == 15) {
		inst_base->br = INDIRECT_BRANCH;
	}

	if (CHECK_RN) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uaddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhaddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhsub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhsub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uhsubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(umaal)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(umlal)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(umlal_inst));
	umlal_inst *inst_cream = (umlal_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rm	 = BITS(inst, 0, 3);
	inst_cream->Rs	 = BITS(inst, 8, 11);
	inst_cream->RdHi = BITS(inst, 16, 19);
	inst_cream->RdLo = BITS(inst, 12, 15);

	if (CHECK_RM || CHECK_RS) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(umull)(unsigned int inst, int index)
{
	arm_inst *inst_base = (arm_inst *)AllocBuffer(sizeof(arm_inst) + sizeof(umull_inst));
	umull_inst *inst_cream = (umull_inst *)inst_base->component;

	inst_base->cond  = BITS(inst, 28, 31);
	inst_base->idx	 = index;
	inst_base->br	 = NON_BRANCH;
	inst_base->load_r15 = 0;

	inst_cream->S	 = BIT(inst, 20);
	inst_cream->Rm	 = BITS(inst, 0, 3);
	inst_cream->Rs	 = BITS(inst, 8, 11);
	inst_cream->RdHi = BITS(inst, 16, 19);
	inst_cream->RdLo = BITS(inst, 12, 15);

	if (CHECK_RM || CHECK_RS) 
		inst_base->load_r15 = 1;
	return inst_base;
}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqadd16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqadd8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqaddsubx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqsub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqsub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uqsubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usad8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usada8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usat)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usat16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usub16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usub8)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(usubaddx)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxtab16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}
ARM_INST_PTR INTERPRETER_TRANSLATE(uxtb16)(unsigned int inst, int index){printf("in func %s\n", __FUNCTION__);exit(-1);}


typedef ARM_INST_PTR (*transop_fp_t)(unsigned int, int);

const transop_fp_t arm_instruction_trans[] = {
	INTERPRETER_TRANSLATE(adc),
	INTERPRETER_TRANSLATE(add),
	INTERPRETER_TRANSLATE(and),
	INTERPRETER_TRANSLATE(bbl),
	INTERPRETER_TRANSLATE(bic),
	INTERPRETER_TRANSLATE(bkpt),
	INTERPRETER_TRANSLATE(blx),
	INTERPRETER_TRANSLATE(blx),
	INTERPRETER_TRANSLATE(bx),
	INTERPRETER_TRANSLATE(bxj),
	INTERPRETER_TRANSLATE(cdp),
	INTERPRETER_TRANSLATE(clrex),
	INTERPRETER_TRANSLATE(clz),
	INTERPRETER_TRANSLATE(cmn),
	INTERPRETER_TRANSLATE(cmp),
	INTERPRETER_TRANSLATE(cps),
	INTERPRETER_TRANSLATE(cpy),
	INTERPRETER_TRANSLATE(eor),
	INTERPRETER_TRANSLATE(ldc),
	INTERPRETER_TRANSLATE(ldm),
	INTERPRETER_TRANSLATE(ldm),
	INTERPRETER_TRANSLATE(ldm),
	INTERPRETER_TRANSLATE(sxth),
	INTERPRETER_TRANSLATE(uxth),
	INTERPRETER_TRANSLATE(uxtah),
	INTERPRETER_TRANSLATE(rev),
	INTERPRETER_TRANSLATE(revsh),
	INTERPRETER_TRANSLATE(ldrbt),
	INTERPRETER_TRANSLATE(ldrt),
	INTERPRETER_TRANSLATE(ldr),
	INTERPRETER_TRANSLATE(pld),
	INTERPRETER_TRANSLATE(ldrb),
	INTERPRETER_TRANSLATE(ldrd),
	INTERPRETER_TRANSLATE(ldrex),
	INTERPRETER_TRANSLATE(ldrexb),
	INTERPRETER_TRANSLATE(ldrh),
	INTERPRETER_TRANSLATE(ldrsb),
	INTERPRETER_TRANSLATE(ldrsh),
	INTERPRETER_TRANSLATE(mcr),
	INTERPRETER_TRANSLATE(mcrr),
	INTERPRETER_TRANSLATE(mla),
	INTERPRETER_TRANSLATE(mov),
	INTERPRETER_TRANSLATE(mrc),
	INTERPRETER_TRANSLATE(mrrc),
	INTERPRETER_TRANSLATE(mrs),
	INTERPRETER_TRANSLATE(msr),
	INTERPRETER_TRANSLATE(msr),
	INTERPRETER_TRANSLATE(mul),
	INTERPRETER_TRANSLATE(mvn),
	INTERPRETER_TRANSLATE(orr),
	INTERPRETER_TRANSLATE(pkhbt),
	INTERPRETER_TRANSLATE(pkhtb),
	INTERPRETER_TRANSLATE(qadd),
	INTERPRETER_TRANSLATE(qadd16),
	INTERPRETER_TRANSLATE(qadd8),
	INTERPRETER_TRANSLATE(qaddsubx),
	INTERPRETER_TRANSLATE(qdadd),
	INTERPRETER_TRANSLATE(qdsub),
	INTERPRETER_TRANSLATE(qsub),
	INTERPRETER_TRANSLATE(qsub16),
	INTERPRETER_TRANSLATE(qsub8),
	INTERPRETER_TRANSLATE(qsubaddx),
	INTERPRETER_TRANSLATE(rfe),
	INTERPRETER_TRANSLATE(rsb),
	INTERPRETER_TRANSLATE(rsc),
	INTERPRETER_TRANSLATE(sadd16),
	INTERPRETER_TRANSLATE(sadd8),
	INTERPRETER_TRANSLATE(saddsubx),
	INTERPRETER_TRANSLATE(sbc),
	INTERPRETER_TRANSLATE(sel),
	INTERPRETER_TRANSLATE(setend),
	INTERPRETER_TRANSLATE(shadd16),
	INTERPRETER_TRANSLATE(shadd8),
	INTERPRETER_TRANSLATE(shaddsubx),
	INTERPRETER_TRANSLATE(shsub16),
	INTERPRETER_TRANSLATE(shsub8),
	INTERPRETER_TRANSLATE(shsubaddx),
	INTERPRETER_TRANSLATE(smla),
	INTERPRETER_TRANSLATE(smlad),
	INTERPRETER_TRANSLATE(smlal),
	INTERPRETER_TRANSLATE(smlalxy),
	INTERPRETER_TRANSLATE(smlald),
	INTERPRETER_TRANSLATE(smlaw),
	INTERPRETER_TRANSLATE(smlsd),
	INTERPRETER_TRANSLATE(smlsld),
	INTERPRETER_TRANSLATE(smmla),
	INTERPRETER_TRANSLATE(smmls),
	INTERPRETER_TRANSLATE(smmul),
	INTERPRETER_TRANSLATE(smuad),
	INTERPRETER_TRANSLATE(smul),
	INTERPRETER_TRANSLATE(smull),
	INTERPRETER_TRANSLATE(smulw),
	INTERPRETER_TRANSLATE(smusd),
	INTERPRETER_TRANSLATE(srs),
	INTERPRETER_TRANSLATE(ssat),
	INTERPRETER_TRANSLATE(ssat16),
	INTERPRETER_TRANSLATE(ssub16),
	INTERPRETER_TRANSLATE(ssub8),
	INTERPRETER_TRANSLATE(ssubaddx),
	INTERPRETER_TRANSLATE(stc),
	INTERPRETER_TRANSLATE(stm),
	INTERPRETER_TRANSLATE(stm),
	INTERPRETER_TRANSLATE(sxtb),
	INTERPRETER_TRANSLATE(uxtb),
	INTERPRETER_TRANSLATE(uxtab),
	INTERPRETER_TRANSLATE(strbt),
	INTERPRETER_TRANSLATE(strt),
	INTERPRETER_TRANSLATE(str),
	INTERPRETER_TRANSLATE(strb),
	INTERPRETER_TRANSLATE(strd),
	INTERPRETER_TRANSLATE(strex),
	INTERPRETER_TRANSLATE(strexb),
	INTERPRETER_TRANSLATE(strh),
	INTERPRETER_TRANSLATE(sub),
	INTERPRETER_TRANSLATE(swi),
	INTERPRETER_TRANSLATE(swp),
	INTERPRETER_TRANSLATE(swpb),
	INTERPRETER_TRANSLATE(sxtab),
	INTERPRETER_TRANSLATE(sxtab16),
	INTERPRETER_TRANSLATE(sxtah),
	INTERPRETER_TRANSLATE(sxtb16),
	INTERPRETER_TRANSLATE(teq),
	INTERPRETER_TRANSLATE(tst),
	INTERPRETER_TRANSLATE(uadd16),
	INTERPRETER_TRANSLATE(uadd8),
	INTERPRETER_TRANSLATE(uaddsubx),
	INTERPRETER_TRANSLATE(uhadd16),
	INTERPRETER_TRANSLATE(uhadd8),
	INTERPRETER_TRANSLATE(uhaddsubx),
	INTERPRETER_TRANSLATE(uhsub16),
	INTERPRETER_TRANSLATE(uhsub8),
	INTERPRETER_TRANSLATE(uhsubaddx),
	INTERPRETER_TRANSLATE(umaal),
	INTERPRETER_TRANSLATE(umlal),
	INTERPRETER_TRANSLATE(umull),
	INTERPRETER_TRANSLATE(uqadd16),
	INTERPRETER_TRANSLATE(uqadd8),
	INTERPRETER_TRANSLATE(uqaddsubx),
	INTERPRETER_TRANSLATE(uqsub16),
	INTERPRETER_TRANSLATE(uqsub8),
	INTERPRETER_TRANSLATE(uqsubaddx),
	INTERPRETER_TRANSLATE(usad8),
	INTERPRETER_TRANSLATE(usada8),
	INTERPRETER_TRANSLATE(usat),
	INTERPRETER_TRANSLATE(usat16),
	INTERPRETER_TRANSLATE(usub16),
	INTERPRETER_TRANSLATE(usub8),
	INTERPRETER_TRANSLATE(usubaddx),
	INTERPRETER_TRANSLATE(uxtab16),
	INTERPRETER_TRANSLATE(uxtb16)
};

typedef map<unsigned int, int> bb_map;
bb_map CreamCache[65536];

//#define USE_DUMMY_CACHE

#ifdef USE_DUMMY_CACHE
unsigned int DummyCache[0x100000];
#endif

#define HASH(x) ((x + (x << 2) + (x >> 6)) % 65536)
void insert_bb(unsigned int addr, int start)
{
#ifdef USE_DUMMY_CACHE
	DummyCache[addr] = start;
#else
//	CreamCache[addr] = start;
	CreamCache[HASH(addr)][addr] = start;
#endif
}

int find_bb(unsigned int addr, int &start)
{
	int ret = -1;
#ifdef USE_DUMMY_CACHE
	start = DummyCache[addr];
	if (start) {
		ret = 0;
	} else
		ret = -1;
#else
	bb_map::const_iterator it = CreamCache[HASH(addr)].find(addr);
	if (it != CreamCache[HASH(addr)].end()) {
		start = static_cast<int>(it->second);
		ret = 0;
	} else {
		ret = -1;
	}
#endif
	return ret;
}


enum {
	FETCH_SUCCESS,
	FETCH_FAILURE
};

int FetchInst(cpu_t *core, unsigned int &inst)
{
	arm_processor *cpu = (arm_processor *)get_cast_conf_obj(core->cpu_data, "arm_core_t");
//	fault_t fault = interpreter_read_memory(core, cpu->translate_pc, inst, 32);
	fault_t fault = interpreter_fetch(core, cpu->translate_pc, inst, 32);
	if (!core->is_user_mode) {
		if (fault) {
			cpu->abortSig = true;
			cpu->Aborted = ARMul_PrefetchAbortV;
			cpu->AbortAddr = cpu->translate_pc;
			cpu->CP15[CP15(CP15_INSTR_FAULT_STATUS)] = fault & 0xff;
			cpu->CP15[CP15(CP15_FAULT_ADDRESS)] = cpu->translate_pc;
			return FETCH_FAILURE;
		}
	}
	return FETCH_SUCCESS;
}

unsigned int *InstLength;

enum {
	KEEP_GOING,
	FETCH_EXCEPTION
};
struct instruction_set_encoding_item {
        const char *name;
        int attribute_value;
        int version;
        int content[12];//12 is the max number
};

typedef struct instruction_set_encoding_item ISEITEM;

extern const ISEITEM arm_instruction[];
int InterpreterTranslate(cpu_t *core, int &bb_start)
{
	/* Decode instruction, get index */
	/* Allocate memory and init InsCream */
	/* Go on next, until terminal instruction */
	/* Save start addr of basicblock in CreamCache */
	arm_processor *cpu = (arm_processor *)get_cast_conf_obj(core->cpu_data, "arm_core_t");
	ARM_INST_PTR inst_base = NULL;
	unsigned int inst;
	int idx;
	int ret = NON_BRANCH;
	/* (R15 - 8) ? */
	unsigned int pc_start = cpu->Reg[15];
	cpu->translate_pc = cpu->Reg[15];
	bb_start = top;
	while(ret == NON_BRANCH) {
		ret = FetchInst(core, inst);
		if (ret == FETCH_FAILURE) {
			return FETCH_EXCEPTION;
		}
		ret = decode_arm_instr(inst, &idx);
		if (ret == DECODE_FAILURE) {
			printf("[info] : Decode failure.\tPC : [0x%x]\tInstruction : [%x]\n", cpu->translate_pc, inst);
			exit(-1);
		}
//		printf("PC : [0x%x] INST : %s\n", cpu->translate_pc, arm_instruction[idx].name);
		inst_base = arm_instruction_trans[idx](inst, idx);
//		printf("translated @ %x INST : %x\n", cpu->translate_pc, inst);
//		printf("inst size is %d\n", InstLength[idx]);
		cpu->translate_pc += 4;

		if ((cpu->translate_pc & 0xfff) == 0) {
			inst_base->br = END_OF_PAGE;
		}
		ret = inst_base->br;
	};
	insert_bb(pc_start, bb_start);
	return KEEP_GOING;
}

uint32_t follow_mode(cpu_t *cpu);
uint32_t is_int_in_interpret(cpu_t *cpu);

/* save all original registers */
uint32_t mirror_reg_list[100];

typedef struct _register_change {
	int index;
	uint32_t value;
} reg_chg;

#define REG_SIZE	44
typedef struct _step_status {
	int 		top;
	uint64_t 	mask;
	reg_chg 	reg[REG_SIZE];
} stp_sts;
stp_sts run_pass, log_pass;

extern const char* arm_regstr[MAX_REG_NUM];
void init_mirror_reg_list(arm_processor *cpu)
{
	for (int i = 0; i < 16; i ++) {
		mirror_reg_list[i] = cpu->Reg[i];
	}
	mirror_reg_list[CPSR_REG] = cpu->Cpsr;
	mirror_reg_list[R13_SVC]  = cpu->Reg_svc[0];
	mirror_reg_list[R14_SVC]  = cpu->Reg_svc[1];

	mirror_reg_list[R13_ABORT] = cpu->Reg_abort[0];
	mirror_reg_list[R14_ABORT] = cpu->Reg_abort[1];

	mirror_reg_list[R13_UNDEF] = cpu->Reg_undef[0];
	mirror_reg_list[R14_UNDEF] = cpu->Reg_undef[1];

	mirror_reg_list[R13_IRQ]   = cpu->Reg_irq[0];
	mirror_reg_list[R14_IRQ]   = cpu->Reg_irq[1];

	mirror_reg_list[R8_FIRQ]    = cpu->Reg_firq[0];
	mirror_reg_list[R9_FIRQ]    = cpu->Reg_firq[1];
	mirror_reg_list[R10_FIRQ]   = cpu->Reg_firq[2];
	mirror_reg_list[R11_FIRQ]   = cpu->Reg_firq[3];
	mirror_reg_list[R12_FIRQ]   = cpu->Reg_firq[4];
	mirror_reg_list[R13_FIRQ]   = cpu->Reg_firq[5];
	mirror_reg_list[R14_FIRQ]   = cpu->Reg_firq[6];
}

void init_step_status()
{
	run_pass.top = 0;
	run_pass.mask = 0;
	for (int i = 0; i < REG_SIZE; i ++) {
		run_pass.reg[i].index = -1;
	}
	log_pass.top = 0;
	log_pass.mask = 0;
	for (int i = 0; i < REG_SIZE; i ++) {
		log_pass.reg[i].index = -1;
	}
}

void reset_pass(stp_sts &pass)
{
	int i = 0;
	#if 0
	while((pass.reg[i].index != -1) && (i < REG_SIZE)) {
		pass.reg[i].index = -1;
		i ++;
	}
	pass.top = 0;
	#endif
	pass.mask = 0;
}

inline int get_reg_chg_item(stp_sts &pass, reg_chg &item)
{
	if (pass.reg[pass.top].index != -1) {
		item = pass.reg[pass.top];
		pass.top ++;
		return 1;
	} else return 0;
}

int print = 0;
void show_same_reg(arm_processor *cpu, uint64_t mask)
{
	int i = 0;
	static int times = 0;
	while (i < REG_SIZE) {
		if (mask & (1 << i)) {
			if (i != 16) {
				if (log_pass.reg[i].value != run_pass.reg[i].value) {
					if (i < 16 && !print) {
						times ++;
					}
					if (!print) {
						printf("--------------------------------------------\n");
						printf("[   PC   ]:[0x%08x]\n", cpu->Reg[15]);
						printf("[ICOUNTER]:[%lld]\n", cpu->icounter);
						print = 1;
					}
					if (cpu->Reg[15] == 0xc0020ab0) {
						cpu->Reg[i] = log_pass.reg[i].value;
						printf("Modification:[%s]:%x\n", arm_regstr[i], cpu->Reg[i]);
					}
					#if 1
					if (cpu->Reg[15] == 0xc019ca44 || cpu->Reg[15] == 0xc019ca58 || cpu->Reg[15] == 0xc019ca64 
					    || cpu->Reg[15] == 0xc019ca84 || cpu->Reg[15] == 0xc019cb44 || cpu->Reg[15] == 0xc019cb38 
					    || cpu->Reg[15] == 0xc019e8f4) {
						cpu->Reg[i] = log_pass.reg[i].value;
					}
					if (cpu->Reg[15] == 0xc01218a0) {
						cpu->Reg[i] = log_pass.reg[i].value;
					}
					#endif
					if (cpu->icounter == 20363310 || cpu->icounter == 20363317 || cpu->icounter == 20384995 || cpu->icounter == 20406676) {
						cpu->Reg[i] = log_pass.reg[i].value;
					}
					if (cpu->Reg[15] == 0x00008a80 && cpu->icounter == 248320096) {
						cpu->Reg[i] = log_pass.reg[i].value;
					}
					printf("[log mode][%s]:[0x%08x]\n", arm_regstr[i], log_pass.reg[i].value);
					printf("[run mode][%s]:[0x%08x]\n", arm_regstr[i], run_pass.reg[i].value);
				}
			}
		}
		i ++;
	}
	if (times == 250) {
		exit(-1);
	}
}

void show_run_reg(arm_processor *cpu, uint64_t mask)
{
	int i = 0;
	while (i < REG_SIZE) {
		if (mask & (1 << i)) {
			if (i != 16) {
				if (!print) {
					printf("--------------------------------------------\n");
					printf("[   PC   ]:[0x%08x]\n", cpu->Reg[15]);
					printf("[ICOUNTER]:[%lld]\n", cpu->icounter);
					print = 1;
				}
				printf("[run mode][%s]:[0x%08x]\n", arm_regstr[i], run_pass.reg[i].value);
				if (cpu->Reg[15] == 0xffff0010 && cpu->icounter == 248231713 && i < 16) {
					cpu->Reg[i] = 0x000bc4a0;
				}
			}
		}
		i ++;
	}
}

void show_log_reg(arm_processor *cpu, uint64_t mask)
{
	int i = 0;
	while (i < REG_SIZE) {
		if (mask & (1 << i)) {
			if (i != 16) {
				if (!print) {
					printf("--------------------------------------------\n");
					printf("[   PC   ]:[0x%08x]\n", cpu->Reg[15]);
					printf("[ICOUNTER]:[%lld]\n", cpu->icounter);
					print = 1;
				}
				printf("[log mode][%s]:[0x%08x]\n", arm_regstr[i], log_pass.reg[i].value);
				if ((cpu->Reg[15] == 0xc0020ab0) && (i < 15)) {
					cpu->Reg[i] = log_pass.reg[i].value;
					printf("Modification:[%s]:%x\n", arm_regstr[i], cpu->Reg[i]);
				}
			}
		}
		i ++;
	}
}

void compare_pass(arm_processor *cpu, stp_sts &rp, stp_sts &lp)
{
	reg_chg run_item, log_item;
	int ret1 = 1, ret2 = 1;
	int print_heading = 0;
	uint64_t result;
	#if 0
	if (cpu->Reg[15] == 0xc0067914 && cpu->icounter == 1879690) {
		printf("lp.top:%d\n", lp.top);
		printf("rp.top:%d\n", rp.top);
	}
	#endif
	do {
		result = rp.mask & lp.mask;
		if (cpu->icounter >= 1877686) {
//			printf("result:%x rp.mask:%x lp.mask:%x\n", result, rp.mask, lp.mask);
		}
		if (result) {
			show_same_reg(cpu, result);
		}
		rp.mask = rp.mask & (~result);
		if (rp.mask) {
			show_run_reg(cpu, rp.mask);
		}
		lp.mask = lp.mask & (~result);
		if (lp.mask) {
			show_log_reg(cpu, lp.mask);
		}
	} while (0);
	print = 0;
}

void diff_self(cpu_t *cpu)
{
	arm_core_t* state = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	int reg_index;
	int array_index = 0;
	int last_no = 0;
	for (reg_index = 0; reg_index < 15; reg_index ++) {
		if (state->Reg[reg_index] != mirror_reg_list[reg_index]) {
			#if 0
			if (state->Reg[15] == 0xc0067914 && state->icounter == 1879690) {
				printf("[NEW][R5]:0x%x\n", state->Reg[5]);
				printf("[OLD][R5]:0x%x\n", mirror_reg_list[5]);
				exit(-1);
			}
			#endif
			//printf("R%d:0x%x\n", reg_index, state->Reg[reg_index]);
			run_pass.reg[reg_index].index = reg_index;
			run_pass.reg[reg_index].value = state->Reg[reg_index];
			mirror_reg_list[reg_index] = state->Reg[reg_index];
			run_pass.mask |= (1 << reg_index);
		}
	}
	if (state->Cpsr != mirror_reg_list[CPSR_REG]) {
		reg_index = CPSR_REG;
		run_pass.reg[reg_index].index = CPSR_REG;
		run_pass.reg[reg_index].value = state->Cpsr;
		mirror_reg_list[CPSR_REG] = state->Cpsr;
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_svc[0] != mirror_reg_list[R13_SVC]) {
		reg_index = R13_SVC;
		run_pass.reg[reg_index].index = R13_SVC;
		run_pass.reg[reg_index].value = state->Reg_svc[0];
		mirror_reg_list[R13_SVC] = state->Reg_svc[0];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_svc[1] != mirror_reg_list[R14_SVC]) {
		reg_index = R14_SVC;
		run_pass.reg[reg_index].index = R14_SVC;
		run_pass.reg[reg_index].value = state->Reg_svc[1];
		mirror_reg_list[R14_SVC] = state->Reg_svc[1];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_abort[0] != mirror_reg_list[R13_ABORT]) {
		reg_index = R13_ABORT;
		run_pass.reg[reg_index].index = R13_ABORT;
		run_pass.reg[reg_index].value = state->Reg_abort[0];
		mirror_reg_list[R13_ABORT] = state->Reg_abort[0];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_abort[1] != mirror_reg_list[R14_ABORT]) {
		reg_index = R14_ABORT;
		run_pass.reg[reg_index].index = R14_ABORT;
		run_pass.reg[reg_index].value = state->Reg_abort[1];
		mirror_reg_list[R14_ABORT] = state->Reg_abort[1];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_undef[0] != mirror_reg_list[R13_UNDEF]) {
		reg_index = R13_UNDEF;
		run_pass.reg[reg_index].index = R13_UNDEF;
		run_pass.reg[reg_index].value = state->Reg_undef[0];
		mirror_reg_list[R13_UNDEF] = state->Reg_undef[0];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_undef[1] != mirror_reg_list[R14_UNDEF]) {
		reg_index = R14_UNDEF;
		run_pass.reg[reg_index].index = R14_UNDEF;
		run_pass.reg[reg_index].value = state->Reg_undef[1];
		mirror_reg_list[R14_UNDEF] = state->Reg_undef[1];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_irq[0] != mirror_reg_list[R13_IRQ]) {
		reg_index = R13_IRQ;
		run_pass.reg[reg_index].index = R13_IRQ;
		run_pass.reg[reg_index].value = state->Reg_irq[0];
		mirror_reg_list[R13_IRQ] = state->Reg_irq[0];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_irq[1] != mirror_reg_list[R14_IRQ]) {
		reg_index = R14_IRQ;
		run_pass.reg[reg_index].index = R14_IRQ;
		run_pass.reg[reg_index].value = state->Reg_irq[1];
		mirror_reg_list[R14_IRQ] = state->Reg_irq[1];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[0] != mirror_reg_list[R8_FIRQ]) {
		reg_index = R8_FIRQ;
		run_pass.reg[reg_index].index = R8_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[0];
		mirror_reg_list[R8_FIRQ] = state->Reg_firq[0];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[1] != mirror_reg_list[R9_FIRQ]) {
		reg_index = R9_FIRQ;
		run_pass.reg[reg_index].index = R9_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[1];
		mirror_reg_list[R9_FIRQ] = state->Reg_firq[1];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[2] != mirror_reg_list[R10_FIRQ]) {
		reg_index = R10_FIRQ;
		run_pass.reg[reg_index].index = R10_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[2];
		mirror_reg_list[R10_FIRQ] = state->Reg_firq[2];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[3] != mirror_reg_list[R11_FIRQ]) {
		reg_index = R11_FIRQ;
		run_pass.reg[reg_index].index = R11_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[3];
		mirror_reg_list[R11_FIRQ] = state->Reg_firq[3];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[4] != mirror_reg_list[R12_FIRQ]) {
		reg_index = R12_FIRQ;
		run_pass.reg[reg_index].index = R12_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[4];
		mirror_reg_list[R12_FIRQ] = state->Reg_firq[4];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[5] != mirror_reg_list[R13_FIRQ]) {
		reg_index = R13_FIRQ;
		run_pass.reg[reg_index].index = R13_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[5];
		mirror_reg_list[R13_FIRQ] = state->Reg_firq[5];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Reg_firq[6] != mirror_reg_list[R14_FIRQ]) {
		reg_index = R14_FIRQ;
		run_pass.reg[reg_index].index = R14_FIRQ;
		run_pass.reg[reg_index].value = state->Reg_firq[6];
		mirror_reg_list[R14_FIRQ] = state->Reg_firq[6];
		run_pass.mask |= (1 << reg_index);
	}
	if (state->Spsr[SVCBANK] != mirror_reg_list[SPSR_SVC]) {
		//fprintf(state->state_log, "SPSR_SVC:0x%x\n", state->Spsr[SVCBANK]);
		//mirror_reg_list[SPSR_SVC] = state->RegBank[SVCBANK];
	}
	if (state->Spsr[ABORTBANK] != mirror_reg_list[SPSR_ABORT]) {
		//fprintf(state->state_log, "SPSR_ABORT:0x%x\n", state->Spsr[ABORTBANK]);
		//mirror_reg_list[SPSR_ABORT] = state->RegBank[ABORTBANK];
	}
	if (state->Spsr[UNDEFBANK] != mirror_reg_list[SPSR_UNDEF]) {
		//fprintf(state->state_log, "SPSR_UNDEF:0x%x\n", state->Spsr[UNDEFBANK]);
		//mirror_reg_list[SPSR_UNDEF] = state->RegBank[UNDEFBANK];
	}
	if (state->Spsr[IRQBANK] != mirror_reg_list[SPSR_IRQ]) {
		//fprintf(state->state_log, "SPSR_IRQ:0x%x\n", state->Spsr[IRQBANK]);
		//mirror_reg_list[SPSR_IRQ] = state->RegBank[IRQBANK];
	}
	if (state->Spsr[FIQBANK] != mirror_reg_list[SPSR_FIRQ]) {
		//fprintf(state->state_log, "SPSR_FIRQ:0x%x\n", state->Spsr[FIQBANK]);
		//mirror_reg_list[SPSR_FIRQ] = state->RegBank[FIQBANK];
	}
	#if 0
	printf("array_index is %d\n", array_index);
	for (int i = 0; i < REG_SIZE; i ++) {
		if (run_pass.reg[i].index != -1) {
			printf("[index:%d][value:%x]\n", run_pass.reg[i].index, run_pass.reg[i].value);
		}
	}
	#endif
}

static int arm_get_id_from_string(char *reg_name, bool print_regname)
{
	if (print_regname) {
	//	printf("in %s\n", __FUNCTION__);
		printf("reg_name is %s\n", reg_name);
	}
        int i = 0;
        for (i = 0; i < 64; i ++) {
                if (0 == strcmp(arm_regstr[i], reg_name))
                        return i;
        }
	return -1;
}
#define LOG_IN_CLR	skyeye_printf_in_color

void update_int_array(cpu_t *cpu, uint32_t icounter);
uint32_t am_diff(cpu_t *cpu)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	uint32_t pc;
	static uint32_t last_pc = 0;
	char string[30];
	char *first_colon = NULL;
	char *pValue = NULL;
	uint32_t value;
	int idx;
	int array_index = 0;

	/* load PC */
	if (last_pc == 0) 
		fscanf(cpu->src_log, "PC:0x%x\n", &pc);
	else
		pc = last_pc;

	/* pc flow diverged */
	if (pc != core->Reg[15]) {
		if (core->Reg[15] == 0xffff000c) {
			printf("skip the first page fault in user mode...\n");
			while (1) {
				fscanf(cpu->src_log, "%s", string);
//				printf("%s", string);
				if (string[0] == 'P' && string[1] == 'C') { 
//					exit(-1);
					pValue = string + 3;
					pc = strtoul(pValue, NULL, 16);
//					printf("PC:[0x%08x].\n", pc);
					if (pc == core->Reg[15]) {
						break;
					}
				}
			}
		} else {
			#if 0
			if (pc == 0xffff0018) {
				LOG_IN_CLR(RED, "SYNC WITH IRQ...\n");
				cpu->check_int_flag = 1;
				update_int_array(cpu, core->icounter);
	//			core->Reg[15] -= 4;
				last_pc = pc;
	//			core->Reg[15] = 0xffff0018;
				return 1;
			} else {
			#endif
				LOG_IN_CLR(RED, "pc is wrong\n");
		//                printf("dynom mode pc is %x\n", core->Reg[15]);
				LOG_IN_CLR(BLUE, "dyncom mode pc is %x\n", core->Reg[15]);
				LOG_IN_CLR(CYAN, "dyncom mode phys_pc is %x\n", core->phys_pc);
				LOG_IN_CLR(LIGHT_RED, "interpreter mode is %x\n", pc);
				LOG_IN_CLR(PURPLE, "icounter is %lld\n", core->icounter);
		//		LOG_IN_CLR(RED, "adjust pc...\n");
				exit(-1);
		}
//		}
	}
	/* get registers change from last instruction */
	diff_self(cpu);
	/* get registers change from log file on disk */
	while(1) {
		#if 0
		array_index = 0;
		while(array_index < 30) {
			string[array_index] = 0;
			array_index ++;
		}
		#endif
		/* get register index */
		fscanf(cpu->src_log, "%s", string);
		first_colon = strchr(string, ':');
		pValue = first_colon + 1;
		if (string[0] == 'P' && string[1] == 'C') {
			last_pc = strtoul(pValue, NULL, 16);
			/* interrupt trigger */
			if (last_pc == 0xffff0018) {
				LOG_IN_CLR(RED, "SYNC WITH IRQ...\n");
				cpu->check_int_flag = 1;
				update_int_array(cpu, core->icounter);
//				core->Reg[15] -= 4;
//				last_pc = pc;
//				core->Reg[15] = 0xffff0018;
				return 1;
			}
			break;
		}
		*first_colon = '\0';
//		printf("string:%s", string);
		idx = arm_get_id_from_string(string, false);
		*first_colon = ':';
		if (idx < 0 || idx > 30) {
//			printf("string:%s\n", string);
//			printf("idx is %d\n", idx);
//			exit(-1);
			continue;
		}
		log_pass.mask |= (1 << idx);
		/* get register value */
		value = strtoul(pValue, NULL, 16);
		/* insert log_pass */
		#if 0
		if (core->icounter >= 1877747) {
			printf("%s\n", string);
			printf("idx:%d\n", idx);
			printf("value:%x\n", value);
		}
		#endif
		log_pass.reg[idx].index = idx;
		log_pass.reg[idx].value = value;
//		printf("string:%s\nidx:%d\n", string, idx);
		array_index ++;
	}
	/* compare registers */
	compare_pass(core, run_pass, log_pass);
	/* clear pass */
	reset_pass(run_pass);
	reset_pass(log_pass);

	return 0;
}

static int
debug_function(cpu_t *cpu)
{
	#if 0
	unsigned int value;
	unsigned int dummy;
	bool is_wrong = false;
	fscanf(cpu->state_log, "PC:0x%x\n", &value);
	if (value != cpu->Reg[15]) {
		printf("PC is worng [%x]:[%x]\n", cpu->Reg[15], value);
		exit(-1);
	}

	bool print_sw = false;
	if (cpu->icounter > 42468) {
		print_sw = true;
	}

	if (cpu->icounter == 42475) {
		exit(-1);
	}
	if (print_sw) {
		for (int i = 0; i < 16; i++) {
			printf("R%2d : %8x\t", i, cpu->Reg[i]);
			if (i % 4 == 3) {
				printf("\n");
			}
		}
	}

	for (int i = 0; i < 16; i++) {
		fscanf(cpu->state_log, "R%d:0x%x\n", &dummy, &value);
		if (cpu->Reg[i] != value) {
			printf("[CORRECT] R%d wrong [%x]: right [%x]\n", i, cpu->Reg[i], value);
			is_wrong = true;
			cpu->Reg[i] = value;
		}
	}


	if (cpu->icounter % 1000000 == 0) {
		printf("icounter is %lld\n", cpu->icounter);
	}
	if (is_wrong) {
		printf("icounter is %lld\n", cpu->icounter);
		printf("PC is %x\n", cpu->Reg[15]);
//		exit(-1);
	}
	#endif
#if 0
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	if (core->icounter >= 249111596 && core->icounter <= 249111596) {
//	if ((core->icounter >= 1877660)) {
	if ((core->icounter >= 249111590)) {
		printf("----------------------------------------------------------\n");
		printf("[ICOUNTER]:%lld\n", core->icounter);
		for (int i = 0; i < 16; i ++) {
			printf("[R%02d]:[0x%08x]\t", i, core->Reg[i]);
			if (i % 4 == 3) {
				printf("\n");
			}
		}
		printf("N : %d Z : %d C : %d V : %d\n", core->NFlag, core->ZFlag, core->CFlag, core->VFlag);
	}
#if 0
	if (core->icounter == 50) {
		exit(-1);
	}
#endif
#endif
	#if 0
	if (is_int_in_interpret(cpu)) {
		cpu->check_int_flag = 1;
		return 1;
	}
	#endif
//	return follow_mode(cpu);
	return am_diff(cpu);
}

int cmp(const void *x, const void *y)
{
	return *(unsigned long long int*)x - *(unsigned long long int *)y;
}

void InterpreterInitInstLength(unsigned long long int *ptr, size_t size)
{
	int array_size = size / sizeof(void *);
	unsigned long long int *InstLabel = new unsigned long long int[array_size];
	memcpy(InstLabel, ptr, size);
	qsort(InstLabel, array_size, sizeof(void *), cmp);
	InstLength = new unsigned int[array_size - 4];
	for (int i = 0; i < array_size - 4; i ++) {
		for (int j = 0; j < array_size; j ++) {
			if (ptr[i] == InstLabel[j]) {
				InstLength[i] = InstLabel[j + 1] - InstLabel[j];
				break;
			}
		}
	}
	for (int i = 0; i < array_size - 4; i ++)
		printf("[%d]:%d\n", i, InstLength[i]);
}

int clz(unsigned int x)
{
	int n;
	if (x == 0) return (32);
	n = 1;
	if ((x >> 16) == 0) { n = n + 16; x = x << 16;}
	if ((x >> 24) == 0) { n = n +  8; x = x <<  8;}
	if ((x >> 28) == 0) { n = n +  4; x = x <<  4;}
	if ((x >> 30) == 0) { n = n +  2; x = x <<  2;}
	n = n - (x >> 31);
	return n;
}

extern "C" unsigned arm_dyncom_SWI (ARMul_State * state, ARMword number);

static bool InAPrivilegedMode(arm_core_t *core)
{
	return (core->Mode != USER32MODE);
}
						#if 0                                                              
						if (cpu->icounter > 12170500) {                                    
							printf("%s\n", arm_instruction[inst_base->idx].name);      
						}                                                                  
						#endif                                                             

/* r15 = r15 + 8 */
void InterpreterMainLoop(cpu_t *core)
{
	#define CRn				inst_cream->crn
	#define OPCODE_2			inst_cream->opcode_2
	#define CRm				inst_cream->crm
	#define CP15_REG(n)			cpu->CP15[CP15(n)]
	#define RD				cpu->Reg[inst_cream->Rd]
	#define RN				cpu->Reg[inst_cream->Rn]
	#define RM				cpu->Reg[inst_cream->Rm]
	#define RS				cpu->Reg[inst_cream->Rs]
	#define RDHI				cpu->Reg[inst_cream->RdHi]
	#define RDLO				cpu->Reg[inst_cream->RdLo]
	#define LINK_RTN_ADDR			(cpu->Reg[14] = cpu->Reg[15] + 4)
	#define SET_PC				(cpu->Reg[15] = cpu->Reg[15] + 8 + inst_cream->signed_immed_24)
	#define SHIFTER_OPERAND			inst_cream->shtop_func(cpu, inst_cream->shifter_operand)

	#define INC_ICOUNTER			//cpu->icounter++;                                                   \
						if (debug_function(core))                                          \
							if (core->check_int_flag)                                  \
								goto END
						//printf("icounter is %llx line is %d pc is %x\n", cpu->icounter, __LINE__, cpu->Reg[15])
	#define FETCH_INST			if (inst_base->br != NON_BRANCH)                                   \
							goto DISPATCH;                                             \
						inst_base = (arm_inst *)&inst_buf[ptr]                             
	#define INC_PC(l)			ptr += sizeof(arm_inst) + l
	#define GOTO_NEXT_INST			goto *InstLabel[inst_base->idx]

	#define UPDATE_NFLAG(dst)		(cpu->NFlag = BIT(dst, 31) ? 1 : 0)
	#define UPDATE_ZFLAG(dst)		(cpu->ZFlag = dst ? 0 : 1)
//	#define UPDATE_CFLAG(dst, lop, rop)	(cpu->CFlag = ((ISNEG(lop) && ISPOS(rop)) ||                        \
								(ISNEG(lop) && ISPOS(dst)) ||                       \
								(ISPOS(rop) && ISPOS(dst))))
	#define UPDATE_CFLAG(dst, lop, rop)	(cpu->CFlag = (dst < lop))
	#define UPDATE_CFLAG_NOT_BORROW_FROM(lop, rop)	(cpu->CFlag = (lop >= rop))
	#define UPDATE_CFLAG_WITH_NOT(dst, lop, rop)	(cpu->CFlag = !(dst < lop))
	#define UPDATE_CFLAG_WITH_SC		cpu->CFlag = cpu->shifter_carry_out
//	#define UPDATE_CFLAG_WITH_NOT(dst, lop, rop)	cpu->CFlag = !((ISNEG(lop) && ISPOS(rop)) ||                        \
								(ISNEG(lop) && ISPOS(dst)) ||                       \
								(ISPOS(rop) && ISPOS(dst)))
	#define UPDATE_VFLAG(dst, lop, rop)	(cpu->VFlag = (((lop < 0) && (rop < 0) && (dst >= 0)) ||            \
								((lop >= 0) && (rop) >= 0 && (dst < 0))))
	#define UPDATE_VFLAG_WITH_NOT(dst, lop, rop)	(cpu->VFlag = !(((lop < 0) && (rop < 0) && (dst >= 0)) ||            \
								((lop >= 0) && (rop) >= 0 && (dst < 0))))
	#define UPDATE_VFLAG_OVERFLOW_FROM(dst, lop, rop)	(cpu->VFlag = (((lop ^ rop) & (lop ^ dst)) >> 31))

	#define SAVE_NZCV			cpu->Cpsr = (cpu->Cpsr & 0x0fffffff) | \
						(cpu->NFlag << 31)   |                 \
						(cpu->ZFlag << 30)   |                 \
						(cpu->CFlag << 29)   |                 \
						(cpu->VFlag << 28)
	#define LOAD_NZCV			cpu->NFlag = (cpu->Cpsr >> 31);   \
						cpu->ZFlag = (cpu->Cpsr >> 30) & 1;   \
						cpu->CFlag = (cpu->Cpsr >> 29) & 1;   \
						cpu->VFlag = (cpu->Cpsr >> 28) & 1
	#define CurrentModeHasSPSR		(cpu->Mode != SYSTEM32MODE) && (cpu->Mode != USER32MODE)

	arm_processor *cpu = (arm_processor *)get_cast_conf_obj(core->cpu_data, "arm_core_t");

	void *InstLabel[] = {
		&&ADC_INST,&&ADD_INST,&&AND_INST,&&BBL_INST,&&BIC_INST,&&BKPT_INST,&&BLX_INST,&&BLX_INST,&&BX_INST,
		&&BXJ_INST,&&CDP_INST,&&CLREX_INST,&&CLZ_INST,&&CMN_INST,&&CMP_INST,&&CPS_INST,&&CPY_INST,&&EOR_INST,
		&&LDC_INST,&&LDM_INST,&&LDM_INST,&&LDM_INST,&&SXTH_INST,&&UXTH_INST,&&UXTAH_INST,&&REV_INST,&&REVSH_INST,
		&&LDRBT_INST,&&LDRT_INST,&&LDR_INST,&&PLD_INST,&&LDRB_INST,&&LDRD_INST,&&LDREX_INST,&&LDREXB_INST,&&LDRH_INST,
		&&LDRSB_INST,&&LDRSH_INST,&&MCR_INST,&&MCRR_INST,&&MLA_INST,&&MOV_INST,&&MRC_INST,&&MRRC_INST,&&MRS_INST,
		&&MSR_INST,&&MSR_INST,&&MUL_INST,&&MVN_INST,&&ORR_INST,&&PKHBT_INST,&&PKHTB_INST,&&QADD_INST,&&QADD16_INST,
		&&QADD8_INST,&&QADDSUBX_INST,&&QDADD_INST,&&QDSUB_INST,&&QSUB_INST,&&QSUB16_INST,&&QSUB8_INST,&&QSUBADDX_INST,
		&&RFE_INST,&&RSB_INST,&&RSC_INST,&&SADD16_INST,&&SADD8_INST,&&SADDSUBX_INST,&&SBC_INST,&&SEL_INST,&&SETEND_INST,
		&&SHADD16_INST,&&SHADD8_INST,&&SHADDSUBX_INST,&&SHSUB16_INST,&&SHSUB8_INST,&&SHSUBADDX_INST,&&SMLA_INST,&&SMLAD_INST,
		&&SMLAL_INST,&&SMLALXY_INST,&&SMLALD_INST,&&SMLAW_INST,&&SMLSD_INST,&&SMLSLD_INST,&&SMMLA_INST,&&SMMLS_INST,
		&&SMMUL_INST,&&SMUAD_INST,&&SMUL_INST,&&SMULL_INST,&&SMULW_INST,&&SMUSD_INST,&&SRS_INST,&&SSAT_INST,&&SSAT16_INST,
		&&SSUB16_INST,&&SSUB8_INST,&&SSUBADDX_INST,&&STC_INST,&&STM_INST,&&STM_INST,&&SXTB_INST,&&UXTB_INST,&&UXTAB_INST,
		&&STRBT_INST,&&STRT_INST,&&STR_INST,&&STRB_INST,&&STRD_INST,&&STREX_INST,&&STREXB_INST,&&STRH_INST,&&SUB_INST,
		&&SWI_INST,&&SWP_INST,&&SWPB_INST,&&SXTAB_INST,&&SXTAB16_INST,&&SXTAH_INST,&&SXTB16_INST,&&TEQ_INST,&&TST_INST,
		&&UADD16_INST,&&UADD8_INST,&&UADDSUBX_INST,&&UHADD16_INST,&&UHADD8_INST,&&UHADDSUBX_INST,&&UHSUB16_INST,&&UHSUB8_INST,
		&&UHSUBADDX_INST,&&UMAAL_INST,&&UMLAL_INST,&&UMULL_INST,&&UQADD16_INST,&&UQADD8_INST,&&UQADDSUBX_INST,&&UQSUB16_INST,
		&&UQSUB8_INST,&&UQSUBADDX_INST,&&USAD8_INST,&&USADA8_INST,&&USAT_INST,&&USAT16_INST,&&USUB16_INST,&&USUB8_INST,
		&&USUBADDX_INST,&&UXTAB16_INST,&&UXTB16_INST,&&DISPATCH,&&INIT_INST_LENGTH,&&END
	};

	int ptr;
	arm_inst * inst_base;
	unsigned int lop, rop, dst;
	unsigned int addr;
	unsigned int phys_addr;
	fault_t fault;
	int counter = 4;
	#if 0
	if (use == 1)
		goto INIT_INST_LENGTH;
	#endif

	LOAD_NZCV;
	DISPATCH:
	{
		counter --;
		if (counter == 0) {
			goto END;
		}
		/* check next instruction address is valid. */
		fault = check_address_validity(cpu, cpu->Reg[15], &phys_addr, 1);
		if (fault) {
			cpu->abortSig = true;
			cpu->Aborted = ARMul_PrefetchAbortV;
			cpu->AbortAddr = cpu->Reg[15];
			cpu->CP15[CP15(CP15_INSTR_FAULT_STATUS)] = fault & 0xff;
			cpu->CP15[CP15(CP15_FAULT_ADDRESS)] = cpu->Reg[15];
			goto END;
		}
		if (find_bb(cpu->Reg[15], ptr) == -1) {
			if (InterpreterTranslate(core, ptr) == FETCH_EXCEPTION)
				goto END;
		}
		inst_base = (arm_inst *)&inst_buf[ptr];
		GOTO_NEXT_INST;
	}
	ADC_INST:
	{
		INC_ICOUNTER;
		adc_inst *inst_cream = (adc_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND + cpu->CFlag;
			RD = dst = lop + rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG(dst, lop, rop);
				UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(adc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	ADD_INST:
	{
		INC_ICOUNTER;
		add_inst *inst_cream = (add_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			if (inst_cream->Rn == 15) {
				lop += 8;
			}
			rop = SHIFTER_OPERAND;
			RD = dst = lop + rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr*/
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Cpsr & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG(dst, lop, rop);
				UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15) {
			goto DISPATCH;
		}
		INC_PC(sizeof(add_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
#if 0
		FETCH_INST;
		GOTO_NEXT_INST;
#endif
	}
	AND_INST:
	{
		INC_ICOUNTER;
		and_inst *inst_cream = (and_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND;
			RD = dst = lop & rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr*/
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Cpsr & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
				UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(and_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	BBL_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			bbl_inst *inst_cream = (bbl_inst *)inst_base->component;
			if (inst_cream->L) {
				LINK_RTN_ADDR;
			}
			SET_PC;
			goto DISPATCH;
		}
		cpu->Reg[15] += 4;
		goto DISPATCH;
	}
	BIC_INST:
	{
		INC_ICOUNTER;
		bic_inst *inst_cream = (bic_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND;
			RD = dst = lop & (rop ^ 0xffffffff);
			if ((inst_cream->S) && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15) {
			goto DISPATCH;
		}
		INC_PC(sizeof(bic_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	BKPT_INST:
	BLX_INST:
	{
		INC_ICOUNTER;
		blx_inst *inst_cream = (blx_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int inst = inst_cream->inst;
			if (BITS(inst, 20, 27) == 0x12 && BITS(inst, 4, 7) == 0x3) {
				LINK_RTN_ADDR;
				cpu->Reg[15] = cpu->Reg[BITS(inst, 0, 3)] & 0xfffffffe;
			} else {
				DEBUG_MSG;
			}
			goto DISPATCH;
		}
		cpu->Reg[15] += 4;
		goto DISPATCH;
	}
	BX_INST:
	{
		INC_ICOUNTER;
		bx_inst *inst_cream = (bx_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			cpu->Reg[15] = RM & 0xfffffffe;
			goto DISPATCH;
		}
		cpu->Reg[15] += 4;
//		INC_PC(sizeof(bx_inst));
		goto DISPATCH;
	}
	BXJ_INST:
	CDP_INST:
	CLREX_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		cpu->Reg[15] += 4;
		INC_PC(sizeof(clrex_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	CLZ_INST:
	{
		INC_ICOUNTER;
		clz_inst *inst_cream = (clz_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			RD = clz(RM);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(clz_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	CMN_INST:
	{
		INC_ICOUNTER;
		cmn_inst *inst_cream = (cmn_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
//			printf("RN is %x\n", RN);
			lop = RN;
			rop = SHIFTER_OPERAND;
			dst = lop + rop;
			UPDATE_NFLAG(dst);
			UPDATE_ZFLAG(dst);
			UPDATE_CFLAG(dst, lop, rop);
			UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(cmn_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	CMP_INST:
	{
//		printf("cmp inst\n");
//		printf("pc:       %x\n", cpu->Reg[15]);
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
//			printf("r0 is %x\n", cpu->Reg[0]);
			cmp_inst *inst_cream = (cmp_inst *)inst_base->component;
			lop = RN;
			rop = SHIFTER_OPERAND;
			dst = lop - rop;

			UPDATE_NFLAG(dst);
			UPDATE_ZFLAG(dst);
//			UPDATE_CFLAG(dst, lop, rop);
			UPDATE_CFLAG_NOT_BORROW_FROM(lop, rop);
//			UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
			UPDATE_VFLAG_OVERFLOW_FROM(dst, lop, rop);
//			UPDATE_VFLAG_WITH_NOT(dst, lop, rop);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(cmp_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	CPS_INST:
	{
		INC_ICOUNTER;
		cps_inst *inst_cream = (cps_inst *)inst_base->component;
		uint32_t aif_val = 0;
		uint32_t aif_mask = 0;
		/* isInAPrivilegedMode */
		if (inst_cream->imod1) {
			if (inst_cream->A) {
				aif_val |= (inst_cream->imod0 << 8);
				aif_mask |= 1 << 8;
			}
			if (inst_cream->I) {
				aif_val |= (inst_cream->imod0 << 7);
				aif_mask |= 1 << 7;
			}
			if (inst_cream->F) {
				aif_val |= (inst_cream->imod0 << 6);
				aif_mask |= 1 << 6;
			}
			aif_mask = ~aif_mask;
			cpu->Cpsr = (cpu->Cpsr & aif_mask) | aif_val;
		}
		if (inst_cream->mmod) {
			cpu->Cpsr = (cpu->Cpsr & 0xffffffe0) | inst_cream->mode;
			switch_mode(cpu, inst_cream->mode);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(cps_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	CPY_INST:
	{
//		printf("cpy inst %x\n", cpu->Reg[15]);
//		printf("pc:       %x\n", cpu->Reg[15]);
//		debug_function(cpu);
//		cpu->icount ++;
		INC_ICOUNTER;
		mov_inst *inst_cream = (mov_inst *)inst_base->component;
//		cpy_inst *inst_cream = (cpy_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			RD = SHIFTER_OPERAND;
//			RD = RM;
			if ((inst_cream->Rd == 15))
				goto DISPATCH;
		}
//		printf("cpy inst %x\n", cpu->Reg[15]);
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(mov_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	EOR_INST:
	{
		INC_ICOUNTER;
		eor_inst *inst_cream = (eor_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND;
			RD = dst = lop ^ rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr*/
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
//				UPDATE_CFLAG(dst, lop, rop);
//				UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(eor_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDC_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		cpu->Reg[15] += 4;
		INC_PC(sizeof(ldc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDM_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
//			printf("in ldm_inst\n");
			int i;
			unsigned int ret;
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) {
				goto MMU_EXCEPTION;
			}
			unsigned int inst = inst_cream->inst;
			if (BITS(inst, 25, 27) == 4 && BIT(inst, 22) && BIT(inst, 20) && !BIT(inst, 15)) {
//				DEBUG_MSG;
				#if 1
				/* LDM (2) user */
				for (i = 0; i < 13; i++) {
					if(BIT(inst, i)){
						fault = interpreter_read_memory(core, addr, phys_addr, ret, 32);
						if (fault) {
							goto MMU_EXCEPTION;
						}
						cpu->Reg[i] = ret;
						addr += 4;
						phys_addr += 4;
					}
				}
				if (BIT(inst, 13)) {
					fault = interpreter_read_memory(core, addr, phys_addr, ret, 32);
					if (fault) {
						goto MMU_EXCEPTION;
					}
					if (cpu->Mode == USER32MODE) 
						cpu->Reg[13] = ret;
					else
						cpu->Reg_usr[0] = ret;
					addr += 4;
					phys_addr += 4;
				}
				if (BIT(inst, 14)) {
					fault = interpreter_read_memory(core, addr, phys_addr, ret, 32);
					if (fault) {
						goto MMU_EXCEPTION;
					}
					if (cpu->Mode == USER32MODE) 
						cpu->Reg[14] = ret;
					else
						cpu->Reg_usr[1] = ret;
				}
				#endif
			} else {
				for( i = 0; i < 16; i ++ ){
					if(BIT(inst, i)){
						//bus_read(32, addr, &ret);
						fault = interpreter_read_memory(core, addr, phys_addr, ret, 32);
						if (fault) {
							goto MMU_EXCEPTION;
						}
						cpu->Reg[i] = ret;
						addr += 4;
						phys_addr += 4;
					}
				}
				if (BITS(inst, 25, 27) == 4 && BIT(inst, 22) && BIT(inst, 20)) {
					if (CurrentModeHasSPSR) {
						cpu->Cpsr = cpu->Spsr_copy;
						switch_mode(cpu, cpu->Cpsr & 0x1f);
						LOAD_NZCV;
					}
				}
			}
			if (BIT(inst, 15)) {
//				printf("new pc is %x\n", cpu->Reg[15]);
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BIT(inst_cream->inst, 15)) {
			goto DISPATCH;
		}
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SXTH_INST:
	{
		INC_ICOUNTER;
		sxth_inst *inst_cream = (sxth_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int operand2 = rotr(RM, 8 * inst_cream->rotate);
			if (BIT(operand2, 15)) {
				operand2 |= 0xffff0000;
			} else {
				operand2 &= 0xffff;
			}
			RD = operand2;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(sxth_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDR_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			//bus_read(32, addr, &value);
			fault = interpreter_read_memory(core, addr, phys_addr, value, 32);
			if (fault) {
				goto MMU_EXCEPTION;
			}
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UXTH_INST:
	{
		INC_ICOUNTER;
		uxth_inst *inst_cream = (uxth_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int operand2 = rotr(RM, 8 * inst_cream->rotate) 
						& 0xffff;
			RD = operand2;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(uxth_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UXTAH_INST:
	{
		INC_ICOUNTER;
		uxtah_inst *inst_cream = (uxtah_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int operand2 = rotr(RM, 8 * inst_cream->rotate) 
						& 0xffff;
			RD = RN + operand2;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(uxtah_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRB_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(8, addr, &value);
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRBT_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(8, addr, &value);
//			printf("PC:%x\n", cpu->Reg[15]);
//			printf("before : %x\n", cpu->Reg[BITS(inst_cream->inst, 12, 15)]);
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
//			printf("after : %x\n", cpu->Reg[BITS(inst_cream->inst, 12, 15)]);
//			printf("before : %x\n", cpu->Reg[BITS(inst_cream->inst, 16, 19)]);
//			cpu->Reg[BITS(inst_cream->inst, 16, 19)] = addr;
//			printf("after : %x\n", cpu->Reg[BITS(inst_cream->inst, 16, 19)]);
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRD_INST:
	LDREX_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			addr = cpu->Reg[BITS(inst_cream->inst, 16, 19)];
			fault = check_address_validity(cpu, addr, &phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(32, addr, &value);
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDREXB_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			addr = cpu->Reg[BITS(inst_cream->inst, 16, 19)];
			fault = check_address_validity(cpu, addr, &phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(8, addr, &value);
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRH_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 16);
//			fault = interpreter_read_memory(core, addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
			if (value == 0xffff && cpu->icounter > 190000000 && cpu->icounter < 210000000) {
				value = 0xffffffff;
			}
			//bus_read(16, addr, &value);
//			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value & 0xffff;
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRSB_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
//			printf("ldrsb addr is %x\n", addr);
			fault = interpreter_read_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(8, addr, &value);
			if (BIT(value, 7)) {
				value |= 0xffffff00;
			}
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRSH_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 16);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(16, addr, &value);
			if (BIT(value, 15)) {
				value |= 0xffff0000;
			}
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	LDRT_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value;
			fault = interpreter_read_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
			//bus_read(8, addr, &value);
//			printf("PC:%x\n", cpu->Reg[15]);
//			printf("before : %x\n", cpu->Reg[BITS(inst_cream->inst, 12, 15)]);
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = value;
//			printf("after : %x\n", cpu->Reg[BITS(inst_cream->inst, 12, 15)]);
//			printf("before : %x\n", cpu->Reg[BITS(inst_cream->inst, 16, 19)]);
//			cpu->Reg[BITS(inst_cream->inst, 16, 19)] = addr;
//			printf("after : %x\n", cpu->Reg[BITS(inst_cream->inst, 16, 19)]);
			if (BITS(inst_cream->inst, 12, 15) == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MCR_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		mcr_inst *inst_cream = (mcr_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int inst = inst_cream->inst;
			if (inst_cream->Rd == 15) {
				DEBUG_MSG;
			} else {
				if (inst_cream->cp_num == 15) {
					if(CRn == 0 && OPCODE_2 == 0 && CRm == 0) {
						//LET(RD, CONST(0x0007b000));
						//LET(RD, CONST(0x410FB760));
						//LET(CP15_MAIN_ID, R(RD));
						CP15_REG(CP15_MAIN_ID) = RD;
					} else if(CRn == 1 && CRm == 0 && OPCODE_2 == 0) {
						//LET(CP15_CONTROL, R(RD));
						CP15_REG(CP15_CONTROL) = RD;
					} else if (CRn == 3 && CRm == 0 && OPCODE_2 == 0) {
						//LET(CP15_DOMAIN_ACCESS_CONTROL, R(RD));
						CP15_REG(CP15_DOMAIN_ACCESS_CONTROL) = RD;
					} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 0) {
						//LET(CP15_TRANSLATION_BASE_TABLE_0, R(RD));
						CP15_REG(CP15_TRANSLATION_BASE_TABLE_0) = RD;
					} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 1) {
						//LET(CP15_TRANSLATION_BASE_TABLE_1, R(RD));
						CP15_REG(CP15_TRANSLATION_BASE_TABLE_1) = RD;
					} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 2) {
						//LET(CP15_TRANSLATION_BASE_CONTROL, R(RD));
						CP15_REG(CP15_TRANSLATION_BASE_CONTROL);
			//		} else if (CRn == 7 && CRm == 14 && OPCODE_2 == 0) {
			//			LET(R(RD));
					} else {
//						printf("mcr is not implementated. CRn is %d, CRm is %d, OPCODE_2 is %d\n", CRn, CRm, OPCODE_2);
					}
				}
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(mcr_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MCRR_INST:
	MLA_INST:
	{
		INC_ICOUNTER;
		mla_inst *inst_cream = (mla_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			uint64_t rm = RM;
			uint64_t rs = RS;
			uint64_t rn = RN;
//			RD = dst = RM * RS + RN;
			RD = dst = static_cast<uint32_t>((rm * rs + rn) & 0xffffffff);
			if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
			}
			if (inst_cream->Rd == 15)
				goto DISPATCH;
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(mla_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MOV_INST:
	{
//		printf("mov inst\n");
//		printf("pc:       %x\n", cpu->Reg[15]);
//		debug_function(cpu);
//		cpu->icount ++;
		INC_ICOUNTER;
		mov_inst *inst_cream = (mov_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			RD = dst = SHIFTER_OPERAND;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
			}
			if (inst_cream->Rd == 15)
				goto DISPATCH;
//				return;
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(mov_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MRC_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		mrc_inst *inst_cream = (mrc_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int inst = inst_cream->inst;
			if (inst_cream->Rd == 15) {
				DEBUG_MSG;
			}
			if (inst_cream->inst == 0xeef04a10) {
				/* undefined instruction fmrx */
				RD = 0x20000000;
				goto END;
			} else {
				if (inst_cream->cp_num == 15) {
					if(CRn == 0 && OPCODE_2 == 0 && CRm == 0) {
						//LET(RD, CONST(0x0007b000));
						//LET(RD, CONST(0x410FB760));
						//LET(RD, R(CP15_MAIN_ID));
						RD = cpu->CP15[CP15(CP15_MAIN_ID)];
					} else if (CRn == 1 && CRm == 0 && OPCODE_2 == 0) {
						//LET(RD, R(CP15_CONTROL));
						RD = cpu->CP15[CP15(CP15_CONTROL)];
					} else if (CRn == 3 && CRm == 0 && OPCODE_2 == 0) {
						//LET(RD, R(CP15_DOMAIN_ACCESS_CONTROL));
						RD = cpu->CP15[CP15(CP15_DOMAIN_ACCESS_CONTROL)];
					} else if (CRn == 2 && CRm == 0 && OPCODE_2 == 0) {
						//LET(RD, R(CP15_TRANSLATION_BASE_TABLE_0));
						RD = cpu->CP15[CP15(CP15_TRANSLATION_BASE_TABLE_0)];
					} else if (CRn == 5 && CRm == 0 && OPCODE_2 == 0) {
						//LET(RD, R(CP15_FAULT_STATUS));
						RD = cpu->CP15[CP15(CP15_FAULT_STATUS)];
					} else if (CRn == 6 && CRm == 0 && OPCODE_2 == 0) {
						//LET(RD, R(CP15_FAULT_ADDRESS));
						RD = cpu->CP15[CP15(CP15_FAULT_ADDRESS)];
					} else if (CRn == 0 && CRm == 0 && OPCODE_2 == 1) {
						//LET(RD, R(CP15_CACHE_TYPE));
						RD = cpu->CP15[CP15(CP15_CACHE_TYPE)];
					} else if (CRn == 5 && CRm == 0 && OPCODE_2 == 1) {
						//LET(RD, R(CP15_INSTR_FAULT_STATUS));
						RD = cpu->CP15[CP15(CP15_INSTR_FAULT_STATUS)];
					}
					else {
						printf("mrc is not implementated. CRn is %d, CRm is %d, OPCODE_2 is %d\n", CRn, CRm, OPCODE_2);
					}
				}
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(mrc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MRRC_INST:
	MRS_INST:
	{
		INC_ICOUNTER;
		mrs_inst *inst_cream = (mrs_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			if (inst_cream->R) {
				RD = cpu->Spsr_copy;
			} else {
				cpu->Cpsr = (cpu->Cpsr & 0xfffffff) |
						(cpu->NFlag << 31)  |
						(cpu->ZFlag << 30)  |
						(cpu->CFlag << 29)  |
						(cpu->VFlag << 28);
				RD = cpu->Cpsr;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(mrs_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MSR_INST:
	{
		INC_ICOUNTER;
		msr_inst *inst_cream = (msr_inst *)inst_base->component;
		const uint32_t UnallocMask = 0x06f0fc00, UserMask = 0xf80f0200, PrivMask = 0x000001df, StateMask = 0x01000020;
		unsigned int inst = inst_cream->inst;
		unsigned int operand;

		if (BIT(inst, 25)) {
			int rot_imm = BITS(inst, 8, 11) * 2;
			//operand = ROTL(CONST(BITS(0, 7)), CONST(32 - rot_imm));
			operand = rotr(BITS(inst, 0, 7), (32 - rot_imm));
		} else {
			//operand = R(RM);
			operand = cpu->Reg[BITS(inst, 0, 3)];
		}
		uint32_t byte_mask = (BIT(inst, 16) ? 0xff : 0) | (BIT(inst, 17) ? 0xff00 : 0)
					| (BIT(inst, 18) ? 0xff0000 : 0) | (BIT(inst, 19) ? 0xff000000 : 0);
		uint32_t mask;
		if (!inst_cream->R) {
			if (InAPrivilegedMode(cpu)) {
				mask = byte_mask & (UserMask | PrivMask);
			} else {
				mask = byte_mask & UserMask;
			}
			//LET(CPSR_REG, OR(AND(R(CPSR_REG), COM(CONST(mask))), AND(operand, CONST(mask))));
			SAVE_NZCV;

			cpu->Cpsr = (cpu->Cpsr & ~mask) | (operand & mask);
			switch_mode(cpu, cpu->Cpsr & 0x1f);
			LOAD_NZCV;
		} else {
			if (CurrentModeHasSPSR) {
				mask = byte_mask & (UserMask | PrivMask | StateMask);
				//LET(SPSR_REG, OR(AND(R(SPSR_REG), COM(CONST(mask))), AND(operand, CONST(mask))));
				cpu->Spsr_copy = (cpu->Spsr_copy & ~mask) | (operand & mask);
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(msr_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MUL_INST:
	{
		INC_ICOUNTER;
		mul_inst *inst_cream = (mul_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
//			RD = dst = SHIFTER_OPERAND;
			RD = dst = RM * RS;
			if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
			}
			if (inst_cream->Rd == 15)
				goto DISPATCH;
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(mul_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	MVN_INST:
	{
		INC_ICOUNTER;
		mvn_inst *inst_cream = (mvn_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			RD = dst = (SHIFTER_OPERAND ^ 0xffffffff);
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
			}
			if (inst_cream->Rd == 15)
				goto DISPATCH;
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(mvn_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	ORR_INST:
	{
		INC_ICOUNTER;
		orr_inst *inst_cream = (orr_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND;
//			printf("lop is %x, rop is %x, r2 is %x, r3 is %x\n", lop, rop, cpu->Reg[2], cpu->Reg[3]);
			RD = dst = lop | rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr*/
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_WITH_SC;
//				UPDATE_CFLAG(dst, lop, rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		if (inst_cream->Rd == 15)
			goto DISPATCH;
		INC_PC(sizeof(orr_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	PKHBT_INST:
	PKHTB_INST:
	PLD_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		cpu->Reg[15] += 4;
		INC_PC(sizeof(stc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	QADD_INST:
	QADD16_INST:
	QADD8_INST:
	QADDSUBX_INST:
	QDADD_INST:
	QDSUB_INST:
	QSUB_INST:
	QSUB16_INST:
	QSUB8_INST:
	QSUBADDX_INST:
	REV_INST:
	{
		INC_ICOUNTER;
		rev_inst *inst_cream = (rev_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			RD = ((RM & 0xff) << 24) |
				(((RM >> 8) & 0xff) << 16) |
				(((RM >> 16) & 0xff) << 8) |
				((RM >> 24) & 0xff);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(rev_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	REVSH_INST:
	RFE_INST:
	RSB_INST:
	{
		INC_ICOUNTER;
		rsb_inst *inst_cream = (rsb_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			rop = SHIFTER_OPERAND;
			RD = dst = rop - lop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
				UPDATE_CFLAG_NOT_BORROW_FROM(rop, lop);
				UPDATE_VFLAG_OVERFLOW_FROM(dst, rop, lop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(rsb_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	RSC_INST:
	{
		INC_ICOUNTER;
		rsc_inst *inst_cream = (rsc_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN + !cpu->CFlag;
			rop = SHIFTER_OPERAND;
			RD = dst = rop - lop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
//				UPDATE_CFLAG(dst, lop, rop);
				UPDATE_CFLAG_WITH_NOT(dst, rop, lop);
//				cpu->CFlag = !((ISNEG(lop) && ISPOS(rop)) || (ISNEG(lop) && ISPOS(dst)) || (ISPOS(rop) && ISPOS(dst)));
				UPDATE_VFLAG((int)dst, (int)rop, (int)lop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(rsc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SADD16_INST:
	SADD8_INST:
	SADDSUBX_INST:
	SBC_INST:
	{
		INC_ICOUNTER;
		sbc_inst *inst_cream = (sbc_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = SHIFTER_OPERAND + !cpu->CFlag;
			rop = RN;
			RD = dst = rop - lop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
//				UPDATE_CFLAG(dst, lop, rop);
				UPDATE_CFLAG_NOT_BORROW_FROM(rop, lop);
//				cpu->CFlag = !((ISNEG(lop) && ISPOS(rop)) || (ISNEG(lop) && ISPOS(dst)) || (ISPOS(rop) && ISPOS(dst)));
				UPDATE_VFLAG((int)dst, (int)rop, (int)lop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(sbc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SEL_INST:
	SETEND_INST:
	SHADD16_INST:
	SHADD8_INST:
	SHADDSUBX_INST:
	SHSUB16_INST:
	SHSUB8_INST:
	SHSUBADDX_INST:
	SMLA_INST:
	SMLAD_INST:
	SMLAL_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			umlal_inst *inst_cream = (umlal_inst *)inst_base->component;
			long long int rm = RM;
			long long int rs = RS;
			long long int rst = rm * rs;
//			printf("rm : [%llx] rs : [%llx] rst [%llx]\n", RM, RS, rst);
			rst += RDLO + (RDHI << 32);
			RDLO = BITS(rst,  0, 31);
			RDHI = BITS(rst, 32, 63);

			cpu->NFlag = BIT(RDHI, 31);
			cpu->ZFlag = (RDHI == 0 && RDLO == 0);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(umlal_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SMLALXY_INST:
	SMLALD_INST:
	SMLAW_INST:
	SMLSD_INST:
	SMLSLD_INST:
	SMMLA_INST:
	SMMLS_INST:
	SMMUL_INST:
	SMUAD_INST:
	SMUL_INST:
	SMULL_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			umull_inst *inst_cream = (umull_inst *)inst_base->component;
			unsigned long long int rm = RM;
			unsigned long long int rs = RS;
			unsigned long long int rst = rm * rs;
//			printf("rm : [%llx] rs : [%llx] rst [%llx]\n", RM, RS, rst);
			RDHI = BITS(rst, 32, 63);
			RDLO = BITS(rst,  0, 31);

			if (inst_cream->S) {
				cpu->NFlag = BIT(RDHI, 31);
				cpu->ZFlag = (RDHI == 0 && RDLO == 0);
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(umull_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SMULW_INST:
	SMUSD_INST:
	SRS_INST:
	SSAT_INST:
	SSAT16_INST:
	SSUB16_INST:
	SSUB8_INST:
	SSUBADDX_INST:
	STC_INST:
	{
		INC_ICOUNTER;
		/* NOT IMPL */
		cpu->Reg[15] += 4;
		INC_PC(sizeof(stc_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STM_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		unsigned int inst = inst_cream->inst;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			int i;
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			if (BITS(inst_cream->inst, 25, 27) == 4 && BITS(inst_cream->inst, 20, 22) == 4) {
//				DEBUG_MSG;
				#if 1
				for (i = 0; i < 13; i++) {
					if(BIT(inst_cream->inst, i)){
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
					}
				}
				if (BIT(inst_cream->inst, 13)) {
					if (cpu->Mode == USER32MODE) {
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
					} else {
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg_usr[0], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
					}
				}
				if (BIT(inst_cream->inst, 14)) {
					if (cpu->Mode == USER32MODE) {
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
					} else {
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg_usr[1], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
					}
				}
				if (BIT(inst_cream->inst, 15)) {
					fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i] + 8, 32);
					if (fault) goto MMU_EXCEPTION;
				}
				#endif
			} else {
				for( i = 0; i < 15; i ++ ){
					if(BIT(inst_cream->inst, i)){
						//arch_write_memory(cpu, bb, Addr, R(i), 32);
						//bus_write(32, addr, cpu->Reg[i]);
						fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i], 32);
						if (fault) goto MMU_EXCEPTION;
						addr += 4;
						phys_addr += 4;
						//Addr = ADD(Addr, CONST(4));
					}
				}

				/* check pc reg*/
				if(BIT(inst_cream->inst, i)){
					//arch_write_memory(cpu, bb, Addr, STOREM_CHECK_PC, 32);
					//bus_write(32, addr, cpu->Reg[i] + 8);
					fault = interpreter_write_memory(core, addr, phys_addr, cpu->Reg[i] + 8, 32);
					if (fault) goto MMU_EXCEPTION;
				}
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SXTB_INST:
	{
		INC_ICOUNTER;
		sxtb_inst *inst_cream = (sxtb_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int operand2 = rotr(RM, 8 * inst_cream->rotate);
			if (BIT(operand2, 7)) {
				operand2 |= 0xffffff00;
			} else
				operand2 &= 0xff;
			RD = operand2;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(sxtb_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STR_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)];
			//bus_write(32, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UXTB_INST:
	{
		INC_ICOUNTER;
		uxtb_inst *inst_cream = (uxtb_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			unsigned int operand2 = rotr(RM, 8 * inst_cream->rotate) 
						& 0xff;
			RD = operand2;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(uxtb_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UXTAB_INST:
	STRB_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)] & 0xff;
			//bus_write(8, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STRBT_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)] & 0xff;
			//bus_write(8, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STRD_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)];
			//bus_write(32, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
			value = cpu->Reg[BITS(inst_cream->inst, 12, 15) + 1];
			//bus_write(32, addr, value);
			fault = interpreter_write_memory(core, addr + 4, phys_addr + 4, value, 32);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STREX_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			addr = cpu->Reg[BITS(inst_cream->inst, 16, 19)];
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 0, 3)];
			fault = check_address_validity(cpu, addr, &phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
//			bus_write(32, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = 0;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STREXB_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			addr = cpu->Reg[BITS(inst_cream->inst, 16, 19)];
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 0, 3)] & 0xff;
			fault = check_address_validity(cpu, addr, &phys_addr, 0);
			if (fault) goto MMU_EXCEPTION;
			//bus_write(8, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 8);
			if (fault) goto MMU_EXCEPTION;
			cpu->Reg[BITS(inst_cream->inst, 12, 15)] = 0;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STRH_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)] & 0xffff;
			//bus_write(16, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 16);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	STRT_INST:
	{
		INC_ICOUNTER;
		ldst_inst *inst_cream = (ldst_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			fault = inst_cream->get_addr(cpu, inst_cream->inst, addr, phys_addr, 1);
			if (fault) goto MMU_EXCEPTION;
			unsigned int value = cpu->Reg[BITS(inst_cream->inst, 12, 15)];
			//bus_write(16, addr, value);
			fault = interpreter_write_memory(core, addr, phys_addr, value, 32);
			if (fault) goto MMU_EXCEPTION;
		}
		cpu->Reg[15] += 4;
		if (BITS(inst_cream->inst, 12, 15) == 15)
			goto DISPATCH;
		INC_PC(sizeof(ldst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SUB_INST:
	{
		INC_ICOUNTER;
		sub_inst *inst_cream = (sub_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			lop = RN;
			if (inst_cream->Rn == 15) {
				lop += 8;
			}
			rop = SHIFTER_OPERAND;
			RD = dst = lop - rop;
			if (inst_cream->S && (inst_cream->Rd == 15)) {
				/* cpsr = spsr */
				if (CurrentModeHasSPSR) {
					cpu->Cpsr = cpu->Spsr_copy;
					switch_mode(cpu, cpu->Spsr_copy & 0x1f);
					LOAD_NZCV;
				}
			} else if (inst_cream->S) {
				UPDATE_NFLAG(dst);
				UPDATE_ZFLAG(dst);
//				UPDATE_CFLAG(dst, lop, rop);
				UPDATE_CFLAG_NOT_BORROW_FROM(lop, rop);
	//			UPDATE_VFLAG((int)dst, (int)lop, (int)rop);
				UPDATE_VFLAG_OVERFLOW_FROM(dst, lop, rop);
			}
			if (inst_cream->Rd == 15) {
				goto DISPATCH;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(sub_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SWI_INST:
	{
		INC_ICOUNTER;
		swi_inst *inst_cream = (swi_inst *)inst_base->component;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			if (core->is_user_mode) {
				arm_dyncom_SWI(cpu, inst_cream->num);
			} else {
				cpu->syscallSig = 1;
				goto END;
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(swi_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	SWP_INST:
	SWPB_INST:
	SXTAB_INST:
	SXTAB16_INST:
	SXTAH_INST:
	SXTB16_INST:
	TEQ_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			teq_inst *inst_cream = (teq_inst *)inst_base->component;
			lop = RN;
			rop = SHIFTER_OPERAND;
			dst = lop ^ rop;

			UPDATE_NFLAG(dst);
			UPDATE_ZFLAG(dst);
			UPDATE_CFLAG_WITH_SC;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(teq_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	TST_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			tst_inst *inst_cream = (tst_inst *)inst_base->component;
			lop = RN;
			rop = SHIFTER_OPERAND;
			dst = lop & rop;

			UPDATE_NFLAG(dst);
			UPDATE_ZFLAG(dst);
			UPDATE_CFLAG_WITH_SC;
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(tst_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UADD16_INST:
	UADD8_INST:
	UADDSUBX_INST:
	UHADD16_INST:
	UHADD8_INST:
	UHADDSUBX_INST:
	UHSUB16_INST:
	UHSUB8_INST:
	UHSUBADDX_INST:
	UMAAL_INST:
	UMLAL_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			umlal_inst *inst_cream = (umlal_inst *)inst_base->component;
			unsigned long long int rm = RM;
			unsigned long long int rs = RS;
			unsigned long long int rst = rm * rs;
//			printf("rm : [%llx] rs : [%llx] rst [%llx]\n", RM, RS, rst);
			rst += RDLO + (RDHI << 32);
			RDLO = BITS(rst,  0, 31);
			RDHI = BITS(rst, 32, 63);

			cpu->NFlag = BIT(RDHI, 31);
			cpu->ZFlag = (RDHI == 0 && RDLO == 0);
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(umlal_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UMULL_INST:
	{
		INC_ICOUNTER;
		if ((inst_base->cond == 0xe) || CondPassed(cpu, inst_base->cond)) {
			umull_inst *inst_cream = (umull_inst *)inst_base->component;
			unsigned long long int rm = RM;
			unsigned long long int rs = RS;
			unsigned long long int rst = rm * rs;
//			printf("rm : [%llx] rs : [%llx] rst [%llx]\n", RM, RS, rst);
			RDHI = BITS(rst, 32, 63);
			RDLO = BITS(rst,  0, 31);

			if (inst_cream->S) {
				cpu->NFlag = BIT(RDHI, 31);
				cpu->ZFlag = (RDHI == 0 && RDLO == 0);
			}
		}
		cpu->Reg[15] += 4;
		INC_PC(sizeof(umull_inst));
		FETCH_INST;
		GOTO_NEXT_INST;
	}
	UQADD16_INST:
	UQADD8_INST:
	UQADDSUBX_INST:
	UQSUB16_INST:
	UQSUB8_INST:
	UQSUBADDX_INST:
	USAD8_INST:
	USADA8_INST:
	USAT_INST:
	USAT16_INST:
	USUB16_INST:
	USUB8_INST:
	USUBADDX_INST:
	UXTAB16_INST:
	UXTB16_INST:
	MMU_EXCEPTION:
	{
		SAVE_NZCV;
		cpu->abortSig = true;
		cpu->Aborted = ARMul_DataAbortV;
		cpu->AbortAddr = addr;
		cpu->CP15[CP15(CP15_FAULT_STATUS)] = fault & 0xff;
		cpu->CP15[CP15(CP15_FAULT_ADDRESS)] = addr;
		return;
	}
	END:
	{
		SAVE_NZCV;
		return;
	}
	INIT_INST_LENGTH:
	{
#if 0
	printf("InstLabel:%d\n", sizeof(InstLabel));
	for (int i = 0; i < (sizeof(InstLabel) / sizeof(void *)); i ++)
		printf("[%llx]\n", InstLabel[i]);
	printf("InstLabel:%d\n", sizeof(InstLabel));
#endif
	InterpreterInitInstLength((unsigned long long int *)InstLabel, sizeof(InstLabel));
#if 0
	for (int i = 0; i < (sizeof(InstLabel) / sizeof(void *)); i ++)
		printf("[%llx]\n", InstLabel[i]);
	printf("%llx\n", InstEndLabel[1]);
	printf("%llx\n", InstLabel[1]);
	printf("%lld\n", (char *)InstEndLabel[1] - (char *)InstLabel[1]);
#endif
	return;
	}
}

