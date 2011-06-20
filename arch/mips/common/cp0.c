/*
        cp0.c - read/write for cp0 register 

        Copyright (C) 2003 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 12/21/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "skyeye_config.h"
#include "instr.h"
#include "emul.h"
#include "mips_cpu.h"
#include <stdio.h>

// Access to the System Coprocessor registers.

extern FILE *skyeye_logfd;

UInt32 read_cp0_mt(MIPS_State* mstate, int n, int sel);
void write_cp0_mt(MIPS_State* mstate, int n, int sel, UInt32 x);
int decode_cop0_mt(MIPS_State* mstate, Instr instr);

UInt32
read_cp0(MIPS_State* mstate, int n, int sel)
{
	if(mstate->mt_flag == 1)
		read_cp0_mt(mstate, n, sel);
	else{
		switch (n) {
			case Index:
			{
				#if 0
				mstate->cp0[Index] = clear_bits(x, 30, 17); //Shi yang 2006-08-11
				mstate->cp0[Index] = clear_bits(x, 7, 0);
				#endif
				//printf("In %s, Index,x=0x%x\n", __FUNCTION__, mstate->cp0[Index]);
				return mstate->cp0[Index];
			}

			case Random:
			{
				return get_random(mstate);
			}
			case Count:
			{
				return mstate->cp0[Count] + ((mstate->now - mstate->count_seed) / 2);
			}
			case Cause:
			{
				//printf("KSDBG:read cause=0x%x, pc=0x%x\n", mstate->cp0[Cause], mstate->pc);
				return mstate->cp0[Cause];
				//return mstate->cp0[Cause] | (mstate->events & bitsmask(Cause_IP_Last, Cause_IP_First));
			}
			case EPC:
			{
				//fprintf(stderr, "read EPC, EPC = 0x%x, pc=0x%x\n", mstate->cp0[EPC], mstate->pc);
				return mstate->cp0[EPC];
			}
			case PRId:
			{
				return mstate->cp0[PRId]; /* the id of au1200 */
			}
			case Config:
			{
				if(sel) /* config1 */
					return mstate->cp0_config1; /* config1 for au1200 */
				else
					return mstate->cp0[Config]; /* config0 for au1200 */
			}
			default:
				return mstate->cp0[n];
	    }
	}
}

UInt32 
read_cp0_mt(MIPS_State* mstate, int n, int sel)
{
	MIPS_CPU_State* cpu = get_current_cpu();
	int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* get vpe of the current tc num */
	mips_core_t* VPE = &cpu->core[TcBindVPE+mstate->tc_num];	/* get the vpe pointer*/
	int VPECtrl_TargetTc = bits(VPE->cp0_VPECR, 7, 0);		/* get tc num on contorl */
	mips_core_t* curTC = &cpu->core[VPECtrl_TargetTc];		/* get the tc pointer */
	int curTcBindVPE = curTC->cp0_TCBind & 0xf;			/* get vpe num on contorl */
	mips_core_t* curVPE = &cpu->core[curTcBindVPE+mstate->tc_num];  /* get the vpe pointer */

	switch(sel){
		case 0:
		{

			switch (n) {
				case Index:
				{
						return curVPE->cp0[Index];
				}

				case Random:
				{
						return get_random(curVPE);
				}
				case Count:
				{
						return curVPE->cp0[Count] + ((curVPE->now - curVPE->count_seed) / 2);
				}
				case Cause:
				{
						return curVPE->cp0[Cause];
				}
				case EPC:
				{
						return mstate->cp0[EPC];
				}
				case PRId:
				{
						return curVPE->cp0[PRId];
				}
				case Config:
				{
						return curVPE->cp0[Config];
				}
				case SR:
						return curVPE->cp0[SR];

				default:
						return curVPE->cp0[n];
			}
		}

		case 1:
		{
			switch(n){
				case MVPCtr:
					return curVPE->cp0_MVPCR;

				case VPECtr:
					return curVPE->cp0_VPECR;

				case TCStatus:
				{
					return curTC->cp0_TCStatus;
				}

				case SRSConf0:
					return curVPE->cp0_SRSConf0;

				case Config:
					return curVPE->cp0_config1;

				case IntCtl:
					return curVPE->cp0_IntCtl;

				case EBase:
					return curVPE->cp0_EBase;

				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 2:
		{
			switch(n){
				case MVPConf0:
					return curVPE->cp0_MVPConf0;
				case VPEConf0:
					return curVPE->cp0_VPEConf0;
				case TCBind:
					return curTC->cp0_TCBind;
				case SRSConf1:
					return curVPE->cp0_SRSConf1;
				case Config:
					return curVPE->cp0_config2;
				case SRSCtl:
					return curVPE->cp0_SRSCtl;

				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 3:
		{
			switch(n){
				case MVPConf1:
					return curVPE->cp0_MVPConf1;
				case VPEConf1:
					return curVPE->cp0_VPEConf1;
				case TCRestart:
					return curTC->cp0_TCRestart;
				case SRSConf2:
					return curVPE->cp0_SRSConf2;
				case Config:
					return curVPE->cp0_config3;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 4:
		{
			switch(n){
				case YQMask:
					return curVPE->cp0_YQMask;
				case TCHalt:
					return curTC->cp0_TCHalt;
				case SRSConf3:
					return curVPE->cp0_SRSConf3;
				case Config:
					return curVPE->cp0_config4;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 5:
		{
			switch(n){
				case VPESche:
					return curVPE->cp0_VPESche;
				case TCContext:
					return curTC->cp0_TCContext;
				case SRSConf4:
					return curVPE->cp0_SRSConf4;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 6:
		{
			switch(n){
				case VPEScheFBack:
					return curVPE->cp0_VPEScheFBack;
				case TCSche:
					return curTC->cp0_TCSche;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		case 7:
		{
			switch(n){
				case VPEOpt:
					return curVPE->cp0_VPEOpt;
				case TCScheFBack:
					return curTC->cp0_TCScheFBack;
				case TCOpt:
					return curTC->cp0_TCOpt;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					return;
			}
		}

		default:
		{
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);

		}
	}
}

void 
write_cp0(MIPS_State* mstate, int n, int sel, UInt32 x)
{
	if(mstate->mt_flag)
		write_cp0_mt(mstate, n, sel, x);
	else{
		switch (n) {
			case Index:
			{
				#if 0
				mstate->cp0[Index] = clear_bits(x, 30, 17); //Shi yang 2006-08-11
				mstate->cp0[Index] = clear_bits(x, 7, 0);
				#endif
				mstate->cp0[Index] = x;
				//printf("In %s,Write Index,x=0x%x\n", __FUNCTION__, x);
				break;
			}
			case Random: //Random register is a read-only register
			{
				break;
			}
			case EntryLo0:
			{
				//mstate->cp0[EntryLo0] = clear_bits(x, 7, 0); //Shi yang 2006-08-11
				mstate->cp0[EntryLo0] = x;
				//printf("In %s,Write lo,x=0x%x,pc=0x%x\n", __FUNCTION__, x, mstate->pc);
				break;
			}
			case EntryLo1:
			{
				//mstate->cp0[EntryLo0] = clear_bits(x, 7, 0); //Shi yang 2006-08-11
				mstate->cp0[EntryLo1] = x;
				//printf("In %s,Write lo1,x=0x%x,pc=0x%x,\n", __FUNCTION__, x, mstate->pc);
				break;
			}
			case Context:
			{
				mstate->cp0[Context] = clear_bits(x, 1, 0); //Shi yang 2006-08-11
				break;
			}
			case BadVAddr: //BadVAddr register is a read-only register
			{
				break;
			}
			case Count:
			{
				mstate->count_seed = mstate->now;
				mstate->cp0[Count] = x;
				mstate->now = mstate->now + (mstate->cp0[Compare] - (mstate->cp0[Count] + ((mstate->now - mstate->count_seed) / 2))) * 2;
				break;
			}
			case EntryHi:
			{
				mstate->cp0[EntryHi] = x;
				break;
			}
			case Compare:
			{
				//fprintf(stderr, "KSDBG: in %s,write 0x%x to compare\n", __FUNCTION__, x);
				mstate->cp0[Compare] = x;
				mstate->events = clear_bit(mstate->events, 7 + Cause_IP_First);
				mstate->now = mstate->now + (mstate->cp0[Compare] - (mstate->cp0[Count] + ((mstate->now - mstate->count_seed) / 2))) * 2;
				mstate->cp0[Cause] &= 0xFFFF7FFF; /* clear IP bit in cause register for timer */
				break;
			}
			case SR:
			{
				mstate->cp0[SR] = x & ~(bitsmask(27, 26) | bitsmask(24, 23) | bitsmask(7, 6)); //Shi yang 2006-08-11
				//leave_kernel_mode(mstate);
				break;
			}
			case Cause:
			{
				//fprintf(stderr, "write cause, cause = 0x%x,pc=0x%x\n", x, mstate->pc);
				mstate->events |= x & bitsmask(Cause_IP1, Cause_IP0);
				break;
			}
			case EPC:
			{
				//fprintf(stderr, "write EPC, EPC = 0x%x, pc=0x%x\n", x, mstate->pc);
				mstate->cp0[EPC] = x;
				break;
			}
			case PRId: //PRId register is a read-only register
			{
				break;
			}
			default:
				printf("Reg=0x%x, not implemented instruction in %s\n",n, __FUNCTION__);
				break;
		}
	}
}

void
write_cp0_mt(MIPS_State* mstate, int n, int sel, UInt32 x)
{
	MIPS_CPU_State* cpu = get_current_cpu();
	int TcBindVPE = mstate->cp0_TCBind & 0xf;				/* get vpe of the current tc num */
	mips_core_t* VPE = &cpu->core[TcBindVPE+mstate->tc_num];                /* get the vpe pointer*/
	int VPECtrl_TargetTc = bits(VPE->cp0_VPECR, 7, 0);                      /* get tc num on contorl */
	mips_core_t* curTC = &cpu->core[VPECtrl_TargetTc];                      /* get the tc pointer */
	int curTcBindVPE = curTC->cp0_TCBind & 0xf;                             /* get vpe num on contorl */
	mips_core_t* curVPE = &cpu->core[curTcBindVPE+mstate->tc_num];          /* get the vpe pointer */

	switch(sel){
		case 0:
		{
			switch (n) {
				case Index:
					curVPE->cp0[Index] = x;
					break;
				case Random: //Random register is a read-only register
					break;
				case EntryLo0:
					curVPE->cp0[EntryLo0] = x;
					break;
				case EntryLo1:
					curVPE->cp0[EntryLo1] = x;
					break;
				case Context:
					curVPE->cp0[Context] = clear_bits(x, 1, 0);
					break;
				case BadVAddr:
					break;
				case Count:
				{
					curVPE->count_seed = curVPE->now;
					curVPE->cp0[Count] = x;
					curVPE->now = curVPE->now + (curVPE->cp0[Compare] - (curVPE->cp0[Count] + ((curVPE->now - curVPE->count_seed) / 2))) * 2;
					break;
				}
				case EntryHi:
					curVPE->cp0[EntryHi] = x;
					break;
				case Compare:
				{
					curVPE->cp0[Compare] = x;
					curVPE->events = clear_bit(curVPE->events, 7 + Cause_IP_First);
					curVPE->now = curVPE->now + (curVPE->cp0[Compare] - (curVPE->cp0[Count] + ((curVPE->now - curVPE->count_seed) / 2))) * 2;
					curVPE->cp0[Cause] &= 0xFFFF7FFF; /* clear IP bit in cause register for timer */
					break;
				}
				case SR:
				{
					MIPS_CPU_State* cpu = get_current_cpu();
					cpu->core[4].cp0[SR] = x;		/* set interrupt to all vpe tmep */
					cpu->core[5].cp0[SR] = x;

//					curVPE->cp0[SR] &= ~0x3;
					//leave_kernel_mode(mstate);
					break;
				}
				case Cause:
				{
					int tmp = curVPE->cp0[Cause];
					curVPE->cp0[Cause] = x & bitsmask(Cause_IP1, Cause_IP0);
					if(x & 0x300){
						curVPE->int_flag = 1;	/* if has a soft interrupt set the Int flag */
					}
					break;
				}
				case EPC:
					mstate->cp0[EPC] = x;
					break;
				case PRId: //PRId register is a read-only register
					break;
				case Config:
					curVPE->cp0[Config] = x;
					break;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
					break;
				}
			break;
		}

		case 1:
		{
			switch(n){
				case MVPCtr:
					if(curVPE->cp0_VPEConf0 & 0x1){
						int i;
						for(i = 0; i < cpu->core_num; i ++ )
							cpu->core[i].cp0_MVPCR = x;
						printf("in modify MVPCR  x is %x, pc is %x\n", mstate->pc);
					}
					break;
				case VPECtr:
					curVPE->cp0_VPECR = x;
					break;
				case TCStatus:
				{
					curTC->cp0_TCStatus = x;
					break;
				}
				case SRSConf0:
					curVPE->cp0_SRSConf0 = x;
					break;
				case Config:
					curVPE->cp0_config1 = x;
					break;
				case IntCtl:
					curVPE->cp0_IntCtl = x;
					break;
				case EBase:
					curVPE->cp0_EBase = x;
					break;

				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
			}
			break;
		}

		case 2:
		{
			switch(n){
				case MVPConf0:
					if(curVPE->cp0_VPEConf0 & 0x1){
						int i;
						for(i = 0; i < cpu->core_num; i ++ )
							cpu->core[i].cp0_MVPConf0 = x;
					}
					break;
				case VPEConf0:
					curVPE->cp0_VPEConf0 = x;
					break;
				case TCBind:
					curTC->cp0_TCBind = x;
					break;
				case SRSConf1:
					curVPE->cp0_SRSConf1 = x;
					break;
				case Config:
					curVPE->cp0_config2 = x;
					break;
				case SRSCtl:
					curVPE->cp0_SRSCtl = x;
					break;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
			}
			break;
		}

		case 3:
		{
			switch(n){
				case MVPConf1:
					if(curVPE->cp0_VPEConf0 & 0x1){
						int i;
						for(i = 0; i < cpu->core_num; i ++ )
							cpu->core[i].cp0_MVPConf1 = x;
					}
					break;
				case VPEConf1:
					curVPE->cp0_VPEConf1 = x;
					break;
				case TCRestart:
				{
					curTC->cp0_TCRestart = x;
					break;
				}
				case SRSConf2:
					curVPE->cp0_SRSConf2 = x;
					break;
				case Config:
					curVPE->cp0_config3 = x;
					break;
				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
			}
			break;
		}

		case 4:
		{
			switch(n){
				case YQMask:
					curVPE->cp0_YQMask = x;
					break;
				case TCHalt:
					curTC->cp0_TCHalt = x;
					break;
				case SRSConf3:
					curVPE->cp0_SRSConf3 = x;
					break;
				case Config:
					curVPE->cp0_config4 = x;
					break;

				default:
					printf("Reg=0x%x,sel=0x%x, pc = %x, not implemented instruction in %s\n",n, sel, mstate->pc, __FUNCTION__);
			}
			break;
		}
	}
}

int 
decode_cop0(MIPS_State* mstate, Instr instr)
{
	if(mstate->mt_flag)
		decode_cop0_mt(mstate, instr);
	else{
		// CP0 is usable in kernel more or when the CU bit in SR is set.
		if (!(mstate->mode & kmode) && !bit(mstate->cp0[SR], SR_CU0))
			process_coprocessor_unusable(mstate, 0);

		/* Only COP0, MFC0 and MTC0 make sense, although the R3K
		 * manuals say nothing about handling the others.
		 */

		if (bit(instr, 25)) {
			switch (funct(instr)) {
				case TLBR:
				{
					// Read Indexed TLB Entry
					TLBEntry* e = &mstate->tlb[bits(mstate->cp0[Index], 13, 6)]; //Shi yang 2006-08-11
					mstate->cp0[EntryHi] = e->hi ;
					mstate->cp0[EntryLo0] = e->lo_reg[0];
					mstate->cp0[EntryLo1] = e->lo_reg[1];
					return nothing_special;
				}
				case TLBWI:
				{
					// Write Indexed TLB Entry
					//printf("TLBWI,index=0x%x, pc=0x%x\n", mstate->cp0[Index], mstate->pc);
					set_tlb_entry(mstate, mstate->cp0[Index]); //Shi yang 2006-08-11
					return nothing_special;
				}
				case TLBWR:
				{
					// Write Random TLB Entry
					//printf("TLBWR,index=0x%x, pc=0x%x\n", get_random(mstate), mstate->pc);
					set_tlb_entry(mstate, get_random(mstate));
					return nothing_special;
				}
				case TLBP:
				{
					// Probe TLB For Matching Entry
					VA va = mstate->cp0[EntryHi];
					//printf("TLBP, va=0x%x\n", va);
					TLBEntry* e = probe_tlb(mstate, va);
					//printf("TLBP, index=0x%x\n", e->index);
					mstate->cp0[Index] = (e) ? e->index : bitmask(31);
					return nothing_special;
				}
				case RFE: //Shi yang 2006-08-11
				{
					printf("RFE, return from exp, pc=0x%x\n", mstate->pc);

					// Exception Return
					leave_kernel_mode(mstate);
					return nothing_special;
				}
				case ERET:
				{
					/* enable interrupt */
					mstate->cp0[SR] |= 0x1;
					//fprintf(stderr, "ERET, return from exp, SR=0x%x, pc=0x%x,epc=0x%x\n", mstate->cp0[SR], mstate->pc, mstate->cp0[EPC]);
					mstate->pc =  mstate->cp0[EPC];
					//fprintf(stderr, "ERET, return from exp, epc=0x%x\n", mstate->cp0[EPC]);
					if(mstate->cp0[Cause] & 1 << Cause_IP2){
						skyeye_config_t* config = get_current_config();
						config->mach->mach_set_intr(0);/* clear the corresponding interrupt status register */
					}
					/*
					if(mstate->cp0[Cause] & 1 << Cause_IP4)
						mstate->cp0[Cause] &= ~(1 << Cause_IP4);
					*/
					mstate->pipeline = branch_nodelay;
					return nothing_special;
				}
				default:
					process_reserved_instruction(mstate);
					return nothing_special;
			}
		} else {
			switch (rs(instr)) {
				case MFCz:
				{
					// Move From System Control Coprocessor
					mstate->gpr[rt(instr)] = read_cp0(mstate, rd(instr), sel(instr));
					return nothing_special;
				}
				case DMFCz:
				{
					// Doubleword Move From System Control Coprocessor
					process_reserved_instruction(mstate);
					return nothing_special;
				}
				case CFCz:
				{
					// Move Control From Coprocessor
					return nothing_special;
				}
				case MTCz:
				{
					// Move To System Control Coprocessor
					write_cp0(mstate, rd(instr), sel(instr), mstate->gpr[rt(instr)]);
					return nothing_special;
				}
				case DMTCz:
				{
					// Doubleword Move To System Control Coprocessor
					process_reserved_instruction(mstate);
					return nothing_special;
				}
				case CTCz:
				{
					process_reserved_instruction(mstate);

					// Move Control To Coprocessor
					return nothing_special;
				}
				case BCz:
				{
					// Branch On Coprocessor Condition
					switch (rt(instr)) {
						case BCzF:
						case BCzT:
						case BCzFL:
						case BCzTL:
							process_reserved_instruction(mstate);
							return nothing_special;
						default:
							process_reserved_instruction(mstate);
							return nothing_special;
					}
				}
				default:
						process_reserved_instruction(mstate);
						return nothing_special;
			}
		}
		return nothing_special;
	}
}


int
decode_cop0_mt(MIPS_State* mstate, Instr instr)
{
	// CP0 is usable in kernel more or when the CU bit in SR is set.
#if 0
    	if (!(mstate->mode & kmode) && !bit(mstate->cp0[SR], SR_CU0))
		process_coprocessor_unusable(mstate, 0);
#endif
    	/* Only COP0, MFC0 and MTC0 make sense, although the R3K
    	 * manuals say nothing about handling the others.
         */

    	if (bit(instr, 25)) {
		switch (funct(instr)) {
			case TLBR:
			{
			    	// Read Indexed TLB Entry
				TLBEntry* e = &mstate->tlb[bits(mstate->cp0[Index], 13, 6)]; //Shi yang 2006-08-11
			    	mstate->cp0[EntryHi] = e->hi ;
			    	mstate->cp0[EntryLo0] = e->lo_reg[0];
				mstate->cp0[EntryLo1] = e->lo_reg[1];
			    	return nothing_special;
			}
			case TLBWI:
			{
			    	// Write Indexed TLB Entry
				//printf("TLBWI,index=0x%x, pc=0x%x\n", mstate->cp0[Index], mstate->pc);
			    	set_tlb_entry(mstate, mstate->cp0[Index]); //Shi yang 2006-08-11
			    	return nothing_special;
			}
			case TLBWR:
			{
		    		// Write Random TLB Entry
				//printf("TLBWR,index=0x%x, pc=0x%x\n", get_random(mstate), mstate->pc);
			    	set_tlb_entry(mstate, get_random(mstate));
			    	return nothing_special;
			}
			case TLBP:
			{
		    		// Probe TLB For Matching Entry
			    	VA va = mstate->cp0[EntryHi];
				//printf("TLBP, va=0x%x\n", va);
		    		TLBEntry* e = probe_tlb(mstate, va);
				//printf("TLBP, index=0x%x\n", e->index);
			    	mstate->cp0[Index] = (e) ? e->index : bitmask(31);
			    	return nothing_special;
			}
			case RFE: //Shi yang 2006-08-11
			{
				// Exception Return
				leave_kernel_mode(mstate);
				MIPS_CPU_State* cpu = get_current_cpu();
				int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* get current vpe num */
				mips_core_t* curVPE = &cpu->core[TcBindVPE+mstate->tc_num];
				curVPE->cp0[Cause] &= ~(0x4000ff00);
				return nothing_special;
			}
			case ERET:
			{	
				MIPS_CPU_State* cpu = get_current_cpu();
				int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* get current vpe num */
				mips_core_t* curVPE = &cpu->core[TcBindVPE+mstate->tc_num];
				curVPE->cp0[Cause] &= ~(0x4000ff00);
				curVPE->cp0[SR] |= 0x1;
				/* enable interrupt */
				//fprintf(stderr, "ERET, return from exp, SR=0x%x, pc=0x%x,epc=0x%x\n", mstate->cp0[SR], mstate->pc, mstate->cp0[EPC]);
				mstate->pc =  mstate->cp0[EPC];
				//fprintf(stderr, "ERET, return from exp, epc=0x%x\n", mstate->cp0[EPC]);
				if(mstate->cp0[Cause] & 1 << Cause_IP2){
					skyeye_config_t* config = get_current_config();
					config->mach->mach_set_intr(0);/* clear the corresponding interrupt status register */
				}
				/*
				if(mstate->cp0[Cause] & 1 << Cause_IP4)
					mstate->cp0[Cause] &= ~(1 << Cause_IP4);
				*/
				mstate->pipeline = branch_nodelay;
				return nothing_special;
			}
			case WAIT:
				return nothing_special;

			default:
				process_reserved_instruction(mstate);
		    		return nothing_special;
		}
    	} else {
		switch (rs(instr)) {
			case MFCz:
			{
		    		// Move From System Control Coprocessor
				mstate->gpr[rt(instr)] = read_cp0(mstate, rd(instr), sel(instr));
			    	return nothing_special;
			}
			case DMFCz:
			{
		    		// Doubleword Move From System Control Coprocessor
				process_reserved_instruction(mstate);
				return nothing_special;
			}
			case CFCz:
			{
		    		// Move Control From Coprocessor
			    	return nothing_special;
			}
			case MTCz:
			{
		    		// Move To System Control Coprocessor
				write_cp0(mstate, rd(instr), sel(instr), mstate->gpr[rt(instr)]);
			    	return nothing_special;
			}
			case DMTCz:
			{
			    	// Doubleword Move To System Control Coprocessor
				process_reserved_instruction(mstate);
			    	return nothing_special;
			}
			case CTCz:
			{
				process_reserved_instruction(mstate);

		    		// Move Control To Coprocessor
			    	return nothing_special;
			}

#if 0
			case BCz:
			{
			    	// Branch On Coprocessor Condition
	    			switch (rt(instr)) {
				    	case BCzF:
				    	case BCzT:
				    	case BCzFL:
				    	case BCzTL:
						process_reserved_instruction(mstate);
						return nothing_special;
	    				default:
						process_reserved_instruction(mstate);
						return nothing_special;
	    			}
			}
#endif

			/* have the same value with BCz */
			case MFTR:
			{
				MIPS_CPU_State* cpu = get_current_cpu();
				int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* vpe num of the current tc */
				mips_core_t* curVPE = &cpu->core[TcBindVPE+mstate->tc_num];
				int VPECtrl_TargetTc = bits(curVPE->cp0_VPECR, 7, 0);		/* get the tc num on control */
				mips_core_t* curTC = &cpu->core[VPECtrl_TargetTc];		/* get the tc pointer */
				int VPEConf0_MVP = bit(curVPE->cp0_VPEConf0,1);			/* get MVP bit */
				int MVPConf0_PTC = bits(curVPE->cp0_MVPConf0, 7, 0);		/* get PTC bit */
				int u = bit(instr, 5);						/* get MFTR u bit */
				int sel = bits(instr, 2, 0);					/* get sel from instr */
				int h = bit(instr,4);						/* get h bit from instr */
				int data;


				if(u == 0){
					data = read_cp0(mstate, rt(instr), sel);
				}
				else{
					switch(sel){
						case 0:
							data = curTC->gpr[rt(instr)];
							break;
						case 1:
							switch(rt(instr)){
								case 0:
								{
									data = curTC->lo;
									break;
								}
								case 1:
								{
									data = curTC->hi;
									break;
								}
								case 2:
								{
									printf("in MFTC unimplemented ACX\n");
									data = -1;
									break;
								}
								case 4:
								{
									printf("in MFTC unimplemented DSPLo[1]\n");
									data = -1;
									break;
								}
								case 5:
								{
									printf("in MFTC unimplemented DSPHi[1]\n");
									data = -1;
									break;
								}
								case 6:
								{
									printf("in MFTC unimplemented DSPACX[1]\n");
									data = -1;
									break;
								}
								case 8:
								{
									printf("in MFTC unimplemented DSPLO[2]\n");
									data = -1;
									break;
								}
								case 9:
								{
									printf("in MFTC unimplemented DSPHI[2]\n");
									data = -1;
									break;
								}
								case 10:
								{
									printf("in MFTC unimplemented DSPACX[2]\n");
									data = -1;
									break;
								}
								case 12:
								{
									printf("in MFTC unimplemented DSPLo[3]\n");
									data = -1;
									break;
								}
								case 13:
								{
									printf("in MFTC unimplemented DSPHi[3]\n");
									data = -1;
									break;
								}
								case 14:
								{
									printf("in MFTC unimplemented DSPACX[3]\n");
									data = -1;
									break;
								}
								case 16:
								{
									printf("in MFTC unimplemented DSPHi[2]\n");
									data = -1;
									break;
								}
								default:
									printf("in MFTC, unpredictable cp0 rt %d\n", rt(instr));
							}
							break;
						case 2:
						{
							printf("in MFTC unimplemented FPR\n");
							data = -1;
							break;
						}
						case 3:
						{
							printf("in MFTC unimplemented FPCR\n");
							data = -1;
							break;
						}
						case 4:
						{
							printf("in MFTC unimplemented cp2CPR\n");
							data = -1;
							break;
						}
						case 5:
						{
							printf("in MFTC unimplemented cp2CCR\n");
							data = -1;
							break;
						}
						default:
							printf("in MFTC, unimplement this sel %d\n", sel);
					}
				}

				if( h == 1 )
					printf("in MFTC, unimplement h selection %d\n", sel);

				mstate->gpr[rd(instr)] = data;
				//process_reserved_instruction(mstate);
				return nothing_special;
			}
			case MTTR:
			{
				MIPS_CPU_State* cpu = get_current_cpu();
				int TcBindVPE = mstate->cp0_TCBind & 0xf;
				mips_core_t* curVPE = &cpu->core[TcBindVPE+mstate->tc_num];	/* get current VPE */
				int VPECtrl_TargetTc = bits(curVPE->cp0_VPECR, 7, 0);
				mips_core_t* curTC = &cpu->core[VPECtrl_TargetTc];		/* get Tc on control */
				int VPEConf0_MVP = bit(curVPE->cp0_VPEConf0,1);
				int MVPConf0_PTC = bits(curVPE->cp0_MVPConf0, 7, 0);
				int u = bit(instr, 5);
				int sel = bits(instr, 2, 0);
				int h = bit(instr,4);


				if(VPEConf0_MVP == 0){
					printf("return MVP = 0\n");
//			    		return nothing_special;
				}else if(VPECtrl_TargetTc > MVPConf0_PTC){
					printf("return Tc > PTC\n");
					return nothing_special;
				}else{
						printf("unimplemented h bit\n");
				}

				if(u == 0){
					write_cp0(mstate, rd(instr), sel, mstate->gpr[rt(instr)]);
				}else{
					switch(sel){
						case 0:{
							curTC->gpr[rd(instr)] = mstate->gpr[rt(instr)];
							break;
						}
						case 1:{
							switch(rd(instr)){
								case 0:
								{
									curTC->lo = mstate->gpr[rt(instr)];
									break;
								}
								case 1:
								{
									curTC->hi = mstate->gpr[rt(instr)];
									break;
								}
								case 2:
								{
									printf("in MTTC unimplemented ACX\n");
									break;
								}
								case 4:
								{
									printf("in MTTC unimplemented DSPLo[1]\n");
									break;
								}
								case 5:
								{
									printf("in MTTC unimplemented DSPHi[1]\n");
									break;
								}
								case 6:
								{
									printf("in MTTC unimplemented DSPACX[1]\n");
									break;
								}
								case 8:
								{
									printf("in MTTC unimplemented DSPLO[2]\n");
									break;
								}
								case 9:
								{
									printf("in MTTC unimplemented DSPHI[2]\n");
									break;
								}
								case 10:
								{
									printf("in MTTC unimplemented DSPACX[2]\n");
									break;
								}
								case 12:
								{
									printf("in MTTC unimplemented DSPLo[3]\n");
									break;
								}
								case 13:
								{
									printf("in MTTC unimplemented DSPHi[3]\n");
									break;
								}
								case 14:
								{
									printf("in MTTC unimplemented DSPACX[3]\n");
									break;
								}
								case 16:
								{
									printf("in MTTC unimplemented DSPHi[2]\n");
									break;
								}

								default:
									printf("in MTTC, unpredictable cp0 rd %d\n", rd(instr));
							}
							break;
						}
						case 2:
						{
							printf("in MTTC unimplemented FPR\n");
							break;
						}
						case 3:
						{
							printf("in MTTC unimplemented FPCR\n");
							break;
						}
						case 4:
						{
							printf("in MTTC unimplemented cp2CPR\n");
							break;
						}
						case 5:
						{
							printf("in MTTC unimplemented cp2CCR\n");
							break;
						}
						default:
							printf("in MTTC, unimplement this sel %d\n", sel);
					}
				}
				return nothing_special;
			}
			case MFMC0:
			{
				MIPS_CPU_State* cpu = get_current_cpu();
				int TcBindVPE = mstate->cp0_TCBind & 0xf;			/* get current VPE */
				mips_core_t* curVPE = &cpu->core[TcBindVPE+mstate->tc_num];
				int VPECtrl_TargetTc = bits(curVPE->cp0_VPECR, 7, 0);		/* get TC on control */
				mips_core_t* curTC = &cpu->core[VPECtrl_TargetTc];

				switch(bits(instr, 15, 6))
				{
					case 0x02f:
					{
						if(bit((instr), 5)){
							/* EMT */
							mstate->gpr[rt(instr)] = curVPE->cp0_VPECR;
							curVPE->cp0_VPECR |= 0x00008000;
						}else{
							/* DMT */
							mstate->gpr[rt(instr)] = curVPE->cp0_VPECR;
							curVPE->cp0_VPECR &= 0xFFFF7FFF;
						}

						return nothing_special;
					}

					case 0x0:
					{
						int i;
						if(bit((instr), 5)){
							/* EVPE */
							mstate->gpr[rt(instr)] = curVPE->cp0_MVPCR;
							if(curVPE->cp0_VPEConf0 & 0x1){
								for(i = 0; i < cpu->core_num; i ++ )
									cpu->core[i].cp0_MVPCR |= 0x1;
								for( i = 0; i < ((curVPE->cp0_MVPConf0 >> 10) & 0xf) + 1; i ++ ){
									cpu->core[i].active = 1;

									if(&cpu->core[i] != mstate){
										if(!cpu->core[i].cp0_TCHalt && i){
											cpu->core[i].pc = cpu->core[i].cp0_TCRestart;
										if(cpu->core[i].cp0_TCHalt && i == 0)
											cpu->core[0].pc = cpu->core[0].cp0_TCRestart;
										}
									}
								}
							}
						}else{
							/* DVPE */
							mstate->gpr[rt(instr)] = curVPE->cp0_MVPCR;
							if(curVPE->cp0_VPEConf0 & 0x1){
								for(i = 0; i < cpu->core_num; i ++ )
									cpu->core[i].cp0_MVPCR &= 0xFFFFFFFE;
								for( i = 0; i < ((curVPE->cp0_MVPConf0 >> 10) & 0xf) + 1; i ++ ){
									cpu->core[i].active = 0;
									cpu->core[i].cp0_TCRestart = cpu->core[i].pc;
								}
								mstate->active = 1;
							}
						}
						return nothing_special;
					}

					case 0x180:
					{
						if(bit(instr,5)){
							/* EI */
							//printf("EI############,pc is %x, mstate is %x\n", mstate->pc, mstate);
							mstate->gpr[rt(instr)] = curVPE->cp0[SR];
							//curVPE->cp0[SR] |= 0x1;
							cpu->core[4].cp0[SR] |= 0x1;
							cpu->core[5].cp0[SR] |= 0x1;
							return nothing_special;
						}else{
							/* DI */
							//printf("DI############,pc is %x, mstate is %x\n", mstate->pc, mstate);
							mstate->gpr[rt(instr)] = curVPE->cp0[SR];
							//curVPE->cp0[SR] &= 0xFFFFFFFE;
							cpu->core[4].cp0[SR] &= ~0x1;
							cpu->core[5].cp0[SR] &= ~0x1;
							return nothing_special;
						}
					}
					default:
						process_reserved_instruction(mstate);
						return nothing_special;
				}
			}
			default:
				    	process_reserved_instruction(mstate);
					return nothing_special;
		}
    	}
    	return nothing_special;
}
int 
decode_cop1(MIPS_State* mstate, Instr instr)
{
	// CP1 is usable in kernel more or when the CU bit in SR is set.
    	if (!bit(mstate->cp0[SR], SR_CU1)){
		process_coprocessor_unusable(mstate, 1);
		return nothing_special;
	}

    	/* Only COP0, MFC0 and MTC0 make sense, although the R3K
    	 * manuals say nothing about handling the others.
         */
	switch (function(instr)){
				case WAIT:
			return nothing_special;
		default:
			//process_reserved_instruction(mstate);
                        break;

	}
	switch (fmt(instr)){
		case CF:		
			if(fs(instr) == 0)
				mstate->gpr[ft(instr)] = mstate->fir;
			else
				fprintf(stderr, "In %s, not implement for CFC1 instruction\n", __FUNCTION__);
			break;
								
		default:
			//process_reserved_instruction(mstate);
			fprintf(stderr, "In %s,not implement instruction 0x%x,pc=0x%x\n", __FUNCTION__, instr, mstate->pc);

			break;
	}
	return nothing_special;
}

