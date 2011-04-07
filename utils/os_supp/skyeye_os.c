#include "skyeye_os.h"
#include "skyeye_config.h"

/* the number of supported os */
#define MAX_SUPP_OS 32
static os_config_t *skyeye_os[MAX_SUPP_OS];

/* defaut os chose if OS not set or set fault value */
static char *default_os_name = "linux";

/* Support os list */
static os_config_t os_list[] = {
	{"linux"},
	{"vxworks"},
	{NULL}
};

/* init array skyeye_os[] */
void init_os_option()
{
	int i;

	for (i = 0; i < MAX_SUPP_OS; i++) {
		if (os_list[i].os_name == NULL)
			return ;
		skyeye_os[i] = &os_list[i];
	}
}

/* Match os between skyeye.conf and skyeye_os */
int do_os_option(skyeye_option_t *this_option, int num_params,
		const char *params[])
{

	int i;
	skyeye_config_t *config = get_current_config();

	for (i = 0; i < MAX_SUPP_OS; i++) {
		if (skyeye_os[i] == NULL)
			continue;
		if (!strncasecmp
				(params[0], skyeye_os[i]->os_name, MAX_PARAM_NAME)) {
			config->os = skyeye_os[i];
			printf("OS name: %s\n",
					skyeye_os[i]->os_name);
			return 0;
		}
	}
	SKYEYE_ERR("Error: Unknown OS name \"%s\", we just support \"linux\" & \"vxworks\".\n",
			params[0]);
	SKYEYE_ERR("It would choose \"linux\" defaultly.\n");

	/* Select default os name */
	for (i = 0; i < MAX_SUPP_OS; i++) {
		if (!strncasecmp
				(default_os_name, skyeye_os[i]->os_name, MAX_PARAM_NAME)) {
			config->os = skyeye_os[i];
			printf("default OS name: %s\n\n",
					skyeye_os[i]->os_name);
			return 0;
		}
	}
}
