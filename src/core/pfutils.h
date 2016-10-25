/******************************************************************************
 * Name:        src/pfutils.h
 * Purpose:     Some base data struct functions.
 * Author:      Yunhui Fu
 * Created:     2008-03-30
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2007 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/
#ifndef _PFUTILS_H
#define _PFUTILS_H

#include "config.h"

#if HAVE_STDINT_H
#include <stdint.h> // uint8_t
#else
#define uint8_t unsigned char
#endif

#ifdef DEBUG
#ifndef MEMWATCH
//#define MEMWATCH 1
#endif
#endif

#if MEMWATCH
#include "memwatch.h"
#else
#define CHECK()
#endif

#ifndef _PSF_BEGIN_EXTERN_C
#include <stdlib.h> // size_t
#include <sys/types.h> // ssize_t

  #ifdef __cplusplus
    #define _PSF_BEGIN_EXTERN_C extern "C" {
    #define _PSF_END_EXTERN_C }
  #else
    #define _PSF_BEGIN_EXTERN_C
    #define _PSF_END_EXTERN_C
  #endif /* __cplusplus */

#define MemHandle void *
#define MemHandleRealloc realloc
#define MemHandleFree    free
#define MemHandleLock(a) (a)
#define MemHandleUnlock(a)

#define CODE_SECTION_UTILS

#endif /* ! _PSF_BEGIN_EXTERN_C */

_PSF_BEGIN_EXTERN_C

typedef struct _memarr_t {
    size_t max;
    size_t inc;
    size_t cur;
    size_t itemsize;
    MemHandle data;
} memarr_t;
typedef int (* memarr_cb_destroy_t)(void * data);

CODE_SECTION_UTILS int ma_init (memarr_t *pma, size_t datasize);
CODE_SECTION_UTILS int ma_clear (memarr_t *pma, memarr_cb_destroy_t cb_destroy);

#ifdef DEBUG
/* insert 传入的data指针为指向需要拷贝的长度为datasize的数据缓冲 */
CODE_SECTION_UTILS int ma_insert_dbg (memarr_t *pma, size_t idx, void * data, const char *cstr_func, const char *cstr_file, int line);
CODE_SECTION_UTILS int ma_replace_dbg (memarr_t *pma, size_t idx, void * data, const char *cstr_func, const char *cstr_file, int line);
int ma_copy_dbg (memarr_t *pma_to, memarr_t *pma_from, const char *cstr_func, const char *cstr_file, int line);
#define ma_insert(p,i,d) ma_insert_dbg((p),(i),(d),__func__,__FILE__,__LINE__)
#define ma_replace(p,i,d) ma_replace_dbg((p),(i),(d),__func__,__FILE__,__LINE__)
#define ma_copy(p1,p2) ma_copy_dbg((p1),(p2),__func__,__FILE__,__LINE__)
#else
CODE_SECTION_UTILS int ma_insert (memarr_t *pma, size_t idx, void * data);
CODE_SECTION_UTILS int ma_replace (memarr_t *pma, size_t idx, void * data);
CODE_SECTION_UTILS int ma_copy (memarr_t *pma_to, memarr_t *pma_from);
#endif

/* ma_data 传入的data指针为指向长度为datasize的数据缓冲，指针不能为空；这个缓冲由ma_data填入 */
CODE_SECTION_UTILS int ma_data (const memarr_t *pma, size_t idx, void * data);

/* 返回缓冲的指针 */
#if __GNUC__
#if 1
#define ma_data_lock(pma, idx) ((char *)MemHandleLock ((pma)->data) + ((idx) * (pma)->itemsize))
#else
inline void * ma_data_lock (memarr_t *pma, size_t idx) __attribute__((always_inline));
inline void *
ma_data_lock (memarr_t *pma, size_t idx)
{
    char * realdata;
    //assert (0 != pma);

    // TODO:
    // 同一个 pma ma_inc 申请内存可能导致其基址被改变，而之前如果有另外一个lock的（如只读的），则其指针在这次ma_inc 后失效(被free的内存空间)
    // 所以需要重新获取 （ma_data_lock） 指针。
    // 所以最好限制为每次只允许一个 lock, 或者, 被 lock 之后不允许 ma_inc 而抛出异常 (使用 assert)

    if ((pma->cur < 1) || (idx >= pma->cur))
        return NULL;

    realdata = (char *)MemHandleLock (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        MemHandleUnlock (pma->data);
        return NULL;
    }
    //MemHandleUnlock (pma->data);
    return realdata + (idx * pma->itemsize);
}
#endif // 0

#else
CODE_SECTION_UTILS void * ma_data_lock (memarr_t *pma, size_t idx);
#endif
//CODE_SECTION_UTILS void ma_data_unlock (memarr_t *pma, size_t idx);
#define ma_data_unlock(pma, idx) MemHandleUnlock (pma->data)

CODE_SECTION_UTILS ssize_t ma_inc (memarr_t *pma);

/* ma_rmdata 传入的data指针为指向长度为datasize的数据缓冲，指针可以为空；这个缓冲由ma_rmdata填入将要从内部删除的数据内容 */
CODE_SECTION_UTILS int ma_rmdata (memarr_t *pma, size_t idx, void * data);
CODE_SECTION_UTILS int ma_rmalldata (memarr_t *pma, memarr_cb_destroy_t cb_destroy);
#define ma_size(pma) ((pma)->cur)
#define ma_append(pma, pdata) ma_insert(pma, ma_size(pma), pdata)
#define ma_itemsize(pma) ((pma)->itemsize)

/* 比较时的回调函数定义
 data1, data2 均指向内部的两个数据缓冲，用户需要从该指针指向的地址拿数据，而不是这两个指针本身. */
typedef int (* ma_cb_swap_t)(void *userdata, void * data1, void * data2); /*"*data1 <-> *data2"*/
typedef int (* ma_cb_comp_t)(void *userdata, void * data1, void * data2); /*"*data1 - *data2"*/
CODE_SECTION_UTILS int ma_sort (memarr_t *pma, void *userdata, ma_cb_comp_t cb_comp, ma_cb_swap_t cb_swap);

typedef void (* memarr_cb_iter_t)(void * userdata, size_t idx, void *data);
int ma_foreach (memarr_t *pma, memarr_cb_iter_t cb_iter, void *userdata);

/* 将memarr_t内部申请的动态内存移动到新的pma_to中，原来的数据结构相关的项被清除 */
#ifdef DEBUG
#define ma_transfer(pma_to, pma_from, cb_destroy) ma_transfer_dbg(pma_to, pma_from, cb_destroy, __func__,__FILE__,__LINE__)
CODE_SECTION_UTILS int ma_transfer_dbg (memarr_t *pma_to, memarr_t *pma_from, memarr_cb_destroy_t cb_destroy, const char *cstr_func, const char *cstr_file, int line);
#else
CODE_SECTION_UTILS int ma_transfer (memarr_t *pma_to, memarr_t *pma_from, memarr_cb_destroy_t cb_destroy);
#endif

//---------------
// <sys/signal.h> stack_t

#define stk_t memarr_t
#define stack_cb_destroy_t memarr_cb_destroy_t
#define st_init(stp, sz) ma_init((stp), (sz))
#define st_clear(stp, cb_destroy) ma_clear((stp), cb_destroy)
#define st_push(stp, pdata) ma_append((stp), pdata)
#define st_pop(stp, pdata) ma_rmdata((stp), ma_size(stp) - 1, pdata)
#define st_popall(stp, cb_destroy) ma_rmalldata((stp), cb_destroy)
#define st_rmdata(stp, idx, pdata) ma_rmdata(stp, idx, pdata)
#define st_data(stp, idx, pdata) ma_data(stp, idx, pdata)
#define st_size(stp) ma_size(stp)

//---------------

/* 有序序列，用户可以通过提供索引和比较函数来存入一组数据 */

//typedef int (* slist_cb_comp_t)(void * data1, void * data2); /*"*data1 - *data2"*/
#define slist_cb_comp_t ma_cb_comp_t
#define slist_cb_swap_t ma_cb_swap_t
/*typedef int (* slist_cb_comp_heter_t)(void *userdata, void * data_pin, void * data2); //"*data1 - *data2"*/

typedef struct _sortedlist_t {
    memarr_t marr;
    slist_cb_comp_t cb_comp;
    slist_cb_swap_t cb_swap;
    void *userdata;
    slist_cb_comp_t cb_comp_heter; // 临时，slist_find()中用
} sortedlist_t;

#define slist_cb_destroy_t memarr_cb_destroy_t
#define slist_cb_iter_t memarr_cb_iter_t
#define slist_size(psl) ma_size(&((psl)->marr))
#define slist_rmalldata(psl, cb_destroy) ma_rmalldata(&((psl)->marr), cb_destroy)
#define slist_rmdata(psl, idx, pdata) ma_rmdata(&((psl)->marr), idx, pdata)
#define slist_data(psl, idx, pdata) ma_data(&((psl)->marr), idx, pdata)

#define slist_data_lock(psl, idx) ma_data_lock (&((psl)->marr), idx)
#define slist_data_unlock(psl, idx) ma_data_unlock (&((psl)->marr), idx)
#define slist_copy(psl_to, psl_from) ((psl_to)->cb_comp = (psl_from)->cb_comp, ma_copy (&((psl_to)->marr), &((psl_from)->marr)))
#define slist_foreach(psl,cb_iter,userdata) ma_foreach (&((psl)->marr), cb_iter, userdata)

#define slist_inc(psl) ma_inc (&((psl)->marr));

#ifdef DEBUG
#define slist_transfer_dbg(psl_to, psl_from, cb_destroy, cstr_func, cstr_file, line) ((psl_to)->cb_comp = (psl_from)->cb_comp, ma_transfer_dbg (&((psl_to)->marr), &((psl_from)->marr), cb_destroy, cstr_func, cstr_file, line))
#define slist_transfer(psl_to, psl_from, cb_destroy) slist_transfer_dbg(psl_to, psl_from, cb_destroy, __func__,__FILE__,__LINE__)
#else
#define slist_transfer(psl_to, psl_from, cb_destroy) ((psl_to)->cb_comp = (psl_from)->cb_comp, ma_transfer (&((psl_to)->marr), &((psl_from)->marr), cb_destroy))
#endif

/* 比较时的回调函数定义
 data1, data2 均指向内部的两个数据缓冲，用户需要从该指针指向的地址拿数据，而不是这两个指针本身. */
CODE_SECTION_UTILS int slist_sort (sortedlist_t *psl);

CODE_SECTION_UTILS int slist_init (sortedlist_t *psl, size_t datasize, void *userdata, slist_cb_comp_t cb_comp, slist_cb_swap_t cb_swap);
CODE_SECTION_UTILS int slist_clear (sortedlist_t *psl, slist_cb_destroy_t cb_destroy);

/* data 为指向需要存储数据的起始地址 */
#ifdef DEBUG
CODE_SECTION_UTILS int slist_store_dbg (sortedlist_t *psl, void *data, const char *cstr_func, const char *cstr_file, int line);
#define slist_store(p,d) slist_store_dbg((p),(d),__func__,__FILE__,__LINE__)
#else
CODE_SECTION_UTILS int slist_store (sortedlist_t *psl, void *data);
#endif

/* data_pinpoint 为指向需要比较的数据的起始地址, 成功返回0, 失败返回-1 */
/* 为了支持异种数据比较，函数接受临时更换比较函数 cb_comb_heter，这时回调函数的定义和 slist_init 传入的回调函数不一样。

typedef int (* slist_cb_comp_heter_t)(void *userdata, void * data_pin, void * data2); //"*data_pin - *data2"

   回调函数中的 userdata 是在slist_init 传入的， data_pin 的定义可以和slist中的定义不一样，由cb_comb_heter决定；
   data2是存储在slist中的数据 */
CODE_SECTION_UTILS int slist_find (sortedlist_t *psl, void *data_pinpoint, slist_cb_comp_t cb_comb_heter, size_t *ret_idx);

CODE_SECTION_UTILS int slist_cb_comp_sizet (void *userdata, void * data1, void * data2);
CODE_SECTION_UTILS int slist_cb_swap_sizet (void *userdata, void * data1, void * data2);

#define NUM_TYPE(p, type) (sizeof (p) / sizeof (type))

#ifndef ntohll
# if __BYTE_ORDER == __BIG_ENDIAN
/* The host byte order is the same as network byte order,
   so these functions are all just identity.  */
#  define ntohll(x)       (x)
#  define htonll(x)       (x)
# else
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ntohll(x) (((uint64_t)(ntohl((int)((x << 32) >> 32))) << 32) | (unsigned int)ntohl(((int)(x >> 32))))
#   define htonll(x) ntohll(x)
#  endif
# endif
#endif

#ifndef ntohll
# if defined(__DARWIN__)
#  define ntohll(_x_) NXSwapBigLongLongToHost(_x_)
# elif defined(__SOLARIS__)
#  if defined(_LITTLE_ENDIAN)
#   define ntohll(_x_) ((((uint64_t)ntohl((_x_) >> 32)) & 0xffffffff) | (((uint64_t)ntohl(_x_)) << 32))
#   define htonll(x) ntohll(x)
#  else
#   define ntohll(_x_) (_x_)
#  endif
# elif defined(__FreeBSD__)
#  define ntohll(_x_) __bswap_64(_x_)
# else
#  define ntohll(_x_) __bswap_64(_x_)
# endif
#endif
#ifndef htonll
# if defined(__DARWIN__)
#  define htonll(_x_) NXSwapHostLongLongToBig(_x_)
# elif defined(__SOLARIS__)
#  if defined(_LITTLE_ENDIAN)
#   define htonll(_x_) ((htonl((_x_ >> 32) & 0xffffffff) | ((uint64_t) (htonl(_x_ & 0xffffffff)) << 32)))
#  else
#   define htonll(_x_) (_x_)
#  endif
# elif defined(__FreeBSD__)
#  define  htonll(_x_) __bswap_64(_x_)
# else
#  define htonll(_x_) __bswap_64(_x_)
# endif
#endif

_PSF_END_EXTERN_C
#endif // _PFUTILS_H
