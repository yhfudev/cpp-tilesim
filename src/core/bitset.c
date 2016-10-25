/******************************************************************************
 * Name:        bitset.c
 * Purpose:     Set the bit
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "pfutils.h" // _PSF_BEGIN_EXTERN_C
#include "bitset.h"

//#define DEBUG 1
#if DEBUG
typedef struct _bit_map_t {
    unsigned char *buf;
    size_t maxnum;
} bit_map_t;
#endif

// clear all of the bit to 0
void
bm_reset (unsigned char *buf, size_t num)
{
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    assert (num == pbm->maxnum);
    buf = pbm->buf;
#endif
    memset (buf, 0, (num + 7) / 8);
}

// set all of the bit to 1
void
bm_setall (unsigned char *buf, size_t num)
{
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    assert (num == pbm->maxnum);
    buf = pbm->buf;
#endif
    memset (buf, 0xFF, (num + 7) / 8);
}

#if MEMWATCH
#define MA_MEMMARK(p, cstr_func, cstr_file, line) mwMark((p), (cstr_func), (cstr_file), (line));
#else
#define MA_MEMMARK(p, cstr_func, cstr_file, line)
#endif

#if DEBUG
unsigned char *
bm_create_dbg (size_t num, const char *cstr_func, const char *cstr_file, int line)
#else
unsigned char *
bm_create (size_t num)
#endif
{
    unsigned char * buf;
#if DEBUG
    bit_map_t *pbm;
    pbm = (bit_map_t *)malloc (sizeof (*pbm));
    if (NULL == pbm) {
        return NULL;
    }

    MA_MEMMARK (pma->data, /*description*/cstr_func, /*file*/cstr_file, /*line*/line);

    pbm->maxnum = num;
#endif

    num = (num + 7) / 8;
    if (num < 20) {
        num = 20;
    }
    buf = (unsigned char *)malloc (num);
    if (NULL == buf) {
#if DEBUG
        free (pbm);
#endif
        return NULL;
    }
    memset (buf, 0, num);

#if DEBUG
    pbm->buf = buf;
    buf = (unsigned char *)pbm;
#endif

    return buf;
}

void
bm_free (unsigned char *buf)
{
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    buf = pbm->buf;
    free (pbm);
#endif
    free (buf);
}

// set one bit at the position
void
bm_set (unsigned char *buf, size_t pos)
{
    unsigned char ch = 0x01;
    int seg = pos % 8;
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    assert (pos < pbm->maxnum);
    buf = pbm->buf;
#endif
    buf += (pos / 8);
    ch <<= seg;
    *buf |= ch;
}

// clear one bit at the position pos
void
bm_clr (unsigned char *buf, size_t pos)
{
    unsigned char ch = 0x01;
    int seg = pos % 8;
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    assert (pos < pbm->maxnum);
    buf = pbm->buf;
#endif
    buf += (pos / 8);
    ch <<= seg;
    *buf &= (~ch);
}

// check if the bit is set 1, if so, return 1, otherwise return 0
#if DEBUG
int
bm_is_set (unsigned char *buf, size_t pos)
{
    int seg = pos % 8;
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf;
    assert (pos < pbm->maxnum);
    buf = pbm->buf;
#endif
    buf += (pos / 8);
    return (((*buf) >> seg) & 0x01);
}
#endif

// compare two bitset, return 0 if equal
int
bm_cmp (unsigned char *buf1, unsigned char *buf2, size_t len)
{
    //unsigned char ch1;
    //unsigned char ch2;
    int ret;
#if DEBUG
    bit_map_t *pbm;
    pbm = (bit_map_t *)buf1;
    assert (len == pbm->maxnum);
    buf1 = pbm->buf;
    pbm = (bit_map_t *)buf2;
    assert (len == pbm->maxnum);
    buf2 = pbm->buf;
#endif

    if (len > 8) {
        ret = memcmp (buf1, buf2, len / 8);
        if (0 != ret) {
            return ret;
        }
    }
    ret = 0x01;
    ret <<= len % 8;
    ret --;
    buf1 += (len / 8);
    buf2 += (len / 8);
    *buf1 &= ret;
    *buf2 &= ret;
    return ((int)(*buf1) - (int)(*buf2));
}

/*
  the rotation of bitset2d, return the buf_new, if buf_new == nul, then create one.
  num: the times to rotate clockwise, 90 degree each times
 */
unsigned char *
bm2d_rotate (unsigned char *buf_orig, unsigned char * buf_new,
    size_t maxx, size_t maxy, int num)
{
    size_t i, j;
    size_t newx, newy;
#if DEBUG
    bit_map_t *pbm = (bit_map_t *)buf_orig;
    assert (maxx * maxy == pbm->maxnum);
#endif

    if (NULL == buf_new) {
        buf_new = BM2D_CREATE (maxx, maxy);
    }
    if (NULL == buf_new) {
        return NULL;
    }
    BM2D_RESET (buf_new, maxx, maxy);
    switch (num % 4) {
    /*
        origin: maxx, maxy; x0, y0
        90: [maxy,maxx]
            X90 = y0
            Y90 = maxx - 1 - x0
        180: [maxx,maxy]
            X180 = maxx - 1 - x0
            Y180 = maxy - 1 - y0
        270: [maxy,maxx]
            X270 = maxy - 1 - y0
            Y270 = x0
    */
    case 0:
        for (i = 0; i < maxy; i ++) {
            for (j = 0; j < maxx; j ++) {
                newx = j;
                newy = i;
                if (BM2D_IS_SET (buf_orig, maxx, j, i)) {
                    BM2D_SET (buf_new, maxx, newx, newy);
                }
            }
        }
        break;
    case 1: /* 90 */
        for (i = 0; i < maxy; i ++) {
            for (j = 0; j < maxx; j ++) {
                newx = i;
                newy = maxx - 1 - j;
                if (BM2D_IS_SET (buf_orig, maxx, j, i)) {
                    BM2D_SET (buf_new, maxy, newx, newy);
                }
            }
        }
        break;
    case 2: /* 180 */
        for (i = 0; i < maxy; i ++) {
            for (j = 0; j < maxx; j ++) {
                newx = maxx - 1 - j;
                newy = maxy - 1 - i;
                if (BM2D_IS_SET (buf_orig, maxx, j, i)) {
                    BM2D_SET (buf_new, maxx, newx, newy);
                }
            }
        }
        break;
    case 3: /* 270 */
        for (i = 0; i < maxy; i ++) {
            for (j = 0; j < maxx; j ++) {
                newx = maxy - 1 - i;
                newy = j;
                if (BM2D_IS_SET (buf_orig, maxx, j, i)) {
                    BM2D_SET (buf_new, maxy, newx, newy);
                }
            }
        }
        break;
    default:
        break;
    }
    return buf_new;
}

///////////////////////////////////////////////////////
#include <assert.h>

//#define NUM_TYPE(p,type) (sizeof(p)/sizeof(type))

// test function for bitset
void
test_bitset (void)
{
    size_t i;
    unsigned char * bm_tst;
    int array[100];

    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        array [i] = rand () % 2;
    }
    bm_tst = bm_create (NUM_TYPE (array, int));
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (bm_is_set (bm_tst, i)) {
            assert (0);
            break;
        }
    }

    // test set
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (array[i]) {
            bm_set (bm_tst, i);
        }
    }
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (array[i]) {
            if (0 == bm_is_set (bm_tst, i)) {
                assert (0);
                break;
            }
        }
    }

    // test reset
    bm_reset (bm_tst, NUM_TYPE (array, int));
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (bm_is_set (bm_tst, i)) {
            assert (0);
            break;
        }
    }

    // test clr
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (array[i]) {
            bm_clr (bm_tst, i);
        } else {
            bm_set (bm_tst, i);
        }
    }
    for (i = 0; i < NUM_TYPE (array, int); i ++) {
        if (bm_is_set (bm_tst, i)) {
            if (array[i]) {
                assert (0);
                break;
            }
        } else {
            if (0 == array[i]) {
                assert (0);
                break;
            }
        }
    }

    memset (array, 0, sizeof(array));
    bm_reset (bm_tst, NUM_TYPE (array, int));
    for (i = 10000; i > 0; i --) {
        size_t j;
        j = rand () % NUM_TYPE (array, int);
        array [j] = rand () % 2;
        if (array [j]) {
            bm_set (bm_tst, j);
        } else {
            bm_clr (bm_tst, j);
        }
        for (j = 0; j < NUM_TYPE (array, int); j ++) {
            if (array [j] != bm_is_set (bm_tst, j)) {
                assert (0);
            }
        }
    }
}
