#ifndef __MIPS_DYNCOM_INSTR_SPECIAL__
#define __MIPS_DYNCOM_INSTR_SPECIAL__
int opc_sll_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_srl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_sra_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_sllv_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_srlv_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_srav_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_movz_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_movn_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_jr_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_jalr_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_syscall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_break_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_sync_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_mfhi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_mthi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_mflo_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_mtlo_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_mult_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_multu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_div_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_divu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_add_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_sub_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_subu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_and_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_or_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_xor_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_nor_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_slt_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);

#endif
