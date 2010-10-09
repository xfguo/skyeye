/*
skyeye_mach_s3c4510b.c - define machine s3c4510b for skyeye
Copyright (C) 2003 Skyeye Develop Group
for help please send mail to <skyeye-developer@lists.gro.clinux.org>

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
* 06/20/2005	move ethernet simulation to dev_net_s3c4510b.c
* 		walimis <wlm@student.dlut.edu.cn>
*
* 9/05/2004	add ethernet controller support.
*		get patch from <telpro2003@yahoo.com.cn>
* 		walimis <wlm@student.dlut.edu.cn>
*
* 10/05/2003	correct interrupt support. now it can boot uclinux to invoke shell.
* 		walimis <wlm@student.dlut.edu.cn>
*
* 7/21/2003	correct timer support.
* 		now it can boot uclinux.
* 		walimis <wlm@student.dlut.edu.cn>
*
* 7/17/2003	add interrupt, timer and uart support.
* 		walimis <wlm@student.dlut.edu.cn>
*
* 3/28/2003 	init this file.
* 		add machine s3c4510b's function.Most taken from original armio.c
* 		include: s3c4510b_mach_init, s3c4510b_io_do_cycle
* 		s3c4510b_io_read_word, s3c4510b_io_write_word
*		walimis <walimi@peoplemail.com.cn>
*
* */

#include <skyeye_config.h>
#include <skyeye_arch.h>
#include <skyeye_sched.h>
#include <skyeye_lock.h>
#include "s3c4510b.h"
//zzc:2005-1-1
#ifdef __CYGWIN__
//chy 2005-07-28
#include <time.h>
//teawater add DBCT_TEST_SPEED 2005.10.04---------------------------------------
/*struct timeval
{
int tv_sec;
int tv_usec;
};*/
//AJ2D--------------------------------------------------------------------------
#endif /*  */

/* 2007-01-18 added by Anthony Lee : for new uart device frame */
#include "skyeye_uart.h"

uint32_t s3c4510b_io_read_word (void *state, uint32_t addr);
void s3c4510b_io_write_word (void *state, uint32_t addr, uint32_t data);

/* s3c4510b Internal IO Registers
* */
typedef struct s3c4510b_io
{

	/*System Manager control */
	uint32_t syscfg;
	uint32_t clkcon;

	/*Interrupt Controller Registers */
	uint32_t intmod;
	uint32_t intpnd;
	uint32_t intmsk;
	uint32_t intoffset;
	uint32_t intpndtst;
	uint32_t intoset_fiq;
	uint32_t intoset_irq;

	/*UART Registers */
	uint32_t ulcon0;
	uint32_t ulcon1;
	uint32_t ucon0;
	uint32_t ucon1;
	uint32_t ustat0;
	uint32_t ustat1;
	uint32_t utxbuf0;
	uint32_t utxbuf1;
	uint32_t urxbuf0;
	uint32_t urxbuf1;
	uint32_t ubrdiv0;
	uint32_t ubrdiv1;

	/*Timers Registers */
	uint32_t tmod;
	uint32_t tdata0;
	uint32_t tdata1;
	int tcnt0;
	int tcnt1;
} s3c4510b_io_t;
static s3c4510b_io_t s3c4510b_io;

#define io s3c4510b_io
#define ENABLE_TIMER0 io.tmod & 0x1
#define ENABLE_TIMER1 io.tmod & 0x8

//extern int skyeye_net_on;
static unsigned char mac_buf[4096];
static void
s3c4510b_update_int (void *state)
{
	uint32_t requests = io.intpnd & (~io.intmsk & INT_MASK_INIT);
#if 0
	state->NfiqSig = (requests & io.intmod) ? LOW : HIGH;
	state->NirqSig = (requests & ~io.intmod) ? LOW : HIGH;
#endif
	interrupt_signal_t interrupt_signal;
	interrupt_signal.arm_signal.firq = (requests & io.intmod) ? Low_level : High_level;
	interrupt_signal.arm_signal.irq = (requests & ~io.intmod) ? Low_level : High_level;
	interrupt_signal.arm_signal.reset = Prev_level;
	send_signal(&interrupt_signal);
}

static void
s3c4510b_set_interrupt (unsigned int irq)
{
	io.intpnd |= (1 << irq);
}

static int
s3c4510b_pending_intr (uint32_t interrupt)
{
	return ((io.intpnd & (1 << interrupt)));
}

static void
s3c4510b_update_intr (void *mach)
{
	struct machine_config *mc = (struct machine_config *) mach;
	void *state = (void *) mc->state;
	s3c4510b_update_int (state);
}

static void
s3c4510b_io_reset (void *state)
{
	memset (&s3c4510b_io, 0, sizeof (s3c4510b_io));
	io.syscfg = 0x37ffff91;
	io.intmsk = INT_MASK_INIT;
	io.intoffset = io.intoset_fiq = io.intoset_irq = 0x00000054;
	io.ustat0 = io.ustat1 = 0xc0;
	io.tcnt0 = io.tcnt1 = 0xffffffff;
};

void
s3c4510b_io_do_cycle (void *state)
{
	/*Timer */
	if (ENABLE_TIMER0) {
		io.tcnt0--;
		if (io.tcnt0 < 0) {
			io.tcnt0 = io.tdata0;
			s3c4510b_set_interrupt (INT_TIMER0);
			s3c4510b_update_int (state);
			return;
		}
	}
	if (ENABLE_TIMER1) {
		if (--io.tcnt1 < 0) {
			io.tcnt1 = io.tdata1;
			s3c4510b_set_interrupt (INT_TIMER1);
			s3c4510b_update_int (state);
		}
	}
	 /*UART*/
	{
		/* 2007-01-18 modified by Anthony Lee : for new uart device frame */
		struct timeval tv;
		unsigned char buf;

		tv.tv_sec = 0;
		tv.tv_usec = 0;

		if(skyeye_uart_read(-1, &buf, 1, &tv, NULL) > 0)
		{
			//ctrl_c support,replace by ctrl_a
			if (buf == 1) buf = 3;
			io.urxbuf0 = io.urxbuf1 = (int) buf;
			io.ustat0 |= UART_LSR_DR;
			io.ustat1 |= UART_LSR_DR;
			if (!(io.intpnd & (1 << INT_UARTRX0))
			    || !(io.intpnd & (1 << INT_UARTRX1))) {
				if ((io.ucon0 & 0x3) == 0x1) {
					s3c4510b_set_interrupt (INT_UARTRX0);
					s3c4510b_update_int (state);
					return;
				}
				if ((io.ucon1 & 0x3) == 0x1) {
					s3c4510b_set_interrupt (INT_UARTRX1);
					s3c4510b_update_int (state);
					return;
				}
			}
		}
	}

	s3c4510b_update_int (state);
}

uint32_t
s3c4510b_io_read_byte (void *state, uint32_t addr)
{

	//printf("SKYEYE: s3c4510b_io_read_byte error\n");
	s3c4510b_io_read_word (state, addr);
}

uint32_t
s3c4510b_io_read_halfword (void *state, uint32_t addr)
{

	//printf("SKYEYE: s3c4510b_io_read_halfword error\n");
	s3c4510b_io_read_word (state, addr);
}

uint32_t
s3c4510b_io_read_word (void *state, uint32_t addr)
{
	uint32_t data = -1;
	switch (addr) {
	case SYSCFG:
		data = io.syscfg;
		break;
	case CLKCON:
		data = io.clkcon;
		break;
	case INTMOD:
		data = io.intmod;
		break;
	case INTPND:
		data = io.intpnd;
		break;
	case INTMSK:
		data = io.intmsk;
		break;
	case INTOFFSET:
	case INTPNDTST:
		data = io.intpndtst;
		break;
	case INTOSET_FIQ:
		data = io.intoset_fiq;
		break;
	case INTOSET_IRQ:

		{

			/*find which interrupt is pending */
			int i;
			for (i = 0; i < 26; i++) {
				if (io.intpnd & (1 << i))
					break;
			}
			if (i < 26) {
				data = (i << 2);
			} else
				data = 0x54;	/*no interrupt is pending, 0x54 is init data. */
		}

		//data = io.intoset_irq;
		break;
	 /*UART*/ case ULCON0:
		data = io.ulcon0;
		break;
	case ULCON1:
		data = io.ulcon1;
		break;
	case UCON0:
		data = io.ucon0;
		break;
	case UCON1:
		data = io.ucon1;
		break;
	case USTAT0:
		data = io.ustat0;
		io.ustat0 &= ~0xf;
		break;
	case USTAT1:
		data = io.ustat1;
		io.ustat1 &= ~0xf;
		break;

		//case UTXBUF0:
		//case UTXBUF1:
	case URXBUF0:
		data = io.urxbuf0;
		io.ustat0 &= ~UART_LSR_DR;
		break;
	case URXBUF1:
		data = io.urxbuf1;
		io.ustat1 &= ~UART_LSR_DR;
		break;
	case UBRDIV0:
		data = io.ubrdiv0;
		break;
	case UBRDIV1:
		data = io.ubrdiv1;
		break;

		/*Timer */
	case TMOD:
		data = io.tmod;
		break;
	case TDATA0:
		data = io.tdata0;
		break;
	case TDATA1:
		data = io.tdata1;
		break;
	case TCNT0:
		data = io.tcnt0;
		break;
	case TCNT1:
		data = io.tcnt1;
		break;
	default:
		SKYEYE_DBG ("%s (addr = 0x%08x)\n", __FUNCTION__, addr);
		break;
	}
	return data;
}

void
s3c4510b_io_write_byte (void *state, uint32_t addr, uint32_t data)
{

	//printf("SKYEYE: s3c4510b_io_write_byte error\n");
	s3c4510b_io_write_word (state, addr, data);
}

void
s3c4510b_io_write_halfword (void *state, uint32_t addr, uint32_t data)
{

	//printf("SKYEYE: s3c4510b_io_write_halfword error\n");
	s3c4510b_io_write_word (state, addr, data);
}

void
s3c4510b_io_write_word (void *state, uint32_t addr, uint32_t data)
{
	switch (addr) {
	case SYSCFG:
		io.syscfg = data;
		break;
	case CLKCON:
		io.clkcon = data;
		break;
	case INTMOD:
		io.intmod = data;
		break;
	case INTPND:

		/*when write bit 1, we clear apropiate pendind bit.
		 * */
		io.intpnd &= (~data & INT_MASK_INIT);
		break;
	case INTMSK:
		io.intmsk = data;
		break;
	case INTOFFSET:
	case INTPNDTST:
		io.intpndtst = io.intpnd = data;
		break;
	case INTOSET_FIQ:
		io.intoset_fiq = data;
		break;
	case INTOSET_IRQ:
		io.intoset_irq = data;
		break;
	 /*UART*/ case ULCON0:
		io.ulcon0 = data;
		break;
	case ULCON1:
		io.ulcon1 = data;
		break;
	case UCON0:
		io.ucon0 = data;
		break;
	case UCON1:
		io.ucon1 = data;
		break;
	case USTAT0:
		io.ustat0 = data;
		break;
	case USTAT1:
		io.ustat1 = data;
		break;
	case UTXBUF0:

		{
			char c = data;

			/* 2007-01-18 modified by Anthony Lee : for new uart device frame */
			skyeye_uart_write(-1, &c, 1, NULL);

			io.ustat0 |= (UART_LSR_THRE | UART_LSR_TEMT);
			if ((io.ucon0 & 0xc) == 0xc) {	/*enable interrupt */
				s3c4510b_set_interrupt (INT_UARTTX0);
				s3c4510b_update_int (state);
			}
		}
		break;
	case UTXBUF1:
		break;
	case UBRDIV0:
		io.ubrdiv0 = data;
		break;
	case UBRDIV1:
		io.ubrdiv1 = data;
		break;

		/*Timer */
	case TMOD:
		io.tmod = data;
		if (ENABLE_TIMER0)
			io.tcnt0 = io.tdata0;
		if (ENABLE_TIMER1)
			io.tcnt1 = io.tdata1;
		break;
	case TDATA0:
		if (!ENABLE_TIMER0)
			io.tdata0 = data;

		/*we manually set tdata0 register,  uclinux's data is so big */
		io.tdata0 = 0xfff;
		break;
	case TDATA1:

		//if (!ENABLE_TIMER1)
		io.tdata1 = data;
		break;
	case TCNT0:
		io.tcnt0 = data;
		break;
	case TCNT1:
		io.tcnt1 = data;
		break;
	default:
		SKYEYE_DBG ("%s(0x%08x) = 0x%08x\n", __FUNCTION__, addr,
			    data);
		break;
	}
}

void
s3c4510b_mach_init (void *arch_instance, machine_config_t *this_mach)
{
#if 0
	ARMul_SelectProcessor (state, ARM_v4_Prop);
	state->lateabtSig = HIGH;
#endif
	this_mach->mach_io_do_cycle = s3c4510b_io_do_cycle;
	this_mach->mach_io_reset = s3c4510b_io_reset;
	this_mach->mach_io_read_word = s3c4510b_io_read_word;
	this_mach->mach_io_write_word = s3c4510b_io_write_word;
	this_mach->mach_io_read_halfword = s3c4510b_io_read_halfword;
	this_mach->mach_io_write_halfword = s3c4510b_io_write_halfword;
	this_mach->mach_io_read_byte = s3c4510b_io_read_byte;
	this_mach->mach_io_write_byte = s3c4510b_io_write_byte;
	this_mach->mach_update_int = s3c4510b_update_int;
	this_mach->mach_set_intr = s3c4510b_set_interrupt;
	this_mach->mach_pending_intr = s3c4510b_pending_intr;
	this_mach->mach_update_intr = s3c4510b_update_intr;

	//this_mach->mach_mem_read_byte = s3c4510b_mem_read_byte;
	//this_mach->mach_mem_write_byte = s3c4510b_mem_write_byte;
	this_mach->state = (void *) arch_instance;
}
