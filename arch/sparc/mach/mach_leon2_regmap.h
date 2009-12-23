/*
 * =====================================================================================
 *
 *       Filename:  mach_leon2_regmap.h
 *
 *    Description:  LEON2 register map
 *
 *        Version:  1.0
 *        Created:  04/27/2008 17:49:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _REGMAP_H_
#define _REGMAP_H_

#ifndef __SPARC_TYPES_H_
#error "types.h header file must be included before including mach_leon2_regmap.h"
#endif

#define LEON2_REGMAP_MCFG1              0x00
#define LEON2_REGMAP_MCFG2              0x04
#define LEON2_REGMAP_MCFG3              0x08
#define LEON2_REGMAP_AHB_FAIL_ADDR      0x0c
#define LEON2_REGMAP_AHB_STATUS         0x10
#define LEON2_REGMAP_CACHE_CTRL         0x14
#define LEON2_REGMAP_POWER_DOWN         0x18
#define LEON2_REGMAP_WRITE_PROT1        0x1c
#define LEON2_REGMAP_WRITE_PROT2        0x20
#define LEON2_REGMAP_LEON_CFG           0x24
/*  0x28    */
/*  0x2c    */
/*  0x30    */
/*  0x34    */
/*  0x38    */
/*  0x3c    */
#define LEON2_REGMAP_TIMER1_COUNTER     0x40
#define LEON2_REGMAP_TIMER1_RELOAD      0x44
#define LEON2_REGMAP_TIMER1_CTRL        0x48
#define LEON2_REGMAP_WDOG               0x4c
#define LEON2_REGMAP_TIMER2_COUNTER     0x50
#define LEON2_REGMAP_TIMER2_RELOAD      0x54
#define LEON2_REGMAP_TIMER2_CTRL        0x58
#define LEON2_REGMAP_PRESCALER_COUNTER  0x60
#define LEON2_REGMAP_PRESCALER_RELOAD   0x64
/*  0x68    */
#define LEON2_REGMAP_UART1_DATA         0x70
#define LEON2_REGMAP_UART1_STATUS       0x74
#define LEON2_REGMAP_UART1_CONTROL      0x78
#define LEON2_REGMAP_UART1_SCALER       0x7c
#define LEON2_REGMAP_UART2_DATA         0x80
#define LEON2_REGMAP_UART2_STATUS       0x84
#define LEON2_REGMAP_UART2_CONTROL      0x88
#define LEON2_REGMAP_UART2_SCALER       0x8c
#define LEON2_REGMAP_IMASK              0x90
#define LEON2_REGMAP_IPENDING           0x94
#define LEON2_REGMAP_IFORCE             0x98
#define LEON2_REGMAP_ICLEAR             0x9c
#define LEON2_REGMAP_IO_IO              0xa0
#define LEON2_REGMAP_IO_DIRECTION       0xa4
#define LEON2_REGMAP_IO_INTR_CONFIG     0xa8

/**
 * \brief Structure used to manage the registers of the On-Chip Peripherals
 *        in C/C++ Code.
 */
struct _leon2_regmap
{
	/** \brief Memory Configuration Register 1 */
	uint32 memory_configuration_1;            /* 00 */
	/** \brief Memory Configuration Register 2 */
	uint32 memory_configuration_2;            /* 04 */
	/** \brief Memory Configuration Register 3 */
	uint32 memory_configuration_3;            /* 08 */
	/** \brief AHB Failing Address Register */
	uint32 ahb_failing_address;               /* 0C */
	/** \brief AHB Status Register */
	uint32 ahb_status;                        /* 10 */
	/** \brief Cache Control Register */
	uint32 cache_control;                     /* 14 */
	/** \brief Power Down Register */
	uint32 power_down;                        /* 18 */
	/** \brief Write Protection Register 1 */
	uint32 write_protection_1;                /* 1C */
	/** \brief Write Protection Register 2 */
	uint32 write_protection_2;                /* 20 */
	/** \brief LEON Configuration Register */
	uint32 leon_configuration;                /* 24 */
	/** \brief Padding */
	uint32 foo1[6];                           /* 28 */
	/** \brief Timer 1 Counter Register */ 
	uint32 timer_1_counter;                   /* 40 */
	/** \brief Timer 1 Reload Register */
	uint32 timer_1_reload;                    /* 44 */
	/** \brief Timer 1 Control Register */
	uint32 timer_1_control;                   /* 48 */
	/** \brief Watchdog Register */
	uint32 watchdog_register;                 /* 4C */
	/** \brief Timer 2 Counter Register */ 
	uint32 timer_2_counter;                   /* 50 */
	/** \brief Timer 2 Reload Register */
	uint32 timer_2_reload;                    /* 54 */
	/** \brief Timer 2 Control Register */
	uint32 timer_2_control;                   /* 58 */
	/** \brief Padding */
	uint32 foo2;                              /* 5C */
	/** \brief Prescaler Counter Register */
	uint32 prescaler_counter;                 /* 60 */
	/** \brief Prescaler Reload Register */
	uint32 prescaler_reload;                  /* 64 */
	/** \brief Padding */
	uint32 foo3[2];                           /* 68 */
	/** \brief UART 1 Data Register */
	uint32 uart_1_data;                       /* 70 */
	/** \brief UART 1 Status Register */
	uint32 uart_1_status;                     /* 74 */
	/** \brief UART 1 Control Register */
	uint32 uart_1_control;                    /* 78 */
	/** \brief UART 1 Scaler Register */
	uint32 uart_1_scaler;                     /* 7C */
	/** \brief UART 2 Data Register */
	uint32 uart_2_data;                       /* 80 */
	/** \brief UART 2 Status Register */
	uint32 uart_2_status;                     /* 84 */
	/** \brief UART 2 Control Register */
	uint32 uart_2_control;                    /* 88 */
	/** \brief UART 2 Scaler Register */
	uint32 uart_2_scaler;                     /* 8C */
	/** \brief Interrupt Mask and Priority Register */
	uint32 irq_mask_and_priority;             /* 90 */
	/** \brief Interrupt Pending Register */
	uint32 irq_pending;                       /* 94 */
	/** \brief Interrupt Force Register */
	uint32 irq_force;                         /* 98 */
	/** \brief Interrupt Clear Register */
	uint32 irq_clear;                         /* 9C */
	/** \brief I/O Port input/output register */
	uint32 ioport_io;                         /* A0 */
	/** \brief I/O Port Direction Register */
	uint32 ioport_direction;                  /* A4 */
	/** \brief I/O Port Interrupt Config Register */
	uint32 ioport_irq_config;                 /* A8 */
	/** \brief Padding */
	uint32 foo4;                              /* AC */
	/** \brief Secondary Interrupt Mask Register */
	uint32 secondary_irq_mask;                /* B0 */
	/** \brief Secondary Interrupt Pending Register */
	uint32 secondary_irq_pending;             /* B4 */
	/** \brief Secondary Interrupt Status Register */
	uint32 secondary_irq_status;              /* B8 */
	/** \brief Secondary Interrupt Clear Register */
	uint32 secondary_irq_clear;               /* BC */
	/** \brief Padding */
	uint32 foo5;                              /* C0 */
	/** \brief DSU UART Status Register */
	uint32 dsu_uart_status;                   /* C4 */
	/** \brief DSU UART Control Register */
	uint32 dsu_uart_control;                  /* C8 */
	/** \brief DSU UART Scaler Register */
	uint32 dsu_uart_scaler;                   /* CC */
};

/**
 * Interrupt controller interrupt assigment
 */
#define USER_DEFINED_INT1   15
#define PCI_INT             14
#define USER_DEFINED_INT2   13 
#define ETHERNET_MAC_INT    12
#define DUS_TRACE_BUFF_INT  11
#define SECOND_INT_CTRL_INT 10
#define TIMER1_INT          9
#define TIMER2_INT          8
#define PIO3_INT            7
#define PIO2_INT            6
#define PIO1_INT            5
#define PIO0_INT            4
#define UART1_INT           3
#define UART2_INT           2
#define AHB_ERR_INT         1

/*-----------------------------------------------------------------------------
 *  INTERFACES
 *-----------------------------------------------------------------------------*/

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  REGMAP
 *  Description:  This structure contains all the on-chip registers. Those
 *  registers support the functions that are provided on-chip.
 *  The instantiation shall be defined in any other file.
 * =====================================================================================
 */
extern struct _leon2_regmap regmap;

/*  These functions must be implemented for the I/O  */
void leon2_io_do_cycle(void * state);
uint32 leon2_io_read_byte (void * state, uint32 addr);
uint32 leon2_io_read_word (void * state, uint32 addr);
uint32 leon2_io_read_long (void * state, uint32 addr);

void leon2_set_int (int irq);
void leon2_io_write_byte (void * state, uint32 addr, uint32 v);
void leon2_io_write_word (void * state, uint32 addr, uint32 v);
void leon2_io_write_long (void * state, uint32 addr, uint32 v);
void leon2_io_reset(void *state);


uint32 leon2_regmap_read(unsigned int *result, int size, int offset);
uint32 leon2_regmap_write(int size, int offset, unsigned int value);

#endif

