#include<stdlib.h>
#include<stdio.h>

static FILE *fp;
static char* file_name = "pc_reg.log";
static int flag = 0;
void ppc_dyncom_diff_log(const unsigned int pc,
		const unsigned int lr,
		const unsigned int cr,
		const unsigned int ctr,
		const unsigned int reg[]){
	if(flag == 0){
		if(!(fp = fopen(file_name, "r"))){
			printf("failed open file %s\n", file_name);
			exit(0);
		}
		printf("ppc dyncom diff log!\n");
		flag = 1;
	}
	int i;
	unsigned int pc_log = 0;
	unsigned int lr_log = 0;
	unsigned int cr_log = 0;
	unsigned int ctr_log = 0;
	unsigned int reg_log[32] = {0};
	/* pc */
	fscanf(fp, "%x", &pc_log);
	if(pc_log != pc){
		fprintf(stderr, "\n\nIn %s:pc 0x%x error occured,should be 0x%x\n\n", __func__, pc, pc_log);
		exit(0);
	}
	/* gpr */
	for(i = 0; i < 32; i ++){
		fscanf(fp, "%x", &reg_log[i]);
		if(reg_log[i] != reg[i]){
			fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
			fprintf(stderr, "In %s:register gpr[%d]=0x%x error occured,should be 0x%x\n\n", __func__, i, reg[i], reg_log[i]);
			exit(0);
		}
	}
	/* lr,cr,ctr */
	fscanf(fp, "%x", &lr_log);
	fscanf(fp, "%x", &cr_log);
	fscanf(fp, "%x", &ctr_log);
	if(lr != lr_log){
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:lr error 0x%x should be 0x%x\n", __func__, lr, lr_log);
		exit(0);
	}
	if(cr != cr_log){
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:cr error 0x%x should be 0x%x\n", __func__, cr, cr_log);
		exit(0);
	}
	if(ctr != ctr_log){
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:ctr error 0x%x should be 0x%x\n", __func__, ctr, ctr_log);
		exit(0);
	}
}
