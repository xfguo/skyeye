/* 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * author gbf0871 <gbf0871@126.com>
 */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//#include "armdefs.h"
#include "skyeye_device.h"

#include "skyeye_options.h"
#include "skyeye.h"
#include "skyeye_nandflash.h"

extern void nandflash_s3c2410_init(struct device_module_set *mod_set);

static int
do_nandflash_option (skyeye_option_t * this_option, int num_params,
		 const char *params[]);
/* initialize the nandflash module set.
 * If you want to add a new nandflash simulation, just add a "flash_*_init" function to it.
 * */
static void
nandflash_init (struct device_module_set *mod_set)
{
	nandflash_s3c2410_init (mod_set);
}

nandflash_module_option * get_nandflashmd(char *name)
{
	nandflash_module_option * md=nandflash_module_data;
	int i;
	for(i=0;i<sizeof(nandflash_module_data);i++,md++)
	{
		if (!strcmp(name,md->name))
		  return md;
	}
	return NULL;
}

int nandflash_module_setup(struct nandflash_device *dev,char *name)
{
  	nandflash_module_option * md;
  	int ret=-1;
  	int i;
  	md=get_nandflashmd(name);
  	if(md!=NULL){
  		dev->install=md->install_dev;
  		dev->uinstall=md->uinstall_dev;
  		dev->pagesize=md->pagesize;
  		dev->obbsize=md->obbsize;
  		dev->pagenum=md->pagenum;
  		dev->blocknum=md->blocknum;
  		dev->pagedumpsize=dev->pagesize+dev->obbsize;
  		dev->erasesize=dev->pagedumpsize*dev->pagenum;
  		dev->devicesize=dev->erasesize*dev->blocknum;
  		for(i=0;i<5;i++)
  			dev->ID[i]=md->ID[i];
		//md->install_dev(dev);
  		ret=0;
  	}
	else
	{
		printf("nandflash not support this:%s\n",name);
		exit(1);
	}
}

static int
nandflash_setup (struct device_desc *dev, void *option)
{
	struct nandflash_device *nandflash_dev;
	struct flash_option *nandflash_opt = (struct flash_option *) option;
	int ret = 0;

	nandflash_dev =
		(struct nandflash_device *) malloc (sizeof (struct nandflash_device));
	if (nandflash_dev == NULL)
		return 1;

	memset (nandflash_dev, 0, sizeof (struct nandflash_device));
	memcpy (&nandflash_dev->dump[0], &nandflash_opt->dump[0], MAX_STR_NAME);

	dev->dev = (void *) nandflash_dev;
	return ret;

}
static struct device_module_set nandflash_mod_set = {
	.name = "nandflash",
	.count = 0,
	.count_max = 0,
	.init = nandflash_init,
	.initialized = 0,
	.setup_module = nandflash_setup,
};

/* used by global device initialize function. 
 * */
void
nandflash_register ()
{
	if (register_device_module_set (&nandflash_mod_set))
		SKYEYE_ERR ("\"%s\" module set register error!\n",
			    nandflash_mod_set.name);
	register_option("nandflash", do_nandflash_option, "");
}
int
do_nandflash_option (skyeye_option_t * this_option, int num_params,
		 const char *params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	struct flash_option flash_opt;
	int i, fd;

	memset(&flash_opt, 0, sizeof(struct flash_option));

	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: flash has wrong parameter \"%s\".\n",
				 name);

		if (!strncmp("dump", name, strlen(name))) {
			memcpy(&flash_opt.dump[0], value, strlen(value) + 1);
		}
	}

	SKYEYE_INFO ("nandflash: dump %s\n",
		     flash_opt.dump[0] == 0 ? "none" : flash_opt.dump);

	setup_device_option (this_option->option_name, (void *) &flash_opt,
			     num_params, params);

	return 0;
}

