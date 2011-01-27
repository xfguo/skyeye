/*
	skyeye_net_vhub.c - vhub net device file support functions
	Copyright (C) 2003 Skyeye Develop Group
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

//#include "armdefs.h"

#if !(defined(__MINGW32__) || defined(__CYGWIN__) || defined(__BEOS__))

#ifdef __linux__
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#ifdef __svr4__
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
//#include <net/if_tun.h>
#include <fcntl.h>
#endif

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_tun.h>
#include <fcntl.h>
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#endif

#include "skyeye_net.h"

#define DEBUG 0
#if DEBUG
#define DBG_PRINT(a...) fprintf(stderr, ##a)
#else
#define DBG_PRINT(a...)
#endif

static name_index = 0;

int
vhub_open(struct net_device *net_dev)
{
	int vhub_fd = -1;
	if ((vhub_fd = open("/dev/vhub", O_RDWR|O_NONBLOCK, S_IRUSR | S_IWUSR)) < 0)
	{
		perror("open dev vhub error\n");
		return 1;
	}
	if ((ioctl(vhub_fd, 0, net_dev->macaddr)) < 0)
	{
		perror("ioctl dev vhub error\n");
		return 1;
	}
	net_dev->net_fd = vhub_fd;
	printf("vhub init open\n");
	return 0;
}


int
vhub_close(struct net_device *net_dev)
{
	close(net_dev->net_fd);
	return 0;
}

int
vhub_read(struct net_device *net_dev, void *buf, size_t count)
{
	return read(net_dev->net_fd, buf, count);
}

int
vhub_write(struct net_device *net_dev, void *buf, size_t count)
{
	printf("vhub write\n");
	return write(net_dev->net_fd, buf, count);
}

int
vhub_wait_packet(struct net_device *net_dev, struct timeval *tv)
{
	fd_set frds;
	int ret;

	FD_ZERO(&frds);
	FD_SET(net_dev->net_fd, &frds);
	if ((ret = select(net_dev->net_fd + 1, &frds, NULL, NULL, tv)) <= 0)
        return -1;

	if (!FD_ISSET(net_dev->net_fd, &frds))
        return -1;

	return 0;
}

#else /* other systems */

#if (defined(__MINGW32__) || defined(__CYGWIN__))
#define SKYEYE_NET_vhub_SUPPORT
#include "./skyeye_net_tap_win32.c"
#endif /* defined(__MINGW32__) || defined(__CYGWIN__) */

#ifdef __BEOS__
#define SKYEYE_NET_vhub_SUPPORT
#include "./skyeye_net_tap_beos.c"
#endif /* __BEOS__ */

#ifndef SKYEYE_NET_vhub_SUPPORT

int
vhub_open(struct net_device *net_dev)s
{
	return -1;
}

int
vhub_close(struct net_device *net_dev)
{
	return 0;
}

int
vhub_read(struct net_device *net_dev, void *buf, size_t count)
{
	return 0;
}

int
vhub_write(struct net_device *net_dev, void *buf, size_t count)
{
	return 0;
}

int
vhub_wait_packet(struct net_device *net_dev, struct timeval *tv)
{
	return -1;
}

#endif /* SKYEYE_NET_vhub_SUPPORT */

#endif
