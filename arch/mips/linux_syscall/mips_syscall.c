#include "../common/emul.h"
#include "../common/mips_regformat.h"
#include "syscall_nr.h"
#include <bank_defs.h>
#include <skyeye_ram.h>
#include "syscall_nr.h"
#include <bank_defs.h>
#include <skyeye_config.h>
#include <skyeye_swapendian.h>
#include <sim_control.h>
#include <skyeye_types.h>
#include <skyeye_command.h>
#include "dyncom/defines.h"

#include <sys/types.h>
#include <unistd.h>			//read,write...
#include <string.h>
#include <fcntl.h>
#include <sys/utsname.h>	//uname
#include <time.h>
#include <sys/uio.h>		//writev
#include <sys/mman.h>		//mmap
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <linux/unistd.h>			//exit_group

static void bus_read_string(char *buf, char *str){
	int i = 0;
	char tmp[1024];
	while(1){
		bus_read(8, str + i ,tmp + i);
		if(tmp[i] != '\0'){
			i++;
		}else{
			buf[i + 1] = '\0';
			strcpy(buf, tmp);
			break;
		}
	}
}
static void bus_write_string(char *buf, char *str){
	int len = strlen(str);
	int i = 0;
	for (i = 0; i < len; i++){
		bus_write(8, buf + i, *((char *)(str + i)));
	}
	bus_write(8, buf + i, '\0');
}

static int brk_static = 0x20000000;
int mips_syscall(mips_core_t* core, int num){
	int syscall_number = num;
	switch(syscall_number){
		case SYSCALL_exit:{		/* 1 */
			exit(0);
			break;
		}
		case SYSCALL_read:{		/* 3 */
			printf("syscall 3\n");
			int fd = core->gpr[a0];
			char *buff = core->gpr[a1];
			int count = core->gpr[a2];
			void *tmp = malloc(count);
			int size = read(fd, tmp, count);
			int i;
			for( i = 0; i < size; i ++){
				bus_write(8, buff + i, ((char*)tmp)[i]);
			}
			free(tmp);
			core->gpr[v0] = size;
			core->gpr[a3] = 0;

			break;
		}
		case SYSCALL_write:{		/* 4 */
			printf("syscall 4 \n");
			int fd = core->gpr[a0];
			char *buff = core->gpr[a1];
			int bytes = core->gpr[a2];
			void *tmp = malloc(bytes);
			char tmp8;

			if(tmp == NULL){
				printf("error: syscall write (malloc return NULL)\n");
				exit(0);
			}
			int i = 0;
			for (; i<bytes; i++){
				bus_read(8, buff + i, &tmp8);
				((uint8_t*)tmp)[i] = (uint8_t)tmp8;
			}
			int result = write(fd, tmp, bytes);
			core->gpr[v0] = result;
			core->gpr[a3] = 0;
			free(tmp);
			break;
		}
		case SYSCALL_open:{		/* 5 */
			printf("syscall 5\n");
			char *path = (char *)core->gpr[a0];
			int flags = core->gpr[a1];
			int mode = core->gpr[a2];
			char path_name[1024];
			bus_read_string(path_name, path);
			printf(" path name is %s\n", path_name);
			//FIXME:how about open without mode ???
			core->gpr[v0] = open(path_name, flags, mode);
			core->gpr[a3] = 0;
			break;
		}
		case SYSCALL_close:{		/* 6 */
			printf("syscall 6\n");
			int fd = core->gpr[a0];
			close(fd);
			core->gpr[a3] = 0;
			break;
		}
		case SYSCALL_time:{		/* 13 */
			printf("syscall 13 null\n");
			break;
		}
		case SYSCALL_getuid32:	/* 24 */
			printf("syscall 24 \n");
			core->gpr[v0] = getuid();
			core->gpr[a0] = 0;
			break;
		case SYSCALL_times:{		/* 43 */
			printf("syscall 43 null\n");
			break;
		}
		case SYSCALL_brk:{		/* 45 */
			printf("syscall 45\n");
			if(core->gpr[a0]){
				brk_static = core->gpr[a0];
				core->gpr[v0] = 0;
			}else
				core->gpr[v0] = brk_static;

			core->gpr[a3] = 0;
			break;
		}
		case SYSCALL_getgid32:	/* 47 */
			printf("syscall 47\n");
			core->gpr[v0] = getgid();
			core->gpr[a3] = 0;
			break;
		case SYSCALL_geteuid32:	/* 49 */
			printf("syscall 49 \n");
			core->gpr[v0] = geteuid();
			core->gpr[a3] = 0;
			break;
		case SYSCALL_getegid32:	/* 50 */
			printf("syscall 50 \n");
			core->gpr[v0] = getegid();
			core->gpr[a3] = 0;
			break;
		case SYSCALL_ioctl:{		/* 54 */
			printf("syscall 54 null\n");
			break;
		}
		case SYSCALL_mmap:{		/* 90 */
			printf("syscall 90 null\n");
			break;
		}
		case SYSCALL_munmap:{		/* 91 */
			printf("syscall 91 null\n");
			break;
		}
		case SYSCALL_uname:{		/* 122 */
			printf("syscall 122\n");
			struct utsname *tmp = (struct utsname *)core->gpr[a0];
			struct utsname uname_tmp;
			int result = uname(&uname_tmp);
			if (result == 0){
				bus_write_string(tmp->sysname, "Linux");
				bus_write_string(tmp->nodename, "skyeye");
				bus_write_string(tmp->release, uname_tmp.release);
				bus_write_string(tmp->version, uname_tmp.version);
				bus_write_string(tmp->machine, "mips");
				core->gpr[a3] = 0;
			}else{
				core->gpr[a3] = -1;
				//TODO:ERROR handle
			}
			break;
		}
		case SYSCALL_writev:{		/* 146 */
			printf("syscall 146 null\n");
			break;
		}
		case SYSCALL_exit_group:{		/* 234 */
			printf("syscall 234 null\n");
			break;
		}
		case SYSCALL_splice:{		/* 283 */
			printf("syscall 283 null\n");

			core->gpr[a3] = 0;
			break;
		}
		default:
			printf("in syscall num is %d\n", syscall_number);
			exit(-1);
	}
	return 0;
}
