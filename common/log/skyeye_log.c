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
* @file skyeye_log.c
* @brief the log module of skyeye
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdio.h>
#include <stdarg.h>
#include <config.h>
#include "skyeye_options.h"
#include "skyeye_config.h"
#include "skyeye_log.h"

/**
* @brief the welcome message of cli
*/
const char* Front_message = "SkyEye is an Open Source project under GPL. All rights of different parts or modules are reserved by their author. Any modification or redistributions of SkyEye should not remove or modify the annoucement of SkyEye copyright. \n\
Get more information about it, please visit the homepage http://www.skyeye.org.\n\
Type \"help\" to get command list. \n ";

/**
* @brief get the welcome message
*
* @return 
*/
char* get_front_message(){
	printf("%s\n%s", PACKAGE_STRING, Front_message);
	return Front_message;
}


/**
* @brief By default, only warnning and some error displayed
*/
static log_level_t current_log_level = Quiet_log;

/**
* @brief the handler for log option
*
* @param this_option
* @param num_params
* @param params[]
*
* @return 
*/
int
do_log_option (skyeye_option_t * this_option, int num_params,
	       const char *params[])
{
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	log_level_t level = current_log_level;
	int i;
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0){
			SKYEYE_ERR
				("log_info: Error: log has wrong parameter \"%s\".\n",
				 name);
			continue;
		}
		if (!strncmp ("level", name, strlen (name))) {
			sscanf (value, "%d", &level);
			if (level < Quiet_log || level >= MAX_LOG_LEVEL){
				SKYEYE_ERR
					("log_info: Error log level %d\n",
					 level);
			}
			else
				current_log_level = level;
			break;
		}
	}
	return 0;
}

/**
* @brief the exception string for various exception type
*/
static char* exp_str[] = {
	"No exception",
	"Malloc failed",
	"Can not open file",
	"Can not open DLL",
	"Invalid argument",
	"Invalid module",
	NULL
};

/**
* @brief get the exception string for the given exception
*
* @param exp
*
* @return 
*/
char* get_exp_str(exception_t exp){
	const char* no_exp_str = "No such exception";
	int exp_str_len = 0;
	while(exp_str[exp_str_len++] != NULL);
	/* the last element is NULL in exp_str */
	if(exp >= exp_str_len - 1)
		return no_exp_str;
	return exp_str[exp];
}

void skyeye_info(char* format, ...){
	va_list args;
	va_start(args, format);
	skyeye_log(Info_log, NULL, format, args);
	va_end(args);
	return;
}

/**
* @brief log function of skyeye
*
* @param log_level
* @param func_name
* @param format
* @param ...
*/
void skyeye_log(log_level_t log_level,const char* func_name, char* format, ...){
	static char buf[1024];
	memset(buf, '\0', 1024);
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	if(log_level >= Error_log)
		fprintf(stderr, "In %s, %s\n", func_name, buf);
	else{
		if(current_log_level == Quiet_log){
			/* output nothing */
		}
		else if(log_level >= current_log_level)
			if(log_level == Info_log)
				printf("INFO: %s\n", buf);
			else
				printf("DEBUG:[func %s], %s\n", func_name, buf);
		else
			;/* output nothing */
	}
}

void skyeye_printf_in_color(COLOR_TYPE type, char *format, ...)
{
	static char buf[1024];
	memset(buf, '\0', 1024);
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	switch (type) {
	case BLACK:
		_debug_in_black("%s", buf);
		break;
	case BLUE:
		_debug_in_blue("%s", buf);
		break;
	case BROWN:
		_debug_in_brown("%s", buf);
		break;
	case CYAN:
		_debug_in_cyan("%s", buf);
		break;
	case DARK_GRAY:
		_debug_in_dark_gray("%s", buf);
		break;
	case GREEN:
		_debug_in_green("%s", buf);
		break;
	case LIGHT_BLUE:
		_debug_in_light_blue("%s", buf);
		break;
	case LIGHT_CYAN:
		_debug_in_light_cyan("%s", buf);
		break;
	case LIGHT_GRAY:
		_debug_in_light_gray("%s", buf);
		break;
	case LIGHT_GREEN:
		_debug_in_light_green("%s", buf);
		break;
	case LIGHT_PURPLE:
		_debug_in_light_purple("%s", buf);
		break;
	case LIGHT_RED:
		_debug_in_light_red("%s", buf);
		break;
	case PURPLE:
		_debug_in_purple("%s", buf);
		break;
	case RED:
		_debug_in_red("%s", buf);
		break;
	default:
		fprintf(stderr, "Not supported color type %d\n", type);
		break;
	}
}
