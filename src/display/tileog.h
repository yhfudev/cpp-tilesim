/******************************************************************************
 * Name:        tileog.h
 * Purpose:     Display the supertiles (2D mode) by OpenGL
 * Author:      Yunhui Fu
 * Created:     2009-08-22
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef TILEOG_H
#define TILEOG_H

#include "pfutils.h"
#include "bitset.h"
#include "tilestruct.h"
#include "glutfont.h"

_PSF_BEGIN_EXTERN_C

typedef void (* ogl_cb_postredisplay_t) (void);

extern int ogl_reset_view (int w, int h, tsposition_t *pmaxpos);
extern int ogl_reset_projection (int w, int h);
extern void ogl_initgl (ogl_cb_postredisplay_t cb_postredisplay);
extern void ogl_clear (void);
extern void ogl_drawother (void);
void ogl_jump2page (tssiminfo_t *ptsim, size_t page);
extern void ogl_current_page_inc (tssiminfo_t *ptsim);
extern void ogl_current_page_dec (tssiminfo_t *ptsim);
extern int ogl_set_target_tilesim (tssiminfo_t *ptsim);

extern void ogl_draw_tilesim (tssiminfo_t *ptsim, size_t idx_cur, char flg_draw_info);
extern void ogl_draw_current_tilesim (tssiminfo_t *ptsim, char flg_draw_info);

extern void ogl_on_mouse (int button, int state, int x, int y);
extern void ogl_on_mouse_motion (int x, int y);
extern void ogl_on_keyboard (unsigned char glutKey, int glutMod);

extern void ogl_reshape (int width, int height);
extern void ogl_on_special ( int key, int x, int y );

#if USE_PRESENTATION

typedef struct _tssim_adhere_info_t {
    tssiminfo_t *psim; // only use part 1 of the datastruct; set by the user

    // the following parameters are set by the callback functions
    tstilecomb_t *tc_base;
    tstilecomb_t *tc_test;
    tstilecombitem_t info_test; // 当前 test 状态，rotnum 表示 test 被旋转的次数，x,y,z 表示其位置

    memarr_t poslist; // the positions of the adhere information
                      // supertile上各个tile粘合位置
                      // 存储的是 tstilecombitem_t 类型的数据，其中 rotnum,x,y,z 的数据是从回调函数的
                      // posinfo 中获取，idtile被重定义为是否可以粘合的标记（0:不能粘合，1:可以粘合）。
                      // rotnum 为glue的位置,在该面上的 glue 可以和base上对应位置粘合
} tssim_adhere_info_t;

int tssim_cb_initadheredect_ogshow (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test);
int tssim_cb_adherepos_ogshow (void *userdata, char flg_canadhere, tstilecombitem_t * pposinfo);
int tssim_cb_clearadheredect_ogshow (void *userdata);

void tssim_adhere_display_ogshow (tssim_adhere_info_t * pinfo, const char *msg);
#endif

_PSF_END_EXTERN_C
#endif // TILEOG_H
