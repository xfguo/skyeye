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
* @file flash.c
* @brief the read/write implementation for Nor flash
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "skyeye_types.h"
#include "skyeye_config.h"

//#include "armdefs.h"

/**
* @brief the byte read function of flash
*
* @param addr the data address
*
* @return  data
*/
static uint32
flash_read_byte (uint32 addr)
{
	struct device_desc *dev;
	uint32 data;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->read_byte)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {

			if (dev->read_byte (dev, addr, (uint8 *) & data) !=
			    ADDR_NOHIT)
				return data & 0xff;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {

			if (dev->read_byte (dev, addr, (uint8 *) & data) !=
			    ADDR_NOHIT)
				return data & 0xff;
		}
	}
	printf("In %s,Can not find address 0x%x\n", __FUNCTION__, addr);
	return data;
}

/**
* @brief the halfword reading of flash
*
* @param addr the data address
*
* @return data
*/
static uint32
flash_read_halfword (uint32 addr)
{
	struct device_desc *dev;
	uint32 data;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->read_halfword)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {
			if (dev->read_halfword (dev, addr, (uint16 *) & data) !=
			    ADDR_NOHIT)
				return data & 0xffff;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {
			if (dev->read_halfword (dev, addr, (uint16 *) & data) !=
			    ADDR_NOHIT)
				return data & 0xffff;
		}
	}
	printf("In %s,Can not find address 0x%x\n", __FUNCTION__, addr);
	return data;
}

/**
* @brief The word read function of flash
*
* @param addr data address
*
* @return data 
*/
static uint32
flash_read_word (uint32 addr)
{
	struct device_desc *dev;
	uint32 data;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->read_word)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {
			if (dev->read_word (dev, addr, (uint32 *) & data) !=
			    ADDR_NOHIT)
				return data;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {
			if (dev->read_word (dev, addr, (uint32 *) & data) !=
			    ADDR_NOHIT)
				return data;
		}
	}
	printf("In %s,Can not find address 0x%x\n", __FUNCTION__, addr);
	return data;
}

/**
* @brief the byte write function of the flash
*
* @param addr the data address
* @param data the written data
*/
static void
flash_write_byte (uint32 addr, uint32 data)
{
	struct device_desc *dev;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->write_byte)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {
			if (dev->write_byte (dev, addr, (uint8) data) !=
			    ADDR_NOHIT)
				return;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {
			if (dev->write_byte (dev, addr, (uint8) data) !=
			    ADDR_NOHIT)
				return;
		}
	}
	fprintf(stderr, "In %s, addr 0x%x is invalid.\n", __FUNCTION__, addr);
}

/**
* @brief the halfword write function of the flash
*
* @param addr the data address
* @param data the written data
*/
void
flash_write_halfword (uint32 addr, uint32 data)
{
	struct device_desc *dev;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->write_halfword)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {
			if (dev->write_halfword (dev, addr, (uint16) data) !=
			    ADDR_NOHIT)
				return;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {
			if (dev->write_halfword (dev, addr, (uint16) data) !=
			    ADDR_NOHIT)
				return;
		}
	}
	fprintf(stderr, "In %s, addr 0x%x is invalid.\n", __FUNCTION__, addr);
}

/**
* @brief The word write function of the flash
*
* @param addr the address of written data
* @param data the data
*/
static void
flash_write_word (uint32 addr, uint32 data)
{
	struct device_desc *dev;
	skyeye_config_t* config = get_current_config();
	int i;
	for (i = 0; i < config->mach->dev_count; i++) {
		dev = config->mach->devices[i];
		if (!dev->write_word)
			continue;
		/* if we specify size=0, we don't check 
		 * whether "addr" is in the range of address space of device.
		 * */
		if (dev->size == 0) {
			if (dev->write_word (dev, addr, data) != ADDR_NOHIT)
				return;
		}
		else if ((addr >= dev->base)
			 && (addr < (dev->base + dev->size))) {
			if (dev->write_word (dev, addr, data) != ADDR_NOHIT)
				return;
		}
	}
	fprintf(stderr, "In %s, addr 0x%x is invalid.\n", __FUNCTION__, addr);
}

/**
* @brief The common flash read function
*
* @param size the data width
* @param offset the data offset
* @param value the data read from
*
* @return the flash to indicate the success or failure
*/
char flash_read(short size, int offset, uint32 * value){
	switch(size){
		case 8:
			*(uint8 *)value = (uint8)flash_read_byte (offset);
			break;
		case 16:
			*(uint16 *)value = (uint16)flash_read_halfword(offset);
			break;
		case 32:
			*value = flash_read_word(offset);
			break;
		default:
			return -1;
	}
	return 0;

}

/**
* @brief The flash write function
*
* @param sizea the data width
* @param offset the offset of write operation
* @param value the data write to
*
* @return 
*/
char flash_write(short size, int offset, uint32 value){
	switch(size){
		case 8:
                        flash_write_byte (offset, value);
                        break;
                case 16:
                        flash_write_halfword(offset, value);
                        break;
                case 32:
                        flash_write_word(offset, value);
                        break;
                default:
                        return -1;

	}
	return 0;
}
