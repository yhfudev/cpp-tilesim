#ifndef PTH_WARP_TO_BTH_H
#define PTH_WARP_TO_BTH_H

//#include "gw_config.h"
#define HAVE_PTHREAD_H 1

#if !HAVE_PTHREAD_H
#error "You need Posix threads and <pthread.h>"
#endif

#include <pthread.h>

#if ! defined(pthread_func_t)
typedef void * (*pthread_func_t) (void *arg);
#endif

#define bthread_attr_t void *
#define bthread_func_t pthread_func_t
#define bthread_t pthread_t

//int bthread_init (void);
//int bthread_shutdown (void);
#if defined(WIN32)
#define bthread_init() pthread_init_np()
#define bthread_shutdown() pthread_shutdown_np()
#else
#define bthread_init() (0)
#define bthread_shutdown()
#endif

#define bthread_create pthread_create
#define bthread_join   pthread_join
#define bthread_detach pthread_detach
//#define bthread_join_every(func) pthread_yield()
//#define bthread_join_all() pthread_yield()

#define bthread_self() pthread_self()
#if defined (WIN32)
#define bthread_selfid() GetCurrentThreadId()
#else
#define bthread_selfid() bthread_self()
#endif

#define bthread_sleep(seconds) sleep((int)(seconds))
#if defined(WIN32)
//#define bthread_suspend()  SuspendThread(GetCurrentThread())
//#define bthread_wakeup(ti) ResumeThread(ti)
#define bthread_wakeup(ti) ((NULL == ti)?(-1):ResumeThread(ti->pi))
#else
//#define bthread_suspend()  bthread_sleep(1)
#define bthread_wakeup(ti) pthread_yield()
#endif
//#define bthread_wakeup_all() pthread_yield()

#if 0
#define bthread_pollfd(fd, events, timeout) (-1)
#define bthread_poll(fds, numfds, timeout) (-1)

#define bthread_dumpsigmask() (-1)
#endif
#define bthread_shouldhandlesignal(signal) (-1)


#define GWTHREAD_MUTEX_FAST_NP       PTHREAD_MUTEX_FAST_NP
#define GWTHREAD_MUTEX_RECURSIVE_NP  PTHREAD_MUTEX_RECURSIVE_NP
#define GWTHREAD_MUTEX_ERRORCHECK_NP PTHREAD_MUTEX_ERRORCHECK_NP
#define bthread_mutexattr_t       pthread_mutexattr_t
#define bthread_mutexattr_init    pthread_mutexattr_init
#define bthread_mutexattr_destroy pthread_mutexattr_destroy
#define bthread_mutexattr_settype pthread_mutexattr_settype
#define bthread_mutexattr_gettype pthread_mutexattr_gettype

#define bthread_mutex_t pthread_mutex_t
#define bthread_cond_t  pthread_cond_t
//#define bthread_sema_t  pthread_sema_t

#define bthread_mutex_init(pmutex, attrib) pthread_mutex_init((bthread_mutex_t *)(pmutex), (attrib))
#define bthread_mutex_destroy(pmutex)      pthread_mutex_destroy((bthread_mutex_t *)(pmutex))
#define bthread_mutex_lock(pmutex)             pthread_mutex_lock((bthread_mutex_t *)(pmutex))
#define bthread_mutex_unlock(pmutex)           pthread_mutex_unlock((bthread_mutex_t *)(pmutex))

#define bthread_cond_init(pcond, attrib) pthread_cond_init((bthread_cond_t *)(pcond), (attrib))
#define bthread_cond_destroy(pcond)      pthread_cond_destroy((bthread_cond_t *)(pcond))
#define bthread_cond_signal(pcond)       pthread_cond_signal((bthread_cond_t *)(pcond))
#define bthread_cond_broadcast(pcond)    pthread_cond_broadcast((bthread_cond_t *)(pcond))
#define bthread_cond_wait(pcond, pmutex)  pthread_cond_wait((bthread_cond_t *)(pcond), (bthread_mutex_t *)(pmutex))
#define bthread_cond_timedwait(pcond, mutex, ts) pthread_cond_timedwait((bthread_cond_t *)(pcond), (bthread_mutex_t *)(mutex), (ts))

//#define bthread_sema_init(sema)    pthread_sema_init(sema)
//#define bthread_sema_destroy(sema) pthread_sema_destroy(sema)
//#define bthread_sema_signal(sema)  pthread_sema_signal(sema)
//#define bthread_sema_wait(sema)    pthread_sema_wait(sema)
#define bsem_t          sem_t
#define bsem_init       sem_init
#define bsem_destroy    sem_destroy
#define bsem_wait       sem_wait
#define bsem_trywait    sem_trywait
#define bsem_post       sem_post
#define bsem_getvalue   sem_getvalue

#endif // PTH_WARP_TO_BTH_H
