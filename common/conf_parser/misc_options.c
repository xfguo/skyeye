#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_pref.h"

#include "skyeye_types.h"
#ifdef DBCT_TEST_SPEED
int
do_dbct_test_speed_sec(struct skyeye_option_t *this_opion, int num_params, const char *params[])
{
	if (num_params != 1) {
		goto error_out;
	}
	errno = 0;
	skyeye_config.dbct_test_speed_sec = strtol(params[0], NULL, 10);
	if (errno == ERANGE) {
		goto error_out;
	}
	printf("dbct_test_speed_sec %ld\n", skyeye_config.dbct_test_speed_sec);

error_out:
	SKYEYE_ERR("Error :usage: step_disassemble: dbct_test_speed_sec\n");
	return(-1);
}
#endif	//DBCT_TEST_SPEED
//AJ2D--------------------------------------------------------------------------

/* set values for some register */
int
do_regfile_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	skyeye_config_t* config = get_current_config();
	if(config->arch && config->arch->parse_regfile)
		config->arch->parse_regfile(num_params, params);
	else
		SKYEYE_WARNING("regfile option is not implemented by current arch\n");
	return 0;
}

/* set load address for elf image */
int
do_load_addr_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	int i;
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	unsigned long load_base;
	unsigned long load_mask;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: sound has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("base", name, strlen (name))) {
			sscanf (value, "%x", &load_base);
		}
		else if (!strncmp ("mask", name, strlen (name))) {
			sscanf (value, "%x", &load_mask);
		}
		else
                        SKYEYE_ERR ("Error: Unkonw load_addr option  \"%s\"\n", params[i]);
	}
	sky_pref_t *pref;
	/* get the current preference for simulator */
	pref = get_skyeye_pref();
	pref->exec_load_base = load_base;
	pref->exec_load_mask = load_mask;
	/* FIXME, we should update load_base and load_mask to preference of SkyEye */
	fprintf(stderr, "%s not finished.\n", __FUNCTION__);
	//printf("Your elf file will be load to: base address=0x%x,mask=0x%x\n", load_base, load_mask);
	return 0;
}

int
do_deprecated_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	skyeye_log(Warnning_log, __FUNCTION__, "Deprecated option. Do not use any more.\n");
}
