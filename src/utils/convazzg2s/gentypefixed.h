/******************************************************************************
 * Name:        gentypefixed.h
 * Purpose:     generate the tiles of type FW/FE
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GENERATE_FIXED_TYPE_H
#define _GENERATE_FIXED_TYPE_H

#include "gentypeconn.h"

#ifdef __cplusplus
extern "C" {
#endif

int gen_fixed_tile_to_west (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k);
int gen_fixed_tile_to_east (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k);

#define GEN_FIXED_TILE_FW(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_fixed_tile_to_west(fpout,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_FW(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_FIXED_TILE_FE(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_fixed_tile_to_east(fpout,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_FE(fpout,group,tname,gin,gout,maxbits,k))

#ifdef __cplusplus
}
#endif

#endif /* _GENERATE_FIXED_TYPE_H */
