/******************************************************************************
 * Name:        zzinfo.h
 * Purpose:     functions of the tile set information, such as load data from file,
 *              convertions between the TM and zig-zag tiles.
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _ZIGZAG_INFORMATION_H
#define _ZIGZAG_INFORMATION_H

#include "pfutils.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#define strtok_r(a,b,c) strtok((a),(b))
#else
#include <arpa/inet.h> // htonl()
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NUM_TYPE
#define NUM_TYPE(p,typ) (sizeof(p)/sizeof(typ))
#endif

typedef struct _glue_t {
    char *name;
    size_t strength;
} glue_t;

// utilities for glue array
size_t gluearr_name2idx (glue_t *pglue, size_t sz_glues, const char *gluename);
size_t gluearr_name2val (glue_t *pglue, size_t sz_glues, const char *gluename);

#define TILETYPE_NONE      0x00
#define TILETYPE_UNKNOWN   0x01

#define TILETYPE_DWW1 0x02
#define TILETYPE_DWW2 0x03
#define TILETYPE_TEW1 0x04
#define TILETYPE_TEW2 0x05
#define TILETYPE_TWW2 0x06
#define TILETYPE_DEE1 0x07
#define TILETYPE_DEE2 0x08
#define TILETYPE_TWE1 0x09
#define TILETYPE_TWE2 0x0A
#define TILETYPE_TEE2 0x0B
#define TILETYPE_FE   0x0C
#define TILETYPE_FW   0x0D
#define TILETYPE_SWN2 0x0E
#define TILETYPE_SEN2 0x0F
#define TILETYPE_DWN2 0x10
#define TILETYPE_DEN2 0x11

#define TILETYPE_MAX 0x12

typedef struct _tile_t {
    char *gN;
    char *gE;
    char *gS;
    char *gW;
    char *name;
    char *label;
    char type; // left-side, right-side, dir-left, dir-right
    size_t group;
} tile_t;

typedef struct _zigzag_info_t {
    // the input:
    size_t sz_tiles;
    tile_t *tiles;
    size_t sz_glues;
    glue_t *glues;
    size_t temperature;

    // the result:
    size_t pos_max_x;  // the tile x range: 0 ~ pos_max_x
    size_t pos_max_y;  // the tile y range: 0 ~ pos_max_y
    size_t pos_seed_x; // seed is at (pos_seed_x, 0)
    size_t idx_last;   // the last tile id.
    size_t tiles_filled;  // the number of tested steps
} zigzag_info_t;

const char * tiletype_val2cstr (int type);
size_t gluearr_name2idx (glue_t *pglue, size_t sz_glues, const char *gluename);
size_t gluearr_name2val (glue_t *pglue, size_t sz_glues, const char *gluename);

void zzi_init (zigzag_info_t *pzzi);
// don't try to use zzi_clear() with static zigzag_info_t
void zzi_clear (zigzag_info_t *pzzi);
//int zzi_load_tds (const char *fn_tas, zigzag_info_t *pzzi);
int zzi_load_tdp (const char *fn_tas, zigzag_info_t *pzzi);

int zzi_comapre (zigzag_info_t *pzzi_old, zigzag_info_t *pzzi_new, char flg_chktype);
int zzi_copy (zigzag_info_t *pzzi_dest, zigzag_info_t *pzzi_src);
#if DEBUG
void zzi_output_c (zigzag_info_t *pzzi, char *name, FILE *fpout);
#else
#define zzi_output_c(a,b,c)
#endif
void zzi_output_buf_tilecolor (char *buffer, size_t group, char type);

void zzi_output_tds_2d2t (zigzag_info_t *pzzi, FILE *fpout);
void zzi_output_xgrow_tiles (zigzag_info_t *pzzi, FILE *fpout);

#define TM_TRAN_MOVE_NOMOVE 0x00
#define TM_TRAN_MOVE_LEFT   0x01
#define TM_TRAN_MOVE_RIGHT  0x02
typedef struct _tm_tranfunc_t {
    char *state_in; // q
    char *input;    // c
    char *state_out; // q'
    char *output;    // c'
    size_t move; // L-move left, R-move right, or H-don't move
} tm_tranfunc_t;

typedef struct tm_info_t {
    size_t sz_tranfunc;
    tm_tranfunc_t *tranfunc;
    size_t sz_tape;
    char **tape;
    char *q0;
} tm_info_t;

int zzi_tmi_to_zigzag2d2t (zigzag_info_t *pzzi, tm_info_t *ptmi);

// utilities
typedef struct _zigzag_form_t {
    size_t sz_buf_tilevec;
    zigzag_info_t *pzzi; // zzi_init () before set this.

    memarr_t ma_val;
    memarr_t ma_cstr;
    sortedlist_t sl_label;
} zigzag_form_t;
void zzf_init (zigzag_form_t *pzzf, zigzag_info_t *pzzi);
void zzf_clear (zigzag_form_t *pzzf);
ssize_t zzf_feed_glue (zigzag_form_t *pzzf, const char *label, size_t strength);
int zzf_feed_tile (zigzag_form_t *pzzf, const tile_t *ptile);
void zzf_feed_end (zigzag_form_t *pzzf);

#ifdef __cplusplus
}
#endif

#endif /* _ZIGZAG_INFORMATION_H */
