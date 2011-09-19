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
    if (s->fb->pixel_format < 0) {
//        (void) s3c6410_fb_get_pixel_format(s);
    }
    return s->fb->bytes_per_pixel;
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
	struct s3c6410_fb_device *s = opaque;
	DBG("In %s, offset=0x%x\n", __FUNCTION__, offset);
	switch(offset) {
        case VIDCON0:
		return ret;

        case VIDCON1:
		return ret;

        case VIDCON2:
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
    struct s3c6410_fb_device *s = opaque;

	uint32_t val = *(uint32_t*)buf;
    switch(offset) {
        case FB_INT_ENABLE:
            s->fb->int_enable = val;
            //s3c6410_device_set_irq(&s->fb->dev, 0, (s->fb->int_status & s->fb->int_enable));
            break;
        case FB_SET_BASE: {
            int need_resize = !s->fb->base_valid;
            s->fb->fb_base = val;
            s->fb->int_status &= ~FB_INT_BASE_UPDATE_DONE;
            s->fb->need_update = 1;
            s->fb->need_int = 1;
            s->fb->base_valid = 1;
            if(s->fb->set_rotation != s->fb->rotation) {
                //printf("FB_SET_BASE: rotation : %d => %d\n", s->fb->rotation, s->fb->set_rotation);
                s->fb->rotation = s->fb->set_rotation;
                need_resize = 1;
            }
            //s3c6410_device_set_irq(&s->fb->dev, 0, (s->fb->int_status & s->fb->int_enable));
            if (need_resize) {
                //printf("FB_SET_BASE: need resize (rotation=%d)\n", s->fb->rotation );
               // dpy_resize(s->fb->ds);
            }
            } break;
        case FB_SET_ROTATION:
            //printf( "FB_SET_ROTATION %d\n", val);
            s->fb->set_rotation = val;
            break;
        case FB_SET_BLANK:
            s->fb->blank = val;
            s->fb->need_update = 1;
            break;
        default:
            break;
            //cpu_abort (cpu_single_env, "s3c6410_fb_write: Bad offset %x\n", offset);
    }
}
static conf_object_t* new_s3c6410_lcd(char* obj_name){
	s3c6410_fb_device* dev = skyeye_mm_zero(sizeof(s3c6410_fb_device));
	dev->obj = new_conf_object(obj_name, dev);
	fb_state_t* fb =  skyeye_mm_zero(sizeof(fb_state_t));
	fb->dev = dev;

	dev->fb = fb;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->read = s3c6410_fb_read;
	io_memory->write = s3c6410_fb_write;
	SKY_register_interface(dev->obj, obj_name, MEMORY_SPACE_INTF_NAME);	
	return dev;
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
