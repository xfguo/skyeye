/*
	skyeye_mach_s3c6410x.c - define machine S3C6410 for skyeye
	Copyright (C) 2010 Skyeye Develop Group
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

#include <skyeye_config.h>
#include <skyeye_arch.h>
#include <skyeye_sched.h>
#include <skyeye_lock.h>
#include "s3c6410x.h"
#include "skyeye_internal.h"

#ifdef __CYGWIN__
#include <time.h>
#endif
s3c6410x_io_t s3c6410x_io;
#define io s3c6410x_io

static inline void
s3c6410x__update_uart_adc ()
{
	u32 request;
#if 0
	/* Enable the UART0-2 IRQ*/
	request = (io.vicx_status[0].vicxirqstatus & io.vicx_status[0].vicxintenable);
	io.vicx_status[0].vicxintenable |= INT_UART0;
	io.vicx_status[0].vicxintenable |= INT_UART1;
	io.vicx_status[0].vicxintenable |= INT_UART2;

	/* Enable ANC IRQ*/
	io.vicx_status[1].vicxintenable |= INT_ADC;
	io.vicx_status[1].vicxintenable |= INT_PENDNUP;
#endif
}

static inline void
s3c6410x_update_extint ()
{
#if 0
	io.vicx_status[0].vicxintenable |= EINT0; /* EINT 4~11 */
	io.vicx_status[0].vicxintenable |= EINT1; /* EINT 0~3 */
#endif
}

static void
s3c6410x_update_int (void *arch_instance)
{
#if 0
	uint32 requests;
	/*UART and ADC interrupt set function*/
	s3c6410x_update_uart_adc ();
	/*EINT interrupt set function*/
	s3c6410x_update_extint ();
	/*IRQ and FIQ set*/
	requests = io.srcpnd & (~io.intmsk & INT_MASK_INIT);
	state->NfiqSig = (requests & io.intmod) ? LOW : HIGH;
	state->NirqSig = (requests & ~io.intmod) ? LOW : HIGH;
#endif
	interrupt_signal_t interrupt_signal;
	if (io.vic0fiqstatus != 0 || io.vic1fiqstatus != 0) {
		interrupt_signal.arm_signal.firq =  Low_level;
	} else {
		interrupt_signal.arm_signal.firq =  High_level;
	}

	if (io.vic0irqstatus != 0 || io.vic1irqstatus != 0) {
		interrupt_signal.arm_signal.irq =  Low_level;
	} else {
		interrupt_signal.arm_signal.irq =  High_level;
	}
	interrupt_signal.arm_signal.reset =  Prev_level;
	send_signal(&interrupt_signal);
}

static void
s3c6410x_set_ext_intr (u32 interrupt)
{
	/* io.eintpend |= (1 << interrupt); */
}

static int
s3c6410x_pending_ext_intr (u32 interrupt)
{
	/* return ((io.eintpend & (1 << interrupt))); */
	return 0;
}

static void
s3c6410x_update_intr (void *mach)
{
	generic_arch_t *arch_instance = get_arch_instance("");
	s3c6410x_update_int (arch_instance);
}

#define UART_UTRSTAT_INIT	0x6
static void
s3c6410x_io_reset (generic_arch_t *arch_instance)
{
	int i;

	memset (&io, 0, sizeof (s3c6410x_io));

	/* uart reigsters reset */
	for (i = 0; i < 3; i++) {
		io.uart[i].utrstat = UART_UTRSTAT_INIT;
	}

	io.tc_prescale = 1;
	/* time reigsters reset */
	io.timer.tcfg0 = 0x101;

	/* clock register reset */
	io.clkpower.locktime = 0x00FFFFFF;
	io.clkpower.apllcon = 0x01900302;
	io.clkpower.mpllcon = 0x02140603;
	io.clkpower.epllcon0 = 0x00200102;
	io.clkpower.epllcon1 = 0x00009111;
	io.clkpower.clkdiv0 = 0x01051000;

	/* machine ID for SMDK6410 */
	arch_instance->set_regval_by_id(1, 1626);
}

/* s3c6410x io_do_cycle */
static void
s3c6410x_io_do_cycle (generic_arch_t *state)
{
	int i;
	io.tc_prescale --;
	if (io.tc_prescale < 0) {
		io.tc_prescale = 1;
		/* 0x100000 equals [bit:20] = 1 start timer 4*/
		if ((io.timer.tcon & 0x100000) != 0) {
			/*tcntx is the orignal value we set, it equals tcntbx firstly*/
			io.timer.tcnt[4]--;
			if (io.timer.tcnt[4] <= 0) {
				/* whe the tcntx is 0, reset the timer tcntx as the value of
				 * tcntb
				 */
				io.timer.tcnt[4] = io.timer.tcntb[4];
				/*timer 4 hasn't tcmp */
				io.timer.tcnto[4] = io.timer.tcntb[4];
				/* Timer4 request status*/
	
				/* set timer4 interrupt */
				io.vic0rawintr |= 1 << INT_TIMER4;
				io.vic0irqstatus |=  ((1 << INT_TIMER4) & ~(io.vic0intselect) & io.vic0intenable);
				io.vic0fiqstatus |=  ((1 << INT_TIMER4) & io.vic0intselect & io.vic0intenable);
	
				s3c6410x_update_int (state);
				return;
			}
		}
	
		for (i = 0; i < 3; i++) {
			if (((io.uart[i].utrstat & 0x1) == 0x0) && ((io.uart[i].ucon & 0x3) == 0x1)) {
				struct timeval tv;
				unsigned char buf;
	
				tv.tv_sec = 0;
				tv.tv_usec = 0;
	
				if (skyeye_uart_read(i, &buf, 1, &tv, NULL) > 0) {
					/* convert ctrl+c to ctrl+a. */
					if (buf == 1) buf = 3;
					io.uart[i].urxh = buf;
	
					/* Receiver Ready
					 * */
					io.uart[i].ufstat |= (0x1); /* 2007-02-09 by Anthony Lee : for 1 bytes */
	
					/* pending usart0 interrupt
					 * */
					io.uart[i].uintp |= (0x1 & ~io.uart[i].uintm);
					io.uart[i].uintsp |= 0x1;
	
					if (io.uart[i].uintp) {
						io.vic1rawintr	 |=  1 << (INT_UART0 + i);
						io.vic1irqstatus |=  ((1 << (INT_UART0 + i)) & ~(io.vic1intselect) & io.vic1intenable);
						io.vic1fiqstatus |=  ((1 << (INT_UART0 + i)) & io.vic1intselect & io.vic1intenable);
	
						s3c6410x_update_int (state);
					}
				}
			}
		}
	}
}

static void
s3c6410x_uart_read (u32 offset, u32 *data, int index)
{
	switch (offset) {

	case ULCON:
		*data = io.uart[index].ulcon;
		break;
	case UCON:
		*data = io.uart[index].ucon;
		break;
	case UFCON:
		*data = io.uart[index].ufcon;
		break;
	case UMCON:
		*data = io.uart[index].umcon;
		break;
	case UTRSTAT:
		*data = io.uart[index].utrstat;
		break;
	case UERSTAT:
		*data = io.uart[index].uerstat;
		break;
	case UFSTAT:
		*data = io.uart[index].ufstat;
		break;
	case UMSTAT:
		*data = io.uart[index].umstat;
		break;
	case URXH:
		/* receive char
		 * */
		*data = io.uart[index].urxh;
		io.uart[index].utrstat &= (~0x1);	/* clear strstat register bit[0] */
		io.uart[index].ufstat &= ~(0x1); /* 2007-02-09 by Anthony Lee : for 0 bytes */
		break;
	case UBRDIV:
		*data = io.uart[index].ubrdiv;
		break;
	case UDIVSLOT:
		*data = io.uart[index].ubrdivslot;
		break;
	case UINTP:
		*data = io.uart[index].uintp;
		break;
	case UINTSP:
		*data = io.uart[index].uintsp;
		break;
	case UINTM:
		*data = io.uart[index].uintm;
		break;
	default:
		break;
	}
	SKYEYE_DBG ("%s(UART%d: 0x%x, 0x%x)\n", __FUNCTION__, index, offset, data);
}

static void
s3c6410x_uart_write (generic_arch_t *state, u32 offset, u32 data, int index)
{

	SKYEYE_DBG ("%s(UART%d: 0x%x, 0x%x)\n", __FUNCTION__, index, offset, data);
	switch (offset) {
	case ULCON:
		io.uart[index].ulcon = data;
		break;
	case UCON:
		io.uart[index].ucon = data;
		break;
	case UFCON:
		io.uart[index].ufcon = data;
		break;
	case UMCON:
		io.uart[index].umcon = data;
		break;
	case UMSTAT:
		io.uart[index].umstat = data;
		break;
	case UTXH:
		{
			char c = data;
			/* 2007-01-18 modified by Anthony Lee : for new uart device frame */
			skyeye_uart_write(index, &c, 1, NULL);
			io.uart[index].utrstat |= 0x6;	/* set strstat register bit[0] */
			io.uart[index].ufstat &= 0xff;	/* set strstat register bit[0] */

			io.uart[index].uintp |= (0x4 & ~io.uart[index].uintm);
			io.uart[index].uintsp |= 0x4;
			{

				io.vic1rawintr	 |=  1 << (INT_UART0 + index);
				io.vic1irqstatus |=  ((1 << (INT_UART0 + index)) & ~(io.vic1intselect) & io.vic1intenable);
				io.vic1fiqstatus |=  ((1 << (INT_UART0 + index)) & io.vic1intselect & io.vic1intenable);
			}

			s3c6410x_update_int (state);
		}
		break;
	case UBRDIV:
		io.uart[index].ubrdiv = data;
		break;
	case UDIVSLOT:
		io.uart[index].ubrdivslot = data;
		break;
	case UINTP:
		io.uart[index].uintp &= ~data;
		break;
	case UINTSP:
		io.uart[index].uintsp &= ~data;
		break;
	case UINTM:
		io.uart[index].uintm = data;
		{

			io.uart[index].uintp |= (0x4 & ~io.uart[index].uintm);
			io.uart[index].uintsp |= 0x4;
			{

				io.vic1rawintr	 |=  1 << (INT_UART0 + index);
				io.vic1irqstatus |=  ((1 << (INT_UART0 + index)) & ~(io.vic1intselect) & io.vic1intenable);
				io.vic1fiqstatus |=  ((1 << (INT_UART0 + index)) & io.vic1intselect & io.vic1intenable);
			}

			s3c6410x_update_int (state);

		}
		break;
	default:
		break;
	}
}

static void
s3c6410x_timer_read (u32 offset, u32 *data)
{
	switch (offset) {
	case TCFG0:
		*data = io.timer.tcfg0;
		break;
	case TCFG1:
		*data = io.timer.tcfg1;
		break;
	case TCON:
		*data = io.timer.tcon;
		break;
	case TCNTB0:
	case TCNTB1:
	case TCNTB2:
	case TCNTB3:
	case TCNTB4:
		{
			int n = (offset - 0xC) / 0xC;
			*data = io.timer.tcntb[n];
		}
		break;
	case TCMPB0:
	case TCMPB1:
		{
			int n = (offset - 0x10) / 0xC;
			*data = io.timer.tcmpb[n];
		}
		break;
	case TCNTO0:
	case TCNTO1:
	case TCNTO2:
	case TCNTO3:
		{
			int n = (offset - 0x10) / 0xC;
			*data = io.timer.tcnto[n];
		}
		break;
	case TCNTO4:
		*data = io.timer.tcnt[4];
		break;
	default:
		break;
	}
}

static void
s3c6410x_timer_write (generic_arch_t *state, u32 offset, u32 data)
{
	switch (offset) {
	case TCFG0:
		io.timer.tcfg0 = data;
		break;
	case TCFG1:
		io.timer.tcfg1 = data;
		break;
	case TCON:
		{
			io.timer.tcon = data;
			if (io.timer.tcon) {
			}
		}
		break;
	case TCNTB0:
	case TCNTB1:
	case TCNTB2:
	case TCNTB3:
	case TCNTB4:
		{
			int n = (offset - 0xC) / 0xC;
			/* io.timer.tcntb[n] = data; */
			/* temp data taken from linux source */
			io.timer.tcntb[n] = 25350 / 20;
		}
		break;
	case TCMPB0:
	case TCMPB1:
#if 0
	case TCMPB2:
	case TCMPB3:
#endif
		{
			int n = (offset - 0x10) / 0xC;
			io.timer.tcmpb[n] = data;
		}
		break;
	default:
		break;
	}
}

static uint32
s3c6410x_io_read_word (void *arch_instance, uint32 addr)
{
	uint32 data = -1;
	int i;
	/* uart */
	if ((addr >= UART_CTL_BASE0)
	    && (addr < (UART_CTL_BASE0 + UART_CTL_SIZE))) {
		s3c6410x_uart_read ((u32) ((addr - UART_CTL_BASE0) % 0x400),
				    (u32 *) &data,
				    (addr - UART_CTL_BASE0) / 0x400);
		return data;
	}
	if ((addr >= PWM_CTL_BASE) && (addr < (PWM_CTL_BASE + PWM_CTL_SIZE))) {
		s3c6410x_timer_read ((u32) (addr - PWM_CTL_BASE),
				     (u32 *) &data);
		return data;
	}
#if 0
	/*
	 * 2007-02-09 by Anthony Lee
	 * changed 0xC0 to 0xA4 for running linux-2.6.20,
	 * because GSTATUS1 is 0xB0, the "0xC0" make it like S3C2400
	 */
	if ((addr >= GPIO_CTL_BASE) && (addr < (GPIO_CTL_BASE + 0xA4))) {
		int offset = addr - GPIO_CTL_BASE;
		return io.gpio_ctl[offset];
	}
#endif
	#define BOARD_ID 0x7e00f118
	switch (addr) {
	case BOARD_ID:
		data = 0x36410100;
		break;
	case VIC0IRQSTATUS:
		data = io.vic0irqstatus;
		io.vic0irqstatus = 0;
		s3c6410x_update_int(arch_instance);
		break;
	case VIC0FIQSTATUS:
		data = io.vic0fiqstatus;
		io.vic0fiqstatus = 0;
		break;
	case VIC0RAWINTR:
		data = io.vic0rawintr;
		io.vic0rawintr = 0;
		break;
	case VIC0INTSELECT:
		data = io.vic0intselect;
		break;
	case VIC0INTENABLE:
		data = io.vic0intenable;
		break;

	case VIC0SOFTINT:
		data = data = io.vic0softint;
		break;
	case VIC0PROTECTION:
		data = io.vic0protection;
		break;
	case VIC0SWPRIORITYMASK:
		data = io.vic0swprioritymask;
		break;
	case VIC0PRIORITYDAISY:
		data = io.vic0prioritydaisy;
		break;

	case VIC1IRQSTATUS:
		data = io.vic1irqstatus;
		io.vic1irqstatus = 0;
		s3c6410x_update_int(arch_instance);
		break;
	case VIC1FIQSTATUS:
		data = io.vic1fiqstatus;
		io.vic1fiqstatus = 0;
		break;
	case VIC1RAWINTR:
		data = io.vic1rawintr;
		io.vic1rawintr = 0;
		break;

	case VIC1INTSELECT:
		data = io.vic1intselect;
		break;
	case VIC1INTENABLE:
		data = io.vic1intenable;
		break;
	case VIC1INTENCLEAR:
		/* data = io.vic1intenclear; */
		data = io.vic1intenable &= ~data;
		break;
	case VIC1SOFTINT:
		data = io.vic1softint;
		break;
	case VIC1SOFTINTCLEAR:
		data = io.vic1softintclear;
		break;
	case VIC1PROTECTION:
		data = io.vic1protection;
		break;
	case VIC1SWPRIORITYMASK:
		data = io.vic1swprioritymask;
		break;
	case VIC1PRIORITYDAISY:
		data = io.vic1prioritydaisy;
		break;

	case APLL_CON:
                data = io.clkpower.apllcon;
		break;
	case MPLL_CON:
		data = io.clkpower.mpllcon;
		break;
	case EPLL_CON0:
		data = io.clkpower.epllcon0;
		break;
	case EPLL_CON1:
		data = io.clkpower.epllcon1;
		break;
	case CLK_SRC:
		data = io.clkpower.clksrc;
		break;
	case CLK_DIV0:
		data = io.clkpower.clkdiv0;
		break;
	case CLK_DIV1:
		data = io.clkpower.clkdiv1;
		break;
	case CLK_DIV2:
		data = io.clkpower.clkdiv2;
		break;
#if 0
	case BWSCON:
		data = io.memctl.bwscon;
		break;
	case BANKCON0:
		data = io.memctl.bankcon[0];
		break;
	case BANKCON1:
		data = io.memctl.bankcon[1];
		break;
	case BANKCON2:
		data = io.memctl.bankcon[2];
		break;
	case BANKCON3:
		data = io.memctl.bankcon[3];
		break;
	case BANKCON4:
		data = io.memctl.bankcon[4];
		break;
	case BANKCON5:
		data = io.memctl.bankcon[5];
		break;
	case BANKCON6:
		data = io.memctl.bankcon[6];
		break;
	case BANKCON7:
		data = io.memctl.bankcon[7];
		break;
	case REFRESH:
		data = io.memctl.refresh;
		break;
	case BANKSIZE:
		data = io.memctl.banksize;
		break;
	case MRSRB6:
		data = io.memctl.mrsrb6;
		break;
	case MRSRB7:
		data = io.memctl.mrsrb7;
		break;
	case WDCON:
		data = io.wd_timer.wtcon;
		break;
	case WDDAT:
		data = io.wd_timer.wtdat;
		break;
	case WDCNT:
		data = io.wd_timer.wtcnt;
		break;
#endif
	default:
		/* fprintf(stderr, "ERROR: %s(0x%08x) \n", __FUNCTION__, addr); */
		if (addr - VIC0VECTADDR0 >= 0 && addr - VIC0VECTADDR0 <= 0x7c && (addr - VIC0VECTADDR0) & 0x3 == 0)
			data = io.vic0vectaddr[(addr - VIC0VECTADDR0)>>2];
		else
		if (addr - VIC1VECTADDR0 >= 0 && addr - VIC1VECTADDR0 <= 0x7c && (addr - VIC1VECTADDR0) & 0x3 == 0)
			data = io.vic1vectaddr[(addr - VIC0VECTADDR1)>>2];
		else
		if (addr - VIC0VECPRIORITY0 >= 0 && addr - VIC0VECPRIORITY0 <= 0x7c &&  (addr - VIC0VECPRIORITY0) & 0x3 == 0)
			data = io.vic0vecpriority[(addr - VIC0VECTADDR0)>>2];
		else
		if (addr - VIC1VECPRIORITY0 >= 0 && addr - VIC1VECPRIORITY0 <= 0x7c &&  (addr - VIC1VECPRIORITY0) & 0x3 == 0)
			data = io.vic1vecpriority[(addr - VIC1VECTADDR0)>>2];
		break;
	}
	return data;
}

static uint32
s3c6410x_io_read_byte (void *arch_instance, uint32 addr)
{
	s3c6410x_io_read_word (arch_instance, addr);
}

static uint32
s3c6410x_io_read_halfword (void *arch_instance, uint32 addr)
{
	s3c6410x_io_read_word (arch_instance, addr);
}

static void
s3c6410x_io_write_word (generic_arch_t *state, uint32 addr, uint32 data)
{
	if ((addr >= UART_CTL_BASE0)
	    && (addr < UART_CTL_BASE0 + UART_CTL_SIZE)) {
#if 0
		s3c6410x_uart_write (state, (addr - UART_CTL_BASE0) % 0x4000,
				     data, (addr - UART_CTL_BASE0) / 0x4000);
#endif
		s3c6410x_uart_write (state, (addr - UART_CTL_BASE0) % 0x400,
				     data, (addr - UART_CTL_BASE0) / 0x400);
		return;
	}

	if ((addr >= PWM_CTL_BASE) && (addr < (PWM_CTL_BASE + PWM_CTL_SIZE))) {
		s3c6410x_timer_write (state, addr - PWM_CTL_BASE, data);
		return;
	}

	/*
	 * 2007-02-09 by Anthony Lee
	 * changed 0xC0 to 0xA4 for running linux-2.6.20,
	 * because GSTATUS1 is 0xB0, the "0xC0" make it like S3C2400
	 */
#if 0
	if ((addr >= GPIO_CTL_BASE) && (addr < (GPIO_CTL_BASE + 0xA4))) {
		int offset = addr - GPIO_CTL_BASE;
		io.gpio_ctl[offset] = data;
		return;
	}
#endif
	switch (addr) {

	case VIC0INTSELECT:
		io.vic0intselect = data;
		break;
	case VIC0INTENABLE:
		io.vic0intenable |= data;
		break;
	case VIC0INTENCLEAR:
		/* io.vic0intenclear = data; */
		/* write 1 clear the intenable register */
		io.vic0intenable &= ~data;
		break;
	case VIC0SOFTINT:
		io.vic0softint = data;
		break;
	case VIC0SOFTINTCLEAR:
		io.vic0softintclear = data;
		break;
	case VIC0PROTECTION:
		io.vic0protection = data;
		break;
	case VIC0SWPRIORITYMASK:
		io.vic0swprioritymask = data;
		break;
	case VIC0PRIORITYDAISY:
		io.vic0prioritydaisy = data;
		break;

	case VIC1INTSELECT:
		io.vic1intselect = data;
		break;
	case VIC1INTENABLE:
		io.vic1intenable |= data;
		break;
	case VIC1INTENCLEAR:
		/* write 1 clear the intenable register */
		io.vic1intenable &= ~data;
		break;
	case VIC1SOFTINT:
		io.vic1softint = data;
		break;
	case VIC1SOFTINTCLEAR:
		io.vic1softintclear = data;
		break;
	case VIC1PROTECTION:
		io.vic1protection = data;
		break;
	case VIC1SWPRIORITYMASK:
		io.vic1swprioritymask = data;
		break;
	case VIC1PRIORITYDAISY:
		io.vic1prioritydaisy = data;
		break;

	case APLL_CON:
		io.clkpower.apllcon = data;
		break;
	case MPLL_CON:
		io.clkpower.mpllcon = data;
		break;
	case EPLL_CON0:
		io.clkpower.epllcon0 = data;
		break;
	case EPLL_CON1:
		io.clkpower.epllcon1 = data;
		break;
	case CLK_SRC:
		io.clkpower.clksrc = data;
		break;
	case CLK_DIV0:
		io.clkpower.clkdiv0 = data;
		break;
	case CLK_DIV1:
		io.clkpower.clkdiv1 = data;
		break;
	case CLK_DIV2:
		io.clkpower.clkdiv2 = data;
		break;

#if 0
	case BWSCON:
		io.memctl.bwscon = data;
		break;
	case MPLLCON:
		io.clkpower.mpllcon = data;
		break;
	case BANKCON0:
		io.memctl.bankcon[0] = data;
		break;
	case BANKCON1:
		io.memctl.bankcon[1] = data;
		break;
	case BANKCON2:
		io.memctl.bankcon[2] = data;
		break;
	case BANKCON3:
		io.memctl.bankcon[3] = data;
		break;
	case BANKCON4:
		io.memctl.bankcon[4] = data;
		break;
	case BANKCON5:
		io.memctl.bankcon[5] = data;
		break;
	case BANKCON6:
		io.memctl.bankcon[6] = data;
		break;
	case BANKCON7:
		io.memctl.bankcon[7] = data;
		break;
	case REFRESH:
		io.memctl.refresh = data;
		break;
	case BANKSIZE:
		io.memctl.banksize = data;
		break;
	case MRSRB6:
		io.memctl.mrsrb6 = data;
		break;
	case MRSRB7:
		io.memctl.mrsrb7 = data;
		break;
	case WDCON:
		io.wd_timer.wtcon = data;
		break;
	case WDDAT:
		io.wd_timer.wtdat = data;
		break;
	case WDCNT:
		io.wd_timer.wtcnt = data;
		break;
#endif
	default:
		if (addr - VIC0VECTADDR0 >= 0 && addr - VIC0VECTADDR0 <= 0x7c &&  (addr - VIC0VECTADDR0) & 0x3 == 0)
			io.vic0vectaddr[(addr - VIC0VECTADDR0)>>2] = data;
		else
		if (addr - VIC1VECTADDR0 >= 0 && addr - VIC1VECTADDR0 <= 0x7c &&  (addr - VIC1VECTADDR0) & 0x3 == 0)
			io.vic1vectaddr[(addr - VIC0VECTADDR1)>>2] = data;
		else
		if (addr - VIC0VECPRIORITY0 >= 0 && addr - VIC0VECPRIORITY0 <= 0x7c &&  (addr - VIC0VECPRIORITY0) & 0x3 == 0)
			io.vic0vecpriority[(addr - VIC0VECTADDR0)>>2] = data;
		else
		if (addr - VIC1VECPRIORITY0 >= 0 && addr - VIC1VECPRIORITY0 <= 0x7c &&  (addr - VIC1VECPRIORITY0) & 0x3 == 0)
			io.vic1vecpriority[(addr - VIC1VECTADDR0)>>2] = data;

/* 		SKYEYE_DBG ("io_write_word(0x%08x) = 0x%08x\n", addr, data); */
/* 		fprintf(stderr, "ERROR: %s(0x%08x) = 0x%08x\n", __FUNCTION__, addr ,data); */
		break;
	}
}

static void
s3c6410x_io_write_byte (generic_arch_t *state, uint32 addr, uint32 data)
{
	SKYEYE_DBG ("SKYEYE: s3c6410x_io_write_byte error\n");
	s3c6410x_io_write_word (state, addr, data);
}

static void
s3c6410x_io_write_halfword (generic_arch_t *state, uint32 addr, uint32 data)
{
	SKYEYE_DBG ("SKYEYE: s3c6410x_io_write_halfword error\n");
	s3c6410x_io_write_word (state, addr, data);
}

void
s3c6410x_mach_init (void *arch_instance, machine_config_t *this_mach)
{
	this_mach->mach_io_do_cycle = s3c6410x_io_do_cycle;
	this_mach->mach_io_reset = s3c6410x_io_reset;
	this_mach->mach_io_read_byte = s3c6410x_io_read_byte;
	this_mach->mach_io_write_byte = s3c6410x_io_write_byte;
	this_mach->mach_io_read_halfword = s3c6410x_io_read_halfword;
	this_mach->mach_io_write_halfword = s3c6410x_io_write_halfword;
	this_mach->mach_io_read_word = s3c6410x_io_read_word;
	this_mach->mach_io_write_word = s3c6410x_io_write_word;
	this_mach->mach_update_int = s3c6410x_update_int;
	this_mach->mach_set_intr = s3c6410x_set_ext_intr;
	this_mach->mach_pending_intr = s3c6410x_pending_ext_intr;
	this_mach->mach_update_intr = s3c6410x_update_intr;
	this_mach->state = (void *) arch_instance;

	add_chp_data(&s3c6410x_io, sizeof(s3c6410x_io_t), "6410io");
}
