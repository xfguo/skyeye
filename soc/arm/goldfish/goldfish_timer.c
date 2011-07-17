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
#include <skyeye_types.h>
#include <skyeye_sched.h>
#include "skyeye_mach_goldfish.h"

//#define DEBUG
#include <skyeye_log.h>

enum {
    TIMER_TIME_LOW          = 0x00, // get low bits of current time and update TIMER_TIME_HIGH
    TIMER_TIME_HIGH         = 0x04, // get high bits of time at last TIMER_TIME_LOW read
    TIMER_ALARM_LOW         = 0x08, // set low bits of alarm and activate it
    TIMER_ALARM_HIGH        = 0x0c, // set high bits of next alarm
    TIMER_CLEAR_INTERRUPT   = 0x10,
    TIMER_CLEAR_ALARM       = 0x14
};


#define  GOLDFISH_TIMER_SAVE_VERSION  1
#if 0
static void  goldfish_timer_save(QEMUFile*  f, void*  opaque)
{
    struct timer_state*  s   = opaque;

    qemu_put_be64(f, s->now_ns);  /* in case the kernel is in the middle of a timer read */
    qemu_put_byte(f, s->armed);
    if (s->armed) {
        int64_t  now_ns   = qemu_get_clock_ns(vm_clock);
        int64_t  alarm_ns = (s->alarm_low_ns | (int64_t)s->alarm_high_ns << 32);
        qemu_put_be64(f, alarm_ns - now_ns);
    }
}

static int  goldfish_timer_load(QEMUFile*  f, void*  opaque, int  version_id)
{
    struct timer_state*  s   = opaque;

    if (version_id != GOLDFISH_TIMER_SAVE_VERSION)
        return -1;

    s->now_ns = qemu_get_be64(f);
    s->armed  = qemu_get_byte(f);
    if (s->armed) {
        int64_t  now_tks   = qemu_get_clock(vm_clock);
        int64_t  diff_tks  = qemu_get_be64(f);
        int64_t  alarm_tks = now_tks + diff_tks;

        if (alarm_tks <= now_tks) {
            goldfish_device_set_irq(&s->dev, 0, 1);
            s->armed = 0;
        } else {
            qemu_mod_timer(s->timer, alarm_tks);
        }
    }
    return 0;
}
#endif
static exception_t timer_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	goldfish_timer_device *dev = (goldfish_timer_device*)(opaque->obj);
	timer_state_t* s = dev->timer;
	switch(offset) {
        case TIMER_TIME_LOW:
		s->now_us = get_clock_us();
		*(uint32 *)buf = s->now_us;
		break;
        case TIMER_TIME_HIGH:
		*(uint32 *)buf = s->now_us >> 32;
		break;
        default:
            //cpu_abort (cpu_single_env, "goldfish_timer_read: Bad offset %x\n", offset);
		return Invarg_exp;
	}
	DBG("In %s, offset=0x%x, *buf=0x%x\n", __FUNCTION__, offset, *(uint32 *)buf);
	return No_exp;
}

static exception_t timer_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	goldfish_timer_device *dev = (goldfish_timer_device*)(opaque->obj);
	timer_state_t* s = dev->timer;
	uint32_t value_us = *(uint32_t*)buf;
	DBG("In %s, write offset=0x%x, value_us=0x%x\n", __FUNCTION__, offset, value_us);
	int64_t alarm_us, now_us;
	switch(offset) {
		case TIMER_ALARM_LOW:
			/* Change the ns unit to the us unit */
			//value_us >>= 3;
			s->alarm_low_us = value_us;
			//alarm_us = (s->alarm_low_us | (int64_t)s->alarm_high_us << 32);
			alarm_us = (s->alarm_low_us | (int64_t)s->alarm_high_us << 32) + value_us;
			now_us   = get_clock_us();
			DBG("In %s, now_us = 0x%llx, alarm_us=0x%x, value_us=0x%x\n", __FUNCTION__, now_us, alarm_us, value_us);
			if (alarm_us <= now_us) {
				assert(dev->signal_target != NULL);
				assert(dev->master != NULL);
				dev->master->raise_signal(dev->signal_target, dev->line_no);
        	        //goldfish_device_set_irq(&s->dev, 0, 1);
			} else {
				mod_thread_scheduler(s->id, value_us - now_us, Periodic_sched);
				//mod_thread_scheduler(s->id, alarm_us, Periodic_sched);
				s->armed = 1;
			}
			break;
		case TIMER_ALARM_HIGH:
			s->alarm_high_us = value_us;
			break;
		case TIMER_CLEAR_ALARM:
		//qemu_del_timer(s->timer);
			//del_thread_scheduler(s->id);
			s->armed = 0;
		/* fall through */
		case TIMER_CLEAR_INTERRUPT:
			dev->master->lower_signal(dev->signal_target, dev->line_no);
			break;
		default:
            //cpu_abort (cpu_single_env, "goldfish_timer_write: Bad offset %x\n", offset);
			return Invarg_exp;
	}
	return No_exp;
}

static void goldfish_timer_tick(conf_object_t *opaque)
{
	goldfish_timer_device *dev = (goldfish_timer_device*)(opaque->obj);
	//DBG("In %s\n", __FUNCTION__);
	if(dev->master != NULL){
		DBG("In %s, timer_tick triggered.\n", __FUNCTION__);
		dev->master->raise_signal(dev->signal_target, dev->line_no);
	}

}

struct rtc_state {
    //struct goldfish_device dev;
    uint32_t alarm_low;
    int32_t alarm_high;
    int64_t now;
};

/* we save the RTC for the case where the kernel is in the middle of a rtc_read
 * (i.e. it has read the low 32-bit of s->now, but not the high 32-bits yet */
#define  GOLDFISH_RTC_SAVE_VERSION  1
#if 0
static void  goldfish_rtc_save(QEMUFile*  f, void*  opaque)
{
    struct rtc_state*  s = opaque;

    qemu_put_be64(f, s->now);
}

static int  goldfish_rtc_load(QEMUFile*  f, void*  opaque, int  version_id)
{
    struct  rtc_state*  s = opaque;

    if (version_id != GOLDFISH_RTC_SAVE_VERSION)
        return -1;

    /* this is an old value that is not correct. but that's ok anyway */
    s->now = qemu_get_be64(f);
    return 0;
}
static uint32_t goldfish_rtc_read(void *opaque, target_phys_addr_t offset)
{
    struct rtc_state *s = (struct rtc_state *)opaque;
    switch(offset) {
        case 0x0:
            s->now = (int64_t)time(NULL) * 1000000000;
            return s->now;
        case 0x4:
            return s->now >> 32;
        default:
            cpu_abort (cpu_single_env, "goldfish_rtc_read: Bad offset %x\n", offset);
            return 0;
    }
}

static void goldfish_rtc_write(void *opaque, target_phys_addr_t offset, uint32_t value)
{
    struct rtc_state *s = (struct rtc_state *)opaque;
    int64_t alarm;
    switch(offset) {
        case 0x8:
            s->alarm_low = value;
            alarm = s->alarm_low | (int64_t)s->alarm_high << 32;
            //printf("next alarm at %lld, tps %lld\n", alarm, ticks_per_sec);
            //qemu_mod_timer(s->timer, alarm);
            break;
        case 0xc:
            s->alarm_high = value;
            //printf("alarm_high %d\n", s->alarm_high);
            break;
        case 0x10:
            goldfish_device_set_irq(&s->dev, 0, 0);
            break;
        default:
            cpu_abort (cpu_single_env, "goldfish_rtc_write: Bad offset %x\n", offset);
    }
}


static struct timer_state rtc_state = {
    .dev = {
        .name = "goldfish_rtc",
        .id = -1,
        .size = 0x1000,
        .irq_count = 1,
    }
};

#endif
goldfish_timer_device* new_goldfish_timer_device(char* obj_name){
	goldfish_timer_device* dev = skyeye_mm_zero(sizeof(goldfish_timer_device));
	dev->obj = new_conf_object(obj_name, dev);
	timer_state_t* timer =  skyeye_mm_zero(sizeof(timer_state_t));
	timer->dev = dev;
	//timer_state.timer = qemu_new_timer_ns(vm_clock, goldfish_timer_tick, &timer_state);

	int timer_id;
	create_thread_scheduler(10000000, Periodic_sched, goldfish_timer_tick, dev->obj, &timer_id);
	timer->id = timer_id;

	dev->timer = timer;
	dev->io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	dev->io_memory->read = timer_read;
	dev->io_memory->write = timer_write;
	
	return dev;
}
void del_goldfish_timer_device(conf_object_t* dev){
	
}

void goldfish_timer_device_init()
{
#if 0
    timer_state.dev.base = timerbase;
    timer_state.dev.irq = timerirq;
    timer_state.timer = qemu_new_timer_ns(vm_clock, goldfish_timer_tick, &timer_state);
    goldfish_device_add(&timer_state.dev, goldfish_timer_readfn, goldfish_timer_writefn, &timer_state);
    register_savevm( "goldfish_timer", 0, GOLDFISH_TIMER_SAVE_VERSION,
                     goldfish_timer_save, goldfish_timer_load, &timer_state);

    goldfish_device_add(&rtc_state.dev, goldfish_rtc_readfn, goldfish_rtc_writefn, &rtc_state);
    register_savevm( "goldfish_rtc", 0, GOLDFISH_RTC_SAVE_VERSION,
                     goldfish_rtc_save, goldfish_rtc_load, &rtc_state);
#endif
}

