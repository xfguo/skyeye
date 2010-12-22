#ifndef __ARM_REGFORMAT_H__
#define __ARM_REGFORMAT_H__

enum arm_regno{
	R0 = 0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	LR,
	R15, //PC,
	CPSR_REG,
	MAX_REG_NUM,
};
const char* arm_regstr[] = {
	"R0",
	"R1",
	"R2",
	"R3",
	"R4",
	"R5",
	"R6",
	"R7",
	"R8",
	"R9",
	"R10",
	"R11",
	"R12",
	"R13",
	"LR",
	"PC",
	"CPSR",
	NULL
};

#endif
