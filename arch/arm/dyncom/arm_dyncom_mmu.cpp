#include "arm_dyncom_mmu.h"
#include "skyeye_dyncom.h"
#include <skyeye_log.h>
#include "skyeye_obj.h"
#include "arm_dyncom_dec.h"
#include "dyncom/tag.h"
static bool_t is_inside_page(cpu_t *cpu, addr_t a)
{
//	return ((a & 0xfffff000) == cpu->current_page_phys) ? True : False;
	return True;
}
static bool_t is_page_start(addr_t a)
{
	return ((a & 0x00000fff) == 0x0) ? True : False;
}

static bool_t is_page_end(addr_t a)
{
	return ((a & 0x00000fff) == 0xffc) ? True : False;
}

int
dyncom_check_perms (arm_core_t *core, int ap, int read)
{
	int s, r, user;

	s = CP15REG(CP15_CONTROL) & CONTROL_SYSTEM;
	r = CP15REG(CP15_CONTROL) & CONTROL_ROM;
	/* chy 2006-02-15 , should consider system mode, don't conside 26bit mode */
	user = (core->Mode == USER32MODE) || (core->Mode == USER26MODE) || (core->Mode == SYSTEM32MODE);
//	printf("mode is %x\n", core->Mode);
//	printf("ap is %x, user is %x, s is %x, read is %x\n", ap, user, s, read);
	switch (ap) {
	case 0:
		return read && ((s && !user) || r);
	case 1:
		return !user;
	case 2:
		return read || !user;
	case 3:
		return 1;
	}
	return 0;
}

fault_t
dyncom_mmu_translate (arm_core_t *core, ARMword virt_addr, ARMword *phys_addr, int *ap, int *sop)
{
	{
		/* walk the translation tables */
		ARMword l1addr, l1desc;
		if (CP15REG(CP15_TRANSLATION_BASE_CONTROL) && virt_addr << CP15REG(CP15_TRANSLATION_BASE_CONTROL) >> (32 - CP15REG(CP15_TRANSLATION_BASE_CONTROL) - 1)) {
			l1addr = CP15REG(CP15_TRANSLATION_BASE_TABLE_1);
			l1addr = (((l1addr >> 14) << 14) | (virt_addr >> 18)) & ~3;
		} else {
			l1addr = CP15REG(CP15_TRANSLATION_BASE_TABLE_0);
			l1addr = (((l1addr >> (14 - CP15REG(CP15_TRANSLATION_BASE_CONTROL))) << (14 - CP15REG(CP15_TRANSLATION_BASE_CONTROL))) | (virt_addr << CP15REG(CP15_TRANSLATION_BASE_CONTROL)) >> (18 + CP15REG(CP15_TRANSLATION_BASE_CONTROL))) & ~3;
		}

		/* l1desc = mem_read_word (state, l1addr); */
		bus_read(32, l1addr, &l1desc);
#if 0
		if (virt_addr == 0xbe8f9f10) {
			printf("virt_addr is %x\n", virt_addr);
			printf("l1addr is %x l1desc is %x\n", l1addr, l1desc);
			printf("mmu_control is %x\n", CP15REG(CP15_TRANSLATION_BASE_CONTROL));
			printf("mmu_table_0 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_0));
			printf("mmu_table_1 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_1));
//			bus_read(32, 0x50007004, &l1desc);
			printf("l1desc is %x\n", l1desc);
		}
#endif
		switch (l1desc & 3) {
		case 0:
		case 3:
			/*
			 * according to Figure 3-9 Sequence for checking faults in arm manual,
			 * section translation fault should be returned here.
			 */
			{
				#if 0
				printf("virt_addr is %x\n", virt_addr);
				printf("l1addr is %x l1desc is %x\n", l1addr, l1desc);
				printf("mmu_control is %x\n", CP15REG(CP15_TRANSLATION_BASE_CONTROL));
				printf("mmu_table_0 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_0));
				printf("mmu_table_1 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_1));
				bus_read(32, 0x50007004, &l1desc);
				printf("l1desc is %x\n", l1desc);
				#endif
				return SECTION_TRANSLATION_FAULT;
			}
		case 1:
			/* coarse page table */
			{
				ARMword l2addr, l2desc;


				l2addr = l1desc & 0xFFFFFC00;
				l2addr = (l2addr |
					  ((virt_addr & 0x000FF000) >> 10)) &
					~3;

				bus_read(32, l2addr, &l2desc);
				/* chy 2003-09-02 for xscale */
				*ap = (l2desc >> 4) & 0x3;
				*sop = 1;	/* page */
				switch (l2desc & 3) {
				case 0:
					#if 0
					printf("virt_addr is %x\n", virt_addr);
					printf("l1addr is %x l1desc is %x\n", l1addr, l1desc);
					printf("mmu_control is %x\n", CP15REG(CP15_TRANSLATION_BASE_CONTROL));
					printf("mmu_table_0 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_0));
					printf("mmu_table_1 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_1));
					printf("l2addr is %x l2desc is %x\n", l2addr, l2desc);
					#endif
					return PAGE_TRANSLATION_FAULT;
					break;
				case 1:
					*phys_addr = (l2desc & 0xFFFF0000) | (virt_addr & 0x0000FFFF);
					break;
				case 2:
				case 3:
					*phys_addr = (l2desc & 0xFFFFF000) | (virt_addr & 0x00000FFF);
					break;

				}
			}
			break;
		case 2:
			/* section */

			*ap = (l1desc >> 10) & 3;
			*sop = 0; 	/* section */
			if (l1desc & 0x30000)
				*phys_addr = (l1desc & 0xFF000000) | (virt_addr & 0x00FFFFFF);
			else
				*phys_addr = (l1desc & 0xFFF00000) | (virt_addr & 0x000FFFFF);
			break;
		}
#if 0
		if (*ap == 1) {
			printf("virt_addr is %x\n", virt_addr);
			printf("l1addr is %x l1desc is %x\n", l1addr, l1desc);
			printf("mmu_control is %x\n", CP15REG(CP15_TRANSLATION_BASE_CONTROL));
			printf("mmu_table_0 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_0));
			printf("mmu_table_1 is %x\n", CP15REG(CP15_TRANSLATION_BASE_TABLE_1));
		}
#endif
	}
	return NO_FAULT;
}


fault_t get_phys_addr(cpu_t *cpu, addr_t virt_addr, addr_t *phys_addr, uint32_t size, uint32_t rw)
{
	ARMword pa, real_va, temp, offset;
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	ARMword perm;		/* physical addr access permissions */
	fault_t fault = NO_FAULT;
	int ap, sop;
	#if MMU_DEBUG
	printf("enter %s\n", __FUNCTION__);
	#endif
	if ((CP15REG(CP15_CONTROL) & 1) == 0) {
		/* MMU or MPU disabled. */
		*phys_addr = virt_addr;
//		return NO_FAULT;
	} else {
		fault = dyncom_mmu_translate(core, virt_addr, phys_addr, &ap, &sop);
		#if MMU_DEBUG
		printf("virt_addr is %x phys_addr is %x\n", virt_addr, *phys_addr);
		#endif
		if (fault) {
			printf("fault\n");
			printf("icounter is %lld\n", cpu->icounter);
			return fault;
		}
		/* no tlb, only check permission */
		if (!dyncom_check_perms(core, ap, rw)) {
			if (virt_addr == 0xbe8f9f10) {
				printf("ap is %d, sop is %d\n", ap, sop);
			}
			if (sop == 0) {
				return SECTION_PERMISSION_FAULT;
			} else {
				return SUBPAGE_PERMISSION_FAULT;
			}
		}
	}
	if (size == 8) {
		*phys_addr = (*phys_addr) | (virt_addr & 3);
	} else if (size == 16) {
		*phys_addr = (*phys_addr) | (virt_addr & 3);
	}
	#if MMU_DEBUG
	printf("exit %s\n", __FUNCTION__);
	#endif
	return fault;
}


static uint32_t arch_arm_read_memory(cpu_t *cpu, addr_t virt_addr, uint32_t size)
{
	uint32_t value;
	addr_t phys_addr = 0;
	fault_t fault = NO_FAULT;
	fault = get_phys_addr(cpu, virt_addr, &phys_addr, size, 1);
	if (NO_FAULT != fault) {
		printf("mmu read fault %d\n", fault);
		phys_addr = virt_addr;
		exit(-1);
	}
	#if MMU_DEBUG
	printf("bus read at %x\n", phys_addr);
	#endif
	if (size == 8) {
		bus_read(8, phys_addr | (virt_addr & 3), &value);
	} else if (size == 16) {
		bus_read(16, phys_addr | (virt_addr & 3), &value);
	} else {
		bus_read(32, phys_addr, &value);
	}
#if 0
	if (cpu->icounter > 170960)
	{
		printf("pc is %x virt_addr is %x bus read at %x value is %x size is %d\n", ((arm_core_t *)cpu->cpu_data)->Reg[15], virt_addr, phys_addr, value, size);
	}
#endif
#if SYNC_WITH_INTERPRET
	if (cpu->icounter == 4149571 || cpu->icounter == 5683966 || cpu->icounter == 5781965 || cpu->icounter == 7598893
	    || cpu->icounter == 7844811 || cpu->icounter == 8012434 || cpu->icounter == 12160365 || cpu->icounter == 19370686 || cpu->icounter == 19511535
	    || cpu->icounter == 56078607 || cpu->icounter == 56572308 || cpu->icounter == 59931102 || cpu->icounter == 61497190 || cpu->icounter == 61766672
	    || cpu->icounter == 191073541 || cpu->icounter == 191384481 || cpu->icounter == 196265895 || cpu->icounter == 202478273 || cpu->icounter == 202878010
	    || cpu->icounter == 203029447 || cpu->icounter == 206010172 || cpu->icounter == 206140964 || cpu->icounter == 236624036 || cpu->icounter == 236755045
	    || cpu->icounter == 236883917 || cpu->icounter == 237014284 || cpu->icounter == 237143288 || cpu->icounter == 237274828 || cpu->icounter == 237408580
	    || cpu->icounter == 248090679 || cpu->icounter == 248081379) {
		if (phys_addr == 0x71200000) {
			return 0;
		}
	}
	if (cpu->icounter == 20406671) {
		if (phys_addr == 0x71200008) {
			return 0;
		}
	}
	if ((cpu->icounter == 20363304 && phys_addr == 0x7f006040) ||
	    (cpu->icounter == 20363311 && phys_addr == 0x7f006040)) {
		return 0x2b3;
	}
	if ((cpu->icounter == 20384989 && phys_addr == 0x7f006040)) {
		return 0x1de;
	}
	if ((cpu->icounter == 20406670 && phys_addr == 0x7f006040)) {
		return 0x10a;
	}
	if (cpu->icounter == 5705371 || cpu->icounter == 5832564) {
		if (phys_addr == 0x71200000) {
			return 0x10000000;
		}
	}
#endif
	uint32_t dummy;

#if 0
	if (cpu->icounter > 248306790) {
		printf("pc is %x virt_addr is %x bus read at %x value is %x size is %d\n", ((arm_core_t *)cpu->cpu_data)->Reg[15], virt_addr, phys_addr, value, size);
	}
#endif
	//bus_read(size, virt_addr, &value);
	return value;
}
#define LOG_IN_CLR	skyeye_printf_in_color
static void arch_arm_write_memory(cpu_t *cpu, addr_t virt_addr, uint32_t value, uint32_t size)
{
	addr_t phys_addr = 0;
	fault_t fault = NO_FAULT;
	fault = get_phys_addr(cpu, virt_addr, &phys_addr, size, 0);
//	if (phys_addr >= 0x71200000 && phys_addr <= 0x71200003) {
#if 0
	if (cpu->icounter > 248319240) {
		LOG_IN_CLR(LIGHT_GREEN, "WRITE virt_addr is %x phys_addr is %x value is %x\n", virt_addr, phys_addr, value);
		LOG_IN_CLR(LIGHT_GREEN, "icounter is %x\n", cpu->icounter);
	}
#endif
	if (NO_FAULT != fault) {
		printf("mmu write fault %d\n", fault);
		exit(-1);
	}
	#if MMU_DEBUG
	printf("bus write at %x\n", phys_addr);
	#endif
#if 0
	if (phys_addr == 0x50c27fe4 && cpu->icounter > 248306791) {
		LOG_IN_CLR(RED, "pc is %x bus write at %x value is %x size is %d\n", ((arm_core_t *)cpu->cpu_data)->Reg[15], phys_addr, value, size);
		exit(-1);
	}
#endif
	//bus_write(size, phys_addr, value);
	if (size == 8) {
		bus_write(8, phys_addr | (virt_addr & 3), value);
	} else if (size == 16) {
		bus_write(16, phys_addr | (virt_addr & 3), value);
	} else {
		bus_write(32, phys_addr, value);
	}
	if (is_translated_code(cpu, phys_addr)) {
		//clear native code when code section was written.
		addr_t addr = find_bb_start(cpu, phys_addr);
#if L3_HASHMAP
		clear_cache_item(cpu->dyncom_engine->fmap, addr);
#else
		fprintf(stderr, "Warnning: not clear the cache");
#endif
	}
	//bus_write(size, virt_addr, value);
}

#define CHECK_REG15() \
	do{ \
		if(RN == 15) \
			Addr = Addr + 8; \
	}while(0)

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
static addr_t WOrUBGetAddrImmOffset(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr;
	if(LSUBIT)
//		Addr =  ADD(R(RN), CONST(OFFSET12));
		Addr = core->Reg[RN] + OFFSET12;
	else
//		Addr =  SUB(R(RN), CONST(OFFSET12));
		Addr = core->Reg[RN] - OFFSET12;

//	printf("R%d : %08x\n", RN, core->Reg[RN]);
	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address register offset operand.in arm doc */
static addr_t WOrUBGetAddrRegOffset(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr;
	if(LSUBIT)
//		Addr =  ADD(R(RN), R(RM));
		Addr = core->Reg[RN] + core->Reg[RM];
	else
//		Addr =  SUB(R(RN), R(RM));
		Addr = core->Reg[RN] - core->Reg[RM];

	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled register offset operand.in arm doc */
static addr_t WOrUBGetAddrScaledRegOffset(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr;
	int shift = SHIFT;
	uint32_t index;
	switch(shift) {
	case 0:	/* LSL */
//		index = SHL(R(RM), CONST(SHIFT_IMM));
		index = core->Reg[RM] << SHIFT_IMM;
		break;
	case 1: /* LSR */
		if(SHIFT_IMM == 0)
			index = 0;
		else
//			index = LSHR(R(RM), CONST(SHIFT_IMM));
			index = core->Reg[RM] >> SHIFT_IMM;
		break;
	case 2:	/* ASR */
		if(SHIFT_IMM == 0)
//			index = ADD(XOR(LSHR(R(RM), CONST(31)), CONST(-1)), CONST(1));
		{
			index = (core->Reg[RM] >> 31) ^ 0xffffffff + 1;
		}
		else
//			index = ASHR(R(RM), CONST(SHIFT_IMM));
			index = ((int)core->Reg[RM]) >> SHIFT_IMM;
		break;
	case 3:	/* ROR or RRX */
		if(SHIFT_IMM == 0)
			;/* CFLAG? */
		else
//			index = ROTL(R(RM), CONST(SHIFT_IMM));
			index = (core->Reg[RM] >> SHIFT_IMM) | (core->Reg[RM] >> (32 - SHIFT_IMM));
		break;
	}

	if(LSUBIT)
//		Addr = ADD(R(RN), index);
		Addr = core->Reg[RN] + index;
	else
//		Addr = SUB(R(RN), index);
		Addr = core->Reg[RN] - index;

	CHECK_REG15();
	return Addr;
}

/* Getting Word or Unsigned Byte Address Immediate Preload operand.in arm doc */
static addr_t WOrUBGetAddrImmPre(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr = WOrUBGetAddrImmOffset(cpu, instr);
//	LET(RN, Addr);
//	core->Reg[RN] = Addr;
	return Addr;
}

/* Getting Word or Unsigned Byte Address Register Preload operand.in arm doc */
static addr_t WOrUBGetAddrRegPre(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr = WOrUBGetAddrRegOffset(cpu, instr);
//	LET(RN, Addr);
//	core->Reg[RN] = Addr;
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled Register Pre-indexed operand.in arm doc */
static addr_t WOrUBGetAddrScaledRegPre(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr = WOrUBGetAddrScaledRegOffset(cpu, instr);
//	LET(RN, Addr);
//	core->Reg[RN] = Addr;
	return Addr;
}

/* Getting Word or Unsigned Byte Immediate Post-indexed operand.in arm doc */
static addr_t WOrUBGetAddrImmPost(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	Value *Addr = R(RN);
	addr_t Addr = core->Reg[RN];
//	LET(RN,WOrUBGetAddrImmOffset(cpu, instr, bb));
//	core->Reg[RN] = WOrUBGetAddrImmOffset(cpu, instr);
	return Addr;
}

/* Getting Word or Unsigned Byte Address register Post-indexed operand.in arm doc */
static addr_t WOrUBGetAddrRegPost(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	Value *Addr = R(RN);
	addr_t Addr = core->Reg[RN];
//	LET(RN,WOrUBGetAddrRegOffset(cpu, instr, bb));
//	core->Reg[RN] = WOrUBGetAddrScaledRegOffset(cpu, instr);
	return Addr;
}

/* Getting Word or Unsigned Byte Address scaled register Post-indexed operand.in arm doc */
static addr_t WOrUBGetAddrScaledRegPost(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	Value *Addr = R(RN);
	addr_t Addr= core->Reg[RN];
//	LET(RN,WOrUBGetAddrScaledRegOffset(cpu, instr, bb));
//	core->Reg[RN] = WOrUBGetAddrScaledRegOffset(cpu, instr);
	return Addr;
}

/* Getting Word or Unsigned Byte Address Immediate operand operations collection */
static addr_t WOrUBGetAddrImm(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x5){
		if(!BIT(21)){
		/* ImmOff */
			return WOrUBGetAddrImmOffset(cpu, instr);
		}else{
		/* ImmPre */
			return WOrUBGetAddrImmPre(cpu, instr);
		}
	}else if(BITS(24,27) == 0x4){
		/* ImmPost */
		if(!BIT(21) || BIT(21)){
			return WOrUBGetAddrImmPost(cpu, instr);
		}
	}
	printf(" Error in WOrUB Get Imm Addr instr is %x \n", instr);
}

/* Getting Word or Unsigned Byte Address reg operand operations collection */
static addr_t WOrUBGetAddrReg(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x7){
		if(!BIT(21)){
		/* Reg off */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegOffset(cpu, instr);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegOffset(cpu, instr);
			}
		} else {
		/* Reg pre */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegPre(cpu, instr);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegPre(cpu, instr);
			}
		}
	}else if(BITS(24,27) == 0x6){
		if(!BIT(21)){
		/* Reg post */
			if(!BITS(4,11)){
				return WOrUBGetAddrRegPost(cpu, instr);
			}else{
			/* scaled reg */
				return WOrUBGetAddrScaledRegPost(cpu, instr);
			}
		}
	} else if (BITS(24, 27) == 0x5 && BIT(21) == 0) {
		return WOrUBGetAddrImmOffset(cpu, instr);
	}
	printf(" Error in WOrUB Get Reg Addr inst is %x\n", instr);
}

/* Getting Word or Unsigned Byte Address operand operations collection */
static addr_t WOrUBGetAddr(cpu_t *cpu, uint32_t instr)
{
	if(!BIT(25))
		return WOrUBGetAddrImm(cpu, instr);
	else
		return WOrUBGetAddrReg(cpu, instr);
}

/* Addr Mode 3, following arm operand doc */
/* Getting Miscellaneous Address Immidiate offset operand.in arm doc */
static addr_t MisGetAddrImmOffset(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr;
	addr_t Offset_8;

	Offset_8 = IMMH << 4 | IMML;
	if(LSUBIT)
//		Addr =  ADD(R(RN), Offset_8);
		Addr = core->Reg[RN] + Offset_8;
	else
//		Addr =  SUB(R(RN), Offset_8);
		Addr = core->Reg[RN] - Offset_8;

	CHECK_REG15();
	return Addr;
}

/* Getting Miscellaneous Address register offset operand.in arm doc */
static addr_t MisGetAddrRegOffset(cpu_t *cpu, uint32_t instr)
{
	addr_t Addr;
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	if(LSUBIT)
	//	Addr =  ADD(R(RN), R(RM));
		Addr = core->Reg[RN] + core->Reg[RM];
	else
//		Addr =  SUB(R(RN), R(RM));
		Addr = core->Reg[RN] - core->Reg[RM];

	CHECK_REG15();
	return Addr;
}

/* Getting Miscellaneous Address immdiate pre-indexed operand.in arm doc */
static addr_t MisGetAddrImmPre(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr = MisGetAddrImmOffset(cpu, instr);
	//LET(RN, Addr);
//	core->Reg[RN] = Addr;
	return Addr;
}

/* Getting Miscellaneous Address registers pre-indexed operand.in arm doc */
static addr_t MisGetAddrRegPre(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t Addr = MisGetAddrRegOffset(cpu, instr);
//	LET(RN, Addr);
//	core->Reg[RN] = Addr;

	return Addr;
}

/* Getting Miscellaneous Address immdiate post-indexed operand.in arm doc */
static addr_t MisGetAddrImmPost(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	Value *Addr = R(RN);
	addr_t Addr = core->Reg[RN];
//	LET(RN, MisGetAddrImmOffset(cpu, instr, bb));
//	core->Reg[RN] = MisGetAddrImmOffset(cpu, instr);
	return Addr;
}

/* Getting Miscellaneous Address register post-indexed operand.in arm doc */
static addr_t MisGetAddrRegPost(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
//	Value *Addr = R(RN);
	addr_t Addr = core->Reg[RN];
//	LET(RN, MisGetAddrRegOffset(cpu, instr, bb));
//	core->Reg[RN] = MisGetAddrRegOffset(cpu, instr);

	return Addr;
}

/* Getting Miscellaneous Address immdiate operand operation collection. */
static addr_t MisGetAddrImm(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x0){
		if(BITS(21,22) == 0x2){
		/* Imm Post */
			return MisGetAddrImmPost(cpu, instr);
		}
	}else if(BITS(24,27) == 0x1){
		if(BITS(21,22) == 0x2){
		/* Imm Offset */
			return MisGetAddrImmOffset(cpu, instr);
		}else if(BITS(21,22) == 0x3){
		/* Imm pre */
			return MisGetAddrImmPre(cpu, instr);
		}
	}
	printf(" Error in Mis Get Imm Addr \n");
}

/* Getting Miscellaneous Address register operand operation collection. */
static addr_t MisGetAddrReg(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x0){
		if(BITS(21,22) == 0x0){
		/* Reg Post */
			return MisGetAddrRegPost(cpu, instr);
		}
	}else if(BITS(24,27) == 0x1){
		if(BITS(21,22) == 0x0){
		/* Reg offset */
			return MisGetAddrRegOffset(cpu, instr);
		}else if(BITS(21,22) == 0x1){
		/* Reg pre */
			return MisGetAddrRegPre(cpu, instr);
		}
	}
	printf(" Error in Mis Get Reg Addr \n");
}


/* Getting Miscellaneous Address operand operation collection. */
static addr_t MisGetAddr(cpu_t *cpu, uint32_t instr)
{
	if(BIT(22))
		return MisGetAddrImm(cpu, instr);
	else
		return MisGetAddrReg(cpu, instr);
}

/* Addr Mode 4 */
/* Getting Load Store Multiple Address and Increment After operand */
static addr_t LSMGetAddrIA(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	int i =  BITS(0,15);
	int count = 0;
	addr_t Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

//	Addr = R(RN);
	Addr = core->Reg[RN];
#if 0
	if(LSWBIT)
		core->Reg[RN] = core->Reg[RN] + count * 4;
//		LET(RN, ADD(R(RN), CONST(count * 4)));
#endif
	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Increment Before operand */
static addr_t LSMGetAddrIB(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	int i =  BITS(0,15);
	int count = 0;
	addr_t Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}
	Addr = core->Reg[RN] + 4;
//	Addr = ADD(R(RN), CONST(4));
#if 0
	if(LSWBIT)
//		LET(RN, ADD(R(RN), CONST(count * 4)));
		core->Reg[RN] = core->Reg[RN] + count * 4;
#endif
	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Decrement After operand. */
static addr_t LSMGetAddrDA(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	int i =  BITS(0,15);
	int count = 0;
	addr_t Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

//	Addr = ADD(SUB(R(RN), CONST(count * 4)), CONST(4));
	Addr = core->Reg[RN] - count * 4 + 4;
//	if(LSWBIT)
//		LET(RN, SUB(R(RN), CONST(count * 4)));
//		core->Reg[RN] = core->Reg[RN] - count * 4;

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address and Decrement Before operand. */
static addr_t LSMGetAddrDB(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	int i =  BITS(0,15);
	int count = 0;
	addr_t Addr;
	while(i){
		if(i & 1)
			count ++;
		i = i >> 1;
	}

	Addr = core->Reg[RN] - count * 4;
//	if(LSWBIT)
//		LET(RN, SUB(R(RN), CONST(count * 4)));
//		core->Reg[RN] = Addr;

	CHECK_REG15();
	return  Addr;
}

/* Getting Load Store Multiple Address operand operation collection. */
static addr_t LSMGetAddr(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x8){
		if(BIT(23)){
		/* IA */
			return LSMGetAddrIA(cpu, instr);
		}else{
		/* DA */
			return LSMGetAddrDA(cpu, instr);
		}
	}else if(BITS(24,27) == 0x9){
		if(BIT(23)){
		/* IB */
			return LSMGetAddrIB(cpu, instr);
		}else{
		/* DB */
			return LSMGetAddrDB(cpu, instr);
		}
	}

	printf(" Error in LSM Get Imm Addr BITS(24,27) is 0x%x\n", BITS(24,27));
	exit(-1);
}

static addr_t GetAddr(cpu_t *cpu, uint32_t instr)
{
	if(BITS(24,27) == 0x1 || BITS(24,27) == 0x2 || BITS(24, 27) == 0){
		return MisGetAddr(cpu,instr);
	}else if(BITS(24,27) == 0x4 || BITS(24,27) == 0x5 || BITS(24,27) == 0x6 || BITS(24,27) == 0x7 ){
		return WOrUBGetAddr(cpu,instr);
	}else if(BITS(24,27) == 0x8 || BITS(24,27) == 0x9){
		return LSMGetAddr(cpu,instr);
	}

	printf("Not a Load Store Addr operation %x\n", instr);
	exit(-1);
//	return CONST(0);
	return 0;
}

/* Check address load/store instruction access.*/
static uint32_t arch_arm_check_mm(cpu_t *cpu, uint32_t instr)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t addr;
	fault_t fault = NO_FAULT;
	addr_t phys_addr;
	uint32_t rw = BIT(20) ? 1 : 0;
	if (BITS(20, 27) == 0x19 && BITS(4, 7) == 9) {
		/* ldrex */
		addr = core->Reg[RN];
	} else if (BITS(20, 27) == 0x18 && BITS(4, 7) == 9) {
		/* strex */
		addr = core->Reg[RN];
	} else
		addr = GetAddr(cpu, instr);
//	printf("pc is %x phys_pc is %x instr is %x\n", core->Reg[15], addr, instr);
	fault = get_phys_addr(cpu, addr, &phys_addr, 32, rw);
	if (fault) {
		printf("pc is %x phys_pc is %x instr is %x\n", core->Reg[15], addr, instr);
		printf("mmu fault in %s addr is %x\n", __FUNCTION__, addr);
		printf("fault is %d\n", fault);
		core->abortSig = true;
		core->Aborted = ARMul_DataAbortV;
		core->AbortAddr = addr;
		core->CP15[CP15(CP15_FAULT_STATUS)] = fault & 0xff;
		core->CP15[CP15(CP15_FAULT_ADDRESS)] = addr;
		return 1;
	}
	return 0;
}

static int arch_arm_effective_to_physical(cpu_t *cpu, uint32_t addr, uint32_t *result){
        arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	fault_t fault = NO_FAULT;
	addr_t phys_addr;
	if(is_user_mode(cpu)) {
		*result = addr;
		return 0;
	} else {
		fault = get_phys_addr(cpu, addr, &phys_addr, 32, 1);
		if (fault) {
			printf("mmu fault in %s addr is %x\n", __FUNCTION__, addr);
			printf("fault is %d\n", fault);
			core->abortSig = true;
			core->Aborted = ARMul_PrefetchAbortV;
			core->AbortAddr = addr;
			core->CP15[CP15(CP15_INSTR_FAULT_STATUS)] = fault & 0xff;
			core->CP15[CP15(CP15_FAULT_ADDRESS)] = addr;
			return fault;
		}
		*result = phys_addr;
		return 0;
	}
}


arch_mem_ops_t arm_dyncom_mem_ops = {
	is_inside_page,
	is_page_start,
	is_page_end,
	arch_arm_read_memory,
	arch_arm_write_memory,
	arch_arm_check_mm,
	arch_arm_effective_to_physical
};

