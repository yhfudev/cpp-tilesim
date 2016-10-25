/******************************************************************************
 * Name:        gentypeconn.h
 * Purpose:     generate the tiles of type DEN2, DWN2, SWN2, SEN2
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GENERATE_CONN_TYPE_H
#define _GENERATE_CONN_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define GENCONN_TYPE_DXN2 0x01
#define GENCONN_TYPE_SXN2 0x02

// the connecting version(north side is null) of DWW1,DWW2,TEW1,TEW2,TWW2; DEE1,DEE2,TWE1,TWE2,TEE2
#define GENCONN_TYPE_DXX1 0x03
#define GENCONN_TYPE_DXX2 0x04
#define GENCONN_TYPE_TYX1 0x05
#define GENCONN_TYPE_TYX2 0x06
#define GENCONN_TYPE_TXX2 0x07
#define GENCONN_TYPE_FX   0x08

// DWN2,DEN2,SWN2,SEN2
int gen_conn_tile_at_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k);
int gen_conn_tile_at_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k);

//DWW1,DWW2,TEW1,TEW2,TWW2; DEE1,DEE2,TWE1,TWE2,TEE2
int gen_conn_tile_to_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k);
int gen_conn_tile_to_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k);

#define GEN_CONN_TILE_DWN2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_at_west (fpout,GENCONN_TYPE_DXN2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_DEN2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_at_east (fpout,GENCONN_TYPE_DXN2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_SWN2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_at_west (fpout,GENCONN_TYPE_SXN2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_SEN2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_at_east (fpout,GENCONN_TYPE_SXN2,group,tname,gin,gout,maxbits,k)

#define GEN_CONN_TILE_DWW1(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_DXX1,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_DWW2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_DXX2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TEW1(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_TYX1,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TEW2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_TYX2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TWW2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_TXX2,group,tname,gin,gout,maxbits,k)

#define GEN_CONN_TILE_DEE1(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_DXX1,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_DEE2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_DXX2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TWE1(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_TYX1,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TWE2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_TYX2,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_TEE2(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_TXX2,group,tname,gin,gout,maxbits,k)

#define GEN_CONN_TILE_FW(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_west (fpout,GENCONN_TYPE_FX,group,tname,gin,gout,maxbits,k)
#define GEN_CONN_TILE_FE(fpout,group,tname,gin,gout,maxbits,k) gen_conn_tile_to_east (fpout,GENCONN_TYPE_FX,group,tname,gin,gout,maxbits,k)

#ifdef __cplusplus
}
#endif

#endif /* _GENERATE_CONN_TYPE_H */
