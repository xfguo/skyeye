/*
        skyeye_lock.h - some definition for skyeye pthread lock.

        Copyright (C) 2009 - 2010 Michael.Kang
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
 * 07/27/2010   Jeff.Du <jeffdo.du@gmail.com>
 */
#ifndef __SKYEYE_LOCK_H__
#define __SKYEYE_LOCK_H__

/* pthread rwlock */
#include <pthread.h>

#define RWLOCK_T pthread_rwlock_t
#define RWLOCK_INIT(lock) pthread_rwlock_init(&lock, NULL)
#define RWLOCK_DESTROY(lock) pthread_rwlock_destroy(&lock)
#define RW_RDLOCK(lock)	pthread_rwlock_rdlock(&lock)
#define RW_WRLOCK(lock)	pthread_rwlock_wrlock(&lock)
#define RW_UNLOCK(lock) pthread_rwlock_unlock(&lock)





#endif
