#include "llvm/Instructions.h"
#include "arm_regformat.h"
#include "skyeye_dyncom.h"
#include "dyncom/dyncom_llvm.h"
#include "dyncom/frontend.h"
#include "arm_internal.h"
#include "arm_types.h"
#include "dyncom/tag.h"

#include "arm_dyncom_dec.h"
#include "arm_dyncom_translate.h"

using namespace llvm;
#if 0
#define BITS(a,b) ((instr >> (a)) & ((1 << (1+(b)-(a)))-1))
#define BIT(n) ((instr >> (n)) & 1)
#define BAD	do{printf("meet BAD at %s, instr is %x\n", __FUNCTION__, instr ); /*exit(0);*/}while(0);
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
#endif
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
		LoadByte(cpu, instr, bb, addr); // alex-ykl fix 2011-07-26 : was loading a signed byte
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
		{
			if (LSLBIT)
				LoadSByte(cpu,instr,bb,addr);
			else
				LoadDWord(cpu,instr,bb,addr);
		}
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
	if ((LSLBIT) || ((LSLBIT==0) && (LSSHBITS==0x2)))
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
	if (BITS(25, 27) == 4 && BIT(22) && BIT(20) && !BIT(15)) {
		/* LDM (2) user */
		for (i = 0; i < 13; i++) {
			if(BIT(i)){
				ret = arch_read_memory(cpu, bb, Addr, 0, 32);
				LET(i, ret);
				Addr = ADD(Addr, CONST(4));
			}
		}
		if (BIT(13)) {
			ret = arch_read_memory(cpu, bb, Addr, 0, 32);
			LET(R13_USR, ret);
			Addr = ADD(Addr, CONST(4));
		}
		if (BIT(14)) {
			ret = arch_read_memory(cpu, bb, Addr, 0, 32);
			LET(R14_USR, ret);
			Addr = ADD(Addr, CONST(4));
		}
		return;
	}
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
	if (BITS(25, 27) == 4 && BITS(20, 22) == 4) {
		for (i = 0; i < 13; i++) {
			if(BIT(i)){
				arch_write_memory(cpu, bb, Addr, R(i), 32);
				Addr = ADD(Addr, CONST(4));
			}
		}
		if (BIT(13)) {
			arch_write_memory(cpu, bb, Addr, R(R13_USR), 32);
			Addr = ADD(Addr, CONST(4));
		}
		if (BIT(14)) {
			arch_write_memory(cpu, bb, Addr, R(R14_USR), 32);
			Addr = ADD(Addr, CONST(4));
		}
		if(BIT(15)){
			arch_write_memory(cpu, bb, Addr, STOREM_CHECK_PC, 32);
		}
		return;
	}
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
	if (BITS(20, 27) == 0x19 && BITS(0, 11) == 0xf9f) {
		/* LDREX */
		LoadWord(cpu, instr, bb, addr);
		return;
	}
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
	/* N */ new StoreInst(ICMP_SLT(ret, CONST(0)), ptr_N, bb);
	/* Z */ new StoreInst(ICMP_EQ(ret, CONST(0)), ptr_Z, bb);
	/* C */ new StoreInst(ICMP_ULT(ret, op1), ptr_C, false, bb);
	/* V */ new StoreInst(ICMP_SLT(AND((XOR(op1, op2)), XOR(op1,ret)), CONST(0)), ptr_V, bb);
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
		if(!BIT(21) || BIT(21)){
			return WOrUBGetAddrImmPost(cpu, instr, bb);
		}
	}
	printf(" Error in WOrUB Get Imm Addr instr is %x \n", instr);
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
	} else if (BITS(24, 27) == 0x5 && BIT(21) == 0) {
		return WOrUBGetAddrImmOffset(cpu, instr, bb);
	}
	printf(" Error in WOrUB Get Reg Addr inst is %x\n", instr);
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
	printf(" (DEC) Error in Mis Get Reg Addr %x\n", instr);
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
	if(BITS(24,27) == 0x1 || BITS(24,27) == 0x2 || BITS(24, 27) == 0){
		return MisGetAddr(cpu,instr,bb);
	}else if(BITS(24,27) == 0x4 || BITS(24,27) == 0x5 || BITS(24,27) == 0x6 || BITS(24,27) == 0x7 ){
		return WOrUBGetAddr(cpu,instr,bb);
	}else if(BITS(24,27) == 0x8 || BITS(24,27) == 0x9){
		return LSMGetAddr(cpu,instr,bb);
	}

	printf("Not a Load Store Addr operation %x\n", instr);
	return CONST(0);
}

#define OPERAND_RETURN_CHECK_PC  do{  \
	if(RM == 15)		\
		return ADD(R(RM), CONST(8));	\
	else	\
		return R(RM);	\
}while(0)

#if 1
/* index:0 */
/* register immediate */
Value *Data_ope_Reg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	if (!shift_imm) { /* Register */
		OPERAND_RETURN_CHECK_PC;
		/* No changes in SCO */
	} else {	/* logic shift left by imm */
		if (sco != NULL)
		{
			new StoreInst(ICMP_SLT(SHL(R(RM), CONST(shift_imm-1)), CONST(0)),sco,bb);
		}
		return SHL(R(RM), CONST(shift_imm));
	}
}

/* Get date from instruction operand */
/* index:1 */
/* Getting data form Logic Shift Left register operand. following arm doc. */
Value *Data_ope_LogLReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	if (sco != NULL)
	{
		Value *flag = SELECT(ICMP_EQ(shamt, CONST(0)), new LoadInst(ptr_C, "", false, bb), /* Rs[7:0] == 0 */
				     SELECT(ICMP_ULT(shamt, CONST(32)), ICMP_SLT(SHL(R(RM), SUB(shamt, CONST(1))), CONST(0)), /* Rs[7:0] < 32 */
					    SELECT(ICMP_EQ(shamt, CONST(32)), TRUNC1(R(RM)), /* Rs[7:0] == 32*/
						   CONST1(0) /* Rs[7:0] > 32 */
						   )
					    )
				     );
		new StoreInst(flag, sco, bb);
	}
	/* logic shift left by reg ICMP_ULE(shamt, CONST(32)) ?????? */
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), SHL(R(RM), shamt)));
}

/* index:2 */
/* Getting data form Logic Shift Right immdiate operand. following arm doc. */
Value *Data_ope_LogRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	/* logic shift right by imm */
	if(!shift_imm) {
		if (sco != NULL)
			new StoreInst(ICMP_SLT(R(RM), CONST(0)), sco, bb);
 		return CONST(0);
	} else {
		if (sco != NULL)
			new StoreInst(ICMP_SLT(SHL(R(RM), CONST(32 - shift_imm)), CONST(0)), sco, bb);
 		return LSHR(R(RM), CONST(shift_imm));
	}
}

/* index:3 */
/* Getting data form Logic Shift Right register operand. following arm doc. */
Value *Data_ope_LogRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	if (sco != NULL)
	{
		Value *flag = SELECT(ICMP_EQ(shamt, CONST(0)), new LoadInst(ptr_C, "", false, bb), /* Rs[7:0] == 0 */
				     SELECT(ICMP_ULT(shamt, CONST(32)), ICMP_SLT(SHL(R(RM), SUB(CONST(32), shamt)), CONST(0)), /* Rs[7:0] < 32 */
					    SELECT(ICMP_EQ(shamt, CONST(32)), ICMP_SLT(shamt, CONST(0)), /* Rs[7:0] == 32*/
						   CONST1(0) /* Rs[7:0] > 32 */
						   )
					    )
				     );
		new StoreInst(flag, sco, bb);
	}
	/* logic shift right by reg*/
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_UGE(shamt, CONST(32)), CONST(0), LSHR(R(RM), shamt)));
}

/* index:4 */
/* Getting data form Shift Right immdiate operand. following arm doc. */
Value *Data_ope_AriRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
 	/* shift right by imm */
	if(!shift_imm) {
		if (sco != NULL)
			new StoreInst(ICMP_SLT(R(RM), CONST(0)), sco, bb);
		return SELECT(LSHR(R(RM), CONST(31)), CONST(0xffffffff), CONST(0));
	} else {
		if (sco != NULL)
			new StoreInst(ICMP_SLT(SHL(R(RM), CONST(32-shift_imm)), CONST(0)), sco, bb);
 		return ASHR(R(RM), CONST(shift_imm));
	}
}

/* index:5 */
/* Getting data form Shift Right register operand. following arm doc. */
Value *Data_ope_AriRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	/* arth shift right by reg */
	if (sco != NULL)
	{
		Value *flag = SELECT(ICMP_EQ(shamt, CONST(0)), new LoadInst(ptr_C, "", false, bb), /* Rs[7:0] == 0 */
				     SELECT(ICMP_ULT(shamt, CONST(32)), ICMP_SLT(SHL(R(RM), SUB(CONST(32), shamt)), CONST(0)), /* Rs[7:0] < 32 */
					    ICMP_SLT(R(RM), CONST(0)) /* Rs[7:0] <= 32 */
					    )
				     );
		new StoreInst(flag, sco, bb);
	}
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM),
			SELECT(ICMP_ULT(shamt, CONST(32)), ASHR(R(RM), shamt),
				SELECT(ICMP_EQ(LSHR(R(RM), CONST(31)), CONST(0x80000000)), CONST(0xffffffff), CONST(0))));
}

/* index:6 */
/* Getting data form Rotate Shift Right immdiate operand. following arm doc. */
Value *Data_ope_RotRImm(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
 	if(!shift_imm){
 		/* Rotate right with extend */
		Value *ret = ROTL(OR(SHL(ptr_C, CONST(31)), ASHR(R(RM), CONST(1))), CONST(1));
		if (sco != NULL)
			new StoreInst(TRUNC1(R(RM)), sco, bb); /* Beware, ptr_C nust be modified after */
		return ret;
	} else {
		if (sco != NULL)
			new StoreInst(ICMP_SLT(SHL(R(RM), CONST(32 - shift_imm)), CONST(0)), sco, bb);
 		/* Rotate right by imm */
 		return ROTL(R(RM), CONST(32 - shift_imm));
	}
}

/* index:7 */
/* Getting data form Rotate Shift Right register operand. following arm doc. */
Value *Data_ope_RotRReg(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, uint32_t shift_imm, Value *shamt, Value *sco)
{
	Value *sham = AND(R(BITS(8, 11)), CONST(0x1f));
	/* Rotate right by reg */
	if (sco != NULL) {
		Value *flag = SELECT(ICMP_EQ(shamt, CONST(0)), new LoadInst(ptr_C, "", false, bb), /* Rs[7:0] == 0 */
				     SELECT(ICMP_EQ(sham, CONST(0)), ICMP_SLT(R(RM), CONST(0)), /* Rs[4:0] == 0 */
					    ICMP_SLT(SHL(R(RM), SUB(CONST(32), sham)), CONST(0)) /* Rs[4:0] > 0 */
					    )
				     );
		new StoreInst(flag, sco, bb);
	}
	return SELECT(ICMP_EQ(shamt, CONST(0)), R(RM), SELECT(ICMP_EQ(sham, CONST(0)), R(RM), ROTL(R(RM), SUB(CONST(32), sham))));
}

Value *(*Data_operand[8])(cpu_t*, uint32_t, BasicBlock *, uint32_t, Value*, Value*) = {Data_ope_Reg, Data_ope_LogLReg, Data_ope_LogRImm, Data_ope_LogRReg, Data_ope_AriRImm, Data_ope_AriRReg, Data_ope_RotRImm, Data_ope_RotRReg};

/* Getting data form operand collection. */
Value *operand(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, Value *sco)
{
	uint32_t shift = BITS(4, 6);
	uint32_t shift_imm = BITS(7,11);
	Value *shamt = AND(R(BITS(8,11)), CONST(0xff));

	if(I) {
		/* 32-bit immediate */
		uint32_t immed_8 = instr & 0xFF;
		int rotate_immx2 = (instr & 0xF00) >> 7; //((instr >> 8) & 0xF) << 1;
		uint32_t immediate = (immed_8 >> (rotate_immx2)) | (immed_8 << (32 - rotate_immx2));
			
		if (sco != NULL)
		{
			if(rotate_immx2) {
				new StoreInst(CONST1(immediate >> 31),sco, bb);
			}
			/* No changes in C flag else */
		}
		
		return CONST(immediate);
	} else if (BITS(4, 11) == 0x6 && BITS(25, 27) == 0) {
		/*  Rotate right with extend  */
		Value *rm = LSHR(R(RM), CONST(1));
		Value *tmp = SELECT(ICMP_EQ(LOAD(ptr_C), CONST1(0)), CONST(0), CONST(0x80000000));
		return OR(rm, tmp);
	} else {
		/* operand with BIT 4 ~ 6 */
		return (Data_operand[shift])(cpu, instr, bb, shift_imm, shamt, sco);
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
uint32_t boperand(uint32_t instr)
{
	#if 1
               uint32_t rotate_imm = instr;
               if(instr &  0x800000)
                       rotate_imm = (~rotate_imm + 1) & 0x0ffffff;
#else
		uint32_t rotate_imm = instr & 0xffffff;
		if(rotate_imm &  0x800000) {
			rotate_imm |= 0xff000000;
			//rotate_imm = (~rotate_imm + 1) & 0x3fffffff;
			//rotate_imm &= 0x3fffffff;
		}
#endif
		else
			rotate_imm &= 0x0ffffff;

		rotate_imm = rotate_imm << 2;

//		printf("rotate_imm is %x\n", rotate_imm);
		return rotate_imm;
}
#if 0
#define OPERAND operand(cpu,instr,bb)
#define BOPERAND boperand(cpu,instr,bb)

#define CHECK_RN_PC  (RN==15? ADD(R(RN), CONST(8)):R(RN))
#endif


const ISEITEM arm_instruction[] = {
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
	{"uxth",        2,      ARMV6,          16, 27, 0x6ff,  4, 7, 7},
	{"uxtah",       2,      ARMV6,          20, 27, 0x6f,   4, 7, 7},
	{"rev",         2,      ARMV6,          20, 27, 0x6b,   4, 7, 3},
	{"rev16",       2,      ARMV6,          16, 27, 0x6bf,  4, 11, 0xfb},
	{"revsh",       2,      ARMV6,          20, 27, 0x6f,   4, 7, 0xb},
	{"ldrbt",       3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 7},
	{"ldrt",        3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 3},
        {"ldr",         3,      ARMALL,         26, 27, 1,      22, 22, 0,      20, 20, 1},
        {"pld",         4,      ARMV5TE,        26, 31, 0x3d,   24, 24, 1,      20, 22, 5,      12, 15, 0xf},
        {"ldrb",        3,      ARMALL,         26, 27, 1,      22, 22, 1,      20, 20, 1},
        {"ldrd",        3,      ARMV5TE,        25, 27, 0,      20, 20, 0,      4, 7, 0xd},
        {"ldrex",       2,      ARMALL,         20, 27, 0x19,   4, 7, 9},
	{"ldrexb",	2,	ARM1176JZF_S,	20, 27, 0x1d,	4, 7, 9},
        {"ldrh",        3,      ARMALL,         25, 27, 0,      20, 20, 1,      4, 7, 0xb},
        {"ldrsb",       3,      ARMV4T,         25, 27, 0,      20, 20, 1,      4, 7, 0xd},
        {"ldrsh",       3,      ARMV4T,         25, 27, 0,      20, 20, 1,      4, 7, 0xf},
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
	{"uxtb",        2,      ARMV6,          16, 27, 0x6ef,  4, 7, 7},
	{"uxtab",       2,      ARMV6,          20, 27, 0x6e,   4, 9, 7},
        {"strbt",       3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 6},
        {"strt",        3,      ARMALL,         26, 27, 1,      24, 24, 0,      20, 22, 2},
        {"str",         3,      ARMALL,         26, 27, 1,      22, 22, 0,      20, 20, 0},
        {"strb",        3,      ARMALL,         26, 27, 1,      22, 22, 1,      20, 20, 0},
        {"strd",        3,      ARMV5TE,        25, 27, 0,      20, 20, 0,      4, 7, 0xf},//ARMv5TE and above, excluding ARMv5TExP
        {"strex",       2,      ARMV6,          20, 27, 0x18,   4, 7, 9},
	{"strexb",	2,	ARM1176JZF_S,	20, 27, 0x1c,	4, 7, 9},
        {"strh",        3,      ARMALL,         25, 27, 0,      20, 20, 0,      4, 7, 0xb},
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


const ISEITEM arm_exclusion_code[] = {
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
	{"uxth",        0,      ARMV6, 0},
	{"uxtah",       0,      ARMV6, 0},         
	{"rev",         0,      ARMV6, 0},        
	{"rev16",       0,      ARMV6, 0}, 
	{"revsh",       0,      ARMV6, 0},         
	{"ldrbt",       0,      ARMALL, 0},        
	{"ldrt",        0,      ARMALL, 0},        
        {"ldr",         0,      ARMALL, 0},        
        {"pld",         0,      ARMV5TE, 0},       
        {"ldrb",        0,      ARMALL, 0},        
        {"ldrd",        0,      ARMV5TE, 0},       
        {"ldrex",       0,      ARMALL, 0},        
	{"ldrexb",	0,	ARM1176JZF_S, 0},	
        {"ldrh",        0,      ARMALL, 0},        
        {"ldrsb",       0,      ARMV4T, 0},        
        {"ldrsh",       0,      ARMV4T, 0},        
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
	{"uxtb",        0,      ARMV6, 0},         
	{"uxtab",       0,      ARMV6, 0},         
        {"strbt",       0,      ARMALL, 0},        
        {"strt",        0,      ARMALL, 0},        
        {"str",         0,      ARMALL, 0},        
        {"strb",        0,      ARMALL, 0},        
        {"strd",        0,      ARMV5TE, 0},       
        {"strex",       0,      ARMV6, 0},         
	{"strexb",	0,	ARM1176JZF_S, 0},	
        {"strh",        0,      ARMALL, 0},        
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



int decode_arm_instr(uint32_t instr, int32_t *idx)
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
			} else if (BITS(arm_instruction[i].content[base], arm_instruction[i].content[base + 1]) != arm_instruction[i].content[base + 2]) {
				break;
			}
			base += 3;
			n --;
		}
		//All conditions is satisfied.
		if (n == 0)
			ret = DECODE_SUCCESS;

		if (ret == DECODE_SUCCESS) {
			n = arm_exclusion_code[i].attribute_value;
			if (n != 0) {
				base = 0;
				while (n) {
					if (BITS(arm_exclusion_code[i].content[base], arm_exclusion_code[i].content[base + 1]) != arm_exclusion_code[i].content[base + 2]) {
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

