/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file skyeye_uart_ops.c
* @brief register the uart operation here
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include "skyeye_config.h"
#include "skyeye_uart_ops.h" 

/**
* @brief uart read operation
*/
static uart_read_t uart_read_ops;

/**
* @brief uart write operation
*/
static uart_write_t uart_write_ops;

/**
* @brief register uart operations to the common library
*
* @param uart_read
* @param uart_write
*/
void register_uart_operation(uart_read_t uart_read, uart_write_t uart_write){
	uart_write_ops = uart_write;
	uart_read_ops = uart_read;
	//skyeye_log(Debug_log, __FUNCTION__, "register read&write for uart\n");
}

/**
* @brief the uart write operation provided by common library
*
* @param devIndex
* @param buf
* @param count
* @param wroteBytes[MAX_UART_DEVICE_NUM]
*
* @return 
*/
int skyeye_uart_write(int devIndex, void *buf, size_t count, int *wroteBytes[MAX_UART_DEVICE_NUM]){
	//skyeye_module_t* uart_module  = get_module_by_name("uart");
	//if()
	//uart_write_ops = 
	return uart_write_ops(devIndex, buf, count, wroteBytes);
}

/**
* @brief the uart read operation provided by common library
*
* @param devIndex
* @param buf
* @param count
* @param timeout
* @param retDevIndex
*
* @return 
*/
int skyeye_uart_read(int devIndex, void *buf, size_t count, struct timeval *timeout, int *retDevIndex){
	return uart_read_ops(devIndex, buf, count, timeout, retDevIndex);
}

