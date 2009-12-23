#ifndef __SKYEYE_UART_OPS_H__
#define __SKYEYE_UAER_OPS_H__
#include <sys/time.h>
/* skyeye_uart_read(), skyeye_uart_write():
 * devIndex:
 *      >= 0    ---     the index of device
 *      < 0     ---     all devices, skyeye_uart_write() return the maximum bytes
of the actions
 * retDevIndex:
 *      if you don't pass NULL to it, skyeye_uart_read() replace the value of it b
e the index of device that got something.
 * wroteBytes:
 *      if you don't pass NULL to it, skyeye_uart_write() replace the value of it
be the bytes of each action of devices.
 */
int skyeye_uart_write(int devIndex, void *buf, size_t count, int *wroteBytes[MAX_UART_DEVICE_NUM]);
int skyeye_uart_read(int devIndex, void *buf, size_t count, struct timeval *timeout, int *retDevIndex);
/*
int skyeye_uart_read(int devIndex, void *buf, size_t count, struct timeval *timeout, int *retDevIndex);
int skyeye_uart_write(int devIndex, void *buf, size_t count, int *wroteBytes[MAX_UART_DEVICE_NUM]);
*/
typedef int(*uart_read_t)(int devIndex, void *buf, size_t count, struct timeval *timeout, int *retDevIndex);
typedef int(*uart_write_t)(int devIndex, void *buf, size_t count, int *wroteBytes[MAX_UART_DEVICE_NUM]);
void register_uart_operation(uart_read_t, uart_write_t);
#endif
