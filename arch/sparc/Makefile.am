
sparc_mach = mach/leon2_timer_unit.c mach/leon2_uart.c mach/mach_leon2_io.c mach/skyeye_mach_leon2.c mach/leon2_mcfg.c mach/leon2_cfg.c mach/leon2_irqctrl.c
sparc_comm = common/sparc_arch_interface.c common/traps.c common/iu.c common/sparcemu.c common/stat.c common/helper.c common/tcg-target.c common/sparc_module.c
sparc_insn = instructions/i_add.c instructions/i_addcc.c instructions/i_addx.c instructions/i_addxcc.c instructions/i_and.c instructions/i_andcc.c instructions/i_andn.c	\
			 instructions/i_andncc.c instructions/i_mulscc.c	\
			 instructions/i_ba.c instructions/i_bcc.c instructions/i_bcs.c instructions/i_be.c instructions/i_bg.c instructions/i_bge.c instructions/i_bgu.c instructions/i_bl.c 	\
			 instructions/i_ble.c instructions/i_bleu.c instructions/i_bn.c instructions/i_bne.c instructions/i_bneg.c instructions/i_bpos.c instructions/i_bvc.c instructions/i_bvs.c	\
			 instructions/i_call.c instructions/i_flush.c instructions/i_jmpl.c instructions/i_ld.c instructions/i_ldd.c instructions/i_lddf.c instructions/i_ldf.c	\
			 instructions/i_ldsb.c instructions/i_ldsh.c instructions/i_ldub.c instructions/i_lduh.c instructions/i_nop.c 	\
			 instructions/i_or.c instructions/i_orcc.c instructions/i_orn.c instructions/i_rdpsr.c instructions/i_rdtbr.c instructions/i_rdwim.c instructions/i_rdy.c	\
			 instructions/i_restore.c instructions/i_rett.c instructions/i_save.c instructions/i_sethi.c instructions/i_sll.c instructions/i_smul.c	instructions/i_umul.c	\
			 instructions/i_sra.c instructions/i_srl.c instructions/i_stb.c instructions/i_st.c instructions/i_std.c instructions/i_stdf.c instructions/i_stf.c instructions/i_stfsr.c	\
			 instructions/i_sth.c instructions/i_sub.c instructions/i_subcc.c instructions/i_subx.c instructions/i_subxcc.c instructions/i_ta.c	\
			 instructions/i_udiv.c instructions/i_utils.c instructions/i_wrpsr.c instructions/i_wrtbr.c instructions/i_wrwim.c instructions/i_wry.c	\
			 instructions/i_xnor.c instructions/i_xor.c

libsparc_la_SOURCES = $(sparc_mach) $(sparc_comm) $(sparc_insn)
pkglib_LTLIBRARIES = libsparc.la
# we will link the common module with main
libsparc_la_LDFLAGS = -module

#DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0
#DEFS = -g -O2 -DSPARC_DEBUG -DSPARC_ENABLE_STAT
#DEFS = -g -O2 -DSPARC_DEBUG
DEFS = -g -O2

INCLUDES = -I$(top_srcdir)/utils/share -I$(top_srcdir)/utils/debugger -I$(top_srcdir)/utils/profile/ -Icommon/ -Imach -I$(top_srcdir)/utils/debugger -I$(top_srcdir)/device/uart -I$(top_srcdir)/device/flash @COMMON_INCLUDES@
#INCLUDES = -I./common -I$(top_srcdir)/utils/config/ -I$(top_srcdir)/utils/profile/ -I$(top_srcdir)/utils/debugger -I$(top_srcdir)/device/uart @COMMON_INCLUDES@

