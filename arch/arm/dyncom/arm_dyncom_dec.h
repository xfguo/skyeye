#ifndef __ARM_DYNCOM_DEC__
#define __ARM_DYNCOM_DEC__

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

/* CP15 registers */
#define OPCODE_1        BITS(21, 23)
#define CRn             BITS(16, 19)
#define CRm             BITS(0, 3)
#define OPCODE_2        BITS(5, 7)

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

#define OPERAND operand(cpu,instr,bb,NULL)
#define SCO_OPERAND(sco) operand(cpu,instr,bb,sco)
#define BOPERAND boperand(instr)

#define CHECK_RN_PC  (RN==15? ADD(R(RN), CONST(8)):R(RN))

Value *operand(cpu_t *cpu,  uint32_t instr, BasicBlock *bb, Value *sco);
uint32_t boperand(uint32_t instr);
int set_condition(cpu_t *cpu, Value *ret, BasicBlock *bb, Value *op1, Value *op2);
Value *GetAddr(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
void LoadStore(cpu_t *cpu, uint32_t instr, BasicBlock *bb, Value *addr);

int decode_arm_instr(uint32_t instr, int32_t *idx);

enum DECODE_STATUS {
	DECODE_SUCCESS,
	DECODE_FAILURE
};

struct instruction_set_encoding_item {
        const char *name;
        int attribute_value;
        int version;
        int content[12];//12 is the max number
};

typedef struct instruction_set_encoding_item ISEITEM;

enum ARMVER {
	INVALID = 0,
        ARMALL,
        ARMV4,
        ARMV4T,
        ARMV5T,
        ARMV5TE,
        ARMV5TEJ,
        ARMV6,
	ARM1176JZF_S
};

//extern const INSTRACT arm_instruction_action[];
extern const ISEITEM arm_instruction[];

int arm_opc_trans_00(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_01(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_02(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_03(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_04(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_05(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_06(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_07(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_08(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_09(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_0f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_10(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_11(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_12(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_13(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_14(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_15(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_16(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_17(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_18(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_19(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_1f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_20(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_21(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_22(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_23(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_24(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_25(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_26(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_27(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_28(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_29(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_2f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_30(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_31(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_32(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_33(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_34(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_35(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_36(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_37(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_38(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_39(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_3f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_40(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_41(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_42(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_43(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_44(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_45(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_46(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_47(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_48(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_49(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_4f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_50(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_51(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_52(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_53(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_54(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_55(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_56(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_57(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_58(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_59(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_5f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_60(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_61(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_62(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_63(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_64(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_65(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_66(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_67(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_68(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_69(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_6f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_70(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_71(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_72(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_73(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_74(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_75(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_76(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_77(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_78(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_79(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_7f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_80(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_81(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_82(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_83(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_84(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_85(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_86(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_87(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_88(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_89(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_8f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_90(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_91(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_92(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_93(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_94(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_95(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_96(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_97(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_98(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_99(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9a(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9b(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9c(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9d(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9e(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_9f(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_a0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_a9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_aa(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ab(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ac(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ad(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ae(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_af(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_b0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_b9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ba(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_bb(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_bc(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_bd(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_be(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_bf(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_c0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_c9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ca(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_cb(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_cc(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_cd(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ce(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_cf(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_d0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_d9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_da(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_db(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_dc(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_dd(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_de(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_df(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_e0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_e9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ea(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_eb(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ec(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ed(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ee(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ef(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

int arm_opc_trans_f0(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f1(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f2(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f3(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f4(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f5(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f6(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f7(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f8(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_f9(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_fa(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_fb(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_fc(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_fd(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_fe(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int arm_opc_trans_ff(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
#endif
