#ifndef __LCD_S3C6410_H__
#define __LCD_S3C6410_H__
struct s3c6410_fb_device;
typedef struct fb_state {
	struct s3c6410_fb_device* dev;
	//    DisplayState*  ds;
	int      pixel_format;
	int      bytes_per_pixel;
	uint32_t fb_base;
	uint32_t base_valid : 1;
	uint32_t need_update : 1;
	uint32_t need_int : 1;
	uint32_t set_rotation : 2;
	uint32_t blank : 1;
	uint32_t int_status;
	uint32_t int_enable;
	int      rotation;   /* 0, 1, 2 or 3 */
	int      dpi;
}fb_state_t;
typedef struct s3c6410_fb_device{
	conf_object_t* obj;
	int line_no;
	fb_state_t* fb;
	conf_object_t* signal_target;
	general_signal_intf* master;
	memory_space_intf* io_memory;
}s3c6410_fb_device;
#endif
