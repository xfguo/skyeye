x86_mach = mach/skyeye_mach_pc.cc
x86_cpu = cpu/3dnow.cc          cpu/data_xfer8.cc     cpu/proc_ctrl.cc \
cpu/access32.cc       cpu/debugstuff.cc     cpu/protect_ctrl.cc	\
cpu/access64.cc       cpu/exception.cc      cpu/resolver.cc	\
cpu/access.cc         cpu/fetchdecode64.cc  cpu/ret_far.cc	\
cpu/aes.cc            cpu/fetchdecode.cc    cpu/segment_ctrl.cc	\
cpu/apic.cc           cpu/flag_ctrl.cc      cpu/segment_ctrl_pro.cc	\
cpu/arith16.cc        cpu/flag_ctrl_pro.cc  cpu/shift16.cc	\
cpu/arith32.cc        cpu/fpu_emu.cc        cpu/shift32.cc	\
cpu/arith64.cc        cpu/icache.cc         cpu/shift64.cc	\
cpu/arith8.cc         cpu/init.cc           cpu/shift8.cc	\
cpu/bcd.cc            cpu/io.cc             cpu/smm.cc	\
cpu/bit16.cc          cpu/iret.cc           cpu/soft_int.cc	\
cpu/bit32.cc          cpu/jmp_far.cc        cpu/sse.cc		\
cpu/bit64.cc          cpu/lazy_flags.cc     cpu/sse_move.cc	\
cpu/bit.cc            cpu/load32.cc         cpu/sse_pfp.cc	\
cpu/call_far.cc       cpu/logical16.cc      cpu/sse_rcp.cc	\
cpu/cpu.cc            cpu/logical32.cc      cpu/sse_string.cc	\
cpu/cpuid.cc          cpu/logical64.cc      cpu/stack16.cc	\
cpu/crc32.cc          cpu/logical8.cc       cpu/stack32.cc	\
cpu/ctrl_xfer16.cc    cpu/mmx.cc            cpu/stack64.cc	\
cpu/ctrl_xfer32.cc    cpu/msr.cc            cpu/string.cc	\
cpu/ctrl_xfer64.cc    cpu/mult16.cc         cpu/tasking.cc	\
cpu/ctrl_xfer_pro.cc  cpu/mult32.cc         cpu/vm8086.cc	\
cpu/data_xfer16.cc    cpu/mult64.cc         cpu/vmexit.cc	\
cpu/data_xfer32.cc    cpu/mult8.cc          cpu/vmx.cc		\
cpu/paging.cc cpu/xsave.cc

x86_fpu = fpu/f2xm1.cc fpu/fprem.cc fpu/fpu_compare.cc  fpu/fpu_misc.cc   fpu/fsincos.cc  fpu/softfloat.cc fpu/softfloatx80.cc fpu/ferr.cc    fpu/fpu_arith.cc  fpu/fpu_const.cc       fpu/fpu_tags.cc   fpu/fyl2x.cc    fpu/softfloat-round-pack.cc fpu/fpatan.cc  fpu/fpu.cc        fpu/fpu_load_store.cc  fpu/fpu_trans.cc  fpu/poly.cc     fpu/softfloat-specialize.cc

x86_common = common/x86_arch_interface.cc common/logio.cc common/pc_system.cc common/x86_module.cc common/main.cc common/config.cc common/plugin.cc common/osdep.cc

x86_memory = memory/memory.cc memory/misc_mem.cc

x86_iodev = iodev/acpi.cc             iodev/extfpuirq.cc   iodev/scancodes.cc\
iodev/biosdev.cc          iodev/floppy.cc      iodev/scsi_device.cc\
iodev/busmouse.cc         iodev/gameport.cc    iodev/serial.cc\
   iodev/serial_raw.cc\
  iodev/harddrv.cc     iodev/slowdown_timer.cc\
iodev/cdrom.cc            iodev/hdimage.cc     iodev/soundlnx.cc\
iodev/cmos.cc             iodev/ioapic.cc      iodev/soundosx.cc\
   iodev/iodebug.cc     iodev/soundwin.cc\
iodev/devices.cc          iodev/keyboard.cc    iodev/speaker.cc\
iodev/dma.cc              iodev/ne2k.cc        iodev/svga_cirrus.cc\
iodev/eth_arpback.cc      iodev/parallel.cc    iodev/unmapped.cc\
iodev/eth.cc              iodev/pci2isa.cc     iodev/usb_common.cc\
iodev/eth_fbsd.cc         iodev/pci.cc         iodev/usb_hid.cc\
iodev/eth_linux.cc        iodev/pcidev.cc      iodev/usb_hub.cc\
iodev/eth_null.cc         iodev/pci_ide.cc     iodev/usb_msd.cc\
iodev/eth_packetmaker.cc  iodev/pcipnic.cc     iodev/usb_ohci.cc\
iodev/eth_tap.cc          iodev/pcivga.cc      iodev/usb_uhci.cc\
iodev/eth_tuntap.cc       iodev/pic.cc         iodev/vga.cc\
iodev/eth_vde.cc          iodev/pit82c54.cc    iodev/virt_timer.cc\
iodev/eth_vnet.cc         iodev/pit_wrap.cc    iodev/vmware3.cc\
iodev/eth_win32.cc        iodev/sb16.cc        iodev/vmware4.cc

x86_gui = gui/enh_dbg.cc  gui/keymap.cc        gui/textconfig.cc \
gui/gui.cc      gui/siminterface.cc 

if WIN32_LCD
x86_gui +=  gui/win32dialog.cc gui/win32.cc
else
x86_gui += gui/x.cc
endif

x86_disasm = disasm/dis_decode.cc  disasm/resolve.cc \
disasm/dis_groups.cc  disasm/syntax.cc

##
libx86_la_SOURCES =  $(x86_cpu) $(x86_fpu) $(x86_common) $(x86_memory) $(x86_mach) $(x86_iodev) $(x86_gui) $(x86_disasm)
libx86_la_LDFLAGS = -lXpm -lstdc++ -module
pkglib_LTLIBRARIES = libx86.la
DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0
INCLUDES = -I$(srcdir)/common -I$(srcdir)/cpu -I$(srcdir)/include -I$(srcdir)/fpu -I$(srcdir)/ -I$(srcdir)/memory -I$(srcdir)/instrument/stubs -I$(srcdir)/iodev/  @COMMON_INCLUDES@
