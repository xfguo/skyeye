#dev_uart = uart/skyeye_uart.c          uart/skyeye_uart_net.c   uart/skyeye_uart_stdio.c uart/uart_module.c \
#uart/skyeye_uart_cvt_dcc.c  uart/skyeye_uart_pipe.c
dev_uart = uart/skyeye_uart.c          uart/skyeye_uart_net.c   uart/skyeye_uart_stdio.c uart/uart_module.c \
uart/skyeye_uart_pipe.c uart/skyeye_uart_term.c

dev_uart_16550 = uart_16550/uart_16550.c uart_16550/uart_16550_module.c
dev_lcd_s3c6410 = lcd_s3c6410/lcd_s3c6410.c lcd_s3c6410/lcd_s3c6410_module.c
dev_sdhci_s3c6410 = sdhci_s3c6410/sdhci_s3c6410.c sdhci_s3c6410/sdhci_s3c6410_module.c

#dev_flash = flash/armflash.c  flash/dev_flash_sst39lvf160.c flash/flash_module.c\
#flash/dev_flash_am29.c  flash/dev_flash_intel.c  flash/skyeye_flash.c
dev_flash = flash/dev_flash_sst39lvf160.c flash/flash_module.c\
flash/dev_flash_am29.c  flash/dev_flash_intel.c  flash/skyeye_flash.c

dev_nandflash = nandflash/dev_nandflash_s3c2410.c  nandflash/nandflash_smallblock.c  nandflash/skyeye_nandflash.c nandflash/nandflash_module.c

#dev_sound = sound/dev_sound_s3c44b0x.c  sound/skyeye_sound.c  sound/skyeye_sound_pcm.c sound/sound_module.c
dev_sound = sound/skyeye_sound.c  sound/skyeye_sound_pcm.c sound/sound_module.c

dev_lcd = lcd/dev_lcd_au1100.c  lcd/dev_lcd_s3c2410.c   lcd/skyeye_lcd.c \
lcd/dev_lcd_ep7312.c  lcd/dev_lcd_s3c44b0x.c lcd/dev_lcd_pxa.c lcd/lcd_module.c

gtk_lcd = lcd_gtk/lcd_gtk_module.c lcd_gtk/lcd_gtk.c

#dev_net = net/dev_net_cs8900a.c  net/dev_net_s3c4510b.c net/skyeye_net_tuntap.c \
#net/dev_net_rtl8019.c  net/skyeye_net.c    net/skyeye_net_vnet.c net/net_module.c
#dev_net = net/dev_net_cs8900a.c  net/skyeye_net_tuntap.c net/dev_net_s3c4510b.c

dev_net = net/dev_net_cs8900a.c  net/skyeye_net_tuntap.c \
net/dev_net_rtl8019.c  net/skyeye_net.c    net/skyeye_net_vnet.c net/net_module.c   net/skyeye_net_vhub.c

dev_ts = touchscreen/dev_touchscreen_skyeye.c  touchscreen/skyeye_touchscreen.c touchscreen/ts_module.c
dev_pci = pci/pci_bus/pci.c  pci/pci_bus/pci_module.c
#libdev_a_SOURCES = skyeye_device.c $(dev_uart) $(dev_flash) $(dev_nandflash) $(dev_sound) $(dev_net) $(dev_ts)
libuart_la_SOURCES = $(dev_uart)
libuart_la_LDFLAGS = -module

libuart_16550_la_SOURCES = $(dev_uart_16550)
libuart_16550_la_LDFLAGS = -module

liblcd_s3c6410_la_SOURCES = $(dev_lcd_s3c6410)
liblcd_s3c6410_la_LDFLAGS = -module

if GTK_LCD
libgtk_lcd_la_SOURCES = $(gtk_lcd) 
libgtk_lcd_la_LDFLAGS = -module $(LCD_LIBS)
endif

libsdhci_s3c6410_la_SOURCES = $(dev_sdhci_s3c6410)
libsdhci_s3c6410_la_LDFLAGS = -module

libflash_la_SOURCES = $(dev_flash)
libflash_la_LDFLAGS = -module

libnandflash_la_SOURCES = $(dev_nandflash)
libnandflash_la_LDFLAGS = -module

libsound_la_SOURCES = $(dev_sound)
libsound_la_LDFLAGS = -module

liblcd_la_SOURCES = $(dev_lcd)
liblcd_la_LDFLAGS = -module $(LCD_LIBS)

libnet_la_SOURCES = $(dev_net)
libnet_la_LDFLAGS = -module

libts_la_SOURCES = $(dev_ts)
libts_la_LDFLAGS = -module

libpci_la_SOURCES = $(dev_pci)
libpci_la_LDFLAGS = -module

pkglib_LTLIBRARIES = libuart.la libuart_16550.la libflash.la libnandflash.la libsound.la liblcd.la libnet.la libts.la libpci.la

if WIN32
libuart_la_LDFLAGS +=  -no-undefined -shared -module -Xlinker -L../common/.libs/ -L/mingw/lib -Xlinker -lcommon -Xlinker --export-all-symbol
else
pkglib_LTLIBRARIES += liblcd_s3c6410.la libsdhci_s3c6410.la
endif

if GTK_LCD
pkglib_LTLIBRARIES += libgtk_lcd.la
endif


DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0  @COMMON_CFLAGS@

skyeye_bin_dir = \"$(prefix)/bin/\"
INCLUDES = -I./ @COMMON_INCLUDES@ -DSKYEYE_BIN=$(skyeye_bin_dir)

INCLUDES += @LCD_CFLAGS@
if GTK_LCD
liblcd_la_SOURCES += lcd/skyeye_lcd_gtk.c
endif # GTK_LCD
if WIN32_LCD
liblcd_la_SOURCES += lcd/skyeye_lcd_win32.c
endif # WIN32_LCD
