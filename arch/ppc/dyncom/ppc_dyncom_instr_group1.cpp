#include "debug.h"
#include "tracers.h"
#include "ppc_dyncom_dec.h"
#include "ppc_exc.h"
#include "ppc_cpu.h"
#include "ppc_dyncom_alu.h"
#include "ppc_dyncom_run.h"
#include "ppc_tools.h"
#include "ppc_mmu.h"

#include "llvm/Instructions.h"
#include <dyncom/dyncom_llvm.h>
#include <dyncom/frontend.h>
#include "dyncom/basicblock.h"
#include "skyeye.h"

#include "ppc_dyncom_debug.h"

ppc_opc_func_t ppc_opc_crnor_func;
ppc_opc_func_t ppc_opc_crandc_func;
ppc_opc_func_t ppc_opc_crxor_func;
ppc_opc_func_t ppc_opc_crnand_func;
ppc_opc_func_t ppc_opc_crand_func;
ppc_opc_func_t ppc_opc_creqv_func;
ppc_opc_func_t ppc_opc_crorc_func;
ppc_opc_func_t ppc_opc_cror_func;
ppc_opc_func_t ppc_opc_bcctrx_func; 
ppc_opc_func_t ppc_opc_bclrx_func;
ppc_opc_func_t ppc_opc_mcrf_func;
ppc_opc_func_t ppc_opc_rfi_func;
ppc_opc_func_t ppc_opc_isync_func;
