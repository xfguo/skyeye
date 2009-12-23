#ifndef __SKYEYE_ARCH_H__
#define __SKYEYE_ARCH_H__
#include "skyeye_types.h"
#include "skyeye_config.h"

#ifdef __cplusplus
 extern "C" {
#endif
/*
 * a running instance for a specific archteciture.
 */
typedef struct generic_arch_s
{
	char* arch_name;
	void (*init) ();
	void (*reset) ();
	void (*step_once) ();
	void (*set_pc)(generic_address_t addr);
	generic_address_t (*get_pc)();
	uint32 (*get_step)();
	//chy 2004-04-15 
	//int (*ICE_write_byte) (generic_address_t addr, uint8_t v);
	//int (*ICE_read_byte)(generic_address_t addr, uint8_t *pv);
	uint32 (*get_regval_by_id)(int id);
	char* (*get_regname_by_id)(int id);
	exception_t (*set_regval_by_id)(int id, uint32 value);
	/*
	 * read a data by virtual address.
	 */
	exception_t (*mmu_read)(short size, generic_address_t addr, uint32_t * value);
	/*
	 * write a data by a virtual address.
	 */
	exception_t (*mmu_write)(short size, generic_address_t addr, uint32_t value);
	endian_t endianess;
} generic_arch_t;

/*
 * a running instance for a core.
 */
typedef struct generic_core_s
{
	char* core_name;
	void (*init) ();
	void (*reset) ();
	void (*step_once) ();
	int (*get_current_cycles)();
	int (*get_current_steps)();
	void (*set_pc)(generic_address_t addr);
	generic_address_t (*get_pc)();
	uint32 (*get_regval_by_id)(int id);
	exception_t (*set_register_by_id)(int id, uint32 value);
	endian_t endianess;
} generic_core_t;

/*
 * register a simulation module for an architecture.
 */
void register_arch(arch_config_t * arch);

#ifdef __cplusplus
}
#endif

#endif
