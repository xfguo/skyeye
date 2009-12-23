#ifndef __ARCH_REGDEFS_H__
#define __ARCH_REGDEFS_H__
#if 0
struct register_defs{
	char * name;
	int (*register_raw_size)(int x);
	int register_bytes;
	int (*register_byte)(int x);
	int num_regs;
	int max_register_raw_size;
	int endian_flag;
	int pc_regnum;
	int sp_regnum;
	int fp_regnum;
	int (*store_register)(int rn, unsigned char * memory);
	int (*fetch_register)(int rn, unsigned char * memory); 	
};
typedef struct register_defs register_defs_t;
#endif
#endif
