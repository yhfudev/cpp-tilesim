/******************************************************************************
 * Name:        tilestruct.c
 * Purpose:     some core functions for the tiles
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <assert.h>
#include <string.h> // memset()
#include <time.h> // time()

// xml parser
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "pfdebug.h"
#include "pfrandom.h"
#include "cstrutils.h"
#include "cubeface.h"
#include "colortab.h"
#include "tilestruct.h"
#include "tilestruct2d.h"

#if ! DEBUG
#if MEMWATCH
#error Please remove MEMWATCH before compiling release version!
#endif
#endif

#if DEBUG
/* check the result of the tssim functions in ts_simulate_main() */
#define USE_DBG_CHK_RESULT 0
#endif

#define SWAP(type,p1,p2) \
    { type tmp; tmp=p1; p1=p2; p2=tmp; }

#if DEBUG
static int tssim_tc_init (tssiminfo_testcase_t *ptstc);
static int tssim_tc_clear (tssiminfo_testcase_t *ptstc);
static int tssim_tc_posnum (tssiminfo_testcase_t *ptstc);
static int tssim_tc_search (tssiminfo_testcase_t *ptstc, tssiminfo_t *ptsim_ref, tstilecombitem_t *ptstci_pos, tstilecomb_t *ptstc_result);
static int tssim_tc_xml_load (memarr_t *ptclist, char flg_is2d, char flg_norotate, xmlDocPtr doc, xmlNodePtr cur_child0);
static int tssim_tc_xml_save (memarr_t *ptclist, char flg_is2d, FILE *fp_xml);

#endif // DEBUG

// cmsg_birth: the comment for birth tag
static void tssim_save_xml_supertile (tstilecomb_t *ptc, size_t idx, size_t birth, char * cmsg_birth, size_t cnt, char flg_is2d, FILE *fp_xml);
static int tssim_read_xml_tileitem (xmlDocPtr doc, xmlNodePtr cur_child, tstilecombitem_t *ptci, char *ret_flg_is2d);

#define GLUE_CAN_ADHERE(a,b) ((a) == (b))

//////////////////////////////////////////

#define CUBE_POS_ROTATION_AXI(rottimes1, ppos_max_in, ppos_in, ppos_out, xxx,yyy,zzz) \
    switch (rottimes1) { \
    case 0: \
        memmove (ppos_out, ppos_in, sizeof (tsposition_t)); \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "rotimes=0, copy directly"); \
        break; \
    case 1: /* 90 */ \
        ppos_out->zzz = ppos_in->zzz; \
        ppos_out->xxx = ppos_in->yyy; \
        ppos_out->yyy = ppos_max_in->xxx - 1 - ppos_in->xxx; \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "rotimes=1, ppos_out->"#xxx"(%d)=ppos_in->"#yyy"(%d)\n\tppos_out->"#yyy"(%d) = ppos_max_in->"#xxx"(%d) - 1 - ppos_in->"#xxx"(%d)",\
                ppos_out->xxx, ppos_in->yyy, ppos_out->yyy, ppos_max_in->xxx, ppos_in->xxx); \
        break; \
    case 2: /* 180 */ \
        ppos_out->zzz = ppos_in->zzz; \
        ppos_out->xxx = ppos_max_in->xxx - 1 - ppos_in->xxx; \
        ppos_out->yyy = ppos_max_in->yyy - 1 - ppos_in->yyy; \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "rotimes=2, ppos_out->"#xxx"(%d)=ppos_max_in->"#xxx"(%d) - 1 - ppos_in->"#xxx"(%d)\n\tppos_out->"#yyy"(%d) = ppos_max_in->"#yyy"(%d) - 1 - ppos_in->"#yyy"(%d)",\
                ppos_out->xxx, ppos_max_in->xxx, ppos_in->xxx, ppos_out->yyy, ppos_max_in->yyy, ppos_in->yyy); \
        break; \
    case 3: /* 270 */ \
        ppos_out->zzz = ppos_in->zzz; \
        ppos_out->xxx = ppos_max_in->yyy - 1 - ppos_in->yyy; \
        ppos_out->yyy = ppos_in->xxx; \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "rotimes=3, ppos_out->"#xxx"(%d)=ppos_max_in->"#yyy"(%d) - 1 - ppos_in->"#yyy"(%d)\n\tppos_out->"#yyy"(%d) = ppos_in->"#xxx"(%d)",\
                ppos_out->xxx, ppos_max_in->yyy, ppos_in->yyy, ppos_out->yyy, ppos_in->xxx); \
        break; \
    default: \
        assert (0); \
        break; \
    }

#if USE_THREEDIMENTION

// test 旋转切换到某个面的操作顺序是 先 顺时针旋转 而后 上翻
// 求旋转切换后状态的初始状态是逆操作，可以通过 先 上翻 而后 顺时针旋转 来得到
#define GET_CUBEFACE_4ROT_TEST(num_p90, num_down, cur_state, ret_state)         GET_CUBEFACE_4ROT_P90_UP(num_p90, num_down, cur_state, ret_state)
#define GET_CUBEFACE_4ROT_TEST_REVERSE(num_p90, num_down, cur_state, ret_state) GET_CUBEFACE_4ROT_UP_P90(num_down, num_p90, cur_state, ret_state)

/*
求出立方体经过旋转后立方体上一点的坐标
ppos_max_in 是旋转前立方体各个坐标方向的最大值
ppos_in 是旋转前立方体上一点的坐标（指针）
ppos_out 是立方体上该点旋转后的坐标（指针）
注意：ppos_in 和 ppos_out 不能指向同一个地址
*/
#define CUBE_POS_ROTATION(rottype1, rottimes1, ppos_max_in, ppos_in, ppos_out) \
    switch (rottype1) { \
    case CUBE_ROT_P90: \
        CUBE_POS_ROTATION_AXI (rottimes1, ppos_max_in, ppos_in, ppos_out, x,y,z); \
        break; \
    case CUBE_ROT_DOWN: \
        CUBE_POS_ROTATION_AXI (rottimes1, ppos_max_in, ppos_in, ppos_out, y,z,x); \
        break; \
    default: \
        assert (0); \
        break; \
    }

// 先顺时针旋转 rotnum_p90_1 次, 然后向上翻滚 rotnum_up 次，最后顺时针旋转 rotnum_p90_2 次，得到的新坐标在ppos_out
static int
get_cubepos_4rot_p90_down_p90 (size_t rotnum_p90_1, size_t rotnum_down, size_t rotnum_p90_2, tsposition_t *ppos_max_in, tsposition_t *ppos_in, tsposition_t *ppos_max_out, tsposition_t *ppos_out)
{
    tsposition_t pos_max_in1;
    tsposition_t pos_max_in2;
    tsposition_t *ppos_max_in1;
    tsposition_t *ppos_max_in2;
    tsposition_t pos_tmp;
    tsposition_t *ppos_out1;
    tsposition_t *ppos_tmp;

#ifndef DEBUG
    rotnum_p90_1 %= 4;
    rotnum_down %= 4;
    rotnum_p90_2 %= 4;
#else
    assert ((0 <= rotnum_p90_1) && (rotnum_p90_1 < 4));
    assert ((0 <= rotnum_down) && (rotnum_down < 4));
    assert ((0 <= rotnum_p90_2) && (rotnum_p90_2 < 4));
#endif

    memmove (&pos_max_in1, ppos_max_in, sizeof (pos_max_in1));
    ppos_max_in1 = &pos_max_in1;
    ppos_max_in2 = &pos_max_in2;

    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step 1 -----");
    ppos_out1 = ppos_out;
    ppos_max_in2->x = ppos_max_in1->y;
    ppos_max_in2->y = ppos_max_in1->x;
    ppos_max_in2->z = ppos_max_in1->z;
    assert (ppos_max_in1->x >= 1 + ppos_in->x);
    assert (ppos_max_in1->y >= 1 + ppos_in->y);
#if USE_THREEDIMENTION
    assert (ppos_max_in1->z >= ppos_in->z);
#endif
    CUBE_POS_ROTATION (CUBE_ROT_P90, rotnum_p90_1, ppos_max_in1, ppos_in, ppos_out1);
    if (rotnum_p90_1 % 2 == 1) {
        // swap
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "SWAP maxpos");
        ppos_tmp = ppos_max_in2; ppos_max_in2 = ppos_max_in1; ppos_max_in1 = ppos_tmp;
    }

    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step 2 -----");
    ppos_in = ppos_out1;
    ppos_out1 = &pos_tmp;
    ppos_max_in2->x = ppos_max_in1->x;
    ppos_max_in2->y = ppos_max_in1->z;
    ppos_max_in2->z = ppos_max_in1->y;
    CUBE_POS_ROTATION (CUBE_ROT_DOWN, rotnum_down,  ppos_max_in1, ppos_in, ppos_out1);
    if (rotnum_down % 2 == 1) {
        // swap
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "SWAP maxpos");
        ppos_tmp = ppos_max_in2; ppos_max_in2 = ppos_max_in1; ppos_max_in1 = ppos_tmp;
    }

    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step 3 -----");
    ppos_in = ppos_out1;
    ppos_out1 = ppos_out;
    ppos_max_in2->x = ppos_max_in1->y;
    ppos_max_in2->y = ppos_max_in1->x;
    ppos_max_in2->z = ppos_max_in1->z;
    CUBE_POS_ROTATION (CUBE_ROT_P90, rotnum_p90_2, ppos_max_in1, ppos_in, ppos_out1);
    if (rotnum_p90_2 % 2 == 1) {
        // swap
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "SWAP maxpos");
        ppos_tmp = ppos_max_in2; ppos_max_in2 = ppos_max_in1; ppos_max_in1 = ppos_tmp;
    }
    if (ppos_max_out != NULL) {
        memmove (ppos_max_out, ppos_max_in1, sizeof (*ppos_max_in1));
    }
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step end -----");

    return 0;
}

#if DEBUG
/*
处理 CUBE_ROT_DOWN(下翻) 和 CUBE_ROT_P90(顺时针旋转) 两种基本的翻滚，其他类型的可以通过 (3 - (rottimes % 4)) 获得

CUBE_ROT_P90:
    z 不变。(x,y) 坐标的变化在这种类型中和二维的顺时针旋转相同。
CUBE_ROT_DOWN:
    x 不变。如果 y 被当成 x,z 被当成 y 话，则和二维的顺时针旋转相同。即 (y,z) 的二维的顺时针旋转。
*/
tstilecomb_t *
tstc_rotate3d_onetype (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, size_t rottype, size_t rottimes, tsposition_t *ptrans)
{
    tstilecombitem_t tci;
    size_t i;
    size_t tmp;
    char flg_newbuf = 0;

    assert ((CUBE_ROT_NONE <= rottype) && (rottype < CUBE_ROT_MAX));
    assert ((CUBE_ROT_DOWN == rottype) || (CUBE_ROT_P90 == rottype));

    rottimes %= 4;
    switch (rottype) {
    case CUBE_ROT_P90:
        break;
    case CUBE_ROT_DOWN:
        break;
    default:
        assert (0);
        return NULL;
        break;
    }

    if (NULL == buf_new) {
        buf_new = tstc_create ();
        flg_newbuf = 1;
    }
    if (NULL == buf_new) {
        return NULL;
    }
    tstc_reset (buf_new);

    /*
    rotation model:
      (the clockwise and counter-clockwise of the rotation related to the coordinate system)
        origin: maxx, maxy; x0, y0
        90: [maxy,maxx]
            X90 = y0
            Y90 = maxx - 1 - x0
        180: [maxx,maxy]
            X180 = maxx - 1 - x0
            Y180 = maxy - 1 - y0
        270: [maxy,maxx]
            X270 = maxy - 1 - y0
            Y270 = x0
    */

// xxx,yyy,zzz: the type of the axies maped to the rotation model
#define CUBE_ROTATION_TIMES(xxx,yyy,zzz) \
    switch (rottimes) { \
    case 0: \
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) { \
            slist_data (&(buf_orig->tbuf), i, &tci); \
            TSPOS_ADDSELF (tci.pos, *ptrans); \
            slist_store (&(buf_new->tbuf), &tci); \
            UPDATE_MAX_AXI (buf_new->maxpos, tci.pos); \
        } \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "0) buf_new->maxpos." #xxx "=%d; buf_orig->maxpos." #xxx "=%d; ptrans->" #xxx "=%d", buf_new->maxpos.xxx, buf_orig->maxpos.xxx, ptrans->xxx); \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "0) buf_new->maxpos." #yyy "=%d; buf_orig->maxpos." #yyy "=%d; ptrans->" #yyy "=%d", buf_new->maxpos.yyy, buf_orig->maxpos.yyy, ptrans->yyy); \
        assert (buf_new->maxpos.xxx == buf_orig->maxpos.xxx + ptrans->xxx); \
        assert (buf_new->maxpos.yyy == buf_orig->maxpos.yyy + ptrans->yyy); \
        break; \
    case 1: /* 90 */ \
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) { \
            slist_data (&(buf_orig->tbuf), i, &tci); \
            tmp = (buf_orig->maxpos.xxx - 1 - tci.pos.xxx); \
            tci.pos.xxx = tci.pos.yyy; \
            tci.pos.yyy = tmp; \
            TSPOS_ADDSELF (tci.pos, *ptrans); \
            /* also rotate the tile dir */ \
            assert ((0 <= tci.rotnum) && (tci.rotnum < TILE_STATUS_MAX)); \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            slist_store (&(buf_new->tbuf), &tci); \
            UPDATE_MAX_AXI (buf_new->maxpos, tci.pos); \
        } \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "1) buf_new->maxpos." #xxx "(%d) == buf_orig->maxpos." #yyy "(%d) + ptrans->" #xxx "(%d)", buf_new->maxpos.xxx, buf_orig->maxpos.yyy, ptrans->xxx); \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "1) buf_new->maxpos." #yyy "(%d) == buf_orig->maxpos." #xxx "(%d) + ptrans->" #yyy "(%d)", buf_new->maxpos.yyy, buf_orig->maxpos.xxx, ptrans->yyy); \
        assert (buf_new->maxpos.xxx == buf_orig->maxpos.yyy + ptrans->xxx); \
        assert (buf_new->maxpos.yyy == buf_orig->maxpos.xxx + ptrans->yyy); \
        break; \
    case 2: /* 180 */ \
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) { \
            slist_data (&(buf_orig->tbuf), i, &tci); \
            tci.pos.xxx = buf_orig->maxpos.xxx - 1 - tci.pos.xxx; \
            tci.pos.yyy = buf_orig->maxpos.yyy - 1 - tci.pos.yyy; \
            TSPOS_ADDSELF (tci.pos, *ptrans); \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            slist_store (&(buf_new->tbuf), &tci); \
            UPDATE_MAX_AXI (buf_new->maxpos, tci.pos); \
        } \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "2) buf_new->maxpos." #xxx "(%d) == buf_orig->maxpos." #xxx "(%d) + ptrans->" #xxx "(%d)", buf_new->maxpos.xxx, buf_orig->maxpos.xxx, ptrans->xxx); \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "2) buf_new->maxpos." #yyy "(%d) == buf_orig->maxpos." #yyy "(%d) + ptrans->" #yyy "(%d)", buf_new->maxpos.yyy, buf_orig->maxpos.yyy, ptrans->yyy); \
        assert (buf_new->maxpos.xxx == buf_orig->maxpos.xxx + ptrans->xxx); \
        assert (buf_new->maxpos.yyy == buf_orig->maxpos.yyy + ptrans->yyy); \
        break; \
    case 3: /* 270 */ \
        for (i = 0; i < slist_size (&(buf_orig->tbuf)); i ++) { \
            slist_data (&(buf_orig->tbuf), i, &tci); \
            tmp = tci.pos.xxx; \
            tci.pos.xxx = (buf_orig->maxpos.yyy - 1 - tci.pos.yyy); \
            tci.pos.yyy = tmp; \
            TSPOS_ADDSELF (tci.pos, *ptrans); \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            tci.rotnum = g_cubeface_rottab[tci.rotnum][rottype]; \
            slist_store (&(buf_new->tbuf), &tci); \
            UPDATE_MAX_AXI (buf_new->maxpos, tci.pos); \
        } \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "3) buf_new->maxpos." #xxx "(%d) == buf_orig->maxpos." #yyy "(%d) + ptrans->" #xxx "(%d)", buf_new->maxpos.xxx, buf_orig->maxpos.yyy, ptrans->xxx); \
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "3) buf_new->maxpos." #yyy "(%d) == buf_orig->maxpos." #xxx "(%d) + ptrans->" #yyy "(%d)", buf_new->maxpos.yyy, buf_orig->maxpos.xxx, ptrans->yyy); \
        assert (buf_new->maxpos.xxx == buf_orig->maxpos.yyy + ptrans->xxx); \
        assert (buf_new->maxpos.yyy == buf_orig->maxpos.xxx + ptrans->yyy); \
        break; \
    default: \
        assert (0); \
        break; \
    }

    switch (rottype) {
    case CUBE_ROT_P90:
        CUBE_ROTATION_TIMES (x,y,z);
        break;
    case CUBE_ROT_DOWN:
        CUBE_ROTATION_TIMES (y,z,x);
        break;
    default:
        assert (0);
        if (flg_newbuf) {
            tstc_destroy (buf_new);
        }
        buf_new = NULL;
        return NULL;
        break;
    }
    return buf_new;
}

tstilecomb_t *
tstc_rotate3d_1 (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, size_t rotnum, tsposition_t *ptrans)
{
    tstilecomb_t *buf_tmp1;
    tstilecomb_t *buf_tmp2;
    tsposition_t pos_tmp;
    memset (&pos_tmp, 0, sizeof(pos_tmp));

    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_INFO, "step 1 p90 ...");
    buf_tmp1 = tstc_rotate3d_onetype (buf_orig, NULL, CUBE_ROT_P90, g_cubeface_map_cur2all[0][rotnum][0], &pos_tmp);
    if (NULL == buf_tmp1) {
        return NULL;
    }
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_INFO, "step 2 down ...");
    buf_tmp2 = tstc_rotate3d_onetype (buf_tmp1, NULL, CUBE_ROT_DOWN, g_cubeface_map_cur2all[0][rotnum][1], &pos_tmp);
    tstc_destroy (buf_tmp1);
    if (NULL == buf_tmp2) {
        return NULL;
    }
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_INFO, "step 3 p90 ...");
    buf_orig = tstc_rotate3d_onetype (buf_tmp2, buf_new, CUBE_ROT_P90, g_cubeface_map_cur2all[0][rotnum][2], ptrans);
    tstc_destroy (buf_tmp2);
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_INFO, "done!");
    if (NULL == buf_orig) {
        return NULL;
    }
    return buf_orig;
}
#endif // 0

#else /* USE_THREEDIMENTION */
#define CUBE_POS_ROTATION(rottype1, rottimes1, ppos_max_in, ppos_in, ppos_out) \
    assert (CUBE_ROT_P90 == (rottype1)); \
    CUBE_POS_ROTATION_AXI (rottimes1, ppos_max_in, ppos_in, ppos_out, x,y, x);

static int
get_cubepos_4rot_p90_2d (size_t rotnum_p90_2, tsposition_t *ppos_max_in, tsposition_t *ppos_in, tsposition_t *ppos_max_out, tsposition_t *ppos_out)
{
    tsposition_t pos_max_in1;
    tsposition_t pos_max_in2;
    tsposition_t *ppos_max_in1;
    tsposition_t *ppos_max_in2;
    tsposition_t *ppos_out1;
    tsposition_t *ppos_tmp;

#ifndef DEBUG
    rotnum_p90_2 %= 4;
#else
    assert ((0 <= rotnum_p90_2) && (rotnum_p90_2 < 4));
#endif

    memmove (&pos_max_in1, ppos_max_in, sizeof (pos_max_in1));
    ppos_max_in1 = &pos_max_in1;
    ppos_max_in2 = &pos_max_in2;

    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step 1 -----");
    ppos_out1 = ppos_out;
    ppos_max_in2->x = ppos_max_in1->y;
    ppos_max_in2->y = ppos_max_in1->x;
    CUBE_POS_ROTATION (CUBE_ROT_P90, rotnum_p90_2, ppos_max_in1, ppos_in, ppos_out1);
    if (rotnum_p90_2 % 2 == 1) {
        // swap
        DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "SWAP maxpos");
        ppos_tmp = ppos_max_in2; ppos_max_in2 = ppos_max_in1; ppos_max_in1 = ppos_tmp;
    }
    if (ppos_max_out != NULL) {
        memmove (ppos_max_out, ppos_max_in1, sizeof (*ppos_max_in1));
    }
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "---- step end -----");

    return 0;
}
#define get_cubepos_4rot_p90_down_p90(rotnum_p90_1, rotnum_down, rotnum_p90_2, ppos_max_in, ppos_in, ppos_max_out, ppos_out) get_cubepos_4rot_p90_2d(rotnum_p90_2, ppos_max_in, ppos_in, ppos_max_out, ppos_out)

#endif /* USE_THREEDIMENTION */

#define GET_CUBEPOS_4ROT_TEST(rotnum_p90_1, rotnum_down, rotnum_p90_2, ppos_max_in, ppos_in, ppos_max_out, ppos_out)         get_cubepos_4rot_p90_down_p90 (rotnum_p90_1, rotnum_down, rotnum_p90_2, ppos_max_in, ppos_in, ppos_max_out, ppos_out)
#define GET_CUBEPOS_4ROT_TEST_REVERSE(rotnum_p90_1, rotnum_down, rotnum_p90_2, ppos_max_in, ppos_in, ppos_max_out, ppos_out) get_cubepos_4rot_p90_down_p90 ((4 - (rotnum_p90_2)) % 4, (4 - (rotnum_down)) % 4, (4 - (rotnum_p90_1)) % 4, ppos_max_in, ppos_in, ppos_max_out, ppos_out)

/*
   rotate the buf_orig, and translate (x,y) after rotating
   store the result to buf_new
   num: the times for rotating 90 degree
 */
tstilecomb_t *
tstc_rotate3d (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, size_t rotnum, tsposition_t *ptrans)
{
    size_t i;
    tstilecombitem_t tci_old;
    tstilecombitem_t tci_new;
    char flg_newbuf = 0;

    if (NULL == buf_new) {
        buf_new = tstc_create ();
        flg_newbuf = 1;
    }
    if (NULL == buf_new) {
        return NULL;
    }
    tstc_reset (buf_new);

    for (i = 0; i < slist_size(&(buf_orig->tbuf)); i ++) {
        slist_data (&(buf_orig->tbuf), i, &tci_old);
#if DEBUG
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "i=%d/%d", i+1, slist_size(&(buf_orig->tbuf)));
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "buf_orig->maxpos", buf_orig->maxpos);
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tci_old.pos", tci_old.pos);
#endif
#if 0
#define MAX_COORD 50000
        assert ((0 <= tci_old.pos.x) && (tci_old.pos.x < MAX_COORD));
        assert ((0 <= tci_old.pos.y) && (tci_old.pos.y < MAX_COORD));
        assert ((0 <= tci_old.pos.z) && (tci_old.pos.z < MAX_COORD));
#endif
        memset (&tci_new, 0, sizeof (tci_new));
        get_cubepos_4rot_p90_down_p90 (g_cubeface_map_cur2all[0][rotnum][0], g_cubeface_map_cur2all[0][rotnum][1],
            g_cubeface_map_cur2all[0][rotnum][2], &(buf_orig->maxpos), &(tci_old.pos), NULL, &(tci_new.pos));
        GET_CUBEFACE_4ROT_P90_DOWN_P90 (g_cubeface_map_cur2all[0][rotnum][0], g_cubeface_map_cur2all[0][rotnum][1],
            g_cubeface_map_cur2all[0][rotnum][2], tci_old.rotnum, tci_new.rotnum);
        tci_new.idtile = tci_old.idtile;
#if 0
        assert ((0 <= tci_new.pos.x) && (tci_new.pos.x < MAX_COORD));
        assert ((0 <= tci_new.pos.y) && (tci_new.pos.y < MAX_COORD));
        assert ((0 <= tci_new.pos.z) && (tci_new.pos.z < MAX_COORD));
#endif
        // transfer to ptrans
        if (ptrans) {
            TSPOS_ADDSELF (tci_new.pos, *ptrans);
        }
#if DEBUG
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tci_old", tci_old.pos);
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\tidtile=%d", tci_old.idtile);
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\trotnum=%d", tci_old.rotnum);
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tci_new", tci_new.pos);
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\tidtile=%d", tci_new.idtile);
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\trotnum=%d", tci_new.rotnum);

        memmove (&tci_old, &tci_new, sizeof (tci_new)); // debug: backup
#endif

        slist_store (&(buf_new->tbuf), &tci_new);

#if DEBUG
        if (0 != tstci_cmp (&tci_old, &tci_new)) { // debug: assert
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "before", tci_old.pos);
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\tidtile=%d", tci_old.idtile);
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\trotnum=%d", tci_old.rotnum);
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "after", tci_new.pos);
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\tidtile=%d", tci_new.idtile);
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "\trotnum=%d", tci_new.rotnum);
            assert (0);
        }
#endif
        UPDATE_MAX_AXI (buf_new->maxpos, tci_new.pos);
#if DEBUG
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "maxpos is updated to:", buf_new->maxpos);
#endif
    }
    return buf_new;
}

//////////////////////////////////////////

// Get the glue value of the designate direction of one tile.
// The tile information contained in tstilecombitem_t include the item idx which
// is the index of a global tile table.
// The parameter rotnum of the function denote that the tile is a part of one
// supertile and the supertile is rotated and then the tile have to rotate
// acording to the rotation.
// ptilevec: tssiminfo_t.ptilevec
// ptsi: the tile information
// dirglue: the side of the tile, ORI_NORTH, ORI_EAST, ...
#if __GNUC__
//inline int tile_get_glue (tstile_t *ptilevec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum) __attribute__((always_inline));
//inline
#endif

#if USE_THREEDIMENTION
int
tile_get_glue (tstile_t *ptilevec, size_t maxtvec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum)
{
    dirglue %= 6;
    if (0 != rotnum) {
        // 在当前 tile 的旋转状态下，看看经过与主 supertile 旋转相同 rotnum 后是什么状态
        size_t rotnum2;
        GET_CUBEFACE_4ROT_P90_DOWN_P90 (g_cubeface_map_cur2all[0][rotnum][0], g_cubeface_map_cur2all[0][rotnum][1],
            g_cubeface_map_cur2all[0][rotnum][2], ptsi->rotnum, rotnum2);
        rotnum = rotnum2;
    } else {
        rotnum = ptsi->rotnum;
    }

    assert (ptsi->idtile < maxtvec);

    assert ((0 <= dirglue) && (dirglue < 6));
    assert ((0 <= rotnum) && (rotnum < TILE_STATUS_MAX));
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "g_cubeface_map2ori[g_cubeface_rotpos[rotnum=%d][g_cubeface_ori2map[dirglue=%d]]]: %d", rotnum, dirglue, g_cubeface_map2ori[g_cubeface_rotpos[rotnum][g_cubeface_ori2map[dirglue]]]);
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_LOG, "tilevec(0x%08X) [%d]={N %d, E %d, S %d, W %d"
#if USE_THREEDIMENTION
        ", F %d, B %d"
#endif
        "}"
        , ptilevec
        , ptsi->idtile
        , ptilevec[ptsi->idtile].glues[ORI_NORTH]
        , ptilevec[ptsi->idtile].glues[ORI_EAST]
        , ptilevec[ptsi->idtile].glues[ORI_SOUTH]
        , ptilevec[ptsi->idtile].glues[ORI_WEST]
#if USE_THREEDIMENTION
        , ptilevec[ptsi->idtile].glues[ORI_FRONT]
        , ptilevec[ptsi->idtile].glues[ORI_BACK]
#endif
        );
    switch (g_cubeface_map2ori[g_cubeface_rotpos[rotnum][g_cubeface_ori2map[dirglue]]]) {
    case ORI_NORTH:
    case ORI_EAST:
    case ORI_SOUTH:
    case ORI_WEST:
#if USE_THREEDIMENTION
    case ORI_FRONT:
    case ORI_BACK:
#endif
        return ptilevec[ptsi->idtile].glues[g_cubeface_map2ori[g_cubeface_rotpos[rotnum][g_cubeface_ori2map[dirglue]]]];
    }
    DBGMSG (PFDBG_CATLOG_3DBASE, PFDBG_LEVEL_WARNING, "out of range: %d", g_cubeface_map2ori[g_cubeface_rotpos[rotnum][g_cubeface_ori2map[dirglue]]]);
    return -1;
}

#else
int
tile_get_glue (tstile_t *ptilevec, size_t maxtvec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum)
{
    rotnum += ptsi->rotnum;
    rotnum %= 4;
    dirglue %= 4;
    //for (; dirglue < rotnum; dirglue += 4);
    if (dirglue < rotnum) {
        assert (dirglue + 4 >= rotnum);
        dirglue += 4;
    }
    dirglue -= rotnum;
    assert (dirglue >= 0);
    switch (dirglue) {
    case ORI_NORTH:
        return ptilevec[ptsi->idtile].glueN;
    case ORI_EAST:
        return ptilevec[ptsi->idtile].glueE;
    case ORI_SOUTH:
        return ptilevec[ptsi->idtile].glueS;
    case ORI_WEST:
        return ptilevec[ptsi->idtile].glueW;
    }
    return -1;
}
#endif // USE_THREEDIMENTION

//////////////////////////////////////////
int
slist_cb_comp_tspos (void * userdata, void * data1, void * data2)
{
    return tspos_cmp ((tsposition_t *)data1, (tsposition_t *)data2);
}

int
slist_cb_swap_tspos (void *userdata, void * data1, void * data2)
{
    tsposition_t * ptci1 = (tsposition_t *)data1;
    tsposition_t * ptci2 = (tsposition_t *)data2;
    tsposition_t tmp;
    memmove (&tmp, ptci1, sizeof (tmp));
    memmove (ptci1, ptci2, sizeof (tmp));
    memmove (ptci2, &tmp, sizeof (tmp));
    return 0;
}

static int
slist_cb_comp_tcitem (void * userdata, void * data1, void * data2)
{
    tstilecombitem_t * ptci1 = (tstilecombitem_t *)data1;
    tstilecombitem_t * ptci2 = (tstilecombitem_t *)data2;
    return tspos_cmp (&(ptci1->pos), &(ptci2->pos));
    /*
    if (ptci1->idx > ptci2->idx) {
        return 1;
    }
    if (ptci1->idx < ptci2->idx) {
        return -1;
    }
    if (ptci1->dir > ptci2->dir) {
        return 1;
    }
    if (ptci1->dir < ptci2->dir) {
        return -1;
    }
    return 0;*/
}

int
slist_cb_swap_tcitem (void *userdata, void * data1, void * data2)
{
    tstilecombitem_t * ptci1 = (tstilecombitem_t *)data1;
    tstilecombitem_t * ptci2 = (tstilecombitem_t *)data2;
    tstilecombitem_t tmp;
    memmove (&tmp, ptci1, sizeof (tmp));
    memmove (ptci1, ptci2, sizeof (tmp));
    memmove (ptci2, &tmp, sizeof (tmp));
    return 0;
}

int
tstc_init (tstilecomb_t *tc)
{
    assert (NULL != tc);
    memset (tc, 0, sizeof (*tc));
    return slist_init (&(tc->tbuf), sizeof (tstilecombitem_t), NULL, slist_cb_comp_tcitem, slist_cb_swap_tcitem);
}

int
tstc_clear (tstilecomb_t *tc)
{
    assert (NULL != tc);
    return slist_clear (&(tc->tbuf), NULL);
}

int
tstc_reset (tstilecomb_t *tc)
{
    assert (NULL != tc);
    tc->id = 0;
    memset (&(tc->maxpos), 0, sizeof (tc->maxpos));
    return slist_rmalldata (&(tc->tbuf), NULL);
}

tstilecomb_t *
tstc_create (void)
{
    tstilecomb_t *ptc;
    ptc = (tstilecomb_t *)malloc (sizeof (*ptc));
    if (NULL == ptc) {
        return NULL;
    }
    tstc_init (ptc);
    return ptc;
}

int
tstc_destroy (tstilecomb_t *tc)
{
    tstc_clear (tc);
    free (tc);
    return 0;
}

// copy the content of tstilecomb_t tc_from to tc_to
int
tstc_copy (tstilecomb_t *tc_to, tstilecomb_t *tc_from)
{
    memmove (&(tc_to->maxpos), &(tc_from->maxpos), sizeof (tc_to->maxpos));
    if (slist_copy (&(tc_to->tbuf), &(tc_from->tbuf)) < 0) {
        return -1;
    }
    return 0;
}

// 将tc_from 中的数据移动到 tc_to， 避免重新申请内存
// move the content of tstilecomb_t tc_from to tc_to
// The content of the from will be erased.
// The re-allocation of dynamic memory will be avoided in this function.
int
tstc_transfer (tstilecomb_t *tc_to, tstilecomb_t *tc_from)
{
    memmove (&(tc_to->maxpos), &(tc_from->maxpos), sizeof (tc_to->maxpos));
    if (slist_transfer (&(tc_to->tbuf), &(tc_from->tbuf), NULL) < 0) {
        return -1;
    }
    return 0;
}

// add one tstilecombitem_t item to the tc
int
tstc_additem (tstilecomb_t *tc, tstilecombitem_t *ptci)
{
    int ret;
    ret = slist_store (&(tc->tbuf), ptci);
    if (ret < 0) {
        return -1;
    }
    if (ptci->pos.x >= tc->maxpos.x) {
        tc->maxpos.x = ptci->pos.x + 1;
    }
    if (ptci->pos.y >= tc->maxpos.y) {
        tc->maxpos.y = ptci->pos.y + 1;
    }
#if USE_THREEDIMENTION
    if (ptci->pos.z >= tc->maxpos.z) {
        tc->maxpos.z = ptci->pos.z + 1;
    }
#endif
    return 0;
}

#if DEBUG
// check the supertile to see if there's any errors.
static int
tstc_chkassert (tstilecomb_t *ptc)
{
    int i;

    tsposition_t posmin;
    tsposition_t posmax;
    tstilecombitem_t *pcuritem;

    assert (NULL != ptc);
    assert (slist_size (&(ptc->tbuf)) > 0);

    // find the left botton corner and top right corner.
    i = 0;
    pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
    memmove (&posmin, &(pcuritem->pos), sizeof (posmin));
    memmove (&posmax, &(pcuritem->pos), sizeof (posmax));

    slist_data_unlock (&(ptc->tbuf), i);

    for (i = 1; i < slist_size (&(ptc->tbuf)); i ++) {
        pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
        assert (NULL != pcuritem);

        if (posmin.x > pcuritem->pos.x) posmin.x = pcuritem->pos.x;
        if (posmax.x < pcuritem->pos.x) posmax.x = pcuritem->pos.x;
        if (posmin.y > pcuritem->pos.y) posmin.y = pcuritem->pos.y;
        if (posmax.y < pcuritem->pos.y) posmax.y = pcuritem->pos.y;
#if USE_THREEDIMENTION
        if (posmin.z > pcuritem->pos.z) posmin.x = pcuritem->pos.z;
        if (posmax.z < pcuritem->pos.z) posmax.z = pcuritem->pos.z;
#endif
        slist_data_unlock (&(ptc->tbuf), i);
    }
    assert (posmin.x == 0);
    assert (posmin.y == 0);
    assert (ptc->maxpos.x == posmax.x + 1);
    assert (ptc->maxpos.y == posmax.y + 1);
#if USE_THREEDIMENTION
    assert (posmin.z == 0);
    assert (ptc->maxpos.z == posmax.z + 1);
#endif
    return 0;
}
#else
#define tstc_chkassert(p) (0)
#endif

// nomalize the ptc: let the left bottom of the supertile segment move to the (0,0) position
int
tstc_nomalize (tstilecomb_t *ptc)
{
    int ret = 0;
    int i;

    tsposition_t posmin;
    tsposition_t posmax;
    tstilecombitem_t *pcuritem;

    assert (NULL != ptc);
    assert (slist_size (&(ptc->tbuf)) > 0);

    // find the left botton corner and top right corner.
    i = 0;
    pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
    memmove (&posmin, &(pcuritem->pos), sizeof (posmin));
    memmove (&posmax, &(pcuritem->pos), sizeof (posmax));

    slist_data_unlock (&(ptc->tbuf), i);

    for (i = 1; i < slist_size (&(ptc->tbuf)); i ++) {
        pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
        assert (NULL != pcuritem);

        if (posmin.x > pcuritem->pos.x) posmin.x = pcuritem->pos.x;
        if (posmin.y > pcuritem->pos.y) posmin.y = pcuritem->pos.y;
        if (posmax.x < pcuritem->pos.x) posmax.x = pcuritem->pos.x;
        if (posmax.y < pcuritem->pos.y) posmax.y = pcuritem->pos.y;
#if USE_THREEDIMENTION
        if (posmin.z > pcuritem->pos.z) posmin.z = pcuritem->pos.z;
        if (posmax.z < pcuritem->pos.z) posmax.z = pcuritem->pos.z;
#endif
        slist_data_unlock (&(ptc->tbuf), i);
    }
    //assert (ptc->posmax.x == posmax.x + 1);
    //assert (ptc->posmax.y == posmax.y + 1);
#if USE_THREEDIMENTION
    //assert (ptc->posmax.z == posmax.z + 1);
#endif
    if ((posmin.x != 0)
      || (posmin.y != 0)
#if USE_THREEDIMENTION
      || (posmin.z != 0)
#endif
      ) {
        for (i = 0; i < slist_size (&(ptc->tbuf)); i ++) {
            pcuritem = (tstilecombitem_t *) slist_data_lock (&(ptc->tbuf), i);
            assert (NULL != pcuritem);

            pcuritem->pos.x -= posmin.x;
            pcuritem->pos.y -= posmin.y;
#if USE_THREEDIMENTION
            pcuritem->pos.z -= posmin.z;
#endif
            slist_data_unlock (&(ptc->tbuf), i);
        }
        ret = 1;
    }
    ptc->maxpos.x = posmax.x - posmin.x + 1;
    ptc->maxpos.y = posmax.y - posmin.y + 1;
#if USE_THREEDIMENTION
    ptc->maxpos.z = posmax.z - posmin.z + 1;
#endif

    tstc_chkassert (ptc);
    return ret;
}

int
tstci_cmp (tstilecombitem_t *ptci1, tstilecombitem_t *ptci2)
{
    int ret;
    ret = tspos_cmp (&(ptci1->pos), &(ptci2->pos));
    if (ret != 0) {
        return ret;
    }
    if (ptci1->idtile > ptci2->idtile) {
        return 1;
    }
    if (ptci1->idtile < ptci2->idtile) {
        return -1;
    }
    if (ptci1->rotnum > ptci2->rotnum) {
        return 1;
    }
    if (ptci1->rotnum < ptci2->rotnum) {
        return -1;
    }
    return 0;
}

// if the two tstilecomb_t is equal, return 1.
// otherwise return 0
int
tstc_is_equal (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable)
{
    // 先检测两个supertile的tile个数是否相等，如不是则返回不等
    // 如果不可旋转，则检查当前大小范围是否相等，如不是则返回不等
    // 循环6个面的4个方向做如下步骤：
    //   对 tc2 中每个点作旋转后 查找 tc1 中是否有相同的点
    // 是否循环完都没有找到，如没找到则不相同，返回
    // 到此相同,返回
    int i;
    int j;
    int k;
    size_t idx;
    tsposition_t pos_tmp;
    tsposition_t pos_max;
    tstilecombitem_t *ptci1 = NULL;
    tstilecombitem_t *ptci2 = NULL;
    size_t rotnum_new;

    assert (NULL != tc1);
    assert (NULL != tc2);
    if (slist_size (&(tc1->tbuf)) != slist_size (&(tc2->tbuf))) {
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "size1!=size2");
        return 0;
    }
    if (! flg_nonrotatable) {
        if (! TSPOS_ISEQUAL (tc1->maxpos, tc2->maxpos)) {
            // 如果系统不支持旋转，而两个 supertile 必须要其中一个旋转才能大小相等，则必不等
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1->maxpos!=tc2->maxpos");
            return 0;
        }
    }
    if (slist_size (&(tc1->tbuf)) < 1) {
        assert (slist_size (&(tc1->tbuf)) == slist_size (&(tc2->tbuf)));
        memset (&pos_max, 0, sizeof (pos_max));
        if (! TSPOS_ISEQUAL (pos_max, tc1->maxpos)) {
            return 0;
        }
        if (! TSPOS_ISEQUAL (pos_max, tc2->maxpos)) {
            return 0;
        }
        return 1;
    }

    // check if the size of the two tstilecomb_t are the same.
    // 对每个面比较相邻两个旋转的大小，如果边界不一样，则必不等
    for (i = 0; i < CUBE_FACE_NUM; i ++) {
        // 六个面
        for (j = 0; j < 4; j ++) {
            // 每个面的4个状态
            //DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "get_cubepos_4rot_p90_down_p90 () ...");
            //DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_DEBUG, "tc1->maxpos", tc1->maxpos);
            // 旋转tc1到目的状态，得到旋转后的最大边界
            memset (&pos_tmp, 0, sizeof (pos_tmp));
            get_cubepos_4rot_p90_down_p90 (g_cubeface_cur2all[i][0], g_cubeface_cur2all[i][1], j,
                &(tc1->maxpos), &pos_tmp, &pos_max, &pos_tmp);
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1->maxpos", tc1->maxpos);
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc2->maxpos", tc2->maxpos);
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "    pos_max", pos_max);
            // 看是否该状态的边界范围和 tc2 的一样，不一样则不必比较，直接看下个状态是否相等
            if (TSPOS_ISEQUAL (pos_max, tc2->maxpos)) {
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "same pos_max: ", pos_max);
                // 边界范围相同，则逐个比较各个tile
                assert (slist_size (&(tc1->tbuf)) == slist_size (&(tc2->tbuf)));
                for (k = 0; k < slist_size (&(tc1->tbuf)); k ++) {
                    ptci1 = (tstilecombitem_t *) slist_data_lock (&(tc1->tbuf), k);
                    // rotate it
                    //DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "get_cubepos_4rot_p90_down_p90 () ...");
                    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_DEBUG, "ptci1->pos", ptci1->pos);
                    get_cubepos_4rot_p90_down_p90 (g_cubeface_cur2all[i][0], g_cubeface_cur2all[i][1], j,
                        &(tc1->maxpos), &(ptci1->pos), NULL, &pos_tmp);
                    if (slist_find (&(tc2->tbuf), &pos_tmp, NULL, &idx) < 0) {
                        slist_data_unlock (&(tc2->tbuf), k);
                        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_INFO, "not found one item in tc2");
                        break;
                    }
                    // rotate the face
                    GET_CUBEFACE_4ROT_P90_DOWN_P90 (g_cubeface_cur2all[i][0], g_cubeface_cur2all[i][1], j,
                        ptci1->rotnum, rotnum_new);
                    ptci2 = (tstilecombitem_t *) slist_data_lock (&(tc2->tbuf), idx);
                    if ((ptci1->idtile != ptci2->idtile) || (rotnum_new != ptci2->rotnum)) {
                        slist_data_unlock (&(tc2->tbuf), idx);
                        slist_data_unlock (&(tc1->tbuf), k);
                        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "(idtile,rotnum): tc1(%d,%d) != tc2(%d,%d)"
                            , ptci1->idtile, ptci1->rotnum, ptci2->idtile, ptci2->rotnum);
                        break;
                    }
                    slist_data_unlock (&(tc2->tbuf), idx);
                    slist_data_unlock (&(tc1->tbuf), k);
                }
                if (k >= slist_size (&(tc1->tbuf))) {
                    // all of the point are same
                    DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_INFO, "found! tc1@(P90=%d,UP=%d,P90=%d)",
                        g_cubeface_cur2all[i][0], g_cubeface_cur2all[i][1], j);
                    break;
                }
            }
        }
        if (j < 4) {
            // found
            break;
        }
        j = 0;
    }
    if (i < CUBE_FACE_NUM) {
        // found
        return 1;
    }
    return 0;
}

int
tstc_compare (tstilecomb_t *tc1, tstilecomb_t *tc2)
{
    size_t i;
    int ret = 0;
    tstilecombitem_t *ptsci1;
    tstilecombitem_t *ptsci2;

    // 然后再根据 (x,y), idtile, rotnun, 来排序
    for (i = 0; i < slist_size (&(tc1->tbuf)); i ++) {
        if (i >= slist_size (&(tc2->tbuf))) {
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1>tc2");
            return 1;
        }
        ret = 0;
        ptsci1 = (tstilecombitem_t *)slist_data_lock (&(tc1->tbuf), i);
        ptsci2 = (tstilecombitem_t *)slist_data_lock (&(tc2->tbuf), i);
        ret = tstci_cmp (ptsci1, ptsci2);
        slist_data_unlock (&(tc1->tbuf), i);
        slist_data_unlock (&(tc2->tbuf), i);
        if (ret != 0) {
            DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1-tc2=%d", ret);
            return ret;
        }
    }
    if (i < slist_size (&(tc2->tbuf))) {
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1<tc2");
        return -1;
    }

    return 0;
}

int
tstc_compare_full (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable)
{
    int ret = 0;

    // 首先判断两个 supertile 在可旋转状态下是否相等
    if (tstc_is_equal (tc1, tc2, flg_nonrotatable)) {
        DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_LOG, "tc1==tc2");
        return 0;
    }
    // 然后再根据 (x,y), idtile, rotnun, 来排序
    ret = tstc_compare (tc1, tc2);
    if (ret != 0) {
        return ret;
    }

    // 最后如果还没有分出大小，则以上算法有问题
    DBGMSG (PFDBG_CATLOG_TSTC, PFDBG_LEVEL_ERROR, "tc1=?=tc2");
    assert (0);
    return 0;
}

// 在可旋转模式下，将其调整到最小的状态，用于两个tstilecomb_t之间的比较
int
tstc_adjust_lower (tstilecomb_t *ptstc, char flg_is2d)
{
#if ! DEBUG
    size_t i;
    size_t max_status = TILE_STATUS_MAX;
    tstilecomb_t *ptstc1;
    tstilecomb_t *ptstc2;
    tstilecomb_t tstc1;
    tstilecomb_t tstc2;
#if USE_THREEDIMENTION
    if (flg_is2d) {
        max_status = 4;
    }
#endif

    tstc_init (&tstc1);
    tstc_init (&tstc2);
    ptstc1 = ptstc;
    ptstc2 = &tstc2;
    for (i = 1; i < max_status; i ++) {
        // convert ptstc[0] to ptstc[i]
        tstc_rotate3d (ptstc, ptstc2, i, NULL);
        if (tstc_compare (ptstc1, ptstc2) > 0) {
            if (ptstc1 == ptstc) {
                assert (ptstc2 == &tstc2);
                ptstc1 = &tstc1;
            }
            tstilecomb_t *ptmp;
            ptmp = ptstc1;
            ptstc1 = ptstc2;
            ptstc2 = ptmp;
        }
    }
    if (ptstc1 != ptstc) {
        tstc_reset (ptstc);
        tstc_transfer (ptstc, ptstc1);
    }
    tstc_clear (&tstc1);
    tstc_clear (&tstc2);
#endif // DEBUG
    return 0;
}

int
tile_list_search (memarr_t *ptilelist, tstilecomb_t *pneedle, char flg_nonrotatable, size_t *ppos)
{
    char flg_found = 0;
    size_t i;
    tstilecomb_t *ptc;

    assert (NULL != ptilelist);
    assert (NULL != ppos);
    *ppos = 0;
    if (NULL == pneedle) {
        return -1;
    }
    assert (NULL != pneedle);

    for (i = 0; (0 == flg_found) && (i < ma_size (ptilelist)); i ++) {
        ptc = (tstilecomb_t *) ma_data_lock (ptilelist, i);
        if (tstc_is_equal (pneedle, ptc, flg_nonrotatable)) {
            *ppos = i;
            flg_found = 1;
        }
        ma_data_unlock (ptilelist, i);
    }
    if (0 == flg_found) {
        return -1;
    }
    return 0;
}

static int slist_cb_comp_heter_addst (void *userdata, void * data_pin, void * data2);

// 从ptsim->tilelstidx中查找与pneedle一样的supertile 的在 ptsim->tilelstidx 中的索引值
int
tssim_search_sorted_supertile (tssiminfo_t *ptsim, tstilecomb_t *pneedle, size_t *ppos)
{
    assert (NULL != ptsim);
    assert (NULL != ppos);
    return slist_find (&(ptsim->tilelstidx), pneedle, slist_cb_comp_heter_addst, ppos);
}
//#define tssim_search_sorted_supertile(ptsim, pneedle, ppos) slist_find (&(ptsim->tilelstidx), pneedle, slist_cb_comp_heter_addst, ppos);

int
tssim_unsorted2sorted_idx (tssiminfo_t *ptsim, size_t idx_unsorted, size_t *pidx_sorted)
{
    size_t idx;
    size_t i;

    assert (NULL != pidx_sorted);

    for (i = 0; i < TS_NUMTYPE_SUPERTILE (ptsim); i ++) {
        slist_data (&(ptsim->tilelstidx), i, &idx);
        if (idx_unsorted == idx) {
            break;
        }
    }
    if (i < TS_NUMTYPE_SUPERTILE (ptsim)) {
        *pidx_sorted = i;
        return 0;
    }
    return -1;
}

/*
坐标：

^
#
#
* maxy     +----------+
#          |          |
#          |   base   |
#          |          |
+----------+----------+
#          |
#   test   |
#          |
+==========+==========*==>
                      ^ maxx=(base->maxx + test->maxx)

*/

/* merge tc_base with tc_test which is rotated by rotnum and is at the position (x,y) adopt the slidetest coordinate system,
   the result is stored in tc_result */
/* 在(x,y)点处添加test, 并且 */
int
tstc_merge (tstilecomb_t *tc_base, tstilecomb_t *tc_test, size_t rotnum, tsposition_t *ppos, tstilecomb_t *tc_result)
{
    // 首先检测使用坐标偏移(x,y)是否会导致tc_add上的tile覆盖tc_base上的tile.
    // 将tc_add上各个tile加上 x,y 后添加到 tc_base 中

    tsposition_t incbase; // the shift of the base coordinate
    tsposition_t inctest; // the shift of the test coordinate
    tsposition_t maxtest; // the size of the test after rotation
    size_t i; // supertile 的内部 tile 索引
    tstilecombitem_t stitem; // 临时存储的一个tile的信息
    tstilecomb_t *tc_ret;

    assert (NULL != ppos);
    assert (tc_base != tc_result);
    assert (tc_test != tc_result);

    // get the range of test after rotating:
    memset (&inctest, 0, sizeof(inctest));
    get_cubepos_4rot_p90_down_p90 (g_cubeface_map_cur2all[0][rotnum][0], g_cubeface_map_cur2all[0][rotnum][1],
        g_cubeface_map_cur2all[0][rotnum][2], &(tc_test->maxpos), &inctest, &maxtest, &inctest);
    // 因为结合后，最小位置的起始点究竟在哪个supertile上是根据其是否于(0,0)更近来确定的.
#define INCRANGE(xxx) \
    if (ppos->xxx <= maxtest.xxx) { \
        inctest.xxx = 0; \
        incbase.xxx = maxtest.xxx - ppos->xxx; \
    } else { \
        inctest.xxx = ppos->xxx - maxtest.xxx; \
        incbase.xxx = 0; \
    }
    INCRANGE(x);
    INCRANGE(y);
#if USE_THREEDIMENTION
    INCRANGE(z);
#endif
    // 数量少的一方用作插入
    //if (slist_size (&(tc_base->tbuf)) < slist_size (&(tc_test->tbuf))) {
        tc_ret = tstc_rotate3d (tc_test, tc_result, rotnum, &inctest);
        if (tc_ret != tc_result) {
            tstc_transfer (tc_result, tc_ret);
            tstc_destroy (tc_ret);
        }
        for (i = 0; i < slist_size (&(tc_base->tbuf)); i ++) {
            slist_data (&(tc_base->tbuf), i, &stitem);
            TSPOS_ADDSELF (stitem.pos, incbase);
            if (slist_store (&(tc_result->tbuf), &stitem) < 0) {
                return -1;//break;
            }
            UPDATE_MAX_AXI (tc_result->maxpos, stitem.pos);
        }
    //} else {}
    return 0;
}

////////////////////////////
// 计算坐标的辅助函数

/*
  maxtest: test 的长度
  maxbase: base 的长度
  posx: test 左上角第一个 tile(有可能并不存在) 的位置
  rang_start: 范围最小值
  rang_num: 范围个数
*/
// 在slidetest中当固定的base和在某一个位置停留的test在做相遇检查时其相互覆盖的区域
// 输入当前MaxBase,MaxTest,x0，求出绝对位置重合范围
#define TS_ABSOLUTE_RANGE_OVERLAP_AXI(ptsrg_maxtest, ptsrg_maxbase, ptsrg_posx, ptsrg_rang_start, ptsrg_rang_num, ret) \
{ \
    if ((ptsrg_posx < 1) || (ptsrg_posx >= ptsrg_maxtest + ptsrg_maxbase)) { \
        /* 因为初始时两个 Test 和 Base 是在x,y轴上均没有覆盖地紧密相连, 所以只有posx从1开始时才有覆盖。在最后的一个tile覆盖位置为 MaxTest + MaxBase - 1 */ \
        /*assert (0);*/ \
        ptsrg_rang_num = 0; \
        ret = -1; \
        /*DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_WARNING, "WARNING: '" # ptsrg_rang_num "' is set to 0 !"); \
        DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_WARNING, "WARNING: '" # ptsrg_posx "'=%d; '" # ptsrg_maxtest "'=%d; '" # ptsrg_maxbase "'=%d;", (int)(ptsrg_posx), (int)(ptsrg_maxtest), (int)(ptsrg_maxbase)); */\
    } else { \
        if (ptsrg_maxtest > ptsrg_maxbase) { \
            if (ptsrg_posx < ptsrg_maxbase) { \
                /* (0 <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxbase) */ \
                ptsrg_rang_start = ptsrg_maxtest; \
                ptsrg_rang_num = ptsrg_posx; \
            } else if (ptsrg_posx < ptsrg_maxtest) { \
                /* (ptsrg_maxbase <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest) */ \
                ptsrg_rang_start = ptsrg_maxtest; \
                ptsrg_rang_num = ptsrg_maxbase; \
            } else { \
                /* (ptsrg_maxtest <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest + ptsrg_maxbase) */ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest + ptsrg_maxbase - ptsrg_posx; \
            } \
        } else { \
            /* ptsrg_maxtest <= ptsrg_maxbase */ \
            if (ptsrg_posx < ptsrg_maxtest) { \
                /* (0 <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest) */ \
                ptsrg_rang_start = ptsrg_maxtest; \
                ptsrg_rang_num = ptsrg_posx; \
            } else if (ptsrg_posx < ptsrg_maxbase) { \
                /* (ptsrg_maxtest <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxbase) */ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest; \
            } else { \
                /* (ptsrg_maxbase <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest + ptsrg_maxbase) */ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest + ptsrg_maxbase - ptsrg_posx; \
            } \
        } \
        ret = 0; \
    } \
}

// 在slidetest中当固定的base和在某一个位置停留的test在做相遇检查时需要的测试范围
// 包括了 Test 外一圈的tile,因为外一圈的test tile 也可能会和base 的外沿有接触
//static int ts_absolute_range_stest (tsrange_t *ptsrg)
#define TS_ABSOLUTE_RANGE_STEST_AXI(ptsrg_maxtest, ptsrg_maxbase, ptsrg_posx, ptsrg_rang_start, ptsrg_rang_num, ret) \
{ \
    if (ptsrg_posx > ptsrg_maxtest + ptsrg_maxbase) { \
        /* 因为初始时两个 Test 和 Base 是在x,y轴上均没有覆盖地紧密相连, 所以只有posx从1开始时才有覆盖。在最后的一个tile覆盖位置为 MaxTest + MaxBase - 1*/ \
        /*assert (0);*/ \
        ptsrg_rang_num = 0; \
        ret = -1; \
    } else { \
        if (ptsrg_maxtest > ptsrg_maxbase) { \
            if (ptsrg_posx <= ptsrg_maxbase) { \
                /* (0 <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxbase)*/ \
                ptsrg_rang_start = ptsrg_maxtest - 1; \
                ptsrg_rang_num = ptsrg_posx + 1; \
            } else if (ptsrg_posx < ptsrg_maxtest) { \
                /* (ptsrg_maxbase <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest)*/ \
                ptsrg_rang_start = ptsrg_maxtest - 1; \
                ptsrg_rang_num = ptsrg_maxbase + 2; \
            } else { \
                /* (ptsrg_maxtest <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest + ptsrg_maxbase)*/ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest + ptsrg_maxbase - ptsrg_posx + 1; \
            } \
        } else { \
            /* ptsrg_maxtest <= ptsrg_maxbase*/ \
            if (ptsrg_posx < ptsrg_maxtest) { \
                /* (0 <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest)*/ \
                ptsrg_rang_start = ptsrg_maxtest - 1; \
                ptsrg_rang_num = ptsrg_posx + 1; \
            } else if (ptsrg_posx <= ptsrg_maxbase) { \
                /* (ptsrg_maxtest <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxbase)*/ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest; \
            } else { \
                /* (ptsrg_maxbase <= ptsrg_posx) && (ptsrg_posx < ptsrg_maxtest + ptsrg_maxbase)*/ \
                ptsrg_rang_start = ptsrg_posx; \
                ptsrg_rang_num = ptsrg_maxtest + ptsrg_maxbase - ptsrg_posx + 1; \
            } \
        } \
    } \
    ret = 0; \
}

static int
ts_absolute_range_overlap (tsposition_t *pmaxtest/*IN*/, tsposition_t *pmaxbase/*IN*/, tsposition_t *ppos/*IN*/, tsposition_t *prang_start/*OUT*/, tsposition_t *prang_num/*OUT*/)
{
    int ret;
    int cnt_error = 0;
    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "pmaxtest", (*pmaxtest));
    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "pmaxbase", (*pmaxbase));
    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "ppos", (*ppos));
    TS_ABSOLUTE_RANGE_OVERLAP_AXI (pmaxtest->x, pmaxbase->x, ppos->x, prang_start->x, prang_num->x, ret);
    if (ret < 0) {
        cnt_error ++;
    }
    TS_ABSOLUTE_RANGE_OVERLAP_AXI (pmaxtest->y, pmaxbase->y, ppos->y, prang_start->y, prang_num->y, ret);
    if (ret < 0) {
        cnt_error ++;
    }
#if USE_THREEDIMENTION
    TS_ABSOLUTE_RANGE_OVERLAP_AXI (pmaxtest->z, pmaxbase->z, ppos->z, prang_start->z, prang_num->z, ret);
    if (ret < 0) {
        cnt_error ++;
    }
#endif
    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "prang_start", (*prang_start));
    //DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "prang_num", (*prang_num));
    if (cnt_error >= 3) {
        //DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_WARNING, "error in porcess overlay of X,Y,Z !");
        return -1;
    }
    return 0;
}

static int
ts_absolute_range_stest (tsposition_t *pmaxtest/*IN*/, tsposition_t *pmaxbase/*IN*/, tsposition_t *ppos/*IN*/, tsposition_t *prang_start/*OUT*/, tsposition_t *prang_num/*OUT*/)
{
    int ret;
    int cnt_error = 0;
    TS_ABSOLUTE_RANGE_STEST_AXI (pmaxtest->x, pmaxbase->x, ppos->x, prang_start->x, prang_num->x, ret);
    if (ret < 0) {
        cnt_error ++;
    }
    TS_ABSOLUTE_RANGE_STEST_AXI (pmaxtest->y, pmaxbase->y, ppos->y, prang_start->y, prang_num->y, ret);
    if (ret < 0) {
        cnt_error ++;
    }
#if USE_THREEDIMENTION
    TS_ABSOLUTE_RANGE_STEST_AXI (pmaxtest->z, pmaxbase->z, ppos->z, prang_start->z, prang_num->z, ret);
    if (ret < 0) {
        cnt_error ++;
    }
#endif
    if (cnt_error >= 3) {
        return -1;
    }
    return 0;
}

#if USE_THREEDIMENTION
// pos(x,y,z) 是否在指定的范围内
// pos, start, num are all of the type of tsposition_t
#define RNG_INRANGE(pos, start, num) \
    ( \
    ((((start).x <= (pos).x) && ((pos).x < (start).x + (num).x))) \
    && ((((start).y <= (pos).y) && ((pos).y < (start).y + (num).y))) \
    && ((((start).z <= (pos).z) && ((pos).z < (start).z + (num).z))) \
    )
#else
#define RNG_INRANGE(pos, start, num) \
    ( \
    ((((start).x <= (pos).x) && ((pos).x < (start).x + (num).x))) \
    && ((((start).y <= (pos).y) && ((pos).y < (start).y + (num).y))) \
    )
#endif /* USE_THREEDIMENTION */

////////////////////////////

/*
  get the neighbor position of the side of tile in the supertile
  the value related to the coordinate system be selected.
  dir: ORI_NORTH, ORI_EAST, ORI_SOUTH, ORI_WEST, ORI_FRONT, ORI_BACK
*/
#if 1
inline int tspos_step2dir (tsposition_t *ptp, int dir) __attribute__((always_inline));
inline
#endif
int
tspos_step2dir (tsposition_t *ptp, int dir)
{
    assert (NULL != ptp);
    dir = dir % 6;
    switch (dir) {
    case ORI_NORTH:
        ptp->y ++;
        break;
    case ORI_EAST:
        ptp->x ++;
        break;
    case ORI_SOUTH:
        if (ptp->y < 1) {
            return -1;
        }
        ptp->y --;
        break;
    case ORI_WEST:
        if (ptp->x < 1) {
            return -1;
        }
        ptp->x --;
        break;
#if USE_THREEDIMENTION
    case ORI_FRONT:
        if (ptp->z < 1) {
            return -1;
        }
        ptp->z --;
        break;
    case ORI_BACK:
        ptp->z ++;
        break;
#endif
    }
    return 0;
}

// 获取某方向上相临的tile坐标，如果超出当前supertile的范围，则返回-1
// ptp_max 是位置 x,y,z 个数的的最大值，非值的最大值
int
set_dir_pos_max (int dir, tsposition_t *ptp_max, tsposition_t *ptp)
{
    assert (NULL != ptp);
    dir = dir % 6;
    switch (dir) {
    case ORI_NORTH:
        if (ptp->y + 1 >= ptp_max->y) {
            return -1;
        }
        ptp->y ++;
        break;
    case ORI_EAST:
        if (ptp->x + 1 >= ptp_max->x) {
            return -1;
        }
        ptp->x ++;
        break;
    case ORI_SOUTH:
        if (ptp->y < 1) {
            return -1;
        }
        ptp->y --;
        break;
    case ORI_WEST:
        if (ptp->x < 1) {
            return -1;
        }
        ptp->x --;
        break;
#if USE_THREEDIMENTION
    case ORI_FRONT:
        if (ptp->z < 1) {
            return -1;
        }
        ptp->z --;
        break;
    case ORI_BACK:
        if (ptp->z + 1 >= ptp_max->z) {
            return -1;
        }
        ptp->z ++;
        break;
#endif
    }
    return 0;
}

/*
3D 测试，需要从六个面测试所有的情况
3D 下，滑动面和垂直方向关系
滑动面           滑动变化方向  滑动/垂直x,y,z变化  xyz变化方向
ORI_SOUTH(Down) (E,B)      +x,+z, +y          E,B, N
ORI_WEST(Left)  (B,N)      +z,+y, +x          B,N, E
ORI_FRONT       (N,E)      +y,+x, +z          N,E, B
ORI_NORTH(Top)  (F,W)      -x,-z, -y          W,F, S
ORI_EAST(Right) (S,F)      -z,-y, -x          F,S, W
ORI_BACK        (W,S)      -y,-x, -z          S,W, F

2D 下，滑动面和垂直方向关系
滑动面           滑动变化方向  滑动/垂直x,y,z变化  xyz变化方向
ORI_SOUTH(Down) (E)        +x, +y            E, N
ORI_WEST(Left)  (N)        +y, +x            N, E
ORI_NORTH(Top)  (W)        -x, -y            W, S
ORI_EAST(Right) (S)        -y, -x            S, W

所以，3D的表格可以和2D的合用
*/
#if USE_THREEDIMENTION
int g_slide_face_xyz[ORI_MAX][4] = {
// /*滑动面*/ [0]数值起始位置(0表示从0开始，1表示从max开始) 滑动时[1]XXX,[2]YYY 变化 [3]垂直变化方向/滑动面(方向)对应的反方向
/*ORI_NORTH*/ {1/*max*/, ORI_WEST,  ORI_FRONT, ORI_SOUTH},
/*ORI_EAST*/  {1/*max*/, ORI_FRONT, ORI_SOUTH, ORI_WEST},
/*ORI_SOUTH*/ {0/*0*/,   ORI_EAST,  ORI_BACK,  ORI_NORTH},
/*ORI_WEST*/  {0/*0*/,   ORI_BACK,  ORI_NORTH, ORI_EAST},
/*ORI_FRONT*/ {0/*0*/,   ORI_NORTH, ORI_EAST,  ORI_BACK},
/*ORI_BACK*/  {1/*max*/, ORI_SOUTH, ORI_WEST,  ORI_FRONT},
};
#endif

int g_slide_face_xyz_2d[][4] = {
// 2D data, copied from 3D, but only column [1]XXX,[3]垂直变化方向/滑动面(方向)对应的反方向 are used
/*ORI_NORTH*/ {1/*max*/, ORI_WEST,  0, ORI_SOUTH},
/*ORI_EAST*/  {1/*max*/, ORI_SOUTH, 0, ORI_WEST},
/*ORI_SOUTH*/ {0/*0*/,   ORI_EAST,  0, ORI_NORTH},
/*ORI_WEST*/  {0/*0*/,   ORI_NORTH, 0, ORI_EAST},
#if USE_THREEDIMENTION
// used only for GET_OPPOSITE()
/*ORI_FRONT*/ {0/*0*/,   0, 0,  ORI_BACK},
/*ORI_BACK*/  {1/*max*/, 0, 0,  ORI_FRONT},
#endif
};

// 各个方向的反方向
#define GET_OPPOSITE(ori_dir) (g_slide_face_xyz_2d[ori_dir][3])

#if USE_THREEDIMENTION
// 获取循环测试的下个坐标
// 在平面 slide_face 上移动到下个测试点所在的坐标，返回 -1 表示没有下一个了，返回0表示成功将下个点的坐标放到了ppos_cur中
// ppos_max 为 (x,y,z)最大值, 即 (x,y,z)最大值 < ppos_max
int
face_move_3d (int slide_face, tsposition_t * ppos_max, tsposition_t * ppos_cur)
{
    assert (NULL != ppos_max);
    assert (NULL != ppos_cur);
    assert ((0 <= slide_face) && (slide_face < ORI_MAX));
    if (set_dir_pos_max (g_slide_face_xyz[(slide_face)][1], ppos_max, (ppos_cur)) < 0) {
        // 在某坐标系 (A,B) 上A方向失败
        if (set_dir_pos_max (g_slide_face_xyz[(slide_face)][2], ppos_max, (ppos_cur)) < 0) {
            // 在某坐标系 (A,B) 上B方向失败
            return -1;
        }

        // 在某坐标系 (A,B) 上A方向失败，但B方向成功
        // 则将 A 方向置到初始值
        switch (g_slide_face_xyz[(slide_face)][1]) {
        case ORI_NORTH:
            ppos_cur->y = 0;
            break;
        case ORI_EAST:
            ppos_cur->x = 0;
            break;
        case ORI_SOUTH:
            ppos_cur->y = ppos_max->y - 1;
            break;
        case ORI_WEST:
            ppos_cur->x = ppos_max->x - 1;
            break;
        case ORI_FRONT:
            ppos_cur->z = ppos_max->z - 1;
            break;
        case ORI_BACK:
            ppos_cur->z = 0;
            break;
        }
    }
    return 0;
}
#endif /* USE_THREEDIMENTION */

int
face_move_2d (int slide_face, tsposition_t * ppos_max, tsposition_t * ppos_cur)
{
    assert (NULL != ppos_max);
    assert (NULL != ppos_cur);
    assert ((0 <= slide_face) && (slide_face <= ORI_WEST));
    if (set_dir_pos_max (g_slide_face_xyz_2d[(slide_face)][1], ppos_max, (ppos_cur)) < 0) {
        return -1;
    }
#if USE_THREEDIMENTION
    ppos_cur->z = 1;
#endif
    return 0;
}

#if USE_THREEDIMENTION
char g_dir_stestofslide_3d[ORI_MAX][ORI_MAX] = {
// 索引为当前滑动面，结果为测试方向顺序
    /* 垂直方向,  右手(前进)方向, 前进方向2, 后退方向, 后退方向2, 垂直反方向 */
    {ORI_SOUTH, ORI_WEST,  ORI_FRONT, ORI_EAST,  ORI_BACK,  ORI_NORTH}, /* ORI_NORTH */
    {ORI_WEST,  ORI_FRONT, ORI_SOUTH, ORI_BACK,  ORI_NORTH, ORI_EAST},  /* ORI_EAST */
    {ORI_NORTH, ORI_EAST,  ORI_BACK,  ORI_WEST,  ORI_FRONT, ORI_SOUTH}, /* ORI_SOUTH */
    {ORI_EAST,  ORI_NORTH, ORI_BACK,  ORI_SOUTH, ORI_FRONT, ORI_WEST},  /* ORI_WEST */
    {ORI_BACK,  ORI_EAST,  ORI_NORTH, ORI_SOUTH, ORI_WEST,  ORI_FRONT}, /* ORI_FRONT */
    {ORI_FRONT, ORI_WEST,  ORI_SOUTH, ORI_EAST,  ORI_NORTH, ORI_BACK},  /* ORI_BACK */
};
#endif
char g_dir_stestofslide_2d[4/*ORI_MAX*/][ORI_MAX] = {
// 索引为当前滑动方向，结果为测试方向顺序
    /* 垂直方向,  右手(前进)方向, 后退方向, 垂直反方向 */
    {ORI_EAST,  ORI_NORTH, ORI_SOUTH, ORI_WEST},  /* ORI_NORTH */
    {ORI_NORTH, ORI_EAST,  ORI_WEST,  ORI_SOUTH}, /* ORI_EAST */
    {ORI_WEST,  ORI_SOUTH, ORI_NORTH, ORI_EAST},  /* ORI_SOUTH */
    {ORI_SOUTH, ORI_WEST,  ORI_EAST,  ORI_NORTH}, /* ORI_WEST */
};

typedef struct _tsslidetestinfo_t {
    tssiminfo_t *ptsim;     /* IN; ptsim 的第1部分被用到 */

    tstilecomb_t *tc_base; /* IN; Base的数据 */
    tstilecomb_t *tc_test; /* IN; Test的数据 */

    size_t rotnum;     /* IN; tc_test 被 顺时针旋转、上翻、顺时针旋转的次数的索引，在g_cubeface_map_cur2all[0][rotnum]中. 在作 plumb 和 mesh 测试时，旋转后的位置是临时计算出来的 */
    char dir_cur_slide;    /* IN; 当前滑动的方向 */
    stk_t *pstk_local;     /* IN/USE; 待检测的位置 */

    sortedlist_t sl_checked_global; /* IN/UPDATE; 检查哪些位置被递归测试过,全局 */
    sortedlist_t sl_checked_local; /* USE; 检查哪些位置被测试过,在某个posx下 */

    tsposition_t tsrgabs_maxtest;
    tsposition_t tsrgabs_maxbase;
    tsposition_t tsrgabs_pos; // 当前 test 测试所在坐标位置
    tsposition_t tsrgabs_rang_start;
    tsposition_t tsrgabs_rang_num;

    tsposition_t tsrgstst_maxtest;
    tsposition_t tsrgstst_maxbase;
    tsposition_t tsrgstst_pos;
    tsposition_t tsrgstst_rang_start;
    tsposition_t tsrgstst_rang_num;
} tsslidetestinfo_t;

// 从绝对位置求Test的内部相对坐标
static void
pos_abs2real_4test (tsslidetestinfo_t *psti/*IN*/, tsposition_t *ppos_in/*IN*/, tsposition_t *ppos_out/*OUT*/)
{
    ppos_out->x = ppos_in->x - psti->tsrgabs_pos.x;
    ppos_out->y = ppos_in->y - psti->tsrgabs_pos.y;
#if USE_THREEDIMENTION
    ppos_out->z = ppos_in->z - psti->tsrgabs_pos.z;
#endif
}

// 从绝对位置求Base的内部相对坐标
static void
pos_abs2real_4base (tsslidetestinfo_t *psti/*IN*/, tsposition_t *ppos_in/*IN*/, tsposition_t *ppos_out/*OUT*/)
{
    ppos_out->x = ppos_in->x - psti->tsrgabs_maxtest.x;
    ppos_out->y = ppos_in->y - psti->tsrgabs_maxtest.y;
#if USE_THREEDIMENTION
    ppos_out->z = ppos_in->z - psti->tsrgabs_maxtest.z;
#endif
}

int
tsstinfo_init (tsslidetestinfo_t *psti, tstilecomb_t *tc_base, tstilecomb_t *tc_test, size_t rotnum)
{
    tsposition_t tspos_tmp1;
    tsposition_t tspos_tmp2;
    memset (psti, 0, sizeof (*psti));

    psti->rotnum = rotnum;
    psti->tc_test = tc_test;
    psti->tc_base = tc_base;

    memset (&tspos_tmp1, 0, sizeof (tspos_tmp1));

    // 注意，只有在 tsrgabs_x/tsrgabs_y 中的 maxtest才是当前test的该方向上的最大值,因为此时test被旋转了
    // 而base不被旋转，所以其最大值可以在任何有 maxbase 的地方获取
    memmove (&(psti->tsrgabs_maxbase), &(tc_base->maxpos), sizeof (tsposition_t));
    GET_CUBEPOS_4ROT_TEST (g_cubeface_map_cur2all[0][psti->rotnum][0], g_cubeface_map_cur2all[0][psti->rotnum][1],
        g_cubeface_map_cur2all[0][psti->rotnum][2], &(tc_test->maxpos), &tspos_tmp1,
        &(psti->tsrgabs_maxtest), &tspos_tmp2);

    memmove (&(psti->tsrgstst_maxbase), &(tc_base->maxpos), sizeof (tsposition_t));
    GET_CUBEPOS_4ROT_TEST (g_cubeface_map_cur2all[0][psti->rotnum][0], g_cubeface_map_cur2all[0][psti->rotnum][1],
        g_cubeface_map_cur2all[0][psti->rotnum][2], &(tc_test->maxpos), &tspos_tmp1,
        &(psti->tsrgstst_maxtest), &tspos_tmp2);

    slist_init (&(psti->sl_checked_local), sizeof (tsposition_t), NULL, slist_cb_comp_tspos, slist_cb_swap_tspos);
    slist_init (&(psti->sl_checked_global), sizeof (tsposition_t), NULL, slist_cb_comp_tspos, slist_cb_swap_tspos);

    return 0;
}

void
tsstinfo_clear (tsslidetestinfo_t *psti)
{
    slist_clear (&(psti->sl_checked_local), NULL);
    slist_clear (&(psti->sl_checked_global), NULL);
}

// if there has the tile in the position
int
slist_cb_comp_heter_tstilecomb_hastile (void *userdata, void * data_pin, void * data2)
{
    tstilecombitem_t *ptsci2 = (tstilecombitem_t *)data2;

    return tspos_cmp ((tsposition_t *)data_pin, &(ptsci2->pos));
}

// 测试 test 和 base 在当前位置是否能够结合
// 通过计算各个 test tile 的四周对应 base tile 之间的glue 及其 强度和 是否大于当前温度来确定
// 由 tstc_plumb_test() 或 tstc_mesh_test_godhand() 调用
// plist_points_ok: 返回结合的位置信息。 tsstpos_t 类型的列表
static int
tstc_plumb_position_test (tsslidetestinfo_t *psti, tsposition_t *ppos, memarr_t *plist_points_ok, size_t cnt_resi_dir[ORI_MAX])
{
    size_t kk;
    tsposition_t pos;
    size_t cnt_glue; // 某个位置上的 glue 的总计；
    tsposition_t tspos_base; // 临时，base的某个点
    tsposition_t tspos_test; // 临时，test的某个点
    int ret;
#if USE_THREEDIMENTION
    size_t max_z;
    size_t min_z = 0;
#endif
    cnt_glue = 0;
    memset (cnt_resi_dir, 0, sizeof (cnt_resi_dir));

    assert (NULL != ppos);
    // 用作检测Test测试范围的参数
    memmove (&(psti->tsrgstst_pos), ppos, sizeof (*ppos));
    ts_absolute_range_stest (&(psti->tsrgstst_maxtest), &(psti->tsrgstst_maxbase), ppos, &(psti->tsrgstst_rang_start), &(psti->tsrgstst_rang_num));

    // 用作检测Base范围的参数. 如果该区域的test上的某位置不存在tile, ....
    memmove (&(psti->tsrgabs_pos), ppos, sizeof (*ppos));
    ts_absolute_range_overlap (&(psti->tsrgabs_maxtest), &(psti->tsrgabs_maxbase), ppos, &(psti->tsrgabs_rang_start), &(psti->tsrgabs_rang_num));

#if USE_PRESENTATION
    if (psti->ptsim->cb_initadheredect) {
        tstilecombitem_t info_test; // 当前 test 状态，rotnum 表示 test 被旋转的次数，x,y,z 表示其位置
        info_test.idtile = 0; /*NO USED*/
        info_test.rotnum = psti->rotnum;
        memmove (&(info_test.pos), ppos, sizeof (*ppos));
        psti->ptsim->cb_initadheredect (psti->ptsim->userdata, psti->tc_base, psti->tc_test, psti->ptsim->temperature, &info_test);
    }
#endif
    // 对每个需要测试的Test supertile 上的每个 tile 的四方向上作检查

#if USE_THREEDIMENTION
    max_z = psti->tsrgstst_rang_num.z;
    if (psti->ptsim->flg_is2d) {
        max_z = 1;
    }

    for (pos.z = min_z; pos.z < max_z; pos.z ++)
#endif
    for (pos.y = 0; pos.y < psti->tsrgstst_rang_num.y; pos.y ++) {
        for (pos.x = 0; pos.x < psti->tsrgstst_rang_num.x; pos.x ++) {
            // find the tile of the Base at 4 dir of one tile of the Test
            // i + tsrgstst_y->rang_start;
            // j + tsrgstst_x->rang_start;

            // 当前
            memmove (&tspos_base, &(psti->tsrgstst_rang_start), sizeof (tspos_base));
            TSPOS_ADDSELF (tspos_base, pos);

            DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "tspos_test", tspos_base);

            pos_abs2real_4test (psti, &tspos_base, &tspos_base);

            // Test 上是否有tile
            // if (0 == BM2D_IS_SET (psti->bmbuf_test, psti->tsrgabs_maxtest.x, tspos_base.x, tspos_base.y)) {
            // 因为 test 被旋转过，所以需要求出旋转前原始的位置
            GET_CUBEPOS_4ROT_TEST_REVERSE (g_cubeface_map_cur2all[0][psti->rotnum][0],
                g_cubeface_map_cur2all[0][psti->rotnum][1], g_cubeface_map_cur2all[0][psti->rotnum][2],
                &(psti->tsrgabs_maxtest), &tspos_base, NULL, &tspos_test);
            if (! tstc_hastile (psti->tc_test, &tspos_test)) {
                // Test 的该位置上没有 tile, 则忽略
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "no such test tile @ ", tspos_test);
                continue;
            }

            for (kk = 0; kk < psti->ptsim->max_faces; kk ++) {
                // 对每个方向都检测是否有Base的tile
                int glue_base;
                int glue_test;
                size_t idx_tile_base;
                size_t idx_tile_test;
                tstilecombitem_t tsipin;
                tstilecombitem_t *ptsi_test;
                tstilecombitem_t *ptsi_base;
                //memset (&tsipin, 0, sizeof (tsipin));

                // 获取一个方向上邻接的Base的Tile的坐标
                memmove (&tspos_base, &(psti->tsrgstst_rang_start), sizeof (tspos_base));
                TSPOS_ADDSELF (tspos_base, pos);

                assert ((psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_2d)
#if USE_THREEDIMENTION
                    || (psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_3d)
#endif
                );
                if (tspos_step2dir (&tspos_base, (int) G_DIR_STESTOFSLIDE (psti->ptsim, (int)psti->dir_cur_slide, kk)) < 0) {
                    // 坐标超出范围(x,y其中一项<0)
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "tspos_step2dir() - out of range: "
                        , tspos_base);
                    continue;
                }

                // 邻接当前位置的该方向上的点是否在Base矩阵上
                ret = RNG_INRANGE (tspos_base, psti->tsrgabs_maxtest, psti->tsrgabs_maxbase);
                if (ret == 0) {
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "this dir not in range: ", tspos_base);
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "\ttsrgabs_maxtest: ", psti->tsrgabs_maxtest);
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "\ttsrgabs_maxbase: ", psti->tsrgabs_maxbase);
                    continue;
                }
                // 该绝对位置处于Base的范围

                // 看Base上该处是否存在tile
                tsipin.idtile = 0;
                tsipin.rotnum = 0;

                // 位置转换成 base 自身的相对位置。因为base没有被旋转，所以不必作逆旋转
                pos_abs2real_4base (psti, &tspos_base, &(tsipin.pos));
                //if (0 == BM2D_IS_SET (psti->bmbuf_base, psti->tc_base->maxpos.x, tsipin.pos.x, tsipin.pos.y)) {
                //    continue;
                //}
                // test the glue:
                //   get the info from the list

                // base position
                if (slist_find (&(psti->tc_base->tbuf), &tsipin, NULL, &idx_tile_base) < 0) {
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "not fould base: ", tspos_base);
                    continue;
                }

                //slist_data (&(tc_add->tbuf), idx_tile, &tci);
                ptsi_base = (tstilecombitem_t *) slist_data_lock (&(psti->tc_base->tbuf), idx_tile_base);

                // test position
                memmove (&(tsipin.pos), &(psti->tsrgstst_rang_start), sizeof (tsipin.pos));
                TSPOS_ADDSELF (tsipin.pos, pos);

                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "check test position(abs):", tsipin.pos);
                pos_abs2real_4test (psti, &(tsipin.pos), &tspos_base);
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "check test position(abs2real):", tspos_base);

                // convert the rotated coordinate to the origin coordinate of the tc_test
                // 因为test被旋转过，所以要作逆旋转
                GET_CUBEPOS_4ROT_TEST_REVERSE (g_cubeface_map_cur2all[0][psti->rotnum][0],
                    g_cubeface_map_cur2all[0][psti->rotnum][1], g_cubeface_map_cur2all[0][psti->rotnum][2],
                    &(psti->tsrgstst_maxtest), &tspos_base, NULL, &(tsipin.pos));
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "check test position(real2rotback):", tsipin.pos);

                if (slist_find (&(psti->tc_test->tbuf), &tsipin, NULL, &idx_tile_test) < 0) {
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_ERROR, "fatal error, not found test position:", tsipin.pos);
                    assert (0);
                    continue;
                }
                ptsi_test = (tstilecombitem_t *) slist_data_lock (&(psti->tc_test->tbuf), idx_tile_test);

                assert (NULL != ptsi_base);
                assert (NULL != ptsi_test);
                assert (NULL != psti->ptsim);

                // to check if the glues can be stick with each other?
                assert ((psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_2d)
#if USE_THREEDIMENTION
                    || (psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_3d)
#endif
                );
                // calculate the rotated direction of the glue of the tc_test
                glue_test = tile_get_glue (psti->ptsim->ptilevec, psti->ptsim->num_tilevec, ptsi_test,
                        (int) G_DIR_STESTOFSLIDE(psti->ptsim, (int)psti->dir_cur_slide, kk), psti->rotnum);
                glue_base = tile_get_glue (psti->ptsim->ptilevec, psti->ptsim->num_tilevec, ptsi_base,
                        (int)(GET_OPPOSITE( (int) G_DIR_STESTOFSLIDE(psti->ptsim, (int)psti->dir_cur_slide, kk) )), 0);

                slist_data_unlock (&(psti->tc_base->tbuf), idx_tile_base);
                slist_data_unlock (&(psti->tc_test->tbuf), idx_tile_test);

#if USE_PRESENTATION
                tsipin.rotnum = (int)G_DIR_STESTOFSLIDE(psti->ptsim, (int)psti->dir_cur_slide, kk);

                memmove (&(tsipin.pos), &(psti->tsrgstst_rang_start), sizeof (tsipin.pos));
                TSPOS_ADDSELF (tsipin.pos, pos);
#endif

                if (GLUE_CAN_ADHERE (glue_test, glue_base)) {
                    //assert (glue_test < psti->num_gluevec);
                    cnt_glue += psti->ptsim->pgluevec[glue_test];
#if USE_PRESENTATION
                    if (NULL != psti->ptsim->cb_adherepos) {
                        psti->ptsim->cb_adherepos (psti->ptsim->userdata, 1/*true*/,  &tsipin);
                    }
                } else {
                    if (NULL != psti->ptsim->cb_adherepos) {
                        psti->ptsim->cb_adherepos (psti->ptsim->userdata, 0/*false*/, &tsipin);
                    }
#endif
                }

                // calculate the abuted tile of Base
                cnt_resi_dir[kk] ++;
                // ...
                // 判断Test与Base的距离不要超过1格
            }
        }
    }
#if USE_PRESENTATION
    if (NULL != psti->ptsim->cb_clearadheredect) {
        psti->ptsim->cb_clearadheredect (psti->ptsim->userdata);
    }
#endif
    if (cnt_glue >= psti->ptsim->temperature) {
        // save the position
        tsstpos_t stpos;
        stpos.rotnum = psti->rotnum;
        memmove (&(stpos.pos), ppos, sizeof (stpos.pos));
        ma_insert (plist_points_ok, ma_size(plist_points_ok), &stpos);
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_INFO, "!! add possible position:", stpos.pos);
    }
    return 0;
}

// 递归查看从一个位置开始，看能否粘合两个supertile
// plist_points_ok: 返回结合的位置信息。 tsstpos_t 类型的列表
// 注意psti->tc_test 是原始没有根据 psti->rotnum 转置的，需要在使用中转换坐标。
static void
tstc_plumb_test (tsslidetestinfo_t *psti, memarr_t *plist_points_ok)
{
    size_t k;

    size_t cnt_resi_dir[ORI_MAX]; // The number of the base tile in six directions

    tsposition_t tspos_popup; // 从栈中弹出的一个点的坐标
    tsposition_t tspos_base; // 临时，base的某个点

    DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "BEGIN %s() ...", __func__);

    // sl_checked_local 保存垂直探测时那些不需要被邻接探测的点，如已经被压入栈待检测的点和被忽略的点（如不处于Base上）
    slist_rmalldata (&(psti->sl_checked_local), NULL);

    for (; st_size (psti->pstk_local) > 0; ) {
        st_pop (psti->pstk_local, &tspos_popup);
        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "check", tspos_popup);

        if (slist_find (&(psti->sl_checked_global), &tspos_popup, NULL, NULL) >= 0) {
            // 在该rotnum下的test的该位置已经被探测过了，这个位置可能是其他出发点出发滑动测试检测过的
            DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_DEBUG, "other checked, ignore:", tspos_popup);
            continue;
        }
        slist_store (&(psti->sl_checked_global), &tspos_popup);

        memset (cnt_resi_dir, 0, sizeof (cnt_resi_dir));
        tstc_plumb_position_test (psti, &(tspos_popup), plist_points_ok, cnt_resi_dir);

        for (k = 0; k < psti->ptsim->max_faces; k ++) {
            // 判断6个方向上是否可移动
            if (cnt_resi_dir[k] < 1) {
                // 该方向上没有Base的tile
                // push the position to stack

                assert ((psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_2d)
#if USE_THREEDIMENTION
                    || (psti->ptsim->p_g_dir_stestofslide == (char *)g_dir_stestofslide_3d)
#endif
                );
                // 获取一个方向上邻接的点的坐标
                memmove (&tspos_base, &tspos_popup, sizeof (tspos_popup));
                if (tspos_step2dir (&tspos_base, (int) G_DIR_STESTOFSLIDE(psti->ptsim, (int)psti->dir_cur_slide, k)) < 0) {
                    continue;
                }
                // 检查是否与base没有交界了
                if ((tspos_base.x > psti->tc_base->maxpos.x + psti->tsrgabs_maxtest.x)
                    || (tspos_base.y > psti->tc_base->maxpos.y + psti->tsrgabs_maxtest.y)
#if USE_THREEDIMENTION
                    || (tspos_base.z > psti->tc_base->maxpos.z + psti->tsrgabs_maxtest.z)
#endif
                   )
                {
                    continue;
                }

                if (slist_find (&(psti->sl_checked_local), &tspos_base, NULL, NULL) >= 0) {
                    // 已经被探测过了
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "the position was processed, ignore:",tspos_base);
                    continue;
                }

                slist_store (&(psti->sl_checked_local), &tspos_base);
                if (slist_find (&(psti->sl_checked_global), &tspos_base, NULL, NULL) < 0) {
                    // 没被探测过
                    // 保存下个可以移动到的位置到堆栈中
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "save the position to stack:",tspos_base);
                    st_push (psti->pstk_local, &tspos_base);
                }

            }
        }
    }
    DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "END %s() ...", __func__);
}

typedef struct _ts_meshtest_data_t {
    size_t face;
    tsposition_t pos_init;
} ts_meshtest_data_t;
// 用作在各个面上作 plumb 测试时起始位置的设置。本g_meshtest_data0用于初始化实际使用的g_meshtest_data。g_meshtest_data的某些面（如 BACK、EAST、NORTH等）的初始值设置为当前supertile上tile坐标的最大值。
ts_meshtest_data_t g_meshtest_data0[ORI_MAX] = {
#if USE_THREEDIMENTION
    {ORI_WEST,  {0, 0, 0}},
    {ORI_SOUTH, {0, 0, 0}},
    {ORI_EAST,  {0, 0, 0}},
    {ORI_NORTH, {0, 0, 0}},
    {ORI_FRONT, {0, 0, 0}},
    {ORI_BACK,  {0, 0, 0}},
#else
    {ORI_WEST,  {0, 0}},
    {ORI_SOUTH, {0, 0}},
    {ORI_EAST,  {0, 0}},
    {ORI_NORTH, {0, 0}},
#endif
};

/* test if two tilecomp can be merged, return the positions(dir,gluevalue,x,y) */
// plist_points_ok: 返回结合的位置信息。 tsstpos_t 类型的列表
// ptsim: the shared part of it will be used in tstc_plumb_test()
// 注意 tc_test 是原始没有根据 rotnum 转置的，需要在使用中转换坐标。转换的方法是：
//   根据 rotface 切换到立方体的六面中的一面，其中当前面放在最前面处理，旋转方法是根据查表得到是先旋转n1次再上翻n2次；
//   然后根据 rotnum 旋转到四个位置的一个，其中当前面放在最前面处理，然后是顺时针旋转一次、两次、三次等。
int
tstc_mesh_test_nature (tssiminfo_t *ptsim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok)
{
    // slide test
    // bitmap test
    // 步骤：
    // 先使用bitmap检测一个接触位置，该接触位置是逆时针方向每次固定测试。
    // 如果检测可以接触的话，则检测接触位置各个方向上是否可以粘合，即满足 temperature 条件。
    // 如果满足，则记录下该位置

    int ii;
    size_t i;
    tsposition_t tspos_push; // 将压入栈中的一个坐标点
    tsposition_t tspos_max;  // 最大的坐标值
    tsslidetestinfo_t slidetestinfo;

    assert (NULL != tc_base);
    assert (NULL != tc_test);
    assert (NULL != ptsim);
    assert (NULL != plist_points_ok);

    DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "BEGIN %s() ...", __func__);

    stk_t stk_local; // 在某 posx , 将可以访问的临近位置压入该栈，待稍后做深度优先搜索。
    st_init (&stk_local, sizeof (tsposition_t));

#if USE_THREEDIMENTION
    ii = 0;
    if (ptsim->flg_is2d) {
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Simulation for 2D");
        ii = 20;
    }
#else
    ii = 20;
#endif
    if (ptsim->flg_norotate) {
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Simulation for NON-rotatable");
        ii = 23;
    }
    assert ((0 <= ii) && (ii < 24));
    assert ((0 <= (23 - ii)) && ((23 - ii) < TILE_STATUS_MAX));
    //DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "rotations checked: %d", 24 - ii);
    for (; ii < 24; ii ++) {
        // switch to all of the six faces of test supertile
        ts_meshtest_data_t g_meshtest_data[ORI_MAX];

        // rotate the four position of the test supertile
        // 2D test supertile 的四个旋转位置的测试
        tsstinfo_init (&slidetestinfo, tc_base, tc_test, 23 - ii);
        slidetestinfo.ptsim = ptsim;
        slidetestinfo.pstk_local = &stk_local;

        memmove (g_meshtest_data, g_meshtest_data0, sizeof (g_meshtest_data));
        for (i = 0; i < ORI_MAX; i ++) {
            switch (g_meshtest_data[i].face) {
#if USE_THREEDIMENTION
            case ORI_BACK:
#endif
            case ORI_EAST:
            case ORI_NORTH:
                TSPOS_ADDSELF (g_meshtest_data[i].pos_init, slidetestinfo.tsrgabs_maxtest);
                TSPOS_ADDSELF (g_meshtest_data[i].pos_init, slidetestinfo.tsrgabs_maxbase);
                break;
            }
        }
        memmove (&tspos_max, &(slidetestinfo.tsrgabs_maxtest), sizeof (tspos_max));
        TSPOS_ADDSELF (tspos_max, slidetestinfo.tsrgabs_maxbase);

        // sl_checked_global 保存在test的该旋转角度下已经被检测的点和被忽略的点（如不处于Base上）
        slist_rmalldata (&(slidetestinfo.sl_checked_global), NULL);

        // 在各个面（6个面）上滑动，看是否能够作垂直测试
        // 如果是2D，则在前四个面(NORTH,EAST,SOUTH,WEST)上滑动
        for (i = 0; i < ptsim->max_faces; i ++) {
            DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "face number: %d", i);
            memmove (&tspos_push, &(g_meshtest_data[i].pos_init), sizeof (tspos_push));
            for (;;) {
                // 在该平面上的 'x' 'y' 轴上变化。
                assert ((ptsim->cb_face_move == face_move_2d)
#if USE_THREEDIMENTION
                    || (ptsim->cb_face_move == face_move_3d)
#endif
                    );
                if (ptsim->cb_face_move (g_meshtest_data[i].face, &tspos_max, &tspos_push) < 0) {
                    break;
                }
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "face move to:", tspos_push);

                slidetestinfo.dir_cur_slide = g_meshtest_data[i].face;
                st_push (&stk_local, &tspos_push);
                tstc_plumb_test (&slidetestinfo, plist_points_ok);
            }
        }
    }

    st_clear (&stk_local, NULL);
    tsstinfo_clear (&slidetestinfo);

    DBGMSG (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "END %s() ...", __func__);
    return 0;
}

// test all of the positions of the base supertile, including the holes in the base supertile.
// call this function when ptsim->flg_inserthole = 1
// plist_points_ok: 返回结合的位置信息。 tsstpos_t 类型的列表
int
tstc_mesh_test_godhand (tssiminfo_t *ptsim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok)
{
    tsposition_t pos;
    tsposition_t pos_ol;
    int ii;
    tsslidetestinfo_t slidetestinfo;
    tsslidetestinfo_t *psti = &slidetestinfo;
    size_t cnt_resi_dir[ORI_MAX]; // test立方体六个方向上遇到的Base的tile的个数
    tsposition_t tspos_base; // 临时，base的某个点
    tsposition_t tspos_test; // 临时，test的某个点
#if USE_THREEDIMENTION
    size_t min_z; // used for 2D
    size_t max_z; // used for 2D
#endif
    char flg_overlaped = 0;

    assert (NULL != tc_base);
    assert (NULL != tc_test);
    assert (NULL != ptsim);
    assert (NULL != plist_points_ok);

#if USE_THREEDIMENTION
    ii = 0;
    min_z = 0;
    if (ptsim->flg_is2d) {
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Simulation for 2D");
        ii = 20;
        // 在2D下，z的变化范围只限于1
        min_z = 1;
    }
#else
    ii = 20;
#endif
    if (ptsim->flg_norotate) {
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Simulation for NON-rotatable");
        ii = 23;
    }
    assert ((0 <= ii) && (ii < 24));
    assert ((0 <= (23 - ii)) && ((23 - ii) < TILE_STATUS_MAX));
    for (; ii < 24; ii ++) {
        // switch to all of the six faces of test supertile

        // rotate the four position of the test supertile
        // 2D test supertile 的四个旋转位置的测试
        tsstinfo_init (&slidetestinfo, tc_base, tc_test, 23 - ii);
        slidetestinfo.ptsim = ptsim;

#if USE_THREEDIMENTION
        if (ptsim->flg_is2d) {
            max_z = 2;
        } else {
            max_z = slidetestinfo.tsrgabs_maxtest.z + slidetestinfo.tsrgabs_maxbase.z + 1;
        }
        for (pos.z = min_z; pos.z < max_z; pos.z ++)
#endif
        for (pos.x = 0; pos.x < slidetestinfo.tsrgabs_maxtest.x + slidetestinfo.tsrgabs_maxbase.x + 1; pos.x ++) {
            for (pos.y = 0; pos.y < slidetestinfo.tsrgabs_maxtest.y + slidetestinfo.tsrgabs_maxbase.y + 1; pos.y ++) {

                // check the overlay area of the two supertiles
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "try to chk point:", pos);

                // 用作检测Test测试范围的参数
                memmove (&(psti->tsrgstst_pos), &pos, sizeof(pos));
                ts_absolute_range_stest (&(psti->tsrgstst_maxtest), &(psti->tsrgstst_maxbase), &pos
                    , &(psti->tsrgstst_rang_start), &(psti->tsrgstst_rang_num));

                // 用作检测Base范围的参数
                memmove (&(psti->tsrgabs_pos), &pos, sizeof(pos));
                ts_absolute_range_overlap (&(psti->tsrgabs_maxtest), &(psti->tsrgabs_maxbase), &pos
                    , &(psti->tsrgabs_rang_start), &(psti->tsrgabs_rang_num));

                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "psti->tsrgabs_rang_num: "
                    , psti->tsrgabs_rang_num);
                DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "psti->tsrgstst_rang_num: "
                    , psti->tsrgstst_rang_num);

                // make sure that there're no tiles in the same position
                flg_overlaped = 0;
#if USE_THREEDIMENTION
                for (pos_ol.z = 0; (0 == flg_overlaped) && (pos_ol.z < psti->tsrgstst_rang_num.z); pos_ol.z ++)
#endif
                for (pos_ol.x = 0; (0 == flg_overlaped) && (pos_ol.x < psti->tsrgstst_rang_num.x); pos_ol.x ++)
                for (pos_ol.y = 0; (0 == flg_overlaped) && (pos_ol.y < psti->tsrgstst_rang_num.y); pos_ol.y ++) {
                    memmove (&tspos_base, &(psti->tsrgstst_rang_start), sizeof (tspos_base));
                    TSPOS_ADDSELF (tspos_base, pos_ol);

                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "base abs ", tspos_base);
                    pos_abs2real_4base (psti, &tspos_base, &tspos_base);
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "base real", tspos_base);

                    if (tstc_hastile (psti->tc_base, &tspos_base)) {
                        memmove (&tspos_base, &(psti->tsrgstst_rang_start), sizeof (tspos_base));
                        TSPOS_ADDSELF (tspos_base, pos_ol);

                        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "test abs ", tspos_base);
                        pos_abs2real_4test (psti, &tspos_base, &tspos_base);
                        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "test real", tspos_base);

                        // Test 上是否有tile
                        // 因为 test 被旋转过，所以需要求出旋转前原始的位置
                        GET_CUBEPOS_4ROT_TEST_REVERSE (g_cubeface_map_cur2all[0][psti->rotnum][0],
                            g_cubeface_map_cur2all[0][psti->rotnum][1], g_cubeface_map_cur2all[0][psti->rotnum][2],
                            &(psti->tsrgabs_maxtest), &tspos_base, NULL, &tspos_test);
                        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_LOG, "test real rot", tspos_test);
                        if (tstc_hastile (psti->tc_test, &tspos_test)) {
                            // 存在相同位置上既有属于base的tile 也有属于test的tile
                            // 則退出并忽略掉該位置
                            flg_overlaped = 1;
                            DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_INFO, "test & base tile overlap @ ", tspos_test);
                            break;
                        }
#if 0 //DEBUG
                    } else {
                        DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_WARNING, "base has NO tile @ ", tspos_base);
#endif // DEBUG
                    }
                }
                if (flg_overlaped) {
                    // 存在相同位置上同時有 test 和 base 的 tile
                    // 忽略掉該位置
                    DBGTS_SHOWPOS (PFDBG_CATLOG_TSMESH, PFDBG_LEVEL_INFO, "test & base tile overlap (2) @ ", tspos_test);
                    continue;
                }

                // check the four sides of one tile of the test and calculate the glue
                // if glue >= t then save the position to the plist_points_ok
                memset (cnt_resi_dir, 0, sizeof (cnt_resi_dir));
                tstc_plumb_position_test (psti, &pos, plist_points_ok, cnt_resi_dir);

            }
        }
    }

    tsstinfo_clear (&slidetestinfo);
    return 0;
}

#if 0
/*
^
#
#
* maxy     +----------+
#          |          |
#          |   base   |
#          |          |
+----------+----------+
#          |
#   test   |
#          |
+==========+==========*==>
                      ^ maxx=(base->maxx + test->maxx)

// if the two supertiles could be combined at postion (x,y), return 1, other wise return 0
  x,y: the position of TEST Supertile
    please be careful: the position of the base is placed at (MaxXtest, MaxYtest)
    and the range of the position of test is [0,MaxTest+MaxBase]
// idx_base, idx_test: is the index of usb-supertiles of the pbmplist_subst
   ptilelist: the list of all supertiles. (list of tstilecomb_t)
*/
static int
tstc_canbecombined (memarr_t *ptilelist, tsposition_t *ppos, size_t idx_base, size_t idx_test)
//(memarr_t *pbmplist_subst, tssiminfo_t *ptsim, tstilecomb_t *ptc_2bsplit, size_t temperature, size_t idx1, size_t idx2)
{
    size_t i;
    size_t j;
    size_t k;
    size_t idx_tile;
    tsposition_t tspos;

    tstilecomb_t *pitem_test;
    tstilecomb_t *pitem_base;

    size_t cnt_glue;
    size_t glue_cur;
    size_t glue_alien;

    tstilecombitem_t tsci;

    assert (NULL != ppos);
    assert (NULL != ptilelist);
    if (idx1 >= ma_size (ptilelist)) {
        return -1;
    }
    if (idx2 >= ma_size (ptilelist)) {
        return -1;
    }

    pitem_test = (tstilecomb_t *)ma_data_lock (ptilelist, idx_test);
    pitem_base = (tstilecomb_t *)ma_data_lock (ptilelist, idx_base);

    // 使用 min (slist_size (pitem1), slist_size (pitem2)) 减少判断
    //if (slist_size (pitem1) > slist_size (pitem2)) {
    //    pitem_tmp = pitem1;
    //    pitem1 = pitem2;
    //    pitem2 = pitem_tmp;
    //}

    cnt_glue = 0;
    for (i = 0; i < slist_size (&(pitem_test->tbuf)); i ++) {
        // get the tile information from ptc_2bsplit
        slist_data (&(pitem_test->tbuf), i, &tsci);
        // check if there exist a tile at the current position of base supertile
        // if have, then return FAIL
        memmove (&tspos, &tsci, sizeof (tsci));
        if (slist_find (&(pitem_base->tbuf), &tspos, NULL, &idx_tile) < 0) {
            // have tile in this position, error
            ret = -1;
            break;
        }

        // check the 4 edges of the tile
        for (k = 0; k < 4; k ++) {
            tstilecombitem_t tsci2;
            tstilecombitem_t *ptsi;
            // get the coordinate of tile which is alien to this tile
            memmove (&tspos, &tsci, sizeof (tsci));
            if (tspos_step2dir (&tspos, k) < 0) {
                // out of range (x or y < 0)
                continue;
            }
            if (slist_find (&(pitem_base->tbuf), &tspos, NULL, &idx_tile) < 0) {
                // no such tile in this position, skip
                continue;
            }
            // check the glue function
            ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), j);
            glue_cur   = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)k, 0);
            slist_data_unlock (&(ptc_2bsplit->tbuf), j);
            ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), idx_tile);
            glue_alien = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)(GET_OPPOSITE((int)k)), 0);
            slist_data_unlock (&(ptc_2bsplit->tbuf), idx_tile);

            if (GLUE_CAN_ADHERE (glue_cur, glue_alien)) {
                // add the glue
                cnt_glue += ptsim->pgluevec[glue_cur];
            }
        }
    }

    ma_data_unlock (pbmplist_subst, idx1);
    ma_data_unlock (pbmplist_subst, idx2);
    if (cnt_glue >= temperature) {
        return 1;
    }
    return 0;
}
#endif

#if 0
// if the two supertiles could be combined, return 1, other wise return 0
// idx1, idx2: is the index of usb-supertiles of the pbmplist_subst
// ptc_2bsplit: the supertile
static int
stsub_cancombine (memarr_t *pbmplist_subst, tssiminfo_t *ptsim, tstilecomb_t *ptc_2bsplit, size_t temperature, size_t idx1, size_t idx2)
{
    size_t i;
    size_t j;
    size_t k;
    size_t idx_tile;

    sortedlist_t *pitem1;
    sortedlist_t *pitem2;
    sortedlist_t *pitem_tmp;
    size_t cnt_glue;
    size_t glue_cur;
    size_t glue_alien;

    assert (NULL != pbmplist_subst);
    if (idx1 >= ma_size (pbmplist_subst)) {
        return -1;
    }
    if (idx2 >= ma_size (pbmplist_subst)) {
        return -1;
    }

    pitem1 = (sortedlist_t *)ma_data_lock (pbmplist_subst, idx1);
    pitem2 = (sortedlist_t *)ma_data_lock (pbmplist_subst, idx2);

    // 使用 min (slist_size (pitem1), slist_size (pitem2)) 减少判断
    if (slist_size (pitem1) > slist_size (pitem2)) {
        pitem_tmp = pitem1;
        pitem1 = pitem2;
        pitem2 = pitem_tmp;
    }
    cnt_glue = 0;
    for (i = 0; i < slist_size (pitem1); i ++) {
        // get the tile information from ptc_2bsplit
        slist_data (pitem1, i, &j);
        // check the 4 edges of the tile
        for (k = 0; k < 4; k ++) {
            tstilecombitem_t *ptsi;
            tstilecombitem_t tsci;
            tsposition_t tspos;
            // get the coordinate of tile which is alien to this tile
            if (slist_data (&(ptc_2bsplit->tbuf), j, &tsci) < 0) {
                assert (0);
            }
            if (tspos_step2dir (&tsci, k) < 0) {
                // out of range (x or y < 0)
                continue;
            }
            if (slist_find (&(ptc_2bsplit->tbuf), &tspos, NULL, &idx_tile) < 0) {
                // no such tile in this position, skip
                continue;
            }
            if (slist_find (pitem2, &idx_tile, NULL, NULL) < 0) {
                // the tile is not belong to pitem2, skip
                continue;
            }
            // check the glue function
            ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), j);
            glue_cur   = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)k, 0);
            slist_data_unlock (&(ptc_2bsplit->tbuf), j);
            ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), idx_tile);
            glue_alien = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)(GET_OPPOSITE((int)k)), 0);
            slist_data_unlock (&(ptc_2bsplit->tbuf), idx_tile);

            if (GLUE_CAN_ADHERE (glue_cur, glue_alien)) {
                // add the glue
                cnt_glue += ptsim->pgluevec[glue_cur];
            }
        }
    }

    ma_data_unlock (pbmplist_subst, idx1);
    ma_data_unlock (pbmplist_subst, idx2);
    if (cnt_glue >= temperature) {
        return 1;
    }
    return 0;
}

// merge two sub-supertiles to a new one, and delete old two & put the new one to the end of list
static int
stsub_merge2new (memarr_t *pbmplist_subst, size_t idx1, size_t idx2)
{
    sortedlist_t *pitem1;
    sortedlist_t *pitem2;
    sortedlist_t *pitem_new;
    size_t idx_new; // the index of the sub-supertile in the pbmplist_subst
    size_t i;
    size_t idx_tile; // the index of the tile in ptc_2bsplit

    assert (NULL != pbmplist_subst);
    if (idx1 >= ma_size (pbmplist_subst)) {
        return -1;
    }
    if (idx2 >= ma_size (pbmplist_subst)) {
        return -1;
    }

    pitem1 = (sortedlist_t *)ma_data_lock (pbmplist_subst, idx1);
    pitem2 = (sortedlist_t *)ma_data_lock (pbmplist_subst, idx2);
    // merge
    idx_new = ma_inc (pbmplist_subst);
    pitem_new = (sortedlist_t *)ma_data_lock (pbmplist_subst, idx_new);
    slist_init (pitem_new, sizeof (size_t), NULL, slist_cb_comp_sizet, slist_cb_swap_sizet);

    for (i = 0; i < slist_size (pitem1); i ++) {
        slist_data (pitem1, i, &idx_tile);
        slist_store (pitem_new, &idx_tile);
    }
    for (i = 0; i < slist_size (pitem2); i ++) {
        slist_data (pitem1, i, &idx_tile);
        slist_store (pitem_new, &idx_tile);
    }

    // delete the old two sub-supertiles
    slist_clear (pitem1, NULL);
    slist_clear (pitem2, NULL);
    ma_data_unlock (pbmplist_subst, idx_new);
    ma_data_unlock (pbmplist_subst, idx1);
    ma_data_unlock (pbmplist_subst, idx2);
    ma_rmdata (pbmplist_subst, idx1, NULL);
    ma_rmdata (pbmplist_subst, idx2, NULL);
    return 0;
}

/*
  tstc_split0(): find the list of the spliting
  use the method of bottom-to-top
  ptc_2bsplit:
  ptsim:
  temperature:
  plist_splited: the list of spliting result (tstilecomb_t), the number of this type supertile stored in .id
 */
int
tstc_split0 (tstilecomb_t *ptc_2bsplit, tssiminfo_t *ptsim, size_t temperature, memarr_t *plist_splited)
{
    // Exist problems：
    //   1) if tile around by a big supertile, can this tile be splitted?
    //   2) Four tile glued each other and the glues between then is 1 and the temp=2, can this be splitted?
    // 1. find out all of the sub-supetiles
    //   1) record all of the tile's positions of the supertile to bitmap: BMPLEFT
    //   create one stack STKCUR
    //   2) for each of the next position of the BMPLEFT do
    //     assert(STKCUR.size == 0);
    //     2.1) create one sub-supertile: BMPCUR (new bitmap with cleared)
    //     2.2) add the position both to STKCUR & BMPCUR; clear the position in the BMPLEFT
    //     2.3) for each of the item in STKCUR, push out one item, check the tile alien to each of the 4 edges
    //         2.3.1) if the tile is at the BMPCUR, next
    //         2.3.2) if the tile is not at the BMPLEFT, next
    //         2.3.3) if the glue function between these two tiles less than the temperature, next
    //         2.3.4) add the new position both to STKCUR & BMPCUR; clear the position in the BMPLEFT
    //     2.4) store the BMPCUR to a temp list: bmplist_subst (bitmap list of sub-supertiles)
    // 2. combine these sub-supertiles whose sum of the glue functions equal or large than temperature.
    //   for (i = 0; i < bmplist_subst.size() - 1; i ++) {
    //       for (j = i + 1; j < bmplist_subst.size(); j ++) {
    //           if (cancombine (bmplist_subst[i], bmplist_subst[j])) {
    //               // combined during the test one by one (顺序检测时发生合并)
    //               combine bmplist_subst[i], bmplist_subst[j] to bmplist_subst[bmplist_subst.size()]
    //               delete bmplist_subst[i] & bmplist_subst[j]
    //               prelast = i; // the sequence number of the left supertile, when combination occured (记录发生合并的左边(序号在前面的)supertile序号)
    //               for (k = prelast; k > 0; k --) {
    //                   // backtrace detect (回溯检测)
    //                  lastidx = bmplist_subst.size()-1
    //                   if (cancombine (bmplist_subst[lastidx], bmplist_subst[k-1])) {
    //                       combine bmplist_subst[lastidx], bmplist_subst[k-1] to bmplist_subst[bmplist_subst.size()]
    //                       delete bmplist_subst[lastidx] & bmplist_subst[k-1]
    //                       prelast --; // The last position should minus 1 because the merging in front of it. (前面的又发生了合并，则最后的位置需要向前挪一个位置)
    //                       k = prelast; // k is resetted to the position of merging of sequence testing (k 被重置为先前顺序检测时发生合并的位置)
    //                   }
    //               }
    //               i = prelast - 1; // the next value of i = prelast, but i will be add by one in the for(...), so here only let i = prelast - 1
    //               j = i; // in fact that the next value of j = i + 1, but j will be add by one in the for(...), so here only let j = i;
    //           }
    //       }
    //   }

    size_t i;
    size_t j;
    size_t k;
    size_t idx_tile;
    size_t prelast;
    size_t glue_cur;
    size_t glue_alien;
    unsigned char * pbmpleft;
    size_t num_pos; // the number of the tiles in the supertile
    stk_t posstack;
    // sub-supertile 列表中每项sub-supertile存储了每个点在ptc_2bsplit里的索引
    // sub-supertile 列表为malist, sub-supertile 为 slist
    memarr_t bmplist_subst; // sub-supertile 列表
    sortedlist_t *pitem;    // sub-supertile 存储类型为 size_t

    num_pos = slist_size (&(ptc_2bsplit->tbuf));
    pbmpleft = bm_create (num_pos);
    if (NULL == pbmpleft) {
        return -1;
    }
    bm_setall (pbmpleft, num_pos);
    ma_init (&bmplist_subst, sizeof (size_t));
    st_init (&posstack, sizeof (size_t));
    for (i = 0; i < num_pos; i ++) {
        if (1 != bm_is_set (pbmpleft, i)) {
            // the position is processed, skip
            continue;
        }
        // clear the pos in bmpleft
        bm_clr (pbmpleft, i);
        // create a new sub-supertile and store the first tile to it
        j = ma_inc (&bmplist_subst);
        pitem = (sortedlist_t *)ma_data_lock (&bmplist_subst, j);
        slist_init (pitem, sizeof (size_t), NULL, slist_cb_comp_sizet, slist_cb_swap_sizet);
        slist_store (pitem, &i);
        ma_data_unlock (&bmplist_subst, j);
        // push the current pos to stack
        st_push (&posstack, &i);
        for (; st_size (&posstack) > 0;) {
            st_pop (&posstack, &j);
            // check the 4 edges of the tile
            for (k = 0; k < 4; k ++) {
                tstilecombitem_t *ptsi;
                tstilecombitem_t tsci;
                tsposition_t tspos;
                // get the coordinate of tile which is alien to this tile
                if (slist_data (&(ptc_2bsplit->tbuf), j, &tsci) < 0) {
                    assert (0);
                }
                if (tspos_step2dir (&tsci, k) < 0) {
                    // out of range (x or y < 0)
                    continue;
                }
                if (slist_find (&(ptc_2bsplit->tbuf), &tspos, NULL, &idx_tile) < 0) {
                    // no such tile in this position, skip
                    continue;
                }
                if (0 == bm_is_set (pbmpleft, idx_tile)) {
                    // is cleared, skip
                    continue;
                }
                /* this is recorded in pbmpleft
                if (slist_find (&(pitem->tbuf), &idx_tile, NULL, NULL) >= 0) {
                    // is in current sub-supertile, skip
                    continue;
                }*/
                // check the glue function
                ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), j);
                glue_cur   = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)k, 0);
                slist_data_unlock (&(ptc_2bsplit->tbuf), j);
                ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), idx_tile);
                glue_alien = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi, (int)(GET_OPPOSITE((int)k)), 0);
                slist_data_unlock (&(ptc_2bsplit->tbuf), idx_tile);

                if (GLUE_CAN_ADHERE (glue_cur, glue_alien)) {
                    if (glue_cur > temperature) {
                        // absolutly could be combined
                        // store the pos to the sub-supertile
                        st_push (&posstack, &idx_tile);
                        slist_store (pitem, &idx_tile);
                    }
                }
            }
        }
        assert (st_size (&posstack) < 1);
    }
    assert (st_size (&posstack) < 1);
    for (i = 0; i < ma_size (&bmplist_subst) - 1; i ++) {
        for (j = i + 1; j < ma_size (&bmplist_subst); j ++) {
            if (stsub_cancombine (&bmplist_subst, ptsim, ptc_2bsplit, temperature, i, j) > 0) {
                // the sub-supertile recorded by the bmplist_subst[i] can combine
                // with the sub-supertile recorded by the bmplist_subst[j]
                // 顺序检测时发生合并
                stsub_merge2new (&bmplist_subst, i, j);
                prelast = i; // 记录发生合并的左边(序号在前面的)supertile序号
                for (k = prelast; k > 0; k --) {
                    // backtrace detect (回溯检测)
                    if (stsub_cancombine (&bmplist_subst, ptsim, ptc_2bsplit, temperature, ma_size (&bmplist_subst) - 1, k - 1) > 0) {
                        stsub_merge2new (&bmplist_subst, ma_size (&bmplist_subst) - 1, k - 1);
                        prelast --; // The last position should minus 1 because the merging in front of it. (前面的又发生了合并，则最后的位置需要向前挪一个位置)
                        k = prelast; // k is resetted to the position of merging of sequence testing (k 被重置为先前顺序检测时发生合并的位置)
                    }
                }
                i = prelast - 1; // the next value of i = prelast, but i will be add by one in the for(...), so here only let i = prelast - 1
                j = i; // in fact that the next value of j = i + 1, but j will be add by one in the for(...), so here only let j = i;
            }
        }
    }
    for (i = 0; i < ma_size (&bmplist_subst); i ++) {
        tstilecomb_t *ptsc;
        // save the result to plist_splited
        pitem = (sortedlist_t *)ma_data_lock (&bmplist_subst, i);
        k = ma_inc (plist_splited);
        ptsc = (tstilecomb_t *)ma_data_lock (plist_splited, k);
        for (j = 0; j < slist_size (pitem); j ++) {
            tstilecombitem_t *ptsi;
            slist_data (pitem, j, &idx_tile);
            ptsi = (tstilecombitem_t *) slist_data_lock (&(ptc_2bsplit->tbuf), idx_tile);
            tstc_additem (ptsc, ptsi);
            slist_data_unlock (&(ptc_2bsplit->tbuf), idx_tile);
        }
        ptsc->id = 1;
        ma_data_unlock (plist_splited, k);
        ma_data_unlock (&bmplist_subst, i);
    }

    return 0;
}

#else /* 0 */
#include <igraph/igraph.h>

/*2D=2;3D=3*/
// 2D: for the matrix of tiles, the edges of the supertile can be get from the directions ORI_NORTH and ORI_EAST of each tile.
static int g_dir_split_test[] = {
    ORI_NORTH,
    ORI_EAST,
#if USE_THREEDIMENTION
    ORI_BACK,
#endif
};

/*
  tstc_split(): find the list of the spliting
  use the method of min-cut of graph

  ptc_2bsplit:
  ptsim:
  temperature:
  plist_splited: the list of spliting result (tstilecomb_t), the tstilecomb_t.id is the number of this type of tile
 */
int
tstc_split (tstilecomb_t *ptc_2bsplit, tssiminfo_t *ptsim, size_t temperature, memarr_t *plist_splited)
{
    int ret_func = 0;
    int ret;
    igraph_t g;
    size_t num_pos;
    size_t i;
    size_t j;
    size_t k;
    size_t m;
    size_t idx;
    size_t cnt_edges;
    tstilecombitem_t curitem;
    tstilecombitem_t pinitem;

    int glue_base;
    int glue_test;
    tstilecomb_t *pttcomb_cur;
    tstilecomb_t *pttcomb_new;

    igraph_real_t val_mincut;
    igraph_vector_t vedges;
    igraph_vector_t vweights;
    igraph_vector_t vpart1;
    igraph_vector_t vpart2;
    igraph_vector_t vcut;

    pttcomb_cur = ptc_2bsplit;

    tstc_chkassert (pttcomb_cur);
    // get the number of the tiles in the supertile
    num_pos = slist_size (&(pttcomb_cur->tbuf));
    if (num_pos < 2) {
        // do not need to calculate
        assert (num_pos == 1);
        return 0;
    }
    /*if (num_pos < 3) {
        assert (num_pos == 2);
        // test the glues between two tiles
        return ?;
    }*/
    // for each of the current items in list, do:
    //   calculate the min cut of an un-directed graph
    for (k = 0; ;) {

        igraph_vector_init (&vedges,   num_pos * 2);
        igraph_vector_init (&vweights, num_pos);
        igraph_vector_resize (&vedges, 0);
        igraph_vector_resize (&vweights, 0);

        cnt_edges = 0;
        // int igraph_add_vertices (igraph_t *graph, igraph_integer_t nv, void *attr);
        // add the edges to the graph
        for (i = 0; i < num_pos; i ++) {
            // add the edges: the east and south of each tile
            slist_data (&(pttcomb_cur->tbuf), i, &curitem);
            for (j = 0; j < NUM_TYPE (g_dir_split_test, int); j ++) {
                memmove (&pinitem, &curitem, sizeof (curitem));
                tspos_step2dir (&(pinitem.pos), g_dir_split_test[j]);
                assert (pinitem.pos.x >= curitem.pos.x);
                assert (pinitem.pos.y >= curitem.pos.y);
#if USE_THREEDIMENTION
                assert (pinitem.pos.z >= curitem.pos.z);
#endif
                ret = slist_find (&(pttcomb_cur->tbuf), &pinitem, NULL, &idx);
                if (ret >= 0) {
                    // here (idx > i) means that the link is non-direct
                    assert (idx > i);
                    // found
                    slist_data (&(pttcomb_cur->tbuf), idx, &pinitem);
                    // get the glue index: glue_base & glue_test
                    glue_base = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, &curitem, g_dir_split_test[j], 0);
                    glue_test = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, &pinitem, GET_OPPOSITE(g_dir_split_test[j]), 0);
                    if (GLUE_CAN_ADHERE (glue_test, glue_base)) {
                        //igraph_add_edge (&g, i, idx);
                        igraph_vector_resize (&vedges, cnt_edges * 2 + 2);
                        igraph_vector_set (&vedges, cnt_edges * 2,     i);
                        igraph_vector_set (&vedges, cnt_edges * 2 + 1, idx);
                        // add weight:
                        igraph_vector_resize (&vweights, cnt_edges + 1);
                        // get the glue value: ptsim->pgluevec[glue_base]
                        //VECTOR(vweights)[cnt_edges] = ptsim->pgluevec[glue_base];
                        igraph_vector_set (&vweights, cnt_edges, ptsim->pgluevec[glue_base]);
                        cnt_edges ++;
                    }
                }
            }
        }
        assert (igraph_vector_size (&vedges) == cnt_edges * 2);
        assert (igraph_vector_size (&vweights) == cnt_edges);

        // int igraph_empty (igraph_t *graph, igraph_integer_t n, igraph_bool_t directed);
        // ret = igraph_empty (&g, num_pos, 0/* 0: not directed */);
        ret = igraph_create (&g, &vedges, num_pos, 0/* 0: not directed */);
        if (ret == IGRAPH_EINVAL) {
            assert (0);
            ret_func = -1;
            igraph_vector_destroy (&vedges);
            igraph_vector_destroy (&vweights);
            break;
        }

        igraph_vector_init (&vpart1, 0);
        igraph_vector_init (&vpart2, 0);
        igraph_vector_init (&vcut, 0);

        igraph_mincut (&g, &val_mincut, &vpart1, &vpart2, NULL, &vweights);

        if (val_mincut >= temperature) {
            // the supertile segment will not split in this temperature
            k ++;
        } else /*if (value < temperature) */{
            // the sum of glue strenth less than temperature, the supertile will break away at this line
            assert (igraph_vector_size (&vpart1) > 0);
            assert (igraph_vector_size (&vpart2) > 0);
            assert (num_pos == igraph_vector_size (&vpart1) + igraph_vector_size (&vpart2));

            // 1. the second part of the supertile segment
            // copy the 2nd part segment to a new location of the supertile segment list (SSL)
            igraph_vector_sort (&vpart2);

            // 同一个plist_splited申请内存可能导致其基址被改变，所以重新获取当前 pttcomb_cur
            if (pttcomb_cur != ptc_2bsplit) {
                ma_data_unlock (plist_splited, k);
            }
            idx = ma_inc (plist_splited);
            pttcomb_new = (tstilecomb_t *) ma_data_lock (plist_splited, idx);
            if (pttcomb_cur != ptc_2bsplit) {
                pttcomb_cur = (tstilecomb_t *) ma_data_lock (plist_splited, k);
            }
            tstc_init (pttcomb_new);
            for (m = 0; m < igraph_vector_size (&vpart2); m ++) {
                slist_data (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart2, m), &curitem);
                tstc_additem (pttcomb_new, &curitem);
            }
            // nomalize the pttcomb_new: let the left bottom of the supertile segment move to the (0,0) position
            tstc_nomalize (pttcomb_new);
            // 对生成的 supertile 调整到其最低值
            if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                tstc_adjust_lower (pttcomb_new, ptsim->flg_is2d);
#else
                tstc_adjust_lower (pttcomb_new, 1);
#endif
            }
            tstc_chkassert (pttcomb_new);
            pttcomb_new->id = 1;
            assert (slist_size (&(pttcomb_new->tbuf)) > 0);
            assert (slist_size (&(pttcomb_new->tbuf)) == igraph_vector_size (&vpart2));
            ma_data_unlock (plist_splited, idx);

            // 2. the first part of the supertile segment
            igraph_vector_sort (&vpart1);
            if (pttcomb_cur == ptc_2bsplit) {
                // copy the other half to the list SSL
                assert (ma_size (plist_splited) == 1);
                idx = ma_inc (plist_splited);
                pttcomb_new = (tstilecomb_t *) ma_data_lock (plist_splited, idx);
                tstc_init (pttcomb_new);
                for (m = 0; m < igraph_vector_size (&vpart1); m ++) {
                    slist_data (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart1, m), &curitem);
                    tstc_additem (pttcomb_new, &curitem);
                }
                // nomalize the pttcomb_new: let the left bottom of the supertile segment move to the (0,0) position
                tstc_nomalize (pttcomb_new);
                // 对生成的 supertile 调整到其最低值
                if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                    tstc_adjust_lower (pttcomb_new, ptsim->flg_is2d);
#else
                    tstc_adjust_lower (pttcomb_new, 1);
#endif
                }
                tstc_chkassert (pttcomb_new);
                pttcomb_new->id = 1;
                assert (slist_size (&(pttcomb_new->tbuf)) > 0);
                assert (slist_size (&(pttcomb_new->tbuf)) == igraph_vector_size (&vpart1));
                ma_data_unlock (plist_splited, idx);
            } else {
                // remove the items denoted by vect2 in the current SSL node reversely
                for (m = igraph_vector_size (&vpart2); m > 0; m --) {
                    assert (slist_size (&(pttcomb_cur->tbuf)) > 1);
                    slist_rmdata (&(pttcomb_cur->tbuf), (int)igraph_vector_e (&vpart2, m - 1), NULL);
                    assert (slist_size (&(pttcomb_cur->tbuf)) > 0);
                }
                // nomalize the pttcomb_cur: let the left bottom of the supertile segment move to the (0,0) position
                tstc_nomalize (pttcomb_cur);
                // 对生成的 supertile 调整到其最低值
                if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                    tstc_adjust_lower (pttcomb_new, ptsim->flg_is2d);
#else
                    tstc_adjust_lower (pttcomb_new, 1);
#endif
                }
                tstc_chkassert (pttcomb_cur);
                assert (slist_size (&(pttcomb_cur->tbuf)) > 0);
                assert (slist_size (&(pttcomb_cur->tbuf)) == igraph_vector_size (&vpart1));
                // free pttcomb_cur
                ma_data_unlock (plist_splited, k);
            }
            if (k >= ma_size (plist_splited)) {
                // reach to the end of the list of split segments
                break;
            }
            // next supertile segment to be processed
            pttcomb_cur = (tstilecomb_t *)ma_data_lock (plist_splited, k);
            assert (NULL != pttcomb_cur);
            tstc_chkassert (pttcomb_cur);

            // skip the supertile in which the number of tiles less than 2( == 1).
            // get the number of the tiles in the supertile
            num_pos = slist_size (&(pttcomb_cur->tbuf));
            for (; (num_pos < 2) && (k < ma_size (plist_splited)); ) {
                // do not need to calculate
                assert (num_pos == 1);
                ma_data_unlock (plist_splited, k);
                k ++;
                if (k >= ma_size (plist_splited)) {
                    break;
                }
                pttcomb_cur = (tstilecomb_t *)ma_data_lock (plist_splited, k);
                assert (NULL != pttcomb_cur);
                tstc_chkassert (pttcomb_cur);
                num_pos = slist_size (&(pttcomb_cur->tbuf));
            }
            if (k >= ma_size (plist_splited)) {
                // reach to the end of the list of split segments
                break;
            }
        }
        igraph_vector_destroy (&vedges);
        igraph_vector_destroy (&vweights);
        igraph_vector_destroy (&vpart1);
        igraph_vector_destroy (&vpart2);
        igraph_vector_destroy (&vcut);
        igraph_destroy (&g);
        if (k >= ma_size (plist_splited)) {
            // reach to the end of the list of split segments
            break;
        }
    }
    return ret_func;
}
#endif /* 0 */

static int
slist_cb_comp_heter_addst (void *userdata, void * data_pin, void * data2)
{
    int ret;
    tssiminfo_t *ptsim = (tssiminfo_t *)userdata;
    tstilecomb_t *ptc_data_pin = (tstilecomb_t *)data_pin;
    tstilecomb_t *ptc_list;
    size_t idx;
    assert (NULL != ptsim);
    assert (NULL != data_pin);
    assert (NULL != data2);
    idx = *((size_t *)data2);
    ptc_list = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx);
    assert (NULL != ptc_list);
    ret = tstc_compare_full (ptc_data_pin, ptc_list, ptsim->flg_norotate);
    ma_data_unlock (&(ptsim->tilelist), idx);
    return ret;
}

// 改变当前 idx 的个数。更新对应的 ptsim->countlist
// 如果该个数减到0，且 flg_shrink_record，则还要从 ptsim->tilelstidx 删除该idx, 并将该 idx 加入到 ptsim->freedlist
// idx_unsorted: ptsim->countlist 或 ptsim->tilelist 中的索引
int
ts_update_count (tssiminfo_t *ptsim, size_t idx_unsorted, size_t count)
{
    char flg_found;
    size_t idx_rd;
    size_t i;
    int ret;
    tstilecomb_t * ptc;
    assert (NULL != ptsim);

    // update the count
    ret = ma_replace (&(ptsim->countlist), idx_unsorted, &count);
    if (ret < 0) {
        return -1;
    }
    if (ptsim->flg_shrink_record) {
        if (count < 1) {
            // remove all of the data of supertile
            ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_unsorted);
            tstc_reset (ptc);
            ma_data_unlock (&(ptsim->tilelist), idx_unsorted);
            // remove the index from the tilelstidx
            flg_found = 0;
            for (i = 0; i < slist_size (&(ptsim->tilelstidx)); i ++) {
                slist_data (&(ptsim->tilelstidx), i, &idx_rd);
                if (idx_rd == idx_unsorted) {
                    slist_rmdata (&(ptsim->tilelstidx), i, NULL);
                    flg_found = 1;
                    break;
                }
            }
            if (flg_found == 0) {
                // ignore this error, because this function may be used during the adding new supertile while the index is not updated to the ptsim->tilelstidx
            }
            // add the index to the freedlist
            ret = slist_store (&(ptsim->freedlist), &idx_unsorted);
        }
    }
    return 0;
}

// 在 tilelist 中申请新的空间。如果 ptsim->flg_shrink_record 被置位，则从 ptsim->freedlist 查找 count 为0的空余位置。
// pret_idx_unsorted: ptsim->countlist 或 ptsim->tilelist 中的索引
int
ts_tilelist_st_new (tssiminfo_t *ptsim, size_t *pret_idx_unsorted, tstilecomb_t **pret_ptc)
{
    size_t cnt;
    int ret;
    size_t i;
    tstilecomb_t * ptc;
    char flg_create = 1;
    if (ptsim->flg_shrink_record) {
        // 查找缓冲freedlist中是否有空余的
        if (slist_size (&(ptsim->freedlist)) > 0) {
            flg_create = 0;
            if (slist_rmdata (&(ptsim->freedlist), 0, pret_idx_unsorted) < 0) {
                flg_create = 1;
            } else {
                // birth
                cnt = 0;
                ret = ma_replace (&(ptsim->countlist), *pret_idx_unsorted, &cnt);
                ret = ma_replace (&(ptsim->birthlist), *pret_idx_unsorted, &(ptsim->year_current));
                if (NULL != pret_ptc) {
                    *pret_ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), *pret_idx_unsorted);
                }
                return 0;
            }
        }
    }
    if (flg_create) { for (;;) {
        i = ma_inc (&(ptsim->tilelist));
        if (i < 0) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in ma_inc()");
            ret = -1;
            break;
        }
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
        if (NULL == ptc) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in ma_data_lock()");
            ret = -1;
            break;
        }
        if (tstc_init (ptc) < 0) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in tstc_init()");
            ma_data_unlock (&(ptsim->tilelist), i);
            ma_rmdata (&(ptsim->tilelist), i, NULL);
            ret = -1;
            break;
        }
        cnt = 0;
        ret = ma_replace (&(ptsim->countlist), i, &cnt);
        ret = ma_replace (&(ptsim->birthlist), i, &(ptsim->year_current));
        if (NULL != pret_idx_unsorted) {
            *pret_idx_unsorted = i;
        }
        ret = 0;
        if (NULL != pret_ptc) {
            *pret_ptc = ptc;
            break;
        }
        ma_data_unlock (&(ptsim->tilelist), i);
    }}
    return ret;
}

// 删除由 ts_tilelist_st_new 刚刚申请的 supertile 空间
// idx_unsorted: ptsim->countlist 或 ptsim->tilelist 中的索引
static int
ts_tilelist_st_delete (tssiminfo_t *ptsim, size_t idx_unsorted)
{
    int ret;
    size_t cnt;
    tstilecomb_t * ptc;

    if (ptsim->flg_shrink_record) {
        if (ts_update_count (ptsim, idx_unsorted, 0) < 0) {
            return -1;
        } else {
            cnt = 0;
            ret = ma_replace (&(ptsim->birthlist), idx_unsorted, &cnt);
        }
    } else {
        assert (idx_unsorted + 1 == ma_size (&(ptsim->tilelist)));
        assert (idx_unsorted + 1 == ma_size (&(ptsim->countlist)));
        assert (idx_unsorted + 1 == ma_size (&(ptsim->birthlist)));
        // the number of the supertile is 0:
        ma_data (&(ptsim->countlist), idx_unsorted, &cnt);
        assert (cnt == 0);
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_unsorted);
        tstc_clear (ptc);
        ma_data_unlock (&(ptsim->tilelist), idx_unsorted);
        ma_rmdata (&(ptsim->tilelist), idx_unsorted, NULL);
        ma_rmdata (&(ptsim->countlist), idx_unsorted, NULL);
        ma_rmdata (&(ptsim->birthlist), idx_unsorted, NULL);
    }
    return 0;
}

// add a supertile to the current list: search the same shape, if exist, then increase the counter; if not, create one record
// the number of this type of supertile to be added is stored in the ptc_new_2b_transfered->id !
// return -1: error
// return >= 0: add ok, ret==1:new item, ret==0:just update the counter
// the updated number of the supertile is stored in pret_newcount
// the index of the supertile in the list ptsim->tilelist is stored in pret_newid
int
ts_add_supertile (tssiminfo_t *ptsim, tstilecomb_t *ptc_new_2b_transfered, size_t *pret_newid, size_t *pret_newcount)
{
    size_t i;
    int ret;
    char flg_newitem;
    tstilecomb_t * ptc;
    size_t idxinc;

    assert (ptc_new_2b_transfered->id > 0);

    // FIXME: before searching, the ptc_new_2b_transfered should be rotated to the max(min) value.
    // ...

    // here's fix: improve the performance
    ret = slist_find (&(ptsim->tilelstidx), ptc_new_2b_transfered, slist_cb_comp_heter_addst, &idxinc);
    if (ret >= 0) {
        // get the real idx of the malist ptsim->tilelist
        TS_UNSORTED_IDX (ptsim, idxinc, &i);
    }

    flg_newitem = 0;
    if (ret < 0) { for (;;) {
        // not found the same tile.
        // the supertile is inserted in the unsorted list and the index is stored in sorted list.
        // 比较两个 supertile, 首先确定比较顺序，确保 (a1, a2) 或 (a2, a1) 的顺序出来的结果相同
        // 比较 (x,y)
        flg_newitem = 1;

        ptc = NULL;
        if (ts_tilelist_st_new (ptsim, &i, &ptc) < 0) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in ts_tilelist_st_new()");
            ret = -1;
            break;
        }

        assert (NULL != ptc);
        idxinc = ptc_new_2b_transfered->id;
        ptc_new_2b_transfered->id = 0;
        ret = tstc_transfer (ptc, ptc_new_2b_transfered);
        ma_data_unlock (&(ptsim->tilelist), i);
        if (ret < 0) {
            ts_tilelist_st_delete (ptsim, i);
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in tstc_transfer()");
            ret = -1;
            break;
        }

        // adjust the data
        if (ts_update_count (ptsim, i, idxinc) < 0) {
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in ts_update_count()");
            ts_tilelist_st_delete (ptsim, i);
            ret = -1;
            break;
        }

        // add new index
        ret = slist_store (&(ptsim->tilelstidx), &i);
        assert (ret >= 0);
        if (ret < 0) {
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in slist_store()");
            ts_tilelist_st_delete (ptsim, i);
            break;
        }

        assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->countlist)));
        assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->birthlist)));

        //ret = ma_data (&(ptsim->birthlist), idxinc, &i);
        //assert (ret >= 0);
        //assert (i == ptsim->year_current);

    break;}} else {
        // there exist the same supertile in the list.
        // update the information
        ret = ma_data (&(ptsim->countlist), i, &idxinc);
        assert (ret >= 0);
        idxinc += ptc_new_2b_transfered->id;
        ptc_new_2b_transfered->id = 0;
        ret = ma_replace (&(ptsim->countlist), i, &idxinc);
        assert (ret >= 0);
    }
    if (ret >= 0) {
        if (pret_newcount) {
            *pret_newcount = idxinc;
        }
        if (pret_newid) {
            *pret_newid = i;
        }
        ret = 0;
        if (flg_newitem) {
            ret = 1;
        }
    }

    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->countlist)));
    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->birthlist)));

    return ret;
}

size_t
ts_random_loopyear (tssiminfo_t *ptsim)
{
    size_t num = 0;

    for (; num < 1; ) {
        if (ptsim->steps_1year_max) {
            num = my_irrand (ptsim->steps_1year_max - ptsim->steps_1year_min);
        } else {
            num = my_irand ();
        }

    }
    num += ptsim->steps_1year_min;

    return num;
}

size_t
ts_get_total_supertiles (tssiminfo_t *ptsim)
{
    size_t i;
    size_t inc;
    int ret;
    size_t num_total = 0;
    for (i = 0; i < ma_size (&(ptsim->countlist)); i ++) {
        ret = ma_data (&(ptsim->countlist), i, &inc);
        assert (ret >= 0);
        num_total += inc;
    }
    return num_total;
}
#if DEBUG

// plist_stile_mincut: the list of tstilecomb_t, store the result of mincut
int
tstclist_tcomb_is_equal (memarr_t *plist_stile_mincut, memarr_t *plist_stile_mincut_2d, char flg_norotate)
{
    int ret = 1;
    size_t k;
    size_t j;
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;

    assert (NULL != plist_stile_mincut);
    assert (NULL != plist_stile_mincut_2d);

    // compare list_stile_mincut & list_stile_mincut_2d
    if (ma_size (plist_stile_mincut) != ma_size (plist_stile_mincut_2d)) {
        // error
        return 0;
    }
    for (j = 0; j < ma_size (plist_stile_mincut); j ++) {
        ptc_base = (tstilecomb_t *)ma_data_lock (plist_stile_mincut, j);
        for (k = 0; k < ma_size (plist_stile_mincut_2d); k ++) {
            ptc_test = (tstilecomb_t *)ma_data_lock (plist_stile_mincut_2d, k);
            if (tstc_is_equal (ptc_test, ptc_base, flg_norotate)) {
                ma_data_unlock (plist_stile_mincut_2d, k);
                break;
            }
            ma_data_unlock (plist_stile_mincut_2d, k);
        }
        ma_data_unlock (plist_stile_mincut, j);
        if (k >= ma_size (plist_stile_mincut_2d)) {
            // not found!
            ret = 0;
            break;
        }
    }
    return ret;
}

// plist_points_ok: the list of tsstpos_t
int
tstclist_pos_is_equal (memarr_t *plist_points_ok, memarr_t *plist_points_ok_2d)
{
    int ret = 1;
    tsstpos_t *ptp_1;
    tsstpos_t *ptp_2;
    size_t i;
    size_t j;
    if (ma_size (plist_points_ok) != ma_size (plist_points_ok_2d)) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "size is not equal.");
        return 0;
    }
    for (i = 0; i < ma_size (plist_points_ok); i ++) {
        ptp_1 = (tsstpos_t *)ma_data_lock (plist_points_ok, i);
        for (j = 0; j < ma_size (plist_points_ok_2d); j ++) {
            ptp_2 = (tsstpos_t *)ma_data_lock (plist_points_ok_2d, j);
            if (0 == tstci_cmp (ptp_1, ptp_2)) {
                ma_data_unlock (plist_points_ok_2d, j);
                //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Found!");
                break;
            }
            ma_data_unlock (plist_points_ok_2d, j);
        }
        ma_data_unlock (plist_points_ok, i);
        if (j >= ma_size (plist_points_ok_2d)) {
            // not found
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Not found item");
            ret = 0;
            break;
        }
    }
    return ret;
}

#endif // DEBUG

/*
    通过每次选取两个supertile来尝试是否能够结合的方式模拟各个supertile间的结合过程。
    用户可以通过设置3个回调函数来控制模拟的过程：
    1. cb_getdata: 获取比较的两个supertile的索引值。可以通过不同种类的回调函数从不同来源得到最终选取的索引值。如随机、从xml数据文件中(可以用来测试)等。
    2. cb_selectone: 从传入的包括了所有可以放置的点的列表中选一个返回用作该次测试的两个supertile的摆放位置。如随机／固定读取预置（可以用来测试）等。
    3. cb_resultinfo: 通知结合的结果。如可以编写回调函数用来慢放模拟的过程、测试模拟系统是否工作正常等。
    4. cb_notifyfail: 通知连续结合测试失败的次数。每次结合成功，则该值为0
*/
void
tilesim_2htam (tssiminfo_t *ptsim, const char *name_sim)
{
    // 重复从列表中获取两个,根据占据百分比
    // 检查可结合性，如可以结合，则从队列中删除对应的两个，同时生成新的内容插入到队列中:需要检查是否已经存在相同.

    size_t num_oneyear; // the number of combinations of the supertiles in one loop (year)
                        // this value is changed during the simulation loop

    size_t num_total; // 所有tile 的个数
    size_t i;
    size_t j;
    size_t inc;
    int ret;
    int ret_chkresult;
    memarr_t list_points_ok; // 每次计算两个supertile组合的所有可用位置， tsstpos_t 类型的列表
    memarr_t list_stile_mincut; // the list of tstilecomb_t, store the result of mincut
#if USE_DBG_CHK_RESULT
    memarr_t list_points_ok_2d;
    memarr_t list_stile_mincut_2d;
#endif
    size_t cnt_fail; // 尝试失败的次数
    char flg_newitem; // 如果合并的是产生一个新的，则置为1，否则为0

    size_t idx;
    size_t idx_base;
    size_t idx_test;
    size_t num_base;
    size_t num_test;
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;

    tstilecomb_t tc0;
    size_t idxinc;
    tsstpos_t stpos;

    assert (NULL != ptsim);

    ma_init (&list_stile_mincut, sizeof (tstilecomb_t));
#if USE_DBG_CHK_RESULT
    ma_init (&list_stile_mincut_2d, sizeof (tstilecomb_t));
    ret = ma_init (&list_points_ok_2d, sizeof (tsstpos_t));
#endif
    ret = ma_init (&list_points_ok, sizeof (tsstpos_t));
    assert (ret >= 0);

    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "check split ...");
    // check the splitting of the supertiles under the current temperature
    // for each of the supertile, check if the supertile could be split or not.
    // if it can be split in this temperature, then processing splitting.
    // int tstc_split (tstilecomb_t *ptc_2bsplit, tssiminfo_t *ptsim, size_t temperature, memarr_t *plist_splited)
    for (i = 0; i < TS_NUMTYPE_SUPERTILE (ptsim); i ++) {
        TS_UNSORTED_IDX (ptsim, i, &idx);
        num_test = 0;
        ma_data (&(ptsim->countlist), idx, &num_test);
        if (num_test < 1) {
            continue;
        }

        ptc_test = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx);
        tstc_split (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut);

#if USE_DBG_CHK_RESULT
#if USE_THREEDIMENTION
        if (ptsim->flg_is2d)
#endif
        {
            ret = ma_rmalldata (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
            tstc_split_2d (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut_2d);
        }
#endif /* USE_DBG_CHK_RESULT */

        ma_data_unlock (&(ptsim->tilelist), idx);

#if USE_DBG_CHK_RESULT
#if USE_THREEDIMENTION
        if (ptsim->flg_is2d)
#endif
            assert (tstclist_tcomb_is_equal (&list_stile_mincut, &list_stile_mincut_2d, ptsim->flg_norotate));
#endif /* USE_DBG_CHK_RESULT */

        for (j = 0; j < ma_size (&list_stile_mincut); j ++) {
            ptc_base = (tstilecomb_t *)ma_data_lock (&list_stile_mincut, j);
            assert (NULL != ptc_base);
            // adjust the number of the supertile segments.
            assert (ptc_base->id > 0);
            ptc_base->id *= num_test;
            ts_add_supertile (ptsim, ptc_base, NULL, NULL);
            ma_data_unlock (&list_stile_mincut, j);
        }
        if (ma_size (&list_stile_mincut) > 0) {
            // the current supertile is splitted, set the number of the supertile to 0
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_INFO, "supertile splited, idx=%d ...", i);
            num_test = 0;
            ret = ts_update_count (ptsim, idx, num_test);
            assert (ret >= 0);
            if (ptsim->idx_last == idx) {
                ptsim->idx_last = -1;
            }
        }
        ret = ma_rmalldata (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
        assert (ret >= 0);
    }

    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->countlist)));
    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->birthlist)));

    num_total = ts_get_total_supertiles (ptsim);
    if (num_total < 2) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "total amount of supertiles(%d) < 2 ...", num_total);
        goto tssim_main_end;
    }
    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_INFO, "total amount of supertiles=%d ...", num_total);

    ma_rmalldata (&list_points_ok, NULL);
    cnt_fail = 0;
    num_oneyear = 0;
    // the simulation main loop
    for (ret_chkresult = 0; ret_chkresult >= 0; ) {

        if (ptsim->cb_getdata) {
            // 用户提供的选择两个测试 supertile 的索引
            ret = ptsim->cb_getdata (ptsim->userdata, &(ptsim->countlist), &(ptsim->year_current), &idx_base, &idx_test);
            if (ret < 1) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "cb_getdata() return < 1");
                break;
            }
            if ((ptsim->algorithm != TSIM_ALGO_2HATAM) && (ptsim->idx_last >= 0)) {
                if (ptsim->idx_last == idx_base) {
                    if (ptsim->idx_last == idx_test) {
                        // ignore this round
                        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "ignore this round");
                        continue;
                    }
                } else if (ptsim->idx_last != idx_test) {
                    //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "set test from %d to %d", idx_test, ptsim->idx_last);
                    idx_test = ptsim->idx_last;
                }
            }
            ret = ma_data (&(ptsim->countlist), idx_base, &num_base);
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "ma_data(idx_base(%d)) error", idx_base);
                continue;
            }
            assert (ret >= 0);
            ret = ma_data (&(ptsim->countlist), idx_test, &num_test);
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "ma_data(idx_test(%d)) error", idx_test);
                continue;
            }
            assert (ret >= 0);
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_INFO, "base=%d,test=%d,year=%d", idx_base, idx_test, ptsim->year_current);
        } else {
            // the default random selection of two index of the base and test
            if (num_oneyear < 1) {
                num_oneyear = ts_random_loopyear (ptsim);
                ptsim->year_current ++;
            }
            assert (num_oneyear > 0);
            num_oneyear --;
#if DEBUG
            idx = ma_size (&(ptsim->countlist)); // for debug
            idx_base = idx + 1; // for debug
#endif
            if ((ptsim->algorithm != TSIM_ALGO_2HATAM) && (ptsim->idx_last >= 0)) {
                idx_base = ptsim->idx_last;
                ret = ma_data (&(ptsim->countlist), idx_base, &num_base);
                assert (ret >= 0);
            } else {
            // select one tile random
            inc = my_irrand (num_total);
            for (i = 0; i < TS_NUMTYPE_SUPERTILE (ptsim); i ++) {
                TS_UNSORTED_IDX (ptsim, i, &idx);
                assert (idx < ma_size (&(ptsim->countlist)));

                ret = ma_data (&(ptsim->countlist), idx, &num_base);
                assert (ret >= 0);
                if (num_base >= inc) {
                    // get it
                    idx_base = idx;
                    break;
                }
                inc -= num_base;
            }
            assert (idx < ma_size (&(ptsim->countlist)));
            assert (idx == idx_base);
            }

            // select another one tile random
#if DEBUG
            idx = ma_size (&(ptsim->countlist)); // for debug
            idx_test = idx + 1; // for debug
#endif
{size_t maxinc = my_irrand (num_total);
            inc = 0;
            for (i = 0; i < TS_NUMTYPE_SUPERTILE (ptsim); i ++) {
                TS_UNSORTED_IDX (ptsim, i, &idx);
                assert (idx < ma_size (&(ptsim->countlist)));

                ret = ma_data (&(ptsim->countlist), idx, &num_test);
                assert (ret >= 0);
                //if (num_test >= inc) {
                if (inc + num_test >= maxinc) {
                    // get it
                    idx_test = idx;
                    break;
                }
                //inc -= num_test;
                inc += num_test;
            }
            assert (idx < ma_size (&(ptsim->countlist)));
            assert (idx == idx_test);
}
        }
        // 检查是否 idx_base, 和 idx_test 有效
        if (ptsim->flg_shrink_record) {
            if (slist_find (&(ptsim->freedlist), &idx_base, NULL, NULL) >= 0) {
                idx_base = idx_test;
                num_base = 1; // force skip this round
            } else  if (slist_find (&(ptsim->freedlist), &idx_test, NULL, NULL) >= 0) {
                idx_base = idx_test;
                num_base = 1; // force skip this round
            }
        }
        // check if the two items is the same and the number of item is
        // less than 2:
        if (idx_base == idx_test) {
            if ((num_base < 2) || ((ptsim->algorithm != TSIM_ALGO_2HATAM) && (ptsim->idx_last >= 0))) {
                // check the next loop
                cnt_fail ++;
                if (ptsim->cb_notifyfail) {
                    ret_chkresult = ptsim->cb_notifyfail (ptsim->userdata, cnt_fail);
                    if (ret_chkresult < 0) {
                        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Error in user's cb_notifyfail 1");
                        break;
                    }
                }
                continue;
            }
        }
        if ((num_base < 1) || (num_test < 1)) {
            cnt_fail ++;
            if (ptsim->cb_notifyfail) {
                ret_chkresult = ptsim->cb_notifyfail (ptsim->userdata, cnt_fail);
                if (ret_chkresult < 0) {
                    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Error in user's cb_notifyfail 2");
                    break;
                }
            }
            continue;
        }

        // get one tile from each of the selected group and
        // apply combination testing
        ptc_base = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_base);
        if (idx_test != idx_base) {
            ptc_test = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_test);
        } else {
            ptc_test = ptc_base;
        }
        assert (NULL != ptc_base);
        assert (NULL != ptc_test);

        if (slist_size (&(ptc_base->tbuf)) < slist_size (&(ptc_test->tbuf))) {
            // swap the two items
            // for stored in database
            tstilecomb_t *ptc_tmp;
            size_t tmp;
            tmp = idx_base;
            idx_base = idx_test;
            idx_test = tmp;
            tmp = num_base;
            num_base = num_test;
            num_test = tmp;
            ptc_tmp = ptc_base;
            ptc_base = ptc_test;
            ptc_test = ptc_tmp;
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "swap (base %d:num %d) and (test %d:num %d) for best perfomance.", idx_base, num_base, idx_test, num_test);
        }
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "check (base %d:num %d) + (test %d:num %d) at year=%d", idx_base, num_base, idx_test, num_test, ptsim->year_current);

#if 1

/* second, if the time for calculating the merge position are more than 1 second, then store it to db */
#define MAX_WAIT_TIME_MESHTEST 1

        ma_rmalldata (&list_points_ok, NULL);
        if (ptsim->cb_findmergepos) {
            time_t tstart;
            time_t tstop;
            ret = ptsim->cb_findmergepos (ptsim->userdata, idx_base, idx_test, ptsim->temperature, &list_points_ok);
            if (ret < 0) {
                // start timer ...
                tstart = time (NULL);
                assert ((ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature)
                    || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_godhand)
                    || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test2d_nature)
                    || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test2d_godhand)
                    );
#if DEBUG
                ret = ptsim->cb_mesh_test (ptsim, ptc_base, ptc_test, &list_points_ok);
#else
#if USE_THREEDIMENTION
                if (ptsim->flg_is2d) {
#endif
                    if (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature) {
                        ret = tstc_mesh_test2d_nature (ptsim, ptc_base, ptc_test, &list_points_ok);
                    } else {
                        ret = tstc_mesh_test2d_godhand (ptsim, ptc_base, ptc_test, &list_points_ok);
                    }
#if USE_THREEDIMENTION
                } else {
                    ret = ptsim->cb_mesh_test (ptsim, ptc_base, ptc_test, &list_points_ok);
                }
#endif
#endif

#if USE_DBG_CHK_RESULT
#if USE_THREEDIMENTION
                if (ptsim->flg_is2d)
#endif
                {
                    ma_rmalldata (&list_points_ok_2d, NULL);
                    if (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature) {
                        tstc_mesh_test2d_nature (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                        //tstc_mesh_test2d_godhand (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                    } else {
                        assert (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_godhand);
                        tstc_mesh_test2d_godhand (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                        //tstc_mesh_test2d_nature (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                    }
                    // compare the two list
                    assert (tstclist_pos_is_equal (&list_points_ok, &list_points_ok_2d));
                }
#endif /* USE_DBG_CHK_RESULT */

                // stop timer
                tstop = time (NULL);
                if (tstop > tstart + MAX_WAIT_TIME_MESHTEST) {
                    // store this info
                    if (ptsim->cb_storemergepos) {
                        ptsim->cb_storemergepos (ptsim->userdata, idx_base, idx_test, ptsim->temperature, &list_points_ok);
                   }
                }
            }
        }
        else
#endif
        {
            assert ((ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature)
                || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_godhand)
                || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test2d_nature)
                || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test2d_godhand)
                );
#if DEBUG
            ret = ptsim->cb_mesh_test (ptsim, ptc_base, ptc_test, &list_points_ok);
#else
#if USE_THREEDIMENTION
            if (ptsim->flg_is2d) {
#endif
                if (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature) {
                    ret = tstc_mesh_test2d_nature (ptsim, ptc_base, ptc_test, &list_points_ok);
                } else {
                    ret = tstc_mesh_test2d_godhand (ptsim, ptc_base, ptc_test, &list_points_ok);
                }
#if USE_THREEDIMENTION
            } else {
                ret = ptsim->cb_mesh_test (ptsim, ptc_base, ptc_test, &list_points_ok);
            }
#endif
#endif

#if USE_DBG_CHK_RESULT
#if USE_THREEDIMENTION
            if (ptsim->flg_is2d)
#endif
            {
                ma_rmalldata (&list_points_ok_2d, NULL);
                if (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature) {
                    tstc_mesh_test2d_nature (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                } else {
                    assert (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_godhand);
                    tstc_mesh_test2d_godhand (ptsim, ptc_base, ptc_test, &list_points_ok_2d);
                }
                // compare the two list
                assert (tstclist_pos_is_equal (&list_points_ok, &list_points_ok_2d));
            }
#endif /* USE_DBG_CHK_RESULT */

        }
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "tstc_mesh_test() return %d", ret);

        tstc_init (&tc0);
        if (ret >= 0) {
            if (ma_size (&list_points_ok) < 1) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "tstc_mesh_test() return no such position", ret);
                ret = -1;
            }
        }
        if (ret >= 0) {
            // join the new tile and adjust the data

            cnt_fail = 0;

            // select one position from the list
            if (ptsim->cb_selectone) {
                ptsim->cb_selectone (ptsim->userdata, &list_points_ok, &idxinc);
            } else {
                // 随机选取一个
                idxinc = my_irrand (ma_size (&list_points_ok));
            }
            ret = ma_data (&list_points_ok, idxinc, &stpos);
            assert (ret >= 0);

            // 1) for testing if there exist one such tile
            //tstc_copy (&tc0, ptc_base);
            ret = tstc_merge (ptc_base, ptc_test, stpos.rotnum, &(stpos.pos), &tc0);
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "tstc_merge() return %d", ret);
            assert (TSTC_NUM_TILE(&tc0) >= 2);
            assert (tc0.maxpos.x > 0);
            assert (tc0.maxpos.y > 0);
#if USE_DBG_CHK_RESULT
#if USE_THREEDIMENTION
            if (ptsim->flg_is2d)
#endif
            {
                tstilecomb_t tc_2d;
                tstc_init (&tc_2d);
                tstc_merge_2d (ptc_base, ptc_test, stpos.rotnum, &(stpos.pos), &tc_2d);
#if USE_THREEDIMENTION
                tc_2d.maxpos.z = 1;
#endif
                // compare the two tstilecomb_t
                assert (tstc_is_equal (&tc0, &tc_2d, ptsim->flg_norotate));
                assert (tstc_is_equal_2d (&tc0, &tc_2d, ptsim->flg_norotate));
                tstc_clear (&tc_2d);
            }

            // 测试是否merge正确
            ret = ma_rmalldata (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
            tstc_split (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut);

#if USE_THREEDIMENTION
            if (ptsim->flg_is2d)
#endif
            {
                ret = ma_rmalldata (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
                tstc_split_2d (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut_2d);
                assert (tstclist_tcomb_is_equal (&list_stile_mincut, &list_stile_mincut_2d, ptsim->flg_norotate));
            }
            assert (ma_size (&list_stile_mincut) < 1);
#endif /* USE_DBG_CHK_RESULT */

            // 对生成的 supertile 调整到其最低值
            if (0 == ptsim->flg_norotate) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "tstc_adjust_lower() ...");
#if USE_THREEDIMENTION
                tstc_adjust_lower (&tc0, ptsim->flg_is2d);
#else
                tstc_adjust_lower (&tc0, 1);
#endif
                assert (TSTC_NUM_TILE(&tc0) >= 2);
                assert (tc0.maxpos.x > 0);
                assert (tc0.maxpos.y > 0);
#if USE_DBG_CHK_RESULT
                // 测试是否 lower 正确
                ret = ma_rmalldata (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
                tstc_split (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut);
                assert (ma_size (&list_stile_mincut) < 1);

#if USE_THREEDIMENTION
                if (ptsim->flg_is2d)
#endif
                {
                    ret = ma_rmalldata (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
                    tstc_split_2d (ptc_test, ptsim, ptsim->temperature, &list_stile_mincut_2d);
                    assert (tstclist_tcomb_is_equal (&list_stile_mincut, &list_stile_mincut_2d, ptsim->flg_norotate));
                }
#endif /* USE_DBG_CHK_RESULT */

            }
        }
        if (ret >= 0) {
            tstilecomb_t *ptc;

            tc0.id = 1;
            assert (sizeof(i) == sizeof(size_t));
            ret = ts_add_supertile (ptsim, &tc0, &i, &idxinc);
            assert (ret >= 0);
            flg_newitem = 0;
            if (ret < 0) {
                // error
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Error in ts_add_supertile()");
            } else if (ret > 0) {
                flg_newitem = 1;
            }
            ret = ma_data (&(ptsim->countlist), i, &idxinc);
            assert (ret >= 0);
#if USE_THREEDIMENTION
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') select (% 3d:num % 3d) + (% 3d:num % 3d) \t@ (%d,%d,%d)rot[%d]\t= (% 3d:num % 3d)\tyear=%d", name_sim
                , idx_base, num_base, idx_test, num_test
                , stpos.pos.x, stpos.pos.y, stpos.pos.z
                , stpos.rotnum, i, idxinc, ptsim->year_current);
#else
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') select (% 3d:num % 3d) + (% 3d:num % 3d) \t@ (%d,%d)rot[%d]\t= (% 3d:num % 3d)\tyear=%d", name_sim
                , idx_base, num_base, idx_test, num_test
                , stpos.pos.x, stpos.pos.y
                , stpos.rotnum, i, idxinc, ptsim->year_current);
#endif
            if (ptsim->cb_resultinfo) {
                assert (cnt_fail == 0);
                ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
                assert (stpos.rotnum == (stpos.rotnum % TILE_STATUS_MAX));
                ret_chkresult = ptsim->cb_resultinfo (ptsim->userdata, idx_base, idx_test, &stpos, ptsim->temperature, i, (flg_newitem?ptc:NULL), &list_points_ok);
                ma_data_unlock (&(ptsim->tilelist), i);
                if (ret_chkresult < 0) {
                    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "break by caller's cb_resultinfo");
                }
            }
            // adjust the data
            if (idx_base == idx_test) {
                num_base -= 2;
                ts_update_count (ptsim, idx_base, num_base);
            } else {
                num_base --;
                ret = ts_update_count (ptsim, idx_base, num_base);
                assert (ret >= 0);
                num_test --;
                ret = ts_update_count (ptsim, idx_test, num_test);
                assert (ret >= 0);
            }
            num_total --; //num_total = num_total - 2 + 1;
            tstc_clear (&tc0);

            //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "set idx_last from %d to %d", ptsim->idx_last, i);
            ptsim->idx_last = i;

            /*
            if ((flg_newitem) && (ptsim->birth_target < 1) && (ptsim->ptarget)) {
                ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
                if (tstc_is_equal (ptc, ptsim->ptarget, ptsim->flg_norotate)) {
                    ptsim->birth_target = ptsim->year_current;
                }
                ma_data_unlock (&(ptsim->tilelist), i);
            }*/
        } else {
            tstc_clear (&tc0);

            cnt_fail ++;
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "fail times: %d", cnt_fail);
            if (ptsim->cb_notifyfail) {
                //assert (0 == flg_newitem);
                ret_chkresult = ptsim->cb_notifyfail (ptsim->userdata, cnt_fail);
                if (ret_chkresult < 0) {
                    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Error in user's cb_notifyfail @ %d times failures", cnt_fail);
                    break;
                }
            }
        }
        ma_data_unlock (&(ptsim->tilelist), idx_base);
        if (idx_test != idx_base) {
            ma_data_unlock (&(ptsim->tilelist), idx_test);
        }

        if (ret_chkresult < 0) {
            // 当用户测试结果返回是负值时表示需要退出
            //ret_chkresult = 0;
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "break here");
            break;
        }
    }

DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "trace here 1000: ret_chkresult=%d", ret_chkresult);

tssim_main_end:
    ma_clear (&list_points_ok, NULL);
#if USE_DBG_CHK_RESULT
    ma_clear (&list_points_ok_2d, NULL);
    ret = ma_clear (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
#endif /* USE_DBG_CHK_RESULT */
    ret = ma_clear (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
    assert (ret >= 0);
}

int
ts_simulate_main (tssiminfo_t *ptsim, const char *name_sim)
{
    switch (ptsim->algorithm) {
    case TSIM_ALGO_2HATAM:
        tilesim_2htam (ptsim, name_sim);
        break;
    case TSIM_ALGO_ATAM:
        tilesim_atam (ptsim, name_sim);
        break;
    case TSIM_ALGO_KTAM:
        tilesim_ktam (ptsim, name_sim);
        break;
    default:
        return -1;
        break;
    }
    return 0;
}

void
ts_sim_add_data_random (tssiminfo_t *ptsim)
{
    tstile_t *ptile;
    size_t i;
    size_t cnt;
    tstilecombitem_t tci;
    int idxinc;
    tstilecomb_t *ptc;

    ptsim->temperature = 2;

    ptsim->num_tilevec = 15;
    ptsim->ptilevec    = (tstile_t *)malloc (ptsim->num_tilevec * sizeof (tstile_t));
    assert (NULL != ptsim->ptilevec);
    ptile = ptsim->ptilevec;
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        size_t j;
        for (j = 0; j < ORI_MAX; j ++) {
            ptile->glues[j] = my_irrand (ptsim->num_tilevec);
        }
        ptile ++;
    }

    // add supertiles into the list
    memset (&tci, 0, sizeof (tci));

    for (i = 0; i < ptsim->num_tilevec; i ++) {
        tci.idtile = i;
        idxinc = ma_inc (&(ptsim->tilelist));
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idxinc);
        tstc_init (ptc);
        tstc_additem (ptc, &tci);
        ma_data_unlock (&(ptsim->tilelist), idxinc);
        // update tilelist and countlist simultaneously
        cnt = 100;
        ma_insert (&(ptsim->countlist), ma_size (&(ptsim->countlist)), &cnt);
    }
}

static const char * g_algo_lst[] = {
    /**/"(none)",
    /*TSIM_ALGO_2HATAM*/"2haTAM",
    /*TSIM_ALGO_ATAM*/"aTAM",
    /*TSIM_ALGO_KTAM*/"kTAM",
};

const char *
tssim_algo_type2cstr (int type)
{
    if (type >= NUM_TYPE(g_algo_lst, const char *)) {
        return "(unknown)";
    }
    return g_algo_lst[type];
}

int
tssim_algo_cstr2type (const char *name)
{
    size_t i;
    for (i = 0; i < NUM_TYPE(g_algo_lst, const char *); i ++) {
        if (0 == strcmp (name, g_algo_lst[i])) {
            return i;
        }
    }
    return -1;
}

/* 主版本表示一类可以兼容的格式，不同的主版本间有兼容问题 */
/* 次版本表示增强的功能，在同一主版本下不同次版本间可以相互兼容 */
#define XML_VERSION_MAJOR 0
#define XML_VERSION_MINOR 1

int
ts_sim_save_data_xml (tssiminfo_t *ptsim, const char *fn_xml)
{
    int i;
    size_t *pglue;
    tstile_t *ptile;
    size_t cnt;
    size_t birth;
    size_t idx;
    tstilecomb_t *ptc;
    FILE *fp_xml;

    assert (NULL != ptsim);
    assert (NULL != &(ptsim->tilelist));
    assert (NULL != &(ptsim->countlist));
    assert (NULL != &(ptsim->birthlist));

    fp_xml = fopen (fn_xml, "wb");
    // write the header
    fprintf (fp_xml, "<?xml version='1.0' encoding='utf-8' ?>\n");
    fprintf (fp_xml, "<!-- Saved by the Supertile Self-assembly Simulator. -->\n");
    fprintf (fp_xml, "<tilesim>\n  <version>\n    <major>%d</major>\n    <minor>%d</minor>\n  </version>\n", XML_VERSION_MAJOR, XML_VERSION_MINOR);
#if USE_THREEDIMENTION
    tssim_save_encoding_3d (fp_xml);
#endif
    fprintf (fp_xml, "  <temperature>%d</temperature>\n", ptsim->temperature);
    fprintf (fp_xml, "  <rotatable>%s</rotatable>\n", ((0 == ptsim->flg_norotate)?"true":"false"));

    assert (((0 != ptsim->steps_1year_max) && (ptsim->steps_1year_max > ptsim->steps_1year_min)) || (0 == ptsim->steps_1year_max));
    fprintf (fp_xml, "  <simulationsetup>\n");
    //fprintf (fp_xml, "    <inserthole>%s</inserthole>\n", ((0 == ptsim->flg_inserthole)?"true":"false"));
    fprintf (fp_xml, "    <currentyear>%d</currentyear>\n", ptsim->year_current);
    fprintf (fp_xml, "    <stepsmin>%d</stepsmin>\n", ptsim->steps_1year_min);
    fprintf (fp_xml, "    <stepsmax>%d</stepsmax>\n", ptsim->steps_1year_max);

    fprintf (fp_xml, "    <algorithm>%s</algorithm>\n", tssim_algo_type2cstr (ptsim->algorithm));
#if USE_TILESIM_ATAM
    fprintf (fp_xml, "    <ktam_gmc>%f</ktam_gmc>\n", ptsim->Gmc);
    fprintf (fp_xml, "    <ktam_gse>%f</ktam_gse>\n", ptsim->Gse);
    fprintf (fp_xml, "    <ktam_ratek>%f</ktam_ratek>\n", ptsim->ratek);
#endif
    fprintf (fp_xml, "  </simulationsetup>\n");

    pglue = ptsim->pgluevec;
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        fprintf (fp_xml,
            "  <glue>\n"
            "    <id>%d</id>\n"
            /*"    <name>%d</name>\n"*/
            "    <strength>%d</strength>\n"
            "  </glue>\n"
            , i, *pglue);
        pglue ++;
    }

    ptile = ptsim->ptilevec;
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        fprintf (fp_xml,
            "  <tile>\n"
            "    <id>%d</id>\n"
            "    <group>%d</group>\n"
            "    <north>%d</north>\n"
            "    <east>%d</east>\n"
            "    <south>%d</south>\n"
            "    <west>%d</west>\n"
            , i, ptile->group
            , ptile->glues[ORI_NORTH]
            , ptile->glues[ORI_EAST]
            , ptile->glues[ORI_SOUTH]
            , ptile->glues[ORI_WEST]
            );
#if USE_THREEDIMENTION
        if (0 == ptsim->flg_is2d) {
            fprintf (fp_xml,
                "    <front>%d</front>\n"
                "    <back>%d</back>\n"
                , ptile->glues[ORI_FRONT], ptile->glues[ORI_BACK]
                );
        }
#endif
        if (NULL != ptile->label) {
            fprintf (fp_xml, "    <label>%s</label>\n", ptile->label);
        }
        if (NULL != ptile->name) {
            fprintf (fp_xml, "    <name>%s</name>\n", ptile->name);
        }
        fprintf (fp_xml, "  </tile>\n");
        ptile ++;
    }

    if ((NULL != ptsim->ptarget) && (slist_size (&(ptsim->ptarget->tbuf)) > 0)) {
        // store the <targetsupertile>
        fprintf (fp_xml, "  <targetsupertile>\n");
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx);
#if USE_THREEDIMENTION
        tssim_save_xml_supertile (ptsim->ptarget, i, ptsim->birth_target, "targetsupertile's birth, for automatic text porcess", 1, ptsim->flg_is2d, fp_xml);
#else
        tssim_save_xml_supertile (ptsim->ptarget, i, ptsim->birth_target, "targetsupertile's birth, for automatic text porcess", 1, 1, fp_xml);
#endif
        ma_data_unlock (&(ptsim->tilelist), idx);
        fprintf (fp_xml, "  </targetsupertile>\n");
    }

    for (i = 0; i < TS_NUMTYPE_SUPERTILE (ptsim); i ++) {
        if (ptsim->flg_shrink_record) {
            TS_UNSORTED_IDX (ptsim, i, &idx);
            ma_data (&(ptsim->countlist), idx, &cnt);
            if (cnt < 1) {
                continue; // skip the supertile with count 0
            }
        } else {
            idx = i;
            assert (TS_NUMTYPE_SUPERTILE (ptsim) == ma_size (&(ptsim->tilelist)));
            ma_data (&(ptsim->countlist), idx, &cnt);
        }

        //ma_data (&(ptsim->countlist), idx, &cnt);
        ma_data (&(ptsim->birthlist), idx, &birth);
        fprintf (fp_xml, "  <supertile>\n");
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx);
#if USE_THREEDIMENTION
        tssim_save_xml_supertile (ptc, i, birth, NULL, cnt, ptsim->flg_is2d, fp_xml);
#else
        tssim_save_xml_supertile (ptc, i, birth, NULL, cnt, 1, fp_xml);
#endif
        ma_data_unlock (&(ptsim->tilelist), idx);
        fprintf (fp_xml, "  </supertile>\n");
    }
#if DEBUG
    // 在 ptsim->flg_shrink_record = 1 的情况下，所有count为0的supertile 都会被回收而不保证以前记录的 id 对应到原始的 supertile
    if (0 == ptsim->flg_shrink_record) {
#if USE_THREEDIMENTION
        tssim_tc_xml_save (&(ptsim->tclist), ptsim->flg_is2d, fp_xml);
#else
        tssim_tc_xml_save (&(ptsim->tclist), 1, fp_xml);
#endif
    }
#endif
    fprintf (fp_xml, "</tilesim>\n");
    fclose (fp_xml);
    return 0;
}

// 读取 supertile 的数据。
// 该函数可以读取结构为 supertile 的 <supertile> 和 <targetsupertile>
// cur_child: if (! xmlStrcmp (cur->name, (const xmlChar *)"supertile")) cur_child = cur->xmlChildrenNode;
// ret_st: the structure was inited and cleared by the caller
// ret_flg_is2d: return if the data is 2D
static int
ts_sim_lddata_rdsupertile (xmlDocPtr doc, xmlNodePtr cur_child0, tstilecomb_t *ret_st, size_t *ret_cnt, size_t *ret_birth, char *ret_flg_is2d)
{
    int ret = 0;
    xmlChar *key;
    tstilecombitem_t tci;

    for (; (ret >= 0) && (NULL != cur_child0); cur_child0 = cur_child0->next) {
        if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"quantity")) {
            if (NULL != ret_cnt) {
                key = xmlNodeListGetString (doc, cur_child0->xmlChildrenNode, 1);
                *ret_cnt = atoi ((const char *)key);
                xmlFree (key);
            }
        } else if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"id")) {
            key = xmlNodeListGetString (doc, cur_child0->xmlChildrenNode, 1);
            ret_st->id = atoi ((const char *)key);
            xmlFree (key);
            //flg_noquantity = 0;
        } else if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"birth")) {
            if (NULL != ret_birth) {
                key = xmlNodeListGetString (doc, cur_child0->xmlChildrenNode, 1);
                *ret_birth = atoi ((const char *)key);
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "get birth: '%s' = %d", key, *ret_birth);
                xmlFree (key);
            }
        } else if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"tileitem")) {
            memset (&tci, 0, sizeof (tci));
            ret = tssim_read_xml_tileitem (doc, cur_child0->xmlChildrenNode, &tci, ret_flg_is2d);
            if (ret < 0) {
                break;
            }
            tstc_additem (ret_st, &tci);
        }
    }
    return ret;
}

int
ts_sim_load_data_xml (tssiminfo_t *ptsim, const char *fn_xml)
{
    int ret = 0;
    size_t *pglue;
    tstile_t *ptile;
    //size_t i;
    size_t cnt;
    tstilecombitem_t tci;
    size_t idxinc;
    tstilecomb_t *ptc;

    xmlChar *key;
    xmlDocPtr doc;
    xmlNodePtr pxn;
    xmlNodePtr cur;
    xmlNodePtr cur_child;
    size_t cur_idx;
    size_t cur_glue;
    tstile_t cur_tile;
    char flg_read_idx;
    char flg_read_data;
    size_t birth;
    char flg_notarget = 1;
    char flg_noquantity;

    assert (NULL != ptsim);

    ptsim->idx_last = -1;

    // we have to reset the tssiminfo_t before load a new data
    tssiminfo_reset (ptsim);

    //doc = xmlReadFile (fn_xml, NULL, 0);
    doc = xmlParseFile (fn_xml);
    if (NULL == doc) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "error in xmlReadFile('%s')!", fn_xml);
        return -1;
    }

    pxn = xmlDocGetRootElement (doc);
    if (NULL == pxn) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "ERR: XML file error");
        ret = -1;
        goto end_ts_sim_load_data_xml;
    }
#if USE_THREEDIMENTION
    ptsim->flg_is2d = 1;
#endif
    ret = 0;
    // check the version
    //fscanf ("%d", &(ptsim->temperature));
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"version")) {
            for (cur_child = cur->xmlChildrenNode; NULL != cur_child; cur_child = cur_child->next) {
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"major")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ret = atoi ((const char *)key);
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "INFO: Data file version, major %d, '%s'='%s'", ret, cur_child->name, key);
                    xmlFree (key);
                    break;
                }
            }
#if USE_THREEDIMENTION
        } else if (! xmlStrcmp (cur->name, (const xmlChar *)"encoding3d")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "tssim_read_encoding_3d() ....");
            tssim_read_encoding_3d (doc, cur->xmlChildrenNode);
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "tssim_read_encoding_3d() END ");
#endif
        }
    }
    if (ret != XML_VERSION_MAJOR) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "ERR: Data file version, supported ver %d, but file ver %d", XML_VERSION_MAJOR, ret);
        ret = -1;
        goto end_ts_sim_load_data_xml;
    }

    // read the temperature
    //fscanf ("%d", &(ptsim->temperature));
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"temperature")) {
            key = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
            ptsim->temperature = atoi ((const char *)key);
            xmlFree (key);
            break;
        }
    }
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"rotatable")) {
            key = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
            ptsim->flg_norotate = 0;
            if (! xmlStrcmp (key, (const xmlChar *)"false")) {
                ptsim->flg_norotate = 1;
            }
            xmlFree (key);
            break;
        }
    }

    cur = pxn->xmlChildrenNode;
    cnt = 0;
    while (NULL != cur) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"glue")) {
            cnt ++;
        }
        cur = cur->next;
    }
    ptsim->num_gluevec = cnt;
    ptsim->pgluevec = (size_t *)realloc (ptsim->pgluevec, ptsim->num_gluevec * sizeof (size_t));
    assert (NULL != ptsim->pgluevec);
    //memset (ptsim->pgluevec, 0, ptsim->num_gluevec * sizeof (size_t));
    //memset (&cur_glue, 0, sizeof (cur_glue));

    pglue = &cur_glue;
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"glue")) {
            flg_read_idx = 0;
            flg_read_data = 0;
            for (cur_child = cur->xmlChildrenNode; NULL != cur_child; cur_child = cur_child->next) {
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"id")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    cur_idx = atoi ((const char *)key);
                    xmlFree (key);
                    assert (cur_idx < ptsim->num_gluevec);
                    flg_read_idx = 1;
                    if (pglue != &(ptsim->pgluevec[cur_idx])) {
                        if (flg_read_data) {
                            ptsim->pgluevec[cur_idx] = *pglue;
                            break;
                        }
                        pglue = &(ptsim->pgluevec[cur_idx]);
                    }
                }
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"strength")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    *pglue = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                }
            }
            if (0 == flg_read_idx) {
                // error: lack of "id" item
                assert (0);
            }
        } else if (! xmlStrcmp (cur->name, (const xmlChar *)"simulationsetup")) {
            for (cur_child = cur->xmlChildrenNode; NULL != cur_child; cur_child = cur_child->next) {
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"currentyear")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->year_current = atoi ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"stepsmin")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->steps_1year_min = atoi ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"stepsmax")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->steps_1year_max = atoi ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"algorithm")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->algorithm = tssim_algo_cstr2type ((const char *)key);
                    xmlFree (key);
#if USE_TILESIM_ATAM
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"ktam_gmc")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->Gmc = atof ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"ktam_gse")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->Gse = atof ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"ktam_ratek")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptsim->ratek = atof ((const char *)key);
                    xmlFree (key);
#endif
                }
            }
        } else if (! xmlStrcmp (cur->name, (const xmlChar *)"targetsupertile")) {
            if (NULL != ptsim->ptarget) {
                tstc_clear (ptsim->ptarget);
                tstc_init (ptsim->ptarget);
            } else {
                ptsim->ptarget = tstc_create ();
            }

            cnt = 0;
            ptsim->birth_target = 0;
#if USE_THREEDIMENTION
            ret = ts_sim_lddata_rdsupertile (doc, cur->xmlChildrenNode, ptsim->ptarget, &cnt, &(ptsim->birth_target), &(ptsim->flg_is2d));
#else
            ret = ts_sim_lddata_rdsupertile (doc, cur->xmlChildrenNode, ptsim->ptarget, &cnt, &(ptsim->birth_target), NULL);
#endif
            if (ret < 0) {
                break;
            }
            if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                tstc_adjust_lower (ptsim->ptarget, ptsim->flg_is2d);
#else
                tstc_adjust_lower (ptsim->ptarget, 1);
#endif
            }
            flg_notarget = 0;

        }
    }
    if (ret < 0) {
        goto end_ts_sim_load_data_xml;
    }
    if ((0 != ptsim->steps_1year_max) && (ptsim->steps_1year_max < ptsim->steps_1year_min)) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Error in the xml data: the stepsmax(%d) < stepsmin(%d)"
            , ptsim->steps_1year_max, ptsim->steps_1year_min);
        ret = -1;
        goto end_ts_sim_load_data_xml;
    }

    cur = pxn->xmlChildrenNode;
    cnt = 0;
    while (NULL != cur) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"tile")) {
            cnt ++;
        }
        cur = cur->next;
    }
    assert (0 == ptsim->num_tilevec);
    assert (NULL == ptsim->ptilevec);
    ptsim->num_tilevec = cnt;
    ptsim->ptilevec = (tstile_t *)realloc (ptsim->ptilevec, ptsim->num_tilevec * sizeof (tstile_t));
    memset (ptsim->ptilevec, 0, ptsim->num_tilevec * sizeof (tstile_t));
    assert (NULL != ptsim->ptilevec);
    //ptile = ptsim->ptilevec;
    ret = 0;
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"tile")) {
            flg_read_idx = 0;
            flg_read_data = 0;
            memset (&cur_tile, 0, sizeof (cur_tile));
            ptile = &cur_tile;
            for (cur_child = cur->xmlChildrenNode; NULL != cur_child; cur_child = cur_child->next) {
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"id")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    cur_idx = atoi ((const char *)key);
                    xmlFree (key);
                    if (cur_idx >= ptsim->num_tilevec) {
                        // skip
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "read false idx: %d", cur_idx);
                        ret = -1;
                        break;
                    }
                    flg_read_idx = 1;
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile id: %d", cur_idx);
                    if (ptile != &(ptsim->ptilevec[cur_idx])) {
                        if (flg_read_data) {
                            memmove (&(ptsim->ptilevec[cur_idx]), ptile, sizeof (*ptile));
                            ptile->label = NULL;
                        }
                        ptile = &(ptsim->ptilevec[cur_idx]);
                    }
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"group")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->group = atoi ((const char *)key);
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"north")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_NORTH] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) north: %d", cur_idx, ptile->glues[ORI_NORTH]);
                    assert (ptile->glues[ORI_NORTH] < ptsim->num_gluevec);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"east")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_EAST] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    //DBGMSG(PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "glueE=%d; num_gluevec=%d", ptile->glueE, ptsim->num_gluevec);
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) east: %d", cur_idx, ptile->glues[ORI_EAST]);
                    assert (ptile->glues[ORI_EAST] < ptsim->num_gluevec);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"south")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_SOUTH] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) south: %d", cur_idx, ptile->glues[ORI_SOUTH]);
                    assert (ptile->glues[ORI_SOUTH] < ptsim->num_gluevec);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"west")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_WEST] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) west: %d", cur_idx, ptile->glues[ORI_WEST]);
                    assert (ptile->glues[ORI_WEST] < ptsim->num_gluevec);

#if USE_THREEDIMENTION
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"front")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_FRONT] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    //DBGMSG(PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "glueF=%d; num_gluevec=%d", ptile->glueF, ptsim->num_gluevec);
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) front: %d", cur_idx, ptile->glues[ORI_FRONT]);
                    assert (ptile->glues[ORI_FRONT] < ptsim->num_gluevec);
                    ptsim->flg_is2d = 0;
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"back")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->glues[ORI_BACK] = atoi ((const char *)key);
                    xmlFree (key);
                    flg_read_data = 1;
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "read tile (id=%d) back: %d", cur_idx, ptile->glues[ORI_BACK]);
                    assert (ptile->glues[ORI_BACK] < ptsim->num_gluevec);
                    ptsim->flg_is2d = 0;
#else
                } else if ((! xmlStrcmp (cur_child->name, (const xmlChar *)"front")) || (! xmlStrcmp (cur_child->name, (const xmlChar *)"back"))) {
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR,
                        "ERR: This version of the sss could only support 2D mode of the simulation, please use 3D/2D mode of the sss version."
                        );
                    ret = -1;
                    break;
#endif
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"label")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->label = strdup ((const char *)key); // utf-8?
                    xmlFree (key);
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"name")) {
                    key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
                    ptile->name = strdup ((const char *)key); // utf-8?
                    xmlFree (key);
                }
            }
            if (NULL != cur_tile.label) {
                free (cur_tile.label);
            }
            if (ret < 0) {
                break;
            }
            if (0 == flg_read_idx) {
                // error: lack of "id" item
                assert (0);
            }
        }
    }
    if (ret < 0) {
        goto end_ts_sim_load_data_xml;
    }

    // add supertiles into the list
    memset (&tci, 0, sizeof (tci));
    for (cur = pxn->xmlChildrenNode; NULL != cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, (const xmlChar *)"supertile")) {
            flg_noquantity = 1;

            assert (sizeof (idxinc) == sizeof(size_t));
            ret = ts_tilelist_st_new (ptsim, &idxinc, &ptc);
            tstc_reset (ptc);

            cnt = 0;
            birth = 0;
#if USE_THREEDIMENTION
            ret = ts_sim_lddata_rdsupertile (doc, cur->xmlChildrenNode, ptc, &cnt, &birth, &(ptsim->flg_is2d));
#else
            ret = ts_sim_lddata_rdsupertile (doc, cur->xmlChildrenNode, ptc, &cnt, &birth, NULL);
#endif
            ma_data_unlock (&(ptsim->tilelist), idxinc);
            if (ret < 0) {
                ts_tilelist_st_delete (ptsim, idxinc);
                break;
            }
            if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                tstc_adjust_lower (ptc, ptsim->flg_is2d);
#else
                tstc_adjust_lower (ptc, 1);
#endif

#if DEBUG
{
    memarr_t list_stile_mincut;
    ma_init (&list_stile_mincut, sizeof (tstilecomb_t));

                // 测试是否 adjust 正确
                //ret = ma_rmalldata (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
                tstc_split (ptc, ptsim, ptsim->temperature, &list_stile_mincut);

#if USE_THREEDIMENTION
                if (ptsim->flg_is2d)
#endif
                {
                    memarr_t list_stile_mincut_2d;
                    ma_init (&list_stile_mincut_2d, sizeof (tstilecomb_t));
                    //ret = ma_rmalldata (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
                    tstc_split_2d (ptc, ptsim, ptsim->temperature, &list_stile_mincut_2d);
                    assert (tstclist_tcomb_is_equal (&list_stile_mincut, &list_stile_mincut_2d, ptsim->flg_norotate));
                    ma_clear (&list_stile_mincut_2d, (memarr_cb_destroy_t)tstc_clear);
                }
                assert (ma_size (&list_stile_mincut) < 1);

    ma_clear (&list_stile_mincut, (memarr_cb_destroy_t)tstc_clear);
}
#endif // DEBUG
            }

            // update tilelist and countlist simultaneously
            ret = slist_store (&(ptsim->tilelstidx), &idxinc);
            if (ret < 0) {
                ts_tilelist_st_delete (ptsim, idxinc);
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to insert supertile to list. Data duplicated? Anyway, SKIP!");
                continue;
            }
            assert (ret >= 0);
            ret = ts_update_count (ptsim, idxinc, cnt);
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to update the count of supertile.");
                break;
            }
            assert (ret >= 0);
            ret = ma_replace (&(ptsim->birthlist), idxinc, &birth);
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to update the count of supertile.");
                break;
            }
            assert (ret >= 0);

            ptsim->num_tiles += (cnt * TSTC_NUM_TILE(ptc));
#if DEBUG
        } else if (! xmlStrcmp (cur->name, (const xmlChar *)"testcases")) {
            // add testcase, only for debug
#if USE_THREEDIMENTION
            ret = tssim_tc_xml_load (&(ptsim->tclist), ptsim->flg_is2d, ptsim->flg_norotate, doc, cur->xmlChildrenNode);
#else
            ret = tssim_tc_xml_load (&(ptsim->tclist), 1, ptsim->flg_norotate, doc, cur->xmlChildrenNode);
#endif
            if (ret < 0) {
                break;
            }
#endif
        }
    }
    if (ret < 0) {
        goto end_ts_sim_load_data_xml;
    }

    // adjust the number of the "birth"
    for (; ma_size (&(ptsim->birthlist)) < ma_size (&(ptsim->tilelist)); ) {
        ma_inc (&(ptsim->birthlist));
    }

    if (flg_notarget) {
        if (NULL != ptsim->ptarget) {
            tstc_destroy (ptsim->ptarget);
            ptsim->ptarget = NULL;
        }
    }

    ptsim->p_g_dir_stestofslide = (char *)g_dir_stestofslide_2d;
    ptsim->cb_face_move = face_move_2d;
    ptsim->max_faces = 4;
#if USE_THREEDIMENTION
    if (0 == ptsim->flg_is2d) {
        ptsim->p_g_dir_stestofslide = (char *)g_dir_stestofslide_3d;
        ptsim->cb_face_move = face_move_3d;
        ptsim->max_faces = ORI_MAX;
        assert (ORI_MAX == 6);
    }
#else
    assert (ORI_MAX == 4);
#endif
//ptsim->p_g_dir_stestofslide = (char *)g_dir_stestofslide_3d;

end_ts_sim_load_data_xml:
    xmlFreeDoc (doc);
    if (ret < 0) {
        tssiminfo_reset (ptsim);
    }
    return ret;
}

static int
slist_cb_comp_tilelstidx (void *userdata, void * data1, void * data2)
{
    int ret;
    tssiminfo_t *ptsim = (tssiminfo_t *)userdata;
    tstilecomb_t *tc1;
    tstilecomb_t *tc2;
    assert (NULL != userdata);
    // compare the two supertiles
    tc1 = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), *((size_t *)data1));
    tc2 = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), *((size_t *)data2));
    assert (NULL != tc1);
    assert (NULL != tc2);
    ret = tstc_compare_full (tc1, tc2, ptsim->flg_norotate);
    ma_data_unlock (&(ptsim->tilelist), *((size_t *)data1));
    ma_data_unlock (&(ptsim->tilelist), *((size_t *)data2));
    return ret;
}
/*
static int
slist_cb_swap_tilelstidx (void *userdata, void * data1, void * data2)
{
    tssiminfo_t *ptsim = (tssiminfo_t *)userdata;
    assert (NULL != userdata);
}
*/

int
tssiminfo_init (tssiminfo_t *ptsim)
{
    memset (ptsim, 0, sizeof (*ptsim));
    ma_init (&(ptsim->tilelist), sizeof (tstilecomb_t));
    ma_init (&(ptsim->countlist), sizeof (size_t));
    ma_init (&(ptsim->birthlist), sizeof (size_t));
    slist_init (&(ptsim->tilelstidx), sizeof (size_t), ptsim, slist_cb_comp_tilelstidx, slist_cb_swap_sizet);

    slist_init (&(ptsim->freedlist), sizeof (size_t), ptsim, slist_cb_comp_sizet, slist_cb_swap_sizet);

#if DEBUG
    ma_init (&(ptsim->tclist), sizeof (tssiminfo_testcase_t));
#endif
    TSIM_MESHTEST_MODE (ptsim, 0);
    TSIM_RECORD_MODE (ptsim, 0);
    TSIM_SIM_ALGO (ptsim, TSIM_ALGO_2HATAM);
    ptsim->idx_last = -1;

#if USE_TILESIM_ATAM
    // parameters for kTam model
    ptsim->Gmc = 15; // 15
    ptsim->Gse = 7.8; // 7.8;
    ptsim->ratek = 1000000.0; // 1000000.0;
#endif
    return 0;
}

int
tssiminfo_reset (tssiminfo_t *ptsim)
{
    ma_rmalldata (&(ptsim->tilelist), (memarr_cb_destroy_t)tstc_clear);
    ma_rmalldata (&(ptsim->countlist), NULL);
    ma_rmalldata (&(ptsim->birthlist), NULL);

    if (ptsim->pgluevec) {
        free (ptsim->pgluevec);
        ptsim->pgluevec = NULL;
    }
    ptsim->num_gluevec = 0;
    if (ptsim->ptilevec) {
        size_t i;
        // reset the ptilevect
        for (i = 0; i < ptsim->num_tilevec; i ++) {
            if (ptsim->ptilevec[i].name) {
                free (ptsim->ptilevec[i].name);
            }
            if (ptsim->ptilevec[i].label) {
                free (ptsim->ptilevec[i].label);
            }
        }
        free (ptsim->ptilevec);
        ptsim->ptilevec = NULL;
    }
    ptsim->num_tilevec = 0;

    if (ptsim->ptarget) {
        tstc_destroy (ptsim->ptarget);
        ptsim->ptarget = NULL;
    }
    slist_rmalldata (&(ptsim->tilelstidx), NULL);
    slist_rmalldata (&(ptsim->freedlist), NULL);

#if DEBUG
    ma_rmalldata (&(ptsim->tclist), (memarr_cb_destroy_t)tssim_tc_clear);
#endif // DEBUG
    return 0;
}

int
tssiminfo_clear (tssiminfo_t *ptsim)
{
    assert (ptsim);
    tssiminfo_reset (ptsim);

    if (ptsim->ptarget) {
        tstc_destroy (ptsim->ptarget);
    }
    ma_clear (&(ptsim->tilelist), (memarr_cb_destroy_t)tstc_clear);
    ma_clear (&(ptsim->countlist), NULL);
    ma_clear (&(ptsim->birthlist), NULL);
    slist_clear (&(ptsim->tilelstidx), NULL);
    slist_clear (&(ptsim->freedlist), NULL);

#if DEBUG
    ma_clear (&(ptsim->tclist), (memarr_cb_destroy_t)tssim_tc_clear);
#endif // DEBUG
    return 0;
}

////////////////////////////////////////

// 检查所有的 supertile 之间是否可以合并，如果查出有两个可以合并，则检查完一遍后，继续再检查一遍
// 检查所有当前
// for (i = 0; i < the size of supertile list; i ++)
//   for (j = i; j < the size of supertile list; j ++)
//     check combination of (i, j)
int
tssim_cb_getdata_chkall (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    if (ptci->idx_test >= ptci->cur_max) {
        ptci->idx_base ++;
        if (ptci->idx_base >= ptci->cur_max) {
            if (ptci->flg_rechk) {
                // 有新值插入，重新检查
                ptci->cur_max = ma_size (pcountlist);
                ptci->idx_base = 0;
                ptci->flg_rechk = 0;
                //ptci->idx_test = 0;
            } else {
                return 0;
            }
        }
        ptci->idx_test = ptci->idx_base;
    }
    *pidx_base = ptci->idx_base;
    *pidx_test = ptci->idx_test;
    ptci->idx_test ++;
    return 1;
}

int
tssim_cb_resultinfo_chkall (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    if (NULL != pstpos) {
        assert (NULL != ptci);
#if 0
        if (0 == flg_newitem)) {
            if ((idxnew <= ptci->idx_base) || (idxnew <= ptci->idx_test)) {
                ptci->flg_rechk = 1;
            }
        } else {
            ptci->cur_max = idxnew + 1;
        }
#else
        ptci->flg_rechk = 1;
#endif
    }
    if (NULL != ptc) {
        ptci->num_supertiles --;//num_supertiles = ts_get_total_supertiles (ptsim);
    }
    return 0;
}

int
tssim_cb_notifyfail_chkall (void *userdata, size_t cnt_fail)
{
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    assert (NULL != ptci);
    if (cnt_fail / 1000 > ptci->num_supertiles) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "the count of failure > %d", ptci->num_supertiles);
        return -1;
    }
    return 0;
}

////////////////////////////////////////

// 当测试两个 supertile 合并失败的次数连续达到1000次的时候需要终止测试，返回程序
int
tssim_cb_notifyfail_common (void *userdata, size_t cnt_fail)
{
    if (cnt_fail > 1000) {
        return -1;
    }
    return 0;
}

int
tssim_cb_getdata_random_test (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    size_t i;
    size_t inc;
    size_t num_total;
    size_t num_base;
    size_t num_test;
    size_t idx_base;
    size_t idx_test;
    num_total = 0;
    for (i = 0; i < ma_size (pcountlist); i ++) {
        //
        ma_data (pcountlist, i, &inc);
        num_total += inc;
    }
    // select one tile random
    inc = my_irrand (num_total);
    for (i = 0; i < ma_size (pcountlist); i ++) {
        ma_data (pcountlist, i, &num_base);
        if (num_base > inc) {
            // get it
            idx_base = i;
            break;
        }
        inc -= num_base;
    }
    assert (i < ma_size (pcountlist));
    assert (i == idx_base);
    // select another one tile random
    inc = my_irrand (num_total);
    for (i = 0; i < ma_size (pcountlist); i ++) {
        ma_data (pcountlist, i, &num_test);
        if (num_test > inc) {
            // get it
            idx_test = i;
            break;
        }
        inc -= num_test;
    }
    assert (i < ma_size (pcountlist));
    assert (i == idx_test);
    *pidx_base = idx_base;
    *pidx_test = idx_test;
    return 1;
}

// 对所有每个在pcountlist中的 idx_base，随机挑选一个与其做结合测试。
// 挑选的每个 idx 其个数大于等于1，如果两个相同，则其个数应该大于等于2
// 对每个轮完一圈后，year增加1
// 初始化：ptci->idx_base = ptci->cur_max + 1;
int
tssim_cb_getdata_timeofsquare (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    size_t num_base;
    size_t num_test;
    size_t cnt;
    size_t i;
    int ret;
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    int flg_ypp = 0; // year should increase one

    assert (NULL != pyear_cur);
    assert (NULL != pidx_base);
    assert (NULL != pidx_test);

    for (; 0 == flg_ypp || (flg_ypp && (ptci->idx_base < ptci->cur_max));) {
        if (ptci->idx_base >= ptci->cur_max) {
            flg_ypp = 1;
            ptci->idx_base = 0;
            ptci->cur_max = ma_size (pcountlist);
        } else {
            ptci->idx_base ++;
        }
        ma_data (pcountlist, ptci->idx_base, &num_base);
#if 1
        // select one tile random
        ptci->idx_test = ptci->idx_base;
        assert (ptci->num_supertiles > 0);
        cnt = my_irrand (ptci->num_supertiles);
        num_test = cnt;
        for (i = 0; i < ma_size (&(ptci->ptsim->countlist)); i ++) {
            ret = ma_data (&(ptci->ptsim->countlist), i, &num_test);
            assert (ret >= 0);
            if (num_test > cnt) {
                // get it
                ptci->idx_test = i;
                break;
            }
            cnt -= num_test;
        }
        if (num_test > cnt) {
            break;
        }
#else
        cnt = 1000;
        for (; cnt > 0; cnt --) {
            ptci->idx_test = my_irrand (ptci->cur_max);
            // check the item
            ma_data (pcountlist, ptci->idx_test, &num_test);
            if ((ptci->idx_test == ptci->idx_base) && (num_test >= 2)) {
                // got
                break;
            }
            if ((num_test >= 1) && (num_base >= 1)) {
                break;
            }
        }
        if (cnt > 0) {
            break;
        }
#endif
    }
    if (0 != flg_ypp) {
        if (ptci->idx_base < ptci->cur_max) {
            *pyear_cur = *pyear_cur + 1;
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_WARNING, "Info: year ++ == %d", *pyear_cur);
        } else {
            DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in random");
            //assert (0);
            //return -1;
        }
    }
    *pidx_base = ptci->idx_base;
    *pidx_test = ptci->idx_test;

    return 1;
}

// the callback function for notify the user level that the one supertile was created.
// this function will compare the supertile with the target supertile preset by the xml datafile and
// find out if the current amount of the target supertiles is exceed the percentage watched by system.
int
tssim_cb_resultinfo_timeofsquare (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    int ret = 0;
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    size_t num_created;

    if (NULL != ptc) {
        ptci->num_supertiles --;//num_total = ts_get_total_supertiles (ptsim);
    }
    if ((NULL != ptci->ptsim->ptarget) && (slist_size (&(ptci->ptsim->ptarget->tbuf)) > 0)) {
        if (NULL == ptc) {
            ptc = (tstilecomb_t *) ma_data_lock (&(ptci->ptsim->tilelist), idx_created);
            assert (NULL != ptc);
            assert (ptci->ptsim->ptarget);
            if (tstc_is_equal (ptci->ptsim->ptarget, ptc, ptci->ptsim->flg_norotate)) {
                double per;
                assert (ptci->target_percent >= 0.0);
                assert (ptci->target_percent <= 100.0);
                ma_data (&(ptci->ptsim->countlist), idx_created, &num_created);
                per = num_created * slist_size (&(ptc->tbuf));
                per = per * 100.0;
                per = per / ptci->ptsim->num_tiles;
                if (per >= ptci->target_percent) {
                    if (ptci->ptsim->birth_target < 1) {
                        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_WARNING, "The tiles of the target supertiles is %.2f%% of the total tiles. Year(%d)", ptci->target_percent, ptci->ptsim->year_current);
                        ptci->ptsim->birth_target = ptci->ptsim->year_current;
                    }
                    ret = -1;
                }
            }
            ma_data_unlock (&(ptci->ptsim->tilelist), idx_created);
        } else if (0.0 == ptci->target_percent) {
            assert (ptci->ptsim->ptarget);
            if (tstc_is_equal (ptci->ptsim->ptarget, ptc, ptci->ptsim->flg_norotate)) {
                // new item, check it
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "Find the target !!! birth = %d", ptci->ptsim->year_current);
                assert (ptci->ptsim->birth_target < 1);
                ptci->ptsim->birth_target = ptci->ptsim->year_current;
                ret = -1;
            }
        }
    }
    return ret;
}

int
ts_simulate (tssiminfo_t *ptsim, const char *name_sim)
{
    tssim_chkall_info_t tsimchk;

    printf ("**** Start Simulation ****\n");
    // default:

    ptsim->cb_getdata       = tssim_cb_getdata_random_test;
    ptsim->cb_selectone     = NULL;
    ptsim->cb_resultinfo    = NULL;
    ptsim->cb_notifyfail    = tssim_cb_notifyfail_common;
    ptsim->cb_findmergepos  = NULL;
    ptsim->cb_storemergepos = NULL;
    ptsim->userdata = NULL;

    ts_simulate_main (ptsim, name_sim);

#if 1
    printf ("**** Check the remains ****\n");
    // check the remains:
    memset (&tsimchk, 0, sizeof (tsimchk));
    tsimchk.cur_max = ma_size (&(ptsim->countlist));
    tsimchk.num_supertiles = ts_get_total_supertiles (ptsim);
    ptsim->cb_getdata    = tssim_cb_getdata_chkall;
    ptsim->cb_resultinfo = tssim_cb_resultinfo_chkall;
    ptsim->userdata = &tsimchk;
    ts_simulate_main (ptsim, name_sim);
#endif

    return 0;
}

int
ts_simulate_xmlfile_test (const char *fnxml_load, const char *fnxml_save)
{
    tssiminfo_t tsim;

    tssiminfo_init (&tsim);

    // add some tile into the list
    if (ts_sim_load_data_xml (&tsim, fnxml_load) < 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
        return -1;
    }

    ts_simulate (&tsim, "xmlfiletest");

    ts_sim_save_data_xml (&tsim, fnxml_save);

    tssiminfo_clear (&tsim);
    return 0;
}

#if USE_DBRECORD
#include "datasql.h"
#define TN_TEST "my6-tile-to-3x3square"

extern void * tsim_dbisqlite3_conn (const char *path, const char *dbname);

void
ts_simulate_test_db (void)
{
    tssiminfo_t tsim;
    tssiminfo_t *ptsim = &tsim;
    dbmergeinfo_t dbmerginfo;

    tssiminfo_init (&tsim);

    tsim_dbi_init ();
    if ((dbmerginfo.conn = tsim_dbisqlite3_conn ("./data/", "tsim.sdb")) == NULL) {
        if ((dbmerginfo.conn = tsim_dbisqlite3_conn ("../data/", "tsim.sdb")) == NULL) {
            if ((dbmerginfo.conn = tsim_dbisqlite3_conn ("../../data/", "tsim.sdb")) == NULL) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in not open database: ./data/tsim.sdb");
                goto end_tssimtstdb;
            }
        }
    }
    if (tsim_load_baseinfo_dbi (dbmerginfo.conn, TN_TEST, ptsim) < 0) {
        if (ts_sim_load_data_xml (ptsim, "./test/testdata2a.xml") < 0) {
            if (ts_sim_load_data_xml (ptsim, "../test/testdata2a.xml") < 0) {
                if (ts_sim_load_data_xml (ptsim, "../../test/testdata2a.xml") < 0) {
                    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
                    goto end_tssimtstdb;
                }
            }
        }
        tsim_save_baseinfo_dbi (dbmerginfo.conn, TN_TEST, ptsim);
    }
    dbmerginfo.id = ptsim->id;

    printf ("**** Start Simulation ****\n");
    // default:
    ptsim->cb_getdata       = NULL;
    ptsim->cb_selectone     = NULL;
    ptsim->cb_resultinfo    = tssim_cb_resultinfo_record_history_dbi;
    ptsim->cb_notifyfail    = tssim_cb_notifyfail_common;
    ptsim->cb_findmergepos  = tssim_cb_findmergepos_dbi;
    ptsim->cb_storemergepos = tssim_cb_storemergepos_dbi;
    ptsim->userdata         = &dbmerginfo;
    ts_simulate_main (ptsim);

    tsim_dbi_disconn (dbmerginfo.conn);
end_tssimtstdb:
    tsim_dbi_shutdown ();

    tssiminfo_clear (&tsim);
}
#endif /*USE_DBRECORD*/

static int
tssim_read_xml_tileitem (xmlDocPtr doc, xmlNodePtr cur_child, tstilecombitem_t *ptci, char *ret_flg_is2d)
{
    int ret = 0;
    xmlChar *key;
    tstilecombitem_t tci;

    memset (&tci, 0, sizeof (tci));
    for (; NULL != cur_child; cur_child = cur_child->next) {
        if (! xmlStrcmp (cur_child->name, (const xmlChar *)"tileid")) {
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            tci.idtile = atoi ((const char *)key);
            xmlFree (key);
        } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"rotnum")) {
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            tci.rotnum = atoi ((const char *)key);
            xmlFree (key);
        } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"x")) {
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            tci.pos.x = atoi ((const char *)key);
            xmlFree (key);
        } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"y")) {
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            tci.pos.y = atoi ((const char *)key);
            xmlFree (key);
        } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"z")) {
#if USE_THREEDIMENTION
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            tci.pos.z = atoi ((const char *)key);
            xmlFree (key);
            if (ret_flg_is2d) {
                *ret_flg_is2d = 0;
            }
#else
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR,
                "ERR: This version of the sss could only support 2D mode of the simulation, please use 3D/2D mode of the sss version."
                );
            ret = -1;
            break;

#endif
        }
    }
    memmove (ptci, &tci, sizeof (tci));
    return ret;
}

static void
tssim_tc_xml_save_tileitem (tstilecombitem_t *ptstci, size_t idx, char flg_is2d, FILE *fp_xml)
{
    fprintf (fp_xml,
        "      <id>%d</id>\n"
        "      <tileid>%d</tileid>\n"
        "      <rotnum>%d</rotnum>\n"
        "      <x>%d</x>\n"
        "      <y>%d</y>\n"
        , idx, ptstci->idtile, ptstci->rotnum, ptstci->pos.x, ptstci->pos.y
        );
#if USE_THREEDIMENTION
    if (0 == flg_is2d) {
        fprintf (fp_xml,
            "      <z>%d</z>\n"
            , ptstci->pos.z
            );
        }
#endif
}

static void
tssim_save_xml_supertile (tstilecomb_t *ptc, size_t idx, size_t birth, char * cmsg_birth, size_t cnt, char flg_is2d, FILE *fp_xml)
{
    size_t j;
    tstilecombitem_t tci;

    fprintf (fp_xml,
        "    <id>%d</id>\n"
        "    <quantity>%d</quantity>\n"
        , idx, cnt);

    fprintf (fp_xml, "    <birth>%d</birth>", birth);
    if (cmsg_birth) {
        fprintf (fp_xml, "    <!-- %s -->", cmsg_birth);
    }
    fprintf (fp_xml, "\n");
    for (j = 0; j < slist_size (&(ptc->tbuf)); j ++) {
        slist_data (&(ptc->tbuf), j, &tci);
        fprintf (fp_xml, "    <tileitem>\n");
        tssim_tc_xml_save_tileitem (&tci, j, flg_is2d, fp_xml);
        fprintf (fp_xml, "    </tileitem>\n");
    }
}

void
tstc_dump (const char * msg, size_t *pgluevec, size_t num_gluevec, tstile_t *ptilevec, size_t num_tilevec, tstilecomb_t *ptc)
{
    size_t i;
    tstilecombitem_t *ptstci;
    assert (NULL != ptc);
    fprintf (stderr, "%s (tstilecomb_t) {\n", (NULL == msg?"":msg));
    fprintf (stderr, "\tid=%d\n", ptc->id);
#if USE_THREEDIMENTION
    fprintf (stderr, "\tmaxpos={%d,%d,%d}\n", ptc->maxpos.x, ptc->maxpos.y, ptc->maxpos.z);
#else
    fprintf (stderr, "\tmaxpos={%d,%d}\n", ptc->maxpos.x, ptc->maxpos.y);
#endif
    for (i = 0; i < slist_size (&(ptc->tbuf)); i ++) {
        ptstci = (tstilecombitem_t *)slist_data_lock (&(ptc->tbuf), i);
        assert (ptstci->idtile < num_tilevec);
#if USE_THREEDIMENTION
        fprintf (stderr, "\t\ttileid[%d]{N(%d:%d),E(%d:%d),S(%d:%d),W(%d:%d),F(%d:%d),B(%d:%d),rot=%d,(%d,%d,%d)},\n"
            , ptstci->idtile
            , ptilevec[ptstci->idtile].glues[ORI_NORTH], pgluevec[ptilevec[ptstci->idtile].glues[ORI_NORTH]]
            , ptilevec[ptstci->idtile].glues[ORI_EAST],  pgluevec[ptilevec[ptstci->idtile].glues[ORI_EAST]]
            , ptilevec[ptstci->idtile].glues[ORI_SOUTH], pgluevec[ptilevec[ptstci->idtile].glues[ORI_SOUTH]]
            , ptilevec[ptstci->idtile].glues[ORI_WEST],  pgluevec[ptilevec[ptstci->idtile].glues[ORI_WEST]]
            , ptilevec[ptstci->idtile].glues[ORI_FRONT], pgluevec[ptilevec[ptstci->idtile].glues[ORI_FRONT]]
            , ptilevec[ptstci->idtile].glues[ORI_BACK],  pgluevec[ptilevec[ptstci->idtile].glues[ORI_BACK]]
            , ptstci->rotnum, ptstci->pos.x, ptstci->pos.y, ptstci->pos.z
            );
#else
        fprintf (stderr, "\t\ttileid[%d]{N(%d:%d),E(%d:%d),S(%d:%d),W(%d:%d),rot=%d,(%d,%d)},\n"
            , ptstci->idtile
            , ptilevec[ptstci->idtile].glues[ORI_NORTH], pgluevec[ptilevec[ptstci->idtile].glues[ORI_NORTH]]
            , ptilevec[ptstci->idtile].glues[ORI_EAST],  pgluevec[ptilevec[ptstci->idtile].glues[ORI_EAST]]
            , ptilevec[ptstci->idtile].glues[ORI_SOUTH], pgluevec[ptilevec[ptstci->idtile].glues[ORI_SOUTH]]
            , ptilevec[ptstci->idtile].glues[ORI_WEST],  pgluevec[ptilevec[ptstci->idtile].glues[ORI_WEST]]
            , ptstci->rotnum, ptstci->pos.x, ptstci->pos.y
            );
#endif
        slist_data_unlock (&(ptc->tbuf), i);
    }
    fprintf (stderr, "}\n");
}

#if DEBUG

static int
tssim_tc_clear (tssiminfo_testcase_t *ptstc)
{
    assert (NULL != ptstc);
    ma_clear (&(ptstc->poslist), (memarr_cb_destroy_t)NULL);
    return ma_clear (&(ptstc->stlist), (memarr_cb_destroy_t)tstc_clear);
}

static int
tssim_tc_init (tssiminfo_testcase_t *ptstc)
{
    memset (ptstc, 0, sizeof (*ptstc));
    ma_init (&(ptstc->poslist), sizeof (tstilecombitem_t));
    return ma_init (&(ptstc->stlist), sizeof (tstilecomb_t));
}

// the number position
static int
tssim_tc_posnum (tssiminfo_testcase_t *ptstc)
{
    assert (ma_size (&(ptstc->poslist)) == ma_size (&(ptstc->stlist)));
    return ma_size (&(ptstc->poslist));
}

// compare the result. is there exist such result?
static int
tssim_tc_search (tssiminfo_testcase_t *ptstc, tssiminfo_t *ptsim_ref, tstilecombitem_t *ptstci_pos, tstilecomb_t *ptstc_result)
{
    size_t i;
    int ret;
    tstilecombitem_t *ptstci_list;
    tstilecomb_t *ptstc_list;

    DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "the pos in list: %d", ma_size (&(ptstc->poslist)));
    DBGTS_SHOWPOS (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "compare the pos:", ptstci_pos->pos);
    for (i = 0; i < ma_size (&(ptstc->poslist)); i ++) {
        ptstci_list = (tstilecombitem_t *) ma_data_lock (&(ptstc->poslist), i);
        DBGTS_SHOWPOS (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "with list:", ptstci_list->pos);
        //if (0 == memcmp (ptstci_pos, ptstci_list, sizeof (*ptstci_list))) {
        if ( (ptstci_pos->rotnum == ptstci_list->rotnum) && TSPOS_ISEQUAL (ptstci_pos->pos, ptstci_list->pos) ) {
            // ok
            DBGTS_SHOWPOS (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "ok, pos is correct: ", ptstci_pos->pos);
            break;
        }
        ma_data_unlock (&(ptstc->poslist), i);
    }
    if (i >= ma_size (&(ptstc->poslist))) {
        DBGTS_SHOWPOS (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "not found the pos", ptstci_pos->pos);
        return -1;
    }
    ptstc_list = (tstilecomb_t *) ma_data_lock (&(ptstc->stlist), i);
    if (NULL == ptstc_list) {
        ma_data_unlock (&(ptstc->poslist), i);
        return -1;
    }
    ret = -1;
    if (tstc_is_equal (ptstc_list, ptstc_result, 1)) {
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "ok, the result");
        ret = 0;
#if DEBUG
    } else {
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "fail, result error");
        tstc_dump ("supertile list:", ptsim_ref->pgluevec, ptsim_ref->num_gluevec, ptsim_ref->ptilevec, ptsim_ref->num_tilevec, ptstc_list);
        tstc_dump ("supertile result:", ptsim_ref->pgluevec, ptsim_ref->num_gluevec, ptsim_ref->ptilevec, ptsim_ref->num_tilevec, ptstc_result);
#endif
    }
    ma_data_unlock (&(ptstc->poslist), i);
    ma_data_unlock (&(ptstc->stlist), i);
    return ret;
}

static int
tssim_tc_xml_load_item (tssiminfo_testcase_t *ptstc, char flg_is2d, char flg_norotate, xmlDocPtr doc, xmlNodePtr cur_child0)
{
    int ret = 0;
    size_t idx_new;
    xmlNodePtr cur_child;
    xmlChar *key;
    char flg_tileitem;
    char flg_supertile;

    for (; (ret >= 0) && (NULL != cur_child0); cur_child0 = cur_child0->next) {
        if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"supertileid_base")) {
            key = xmlNodeListGetString (doc, cur_child0->xmlChildrenNode, 1);
            ptstc->tileid_base = atoi ((const char *)key);
            xmlFree (key);
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load tileid_base %d", ptstc->tileid_base);
        } else if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"supertileid_test")) {
            key = xmlNodeListGetString (doc, cur_child0->xmlChildrenNode, 1);
            ptstc->tileid_test = atoi ((const char *)key);
            xmlFree (key);
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load tileid_test %d", ptstc->tileid_test);
        } else if (! xmlStrcmp (cur_child0->name, (const xmlChar *)"testposition")) {
            flg_tileitem = 0; // 预防多个 tileitem 情况
            flg_supertile = 0;
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load testposition");
            for (cur_child = cur_child0->xmlChildrenNode; NULL != cur_child; cur_child = cur_child->next) {
                if (! xmlStrcmp (cur_child->name, (const xmlChar *)"tileitem")) {
                    tstilecombitem_t *pitem_new;
                    if (flg_tileitem) {
                        // error, ignore
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "WARNING: multiply tileitem!");
                        continue;
                    }
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load tileitem");
                    flg_tileitem = 1;
                    idx_new = ma_inc (&(ptstc->poslist));
                    pitem_new = (tstilecombitem_t *)ma_data_lock (&(ptstc->poslist), idx_new);
                    memset (pitem_new, 0, sizeof (*pitem_new));
                    ret = tssim_read_xml_tileitem (doc, cur_child->xmlChildrenNode, pitem_new, NULL);
                    ma_data_unlock (&(ptstc->poslist), idx_new);
                    if (ret < 0) {
                        break;
                    }
#if USE_THREEDIMENTION
                    if (flg_is2d) {
                        pitem_new->pos.z = 1; // 在3维下模拟2维时，test一定是在该平面与base结合
                    }
#endif
                } else if (! xmlStrcmp (cur_child->name, (const xmlChar *)"supertile")) {
                    tstilecomb_t *pitem_new;
                    if (flg_supertile) {
                        // error, ignore
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "WARNING: multiply supertile!");
                        continue;
                    }
                    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load supertile");
                    flg_supertile = 1;
                    idx_new = ma_inc (&(ptstc->stlist));
                    pitem_new = (tstilecomb_t *)ma_data_lock (&(ptstc->stlist), idx_new);
                    tstc_init (pitem_new);
                    ret = ts_sim_lddata_rdsupertile (doc, cur_child->xmlChildrenNode, pitem_new, NULL, NULL, NULL);
                    if (0 == flg_norotate) {
#if USE_THREEDIMENTION
                        tstc_adjust_lower (pitem_new, flg_is2d);
#else
                        tstc_adjust_lower (pitem_new, 1);
#endif
                    }
                    ma_data_unlock (&(ptstc->stlist), idx_new);
                }
            }
        }
    }
    return ret;
}

static int
tssim_tc_xml_load (memarr_t *ptclist/*tssiminfo_testcase_t*/, char flg_is2d, char flg_norotate, xmlDocPtr doc, xmlNodePtr cur_child)
{
    ssize_t idx_new;
    tssiminfo_testcase_t *pitem_new;
    for (; NULL != cur_child; cur_child = cur_child->next) {
        if (! xmlStrcmp (cur_child->name, (const xmlChar *)"testcaseitem")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_LOG, "--XML--load testcaseitem");
            idx_new = ma_inc (ptclist);
            pitem_new = (tssiminfo_testcase_t *) ma_data_lock (ptclist, idx_new);
            tssim_tc_init (pitem_new);
            tssim_tc_xml_load_item (pitem_new, flg_is2d, flg_norotate, doc, cur_child->xmlChildrenNode);
            ma_data_unlock (ptclist, idx_new);
        }
    }
    return 0;
}

static int
tssim_tc_xml_save (memarr_t *ptclist, char flg_is2d, FILE *fp_xml)
{
    int ret;
    size_t i;
    size_t j;
    tstilecombitem_t *ptstci;
    tssiminfo_testcase_t *ptstc;

    assert (NULL != ptclist);
    ret = 0;
    DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "the number of test cases: %d", ma_size (ptclist));
    if (ma_size (ptclist) < 1) {
        return 0;
    }
    fprintf (fp_xml, "<testcases>\n");
    for (i = 0; i < ma_size (ptclist); i ++) {
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "save test case %d ...", i);
        fprintf (fp_xml, "<testcaseitem>\n");
        ptstc = (tssiminfo_testcase_t *) ma_data_lock (ptclist, i);
        if (NULL == ptstc) {
            ret = -1;
            DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "Unable to get data from testcase list.");
            break;
        }
        fprintf (fp_xml, "<supertileid_base>%d</supertileid_base>", ptstc->tileid_base);
        fprintf (fp_xml, "<supertileid_test>%d</supertileid_test>", ptstc->tileid_test);

        for (j = 0; j < ma_size (&(ptstc->poslist)); j ++) {
            tstilecomb_t *ptc;

            fprintf (fp_xml, "<testposition>\n");

            // tileitem
            ptstci = (tstilecombitem_t *) ma_data_lock (&(ptstc->poslist), j);
            fprintf (fp_xml, "    <tileitem>\n");
            tssim_tc_xml_save_tileitem (ptstci, 0, flg_is2d, fp_xml);
            fprintf (fp_xml, "    </tileitem>\n");

            ma_data_unlock (&(ptstc->poslist), j);

            // supertile
            fprintf (fp_xml, "  <supertile>\n");
            ptc = (tstilecomb_t *) ma_data_lock (&(ptstc->stlist), j);
            tssim_save_xml_supertile (ptc, 0, 0, NULL, 1, flg_is2d, fp_xml);
            ma_data_unlock (&(ptstc->stlist), j);
            fprintf (fp_xml, "  </supertile>\n");

            fprintf (fp_xml, "</testposition>\n");
        }
        ptstc = (tssiminfo_testcase_t *) ma_data_lock (ptclist, i);
        fprintf (fp_xml, "</testcaseitem>\n");
    }
    fprintf (fp_xml, "<testcases>\n");

    return 0;
}

int
tssim_testcase (tssiminfo_t *ptsim)
{
    // list_points_ok: 结合的位置信息。 tsstpos_t 类型的列表
    memarr_t list_points_ok;
    int ret;
    size_t i;
    size_t j;
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;
    tstilecombitem_t *ptstci;
    tssiminfo_testcase_t *ptstc;

    assert (NULL != ptsim);
#if 0 //DEBUG
    for (i = 0; i < ma_size (&(ptsim->tilelist)); i ++) {
        ptc_test = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
        tstc_dump ("ALL:", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, ptc_test);
    }
#endif

    ret = ma_init (&list_points_ok, sizeof (tsstpos_t));
    assert (ret >= 0);

    ret = 0;
    DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "the number of test cases: %d", ma_size (&(ptsim->tclist)));
    for (i = 0; i < ma_size (&(ptsim->tclist)); i ++) {
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "get test case %d ...", i);
        ptstc = (tssiminfo_testcase_t *) ma_data_lock (&(ptsim->tclist), i);
        if (NULL == ptstc) {
            ret = -1;
            DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "Unable to get data from testcase list.");
            break;
        }

        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "get base supertile %d ...", ptstc->tileid_base);
        ptc_base = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), ptstc->tileid_base);
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "get test supertile %d ...", ptstc->tileid_test);
        if (ptstc->tileid_test != ptstc->tileid_base) {
            ptc_test = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), ptstc->tileid_test);
            tstc_dump ("supertile test:", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, ptc_test);
        } else {
            ptc_test = ptc_base;
        }
        tstc_dump ("supertile base:", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, ptc_base);

        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "check (base %d:num %d) + (test %d:num %d) at year=%d"
            , ptstc->tileid_base, 1
            , ptstc->tileid_test, 1
            , ptsim->year_current);

        ma_rmalldata (&list_points_ok, NULL);
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "tstc_mesh_test() ...");
        assert ((ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_nature) || (ptsim->cb_mesh_test == (tssim_cb_mesh_test_t)tstc_mesh_test_godhand));
        ret = ptsim->cb_mesh_test (ptsim, ptc_base, ptc_test, &list_points_ok);

        // check the result
        for (j = 0; j < ma_size (&list_points_ok); j ++) {
            tstilecomb_t tc0;
            tstc_init (&tc0);
            ptstci = (tstilecombitem_t *) ma_data_lock (&list_points_ok, j);
            DBGTS_SHOWPOS (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_LOG, "check mesh pos: ", ptstci->pos);

            ret = tstc_merge (ptc_base, ptc_test, ptstci->rotnum, &(ptstci->pos), &tc0);
            // 对生成的 supertile 调整到其最低值
            if (0 == ptsim->flg_norotate) {
#if USE_THREEDIMENTION
                tstc_adjust_lower (&tc0, ptsim->flg_is2d);
#else
                tstc_adjust_lower (&tc0, 1);
#endif
            }
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "Error in merge two supertiles");
                ret = -1;
            } else if (tssim_tc_search (ptstc, ptsim, ptstci, &tc0) < 0) {
                DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "Error in search the supertile");
                ret = -1;
            }
            if (ret < 0) {
                DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "check the result failed at %d", j);
            }
            ma_data_unlock (&list_points_ok, j);
            tstc_clear (&tc0);
        }
        ma_data_unlock (&(ptsim->tilelist), ptstc->tileid_test);
        if (ptstc->tileid_test != ptstc->tileid_base) {
            ma_data_unlock (&(ptsim->tilelist), ptstc->tileid_base);
        }

        if (tssim_tc_posnum (ptstc) != ma_size (&list_points_ok)) {
            DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR
                , "the size of the result list in testcase(%d) != from calculating(%d)"
                , tssim_tc_posnum (ptstc), ma_size (&list_points_ok));
            //break;
            ret = -1;
        }
        ma_data_unlock (&(ptsim->tclist), i);
        if (ret < 0) {
            break;
        }
    }
    ma_clear (&list_points_ok, NULL);

    if (i < ma_size (&(ptsim->tclist))) {
        DBGMSG (PFDBG_CATLOG_TESTCASE, PFDBG_LEVEL_ERROR, "Error in check the test case.");
        return -1;
    }
    return 0;
}
#endif // DEBUG

#define STRCMP_STATIC(p1,p2_static) strncmp (p1, p2_static, sizeof(p2_static) - 1)

static int
memarr_cb_destroy_cstr (void * data)
{
    char *str;
    str = *((char **)data);
    free (str);
    return 0;
}

// return data1 - data2
static int
slist_cb_comp_glue (void *userdata, void * data1, void * data2)
{
    size_t idx1 = *((size_t *)data1);
    size_t idx2 = *((size_t *)data2);
    memarr_t *pma_str = (memarr_t *)userdata;
    char *str1;
    char *str2;
    ma_data (pma_str, idx1, &str1);
    ma_data (pma_str, idx2, &str2);
    return strcmp (str1, str2);
}

static int
slist_cb_comp_heter_gluelabel (void *userdata, void * data_pin, void * data2)
{
    memarr_t *pma_str = (memarr_t *)userdata;
    size_t idx2 = *((size_t *)data2);
    char *str2;
    ma_data (pma_str, idx2, &str2);
    return strcmp ((char *)data_pin, str2);
}

// search one of the label, if not found, then add it to the list
// return the index of the label in list
static ssize_t
label_search_or_add (memarr_t *pma_str, sortedlist_t *psl_glue, char *label)
{
    char *str;
    size_t idx;
    if (slist_find (psl_glue, label, slist_cb_comp_heter_gluelabel, &idx) < 0) {
        // not found
        str = strdup (label);
        assert (NULL != str);
        idx = ma_size (pma_str);
        ma_append (pma_str, &str);
        slist_store (psl_glue, &idx);
        //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "add label: '%s' to idx=%d", str, idx);
#if DEBUG
{
    char *tmp = NULL;
    assert (NULL != str);
    ma_data (pma_str, idx, &tmp);
    assert (tmp == str);
    assert (0 == strcmp (str, label));
    assert (ma_size (pma_str) == slist_size (psl_glue));
}
        //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "store glue '%s'", label);
#endif
    } else {
        // get the real idx
        slist_data (psl_glue, idx, &idx);
        //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "get label: '%s' at idx=%d", label, idx);
    }
    return idx;
}
#if defined(__MINGW32__)
#define strtok_r(a,b,c) strtok((a),(b))
#endif
// load the data from the TAS data file
static int
ts_sim_load_data_tastds (tssiminfo_t *ptsim, const char *fn_tas)
{
    size_t num_line = 0;
    char str[200];
    FILE *fp_tas = NULL;
    int ret = 0;
    tstile_t tile;
    size_t tmp;
    char flg_isseed = 0;
    char flg_is2d = 1;
    char * tilename = NULL;
    char * tilelabel = NULL;
    size_t tilegroup = 0;
    char * label = NULL;
    memarr_t ma_str; // the C string list, the glue name
    memarr_t ma_val; // the glue value
    sortedlist_t sl_glue; // the sorted glue list by glue name, contains only the glue string index
    char flg_glue_val[ORI_MAX];
    size_t glue_val[ORI_MAX];
    size_t glue_labelidx[ORI_MAX]; // the label index in the ma_str
    size_t stridx_cur = 0; // the current max index of the glue, before processing the current record.
    ssize_t idx_add;
    size_t sz_buf_tilevec = 0;
    tstilecombitem_t tstci;
    tstilecomb_t tstc;
    size_t i;
#if DEBUG
    char *glue_label[ORI_MAX]; // the reference of the label
#define DUP_GLUE_LABEL(dir, val) glue_label[dir]=strdup(val)
    memset (glue_label, 0, sizeof (glue_label));
#else
#define DUP_GLUE_LABEL(dir, val)
#endif

    assert (convert_color_value ("rgb(255,  30, 20)") == 0xFF1E14);
    assert (convert_color_value ("red") == 0xFF0000);
    assert (convert_color_value ("green") == 0x008000);
    assert (convert_color_value ("blue") == 0x0000FF);
    assert (convert_color_value ("darkgreen") == 0x006400);

    fp_tas = fopen (fn_tas, "r");
    if (NULL == fp_tas) {
        return -1;
    }

    tstc_init (&tstc);
    ma_init (&ma_str, sizeof (char *));
    ma_init (&ma_val, sizeof (size_t));
    slist_init (&sl_glue, sizeof (size_t), &ma_str/*userdata*/, slist_cb_comp_glue, slist_cb_swap_sizet);

    memset (glue_val, 0, sizeof (glue_val));
    memset (glue_labelidx, 0, sizeof (glue_labelidx));
    memset (flg_glue_val, 0, sizeof (flg_glue_val));
    memset (str, 0, sizeof (str));

    // set the default label ""
    label = strdup ("");
    idx_add = label_search_or_add (&ma_str, &sl_glue, label);
    i = 0;
    ma_replace (&ma_val, idx_add, &i);
    for (i = 0; i < ORI_MAX; i ++) {
        glue_labelidx[i] = idx_add;
    }
    stridx_cur = ma_size (&ma_str);

    for (; fgets (str, sizeof (str), fp_tas) != NULL; ) {
        char *brks;
        char *word;
        num_line ++;
        // strip trailing '\n' if it exists
        cstr_trim (str);
        cstr_stripblank (str);
        if (strlen (str) < 1) {
            continue;
        }
        word = strtok_r (str, " \n", &brks);
        if (NULL == word) {
            continue;
        }
        //printf("\n%s", str);
        if (0 == strcmp (word, "CREATE")) {
            // save the tile
            // check the glue
            //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "CREATE");
            for (i = 0; i < ORI_MAX; i ++) {
                if (0 == flg_glue_val[i]) {
                    continue;
                }
                if ((size_t)-1 == glue_val[i]) {
                    glue_val[i] = 0;
                }
                if (glue_labelidx[i] >= stridx_cur) {
                    //assert (ma_size (&ma_str) == ma_size (&ma_val));
                    ma_replace (&ma_val, glue_labelidx[i], &(glue_val[i]));
                    ma_data (&ma_str, glue_labelidx[i], &label);
                    //assert (ma_size (&ma_str) == ma_size (&ma_val));
                    //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "store glue '%s'=%d @ idx=%d; stridx_cur=%d", label, glue_val[i], glue_labelidx[i], stridx_cur);
                } else
#if 1 // DEBUG
                if (glue_val[i] > 0)
#endif
                {
                    // check the value of the glue
                    ma_data (&ma_val, glue_labelidx[i], &tmp);
                    if (tmp != glue_val[i]) {
                        // we get the same label and different glue value!
                        char *cstrtmp;
                        ma_data (&ma_str, glue_labelidx[i], &cstrtmp);
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Non-constant glue value! '%s': stored(%d), newadd(%d)", cstrtmp, tmp, glue_val[i]);
#if DEBUG
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Non-constant glue value! current label: '%s'", glue_label[i]);
#endif
                        break;
                    }
                }
#if DEBUG
                if (glue_label[i]) {
                    free (glue_label[i]);
                }
#endif
            }
            if (i < ORI_MAX) {
                // some error in checking the values
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Some errors in the checking of glue values.");
                ret = -1;
                break;
            }
            assert (ma_size (&ma_str) == ma_size (&ma_val));
            // save to tilevec
            memmove (tile.glues, glue_labelidx, sizeof (tile.glues));
            tile.name = tilename;
            tilename = NULL;
            tile.label = tilelabel;
            tilelabel = NULL;
            tile.group = tilegroup;
            tilegroup = 0;
            if (sz_buf_tilevec < ptsim->num_tilevec + 1) {
                void *newbuf;
                newbuf = realloc (ptsim->ptilevec, (ptsim->num_tilevec + 50) * sizeof (tstile_t));
                ptsim->ptilevec = (tstile_t *)newbuf;
                sz_buf_tilevec = ptsim->num_tilevec + 50;
            }
            memmove (&(ptsim->ptilevec[ptsim->num_tilevec]), &tile, sizeof(tile));
            ptsim->num_tilevec ++;
            if (0 != flg_isseed) {
                ptsim->idx_last = ptsim->num_tilevec - 1;
            }

            flg_isseed = 0;
            tilename = NULL;
            tilelabel = NULL;
            assert (ma_size (&ma_str) == ma_size (&ma_val));
            stridx_cur = ma_size (&ma_str);
            memset (&tile, 0, sizeof (tile));
            memset (glue_val, 0, sizeof (glue_val));
            memset (glue_labelidx, 0, sizeof (glue_labelidx));
            memset (flg_glue_val, 0, sizeof (flg_glue_val));
#if DEBUG
            memset (glue_label, 0, sizeof (glue_label));
#endif
            // set the default label ""
            idx_add = label_search_or_add (&ma_str, &sl_glue, "");
            for (i = 0; i < ORI_MAX; i ++) {
                glue_labelidx[i] = idx_add;
            }

        } else if (0 == strcmp (word, "TILENAME")) {
            label = strtok_r (NULL, " \n", &brks);
            if ((NULL != label) && (0 == strcmp (label, "SEED"))) {
                flg_isseed = 1;
            }
            if (NULL != label) {
                cstr_trim (label);
                if (NULL != tilename) {
                    free (tilename);
                    tilename = NULL;
                }
                if (strlen (label) > 0) {
                    tilename = strdup (label);
                }
            }
        } else if (0 == strcmp (word, "LABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                if (NULL != tilelabel) {
                    free (tilelabel);
                    tilelabel = NULL;
                }
                if (strlen (label) > 0) {
                    tilelabel = strdup (label);
                }
            }
        } else if (0 == strcmp (word, "NORTHBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_NORTH] = 0;
            } else {
                glue_val[ORI_NORTH] = atoi (word);
            }
            flg_glue_val[ORI_NORTH] = 1;
        } else if (0 == strcmp (word, "EASTBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_EAST] = 0;
            } else {
                glue_val[ORI_EAST] = atoi (word);
            }
            flg_glue_val[ORI_EAST] = 1;
        } else if (0 == strcmp (word, "SOUTHBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_SOUTH] = 0;
            } else {
                glue_val[ORI_SOUTH] = atoi (word);
            }
            flg_glue_val[ORI_SOUTH] = 1;
        } else if (0 == strcmp (word, "WESTBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_WEST] = 0;
            } else {
                glue_val[ORI_WEST] = atoi (word);
            }
            flg_glue_val[ORI_WEST] = 1;
#if USE_THREEDIMENTION
        } else if (0 == strcmp (word, "UPBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_FRONT] = 0;
            } else {
                glue_val[ORI_FRONT] = atoi (word);
                flg_glue_val[ORI_FRONT] = 1;
            }
            flg_is2d = 0;
        } else if (0 == strcmp (word, "DOWNBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_BACK] = 0;
            } else {
                glue_val[ORI_BACK] = atoi (word);
                flg_glue_val[ORI_BACK] = 1;
            }
            flg_is2d = 0;
#else
        } else if (0 == strcmp (word, "UPBIND")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Unable to support 3D data");
            ret = -1;
            break;
        } else if (0 == strcmp (word, "DOWNBIND")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Unable to support 3D data");
            ret = -1;
            break;
#endif
        } else if (0 == strcmp (word, "NORTHLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_NORTH] = idx_add;
            DUP_GLUE_LABEL (ORI_NORTH, label);
        } else if (0 == strcmp (word, "EASTLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_EAST] = idx_add;
            DUP_GLUE_LABEL (ORI_EAST, label);
        } else if (0 == strcmp (word, "SOUTHLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_SOUTH] = idx_add;
            DUP_GLUE_LABEL (ORI_SOUTH, label);
        } else if (0 == strcmp (word, "WESTLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_WEST] = idx_add;
            DUP_GLUE_LABEL (ORI_WEST, label);
#if USE_THREEDIMENTION
        } else if (0 == strcmp (word, "UPLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                //fprintf (stderr, "UPLABEL '%s'\n", label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_FRONT] = idx_add;
            DUP_GLUE_LABEL (ORI_FRONT, label);
        } else if (0 == strcmp (word, "DOWNLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                //fprintf (stderr, "DOWNLABEL '%s'\n", label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_BACK] = idx_add;
            DUP_GLUE_LABEL (ORI_BACK, label);
#endif

        //} else if (0 == strcmp (word, "CONCENTRATION")) {
        } else if (0 == strcmp (word, "TILECOLOR")) {
            label = strtok_r (NULL, "\n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                tilegroup = convert_color_value (label);
            }
        //} else if (0 == strcmp (word, "TEXTCOLOR")) {
        //} else if (0 == strcmp (word, "icon")) {
        }
    }
    if (ret < 0) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Error in line: %d @ file '%s'", num_line, fn_tas);
        goto end_ts_sim_load_data_tastds;
    }
#if USE_THREEDIMENTION
    ptsim->flg_is2d = flg_is2d;
#endif

    // save all of the glue information
    assert (ma_size (&ma_str) == ma_size (&ma_val));
    ptsim->num_gluevec = ma_size (&ma_val);
    ptsim->pgluevec = (size_t *)realloc (ptsim->pgluevec, ptsim->num_gluevec * sizeof (size_t));
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        ma_data (&ma_val, i, &tmp);
        ptsim->pgluevec[i] = tmp;
    }

    ptsim->num_tiles = 0;
    // generate the supertile
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        memset (&tstci, 0, sizeof (tstci));
        tstci.idtile = i;
        tstc_reset (&tstc);
        tstc_additem (&tstc, &tstci);
        tstc.id = 100000; // the number of the supertile
        ptsim->num_tiles += tstc.id;
        ts_add_supertile (ptsim, &tstc, NULL, NULL);
    }

end_ts_sim_load_data_tastds:
    fclose (fp_tas);
    tstc_clear (&tstc);
    ma_clear (&ma_str, memarr_cb_destroy_cstr);
    ma_clear (&ma_val, NULL);
    slist_clear (&sl_glue, NULL);
    return ret;
}

static int
ts_sim_load_data_tastdp (tssiminfo_t *ptsim, const char *fn_tas)
{
    size_t num_line = 0;
    int ret = 0;
    char str[200];
    FILE *fp_tas = NULL;

    fp_tas = fopen (fn_tas, "r");
    if (NULL == fp_tas) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Unable to open file: '%s'", fn_tas);
        return -1;
    }
    tssiminfo_reset (ptsim);
    ret = 0;
    ptsim->temperature = 2;
    for (; (fgets (str, sizeof(str), fp_tas) != NULL) && (ret >= 0); ) {
        char *brks;
        char *word;
        num_line ++;
        // strip trailing '\n' if it exists
        cstr_trim (str);
        cstr_stripblank (str);
        if (strlen (str) < 1) {
            continue;
        }
        word = strtok_r (str, " =", &brks);
        if (0 == strcmp (word, "SEED")) {
            // skip
        } else if (0 == strcmp (word, "Temperature")) {
            word = strtok_r (NULL, " =", &brks);
            ptsim->temperature = atoi (word);
        } else {
            // check the file name
            if ((0 != strstr (word, ".tds")) || (0 != strstr (word, ".TDS"))) {
                char *p;
                p = strrstr_len (fn_tas, strlen(fn_tas), "/");
                if (NULL == p) {
                    ret = ts_sim_load_data_tastds (ptsim, word);
                } else {
                    if (p - fn_tas < sizeof (str) - strlen(str)) {
                        assert (str == word);
                        memmove (word + (p - fn_tas) + 1, str, strlen(str) + 1);
                        memmove (word, fn_tas, p - fn_tas + 1);
                        ret = ts_sim_load_data_tastds (ptsim, word);
                    } else {
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "No enough buffer. p(0x%x)-fn_tas(0x%x)=%d", p, fn_tas, p - fn_tas);
                        ret = -1;
                    }
                }
            } else {
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unknown keywords in line: %d @ file '%s'. skip it", num_line, fn_tas);
            }
        }
    }
    fclose (fp_tas);
    if (ret >= 0) {
        ptsim->flg_norotate = 1;
        TSIM_SIM_ALGO (ptsim, TSIM_ALGO_ATAM);
    }
    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "return %d", ret);
    return ret;
}

//static int ts_sim_load_data_xml (tssiminfo_t *ptsim, const char *fn_xml);
static int ts_sim_load_data_tastdp (tssiminfo_t *ptsim, const char *fn_tas);

int
ts_sim_load_datafile (tssiminfo_t *ptsim, const char *fn_data)
{
    int ret = -1;
    if (strrstr_len (fn_data, strlen(fn_data), ".xml")) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "loading xml file '%s' ...", fn_data);
        ret = ts_sim_load_data_xml (ptsim, fn_data);
    } else if (strrstr_len (fn_data, strlen(fn_data), ".tdp")) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "loading TAS file '%s' ...", fn_data);
        ret = ts_sim_load_data_tastdp (ptsim, fn_data);
    } else {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Unknown suffix of the data file: '%s'.", fn_data);
        ret = -1;
    }
    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "return %d", ret);
    return ret;
}
