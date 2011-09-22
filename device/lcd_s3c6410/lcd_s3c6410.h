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
typedef struct fb_reg{
	uint32 vidcon[3];
	uint32 vidtcon[3];
	uint32 wincon[5];
	uint32 vidosd[5][3];
	uint32 vidw00add0b0;
	uint32 vidw00add0b1;
	uint32 vidw01add0b0;
	uint32 vidw01add0b1;
	uint32 vidw02add0;
	uint32 vidw03add0;
	uint32 vidw04add0;
	uint32 vidw00add1b0;
	uint32 vidw00add1b1;
	uint32 vidw_buf_size[5];
	uint32 vidintcon0;
	uint32 vidintcon1;
	uint32 wkeycon[8];
	uint32 winmap[5];
	uint32 wpalcon;
	uint32 tgigcon;
	uint32 ituifcon0;
	uint32 sifccon[3];
	uint32 ldi_cmd[12];
	uint32 w2pdata[8];
	uint32 w3pdata[8];
	uint32 w4pdata[2];
}fb_reg_t;
typedef struct s3c6410_fb_device{
	conf_object_t* obj;
	fb_reg_t* regs;
	int line_no;
	fb_state_t* state;
	conf_object_t* signal_target;
	general_signal_intf* master;
	attr_value_t* lcd_ctrl;
}s3c6410_fb_device;
#endif
