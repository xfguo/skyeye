/**
  * @file ppc_syscall.c
  *
  * The implementation of powerpc system call for simulate user mode
  *
  * @author Michael.Kang, ZhijieLi
  * @Version
  * @Date: 2010-11-22
  */

/**
* gpr[0]:syscall number
* gpr[3]~:syscall arguments
* gpr[3]:return value of syscall
* FIXME:Where is ERRNO to write in PowerPC ??? r0
*/
#include "ppc_cpu.h"
#include "syscall_nr.h"
#include <bank_defs.h>
#include <skyeye_ram.h>
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

extern void ppc_dyncom_stop(e500_core_t* core);
struct ppc_stat64{
	unsigned long long st_dev;
    unsigned long long st_ino;
	unsigned int st_mode;
	unsigned int st_nlink;
	unsigned int st_uid;
	unsigned int st_gid;
	unsigned long long st_rdev;
	unsigned long long __pad0;
	long long st_size;
	int st_blksize;
	unsigned int __pad1;
	long long st_blocks;
	int st_atim;
    unsigned int st_atime_nsec;
	int st_mtim;
    unsigned int st_mtime_nsec;
	int st_ctim;
    unsigned int st_ctime_nsec;
};
struct ppc_uname{
	
};
#define SYSCALL_DEBUG 0
#define debug(...) do { if (SYSCALL_DEBUG) printf(__VA_ARGS__); } while(0)
#define DUMP_SYSCALL 0
#define dump(...) do { if (DUMP_SYSCALL) printf(__VA_ARGS__); } while(0)

/**
 * @brief write a string to memory
 *
 * @param buf target string buffer
 * @param str source string buffer
 */
 void bus_write_string(char *buf, char *str){
	int len = strlen(str);
	int i = 0;
	for (i = 0; i < len; i++){
		bus_write(8, buf + i, *((char *)(str + i)));
	}
	bus_write(8, buf + i, '\0');
}
/**
 * @brief read a string from memory
 *
 * @param buf target string buffer
 * @param str source string buffer
 */
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

static void bus_write64_ppc(uint32_t addr, uint64_t value){
	uint32_t value_l = (uint32_t)value;
	uint32_t value_h = (uint32_t)(value >> 32);
	bus_write(32, addr, value_h);
	bus_write(32, addr + 4, value_l);
}

//For debug
static void memory_view(){
	mem_config_t *mem_config = get_global_memmap();
	int bank_num = mem_config->bank_num;
	printf("There are %d memory banks.\n", mem_config->bank_num);
	int i = 0;
	for (; i<bank_num; i++){
		printf("BANK:name=%s,addr=0x%x,len=%x\n",mem_config->mem_banks[i].objname,
											mem_config->mem_banks[i].addr,
											mem_config->mem_banks[i].len);
	}
}

/**
 * @brief For mmap syscall.A mmap_area is a memory bank.
 */
typedef struct mmap_area{
	mem_bank_t bank;
	void *mmap_addr;
	struct mmap_area *next;
}mmap_area_t;
mmap_area_t *mmap_global = NULL;

#ifdef FAST_MEMORY
#define mmap_base 0x20000000
#else
#define mmap_base 0x50000000
#endif
static long mmap_next_base = mmap_base;

static char mmap_mem_write(short size, int addr, uint32_t value);
static char mmap_mem_read(short size, int addr, uint32_t * value);

static mmap_area_t* new_mmap_area(int sim_addr, int len){
	mmap_area_t *area = (mmap_area_t *)malloc(sizeof(mmap_area_t));
	if(area == NULL){
		printf("error ,failed %s\n",__FUNCTION__);
		exit(0);
	}
	memset(area, 0x0, sizeof(mmap_area_t));
	area->bank.addr = mmap_next_base;
	area->bank.len = len;
	area->bank.bank_write = mmap_mem_write;
	area->bank.bank_read = mmap_mem_read;
	area->bank.type = MEMTYPE_RAM;
	area->bank.objname = "mmap";
	addr_mapping(&area->bank);
#ifdef FAST_MEMORY
	area->mmap_addr = (uint8_t*)get_dma_addr(mmap_next_base);
#else
	area->mmap_addr = malloc(len);
	if(area->mmap_addr == NULL){
		printf("error mmap malloc\n");
		exit(0);
	}
	memset(area->mmap_addr, 0x0, len);
#endif
	area->next = NULL;
	if(mmap_global){
		area->next = mmap_global->next;
		mmap_global->next = area;
	}else{
		mmap_global = area;
	}
	mmap_next_base = mmap_next_base + len + 1024;
	return area;
}

static mmap_area_t *get_mmap_area(int addr){
	mmap_area_t *tmp = mmap_global;
	while(tmp){
		if ((tmp->bank.addr <= addr) && (tmp->bank.addr + tmp->bank.len > addr)){
			return tmp;
		}
		tmp = tmp->next;
	}
	printf("cannot get mmap area:addr=0x%x\n", addr);
	return NULL;
}

/**
 * @brief the mmap_area bank write function.
 *
 * @param size size to write, 8/16/32
 * @param addr address to write
 * @param value value to write
 *
 * @return sucess return 1,otherwise 0.
 */
static char mmap_mem_write(short size, int addr, uint32_t value){
	mmap_area_t *area_tmp = get_mmap_area(addr);
	mem_bank_t *bank_tmp = &area_tmp->bank;
	int offset = addr - bank_tmp->addr;
	switch(size){
		case 8:{
			uint8_t value_endian = value;
			*(uint8_t *)&(((char *)area_tmp->mmap_addr)[offset]) = value_endian;
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,value_endian);
			break;
		}
		case 16:{
			uint16_t value_endian = half_to_BE((uint16_t)value);
			*(uint16_t *)&(((char *)area_tmp->mmap_addr)[offset]) = value_endian;
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,value_endian);
			break;
		}
		case 32:{
			uint32_t value_endian = word_to_BE((uint32_t)value);
			*(uint32_t *)&(((char *)area_tmp->mmap_addr)[offset]) = value_endian;
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,value_endian);
			break;
		}
		default:
			printf("invalid size %d\n",size);
			return 0;
	}
	return 1;
}

/**
 * @brief the mmap_area bank read function.
 *
 * @param size size to read, 8/16/32
 * @param addr address to read
 * @param value value to read
 *
 * @return sucess return 1,otherwise 0.
 */
static char mmap_mem_read(short size, int addr, uint32_t * value){
	mmap_area_t *area_tmp = get_mmap_area(addr);
	mem_bank_t *bank_tmp = &area_tmp->bank;
	int offset = addr - bank_tmp->addr;
	switch(size){
		case 8:{
			*(uint8_t *)value = *(uint8_t *)&(((uint8_t *)area_tmp->mmap_addr)[offset]);
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,*(uint8_t*)value);
			break;
		}
		case 16:{
			*(uint16_t *)value = half_from_BE(*(uint16_t *)&(((uint8_t *)area_tmp->mmap_addr)[offset]));
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,*(uint16_t*)value);
			break;
		}
		case 32:
			*value = (uint32_t)word_from_BE(*(uint32_t *)&(((uint8_t *)area_tmp->mmap_addr)[offset]));
			debug("in %s,size=%d,addr=0x%x,value=0x%x\n",__FUNCTION__,size,addr,*(uint32_t*)value);
			break;
		default:
			printf("invalid size %d\n",size);
			return 0;
	}
	return 1;
}

/**
 * @brief The implementation of system call for user mode
 *
 * @param core powerpc e500 core
 *
 * @return 0 if sucess 
 */
int ppc_syscall(e500_core_t* core){
	int syscall_number = core->gpr[0];
	switch(syscall_number){
		case TARGET_NR_read:{		/* 3 */
			int fd = core->gpr[3];
			unsigned long buff = core->gpr[4];
			int count = core->gpr[5];
			void *tmp = malloc(count);
			if(tmp == NULL){
				printf("error: syscall read (malloc return NULL)\n");
				exit(0);
			}
			int size = read(fd, tmp, count);
			int i = 0;
			for (i = 0; i < size; i++){
				bus_write(8, buff + i, ((char*)tmp)[i]);
			}
			free(tmp);
			core->gpr[3] = size;
			dump("syscall %d read(%d, 0x%x, %d) = %d\n",TARGET_NR_read, fd, buff, count, core->gpr[3]);
			break;
		}
		case TARGET_NR_write:{		/* 4 */
			int fd = core->gpr[3];
			unsigned long buff = core->gpr[4];
			int bytes = core->gpr[5];
			void *tmp = malloc(bytes);
			if(tmp == NULL){
				printf("error: syscall write (malloc return NULL)\n");
				exit(0);
			}
			int i = 0;
			uint32_t tmp32;
			for (; i<bytes; i++){
				bus_read(8, buff + i, &tmp32);
				((uint8_t*)tmp)[i] = (uint8_t)tmp32;
			}
			int result = write(fd, tmp, bytes);
			core->gpr[3] = result;
			free(tmp);
			dump("syscall %d write(%d, 0x%x, %d) = %d\n",TARGET_NR_write, fd, buff, bytes, core->gpr[3]);
			break;
		}
		case TARGET_NR_open:{		/* 5 */
			char *path = (char *)core->gpr[3];
			int flags = core->gpr[4];
			int mode = core->gpr[5];
			char path_name[1024];
			bus_read_string(path_name, path);
			debug("path=%s,flag=%d,mode=%d\n",path_name,flags,mode);
			//FIXME:how about open without mode ???
			core->gpr[3] = open(path_name, flags, mode);
			dump("syscall %d open(%s, 0x%x, 0x%x) = %d\n",TARGET_NR_open, path_name, flags, mode, core->gpr[3]);
			break;
		}
		case TARGET_NR_close:{		/* 6 */
			int fd = core->gpr[3];
			close(fd);
			core->gpr[3] = 0;
			dump("syscall %d close(%d)\n",TARGET_NR_close, fd);
			break;
		}
		case TARGET_NR_time:{		/* 13 */
			time_t t;
			time(&t);
			bus_write(32, core->gpr[3], t);
			core->gpr[3] = t;
			dump("syscall %d time\n",TARGET_NR_time);
			break;
		}
		case TARGET_NR_getuid32:	/* 24 */
			core->gpr[3] = getuid();
			dump("syscall %d getuid() = %d\n",TARGET_NR_getuid32, core->gpr[3]);
			break;
		case TARGET_NR_times:{		/* 43 */
			struct tms *buff = (struct tms*)core->gpr[3];
			struct tms tmp;
			uint32_t result = times(&tmp);
			bus_write(32, &buff->tms_utime, tmp.tms_utime);
			bus_write(32, &buff->tms_stime, tmp.tms_stime);
			bus_write(32, &buff->tms_cutime, tmp.tms_cutime);
			bus_write(32, &buff->tms_cstime, tmp.tms_cstime);
			core->gpr[3] = result;
			//////FOR DIFF LOG
///			printf("utime=%d,stime=%d,cutime=%d,cstime=%d,result=%x\n",tmp.tms_utime,tmp.tms_stime,tmp.tms_cutime,tmp.tms_cstime,result);
#if 0
			bus_write(32, &buff->tms_utime, 677);
			bus_write(32, &buff->tms_stime, 20);
			bus_write(32, &buff->tms_cutime, 0);
			bus_write(32, &buff->tms_cstime, 0);
			core->gpr[3] = 0x66cbacf0;	//for diff log
#endif
			dump("syscall %d times(0x%x) = %d\n",TARGET_NR_times, buff, result);
			break;
		}
		case TARGET_NR_brk:{		/* 45 */
			void *addr = core->gpr[3];
			debug("set data segment to 0x%x\n", addr);
			//NOTE:powerpc brk syscall return end of data segment address instead of zero when success??
			int i;
			core->gpr[3] = addr;
			dump("syscall %d brk(0x%x) = 0x%x\n", TARGET_NR_brk, core->gpr[3]);
			break;
		}
		case TARGET_NR_getgid32:	/* 47 */
			core->gpr[3] = getgid();
			dump("syscall %d getgid() = %d\n",TARGET_NR_getgid32, core->gpr[3]);
			break;
		case TARGET_NR_geteuid32:	/* 49 */
			core->gpr[3] = geteuid();
			dump("syscall %d geteuid() = %d\n",TARGET_NR_geteuid32, core->gpr[3]);
			break;
		case TARGET_NR_getegid32:	/* 50 */
			core->gpr[3] = getegid();
			dump("syscall %d getegid() = %d\n",TARGET_NR_getegid32, core->gpr[3]);
			break;
		case TARGET_NR_ioctl:{		/* 54 */
			int fd = core->gpr[3];
			int cmd = core->gpr[4];
			/* FIXME:cannot deal with arguments more than two! */
			/* TODO:...*/
			core->gpr[3] = 0;
			dump("syscall %d ioctl(%x, 0x%x) = %d\n",TARGET_NR_ioctl, fd, cmd, core->gpr[3]);
			break;
		}
		case TARGET_NR_mmap:{		/* 90 */
			int mmap_dbg = 1;
			int addr = core->gpr[3];
			int len = core->gpr[4];
			int prot = core->gpr[5];
			int flag = core->gpr[6];
			int fd = core->gpr[7];
			int offset = core->gpr[8];
			if (mmap_dbg){
				debug("prot: ");
				if (prot & PROT_READ)
					debug("PROT_READ ");
				if (prot & PROT_WRITE)
					debug("PROT_WRITE ");
				if (prot & PROT_EXEC)
					debug("PROT_EXEC");
				debug("\n");
				debug("flag: ");
				/* FIXME:other flags!! */
				if (flag & MAP_SHARED)
					debug("MAP_SHARED ");
				if (flag & MAP_PRIVATE)
					debug("MAP_PRIVATE ");
				if (flag & MAP_FIXED)
					debug("MAP_FIXED ");
				if (flag & MAP_ANONYMOUS)
					debug("MAP_ANONYMOUS ");
				debug("\n");
			}
			mmap_area_t *area = new_mmap_area(addr, len);
			core->gpr[3] = area->bank.addr;
			dump("syscall %d mmap(0x%x,%x,0x%x,0x%x,%d,0x%x) = 0x%x\n",
					TARGET_NR_ioctl, addr, len, prot, flag, fd, offset, core->gpr[3]);
			//memory_view();
			break;
		}
		case TARGET_NR_munmap:{		/* 91 */
			core->gpr[3] = 0;
			dump("syscall %d mummap()\n",TARGET_NR_munmap);
			break;
		}
		case TARGET_NR_uname:{		/* 122 */
			struct utsname *tmp = (struct utsname *)core->gpr[3];
			struct utsname uname_tmp;
			int result = uname(&uname_tmp);
			if (result == 0){
				bus_write_string(tmp->sysname, "Linux");
				bus_write_string(tmp->nodename, "skyeye");
				bus_write_string(tmp->release, uname_tmp.release);
				bus_write_string(tmp->version, uname_tmp.version);
				bus_write_string(tmp->machine, "ppc");
				core->gpr[3] = 0;
			}else{
				core->gpr[3] = -1;
				//TODO:ERROR handle
			}
			dump("syscall %d uname(0x%x) = %d\n", TARGET_NR_munmap, tmp, core->gpr[3]);
			break;
		}
		case TARGET_NR_writev:{		/* 146 */
			int fd = core->gpr[3];
			struct iovec *tmp_iovec = (struct iovec *)core->gpr[4];
			int count = core->gpr[5];
			debug("syscall writev:fd=%d,count=%d\n", fd, count);
			if(count < 0 || count > UIO_MAXIOV){
				printf("error iov count:%d\n", count);
				exit(0);
			}
			int i = 0;
			int j = 0;
			for (i = 0; i < count; i++){
				int len = 0;
				char str[1024];
				bus_read(32, &tmp_iovec[i].iov_len, &len);
				debug("iovec[%d].iov_len=%d\n", i, len);
				for (j=0; j<len; j++){
					bus_read(8, &((tmp_iovec[i].iov_base)[j]), str + j);
					debug("%c\n",str[j]);
				}
				debug("string:%s\n", str);
				write(fd, str, len);
			}
			dump("syscall %d writev\n", TARGET_NR_writev);
			break;
		}
		case TARGET_NR_fstat64:{		/* 197 */
			int fd = core->gpr[3];
			struct ppc_stat64 * buff = (struct ppc_stat64 *)core->gpr[4];
			debug("syscall fstat64:fd=%d,buff=0x%x\n", fd, buff);

			struct stat64 tmp;
			if (fstat64(fd, &tmp) != 0){
				fprintf(stderr, "SYSCALL:Failed fstat64.\n");
				exit(0);
			}
			debug("size of stat64=%d,size of ppc_stat64=%d\n", sizeof(struct stat64),sizeof(struct ppc_stat64));
			debug("st_dev=%lld,st_ino=%lld\n",tmp.st_dev,tmp.st_ino);
			/* TODO:Write tmp to buff!But how do I know the size of struct stat64
			   in the target program ? Some of its items are conditional compiled.*/
			bus_write64_ppc(&buff->st_dev, tmp.st_dev);
			bus_write64_ppc(&buff->st_ino, tmp.st_ino);

			bus_write(32, &buff->st_mode, tmp.st_mode);
			bus_write(32, &buff->st_nlink, tmp.st_nlink);
			bus_write(32, &buff->st_uid, tmp.st_uid);
			bus_write(32, &buff->st_gid, tmp.st_gid);
			bus_write64_ppc(&buff->st_rdev, tmp.st_rdev);
			bus_write64_ppc(&buff->st_size, tmp.st_size);

			bus_write(32, &buff->st_blksize, tmp.st_blksize);
			bus_write64_ppc(&buff->st_blocks, tmp.st_blocks);
			bus_write(32, &buff->st_atim, tmp.st_atime);
			bus_write(32, &buff->st_mtim, tmp.st_mtime);
			bus_write(32, &buff->st_ctim, tmp.st_ctime);
		
			core->gpr[3] = 0;
			dump("syscall %d fstat64(%d, 0x%x) = %d\n", TARGET_NR_fstat64, fd, buff, core->gpr[3]);
			break;
		}
		case TARGET_NR_exit_group:{		/* 234 */ 
			int status = core->gpr[3];
			dump("syscall %d exit_group(%d)\n",TARGET_NR_exit_group, status);
			ppc_dyncom_stop(core);
			run_command("quit");
			break;
		}
		default:
			printf("In %s, syscall number is %d, not implemented.\n", __FUNCTION__, syscall_number);
			exit(-1);
	}
	return 0;
}
