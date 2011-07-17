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

//#define DEBUG
#include <skyeye_log.h>

#include "skyeye_mach_goldfish.h"
#include <stdio.h>
enum {
	INTERRUPT_STATUS        = 0x00, // number of pending interrupts
	INTERRUPT_NUMBER        = 0x04,
	INTERRUPT_DISABLE_ALL   = 0x08,
	INTERRUPT_DISABLE       = 0x0c,
	INTERRUPT_ENABLE        = 0x10
};


#define  GOLDFISH_INT_SAVE_VERSION  1


static void goldfish_pic_signal(pic_state_t* s)
{
	uint32_t flags;
	flags = (s->level & s->irq_enabled);
	interrupt_signal_t interrupt_signal;
	interrupt_signal.arm_signal.irq = (flags != 0)? Low_level : High_level;
	//interrupt_signal.arm_signal.firq = (flags != 0) ? Low_level : High_level;
	interrupt_signal.arm_signal.firq = Prev_level;
	interrupt_signal.arm_signal.reset = Prev_level;
	DBG("In %s, interrupt triggered, flags=%d, irq=%d.\n", __FUNCTION__, flags, interrupt_signal.arm_signal.irq);
	send_signal(&interrupt_signal);
	return;
}

static exception_t goldfish_pic_raise(conf_object_t *opaque, int line_no)
{
	goldfish_pic_device* dev = (goldfish_pic_device*)(opaque->obj); 
	pic_state_t *s = (pic_state_t *)(dev->state);

	uint32_t mask = (1U << line_no);
	if(!(s->level & mask)) {
		if(s->irq_enabled & mask){
			s->pending_count++;
			s->level |= mask;
			DBG("In %s, interrupt triggered.s->pending_count=0x%x, s->level=0x%x\n", __FUNCTION__, s->pending_count, s->level);
			goldfish_pic_signal(s);
		}
	}
	//DBG("In %s, s->level=0x%x, s->irq_enabled=0x%x, mask=0x%x\n", __FUNCTION__, s->level, s->irq_enabled, mask);
#if 0
	if(level) {
		if(!(s->level & mask)) {
			if(s->irq_enabled & mask)
				s->pending_count++;
			s->level |= mask;
		}
	}
	else {
		if(s->level & mask) {
			if(s->irq_enabled & mask)
				s->pending_count--;
			s->level &= ~mask;
		}
	}
#endif
	return No_exp;
}

static exception_t goldfish_pic_lower(conf_object_t *opaque, int line_no)
{
	goldfish_pic_device* dev = (goldfish_pic_device*)(opaque->obj); 
	pic_state_t *s = (pic_state_t *)(dev->state);

	uint32_t mask = (1U << line_no);
	DBG("In %s, interrupt cleared. mask=0x%x, s->level=0x%x, s->pending_count->=0x%x\n", __FUNCTION__, mask, s->level, s->pending_count);
	if(s->level & mask) {
		//if(s->irq_enabled & mask){
		if(s->pending_count > 0)
			s->pending_count--;
		s->level &= ~mask;
		DBG("In %s, interrupt cleared. s->level=0x%x\n", __FUNCTION__, s->level);
	
		//}
		goldfish_pic_signal(s);
		
	}
	return No_exp;
}


static exception_t pic_read(conf_object_t* opaque, generic_address_t offset, void* buf, size_t count)
{
	goldfish_pic_device* dev = (goldfish_pic_device*)(opaque->obj); 
	pic_state_t *s = (pic_state_t *)(dev->state);
	uint32 data;
	switch (offset) {
		case INTERRUPT_STATUS: /* IRQ_STATUS */
			data = s->pending_count;
			break;
		case INTERRUPT_NUMBER: {
			int i;
			uint32_t pending = s->level & s->irq_enabled;
			for(i = 0; i < 32; i++) {
				if(pending & (1U << i)){
					data = i;
					break;
				}
			}
			break;
		}
		default:
			//cpu_abort (cpu_single_env, "goldfish_int_read: Bad offset %x\n", offset);
			return Invarg_exp;
	}
	DBG("In %s, offset=0x%x, data=0x%x\n", __FUNCTION__, offset, data);
	*(uint32 *)buf = data;
	return No_exp;
}

static exception_t pic_write(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	goldfish_pic_device* dev = (goldfish_pic_device*)(opaque->obj); 
	pic_state_t *s = (pic_state_t *)(dev->state);
	uint32 value = *(uint32 *)buf;
	uint32_t mask = (1U << value);

	DBG("In %s, offset=0x%x, value=0x%x\n", __FUNCTION__, offset, value);
	DBG("In %s, s->irq_enabled=0x%x, s->pending_count=0x%x, s->level=0x%x\n", __FUNCTION__, s->irq_enabled, s->pending_count, s->level);
	switch (offset) {
		case INTERRUPT_DISABLE_ALL:
			s->pending_count = 0;
			s->level = 0;
			break;

		case INTERRUPT_DISABLE://0xc
			if(s->irq_enabled & mask) {
				if(s->level & mask)
					if(s->pending_count > 0)
						s->pending_count--;
				s->irq_enabled &= ~mask;
			}
			break;
		case INTERRUPT_ENABLE:
			if(!(s->irq_enabled & mask)) {
				s->irq_enabled |= mask;
				if(s->level & mask)
					s->pending_count++;
			}
			break;
		default:
        //cpu_abort (cpu_single_env, "goldfish_int_write: Bad offset %x\n", offset);
			return Invarg_exp;
	}
	//goldfish_int_update(s);
	return No_exp;
}

goldfish_pic_device* new_goldfish_pic_device(char* obj_name){
	goldfish_pic_device* dev = (goldfish_pic_device *)skyeye_mm_zero(sizeof(goldfish_pic_device));
	if(dev == NULL){
		fprintf(stderr, "MM failed in %s\n", __FUNCTION__);
		return NULL;
	}
	dev->obj = new_conf_object(obj_name, dev);

	dev->slave = skyeye_mm_zero(sizeof(general_signal_intf));
	dev->slave->raise_signal = goldfish_pic_raise;
	dev->slave->lower_signal = goldfish_pic_lower;

	dev->io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	dev->io_memory->read = pic_read;
	dev->io_memory->write = pic_write;

	dev->state = skyeye_mm_zero(sizeof(pic_state_t));
	return dev;
}
void del_goldfish_pic_device(char* obj_name){
}
#if 0
qemu_irq*  goldfish_interrupt_init(uint32_t base, qemu_irq parent_irq, qemu_irq parent_fiq)
{
    int ret;
    struct pic_state_t *s;
    qemu_irq*  qi;

    s = qemu_mallocz(sizeof(*s));
    qi = qemu_allocate_irqs(goldfish_int_set_irq, s, 32);
    s->dev.name = "goldfish_interrupt_controller";
    s->dev.id = -1;
    s->dev.base = base;
    s->dev.size = 0x1000;
    s->parent_irq = parent_irq;
    s->parent_fiq = parent_fiq;

    ret = goldfish_device_add(&s->dev, goldfish_int_readfn, goldfish_int_writefn, s);
    if(ret) {
        qemu_free(s);
        return NULL;
    }

    register_savevm( "goldfish_int", 0, GOLDFISH_INT_SAVE_VERSION,
                     goldfish_int_save, goldfish_int_load, s);

    return qi;
}
#endif
