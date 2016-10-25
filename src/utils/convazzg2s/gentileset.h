/******************************************************************************
 * Name:        gentileset.h
 * Purpose:     declaration of the tile set generate functions.
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GENERATE_TILESET_H
#define _GENERATE_TILESET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gentypefixed.h"
#include "gentypeconn.h"
#include "gentypeenc.h"

// if the start value of the encoding is 0x00, then the INIT_VAL_ENC=-1, otherwise 0
//#define INIT_VAL_ENC ((size_t)(-1))
#define INIT_VAL_ENC (0)
// get bit from buffer, the bits in buffer is network byte order, MSB is indexed by 0.
#define NBO_GET_BIT(p, idx) (((((unsigned char *)(p))[(idx) / 8]) & (0x01 << (7 - ((idx) % 8))))?0x01:0x00)

int gen_tileset_dec (zzconv_t *pzzc);
int gen_tileset_enc (zzconv_t *pzzc);

#ifdef __cplusplus
}
#endif

#endif /* _GENERATE_TILESET_H */
