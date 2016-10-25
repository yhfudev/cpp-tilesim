/******************************************************************************
 * Name:        gentypeenc.h
 * Purpose:     generate the tiles of type DWW1, DWW2, TEW1, TEW2, TWW2, DEE1, DEE2, TWE1, TWE2, TEE2
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GENERATE_CAL_TYPE_H
#define _GENERATE_CAL_TYPE_H

#include "gentypeconn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GENCAL_TYPE_DXX1 0x01
#define GENCAL_TYPE_DXX2 0x02
#define GENCAL_TYPE_TYX1 0x03
#define GENCAL_TYPE_TYX2 0x04
#define GENCAL_TYPE_TXX2 0x05
int gen_enc_tile_to_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t maxbits, size_t k);
int gen_enc_tile_to_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t maxbits, size_t k);

#define GEN_ENC_TILE_DWW1(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_west(fpout,GENCAL_TYPE_DXX1,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_DWW1(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_DWW2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_west(fpout,GENCAL_TYPE_DXX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_DWW2(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TEW1(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_west(fpout,GENCAL_TYPE_TYX1,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TEW1(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TEW2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_west(fpout,GENCAL_TYPE_TYX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TEW2(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TWW2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_west(fpout,GENCAL_TYPE_TXX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TWW2(fpout,group,tname,gin,gout,maxbits,k))

#define GEN_ENC_TILE_DEE1(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_east(fpout,GENCAL_TYPE_DXX1,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_DEE1(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_DEE2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_east(fpout,GENCAL_TYPE_DXX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_DEE2(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TWE1(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_east(fpout,GENCAL_TYPE_TYX1,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TWE1(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TWE2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_east(fpout,GENCAL_TYPE_TYX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TWE2(fpout,group,tname,gin,gout,maxbits,k))
#define GEN_ENC_TILE_TEE2(fpout,group,tname,gin,gout,enc,maxbits,k) ((NULL!=enc)?gen_enc_tile_to_east(fpout,GENCAL_TYPE_TXX2,group,tname,gin,gout,enc,maxbits,k):GEN_CONN_TILE_TEE2(fpout,group,tname,gin,gout,maxbits,k))

#ifdef __cplusplus
}
#endif

#endif /* _GENERATE_CAL_TYPE_H */
