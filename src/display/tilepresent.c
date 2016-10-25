/******************************************************************************
 * Name:        tilepresent.c
 * Purpose:     the code for presentation porpose
 * Author:      Yunhui Fu
 * Created:     2008-11-15
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <assert.h>
#include <string.h> // memset
#include <stdio.h> // printf

#include "bthwrap4pth.h"
#include "tileog.h"

#include "tilepresent.h"

#if USE_PRESENTATION

static void
mysleep (size_t millisec)
{
    //int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict errorfds, struct timeval *restrict timeout);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = millisec;
    select (0, NULL, NULL, NULL, &tv);
}

// 以下代码演示了如何同时使用两个相互独立的模块： checkall 和 presentation

static int
tssim_cb_notifyfail_prestest (void *userdata, size_t cnt_fail)
{
    if (cnt_fail > 1000) {
        return -1;
    }
    return 0;
}

static int
tssim_cb_resultinfo_prestest (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    tsim_test_pres_t *pthinfo = (tsim_test_pres_t *)userdata;
    int ret = 0;
    bthread_mutex_unlock ((bthread_mutex_t *)(pthinfo->mutex)); /* unlock for main gui */
    if (pthinfo->flg_quit) {
        ret = -1;
    }
    bthread_mutex_lock ((bthread_mutex_t *)(pthinfo->mutex));
    return ret;
}

static int
tssim_cb_getdata_prestest (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    tsim_test_pres_t *pthinfo = (tsim_test_pres_t *)userdata;
    assert (NULL != pthinfo);
    return tssim_cb_getdata_chkall (&(pthinfo->chkallinfo), pcountlist, pyear_cur, pidx_base, pidx_test);
}

int
tssim_cb_initadheredect_prestest (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test)
{
    tsim_test_pres_t *pthinfo = (tsim_test_pres_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        return tssim_cb_initadheredect_ogshow (&(pthinfo->adhereinfo), tc_base, tc_test, temperature, pinfo_test);
    }
    return 0;
}

static int
tssim_cb_adherepos_prestest (void *userdata, char flg_canadhere, tstilecombitem_t * pposinfo)
{
    tsim_test_pres_t *pthinfo = (tsim_test_pres_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        tssim_cb_adherepos_ogshow (&(pthinfo->adhereinfo), flg_canadhere, pposinfo);

        bthread_mutex_unlock ((bthread_mutex_t *)(pthinfo->mutex)); /* unlock for main gui */
        if (pthinfo->cb_update) {
            pthinfo->cb_update ();
        }

        if (pthinfo->delay_millisec != 0) {
            mysleep (pthinfo->delay_millisec);
        }

        bthread_mutex_lock ((bthread_mutex_t *)(pthinfo->mutex));
    }
    return 0;
}

static int
tssim_cb_clearadheredect_prestest (void *userdata)
{
    tsim_test_pres_t *pthinfo = (tsim_test_pres_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        return tssim_cb_clearadheredect_ogshow (&(pthinfo->adhereinfo));
    }
    return 0;
}

void
ts_simulate_test_presentation_init (tsim_test_pres_t * pthinfo, tssim_test_cb_presentupdate_t cb_update, size_t delay_millisec)
{
    tssiminfo_t *ptsim;
    ptsim = &(pthinfo->tsim);
    memset (pthinfo, 0, sizeof (*pthinfo));

    pthinfo->mutex = malloc (sizeof (bthread_mutex_t));
    assert (NULL != pthinfo->mutex);
    bthread_mutex_init ((bthread_mutex_t *)(pthinfo->mutex), NULL);
    pthinfo->delay_millisec = delay_millisec;

    tssiminfo_init (ptsim);

    if (ts_sim_load_data_xml (ptsim, "./test/testdata4a_output.xml") < 0) {
        if (ts_sim_load_data_xml (ptsim, "../test/testdata4a_output.xml") < 0) {
            if (ts_sim_load_data_xml (ptsim, "../../test/testdata4a_output.xml") < 0) {
    if (ts_sim_load_data_xml (ptsim, "./test/testdata4a.xml") < 0) {
        if (ts_sim_load_data_xml (ptsim, "../test/testdata4a.xml") < 0) {
            if (ts_sim_load_data_xml (ptsim, "../../test/testdata4a.xml") < 0) {
                goto end_tssimtstpres;
            }
        }
    }
            }
        }
    }

    pthinfo->flg_presentation = 1;
    pthinfo->cb_update = cb_update;

    memset (&(pthinfo->adhereinfo), 0, sizeof (pthinfo->adhereinfo));
    pthinfo->adhereinfo.psim = ptsim;
    ptsim->cb_initadheredect  = tssim_cb_initadheredect_prestest;
    ptsim->cb_adherepos       = tssim_cb_adherepos_prestest;
    ptsim->cb_clearadheredect = tssim_cb_clearadheredect_prestest;

    memset (&(pthinfo->chkallinfo), 0, sizeof (pthinfo->chkallinfo));
    ptsim->cb_getdata       = NULL;
    ptsim->cb_selectone     = NULL;
    ptsim->cb_resultinfo    = tssim_cb_resultinfo_prestest;
    ptsim->cb_notifyfail    = tssim_cb_notifyfail_prestest;
    ptsim->cb_findmergepos  = NULL;
    ptsim->cb_storemergepos = NULL;
    ptsim->userdata = pthinfo;
    return;
end_tssimtstpres:
    tssiminfo_clear (&(pthinfo->tsim));
    return;
}

void
ts_simulate_test_presentation_clear (tsim_test_pres_t * pthinfo)
{
    assert (NULL != pthinfo);
    tssiminfo_clear (&(pthinfo->tsim));
    bthread_mutex_destroy ((bthread_mutex_t *)(pthinfo->mutex));
    free (pthinfo->mutex);
    memset (pthinfo, 0, sizeof (*pthinfo));
}

static void *
ts_simulate_test_presentation_working_thread (void * userdata)
{
    tsim_test_pres_t * pthinfo;
    tssiminfo_t *ptsim;

    pthinfo = (tsim_test_pres_t *)userdata;
    assert (NULL != userdata);
    ptsim = &(pthinfo->tsim);

    bthread_mutex_lock ((bthread_mutex_t *)(pthinfo->mutex));
    pthinfo->flg_running = 1;
    printf ("**** Start Simulation (presentation)****\n");
    ts_simulate_main (ptsim, "presention");

    if (0 == pthinfo->flg_quit) {
        printf ("**** Check the remains (presentation)****\n");
        pthinfo->chkallinfo.cur_max = ma_size (&(ptsim->countlist));
        // check the remains:
        ptsim->cb_getdata    = tssim_cb_getdata_prestest;
        ptsim->cb_resultinfo = tssim_cb_resultinfo_prestest;
        ts_simulate_main (ptsim, "presention");
    }
    bthread_mutex_unlock ((bthread_mutex_t *)(pthinfo->mutex));
    return NULL;
}

void
ts_simulate_test_presentation_new_thread (tsim_test_pres_t * pthinfo)
{
    bthread_t thr;
    bthread_create (&thr, NULL, ts_simulate_test_presentation_working_thread, pthinfo);
    bthread_detach (thr);
}

void
ts_simulate_test_presentation_draw (tsim_test_pres_t * pthinfo)
{
    assert (NULL != pthinfo);
    bthread_mutex_lock ((bthread_mutex_t *)(pthinfo->mutex));
    if (pthinfo->adhereinfo.psim && pthinfo->adhereinfo.tc_base && pthinfo->adhereinfo.tc_test)
    // draw
    tssim_adhere_display_ogshow (&(pthinfo->adhereinfo), "Testing: ");
    bthread_mutex_unlock ((bthread_mutex_t *)(pthinfo->mutex));
}

#endif
