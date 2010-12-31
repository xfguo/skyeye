#include "llvm/Instructions.h"
#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "arm_internal.h"
#include "arm_types.h"
#include "dyncom/tag.h"

#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
#define BIT(n) ((instr >> (n)) & 1)
#define BAD	do{printf("meet BAD at %s\n", __FUNCTION__ ); exit(0);}while(0);
#define ptr_N	cpu->ptr_N
#define ptr_Z	cpu->ptr_Z
#define ptr_C	cpu->ptr_C
#define ptr_V	cpu->ptr_V
#define ptr_I 	cpu->ptr_I
#define	ptr_CPSR cpu->ptr_gpr[16]

/*xxxx xxxx xxxx xxxx 1111 xxxx xxxx xxxx */
#define RD ((instr >> 12) & 0xF)
/*xxxx xxxx xxxx 1111 xxxx xxxx xxxx xxxx */
#define RN ((instr >> 16) & 0xF)
/*xxxx xxxx xxxx xxxx xxxx xxxx xxxx 1111 */
#define RM (instr & 0xF)
#define BIT(n) ((instr >> (n)) & 1)
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
/*xxxx xx1x xxxx xxxx xxxx xxxx xxxx xxxx */
#define I ((instr >> 25) & 1)
/*xxxx xxxx xxx1 xxxx xxxx xxxx xxxx xxxx */
#define S BIT(20)

#define SHIFT BITS(5,6)
#define SHIFT_IMM BITS(7,11)

#define LSPBIT  BIT(24)
#define LSUBIT  BIT(23)
#define LSBBIT  BIT(22)
#define LSWBIT  BIT(21)
#define LSLBIT  BIT(20)
#define OFFSET12 BITS(0,11)

using namespace llvm;

Value *GetLSAddr5x(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

	Value *Addr;
	if(LSUBIT)
		Addr =  ADD(R(RN), CONST(OFFSET12));
	else
		Addr =  SUB(R(RN), CONST(OFFSET12));

	if(LSWBIT)
		LET(RN, Addr);

	if(RN == 15)
		Addr = ADD(Addr, CONST(8));

	return Addr;
}

Value *GetLSAddr7x(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	printf("in GetLSAddr7x may be not compilitable\n");

	Value *Addr;
	if(BITS(4,11) == 0) {
		if(LSUBIT)
			Addr = ADD(R(RN), R(RM));
		else
			Addr = SUB(R(RN), R(RM));
	} else{
		int shift = SHIFT;
		Value *index;
		switch(shift) {
		case 0:	/* LSL */
			index = SHL(R(RM), CONST(SHIFT_IMM));
			break;
		case 1: /* LSR */
			if(SHIFT_IMM == 0)
				index = CONST(0);
			else
				index = LSHR(R(RM), CONST(SHIFT_IMM));
			break;
		case 2:	/* ASR */
			if(SHIFT_IMM == 0)
				index = ADD(NOT(AND(R(RM), CONST(0x80000000))), CONST(1));
			else
				index = ASHR(R(RM), CONST(SHIFT_IMM));
			break;
		case 3:	/* ROR or RRX */
			break;
		}

		if(LSUBIT)
			Addr = ADD(R(RN), index);
		else
			Addr = SUB(R(RN), index);
	}

	if(RN == 15)
		Addr = ADD(Addr, CONST(8));

	return Addr;
}


Value *operand(cpu_t *cpu,  uint32_t instr, BasicBlock *bb)
{
        if (I) { /* 32-bit immediate */
                //XXX TODO: shifter carry out
                uint32_t immed_8 = instr & 0xFF;
                int rotate_imm = ((instr >> 8) & 0xF) << 1;
                return CONST((immed_8 >> rotate_imm) | (immed_8 << (32 - rotate_imm)));
        } else {
                if (!BIT(4)) { /* Immediate shifts */
                        int shift = BITS(5,6);
                        int shift_imm = BITS(7,11);
                        LOG("shift=%x\n", shift);
                        LOG("shift_imm=%x\n", shift_imm);
                        if (!shift && !shift_imm) { /* Register */
                                return R(RM);
                        } else {
                                BAD;
                        }
                } else {
                        if (!BIT(7)) { /* Register shifts */
                                BAD;
                        } else { /* arithmetic or Load/Store instruction extension space */
                                BAD;
                        }
                }
        }
}

Value *boperand(cpu_t *cpu,  uint32_t instr, BasicBlock *bb)
{
		uint32_t rotate_imm = instr;
		if(instr &  0x800000)
			rotate_imm = (~rotate_imm + 1) & 0x0ffffff;
		else
			rotate_imm &= 0x0ffffff;

		rotate_imm = rotate_imm << 2;

		return CONST(rotate_imm);
}
#define OPERAND operand(cpu,instr,bb)
#define BOPERAND boperand(cpu,instr,bb)
#define GETLSADDR5x GetLSAddr5x(cpu,instr,bb)
#define GETLSADDR7x GetLSAddr7x(cpu,instr,bb)

int arm_opc_trans_00(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* AND Reg I = 0 S = 0*/

	if (BITS (4, 7) == 0xB) {
		/* STRH register offset, no write-back, down, post indexed.  */
		/* P = 0, U = 0, I = 0, W = 0 */
		return 0;
	}

	if (BITS (4, 7) == 0xD) {
		/* LDRD P = 0, U = 0, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD P = 0, U = 0, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 9) {
		/* MUL? */
	}
	else {
		/* AND reg.  */
	}

	return 0;

}

int arm_opc_trans_01(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ANDS reg and MULS */
	if ((BITS (4, 11) & 0xF9) == 0x9)
		/* LDR register offset, no write-back, down, post indexed.  */
		return 0;
	if (BITS (4, 7) == 9) {
		/* MULS  S = 1? */
	}
	else {
		/* ANDS reg.  */
	}
	return 0;
}

int arm_opc_trans_02(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EOR reg I = 0, S = 0 and MLA S = 0 */
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, down, post indexed. */
		/* P = 0; U = 0; I = 0; W =1 */
		return 0 ;
	}

	if (BITS (4, 7) == 9) {	/* MLA */

	}
	else {	/* EOR */

	}

}

int arm_opc_trans_03(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EORS reg I = 0, S = 1and MLAS S = 1 */
	if ((BITS (4, 11) & 0xF9) == 0x9) {
		/* LDR register offset, write-back, down, post-indexed.  */
		/* ????????????????????????????*/
	}

	if (BITS (4, 7) == 9) {
		/* MLAS */
	}
	else {
		/* EORS */
	}
	if(RD == 15){
		new StoreInst(SUB(R(14), CONST(4)), cpu->ptr_PC, bb);
	}
	LET(RD, OPERAND);
	return 0;
}

int arm_opc_trans_04(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SUB reg  I = 0 S = 0*/
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, no write-back, down, post indexed.  */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* LDRD */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		return 0;
	}

	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *res = SUB(op1, op2);
	LET(RD, res);

	return 0;
}

int arm_opc_trans_05(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_06(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_07(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_08(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADD reg I = 0, S = 0*/
	if(BITS(4, 7) == 0xB) {
		/* STRH reg offset, no write-back, up, post, indexed */
		/*P = 0, U = 1, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 0,, U = 1, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD */
		/*P = 0, U = 1, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0x9) {
		/* MULL ?*/
		return 0;
	}

	return 0;
}

int arm_opc_trans_09(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADDS reg I = 0, S = 0*/
	if (BITS (4, 7) == 0x9) {
		/* LDREX */
		return 0;
	}

	if (BITS (4, 7) == 0x9) {
		/* MULL ?*/
		return 0;
	}

	return 0;
}

int arm_opc_trans_0a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADC I=0, S=0 */
	if (BITS(4,7) == 0xB) {
		/* SHTR register offset, write-back, up, post-indexed */
		/* P = 0, U = 1, I = 0, W=1 */

		return 0;
	}
	if (BITS(4,7) == 0x9) {
		/* MULL ? */

		return 0;
	}


	return 0;
}

int arm_opc_trans_0b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADCS regs S = 1 I = 0 */
	if (BITS(4, 7) == 0x9) {
		/* LDR register offset, write-back, up, post indexed */
		/* LDRD P = 0, u = 1, I = 0, w = 1 */
		return 0;
	}
	/*
	if (BITS(4, 7) == 0x9) }
	 MULL?
		return 0;
	}
	*/

	return 0;
}

int arm_opc_trans_0c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_0d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_0e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_0f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}


int arm_opc_trans_10(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{



}

int arm_opc_trans_11(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_12(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) == 3) {
	/* v5 BLX(2) */
		return 0;
	}

	if (BITS (4, 11) == 5) {
		/* ElSegundo QSUB insn.  */
		return 0;
	}
				//}
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, down, pre indexed.  */
		/* p = 1, U = 0, I = 0, W = 1 */
		return 0;
	}
	if (BITS (4, 27) == 0x12FFF1) {
		/* BX */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD P = 1, U = 0, I = 0, W = 1, 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* LDRD P = 1, U = 0, I = 0, W = 1, 0 */
		return 0;
	}
				//if (state->is_v5) {
	if (BITS (4, 7) == 0x7) {
		/* BKPT Force the next instruction to be refetched.  */
		return 0;
	}
	//}

	/* ~MODET */
//	new StoreInst(OPERAND, cpu->ptr_PC, bb);

	return 0;
}

int arm_opc_trans_13(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_14(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_15(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMP reg I = 0 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op1, op2);
	/* z */ new StoreInst(ICMP_EQ(ret, CONST(0)), ptr_Z, bb);
	/* N */ new StoreInst(ICMP_SLT(ret, CONST(0)), ptr_N, bb);
	/* C */ new StoreInst(ICMP_SLE(ret, CONST(0)), ptr_N, bb);
	/* V */ new StoreInst(TRUNC1(LSHR(AND(XOR(op1, op2), XOR(op1,ret)),CONST(31))), ptr_V, false, bb);
	return 0;
}

int arm_opc_trans_16(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	//if (state->is_v5e) {
	if (BIT (4) == 0 && BIT (7) == 1
	    && BITS (12, 15) == 0) {
		/* ElSegundo SMULxy insn.  */
		return 0;
	}

	if (BITS (4, 11) == 5) {
		/* ElSegundo QDSUB insn.  */
		return 0;
	}

	if (BITS (4, 11) == 0xF1
	    && BITS (16, 19) == 0xF) {
		/* ARM5 CLZ insn.  */
		return 0;
	}
				//}
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, write-back, down, pre indexed.  */
		/* P = 1, U = 0, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/*LDRD P = 1, U = 0, I = 0, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/*STRD P = 1, U = 0, I = 0, W = 0 */
		return 0;
	}
	return 0;
}

int arm_opc_trans_17(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMN reg I = 0*/
	if ((BITS (4, 7) & 0x9) == 0x9)
		/* LDR immediate offset, write-back, down, pre indexed.  */
		/*  ? */
		return 0;

	return 0;
}

int arm_opc_trans_18(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_19(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_1a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* mov reg  I = 0 S = 0*/
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, up, pre indexed.  */
		/* p = 1, U = 1, I = 0, W = 1 */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* p = 1, U = 1, I = 0, W = 1 */
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD */
		/* p = 1, U = 1, I = 0, W = 1 */
		return 0;
	}
	LET(RD, OPERAND);
	return 0;
}

int arm_opc_trans_1b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* movs reg  I = 0 S = 1*/
	if ((BITS (4, 11) & 0xF9) == 0x9)
		/* LDR register offset, write-back, up, pre indexed.  */

		return 0;
	return 0;
}

int arm_opc_trans_1c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC reg  I = 0, S = 0*/

	if (BITS (4, 7) == 0x9) {
		/* STREXB  v6*/
		return 0;
	}
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, no write-back, up, pre indexed.  */
		/* P = 1, U = 1, I = 1, W = 0 */
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 1, U = 1, I = 1, W = 0 */
		return 0;
	}
	else if (BITS (4, 7) == 0xF) {
		/* STRD */
		/* P = 1, U = 1, I = 1, W = 0 */
		return 0;
	}
	return 0;
}

int arm_opc_trans_1d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BICS reg  I = 0, S = 1 */

	if (BITS(4, 7) == 0xF) {
		/* LDRSH P=1 U=1 W=0 */
		return 0;

	}
	if (BITS (4, 7) == 0xb) {
		/* LDRH immediate offset, no write-back, up, pre indexed.  */
		/* P = 1, U = 1, I = 1, W = 0 */
		return 0;
	}

	/* Continue with instruction decoding.  */
	if ((BITS (4, 7)) == 0x9) {
		/* ldrexb */
		/* LDR immediate offset, no write-back, up, pre indexed.  */
		return 0;
	}
	return 0;
}

int arm_opc_trans_1e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{



}

int arm_opc_trans_1f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{



}

int arm_opc_trans_20(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* AND immed I = 1, S = 0 */
	return 0;
}

int arm_opc_trans_21(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ANDS immed I = 1, S = 1 */
	return 0;
}

int arm_opc_trans_22(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EOR immed I = 1, S = 0 */
	return 0;
}

int arm_opc_trans_23(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EORS immed  I = 0, S = 1*/
	return 0;
}

int arm_opc_trans_24(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SUB immed I = 0, $ = 0 */
	Value* op1 = R(RN);
	Value* op2 = OPERAND;
	LET(RD,SUB(op1, op2));
	return 0;
}

int arm_opc_trans_25(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_26(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_27(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_28(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADD immed  I = 1 S = 0*/
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	LET(RD, ADD(op1, op2));
	return 0;
}

int arm_opc_trans_29(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADDS immed  I = 1 S = 1*/


}

int arm_opc_trans_2a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADC immed  I = 1 S = 0 */

	return 0;
}

int arm_opc_trans_2b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADCS immed  I = 1 S = 1 */

	return 0;
}

int arm_opc_trans_2c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_2d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_2e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_2f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_30(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_31(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

#define CPSR 16
int arm_opc_trans_32(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* TEQ immed and MSR immed to CPSR */
        /* MSR immed to CPSR. R = 0(set CPSR) */

}

int arm_opc_trans_33(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_34(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_35(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMPP immed I = 1, */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op1,op2);
	/* z */ new StoreInst(ICMP_EQ(ret, CONST(0)), ptr_Z, bb);
	/* N */ new StoreInst(ICMP_SLT(ret, CONST(0)), ptr_N, bb);
	/* C */ new StoreInst(ICMP_SLE(ret, CONST(0)), ptr_N, bb);
	/* V */ new StoreInst(TRUNC1(LSHR(AND(XOR(op1, op2), XOR(op1,ret)),CONST(31))), ptr_V, false, bb);
	return 0;
}

int arm_opc_trans_36(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMN immed and MSR immed to SPSR */
	/* MSR R = 1 set SPSR*/

}

int arm_opc_trans_37(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMNP immed.  */
	return 0;
}

int arm_opc_trans_38(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_39(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_3a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MOV immed. I = 1, S = 0 */
	if(RD == 15){
		new StoreInst(SUB(R(14), CONST(4)), cpu->ptr_PC, bb);
	}
	LET(RD, OPERAND);

	return 0;
}

int arm_opc_trans_3b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MOV immed. I = 1, S = 1 */
	return 0;
}

int arm_opc_trans_3c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC immed I = 1, S = 0 */
	return 0;
}

int arm_opc_trans_3d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC immed I = 1, S = 0 */
	return 0;
}

int arm_opc_trans_3e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MVN immed */
	/* I = 1, S = 0 */
	LET(RD, XOR(OPERAND,CONST(0xFFFFFFFF)));

}

int arm_opc_trans_3f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_40(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_41(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_42(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_43(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_44(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_45(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_46(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_47(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_48(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_49(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_4f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_50(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_51(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_52(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_53(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_54(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_55(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_56(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_57(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_58(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Pre Inc, Immed. */
	/* I = 0, P = 1, U = 1, W = 0 */
	Value *addr = GETLSADDR5x;
	arch_write_memory(cpu, bb, addr, R(RD), 32);
}

int arm_opc_trans_59(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Pre Inc, Immed.  */
	/* I = 0, P = 1, U = 0, W = 0 */
	Value *addr = GETLSADDR5x;
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 32);
	LET(RD,ret);
	return 0;
}

int arm_opc_trans_5a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_5b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_5c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_5d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_5e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_5f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_60(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_61(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_62(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_63(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_64(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	BAD;
	return 0;
}

int arm_opc_trans_65(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_66(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_67(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_68(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_69(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_6f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_70(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_71(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_72(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_73(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_74(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_75(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_76(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_77(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_78(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_79(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_7a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_7b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_7c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_7d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Pre Inc, Reg. */
	/* P = 1 , U = 1, W = 0 */
	if(BIT(4)){
		/* UNDEF INSTR */
	}

	Value *addr = GETLSADDR7x;
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 8);
	LET(RD,ret);
}

int arm_opc_trans_7e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_7f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_80(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_81(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_82(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_83(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_84(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_85(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_86(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_87(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_88(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_89(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_8f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_90(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_91(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_92(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, WriteBack, PreDec */
	/* STM(1) P = 1, U = 0, W = 1 */
	int i =  BITS(0,15);
	int count = 0;
	Value *addr;
	while(i){
		count ++;
		i = i >> 1;
	}

	if(!LSUBIT)
		addr = SUB(R(RN), CONST(count * 4));
	for( i = 0; i < 16; i ++ ){
		if(BIT(i)){
			arch_write_memory(cpu, bb, addr, R(i), 32);
			addr = ADD(addr, CONST(4));
		}
	}
	LET(RN, SUB(R(RN), CONST(count * 4)));
}

int arm_opc_trans_93(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_94(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_95(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_96(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_97(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_98(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_99(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_9f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_a0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* 0xa0 - 0xa7 branch postive addr */


}

int arm_opc_trans_a1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

int arm_opc_trans_a8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* 0xa8 - 0xaf negative addr */
	LET(14, ADD(R(15), CONST(4)));
	LET(15, SUB(ADD(R(15), CONST(8)),BOPERAND));
}

int arm_opc_trans_a9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_aa(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_ab(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_ac(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_ad(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_ae(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_af(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

int arm_opc_trans_b0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* b0 - b7 branch and link forward */
	LET(14, ADD(R(15),CONST(4)));
	LET(15, ADD(ADD(R(15),BOPERAND), CONST(8)));
}

int arm_opc_trans_b1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

int arm_opc_trans_b8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* b8 - bf branch and link backward */
	LET(15, ADD(R(15),BOPERAND));
}

int arm_opc_trans_b9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_ba(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_bb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_bc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_bd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_be(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_bf(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

int arm_opc_trans_c0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
}

int arm_opc_trans_c1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 0, W  = 0 */
	return 0;
}

int arm_opc_trans_c2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_c3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 0, W  = 1 */
	return 0;
}

int arm_opc_trans_c4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_c5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 1, W  = 0 */
	/* undef Instr */
	return 0;
}

int arm_opc_trans_c6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_c7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 1, W  = 1 */
	return 0;
}

int arm_opc_trans_c8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_c9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Inc.  */
	/* P = 0, U = 1, N = 0, W  = 0 */
	return 0;
}

int arm_opc_trans_ca(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_cb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Inc.   */
	/* P = 0, U = 1, N = 0, W  = 1 */
	return 0;
}

int arm_opc_trans_cc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_cd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Inc.   */
	/* P = 0, U = 1, N = 1, W  = 0 */
	return 0;
}

int arm_opc_trans_ce(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
}

int arm_opc_trans_cf(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 1, N = 1, W  = 1 */
	return 0;
}

int arm_opc_trans_d0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_d1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Dec.  */
	/* P = 1, U = 0, N = 0, W  = 0 */
	return 0;
}

int arm_opc_trans_d2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_d3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Pre Dec.    */
	/* P = 1, U = 0, N = 0, W  = 1 */
	return 0;
}

int arm_opc_trans_d4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_d5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Dec.  */
	/* P = 1, U = 0, N = 1, W  = 0 */
	return 0;
}

int arm_opc_trans_d6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_d7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Pre Dec.    */
	/* P = 1, U = 0, N = 1, W  = 1 */
	return 0;
}

int arm_opc_trans_d8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_d9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 0, W  = 0 */
	return 0;
}

int arm_opc_trans_da(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_db(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 0, W  = 1 */
	return 0;
}

int arm_opc_trans_dc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_dd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 1, W  = 0 */
	return 0;
}

int arm_opc_trans_de(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_df(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 1, W  = 1 */
	return 0;
}

int arm_opc_trans_e0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MCR e0,e2,e4,e6,e8,ea,ec,ee */

}

int arm_opc_trans_e1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CDP e1, e3, e5, e7, e9, eb, ed, ef,  Co-Processor Register Transfers (MRC) and Data Ops. */

}

int arm_opc_trans_e2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	//if (state->is_XScale)
		switch (BITS (18, 19)) {
		case 0x0:
			if (BITS (4, 11) == 1
			    && BITS (16, 17) == 0) {
				/* XScale MIA instruction.  Signed multiplication of
							   two 32 bit values and addition to 40 bit accumulator.  */
		}
		break;

	case 0x2:
		if (BITS (4, 11) == 1
		    && BITS (16, 17) == 0) {
			/* XScale MIAPH instruction.  */
		}
		break;

	case 0x3:
		if (BITS (4, 11) == 1) {
			/* XScale MIAxy instruction.  */
		}
		break;
	default:
		break;
		}
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_e3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_e4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_e5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_e6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_e7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_e8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_e9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_ea(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_eb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_ec(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

int arm_opc_trans_ed(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_ee(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
}

int arm_opc_trans_ef(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

int arm_opc_trans_f0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

}

int arm_opc_trans_f6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_f7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_f8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_f9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_fa(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_fb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_fc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_fd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_fe(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

int arm_opc_trans_ff(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}
