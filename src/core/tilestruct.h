/******************************************************************************
 * Name:        tilestruct.h
 * Purpose:     some core functions for the tiles
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef TILESTRUCT_H_INCLUDED
#define TILESTRUCT_H_INCLUDED

//#define USE_PRESENTATION   1 /*defined in Makefile*/

#include "cubeface.h"
#include "pfutils.h"
#include "pfdebug.h"

#define USE_TILESIM_ATAM 1

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

_PSF_BEGIN_EXTERN_C

#if USE_THREEDIMENTION
#define DBGTS_SHOWPOS(cat, level, const_cstr_msg, pos) DBGMSG (cat, level, const_cstr_msg "(%d,%d,%d)", pos.x, pos.y, pos.z)
#else
#define DBGTS_SHOWPOS(cat, level, const_cstr_msg, pos) DBGMSG (cat, level, const_cstr_msg "(%d,%d)", pos.x, pos.y)
#endif

#define SSSVER_MAJOR 1
#define SSSVER_MINOR 0
#define SSSVER_COPYRIGHT "Supertile Simulation System ver1.0 (C) 2008-2009 Yunhui Fu"

typedef size_t tsglue_t;

typedef struct _tstile_t {
    char * name; // utf8
    char * label; // utf8
    size_t group; // the group of this tile
    size_t glues[ORI_MAX]; // glue index
} tstile_t;

// the position of tiles
typedef struct _tsposition_t {
    ssize_t x;
    ssize_t y; /* the position of the tile */
#if USE_THREEDIMENTION
    ssize_t z;
#endif
} tsposition_t;

int slist_cb_comp_tspos (void * userdata, void * data1, void * data2);
int slist_cb_swap_tspos (void *userdata, void * data1, void * data2);

// return ptspos1 - ptspos2
int tspos_cmp (tsposition_t *ptspos1, tsposition_t *ptspos2);

#if USE_THREEDIMENTION
#define TSPOS_ADDSELF(a,b) \
  do { \
    (a).x += (b).x; \
    (a).y += (b).y; \
    (a).z += (b).z; \
  } while (0)
#define TSPOS_DELSELF(a,b) \
  do { \
    (a).x -= (b).x; \
    (a).y -= (b).y; \
    (a).z -= (b).z; \
  } while (0)

#define TSPOS_ISEQUAL(a,b) (((a).x == (b).x) && ((a).y == (b).y) && ((a).z == (b).z))

#define UPDATE_MAX_VAL(maxpos,pos) \
    if ((maxpos).x < (pos).x) { \
        (maxpos).x = (pos).x; \
    } \
    if ((maxpos).y < (pos).y) { \
        (maxpos).y = (pos).y; \
    } \
    if ((maxpos).z < (pos).z) { \
        (maxpos).z = (pos).z; \
    }

#define UPDATE_MIN_VAL(minpos,pos) \
    if ((minpos).x > (pos).x) { \
        (minpos).x = (pos).x; \
    } \
    if ((minpos).y > (pos).y) { \
        (minpos).y = (pos).y; \
    } \
    if ((minpos).z > (pos).z) { \
        (minpos).z = (pos).z; \
    }

#define UPDATE_MAX_AXI(maxpos,pos) \
    if ((maxpos).x <= (pos).x) { \
        (maxpos).x = (pos).x + 1; \
    } \
    if ((maxpos).y <= (pos).y) { \
        (maxpos).y = (pos).y + 1; \
    } \
    if ((maxpos).z <= (pos).z) { \
        (maxpos).z = (pos).z + 1; \
    }

#else

#define TSPOS_ADDSELF(a,b) \
  do { \
    (a).x += (b).x; \
    (a).y += (b).y; \
  } while (0)
#define TSPOS_DELSELF(a,b) \
  do { \
    (a).x -= (b).x; \
    (a).y -= (b).y; \
  } while (0)

#define TSPOS_ISEQUAL(a,b) (((a).x == (b).x) && ((a).y == (b).y))

#define UPDATE_MAX_VAL(maxpos,pos) \
    if ((maxpos).x < (pos).x) { \
        (maxpos).x = (pos).x; \
    } \
    if ((maxpos).y < (pos).y) { \
        (maxpos).y = (pos).y; \
    }

#define UPDATE_MIN_VAL(minpos,pos) \
    if ((minpos).x > (pos).x) { \
        (minpos).x = (pos).x; \
    } \
    if ((minpos).y > (pos).y) { \
        (minpos).y = (pos).y; \
    }

#define UPDATE_MAX_AXI(maxpos,pos) \
    if ((maxpos).x <= (pos).x) { \
        (maxpos).x = (pos).x + 1; \
    } \
    if ((maxpos).y <= (pos).y) { \
        (maxpos).y = (pos).y + 1; \
    }
#endif

typedef struct _tstilecombitem_t {
    tsposition_t pos;
    size_t idtile; /* index of the tstile_t list */
    char rotnum; /* the times of turn clockwise 90 */
} tstilecombitem_t;

// return ptstci1 - ptstci2
int tstci_cmp (tstilecombitem_t *ptstci1, tstilecombitem_t *ptstci2);

typedef struct _tstilecomb_t {
    size_t id;
    tsposition_t maxpos;
    sortedlist_t tbuf; /* the tiles(tstilecombitem_t) is place the list ordered by (x,y) */
    /* char *bmp; bitmap of the items, temporal use only */
} tstilecomb_t;

#define TSTC_NUM_TILE(ptc) (slist_size (&((ptc)->tbuf)))

int tstc_is_equal (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable);
int tstc_compare_full (tstilecomb_t *tc1, tstilecomb_t *tc2, char flg_nonrotatable);

int tstc_transfer (tstilecomb_t *tc_to, tstilecomb_t *tc_from);
tstilecomb_t * tstc_rotate3d (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, size_t rotnum, tsposition_t *ptrans);

/* merge tc_base with tc_test which is rotated by dir and is at the position ppos(x,y) adopt the slidetest coordinate system,
   the result is stored in tc_result */
int tstc_merge (tstilecomb_t *tc_base, tstilecomb_t *tc_test, size_t rotnum, tsposition_t *ppos, tstilecomb_t *tc_result);

int tstc_copy (tstilecomb_t *tc_to, tstilecomb_t *tc_from);

int tstc_init (tstilecomb_t *tc);
int tstc_clear (tstilecomb_t *tc);
int tstc_reset (tstilecomb_t *tc);
tstilecomb_t * tstc_create (void);
int tstc_destroy (tstilecomb_t *tc);

// add one item
int tstc_additem (tstilecomb_t *tc, tstilecombitem_t *ptci);

// nomalize the ptc: let the left bottom of the supertile segment move to the (0,0) position
int tstc_nomalize (tstilecomb_t *ptc);

int slist_cb_comp_heter_tstilecomb_hastile (void *userdata, void * data_pin, void * data2);
// if there exist tile
//int tstc_hastile (tstilecomb_t *ptc, tsposition_t *ptsp);
#define tstc_hastile(ptc, ptsp) (0 == slist_find (&((ptc)->tbuf), (ptsp), slist_cb_comp_heter_tstilecomb_hastile, NULL))

// maxtvec: the number of tile in the list ptilevec
int tile_get_glue (tstile_t *ptilevec, size_t maxtvec, tstilecombitem_t *ptsi, size_t dirglue, size_t rotnum);

// search the supertile in the list
int tile_list_search (memarr_t *ptilelist, tstilecomb_t *pneedle, char flg_nonrotatable, size_t *pos);

// the position records created by slide testing
#define tsstpos_t tstilecombitem_t

// the idx_base and idx_test will filled with the index num of the pcountlist array by the callback function.
// return 1: ok, there are parameters created and stored and return.
// return 0: no more data any more. the ts_simulate() will quit from the simulation loop.
// return -1: some error occur
typedef int (* tssim_cb_getdata_t) (void *userdata, const memarr_t *pcountlist, size_t *year_cur, size_t *idx_base, size_t *idx_test);

// idx_base, idx_test: are the index of the base supertile and test supertile seperately.
// idx_created: the index of the new supertile in the pcountlist/ptilelist, it may be the index of existed supertile.
// pstpos: the position of the test supertile, which is selected by the simulation system, could not be NULL
// ptc: if the type of the created supertile is exist, then ptc==NULL, otheriwse it is the detail of the created supertile
// plist_points_ok: the list of the positions that could place the test supertile. it may be NULL.
// return < 0: error, the simulation should exit
// return >= 0: ok, continue next simulation item
// the callback function could check the items of result list.
// one case is that the caller want to show some image in the user screen.
typedef int (* tssim_cb_resultinfo_t) (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok);

// return the idx of the position which will be used to place the test supertile
typedef int (* tssim_cb_selectone_t) (void *userdata/*, size_t idx_base, size_t idx_test*/, memarr_t *plist_points_ok, size_t *pidx_selected);

// notify the times of failed combination in a row
// cnt_fail: the times of failed combination in a row
// return < 0 if the program want to exit from loop
typedef int (* tssim_cb_notifyfail_t) (void *userdata, size_t cnt_fail);

// get the combination position information from other source(such as database or random selection)
// return < 0: if the infomation is not accessable; the caller should call tstc_mesh_test() again to calculate the positions.
// return == 0: if the information is in database; plist_points_ok will contains all of the positions (as the tstc_mesh_test() returned); if the items in the list is zero, then the two supertile could not combine.
typedef int (* tssim_cb_findmergepos_t) (void *userdata, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok);
#define tssim_cb_storemergepos_t tssim_cb_findmergepos_t

#if USE_PRESENTATION
// 以下3个回调函数是用于提示外部检测到的可以粘合的位置
// 1. 进入一个检测位置
// tc_base, tc_test: 分别为 base 和 test 的点数据记录列表, 该指针在检测期间有效
// temperature: 粘合温度
// rotnum 表示 test supertile 旋转次数之后再与 base 做合并测试
typedef int (* tssim_cb_initadheredect_t) (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test);

// 2. 当前检测到的可以粘合的一个glue位置
// flg_canadhere: 是否可以粘合，如果可以，则为1，否则为0
// pposinfo: 为 tstilecombitem_t 类型的数据指针，其中 x,y,z 表示 glue 所属 test tile 的绝对位置(在预约好的放置方式上),rotnum 表示 glue的位置,在该方向上的 glue 可以和base上对应位置粘合。特别注意的是，其为相对于 test 被旋转后的位置。
typedef int (* tssim_cb_adherepos_t) (void *userdata, char flg_canadhere, tstilecombitem_t * pposinfo);

// 3. 退出检测
// 调用该函数后，应该清除所有在 userdata 中存储的与本次检测相关的数据
typedef int (* tssim_cb_clearadheredect_t) (void *userdata);

#endif

/* test if two tilecomp can be merge; if ok, return 0 and the position(x,y); otherwise return -1 */
typedef int (* tssim_cb_mesh_test_t) (void /*tssiminfo_t*/ *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok);

typedef int (* tssim_cb_face_move_t) (int slide_face, tsposition_t * ppos_max, tsposition_t * ppos_cur);

typedef struct _tssiminfo_t {

    // part 1: share with the tstc_mesh_test() or/and tstc_plumb_test()
    char flg_norotate;      // can the test supertile be rotated?

    tssim_cb_mesh_test_t cb_mesh_test;
        // two choices:
        //   tstc_mesh_test_godhand, insert other supertiles into the holes of supertiles
        //   tstc_mesh_test_nature, mesh test

    tssim_cb_face_move_t cb_face_move;
    size_t max_faces;
    char *p_g_dir_stestofslide;

    int temperature;        // the current temperature of the system

    size_t year_current;    // the current number of loop (year)
    size_t steps_1year_min; // the minimum of the steps of one loop(year)
    size_t steps_1year_max; // if 0, then it's no restriction
    size_t num_tiles;       // the total number of the tiles in the supertiles

    size_t num_gluevec;   // the number of items of array pgluevec
    tsglue_t *pgluevec;     // the strength of each glue
                          // the values of tstile_t are the index valule

    size_t num_tilevec;   // the number of items of array ptilevec
    tstile_t *ptilevec;   // store the base tile data, the type is tstile_t
                          // the tstilecombitem_t.idx is the index

    tstilecomb_t *ptarget;// the target supertile, if it contains no tile(item) in it, then it's no target.
    size_t birth_target;  // the birth of the ptarget

    memarr_t tilelist;  // the attribute of each supertile, tstilecomb_t
    memarr_t countlist; // the number of each type of supertile (size_t)
    memarr_t birthlist; // the birth of first item of each type of supertile (size_t)
    // countlist 和 birthlist 中的数据都是和 tilelist 一一对应的

    sortedlist_t tilelstidx; // the index(size_t) of the supertiles in the ptilelist, sorted by the tstc_compare()
    // 这里存储的是各个tilelist中项的排好序的索引表，不包括 count 为0的项

    // 存储的是 tilelist 中 count 为 0 的索引
    sortedlist_t freedlist;  // the index(size_t) of the supertiles in the ptilelist which is no longer be used, it's freed and is removed from the tilelstidx. the birth is also cleard with 0.
    char flg_shrink_record;     // remove the supertile record which number is decrease to 0.

    ssize_t idx_last; // 记录最后形成的 supertile. 可以用于 flg_single_base_mode 模式下，以便一直在其之上贴 tile. -1 表示还没有开始形成
    char algorithm;  // attach one supertile(tile) to a single supertile for each step
    // 排序 supertile 需要给确定的两个 supertile 定位大小，要有一个确定性算法能评估其大小。
    // 比如两个st一样但rot不一样有可能被排序算法排到不同的位置
    // 用旋转自身的方式比较各个状态间的大小，看哪个被比较成最大（小）的，选择那个作为排序用的。注意此比较不调用 tstc_isequal()。但是时间？

    // 在flg_shrink_record为1和不为1时，都有如下几个对系统当前存在的 supertile 类型的操作：
    // insert, lock,unlock, update_count(to >0 or ==0, handle between lock&unlock), setbirth

    void * userdata; // the user defined parameters which are passed to the callback functions

    // part 2: only used in ts_simulate_main()
    size_t id;      // id from database;
    char flg_usedb; // 0: only in memory; 1: use database to store some information

    tssim_cb_getdata_t    cb_getdata;
    tssim_cb_selectone_t  cb_selectone;
    tssim_cb_resultinfo_t cb_resultinfo;
    tssim_cb_notifyfail_t cb_notifyfail;

    // cache function
    tssim_cb_findmergepos_t  cb_findmergepos; // get the combination position information from other source(such as database or random selection
    tssim_cb_storemergepos_t cb_storemergepos;

#if USE_PRESENTATION
    // this functions will be called for demo porpose.
    tssim_cb_initadheredect_t  cb_initadheredect;
    tssim_cb_clearadheredect_t cb_clearadheredect;
    tssim_cb_adherepos_t       cb_adherepos;
#endif

#if USE_THREEDIMENTION
    // the following parameters are used in 3D when compiled with 2D compatibility.
    char flg_is2d;          // 1: use 2D emulator
#endif

#if USE_TILESIM_ATAM
    // parameters for kTam model
    double Gmc;
    double Gse;
    double ratek;
#endif

#if DEBUG
    // used for test cases.
    memarr_t tclist; // type of tssiminfo_testcase_t, the list of the test cases.
#endif // DEBUG
} tssiminfo_t;

#define G_DIR_STESTOFSLIDE(ptsim,a,b) ((ptsim)->p_g_dir_stestofslide[(a) * ORI_MAX + (b)])

// 0: use tstc_mesh_test_nature; 1: use tstc_mesh_test_godhand
#define TSIM_MESHTEST_MODE(ptsim, mode) ((0 == (mode))?((ptsim)->cb_mesh_test = (tssim_cb_mesh_test_t)tstc_mesh_test_nature):((ptsim)->cb_mesh_test = (tssim_cb_mesh_test_t)tstc_mesh_test_godhand))

#define TS_NUMTYPE_SUPERTILE(ptsim) slist_size(&((ptsim)->tilelstidx))
// 获取排序后在 i 位置的tilelist实际位置
#define TS_UNSORTED_IDX(ptsim, i, pidx) slist_data (&((ptsim)->tilelstidx), i, pidx)
// 获取排序后在 i 位置的 count
#define TS_SORTED_COUNT(ptsim, i, pidx) ((slist_data (&((ptsim)->tilelstidx), i, pidx) < 0)?(-1):ma_data(&((ptsim)->countlist), *pidx, pidx))
// 获取排序后在 i 位置的 birth
#define TS_SORTED_BIRTH(ptsim, i, pidx) ((slist_data (&((ptsim)->tilelstidx), i, pidx) < 0)?(-1):ma_data(&((ptsim)->birthlist), *pidx, pidx))

// 0: use shrink mode, 1: close shrink mode
#define TSIM_RECORD_MODE(ptsim, mode) ((0 == (mode))?((ptsim)->flg_shrink_record = 1):((ptsim)->flg_shrink_record = 0))

#define TSIM_ALGO_2HATAM 0x01
#define TSIM_ALGO_ATAM 0x02
#define TSIM_ALGO_KTAM 0x03
#define TSIM_SIM_ALGO(ptsim, alg) ((ptsim)->algorithm = (alg))

extern const char * tssim_algo_type2cstr (int type);
extern int tssim_algo_cstr2type (const char *name);

// 查找 unsorted idx 在 sorted list 对应的位置
#if 0 //__GNUC__
inline int
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
#else
extern int tssim_unsorted2sorted_idx (tssiminfo_t *ptsim, size_t idx_unsorted, size_t *pidx_sorted);
#endif

/* test if two tilecomp can be merge; if ok, return 0 and the position(x,y); otherwise return -1 */
int tstc_mesh_test_godhand (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok);
int tstc_mesh_test_nature (tssiminfo_t *psim, tstilecomb_t *tc_base, tstilecomb_t *tc_test, memarr_t *plist_points_ok);

#if DEBUG

// 单个 testcase, 用于单元测试 tssim
// 测试在 tileid_base 和 tileid_test 之间可能的全部组合，poslist[i] 以及对应的 stlist[i] 分别表示
// test ST 可以在 poslist[i] 上 与 base 合成 stlist[i]
typedef struct _tssiminfo_testcase_t {
    size_t tileid_base;
    size_t tileid_test;
    memarr_t poslist; // type of tstilecombitem_t, the test supertile positions
    memarr_t stlist;  // type of tstilecomb_t, the result supertiles which use the poslist[i] to combine with test and base st.
} tssiminfo_testcase_t;

int tssim_testcase (tssiminfo_t *ptsim);
#else
#define tssim_testcase(ptsim)
#endif // DEBUG

int tssiminfo_init (tssiminfo_t *ptsim);
int tssiminfo_reset (tssiminfo_t *ptsim);
int tssiminfo_clear (tssiminfo_t *ptsim);

// get the totall number of the supertiles in the list
size_t ts_get_total_supertiles (tssiminfo_t *ptsim);

// 从ptsim->tilelstidx中查找与pneedle一样的supertile 的在 ptsim->tilelstidx 中的索引值
int tssim_search_sorted_supertile (tssiminfo_t *ptsim, tstilecomb_t *pneedle, size_t *ppos);

// the core function of simulation
int ts_simulate_main (tssiminfo_t *ptsim, const char *name_sim);

int ts_sim_load_data_xml (tssiminfo_t *ptsim, const char *fn_xml);
int ts_sim_save_data_xml (tssiminfo_t *ptsim, const char *fn_xml);

// load data from *.xml or *.tdp file
int ts_sim_load_datafile (tssiminfo_t *ptsim, const char *fn_data);

//int ts_sim_search_target (tssiminfo_t *ptsim, size_t * ppos);
//#define ts_sim_search_target(ptsim, ppos) (0 == tssim_search_sorted_supertile ((ptsim), ((tssiminfo_t *)ptsim)->ptarget, ppos)?(TS_UNSORTED_IDX(ptsim, *ppos, ppos)):(-1))
#define ts_sim_search_target(ptsim, ppos) (((ptsim)->ptarget)?tssim_search_sorted_supertile ((ptsim), ((tssiminfo_t *)ptsim)->ptarget, ppos):(-1))

// test function
int tssim_cb_notifyfail_common (void *userdata, size_t cnt_fail);

// 检查所有的项目
typedef struct _tssim_chkall_info_t {
    size_t idx_base;
    size_t idx_test;
    size_t cur_max;
    size_t num_supertiles; // the total number of the supertiles in the list
    char flg_rechk; // 是否中途产生了新的 supertile而必须重新扫描
    tssiminfo_t *ptsim; // used in timeofsquare
    double target_percent; // the simulation should end at the percentage of the target supertile
} tssim_chkall_info_t;
int tssim_cb_getdata_chkall (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test);
int tssim_cb_resultinfo_chkall (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok);
int tssim_cb_notifyfail_chkall (void *userdata, size_t cnt_fail);

int tssim_cb_getdata_timeofsquare (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test);
int tssim_cb_resultinfo_timeofsquare (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok);

int ts_simulate (tssiminfo_t *ptsim, const char *name_sim);

int ts_simulate_xmlfile_test (const char *fnxml_load, const char *fnxml_save);

// return ptspos1 - ptspos2
#if __GNUC__
inline int tspos_cmp (tsposition_t *ptspos1, tsposition_t *ptspos2) __attribute__((always_inline));
inline
#endif
int
tspos_cmp (tsposition_t *ptspos1, tsposition_t *ptspos2)
{
#if USE_THREEDIMENTION
    if (ptspos1->z > ptspos2->z) {
        return 1;
    }
    if (ptspos1->z < ptspos2->z) {
        return -1;
    }
#endif
    if (ptspos1->y > ptspos2->y) {
        return 1;
    }
    if (ptspos1->y < ptspos2->y) {
        return -1;
    }
    if (ptspos1->x > ptspos2->x) {
        return 1;
    }
    if (ptspos1->x < ptspos2->x) {
        return -1;
    }
    return 0;
}

_PSF_END_EXTERN_C

#include "tilesimatam.h"
#include "tilesimktam.h"

#endif // TILESTRUCT_H_INCLUDED
