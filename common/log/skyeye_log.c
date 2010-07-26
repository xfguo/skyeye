#include <stdio.h>
#include <stdarg.h>
#include <config.h>
#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_log.h"

const char* Front_message = "SkyEye is an Open Source project under GPL. All rights of different parts or modules are reserved by their author. Any modification or redistributions of SkyEye should not remove or modify the annoucement of SkyEye copyright. \n\
Get more information about it, please visit the homepage http://www.skyeye.org.\n\
Type \"help\" to get command list. \n ";

char* get_front_message(){
	printf("%s\n%s", PACKAGE_STRING, Front_message);
	return Front_message;
}

int
do_log_option (skyeye_option_t * this_option, int num_params,
	       const char *params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	int i, fd, logon, memlogon;
	unsigned long long start, end, length;
#if 0
	/*2004-08-09 chy init skyeye_config.log */
	skyeye_config.log.log_fd = 0;
	skyeye_config.log.logon = 0;
	skyeye_config.log.memlogon = 0;
	skyeye_config.log.start = 0;
	skyeye_config.log.end = 0;
	skyeye_config.log.length = 0;

	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("log_info: Error: log has wrong parameter \"%s\".\n",
				 name);
		if (!strncmp ("logon", name, strlen (name))) {
			sscanf (value, "%d", &logon);
			if (logon != 0 && logon != 1)
				SKYEYE_ERR
					("log_info: Error logon value %d\n",
					 logon);
			if (logon == 1) {
				SKYEYE_INFO ("log_info: log is on.\n");
			}
			else {
				SKYEYE_INFO ("log_info: log is off.\n");
			}
			skyeye_config.log.logon = logon;
		}
		else if (!strncmp ("memlogon", name, strlen (name))) {
			sscanf (value, "%d", &memlogon);
			if (memlogon != 0 && memlogon != 1)
				SKYEYE_ERR
					("log_info: Error logon value %d\n",
					 memlogon);
			if (memlogon == 1) {
				SKYEYE_INFO ("log_info: memory klog is on.\n");
			}
			else {
				SKYEYE_INFO ("log_info: memory log is off.\n");
			}
			skyeye_config.log.memlogon = memlogon;
		}
		else if (!strncmp ("logfile", name, strlen (name))) {
			if ((skyeye_logfd = fopen (value, "w+")) == NULL) {
				//SKYEYE_DBG("SkyEye Error when open log file %s\n", value);
				perror ("SkyEye: Error when open log file:  ");
				skyeye_exit (-1);
			}
			skyeye_config.log.log_fd = skyeye_logfd;
			SKYEYE_INFO ("log_info:log file is %s, fd is 0x%x\n",
				     value, skyeye_logfd);
		}
		else if (!strncmp ("start", name, strlen (name))) {
			start = strtoul (value, NULL, 0);
			skyeye_config.log.start = start;
			SKYEYE_INFO ("log_info: log start clock %llu\n",
				     start);
		}
		else if (!strncmp ("end", name, strlen (name))) {
			end = strtoul (value, NULL, 0);
			skyeye_config.log.end = end;
			SKYEYE_INFO ("log_info: log end clock %llu\n", end);
		}
		else if (!strncmp ("length", name, strlen (name))) {
			sscanf (value, "%llu", &length);
			skyeye_config.log.length = length;
			SKYEYE_INFO ("log_info: log instr length %llu\n",
				     length);
		}
		else
			SKYEYE_ERR ("Error: Unknown cpu name \"%s\"\n", params[0]);
	}
#endif
	return 0;
}

static char* exp_str[] = {
	"No exception",
	"Malloc failed",
	"Can not open file",
	"Can not open DLL",
	"Invalid argument",
	"Invalid module",
	NULL
};
char* get_exp_str(exception_t exp){
	const char* no_exp_str = "No such exception";
	int exp_str_len = 0;
	while(exp_str[exp_str_len++] != NULL);
	/* the last element is NULL in exp_str */
	if(exp >= exp_str_len - 1)
		return no_exp_str;
	return exp_str[exp];
}
void skyeye_log(log_level_t log_level, char* func_name, char* format, ...){
	static char buf[1024];
	memset(buf, '\0', 1024);
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	if(log_level >= Error_log)
		fprintf(stderr, "In %s, %s\n", func_name, buf);
	else{
		printf("In %s, %s\n", func_name, buf);
	}
}
