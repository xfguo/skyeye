/*
        skyeye_mach_mpc8560.c - mpc8560 machine simulation implementation
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
#include "ppc_irq.h"
#include "ppc_e500_exc.h"
#include "sysendian.h"
#include <ppc_cpu.h>

#ifdef __CYGWIN__
#include <sys/time.h>
#endif

#ifdef __MINGW32__
#include <sys/time.h>
#endif

typedef struct ccsr_reg_s
{
	//uint32_t ccsr; /* Configuration,control and status registers base address register */
	uint32_t altcbar;	/* allternate configuration base address register */
	uint32_t altcar;	/* alternate configuration attribute register */
	//uint32_t bptr; /* Boot page translation register */
} ccsr_reg_t;

/* Local bus controll register */
typedef struct lb_ctrl_s
{
	uint32 br[8];
	//uint32 or[8];
	uint32 lcrr;		/* Clock ratio register */
	//uint32 lbcr; /* Configuration register */
} lb_ctrl_t;

typedef struct law_reg_s
{
	uint32_t lawbar[8];
	uint32_t lawar[8];
} law_reg_t;

typedef struct por_conf_s
{
	uint32_t porpllsr;
	uint32_t porbmsr;
	uint32_t porimpscr;
	uint32_t pordevsr;
	uint32_t pordbgmsr;
	uint32_t gpporcr;
} por_conf_t;

/* 0x91a00-0x91a9f: SCC1-SCC4 */
typedef struct cpm_scc_s
{
	uint32 gsmrl;
	uint32 gsmrh;
	uint16 psmr;
	char res1[2];
	uint16 todr;
	uint16 dsr;
	uint16 scce;
	char res2[2];
	uint16 sccm;
	char res3;
	uint8 sccs;
	char res4[8];
} cpm_scc_t;

typedef struct cpm_mux_s
{
	uint32_t cmxfcr;
	uint32_t cmxscr;
} cpm_mux_t;

typedef struct cpm_ioport_s
{
	uint32_t pdira;
	uint32_t ppara;
	uint32_t psora;
	uint32_t podra;
	uint32_t pdata;
	uint32_t pdirb;
	uint32_t pparb;
	uint32_t psorb;
	uint32_t podrb;
	uint32_t pdatb;
	uint32_t pdirc;
	uint32_t pparc;
	uint32_t psorc;
	uint32_t podrc;
	uint32_t pdatc;
	uint32_t pdird;
	uint32_t ppard;
	uint32_t psord;
	uint32_t podrd;
	uint32_t pdatd;
} cpm_ioport_t;

typedef struct int_ctrl_s
{
	uint32 sicr;
	uint32 sipnr_h;
	uint32 sipnr_l;
	uint32 scprr_h;
	uint32 scprr_l;
	uint32 simr_h;
	uint32 simr_l;
} int_ctrl_t;

typedef struct cpm_reg_s
{
	uint32_t cpcr;
	uint32_t rccr;
	uint32_t rter;
	uint32_t rtmr;
	uint32_t rtscr;
	uint32_t rtsr;

	byte *dpram;
	byte *iram;		/* instruction RAM */
	cpm_scc_t scc[4];
	cpm_mux_t mux;
	uint32_t brgc[4];
	cpm_ioport_t ioport;
	int_ctrl_t int_ctrl;
} ppc_cpm_t;

typedef struct i2c_reg_s
{
	uint32 i2cadr;
	uint32 i2ccr;
	uint32 i2csr;
	uint32 i2cdr;
	uint32 i2cfdr;
	uint32 i2cdfsrr;
} i2c_reg_t;

typedef struct debug_ctrl_s
{
	uint32 clkocr;
	uint32 ddrdllcr;
	uint32 lbdrrcr;
} debug_ctrl_t;

typedef struct ppc_dma_s
{
	uint32 satr0;
	uint32 datr0;
} mpc_dma_t;


typedef struct ddr_ctrl_s
{
	uint32 err_disable;
} ddr_ctrl_t;

typedef struct l2_reg_s
{
	uint32 l2ctl;
} l2_reg_t;

typedef struct pci_cfg_s
{
	uint32 cfg_addr;
	uint32 cfg_data;
	uint32 int_ack;
} pci_cfg_t;

typedef struct pci_atmu_s
{
	uint32 potar1;
	uint32 potear1;
	uint32 powbar1;
	uint32 reserv1;
	uint32 powar1;
} pci_atmu_t;

typedef struct pic_global_s
{
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
} pic_global_t;

typedef struct pic_ram_s
{
	uint32 eivpr[11];
	uint32 eidr[11];
	uint32 ctpr0;
	uint32 iivpr[32];
	uint32 iidr[32];
} pic_ram_t;

typedef struct pic_percpu_s
{
	uint32 iack0;
} pic_percpu_t;

typedef struct mpc8560_io_s
{
	ccsr_reg_t ccsr;
	law_reg_t law;		/* Local access window */
	lb_ctrl_t lb_ctrl;	/* Local bus controll register */
	ppc_cpm_t cpm_reg;	/* Communication processor */
	uint32 sccr;		/* System clock control register */
	por_conf_t por_conf;
	i2c_reg_t i2c_reg;
	debug_ctrl_t debug_ctrl;
	ddr_ctrl_t ddr_ctrl;
	mpc_dma_t dma;
	l2_reg_t l2_reg;
	pci_cfg_t pci_cfg;
	pci_atmu_t pci_atmu;
	pic_global_t pic_global;
	int_ctrl_t int_ctrl;
	pic_ram_t pic_ram;
	pic_percpu_t pic_percpu;
} mpc8560_io_t;

static mpc8560_io_t mpc8560_io;
#define MPC8650_DPRAM_SIZE 0xC000


typedef struct bd_s
{
	uint16 flag;
	uint16 len;
	uint32 buf_addr;
} bd_t;

static int
scc1_io_do_cycle (void *state, ppc_cpm_t * cpm)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	byte *ram = &cpm->dpram[0];
	/* Param is stored at 0x8000 for SCC1 */
	int rx_base = 0x8000;	/* Receive buffer base address */
	int tx_base = 0x8002;	/* Transmit buffer base address */

	/* If SCC0 Receive enalbed */
	if (cpm->scc[0].gsmrl & 0x00000020) {
		//printf("In %s, cpm->scc[0].scce=0x%x\n", __FUNCTION__, cpm->scc[0].scce);
		/* If we already in recv interrupt, we go out */
		if (cpm->scc[0].scce & 0x1)
			goto out_of_recv;
		//printf("In %s,cpm->scc[0].sccm=0x%x\n", __FUNCTION__, cpm->scc[0].sccm);
		/* if interrupt is masked */
		if (!(cpm->scc[0].sccm & 0x1))
			goto out_of_recv;

		struct timeval tv;
		unsigned char buf;

		tv.tv_sec = 0;
		tv.tv_usec = 0;
		/* max idle cound ,when it become zero, rx bd will be closed */
#define MAX_IDLE_COUNT 10
		static int max_idle_count = MAX_IDLE_COUNT;
		uint32 buf_addr;
		short datlen;
		static int curr_rx_bd = 0;
		/* if max idle count not expire , we can still receive data */

		/* get a char from current uart */
		if (skyeye_uart_read (-1, &buf, 1, &tv, NULL) > 0) {
			//printf("In %s, get a char %c\n", __FUNCTION__, buf);
			/* build a bd for rx */
			int recv_bd_base =
				ppc_half_from_BE (*(uint16 *) & ram[rx_base]);
			short bd_flag;

			cpm->scc[0].scce |= 0x1;	/* set RX bit */
			/*if int is masked and scc1 interrupt is masked */
			if (!(cpm->int_ctrl.simr_l & 0x10)) {
				cpm->int_ctrl.sipnr_l &= 0x10;	/* set pending bit in SIPNR_L */
				buf_addr =
					ppc_word_from_BE (*
							  ((uint32 *) &
							   ram[recv_bd_base +
							       curr_rx_bd +
							       4]));
				bus_write(8, buf_addr, buf);
				/* Now we only implement that send a char once */
				*((sint16 *) &ram[recv_bd_base + curr_rx_bd + 2]) = ppc_half_to_BE (0x1);

				bd_flag =
					ppc_half_from_BE (*(sint16 *) &
							  ram[recv_bd_base +
							      curr_rx_bd]);
				/* set empty bit to zero ,waiting core to read the date */
				bd_flag &= ~0x8000;
				*((sint16 *) & ram[recv_bd_base + curr_rx_bd])
					= ppc_half_to_BE (bd_flag);
				/* judge empty bit, if current bd not available, we use next bd */
				if (bd_flag & 0x2000)	/* check wrap bit */
					curr_rx_bd = 0;	/* Reset  pointer to the begin addr of the bd table */
				else
					curr_rx_bd += 8;	/* indicate to next BD */

				io->pic_percpu.iack0 = SIU_INT_SCC1;

				core->ipi_flag = 1;	/* we need to inform the core that npc is changed to exception vector */
				/* trigger interrupt */
				ppc_exception (core, EXT_INT, 0, 0);
			}
		}
	}
      	out_of_recv:
	/* If SCC0 transmit enabled */
	if (cpm->scc[0].gsmrl & 0x00000010) {
		static int curr_tx_bd = 0;
		int trans_bd_base =
			ppc_half_from_BE (*(uint16 *) & ram[tx_base]);
		short bd_flag =
			ppc_half_from_BE (*(sint16 *) &
					  ram[trans_bd_base + curr_tx_bd]);
		short bd_len =
			ppc_half_from_BE (*(sint16 *) &
					  ram[trans_bd_base + curr_tx_bd +
					      2]);
		uint32 buf_addr =
			ppc_word_from_BE (*
					  ((uint32 *) &
					   ram[trans_bd_base + curr_tx_bd +
					       4]));
		//fprintf(prof_file, "trans_bd_base=0x%x,bd_flag=0x%x,curr_tx_bd=0x%x,bd_len=0x%x,buf_addr=0x%x\n",trans_bd_base, bd_flag, curr_tx_bd, bd_len, buf_addr);
		/* If data ready */
		if (bd_flag & 0x8000) {
			int i = 0;
			for (; i < bd_len; i++) {
				char c;
				bus_read(8, buf_addr + i, &c);
				skyeye_uart_write (-1, &c, 1, NULL);
			}

			/* set Empty bit */
			*((sint16 *) & ram[trans_bd_base + curr_tx_bd]) &=
				ppc_half_to_BE (~0x8000);
			if (bd_flag & 0x2000)	/* check wrap bit */
				curr_tx_bd = 0;	/* Reset  pointer to the begin addr of the bd table */
			else
				curr_tx_bd += 8;	/* indicate to next BD */
		}
	}
}

static void
mpc8560_io_do_cycle (void *state)
{
	mpc8560_io_t *io = &mpc8560_io;
	scc1_io_do_cycle (state, &io->cpm_reg);	/* MPC8560 */
}
static void
mpc8560_io_reset (void *state){
	mpc8560_io_t *io = &mpc8560_io;
	/* Just for convience of boot linux */
	//io->conf.ccsrbar = 0x000E0000;
	io->por_conf.porpllsr = 0x40004;
	io->lb_ctrl.lcrr = 0x80000008;
	io->i2c_reg.i2csr = 0x81;
	io->i2c_reg.i2cdfsrr = 0x10;
	io->pic_ram.ctpr0 = 0x0000000F;
	io->pic_global.svr = 0xFFFF;

	io->cpm_reg.dpram = (void *) malloc (MPC8650_DPRAM_SIZE);
	if (!io->cpm_reg.dpram) {
		printf ("malloc failed for dpram\n");
		skyeye_exit (-1);
	}


	io->pic_global.frr = 0x370002;
	/* initialize interrupt controller */
	int i = 0;
	for (; i < 32; i++) {
		io->pic_ram.iidr[i] = 0x1;
		io->pic_ram.iivpr[i] = 0x80800000;
	}
	mpc8560_boot_linux();	
}
static uint32_t
mpc8560_io_read_byte (void *state, uint32_t offset)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		case 0x919C0:
			return io->cpm_reg.cpcr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x, pc=0x%x\n",
				 __FUNCTION__, offset, core->pc);
			skyeye_exit (-1);
		}
	}
	if (offset >= 0x80000 && offset < 0x8C000) {
		return *((sint16 *) & io->cpm_reg.dpram[offset - 0x80000]);
		//printf("DBG_CPM:in %s,offset=0x%x,data=0x%x,pc=0x%x\n",__FUNCTION__, offset, *result,io->pc);
	}

	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
		case 0xE0000:
			return io->por_conf.porpllsr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.addr=0x%x,pc=0x%x\n", __FUNCTION__, offset, core->pc);
			//skyeye_exit (-1);
		}

	}
	switch (offset) {
	case 0x0:
		return cpu->ccsr;
	case 0x90C80:
		return io->sccr;
	case 0x300C:
		return io->i2c_reg.i2csr;
		/*
		fprintf (prof_file, "KSDBG:read i2csr result=0x%x\n",
			 *result);
		*/
	case 0x8004:
	case 0x8005:
	case 0x8006:
		return io->pci_cfg.cfg_data;
	default:
		fprintf(stderr,"in %s, error when read CCSR.offset=0x%x, pc=0x%x\n",__FUNCTION__,offset, core->pc);
		//skyeye_exit(-1);
	}

}
static uint32_t
mpc8560_io_read_halfword (void *state, uint32_t offset)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		case 0x919C0:
			return io->cpm_reg.cpcr;
		default:
			
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x\n",
				 __FUNCTION__, offset);
			
			skyeye_exit (-1);
		}
	}
	if ((offset >= 0x80000) && (offset < 0x8C000)) {
		return	ppc_half_from_BE (*
					  ((sint16 *) & io->cpm_reg.
					   dpram[offset - 0x80000]));
	}
	if (offset >= 0x91A00 && offset <= 0x91A3F) {
		int i = (0x20 & offset) >> 5;
		offset = 0x1f & offset;
		switch (offset) {
		case 0x0:
			return io->cpm_reg.scc[i].gsmrl;
		case 0x4:
			return io->cpm_reg.scc[i].gsmrh;
		case 0x8:
			return io->cpm_reg.scc[i].psmr;
		case 0xE:
			return io->cpm_reg.scc[i].dsr;
		case 0x14:
			return io->cpm_reg.scc[i].sccm;

		case 0x10:	/* W1C */
			return io->cpm_reg.scc[i].scce;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit (-1);

		}
	}

	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
		case 0xE0000:
			return io->por_conf.porpllsr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x\n",
				 __FUNCTION__, offset);
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
			 "in %s, error when read CCSR.offset=0x%x\n",
			 __FUNCTION__, offset);
		//skyeye_exit(-1);
	}

}
static uint32_t
mpc8560_io_read_word (void *state, uint32_t offset)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		case 0x919C0:
			return io->cpm_reg.cpcr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x\n",
				 __FUNCTION__, offset);
			skyeye_exit (-1);
		}
	}
	if (offset >= 0x2000 && offset <= 0x2E58) {
		switch (offset) {
		case 0x2E44:
			return io->ddr_ctrl.err_disable;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.offset=0x%x\n",
				 __FUNCTION__, offset);
			skyeye_exit (-1);
		}
	}

	/**
         *  PIC Register Address Map
         */
	if (offset >= 0x50000 && offset <= 0x600B0) {
		if (offset >= 0x50000 && offset <= 0x50170) {
			int index = (offset - 0x50000) >> 4;
			if (index & 0x1)
				return io->pic_ram.eidr[index >> 1];
			else
				return io->pic_ram.eivpr[index >> 1];
		}
		if (offset >= 0x50200 && offset <= 0x505F0) {
			int index = (offset - 0x50200) >> 4;
			if (index & 0x1)
				return io->pic_ram.iidr[index >> 1];
			else
				return io->pic_ram.iivpr[index >> 1];
		}

		switch (offset) {
		case 0x60080:
			return io->pic_ram.ctpr0;
		case 0x600a0:
			return io->pic_percpu.iack0;
		default:
			fprintf (stderr,
				 "in %s, error when write pic ram,offset=0x%x\n",
				 __FUNCTION__, offset);
			break;
		}
	}

			/**
                         * Interrupt Controller
                         */

	if (offset >= 0x90C00 && offset <= 0x90C7F) {
		switch (offset) {
		case 0x90C08:
			return io->int_ctrl.sipnr_h;
		case 0x90C0C:
			return io->int_ctrl.sipnr_l;
		case 0x90C14:
			return io->int_ctrl.scprr_h;
		case 0x90C18:
			return io->int_ctrl.scprr_l;
		case 0x90C1C:
			return io->int_ctrl.simr_h;
		case 0x90C20:
			return io->int_ctrl.simr_l;
		default:
			fprintf (stderr,
				 "in %s, error when read interrupt controller,offset=0x%x\n",
				 __FUNCTION__, offset);
			return 0;
		}
	}

	if (offset >= 0x91A00 && offset <= 0x91A3F) {
		int i = (0x20 & offset) >> 5;
		offset = 0x1f & offset;
		switch (offset) {
		case 0x0:
			return io->cpm_reg.scc[i].gsmrl;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);

			skyeye_exit (-1);
		}
	}

	/* CPM MUX I/O */
	if (offset >= 0x91B00 && offset <= 0x91B1F) {
		switch (offset) {
		case 0x91B08:
			return io->cpm_reg.mux.cmxscr;
		case 0x91B04:
			return io->cpm_reg.mux.cmxfcr;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit (-1);

		}
	}
	/* PIC Global register */
	if (offset >= 0x40000 && offset <= 0x4FFF0) {
		switch (offset) {
		case 0x41000:
			return io->pic_global.frr;	/* according to 8560 manual */
		case 0x400a0:
			return io->pic_global.iack;
		case 0x410f0:
			return io->pic_global.tfrr;
		case 0x41020:
			/* source attribute register for DMA0 */
			return io->pic_global.gcr;
		case 0x410e0:
			return io->pic_global.svr;
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
			fprintf (stderr, "in %s, error when read global.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit(-1);
		}
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
                                \n", __FUNCTION__, offset);
			return;
			//skyeye_exit(-1);
		}
	}
	/* Input/Output port */
	if (offset >= 0x90D00 && offset <= 0x90D70) {
		switch (offset) {
		case 0x90D00:
			return io->cpm_reg.ioport.pdira;
		case 0x90D04:
			return io->cpm_reg.ioport.ppara;
		case 0x90D08:
			return io->cpm_reg.ioport.psora;
		case 0x90D0C:
			return io->cpm_reg.ioport.podra;
		case 0x90D10:
			return io->cpm_reg.ioport.pdata;
		case 0x90D20:
			return io->cpm_reg.ioport.pdirb;
		case 0x90D24:
			return io->cpm_reg.ioport.pparb;
		case 0x90D28:
			return io->cpm_reg.ioport.psorb;
		case 0x90D40:
			return io->cpm_reg.ioport.pdirc;
		case 0x90D44:
			return io->cpm_reg.ioport.pparc;
		case 0x90D48:
			return io->cpm_reg.ioport.psorc;
		case 0x90D60:
			return io->cpm_reg.ioport.pdird;
		case 0x90D64:
			return io->cpm_reg.ioport.ppard;
		case 0x90D68:
			return io->cpm_reg.ioport.psord;
		default:
			fprintf (stderr, "in %s, error when read IO port.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			//return r;
			skyeye_exit(-1);

		}
	}

	if (offset >= 0x80000 && offset < 0x8C000) {
		return	
			ppc_word_from_BE (*((sint32 *) & io->cpm_reg.
					   dpram[offset - 0x80000]));
		//printf("DBG_CPM:in %s,offset=0x%x,data=0x%x,pc=0x%x\n",__FUNCTION__, offset, *result,io->pc);
	}

	if (offset >= 0xE0000 && offset <= 0xE0020) {
		switch (offset) {
		case 0xE0000:
			return io->por_conf.porpllsr;
		case 0xE000C:
			return io->por_conf.pordevsr;
		default:
			fprintf (stderr,
				 "in %s, error when read CCSR.addr=0x%x\n",
				 __FUNCTION__, offset);
			skyeye_exit (-1);
		}
	}
	switch (offset) {
	case 0x0:
		return cpu->ccsr;
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
			 "in %s, error when read CCSR.offset=0x%x\n",
			 __FUNCTION__, offset);
		//skyeye_exit(-1);
	}

}
static void
mpc8560_io_write_byte (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	//printf("DBG:write to CCSR,value=0x%x,offset=0x%x\n", data, offset);
	if (offset >= 0xC08 && offset <= 0xCF0) {
		if (offset & 0x8) {
			io->law.lawbar[(offset - 0xC08) / 0x20] = data;
		}
		else {
			io->law.lawar[(offset - 0xC10) / 0x20] = data;
		}
		return;
	}
	if (offset >= 0x80000 && offset < 0x8C000) {
		//fprintf(prof_file,"DBG_CPM:in %s,offset=0x%x,data=0x%x,\n",__FUNCTION__, offset, data);

		*((byte *) & io->cpm_reg.dpram[offset - 0x80000]) = data;
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
	case 0x8005:
		io->pci_cfg.cfg_data = data;
		break;
	default:
		fprintf (stderr,
			 "in %s, error when write to CCSR.addr=0x%x\n",
			 __FUNCTION__, offset);
		skyeye_exit (-1);
	}
}
static void
mpc8560_io_write_halfword (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
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
		//printf("In %s, offset = 0x%x,data=0x%x, pc=0x%x\n", __FUNCTION__, offset, data, core->pc);
		switch (offset) {
		case 0x0:
			io->cpm_reg.scc[i].gsmrl = data;
			break;
		case 0x4:
			io->cpm_reg.scc[i].gsmrh = data;
			break;
		case 0x8:
			io->cpm_reg.scc[i].psmr = data;
			break;
		case 0xE:
			io->cpm_reg.scc[i].dsr = data;
			break;
		case 0x14:
			io->cpm_reg.scc[i].sccm = data;
			break;
		case 0x10:	/* W1C */
			io->cpm_reg.scc[i].scce &= ~data;
			break;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit (-1);
		}
		return;
	}
	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		case 0x919C0:
			io->cpm_reg.cpcr = data;
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write to CCSR.offset=0x%x\n",
				 __FUNCTION__, offset);
			skyeye_exit(-1);
		}
		return;
	}
	if (offset >= 0x80000 && offset < 0x8C000) {
		//fprintf(prof_file,"DBG_CPM:in %s,offset=0x%x,data=0x%x,\n",__FUNCTION__, offset, data);
		*((sint16 *) & io->cpm_reg.dpram[offset - 0x80000]) = ppc_half_to_BE (data);
		return;
	}

	switch (offset) {
	case 0x0:
		cpu->ccsr = data;
		break;
	case 0x90C00:
		io->int_ctrl.sicr = data;
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
			 "in %s, error when write to CCSR.offset=0x%x,\n",
			 __FUNCTION__, offset);
		//skyeye_exit(-1);
		return;
	}
}
static void
mpc8560_io_write_word (void *state, uint32_t offset, uint32_t data)
{
	PPC_CPU_State *cpu = (PPC_CPU_State *) state;
	e500_core_t *core = &cpu->core[0];

	mpc8560_io_t *io = &mpc8560_io;
	if (offset >= 0xC08 && offset <= 0xCF0) {
		if (offset & 0x8) {
			io->law.lawbar[(offset - 0xC08) / 0x20] = data;
		}
		else {
			io->law.lawar[(offset - 0xC10) / 0x20] = data;
		}
		return;
	}

	if (offset >= 0x2000 && offset <= 0x2E58) {
		switch (offset) {
		case 0x2E44:
			io->ddr_ctrl.err_disable = data;
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write ddr_ctrl,offset=0x%x,\n",
				 __FUNCTION__, offset);
			skyeye_exit (-1);
		}
		return;
	}

	if (offset >= 0x5000 && offset <= 0x50D4) {
		if (offset >= 0x5000 && offset <= 0x5038) {
			io->lb_ctrl.br[(offset - 0x5000) / 0x8] = data;
			return;
		}
#if 0
		switch (offset) {
		case 0x50D0:
			io->lb_ctrl.lbcr = data;
			return r;
		default:
			fprintf (stderr, "in %s, error when read CCSR.addr=0x%x, \
                                pc=0x%x\n", __FUNCTION__, addr,
				 io->pc);

			skyeye_exit (-1);

		}
#endif

		fprintf (stderr, "in %s, error when write lb_ctrl.addr=0x%x, \
                                \n", __FUNCTION__, offset);
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
			fprintf (stderr, "in %s, error when write dma.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit(-1);
		}
		return;
	}

			/**
 			 *  PIC Register Address Map
			 */
	if (offset >= 0x50000 && offset <= 0x600B0) {
		if (offset >= 0x50000 && offset <= 0x50170) {
			int index = (offset - 0x50000) >> 4;
			if (index & 0x1)
				io->pic_ram.eidr[index >> 1] = data;
			else
				io->pic_ram.eivpr[index >> 1] = data;
			return;
		}
		if (offset >= 0x50200 && offset <= 0x505F0) {
			int index = (offset - 0x50200) >> 4;
			if (index & 0x1)
                                io->pic_ram.iidr[index >> 1] = data;
                        else
                                io->pic_ram.iivpr[index >> 1] = data;

			/* do nothing */
			//fprintf(stderr, "In %s, not implement at 0x%x\n", __FUNCTION__, offset);
			return;
		}

		switch (offset) {
		case 0x60080:
			io->pic_ram.ctpr0 = data;
			return;
		default:
			fprintf (stderr,
				 "in %s, error when write pic ram,offset=0x%x\n",
				 __FUNCTION__, offset);
			return;
		}
	}

			/**
			 * Interrupt Controller
			 */

	if (offset >= 0x90C00 && offset <= 0x90C7F) {
		switch (offset) {
		case 0x90C08:	/* W1C */
			io->int_ctrl.sipnr_h &= ~data;
			break;
		case 0x90C0C:	/* W1C */
			io->int_ctrl.sipnr_l &= ~data;
			break;
		case 0x90C14:
			io->int_ctrl.scprr_h = data;
			break;
		case 0x90C18:
			io->int_ctrl.scprr_l = data;
			break;
		case 0x90C1C:
			io->int_ctrl.simr_h = data;
			break;
		case 0x90C20:
			io->int_ctrl.simr_l = data;
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write interrupt controller,offset=0x%x\n",
				 __FUNCTION__, offset);
			skyeye_exit(-1);
		}
		return;
	}

	if (offset >= 0x919C0 && offset <= 0x919E0) {
		switch (offset) {
		case 0x919C0:
			io->cpm_reg.cpcr = data;
			/* set FLG bit to zero, that means we are ready for new command */
			/* get sub block code */
			if ((0x1f & (io->cpm_reg.cpcr >> 21)) == 0x4) {
				;	/* we */
				if ((0xf & io->cpm_reg.cpcr) == 0x0) {
					/* INIT Rx and Tx Param in SCC1 */
				}
			}
			io->cpm_reg.cpcr &= ~(1 << 16);
			break;
		default:
			fprintf (stderr,
				 "in %s, error when write cpm,offset=0x%x\n",
				 __FUNCTION__, offset);
		}
		return;
	}
	if (offset >= 0x91A00 && offset <= 0x91A3F) {
		int i = (0x20 & offset) >> 5;
		int scc_offset = 0x1f & offset;
		//printf("In %s, offset = 0x%x,data=0x%x, pc=0x%x\n", __FUNCTION__, scc_offset, data, core->pc);

		switch (scc_offset) {
		case 0x0:
			io->cpm_reg.scc[i].gsmrl = data;
			if (io->cpm_reg.scc[i].gsmrl & 0x00000020);	/* Enable Receive */
			if (io->cpm_reg.scc[i].gsmrl & 0x00000010);	/* Enable Transmit */
			
			break;
		case 0x4:
			io->cpm_reg.scc[i].gsmrh = data;
			break;
		case 0x8:
			io->cpm_reg.scc[i].psmr = data;
			break;
		case 0xE:
			io->cpm_reg.scc[i].dsr = data;
			break;
		case 0x14:
			io->cpm_reg.scc[i].sccm = data;
			break;
		case 0x10:	/* W1C */
			io->cpm_reg.scc[i].scce &= ~data;
			break;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit (-1);
		}
		return;
	}

	/* CPM MUX I/O */
	if (offset >= 0x91B00 && offset <= 0x91B1F) {
		switch (offset) {
		case 0x91B04:
			io->cpm_reg.mux.cmxfcr = data;
			break;
		case 0x91B08:
			io->cpm_reg.mux.cmxscr = data;
			break;
		default:
			fprintf (stderr, "in %s, error when read CCSR.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			skyeye_exit (-1);
		}
		return;
	}
	/* Input/Output port */
	if (offset >= 0x90D00 && offset <= 0x90D70) {
		switch (offset) {
		case 0x90D00:
			io->cpm_reg.ioport.pdira = data;
			break;
		case 0x90D04:
			io->cpm_reg.ioport.ppara = data;
			break;
		case 0x90D08:
			io->cpm_reg.ioport.psora = data;
			break;
		case 0x90D0C:
			io->cpm_reg.ioport.podra = data;
			break;
		case 0x90D10:
			io->cpm_reg.ioport.pdata = data;
			break;
		case 0x90D20:
			io->cpm_reg.ioport.pdirb = data;
			break;
		case 0x90D24:
			io->cpm_reg.ioport.pparb = data;
			break;
		case 0x90D28:
			io->cpm_reg.ioport.psorb = data;
			break;
		case 0x90D40:
			io->cpm_reg.ioport.pdirc = data;
			break;
		case 0x90D44:
			io->cpm_reg.ioport.pparc = data;
			break;
		case 0x90D48:
			io->cpm_reg.ioport.psorc = data;
			break;
		case 0x90D60:
			io->cpm_reg.ioport.pdird = data;
			break;
		case 0x90D64:
			io->cpm_reg.ioport.ppard = data;
			break;
		case 0x90D68:
			io->cpm_reg.ioport.psord = data;
			break;
		default:
			fprintf (stderr, "in %s, error when write io port.offset=0x%x, \
                                \n", __FUNCTION__, offset);
			//skyeye_exit(-1);

		}
		return;
	}
	/* BRG */
	if (offset >= 0x919F0 && offset <= 0x919FC) {
		io->cpm_reg.brgc[(offset - 0x919F0) / 4] = data;
		return;
	}
	if (offset >= 0x80000 && offset < 0x8C000) {
		//fprintf(prof_file,"DBG_CPM:in %s,offset=0x%x,data=0x%x,\n",__FUNCTION__, offset, data);

		*((sint32 *) & io->cpm_reg.dpram[offset - 0x80000]) =
			ppc_word_to_BE (data);
		return;
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
				 "in %s, error when write to PCI_ATMU.offset=0x%x\n",
				 __FUNCTION__, offset);
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
				 "in %s, error when write global.offset=0x%x\n",
				 __FUNCTION__, offset);
			return;
			//skyeye_exit (-1);

		}

	}

	switch (offset) {
	case 0x0:
		cpu->ccsr = data;
		break;
	case 0x90C80:
		io->sccr = data;
		break;
	case 0x50D4:
		io->lb_ctrl.lcrr = data;
		break;
	case 0x3008:
		io->i2c_reg.i2ccr = data;
		break;
	case 0xe0e10:
		io->debug_ctrl.ddrdllcr = data;
		break;
	case 0x8000:
		io->pci_cfg.cfg_addr = data;
		break;
	case 0x8004:
		io->pci_cfg.cfg_data = data;
		break;
	default:
		fprintf (stderr,
			 "in %s, error when write to CCSR.offset=0x%x\n",
			 __FUNCTION__, offset);
		//skyeye_exit(-1);
	}
}

static void
mpc8560_update_int (void *state)
{
}


void
mpc8560_mach_init (void *arch_instance, machine_config_t * this_mach)
{
	//PPC_CPU_State *cpu = (PPC_CPU_State *) state;

	this_mach->mach_io_do_cycle = mpc8560_io_do_cycle;
	this_mach->mach_io_reset = mpc8560_io_reset;
	this_mach->mach_io_read_byte = mpc8560_io_read_byte;
	this_mach->mach_io_write_byte = mpc8560_io_write_byte;
	this_mach->mach_io_read_halfword = mpc8560_io_read_halfword;
	this_mach->mach_io_write_halfword = mpc8560_io_write_halfword;
	this_mach->mach_io_read_word = mpc8560_io_read_word;
	this_mach->mach_io_write_word = mpc8560_io_write_word;
	this_mach->mach_update_int = mpc8560_update_int;
	//mpc8560_io.conf.ccsrbar = 0x000FF700;

	//cpu->core_num = 1;

}
