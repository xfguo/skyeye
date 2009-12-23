/*
 * =====================================================================================
 *
 *       Filename:  sparc_arch_interface.c
 *
 *    Description:  Implements the SPARC architecture setup
 *
 *        Version:  1.0
 *        Created:  15/04/08 12:07:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/*-----------------------------------------------------------------------------
 *  23/06/08 15:24:39 Aitor Viana Sanchez
 *-----------------------------------------------------------------------------*/

#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_options.h"
#include "sparc_regformat.h"

#include "types.h"
#include "traps.h"
#include "sparc.h"
#include "iu.h"
#include "stat.h"

extern void leon2_mach_init(void * state, machine_config_t * mach);
static iu_config_t *iu;


/**
 *  \class sparc_machines
 *  \brief  This structure contains all the SPARC architectures mach's
 */
machine_config_t sparc_machines[] = {
	{"leon2", leon2_mach_init, NULL, NULL, NULL},
	{NULL,NULL,0,0,0}   /*  last element    */
};


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_init_state
 *  Description:  This function initializes the architecture.
 * =====================================================================================
 */
void sparc_init_state(void)
{
    static int done = 0, i;
    sparc_return_t status;

    // FIXME!: The rest of the initialization need to be performed.
//    SKYEYE_DBG("%s(): Number of banks = %d\n", __func__, mem_bank_list.num_banks);
    if( !done )
    {
        done = 1;
#if 0
        /*  Setup all the memory bank modules    */
        for( i = 0; i < mem_bank_list.num_banks; ++i)
        {
            sparc_mem_bank_info_t *mb = &(mem_bank_list.mb[i]);
            sparc_memory_module_setup_segment(mb->bank_name, NULL, mb->addr , mb->size);
        }
#endif
        /*  Register and init the SPARC integer Unit. The ISA is initialized
         *  here
         */
        status = init_sparc_iu();

        /*  RESET the statistics    */
        STAT_reset();

        /*  Init the FPU unit in case there is one  */
//        init_sparc_FPU();


        if( status != SPARC_SUCCESS )
            SKYEYE_ERR("%s(): ISA init error\n", __func__);
#if 0
        // Machine initialization
        if( skyeye_config.mach->mach_init )
            skyeye_config.mach->mach_init(NULL, skyeye_config.mach);
        else
        {
            SKYEYE_ERR("skyeye arch is not initializd correctly.\n");
            skyeye_exit(-1);
        }
#endif
	printf("In %s, the above code not finished porting.\n", __FUNCTION__);
    }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_reset_state
 *  Description:  This function RESETs the SPARC architecture.
 * =====================================================================================
 */
void sparc_reset_state(void)
{
    //  I/O initialization
#if 0
    if( skyeye_config.mach->mach_io_reset )
        skyeye_config.mach->mach_io_reset(skyeye_config.mach);
    else
    {
        SKYEYE_ERR("mach_io_reset is NULL.\n");
        skyeye_exit(-1);
    }
#endif
	printf("In %s, the above code not finished porting.\n", __FUNCTION__);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_step_once
 *  Description:  This function performs step execution. The function handles
 *  the possible pending traps, and then call the Integer Unit to execute the
 *  next instruction
 * =====================================================================================
 */
static uint32 steps = 0;
void sparc_step_once ()
{
    int n_cycles;

    /*  Execute the next instruction    */
    /*  FIXME:  n_cycles must be used to update the system clock    */
    n_cycles = iu->iu_cycle_step();

    /*  Check for insterrupts   */
    iu->iu_trap_cycle();
	steps++;
    /*  Execute the I/O cycle   */
    //skyeye_config.mach->mach_io_do_cycle ((void*)&sparc_state);
	skyeye_config_t* config = get_current_config();
	config->mach->mach_io_do_cycle((void*)&sparc_state);

}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_set_pc
 *  Description:  This function sets the Program Counter
 * =====================================================================================
 */
static void sparc_set_pc(WORD addr)
{
    iu->iu_set_pc(addr);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_get_pc
 *  Description:  This function returns the Program Counter
 * =====================================================================================
 */
static WORD sparc_get_pc()
{
    return (iu->iu_get_pc());
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_ICE_write_byte
 *  Description:  This function writes the byte 'v' at the give 'addr' address
 * =====================================================================================
 */
static int sparc_ICE_write_byte (WORD addr, uint8_t v)
{
#if 0
    int status;

    if( (status = sparc_memory_store_byte(addr, v)) == 0 )
        return -1;  //  error
    else
        return 0;   // success
#endif
	return bus_write(8, addr, v);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_ICE_read_byte
 *  Description:  This function returns the byte at the give 'addr' address in
 *  the byte pointer 'pv'
 * =====================================================================================
 */
static int sparc_ICE_read_byte(WORD addr, uint8_t *pv)
{
    int status;
#if 0
    if( (status = sparc_memory_read_byte(pv, addr)) == 0 )
        return -1;   // error
    else
        return 0;   // success
#endif
	return bus_read(8, addr, pv);
}

static int sparc_parse_cpu (const char *param[])
{
    /*  FIXME! to be implemented    */
    SKYEYE_DBG("%s()\n", __func__);

    return 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_parse_mach
 *  Description:  This functions identifies which mach is needs to be executed.
 * =====================================================================================
 */
static int sparc_parse_mach (machine_config_t * cpu, const char *params[])
{
#if 0
    int i;
    for (i = 0; i < (sizeof (sparc_machines) / sizeof (machine_config_t));
            i++) {
        if ( !strncmp (params[0], sparc_machines[i].machine_name, MAX_PARAM_NAME) ) 
        {
            skyeye_config.mach = &sparc_machines[i];
            SKYEYE_INFO("mach info: name %s, mach_init addr %p\n",
                    skyeye_config.mach->machine_name,
                    skyeye_config.mach->mach_init);

            return 0;
        }
    }
    SKYEYE_ERR ("Error: Unknown mach name \"%s\"\n", params[0]);
#endif
	printf("In %s, the above code not finished porting.\n", __FUNCTION__);
    return -1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_parse_mem
 *  Description:  This function parses the memory configuration options from the
 *  skyeye.conf file
 * =====================================================================================
 */

#if 0
static int sparc_parse_mem (int num_params, const char *params[])
{
    char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
    int i;
    static int num = -1;

    num++;

    for (i = 0; i < num_params; i++) {
        if (split_param (params[i], name, value) < 0)
            SKYEYE_ERR
                ("Error: mem_bank %d has wrong parameter \"%s\".\n",
                 num, name);

        if (!strncmp ("dev", name, strlen (name))) {
            if (!strncmp ("ram", value, strlen (value))) 
            {
                strcpy(mem_bank_list.mb[num].bank_name, value);
                sparc_memory_module_register("ram", &sparc_ram_setup);

                SKYEYE_DBG("dev = %s ", mem_bank_list.mb[num].bank_name);
            }
            else if (!strncmp ("rom", value, strlen (value))) 
            {
                strcpy(mem_bank_list.mb[num].bank_name, value);
                sparc_memory_module_register("rom", &sparc_rom_setup);

                SKYEYE_DBG("dev = %s ", mem_bank_list.mb[num].bank_name);
            }
            else if (!strncmp ("io", value, strlen (value))) 
            {
                strcpy(mem_bank_list.mb[num].bank_name, value);
                sparc_memory_module_register("io", &sparc_iomem_setup);

                SKYEYE_DBG("dev = %s ", mem_bank_list.mb[num].bank_name);
            }
            else
                SKYEYE_ERR("%s(): Unknown dev type, ram/rom are accepted\n", __func__);
        }
        else if (!strncmp ("map", name, strlen (name))) 
        {
            /*  Memory mapped   */
            if (!strncmp ("M", value, strlen (value))) 
            {
                SKYEYE_DBG("map = %s ", value);
            }
            /*  IO mapped   */
            else if (!strncmp ("I", value, strlen (value))) 
            {
                SKYEYE_DBG("map = %s ", value);
            }
            /*  Flash   */
            else if (!strncmp ("F", value, strlen (value))) 
            {
                SKYEYE_DBG("map = %s ", value);

            }
            else {
                SKYEYE_ERR
                    ("Error: mem_bank %d \"%s\" parameter has wrong value \"%s\"\n",
                     num, name, value);
            }
        }
        else if (!strncmp ("type", name, strlen (name))) 
        {
            if (!strncmp ("R", value, strlen (value))) 
            {
                SKYEYE_DBG("type = %s ", value);
                mem_bank_list.mb[num].type = R;
            }
            else if (!strncmp ("RW", value, strlen (value))) 
            {
                SKYEYE_DBG("type = %s ", value);
                mem_bank_list.mb[num].type = RW;
            }
        }
        else if (!strncmp ("addr", name, strlen (name))) 
        {

            if (value[0] == '0' && value[1] == 'x')
                mem_bank_list.mb[num].addr = strtoul (value, NULL, 16);
            else
                mem_bank_list.mb[num].addr = strtoul (value, NULL, 10);

            SKYEYE_DBG("addr = 0x%x ", mem_bank_list.mb[num].addr);
        }
        else if (!strncmp ("size", name, strlen (name))) 
        {

            if (value[0] == '0' && value[1] == 'x')
                mem_bank_list.mb[num].size = strtoul (value, NULL, 16);
            else
                mem_bank_list.mb[num].size = strtoul (value, NULL, 10);

            SKYEYE_DBG("size = 0x%x (%d Kbytes) ", mem_bank_list.mb[num].size, ( (~(mem_bank_list.mb[num].size - 1)) * (-1) ) / 1024  );
        }
        else {
            SKYEYE_ERR
                ("Error: mem_bank %d has unknow parameter \"%s\".\n",
                 num, name);
        }
    }

    SKYEYE_DBG("\n");
    /*  Store the number of memory banks available  */
    mem_bank_list.num_banks = num + 1;
    return 0;
}
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sparc_register_iu
 *  Description:  This function registers the SPARC Integer Unit module
 * =====================================================================================
 */
void sparc_register_iu(iu_config_t *iu_)
{
    SKYEYE_DBG("%s: Registering the IU\n", __func__);

    if( !iu_ )
    {
        SKYEYE_ERR("%s(): Error registering the IU\n", __func__);
        skyeye_exit(1);
    }
    iu = iu_;
    iu->iu_init_state();
}

static uint32 sparc_get_step(){
        return sparc_state.steps;
}
static char* sparc_get_regname_by_id(int id){
        return sparc_regstr[id];
}
static uint32 sparc_get_regval_by_id(int id){
	return 0;
}       

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_sparc_arch
 *  Description:  This function intializes the SPARC architecture. The function
 *  register all the callbacks needed for the correct architecture execution.
 * =====================================================================================
 */
void init_sparc_arch()
{
    static arch_config_t sparc_arch;

	sparc_arch.arch_name = "sparc";
	sparc_arch.init = sparc_init_state;
	sparc_arch.reset = sparc_reset_state;
	sparc_arch.set_pc = sparc_set_pc;
	sparc_arch.get_pc = sparc_get_pc;
	sparc_arch.step_once = sparc_step_once;
	sparc_arch.ICE_write_byte = sparc_ICE_write_byte;
	sparc_arch.ICE_read_byte = sparc_ICE_read_byte;
	sparc_arch.parse_cpu = sparc_parse_cpu;
	sparc_arch.parse_mach = sparc_parse_mach;
    //sparc_arch.parse_mem = sparc_parse_mem;
	sparc_arch.get_regval_by_id = sparc_get_regval_by_id;
        sparc_arch.get_regname_by_id = sparc_get_regname_by_id;
	sparc_arch.get_step = sparc_get_step;

	register_arch (&sparc_arch);
}

