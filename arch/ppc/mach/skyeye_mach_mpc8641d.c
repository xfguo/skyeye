/*
        skyeye_mach_mpc8641d.c - mpc8641d machine simulation implementation
        Copyright (C) 2003-2007 Skyeye Develop Group
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
 * 01/04/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include <stdint.h>
#include "skyeye_config.h"

#include "sysendian.h"
#include <ppc_cpu.h>
#include <ppc_e500_exc.h>

#ifdef __CYGWIN__
#include <sys/time.h>
#endif

#ifdef __MINGW32__
#include <sys/time.h>
#endif

#define UART_IRQ 26

typedef struct ccsr_reg_s{
	//uint32_t ccsr; /* Configuration,control and status registers base address register */
	uint32_t altcbar; /* allternate configuration base address register */
	uint32_t altcar; /* alternate configuration attribute register */
	//uint32_t bptr; /* Boot page translation register */
}ccsr_reg_t;

/* Local bus controll register */
typedef struct lb_ctrl_s{
	uint32 br[8];
	//uint32 or[8];
	uint32 lcrr; /* Clock ratio register */
	//uint32 lbcr; /* Configuration register */
}lb_ctrl_t;

typedef struct law_reg_s{
	uint32_t lawbar[8];
	uint32_t lawar[8];
}law_reg_t;

typedef struct por_conf_s{
	uint32_t porpllsr;
	uint32_t porbmsr;
	uint32_t porimpscr;
	uint32_t pordevsr;
	uint32_t pordbgmsr;
	uint32_t gpporcr;
}por_conf_t;


typedef struct i2c_reg_s{
	uint32 i2cadr;
	uint32 i2ccr;
	uint32 i2csr;
	uint32 i2cdr;
	uint32 i2cfdr;
	uint32 i2cdfsrr;
}i2c_reg_t;

typedef struct debug_ctrl_s{
	uint32 clkocr;
	uint32 ddrdllcr;
	uint32 lbdrrcr;
}debug_ctrl_t;

typedef struct ppc_dma_s{
	uint32 satr0;
	uint32 datr0;
}mpc_dma_t;

typedef struct ddr_ctrl_s{
	uint32 err_disable;
}ddr_ctrl_t;

typedef struct l2_reg_s{
	uint32 l2ctl;
}l2_reg_t;

typedef struct pci_cfg_s{
	uint32 cfg_addr;
	uint32 cfg_data;
	uint32 int_ack;
}pci_cfg_t;

typedef struct pci_atmu_s{
	uint32 potar1;
	uint32 potear1;
	uint32 powbar1;
	uint32 reserv1;
	uint32 powar1;
}pci_atmu_t;

typedef struct pic_global_s{
	uint32 gcr;
	uint32 frr;
	uint32 tfrr;
	uint32 gtdr0;
	uint32 gtdr1;
	uint32 gtdr2;
	uint32 gtdr3;
	uint32 gtvpr0;
	uint32 gtvpr1;
	uint32 gtvpr2;
	uint32 gtvpr3;
	uint32 svr;
	uint32 iack;
}pic_global_t;

typedef struct pic_ram_s{
	uint32 eivpr[11];
	uint32 eidr[11];
	uint32 ctpr0, ctpr1;
	//uint32 iivpr[32];
	uint32 iivpr[64];
	//uint32 iidr[32];
	uint32 iidr[64];
	uint32 eoi1;
	uint32 eoi0;
}pic_ram_t;

typedef struct pic_percpu_s{
	uint32 iack[2];
}pic_percpu_t;

typedef struct mpic_s{
	uint32 gcr; /* Global configuration register */
	uint32 ipivpr[4]; /* IPI 0-3 vector/priority register */
	uint32 ipidr[4]; /* IPI 0 - 3 dispatch register */
	uint32 iivpr[64];
	uint32 iidr[64];

	uint32 mivpr[4];
	uint32 midr[4];
	uint32 msivpr[8];
	uint32 msidr[8];

}mpic_t;

typedef struct ecm_s{
	uint32 eebacr;
	//uint32 eebpcr;
}ecm_t;

typedef struct std_16550_uart_s{
	uint32 rbr;
	uint32 thr;
	uint32 iir;
	uint32 ier;
	uint32 dmb;
	uint32 fcr;
	uint32 afr;
	uint32 lcr;
	uint32 lsr;
	uint32 mcr;
	uint32 msr;
	uint32 scr;
}std_16550_uart_t;

typedef struct mpc8641d_io_s{	
	uint32 core_num;
	//ppc_cpm_t cpm_reg; /* Communication processor */
	ccsr_reg_t ccsr;
	law_reg_t law; /* Local access window */
	lb_ctrl_t lb_ctrl; /* Local bus controll register */
	uint32 sccr; /* System clock control register */
	por_conf_t por_conf;
	i2c_reg_t i2c_reg;
	debug_ctrl_t debug_ctrl;
	ddr_ctrl_t ddr_ctrl;
	mpc_dma_t dma;
	l2_reg_t l2_reg;
	pci_cfg_t pci_cfg;
	pci_atmu_t pci_atmu;
	pic_global_t pic_global;
	pic_ram_t pic_ram;
	pic_percpu_t pic_percpu;
	mpic_t mpic;
	ecm_t ecm;
	std_16550_uart_t uart[2];
}mpc8641d_io_t;

static mpc8641d_io_t mpc8641d_io;

static void
std8250_io_do_cycle (void * state)
{
	const int core_id = 0; /* currently, we only send uart interrupt to cpu0 */
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	std_16550_uart_t *uart = &io->uart[0];
	/*
	   if(!(core->msr & 0x8000)) 
	   return;
	 */
	if (uart->iir & 0x1) {
		if (uart->ier & 0x2) {	/* THREI enabled */
			//printf("In %s, THR interrupt\n", __FUNCTION__);
			uart->iir = (uart->iir & 0xf0) | 0x2;
			uart->lsr |= 0x60;
		}

		if (uart->ier & 0x1) {	/* RDAI enabled */
			struct timeval tv;
			unsigned char c;

			tv.tv_sec = 0;
			tv.tv_usec = 0;
			if (skyeye_uart_read (-1, &c, 1, &tv, NULL) > 0) {
				uart->rbr = (int) c;
				//printf("SKYEYE: io_do_cycle  set ffiir  or 04, now %x\n",pxa250_io.ffiir);
				uart->lsr |= 0x01;	//Data ready
				uart->iir = (uart->iir & 0xf0) | 0x4;
			}
		}
	}
	if ((!(io->mpic.iivpr[UART_IRQ] & 0x80000000) && (core->msr & 0x8000)
	     && !(core->ipr & 1 << UART_IRQ))) {
		//if(!(io->mpic.iivpr[UART_IRQ] & 0x80000000)){
		if ((!(uart->iir & 0x1)) && (uart->ier & 0x3)
			) {
			//printf ("In %s,uart int triggered. ier=0x%x\n",	__FUNCTION__, uart->ier);
			core->ipr |= (1 << UART_IRQ);
			io->mpic.iivpr[UART_IRQ] |= 0x40000000;	/* set activity bit in vpr */
			io->pic_percpu.iack[core_id] =
				(io->pic_percpu.
				 iack[core_id] & 0xFFFF0000) | (io->mpic.
								ipivpr
								[UART_IRQ] &
								0xFFFF);
			io->pic_percpu.iack[core_id] = 0x2a;
			//printf("In %s, ack=0x%x\n", __FUNCTION__, io->pic_percpu.iack[core_id]);
			core->ipi_flag = 1;	/* we need to inform the core that npc is changed to exception vector */
			ppc_exception (core, EXT_INT, 0, 0);
		}
	}
}

static void
mpc8641d_io_do_cycle (void *state)
{
	std8250_io_do_cycle (state);
}
static void
mpc8641d_io_reset (void *state)
{
	mpc8641d_io_t *io = &mpc8641d_io;

	/* Just for convience of boot linux */
	//io->conf.ccsrbar = 0x000E0000;
	io->pic_global.frr = 0x6b0102;
	io->por_conf.porpllsr = 0x40004;
	io->lb_ctrl.lcrr = 0x80000008;
	io->i2c_reg.i2csr = 0x81;
	io->i2c_reg.i2cdfsrr = 0x10;
	io->pic_ram.ctpr0 = 0x0000000F;
	io->pic_global.svr = 0xFFFF;
	io->mpic.ipivpr[0] = io->mpic.ipivpr[1] = io->mpic.ipivpr[2] = io->mpic.ipivpr[3] = 0x80000000;

	io->uart[0].ier = 0x0;
	io->uart[0].iir = 0x1;
	io->uart[0].rbr = 0x1;
	//gCPU.mpic.iivpr[UART_IRQ] = 0x80800000;

	/* initialize interrupt controller */
	int i = 0;
	for (; i < 32; i++) {
		io->pic_ram.iidr[i] = 0x1;
		io->pic_ram.iivpr[i] = 0x80800000;
	}
	mpc8641d_boot_linux();
}
static uint32_t
mpc8641d_io_read_byte (void *state, uint32_t offset)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.addr=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, core->pc);
			skyeye_exit (-1);
		}
	}

	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
			case 0xE0000:
				return io->por_conf.porpllsr;
			default:
				fprintf (stderr,
					 "in %s, error when read CCSR.addr=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}
	switch (offset) {
		case 0x0:
			return cpu->ccsr;
		case 0x90C80:
			return io->sccr;
		case 0x300C:
			return io->i2c_reg.i2csr;
		case 0x8006:
			return io->pci_cfg.cfg_data;
		case 0x4500:
			io->uart[0].lsr &= ~0x01;
			return io->uart[0].rbr;
		case 0x4501:
			//printf("In %s,read offset=0x%x,pc=0x%x\n", __FUNCTION__, offset, current_core->pc);
			if (io->uart[0].lcr & 0x80)
				return io->uart[0].dmb;
			else
				return io->uart[0].ier;
		case 0x4502:
			if (io->uart[0].lcr & 0x80)
				return io->uart[0].afr;
			else {
				uint32_t tmp = io->uart[0].iir;
				io->uart[0].iir = (io->uart[0].iir & 0xf0) | 0x1;
				//printf("In %s,read offset=0x%x,iir=0x%x, pc=0x%x\n", __FUNCTION__, offset, tmp, current_core->pc);
				return tmp;
			}

		case 0x4503:
			//printf("In %s,read offset=0x%x\n", __FUNCTION__, offset);
			return io->uart[0].lcr;
		case 0x4504:
			//printf("In %s,read offset=0x%x\n", __FUNCTION__, offset);
			return io->uart[0].mcr;
		case 0x4505:
			return io->uart[0].lsr;	/* THRE */
		case 0x4506:
			return io->uart[0].msr;
		case 0x4507:
			return io->uart[0].scr;
		case 0x4600:
			return io->uart[1].rbr;
		case 0x4601:
			//printf("In %s,read offset=0x%x,pc=0x%x\n", __FUNCTION__, offset, current_core->pc);
			if (io->uart[1].lcr & 0x80)
				return io->uart[1].dmb;
			else
				return io->uart[1].ier;
		case 0x4602:
			if (io->uart[1].lcr & 0x80)
				return io->uart[1].afr;
			else
				return io->uart[1].iir;
		case 0x4603:
			//printf("In %s,read offset=0x%x\n", __FUNCTION__, offset);
			return io->uart[1].lcr;
		case 0x4604:
			//printf("In %s,read offset=0x%x\n", __FUNCTION__, offset);
			return io->uart[1].mcr;
		case 0x4607:
			return io->uart[1].scr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR. offset=0x%x, pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
		//skyeye_exit(-1);
	}
}
static uint32_t
mpc8641d_io_read_halfword (void *state, uint32_t offset)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	//int offset = p - GET_CCSR_BASE (io->ccsr.ccsr);
	//printf("DBG:read CCSR,offset=0x%x,pc=0x%x\n", offset, current_core->pc);
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
			default:
				fprintf (stderr,
					 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}
	
	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
		case 0xE0000:
			return io->por_conf.porpllsr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			skyeye_exit (-1);
		}

	}

	switch (offset) {
		case 0x0:
			return cpu->ccsr;
		case 0x90C80:
			return io->sccr;
		case 0x8004:
			return io->pci_cfg.cfg_data;
		case 0x8006:
			return io->pci_cfg.cfg_data;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			//skyeye_exit(-1);
	}
}
static uint32_t
mpc8641d_io_read_word (void *state, uint32_t offset)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;
	//printf("DBG:in %s,read CCSR,offset=0x%x,pc=0x%x\n", __FUNCTION__, offset, current_core->pc);

	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
			default:
				fprintf (stderr,
					 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}
	if (offset >= 0x2000 && offset <= 0x2E58) {
		switch (offset) {
			case 0x2E44:
				return io->ddr_ctrl.err_disable;
			default:
				fprintf (stderr,
					 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}



	/* PIC Global register */
	if (offset >= 0x40000 && offset <= 0x7FFF0) {
#if 0
		int tmp;
		if (offset == 0x41020) {
			/* source attribute register for DMA0 */
			//printf("In %s,read gcr=0x%x\n", __FUNCTION__, *result);
			*result = io->pic_global.gcr & ~0x80000000;	/* we clear RST bit after finish initialization of PIC */
			//*result = io->pic_global.gcr & ~0x1;
			printf ("In %s,read gcr=0x%x, pc=0x%x\n",
				__FUNCTION__, *result, current_core->pc);
			return r;
		}

		r = e500_mpic_read_word (offset, &tmp);
		*result = tmp;
		return r;
#endif
		switch (offset) {
			case 0x400a0:
				return io->pic_global.iack;
			case 0x41000:
				return io->pic_global.frr;	/* according to MPC8572 manual */
			case 0x41020:
			/* source attribute register for DMA0 */
			//printf("In %s,read gcr=0x%x\n", __FUNCTION__, *result);                        
				return io->pic_global.gcr & ~0x80000000;	/* we clear RST bit after finish initialization of PIC */
			//printf("In %s,read gcr=0x%x, pc=0x%x\n", __FUNCTION__, *result, current_core->pc);
			case 0x410a0:
			case 0x410b0:
			case 0x410c0:
			case 0x410d0:
				return io->mpic.ipivpr[(offset - 0x410a0) >> 4];
			case 0x410e0:
				return io->pic_global.svr;
			case 0x410f0:
				return io->pic_global.tfrr;
			case 0x41120:
				return io->pic_global.gtvpr0;
			case 0x41160:
				return io->pic_global.gtvpr1;
			case 0x41170:
				return io->pic_global.gtdr1;
			case 0x411a0:
				return io->pic_global.gtvpr2;
			case 0x411B0:
				return io->pic_global.gtdr2;
			case 0x411E0:
				return io->pic_global.gtvpr3;
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
				return io->pic_ram.eidr[index >> 1];
			else
				return io->pic_ram.eivpr[index >> 1];
		}
		if (offset >= 0x50200 && offset <= 0x509F0) {
			int index = (offset - 0x50200) >> 4;
			if (index & 0x1)
				return io->mpic.iidr[index >> 1];
			else
				return io->mpic.iivpr[index >> 1];
		}
		/*
		   if(offset >= 0x50200 && offset <= 0x509F0){
		   int index = (offset - 0x50200) >> 4;
		   if(index & 0x1)
		   *result = io->pic_ram.iidr[index >> 1];
		   else
		   *result = io->pic_ram.iivpr[index >> 1];
		   return r;
		   }
		 */
		switch (offset) {
			case 0x60080:
				return io->pic_ram.ctpr0;
			case 0x60090:
				return 0;	/* Who as I register for core 0 */
			case 0x61090:
				return 0x1;	/* Who am I register for core 1 */
			case 0x600a0:
				return io->pic_percpu.iack[0];
			case 0x610a0:
				return io->pic_percpu.iack[1];
				//printf("In %s, ack=0x%x\n", __FUNCTION__, *result);
			default:
				break;
		}
		fprintf (stderr,
			 "in %s, error when read pic ram,offset=0x%x,pc=0x%x\n",
			 __FUNCTION__, offset, current_core->pc);

	}

	/* DMA */
	if (offset >= 0x21100 && offset <= 0x21300) {
		switch (offset) {
		case 0x21110:
			/* source attribute register for DMA0 */
			return io->dma.satr0;
		case 0x21118:
			return io->dma.satr0;
		default:
			fprintf (stderr, "in %s, error when read dma.offset=0x%x, \
                                pc=0x%x\n", __FUNCTION__, offset,
				 current_core->pc);
			return;
			//skyeye_exit(-1);

		}
	}
	/* Input/Output port */
	if (offset >= 0x90D00 && offset <= 0x90D70) {
		switch (offset) {

		default:
			fprintf (stderr, "in %s, error when read IO port.offset=0x%x, \
                                pc=0x%x\n", __FUNCTION__, offset,
				 current_core->pc);
			return;
			//skyeye_exit(-1);
		}
	}


	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
			case 0xE0000:
				return io->por_conf.porpllsr;
			case 0xE000C:
				return io->por_conf.pordevsr;
			default:
				fprintf (stderr,
					 "in %s, error when read CCSR.addr=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}

	switch (offset) {
		case 0x0:
			return cpu->ccsr;
		case 0x20:
			return cpu->bptr;
		case 0x1010:
			return cpu->eebpcr;
		case 0xC28:
			return io->law.lawbar[1];
		case 0xC30:
			return io->law.lawar[1];
		case 0x90C80:
			return io->sccr;
		case 0xe0e10:
			return io->debug_ctrl.ddrdllcr;
		case 0x50D4:
			return io->lb_ctrl.lcrr;
		case 0x20000:
			return io->l2_reg.l2ctl;
		case 0x8004:
			return io->pci_cfg.cfg_data;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			//skyeye_exit(-1);
	}
}
static void
mpc8641d_io_write_byte (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	if (offset >= 0xC08 && offset <= 0xCF0) {
		if (offset & 0x8) {
			io->law.lawbar[(offset - 0xC08) / 0x20] = data;
		}
		else {
			io->law.lawar[(offset - 0xC10) / 0x20] = data;
		}
		return;
	}
	switch (offset) {
		case 0x0:
			cpu->ccsr = data;
			break;
		case 0x3000:
			io->i2c_reg.i2cadr = data;
			break;
		case 0x3004:
			io->i2c_reg.i2cfdr = data;
			break;
		case 0x3008:
			io->i2c_reg.i2ccr = data;
			break;
		case 0x300C:
			io->i2c_reg.i2csr = data;
			break;
		case 0x3010:
			io->i2c_reg.i2cdr = data;
			/* set bit of MIF */
			io->i2c_reg.i2csr |= 0x02;
			break;
		case 0x3014:
			io->i2c_reg.i2cdfsrr = data;
			break;
		case 0x8004:
			io->pci_cfg.cfg_data = data;
			break;
		case 0x8005:
			io->pci_cfg.cfg_data = data;
			break;
		case 0x4500:
			skyeye_uart_write (-1, &data, 1, NULL);
			io->uart[0].lsr |= 0x60;	/* set TEMT and THRE bit */
			break;
		case 0x4501:
			//printf("In %s,write ier=0x%x\n", __FUNCTION__, data);
			if (io->uart[0].lcr & 0x80)
				io->uart[0].dmb = data;
			else
				io->uart[0].ier = data;
			break;
		case 0x4502:
			if (io->uart[0].lcr & 0x80)
				io->uart[0].afr = data;
			else
				io->uart[0].fcr = data;
			break;
		case 0x4503:
			io->uart[0].lcr = data;
			break;
		case 0x4504:
			io->uart[0].mcr = data;
			break;
		case 0x4507:
			io->uart[0].scr = data;
			break;
		case 0x4601:
			if (io->uart[1].lcr & 0x80)
				io->uart[1].dmb = data;
			else
				io->uart[1].ier = data;
			break;
		case 0x4602:
			if (io->uart[1].lcr & 0x80)
				io->uart[1].afr = data;
			else
				io->uart[1].fcr = data;
			break;
		case 0x4603:
			io->uart[1].lcr = data;
			break;
		case 0x4604:
			io->uart[1].mcr = data;
			break;
		case 0x4607:
			io->uart[1].scr = data;
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write to CCSR.addr=0x%x,offset=0x%x,ccsr=0x%x, pc=0x%x\n",
				 __FUNCTION__, offset, offset, cpu->ccsr, current_core->pc);
		//skyeye_exit(-1);
	}
}

static void
mpc8641d_io_write_halfword (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	if (offset >= 0xC08 && offset <= 0xCF0) {
		if (offset & 0x8) {
			io->law.lawbar[(offset - 0xC08) / 0x20] = data;
		}
		else {
			io->law.lawar[(offset - 0xC10) / 0x20] = data;
		}
		return;
	}
	if (offset >= 0x5000 && offset <= 0x5038) {
		io->lb_ctrl.br[(offset - 0x5000) / 0x8] = data;
		return;
	}
	if (offset >= 0x91A00 && offset <= 0x91A3F) {
		int i = (0x20 & offset) >> 5;
		offset = 0x1f & offset;
		switch (offset) {
			default:
				fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				skyeye_exit (-1);
		}
	}
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
			default:
				fprintf (stderr,
				 "in %s, error when write to CCSR.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
				//skyeye_exit(-1);
		}
	}
	if (offset >= 0x80000 && offset < 0x8C000) {
		//fprintf(prof_file,"DBG_CPM:in %s,offset=0x%x,data=0x%x,pc=0x%x\n",__FUNCTION__, offset, data, current_core->pc);
		return;
	}

	switch (offset) {
		case 0x0:
			cpu->ccsr = data;
			break;
		case 0x90C80:
			io->sccr = data;
			break;
		case 0x8004:
			io->pci_cfg.cfg_data = data;
			break;
		case 0x8006:
			io->pci_cfg.cfg_data = data;
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write to CCSR.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			//skyeye_exit(-1);
	}
}

static void
mpc8641d_io_write_word (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State* cpu = (PPC_CPU_State *)state;
        e500_core_t * core = &cpu->core[0];

        mpc8641d_io_t *io = &mpc8641d_io;

	if (offset >= 0xC08 && offset <= 0xCF0) {
		if (offset & 0x8) {
			io->law.lawbar[(offset - 0xC08) / 0x20] = data;
		}
		else {
			io->law.lawar[(offset - 0xC10) / 0x20] = data;
		}
	}

	if (offset >= 0x2000 && offset <= 0x2E58) {
		switch (offset) {
			case 0x2E44:
				io->ddr_ctrl.err_disable = data;
				break;
			default:
				fprintf (stderr,
					 "in %s, error when write ddr_ctrl,offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				skyeye_exit (-1);
		}
	}

	if (offset >= 0x5000 && offset <= 0x50D4) {
		if (offset >= 0x5000 && offset <= 0x5038) {
			io->lb_ctrl.br[(offset - 0x5000) / 0x8] = data;
		}
#if 0
		switch (offset) {
		case 0x50D0:
			current_core->lb_ctrl.lbcr = data;
			return r;
		default:
			fprintf (stderr, "in %s, error when read CCSR.addr=0x%x, \
                                pc=0x%x\n", __FUNCTION__, addr,
				 current_core->pc);

			skyeye_exit (-1);

		}
#endif

		fprintf (stderr, "in %s, error when write lb_ctrl.addr=0x%x, \
                                pc=0x%x\n", __FUNCTION__, offset, current_core->pc);

		return;
	}

	/* DMA */
	if (offset >= 0x21100 && offset <= 0x21300) {
		switch (offset) {
			case 0x21110:
				/* source attribute register for DMA0 */
				io->dma.satr0 = data;
				break;
			case 0x21118:
				io->dma.satr0 = data;
				break;
			default:
				fprintf (stderr, "in %s, error when write dma.addr=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				return;
				//skyeye_exit(-1);
		}
	}

	/* PIC Register map: global register */
	if (offset >= 0x40000 && offset <= 0x4FFF0) {
		switch (offset) {
			case 0x410a0:
			case 0x410b0:
			case 0x410c0:
			case 0x410d0:
				io->mpic.ipivpr[(offset - 0x410a0) >> 4] = data;
				//printf("In %s,offset=0x%x, data=0x%x\n", __FUNCTION__, offset, data);
				break;
			case 0x41020:
				/* source attribute register for DMA0 */
				io->pic_global.gcr = data;
				break;
			case 0x410e0:
				io->pic_global.svr = data;
				break;
			case 0x41120:
				io->pic_global.gtvpr0 = data;
				break;
			case 0x41130:
				io->pic_global.gtdr0 = data;
				break;
			case 0x41160:
				io->pic_global.gtvpr1 = data;
				break;
			case 0x41170:
				io->pic_global.gtdr1 = data;
				break;
			case 0x411a0:
				io->pic_global.gtvpr2 = data;
				break;
			case 0x411B0:
				io->pic_global.gtdr2 = data;
				break;
			case 0x411E0:
				io->pic_global.gtvpr3 = data;
				break;
			case 0x411F0:
				io->pic_global.gtdr3 = data;
				break;
			default:
				fprintf (stderr, "in %s, error when write mpic, offset=0x%x, \
					pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				return;
		}
		return;
	}
	/**
	 *  PIC Register Address Map
	 */
	if (offset >= 0x50000 && offset <= 0x5FFF0) {
		if (offset >= 0x50000 && offset <= 0x50170) {
			int index = (offset - 0x50000) >> 4;
			if (index & 0x1)
				io->pic_ram.eidr[index >> 1] = data;
			else
				io->pic_ram.eivpr[index >> 1] = data;
			return;
		}
		if (offset >= 0x50200 && offset <= 0x509F0) {
			int index = (offset - 0x50200) >> 4;
			if (index & 0x1)
				io->mpic.iidr[index >> 1] = data;
			else
				io->mpic.iivpr[index >> 1] = data;
			return;
		}
		if (offset >= 0x50180 && offset <= 0x501f0)	/* Reserved region for MPC8572 */
			return;
		if (offset >= 0x50100 && offset <= 0x515f0)	/* Reserved region for MPC8572 */
			return;

		if (offset >= 0x51600 && offset <= 0x51670) {
			int index = (offset - 0x51600) >> 4;
			if (index & 0x1)
				io->mpic.midr[index >> 1] = data;
			else
				io->mpic.mivpr[index >> 1] = data;
			return;
		}

		if (offset >= 0x51680 && offset <= 0x51bf0)	 /* Reserved region for MPC8641d*/
			return;

		if (offset >= 0x51c00 && offset <= 0x51cf0) {
			int index = (offset - 0x51c00) >> 4;
			if (index & 0x1)
				io->mpic.msidr[index >> 1] = data;
			else
				io->mpic.msivpr[index >> 1] = data;
			return;
		}

		if (offset >= 0x51d00 && offset <= 0x5fff0)	 /* Reserved region for MPC8641d*/
			return;

		fprintf (stderr,
			 "in %s, error when write pic ram,offset=0x%x,pc=0x%x\n",
			 __FUNCTION__, offset, current_core->pc);
		return;
	}

	/* per-CPU */
	if (offset >= 0x60000 && offset <= 0x7FFF0) {
		switch (offset) {
			case 0x60040:
				io->mpic.ipidr[0] = data;
				int core_id = -1;
				if (data & 0x1)	/* dispatch the interrupt to core 0 */
					core_id = 0;		
				if (data & 0x2)	/* dispatch the interrupt to core 1 */
					core_id = 1;
				if(data & 0x3){	
					/* trigger an interrupt to dedicated core */
					e500_core_t* core = &cpu->core[core_id];
				        core->ipr |= IPI0;
				        io->mpic.ipivpr[0] |= 0x40000000; /* set activity bit in vpr */
			        	//core->iack = (core->iack & 0xFFFF0000) | (gCPU.mpic.ipivpr[ipi_id] & 0xFFFF);
			        	io->pic_percpu.iack[core_id] = (io->pic_percpu.iack[core_id] & 0xFFFF0000) | (io->mpic.ipivpr[0] & 0xFFFF);
        //printf("In %s,iack=0x%x, pc=0x%x", __FUNCTION__, gCPU.pic_percpu.iack[core_id], core->pc);
			        	ppc_exception(core, EXT_INT, 0, 0);
			        	core->ipi_flag = 1; /* we need to inform the core that npc is changed to exception vector */
        //printf("In %s, npc=0x%x, pir=0x%x\n", __FUNCTION__, core->npc, core->pir);
				}
				return;
			case 0x60080:
				io->pic_ram.ctpr0 = data;
				return;
			case 0x600b0:
				io->pic_ram.eoi0 = data;
				if (current_core->ipr & (1 << UART_IRQ)) {
					current_core->ipr &= ~(1 << UART_IRQ);
					//printf("In %s, writing to eoi1 for core 0,clear int\n", __FUNCTION__);
				}
				/* clear the interrupt with highest priority in ISR */
				return;
			case 0x61080:
				io->pic_ram.ctpr1 = data;
				return;
			case 0x610b0:	/* processor 1 end of interrupt register */
				io->pic_ram.eoi1 = data;
				/* clear the interrupt with highest priority in ISR */
				//printf("In %s, writing to eoi1 for core 1\n", __FUNCTION__);
				return;
			default:
				fprintf (stderr, "in %s, error when write mpic, offset=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				return;

		}
	}

	/**
	 * Interrupt Controller
	 */

	if (offset >= 0x90C00 && offset <= 0x90C7F) {
		switch (offset) {
			default:
				fprintf (stderr,
					 "in %s, error when write interrupt controller,offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				return;
		}
	}

	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
			default:
				fprintf (stderr,
					 "in %s, error when write cpm_reg,offset=0x%x,pc=0x%x\n",
					 __FUNCTION__, offset, current_core->pc);
				return;
		}
	}
	if (offset >= 0x91A00 && offset <= 0x91A3F) {
		int i = (0x20 & offset) >> 5;
		offset = 0x1f & offset;
		switch (offset) {
			default:
				fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				skyeye_exit (-1);
		}
	}

	/* CPM MUX I/O */
	if (offset >= 0x91B00 && offset <= 0x91B1F) {
		switch (offset) {
			default:
				fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				skyeye_exit (-1);
		}
	}
	/* Input/Output port */
	if (offset >= 0x90D00 && offset <= 0x90D70) {
		switch (offset) {
			default:
				fprintf (stderr, "in %s, error when write io port.offset=0x%x, \
                	                pc=0x%x\n", __FUNCTION__, offset,
					 current_core->pc);
				return;
				//skyeye_exit(-1);
		}
	}
	
	if (offset >= 0x8C00 && offset <= 0x8DFC) {
		switch (offset) {
		case 0x8C20:
			io->pci_atmu.potar1 = data;
			return;
		case 0x8C24:
			io->pci_atmu.potear1 = data;
			return;
		case 0x8C28:
			io->pci_atmu.powbar1 = data;
			return;
		case 0x8C2C:
			io->pci_atmu.reserv1 = data;
			return;
		case 0x8C30:
			io->pci_atmu.powar1 = data;
			return;
		default:
			fprintf (stderr,
				 "in %s, error when write to PCI_ATMU.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			//skyeye_exit(-1);
			return;
		}
	}
	if (offset >= 0x40000 && offset <= 0x4FFF0) {
		switch (offset) {
		case 0x41020:
			/* source a
			   ttribute register for DMA0 */
			io->pic_global.gcr = data;
			return;
		case 0x410e0:
			io->pic_global.svr = data;
			return;
		case 0x41120:
			io->pic_global.gtvpr0 = data;
			return;
		case 0x41130:
			io->pic_global.gtdr0 = data;
			return;
		case 0x41160:
			io->pic_global.gtvpr1 = data;
			return;
		case 0x41170:
			io->pic_global.gtdr1 = data;
			return;
		case 0x411a0:
			io->pic_global.gtvpr2 = data;
			return;
		case 0x411B0:
			io->pic_global.gtdr2 = data;
			return;
		case 0x411E0:
			io->pic_global.gtvpr3 = data;
			return;
		case 0x411F0:
			io->pic_global.gtdr3 = data;
			return;
		default:
			fprintf (stderr,
				 "in %s, error when write global.offset=0x%x,pc=0x%x\n",
				 __FUNCTION__, offset, current_core->pc);
			return;
		}
	}

	switch (offset) {
	case 0x0:
		cpu->ccsr = data;
		break;
	case 0x20:
		//io->ccsr.bptr = data;
		cpu->bptr = data;
		//printf("In %s, write bptr=0x%x\n", __FUNCTION__, data);
		break;
	case 0x1010:
		//io->ecm.eebpcr = data;
		cpu->eebpcr = data;
		//printf("In %s, write eebpcr=0x%x\n", __FUNCTION__, data);
		if (data & 0x2000000)	/* enable CPU1 */
			cpu->core[1].pc = 0xFFFFF000;
		break;
	case 0x90C80:
		io->sccr = data;
		break;
	case 0x50D4:
		io->lb_ctrl.lcrr = data;
		return;
	case 0x3008:
		io->i2c_reg.i2ccr = data;
		return;
	case 0xe0e10:
		io->debug_ctrl.ddrdllcr = data;
		return;
	case 0x8000:
		io->pci_cfg.cfg_addr = data;
		return;
	case 0x8004:
		io->pci_cfg.cfg_data = data;
		return;
	case 0x41020:
		io->mpic.gcr = data;
		return;
	default:
		fprintf (stderr,
			 "in %s, error when write to CCSR.offset=0x%x,pc=0x%x\n",
			 __FUNCTION__, offset, current_core->pc);
		//skyeye_exit(-1);
	}

}
static void
mpc8641d_update_int (void *state)
{
}

static mpc8641d_set_intr(uint32_t irq){

}
void
mpc8641d_mach_init (void *arch_instance, machine_config_t * this_mach)
{
	//PPC_CPU_State *cpu = (PPC_CPU_State *) state;

	this_mach->mach_io_do_cycle = mpc8641d_io_do_cycle;
	this_mach->mach_io_reset = mpc8641d_io_reset;
	this_mach->mach_io_read_byte = mpc8641d_io_read_byte;
	this_mach->mach_io_write_byte = mpc8641d_io_write_byte;
	this_mach->mach_io_read_halfword = mpc8641d_io_read_halfword;
	this_mach->mach_io_write_halfword = mpc8641d_io_write_halfword;
	this_mach->mach_io_read_word = mpc8641d_io_read_word;
	this_mach->mach_io_write_word = mpc8641d_io_write_word;
	this_mach->mach_update_int = mpc8641d_update_int;
	this_mach->mach_set_intr = mpc8641d_set_intr;
	//mpc8641d_io.conf.ccsrbar = 0x000FF700;
	//cpu->core_num = 2;

}
