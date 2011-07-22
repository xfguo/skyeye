#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_module.h"
#include "skyeye_cli.h"

int
cli(const char* prompt)
{
	char new_path[1024];
	char* skyeye_bin = SKYEYE_BIN;
	setenv("SKYEYEBIN", skyeye_bin, 1);
	Py_Initialize();
	sprintf(new_path, "sys.path.append(\"%s\")\n", skyeye_bin);
	//printf("new_path=%s\n", new_path);
	PyRun_SimpleString("import sys\n");
	PyRun_SimpleString(new_path);
	PyRun_SimpleString("from skyeye_cli import *\nSkyEyeCli().cmdloop()\n");
	Py_Finalize();
	return 0;
}
/* module name */
const char* skyeye_module = "python-cli";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	register_cli(cli);
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
}
