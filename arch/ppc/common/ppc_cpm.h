/*
        ppc_cpm.h - definition for PPC CPM core
        Copyright (C) 2003 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 07/21/2008   Michael.Kang  <blackfin.kang@gmail.com>
 */

#ifndef __PPC_CPM_H__
#define __PPC_CPM_H__
#if 0
/* 0x91a00-0x91a9f: SCC1-SCC4 */
typedef struct cpm_scc_s {
	uint32 gsmrl;
	uint32 gsmrh;
	uint16 psmr;
	char res1[2];
	uint16 todr;
	uint16 dsr;
	uint16 scce;
	char res2[2];
	uint16 sccm;
	char res3;
	uint8 sccs;
	char res4[8];
} cpm_scc_t;

typedef struct cpm_mux_s {
	uint32_t cmxfcr;
	uint32_t cmxscr;
} cpm_mux_t;
typedef struct cpm_ioport_s {
	uint32_t pdira;
	uint32_t ppara;
	uint32_t psora;
	uint32_t podra;
	uint32_t pdata;
	uint32_t pdirb;
	uint32_t pparb;
	uint32_t psorb;
	uint32_t podrb;
	uint32_t pdatb;
	uint32_t pdirc;
	uint32_t pparc;
	uint32_t psorc;
	uint32_t podrc;
	uint32_t pdatc;
	uint32_t pdird;
	uint32_t ppard;
	uint32_t psord;
	uint32_t podrd;
	uint32_t pdatd;
} cpm_ioport_t;

typedef struct cpm_int_ctrl_s {
	uint32 sicr;
	uint32 sipnr_h;
	uint32 sipnr_l;
	uint32 scprr_h;
	uint32 scprr_l;
	uint32 simr_h;
	uint32 simr_l;
} cpm_int_ctrl_t;

typedef struct ppc_cpm_s {
	uint32_t cpcr;
	uint32_t rccr;
	uint32_t rter;
	uint32_t rtmr;
	uint32_t rtscr;
	uint32_t rtsr;

	byte *dpram;
	byte *iram;		/* instruction RAM */
	cpm_scc_t scc[4];
	cpm_mux_t mux;
	cpm_int_ctrl_t int_ctrl;
	uint32_t brgc[4];
	cpm_ioport_t ioport;
} ppc_cpm_t;
#endif
#endif
