
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h> // strchr, memset
#include <time.h> // time()

#include "pfutils.h"
#include "pfrandom.h"
#include "pfdebug.h"
#include "tilestruct.h"
#include "tilestruct2d.h"
#include "benchmrk.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

char flg_inited = 0;
tssiminfo_t g_tsim;

void
init_tilesim (void)
{
    if (0 == flg_inited) {
        flg_inited = 1;
        tssiminfo_init (&g_tsim);
    } else {
        tssiminfo_reset (&g_tsim);
    }
}

void
clean_tilesim (void)
{
    tssiminfo_clear (&g_tsim);
}

static void
version (FILE *out_stream)
{
    fprintf( out_stream, "test3d\n");
    fprintf( out_stream, "Copyright 2009 Yunhui Fu (yhfudev@gmail.com)\n\n" );
}

static void
help (FILE *out_stream, const char *progname)
{
    static const char *help_msg[] = {
        "Command line version of the test3d",
        "",
        "-d --dbglevel <level>      debug level number, 0~5, 0-all, 1-error, 2-warning, 3-info, 4-debug, 5-log",
        "-a --dbgcatlog <catlog>    debug catlog number, 0~5, 0-all, 1-app, 2-3dbase, 3-tstc",

        "-V --version               display version information",
        "   --help                  display this help",
        "-v --verbose               be verbose",
        0 };
   const char **p = help_msg;

   fprintf (out_stream, "Usage: %s [options]\n", progname);
   while (*p) fprintf (out_stream, "%s\n", *p++);
}

#ifdef MEMWATCH
static void myOutFunc(int c) { fprintf (stderr, "%c", (unsigned char)c); }
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#if MEMWATCH
static char flg_freesys = 0;

void
mwmem_check_supertile (void)
{
    fprintf (stderr, "CHECK() ...\n");
    CHECK ();
    if (flg_freesys == 0) {
        clean_tilesim ();
        fprintf (stderr, "CHECK() ... 2\n");
        CHECK ();
    }
}
#endif

#if USE_THREEDIMENTION
static int self_testing (tssiminfo_t *ptsim);
#else
#define self_testing(a) printf ("Unable to compile this test program under 2D mode\n");
#endif

int test_2d (tssiminfo_t *ptsim);

int
main (int argc, char *argv[])
{
    char *fn_in = "../../testcube_7_skeleton.xml";
    int c;
    struct option longopts[]  = {
        { "dbglevel",   1, 0, 'd' },
        { "dbgcatlog",  1, 0, 'a' },
        { "version",    0, 0, 'V' },
        { "help",       0, 0, 501 },
        { "verbose",    0, 0, 'v' },
        { 0,            0, 0,  0  }
    };
    int ret = 0;

    tssiminfo_t *ptsim = NULL;

    while ((c = getopt_long( argc, argv, "d:a:i:hVv", longopts, NULL )) != EOF) {
        switch (c) {
        case 'V': version (stdout); exit(1); break;
        case 'v': break;
        case 'd': set_debug_level (atoi (optarg)); break;
        case 'a': set_debug_catlog (atoi (optarg)); break;
        case 'i': fn_in = optarg; break;
        default:
        case 501: help (stdout, argv[0]); exit(1); break;
        }
    }

    init_seed ();

#ifdef MEMWATCH
    mwAutoCheck (/*1=ON*/1);
    mwSetOutFunc (myOutFunc);
    //int atexit(void (*function)(void));
    atexit (mwmem_check_supertile);
#endif

    init_tilesim ();
    ptsim = &g_tsim;
    CHECK ();

    //fprintf (stderr, "[tscli] load file: %s\n", fn_in);
    if (ts_sim_load_data_xml (ptsim, fn_in) < 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
        goto end_tscli;
    }
    CHECK ();

    test_2d (ptsim);

    ret = self_testing (ptsim);

end_tscli:
    clean_tilesim ();
    CHECK ();
#if MEMWATCH
    flg_freesys = 1;
#endif
    if (ret < 0) {
        return 1;
    }
    return 0;
}

///////////////////////////////////////////
// testing function

#if USE_THREEDIMENTION

#define MAX_TSTTIME 2
#define NUM_ORIG 100
#define NUM_TILE 200
#define NUM_ROT  100

#define MAX_COORD 50000

#define TSTC_ADD_TILE(ptstc, ptstci) \
do { \
    slist_store (&((ptstc)->tbuf), (ptstci)); \
    UPDATE_MAX_AXI((ptstc)->maxpos, (ptstci)->pos); \
} while (0)

extern tstilecomb_t * tstc_rotate3d_1 (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, size_t rotnum, tsposition_t *ptrans);

#define NUM_CUBE4X6X3 3
int g_cube4x6x3_rotnum[NUM_CUBE4X6X3];

// 一个 4x6x3 立方体的坐标旋转测试
int g_cube4x6x3[NUM_CUBE4X6X3][24][3] = {
// 坐标 (0,0,0) 在经过 rotnum={0,1,2,...23} 后的数据
{
    {0,0,0}, {0,3,0}, {3,5,0}, {5,0,0},
    {3,2,5}, {2,0,5}, {0,0,5}, {0,3,5},
    {0,2,3}, {2,5,3}, {5,0,3}, {0,0,3},
    {0,2,0}, {2,3,0}, {3,0,0}, {0,0,0},
    {5,2,0}, {2,0,0}, {0,0,0}, {0,5,0},
    {3,0,2}, {0,0,2}, {0,5,2}, {5,3,2},
},
// 坐标 (3,5,2) 在经过 rotnum={0,1,2,...23} 后的数据
{
    {3,5,2}, {5,0,2}, {0,0,2}, {0,3,2},
    {0,0,0}, {0,3,0}, {3,2,0}, {2,0,0},
    {5,0,0}, {0,0,0}, {0,2,0}, {2,5,0},
    {3,0,5}, {0,0,5}, {0,2,5}, {2,3,5},
    {0,0,3}, {0,5,3}, {5,2,3}, {2,0,3},
    {0,5,0}, {5,3,0}, {3,0,0}, {0,0,0},
},
// 坐标 (2,3,1) 在经过 rotnum={0,1,2,...23} 后的数据
{
    {2,3,1}, {3,1,1}, {1,2,1}, {2,2,1},
    {1,1,2}, {1,2,2}, {2,1,2}, {1,1,2},
    {3,1,1}, {1,2,1}, {2,1,1}, {1,3,1},
    {2,1,3}, {1,1,3}, {1,1,3}, {1,2,3},
    {2,1,2}, {1,3,2}, {3,1,2}, {1,2,2},
    {1,3,1}, {3,2,1}, {2,2,1}, {2,1,1},
},
};


// 以 -rotnum 和 -ptrans 的方式得到经过 rotnum 变换以及平移 ptrans 的位置的原始位置信息
// 如，使用 tstc_rotate3d() 从原始矩阵变换得到一个旋转后的矩阵后可以通过 tstc_rotate3d_reverse() 又得到原始矩阵：
// tstc_rotate3d (&tstc_origin, &tsti_rotated, rotnum, &pos_trans);
// tstc_rotate3d_reverse (&tsti_rotated, &tstc_origin2, rotnum, &pos_trans);
// assert (0 == tstc_comp (&tstc_origin, &tstc_origin2));
tstilecomb_t *
tstc_rotate3d_reverse (tstilecomb_t *buf_in, tstilecomb_t * buf_out, size_t rotnum, tsposition_t *ptrans)
{
    size_t i;
    tstilecombitem_t tstci;
    tstilecomb_t tstc;
    tsposition_t trans0;
    tstilecomb_t * ptstc;

    tstc_init (&tstc);
    memset (&(tstc.maxpos), 0, sizeof (tstc.maxpos));
    for (i = 0; i < slist_size(&(buf_in->tbuf)); i ++) {
        slist_data (&(buf_in->tbuf), i, &tstci);
        TSPOS_DELSELF (tstci.pos, *ptrans);
        assert ((0 <= tstci.pos.x) && (tstci.pos.x < MAX_COORD));
        assert ((0 <= tstci.pos.y) && (tstci.pos.y < MAX_COORD));
        assert ((0 <= tstci.pos.z) && (tstci.pos.z < MAX_COORD));
        slist_store (&(tstc.tbuf), &tstci);
        UPDATE_MAX_AXI (tstc.maxpos, tstci.pos);
    }
    //TSPOS_DELSELF (tstci.maxpos, *ptrans);
    memset (&trans0, 0, sizeof (trans0));
    ptstc = tstc_rotate3d (&tstc, buf_out, g_cubeface_rotnum_cur2all[rotnum][0], &trans0);
    tstc_clear (&tstc);
    return ptstc;
}

static int
self_testing (tssiminfo_t *ptsim)
{
    char flg_error = 0;   // if the test have errors?
    size_t cnt_error = 0; // the number of the error test module

    tstilecombitem_t tstci;
    tstilecomb_t tstc_origin[NUM_ORIG];
    tstilecomb_t tstc_4x6x3;
    tstilecomb_t tstc_return;
    tstilecomb_t tstc_return1;
    tsposition_t pos_tran0;
    tsposition_t pos_tran;
    tstilecombitem_t *ptstci;
    size_t rotnums[NUM_ROT];
    benchmark_t bm;
    size_t i;
    size_t j;
    //srand (time (NULL));
    srand (0);

    rottab_gen_default ();

    tstc_init (&tstc_4x6x3);
    tstc_init (&tstc_return);
    tstc_init (&tstc_return1);

    // test base
    memset (&pos_tran0, 0, sizeof(pos_tran0));
    pos_tran.x = rand () % (MAX_COORD / 10);
    pos_tran.y = rand () % (MAX_COORD / 10);
    pos_tran.z = rand () % (MAX_COORD / 10);
    TSPOS_ADDSELF (pos_tran0, pos_tran);
    if (1 != TSPOS_ISEQUAL (pos_tran0, pos_tran)) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error: TSPOS_ADDSELF() or TSPOS_ISEQUAL()");
        assert (1 == TSPOS_ISEQUAL (pos_tran0, pos_tran));
        return -1;
    }
    TSPOS_DELSELF (pos_tran0, pos_tran);
    memset (&pos_tran, 0, sizeof(pos_tran));
    if (1 != TSPOS_ISEQUAL (pos_tran0, pos_tran)) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error: TSPOS_DELSELF() or TSPOS_ISEQUAL()");
        assert (1 == TSPOS_ISEQUAL (pos_tran0, pos_tran));
        return -1;
    }

    // test rotate
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "tstc_init ...");
    memset (&pos_tran0, 0, sizeof(pos_tran0));
    pos_tran.x = rand () % (MAX_COORD / 10);
    pos_tran.y = rand () % (MAX_COORD / 10);
    pos_tran.z = rand () % (MAX_COORD / 10);

    for (j = 0; j < NUM_ORIG; j ++) {
        fprintf (stderr, "%d/%d     \r", j+1, NUM_ORIG);
        tstc_init (&(tstc_origin[j]));
    }
    fprintf (stderr, "\n");
    for (i = 0; i < NUM_ROT; i ++) {
        rotnums[i] = rand () % 24;
    }

    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "test TSTC_ADD_TILE ...");
    for (i = 0; i < NUM_ORIG; i ++) {
        fprintf (stderr, "%d/%d     \r", i+1, NUM_ORIG);
        for (j = 0; j < NUM_TILE; j ++) {
            tstci.pos.x = rand () % MAX_COORD;
            tstci.pos.y = rand () % MAX_COORD;
            tstci.pos.z = rand () % MAX_COORD;
            assert ((0 <= tstci.pos.x) && (tstci.pos.x < MAX_COORD));
            assert ((0 <= tstci.pos.y) && (tstci.pos.y < MAX_COORD));
            assert ((0 <= tstci.pos.z) && (tstci.pos.z < MAX_COORD));
            tstci.idtile = j;
            tstci.rotnum = rand () % 24;
            TSTC_ADD_TILE (&(tstc_origin[i]), &tstci);
        }
        tstc_nomalize (&(tstc_origin[i]));
    }
    fprintf (stderr, "\n");

#if 1
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "test tstc_rotate3d() ...");
    bm_start (&bm, MAX_TSTTIME);
    for (i = 0; i < MAX_TSTTIME; i ++) {
        //fprintf (stderr, "%d/%d     \r", i+1, MAX_TSTTIME);
        for (j = 0; j < NUM_ORIG; j ++) {
            tstc_reset (&tstc_return);
            tstc_rotate3d (&(tstc_origin[j]), &tstc_return, rotnums[i % NUM_ROT], &pos_tran);
        }
    }
    bm_end (&bm, "tstc_rotate3d");
    //fprintf (stderr, "\n");
    //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_WARNING, "tstc_rotate3d() cost time: %d", tm_end - tm_start);
#endif

#if DEBUG
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "test tstc_rotate3d_1() ...");
    //bm_start (&bm, MAX_TSTTIME);
    for (i = 0; i < MAX_TSTTIME; i ++) {
        fprintf (stderr, "%d/%d         \r", i+1, MAX_TSTTIME);
        for (j = 0; j < NUM_ORIG; j ++) {
            tstc_reset (&tstc_return);
            tstc_rotate3d_1 (&(tstc_origin[j]), &tstc_return, rotnums[i % NUM_ROT], &pos_tran);
        }
    }
    bm_end (&bm, "tstc_rotate3d_1");
    //fprintf (stderr, "\n");
    //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_WARNING, "tstc_rotate3d_1() cost time: %d", tm_end - tm_start);
#endif

#if DEBUG
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "test if tstc_rotate3d() == tstc_rotate3d_1() ...");
    for (i = 0; i < NUM_ORIG; i ++) {
        fprintf (stderr, "%d/%d     \r", i+1, NUM_ORIG);
        tstc_reset (&tstc_return);
        tstc_rotate3d (&(tstc_origin[i]), &tstc_return, rotnums[i % NUM_ROT], &pos_tran);
        tstc_rotate3d_1 (&(tstc_origin[i]), &tstc_return1, rotnums[i % NUM_ROT], &pos_tran);
        if (0 != tstc_compare_full (&tstc_return, &tstc_return1, 1)) {
            DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error: not equal, tstc_rotate3d() or tstc_rotate3d_1() or tstc_is_equal() or tstc_compare_full() have problem.");
            break;
        }
    }
    fprintf (stderr, "\n");
    if (i < NUM_ORIG) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "some errors in tstc_rotate3d() == tstc_rotate3d_1(). (i=%d)", i);
        cnt_error ++;
    } else {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "There's no problem in tstc_rotate3d() == tstc_rotate3d_1().");
    }
#endif

#if 1
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_INFO, "test tstc_rotate3d() & tstc_rotate3d_reverse() ...");
    for (i = 0; i < NUM_ORIG; i ++) {
        fprintf (stderr, "%d/%d     \r", i+1, NUM_ORIG);
        tstc_reset (&tstc_return);
        tstc_rotate3d (&(tstc_origin[i]), &tstc_return, rotnums[i % NUM_ROT], &pos_tran);

        //DBGTS_SHOWPOS (PFDBG_CATLOG_APP, PFDBG_LEVEL_DEBUG, "tstc_return.maxpos", tstc_return.maxpos);
        //DBGTS_SHOWPOS (PFDBG_CATLOG_APP, PFDBG_LEVEL_DEBUG, "pos_tran", pos_tran);
        assert (tstc_return.maxpos.x >= pos_tran.x);
        assert (tstc_return.maxpos.y >= pos_tran.y);
        assert (tstc_return.maxpos.z >= pos_tran.z);

        tstc_reset (&tstc_return1);
        tstc_rotate3d_reverse (&tstc_return, &tstc_return1, rotnums[i % NUM_ROT], &pos_tran);
        if (0 != tstc_compare_full (&(tstc_origin[i]), &tstc_return1, 1)) {
             DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error: test (%d) not equal, tstc_rotate3d() or tstc_is_equal() or tstc_compare_full() have problem.", i);
            break;
        }
    }
    fprintf (stderr, "\n");
    if (i < NUM_ORIG) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "some errors in tstc_rotate3d_reverse (tstc_rotate3d()). (i=%d)", i);
        cnt_error ++;
    } else {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "There's no problem in tstc_rotate3d_reverse (tstc_rotate3d()).");
    }
#endif

#if 1
    flg_error = 0;
    // 测试一个 4x6x3 矩形在tstc_rotate3d()变换之后的端点坐标是否与预先知道的坐标值相等。
    // g_cube4x6x3
#define START_ROTNUM 13
    for (i = 0; i < NUM_CUBE4X6X3; i ++) {
        tstci.pos.x = g_cube4x6x3[i][0][0];
        tstci.pos.y = g_cube4x6x3[i][0][1];
        tstci.pos.z = g_cube4x6x3[i][0][2];
        assert ((0 <= tstci.pos.x) && (tstci.pos.x < 4));
        assert ((0 <= tstci.pos.y) && (tstci.pos.y < 6));
        assert ((0 <= tstci.pos.z) && (tstci.pos.z < 3));
        tstci.idtile = i ; // id为数组中点位置数据的索引
        g_cube4x6x3_rotnum[i] = rand () % 24;
        tstci.rotnum = g_cube4x6x3_rotnum[i]; //(START_ROTNUM % 24);
        TSTC_ADD_TILE (&tstc_4x6x3, &tstci);
    }
    for (i = 0; i < 24; i ++) {
        fprintf (stderr, "------ round %d/%d     \r", i+1, 24);

        // 从原始位置0变换到24种其他状态，看是否坐标值和状态正确
        tstc_reset (&tstc_return);
        tstc_rotate3d (&tstc_4x6x3, &tstc_return, i, &pos_tran0);

        assert (NUM_CUBE4X6X3 == slist_size (&(tstc_return.tbuf)));
        for (j = 0; j < slist_size (&(tstc_return.tbuf)); j ++) {
            ptstci = (tstilecombitem_t *)slist_data_lock (&(tstc_return.tbuf), j);
            if (ptstci->pos.x != g_cube4x6x3[ptstci->idtile][i][0]) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ERR: (%d) x(%d)!=origin_x(%d)", ptstci->idtile, ptstci->pos.x, g_cube4x6x3[ptstci->idtile][i][0]);
                flg_error = 1;
            }
            if (ptstci->pos.y != g_cube4x6x3[ptstci->idtile][i][1]) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ERR: (%d) y(%d)!=origin_y(%d)", ptstci->idtile, ptstci->pos.y, g_cube4x6x3[ptstci->idtile][i][1]);
                flg_error = 1;
            }
            if (ptstci->pos.z != g_cube4x6x3[ptstci->idtile][i][2]) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ERR: (%d) z(%d)!=origin_z(%d)", ptstci->idtile, ptstci->pos.z, g_cube4x6x3[ptstci->idtile][i][2]);
                flg_error = 1;
            }
            if (i != g_cubeface_rotnum_cur2all[g_cube4x6x3_rotnum[ptstci->idtile]][ptstci->rotnum]) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ERR: (%d) trans_code(%d)!=origin_rotnum(%d)", ptstci->idtile, i, g_cubeface_rotnum_cur2all[g_cube4x6x3_rotnum[ptstci->idtile]][ptstci->rotnum]);
                flg_error = 1;
            }
            slist_data_unlock (&(tstc_return.tbuf), j);
            if (flg_error) {
                break;
            }
        }
        if (flg_error) {
            break;
        }
        tstc_reset (&tstc_return);
        tstc_rotate3d (&tstc_4x6x3, &tstc_return, i, &pos_tran);
        tstc_reset (&tstc_return1);
        tstc_rotate3d_reverse (&tstc_return, &tstc_return1, i, &pos_tran);
        if (0 != tstc_compare_full (&tstc_4x6x3, &tstc_return1, 1)) {
            DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ERR: rotation error at rotnum(%d)", i);
            flg_error = 1;
            break;
        }
    }
    fprintf (stderr, "\n");
    if (flg_error) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "some errors in testing tstc_rotate3d(CUBE4X6X3). (i=%d)", i);
        cnt_error ++;
    } else {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "There's no problem in testing tstc_rotate3d(CUBE4X6X3)");
    }
#endif

    // test tstc_adjust_lower()
    extern int tstc_adjust_lower (tstilecomb_t *ptstc, char flg_is2d);
    extern void tstc_dump (const char * msg, size_t *pgluevec, size_t num_gluevec, tstile_t *ptilevec, size_t num_tilevec, tstilecomb_t *ptc);
    tstc_dump ("before adjust lower:", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, &tstc_4x6x3);
    tstc_adjust_lower (&tstc_4x6x3, 0);
    tstc_dump ("after adjust lower:", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, &tstc_4x6x3);
    // test tstc_transfer()
    extern int ts_tilelist_st_new (tssiminfo_t *ptsim, size_t *pret_idx, tstilecomb_t **pret_ptc);
    extern int ts_add_supertile (tssiminfo_t *ptsim, tstilecomb_t *ptc_new_2b_transfered, size_t *pret_newid, size_t *pret_newcount);
    extern int ts_update_count (tssiminfo_t *ptsim, size_t idx, size_t count);

    //tstilecomb_t *ptc = NULL;
    ts_update_count (ptsim, 0, 0);
    //if (ts_tilelist_st_new (ptsim, &i, &ptc) >= 0) {
        //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "testing tstc_transfer() ...");
        //tstc_transfer (ptc, &tstc_4x6x3);
        //ma_data_unlock (&(ptsim->tilelist), i);
    //}
    size_t newid;
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "testing ts_add_supertile() ...");
    tstc_4x6x3.id = 5;
    ts_add_supertile (ptsim, &tstc_4x6x3, &newid, NULL);
    tstc_dump ("after ts_add_supertile():", ptsim->pgluevec, ptsim->num_gluevec, ptsim->ptilevec, ptsim->num_tilevec, &tstc_4x6x3);
    // test
    //ts_update_count (ptsim, newid, 0);

    tstc_clear (&tstc_4x6x3);
    tstc_clear (&tstc_return);
    tstc_clear (&tstc_return1);
    for (j = 0; j < NUM_ORIG; j ++) {
        tstc_clear (&(tstc_origin[j]));
    }
    if (cnt_error > 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "some errors in testing (number of errors=%d)", cnt_error);
        return 0;
    } else {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "There's no any problem in testing!");
    }
    return -1;
}

#endif // USE_THREEDIMENTION

int
test_2d (tssiminfo_t *ptsim)
{
    size_t i;
    size_t j;
    size_t k;
    size_t m;
    tstilecombitem_t * ptsi_test;
    tstilecomb_t *tc_base;
    tstilecomb_t tc1;
    tstilecomb_t tc2;
    int glue_base;
    int glue_test;
    tsposition_t tpos_tran;

    tstc_init (&tc1);
    tstc_init (&tc2);

    // parse each of the supertiles in the list
    for (i = 0; i < ma_size (&(ptsim->tilelist)); i ++) {
        printf ("% 3d: ", i);
        tc_base = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
        // test tile_get_glue() with tile_get_glue_2d()
        for (j = 0; j < slist_size (&(tc_base->tbuf)); j ++) {
            ptsi_test = (tstilecombitem_t *) slist_data_lock (&(tc_base->tbuf), j);
            for (k = 0; k < 4; k ++) {
                for (m = 0; m < 4; m ++) {
                    //printf ("g");
                    glue_base = tile_get_glue_2d (ptsim->ptilevec, ptsim->num_tilevec, ptsi_test, k, m);
                    glue_test = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptsi_test, k, m);
                    assert (glue_base == glue_test);
                }
            }
            slist_data_unlock (&(tc_test->tbuf), j);
        }
        // test tstc_rotate() with tstc_rotate_2d()
        // tstilecomb_t * tstc_rotate_2d (tstilecomb_t *buf_orig, tstilecomb_t * buf_new, int num, tsposition_t *ptrans);
        for (j = 0; j < 4; j ++) {
            for (k = 0; k < 255; k ++)
            for (m = 0; m < 255; m ++) {
                //printf ("r");
                memset (&tpos_tran, 0, sizeof (tpos_tran));
                tpos_tran.x = k;
                tpos_tran.y = m;
                tstc_reset (&tc1);
                tstc_reset (&tc2);
                assert (tstc_is_equal (&tc1, &tc2, 1));
                tstc_rotate_2d (tc_base, &tc1, j, &tpos_tran);
                tstc_rotate3d (tc_base, &tc2, j, &tpos_tran);
#if USE_THREEDIMENTION
                if (ptsim->flg_is2d) {
                    tc1.maxpos.z = 1;
                }
#endif
                assert (tstc_is_equal (&tc1, &tc2, 1));
            }
        }
        ma_data_unlock (&(ptsim->tilelist), i);
        printf ("\n");
    }

    tstc_clear (&tc1);
    tstc_clear (&tc2);
}
