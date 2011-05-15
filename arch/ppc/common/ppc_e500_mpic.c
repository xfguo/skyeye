/*
        ppc_e500_core.c - mpic implementation for powerpc e500 core
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
 * 10/21/2008   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <ppc_e500_core.h>
#include <ppc_cpu.h>
#include <types.h>
#include <stdio.h>
#if 0
int e500_mpic_read_word(int offset, uint32 * result)
{
	int r;
	/**
         *  PIC Register Address Map
         */
	if (offset >= 0x40000 && offset <= 0x7FFF0) {

		switch (offset) {
		case 0x400a0:
			*result = gCPU.pic_global.iack;
			return r;
		case 0x41000:
			//*result = gCPU.pic_global.frr= 0x370002; /* according to 8560 manual */
			*result = gCPU.pic_global.frr = 0x6b0102;	/* according to MPC8572 manual */
			return r;
		case 0x41020:
			/* source attribute register for DMA0 */
			//printf("In %s,read gcr=0x%x\n", __FUNCTION__, *result);                        
			//*result = gCPU.pic_global.gcr & ~0x80000000; /* we clear RST bit after finish initialization of PIC */
			*result = gCPU.pic_global.gcr & ~0x1;
			printf("In %s,read gcr=0x%x, pc=0x%x\n", __FUNCTION__,
			       *result, current_core->pc);
			return r;

		case 0x410a0:
		case 0x410b0:
		case 0x410c0:
		case 0x410d0:
			*result = gCPU.mpic.ipivpr[(offset >> 6) & 0x3];
			return r;
		case 0x410e0:
			*result = gCPU.pic_global.svr;
			return r;
		case 0x410f0:
			*result = gCPU.pic_global.tfrr;
			return r;
		case 0x41120:
			*result = gCPU.pic_global.gtvpr0;
			return r;
		case 0x41160:
			*result = gCPU.pic_global.gtvpr1;
			return r;
		case 0x41170:
			*result = gCPU.pic_global.gtdr1;
			return r;
		case 0x411a0:
			*result = gCPU.pic_global.gtvpr2;
			return r;
		case 0x411B0:
			*result = gCPU.pic_global.gtdr2;
			return r;

		case 0x411E0:
			*result = gCPU.pic_global.gtvpr3;
			return r;

		default:
			/*
			   fprintf(stderr,"in %s, error when read global.offset=0x%x, \
			   pc=0x%x\n",__FUNCTION__, offset, current_core->pc);
			   return r;
			 */
			break;
			//skyeye_exit(-1);
		}

		if (offset >= 0x50000 && offset <= 0x50170) {
			int index = (offset - 0x50000) >> 4;
			if (index & 0x1)
				*result = gCPU.pic_ram.eidr[index >> 1];
			else
				*result = gCPU.pic_ram.eivpr[index >> 1];
			return r;
		}
		if (offset >= 0x50200 && offset <= 0x505F0) {
			int index = (offset - 0x50200) >> 4;
			if (index & 0x1)
				*result = gCPU.pic_ram.iidr[index >> 1];
			else
				*result = gCPU.pic_ram.iivpr[index >> 1];
			return r;
		}

		switch (offset) {
		case 0x60080:
			*result = gCPU.pic_ram.ctpr0;
			return r;
		case 0x600a0:
			*result = gCPU.pic_percpu.iack[0];
			return r;
		case 0x610a0:
			*result = gCPU.pic_percpu.iack[1];
			printf("In %s, ack=0x%x\n", __FUNCTION__, *result);
			return r;
		default:
			break;
		}
	}
	fprintf(stderr, "in %s, error when write pic ram,offset=0x%x,pc=0x%x\n",
		__FUNCTION__, offset, current_core->pc);
}

int e500_mpic_write_word();
int e500_mpic_read_half();
int e500_mpic_write_half();
int e500_mpic_read_byte();
int e500_mpic_write_half();
#endif
