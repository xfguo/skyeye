ifneq ($(KERNELRELEASE),)
	obj-m := vmnet.o
else
	KERNELDIR := /usr/src/linux-headers-2.6.32-21-generic/

PWD :=$(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o  *.mod.c *.mod.o *.order *.symvers
endif
