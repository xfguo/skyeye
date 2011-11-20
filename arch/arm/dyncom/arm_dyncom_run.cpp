/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file arm_dyncom_run.cpp
* @brief The dyncom run implementation for arm
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-11-20
*/

#include <llvm/LLVMContext.h>
#include <llvm/Type.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Constant.h>
#include <llvm/Constants.h>
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Instructions.h"

#include <skyeye_dyncom.h>
#include <skyeye_types.h>
#include <skyeye_obj.h>
#include <skyeye.h>
#include <bank_defs.h>
#include <skyeye_pref.h>
#include <skyeye_symbol.h>
#include <dyncom/dyncom_llvm.h>
#include <skyeye_log.h>
#include <dyncom/tag.h>

#include <vector>

#include "arm_regformat.h"
#include <skyeye_ram.h>

#include "armdefs.h"
#include "memory.h"
#include "dyncom/memory.h"
#include "dyncom/frontend.h"
#include "arm_dyncom_translate.h"
#include "arm_dyncom_parallel.h"
#include "dyncom/defines.h"
#include "common/mmu/arm1176jzf_s_mmu.h"
#include "armmmu.h"

#include "dyncom/arm_dyncom_mmu.h"

#define LOG_IN_CLR	skyeye_printf_in_color

void arm_switch_mode(cpu_t *cpu);
//#define MAX_REGNUM 16
extern const char* arm_regstr[MAX_REG_NUM];

enum{
	ARM_DYNCOM_CALLOUT_UNDEF = 2,
	ARM_DYNCOM_MAX_CALLOUT
};

extern "C" {
extern void io_do_cycle (void * state);
}

uint32_t get_end_of_page(uint32 phys_addr){
	const uint32 page_size = 4 * 1024;
	return (phys_addr + page_size) & (~(page_size - 1));
}

void cpu_set_flags_codegen(cpu_t *cpu, uint32_t f)
{
        cpu->dyncom_engine->flags_codegen = f;
}

static uint32_t int_icounter[] = {0x707865,0x707d4e,0x70eb3f,0x70ed84,0x72fce1,0x73026f,0x731726,0x733a75,0x746922,0x7661f4,0x785ac6,0x7a5398,0x7c4c6a,0x7e453c,0x803e0e,0x8236e0,
0x842fb2,0x862884,0x8833aa,0x88390a,0x885256,0x899a88,0x8a1a28,0x8b578a,0x8b6e13,0x8b9783,0x8bd41b,0x8c0e08,0x8c12fa,0x8c2801,0x8e0bcc,0x90049e,
0x91fd70,0x93f642,0x95ef14,0x97e1f3,0x97e7e6,0x99e0b8,0x9bd996,0x9dd25c,0x9fcb2e,0xa1c400,0xa3bcd2,0xa5b5cc,0xa66b6a,0xa77da5,0xa7ae76,0xa83aca,
0xa84f21,0xa9a748,0xaba01a,0xabc2d1,0xad9add,0xaf9222,0xb18a90,0xb38362,0xb57c34,0xb776e4,0xb96dd8,0xbb66aa,0xbd5f99,0xbf5853,0xc15120,0xc34aed,
0xc542c4,0xc73b96,0xc9347a,0xc9ee4b,0xcb30cb,0xcd2644,0xcf2b5a,0xd117b0,0xd31082,0xd50954,0xd70226,0xd8faf8,0xdaf3ca,0xdcecc2,0xdee928,0xe0de40,
0xe2d712,0xe4cfe4,0xe6c8b6,0xe8c211,0xeaba5a,0xecb32c,0xeeabfe,0xf0a4d0,0xf29da2,0xf49674,0xf68f46,0xf88818,0xfa80ea,0xfc79bc,0xfe728e,0x1006b60,
0x1026432,0x1045d04,0x1065607,0x1084ea8,0x10a477a,0x10c404c,0x10e391e,0x10fc235,0x11031f0,0x110ff8c,0x1122ac2,0x1142394,0x1160b46,0x1161c66,
0x1173a73,0x1181538,0x11932d2,0x11941cb,0x119666b,0x119ccc4,0x11a0e43,0x11c06dc,0x11c2228,0x11d1254,0x11dffae,0x11e7ebc,0x11ff880,0x121f152,
0x123ea24,0x125e2f6,0x127dbc8,0x129d49a,0x12bcd6c,0x12dc63e,0x12fc53c,0x131b7e2,0x133b0b4,0x135a986,0x137a258,0x1399b2a,0x13b93fc,0x13d8cce,
0x13f85a0,0x1417e72,0x1437744,0x1457016,0x14768e8,0x1496b63,0x14b5a8c,0x14d535e,0x14f4c30,0x1514502,0x1533dd4,0x15536a6,0x1572f78,0x15928a7,
0x15b211c,0x15d19f1,0x15f12c0,0x1610b92,0x1630cfc,0x164fd36,0x166f608,0x168eeda,0x16ae7ac,0x16ce07e,0x16ed953,0x170d222,0x172caf4,0x174c3c6,
0x1763232,0x176bc98,0x178b56a,0x17bb619,0x17ca70e,0x17e9fe0,0x18098b2,0x1829184,0x1848a56,0x1868328,0x1887bfa,0x18a74d1,0x18c6dc6,0x18e6670,
0x1905f6d,0x1925814,0x194513d,0x19649b8,0x198428a,0x19a3b5c,0x19c3603,0x19e2d00,0x1a025d2,0x1a21ea4,0x1a418be,0x1a61048,0x1a8091a,0x1aa01ec,
0x1abfabe,0x1adf390,0x1afed41,0x1b1e534,0x1b3de06,0x1b5d6d8,0x1b7cfad,0x1b9c87c,0x1bbc14e,0x1bdba20,0x1bfb2f2,0x1c1abc4,0x1c3a496,0x1c59d68,
0x1c7963a,0x1c98f0c,0x1cb87de,0x1cd80f1,0x1cf7982,0x1d17254,0x1d36b26,0x1d563f8,0x1d75cca,0x1d9559c,0x1db4e6e,0x1dbd00f,0x1dd4740,0x1ddbd00,
0x1ddd316,0x1de3b98,0x1de482f,0x1df4012,0x1e138e4,0x1e331b6,0x1e52a88,0x1e7235a,0x1e91c2c,0x1eb14fe,0x1ed0dd0,0x1ef06a2,0x1f0ff74,0x1f2f846,
0x1f4f118,0x1f6e9ea,0x1f8e2bc,0x1fadb8e,0x1fcd460,0x1fecd32,0x200c604,0x202bed6,0x204b7a8,0x206b07a,0x208a94c,0x20aa21e,0x20c9af0,0x20e93c2,
0x2108c94,0x2128566,0x2147e38,0x216770a,0x2186fdc,0x21a68ae,0x21c6180,0x21e5a52,0x2205324,0x2224bf6,0x22444c8,0x2263d9a,0x228366c,0x22a2f3e,
0x22c2810,0x22e20e2,0x23019b4,0x2321286,0x2340b58,0x236042a,0x237fcfc,0x239f5ce,0x23beea0,0x23de772,0x23fe044,0x241d916,0x243d1e8,0x245caba,
0x247c38c,0x249bc5e,0x24bb530,0x24dae02,0x24fa6d4,0x2519fa6,0x2539878,0x255914a,0x2578a1c,0x25982ee,0x25b7bc0,0x25d7492,0x25f6d64,0x2616636,
0x2635f08,0x263bfcf,0x26557da,0x26750ac,0x269497e,0x26b4250,0x26d3b22,0x26f33f4,0x2712cc6,0x2732598,0x2751e6a,0x277173c,0x279100e,0x27b08e0,
0x27d01b2,0x27efa84,0x280f356,0x282ec28,0x284e4fa,0x286ddcc,0x288d69e,0x28acf70,0x28cc842,0x28ec114,0x290b9e6,0x292b2b8,
0x294ab8a,0x296a45c,0x2989d2e,0x29a9600,0x29c8ed2,0x29e87a4,0x2a08076,0x2a27948,0x2a4721a,0x2a66aec,0x2a863be,0x2a93077,
0x2a94226,0x2aa5c90,0x2ac5562,0x2ae4e34,0x2b04706,0x2b23fd8,0x2b438aa,0x2b6317c,0x2b82a4e,0x2ba2320,0x2bc1bf2,0x2be14c4,
0x2c00d96,0x2c20668,0x2c3ff3a,0x2c5f80c,0x2c7f0de,0x2c9e9b0,0x2cbe282,0x2cddb54,0x2cfd426,0x2d1ccf8,0x2d3a0de,0x2d3b999,
0x2d3c5ca,0x2d5d5ae,0x2d5dc70,0x2d7b76e,0x2d9b040,0x2dba912,0x2dd2662,0x2dda1e4,0x2ded1cd,0x2df9c1e,0x2e08b11,0x2e19388,
0x2e2432b,0x2e38c61,0x2e5852c,0x2e77dfe,0x2e976ee,0x2eb6fa2,0x2ed6874,0x2ef6146,0x2f15a18,0x2f352ea,0x2f54bbc,0x2f7448e,
0x2f93d60,0x2fb3632,0x2fd2f04,0x2ff27d6,0x30120a8,0x303197a,0x305124c,0x3070b1e,0x309048d,0x30afcc2,0x30cf594,0x30eee66,
0x310e738,0x312e00a,0x3147a7f,0x314d8dd,0x316d1ae,0x318ca80,0x31ac352,0x31cbc24,0x31eb4f6,0x320adc8,0x322a69a,0x3249f6c,
0x326983e,0x3289110,0x32a89e2,0x32c84cf,0x32d7602,0x32e7ded,0x3305cb9,0x3307458,0x3326d2a,0x3333eb4,0x3335cb8,0x33465fc,
0x3365ece,0x33857a0,0x3391135,0x33a507d,0x33c4944,0x33e4216,0x33ed31f,0x3403ae8,0x34233ba,0x3442c8c,0x346255e,0x34703ff,
0x3481e30,0x34924a7,0x34a1702,0x34b2fa0,0x34c11c5,0x34e08a6,0x34e67d3,0x34ebdca,0x34ed9cc,0x35001d9,0x351316e,0x351687e,
0x3518fde,0x351fa4a,0x353f31c,0x355ebee,0x357e4c0,0x3588306,0x35899d8,
0x358b053,0x358b953,0x359dd92,0x35bd664,0x35c3814,0x35ca955,0x35cb2b8,0x35d471a,0x35d69f5,0x35d9292,0x35dcf36,
0x35ea4ea,0x35f9aae,0x35fc808,0x3609f25,0x3619148,0x361c126,0x3629db9,0x3638fdc,0x363b9ac,0x364a104,0x365a244,
0x365b27e,0x3669f74,0x367953c,0x367ab50,0x3688ec5,0x369933a,0x369a422,0x36a8cc9,0x36b82d9,0x36b9cf4,0x36c8b7f,
0x36d8ac8,0x36d95c6,0x36e8a04,0x36f7d67,0x36f8e98,0x3708aa6,0x3717e09,0x371876a,0x3727d0d,0x3737f8d,0x373803c,
0x3747dd3,0x37574ff,0x375790e,0x3766fce,0x3777583,0x3777bab,0x3787048,0x3796750,0x3796ab2,0x37a713c,0x37b6384,
0x37b6f51,0x37c6761,0x37d5c56,0x37d6ae7,0x37e71af,0x37f5528,0x37f6bff,0x3806380,0x3814dfa,0x3816da0,0x38264ec,
0x38349ad,0x38364fe,0x38458d0,0x3853f9e,0x3856579,0x38658dc,0x3873870,0x38756f6,0x3885976,0x3893142,0x389574a,
0x38a4e9a,0x38b2a14,0x38b5d66,0x38c633f,0x38d22e6,0x38d66c6,0x38e5e44,0x38f1bb8,0x38f67b2,0x3905e92,0x391148a,
0x3915d42,0x39250a5,0x3930d5c,0x3935d48,0x39450ab,0x3950672,0x3954ea7,0x3965127,0x396ff00,0x3974f19,0x3984621,
0x398f843,0x3994078,0x39a4699,0x39af0a4,0x39b40ea,0x39c37f2,0x39cec30,0x39d4a96,0x39e41ec,0x39ee248,0x39f3fe2,
0x3a03345,0x3a0db1a,0x3a1401f,0x3a23382,0x3a2d3ec,0x3a331e4,0x3a43464,0x3a4ccbe,0x3a53238,0x3a62964,0x3a6c590,
0x3a723b5,0x3a8298e,0x3a8be62,0x3a923e5,0x3aa1b35,0x3aab734,0x3ab24a3,0x3ac1b83,0x3acb006,0x3ad22d3,0x3ae1688,
0x3aea8d8,0x3af232b,0x3b0168e,0x3b0a1aa,0x3b1148a,0x3b2170a,0x3b29a7c,0x3b32962,0x3b4206a,0x3b4934e,0x3b51ac1,
0x3b620be,0x3b68c88,0x3b71b0f,0x3b81217,0x3b884f2,0x3b91b8b,0x3ba12b3,0x3ba7dc4,0x3bb10a9,0x3bc049b,0x3bc76f0,
0x3bd1a74,0x3be0e29,0x3be6f68,0x3bf0c43,0x3c00ec3,0x3c0683a,0x3c10cc8,0x3c2043c,0x3c2610c,0x3c2fe8d,0x3c40466,
0x3c459de,0x3c4febd,0x3c5f5e9,0x3c652b0,0x3c6ff57,0x3c7f637,0x3c84b82,0x3c8f47b,0x3c9e7de,0x3ca4454,0x3caf481,
0x3cbe7e4,0x3cc3d26,0x3ccf2b5,0x3cdfc51,0x3ce35f9,0x3cefa43,0x3cff14b,0x3d02eca,0x3d0eba2,0x3d1f419,0x3d2279c,
0x3d2ee6a,0x3d3e572,0x3d4206e,0x3d4eee6,0x3d5e5ea,0x3d61940,0x3d6e3e0,0x3d7d743,0x3d81419,0x3d8e3ec,0x3d9d74f,
0x3da0ae4,0x3daea08,0x3dbec88,0x3dc03b6,0x3dcd413,0x3ddfc88,0x3dff55a,0x3e1ee2c,0x3e3e6fe,0x3e5e071,0x3e7d8a2,
0x3e9d174,0x3ebca46,0x3edc318,0x3efbbea,0x3f1b4bc,0x3f3ad8e,0x3f5a660,0x3f79f35,0x3f99804,0x3fb90d6,0x3fd89a8,
0x3ff827a,0x4017bec,0x403741e,0x4056d87,0x40765c2,0x4095e94,0x40b579c,0x40d5114,0x40f499a,0x41141dd,0x4133aae,
0x4153380,0x4172c58,0x4192524,0x41b1ecb,0x41d1703,0x41f1039,0x421086c,0x423013e,0x424fa10};

static int flush_current_page(cpu_t *cpu);
static int last_idx = 0;

#if __FOLLOW_MODE__
static uint32_t arm_get_id_from_string(char *reg_name, bool print_regname)
{
	if (print_regname) {
	//	printf("in %s\n", __FUNCTION__);
		printf("reg_name is %s\n", reg_name);
	}
        int i = 0;
        for (i = 0; i < MAX_REG_NUM; i ++) {
                if (0 == strcmp(arm_regstr[i], reg_name))
                        return i;
        }
}

void update_int_array(cpu_t *cpu, uint32_t icounter)
{
	if (icounter > int_icounter[last_idx]) {
//		last_idx ++;
		int_icounter[last_idx] = icounter;
	}
}

#define PRINT_LOG 0
uint32_t follow_mode(cpu_t *cpu)
{
        static uint32_t wait_one_step = 1;
	static int adjust_pc = 0;
        char reg_name[20];
        static char string[100] = {0};
        int i = 0;
        uint32_t val = 0;
        uint32_t idx = 0;
        static uint32_t last_pc = 0;
        bool sw = true;
	static bool print_regname = false;
        arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	if (adjust_pc) {
		adjust_pc = 0;
		goto begin;
	}
        if (wait_one_step == 0) {
                wait_one_step = 1;
                last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
                return 0;
        }
        if (string[0] == 0) {
                fscanf(cpu->src_log, "%s", string);
        }
#if PRINT_LOG
        printf("%s\n", string);
#endif
        if (string[0] != 'P' && string[1] != 'C') {
                printf("log format is not wrong!\n");
#if PRINT_LOG
                printf("%s\n", string);
#endif
                exit(-1);
        }
        val = strtol(&string[3], NULL, 16);
        if (val == 0) {
                fscanf(cpu->src_log, "%s", string);
                val = strtol(&string[3], NULL, 16);
//                return;
        }
        last_pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != last_pc) {
		if (val == 0xffff0018) {
			cpu->check_int_flag = 1;
			update_int_array(cpu, cpu->icounter);
			adjust_pc = 1;
			return 1;
		} else if (last_pc == 0xffff000c) {
			do {
				fscanf(cpu->src_log, "%s", string);
			} while (string[0] != 'P' && string[1] != 'C');
			val = strtol(&string[3], NULL, 16);
			if (val != last_pc) {
				printf("try again, but pc is still wrong.\n");
				exit(-1);
			}
		} else {
	//                printf("pc is wrong.\n");
			LOG_IN_CLR(RED, "pc is wrong\n");
	//                printf("dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(BLUE, "dyncom mode pc is %x\n", core->Reg[15]);
			LOG_IN_CLR(CYAN, "dyncom mode phys_pc is %x\n", core->phys_pc);
			LOG_IN_CLR(LIGHT_RED, "interpreter mode is %x\n", val);
			LOG_IN_CLR(PURPLE, "icounter is %d\n", cpu->icounter);
			LOG_IN_CLR(RED, "adjust pc...\n");
//		core->Reg[15] = val;
//		flush_current_page(cpu);
//		return 1;
			exit(-1);
		}
        }
begin:
        fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
        printf("%s\n", string);
#endif
//	if (cpu->icounter >= 163630) {
//		print_regname = true;
//	}
        while(string[0] != 'P' && string[1] != 'C') {
                while(string[i] != ':') i++;
                string[i] = '\0';
                idx = arm_get_id_from_string(string, print_regname);
#if PRINT_LOG
                printf("idx is %d\n", idx);
#endif
                string[i] = ':';
                val = strtol(&string[i + 1], NULL, 16);
#if PRINT_LOG
                printf("val is %x\n", val);
#endif
                if (idx < 15 && core->Reg[idx] != val) { //&& ((val & 0xf) != 3)) {
			if (core->Reg[15] == 0xc0020ab0 && cpu->icounter > 237538020) {
				core->Reg[idx] = val;
			}
                        if (sw) {
                                printf("addr : %x\n", last_pc);
                                sw = false;
                        }
			if (idx != 16) {
				printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
			}
                        printf("R%d's value is wrong.\n", idx);
                        printf("R%d wrong value : %x right value: %x\n", idx, core->Reg[idx], val);
                        if(idx != 15) {
                                fprintf(cpu->fm_log, "%x instr implementation is wrong\n", last_pc);
                        }
                        //core->Reg[idx] = val;
			printf("icounter is %lld\n", cpu->icounter);
                }
		i = 0;
                fscanf(cpu->src_log, "%s", string);
#if PRINT_LOG
                printf("%s\n", string);
#endif
        }
	return 0;
}
#endif
struct symbolInfo *symbol_info_head = NULL;

void add_list_tail(struct symbolInfo *list)
{
        static struct symbolInfo *symbol_info_tail = NULL;
        if(!symbol_info_head) {
                symbol_info_head = symbol_info_tail = list;
                return;
        }
        symbol_info_tail->next = list;
        symbol_info_tail = list;
}

uint8_t *store_string_info(char *info, size_t size)
{
        static uint32_t max_vol = 0x1000;
        static uint32_t offset = 0;
        static uint32_t remain = 0x1000;
        static uint8_t *repo = NULL;

        uint8_t *str = NULL;
        uint8_t *new_repo = NULL;
        struct symbolInfo *item = NULL;

        //printf("%s, %d, %d\n", info, size, remain);
        if (repo == NULL) {
                repo = (uint8_t *)malloc(max_vol);
                printf("allocate %d bytes.\n", max_vol);
        }
        if (remain < size) {
                new_repo = (uint8_t *)malloc(max_vol * 2);
                printf("allocate %d bytes.\n", max_vol * 2);
                memcpy(new_repo, repo, offset);
                for (item = symbol_info_head; item; item = item->next) {
                        //printf("symbol : %s\taddress : %x\n", item->name, item->address);
                        item->name = new_repo + ((uint8_t *)item->name - (uint8_t *)repo);
                }
                free(repo);
                repo = new_repo;
                new_repo = NULL;
                remain += max_vol;
                max_vol *= 2;
        }
        str = repo + offset;
        memcpy(repo + offset, info, size);
        repo[offset + size] = '\0';
        offset += size;
        remain -= size;
        return str;
}
struct symbolInfo *alloc_symbol_info(uint8_t *str, uint32_t address)
{
        struct symbolInfo *item = (struct symbolInfo *)malloc(sizeof(struct symbolInfo));
        if (item == NULL) {
                printf("Can't allocate more memory in %s\n", __FUNCTION__);
                exit(-1);
        }
        item->next = NULL;
        item->name = str;
        item->address = address;
        return item;
}

struct symbolInfo *search_symbol_info_by_addr(uint32_t address)
{
        struct symbolInfo *prev = NULL, *item = NULL;
        for (item = symbol_info_head; item; item = item->next) {
                if(address == item->address) {
                        return item;
                } else if(address > item->address){
                        prev = item;
                        continue;
                } else {
                        return prev;
                }
        }
        printf("Can not found the address 0x%x in System.map.\n", address);
        //exit(-1);
        return NULL;
}

void print_func_name(uint32_t address)
{
        static struct symbolInfo *last_found = NULL;
        static uint32_t last_address = 0;
        struct symbolInfo *new_found = NULL;
        new_found = search_symbol_info_by_addr(address);
        if (new_found == NULL) {
                return;
        }
        if (last_found != new_found) {
                if (last_found) {
                        LOG_IN_CLR(LIGHT_RED, "exit function %s 0x%x\n", last_found->name, last_address);
                }
                printf("%s\n", new_found->name);
                last_found = new_found;
                last_address = address;
        } else {
		last_address = address;
	}
}

void load_symbol_from_sysmap()
{
        char symbol_address[100];
        char symbol_name[100];
        char type = 0;
        char *str = NULL;
        struct symbolInfo *item = NULL;
        int i = 0;

        uint32_t address = 0;
        FILE *sysmap = fopen("/home/myesis/linux-2.6.35.y/System.map", "r");

        do {
                    if (3 != fscanf(sysmap, "%s %c %s", symbol_address, &type, symbol_name)) break;
                    address = strtol(symbol_address, NULL, 16);
                    while (symbol_name[i] != '\0') {
                            //printf("%c\n", symbol_name[i]);
                            i++;
                    }
                    //printf("symbol:%s\taddress:%x\tsize:%d\n", symbol_name, address, i);
                    str = (char *)store_string_info(symbol_name, i + 1);
                    item = alloc_symbol_info((uint8_t *)str, address);
                    add_list_tail(item);
        } while (1);
        for (item = symbol_info_head; item; item = item->next) {
                printf("symbol : %s\taddress : %x\n", item->name, item->address);
        }
}

static uint32_t int_allow = 0;
uint32_t is_int_in_interpret(cpu_t *cpu)
{
	#if SYNC_HYBRID
	if (int_allow)
	{
		int_allow = 0;
		return 1;
	}
	return 0;
	#else
	static int hit = 0;
	int curr_idx = last_idx;
	int length = sizeof(int_icounter) / sizeof(uint32_t);
	for (; curr_idx < length; curr_idx ++) {
		if (cpu->icounter < int_icounter[curr_idx]) {
			return 0;
		}
		if (int_icounter[curr_idx] == cpu->icounter) {
			last_idx = curr_idx;
			return 1;
		}
	}
	if (last_idx == length) {
		last_idx --;
	}
	return 0;
	#endif
}

#ifdef TIMER_PROFILE
static cpu_t* gcpu;
#include <signal.h>
#include "skyeye_thread.h"
void printinfo(int signum)
{
	cpu_print_statistics(gcpu);
}
#endif

static cpu_flags_layout_t arm_flags_layout[4] ={{3,'N',"NFLAG"},{2,'Z',"ZFLAG"},{1,'C',"CFLAG"},{0,'V',"VFLAG"}} ;
/* physical register for arm archtecture */
static void arch_arm_init(cpu_t *cpu, cpu_archinfo_t *info, cpu_archrf_t *rf)
{
	arm_opc_func_init();
	// Basic Information
	info->name = "arm"; info->full_name = "arm_dyncom";

	// This architecture is biendian, accept whatever the
	// client wants, override other flags.
	info->common_flags &= CPU_FLAG_ENDIAN_MASK;
	/* set the flag of save pc */
	cpu->info.common_flags |= CPU_FLAG_SAVE_PC;

	info->delay_slots = 0;
	// The byte size is 8bits.
	// The word size is 32bits.
	// The float size is 64bits.
	// The address size is 32bits.
	info->byte_size = 8;
	info->word_size = 32;
	info->float_size = 64;
	info->address_size = 32;
	// There are 16 32-bit GPRs
	info->register_count[CPU_REG_GPR] = 19;
	info->register_size[CPU_REG_GPR] = info->word_size;
	// There is also 1 extra register to handle PSR.
	//info->register_count[CPU_REG_XR] = PPC_XR_SIZE;
	info->register_count[CPU_REG_XR] = MAX_REG_NUM - 19;
	//info->register_count[CPU_REG_XR] = 0;
	info->register_size[CPU_REG_XR] = 32;
	//info->register_count[CPU_REG_SPR] = MAX_REG_NUM - PHYS_PC;
	info->register_count[CPU_REG_SPR] = 0;
	info->register_size[CPU_REG_SPR] = 32;
	info->psr_size = 32;
	info->flags_count = 4;
	info->flags_layout = arm_flags_layout;
	/* Indicate the pc index for OPT_LOCAL_REGISTERS */
	info->pc_index_in_gpr = 15;

	cpu->redirection = false;
	
#ifdef TIMER_PROFILE
	gcpu = cpu;
	signal(SIGUSR1, printinfo);
	extern void *clock_thread(void*);
	pthread_t thread;
	int ret = pthread_create(&thread, NULL, clock_thread, NULL);
#endif

	//debug
	cpu_set_flags_debug(cpu, 0
	//	| CPU_DEBUG_PRINT_IR
	//	| CPU_DEBUG_LOG
	//	| CPU_DEBUG_PROFILE
               );
        cpu_set_flags_codegen(cpu, CPU_CODEGEN_TAG_LIMIT 
				| CPU_CODEGEN_OPTIMIZE
				| CPU_CODEGEN_VERIFY
			      );
	/* Initilize different register set for different core */

//	set_memory_operator(arch_arm_read_memory, arch_arm_write_memory);
//	arm_dyncom_mcr_init(cpu);
}

static void
arch_arm_done(cpu_t *cpu)
{
	//free(cpu->rf.grf);
}

static addr_t
arch_arm_get_pc(cpu_t *, void *reg)
{
	unsigned int *grf =(unsigned int *) reg;
	return grf[15];
}

static uint64_t
arch_arm_get_psr(cpu_t *, void *reg)
{
	return 0;
}

static int
arch_arm_get_reg(cpu_t *cpu, void *reg, unsigned reg_no, uint64_t *value)
{
	return (0);
}

static int arch_arm_disasm_instr(cpu_t *cpu, addr_t pc, char* line, unsigned int max_line){
	return 0;
}
static int arch_arm_translate_loop_helper(cpu_t *cpu, addr_t pc, BasicBlock *bb_ret, BasicBlock *bb_next, BasicBlock *bb, BasicBlock *bb_zol_cond){
	return 0;
}

static void arch_arm_emit_decode_reg(cpu_t *cpu, BasicBlock *bb)
{
	Value *nzcv = LSHR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xf0000000)), CONST(28));
	Value *n = TRUNC1(AND(LSHR(nzcv, CONST(3)), CONST(1)));
	Value *z = TRUNC1(AND(LSHR(nzcv, CONST(2)), CONST(1)));
	Value *c = TRUNC1(AND(LSHR(nzcv, CONST(1)), CONST(1)));
	Value *v = TRUNC1(AND(LSHR(nzcv, CONST(0)), CONST(1)));
	new StoreInst(n, cpu->ptr_N, false, bb);
	new StoreInst(z, cpu->ptr_Z, false, bb);
	new StoreInst(c, cpu->ptr_C, false, bb);
	new StoreInst(v, cpu->ptr_V, false, bb);
}

static void arch_arm_spill_reg_state(cpu_t *cpu, BasicBlock *bb)
{
		/* Save N Z C V */
	Value *z = SHL(ZEXT32(LOAD(cpu->ptr_Z)), CONST(30));
	Value *n = SHL(ZEXT32(LOAD(cpu->ptr_N)), CONST(31));
	Value *c = SHL(ZEXT32(LOAD(cpu->ptr_C)), CONST(29));
	Value *v = SHL(ZEXT32(LOAD(cpu->ptr_V)), CONST(28));
	Value *nzcv = OR(OR(OR(z, n), c), v);
	Value *cpsr = OR(AND(LOAD(cpu->ptr_gpr[16]), CONST(0xfffffff)), nzcv);
	new StoreInst(cpsr, cpu->ptr_gpr[16], false, bb);
}

static arch_func_t arm_arch_func = {
	arch_arm_init,
	arch_arm_done,
	arch_arm_get_pc,
	arch_arm_emit_decode_reg,
	arch_arm_spill_reg_state,
	arch_arm_tag_instr,
	arch_arm_disasm_instr,
	arch_arm_translate_cond,
	arch_arm_translate_instr,
        arch_arm_translate_loop_helper,
	// idbg support
	arch_arm_get_psr,
	arch_arm_get_reg,
	NULL
};

static uint32_t arm_debug_func(cpu_t* cpu){
	int idx = 0;
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
#if 0	
	for (idx = 0;idx < 16; idx ++) {
		LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
	}
	printf("\n");

	if (cpu->icounter > 248306790) {
		if ((core->Reg[15] & 0xf0000000) == 0x50000000) {
			print_func_name(core->Reg[15] + 0x70000000);
		} else
			print_func_name(core->Reg[15]);
	}
#endif
	//printf("run at %x\n", core->Reg[15]);
#if 0
	if (cpu->icounter == 170965) {
		printf("at 790000\n");
		exit(-1);
	}
#endif
#if 0
//	if (cpu->icounter == 687418 || cpu->icounter == 687417 || cpu->icounter == 687416) {
//	if (cpu->icounter >= 0) {
	if (cpu->icounter > 254940700) {
// 	if (cpu->icounter > 1694550) {
//	if (cpu->icounter > 1287900) {
//	if (cpu->icounter > 1696000) {
//	if (cpu->icounter > 779800) {
		printf("icounter is %lld\n", cpu->icounter);
		for (idx = 0;idx < 16; idx ++) {
			LOG_IN_CLR(RED, "R%d:0x%x\t", idx, core->Reg[idx]);
		}
		LOG_IN_CLR(BLUE, "CPSR:0x%x\n", core->Cpsr);
		LOG_IN_CLR(LIGHT_BLUE, "SPSR:0x%x\n", core->Spsr_copy);
		printf("int is %d\n", core->NirqSig);
//		printf("\n");
//		printf("phys base addr is %x\n", cpu->current_page_phys);
//		printf("effec base addr is %x\n", cpu->current_page_effec);
	}
	if (cpu->icounter == 1696000) {
//		exit(1);
	}
	#if 0
	if (core->Reg[15] == 0xc0122570) {
		printf("hit it\n");
	}
	#endif
	#if 0
	if (core->Reg[15] == 0x500083b0) {
		for (idx = 0;idx < 16; idx ++) {
			printf("R%d:0x%x\t", idx, core->Reg[idx]);
		}
		printf("\n");
	}
	#endif
#endif

#if 0
#if DIFF_LOG
#if SAVE_LOG
	fprintf(core->state_log, "PC:0x%x\n", cpu->f.get_pc(cpu, cpu->rf.grf));
	for (idx = 0;idx < 16; idx ++) {
		fprintf(core->state_log, "R%d:0x%x\n", idx, core->Reg[idx]);
	}
#else
	uint32_t val;
	fscanf(core->state_log, "PC:0x%x\n", &val);
        uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);
        if (val != pc) {
                printf("pc is wrong.\n");
                printf("dyncom mode pc is %x\n", pc);
                printf("adu mode is %x\n", val);
		printf("icounter is %x\n", cpu->icounter);
                exit(-1);
        }
	uint32_t dummy;
	bool flags = 0;
	for (idx = 0; idx < 16; idx ++) {
		fscanf(core->state_log, "R%d:0x%x\n", &dummy, &val);
		//printf("R%d:0x%x\n", dummy, val);
		if (dummy == idx) {
			if (core->Reg[idx] != val) {
				printf("dummy is %d R%d : \t[R]%x \t[W]%x\n", dummy, idx, val, core->Reg[idx]);
				flags = 1;
				//core->Reg[idx] = val;
			}
		} else {
			printf("wrong dummy\n");
			exit(-1);
		}
	}
	if (flags) {
		printf("pc is %x\n", pc);
		printf("icounter is %x\n", cpu->icounter);
		flags = 0;
		exit(-1);
	}
#endif
#endif
#endif

#if 0
#if __FOLLOW_MODE__
#if SYNC_WITH_INTERPRET
	if (is_int_in_interpret(cpu)) {
		cpu->check_int_flag = 1;
		return 1;
	}
	return follow_mode(cpu);
#endif
#else
	return 0;
#endif
#endif

	
// Below is Alex logs
#define BOT_LOG 0
#define TOP_LOG 0xffff0000

// Instruction type counter, needs DEBUG_ME
#if 0

const char* arm_instr[] = {
"adc", "add","and","b,bl","bic","bkpt","blx(1)","blx(2)","bx","bxj","cdp","clrex","clz","cmn","cmp","cps","cpy","eor","ldc","ldm(1)",
"ldm(2)","ldm(3)","sxth","ldr","uxth","uxtah","ldrb","ldrbt","ldrd","ldrex","ldrexb","ldrh","ldrsb","ldrsh","ldrt","mcr","mcrr","mla",
"mov","mrc","mrrc","mrs","msr","msr","mul","mvn","orr","pkhbt","pkhtb","pld","qadd","qadd16","qadd8","qaddsubx","qdadd","qdsub","qsub",
"qsub16","qsub8","qsubaddx","rev","revsh","rfe","rsb","rsc","sadd16","sadd8","saddsubx","sbc","sel","setend","shadd16","shadd8","shaddsubx",
"shsub16","shsub8","shsubaddx","smla<x><y>","smlad","smlal","smlal<x><y>","smlald","smlaw<y>","smlsd","smlsld","smmla","smmls","smmul",
"smuad","smul<x><y>","smull","smulw<y>","smusd","srs","ssat","ssat16","ssub16","ssub8","ssubaddx","stc","stm(1)","stm(2)","sxtb","str",
"uxtb","uxtab","strb","strbt","strd","strex",	"strexb","strh","strt","sub","swi","swp","swpb","sxtab","sxtab16","sxtah","sxtb16","teq",
"tst","uadd16","uadd8","uaddsubx","uhadd16","uhadd8","uhaddsubx","uhsub16","uhsub8","uhsubaddx","umaal","umlal","umull","uqadd16","uqadd8",
"uqaddsubx","uqsub16","uqsub8","uqsubaddx","usad8","usada8","usat","usat16","usub16","usub8","usubaddx","uxtab16","uxtb16"
};
	
	static int32_t hash_to_instr [0x150000];
	static uint32_t instr_count [150];
	static uint32_t icounter = 0;

	static uint32_t instr_flag = 1;
	if (instr_flag)
	{
		printf("Reset instr table\n");
		for (instr_flag = 0; instr_flag < 0x150000; instr_flag++)
			hash_to_instr[instr_flag] = -1;
		for (instr_flag = 0; instr_flag < 0x150; instr_flag++)
			instr_count[instr_flag] = 0;
		instr_flag = 0;
	}
	uint32_t pc = cpu->f.get_pc(cpu, cpu->rf.grf);
	uint32_t instr = *((uint32_t *) pc);
	if (hash_to_instr[pc] == -1)
	{
		decode_arm_instr(instr, &idx);
		hash_to_instr[pc] = idx;
	}
	else
	{
		instr_count[hash_to_instr[pc]] += 1;
	}
	
	icounter++;
	
	if ( (icounter%0x100000) == 0)
	{
		setbuf(core->state_log, NULL);
		printf("Instr table updated %x\n", icounter);
		for(idx = 0; idx < 150; idx++)
		{
			fprintf( core->state_log, "%s\t%d\t% 12ud\n", arm_instr[idx], idx, instr_count[idx]);
			//printf("%d\t% 12ud\n", idx, instr_count[idx]);
		}
	}
#endif

// General log, needs bot_log and top_log from skyeye_pref
#if 1
	// Initialize
	static int flagged = 0; // indicate if we are logging or not
	static uint32_t bot_log = -1; // starting icounter for logging
	static uint32_t top_log = -1; // ending icounter
	if (bot_log == -1)
	{
		sky_pref_t* pref = get_skyeye_pref();
		bot_log = pref->bot_log;
		top_log = pref->top_log;
	}

	// Catch bot_log
	if ((flagged == 0) && (top_log > 0)) {
		if ((core->NumInstrs > bot_log) ||
		    0
		    )
		{
			printf("##### Hit %x at %x\n", cpu->f.get_pc(cpu, cpu->rf.grf), core->NumInstrs);
			get_skyeye_pref()->start_logging = 1;
			//core->NumInstrs = bot_log;
		}
	}

#if 0
	// read instruction
	if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0x8210)
	{
		uint32_t dummy = 0;
		uint32_t pa = 0;
		core->mmu.ops.load_instr(core,cpu->f.get_pc(cpu, cpu->rf.grf),&dummy,&pa);
		uint32_t dummy_tag = get_tag(cpu, pa);
		printf("~~~~ instruction at %x-%x is %x and get tagged %x\n", cpu->f.get_pc(cpu, cpu->rf.grf), pa, dummy, dummy_tag);
	}
#endif

	// Initiate logging via flagged or external triggered logging (start_logging)
	if ((flagged == 0) && (get_skyeye_pref()->start_logging == 1))
		flagged = 1;
	
	// flag used in follow mode (alex version), indicate that we must trigger an interrupt
	static uint32_t int_flag = 0;
	
	if (flagged)
	{
		// Record trace in dyncom
#if 0
		if (core->NumInstrs % 0x100 == 0)
		//	printf("||||| %x |||||\n", core->NumInstrs);
			printf("---|%p|--- %x\n", cpu->f.get_pc(cpu, cpu->rf.grf), core->NumInstrs);
		else
			printf("---|%p|---\n", cpu->f.get_pc(cpu, cpu->rf.grf));   
		
		//if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xc0009810)
		{
		for (idx = 0;idx < 16; idx ++) {
			//printf("R%02d % 8x\n", idx, core->Reg[idx]);
		}
		//printf("CPS %08x\n", core->Cpsr);
		}
		
		if ( (top_log > 0) && (core->NumInstrs > top_log))
		{
			printf("Exiting\n");
			sleep(1);
			exit(-1);
		}
#else
		// Follow trace in dyncom
do_int:
		static FILE* log = NULL;
		static uint32_t resync = 0;
		static uint32_t val = 0;
		uint32_t dummy = 0;
		uint32_t error = 0;
		char trash[100];
		// Initiate log reading
		if (log == NULL)
		{
			log = fopen("/home/yuhe/dev/skyeye-testsuite/dyncom/android_kernel/loge3", "r");
			//for (val = 0; val < 34; val ++)
			//	fgets(trash, 100, log);
			long int pos = bot_log;
			fseek(log, 264*pos, SEEK_CUR);
		}
		core->cycle++;
		// Exit oj JIT immediately and trigger an interrupt
		if (int_flag)
		{
			if (cpu->f.get_pc(cpu, cpu->rf.grf) != 0xffff0018)
			{
				printf("(IRQ) Cycle beginning: we INT at %x at pc %x\n", core->NumInstrs, cpu->f.get_pc(cpu, cpu->rf.grf));
				core->Reg[15] -= 4;
				core->NirqSig = 0;
				int_flag = 0;
				int_allow = 1;
				return -1;
			}
			else
			{
				printf("(IRQ) Cycle beginning: already in INT at %x pc %x\n", core->NumInstrs, cpu->f.get_pc(cpu, cpu->rf.grf));
				int_flag = 0;
			}
		}
		
		long int prev = ftell(log);
		fscanf(log, "---|%x|---\n", &val);
		if (val != cpu->f.get_pc(cpu, cpu->rf.grf))
		{
			printf("(PC) STOPPP at %x, where I read %x instead of %x\n", core->NumInstrs, val, cpu->f.get_pc(cpu, cpu->rf.grf));
			if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xffff000c)
			{
				printf("(SYNC) Prefetch abort, syncing log and jump one cycle at %x\n", core->NumInstrs);
				uint32_t k;
				for (k = 0; k < 19; k++)
				{
					fgets(trash, 100, log);
				}
				goto do_int;
			} else /*if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xffff0010) {
				printf("(SYNC) Data abort, syncing log and jump one cycle at %x\n", core->NumInstrs);
				uint32_t k;
				for (k = 0; k < 19+40; k++)
				{
					fgets(trash, 100, log);
				}
				goto do_int;
			} else*/ {
				error = 1;
			}
		}
		fscanf(log, "CYC %x\n", &val);
		/*if (val != core->cycle) {
			printf("(CYC) SYNC at %x, where I read %x instead of %x, pc being %x\n", 
			       core->NumInstrs, val, core->cycle, cpu->f.get_pc(cpu, cpu->rf.grf));
			while (core->cycle < val) {
				core->cycle++;
				io_do_cycle(cpu);
			}
		}*/
		fscanf(log, "IRQ %x\n", &val);
		if (val != core->NirqSig) {
			printf("(IRQ) STOPPP at %x, where I read %x instead of %x, pc being %x\n", 
			       core->NumInstrs, val, core->NirqSig, cpu->f.get_pc(cpu, cpu->rf.grf));
			error |= 1 << 2;
			//core->NirqSig = val;
			//exit(-1);
		}

	#if 1
		for (dummy = 0; dummy < 16; dummy++) {
			fscanf(log, "R%02d %x\n", &dummy, &val);
			if (core->Reg[dummy] != val) {
				printf("(R%02d) STOPPP at %x, where I read %x instead of %x, pc being %x\n", 
				       dummy, core->NumInstrs, val, core->Reg[dummy], cpu->f.get_pc(cpu, cpu->rf.grf));
				error |= 1 << (3 + dummy);
			}
		}
	#else
		fseek(log, 13 * 16, SEEK_CUR);
	#endif
	
		fscanf(log, "CPS %x\n", &val);
		if (core->Cpsr != val) {
			printf("(CPSR) STOPPP at %x, where I read %x instead of %x, pc being %x\n", 
			       core->NumInstrs, val, core->Cpsr, cpu->f.get_pc(cpu, cpu->rf.grf));
			error |= 1 << (19 + dummy);
		}
		
		// Some output in case there is errors
		if (error)
		{
			#if 0
			fseek(log, prev - 264, SEEK_SET); 
			uint32_t j = 0;
			// print the previous insruction
			for (j = 0; j < 20; j++)
			{
				fgets(trash, 100, log);
				printf("%s", trash);
			}
			// print error lines
			error |= 1; // force the print of current pc
			for (j = 0; j < 20; j++)
			{
				fgets(trash, 100, log);
				if (error & 1) {
					printf(">>> ");
				}
				printf("%s", trash);
				error = error >> 1;
			}
			
			if (cpu->f.get_pc(cpu, cpu->rf.grf) == 0xffff0018)
				if (core->Reg[4] == 0xc0021a48)
				{
					printf("\t\tCorrecting R04\n");
					core->Reg[4] = 0x20000000;
				}
			#endif
			error = 0;
		}
		
		// Checking next instruction. Is it an interrupt? If yes, set a flag
		prev = ftell(log);
		fscanf(log, "---|%x|---\n", &val);
		fseek(log, prev, SEEK_SET);
		if (val == 0xffff0018) {
			int_flag = 1;
		} else
			int_flag = 0;
		
		if ( (top_log > 0) && (core->NumInstrs > top_log))
		{
			printf("Exiting\n");
			sleep(1);
			exit(-1);
		}
	} else {
		// used in follow mode, sync interpreter interrupts with dyncom
		#if SYNC_HYBRID
		static uint32_t int_index = 0;
		uint32_t last_value = int_icounter[int_index];
		int length = sizeof(int_icounter) / sizeof(uint32_t);
		if (core->NumInstrs >= last_value)
		{
			printf("(SYNC) Reached %x\n", core->NumInstrs);
			int_index++;
			int_flag = 1;
			goto do_int;
		}
		#endif
#endif
	}
	
	io_do_cycle(cpu);
	core->NumInstrs++;
	
#endif
	
	return 0;
}

extern "C" unsigned arm_dyncom_SWI (ARMul_State * state, ARMword number);
extern "C" void arm_dyncom_Abort(ARMul_State * state, ARMword vector);

static void arm_dyncom_syscall(cpu_t* cpu, uint32_t num){

	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	sky_pref_t* pref = get_skyeye_pref();
	//printf("in %s user_mode_sim %d", __FUNCTION__, pref->user_mode_sim);
	if(pref->user_mode_sim)
		arm_dyncom_SWI(core, num);
	else
		//ARMul_Abort(core,ARMul_SWIV);
		core->syscallSig = 1;
}

/* Undefined instruction handler, set necessary flags */
void 
arm_undef_instr(cpu_t *cpu){
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	printf("\t\tLet us set a flag, signaling an undefined instruction!\n");
	core->Aborted = ARMul_UndefinedInstrV;
	core->abortSig = HIGH;
}

/**
 * @brief Generate the invoke undef instr exception llvm IR
 *
 * @param cpu CPU core structure
 * @param bb basic block to store llvm IR 
 * @param instr undefined instruction (unused)
 */
void
arch_arm_undef(cpu_t *cpu, BasicBlock *bb, uint32_t instr)
{
	if (cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF] == NULL) {
		printf("in %s Could not find callout\n", __FUNCTION__);
		return;
	}
	Type const *intptr_type = cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX());
	Constant *v_cpu = ConstantInt::get(intptr_type, (uintptr_t)cpu);
	Value *v_cpu_ptr = ConstantExpr::getIntToPtr(v_cpu, PointerType::getUnqual(intptr_type));
	std::vector<Value *> params;
	params.push_back(v_cpu_ptr);
	/* When using a custom callout, must put the callout index as argument for dyncom_callout */
	params.push_back(CONST(ARM_DYNCOM_CALLOUT_UNDEF));
	//params.push_back(CONST(instr)); // no need for now, the callout func takes no argument
	CallInst *ret = CallInst::Create(cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF], params.begin(), params.end(), "", bb);
}

/* Undefined instruction handler initialization. Should be called once at init */
static void 
arch_arm_undef_init(cpu_t *cpu){
	//types
	std::vector<const Type*> type_func_undef_args;
	PointerType *type_intptr = PointerType::get(cpu->dyncom_engine->exec_engine->getTargetData()->getIntPtrType(_CTX()), 0);
	const IntegerType *type_i32 = IntegerType::get(_CTX(), 32);
	type_func_undef_args.push_back(type_intptr);	/* intptr *cpu */
	type_func_undef_args.push_back(type_i32);	/* unsinged int */
	FunctionType *type_func_undef_callout = FunctionType::get(
		Type::getInt32Ty(cpu->dyncom_engine->mod->getContext()),	//return
		type_func_undef_args,	/* Params */
		false);		      	/* isVarArg */
	/* For a custom callout, the dyncom_calloutX functions should be used */
	Constant *undef_const = cpu->dyncom_engine->mod->getOrInsertFunction("dyncom_callout",	//function name
		type_func_undef_callout);	//return
	if(undef_const == NULL)
		fprintf(stderr, "Error:cannot insert function:undefined_instr_callout.\n");
	Function *undef_func = cast<Function>(undef_const);
	undef_func->setCallingConv(CallingConv::C);
	cpu->dyncom_engine->ptr_arch_func[ARM_DYNCOM_CALLOUT_UNDEF] = undef_func;
	cpu->dyncom_engine->arch_func[ARM_DYNCOM_CALLOUT_UNDEF] = (void*)arm_undef_instr;
}

void arm_dyncom_init(arm_core_t* core){
	cpu_t* cpu = cpu_new(0, 0, arm_arch_func);
#if __FOLLOW_MODE__
//	cpu->src_log = fopen("/data/state.log", "r");
	cpu->src_log = fopen("/diff/state.log", "r");
	if (cpu->src_log == NULL) {
		printf("Load log file failed.\n");
		//exit(-1);
	}
	printf("Load source log file successfully.\n");
	cpu->fm_log = fopen("/data/fm.log", "w");
	printf("Create follow mode log file.\n");
#endif

	/* set user mode or not */
	sky_pref_t *pref = get_skyeye_pref();
	if(pref->user_mode_sim)
                cpu->is_user_mode = 1;
        else
                cpu->is_user_mode = 0;
	
	core->NirqSig = HIGH;
	cpu->dyncom_engine->code_entry = 0x80d0;
	if (!pref->user_mode_sim) {
		cpu->dyncom_engine->code_start = 0;
		cpu->dyncom_engine->code_end = 0xffffffff;
	} else {
		cpu->dyncom_engine->code_end = 0x100000;
		cpu->dyncom_engine->code_entry = 0x80d0;
	}

	cpu->switch_mode = arm_switch_mode;
	/* for sync with interpret mode */
	cpu->check_int_flag = 0;
	
	cpu->mem_ops = arm_dyncom_mem_ops;
	//cpu->cpu_data = (conf_object_t*)core;
	cpu->cpu_data = get_conf_obj_by_cast(core, "arm_core_t");
	
	/* init the reg structure */
	cpu->rf.pc = &core->Reg[15];
	/* Under user mode sim, both phys_pc and pc are pointed to Reg 15 */
	if(is_user_mode(cpu))
		cpu->rf.phys_pc = &core->Reg[15];
	else
		cpu->rf.phys_pc = &core->phys_pc;
	cpu->rf.grf = core->Reg;
	//cpu->rf.srf = core->Spsr;
	//cpu->rf.srf = &core->phys_pc;
	cpu->rf.srf = core->Reg_usr;

	
	cpu->debug_func = arm_debug_func;
	
	if(pref->user_mode_sim){
		cpu->syscall_func = arm_dyncom_syscall;
	}
	else
//		cpu->syscall_func = NULL;
		cpu->syscall_func = arm_dyncom_syscall;
	core->dyncom_cpu = get_conf_obj_by_cast(cpu, "cpu_t");
	
	cpu->dyncom_engine->flags &= ~CPU_FLAG_SWAPMEM;

	if (pref->user_mode_sim){
#ifdef FAST_MEMORY
		cpu->dyncom_engine->RAM = (uint8_t*)get_dma_addr(0);
#endif
	}

	//core->CP15[CP15(CP15_MAIN_ID)] = 0x410FB760;
	core->CP15[CP15(CP15_MAIN_ID)] = 0x7b000;
	//core->CP15[CP15_MAIN_ID + 1] = 0x410FB760;
	//core->CP15[CP15_MAIN_ID - 1] = 0x410FB760;
	core->CP15[CP15(CP15_CONTROL)] = 0x00050078;
//	core->CP15[CP15(CP15_CONTROL)] = 0x00000078;
	core->CP15[CP15(CP15_CACHE_TYPE)] = 0xd172172;
	core->Cpsr = 0xd3;
	core->Mode = SVC32MODE;

//	load_symbol_from_sysmap();

	/* undefined instr handler init */
	arch_arm_undef_init(cpu);
	
	init_compiled_queue(cpu);
	return;
}

void switch_mode(arm_core_t *core, uint32_t mode)
{
	uint32_t tmp1, tmp2;
	if (core->Mode == mode) {
		//Mode not changed.
		//printf("mode not changed\n");
		return;
	}
	//printf("%d --->>> %d\n", core->Mode, mode);
	if (mode != USERBANK) {
		switch (core->Mode) {
		case USER32MODE:
			core->Reg_usr[0] = core->Reg[13];
			core->Reg_usr[1] = core->Reg[14];
			break;
		case IRQ32MODE:
			core->Reg_irq[0] = core->Reg[13];
			core->Reg_irq[1] = core->Reg[14];
			core->Spsr[IRQBANK] = core->Spsr_copy;
			break;
		case SVC32MODE:
			core->Reg_svc[0] = core->Reg[13];
			core->Reg_svc[1] = core->Reg[14];
			core->Spsr[SVCBANK] = core->Spsr_copy;
			break;
		case ABORT32MODE:
			core->Reg_abort[0] = core->Reg[13];
			core->Reg_abort[1] = core->Reg[14];
			core->Spsr[ABORTBANK] = core->Spsr_copy;
			break;
		case UNDEF32MODE:
			core->Reg_undef[0] = core->Reg[13];
			core->Reg_undef[1] = core->Reg[14];
			core->Spsr[UNDEFBANK] = core->Spsr_copy;
			break;
		case FIQ32MODE:
			core->Reg_firq[0] = core->Reg[13];
			core->Reg_firq[1] = core->Reg[14];
			core->Spsr[FIQBANK] = core->Spsr_copy;
			break;

		}

		switch (mode) {
		case USER32MODE:
			core->Reg[13] = core->Reg_usr[0];
			core->Reg[14] = core->Reg_usr[1];
			core->Bank = USERBANK;
			break;
		case IRQ32MODE:
			core->Reg[13] = core->Reg_irq[0];
			core->Reg[14] = core->Reg_irq[1];
			core->Spsr_copy = core->Spsr[IRQBANK];
			core->Bank = IRQBANK;
			break;
		case SVC32MODE:
			core->Reg[13] = core->Reg_svc[0];
			core->Reg[14] = core->Reg_svc[1];
			core->Spsr_copy = core->Spsr[SVCBANK];
			core->Bank = SVCBANK;
			break;
		case ABORT32MODE:
			core->Reg[13] = core->Reg_abort[0];
			core->Reg[14] = core->Reg_abort[1];
			core->Spsr_copy = core->Spsr[ABORTBANK];
			core->Bank = ABORTBANK;
			break;
		case UNDEF32MODE:
			core->Reg[13] = core->Reg_undef[0];
			core->Reg[14] = core->Reg_undef[1];
			core->Spsr_copy = core->Spsr[UNDEFBANK];
			core->Bank = UNDEFBANK;
			break;
		case FIQ32MODE:
			core->Reg[13] = core->Reg_firq[0];
			core->Reg[14] = core->Reg_firq[1];
			core->Spsr_copy = core->Spsr[FIQBANK];
			core->Bank = FIQBANK;
			break;

		}
		core->Mode = mode;
	} else {
		printf("user mode\n");
		exit(-2);
	}
}

void arm_switch_mode(cpu_t *cpu)
{
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	switch_mode(core, core->Cpsr & 0x1f);
}

static int flush_current_page(cpu_t *cpu){
	//arm_core_t* core = (arm_core_t*)(cpu->cpu_data);
	arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	addr_t effec_pc = *(addr_t*)cpu->rf.pc;
//	printf("effec_pc is %x\n", effec_pc);
//	printf("in %s\n", __FUNCTION__);
	int ret = cpu->mem_ops.effective_to_physical(cpu, effec_pc, (uint32_t*)cpu->rf.phys_pc);
	cpu->current_page_phys = core->phys_pc & 0xfffff000;
	cpu->current_page_effec = core->pc & 0xfffff000;
	return ret;
}

void arm_dyncom_run(cpu_t* cpu){
	//arm_core_t* core = (arm_core_t*)get_cast_conf_obj(cpu->cpu_data, "arm_core_t");
	arm_core_t* core = (arm_core_t*)(cpu->cpu_data->obj);
	uint32_t mode;

	addr_t phys_pc;
	if(is_user_mode(cpu)){
		addr_t phys_pc = core->Reg[15];
	}
#if 0
	if(mmu_read_(core, core->pc, PPC_MMU_CODE, &phys_pc) != PPC_MMU_OK){
		/* we donot allow mmu exception in tagging state */
		fprintf(stderr, "In %s, can not translate the pc 0x%x\n", __FUNCTION__, core->pc);
		exit(-1);
	}
#endif

#if 0
	cpu->dyncom_engine->code_start = phys_pc;
        cpu->dyncom_engine->code_end = get_end_of_page(phys_pc);
        cpu->dyncom_engine->code_entry = phys_pc;
#endif

	int rc = cpu_run(cpu);
	//printf("### Out of jit return %d - %p %p\n", rc, core->Reg[15], core->Reg[15]);
	
//	printf("pc %x is not found\n", core->Reg[15]);
	switch (rc) {
	case JIT_RETURN_NOERR: /* JIT code wants us to end execution */
	case JIT_RETURN_TIMEOUT:
                        break;
                case JIT_RETURN_SINGLESTEP:
	case JIT_RETURN_FUNCNOTFOUND:
//			printf("pc %x is not found\n", core->Reg[15]);
//			printf("phys_pc is %x\n", core->phys_pc);
//			printf("out of jit\n");
			if(!is_user_mode(cpu)){
				switch_mode(core, core->Cpsr & 0x1f);
				if (flush_current_page(cpu)) {
					return;
				}
				clear_tag_page(cpu, core->phys_pc);
				cpu_tag(cpu, core->phys_pc);
				cpu->dyncom_engine->cur_tagging_pos ++;
				//cpu_translate(cpu, core->Reg[15]);
				cpu_translate(cpu, core->phys_pc);
			}
			else{
				cpu_tag(cpu, core->Reg[15]);
				cpu->dyncom_engine->cur_tagging_pos ++;
				cpu_translate(cpu, core->Reg[15]);
			}

		 /*
                  *If singlestep,we run it here,otherwise,break.
                  */
                        if (cpu->dyncom_engine->flags_debug & CPU_DEBUG_SINGLESTEP){
                                rc = cpu_run(cpu);
                                if(rc != JIT_RETURN_TRAP)
                                        break;
                        }
                        else
                                break;
	case JIT_RETURN_TRAP:
		if (core->syscallSig) {
			return;
		}
		if (cpu->check_int_flag == 1) {
			cpu->check_int_flag = 0;
			return;
		}
		if (core->abortSig) {
			return;
		}
//		printf("cpu maybe changed mode.\n");
//		printf("pc is %x\n", core->Reg[15]);
		//printf("icounter is %lld\n", cpu->icounter);
		//exit(-1);
		//core->Reg[15] += 4;
		mode = core->Cpsr & 0x1f;
		if ( (mode != core->Mode) && (!is_user_mode(cpu)) ) {
			switch_mode(core, mode);
			//exit(-1);
		}
		core->Reg[15] += 4;
		return;
			break;
		default:
                        fprintf(stderr, "unknown return code: %d\n", rc);
			skyeye_exit(-1);
        }// switch (rc)
	return;
}
/**
* @brief Debug function that will be called in every instruction execution, print the cpu state
*
* @param cpu the cpu_t instance
*/

void arm_dyncom_stop(){
}

void arm_dyncom_fini(){
}
