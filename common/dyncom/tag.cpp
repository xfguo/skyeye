/**
 * @file tag.cpp
 *
 * Do a depth search of all reachable code and associate
 * every reachable instruction with flags that indicate
 * instruction type (branch,call,ret, ...), flags
 * (conditional, ...) and code flow information (branch
 * target, ...)
 *
 * @author OS Center,TsingHua University (Ported from libcpu)
 * @date 11/11/2010
 */
#include "skyeye_dyncom.h"
#include "dyncom/tag.h"
#include "sha1.h"

/*
 * TODO: on architectures with constant instruction sizes,
 * this shouldn't waste extra tag data for every byte of
 * code memory, but have one tag per instruction location.
 */

#ifdef _WIN32
#define MAX_PATH 260
extern "C" __declspec(dllimport) uint32_t __stdcall GetTempPathA(uint32_t nBufferLength, char *lpBuffer);
#endif

static const char *
get_temp_dir()
{
#ifdef _WIN32
	static char pathname[MAX_PATH];
	if (GetTempPathA(sizeof(pathname), pathname))
		return pathname;
#endif
	return "/tmp/";
}
static bool
is_tag_level2_table_allocated(cpu_t *cpu, addr_t addr)
{
	uint32_t level1_offset = TAG_LEVEL1_OFFSET(addr);
	return cpu->dyncom_engine->tag_table[level1_offset];
}

static bool
is_tag_level3_table_allocated(cpu_t *cpu, addr_t addr)
{
	uint32_t level1_offset = TAG_LEVEL1_OFFSET(addr);
	uint32_t level2_offset = TAG_LEVEL2_OFFSET(addr);
	return cpu->dyncom_engine->tag_table[level1_offset][level2_offset];
}

/**
 * @brief initialize tag level2 table
 *
 * @param cpu CPU core structure
 * @param addr address to tag
 */
static void
init_tag_level2_table(cpu_t *cpu, addr_t addr)
{
	tag_t **tag = (tag_t**)malloc(TAG_LEVEL2_TABLE_SIZE * sizeof(tag_t *));
	memset(tag, 0, TAG_LEVEL2_TABLE_SIZE * sizeof(tag_t *));

	uint32_t level1_offset = TAG_LEVEL1_OFFSET(addr);
	cpu->dyncom_engine->tag_table[level1_offset] = tag;
}

/**
 * @brief initialize tag level3 table
 *
 * @param cpu CPU core structure
 * @param addr address to tag
 */
static void
init_tag_level3_table(cpu_t *cpu, addr_t addr)
{
	addr_t nitems, i;

	nitems = TAG_LEVEL3_TABLE_SIZE;

	cpu->dyncom_engine->tag = (tag_t*)malloc(nitems * sizeof(tag_t));
	for (i = 0; i < nitems; i++)
		cpu->dyncom_engine->tag[i] = TAG_UNKNOWN;

	uint32_t level1_offset = TAG_LEVEL1_OFFSET(addr);
	uint32_t level2_offset = TAG_LEVEL2_OFFSET(addr);
	cpu->dyncom_engine->tag_table[level1_offset][level2_offset] = cpu->dyncom_engine->tag;

		#if 0
	if (!(cpu->flags_codegen & CPU_CODEGEN_TAG_LIMIT)) {
		/* calculate hash of code */
		SHA1_CTX ctx;
		SHA1Init(&ctx);
		//SHA1Update(&ctx, &cpu->RAM[cpu->code_start - 0x50000000], cpu->code_end - cpu->code_start);

		int32_t offset;
               printf("cpu->code_start : %x\n", cpu->code_start);
               if (cpu->code_start > 0x60000000) {
                       offset = cpu->code_start - 0x60000000;
               } else
                       offset = cpu->code_start - 0x50000000;

               SHA1Update(&ctx, &cpu->RAM[offset], cpu->code_end -cpu->code_start);


		SHA1Final(cpu->code_digest, &ctx);
		char ascii_digest[256];
		char cache_fn[256];
		ascii_digest[0] = 0;
		int j; 
		for (j=0; j<20; j++)
			sprintf(ascii_digest+strlen(ascii_digest), "%02x", cpu->code_digest[j]);
		LOG("Code Digest: %s\n", ascii_digest);
		sprintf(cache_fn, "%slibcpu-%s.entries", get_temp_dir(), ascii_digest);
		
		cpu->file_entries = NULL;
		FILE *f;
		if ((f = fopen(cache_fn, "r"))) {
			LOG("info: entry cache found.\n");
			while(!feof(f)) {
				addr_t entry = 0;
				for (i = 0; i < 4; i++) {
					entry |= fgetc(f) << (i*8);
				}
				tag_start(cpu, entry);
			}
			fclose(f);
		} else {
			LOG("info: entry cache NOT found.\n");
		}
		
		if (!(cpu->file_entries = fopen(cache_fn, "a"))) {
			printf("error appending to cache file!\n");
			exit(1);
		}
		#endif
//	}
}

/**
 * @brief check integrity of tag array memory. Allocate memory on demand.
 *
 * @param cpu CPU core structure
 * @param addr address to tag
 */
static void
check_tag_memory_integrity(cpu_t *cpu, addr_t addr)
{
	if (!is_tag_level2_table_allocated(cpu, addr)) {
		init_tag_level2_table(cpu, addr);
	}
	if (!is_tag_level3_table_allocated(cpu, addr)) {
		init_tag_level3_table(cpu, addr);
	}
}

/**
 * @brief Determine an address is in code area or not
 *
 * @param cpu CPU core structure
 * @param a address
 *
 * @return true if in code area,false otherwise.
 */
bool
is_inside_code_area(cpu_t *cpu, addr_t a)
{
	return a >= cpu->dyncom_engine->code_start && a < cpu->dyncom_engine->code_end;
}
/**
 * @brief Give a tag to an address
 *
 * @param cpu CPU core structure
 * @param a address to be tagged
 * @param t tag
 */
void
or_tag(cpu_t *cpu, addr_t a, tag_t t)
{
	/* NEW_PC_NONE is not a real address. Some branch/call address could not be known at translate-time*/
	if (a == NEW_PC_NONE) {
		return;
	}
	check_tag_memory_integrity(cpu, a);
	uint32_t level1_offset = TAG_LEVEL1_OFFSET(a);
	uint32_t level2_offset = TAG_LEVEL2_OFFSET(a);
	uint32_t level3_offset = TAG_LEVEL3_OFFSET(a);
	cpu->dyncom_engine->tag_table[level1_offset][level2_offset][level3_offset] |= t;
}
void
xor_tag(cpu_t *cpu, addr_t a, tag_t t)
{
	/* NEW_PC_NONE is not a real address. Some branch/call address could not be known at translate-time*/
	if (a == NEW_PC_NONE) {
		return;
	}
	check_tag_memory_integrity(cpu, a);
	uint32_t level1_offset = TAG_LEVEL1_OFFSET(a);
	uint32_t level2_offset = TAG_LEVEL2_OFFSET(a);
	uint32_t level3_offset = TAG_LEVEL3_OFFSET(a);
	cpu->dyncom_engine->tag_table[level1_offset][level2_offset][level3_offset] &= ~t;
}
void clear_tag(cpu_t *cpu)
{
	addr_t nitems, i;

	nitems = cpu->dyncom_engine->code_end - cpu->dyncom_engine->code_start;
	//nitems = cpu->dyncom_engine->tag_end - cpu->dyncom_engine->tag_start + 1;
	//uint32_t offset = arch_xtensa_address_to_offset(cpu->dyncom_engine->tag_start);
	for (i = 0; i < nitems; i++)
		cpu->dyncom_engine->tag[i] = TAG_UNKNOWN;
}

/* access functions */
/**
 * @brief Get the tag of an address 
 *
 * @param cpu CPU core structure
 * @param a address
 *
 * @return tag of the address
 */
tag_t
get_tag(cpu_t *cpu, addr_t a)
{
	/* NEW_PC_NONE is not a real address. Some branch/call address could not be known at translate-time*/
	if (a == NEW_PC_NONE) {
		return TAG_UNKNOWN;
	}
	check_tag_memory_integrity(cpu, a);
	uint32_t level1_offset = TAG_LEVEL1_OFFSET(a);
	uint32_t level2_offset = TAG_LEVEL2_OFFSET(a);
	uint32_t level3_offset = TAG_LEVEL3_OFFSET(a);
	return cpu->dyncom_engine->tag_table[level1_offset][level2_offset][level3_offset];
}
/**
 * @brief Determine an address is code or not
 *
 * @param cpu CPU core structure
 * @param a address
 *
 * @return true if is code,false otherwise
 */
bool
is_code(cpu_t *cpu, addr_t a)
{
	return !!(get_tag(cpu, a) & TAG_CODE);
}
/**
 * @brief Determine an address is translated or not
 *
 * @param cpu CPU core structure
 * @param a address
 *
 * @return true if is translated,false otherwise
 */
bool
is_translated(cpu_t *cpu, addr_t a)
{
	return (get_tag(cpu, a) & TAG_TRANSLATED);
}

extern void disasm_instr(cpu_t *cpu, addr_t pc);

static void
tag_recursive(cpu_t *cpu, addr_t pc, int level)
{
	int bytes;
	tag_t tag;
	addr_t new_pc, next_pc;

	if ((cpu->dyncom_engine->flags_codegen & CPU_CODEGEN_TAG_LIMIT)
	    && (level == LIMIT_TAGGING_DFS))
		return;
	if ((cpu->dyncom_engine->flags_codegen & CPU_CODEGEN_TAG_LIMIT)
	    && (level == 0))
	{
		LOG("tag start at %x\n", pc);
		/* save tag start address */
		cpu->dyncom_engine->tag_start = pc;
	}
	for(;;) {
		if (!is_inside_code_area(cpu, pc))
		{
				
			LOG("In %s pc = %x start = %x end = %x\n", __FUNCTION__, pc, cpu->dyncom_engine->code_start, cpu->dyncom_engine->code_end);
			return;
		}
//		if (is_code(cpu, pc))	/* we have already been here, ignore */
//			return;

		if (LOGGING) {
			LOG("%*s", level, "");
//			disasm_instr(cpu, pc);
		}


		bytes = cpu->f.tag_instr(cpu, pc, &tag, &new_pc, &next_pc);

		or_tag(cpu, pc, tag | TAG_CODE);

		if (tag & (TAG_CONDITIONAL))
			or_tag(cpu, next_pc, TAG_AFTER_COND);

		if (tag & TAG_TRAP)	{
			/* regular trap - no code after it */
			if (!(cpu->dyncom_engine->flags_hint & (CPU_HINT_TRAP_RETURNS | CPU_HINT_TRAP_RETURNS_TWICE)))
				//return;
				break;
			/*
			 * client hints that a trap will likely return,
			 * so tag code after it (optimization for usermode
			 * code that makes syscalls)
			 */
			or_tag(cpu, next_pc, TAG_AFTER_TRAP);
			/*
			 * client hints that a trap will likely return
			 * - to the next instruction AND
			 * - to the instruction after that
			 * OpenBSD on M88K skips an instruction on a trap
			 * return if there was an error.
			 */
			if (cpu->dyncom_engine->flags_hint & CPU_HINT_TRAP_RETURNS_TWICE) {
				tag_t dummy1;
				addr_t next_pc2, dummy2;
				next_pc2 = next_pc + cpu->f.tag_instr(cpu, next_pc, &dummy1, &dummy2, &dummy2);
				or_tag(cpu, next_pc2, TAG_AFTER_TRAP);
				tag_recursive(cpu, next_pc2, level+1);
			}
		}

		if (tag & TAG_CALL) {
			/* tag subroutine, then continue with next instruction */
			or_tag(cpu, new_pc, TAG_SUBROUTINE);
			or_tag(cpu, next_pc, TAG_AFTER_CALL);
			tag_recursive(cpu, new_pc, level+1);
		}

		if (tag & (TAG_BRANCH)) {
			or_tag(cpu, new_pc, TAG_BRANCH_TARGET);
			tag_recursive(cpu, new_pc, level+1);
			if (!(tag & (TAG_CONDITIONAL)))
				//return;
				break;
		}

		if (tag & TAG_WINDOWCHECK) {
			or_tag(cpu, next_pc, TAG_AFTER_WNDCHK);
		}

		if (is_translated(cpu, next_pc)) {
			or_tag(cpu, pc, tag | TAG_STOP | TAG_LAST_INST);
			//return;
		}

		if (tag & (TAG_RET | TAG_STOP))	/* execution ends here, the follwing location is not reached */
			//return;
			break;

		pc = next_pc;
		/* save tag end address */
		cpu->dyncom_engine->tag_end = pc;
	}
	cpu->dyncom_engine->tag_end = pc;
	LOG("tag end at %x\n", pc);
}
/**
 * @brief Start tag from current pc.
 *
 * @param cpu CPU core structure
 * @param pc current address start tagging
 */
void
tag_start(cpu_t *cpu, addr_t pc)
{
	cpu->dyncom_engine->tags_dirty = true;

	/* for singlestep, we don't need this */
	if (cpu->dyncom_engine->flags_debug & (CPU_DEBUG_SINGLESTEP | CPU_DEBUG_SINGLESTEP_BB))
		return;

	/* initialize data structure on demand */
	check_tag_memory_integrity(cpu, pc);

	LOG("starting tagging at $%02llx\n", (unsigned long long)pc);

	if (!(cpu->dyncom_engine->flags_codegen & CPU_CODEGEN_TAG_LIMIT)) {
		int i;
		if (cpu->dyncom_engine->file_entries) {
			for (i = 0; i < 4; i++)
				fputc((pc >> (i*8))&0xFF, cpu->dyncom_engine->file_entries);
			fflush(cpu->dyncom_engine->file_entries);
		}
	}

	or_tag(cpu, pc, TAG_ENTRY); /* client wants to enter the guest code here */

	tag_recursive(cpu, pc, 0);
}
