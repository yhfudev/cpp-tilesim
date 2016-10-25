/******************************************************************************
 * Name:        zzconv.h
 * Purpose:     Convert the arbitrary zig-zag tile set from 2D2T to 2D1T
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _ZIGZAG_CONVERTOR_H
#define _ZIGZAG_CONVERTOR_H

#include "zzinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _zzconv_t;
typedef struct _zzconv_t zzconv_t;

// pzzi should be a categoried tile set.
extern zzconv_t * zzconv_create (zigzag_info_t *pzzi);

extern int zzconv_destroy (zzconv_t *pzzc);

extern int zzconv_conv_categories_to_3d1t (zzconv_t *pzzc, FILE *fpout);

// zig-zag 2D temperature 1, probabilistic zigzag tile set
extern int zzconv_conv_categories_to_2d1t (zzconv_t *pzzc, FILE *fpout, size_t k);

#ifdef __cplusplus
}
#endif

#endif /* _ZIGZAG_CONVERTOR_H */
