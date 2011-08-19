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
* @file pci.c
* @brief the implementation of pci bus
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-08-12
*/

#include <stdlib.h>
#include <assert.h>
#include <skyeye_mm.h>

#include <skyeye_interface.h>
#include <skyeye_types.h>
#include <bank_defs.h>
#include <skyeye_sched.h>
#include <skyeye_options.h>
#include <skyeye_config.h>
#include <skyeye_class.h>
#include <memory_space.h>
#include <skyeye_interface.h>
#define DEBUG
#include <skyeye_log.h>

typedef struct conf_access_reg{
	uint32 pex_config_addr;
	uint32 pex_config_data;
	uint32 pex_otb_cpl_tor;
	uint32 pex_conf_rty_tor;
	uint32 pex_config;
} conf_access_reg_t;

typedef struct pcie{
	conf_object_t* obj;
	conf_access_reg_t* reg;
	mem_bank_t* bank;
	uint32 irq;
	char name[1024];
}pcie_t;

const static char* class_name = "pcie";

static char pcie_read(short size, int addr, unsigned int *result){
	return 0;
}

static char pcie_write(short size, int addr, unsigned int data){
	return 0;
}

static conf_object_t* new_pcie_bus(char* objname){
	pcie_t* pcie = (pcie_t*)malloc(sizeof(pcie_t));
	pcie->obj = new_conf_object(objname, pcie);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
        io_memory->read = pcie_read;
        io_memory->write = pcie_write;
	SKY_register_interface(pcie->obj, objname, MEMORY_SPACE_INTF_NAME);

	return pcie->obj;
}

/**
* @brief Free all the memory of pcie bus object
*
* @param objname
*
* @return 
*/
static exception_t free_pcie_bus(char* objname){
	return No_exp;
}


void init_pcie(){
	static skyeye_class_t class_data = {
		.class_name = "pcie",
		.class_desc = "pcie bus",
		.new_instance = new_pcie_bus,
		.free_instance = free_pcie_bus,
		.get_attr = NULL,
		.set_attr = NULL
	};
	
	SKY_register_class(class_data.class_name, &class_data);
}
