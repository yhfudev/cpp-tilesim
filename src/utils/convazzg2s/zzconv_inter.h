/******************************************************************************
 * Name:        zzconv_inter.h
 * Purpose:     Convert the arbitrary zig-zag tile set from 2D2T to 2D1T
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _ZIGZAG_CONVERTOR_INTER_H
#define _ZIGZAG_CONVERTOR_INTER_H

#include <stddef.h> // size_t

#include <map>
#include <string>

#include "zzconv.h"

typedef struct _glue_info_t {
    size_t strength;
    size_t encoding;
} glue_info_t;

typedef std::map<std::string, glue_info_t> glue_enc_t;

struct _zzconv_t {
    int rotnum; // the rotation of the zigzag system. 
                //   0: the direction for the layer growth is from down to up (south to north);
                //   1: from left to rigth (west to east);
                //   2: from up to down (north to south);
                //   3: from right to left (east to west);
    zigzag_info_t *ref_pzzi; // the parameter pass to the zzconv_create() by user
    FILE *fpout;

    std::map<std::string, size_t> glue_strength; // all of the strength of glues
    glue_enc_t glue_info; // the encoded glue information
    size_t maxbits;
    size_t k; // for zig-zag 2D temperature 1
};

#endif /* _ZIGZAG_CONVERTOR_INTER_H */
