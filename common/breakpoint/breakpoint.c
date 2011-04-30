/* Copyright (C) 
* 2011 - Michael.kang
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
* @file breakpoint.c
* @brief The breakpoint management and implementation.
* @author Michael.kang
* @version 
* @date 2011-04-28
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "skyeye_arch.h"
#include "breakpoint.h"
#include "skyeye_types.h"
#include "skyeye_callback.h"
#include "skyeye_command.h"
#include "sim_control.h"

//const int max_bp_number = 255;

/**
* @brief The max number of breakpoint
*/
#define MAX_BP_NUMBER 255

/**
* @brief The breakpoint struct
*/
typedef struct breakpoint_s{
	breakpoint_id_t id;
	access_t access_type;
	breakpoint_kind_t address_type;
	generic_address_t address;
	uint32 hits;
}breakpoint_t;

/**
* @brief The struct used to management breakpoint
*/
typedef struct breakpoint_mgt{
	/* currently max breakpoint number supported by us is MAX_BP_NUMBER */
	breakpoint_t breakpoint[MAX_BP_NUMBER];
	/* The number point to the last element */
	int bp_number;
}breakpoint_mgt_t;

/**
* @brief the instance of breakpoint management
*/
static breakpoint_mgt_t breakpoint_mgt;

/**
* @brief Check if the breakpoint is trigger
*
* @param arch_instance
*/
static void check_breakpoint(generic_arch_t* arch_instance);

/**
* @brief Get a breakpoint by its id
*
* @param id
*
* @return a breakpoint instance
*/
static breakpoint_t* get_bp_by_id(breakpoint_id_t id){
	/* scan the breakpoint if the breakpoint exists */
	int i;
        for(i = 0; i < breakpoint_mgt.bp_number; i++) {
                if (breakpoint_mgt.breakpoint[i].id == id)
                        return &breakpoint_mgt.breakpoint[i];
        }
	return NULL;
}

/**
* @brief if the breakpoint is valid
*
* @param bp
*
* @return 
*/
static bool_t valid_bp(breakpoint_t* bp){
	if(bp->id != 0)
		return True;
	else 
		return False;
}

/**
* @brief get a breakpoint by its address
*
* @param addr
*
* @return 
*/
breakpoint_t* get_bp_by_addr(generic_address_t addr){
	int i;
	/* scan the breakpoint if the breakpoint exists */
        for(i = 0; i < breakpoint_mgt.bp_number; i++) {
                if (breakpoint_mgt.breakpoint[i].address == addr && 
		valid_bp(&breakpoint_mgt.breakpoint[i]))
                        return &breakpoint_mgt.breakpoint[i];
        }
	return NULL;
}

/**
* @brief insert a breakpoint at an address
*
* @param access_type
* @param address_type
* @param addr
*
* @return 
*/
exception_t skyeye_insert_bp(access_t access_type, breakpoint_kind_t address_type, generic_address_t addr)
{
	breakpoint_t* bp;
#if 0
	int i;
	bp = get_bp_by_addr(addr);
	/* scan the breakpoint if the breakpoint exists */
	for(i = 0; i < breakpoint_mgt.bp_number; i++) {
        	if (breakpoint_mgt.breakpoint[i].address == addr)
	        	return 1;
	}
#endif
	if (breakpoint_mgt.bp_number == MAX_BP_NUMBER)
		return Excess_range_exp;
	
	/* we insert the new bp to the last position. And increase the bp_number */
	bp = &breakpoint_mgt.breakpoint[breakpoint_mgt.bp_number];
	bp->address = addr;
	bp->access_type =  access_type;
	bp->address_type = address_type;

	breakpoint_mgt.bp_number++;
	bp->id = breakpoint_mgt.bp_number;
	return No_exp;
}


/**
* @brief remove a breakpoint by id
*
* @param id
*
* @return 
*/
exception_t skyeye_remove_bp(breakpoint_id_t id)
{
	int i;
	breakpoint_t* bp;
	bp = get_bp_by_id(id);
	if(bp){
		bp->id = 0;
		bp->hits = 0;
		return No_exp;
	}
	return Not_found_exp; 
#if 0
  for(i = 0; i < skyeye_ice.num_bps; i++) {
        if (skyeye_ice.bps[i] == addr)
	       goto found;
  }
  return 0;
found:
  skyeye_ice.num_bps--;
  if (i < skyeye_ice.num_bps)
          skyeye_ice.bps[i] = skyeye_ice.bps[skyeye_ice.num_bps];
#endif
}

/**
* @brief remove a breakpoint at the address
*
* @param addr
*
* @return 
*/
exception_t skyeye_remove_bp_by_addr(generic_address_t addr){
	breakpoint_t* bp = get_bp_by_addr(addr);
	if(bp != NULL)
		return skyeye_remove_bp(bp->id);
	else
		return Not_found_exp; 
}

#if 0
class breakpoint:public breakpoint_interface{
}
#endif

/**
* @brief handler of break command
*
* @param arg
*
* @return 
*/
int com_break(char*arg){
	char** endptr;
	generic_address_t addr;
	if(arg == NULL && *arg == '\0'){
		generic_arch_t* arch_instance = get_arch_instance(NULL);
		addr = arch_instance->get_pc();
	}
	else{
		errno = 0;
		addr = strtoul(arg, endptr, 16);
		/*
		if(**endptr != '\0'){
			printf("Not valid address format.\n");
			return 1;
		}
		*/
		#if 0
		if(errno != 0){
			if(errno == EINVAL)
				printf("Not valid address format.\n");
			else if(errno == ERANGE)
				printf("Out of range for your breakpoint address.\n");
			else
				printf("Some unknown error in your break command.\n");
			return 1;
		}
		#endif
	}
	exception_t exp = skyeye_insert_bp(SIM_access_execute, SIM_Break_Virtual, addr);
	if(exp != No_exp){
		printf("Can not insert breakpoint at address 0x%x",addr);
		return 1;
	}
	printf("Insert breakpoint at address 0x%x successfully.\n", addr);
	return 0;
}

/**
* @brief The handler of list all the breakpoint
*
* @return 
*/
int com_list_bp(){
	int i = 0;
	char* format = "%d\t0x%x\t%d\n";
	printf("%s\t%s\t%s\n", "ID", "Address","Hits");
	while(i < MAX_BP_NUMBER){
		breakpoint_t* bp = &breakpoint_mgt.breakpoint[i];
		if(bp->id != 0)	
			printf(format, bp->id, bp->address, bp->hits);	
		i++;
	}
}

/**
* @brief Delete a breakpoint by id
*
* @param arg
*
* @return 
*/
int com_delete_bp(char*arg){
	char** endptr;
	int id;
	if(arg == NULL && *arg == '\0'){
		return;
	}
	else{
		errno = 0;
		id = strtoul(arg, endptr, 10);
		/*
		if(**endptr != '\0'){
			printf("Not valid address format.\n");
			return 1;
		}
		*/
		#if 0
		if(errno != 0){
			if(errno == EINVAL)
				printf("Not valid address format.\n");
			else if(errno == ERANGE)
				printf("Out of range for your breakpoint address.\n");
			else
				printf("Some unknown error in your break command.\n");
			return 1;
		}
		#endif
	}
	exception_t exp = skyeye_remove_bp(id);
	if(exp != No_exp){
		printf("Can not delete breakpoint at id %d",id);
		return 1;
	}
	printf("Delete breakpoint at id %d successfully.\n", id);
	return 0;
}

/**
* @brief Initilization of breakpoint
*
* @return 
*/
int init_bp(){
	exception_t exp;
	register_callback(check_breakpoint, Step_callback);
	add_command("break", com_break, "set breakpoint for an address.\n");
	add_command("list-bp", com_list_bp, "List all the breakpoint.\n");
	add_command("delete-bp", com_delete_bp, "List all the breakpoint.\n");
	//register_info_command("breakpoint", com_list_bp, "List all the breakpoint.\n");
}

breakpoint_t* get_first_bp(){
}

breakpoint_t* get_next_bp(breakpoint_t* bp){
}

/**
* @brief check if the breakpoint is hit in every step
*
* @param arch_instance
*/
static void check_breakpoint(generic_arch_t* arch_instance){
#if 1
	int i;
	generic_address_t current_pc = arch_instance->get_pc();
	if(!strncmp("arm", arch_instance->arch_name, strlen("arm"))){
		current_pc -= 8;
	}
	
	for (i = 0;i < breakpoint_mgt.bp_number;i++){
		breakpoint_t* bp = &breakpoint_mgt.breakpoint[i];
		/* if id is zero, we think the bp is disabled now. */
		if(bp->id == 0)
			continue;
		/* return if any breakpoint at the address is hit */
		if(bp->address == current_pc){
			//arch_instance->stop();
			bp->hits++;
			printf("The %d# breakpoint at address 0x%x is hit.\n", bp->id, bp->address);
			SIM_stop(arch_instance);
			return;
		}
	} /* for */
#endif
	return;
}
