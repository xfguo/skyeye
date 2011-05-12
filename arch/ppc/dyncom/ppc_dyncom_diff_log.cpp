#include<stdlib.h>
#include<stdio.h>
#include "ppc_dyncom.h"

static FILE *fp;
static char* file_name = "pc_reg.log";
static int flag = 0;
static unsigned int last_gpr[32];
static unsigned int last_spr[PPC_DYNCOM_MAX_SPR_REGNUM];
#define LAST_PC_NUM 10
static unsigned int pc_trace[LAST_PC_NUM] = {0};

static void print_pc_trace(){
	int i = LAST_PC_NUM - 1;
	for(; i >= 0 ; i--)
		printf("[pc trace : %d]\t0x%x\n", i, pc_trace[i]);
}
static int is_diff_log_over(){
	if(feof(fp)){
		printf("### END diff log over! ###\n");
		return 1;
	}else
		return 0;
}
void ppc_dyncom_diff_log(const unsigned long long icount,
		const unsigned int pc,
		const unsigned int gpr[],
		const unsigned int spr[]){
	int i;
	if(flag == 0){
		if(!(fp = fopen(file_name, "r"))){
			printf("failed open file %s\n", file_name);
			exit(0);
		}
		printf("### START ppc dyncom diff log! icount = %d ###\n", icount);
		flag = 1;
		for(i = 0; i < 32; i++)
			last_gpr[i] = gpr[i];
		for(i = 0; i < PPC_DYNCOM_MAX_SPR_REGNUM; i++){
			if(i >= PC_REGNUM && i <= ICOUNT_REGNUM)
				continue;
			if(i == CURRENT_OPC_REGNUM)
				continue;
			if(i == DEC_REGNUM)
				continue;
			if(i == TBL_REGNUM)
				continue;
			last_spr[i] = spr[i];
		}
	}
	unsigned int pc_log = 0;
	unsigned int tmp = 0;
	char str[20];
	/* pc */
	if(is_diff_log_over())
		goto over;
	fscanf(fp, "%x", &pc_log);
	fscanf(fp, "%x", &tmp);
	if(pc_log != pc){
		print_pc_trace();
		fprintf(stderr, "\n\nIn %s:pc 0x%x error occured,should be 0x%x,icount=%d\n\n",
				__func__, pc, pc_log, icount);
		goto exit;
	}
	/*
	fscanf(fp, "%x", &xer_log);
	if(xer_log != xer){
		print_pc_trace();
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:xer=0x%x error occured,should be 0x%x\n\n", __func__, xer, xer_log);
		goto exit;
	}
	*/
	/* gpr */
	for(i = 0; i < 32; i ++){
		if(is_diff_log_over())
			goto over;
		if(last_gpr[i] != gpr[i]){
			fscanf(fp, "%s", str);
			fscanf(fp, "%x", &last_gpr[i]);
			if(last_gpr[i] != gpr[i]){
				print_pc_trace();
				fprintf(stderr, "pc=0x%x,icount=%d,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc, icount);
				fprintf(stderr, "In %s:register gpr[%d]=0x%x error occured,should be 0x%x\n\n",
						__func__, i, gpr[i], last_gpr[i]);
				goto exit;
			}
		}
	}
	/* spr */
	for(i = 0; i < PPC_DYNCOM_MAX_SPR_REGNUM; i ++){
		if(is_diff_log_over())
			goto over;
		if(i >= PC_REGNUM && i <= ICOUNT_REGNUM)
			continue;
		if(i == CURRENT_OPC_REGNUM)
			continue;
		if(i == DEC_REGNUM)
			continue;
		if(i == TBL_REGNUM)
			continue;

		if(last_spr[i] != spr[i]){
			fscanf(fp, "%s", str);
			fscanf(fp, "%x", &last_spr[i]);
			if(last_spr[i] != spr[i]){
				print_pc_trace();
				fprintf(stderr, "pc=0x%x,icount=%d,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc, icount);
				fprintf(stderr, "In %s:register spr[%d]=0x%x error occured,should be 0x%x\n\n",
						__func__, i, spr[i], last_spr[i]);
				goto exit;
			}
		}
	}

	i = LAST_PC_NUM - 1;
	for(; i > 0; i--)
		pc_trace[i] = pc_trace[i - 1];
	pc_trace[0] = pc;
	return;
exit:
	exit(0);
over:
	return;
}
