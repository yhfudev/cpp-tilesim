/******************************************************************************
 * Name:        src/pfutils.c
 * Purpose:     Some base data struct functions.
 * Author:      Yunhui Fu
 * Created:     2008-03-30
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2007 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/

#if 0
#include "pfall.h"
#else
#include <assert.h>
#include <string.h> //memset()
#include <stdio.h> // fprintf()

#include "pfutils.h"
#include "pfsort.h"
#endif

#if 0 //DEBUG
typedef struct _ma_dbg_info_t {
    char *func;
    char *file;
    size_t line;
} ma_dbg_info_t;
#define MA_MEMLOCK(p) ((char *)(MemHandleLock (p)) + sizeof (ma_dbg_info_t))
#define MA_MEMREALLOC(p, newsize) MemHandleRealloc((p), (newsize) + sizeof (ma_dbg_info_t))
//#define MA_MEMREALLOC(p, newsize) mwRealloc((void *)(p), (int)(newsize), cstr_file, line)
#define MA_MEMMARK(p, cstr_func, cstr_file, line) mwMark((p), (cstr_func), (cstr_file), (line));
#else
#define MA_MEMLOCK(p) MemHandleLock (p)
#define MA_MEMREALLOC(p, newsize) MemHandleRealloc((p), (newsize))
#define MA_MEMMARK(p, cstr_func, cstr_file, line)
#endif

#define MA_MEMUNLOCK(p) MemHandleUnlock (p)

//CODE_SECTION_UTILS static int _slist_rec_find (sortedlist_t *psl, void *data_pinpoint, size_t *ret_idx);
CODE_SECTION_UTILS static int bsearch_cb_comp2_slist (void *userdata, size_t idx, void * data_pin);

// mem array init
// mem array is for structure which is
// return 0 if success, < 0 on error
int
ma_init (memarr_t *pma, size_t datasize)
{
    assert(0 != pma);
    assert(datasize > 0);
    pma->max = 0;
    pma->inc = 5;
    pma->cur = 0;
    pma->itemsize = datasize;
    pma->data = 0;
    return 0;
}

int
ma_foreach (memarr_t *pma, memarr_cb_iter_t cb_iter, void *userdata)
{
    char *p;
    size_t i;
    assert(0 != pma);
    if(0 != pma->data) {
        if ((NULL != cb_iter) && (pma->cur > 0)) {
            p = (char *)MA_MEMLOCK (pma->data);
            if (NULL == p) { //if (DmGetLastErr()) {
                // some error
                return -1;
            }
            for (i = 0; i < pma->cur; i ++) {
                cb_iter (userdata, i, p + i * (pma->itemsize));
            }
            MA_MEMUNLOCK (pma->data);
        }
    }
    return 0;
}

#define MEMSIZE_THRESHOLD 500

int
ma_rmalldata (memarr_t *pma, memarr_cb_destroy_t cb_destroy)
{
    char *p;
    size_t i;
    assert(0 != pma);
    if(0 != pma->data) {
        if ((NULL != cb_destroy) && (pma->cur > 0)) {
            p = (char *)MA_MEMLOCK (pma->data);
            if (NULL == p) { //if (DmGetLastErr()) {
                // some error
                return -1;
            }
            for (i = 0; i < pma->cur; i ++) {
                cb_destroy (p + i * (pma->itemsize));
            }
            MA_MEMUNLOCK (pma->data);
        }
        // if the memory is too large, free it.
        if (pma->itemsize * pma->max > MEMSIZE_THRESHOLD) {
            MemHandleFree (pma->data);
            pma->data = 0;
            pma->max = 0;
        }
    }
    pma->cur = 0;
    return 0;
}

int
ma_clear (memarr_t *pma, memarr_cb_destroy_t cb_destroy)
{
    assert(0 != pma);
    ma_rmalldata (pma, cb_destroy);
    if(0 != pma->data) {
        MemHandleFree (pma->data);
    }
    memset (pma, 0, sizeof (*pma));
    return 0;
}


int
#ifdef DEBUG
ma_insert_dbg (memarr_t *pma, size_t idx, void * data, const char *cstr_func, const char *cstr_file, int line)
#else
ma_insert (memarr_t *pma, size_t idx, void * data)
#endif
{
    char * realdata;
    char * pidx;
    size_t sz_cur;

    assert(0 != pma);
    assert(pma->itemsize > 0);

    sz_cur = pma->cur;
    if (idx > sz_cur) {
        sz_cur = idx + 1;
    }
    if(pma->max <= sz_cur)
    {
        MemHandle newdata;
        sz_cur -= pma->max;
        if (sz_cur < pma->inc) {
            sz_cur = pma->inc;
        }
        newdata = (MemHandle)MA_MEMREALLOC (pma->data, pma->itemsize * (pma->max + sz_cur));
        //newdata = mwRealloc((void *)(pma->data), (int)(pma->itemsize * (pma->max + sz_cur)), cstr_file, line);
        if(0 == newdata) {
            fprintf (stderr, "Err ma_insert(): realloc() error\n");
            return -1;
        }
#ifdef DEBUG
        MA_MEMMARK (pma->data, /*description*/cstr_func, /*file*/cstr_file, /*line*/line);
#endif
        realdata = (char *)MA_MEMLOCK (newdata);
        if (NULL == realdata) { //if (DmGetLastErr()) {
            fprintf (stderr, "Err ma_insert(): mem lock error\n");
            return -1;
        }
        memset (realdata + pma->cur * pma->itemsize, 0, pma->itemsize * (pma->max - pma->cur));
        MA_MEMUNLOCK (newdata);
        pma->max += sz_cur;
        pma->data = newdata;
    }
    realdata = (char *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        fprintf (stderr, "Err ma_insert(): mem lock error\n");
        return -1;
    }
    assert (idx >= 0);
    pidx = realdata + (idx * pma->itemsize);
    if (idx < pma->cur) {
        memmove (pidx + pma->itemsize, pidx, (pma->cur - idx) * pma->itemsize);
    }
    memmove (pidx, data, pma->itemsize);
    MA_MEMUNLOCK (pma->data);
    if (idx > pma->cur) {
        pma->cur = idx + 1;
    } else {
        pma->cur ++;
    }
    return 0;
}

int
#ifdef DEBUG
ma_replace_dbg (memarr_t *pma, size_t idx, void * data, const char *cstr_func, const char *cstr_file, int line)
#else
ma_replace (memarr_t *pma, size_t idx, void * data)
#endif
{
    char * realdata;
    char * pidx;
    assert(0 != pma);
    assert(pma->itemsize > 0);
    if (idx >= pma->cur) {
        int ret;
        ret = ma_insert (pma, idx, data);
#ifdef DEBUG
        MA_MEMMARK (pma->data, /*description*/cstr_func, /*file*/cstr_file, /*line*/line);
#endif
        return ret;
    }
    realdata = (char *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        return -1;
    }
    pidx = realdata + (idx * pma->itemsize);
    memmove (pidx, data, pma->itemsize);
    MA_MEMUNLOCK (pma->data);
    return 0;
}

int
ma_data (const memarr_t *pma, size_t idx, void * data)
{
    char * realdata;
    assert (0 != pma);
    assert (0 != data);

    if ((pma->cur < 1) || (idx >= pma->cur))
        return -1;

    realdata = (char *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        return -1;
    }
    memmove (data, realdata + (idx * pma->itemsize), pma->itemsize);
    MA_MEMUNLOCK (pma->data);
    return 0;
}

#if __GNUC__

#else
void *
ma_data_lock (memarr_t *pma, size_t idx)
{
    char * realdata;
    assert (0 != pma);

    // TODO:
    // 同一个 pma ma_inc 申请内存可能导致其基址被改变，而之前如果有另外一个lock的（如只读的），则其指针在这次ma_inc 后失效(被free的内存空间)
    // 所以需要重新获取 （ma_data_lock） 指针。
    // 所以最好限制为每次只允许一个 lock, 或者, 被 lock 之后不允许 ma_inc 而抛出异常 (使用 assert)

    if ((pma->cur < 1) || (idx >= pma->cur))
        return NULL;

    realdata = (char *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        MA_MEMUNLOCK (pma->data);
        return NULL;
    }
    return realdata + (idx * pma->itemsize);
}
#endif

/*
void
ma_data_unlock (memarr_t *pma, size_t idx)
{
    MA_MEMUNLOCK (pma->data);
}
*/

int
#ifdef DEBUG
ma_copy_dbg (memarr_t *pma_to, memarr_t *pma_from, const char *cstr_func, const char *cstr_file, int line)
#else
ma_copy (memarr_t *pma_to, memarr_t *pma_from)
#endif
{
    char * realdata_to;
    char * realdata_from;
    //char * pidx;

    assert(0 != pma_to);
    assert(0 != pma_from);
    assert(pma_to->itemsize > 0);
    assert(pma_from->itemsize > 0);
    // 调整数据大小
    pma_to->inc = pma_from->inc;
    pma_to->itemsize = pma_from->itemsize;
    pma_to->max = ((pma_to->max * pma_to->itemsize) / pma_from->itemsize);
    if (pma_to->max * pma_from->itemsize < pma_from->cur)
    {
        MemHandle newdata;
        newdata = MA_MEMREALLOC (pma_to->data, pma_from->itemsize * (pma_from->cur + pma_from->inc));
        if(0 == newdata) {
            fprintf (stderr, "Err ma_insert(): realloc() error\n");
            return -1;
        }
        pma_to->max += pma_from->inc;
        pma_to->data = newdata;
    }
    pma_to->cur = pma_from->cur;
    realdata_to = (char *)MA_MEMLOCK (pma_to->data);
    if (NULL == realdata_to) { //if (DmGetLastErr()) {
        // some error
        fprintf (stderr, "Err ma_insert(): mem lock error\n");
        return -1;
    }
    realdata_from = (char *)MA_MEMLOCK (pma_from->data);
    if (NULL == realdata_from) { //if (DmGetLastErr()) {
        // some error
        MA_MEMUNLOCK (pma_to->data);
        fprintf (stderr, "Err ma_insert(): mem lock error\n");
        return -1;
    }
    memmove (realdata_to, realdata_from, pma_to->itemsize * pma_from->cur);
    MA_MEMUNLOCK (pma_to->data);
    MA_MEMUNLOCK (pma_from->data);

    return 0;
}

// the pma_to should be clear and inited befor this function.
int
#ifdef DEBUG
ma_transfer_dbg (memarr_t *pma_to, memarr_t *pma_from, memarr_cb_destroy_t cb_destroy, const char *cstr_func, const char *cstr_file, int line)
#else
ma_transfer (memarr_t *pma_to, memarr_t *pma_from, memarr_cb_destroy_t cb_destroy)
#endif
{
    ma_clear (pma_to, cb_destroy);
    memmove (pma_to, pma_from, sizeof (*pma_from));
    memset (pma_from, 0, sizeof (*pma_from));
    ma_init (pma_from, pma_to->itemsize);
    return 0;
}

/*return the index of the item that just added*/
ssize_t
ma_inc (memarr_t *pma)
{
    char *tmpbuf;
    tmpbuf = (char *)malloc (pma->itemsize);
    memset (tmpbuf, 0, pma->itemsize);
    ma_insert (pma, pma->cur, tmpbuf);
    free (tmpbuf);

    return pma->cur - 1;
}

int
ma_rmdata (memarr_t *pma, size_t idx, void * data)
{
    char * realdata;
    assert (0 != pma);

    if ((pma->cur < 1) || (idx >= pma->cur))
        return -1;

    realdata = (char *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        return -1;
    }
    if (NULL != data) {
        memmove (data, realdata + (idx * pma->itemsize), pma->itemsize);
    }
    if (idx + 1 < pma->cur) {
        memmove (realdata + idx * pma->itemsize, realdata + (idx + 1) * pma->itemsize, (pma->cur - idx - 1) * pma->itemsize);
    }
    MA_MEMUNLOCK (pma->data);
    pma->cur --;
    return 0;
}

int
slist_init (sortedlist_t *psl, size_t datasize, void *userdata, slist_cb_comp_t cb_comp, slist_cb_swap_t cb_swap)
{
    memset (psl, 0, sizeof (*psl));
    if (ma_init (&(psl->marr), datasize) < 0) {
        return -1;
    }
    psl->cb_comp = cb_comp;
    psl->cb_swap = cb_swap;
    psl->userdata = userdata;
    psl->cb_comp_heter = NULL;
    return 0;
}

int
slist_clear (sortedlist_t *psl, slist_cb_destroy_t cb_destroy)
{
    return ma_clear (&(psl->marr), cb_destroy);
}

CODE_SECTION_UTILS static int
bsearch_cb_comp_slist (void *userdata, size_t idx, void * data_pin)
{
    void * p;
    int ret;
    sortedlist_t *psl = (sortedlist_t *)userdata;

    assert (NULL != psl);

    p = ma_data_lock (&((psl)->marr), idx);
    if (psl->cb_comp_heter) {
        ret = (psl)->cb_comp_heter (psl->userdata, data_pin, p);
    } else {
        ret = (psl)->cb_comp (psl->userdata, data_pin, p);
    }
    ma_data_unlock (&((psl)->marr), idx);

    return ret;
}

/*
int
_slist_rec_find (sortedlist_t *psl, void *data_pinpoint, size_t *ret_idx)
{
    return pf_bsearch_r ((psl), slist_size (psl), bsearch_cb_comp_slist, data_pinpoint, ret_idx);
}
*/
#define _slist_rec_find(psl, data_pinpoint, ret_idx) pf_bsearch_r ((void *)(psl), slist_size (psl), bsearch_cb_comp_slist, (data_pinpoint), (ret_idx))

static void
swap_buffer (size_t sz_data, uint8_t *pdata1, uint8_t *pdata2)
{
    size_t sz_cpy;
    uint8_t buf[50];
    for (; sz_data > 0; ) {
        if (sz_data > sizeof (buf)) {
            sz_cpy = sizeof (buf);
        } else {
            sz_cpy = sz_data;
        }
        memmove (buf, pdata1, sz_cpy);
        memmove (pdata1, pdata2, sz_cpy);
        memmove (pdata2, buf, sz_cpy);
        sz_data -= sz_cpy;
    }
}

int
#ifdef DEBUG
slist_store_dbg (sortedlist_t *psl, void *data, const char *cstr_func, const char *cstr_file, int line)
#else
slist_store (sortedlist_t *psl, void *data)
#endif
{
    size_t idx;
    assert (NULL == psl->cb_comp_heter);
    if (_slist_rec_find (psl, data, &idx) < 0) {
#ifdef DEBUG
        return ma_insert_dbg (&(psl->marr), idx, data, cstr_func, cstr_file, line);
#else
        return ma_insert (&(psl->marr), idx, data);
#endif
    }
    //fprintf (stderr, "Err slist_store(): Data exist! data=0x%x, idx=%d\n", (int)data, idx);
#if DEBUG
    //exit (1);
    //assert (0);
#endif
    return -1;
}

int
slist_find (sortedlist_t *psl, void *data_pinpoint, slist_cb_comp_t cb_comp_heter, size_t *ret_idx)
{
    int ret;
    size_t idx;
    ret = 0;
    if (cb_comp_heter) {
        psl->cb_comp_heter = cb_comp_heter;
    } else {
        psl->cb_comp_heter = NULL;
    }
    if (pf_bsearch_r (psl, slist_size (psl), bsearch_cb_comp_slist, data_pinpoint, &idx) < 0) {
        ret = -1;
    }
    psl->cb_comp_heter = NULL;
    if (NULL != ret_idx) {
        *ret_idx = idx;
    }
    return ret;
}

typedef struct _masort_tmp_t {
    memarr_t *pmarr;
    uint8_t * realdata;
    slist_cb_comp_t cb_comp;
    slist_cb_swap_t cb_swap;
    void *userdata;
} masort_tmp_t;

int
pf_sort_cb_comp_masort (void *userdata, size_t idx1, size_t idx2)
{
    masort_tmp_t *pmastmp = (masort_tmp_t *)userdata;
    assert (NULL != userdata);
    return pmastmp->cb_comp (pmastmp->userdata,
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx1),
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx2)
                );
}

int
pf_sort_cb_swap_masort (void *userdata, size_t idx1, size_t idx2)
{
    masort_tmp_t *pmastmp = (masort_tmp_t *)userdata;
    assert (NULL != userdata);
    if (pmastmp->cb_swap) {
        return pmastmp->cb_swap (pmastmp->userdata,
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx1),
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx2)
                );
    } else {
        swap_buffer (pmastmp->pmarr->itemsize,
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx1),
                pmastmp->realdata + (pmastmp->pmarr->itemsize * idx2));
        return 0;
    }
}

int
ma_sort (memarr_t *pma, void * userdata, ma_cb_comp_t cb_comp, ma_cb_swap_t cb_swap)
{
    uint8_t * realdata;
    char * pidx;
    masort_tmp_t mastmp;

    assert(0 != pma);
    assert(pma->itemsize > 0);

    realdata = (uint8_t *)MA_MEMLOCK (pma->data);
    if (NULL == realdata) { //if (DmGetLastErr()) {
        // some error
        fprintf (stderr, "Err ma_insert(): mem lock error\n");
        return -1;
    }
    mastmp.pmarr = pma;
    mastmp.cb_comp = cb_comp;
    mastmp.cb_swap = cb_swap;
    mastmp.userdata = userdata;
    mastmp.realdata = realdata;
    //pf_qsort_r (realdata, pma->cur, pma->itemsize, userdata, cb_comp);
    pf_heapsort_r (&mastmp, pma->cur, pf_sort_cb_comp_masort, pf_sort_cb_swap_masort);

    MA_MEMUNLOCK (pma->data);

    return 0;
}

int
slist_sort (sortedlist_t *psl)
{
    return ma_sort (&(psl->marr), psl->userdata, psl->cb_comp, psl->cb_swap);
}

/*"*data1 - *data2"*/
int
slist_cb_comp_sizet (void *userdata, void * data1, void * data2)
{
    if (*((size_t *)data1) > *((size_t *)data2)) {
        return 1;
    }
    if (*((size_t *)data1) < *((size_t *)data2)) {
        return -1;
    }
    return 0;
}

int
slist_cb_swap_sizet (void *userdata, void * data1, void * data2)
{
    size_t tmp;
    tmp = *((size_t *)data1);
    *((size_t *)data1) = *((size_t *)data2);
    *((size_t *)data2) = tmp;
    return 0;
}
