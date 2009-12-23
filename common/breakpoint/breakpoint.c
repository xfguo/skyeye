#include <stdio.h>
#include <errno.h>

#include "skyeye_arch.h"
#include "breakpoint.h"
#include "skyeye_types.h"
#include "skyeye_callback.h"

//const int max_bp_number = 255;
#define MAX_BP_NUMBER 255
typedef struct breakpoint_s{
	breakpoint_id_t id;
	access_t access_type;
	breakpoint_kind_t address_type;
	generic_address_t address;
	uint32 hits;
}breakpoint_t;

typedef struct breakpoint_mgt{
	/* currently max breakpoint number supported by us is MAX_BP_NUMBER */
	breakpoint_t breakpoint[MAX_BP_NUMBER];
	/* The number point to the last element */
	int bp_number;
}breakpoint_mgt_t;

static breakpoint_mgt_t breakpoint_mgt;

static void check_breakpoint(generic_arch_t* arch_instance);

static breakpoint_t* get_bp_by_id(breakpoint_id_t id){
	/* scan the breakpoint if the breakpoint exists */
	int i;
        for(i = 0; i < breakpoint_mgt.bp_number; i++) {
                if (breakpoint_mgt.breakpoint[i].id == id)
                        return &breakpoint_mgt.breakpoint[i];
        }
	return NULL;
}

static breakpoint_t* get_bp_by_addr(generic_address_t addr){
	int i;
	/* scan the breakpoint if the breakpoint exists */
        for(i = 0; i < breakpoint_mgt.bp_number; i++) {
                if (breakpoint_mgt.breakpoint[i].address == addr)
                        return &breakpoint_mgt.breakpoint[i];
        }
	return NULL;
}

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

exception_t skyeye_remove_bp(breakpoint_id_t id)
{
	int i;
	breakpoint_t* bp;
	bp = get_bp_by_id(id);
	if(bp){
		bp->id = 0;
		bp->hits = 0;
	}
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
	return No_exp;
}

#if 0
class breakpoint:public breakpoint_interface{
}
#endif

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

/*
 * Return: 1 means hit of breakpoint, 0 means not.
 */
static void check_breakpoint(generic_arch_t* arch_instance){
#if 1
	int i;
	for (i = 0;i < breakpoint_mgt.bp_number;i++){
		breakpoint_t* bp = &breakpoint_mgt.breakpoint[i];
		/* if id is zero, we think the bp is disabled now. */
		if(bp->id == 0)
			continue;
		/* return if any breakpoint at the address is hit */
		if(bp->address == arch_instance->get_pc()){
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
