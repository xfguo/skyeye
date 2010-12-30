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

ppc_opc_func_t ppc_opc_fdivsx_func;
ppc_opc_func_t ppc_opc_fsubsx_func;
ppc_opc_func_t ppc_opc_faddsx_func;
ppc_opc_func_t ppc_opc_fsqrtsx_func;
ppc_opc_func_t ppc_opc_fresx_func;
ppc_opc_func_t ppc_opc_fmulsx_func;
ppc_opc_func_t ppc_opc_fmsubsx_func;
ppc_opc_func_t ppc_opc_fmaddsx_func;
ppc_opc_func_t ppc_opc_fnmsubsx_func;
ppc_opc_func_t ppc_opc_fnmaddsx_func;
