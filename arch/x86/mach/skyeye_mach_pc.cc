#include "skyeye_config.h"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

/* 2007-01-18 added by Anthony Lee : for new uart device frame */
#include "skyeye_uart.h"

/* Timer Controller */
#define RTC_INIT_REG1			0xb8300000	/* Initial value */
#define RTC_COMPARE1			0xb8300004	/* 0 forever */
#define RTC_INIT_REG2			0xb8300008
#define RTC_COMPARE2			0xb830000c
#define RTC_COUNTER1			0xb8300010	/* Initial value sub 1 per ms */
#define RTC_COUNTER2			0xb8300014
#define RTC_CONTROL1			0xb8300018	/* Ack timer interrupt */
#define RTC_CONTROL2			0xb830001c

/* Interrupt Controller */
#define INC_ISR				0xb8200000
#define INC_IMR				0xb8200004

/* UART Controller */
#define UART_BAUDRATE			0xb8500000	/* 1byte */
#define UART_CHAR			0xb8500004	/* 1byte */

#define INC_PCI_A			(1<<0)
#define INC_PCI_B			(1<<1)
#define INC_PCI_C			(1<<2)
#define INC_PCI_D			(1<<3)
#define INC_PCITBD1			(1<<4)
#define INC_PCITBD2			(1<<5)
#define INC_PCITBD3			(1<<6)
#define INC_PCITBD4			(1<<7)
#define INC_DMA1			(1<<8)
#define INC_DMA2			(1<<9)
#define INC_UART_RX			(1<<10)
#define INC_UART_TX			(1<<11)
#define INC_USB				(1<<12)
#define INC_USBTBD			(1<<13)
#define INC_TIMER1			(1<<14)
#define INC_TIMER2			(1<<15)
#define INC_PM1				(1<<16)
#define INC_PM2				(1<<17)

#define MIPS_CP0_STATUS_IMASK      0x0000FF00
#define  ImpRev 		 0x2070

typedef struct uart_s {
	uint32 uart_baudrate; 
	uint32 uart_char;
}uart_t;

typedef struct rtc_s {
	uint32 timer_initreg1;
	uint32 timer_compare1;
	uint32 timer_initreg2;
	uint32 timer_compare2;
	uint32 timer_counter1;
	uint32 timer_counter2;
	uint32 timer_controlreg1;
	uint32 timer_controlreg2;
	uint32 timer_refreshreg;
}rtc_t;

typedef struct intc_s {
	uint32 isr;
	uint32 imr;
}intc_t;

typedef struct pc_io_s {
	uart_t uart;
	rtc_t timer;
	intc_t intc;
}pc_io_t;

static  pc_io_t io;

static void
pc_io_do_cycle (void* arch_instance)
{
	io.timer.timer_counter1--;
	
	if (io.timer.timer_counter1 == io.timer.timer_compare1) {
		io.timer.timer_counter1 = io.timer.timer_initreg1;
		io.intc.isr |= INC_TIMER1;
	}

	if (io.uart.uart_char == 0)
		io.intc.isr |= INC_UART_RX;//UART TX IRQ

	if (io.uart.uart_char != 0) {
		/* 2007-01-18 modified by Anthony Lee : for new uart device frame */
		struct timeval tv;
		unsigned char buf;

		tv.tv_sec = 0;
		tv.tv_usec = 0;

		//printf("\nUART RX IRQ before skyeye_uart_read\n");
		if(skyeye_uart_read(-1, &buf, 1, &tv, NULL) > 0)
		{
			//printf ("\nUART RX IRQ\n");
			io.uart.uart_char = buf;
			io.intc.isr |= INC_UART_TX; //UART RX IRQ
		}
	}

}

static uint32
pc_io_read_byte(void* arch_instance, uint32 addr)
{
	uint32 ret;

	switch (addr) {
		case RTC_INIT_REG1:
			ret = io.timer.timer_initreg1 & 0xff;
			break;
		case RTC_COMPARE1:
			ret = io.timer.timer_compare1 & 0xff;
			break;
		case RTC_INIT_REG2:
			ret = io.timer.timer_initreg2 & 0xff;
			break;
		case RTC_COMPARE2:
			ret = io.timer.timer_compare2 & 0xff;
			break;
		case RTC_COUNTER1:
			ret = io.timer.timer_counter1 & 0xff;
			break;
		case RTC_COUNTER2:
			ret = io.timer.timer_counter2 & 0xff;
			break;
		case RTC_CONTROL1:
			ret = io.timer.timer_controlreg1 & 0xff;
			break;
		case RTC_CONTROL2:
			ret = io.timer.timer_controlreg2 & 0xff;
			break;
		case INC_IMR:
			ret = io.intc.imr & 0xff;
			break;
		case INC_ISR:
			ret = io.intc.isr & 0xff;
			break;
		case UART_BAUDRATE:
			ret = io.uart.uart_baudrate & 0xff;
			break;
		case UART_CHAR:
			ret = io.uart.uart_char & 0xff;
			if (ret != 0) {
				io.intc.isr &= ~INC_UART_TX;		
			}
			break;
		default:
			break;
	}
	return ret;
}

static uint32
pc_io_read_halfword(void* arch_instance, uint32 addr)
{
	uint32 ret;

	switch (addr) {
		case RTC_INIT_REG1:
			ret = io.timer.timer_initreg1 & 0xffff;
			break;
		case RTC_COMPARE1:
			ret = io.timer.timer_compare1 & 0xffff;
			break;
		case RTC_INIT_REG2:
			ret = io.timer.timer_initreg2 & 0xffff;
			break;
		case RTC_COMPARE2:
			ret = io.timer.timer_compare2 & 0xffff;
			break;
		case RTC_COUNTER1:
			ret = io.timer.timer_counter1 & 0xffff;
			break;
		case RTC_COUNTER2:
			ret = io.timer.timer_counter2 & 0xffff;
			break;
		case RTC_CONTROL1:
			ret = io.timer.timer_controlreg1 & 0xffff;
			break;
		case RTC_CONTROL2:
			ret = io.timer.timer_controlreg2 & 0xffff;
			break;
		case INC_IMR:
			ret = io.intc.imr & 0xffff;
			break;
		case INC_ISR:
			ret = io.intc.isr & 0xffff;
			break;
		case UART_BAUDRATE:
			ret = io.uart.uart_baudrate & 0xffff;
			break;
		case UART_CHAR:
			ret = io.uart.uart_char & 0xffff;
			break;
		default:
			break;
	}
	return ret;
}

static uint32
pc_io_read_word(void* arch_instance, uint32 addr)
{
	uint32 ret;

	switch (addr) {
		case RTC_INIT_REG1:
			ret = io.timer.timer_initreg1;
			break;
		case RTC_COMPARE1:
			ret = io.timer.timer_compare1;
			break;
		case RTC_INIT_REG2:
			ret = io.timer.timer_initreg2;
			break;
		case RTC_COMPARE2:
			ret = io.timer.timer_compare2;
			break;
		case RTC_COUNTER1:
			ret = io.timer.timer_counter1;
			break;
		case RTC_COUNTER2:
			ret = io.timer.timer_counter2;
			break;
		case RTC_CONTROL1:
			ret = io.timer.timer_controlreg1;
			break;
		case RTC_CONTROL2:
			ret = io.timer.timer_controlreg2;
			break;
		case INC_IMR:
			ret = io.intc.imr;
			break;
		case INC_ISR:
			ret = io.intc.isr;
			break;
		case UART_BAUDRATE:
			ret = io.uart.uart_baudrate;
			break;
		case UART_CHAR:
			ret = io.uart.uart_char;
			if (ret != 0) {
				io.intc.isr &= ~INC_UART_TX;		
			}
			break;
		default:
			break;
	}
	return ret;
}

static void
pc_io_write_byte(void* arch_instance, uint32 addr, uint32 data)
{
	unsigned char c = data & 0xff;

}

static void
pc_io_write_halfword(void* arch_instance, uint32 addr, uint32 data)
{
	
}

static void
pc_io_write_word(void* arch_instance, uint32 addr, uint32 data)
{
	
	switch (addr) {
		case RTC_INIT_REG1:
			io.timer.timer_initreg1 = data;
			io.timer.timer_counter1 = data;
			break;
		case RTC_COMPARE1:
			io.timer.timer_compare1 = data;
			break;
		case RTC_INIT_REG2:
			io.timer.timer_initreg2 = data;
			break;
		case RTC_COMPARE2:
			io.timer.timer_compare2 = data;
			break;
		case RTC_COUNTER1:
			io.timer.timer_counter1 = data;
			break;
		case RTC_COUNTER2:
			io.timer.timer_counter2 = data;
			break;
		case RTC_CONTROL1:
			io.timer.timer_controlreg1 = data;
			break;
		case RTC_CONTROL2:
			io.timer.timer_controlreg2 = data;
			break;
		case INC_IMR:
			io.intc.imr = data;
			break;
		case INC_ISR:	
			io.intc.isr = data;
			break;
		case UART_BAUDRATE:
			io.uart.uart_baudrate = data;
			break;
		case UART_CHAR:
			io.uart.uart_char = data;
			break;
		defalut:
			break;
	}
}

static void
pc_set_int(uint32 irq)
{

}

static void pc_io_reset(void* arch_instance){
}

void
pc_mach_init (void * arch_instance, machine_config_t * this_mach)
{		
	this_mach->mach_io_read_byte = pc_io_read_byte;
	this_mach->mach_io_read_halfword = pc_io_read_halfword;
	this_mach->mach_io_read_word = pc_io_read_word;
	this_mach->mach_io_write_byte = pc_io_write_byte;
	this_mach->mach_io_write_halfword = pc_io_write_halfword;
	this_mach->mach_io_write_word = pc_io_write_word;
	this_mach->mach_io_do_cycle = pc_io_do_cycle;
	this_mach->mach_io_reset = pc_io_reset;
}
