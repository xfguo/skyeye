#ifndef __PPC_DYNCOM_RUN_H__
#define __PPC_DYNCOM_RUN_H__

#include <skyeye_types.h>
#include <skyeye_dyncom.h>

#include "ppc_dyncom.h"
#ifdef __cplusplus
 extern "C" {
#endif

void ppc_dyncom_stop(e500_core_t* core);
exception_t init_ppc_dyncom ();
void ppc_dyncom_run(cpu_t* cpu);

void ppc_dyncom_init(e500_core_t* core);

e500_core_t* get_core_from_dyncom_cpu(cpu_t* cpu);
void arch_ppc_dyncom_exception(cpu_t *cpu, BasicBlock *bb, Value *cond, uint32 type, uint32 flags, uint32 a);
void arch_ppc_dyncom_effective_to_physical(cpu_t *cpu, BasicBlock *bb, int flag);
void arch_ppc_dyncom_mmu_set_sdr1(cpu_t *cpu, BasicBlock *bb, uint32_t rS);
#ifdef __cplusplus
}
#endif

#endif
