CC = gcc
SKYEYE_PREFIX := /opt/skyeye
CFLAGS = -L$(SKYEYE_PREFIX)/lib/skyeye/ -L$(SKYEYE_PREFIX)/lib/ -I$(SKYEYE_PREFIX)/include -lcommon -o mm_test mm_test.c -lbfd
mm_test:mm_test.c
	$(CC) -o $@ $<
