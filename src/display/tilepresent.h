/******************************************************************************
 * Name:        tilepresent.h
 * Purpose:     the code for presentation porpose
 * Author:      Yunhui Fu
 * Created:     2008-11-15
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef TILEPRESENT_H_INCLUDED
#define TILEPRESENT_H_INCLUDED

#include "pfutils.h"
#include "tilestruct.h"

_PSF_BEGIN_EXTERN_C

#if USE_PRESENTATION

typedef int (* tssim_test_cb_presentupdate_t) (void);
typedef struct _tsim_test_pres_t {
    char flg_quit;
    char flg_running; /*set by this thread, read by outer*/
    void * mutex;

    tssim_chkall_info_t chkallinfo;

    char flg_presentation;
    tssim_adhere_info_t adhereinfo;
    tssim_test_cb_presentupdate_t cb_update; // callback: update the display
    size_t delay_millisec; // 每步显示的间隔时间

    // used in sim
    memarr_t tilelist; // 各个 tile 的属性
    memarr_t countlist; // 各个 tile 的个数
    tssiminfo_t tsim;
} tsim_test_pres_t;

// delay_millisec: 每步显示的间隔时间
void ts_simulate_test_presentation_init (tsim_test_pres_t * pthinfo, tssim_test_cb_presentupdate_t cb_update, size_t delay_millisec);
void ts_simulate_test_presentation_clear (tsim_test_pres_t * pthinfo);
void ts_simulate_test_presentation_new_thread (tsim_test_pres_t * pthinfo);
void ts_simulate_test_presentation_draw (tsim_test_pres_t * pthinfo);

#endif

_PSF_END_EXTERN_C
#endif // TILEPRESENT_H_INCLUDED
