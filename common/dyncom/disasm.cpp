/**
 * @file disasm.cpp
 *
 * Disassemble and print an instruction.
 *
 * @author OS Center,TsingHua University (Ported from libcpu)
 * @date 11/11/2010
 */

#include "skyeye_dyncom.h"
#include "dyncom/tag.h"
/**
 * @brief Disassemble and print an instruction
 *
 * @param cpu The CPU core structure
 * @param pc The disassemble instruction's pc
 */
void disasm_instr(cpu_t *cpu, addr_t pc) {
	char disassembly_line1[80];
	char disassembly_line2[80];
	int bytes, i;

	bytes = cpu->f.disasm_instr(cpu, pc, disassembly_line1, sizeof(disassembly_line1));

	LOG(".,%04llx ", (unsigned long long)pc);
	for (i=0; i<bytes; i++) {
		//LOG("%02X ", cpu->RAM[pc - 0x50000000 +i]);
//                LOG("%02X ", cpu->RAM[pc  +i]);
	}
	LOG("%*s", (18-3*bytes)+1, ""); /* TODO make this arch neutral */

	/* delay slot */
	tag_t tag;
	addr_t dummy, dummy2;
	cpu->f.tag_instr(cpu, pc, &tag, &dummy, &dummy2);
	if (tag & TAG_DELAY_SLOT)
		bytes = cpu->f.disasm_instr(cpu, pc + bytes, disassembly_line2, sizeof(disassembly_line2));

	if (tag & TAG_DELAY_SLOT)
		LOG("%-23s [%s]\n", disassembly_line1, disassembly_line2);
	else
		LOG("%-23s\n", disassembly_line1);

}
