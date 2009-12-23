#include "skyeye_config.h"
#include "skyeye_uart_ops.h" 

static uart_read_t uart_read_ops;
static uart_write_t uart_write_ops;
void register_uart_operation(uart_read_t uart_read, uart_write_t uart_write){
	uart_write_ops = uart_write;
	uart_read_ops = uart_read;
	//skyeye_log(Debug_log, __FUNCTION__, "register read&write for uart\n");
}

int skyeye_uart_write(int devIndex, void *buf, size_t count, int *wroteBytes[MAX_UART_DEVICE_NUM]){
	//skyeye_module_t* uart_module  = get_module_by_name("uart");
	//if()
	//uart_write_ops = 
	return uart_write_ops(devIndex, buf, count, wroteBytes);
}
	
int skyeye_uart_read(int devIndex, void *buf, size_t count, struct timeval *timeout, int *retDevIndex){
	return uart_read_ops(devIndex, buf, count, timeout, retDevIndex);
}

