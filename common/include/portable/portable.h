#ifndef __PORTABLE__
#define __PORTABLE__
#include "gettimeofday.h"

#ifdef __WIN32__
#include <search.h>
#include <sys/unistd.h>
#include <stdlib.h>
typedef unsigned int uint;
#define __THROW
#define UIO_MAXIOV 1024
#define F_GETFD	1
int mkdir (const char *_path, unsigned int mode);
struct _ENTRY;
struct hsearch_data
{
	struct _ENTRY *table;
	unsigned int size;
	unsigned int filled;
};
extern int hsearch_r (ENTRY __item, ACTION __action, ENTRY **__retval,
		struct hsearch_data *__htab) __THROW;
extern int hcreate_r (size_t __nel, struct hsearch_data *__htab) __THROW;
extern void hdestroy_r (struct hsearch_data *__htab) __THROW;
extern ENTRY *hsearch (ENTRY __item, ACTION __action) __THROW;
extern int hcreate (size_t __nel) __THROW;
extern void hdestroy (void) __THROW;
extern void bzero (void *__s, size_t __n);
extern int fcntl(int fd, int cmd, ...);
extern int setitimer(int which, const struct itimerval *new_value,
				struct itimerval *old_value);
_CRTIMP int __cdecl __MINGW_NOTHROW _fstat64 (int, struct __stat64*);
#define sleep(seconds)  _sleep(seconds)
#define fstat64(fd, buf) _fstat64(fd, buf)
/**********************************************************************/
#define hcreate_r(max_conf_obj, conf_tab)
#define hcreate(number_of_symbols) 1
/**********************************************************************/
#define SIGVTALRM  26
#define ITIMER_VIRTUAL 1
#endif /* __WIN32__ */
#endif
