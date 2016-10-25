/******************************************************************************
 * Name:        bitset.h
 * Purpose:     Set the bit
 * Author:      Yunhui Fu
 * Created:     2008-10-05
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef BITSET_H_INCLUDED
#define BITSET_H_INCLUDED

#include "pfutils.h"

_PSF_BEGIN_EXTERN_C

#if DEBUG
unsigned char * bm_create_dbg (size_t num, const char *cstr_func, const char *cstr_file, int line);
#define bm_create(num) bm_create_dbg(num,__func__,__FILE__,__LINE__)
#else
unsigned char * bm_create (size_t num);
#endif

void bm_free (unsigned char *buf);

// set one bit at the position
void bm_set (unsigned char *buf, size_t pos);
// clear one bit at the position pos
void bm_clr (unsigned char *buf, size_t pos);

// check if the bit is set 1, if so, return 1, otherwise return 0
#ifdef DEBUG
int bm_is_set (unsigned char *buf, size_t pos);
#else
#define bm_is_set(buf, pos) ((*(((unsigned char *)(buf)) + ((pos) / 8)) >> ((pos) % 8)) & 0x01)
#endif

// clear all of the bit to 0
void bm_reset (unsigned char *buf, size_t num);

// set all of the bit to 1
void bm_setall (unsigned char *buf, size_t num);

// compare two bitset, return 0 if equal
int bm_cmp (unsigned char *buf1, unsigned char *buf2, size_t len);

/*
  the rotation of bitset2d, return the buf_new, if buf_new == nul, then create one.
  rotnum: the times to rotate clockwise, 90 degree each times
 */
unsigned char * bm2d_rotate (unsigned char *buf_orig, unsigned char * buf_new, size_t maxx, size_t maxy, int rotnum);

#define BM2D_CREATE(maxx,maxy)    bm_create ((maxx)*(maxy))
#define BM2D_SET(buf,maxx,x,y)    bm_set (buf,((y)*(maxx))+(x))
#define BM2D_CLR(buf,maxx,x,y)    bm_clr (buf,((y)*(maxx))+(x))
#define BM2D_IS_SET(buf,maxx,x,y) bm_is_set (buf,((y)*(maxx))+(x))
#define BM2D_FREE(buf)            bm_free (buf)
#define BM2D_RESET(buf,maxx,maxy) bm_reset (buf,(maxx)*(maxy))

#define BM3D_CREATE(maxx,maxy,maxz)    bm_create ((maxx)*(maxy)*(maxz))
#define BM3D_SET(buf,maxx,maxy,x,y,z)  bm_set (buf,((z)*(maxx)*(maxy))+((y)*(maxx))+(x))
#define BM3D_CLR(buf,maxx,maxy,x,y)    bm_clr (buf,((z)*(maxx)*(maxy))+((y)*(maxx))+(x))
#define BM3D_IS_SET(buf,maxx,maxy,x,y) bm_is_set (buf,((z)*(maxx)*(maxy))+((y)*(maxx))+(x))
#define BM3D_FREE(buf)                 bm_free (buf)
#define BM3D_RESET(buf,maxx,maxy,maxz) bm_reset (buf,(maxx)*(maxy)*(maxz))

_PSF_END_EXTERN_C
#endif // BITSET_H_INCLUDED
