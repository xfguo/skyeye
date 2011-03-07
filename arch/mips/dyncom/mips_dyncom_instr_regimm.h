#ifndef __MIPS_DYNCOM_INSTR_REGIMM__
#define __MIPS_DYNCOM_INSTR_REGIMM__
int opc_bltz_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bgez_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bltzl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bgezl_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_tgei_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_tgeiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_tlti_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_tltiu_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_teqi_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_tnei_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bltzal_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bgezal_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bltzall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
int opc_bgezall_trans(cpu_t *cpu, uint32_t instr, BasicBlock *bb);
#endif
