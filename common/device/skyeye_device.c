/*
	skyeye_device.c - implement the device framework for skyeye
	Copyright (C) 2004 Skyeye Develop Group
	for help please send mail to <skyeye-developer@lists.gro.clinux.org>
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 
*/
/*
 * 05/04/2004  	initial version
 * 		desgin a device framework for skyeye to providing good exetension of device simulations
 *
 *		walimis <wlm@student.dlut.edu.cn> 
 * */
#include "stdlib.h"
#include "skyeye_device.h"
#include "skyeye_config.h"
#include "skyeye_options.h"
#include "skyeye_mm.h"

/**
* @brief the module set of all the device
*/
static struct device_module_set **global_mod_set;
static int mod_set_count = 0, mod_set_count_max = 0;


/**
* @brief initialize all the devices.
* @Deprecated
*/
void
initialize_all_devices ()
{
	/* when module is loaded, they will register itself. */
	//uart_register();
	//timer_register();
#ifndef NO_NET
	//net_register ();
#endif
#ifdef LCD
	//lcd_register ();
#endif
	//flash_register ();
	//touchscreen_register ();
#if 0
	sound_register ();
#endif
	//nandflash_register();
}

void
set_device_default (struct device_desc *dev, struct device_default_value *def)
{
	struct device_default_value *def_p = def;
	struct machine_config *mc = (struct machine_config *) dev->mach;

	while (def_p->name != NULL) {
		if ((strcasecmp (dev->name, def_p->name) == 0) ||
		    (strcasecmp (mc->machine_name, def_p->name) == 0)) {
			if (dev->base == 0)
				dev->base = def_p->base;
			if (dev->size == 0)
				dev->size = def_p->size;
			if (dev->intr.interrupts[0] == 0)
				memcpy (dev->intr.interrupts,
					def_p->interrupts,
					sizeof (def_p->interrupts));
			return;
		}
		def_p++;
	}
}

/* find a module from mod_set
 * */
static struct device_module *
find_module (const char *name, struct device_module_set *mod_set)
{
	int i;
	struct device_module *dev_mod = NULL;
	if (!name || *name == '\0')
		return NULL;
	for (i = 0; i < mod_set->count; i++) {
		dev_mod = mod_set->mod[i];
		if (strcmp (dev_mod->type_name, name) == 0)
			break;
	}
	return dev_mod;
}

/**
* @brief find a device module set according to its device name
*
* @param name
*
* @return 
*/
static struct device_module_set *
find_module_set (const char *name)
{
	int i;
	struct device_module_set *mod_set = NULL;
	for (i = 0; i < mod_set_count; i++) {
		mod_set = global_mod_set[i];
		if (strcmp (mod_set->name, name) == 0)
			break;
	}
	return mod_set;
}

/**
* @brief setup a device
*
* @param dev_name
* @param conf
* @param option
* @param mc
*
* @return 
*/
int
setup_device (char *dev_name, struct common_config *conf,
	      void *option, machine_config_t * mc)
{
	struct device_module_set *mod_set = NULL;
	struct device_module *dev_mod = NULL;
	struct device_desc *dev;

	if ((mod_set = find_module_set (dev_name)) == NULL) {
		SKYEYE_ERR ("can't find module set: %s\n", dev_name);
		return 1;
	}
	if (mod_set->initialized == 0) {
		mod_set->init (mod_set);
		mod_set->initialized = 1;
	}
	if ((dev_mod = find_module (conf->type, mod_set)) == NULL) {
		SKYEYE_ERR ("can't find device module: (name:%s, type:%s)\n",
			    mod_set->name, conf->type);
		return 1;
	}
	mc->devices = realloc (mc->devices,
			       sizeof (struct device_desc *) *
			       (mc->dev_count + 1));
	if (mc->devices == NULL) {
		SKYEYE_ERR ("can't alloc memory for devices pionter: (name:%s, type:%s)\n",
			    mod_set->name, conf->type);
		return 1;
	}
	dev = skyeye_mm_zero (sizeof (struct device_desc));
	if (dev == NULL) {
		SKYEYE_ERR ("can't alloc memory for new device: (name:%s, type:%s)\n",
			    mod_set->name, conf->type);
		return 1;
	}
	/* init device
	 * */
	strncpy (dev->type, conf->type, MAX_STR_NAME);
	dev->type[MAX_STR_NAME - 1] = '\0';

	if (conf->name && *(conf->name) != '\0') {
		strncpy (dev->name, conf->name, MAX_STR_NAME);
		dev->name[MAX_STR_NAME - 1] = '\0';
	}
	dev->base = conf->base;
	dev->size = conf->size;

	memcpy (dev->intr.interrupts, conf->interrupts,
		sizeof (conf->interrupts));
	dev->intr.set_interrupt = mc->mach_set_intr;
	dev->intr.pending_interrupt = mc->mach_pending_intr;
	dev->intr.update_interrupt = mc->mach_update_intr;

	dev->mem_op.write_byte = mc->mach_mem_write_byte;
	dev->mem_op.read_byte = mc->mach_mem_read_byte;
	dev->mach = (void *) mc;


	/* setup device module data
	 * */
	if (mod_set->setup_module) {
		if (mod_set->setup_module (dev, option) != 0) {
			free(dev);
			SKYEYE_ERR ("failed to setup_module (name:%s, type:%s)\n",
				    mod_set->name, conf->type);
			return 1;
		}
	}

	mc->devices[mc->dev_count] = dev;
	mc->dev_count++;

	/* setup device data itself.
	 * */
	dev_mod->setup (dev);

	return 0;
}

/* register a "device module set" to the global module set "global_mod_set". 
 * called by the device register funtions. e.g. uart_register.
 * */
int
register_device_module_set (struct device_module_set *mod_set)
{
	if (mod_set_count == mod_set_count_max) {
		mod_set_count_max += 4;
		global_mod_set = realloc (global_mod_set,
					  sizeof (struct device_module_set *)
					  * mod_set_count_max);
		if (global_mod_set == NULL)
			return 1;
	}
	global_mod_set[mod_set_count] = mod_set;
	mod_set_count++;
	return 0;
}

/* register a "device module" to a specific "device module set", 
 * e.g. uart or net module set
 * called by device module initialize funtions. e.g. uart_s3c4510b_init.
 * */
int
register_device_module (char *name, struct device_module_set *mod_set,
			int (*setup) (struct device_desc * dev))
{
	struct device_module *dev_mod;
	if (mod_set->count == mod_set->count_max) {
		mod_set->count_max += 4;
		mod_set->mod = realloc (mod_set->mod,
					sizeof (struct device_module *) *
					mod_set->count_max);
		if (mod_set->mod == NULL)
			return 1;

	}
	dev_mod = skyeye_mm_zero (sizeof (struct device_module));
	if (dev_mod == NULL)
		return 1;
	dev_mod->type_name = strdup (name);
	dev_mod->setup = setup;
	mod_set->mod[mod_set->count] = dev_mod;
	mod_set->count++;
	return 0;
}

/** 
 * setup device option.
 * all device options may have common parameters. Here we handle these common parameters.
 * 
 */
int
setup_device_option (char *option_name, void *dev_option,
		     int num_params, const char *params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	struct common_config conf;
	int i;

	memset (&conf, 0, sizeof (conf));
	conf.type = NULL;
	conf.name = NULL;

	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: \"%s\" has wrong parameter \"%s\".\n",
				 option_name, name);
		if (!strncmp ("type", name, strlen (name))) {
			conf.type = strdup (value);
		}
		else if (!strncmp ("name", name, strlen (name))) {
			conf.name = strdup (value);
		}
		else if (!strncmp ("base", name, strlen (name))) {
			conf.base = strtoul (value, NULL, 0);
		}
		else if (!strncmp ("size", name, strlen (name))) {
			conf.size = strtoul (value, NULL, 0);
		}
		else if (!strncmp ("int", name, strlen (name))) {
			get_interrupts (value, conf.interrupts);
		}
	}
	skyeye_config_t* config;
	config = get_current_config();
	setup_device (option_name, &conf, dev_option, config->mach);
	if (conf.type)
		free (conf.type);
	if (conf.name)
		free (conf.name);
	return 0;

}
