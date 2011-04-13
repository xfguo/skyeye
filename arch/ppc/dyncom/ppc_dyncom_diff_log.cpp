#include<stdlib.h>
#include<stdio.h>

static FILE *fp;
static char* file_name = "pc_reg.log";
static int flag = 0;
static unsigned int last_gpr[32];
#define LAST_PC_NUM 10
static unsigned int pc_trace[LAST_PC_NUM] = {0};

static void print_pc_trace(){
	int i = LAST_PC_NUM - 1;
	for(; i >= 0 ; i--)
		printf("[pc trace : %d]\t0x%x\n", i, pc_trace[i]);
}
static int diff_log_over_flag = 0;
static int is_diff_log_over(){
	if(feof(fp)){
		if(diff_log_over_flag == 0){
			printf("##diff log over!\n");
			diff_log_over_flag = 1;
		}
		return 1;
	}else
		return 0;
}
void ppc_dyncom_diff_log(const unsigned int pc,
		const unsigned int lr,
		const unsigned int cr,
		const unsigned int ctr,
		const unsigned int reg[]){
	int i;
	if(flag == 0){
		if(!(fp = fopen(file_name, "r"))){
			printf("failed open file %s\n", file_name);
			exit(0);
		}
		printf("ppc dyncom diff log!\n");
		flag = 1;
		for(i = 0; i < 32; i++)
			last_gpr[i] = reg[i];
	}
	unsigned int pc_log = 0;
	unsigned int lr_log = 0;
	unsigned int cr_log = 0;
	unsigned int ctr_log = 0;
	/* pc */
	if(is_diff_log_over())
		goto over;
	fscanf(fp, "%x", &pc_log);
	if(pc_log != pc){
		fprintf(stderr, "\n\nIn %s:pc 0x%x error occured,should be 0x%x\n\n", __func__, pc, pc_log);
		goto exit;
	}
	/* gpr */
	for(i = 0; i < 32; i ++){
		if(is_diff_log_over())
			goto over;
		if(last_gpr[i] != reg[i]){
			fscanf(fp, "%x", &last_gpr[i]);
			fscanf(fp, "%x", &last_gpr[i]);
			if(last_gpr[i] != reg[i]){
				print_pc_trace();
				fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
				fprintf(stderr, "In %s:register gpr[%d]=0x%x error occured,should be 0x%x\n\n", __func__, i, reg[i], last_gpr[i]);
				goto exit;
			}
		}
	}
#if 0
	/* lr,cr,ctr */
	if(is_diff_log_over())
		goto over;
	fscanf(fp, "%x", &lr_log);
	fscanf(fp, "%x", &cr_log);
	fscanf(fp, "%x", &ctr_log);
	if(lr != lr_log){
		print_pc_trace();
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:lr error 0x%x should be 0x%x\n", __func__, lr, lr_log);
		goto exit;
	}
	if(cr != cr_log){
		print_pc_trace();
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:cr error 0x%x should be 0x%x\n", __func__, cr, cr_log);
		goto exit;
	}
	if(ctr != ctr_log){
		print_pc_trace();
		fprintf(stderr, "pc=0x%x,NOTE:error pc is the last one! (pc-4) if no jump...\n", pc);
		fprintf(stderr, "In %s:ctr error 0x%x should be 0x%x\n", __func__, ctr, ctr_log);
		goto exit;
	}
#endif
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
