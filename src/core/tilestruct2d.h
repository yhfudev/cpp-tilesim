/******************************************************************************
 * Name:        tilestruct2d.h
 * Purpose:     some core functions for the tiles
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     LGPL licence
 ******************************************************************************/
#ifndef TILESTRUCT2D_H_INCLUDED
#define TILESTRUCT2D_H_INCLUDED

#include "pfutils.h"
#include "tilestruct.h"

_PSF_BEGIN_EXTERN_C

// otherwise return 0
int tstc_is_equal_2d (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable);
int tstc_compare_2d (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable);

tstilecomb_t * tstc_rotate_2d (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, int num, tsposition_t *ptrans);

/* merge tc_base with tc_test which is rotated by dir and is at the position ppos(x,y) adopt the slidetest coordinate system,
   the result is stored in tc_result */
extern int tstc_merge_2d (tstilecomb_t *tc_base, tstilecomb_t *tc_test,
    char dir, tsposition_t *ppos, tstilecomb_t *tc_result);

// nomalize the ptc: let the left bottom of the supertile segment move to the (0,0) position
extern int tstc_nomalize_2d (tstilecomb_t *ptc);

extern int tile_get_glue_2d (tstile_t *ptilevec, size_t maxtvec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum);

/* test if two tilecomp can be merge; if ok, return 0 and the position(x,y); otherwise return -1 */
extern int tstc_mesh_test2d_godhand (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok);
extern int tstc_mesh_test2d_nature (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok);

extern int tstc_split_2d (tstilecomb_t *ptc_2bsplit, tssiminfo_t *psim, size_t temperature, memarr_t *plist_splited);

_PSF_END_EXTERN_C
#endif // TILESTRUCT_H_INCLUDED
