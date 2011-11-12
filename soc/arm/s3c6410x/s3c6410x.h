/*
	s3c6410x.h - definitions of "s3c6410" machine  for skyeye
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
/**
* @file s3c6410x.h
* @brief The definition of s3c6410
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-11-12
*/


#ifndef __S3C6410_H_
#define __S3C6410_H_

#ifndef u32
#define u32 unsigned int
#endif
#define REGW(addr)	(*(volatile unsigned int *)(addr))
/********************************************
* Memory Controller Registers(only for DRAM)
********************************************/

#define DRAM_CTL_BASE1		(0x7E001000)
#define DRAM_CTL_SIZE1		(0x54)

#define P1MEMSTAT	DRAM_CTL_BASE1+(0x0) /*DRAM controller status register*/
#define P1MEMCCMD	DRAM_CTL_BASE1+(0x4) /*DRAM controller command register*/
#define P1DIRECTCMD	DRAM_CTL_BASE1+(0x8) /*DRAM controller direct command register*/
#define P1MEMCFG	DRAM_CTL_BASE1+(0xC) /*DRAM controller memory config register*/
#define P1REFRESH	DRAM_CTL_BASE1+(0x10) /*DRAM controller memory reflash period register*/
#define PnCASLAT	DRAM_CTL_BASE1+(0x14) /*DRAM controller CAS latency register*/
#define P1T_DQSS	DRAM_CTL_BASE1+(0x18) /*DRAM controller t_DQSS register*/
#define P1T_MRD 	DRAM_CTL_BASE1+(0x1C) /*DRAM controller t_MRD register*/
#define P1T_RAS 	DRAM_CTL_BASE1+(0x20) /*DRAM controller t_RAS register*/
#define P1T_RC	 	DRAM_CTL_BASE1+(0x24) /*DRAM controller t_RC register*/
#define P1T_RCD	 	DRAM_CTL_BASE1+(0x28) /*DRAM controller t_RCD register*/
#define P1T_RFC	 	DRAM_CTL_BASE1+(0x2c) /*DRAM controller t_RFC register*/
#define P1T_RP	 	DRAM_CTL_BASE1+(0x30) /*DRAM controller t_RP register*/
#define P1T_RRD	 	DRAM_CTL_BASE1+(0x34) /*DRAM controller t_RRD register*/
#define P1T_WR	 	DRAM_CTL_BASE1+(0x38) /*DRAM controller t_WR register*/
#define P1T_WTR	 	DRAM_CTL_BASE1+(0x3C) /*DRAM controller t_WTR register*/
#define P1T_XP	 	DRAM_CTL_BASE1+(0x40) /*DRAM controller t_XP register*/
#define P1T_XSR	 	DRAM_CTL_BASE1+(0x44) /*DRAM controller t_XSR register*/
#define P1T_ESR	 	DRAM_CTL_BASE1+(0x48) /*DRAM controller t_ESR register*/
#define P1MEMCFG2 	DRAM_CTL_BASE1+(0x4C) /*DRAM controller configuration register*/
#define P1MEMCFG3 	DRAM_CTL_BASE1+(0x50) /*DRAM controller configuration register*/

/* DRAM id_<n>_cfg register*/
#define DRAM_CTL_BASE2		(0x7E001100)
#define DRAM_CTL_SIZE2		(0x40)

#define P1_id_0_cfg	 DRAM_CTL_BASE2+(0x0) /*DRAM controller id_<n>_cfg register*/
#define P1_id_1_cfg	 DRAM_CTL_BASE2+(0x4) /*DRAM controller id_<n>_cfg register*/
#define P1_id_2_cfg	 DRAM_CTL_BASE2+(0x8) /*DRAM controller id_<n>_cfg register*/
#define P1_id_3_cfg	 DRAM_CTL_BASE2+(0xC) /*DRAM controller id_<n>_cfg register*/
#define P1_id_4_cfg	 DRAM_CTL_BASE2+(0x10) /*DRAM controller id_<n>_cfg register*/
#define P1_id_5_cfg	 DRAM_CTL_BASE2+(0x14) /*DRAM controller id_<n>_cfg register*/
#define P1_id_6_cfg	 DRAM_CTL_BASE2+(0x18) /*DRAM controller id_<n>_cfg register*/
#define P1_id_7_cfg	 DRAM_CTL_BASE2+(0x1C) /*DRAM controller id_<n>_cfg register*/
#define P1_id_8_cfg	 DRAM_CTL_BASE2+(0x20) /*DRAM controller id_<n>_cfg register*/
#define P1_id_9_cfg	 DRAM_CTL_BASE2+(0x24) /*DRAM controller id_<n>_cfg register*/
#define P1_id_10_cfg DRAM_CTL_BASE2+(0x28) /*DRAM controller id_<n>_cfg register*/
#define P1_id_11_cfg DRAM_CTL_BASE2+(0x2C) /*DRAM controller id_<n>_cfg register*/
#define P1_id_12_cfg DRAM_CTL_BASE2+(0x30) /*DRAM controller id_<n>_cfg register*/
#define P1_id_13_cfg DRAM_CTL_BASE2+(0x34) /*DRAM controller id_<n>_cfg register*/
#define P1_id_14_cfg DRAM_CTL_BASE2+(0x38) /*DRAM controller id_<n>_cfg register*/
#define P1_id_15_cfg DRAM_CTL_BASE2+(0x3C) /*DRAM controller id_<n>_cfg register*/

/*DRAM controller chip_<n>_cfg register*/
#define DRAM_CTL_BASE3		(0x7E001200)
#define DRAM_CTL_SIZE3		(0x8)
#define P1_chip_0_cfg	DRAM_CTL_BASE3+(0x0) /*DRAM controller chip_<n>_cfg register*/
#define P1_chip_1_cfg	DRAM_CTL_BASE3+(0x4) /*DRAM controller chip_<n>_cfg register*/

/*DRAM controller user state and configuration register*/
#define DRAM_CTL_BASE4		(0x7E001300)
#define DRAM_CTL_SIZE4		(0x8)
#define P1_user_stat	DRAM_CTL_BASE4+(0x0) /*DRAM controller user_stat register*/
#define P1_user_cfg		DRAM_CTL_BASE4+(0x4) /*DRAM controller user_cfg register*/

/********************************************
* LCD Controller Registers
********************************************/
#define LCD_CTL_BASE           (0x0)
#define LCD_CTL_SIZE           (0x0)

/********************************************
* GPIO Controller Registers
********************************************/

/*Port A to Port Q and Memory Port Register*/
#define GPIO_CTL_BASE1		(0x7F008000)
#define GPIO_CTL_SIZE		(0x1D8)

#define GPACON		GPIO_CTL_BASE1+(0x0)  /*Port A Configuration Register*/
#define GPADAT		GPIO_CTL_BASE1+(0x4)  /*Port A Data Register*/
#define GPAPUD		GPIO_CTL_BASE1+(0x8)  /*Port A Pull-up/down Register*/
#define GPACONSLP	GPIO_CTL_BASE1+(0xC)  /*Port A Sleep mode Configuration Register*/
#define GPAPUDSLP	GPIO_CTL_BASE1+(0x10)  /*Port A Sleep mode Pull-up/down Register*/

#define GPBCON		GPIO_CTL_BASE1+(0x20)  /*Port B Configuration Register*/
#define GPBDAT		GPIO_CTL_BASE1+(0x24)  /*Port B Data Register*/
#define GPBPUD		GPIO_CTL_BASE1+(0x28)  /*Port B Pull-up/down Register*/
#define GPBCONSLP	GPIO_CTL_BASE1+(0x2C)  /*Port B Sleep mode Configuration Register*/
#define GPBPUDSLP	GPIO_CTL_BASE1+(0x30)  /*Port B Sleep mode Pull-up/down Register*/

#define GPCCON		GPIO_CTL_BASE1+(0x40)  /*Port C Configuration Register*/
#define GPCDAT		GPIO_CTL_BASE1+(0x44)  /*Port C Data Register*/
#define GPCPUD		GPIO_CTL_BASE1+(0x48)  /*Port C Pull-up/down Register*/
#define GPCCONSLP	GPIO_CTL_BASE1+(0x4C)  /*Port C Sleep mode Configuration Register*/
#define GPCPUDSLP	GPIO_CTL_BASE1+(0x50)  /*Port C Sleep mode Pull-up/down Register*/

#define GPDCON		GPIO_CTL_BASE1+(0x60)  /*Port D Configuration Register*/
#define GPDDAT		GPIO_CTL_BASE1+(0x64)  /*Port D Data Register*/
#define GPDPUD		GPIO_CTL_BASE1+(0x68)  /*Port D Pull-up/down Register*/
#define GPDCONSLP	GPIO_CTL_BASE1+(0x6C)  /*Port D Sleep mode Configuration Register*/
#define GPDPUDSLP	GPIO_CTL_BASE1+(0x70)  /*Port D Sleep mode Pull-up/down Register*/

#define GPECON		GPIO_CTL_BASE1+(0x80)  /*Port E Configuration Register*/
#define GPEDAT		GPIO_CTL_BASE1+(0x84)  /*Port E Data Register*/
#define GPEPUD		GPIO_CTL_BASE1+(0x88)  /*Port E Pull-up/down Register*/
#define GPECONSLP	GPIO_CTL_BASE1+(0x8C)  /*Port E Sleep mode Configuration Register*/
#define GPEPUDSLP	GPIO_CTL_BASE1+(0x90)  /*Port E Sleep mode Pull-up/down Register*/

#define GPFCON		GPIO_CTL_BASE1+(0xA0)  /*Port F Configuration Register*/
#define GPFDAT		GPIO_CTL_BASE1+(0xA4)  /*Port F Data Register*/
#define GPFPUD		GPIO_CTL_BASE1+(0xA8)  /*Port F Pull-up/down Register*/
#define GPFCONSLP	GPIO_CTL_BASE1+(0xAC)  /*Port F Sleep mode Configuration Register*/
#define GPFPUDSLP	GPIO_CTL_BASE1+(0xB0)  /*Port F Sleep mode Pull-up/down Register*/

#define GPGCON		GPIO_CTL_BASE1+(0xC0)  /*Port G Configuration Register*/
#define GPGDAT		GPIO_CTL_BASE1+(0xC4)  /*Port G Data Register*/
#define GPGPUD		GPIO_CTL_BASE1+(0xC8)  /*Port G Pull-up/down Register*/
#define GPGCONSLP	GPIO_CTL_BASE1+(0xCC)  /*Port G Sleep mode Configuration Register*/
#define GPGPUDSLP	GPIO_CTL_BASE1+(0xD0)  /*Port G Sleep mode Pull-up/down Register*/

#define GPHCON0		GPIO_CTL_BASE1+(0xE0)  /*Port H Configuration Register*/
#define GPHCON1		GPIO_CTL_BASE1+(0xE4)  /*Port H Configuration Register*/
#define GPHDAT		GPIO_CTL_BASE1+(0xE8)  /*Port H Data Register*/
#define GPHPUD		GPIO_CTL_BASE1+(0xEC)  /*Port H Pull-up/down Register*/
#define GPHCONSLP	GPIO_CTL_BASE1+(0xF0)  /*Port H Sleep mode Configuration Register*/
#define GPHPUDSLP	GPIO_CTL_BASE1+(0xF4)  /*Port H Sleep mode Pull-up/down Register*/

/*2010.8.5 added by jeff.du */
#define GPICON		GPIO_CTL_BASE1+(0x100)  /*Port I Configuration Register*/
#define GPIDAT		GPIO_CTL_BASE1+(0x104)  /*Port I Data Register*/
#define GPIPUD		GPIO_CTL_BASE1+(0x108)  /*Port I Pull-up/down Register*/
#define GPICONSLP	GPIO_CTL_BASE1+(0x10c)  /*Port I Sleep mode Configuration Register*/
#define GPIPUDSLP	GPIO_CTL_BASE1+(0x110)  /*Port I Sleep mode Pull-up/down Register*/

#define GPJCON		GPIO_CTL_BASE1+(0x120)  /*Port J Configuration Register*/
#define GPJDAT		GPIO_CTL_BASE1+(0x124)  /*Port J Data Register*/
#define GPJPUD		GPIO_CTL_BASE1+(0x128)  /*Port J Pull-up/down Register*/
#define GPJCONSLP	GPIO_CTL_BASE1+(0x12c)  /*Port J Sleep mode Configuration Register*/
#define GPJPUDSLP	GPIO_CTL_BASE1+(0x130)  /*Port J Sleep mode Pull-up/down Register*/

#define GPKCON0		GPIO_CTL_BASE1+(0x800)  /*Port K Configuration Register*/
#define GPKCON1		GPIO_CTL_BASE1+(0x804)  /*Port K Configuration Register*/
#define GPKDAT		GPIO_CTL_BASE1+(0x808)  /*Port K Data Register*/
#define GPKPUD		GPIO_CTL_BASE1+(0x80c)  /*Port K Pull-up/down Register*/

#define GPLCON0		GPIO_CTL_BASE1+(0x810)  /*Port L Configuration Register*/
#define GPLCON1		GPIO_CTL_BASE1+(0x814)  /*Port L Configuration Register*/
#define GPLDAT		GPIO_CTL_BASE1+(0x818)  /*Port L Data Register*/
#define GPLPUD		GPIO_CTL_BASE1+(0x81c)  /*Port L Pull-up/down Register*/

#define GPMCON		GPIO_CTL_BASE1+(0x820)  /*Port M Configuration Register*/
#define GPMDAT		GPIO_CTL_BASE1+(0x824)  /*Port M Data Register*/
#define GPMPUD		GPIO_CTL_BASE1+(0x828)  /*Port M Pull-up/down Register*/

#define GPNCON		GPIO_CTL_BASE1+(0x830)  /*Port N Configuration Register*/
#define GPNDAT		GPIO_CTL_BASE1+(0x834)  /*Port N Data Register*/
#define GPNPUD		GPIO_CTL_BASE1+(0x838)  /*Port N Pull-up/down Register*/

#define GPOCON		GPIO_CTL_BASE1+(0x140)  /*Port O Configuration Register*/
#define GPODAT		GPIO_CTL_BASE1+(0x144)  /*Port O Data Register*/
#define GPOPUD		GPIO_CTL_BASE1+(0x148)  /*Port O Pull-up/down Register*/
#define GPOCONSLP	GPIO_CTL_BASE1+(0x14c)  /*Port O Sleep mode Configuration Register*/
#define GPOPUDSLP	GPIO_CTL_BASE1+(0x150)  /*Port O Sleep mode Pull-up/down Register*/

#define GPPCON		GPIO_CTL_BASE1+(0x160)  /*Port P Configuration Register*/
#define GPPDAT		GPIO_CTL_BASE1+(0x164)  /*Port P Data Register*/
#define GPPPUD		GPIO_CTL_BASE1+(0x168)  /*Port P Pull-up/down Register*/
#define GPPCONSLP	GPIO_CTL_BASE1+(0x16c)  /*Port P Sleep mode Configuration Register*/
#define GPPPUDSLP	GPIO_CTL_BASE1+(0x170)  /*Port P Sleep mode Pull-up/down Register*/

#define GPQCON		GPIO_CTL_BASE1+(0x180)  /*Port Q Configuration Register*/
#define GPQDAT		GPIO_CTL_BASE1+(0x184)  /*Port Q Data Register*/
#define GPQPUD		GPIO_CTL_BASE1+(0x188)  /*Port Q Pull-up/down Register*/
#define GPQCONSLP	GPIO_CTL_BASE1+(0x18c)  /*Port Q Sleep mode Configuration Register*/
#define GPQPUDSLP	GPIO_CTL_BASE1+(0x190)  /*Port Q Sleep mode Pull-up/down Register*/
/*jeff.du added end */

#define SPCON		GPIO_CTL_BASE1+(0x1A0) /*Special Port COnfiguration Register*/
#define MEM0CONSLP0	GPIO_CTL_BASE1+(0x1C0) /*Memory Port 0 Sleep mode configure 0*/
#define MEM0CONSLP1	GPIO_CTL_BASE1+(0x1C4) /*Memory Port 0 Sleep mode configure 1*/
#define MEM0CONSLP	GPIO_CTL_BASE1+(0x1C8) /*Memory Port 1 Sleep mode configure*/
#define MEM0DRVCON	GPIO_CTL_BASE1+(0x1D0) /*Memory Port 0 Driver strength Control Register*/
#define MEM1DRVCON	GPIO_CTL_BASE1+(0x1D4) /*Memory Port 0 Driver strength Control Register*/

/*External Interrupt Register*/
#define GPIO_CTL_BASE2 (0x7F008900)
#define GPIO_CTL_SIZE2 (0x34)

#define EINT0CON0		GPIO_CTL_BASE2+(0x0)	/*External Interrupt configuration Register 0*/
#define EINT0CON1		GPIO_CTL_BASE2+(0x4)	/*External Interrupt configuration Register 0*/
#define EINT0FLTCON0	GPIO_CTL_BASE2+(0x10)	/*External Interrupt Filter Control Register 0*/
#define EINT0FLTCON1	GPIO_CTL_BASE2+(0x14)	/*External Interrupt Filter Control Register 1*/
#define EINT0FLTCON2	GPIO_CTL_BASE2+(0x18)	/*External Interrupt Filter Control Register 2*/
#define EINT0FLTCON3	GPIO_CTL_BASE2+(0x1C)	/*External Interrupt Filter Control Register 3*/
#define EINT0MASK		GPIO_CTL_BASE2+(0x20)	/*External Interrupt Mask Register*/
#define EINT0PEND		GPIO_CTL_BASE2+(0x24)	/*External Interrupt Pending Register*/
#define SPCONSLP		GPIO_CTL_BASE2-(0x80)	/*Special Port Sleep mode configure Register*/
#define SLPEN			GPIO_CTL_BASE2+(0x30)	/*Sleep mode Pad configure Register*/

/*Externel Interrupt Register*/
#define GPIO_CTL_BASE3 (0x7F008200)
#define GPIO_CTL_SIZE3 (0x8C)

#define EINT12CON		GPIO_CTL_BASE3+(0x0)	/*External Interrupt 1,2 Configuration Register*/
#define EINT34CON		GPIO_CTL_BASE3+(0x4)	/*External Interrupt 3,4 Configuration Register*/
#define EINT56CON		GPIO_CTL_BASE3+(0x8)	/*External Interrupt 5,6 Configuration Register*/
#define EINT78CON		GPIO_CTL_BASE3+(0xC)	/*External Interrupt 7,8 Configuration Register*/
#define EINT9CON		GPIO_CTL_BASE3+(0x10)	/*External Interrupt 9 Configuration Register*/
#define EINT12FLTCON	GPIO_CTL_BASE3+(0x20)	/*External Interrupt 1,2 Filter Control Register*/
#define EINT34FLTCON	GPIO_CTL_BASE3+(0x24)	/*External Interrupt 3,4 Filter Control Register*/
#define EINT56FLTCON	GPIO_CTL_BASE3+(0x28)	/*External Interrupt 5,6 Filter Control Register*/
#define EINT78FLTCON	GPIO_CTL_BASE3+(0x2C)	/*External Interrupt 7,8 Filter Control Register*/
#define EINT9FLTCON		GPIO_CTL_BASE3+(0x30)	/*External Interrupt 9 Filter Control Register*/
#define EINT12MASK		GPIO_CTL_BASE3+(0x40)	/*External Interrupt 1,2 Mask Register*/
#define EINT34MASK		GPIO_CTL_BASE3+(0x44)	/*External Interrupt 3,4 Mask Register*/
#define EINT56MASK		GPIO_CTL_BASE3+(0x48)	/*External Interrupt 5,6 Mask Register*/
#define EINT78MASK		GPIO_CTL_BASE3+(0x4C)	/*External Interrupt 7,8 Mask Register*/
#define EINT9MASK		GPIO_CTL_BASE3+(0x50)	/*External Interrupt 7,8 Mask Register*/
#define EINT12PEND		GPIO_CTL_BASE3+(0x60)	/*External Interrupt 1,2 Pending Register*/
#define EINT34PEND		GPIO_CTL_BASE3+(0x64)	/*External Interrupt 3,4 Pending Register*/
#define EINT56PEND		GPIO_CTL_BASE3+(0x68)	/*External Interrupt 5,6 Pending Register*/
#define EINT78PEND		GPIO_CTL_BASE3+(0x6C)	/*External Interrupt 7,8 Pending Register*/
#define EINT9PEND		GPIO_CTL_BASE3+(0x70)	/*External Interrupt 9 Pending Register*/
#define PRIORITY		GPIO_CTL_BASE3+(0x80)	/*Priority Control Register*/
#define SERVICE			GPIO_CTL_BASE3+(0x84)	/*Current Service Register*/
#define SERVICEPEND		GPIO_CTL_BASE3+(0x88)	/*Current Service Pending Register*/

/********************************************
* UART Control Registers
********************************************/
#define UART_CTL_BASE0		(0x7F005000)
#define UART_CTL_SIZE		(0x3C)

/* only the UART channel 0 register here*/
#define ULCON		(0x0)  /*UART channel 0 line control register*/
#define UCON		(0x4)  /*UART channel 0 line control register*/
#define UFCON		(0x8)  /*UART channel 0 FIFO control register*/
#define UMCON		(0xc)  /*UART channel 0 Modem control register*/
#define UTRSTAT		(0x10) /*UART channel 0 Tx/Rx status control register*/
#define UERSTAT		(0x14) /*UART channel 0 line Rx error register*/
#define UFSTAT		(0x18) /*UART channel 0 FIFO status register*/
#define UMSTAT		(0x1c) /*UART channel 0 Modem status register*/
#define UTXH		(0x20) /*UART channel 0 transmit buffer register*/
#define URXH		(0x24) /*UART channel 0 recerve buffer register*/
#define UBRDIV		(0x28) /*UART channel 0 Baud rate division register*/
#define UDIVSLOT	(0x2C) /*UART channel 0 Dividing slot register*/
#define UINTP		(0x30) /*UART channel 0 Interrupt Pending register*/
#define UINTSP		(0x34) /*UART channel 0 Interrupt Source Pending register*/
#define UINTM		(0x38) /*UART channel 0 Interrupt Mask register*/
#if 0
#define ULCON0		UART_CTL_BASE0+(0x0)  /*UART channel 0 line control register*/
#define UCON0		UART_CTL_BASE0+(0x4)  /*UART channel 0 line control register*/
#define UFCON0		UART_CTL_BASE0+(0x8)  /*UART channel 0 FIFO control register*/
#define UMCON0		UART_CTL_BASE0+(0xc)  /*UART channel 0 Modem control register*/
#define UTRSTAT0	UART_CTL_BASE0+(0x10) /*UART channel 0 Tx/Rx status control register*/
#define UERSTAT0	UART_CTL_BASE0+(0x14) /*UART channel 0 line Rx error register*/
#define UFSTAT0		UART_CTL_BASE0+(0x18) /*UART channel 0 FIFO status register*/
#define UMSTAT0		UART_CTL_BASE0+(0x1c) /*UART channel 0 Modem status register*/
#define UTXH0		UART_CTL_BASE0+(0x20) /*UART channel 0 transmit buffer register*/
#define URXH0		UART_CTL_BASE0+(0x24) /*UART channel 0 recerve buffer register*/
#define UBRDIV0		UART_CTL_BASE0+(0x28) /*UART channel 0 Baud rate division register*/
#define UDIVSLOT0	UART_CTL_BASE0+(0x2C) /*UART channel 0 Dividing slot register*/
#define UINTP0		UART_CTL_BASE0+(0x30) /*UART channel 0 Interrupt Pending register*/
#define UINTSP0		UART_CTL_BASE0+(0x34) /*UART channel 0 Interrupt Source Pending register*/
#define UINTM0		UART_CTL_BASE0+(0x38) /*UART channel 0 Interrupt Mask register*/
#endif

/* UART interrupt status */
#define MODEM (1<<3)  /*Modem interrupt*/
#define TXD   (1<<2)  /*Transmit interrupt*/
#define ERROR (1<<1)  /*Error interrupt*/
#define RXD   (1<<0)  /*Receive interrupt*/

/*****************************************/
/* Vectored Interrupt Controller Register*/
/*****************************************/

/* The S3C6410X supports 64 interrupt sources as shown in the table below.
 * The S3C6410X can not support performance monitoring interrupt of the ARM1176JZF-S.*/

#define VIC0_BASE  (0x71200000)
#define VIC0_SIZE  (0x280)

#define  VIC0IRQSTATUS			VIC0_BASE + (0x000)  /*  IRQ  Status  Register  */
#define  VIC0FIQSTATUS			VIC0_BASE + (0x004)  /*  FIQ  Status  Register  */
#define  VIC0RAWINTR			VIC0_BASE + (0x008)  /*  Raw  Interrupt  Status  Register  */
#define  VIC0INTSELECT			VIC0_BASE + (0x00C)  /*  Interrupt  Select  Register  */
#define  VIC0INTENABLE			VIC0_BASE + (0x010)  /*  Interrupt  Enable  Register  */
#define  VIC0INTENCLEAR			VIC0_BASE + (0x014)  /*  Interrupt  Enable  Clear  Register */
#define  VIC0SOFTINT			VIC0_BASE + (0x018)  /*  Software  Interrupt  Register  */
#define  VIC0SOFTINTCLEAR		VIC0_BASE + (0x01C)  /*  Software  Interrupt  Clear  Register */
#define  VIC0PROTECTION			VIC0_BASE + (0x020)  /*  Protection  Enable  Register */
#define  VIC0SWPRIORITYMASK		VIC0_BASE + (0x024)  /*  Software  Priority  Mask  Register  */
#define  VIC0PRIORITYDAISY		VIC0_BASE + (0x028)  /*  Vector  Priority  Register  for  Daisy  Chain  */
#define  VIC0VECTADDR0			VIC0_BASE + (0x100)  /*  Vector  Address  0  Register  */
#define  VIC0VECTADDR1			VIC0_BASE + (0x104)  /*  Vector  Address  1  Register  */
#define  VIC0VECTADDR2			VIC0_BASE + (0x108)  /*  Vector  Address  2  Register  */
#define  VIC0VECTADDR3			VIC0_BASE + (0x10C)  /*  Vector  Address  3  Register  */
#define  VIC0VECTADDR4			VIC0_BASE + (0x110)  /*  Vector  Address  4  Register  */
#define  VIC0VECTADDR5			VIC0_BASE + (0x114)  /*  Vector  Address  5  Register  */
#define  VIC0VECTADDR6			VIC0_BASE + (0x118)  /*  Vector  Address  6  Register  */
#define  VIC0VECTADDR7			VIC0_BASE + (0x11C)  /*  Vector  Address  7  Register  */
#define  VIC0VECTADDR8			VIC0_BASE + (0x120)  /*  Vector  Address  8  Register  */
#define  VIC0VECTADDR9			VIC0_BASE + (0x124)  /*  Vector  Address  9  Register  */
#define  VIC0VECTADDR10			VIC0_BASE + (0x128)  /*  Vector  Address  10  Register  */
#define  VIC0VECTADDR11			VIC0_BASE + (0x12C)  /*  Vector  Address  11  Register  */
#define  VIC0VECTADDR12			VIC0_BASE + (0x130)  /*  Vector  Address  12  Register  */
#define  VIC0VECTADDR13			VIC0_BASE + (0x134)  /*  Vector  Address  13  Register  */
#define  VIC0VECTADDR14   		VIC0_BASE + (0x138)  /*  Vector  Address  14  Register  */
#define  VIC0VECTADDR15   		VIC0_BASE + (0x13C)  /*  Vector  Address  15  Register  */
#define  VIC0VECTADDR16   		VIC0_BASE + (0x140)  /*  Vector  Address  16  Register  */
#define  VIC0VECTADDR17   		VIC0_BASE + (0x144)  /*  Vector  Address  17  Register  */
#define  VIC0VECTADDR18   		VIC0_BASE + (0x148)  /*  Vector  Address  18  Register  */
#define  VIC0VECTADDR19   		VIC0_BASE + (0x14C)  /*  Vector  Address  19  Register  */
#define  VIC0VECTADDR20   		VIC0_BASE + (0x150)  /*  Vector  Address  20  Register  */
#define  VIC0VECTADDR21   		VIC0_BASE + (0x154)  /*  Vector  Address  21  Register  */
#define  VIC0VECTADDR22   		VIC0_BASE + (0x158)  /*  Vector  Address  22  Register  */
#define  VIC0VECTADDR23   		VIC0_BASE + (0x15C)  /*  Vector  Address  23  Register  */
#define  VIC0VECTADDR24   		VIC0_BASE + (0x160)  /*  Vector  Address  24  Register  */
#define  VIC0VECTADDR25   		VIC0_BASE + (0x164)  /*  Vector  Address  25  Register  */
#define  VIC0VECTADDR26   		VIC0_BASE + (0x168)  /*  Vector  Address  26  Register  */
#define  VIC0VECTADDR27   		VIC0_BASE + (0x16C)  /*  Vector  Address  27  Register  */
#define  VIC0VECTADDR28   		VIC0_BASE + (0x170)  /*  Vector  Address  28  Register  */
#define  VIC0VECTADDR29   		VIC0_BASE + (0x174)  /*  Vector  Address  29  Register  */
#define  VIC0VECTADDR30   		VIC0_BASE + (0x178)  /*  Vector  Address  30  Register  */
#define  VIC0VECTADDR31   		VIC0_BASE + (0x17C)  /*  Vector  Address  31  Register  */
#define  VIC0VECPRIORITY0 		VIC0_BASE + (0x200)  /*  Vector  Priority  0  Register  */
#define  VIC0VECTPRIORITY1  	VIC0_BASE + (0x204)  /*  Vector  Priority  1  Register  */
#define  VIC0VECTPRIORITY2  	VIC0_BASE + (0x208)  /*  Vector  Priority  2  Register  */
#define  VIC0VECTPRIORITY3  	VIC0_BASE + (0x20C)  /*  Vector  Priority  3  Register  */
#define  VIC0VECTPRIORITY4  	VIC0_BASE + (0x210)  /*  Vector  Priority  4  Register  */
#define  VIC0VECTPRIORITY5  	VIC0_BASE + (0x214)  /*  Vector  Priority  5  Register  */
#define  VIC0VECTPRIORITY6  	VIC0_BASE + (0x218)  /*  Vector  Priority  6  Register  */
#define  VIC0VECTPRIORITY7  	VIC0_BASE + (0x21C)  /*  Vector  Priority  7  Register  */
#define  VIC0VECTPRIORITY8  	VIC0_BASE + (0x220)  /*  Vector  Priority  8  Register  */
#define  VIC0VECTPRIORITY9  	VIC0_BASE + (0x224)  /*  Vector  Priority  9  Register  */
#define  VIC0VECTPRIORITY10  	VIC0_BASE + (0x228)  /*  Vector  Priority  10  Register  */
#define  VIC0VECTPRIORITY11  	VIC0_BASE + (0x22C)  /*  Vector  Priority  11  Register  */
#define  VIC0VECTPRIORITY12  	VIC0_BASE + (0x230)  /*  Vector  Priority  12  Register  */
#define  VIC0VECTPRIORITY13  	VIC0_BASE + (0x234)  /*  Vector  Priority  13  Register  */
#define  VIC0VECTPRIORITY14  	VIC0_BASE + (0x238)  /*  Vector  Priority  14  Register  */
#define  VIC0VECTPRIORITY15  	VIC0_BASE + (0x23C)  /*  Vector  Priority  15  Register  */
#define  VIC0VECTPRIORITY16  	VIC0_BASE + (0x240)  /*  Vector  Priority  16  Register  */
#define  VIC0VECTPRIORITY17  	VIC0_BASE + (0x244)  /*  Vector  Priority  17  Register  */
#define  VIC0VECTPRIORITY18  	VIC0_BASE + (0x248)  /*  Vector  Priority  18  Register  */
#define  VIC0VECTPRIORITY19  	VIC0_BASE + (0x24C)  /*  Vector  Priority  19  Register  */
#define  VIC0VECTPRIORITY20  	VIC0_BASE + (0x250)  /*  Vector  Priority  20  Register  */
#define  VIC0VECTPRIORITY21  	VIC0_BASE + (0x254)  /*  Vector  Priority  21  Register  */
#define  VIC0VECTPRIORITY22  	VIC0_BASE + (0x258)  /*  Vector  Priority  22  Register  */
#define  VIC0VECTPRIORITY23  	VIC0_BASE + (0x25C)  /*  Vector  Priority  23  Register  */
#define  VIC0VECTPRIORITY24  	VIC0_BASE + (0x260)  /*  Vector  Priority  24  Register  */
#define  VIC0VECTPRIORITY25  	VIC0_BASE + (0x264)  /*  Vector  Priority  25  Register  */
#define  VIC0VECTPRIORITY26  	VIC0_BASE + (0x268)  /*  Vector  Priority  26  Register  */
#define  VIC0VECTPRIORITY27  	VIC0_BASE + (0x26C)  /*  Vector  Priority  27  Register  */
#define  VIC0VECTPRIORITY28  	VIC0_BASE + (0x270)  /*  Vector  Priority  28  Register  */
#define  VIC0VECTPRIORITY29  	VIC0_BASE + (0x274)  /*  Vector  Priority  29  Register  */
#define  VIC0VECTPRIORITY30  	VIC0_BASE + (0x278)  /*  Vector  Priority  30  Register  */
#define  VIC0VECTPRIORITY31  	VIC0_BASE + (0x27C)  /*  Vector  Priority  31  Register  */
#define  VIC0ADDRESS 			VIC0_BASE + (0xF00)  /*  Vector  Address  Register  */

#define VIC1_BASE  (0x71300000)
#define VIC1_SIZE  (0x0)

#define  VIC1IRQSTATUS			VIC1_BASE + (0x000)  /*  IRQ  Status  Register  */
#define  VIC1FIQSTATUS			VIC1_BASE + (0x004)  /*  FIQ  Status  Register  */
#define  VIC1RAWINTR			VIC1_BASE + (0x008)  /*  Raw  Interrupt  Status  Register  */
#define  VIC1INTSELECT			VIC1_BASE + (0x00C)  /*  Interrupt  Select  Register  */
#define  VIC1INTENABLE			VIC1_BASE + (0x010)  /*  Interrupt  Enable  Register  */
#define  VIC1INTENCLEAR			VIC1_BASE + (0x014)  /*  Interrupt  Enable  Clear  Register */
#define  VIC1SOFTINT			VIC1_BASE + (0x018)  /*  Software  Interrupt  Register  */
#define  VIC1SOFTINTCLEAR		VIC1_BASE + (0x01C)  /*  Software  Interrupt  Clear  Register */
#define  VIC1PROTECTION			VIC1_BASE + (0x020)  /*  Protection  Enable  Register */
#define  VIC1SWPRIORITYMASK		VIC1_BASE + (0x024)  /*  Software  Priority  Mask  Register  */
#define  VIC1PRIORITYDAISY		VIC1_BASE + (0x028)  /*  Vector  Priority  Register  for  Daisy  Chain  */
#define  VIC1VECTADDR0			VIC1_BASE + (0x100)  /*  Vector  Address  0  Register  */
#define  VIC1VECTADDR1			VIC1_BASE + (0x104)  /*  Vector  Address  1  Register  */
#define  VIC1VECTADDR2			VIC1_BASE + (0x108)  /*  Vector  Address  2  Register  */
#define  VIC1VECTADDR3			VIC1_BASE + (0x10C)  /*  Vector  Address  3  Register  */
#define  VIC1VECTADDR4			VIC1_BASE + (0x110)  /*  Vector  Address  4  Register  */
#define  VIC1VECTADDR5			VIC1_BASE + (0x114)  /*  Vector  Address  5  Register  */
#define  VIC1VECTADDR6			VIC1_BASE + (0x118)  /*  Vector  Address  6  Register  */
#define  VIC1VECTADDR7			VIC1_BASE + (0x11C)  /*  Vector  Address  7  Register  */
#define  VIC1VECTADDR8			VIC1_BASE + (0x120)  /*  Vector  Address  8  Register  */
#define  VIC1VECTADDR9			VIC1_BASE + (0x124)  /*  Vector  Address  9  Register  */
#define  VIC1VECTADDR10			VIC1_BASE + (0x128)  /*  Vector  Address  10  Register  */
#define  VIC1VECTADDR11			VIC1_BASE + (0x12C)  /*  Vector  Address  11  Register  */
#define  VIC1VECTADDR12			VIC1_BASE + (0x130)  /*  Vector  Address  12  Register  */
#define  VIC1VECTADDR13			VIC1_BASE + (0x134)  /*  Vector  Address  13  Register  */
#define  VIC1VECTADDR14   		VIC1_BASE + (0x138)  /*  Vector  Address  14  Register  */
#define  VIC1VECTADDR15   		VIC1_BASE + (0x13C)  /*  Vector  Address  15  Register  */
#define  VIC1VECTADDR16   		VIC1_BASE + (0x140)  /*  Vector  Address  16  Register  */
#define  VIC1VECTADDR17   		VIC1_BASE + (0x144)  /*  Vector  Address  17  Register  */
#define  VIC1VECTADDR18   		VIC1_BASE + (0x148)  /*  Vector  Address  18  Register  */
#define  VIC1VECTADDR19   		VIC1_BASE + (0x14C)  /*  Vector  Address  19  Register  */
#define  VIC1VECTADDR20   		VIC1_BASE + (0x150)  /*  Vector  Address  20  Register  */
#define  VIC1VECTADDR21   		VIC1_BASE + (0x154)  /*  Vector  Address  21  Register  */
#define  VIC1VECTADDR22   		VIC1_BASE + (0x158)  /*  Vector  Address  22  Register  */
#define  VIC1VECTADDR23   		VIC1_BASE + (0x15C)  /*  Vector  Address  23  Register  */
#define  VIC1VECTADDR24   		VIC1_BASE + (0x160)  /*  Vector  Address  24  Register  */
#define  VIC1VECTADDR25   		VIC1_BASE + (0x164)  /*  Vector  Address  25  Register  */
#define  VIC1VECTADDR26   		VIC1_BASE + (0x168)  /*  Vector  Address  26  Register  */
#define  VIC1VECTADDR27   		VIC1_BASE + (0x16C)  /*  Vector  Address  27  Register  */
#define  VIC1VECTADDR28   		VIC1_BASE + (0x170)  /*  Vector  Address  28  Register  */
#define  VIC1VECTADDR29   		VIC1_BASE + (0x174)  /*  Vector  Address  29  Register  */
#define  VIC1VECTADDR30   		VIC1_BASE + (0x178)  /*  Vector  Address  30  Register  */
#define  VIC1VECTADDR31   		VIC1_BASE + (0x17C)  /*  Vector  Address  31  Register  */
#define  VIC1VECPRIORITY0 		VIC1_BASE + (0x200)  /*  Vector  Priority  0  Register  */
#define  VIC1VECTPRIORITY1  	VIC1_BASE + (0x204)  /*  Vector  Priority  1  Register  */
#define  VIC1VECTPRIORITY2  	VIC1_BASE + (0x208)  /*  Vector  Priority  2  Register  */
#define  VIC1VECTPRIORITY3  	VIC1_BASE + (0x20C)  /*  Vector  Priority  3  Register  */
#define  VIC1VECTPRIORITY4  	VIC1_BASE + (0x210)  /*  Vector  Priority  4  Register  */
#define  VIC1VECTPRIORITY5  	VIC1_BASE + (0x214)  /*  Vector  Priority  5  Register  */
#define  VIC1VECTPRIORITY6  	VIC1_BASE + (0x218)  /*  Vector  Priority  6  Register  */
#define  VIC1VECTPRIORITY7  	VIC1_BASE + (0x21C)  /*  Vector  Priority  7  Register  */
#define  VIC1VECTPRIORITY8  	VIC1_BASE + (0x220)  /*  Vector  Priority  8  Register  */
#define  VIC1VECTPRIORITY9  	VIC1_BASE + (0x224)  /*  Vector  Priority  9  Register  */
#define  VIC1VECTPRIORITY10  	VIC1_BASE + (0x228)  /*  Vector  Priority  10  Register  */
#define  VIC1VECTPRIORITY11  	VIC1_BASE + (0x22C)  /*  Vector  Priority  11  Register  */
#define  VIC1VECTPRIORITY12  	VIC1_BASE + (0x230)  /*  Vector  Priority  12  Register  */
#define  VIC1VECTPRIORITY13  	VIC1_BASE + (0x234)  /*  Vector  Priority  13  Register  */
#define  VIC1VECTPRIORITY14  	VIC1_BASE + (0x238)  /*  Vector  Priority  14  Register  */
#define  VIC1VECTPRIORITY15  	VIC1_BASE + (0x23C)  /*  Vector  Priority  15  Register  */
#define  VIC1VECTPRIORITY16  	VIC1_BASE + (0x240)  /*  Vector  Priority  16  Register  */
#define  VIC1VECTPRIORITY17  	VIC1_BASE + (0x244)  /*  Vector  Priority  17  Register  */
#define  VIC1VECTPRIORITY18  	VIC1_BASE + (0x248)  /*  Vector  Priority  18  Register  */
#define  VIC1VECTPRIORITY19  	VIC1_BASE + (0x24C)  /*  Vector  Priority  19  Register  */
#define  VIC1VECTPRIORITY20  	VIC1_BASE + (0x250)  /*  Vector  Priority  20  Register  */
#define  VIC1VECTPRIORITY21  	VIC1_BASE + (0x254)  /*  Vector  Priority  21  Register  */
#define  VIC1VECTPRIORITY22  	VIC1_BASE + (0x258)  /*  Vector  Priority  22  Register  */
#define  VIC1VECTPRIORITY23  	VIC1_BASE + (0x25C)  /*  Vector  Priority  23  Register  */
#define  VIC1VECTPRIORITY24  	VIC1_BASE + (0x260)  /*  Vector  Priority  24  Register  */
#define  VIC1VECTPRIORITY25  	VIC1_BASE + (0x264)  /*  Vector  Priority  25  Register  */
#define  VIC1VECTPRIORITY26  	VIC1_BASE + (0x268)  /*  Vector  Priority  26  Register  */
#define  VIC1VECTPRIORITY27  	VIC1_BASE + (0x26C)  /*  Vector  Priority  27  Register  */
#define  VIC1VECTPRIORITY28  	VIC1_BASE + (0x270)  /*  Vector  Priority  28  Register  */
#define  VIC1VECTPRIORITY29  	VIC1_BASE + (0x274)  /*  Vector  Priority  29  Register  */
#define  VIC1VECTPRIORITY30  	VIC1_BASE + (0x278)  /*  Vector  Priority  30  Register  */
#define  VIC1VECTPRIORITY31  	VIC1_BASE + (0x27C)  /*  Vector  Priority  31  Register  */
#define  VIC1ADDRESS 			VIC1_BASE + (0xF00)  /*  Vector  Address  Register  */

/*********************/
/* Interrupt source  */
/*********************/
/* Support 64 interrupt for total, using TrustZone Interrupt Controller TZIC0 and TZIC1 */
#define INT_ADC				(31)	/*	ADC EOC interrupt VIC1 */
#define INT_PENDNUP			(30)	/*	ADC Pen down/up interrupt VIC1 */
#define INT_SEC				(29)	/*	Security interrupt VIC1 */
#define INT_RTC_ALARM		(28)	/*	RTC alarm interrupt VIC1 */
#define INT_IrDA			(27)	/*	IrDA interrupt VIC1 */
#define INT_OTG				(26)	/*	USB OTG interrupt VIC1 */
#define INT_HSMMC1			(25)	/*	HSMMC1 interrupt VIC1 */
#define INT_HSMMC0			(24)	/*	HSMMC0 interrupt VIC1 */
#define INT_HOSTIF			(23)	/*	Host Interface interrupt VIC1 */
#define INT_MSM				(22)	/*	MSM modem I/F interrupt VIC1 */
#define INT_EINT4			(21)	/*	External interrupt Group 1 ~ Group 9 VIC1 */
#define INT_HSIrx			(20)	/*	HSI Rx interrupt VIC1 */
#define INT_HSItx			(19)	/*	HSI Tx interrupt VIC1 */
#define INT_I2C0			(18)	/*	I2C 0 interrupt VIC1 */
#define INT_SPI1/INT_HSMMC2	(17)	/*	SPI1 interrupt or HSMMC2 interrupt VIC1 */
#define INT_SPI0			(16)	/*	SPI0 interrupt VIC1 */
#define INT_UHOST			(15)	/*	USB Host interrupt VIC1 */
#define INT_CFC				(14)	/*	CFCON interrupt VIC1 */
#define INT_NFC				(13)	/*	NFCON interrupt VIC1 */
#define INT_ONENAND1		(12)	/*	OneNAND interrupt from bank 1 VIC1 */
#define INT_ONENAND0		(11)	/*	OneNAND interrupt from bank 0 VIC1 */
#define INT_DMA1			(10)	/*	DMA1 interrupt VIC1 */
#define INT_DMA0			(9)	/*	DMA0 interrupt VIC1 */
#define INT_UART3			(8)	/*	UART3 interrupt VIC1 */
#define INT_UART2			(7)	/*	UART2 interrupt VIC1 */
#define INT_UART1			(6)	/*	UART1 interrupt VIC1 */
#define INT_UART0			(5)	/*	UART0 interrupt VIC1 */
#define INT_AC97			(4)	/*	AC97 interrupt VIC1 */
#define INT_PCM1			(3)	/*	PCM1 interrupt VIC1 */
#define INT_PCM0			(2)	/*	PCM0 interrupt VIC1 */
#define INT_EINT3			(1)	/*	External interrupt 20 ~ 27 VIC1 */
#define INT_EINT2			(0)	/*	External interrupt 12 ~ 19 VIC1 */
#define INT_LCD[2]			(31)	/*	LCD interrupt. System I/F done VICO */
#define INT_LCD[1]			(30)	/*	LCD interrupt. VSYNC interrupt VICO */
#define INT_LCD[0]			(29)	/*	LCD interrupt. FIFO underrun VICO */
#define INT_TIMER4			(28)	/*	Timer 4 interrupt VICO */
#define INT_TIMER3			(27)	/*	Timer 3 interrupt VICO */
#define INT_WDT				(26)	/*	Watchdog timer interrupt VICO */
#define INT_TIMER2			(25)	/*	Timer 2 interrupt VICO */
#define INT_TIMER1			(24)	/*	Timer 1 interrupt VICO */
#define INT_TIMER0			(23)	/*	Timer 0 interrupt VICO */
#define INT_KEYPAD			(22)	/*	Keypad interrupt VICO */
#define INT_ARM_DMAS		(21)	/*	ARM DMAS interrupt VICO */
#define INT_ARM_DMA			(20)	/*	ARM DMA interrupt VICO */
#define INT_ARM_DMAERR		(19)	/*	ARM DMA Error interrupt VICO */
#define INT_SDMA1			(18)	/*	Secure DMA1 interrupt VICO */
#define INT_SDMA0			(17)	/*	Secure DMA0 interrupt VICO */
#define INT_MFC				(16)	/*	MFC interrupt VICO */
#define INT_JPEG			(15)	/*	JPEG interrupt VICO */
#define INT_BATF			(14)	/*	Battery fault interrupt VICO */
#define INT_SCALER			(13)	/*	TV Scaler interrupt VICO */
#define INT_TVENC			(12)	/*	TV Encoder interrupt VICO */
#define INT_2D				(11)	/*	2D interrupt VICO */
#define INT_ROTATOR			(10)	/*	Rotator interrupt VICO */
#define INT_POST0			(9)	/*	Post processor interrupt VICO */
#define INT_3D				(8)	/*  Graphic Controller interrupt VICO */
#define Reserved			(7)	/*	Reserved VICO */
#define INT_I2S0			(6)	/*	| INT_I2S1 |INT_I2SV40 I2S 0 interrupt or I2S 1 interrupt or I2S V40 interrupt VICO */
#define INT_I2C1			(5)	/*	I2C 1 interrupt VICO */
#define INT_CAMIF_P			(4)	/*	Camera interface interrupt VICO */
#define INT_CAMIF_C			(3)	/*	Camera interface interrupt VICO */
#define INT_RTC_TIC			(2)	/*	RTC TIC interrupt VICO */
#define INT_EINT1			(1)	/*	External interrupt 4 ~ 11 VICO */
#define INT_EINT0			(0)	/*	External interrupt 0 ~ 3 VICO */

/*********************/
/* RTC Registers     */
/*********************/
#define RTC_CTL_BASE		(0x7E005000)
#define RTC_CTL_SIZE		(0x0)

#define INTP		RTC_CTL_BASE+(0x30) /*Interrupt Pending Register*/
#define RTCCON		RTC_CTL_BASE+(0x40) /*RTC control Register*/
#define TICNT		RTC_CTL_BASE+(0x44) /*Tick control Register*/
#define RTCALM		RTC_CTL_BASE+(0x50) /*RTC alarm control Register*/
#define ALMSEC		RTC_CTL_BASE+(0x54) /*Alarm second date Register*/
#define ALMMIN		RTC_CTL_BASE+(0x58) /*Alarm minute data Register*/
#define ALMHOUR		RTC_CTL_BASE+(0x5c) /*Alarm hour data Register*/
#define ALMDATE		RTC_CTL_BASE+(0x60) /*Alarm date data Register*/
#define ALMMON		RTC_CTL_BASE+(0x64) /*Alarm month data Register*/
#define ALMYEAR		RTC_CTL_BASE+(0x68) /*Alarm data Register*/
#define BCDSEC		RTC_CTL_BASE+(0x70) /*BCD second Register*/
#define BCDMIN		RTC_CTL_BASE+(0x74) /*BCD minute Register*/
#define BCDHOUR		RTC_CtL_BASE+(0x78) /*BCD hour Register*/
#define BCDDATE		RTC_CTL_BASE+(0x7c) /*BCD date Register*/
#define BCDDAY		RTC_CTL_BASE+(0x80) /*BCD day Register*/
#define BCDMON		RTC_CTL_BASE+(0x84) /*BCD month Register*/
#define BCDYEAR		RTC_CTL_BASE+(0x88) /*BCD year Register*/
#define CURTICCNT	RTC_CTL_BASE+(0x90) /*Current Tick time counter Register*/

/***************************/
/* WatchDog Timer Registers     */
/***************************/

#define WT_TIMER_BASE (0x7E004000)

#define WTCON		WT_TIMER_BASE+(0x0) /*Watchdog timer control register*/
#define WTDAT		WT_TIMER_BASE+(0x4) /*Watchdog timer data register*/
#define WTCNT		WT_TIMER_BASE+(0x8) /*Watchdog timer count register*/
#define WTCLRINT	WT_TIMER_BASE+(0xC) /*Watchdog timer interrupt clear register*/

/***************************/
/* PWM Timer Registers     */
/***************************/
#define PWM_CTL_BASE		(0x7F006000)
#define PWM_CTL_SIZE		(0x48)
#if 0
#define TCFG0		(0x0)  /*Timer Configuration Register 0 that configures the two 8-bit Prescaler and DeadZone Length*/
#define TCFG1		(0x4)  /*Timer Configuration Register 1 that controls 5 MUX and DMA Mode Select Bit*/
#define TCON		(0x8)  /*Timer Control Register*/
#define TCNTB0		(0xc)  /*Timer 0 Count Buffer Register*/
#define TCMPB0		(0x10) /*Timer 0 Compare Buffer Register*/
#define TCNTO0		(0x14) /*Timer 0 Count Observation Register*/
#define TCNTB1		(0x18) /*Timer 1 Count Buffer Register*/
#define TCMPB1		(0x1c) /*Timer 1 Cpmpare Buffer Register*/
#define TCNTO1		(0x20) /*Timer 1 Count Observation Register*/
#define TCNTB2		(0x24) /*Timer 2 Count Buffer Register*/
#define TCNTO2		(0x2c) /*Timer 2 Count Observation Register*/
#define TCNTB3		(0x30) /*Timer 3 Count Buffer Register*/
#define TCNTO3		(0x38) /*Timer 3 Count Observation Register*/
#define TCNTB4		(0x3c) /*Timer 4 Count Buffer Register*/
#define TCNTO4		(0x40) /*Timer 4 Count Observatin Register*/
#define TINT_CSTAT  (0x44) /*Timer interrupt Control and Status Register*/
#endif

#define TCFG0		(0x0)  /*Timer Configuration Register 0 that configures the two 8-bit Prescaler and DeadZone Length*/
#define TCFG1		(0x4)  /*Timer Configuration Register 1 that controls 5 MUX and DMA Mode Select Bit*/
#define TCON		(0x8)  /*Timer Control Register*/
#define TCNTB0		(0xc)  /*Timer 0 Count Buffer Register*/
#define TCMPB0		(0x10) /*Timer 0 Compare Buffer Register*/
#define TCNTO0		(0x14) /*Timer 0 Count Observation Register*/
#define TCNTB1		(0x18) /*Timer 1 Count Buffer Register*/
#define TCMPB1		(0x1c) /*Timer 1 Cpmpare Buffer Register*/
#define TCNTO1		(0x20) /*Timer 1 Count Observation Register*/
#define TCNTB2		(0x24) /*Timer 2 Count Buffer Register*/
#define TCNTO2		(0x2c) /*Timer 2 Count Observation Register*/
#define TCNTB3		(0x30) /*Timer 3 Count Buffer Register*/
#define TCNTO3		(0x38) /*Timer 3 Count Observation Register*/
#define TCNTB4		(0x3c) /*Timer 4 Count Buffer Register*/
#define TCNTO4		(0x40) /*Timer 4 Count Observatin Register*/
#define TINT_CSTAT  (0x44) /*Timer interrupt Control and Status Register*/
#define S3C6410_TIMER_NUM 5

/*******************************/
/* System Controller Registers */
/*******************************/
#define SYS_CTL_BASE (0x7E000000)
#define SYS_CTL_SIZE (0x0)

#define APLL_LOCK		SYS_CTL_BASE+(0xF000) /* Control PLL locking period for APLL */
#define MPLL_LOCK		SYS_CTL_BASE+(0xF004) /* Control PLL locking period for MPLL */
#define EPLL_LOCK		SYS_CTL_BASE+(0xF008) /* Control PLL locking period for EPLL */
#define APLL_CON		SYS_CTL_BASE+(0xF00C) /* Control PLL output frequency for APLL */
#define MPLL_CON		SYS_CTL_BASE+(0xF010) /* Control PLL output frequency for MPLL */
#define EPLL_CON0		SYS_CTL_BASE+(0xF014) /* Control PLL output frequency for EPLL */
#define EPLL_CON1		SYS_CTL_BASE+(0xF018) /* Control PLL output frequency for EPLL */
#define CLK_SRC			SYS_CTL_BASE+(0xF01C) /* Select clock source */
#define CLK_DIV0		SYS_CTL_BASE+(0xF020) /* Set clock divider ratio */
#define CLK_DIV1		SYS_CTL_BASE+(0xF024) /* Set clock divider ratio */
#define CLK_DIV2		SYS_CTL_BASE+(0xF028) /* Set clock divider ratio */
#define CLK_OUT			SYS_CTL_BASE+(0xF02C) /* Select clock output */
#define HCLK_GATE		SYS_CTL_BASE+(0xF030) /* Control HCLK clock gating */
#define PCLK_GATE		SYS_CTL_BASE+(0xF034) /* Control PCLK clock gating */
#define SCLK_GATE		SYS_CTL_BASE+(0xF038) /* Control SCLK clock gating */
#define MEM0_CLK_GATE		SYS_CTL_BASE+(0xF03C) /* Control MEM0 clock gating */
#define AHB_CON0		SYS_CTL_BASE+(0xF100) /* Configure AHB I/P/X/F bus */
#define AHB_CON1		SYS_CTL_BASE+(0xF104) /* Configure AHB M1/M0/T1/T0 bus */
#define AHB_CON2		SYS_CTL_BASE+(0xF108) /* Configure AHB R/S1/S0 bus */
#define CLK_SRC2		SYS_CTL_BASE+(0xF10C) /* Select Audio2 clock source */
#define SDMA_SEL		SYS_CTL_BASE+(0xF110) /* Select secure DMA input */
#define SYS_ID			SYS_CTL_BASE+(0xF118) /* System ID for revision and pass */
#define SYS_OTHERS		SYS_CTL_BASE+(0xF11C) /* SYSCON others control register */
#define MEM_SYS_CFG		SYS_CTL_BASE+(0xF120) /* Configure memory subsystem */
#define QOS_OVERRIDE1		SYS_CTL_BASE+(0xF128) /* Override DMC1 QOS */
#define MEM_CFG_STAT		SYS_CTL_BASE+(0xF12C) /* Memory subsystem setup status */
#define PWR_CFG			SYS_CTL_BASE+(0xF804) /* Configure power manager */
#define EINT_MASK		SYS_CTL_BASE+(0xF808) /* Configure EINTSYS_CTL_BASE+(0xexternal interrupt) mask */
#define NORMAL_CFG		SYS_CTL_BASE+(0xF810) /* Configure power manager at NORMAL mode */
#define STOP_CFG		SYS_CTL_BASE+(0xF814) /* Configure power manager at STOP mode */
#define SLEEP_CFG		SYS_CTL_BASE+(0xF818) /* Configure power manager at SLEEP mode */
#define STOP_MEM_CFG		SYS_CTL_BASE+(0xF81C) /* Configure memory power at STOP mode */
#define OSC_FREQ		SYS_CTL_BASE+(0xF820) /* Oscillator frequency scale counter */
#define OSC_STABLE		SYS_CTL_BASE+(0xF824) /* Oscillator pad stable counter */
#define PWR_STABLE		SYS_CTL_BASE+(0xF828) /* Power stable counter */
#define MTC_STABLE		SYS_CTL_BASE+(0xF830) /* MTC stable counter */
#define MISC_CON		SYS_CTL_BASE+(0xF838) /* Bus control/SYNC667 control */
#define OTHERS			SYS_CTL_BASE+(0xF900) /* Others control register */
#define RST_STAT		SYS_CTL_BASE+(0xF904) /* Reset status register */
#define WAKEUP_STAT		SYS_CTL_BASE+(0xF908) /* Wakeup status register */
#define BLK_PWR_STAT		SYS_CTL_BASE+(0xF90C) /* Block power status register */
#define INFORM0			SYS_CTL_BASE+(0xFA00) /* Information register0 */
#define INFORM1			SYS_CTL_BASE+(0xFA04) /* Information register1 */
#define INFORM2			SYS_CTL_BASE+(0xFA08) /* Information register2 */
#define INFORM3			SYS_CTL_BASE+(0xFA0C) /* Information register3 */

struct s3c6410x_dramctl
{
	u32 p1memstat;     /*DRAM controller status register*/
	u32 p1memccmd;     /*DRAM controller command register*/
	u32 p1directcmd;   /*DRAM controller direct command register*/
	u32 p1memcfG;      /*DRAM controller memory config register*/
	u32 p1refresh;     /*DRAM controller memory reflash period register*/
	u32 pncaslat;      /*DRAM controller CAS latency register*/
	u32 p1t_dqss;      /*DRAM controller t_DQSS register*/
	u32 p1t_mrd;       /*DRAM controller t_MRD register*/
	u32 p1t_ras;       /*DRAM controller t_RAS register*/
	u32 p1t_rc;        /*DRAM controller t_RC register*/
	u32 p1t_rcd;       /*DRAM controller t_RCD register*/
	u32 p1t_rfc;       /*DRAM controller t_RFC register*/
	u32 p1t_rp;        /*DRAM controller t_RP register*/
	u32 p1t_rrd;       /*DRAM controller t_RRD register*/
	u32 p1t_wr;        /*DRAM controller t_WR register*/
	u32 p1t_wtr;       /*DRAM controller t_WTR register*/
	u32 p1t_xp;        /*DRAM controller t_XP register*/
	u32 p1t_xsr;       /*DRAM controller t_XSR register*/
	u32 p1t_esr;       /*DRAM controller t_ESR register*/
	u32 p1memcfg2;     /*DRAM controller configuration register*/
	u32 p1memcfg3;     /*DRAM controller configuration register*/
	/* DRAM id_<n>_cfg register, 16 registers */
	u32 p1_id_cfg[16];/*DRAM controller id_<n>_cfg register*/
	/*DRAM controller chip_<n>_cfg register, 2 registers*/
	u32 p1_chip_cfg[2];    /*DRAM controller chip_<n>_cfg register*/
	/*DRAM controller user state and configuration register*/
#if 0
	u32 P1_user_stat;    /*DRAM controller user_stat register*/
	u32 P1_user_cfg;     /*DRAM controller user_cfg register*/
#endif
};

struct s3c6410x_wd_timer{
	u32 wtcon;     /*Watchdog timer control register*/
	u32 wtdat;     /*Watchdog timer data register*/
	u32 wtcnt;     /*Watchdog timer count register*/
	u32 wtclrint;  /*Watchdog timer interrupt clear register*/
};

struct s3c6410x_clkpower{
	u32 locktime;
	u32 apllcon;
	u32 mpllcon;
	u32 epllcon0;
	u32 epllcon1;
	u32 clksrc;
	u32 clkdiv0;
	u32 clkdiv1;
	u32 clkdiv2;
	u32 clkout;
};

struct s3c6410x_timer_io{
	u32 tcfg0;
	u32 tcfg1;
	u32 tcon;
	int tcnt[S3C6410_TIMER_NUM];
	u32 tcmp[S3C6410_TIMER_NUM];
	int tcntb[S3C6410_TIMER_NUM];
	u32 tcmpb[S3C6410_TIMER_NUM];
	u32 tcnto[S3C6410_TIMER_NUM];
};

struct s3c6410x_uart_io{
	u32 ulcon;		/* UART line control register */
	u32 ucon;		/* UART control register */
	u32 ufcon;		/* UART FIFO control register */
	u32 umcon;		/* UART Modem control register */
	u32 utrstat;		/* UART Tx/Rx status register */
	u32 uerstat;		/* UART Rx error status register */
	u32 ufstat;		/* UART FIFO status register */
	u32 umstat;		/* UART Modem status register */
	u32 utxh;		/* UART transmit buffer register */
	u32 urxh;		/* UART receive buffer register */
	u32 ubrdiv;		/* Baud rate divisor register 0 */
	u32 ubrdivslot;		/* Baud rate divisor register 0 */
	u32 uintp;
	u32 uintsp;
	u32 uintm;
};

struct s3c6410x_sys_ctl{
	u32 apll_lock;		/* Control PLL locking period for APLL */
	u32 mpll_lock;		/* Control PLL locking period for MPLL */
	u32 epll_lock;		/* Control PLL locking period for EPLL */
	u32 apll_con;		/* Control PLL output frequency for APLL */
	u32 mpll_con;		/* Control PLL output frequency for MPLL */
	u32 epll_con0;		/* Control PLL output frequency for EPLL */
	u32 epll_con1;		/* Control PLL output frequency for EPLL */
	u32 clk_src;		/* Select clock source */
	u32 clk_div0;		/* Set clock divider ratio */
	u32 clk_div1;		/* Set clock divider ratio */
	u32 clk_div2;		/* Set clock divider ratio */
	u32 clk_out;		/* Select clock output */
	u32 hclk_gate;		/* Control HCLK clock gating */
	u32 pclk_gate;		/* Control PCLK clock gating */
	u32 sclk_gate;		/* Control SCLK clock gating */
	u32 mem0_clk_gate;	/* Control MEM0 clock gating */
	u32 ahb_con0;		/* Configure AHB I/P/X/F bus */
	u32 ahb_con1;		/* Configure AHB M1/M0/T1/T0 bus */
	u32 ahb_con2;		/* Configure AHB R/S1/S0 bus */
	u32 clk_src2;		/* Select Audio2 clock source */
	u32 sdma_sel;		/* Select secure DMA input */
	u32 sys_id;			/* System ID for revision and pass */
	u32 sys_others;		/* SYSCON others control register */
	u32 mem_sys_cfg;	/* Configure memory subsystem */
	u32 qos_override1;	/* Override DMC1 QOS */
	u32 mem_cfg_stat;	/* Memory subsystem setup status */
	u32 pwr_cfg;		/* Configure power manager */
	u32 eint_mask;		/* Configure EINT (external interrupt) mask */
	u32 normal_cfg;		/* Configure power manager at NORMAL mode */
	u32 stop_cfg;		/* Configure power manager at STOP mode */
	u32 sleep_cfg;		/* Configure power manager at SLEEP mode */
	u32 stop_mem_cfg;	/* Configure memory power at STOP mode */
	u32 osc_freq;		/* Oscillator frequency scale counter */
	u32 osc_stable;		/* Oscillator pad stable counter */
	u32 pwr_stable;		/* Power stable counter */
	u32 mtc_stable;		/* MTC stable counter */
	u32 misc_con;		/* Bus control/SYNC667 control */
	u32 others;			/* Others control register */
	u32 rst_stat;		/* Reset status register */
	u32 wakeup_stat;	/* Wakeup status register */
	u32 blk_pwr_stat;	/* Block power status register */
	u32 inform0;		/* Information register0 */
	u32 inform1;		/* Information register1 */
	u32 inform2;		/* Information register2 */
	u32 inform3;		/* Information register3 */
};

/*vicx status struct*/
struct s3c6410x_vicx_status{
	u32 vicxirqstatus;    /*  IRQ  Status  Register  */
	u32 vicxfiqstatus;    /*  FIQ  Status  Register  */
	u32 vicxrawintr;      /*  Raw  Interrupt  Status  Register  */
	u32 vicxintselect;    /*  Interrupt  Select  Register  */
	u32 vicxintenable;    /*  Interrupt  Enable  Register  */
	u32 vicxintenclear;    /*  Interrupt  Enable  Clear  Register */
	u32 vicxsoftint;    	/*  Software  Interrupt  Register  */
	u32 vicxsoftintclear;    /*  Software  Interrupt  Clear  Register */
	u32 vicxprotection;    /*  Protection  Enable  Register */
};

/*vicx Address register*/
struct s3c6410x_vicx_address{
	u32 vicxvectaddr[32];    /*  Vector  Address  0-31  Register  */
	u32 vicxaddress;    /*  Vector  Address  Register  */
};

/*vic0 priority struct*/
struct s3c6410_vicx_priority{
	u32 vicxswprioritymask;    /*  Software  Priority  Mask  Register  */
	u32 vicxprioritydaisy;    /*  Vector  Priority  Register  for  Daisy  Chain  */
	u32 vicxvecpriority[32];    /*  Vector  Priority  0-31  Register  */
};

typedef struct s3c6410x_io_t{

	/* System Control Register*/
	struct s3c6410x_sys_ctl sys_ctl;

	u32 vic0irqstatus;    /*  IRQ  Status  Register  */
	u32 vic0fiqstatus;    /*  FIQ  Status  Register  */
	u32 vic0rawintr;      /*  Raw  Interrupt  Status  Register  */
	u32 vic0intselect;    /*  Interrupt  Select  Register  */
	u32 vic0intenable;    /*  Interrupt  Enable  Register  */
	u32 vic0intenclear;    /*  Interrupt  Enable  Clear  Register */
	u32 vic0softint;    	/*  Software  Interrupt  Register  */
	u32 vic0softintclear;    /*  Software  Interrupt  Clear  Register */
	u32 vic0protection;    /*  Protection  Enable  Register */
	u32 vic0vectaddr[32];
	u32 vic0address;
	u32 vic0swprioritymask;    /*  Software  Priority  Mask  Register  */
	u32 vic0prioritydaisy;    /*  Vector  Priority  Register  for  Daisy  Chain  */
	u32 vic0vecpriority[32];    /*  Vector  Priority  0-31  Register  */

	u32 vic1irqstatus;    /*  IRQ  Status  Register  */
	u32 vic1fiqstatus;    /*  FIQ  Status  Register  */
	u32 vic1rawintr;      /*  Raw  Interrupt  Status  Register  */
	u32 vic1intselect;    /*  Interrupt  Select  Register  */
	u32 vic1intenable;    /*  Interrupt  Enable  Register  */
	u32 vic1intenclear;    /*  Interrupt  Enable  Clear  Register */
	u32 vic1softint;    	/*  Software  Interrupt  Register  */
	u32 vic1softintclear;    /*  Software  Interrupt  Clear  Register */
	u32 vic1protection;    /*  Protection  Enable  Register */
	u32 vic1vectaddr[32];
	u32 vic1address;
	u32 vic1swprioritymask;    /*  Software  Priority  Mask  Register  */
	u32 vic1prioritydaisy;    /*  Vector  Priority  Register  for  Daisy  Chain  */
	u32 vic1vecpriority[32];    /*  Vector  Priority  0-31  Register  */
#if 0
	/* Interrupt Status Registers */
	struct s3c6410x_vicx_status vicx_status[2];

	/* Interrupt Address Registers */
	struct s3c6410x_vicx_address vicx_address[2];

	/* Interrupt PriorityRegisters */
	struct s3c6410_vicx_priority vicx_priority[2];
#endif
	struct s3c6410x_clkpower clkpower;	/* clock and power management */
	/* Timer Registers */
	struct s3c6410x_timer_io timer;

	/* UART Registers */
	struct s3c6410x_uart_io uart[3];

	struct s3c6410x_wd_timer wd_timer;
	int tc_prescale;
	/* DMA Registers */
} s3c6410x_io_t;

void
s3c6410x_mach_init (void *arch_instance, machine_config_t *this_mach);
#endif /* __S3C6410X_H_ */
