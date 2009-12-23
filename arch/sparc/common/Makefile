CROSS_COMPILE ?=
EXTRA_CFLAGS ?=
UTILS_CFLAGS = -I utils -I utils/share -I utils/main -I utils/config -I utils/debugger

CC = $(CROSS_COMPILE)gcc -g
CFLAGS := $(CFLAGS) $(EXTRA_CFLAGS)

sparc: memory.o ram.o iu.o traps.o

traps.o:traps.c
	$(CC) $(CFLAGS) -c traps.c
iu.o:iu.c
	$(CC) $(CFLAGS) -c iu.c
ram.o:ram.c
	$(CC) $(CFLAGS) -c ram.c
memory.o:memory.c
	$(CC) $(CFLAGS) -c memory.c

clean:
	rm -f *.o 
