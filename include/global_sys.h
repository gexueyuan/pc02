#ifndef _GLOBAL_SYS_H_
#define _GLOBAL_SYS_H_

//操作系统头文件包含.BEGIN
#ifndef WIN32
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <signal.h>
#include <setjmp.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netdb.h>
#include <poll.h>
#ifdef LINUX
#include <stdarg.h>
#endif
#ifdef AIX
#include <varargs.h>
#include <sys/conf.h>
#endif
#include <zmq.h>

#ifndef HANDLE
typedef void * HANDLE;
#endif
#ifndef HRESULT
typedef long HRESULT;
#endif
#ifndef CHAR
typedef char CHAR;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef LPBYTE
typedef BYTE * LPBYTE;
#endif
#ifndef LPVOID
typedef void * LPVOID;
#endif
#ifndef UINT32
typedef unsigned int UINT32;
#endif
#ifndef LPUINT32
typedef UINT32 * LPUINT32;
#endif

static uint64_t sys_clock (void){
	uint64_t tmpres;
    struct timeval tv;
    gettimeofday (&tv, NULL);
	tmpres = tv.tv_sec;
	tmpres = (tmpres * 1000) + (tv.tv_usec / 1000);
    return (tmpres); 
}

#else
//#include <windows.h>
//#include <winsock2.h>
#include <zmq.h>
#include <time.h>
#define strndup(s, l) _strdup(s)
typedef int socklen_t;
typedef DWORD pthread_t;
extern int pthread_create(pthread_t *restrict_tidp,const void *restrict_attr,void*(*start_rtn)(void*),void *restrict_arg);
extern void pthread_exit(void *retval);
extern int pthread_detach(pthread_t thread);

static uint64_t sys_clock (void){
	uint64_t tmpres;
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
 
	tmpres = ft.dwHighDateTime;
	tmpres = ((tmpres << 32) | ft.dwLowDateTime) / 10000; // 100ns->ms
	return tmpres - 11644473600000Ui64;
}
#endif

//操作系统头文件包含.END

#endif
