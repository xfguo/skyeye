#include "skyeye_dyncom.h"
#include "dyncom/defines.h"
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#else
#include <time.h>
#endif

#include <unistd.h>
static uint64_t thread_clock = 0;
#define THREAD_CLOCK_PER_SEC 1000

static uint64_t get_thread_clock(){
	return thread_clock;
}
static void reset_thread_clock(){
	thread_clock = 0;
}
void *clock_thread(void*){
	reset_thread_clock();
	while(1){
		thread_clock += 1;
		usleep(THREAD_CLOCK_PER_SEC);
	}
}

void update_timing(cpu_t *cpu, int index, bool start)
{
	uint64_t usec;

	if ((cpu->dyncom_engine->flags_debug & CPU_DEBUG_PROFILE) == 0)
		return;
#if THREAD_CLOCK
	usec = get_thread_clock();
#else
#if HAVE_GETRUSAGE
	struct rusage r_usage;
	getrusage(RUSAGE_SELF, &r_usage);
	usec = ((uint64_t)r_usage.ru_utime.tv_sec * 1000000) + r_usage.ru_utime.tv_usec;
#else
	usec = (uint64_t)clock()/CLOCKS_PER_SEC;
#endif
#endif

	if (start)
		cpu->timer_start[index] = usec;
	else
		cpu->timer_total[index] += usec - cpu->timer_start[index];
}
