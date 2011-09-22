/* Copyright (C) 2007-2008 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/
//#include "android/android.h"
//#include "android/utils/debug.h"
//#include "android/utils/duff.h"
#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h>
#include <skyeye_attr.h>
#include <memory_space.h>
#define DEBUG
#include <skyeye_log.h>

#include "lcd_s3c6410.h"
#include "regs-fb.h"
#include "regs-fb-v4.h"

/* These values *must* match the platform definitions found under
 * hardware/libhardware/include/hardware/hardware.h
 */
enum {
    HAL_PIXEL_FORMAT_RGBA_8888          = 1,
    HAL_PIXEL_FORMAT_RGBX_8888          = 2,
    HAL_PIXEL_FORMAT_RGB_888            = 3,
    HAL_PIXEL_FORMAT_RGB_565            = 4,
    HAL_PIXEL_FORMAT_BGRA_8888          = 5,
    HAL_PIXEL_FORMAT_RGBA_5551          = 6,
    HAL_PIXEL_FORMAT_RGBA_4444          = 7,
};

enum {
    FB_GET_WIDTH        = 0x00,
    FB_GET_HEIGHT       = 0x04,
    FB_INT_STATUS       = 0x08,
    FB_INT_ENABLE       = 0x0c,
    FB_SET_BASE         = 0x10,
    FB_SET_ROTATION     = 0x14,
    FB_SET_BLANK        = 0x18,
    FB_GET_PHYS_WIDTH   = 0x1c,
    FB_GET_PHYS_HEIGHT  = 0x20,
    FB_GET_FORMAT       = 0x24,

    FB_INT_VSYNC             = 1U << 0,
    FB_INT_BASE_UPDATE_DONE  = 1U << 1
};

/* Type used to record a mapping from display surface pixel format to
 * HAL pixel format */
typedef struct {
    int    pixel_format; /* HAL pixel format */
    uint8_t bits;
    uint8_t bytes;
    uint32_t rmask, gmask, bmask, amask;
} FbConfig;
static int s3c6410_fb_get_bytes_per_pixel(struct s3c6410_fb_device *s)
{
#if 0
    if (s->fb->pixel_format < 0) {
//        (void) s3c6410_fb_get_pixel_format(s);
    }
    return s->fb->bytes_per_pixel;
#endif
}

static int
pixels_to_mm(int  pixels, int dpi)
{
    /* dpi = dots / inch
    ** inch = dots / dpi
    ** mm / 25.4 = dots / dpi
    ** mm = (dots * 25.4)/dpi
    */
    return (int)(0.5 + 25.4 * pixels  / dpi);
}


#define  STATS  0

#if STATS
static int   stats_counter;
static long  stats_total;
static int   stats_full_updates;
static long  stats_total_full_updates;
#endif

/* This structure is used to hold the inputs for
 * compute_fb_update_rect_linear below.
 * This corresponds to the source framebuffer and destination
 * surface pixel buffers.
 */
typedef struct {
    int            width;
    int            height;
    int            bytes_per_pixel;
    const uint8_t* src_pixels;
    int            src_pitch;
    uint8_t*       dst_pixels;
    int            dst_pitch;
} FbUpdateState;
static exception_t s3c6410_fb_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	uint32_t ret;
	struct s3c6410_fb_device *dev = opaque->obj;
	fb_reg_t* regs = dev->regs;
	DBG("In %s, offset=0x%x\n", __FUNCTION__, offset);
	switch(offset) {
        case VIDCON0:
		regs->vidcon[0];
		return ret;

        case VIDCON1:
		regs->vidcon[1];
		return ret;

        case VIDCON2:
		regs->vidcon[2];
		return ret;
	default:
		printf("Can not read the register at 0x%x\n", offset);
		return 0;
	}
}

//static void s3c6410_fb_write(void *opaque, target_phys_addr_t offset,
 //                       uint32_t val)
static exception_t s3c6410_fb_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct s3c6410_fb_device *dev = opaque->obj;
	fb_reg_t* regs = dev->regs;

	uint32_t data = *(uint32_t*)buf;
	DBG("In %s, offset=0x%x, data=0x%x\n", __FUNCTION__, offset, data);
	switch(offset) {
	case VIDCON0:
		regs->vidcon[0] = data;
		break;

        case VIDCON1: 
		regs->vidcon[1] = data;
		break;
	case VIDTCON0:
		regs->vidtcon[0] = data;
		break;
	case VIDTCON1:
		regs->vidtcon[1] = data;
		break;
	case VIDTCON2:
		regs->vidtcon[2] = data;
		break;
	case WINCON(0):
		regs->wincon[0] = data;
		if(data & WINCONx_ENWIN){
		/* Enable the window */
			
		}
		else{
		/* Disable the window */
		}
		int bpp = (data & WINCON0_BPPMODE_MASK) >> WINCON0_BPPMODE_SHIFT;
		if(bpp == WINCON0_BPPMODE_1BPP){
		}
		break;
	case WINCON(1):
		regs->wincon[1] = data;
		break;
	case WINCON(2):
		regs->wincon[2] = data;
		break;
	case WINCON(3):
		regs->wincon[3] = data;
		break;
	case WINCON(4):
		regs->wincon[4] = data;
		break;

	case VIDOSD_BASE:
		regs->vidosd[0][0] = data;
		break;
	case (VIDOSD_BASE + 4):
		regs->vidosd[0][1] = data;
		break;
	case (VIDOSD_BASE + 8):
		regs->vidosd[0][2] = data;
		break;
	case (VIDOSD_BASE + 0x10):
		regs->vidosd[1][0] = data;
		break;
	case (VIDOSD_BASE + 0x14):
		regs->vidosd[1][1] = data;
		break;
	case (VIDOSD_BASE + 0x18):
		regs->vidosd[1][2] = data;
		break;
	case (VIDOSD_BASE + 0x20):
		regs->vidosd[2][0] = data;
		break;
	case (VIDOSD_BASE + 0x24):
		regs->vidosd[2][1] = data;
		break;
	case (VIDOSD_BASE + 0x28):
		regs->vidosd[2][2] = data;
		break;
	case (VIDOSD_BASE + 0x30):
		regs->vidosd[3][0] = data;
		break;
	case (VIDOSD_BASE + 0x34):
		regs->vidosd[3][1] = data;
		break;
	case (VIDOSD_BASE + 0x38):
		regs->vidosd[3][2] = data;
		break;
	case (VIDOSD_BASE + 0x40):
		regs->vidosd[4][0] = data;
		break;
	case (VIDOSD_BASE + 0x44):
		regs->vidosd[4][1] = data;
		break;
	case (VIDOSD_BASE + 0x48):
		regs->vidosd[4][2] = data;
		break;
	case 0xa0:
		regs->vidw00add0b0 = data;
		break;
	case 0xd0:
		regs->vidw00add1b0 = data;
		break;
	case 0x100:
		regs->vidw_buf_size[0] = data;
		break;
	case 0x180:
		regs->winmap[0] = data;
		break;
	default:
		if(offset >= 0x140 && offset <= 0x15c){
			regs->wkeycon[((offset - 0x140) / 4)] = data;
			break;
		}

		printf("Can not read the register at 0x%x\n", offset);
		return Invarg_exp;
            //cpu_abort (cpu_single_env, "s3c6410_fb_write: Bad offset %x\n", offset);
	}
	return No_exp;
}
static conf_object_t* new_s3c6410_lcd(char* obj_name){
	s3c6410_fb_device* dev = skyeye_mm_zero(sizeof(s3c6410_fb_device));
	dev->obj = new_conf_object(obj_name, dev);
	fb_state_t* state =  skyeye_mm_zero(sizeof(fb_state_t));
	dev->state = state;
	fb_reg_t* regs = skyeye_mm_zero(sizeof(fb_reg_t));
	dev->regs = regs;
	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->read = s3c6410_fb_read;
	io_memory->write = s3c6410_fb_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);

	dev->lcd_ctrl = make_new_attr(Val_ptr);
	SKY_register_attr(dev->obj, "lcd_ctrl_0", dev->lcd_ctrl);

	return dev->obj;
}
void free_s3c6410_lcd(conf_object_t* dev){
	
}
void init_s3c6410_lcd(){
	static skyeye_class_t class_data = {
		.class_name = "s3c6410_lcd",
		.class_desc = "s3c6410 lcd",
		.new_instance = new_s3c6410_lcd,
		.free_instance = free_s3c6410_lcd,
		.get_attr = NULL,
		.set_attr = NULL
	};
	
	SKY_register_class(class_data.class_name, &class_data);
}
