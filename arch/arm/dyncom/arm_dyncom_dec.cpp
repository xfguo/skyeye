#include "llvm/Instructions.h"
#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "arm_internal.h"
#include "arm_types.h"
#include "dyncom/tag.h"

using namespace llvm;
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
#define BIT(n) ((instr >> (n)) & 1)
#define BAD	do{printf("meet BAD at %s, instr is %x\n", __FUNCTION__, instr ); exit(0);}while(0);
#define ptr_N	cpu->ptr_N
#define ptr_Z	cpu->ptr_Z
#define ptr_C	cpu->ptr_C
#define ptr_V	cpu->ptr_V
#define ptr_I 	cpu->ptr_I
#define	ptr_CPSR cpu->ptr_gpr[16]

/* for MUL instructions */
/*xxxx xxxx xxxx 1111 xxxx xxxx xxxx xxxx */
#define RDHi ((instr >> 16) & 0xF)
/*xxxx xxxx xxxx xxxx 1111 xxxx xxxx xxxx */
#define RDLo ((instr >> 12) & 0xF)
/*xxxx xxxx xxxx 1111 xxxx xxxx xxxx xxxx */
#define MUL_RD ((instr >> 16) & 0xF)
/*xxxx xxxx xxxx xxxx 1111 xxxx xxxx xxxx */
#define MUL_RN ((instr >> 12) & 0xF)
/*xxxx xxxx xxxx xxxx xxxx 1111 xxxx xxxx */
#define RS ((instr >> 8) & 0xF)

/*xxxx xxxx xxxx xxxx 1111 xxxx xxxx xxxx */
#define RD ((instr >> 12) & 0xF)
/*xxxx xxxx xxxx 1111 xxxx xxxx xxxx xxxx */
#define RN ((instr >> 16) & 0xF)
/*xxxx xxxx xxxx xxxx xxxx xxxx xxxx 1111 */
#define RM (instr & 0xF)
#define BIT(n) ((instr >> (n)) & 1)
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
/*xxxx xx1x xxxx xxxx xxxx xxxx xxxx xxxx */
#define I BIT(25)
/*xxxx xxxx xxx1 xxxx xxxx xxxx xxxx xxxx */
#define S BIT(20)

#define SHIFT BITS(5,6)
#define SHIFT_IMM BITS(7,11)
#define IMMH BITS(8,11)
#define IMML BITS(0,3)

#define LSPBIT  BIT(24)
#define LSUBIT  BIT(23)
#define LSBBIT  BIT(22)
#define LSWBIT  BIT(21)
#define LSLBIT  BIT(20)
#define LSSHBITS BITS(5,6)
#define OFFSET12 BITS(0,11)
#define SBIT  BIT(20)
#define DESTReg (BITS (12, 15))

/* they are in unused state, give a corrent value when using */
#define IS_V5E 0
#define IS_V5  0
#define IS_V6  0
#define LHSReg 0

/* temp define the using the pc reg need implement a flow */
#define STORE_CHECK_RD_PC	ADD(R(RD), CONST(8))

/*
*		LoadStore operations funcs relationship
* 			LoadStore
*          |		    |		    |
* WOrUBLoadStore     MisLoadStore	 LoadStoreM
*/

/* store a word to memory */
void StoreWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(RD == 15)
		arch_write_memory(cpu, bb, addr, STORE_CHECK_RD_PC, 32);
	else
		arch_write_memory(cpu, bb, addr, R(RD), 32);
}

/* store a half word to memory */
void StoreHWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	arch_write_memory(cpu, bb, addr, R(RD), 16);
}

/* store a byte word to memory */
void StoreByte(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	arch_write_memory(cpu, bb, addr, R(RD), 8);
}

/* store a double word to memory */
void StoreDWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	arch_write_memory(cpu, bb, addr, R(RD), 32);
	arch_write_memory(cpu, bb, ADD(addr,CONST(4)), R(RD + 1),32);
}

/* load a word from memory */
void LoadWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 32);
	LET(RD,ret);
}

/* load a half word from memory */
void LoadHWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 16);
	LET(RD,ret);
}

/* load a signed half word from memory */
void LoadSHWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 1, 16);
	LET(RD,ret);
}

/* load a byte from memory */
void LoadByte(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 8);
	LET(RD,ret);
}

/* load a signed byte from memory */
void LoadSByte(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 1, 8);
	LET(RD,ret);
}

/* load a double word from memory */
void LoadDWord(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	Value *ret = arch_read_memory(cpu, bb, addr, 0, 32);
	LET(RD,ret);
	ret = arch_read_memory(cpu, bb, ADD(addr,CONST(4)), 0, 32);
	LET(RD+1,ret);
}

/* word or unsigned byte load operation, following arm doc */
void WOrUBLoad(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(LSBBIT)
		LoadSByte(cpu, instr, bb, addr);
	else
		LoadWord(cpu, instr, bb, addr);
}

/* word or unsigned byte store operation, following arm doc */
void WOrUBStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(LSBBIT)
		StoreByte(cpu, instr, bb, addr);
	else
		StoreWord(cpu, instr, bb, addr);
}

/* word or unsigned byte load operation, following arm doc */
void WOrUBLoadStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(LSLBIT)
		WOrUBLoad(cpu, instr, bb, addr);
	else
		WOrUBStore(cpu, instr, bb, addr);
}

/* Miscellaneous load operations, following arm doc */
void MisLoad(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	switch (LSSHBITS){
		case 0:
			LoadByte(cpu,instr,bb,addr);
			break;
		case 1:
			LoadHWord(cpu,instr,bb,addr);
			break;
		case 2:
			LoadSByte(cpu,instr,bb,addr);
			break;
		case 3:
			LoadHWord(cpu,instr,bb,addr);
			break;
	}
}

/* Miscellaneous store operations, following arm doc */
void MisStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	switch (LSSHBITS){
		case 0:
			StoreByte(cpu,instr,bb,addr);
			break;
		case 1:
			StoreHWord(cpu,instr,bb,addr);
			break;
		case 2:
			LoadDWord(cpu,instr,bb,addr);
			break;
		case 3:
			StoreDWord(cpu,instr,bb,addr);
			break;
	}
}

/* Miscellaneous store load operation collecton, following arm doc */
void MisLoadStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(LSLBIT)
		MisLoad(cpu,instr,bb,addr);
	else
		MisStore(cpu,instr,bb,addr);
}

/* Load multiple operation, following arm doc */
void LoadM(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	int i;
	Value *ret;
	Value *Addr = addr;
	for( i = 0; i < 16; i ++ ){
		if(BIT(i)){
			ret = arch_read_memory(cpu, bb, Addr, 0, 32);
			LET(i, ret);
			Addr = ADD(Addr, CONST(4));
		}
	}
}

/* temp define the using the pc reg need implement a flow */
#define STOREM_CHECK_PC ADD(R(15), CONST(8))
/* store multiple operation, following arm doc */
void StoreM(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	int i;
	Value *Addr = addr;
	for( i = 0; i < 15; i ++ ){
		if(BIT(i)){
			arch_write_memory(cpu, bb, Addr, R(i), 32);
			Addr = ADD(Addr, CONST(4));
		}
	}

	/* check pc reg*/
	if(BIT(i)){
		arch_write_memory(cpu, bb, Addr, STOREM_CHECK_PC, 32);
	}
}

/* load store multiple operations collection, following arm doc */
void LoadStoreM(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(LSLBIT)
		LoadM(cpu,instr,bb,addr);
	else
		StoreM(cpu,instr,bb,addr);
}

/* load store operations collection */
void LoadStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr)
{
	if(BITS(24,27) == 0x4 || BITS(24,27) == 0x5 || BITS(24,27) == 0x6 || BITS(24,27) == 0x7){
		WOrUBLoadStore(cpu, instr, bb, addr);
	}else if(BITS(24,27) == 0x0 || BITS(24,27) == 0x1){
		MisLoadStore(cpu, instr, bb, addr);
	}else if(BITS(24,27) == 0x8 || BITS(24,27) == 0x9){
		LoadStoreM(cpu, instr, bb, addr);
	}else{
		printf("Not a Load Store operation \n");
	}
}

#define CHECK_REG15() \
	do{ \
		if(RN == 15) \
			Addr = ADD(Addr, CONST(8)); \
	}while(0)


// FIXME set_condition added by yukewei
// if S = 1 set CPSR zncv bit
int set_condition(cpu_t *cpu, Value *ret, BasicBlock *bb, Value *op1, Value *op2)
{
	/* z */ new StoreInst(ICMP_EQ(ret, CONST(0)), ptr_Z, bb);
	/* N */ new StoreInst(ICMP_SLT(ret, CONST(0)), ptr_N, bb);
	/* new StoreInst(ICMP_SLE(ret, CONST(0)), ptr_N, bb); */
//	/* C */ new StoreInst(ICMP_SLE(ret, op1), ptr_C, false, bb);
	/* C */ new StoreInst(ICMP_ULE(ret, op1), ptr_C, false, bb);
	/* V */ new StoreInst(TRUNC1(LSHR(AND(XOR(op1, op2), XOR(op1,ret)),CONST(31))), ptr_V, false, bb);

	return 0;
}

/*	Getting Address from a LoadStore instruction
*			GetAddr
*		|	   |		|
*	    MisGetAddr		    LSMGetAddr
*		      WOrUBGetAddr
*
*/
/* Addr Mode 1 */

/* Addr Mode 2, following arm operand doc */
/* Getting Word or Unsigned Byte Address Immediate offset operand.in arm doc */
Value *WOrUBGetAddrImmOffset(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr;
	if(LSUBIT)
		Addr =  ADD(R(RN), CONST(OFFSET12));
	else
		Addr =  SUB(R(RN), CONST(OFFSET12));

	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address register offset operand.in arm doc */
Value *WOrUBGetAddrRegOffset(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr;
	if(LSUBIT)
		Addr =  ADD(R(RN), R(RM));
	else
		Addr =  SUB(R(RN), R(RM));

	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled register offset operand.in arm doc */
Value *WOrUBGetAddrScaledRegOffset(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr;
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
			index = ADD(XOR(LSHR(R(RM), CONST(31)), CONST(-1)), CONST(1));
		else
			index = ASHR(R(RM), CONST(SHIFT_IMM));
		break;
	case 3:	/* ROR or RRX */
		if(SHIFT_IMM == 0)
			;/* CFLAG? */
		else
			index = ROTL(R(RM), CONST(SHIFT_IMM));
		break;
	}

	if(LSUBIT)
		Addr = ADD(R(RN), index);
	else
		Addr = SUB(R(RN), index);

	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address Immediate Preload operand.in arm doc */
Value *WOrUBGetAddrImmPre(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = WOrUBGetAddrImmOffset(cpu, instr, bb);
	LET(RN, Addr);
	return Addr;
}

/* Getting Word or Unsigned Byte Address Register Preload operand.in arm doc */
Value *WOrUBGetAddrRegPre(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = WOrUBGetAddrRegOffset(cpu, instr, bb);
	LET(RN, Addr);
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled Register Pre-indexed operand.in arm doc */
Value *WOrUBGetAddrScaledRegPre(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = WOrUBGetAddrScaledRegOffset(cpu, instr, bb);
	LET(RN, Addr);
	return Addr;
}

/* Getting Word or Unsigned Byte Immediate Post-indexed operand.in arm doc */
Value *WOrUBGetAddrImmPost(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = R(RN);
	LET(RN,WOrUBGetAddrImmOffset(cpu, instr, bb));
	return Addr;
}

/* Getting Word or Unsigned Byte Address register Post-indexed operand.in arm doc */
Value *WOrUBGetAddrRegPost(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = R(RN);
	LET(RN,WOrUBGetAddrRegOffset(cpu, instr, bb));
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled register Post-indexed operand.in arm doc */
Value *WOrUBGetAddrScaledRegPost(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = R(RN);;
	LET(RN,WOrUBGetAddrScaledRegOffset(cpu, instr, bb));
	return Addr;
}

/* Getting Word or Unsigned Byte Address Immediate operand operations collection */
Value *WOrUBGetAddrImm(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x5){
		if(!BIT(21)){
		/* ImmOff */
			return WOrUBGetAddrImmOffset(cpu, instr, bb);
		}else{
		/* ImmPre */
			return WOrUBGetAddrImmPre(cpu, instr, bb);
		}
	}else if(BITS(24,27) == 0x4){
		/* ImmPost */
		if(!BIT(21)){
			return WOrUBGetAddrImmPost(cpu, instr, bb);
		}
	}
	printf(" Error in WOrUB Get Imm Addr \n");
}

/* Getting Word or Unsigned Byte Address reg operand operations collection */
Value *WOrUBGetAddrReg(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x7){
		if(!BIT(21)){
		/* Reg off */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegOffset(cpu, instr, bb);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegOffset(cpu, instr, bb);
			}
		} else {
		/* Reg pre */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegPre(cpu, instr, bb);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegPre(cpu, instr, bb);
			}
		}
	}else if(BITS(24,27) == 0x6){
		if(!BIT(21)){
		/* Reg post */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegPost(cpu, instr, bb);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegPost(cpu, instr, bb);
			}
		}
	}
	printf(" Error in WOrUB Get Reg Addr \n");
}

/* Getting Word or Unsigned Byte Address operand operations collection */
Value *WOrUBGetAddr(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(!BIT(25))
		return WOrUBGetAddrImm(cpu, instr, bb);
	else
		return WOrUBGetAddrReg(cpu, instr, bb);
}

/* Addr Mode 3, following arm operand doc */
/* Getting Miscellaneous Address Immidiate offset operand.in arm doc */
Value *MisGetAddrImmOffset(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{

	Value *Addr;
	Value *Offset_8;

	Offset_8 = CONST(IMMH << 4 | IMML);
	if(LSUBIT)
		Addr =  ADD(R(RN), Offset_8);
	else
		Addr =  SUB(R(RN), Offset_8);

	CHECK_REG15();
	return Addr;
}

/* Getting Miscellaneous Address register offset operand.in arm doc */
Value *MisGetAddrRegOffset(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr;

	if(LSUBIT)
		Addr =  ADD(R(RN), R(RM));
	else
		Addr =  SUB(R(RN), R(RM));

	CHECK_REG15();
	return Addr;
}

/* Getting Miscellaneous Address immdiate pre-indexed operand.in arm doc */
Value *MisGetAddrImmPre(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = MisGetAddrImmOffset(cpu, instr, bb);
	LET(RN, Addr);

	return Addr;
}

/* Getting Miscellaneous Address registers pre-indexed operand.in arm doc */
Value *MisGetAddrRegPre(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = MisGetAddrRegOffset(cpu, instr, bb);
	LET(RN, Addr);

	return Addr;
}

/* Getting Miscellaneous Address immdiate post-indexed operand.in arm doc */
Value *MisGetAddrImmPost(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = R(RN);
	LET(RN, MisGetAddrImmOffset(cpu, instr, bb));

	return Addr;
}

/* Getting Miscellaneous Address register post-indexed operand.in arm doc */
Value *MisGetAddrRegPost(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Value *Addr = R(RN);
	LET(RN, MisGetAddrRegOffset(cpu, instr, bb));

	return Addr;
}

/* Getting Miscellaneous Address immdiate operand operation collection. */
Value *MisGetAddrImm(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x0){
		if(BITS(21,22) == 0x2){
		/* Imm Post */
			return MisGetAddrImmPost(cpu, instr, bb);
		}
	}else if(BITS(24,27) == 0x1){
		if(BITS(21,22) == 0x2){
		/* Imm Offset */
			return MisGetAddrImmOffset(cpu, instr, bb);
		}else if(BITS(21,22) == 0x3){
		/* Imm pre */
			return MisGetAddrImmPre(cpu, instr, bb);
		}
	}
	printf(" Error in Mis Get Imm Addr \n");
}

/* Getting Miscellaneous Address register operand operation collection. */
Value *MisGetAddrReg(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x0){
		if(BITS(21,22) == 0x0){
		/* Reg Post */
			return MisGetAddrRegPost(cpu, instr, bb);
		}
	}else if(BITS(24,27) == 0x1){
		if(BITS(21,22) == 0x0){
		/* Reg offset */
			return MisGetAddrRegOffset(cpu, instr, bb);
		}else if(BITS(21,22) == 0x1){
		/* Reg pre */
			return MisGetAddrRegPre(cpu, instr, bb);
		}
	}
	printf(" Error in Mis Get Reg Addr \n");
}


/* Getting Miscellaneous Address operand operation collection. */
Value *MisGetAddr(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BIT(22))
		return MisGetAddrImm(cpu, instr, bb);
	else
		return MisGetAddrReg(cpu, instr, bb);
}

/* Addr Mode 4 */
/* Getting Load Store Multiple Address and Increment After operand */
Value *LSMGetAddrIA(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int i =  BITS(0,15);
	int count = 0;
	Value *Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

	Addr = R(RN);

	if(LSWBIT)
		LET(RN, ADD(R(RN), CONST(count * 4)));

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Increment Before operand */
Value *LSMGetAddrIB(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int i =  BITS(0,15);
	int count = 0;
	Value *Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

	Addr = ADD(R(RN), CONST(4));
	if(LSWBIT)
		LET(RN, ADD(R(RN), CONST(count * 4)));

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Decrement After operand. */
Value *LSMGetAddrDA(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int i =  BITS(0,15);
	int count = 0;
	Value *Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

	Addr = ADD(SUB(R(RN), CONST(count * 4)), CONST(4));
	if(LSWBIT)
		LET(RN, SUB(R(RN), CONST(count * 4)));

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Decrement Before operand. */
Value *LSMGetAddrDB(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	int i =  BITS(0,15);
	int count = 0;
	Value *Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

	Addr = SUB(R(RN), CONST(count * 4));
	if(LSWBIT)
		LET(RN, SUB(R(RN), CONST(count * 4)));

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address operand operation collection. */
Value *LSMGetAddr(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x8){
		if(BIT(23)){
		/* IA */
			return LSMGetAddrIA(cpu, instr, bb);
		}else{
		/* DA */
			return LSMGetAddrDA(cpu, instr, bb);
		}
	}else if(BITS(24,27) == 0x9){
		if(BIT(23)){
		/* IB */
			return LSMGetAddrIB(cpu, instr, bb);
		}else{
		/* DB */
			return LSMGetAddrDB(cpu, instr, bb);
		}
	}

	printf(" Error in LSM Get Imm Addr BITS(24,27) is 0x%x\n", BITS(24,27));
}

/* all,Getting Load Store Address operand operation collection */
Value *GetAddr(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BITS(24,27) == 0x1 || BITS(24,27) == 0x2 ){
		return MisGetAddr(cpu,instr,bb);
	}else if(BITS(24,27) == 0x4 || BITS(24,27) == 0x5 || BITS(24,27) == 0x6 || BITS(24,27) == 0x7 ){
		return WOrUBGetAddr(cpu,instr,bb);
	}else if(BITS(24,27) == 0x8 || BITS(24,27) == 0x9){
		return LSMGetAddr(cpu,instr,bb);
	}

	printf("Not a Load Store Addr operation \n");
	return CONST(0);
}

#define OPERAND_RETURN_CHECK_PC  do{  \
	if(RM == 15)		\
		return ADD(R(RM), CONST(8));	\
	else	\
		return R(RM);	\
}while(0)

#if 1
/* 0 */
Value *Data_ope_Reg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	if(!shift_imm){ /* Register */
		//return R(RM);
		OPERAND_RETURN_CHECK_PC;
	}else{	/* logic shift left by imm */
		return SHL(R(RM), CONST(shift_imm));
	}
}

/* Get date from instruction operand */
/* index:1 */
/* Getting data form Logic Shift Left register operand. following arm doc. */
Value *Data_ope_LogLReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	/* logic shift left by reg ICMP_ULE(shamt, CONST(32)) ?????? */
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), SHL(R(RM), shamt)));
}

/* index:2 */
/* Getting data form Logic Shift Right immdiate operand. following arm doc. */
Value *Data_ope_LogRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	/* logic shift right by imm */
	if(!shift_imm)
		return CONST(0);
	else
		return LSHR(R(RM), CONST(shift_imm));
}

/* index:3 */
/* Getting data form Logic Shift Right register operand. following arm doc. */
Value *Data_ope_LogRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	/* logic shift right by reg*/
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), LSHR(R(RM), shamt)));
}

/* index:4 */
/* Getting data form Shift Right immdiate operand. following arm doc. */
Value *Data_ope_AriRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	/* shift right by imm */
	if(!shift_imm)
		SELECT(LSHR(R(RM), CONST(31)), CONST(0xffffffff), CONST(0));
	else
		return ASHR(R(RM), CONST(shift_imm));
}

/* index:5 */
/* Getting data form Shift Right register operand. following arm doc. */
Value *Data_ope_AriRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	/* arth shift right by reg */
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM),
			SELECT(ICMP_ULT(shamt, CONST(32)), ASHR(R(RM), shamt),
				SELECT(LSHR(R(RM), CONST(31)), CONST(0xffffffff), CONST(0))));
}

/* index:6 */
/* Getting data form Rotate Shift Right immdiate operand. following arm doc. */
Value *Data_ope_RotRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	if(!shift_imm){
		/* Rotate right with extend */
		return ROTL(OR(SHL(ptr_C, CONST(31)), ASHR(R(RM), CONST(1))), CONST(1));
	}else{
		/* Rotate right by imm */
		return ROTL(R(RM), CONST(shift_imm));
	}
}

/* index:7 */
/* Getting data form Rotate Shift Right register operand. following arm doc. */
Value *Data_ope_RotRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt)
{
	Value *sham = AND(R(BITS(8, 11)), CONST(0xf));
	/* Rotate right by reg */
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_EQ(sham, CONST(0)), R(RM), ROTL(R(RM), sham)));
}

Value *(*Data_operand[8])(cpu_t*, uint32_t, BasicBlock *, uint32_t, Value*) = {Data_ope_Reg, Data_ope_LogLReg, Data_ope_LogRImm, Data_ope_LogRReg, Data_ope_AriRImm, Data_ope_AriRReg, Data_ope_RotRImm, Data_ope_RotRReg};

/* Getting data form operand collection. */
Value *operand(cpu_t *cpu,  uint32_t instr, BasicBlock *bb)
{
	uint32_t shift = BITS(4, 6);
	uint32_t shift_imm = BITS(7,11);
	Value *shamt = AND(R(BITS(8,11)), CONST(0xff));

	if(I){
		/* 32-bit immediate */
		uint32_t immed_8 = instr & 0xFF;
		int rotate_imm = ((instr >> 8) & 0xF) << 1;
		/*
		if(!rotate_imm)
			new StoreInst(ptr_C, shifter_carry_out, bb);
		else
			new StoreInst(AND(ASHR(CONST((immed_8 >> rotate_imm) | (immed_8 << (32 - rotate_imm))),
							CONST(31)), CONST(1)), shifter_carry_out, bb);
		*/
		return CONST((immed_8 >> rotate_imm) | (immed_8 << (32 - rotate_imm)));
	}
	else{
		/* operand with BIT 4 ~ 6 */
		return (Data_operand[shift])(cpu, instr, bb, shift_imm, shamt);
	}
}
#endif
#if 0
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
                                //return R(RM);
				OPERAND_RETURN_CHECK_PC;
                        } else {
				switch(shift){
				case 0: /* logic shift left by imm */
					if(!shift_imm)
						//return R(RM);
						OPERAND_RETURN_CHECK_PC;
					else
						return SHL(R(RM), CONST(shift_imm));
				case 1:	/* logi shift right by imm */
					if(!shift_imm)
						return CONST(0);
					else
						return LSHR(R(RM), CONST(shift_imm));
				case 2:
					if(!shift_imm)
						return SELECT(ICMP_ULE(R(RM), CONST(0x80000000)), CONST(0), LSHR(R(RM), CONST(31)));
					else
						return ASHR(R(RM), CONST(shift_imm));
				case 3:
					if(!shift_imm){
						BAD;
					}
					else
						return ROTL(R(RM), CONST(shift_imm));
                                //BAD;

				}
                        }
                } else {
                        if (!BIT(7)) { /* Register shifts */
				Value *shamt = AND(R(BITS(8,11)), CONST(0xff));
				switch(BITS(5,6)){
					case 0:  /* LSL */
						return SELECT( ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), SHL(R(RM), shamt)));
					case 1:  /* LSR */
						return SELECT( ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), LSHR(R(RM), shamt)));
					case 2:  /* ASR */
						return SELECT( ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), LSHR(R(RM), CONST(31)),LSHR(R(RM), shamt)));
					case 3: /* ROR */
						return SELECT( ICMP_EQ(shamt, CONST(0)), R(RM), ROTL(R(RM),SUB(CONST(32), shamt)));
				}
                        } else { /* arithmetic or Load/Store instruction extension space */
                                BAD;
                        }
                }
        }
}
#endif

/* Getting data from branch instruction operand */
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

#define CHECK_RN_PC  (RN==15? ADD(R(RN), CONST(8)):R(RN))
/* complete ADD logic use llvm asm */
void Dec_ADD(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x08 0x09 0x28 0x29 */
	//Value *op1 = R(RN);
	Value *op1 = CHECK_RN_PC;
	Value *op2 = OPERAND;
	Value *ret = ADD(op1, op2);
	LET(RD, ret);
	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);

}

/* complete ADC logic use llvm asm */
void Dec_ADC(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x0a 0x0b 0x1a 0x1b */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	//Value *ret = ADD(ADD(op1, op2), LOAD(ptr_C));
	Value *ret = SELECT(LOAD(ptr_C), ADD(ADD(op1, op2), CONST(1)), ADD(op1, op2));

	LET(RD, ret);
	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete AND logic use llvm asm */
void Dec_AND(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x00, 0x01, 0x20, 0x21 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = AND(op1,op2);
	LET(RD, ret);
	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete BIC logic use llvm asm */
void Dec_BIC(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x1c 0x2d */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = AND(op1,XOR(op2, CONST(0xffffffff)));
	LET(RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete CMN logic use llvm asm */
void Dec_CMN(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x17 0x37 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = ADD(op1, op2);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete CMP logic use llvm asm */
void Dec_CMP(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x15 0x35 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;

	Value *ret = SUB(op1, op2);

	set_condition(cpu, ret, bb, op1, op2);
}

/* complete EOR logic use llvm asm */
void Dec_EOR(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x02, 0x03, 0x22, 0x23 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = XOR(op1,op2);

	LET(RD,ret);
	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete MOV logic use llvm asm */
void Dec_MOV(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x10 0x11 0x30 0x31 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;

	LET(RD, op2);

	if(SBIT)
		set_condition(cpu, op2, bb, op1, op2);
}

/* complete MVN logic use llvm asm */
void Dec_MVN(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x1e 0x1f 0x3e 0x3f */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = XOR(op2, CONST(-1));
	LET(RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete MLA logic use llvm asm */
void Dec_MLA(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x00, 0x01*/
	Value *op1 = R(RM);
	//Value *op2 = OPERAND;
	Value *op2 = R(RS);
	Value *op3 = R(MUL_RN);
	Value *ret = ADD(MUL(op1,op2), op3);

	LET(MUL_RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, CONST(0), CONST(0));
}

/* complete MUL logic use llvm asm */
void Dec_MUL(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x00, 0x01*/
	Value *op1 = R(RM);
	//Value *op2 = OPERAND;
	Value *op2 = R(RS);
	Value *ret = MUL(op1,op2);

	LET(MUL_RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete ORR logic use llvm asm */
void Dec_ORR(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x18 0x19 0x38 0x39 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = OR(op2, op1);
	LET(RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete ORR logic use llvm asm */
void Dec_RSB(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x06 0x07 0x26 0x27 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op2, op1);
	LET(RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op2, op1);
}

/* complete SUB logic use llvm asm */
void Dec_SUB(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x04 0x05 0x24 0x25 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = SUB(op1, op2);
	LET(RD, ret);

	if(SBIT)
		set_condition(cpu, ret, bb, op1, op2);
}

/* complete TEQ logic use llvm asm */
void Dec_TEQ(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x13, 0x33 */
	Value *op1 = R(RN);
	Value *op2 = OPERAND;

	if(RN == 15)
		op1 = ADD(op1, CONST(8));
	Value *ret = XOR(op1,op2);

	set_condition(cpu, ret, bb, op1, op2);
}

/* complete SWI logic use llvm asm */
void Dec_SWI(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arch_syscall(cpu, bb, BITS(0,19));
}

/* complete TST logic use llvm asm */
void Dec_TST(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x11 0x31*/
	Value *op1 = R(RN);
	Value *op2 = OPERAND;
	Value *ret = AND(op1, op2);

	set_condition(cpu, ret, bb, op1, op2);
}

/* complete UMULL logic use llvm asm */
void Dec_UMULL(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* for 0x08 0x09 */
	Value *op1 = R(RS);
	Value *op2 = R(RM);

	/* We can split the 32x32 into four 16x16 operations. This
	   ensures that we do not lose precision on 32bit only hosts.  */
	Value *Lo = MUL(AND(op1, CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *mid1 = MUL(AND(op1, CONST(0xFFFF)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));
	Value *mid2 = MUL(AND(LSHR(op1, CONST(16)), CONST(0xFFFF)), AND(op2, CONST(0xFFFF)));
	Value *Hi = MUL(LSHR(op1, CONST(16)), AND(LSHR(op2, CONST(16)), CONST(0xFFFF)));

	/* We now need to add all of these results together, taking
	   care to propogate the carries from the additions.  */
	Value *and_op1 = Lo;
	Value *and_op2 = SHL(mid1, CONST(16));
	Value *tmp_RdLo = ADD(and_op1, and_op2);
	//Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), ICMP_NE(and_op2, CONST(0)), ICMP_ULT(tmp_RdLo, and_op1));
	Value *carry =  SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0)));

	and_op1 = tmp_RdLo;
	and_op2 = SHL(mid2, CONST(16));
	tmp_RdLo = ADD(and_op1, and_op2);
	carry =  ADD(carry, SELECT(ICMP_EQ(tmp_RdLo, and_op1), SELECT(ICMP_NE(and_op2, CONST(0)),CONST(1), CONST(0)), SELECT(ICMP_UGT(and_op1,tmp_RdLo), CONST(1), CONST(0))));

	Value *tmp_RdHi = ADD(ADD(LSHR(mid1, CONST(16)), LSHR(mid2, CONST(16))), Hi);
	tmp_RdHi = ADD(tmp_RdHi, carry);

	LET(RDLo, tmp_RdLo);
	LET(RDHi, tmp_RdHi);
}

/* complete the instruction operation which bits 20-27 is 00 */
int arm_opc_trans_00(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* AND Reg I = 0 S = 0*/

	if (BITS (4, 7) == 0xB) {
		/* STRH register offset, no write-back, down, post indexed.  */
		/* P = 0, U = 0, I = 0, W = 0 */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;
	}

	if (BITS (4, 7) == 0xD) {
		/* LDRD P = 0, U = 0, I = 0, W = 0 */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD P = 0, U = 0, I = 0, W = 0 */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;
	}
	if (BITS (4, 7) == 9) {
		/* MUL? */ /* ?S = 0 */
		Dec_MUL(cpu, instr, bb);

	}
	else {
		/* AND reg.  */
		Dec_AND(cpu, instr, bb);
	}

	return 0;

}

/* complete the instruction operation which bits 20-27 is 01 */
int arm_opc_trans_01(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ANDS reg and MULS */
	if ((BITS (4, 11) & 0xF9) == 0x9){
		/* LDR register offset, no write-back, down, post indexed.  */
		BAD;
		return 0;
	}

	if (BITS (4, 7) == 9) {
		/* MULS  S = 1? */
		Dec_MUL(cpu, instr, bb);
	}
	else {
		/* ANDS reg.  */
		Dec_AND(cpu, instr, bb);
	}
	return 0;
}

/* complete the instruction operation which bits 20-27 is 02 */
int arm_opc_trans_02(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EOR reg I = 0, S = 0 and MLA S = 0 */
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, down, post indexed. */
		/* P = 0; U = 0; I = 0; W =1 */
		BAD;
		return 0;
	}

	if (BITS (4, 7) == 9) {	/* MLA */
		Dec_MLA(cpu, instr, bb);
		return 0;
	}
	else {	/* EOR */
		Dec_EOR(cpu, instr, bb);
		return 0;
	}

}

/* complete the instruction operation which bits 20-27 is 03 */
int arm_opc_trans_03(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EORS reg I = 0, S = 1and MLAS S = 1 */
	if ((BITS (4, 11) & 0xF9) == 0x9) {
		/* LDR register offset, write-back, down, post-indexed.  */
		/* ????????????????????????????*/
		BAD;
	}

	if (BITS (4, 7) == 9) {
		/* MLAS */
		Dec_MLA(cpu, instr, bb);
		return 0;
	}
	else {
		/* EORS */
		Dec_EOR(cpu, instr, bb);
	}
	if(RD == 15){
		new StoreInst(SUB(R(14), CONST(4)), cpu->ptr_PC, bb);
	}
	LET(RD, OPERAND);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 04 */
int arm_opc_trans_04(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SUB reg  I = 0 S = 0*/
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, no write-back, down, post indexed.  */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* LDRD */
		/* P = 0 ,  U = 0, I = 1, W = 0 */
		BAD;
		return 0;
	}

	Dec_SUB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 05 */
int arm_opc_trans_05(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, no write-back, down, post inde     xed.  */
		BAD;
		return 0;
	}

	/* SUBS reg */
	Dec_SUB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 06 */
int arm_opc_trans_06(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) == 0xB){
		/* STRH immediate offset, write-back, down, post indexe     d.  */
		BAD;
		return 0;
	}

	/* RSB reg */
	Dec_RSB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 07 */
int arm_opc_trans_07(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, write-back, down, post indexed     .  */
		BAD;
		return 0;
	}

	/* RSBS reg */
	Dec_RSB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 08 */
int arm_opc_trans_08(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(BIT(4) != 1 || BIT(7) != 1)
	{
		Dec_ADD(cpu, instr, bb);
	}
	if(BITS(4, 7) == 0xB) {
		/* STRH reg offset, no write-back, up, post, indexed */
		/*P = 0, U = 1, I = 0, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 0,, U = 1, I = 0, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD */
		/*P = 0, U = 1, I = 0, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0x9) {
		/* MULL ?*/
		Dec_UMULL(cpu, instr, bb);
		return 0;
	}

	return 0;
}

/* complete the instruction operation which bits 20-27 is 09 */
int arm_opc_trans_09(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADDS reg I = 0, S = 0*/
	if (BITS (4, 7) == 0x9) {
		/* LDREX */
		BAD;
		return 0;
	}

	if (BITS (4, 7) == 0x9) {
		/* MULL ?*/
		Dec_UMULL(cpu, instr, bb);
		return 0;
	}

	Dec_ADD(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 0a */
int arm_opc_trans_0a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADC I=0, S=0 */
	if (BITS(4,7) == 0xB) {
		/* SHTR register offset, write-back, up, post-indexed */
		/* P = 0, U = 1, I = 0, W=1 */

		BAD;
		return 0;
	}
	if (BITS(4,7) == 0x9) {
		/* MULL ? */

		BAD;
		return 0;
	}

	Dec_ADC(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 0b */
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

	Dec_ADC(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 0c */
int arm_opc_trans_0c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) == 0x9)
	{
		/* STR immediate offset, no write-back, up post indexe     d.  */
		BAD;
		return 0;
	}

	/* SBC reg */
	BAD;
	return 0;

}

/* complete the instruction operation which bits 20-27 is 0d */
int arm_opc_trans_0d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, no write-back, up, post indexe     d.  */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0x9){
		 /* MULL  32x32=64 */
		BAD;
		return 0;
	}

}

/* complete the instruction operation which bits 20-27 is 0e */
int arm_opc_trans_0e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) == 0xB){
		/* STRH immediate offset, write-back, up, post indexed.       */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0x9){
		 /* MULL  32x32=64 */
		BAD;
		return 0;
	}

}

/* complete the instruction operation which bits 20-27 is 0f */
int arm_opc_trans_0f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, write-back, up, post indexed.       */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0x9){
		/* MULL  32x32=64 */
		BAD;
		return 0;
	}

	/* RSCS reg */
	BAD;
	return 0;
}


/* complete the instruction operation which bits 20-27 is 10 */
int arm_opc_trans_10(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* TST reg and MRS CPSR and SWP word.  */
	if (BITS (4, 7) & 0x9 == 0x9){
		/* 8 ~ 11 SBZ, IPUBWL 010000*/
		/* S H both is 0   SWP ???? */
		BAD;
		return 0;
	}
	if (IS_V5E) {
		if (BIT (4) == 0 && BIT (7) == 1) {
			/* ElSegundo SMLAxy insn.  */

			BAD;
			if (BITS (4, 11) == 5) {
				/* ElSegundo QADD insn.  */
				return 0;
			}
			if (BITS (4, 11) == 9){
				/* SWP */
				return 0;
			}
			else if ((BITS (0, 11) == 0) && (LHSReg == 15)) {
				/* MRS CPSR */
				return 0;
			}
			else {
				/* UNDEF_Test */
				return 0;
			}

		}
		BAD;
		return 0;
	}

	BAD;
	return 0;

}

/* complete the instruction operation which bits 20-27 is 11 */
int arm_opc_trans_11(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 11) & 0xF9) == 0x9){
	/* LDR register offset, no write-back, down, pre indexed.  */
		BAD;
		return 0;
	}
	if (DESTReg == 15) {
		/* TSTP reg */
		BAD;
	}
	else{
		/* TST reg */
		Dec_TST(cpu, instr, bb);
		return 0;
	}

	return 0;

}

/* complete the instruction operation which bits 20-27 is 12 */
int arm_opc_trans_12(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) == 3) {
	/* v5 BLX(2) */
		BAD;
		return 0;
	}

	if (BITS (4, 11) == 5) {
		/* ElSegundo QSUB insn.  */
		BAD;
		return 0;
	}
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, down, pre indexed.  */
		/* p = 1, U = 0, I = 0, W = 1 */
		BAD;
		return 0;
	}
	if (BITS (4, 27) == 0x12FFF1) {
		/* BX */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD P = 1, U = 0, I = 0, W = 1, 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* LDRD P = 1, U = 0, I = 0, W = 1, 0 */
		BAD;
		return 0;
	}
				//if (state->is_v5) {
	if (BITS (4, 7) == 0x7) {
		/* BKPT Force the next instruction to be refetched.  */
		BAD;
		return 0;
	}
	//}

	/* ~MODET */
	return 0;
}

/* complete the instruction operation which bits 20-27 is 13 */
int arm_opc_trans_13(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR register offset, write-back, down, pre indexed.  */
		/* bit 8 ~ 11 SBZ IPUBWL 010011*/
		BAD;
		return 0;
	}
	if (DESTReg == 15) {
		/* TEQP reg */
		BAD;
	}
	else {
		/* TEQ Reg.  */
		Dec_TEQ(cpu, instr, bb);
	}

	return 0;
}

/* complete the instruction operation which bits 20-27 is 14 */
int arm_opc_trans_14(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (IS_V5E){
		if (BIT (4) == 0 && BIT (7) == 1) {
			/* ElSegundo SMLALxy insn.  */
			BAD;
			return 0;
		}
		if (BITS (4, 11) == 5) {
			/* ElSegundo QDADD insn.  */
			BAD;
			return 0;
		}
	}

	if ((BITS (4, 7) & 0x9) == 0x9){
		/* STR immediate offset, no write-back, down, pre indexed.  */
		/* IPUBWL 010100*/
		BAD;
		return 0;
	}


	if (BITS (4, 11) == 9) {
		/* SWP */
		BAD;
		return 0;
	}
	else if ((BITS (0, 11) == 0)
			&& (LHSReg == 15)) {
		/* MRS SPSR */
		return 0;
	}
	else{
		/* UNDEF */
		BAD;
	}
}
#define CLEARC 0
#define CLEARV 0
#define CLEARN 0
/* complete the instruction operation which bits 20-27 is 15 */
int arm_opc_trans_15(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, no write-back, down, pre indexed.  */
		/* IPUBWL 010101*/
		BAD;
		return 0;
	}


	/* Other instruction*/
	if (DESTReg == 15) {
		/* CMPP reg.  */
		BAD;
		return 0;
	}
	else
	{
		/* CMP reg I = 0 */
		Dec_CMP(cpu, instr, bb);
	}

	return 0;
}

/* complete the instruction operation which bits 20-27 is 16 */
int arm_opc_trans_16(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	//if (state->is_v5e) {
	if (BIT (4) == 0 && BIT (7) == 1
	    && BITS (12, 15) == 0) {
		/* ElSegundo SMULxy insn.  */
		BAD;
		return 0;
	}

	if (BITS (4, 11) == 5) {
		/* ElSegundo QDSUB insn.  */
		BAD;
		return 0;
	}

	if (BITS (4, 11) == 0xF1
	    && BITS (16, 19) == 0xF) {
		/* ARM5 CLZ insn.  */
		BAD;
		return 0;
	}
				//}
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, write-back, down, pre indexed.  */
		/* P = 1, U = 0, I = 0, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/*LDRD P = 1, U = 0, I = 0, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/*STRD P = 1, U = 0, I = 0, W = 0 */
		BAD;
		return 0;
	}
	return 0;
}

/* complete the instruction operation which bits 20-27 is 17 */
int arm_opc_trans_17(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMN reg I = 0*/
	if ((BITS (4, 7) & 0x9) == 0x9){
		/* LDR immediate offset, write-back, down, pre indexed.  */
		/*  ? */
		BAD;
		return 0;
	}
	if (DESTReg == 15){

	}
	else{
		/* CMN reg.  */
		Dec_CMN(cpu, instr, bb);
	}

	/* CMNP reg */

	return 0;
}

/* complete the instruction operation which bits 20-27 is 18 */
int arm_opc_trans_18(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
#if 0
	if (IS_V6) {
		if (BITS (4, 7) == 0x9)
			/* P = 1 U = 0 B = 1 W = 0 */
			BAD;
			return 0;
	}
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, no write-back, up, pre indexed.  */
		BAD;
		return 0;
	}

	if(BITS(4, 7) & 0x9 == 0x9){	/* STR */
		/* P = 1 U = 0 B = 1 W = 0 */
		BAD;
		return 0;
	}
#endif
	/* ORR reg */
	Dec_ORR(cpu, instr, bb);
	return 0;

}

/* complete the instruction operation which bits 20-27 is 19 */
int arm_opc_trans_19(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* dyf add armv6 instr ldrex */
	if (IS_V6) {
		if (BITS (4, 7) == 0x9) {
			BAD;
			return 0;
		}
	}

	if ((BITS (4, 11) & 0xF9) == 0x9){
		/* LDR register offset, no write-back, up, pre indexed.  */
		BAD;
		return 0;
	}

	/* ORRS reg */
	Dec_ORR(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 1a */
int arm_opc_trans_1a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* mov reg  I = 0 S = 0*/
	if (BITS (4, 11) == 0xB) {
		/* STRH register offset, write-back, up, pre indexed.  */
		/* p = 1, U = 1, I = 0, W = 1 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* p = 1, U = 1, I = 0, W = 1 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xF) {
		/* STRD */
		/* p = 1, U = 1, I = 0, W = 1 */
		BAD;
		return 0;
	}
//	LET(RD, OPERAND);
	Dec_MOV(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 1b */
int arm_opc_trans_1b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* movs reg  I = 0 S = 1*/
	if ((BITS (4, 11) & 0xF9) == 0x9){
		/* LDR register offset, write-back, up, pre indexed.  */
		BAD;
		return 0;
	}

	/* Continue with remaining instruction decoding.  */
	/* MOVS reg */
	Dec_MOV(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 1c */
int arm_opc_trans_1c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC reg  I = 0, S = 0*/

	if (BITS (4, 7) == 0x9) {
		/* STREXB  v6*/
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xB) {
		/* STRH immediate offset, no write-back, up, pre indexed.  */
		/* P = 1, U = 1, I = 1, W = 0 */
		BAD;
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRD */
		/* P = 1, U = 1, I = 1, W = 0 */
		BAD;
		return 0;
	}
	else if (BITS (4, 7) == 0xF) {
		/* STRD */
		/* P = 1, jjjjjjjkkkkkkkkkkkkkkkkkkkkkkkkkkU = 1, I = 1, W = 0 */
		BAD;
		return 0;
	}
	/* BIC reg */

	Dec_BIC(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 1d */
int arm_opc_trans_1d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BICS reg  I = 0, S = 1 */

	if (BITS(4, 7) == 0xF) {
		/* LDRSH P=1 U=1 W=0 */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;

	}
	if (BITS (4, 7) == 0xb) {
		/* LDRH immediate offset, no write-back, up, pre indexed.  */
		/* P = 1, U = 1, I = 1, W = 0 */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;
	}
	if (BITS (4, 7) == 0xD) {
		/* LDRSB immediate offset, no write-back, up, pre indexed.  */
		/* P = 1, U = 1, I = 1, W = 0 */
		return 0;
	}
	/* Continue with instruction decoding.  */
	if ((BITS (4, 7)) == 0x9) {
		/* ldrexb */
		/* LDR immediate offset, no write-back, up, pre indexed.  */
		Value *addr = GetAddr(cpu, instr, bb);
		LoadStore(cpu,instr,bb,addr);
		return 0;
	}

	/* BICS reg */
	Dec_BIC(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 1e */
int arm_opc_trans_1e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) & 0x9 == 0x9) {
		/* STR immediate offset, write-back, up, pre indexed.  */
		/* PUBWL 11110*/
		BAD;
		return 0;
	}

	/* MVN reg */
	Dec_MVN(cpu, instr, bb);
	return 0;

}

/* complete the instruction operation which bits 20-27 is 1f */
int arm_opc_trans_1f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (BITS (4, 7) & 0x9 == 0x9) {
		/* LDR immediate offset, write-back, up, pre indexed.  */
		/* PUBWL 11110*/
		BAD;
		return 0;
	}
	BAD;

	/* MVNS reg */
	return 0;
}

/* complete the instruction operation which bits 20-27 is 20 */
int arm_opc_trans_20(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* AND immed I = 1, S = 0 */
	Dec_AND(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 21 */
int arm_opc_trans_21(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ANDS immed I = 1, S = 1 */
	Dec_AND(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 22 */
int arm_opc_trans_22(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EOR immed I = 1, S = 0 */
	Dec_EOR(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 23 */
int arm_opc_trans_23(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* EORS immed  I = 0, S = 1*/
	Dec_EOR(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 24 */
int arm_opc_trans_24(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SUB immed I = 0, S = 0 */
	Dec_SUB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 25 */
int arm_opc_trans_25(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SUBS immed S = 1 */
	Dec_SUB(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 26 */
int arm_opc_trans_26(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* RSB immed S = 0*/
	Dec_RSB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 27 */
int arm_opc_trans_27(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* RSBS immed */
	Dec_RSB(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 28 */
int arm_opc_trans_28(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADD immed  I = 1 S = 0*/
	Dec_ADD(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 29 */
int arm_opc_trans_29(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADDS immed  I = 1 S = 1*/
	Dec_ADD(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 2a */
int arm_opc_trans_2a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADC immed  I = 1 S = 0 */
	Dec_ADC(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 2b */
int arm_opc_trans_2b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ADCS immed  I = 1 S = 1 */
	BAD;

	return 0;
}

/* complete the instruction operation which bits 20-27 is 2c */
int arm_opc_trans_2c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SBC immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 2d */
int arm_opc_trans_2d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* SBCS immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 2e */
int arm_opc_trans_2e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* RSC immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 2f */
int arm_opc_trans_2f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* RSCS immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 30 */
int arm_opc_trans_30(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* TST immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 31 */
int arm_opc_trans_31(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* TSTP immed */
	if (DESTReg == 15){
		/* TSTP immed.  */
		BAD;
		return 0;
	}
	else{
		/* TST immed.  */
		Dec_TST(cpu, instr, bb);
	}

	return 0;

}

#define CPSR 16
/* complete the instruction operation which bits 20-27 is 32 */
int arm_opc_trans_32(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* TEQ immed and MSR immed to CPSR */
    /* MSR immed to CPSR. R = 0(set CPSR) */
	if (DESTReg == 15){
		/* MSR immed to CPSR.  */
		return 0;
	}
	else{
		/* UNDEF*/
	}

	BAD;
	return 0;

}

/* complete the instruction operation which bits 20-27 is 33 */
int arm_opc_trans_33(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (DESTReg == 15) {
		/* TEQP immed.  */
		return 0;
	}
	else{
		/* TEQ immed */
		Dec_TEQ(cpu, instr, bb);
		return 0;
	}

	return 0;
}

/* complete the instruction operation which bits 20-27 is 34 */
int arm_opc_trans_34(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMP immed */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 35 */
int arm_opc_trans_35(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (DESTReg == 15) {
		/* CMPP immed I = 1, */
	}
	else{
		/* CMP immed.  */
		Dec_CMP(cpu, instr, bb);

	}
	return 0;
}

/* complete the instruction operation which bits 20-27 is 36 */
int arm_opc_trans_36(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CMN immed and MSR immed to SPSR */
	/* MSR R = 1 set SPSR*/
	if (DESTReg == 15){
		return 0;
	}
	else{
		/*UNDEF*/
	}
	BAD;

	return 0;
}

/* complete the instruction operation which bits 20-27 is 37 */
int arm_opc_trans_37(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if (DESTReg == 15){
		/* CMNP immed.  */
		BAD;
		return 0;
	}
	else{
		/* CMN immed.  */
		Dec_CMN(cpu, instr, bb);
	}
	return 0;
}

/* complete the instruction operation which bits 20-27 is 38 */
int arm_opc_trans_38(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ORR immed.  */
	Dec_ORR(cpu, instr, bb);

}

/* complete the instruction operation which bits 20-27 is 39 */
int arm_opc_trans_39(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* ORRS immed.  */
	Dec_ORR(cpu, instr, bb);

}

/* complete the instruction operation which bits 20-27 is 3a */
int arm_opc_trans_3a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MOV immed. I = 1, S = 0 */
	if(RD == 15){
		new StoreInst(SUB(R(14), CONST(4)), cpu->ptr_PC, bb);
		return 0;
	}
//	LET(RD, OPERAND);
	Dec_MOV(cpu, instr, bb);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 3b */
int arm_opc_trans_3b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MOV immed. I = 1, S = 1 */
	BAD;
	return 0;
}

/* complete the instruction operation which bits 20-27 is 3c */
int arm_opc_trans_3c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC immed I = 1, S = 0 */
	Dec_BIC(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 3d */
int arm_opc_trans_3d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* BIC immed I = 1, S = 0 */
	BAD;
	return 0;
}

/* complete the instruction operation which bits 20-27 is 3e */
int arm_opc_trans_3e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MVN immed */
	/* I = 1, S = 0 */
	Dec_MVN(cpu, instr, bb);

}

/* complete the instruction operation which bits 20-27 is 3f */
int arm_opc_trans_3f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MVNS immed.  */
	Dec_MVN(cpu, instr, bb);

}

/* complete the instruction operation which bits 20-27 is 40 */
int arm_opc_trans_40(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 0, W = 0 */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 41 */
int arm_opc_trans_41(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 0, W = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;

}

/* complete the instruction operation which bits 20-27 is 42 */
int arm_opc_trans_42(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 0, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 43 */
int arm_opc_trans_43(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 0, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 44 */
int arm_opc_trans_44(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 1, W = 0*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 45 */
int arm_opc_trans_45(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 1, W = 0*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 46 */
int arm_opc_trans_46(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 1, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 47 */
int arm_opc_trans_47(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, WriteBack, Post Dec, Immed.  */
	/* I = 0, P = 0, U = 0, B = 1, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 48 */
int arm_opc_trans_48(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 0, W = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 49 */
int arm_opc_trans_49(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 0, W = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 4a */
int arm_opc_trans_4a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 0, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 4b */
int arm_opc_trans_4b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 0, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 4c */
int arm_opc_trans_4c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 1, W = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 4d */
int arm_opc_trans_4d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 1, W = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 4e */
int arm_opc_trans_4e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 1, W = 1*/
	BAD;

}

/* complete the instruction operation which bits 20-27 is 4f */
int arm_opc_trans_4f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, WriteBack, Post Inc, Immed.  */
	/* I = 0, P = 0, U = 1, B = 1, W = 1*/
	BAD;
}

/* complete the instruction operation which bits 20-27 is 50 */
int arm_opc_trans_50(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Pre Dec, Immed.  */
	/* I = 0, P = 1, U = 0, B = 0, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

}

/* complete the instruction operation which bits 20-27 is 51 */
int arm_opc_trans_51(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*LDR No WriteBack, Pre Inc, Regist - Immed. */
	/*I = 0, P = 1, U = 0, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 52 */
int arm_opc_trans_52(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, "Pre Inc", Regist - Immed. */
	/* I = 0, P = 1, U = 0, B = 0, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	//Value *addr = WOrUBGetAddrImmPre(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	//StoreDWord(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 53 */
int arm_opc_trans_53(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*LDR , WriteBack, Pre Inc,  Regist - Immed */
	/*I = 0, P = 1, U = 0, B = 0, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 54 */
int arm_opc_trans_54(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*STRB , No WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 0, B = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 55 */
int arm_opc_trans_55(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*LDRB, No WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 0, B = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 56 */
int arm_opc_trans_56(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*STRB, No WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 0, B = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;

}

/* complete the instruction operation which bits 20-27 is 57 */
int arm_opc_trans_57(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*LDRB, WriteBack, Pre Inc, Regist - Immed */
	/*I = 0, P = 1, U = 0, W = 1, B = 1 */
	BAD;


}

/* complete the instruction operation which bits 20-27 is 58 */
int arm_opc_trans_58(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/*STR, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/* I = 0, P = 1, U = 1, W = 0, B = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	//StoreWord(cpu, instr, bb, addr);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 59 */
int arm_opc_trans_59(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load , No WriteBack, Pre Inc, Regist + Immed.|| Regist  */
	/* I = 0, P = 1, U = 1, W = 0 , B = 0*/
	Value *addr = GetAddr(cpu, instr, bb);
	//LoadWord(cpu, instr, bb, addr);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 5a */
int arm_opc_trans_5a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* STR, WriteBack, Pre Inc, Regist + Immed || Regist */
	/*  I = 0, P = 1, U = 1, B = 0, W = 1 */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 5b */
int arm_opc_trans_5b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDR, WriteBack, Pre Inc, Regist + Immed.|| Regist */
	/*  I = 0, P = 1, U = 1, B = 0, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 5c */
int arm_opc_trans_5c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* STRB, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/*  I = 0, P = 1, U = 1, B = 1, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 5d */
int arm_opc_trans_5d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDRB, No WriteBack, Pre Inc, Regist + Immed || Regist */
	/* I = 0, P = 1, U = 1, B = 1, W = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 5e */
int arm_opc_trans_5e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* STRB, WriteBack, Pre Inc, Regist + Immed || Regist */
	/* I = 0, P = 1, U = 1, B = 1, W = 1 */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 5f */
int arm_opc_trans_5f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDRB, WriteBack, Pre Inc, Immed. */
	/* I = 0, P = 1, U = 1, B = 1, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 60 */
int arm_opc_trans_60(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	 /* Store Word, No WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 61 */
int arm_opc_trans_61(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 62 */
int arm_opc_trans_62(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 63 */
int arm_opc_trans_63(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 64 */
int arm_opc_trans_64(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Post Dec, Reg.  */
	BAD;
	return 0;
}

/* complete the instruction operation which bits 20-27 is 65 */
int arm_opc_trans_65(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 66 */
int arm_opc_trans_66(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 67 */
int arm_opc_trans_67(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, WriteBack, Post Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 68 */
int arm_opc_trans_68(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 69 */
int arm_opc_trans_69(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6a */
int arm_opc_trans_6a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6b */
int arm_opc_trans_6b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6c */
int arm_opc_trans_6c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6d */
int arm_opc_trans_6d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6e */
int arm_opc_trans_6e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 6f */
int arm_opc_trans_6f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, WriteBack, Post Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 70 */
int arm_opc_trans_70(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 71 */
int arm_opc_trans_71(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 72 */
int arm_opc_trans_72(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Pre Dec, Reg.  */
	BAD;


}

/* complete the instruction operation which bits 20-27 is 73 */
int arm_opc_trans_73(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 74 */
int arm_opc_trans_74(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 75 */
int arm_opc_trans_75(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 76 */
int arm_opc_trans_76(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 77 */
int arm_opc_trans_77(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, WriteBack, Pre Dec, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 78 */
int arm_opc_trans_78(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, No WriteBack, Pre Inc, Reg.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 79 */
int arm_opc_trans_79(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, No WriteBack, Pre Inc, Reg.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 7a */
int arm_opc_trans_7a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Word, WriteBack, Pre Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 7b */
int arm_opc_trans_7b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Word, WriteBack, Pre Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 7c */
int arm_opc_trans_7c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, No WriteBack, Pre Inc, Reg.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 7d */
int arm_opc_trans_7d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load Byte, No WriteBack, Pre Inc, Reg. */
	/* P = 1 , U = 1, W = 0 */
	if(BIT(4)){
		/* UNDEF INSTR */
	}

	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is 7e */
int arm_opc_trans_7e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store Byte, WriteBack, Pre Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 7f */
int arm_opc_trans_7f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	 /* Load Byte, WriteBack, Pre Inc, Reg.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 80 */
int arm_opc_trans_80(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, No WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 81 */
int arm_opc_trans_81(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, No WriteBack, Post Dec.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

}

/* complete the instruction operation which bits 20-27 is 82 */
int arm_opc_trans_82(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 83 */
int arm_opc_trans_83(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 84 */
int arm_opc_trans_84(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, No WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 85 */
int arm_opc_trans_85(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, No WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 86 */
int arm_opc_trans_86(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 87 */
int arm_opc_trans_87(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, WriteBack, Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 88 */
int arm_opc_trans_88(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, No WriteBack, Post Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 89 */
int arm_opc_trans_89(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, No WriteBack, Post Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 8a */
int arm_opc_trans_8a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, WriteBack, Post Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

	return 0;
}

/* complete the instruction operation which bits 20-27 is 8b */
int arm_opc_trans_8b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, WriteBack, Post Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 8c */
int arm_opc_trans_8c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, No WriteBack, Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 8d */
int arm_opc_trans_8d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, No WriteBack, Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 8e */
int arm_opc_trans_8e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, WriteBack, Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 8f */
int arm_opc_trans_8f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	 /* Load, Flags, WriteBack, Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 90 */
int arm_opc_trans_90(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, No WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 91 */
int arm_opc_trans_91(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, No WriteBack, Pre Dec.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 92 */
int arm_opc_trans_92(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, WriteBack, PreDec */
	/* STM(1) P = 1, U = 0, W = 1 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
}

/* complete the instruction operation which bits 20-27 is 93 */
int arm_opc_trans_93(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 94 */
int arm_opc_trans_94(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, No WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 95 */
int arm_opc_trans_95(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, No WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 96 */
int arm_opc_trans_96(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 97 */
int arm_opc_trans_97(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, WriteBack, Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 98 */
int arm_opc_trans_98(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, No WriteBack, Pre Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

}

/* complete the instruction operation which bits 20-27 is 99 */
int arm_opc_trans_99(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, No WriteBack, Pre Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);

}

/* complete the instruction operation which bits 20-27 is 9a */
int arm_opc_trans_9a(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 9b */
int arm_opc_trans_9b(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 9c */
int arm_opc_trans_9c(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, No WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 9d */
int arm_opc_trans_9d(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, No WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 9e */
int arm_opc_trans_9e(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store, Flags, WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is 9f */
int arm_opc_trans_9f(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Load, Flags, WriteBack, Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is a0 */
int arm_opc_trans_a0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* 0xa0 - 0xa7 branch postive addr */
//	LET(14, ADD(R(15), CONST(4)));
	LET(15, ADD(ADD(R(15), CONST(8)),BOPERAND));
}

/* complete the instruction operation which bits 20-27 is a1 */
int arm_opc_trans_a1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a2 */
int arm_opc_trans_a2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a3 */
int arm_opc_trans_a3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a4 */
int arm_opc_trans_a4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a5 */
int arm_opc_trans_a5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a6 */
int arm_opc_trans_a6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a7 */
int arm_opc_trans_a7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is a8 */
int arm_opc_trans_a8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* 0xa8 - 0xaf negative addr */
//	LET(14, ADD(R(15), CONST(4)));
	LET(15, SUB(ADD(R(15), CONST(8)),BOPERAND));
}

/* complete the instruction operation which bits 20-27 is a9 */
int arm_opc_trans_a9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is aa */
int arm_opc_trans_aa(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ab */
int arm_opc_trans_ab(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ac */
int arm_opc_trans_ac(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ad */
int arm_opc_trans_ad(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ae */
int arm_opc_trans_ae(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is af */
int arm_opc_trans_af(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_a8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b0 */
int arm_opc_trans_b0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* b0 - b7 branch and link forward */
	LET(14, ADD(R(15),CONST(4)));
	LET(15, ADD(ADD(R(15),BOPERAND), CONST(8)));
}

/* complete the instruction operation which bits 20-27 is b1 */
int arm_opc_trans_b1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b2 */
int arm_opc_trans_b2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b3 */
int arm_opc_trans_b3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b4 */
int arm_opc_trans_b4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b5 */
int arm_opc_trans_b5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b6 */
int arm_opc_trans_b6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b7 */
int arm_opc_trans_b7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is b8 */
int arm_opc_trans_b8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* b8 - bf branch and link backward */
	LET(14, ADD(R(15),CONST(4)));
	LET(15, ADD(R(15),BOPERAND));
}

/* complete the instruction operation which bits 20-27 is b9 */
int arm_opc_trans_b9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ba */
int arm_opc_trans_ba(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is bb */
int arm_opc_trans_bb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is bc */
int arm_opc_trans_bc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is bd */
int arm_opc_trans_bd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is be */
int arm_opc_trans_be(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is bf */
int arm_opc_trans_bf(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_b8(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is c0 */
int arm_opc_trans_c0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
}

/* complete the instruction operation which bits 20-27 is c1 */
int arm_opc_trans_c1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 0, W  = 0 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is c2 */
int arm_opc_trans_c2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{


}

/* complete the instruction operation which bits 20-27 is c3 */
int arm_opc_trans_c3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 0, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is c4 */
int arm_opc_trans_c4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	if(IS_V5){
		/* Reading from R15 is UNPREDICTABLE.  */
		if (BITS (12, 15) == 15
				|| BITS (16, 19) == 15){
			return 0;
		}
		else{
			return 0;
		}
	}

	return 0;
}

/* complete the instruction operation which bits 20-27 is c5 */
int arm_opc_trans_c5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 1, W  = 0 */
	/* undef Instr */
	return 0;
}

/* complete the instruction operation which bits 20-27 is c6 */
int arm_opc_trans_c6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Post Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is c7 */
int arm_opc_trans_c7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Dec.  */
	/* P = 0, U = 0, N = 1, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is c8 */
int arm_opc_trans_c8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is c9 */
int arm_opc_trans_c9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Inc.  */
	/* P = 0, U = 1, N = 0, W  = 0 */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is ca */
int arm_opc_trans_ca(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Post Inc.  */
	Value *addr = GetAddr(cpu, instr, bb);
	LoadStore(cpu,instr,bb,addr);
	return 0;
}

/* complete the instruction operation which bits 20-27 is cb */
int arm_opc_trans_cb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Inc.   */
	/* P = 0, U = 1, N = 0, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is cc */
int arm_opc_trans_cc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Post Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is cd */
int arm_opc_trans_cd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Post Inc.   */
	/* P = 0, U = 1, N = 1, W  = 0 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is ce */
int arm_opc_trans_ce(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Post Inc.  */
	BAD;
}

/* complete the instruction operation which bits 20-27 is cf */
int arm_opc_trans_cf(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , No WriteBack , Post Dec.  */
	/* P = 0, U = 1, N = 1, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is d0 */
int arm_opc_trans_d0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is d1 */
int arm_opc_trans_d1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Dec.  */
	/* P = 1, U = 0, N = 0, W  = 0 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is d2 */
int arm_opc_trans_d2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is d3 */
int arm_opc_trans_d3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Pre Dec.    */
	/* P = 1, U = 0, N = 0, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is d4 */
int arm_opc_trans_d4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is d5 */
int arm_opc_trans_d5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Dec.  */
	/* P = 1, U = 0, N = 1, W  = 0 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is d6 */
int arm_opc_trans_d6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Pre Dec.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is d7 */
int arm_opc_trans_d7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , WriteBack , Pre Dec.    */
	/* P = 1, U = 0, N = 1, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is d8 */
int arm_opc_trans_d8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is d9 */
int arm_opc_trans_d9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 0, W  = 0 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is da */
int arm_opc_trans_da(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is db */
int arm_opc_trans_db(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 0, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is dc */
int arm_opc_trans_dc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , No WriteBack , Pre Inc.  */
	BAD;

}

/* complete the instruction operation which bits 20-27 is dd */
int arm_opc_trans_dd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , No WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 1, W  = 0 */
	BAD;
	return 0;
}

/* complete the instruction operation which bits 20-27 is de */
int arm_opc_trans_de(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* Store , WriteBack , Pre Inc.  */
	BAD;
}

/* complete the instruction operation which bits 20-27 is df */
int arm_opc_trans_df(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* LDC Load , Load , WriteBack , Pre Inc.  */
	/* P = 1, U = 1, N = 1, W  = 1 */
	return 0;
}

/* complete the instruction operation which bits 20-27 is e0 */
int arm_opc_trans_e0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* MCR e0,e2,e4,e6,e8,ea,ec,ee */

}

/* complete the instruction operation which bits 20-27 is e1 */
int arm_opc_trans_e1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	/* CDP e1, e3, e5, e7, e9, eb, ed, ef,  Co-Processor Register Transfers (MRC) and Data Ops. */
	LET(RD,CONST(0));	//tmp use
}

/* complete the instruction operation which bits 20-27 is e2 */
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

/* complete the instruction operation which bits 20-27 is e3 */
int arm_opc_trans_e3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is e4 */
int arm_opc_trans_e4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is e5 */
int arm_opc_trans_e5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is e6 */
int arm_opc_trans_e6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is e7 */
int arm_opc_trans_e7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is e8 */
int arm_opc_trans_e8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is e9 */
int arm_opc_trans_e9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ea */
int arm_opc_trans_ea(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is eb */
int arm_opc_trans_eb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ec */
int arm_opc_trans_ec(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
	return 0;
}

/* complete the instruction operation which bits 20-27 is ed */
int arm_opc_trans_ed(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ee */
int arm_opc_trans_ee(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ef */
int arm_opc_trans_ef(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_e1(cpu, instr, bb);
}

/* Software interrupt*/
/* complete the instruction operation which bits 20-27 is f0 */
int arm_opc_trans_f0(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	Dec_SWI(cpu,instr,bb);
}

/* complete the instruction operation which bits 20-27 is f1 */
int arm_opc_trans_f1(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f2 */
int arm_opc_trans_f2(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f3 */
int arm_opc_trans_f3(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f4 */
int arm_opc_trans_f4(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f5 */
int arm_opc_trans_f5(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f6 */
int arm_opc_trans_f6(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f7 */
int arm_opc_trans_f7(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f8 */
int arm_opc_trans_f8(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is f9 */
int arm_opc_trans_f9(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is fa */
int arm_opc_trans_fa(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is fb */
int arm_opc_trans_fb(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is fc */
int arm_opc_trans_fc(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is fd */
int arm_opc_trans_fd(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is fe */
int arm_opc_trans_fe(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}

/* complete the instruction operation which bits 20-27 is ff */
int arm_opc_trans_ff(cpu_t *cpu, uint32_t instr, BasicBlock *bb)
{
	arm_opc_trans_f0(cpu, instr, bb);
}
