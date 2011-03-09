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

int mips_syscall(mips_core_t* core, int num){
	int syscall_number = num;
	switch(syscall_number){
		case SYSCALL_exit:{		/* 3 */
			exit(0);
			break;
		}
		case SYSCALL_read:{		/* 3 */
			printf("syscall 3 null\n");
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
			free(tmp);
			break;
		}
		case SYSCALL_open:{		/* 5 */
			printf("syscall 5 null\n");
			break;
		}
		case SYSCALL_close:{		/* 6 */
			printf("syscall 6 null\n");
			break;
		}
		case SYSCALL_time:{		/* 13 */
			printf("syscall 13 null\n");
			break;
		}
		case SYSCALL_getuid32:	/* 24 */
			printf("syscall 24 null\n");
			break;
		case SYSCALL_times:{		/* 43 */
			printf("syscall 43 null\n");
			break;
		}
		case SYSCALL_brk:{		/* 45 */
			printf("syscall 45 null\n");
			break;
		}
		case SYSCALL_getgid32:	/* 47 */
			printf("syscall 47 null\n");
			break;
		case SYSCALL_geteuid32:	/* 49 */
			printf("syscall 49 null\n");
			break;
		case SYSCALL_getegid32:	/* 50 */
			printf("syscall 50 null\n");
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
			printf("syscall 122 null\n");
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
		default:
			printf("in syscall num is %x\n", syscall_number);
			exit(-1);
	}
	return 0;
}
