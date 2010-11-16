#include <stdlib.h>
#include <assert.h>

#include <skyeye_types.h>
#include <bank_defs.h>
#include <skyeye_sched.h>
#include <skyeye_options.h>
#include <skyeye_config.h>

typedef struct reg_16550{
	uint32 rbr;
	uint32 thr;
	uint32 ier;
	uint32 iir;
	uint32 fcr;
	uint32 lcr;
	uint32 lsr;
	uint32 msr;
	uint32 scr;
	uint32 dll;
	uint32 dlm;
	uint8 t_fifo[16];
	uint8 r_fifo[16];
} reg_16550_t;

typedef struct uart_16550{
	reg_16550_t* reg;
	mem_bank_t* bank;
	uint32 irq;
	char name[1024];
}uart_16550_t;

const static char* class_name = "uart_16550";

char uart_16550_read(short size, int addr, unsigned int *result){
	uint32 data;
	mem_bank_t* bank = bank_ptr(addr);
	assert(bank != NULL);
	/* get a reference of object by its name */
	uart_16550_t* uart_16550 = (uart_16550_t*)get_conf_obj(bank->objname);

	reg_16550_t* reg = uart_16550->reg;

	switch ((addr & 0xfff) >> 2) {
	case 0x0:		// RbR
		reg->lsr &= ~0x1;
		/*
		if (i == 0)
			io.vic.risr &= art_16550->irq;
		*/
		data = reg->rbr;
		break;

	case 0x1:		// ier
		data = reg->ier;
		break;
	case 0x2:		// iir
		data = reg->iir;
		break;
	case 0x3:		// IDR
	case 0x4:		// IMR
	case 0x5:		// LSR
		data = reg->lsr;
		break;
	case 0x6:		// MSR
		data = reg->msr;
		break;
	case 0x7:		// SCR
		data = reg->scr;
		break;

	default:
		//DBG_PRINT ("uart_read(%s=0x%08x)\n", "uart_reg", addr);

		break;
	}
	*result = data;
	return 0;
}

static char uart_16550_write(short size, int addr, unsigned int data){
	mem_bank_t* bank = bank_ptr(addr);
	assert(bank != NULL);
	/* get a reference of object by its name */
	uart_16550_t* uart_16550 = get_conf_obj(bank->objname);
	assert(uart_16550 != NULL);

	reg_16550_t* reg = uart_16550->reg;
	switch ((addr & 0xfff) >> 2) {
	case 0x0:		// THR
		{
			char c = data;
			skyeye_uart_write(-1, &c, 1, NULL);
			reg->lsr |= 0x20;
		}
	case 0x2:		//FCR
		reg->fcr = data;
		break;
	case 0x7:		// SCR
		reg->scr = data;
		break;
	default:
		//DBG_PRINT ("uart_write(%s=0x%08x)\n", "uart_reg", addr);
		break;
	}

}

static void uart_16550_io_do_cycle(void* uart_16550){
	uart_16550_t* uart = (uart_16550_t *)uart_16550;
	reg_16550_t* reg = uart->reg;

	if (reg->ier & 0x2) {	/* THREI enabled */
		//printf("In %s, THR interrupt\n", __FUNCTION__);
		reg->iir = (reg->iir & 0xf0) | 0x2;
		reg->lsr |= 0x60;
	}


	if (reg->ier & 0x1) {	/* RDAI enabled */
		struct timeval tv;
		unsigned char buf;

		tv.tv_sec = 0;
		tv.tv_usec = 0;
	
		if(skyeye_uart_read(-1, &buf, 1, &tv, NULL) > 0)
		{
			//printf("SKYEYE:get input is %c\n",buf);
			reg->rbr = buf;
			reg->lsr |= 0x1;
			reg->iir = (reg->iir & 0xf0) | 0x4;
			skyeye_config_t* config = get_current_config();
			config->mach->mach_intr_signal(uart->irq, High_level);
		}
	}
}

static void create_16550_uart(uint32 base_addr, uint32 len, uint32 irq){
	uart_16550_t* uart = (uart_16550_t*)malloc(sizeof(uart_16550_t));
	/* register own cycle handler to scheduler. 
	 * set the cycle to 1 ms, and periodic schedle
	 * */
	uart->irq = irq;
	uint32 id;
	create_timer_scheduler(1, Periodic_sched, uart_16550_io_do_cycle, uart, &id);
	sprintf(&uart->name[0],"uart_16550_%d",id);
	reg_16550_t* reg = (reg_16550_t*)malloc(sizeof(reg_16550_t));
	uart->reg = reg;

	mem_bank_t *bank = (mem_bank_t *)malloc(sizeof(mem_bank_t));
	bank->addr = base_addr;
	bank->len = len;
	bank->bank_write = uart_16550_write;
	bank->bank_read = uart_16550_read;
	bank->type = MEMTYPE_IO;
	bank->objname = &uart->name[0];
	bank->filename[0] = '\0';
	/* register io space to the whole address space */
	addr_mapping(bank);
	/* 
	 * FIXME, we have the same bank data structure both in 
	 * global_memmap and here. Should free one.
	 * */
	uart->bank = bank;
	put_conf_obj(bank->objname, uart);
}

static int do_16550_option(skyeye_option_t* this_option, int num_params,
		const char *params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	uint32 addr, len, irq;
	int i;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: uart has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("base", name, strlen (name))) {
			sscanf (value, "%x", &addr);
		}
		else if (!strncmp ("length", name, strlen (name))) {
			sscanf (value, "%x", &len);
		}
		else if (!strncmp ("irq", name, strlen (name))) {
			sscanf (value, "%x", &irq);
		}
		else
                        SKYEYE_ERR ("Error: Unknown uart_16550 option  \"%s\"\n", params[i]);
	}
	create_16550_uart(addr, len, irq);
}

/*
 * Create a 16550 uart by given base, len ,irq
 */
static int com_create_16550(char* arg){
	return 0;
}
void init_uart_16550(){
	/* register options parser */
	register_option("uart_16550", do_16550_option, "Uart settings"); 
	add_command("create_uart_16550", com_create_16550, "Create a new uart of 16550 type.\n");
}
