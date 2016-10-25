/*
 * $Id: pthread.h,v 1.11 2005/06/03 06:19:36 yhfu Exp $
 * simulate the POSIX pthread library in WIN32
 */

#ifndef _BTHREAD_H
#define _BTHREAD_H

#ifndef WIN32
#include "bthwrap4pth.h"

#else
//#include "win32.h"
#include <windows.h>

//#define USE_SIMPLE_THREAD 1
//#define USE_SIMPLE_MUTEX 1        // 1 - map the mutex directly to win32 API, only support BTHREAD_MUTEX_FAST_NP
#define USE_ENCAPSULATED_THREAD 1       // 1 - don't use WaitForSingleObject() wait for exiting of thread

#if defined(USE_SIMPLE_THREAD)
#define bthread_t HANDLE

#else // USE_SIMPLE_THREAD
struct _thread_info_t;
//typedef struct _thread_info_t thread_info_t;

//#define bthread_t thread_info_t *
typedef struct _thread_info_t * bthread_t;

#endif // USE_SIMPLE_THREAD

typedef void *(*bthread_func_t) (void *arg);

#define bthread_key_t DWORD

#define bthread_fast_mutex_t HANDLE
#define bthread_fast_cond_t  HANDLE

#if defined(USE_SIMPLE_MUTEX)
#define bthread_mutex_t bthread_fast_mutex_t
#define bthread_cond_t  bthread_fast_cond_t
#else
typedef struct _bthread_mutex_t
{
    bthread_fast_mutex_t fast_mutex;
    bthread_fast_cond_t wakeup;

    int flag_mode;              // for self recursive, return error, fast. 0x01: recursive; 0x02: return error; 0x04 fast
    int flag_locked;            // if the mutex is unlocked
    bthread_t th_owner;         // the thread id that the mutex currently belong to
    int count;                  // the lock counter
} bthread_mutex_t;
#define bthread_cond_t bthread_fast_cond_t
#endif

typedef struct _bthread_mutexattr_t
{
    int flag_mode;
} bthread_mutexattr_t;

typedef struct tag_bthread_condattr_t
{
    int unused;
} bthread_condattr_t;

typedef struct tag_bthread_attr_t
{
    int unused;
} bthread_attr_t;

#if defined(USE_SIMPLE_THREAD)

//int  bthread_create(bthread_t * thread, bthread_attr_t * attr, void * (*start_routine)(void *), void * arg);
#define bthread_create(thread, attr, start_routine, arg) (((*(thread) = CreateThread(NULL, 0, ((LPTHREAD_START_ROUTINE)start_routine), (arg), 0, NULL)) == NULL)?EAGAIN:0)
//#define bthread_create(thread, attr, start_routine, arg) (((*(thread) = _beginthreadex(NULL, 0, ((LPTHREAD_START_ROUTINE)start_routine), (arg), 0, NULL)) == NULL)?EAGAIN:0)

#define bthread_detach(thread) (0)
//#define bthread_join(thread, thread_return) ((WAIT_OBJECT_0 == WaitForSingleObject (thread, INFINITE))?((NULL != thread_return)?((GetExitCodeThread (thread, thread_return))?((*thread_return != STILL_ACTIVE)?0:-1):-1):0):EINVAL)

#define bthread_self() GetCurrentThread()
#define bthread_equal(t1, t2) ((t1) == (t2))    // || (GetCurrentThreadId(t1) == GetCurrentThreadId(t2)))

//void bthread_exit(void *retval);
#define bthread_exit(v) ExitThread((DWORD)(v))

#define bthread_init_np() (0)
#define bthread_shutdown_np() (0)
#define bthread_cancel(th) (TerminateThread(th, NULL)?0:(ESRCH))

#else // USE_SIMPLE_THREAD

int bthread_init_np_real (void);
int bthread_shutdown_np_real (void);
#define bthread_init_np bthread_init_np_real
#define bthread_shutdown_np bthread_shutdown_np_real

#if defined(_DEBUG)
int bthread_create_real (bthread_t * thp, bthread_attr_t * attr,
                         bthread_func_t func, void *arg, char *func_name,
                         char *function, char *file, int line);
//#define bthread_create(thp, func, arg) bthread_create_real((thp), (func), (arg), #func, __FUNCTION__, __FILE__, __LINE__)
#define bthread_create(thp, attr, func, arg) (GW_TRACE(("thread create: "#func"(0x%08X)", (func))), bthread_create_real((thp), (attr), (func), (arg), #func, __FUNCTION__, __FILE__, __LINE__))
#else
int bthread_create_real (bthread_t * thp, bthread_attr_t * attr,
                         bthread_func_t func, void *arg);
#define bthread_create(thp, attr, func, arg) bthread_create_real((thp), (attr), (func), (arg))
#endif

int bthread_detach (bthread_t th);

bthread_t bthread_self (void);
int bthread_equal (bthread_t thread1, bthread_t thread2);

//void bthread_exit(void *retval);
#define bthread_exit(v) return(v)
int bthread_cancel (bthread_t thread);

#endif // USE_SIMPLE_THREAD

//int bthread_join (bthread_t th, void **thread_return);
int bthread_join_real (bthread_t th, void **thread_return);
#if defined(_DEBUG)
#define bthread_join(a,b) (GW_TRACE(("bthread_join() ...")),bthread_join_real(a,b))
#else
#define bthread_join bthread_join_real
#endif

#define bthread_selfid_np() ((long)GetCurrentThreadId())

//#define bthread_fast_mutex_init(mutex, arrt) (((*(mutex) = CreateMutex(NULL, FALSE, NULL)) == NULL)?ENOMEM:0)
#define bthread_fast_mutex_init(mutex, arrt) (((*(mutex) = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)?ENOMEM:0)
#define bthread_fast_mutex_destroy(mutex) (CloseHandle(*((bthread_t *)mutex))?0:EBUSY)
#define bthread_fast_mutex_trylock(mutex) ((WaitForSingleObject(*((bthread_t *)mutex), 0) == WAIT_OBJECT_0)?0:EINVAL)
#define bthread_fast_mutex_lock(mutex) ((WaitForSingleObject(*((bthread_t *)mutex), INFINITE) == WAIT_OBJECT_0)?0:EINVAL)
//#define bthread_fast_mutex_unlock(mutex) (ReleaseMutex(*((bthread_t *)mutex))?0:EPERM)
#define bthread_fast_mutex_unlock(mutex) (ReleaseSemaphore(*((bthread_t *)mutex), 1, NULL)?0:ENOMEM)

/* Compatibility with LinuxThreads */
#define BTHREAD_MUTEX_FAST_NP       0x01
#define BTHREAD_MUTEX_RECURSIVE_NP  0x02
#define BTHREAD_MUTEX_ERRORCHECK_NP 0x04
#define BTHREAD_MUTEX_TIMED_NP      BTHREAD_MUTEX_FAST_NP,
#define BTHREAD_MUTEX_ADAPTIVE_NP   BTHREAD_MUTEX_FAST_NP,
/* For compatibility with POSIX */
#define BTHREAD_MUTEX_NORMAL        BTHREAD_MUTEX_FAST_NP,
#define BTHREAD_MUTEX_RECURSIVE     BTHREAD_MUTEX_RECURSIVE_NP,
#define BTHREAD_MUTEX_ERRORCHECK    BTHREAD_MUTEX_ERRORCHECK_NP,
#define BTHREAD_MUTEX_DEFAULT       BTHREAD_MUTEX_NORMAL

/*
 * bthread_mutex_t fastmutex = BTHREAD_MUTEX_INITIALIZER;
 * bthread_mutex_t recmutex = BTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
 * bthread_mutex_t errchkmutex = BTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
 *
 * int bthread_mutex_init(bthread_mutex_t *mutex, const bthread_mutexattr_t *mutexattr);
 * int bthread_mutex_lock(bthread_mutex_t *mutex);
 * int bthread_mutex_trylock(bthread_mutex_t *mutex);
 * int bthread_mutex_unlock(bthread_mutex_t *mutex);
 * int bthread_mutex_destroy(bthread_mutex_t *mutex);
 */
#if defined(USE_SIMPLE_MUTEX)

#define bthread_mutexattr_init(a) (0)
#define bthread_mutexattr_destroy(a) (0)
#define bthread_mutexattr_settype(attr, kind) (0)
#define bthread_mutexattr_gettype(attr, kindp) ((NULL == kindp)?(EINVAL):((*(kindp) = BTHREAD_MUTEX_NORMAL), 0))

#define bthread_mutex_init(mutex, arrt) bthread_fast_mutex_init((mutex), (arrt))
#define bthread_mutex_destroy(mutex)    bthread_fast_mutex_destroy(mutex)
#define bthread_mutex_lock(mutex)       bthread_fast_mutex_lock(mutex)
#define bthread_mutex_trylock(mutex)    bthread_fast_mutex_trylock(mutex)
#define bthread_mutex_unlock(mutex)     bthread_fast_mutex_unlock(mutex)
#else

int bthread_mutexattr_init (bthread_mutexattr_t * attr);
int bthread_mutexattr_destroy (bthread_mutexattr_t * attr);
int bthread_mutexattr_settype (bthread_mutexattr_t * attr, int kind);
int bthread_mutexattr_gettype (const bthread_mutexattr_t * attr, int *kind);

int bthread_mutex_init (bthread_mutex_t * mutex,
                        const bthread_mutexattr_t * mutexattr);
int bthread_mutex_lock (bthread_mutex_t * mutex);
int bthread_mutex_trylock (bthread_mutex_t * mutex);
int bthread_mutex_unlock (bthread_mutex_t * mutex);
int bthread_mutex_destroy (bthread_mutex_t * mutex);
#endif

#define	BTHREAD_MAX_COND_NUM	200

#define bthread_fast_cond_init(cond, attr) (((*((bthread_cond_t *)cond) = CreateSemaphore(NULL, 0, BTHREAD_MAX_COND_NUM, NULL)) == NULL)?ENOMEM:0)
#define bthread_fast_cond_destroy(cond)    (CloseHandle(*((bthread_cond_t *)cond))?0:EBUSY)
#define bthread_fast_cond_broadcast(cond)  (ReleaseSemaphore(*((bthread_cond_t *)cond), BTHREAD_MAX_COND_NUM, NULL)?0:EPERM)
#define bthread_fast_cond_signal(cond)     (ReleaseSemaphore(*((bthread_cond_t *)cond), 1, NULL)?0:EPERM)
int bthread_fast_cond_wait (bthread_fast_cond_t * cond,
                            bthread_fast_mutex_t * mutex);
int bthread_fast_cond_timedwait (bthread_fast_cond_t * cond,
                                 bthread_fast_mutex_t * mutex,
                                 const struct timespec *abstime);

/*
 * bthread_cond_t cond = bthread_COND_INITIALIZER;
 *
 * int bthread_cond_init(bthread_cond_t *cond, bthread_condattr_t *cond_attr);
 * int bthread_cond_signal(bthread_cond_t *cond);
 * int bthread_cond_broadcast(bthread_cond_t *cond);
 * int bthread_cond_wait(bthread_cond_t *cond, bthread_mutex_t *mutex);
 * int bthread_cond_timedwait(bthread_cond_t *cond, bthread_mutex_t *mutex, const struct timespec *abstime);
 * int bthread_cond_destroy(bthread_cond_t *cond);
 */
#if defined(USE_SIMPLE_MUTEX)

#define bthread_cond_init(cond, arrt)            bthread_fast_cond_init((cond), (arrt))
#define bthread_cond_destroy(cond)               bthread_fast_cond_destroy(cond)
#define bthread_cond_broadcast(cond)             bthread_fast_cond_broadcast(cond)
#define bthread_cond_signal(cond)                bthread_fast_cond_signal(cond)
#define bthread_cond_wait(cond, mutex)           bthread_fast_cond_wait((cond), (mutex))
#define bthread_cond_timedwait(cond, mutex, abt) bthread_fast_cond_timedwait((cond), (mutex), (abt))

#else
#define bthread_cond_init(cond, arrt)            bthread_fast_cond_init((cond), (arrt))
#define bthread_cond_destroy(cond)               bthread_fast_cond_destroy(cond)
#define bthread_cond_broadcast(cond)             bthread_fast_cond_broadcast(cond)
#define bthread_cond_signal(cond)                bthread_fast_cond_signal(cond)
int bthread_cond_wait (bthread_cond_t * cond, bthread_mutex_t * mutex);
int bthread_cond_timedwait (bthread_cond_t * cond, bthread_mutex_t * mutex,
                            const struct timespec *abstime);
#endif
/*
 * int bthread_key_create(bthread_key_t *key, void (*destr_function) (void *));
 * int bthread_key_delete(bthread_key_t key);
 * int bthread_setspecific(bthread_key_t key, const void *pointer);
 * void * bthread_getspecific(bthread_key_t key);
 */
#define bthread_key_create(key, destructor) (((*((bthread_key_t *)key) = TlsAlloc()) == 0xffffffff)?EAGAIN:0)
#define bthread_key_delete(key) (TlsFree(key)?0:EINVAL)
#define bthread_setspecific(key, value) (TlsSetValue((key), (LPVOID) value)?0:EINVAL)
#define bthread_getspecific(key) TlsGetValue(key)

//int bthread_yield (void);
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
#define bthread_yield() ((SwitchToThread())?0:(-1))
#else
#define bthread_yield() (Sleep(100), 0)
#endif

/*
 * int bthread_setcancelstate(int state, int *oldstate);
 * int bthread_setcanceltype(int type, int *oldtype);
 * void bthread_testcancel(void);
 */

#define bsem_t HANDLE
#define BSEM_VALUE_MAX 10000
//int sem_init(sem_t *sem, int pshared, unsigned int value);
#define bsem_init(psem, pshared, value) (value >= BSEM_VALUE_MAX?(SetLastError(EINVAL),-1):((0 == pshared)?(((*(psem)) = CreateSemaphore(NULL, (LONG)value, BSEM_VALUE_MAX, NULL)) == NULL?(SetLastError(ENOSPC),-1):0):(SetLastError(ENOSYS),-1)))
//int sem_destroy(sem_t * sem);
#define bsem_destroy(psem) (CloseHandle(*(psem))?0:(SetLastError(EBUSY),-1))
//int sem_wait(sem_t * sem);
#define bsem_wait(psem)    ((WaitForSingleObject(*(psem), INFINITE) == WAIT_OBJECT_0)?0:(SetLastError(EINVAL),-1))
//int sem_trywait(sem_t * sem);
#define bsem_trywait(psem) ((WaitForSingleObject(*(psem), 0) == WAIT_OBJECT_0)?0:(SetLastError(EINVAL),-1))
//int sem_post(sem_t * sem);
#define bsem_post(psem)    (ReleaseSemaphore(*(psem), 1, NULL)?0:(SetLastError(ERANGE),-1))

int bsem_getvalue (bsem_t * sem, int *sval);

#endif // WIN32

#if 0
#define bthread_cond_timedwait_millisec(cond, mutex, milliseconds) \
do{ \
  struct timespec tv; \
  struct timeval nowtime; \
  gettimeofday (&nowtime, NULL); \
  tv.tv_sec = nowtime.tv_sec + (milliseconds / 1000); \
  tv.tv_nsec= (nowtime.tv_usec + (milliseconds % 1000)) * 1000; \
  bthread_cond_timedwait((cond), (mutex), &tv); \
}while(0)
#else
int bthread_cond_timedwait_millisec (bthread_cond_t * cond, bthread_mutex_t * mutex, const int milliseconds);
#endif // 0

#endif // _BTHREAD_H
