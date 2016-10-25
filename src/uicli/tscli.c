/******************************************************************************
 * Name:        tscli.c
 * Purpose:     Supertile Self-assembly Simulator command line version
 * Author:      Yunhui Fu
 * Created:     2009-03-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdio.h>
#include <time.h>  
#include <getopt.h>
#include <assert.h>
#include <string.h> // strchr, memset
#include <time.h> // time()
#include <signal.h> // signal

#include "pfutils.h"
#include "pfrandom.h"
#include "pfdebug.h"
#include "bitset.h"
#include "tilestruct.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

char        flg_inited = 0;
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

// get the combination position information from other source(such as database or random selection
static int
tssim_cb_findmergepos_randomly (void *userdata, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok)
{
    tssim_chkall_info_t *pchkallinfo = (tssim_chkall_info_t *)userdata;
    tstilecomb_t *ptc_base;
    tsposition_t rangemax;
    tsstpos_t stpos;

    assert (NULL != pchkallinfo);
    assert (NULL != pchkallinfo->ptsim);

    assert (idx_base >= idx_test);
    assert (NULL != plist_points_ok);
    assert (0 == ma_size (plist_points_ok));

    if (pchkallinfo->ptsim->flg_norotate == 0) {
        stpos.rotnum = my_irrand (4);
    } else {
        stpos.rotnum = 0;
    }
    memset (&rangemax, 0, sizeof (rangemax));
    ptc_base = (tstilecomb_t *)ma_data_lock (&(pchkallinfo->ptsim->tilelist), idx_base);
    TSPOS_ADDSELF (rangemax, ptc_base->maxpos);
    if (idx_base != idx_test) {
        ma_data_unlock (&(pchkallinfo->ptsim->tilelist), idx_base);
        ptc_base = (tstilecomb_t *)ma_data_lock (&(pchkallinfo->ptsim->tilelist), idx_test);
    }
    TSPOS_ADDSELF (rangemax, ptc_base->maxpos);
    ma_data_unlock (&(pchkallinfo->ptsim->tilelist), idx_test);

    stpos.pos.x = my_irrand (rangemax.x);
    stpos.pos.y = my_irrand (rangemax.y);
#if USE_THREEDIMENTION
    stpos.pos.z = my_irrand (rangemax.z);
#endif
    return ma_insert (plist_points_ok, ma_size(plist_points_ok), &stpos);
}

static int
tssim_cb_notifyfail_chkall_mynew (void *userdata, size_t cnt_fail)
{
    tssim_chkall_info_t *ptci = (tssim_chkall_info_t *)userdata;
    assert (NULL != ptci);
    if (cnt_fail / 5 > ptci->num_supertiles) {
        //fprintf (stderr, "the count of failure > %d\n", ptci->num_supertiles * 3);
        if (ptci->ptsim->cb_findmergepos) {
            ptci->ptsim->cb_findmergepos = NULL;
            return 0;
        }
        if (cnt_fail / 1000 > ptci->num_supertiles) {
            fprintf (stderr, "the count of failure > %d\n", ptci->num_supertiles * 10);
            return -1;
        }
    }
    return 0;
}

static void
version (FILE *out_stream)
{
    fprintf( out_stream, "Supertile simulation system version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    fprintf( out_stream, "Copyright 2009 Yunhui Fu (yhfudev@gmail.com)\n\n" );
}

static void
help (FILE *out_stream, const char *progname)
{
    static const char *help_msg[] = {
        "Command line version of the supertile simulation system", 
        "",
        "-i --input  <file name>    specify the input xml data file",
        "-o --outprefix <prefix>    specify the prefix of the output xml file",
        "-t --temperature <temperature> replace the temperature of the system",
        "-e --expiretime <seconds>  the run time of the emulation",
        "-h --inserthole            fill the hole of the supertile with other supertiles",
        "-s --seqrandom             test each type of the ST with other ST random in each loop(year)",
        //"-a --parseall              parse all of the posible postion of two STs",
        "-p --percentage            percentage list of the wait list",
        "-c --checkresult           try to combine all of the ST at the end of test",
        "-r --recorditems           record all of the supertiles which appeared during sim.",
        "-g --algorithm             the algorithm of simulation: 2haTAM, aTAM, or kTAM",
#if DEBUG
        "-d --dbglevel <level>      debug level number, 0~5, 0-all, 1-error, 2-warning, 3-info, 4-debug, 5-log",
        "-a --dbgcatlog <catlog>    debug catlog number, 0~5, 0-all, 1-app, 2-3dbase, 3-tstc",
#endif
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

void
swap_buffer (uint8_t * data1, uint8_t * data2, size_t size)
{
    size_t i;
    uint8_t tmp;
    for (i = 0; i < size; i ++) {
        tmp = *data1;
        *data1 = *data2;
        *data2 = tmp;
        data1 ++;
        data2 ++;
    }
}

int
slist_cb_swap_double (void *userdata, void * data1, void * data2)
{
    swap_buffer ((uint8_t *)data1, (uint8_t *)data2, sizeof (double));
    return 0;
}

int
slist_cb_comp_double (void *userdata, void * data1, void * data2)
{
    double a = *((double *)data1);
    double b = *((double *)data2);;
    if (a > b) {
        return 1;
    } else if (a < b) {
        return -1;
    }
    return 0;
}

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

char *g_fn_out = NULL;

static char flg_freesys = 0;
void
clean_all (void)
{
    CHECK ();
    if (0 == flg_freesys) {
        flg_freesys = 1;
        clean_tilesim ();
        CHECK ();
        //slist_clear (&sl_percent, NULL);
        //CHECK ();
    }
}

void
do_atexit (void)
{
    if (0 == flg_freesys) {
        char fname[MAX_PATH];
        sprintf (fname, "%s_usrbrk.xml", g_fn_out);
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "save unsaved data to file '%s'.", fname);
        ts_sim_save_data_xml (&g_tsim, fname);
    }
    clean_all ();
}

static void
term (int sig)
{
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "received signal %d. Exiting...", sig);
    exit (0);
}

int
sig_proc (void)
{
#ifdef SIGHUP
    signal(SIGHUP,  term);
#endif
    signal(SIGINT,  term);
    signal(SIGTERM, term);
#ifdef SIGKILL
    signal(SIGKILL, term);
#endif
    return 0;
}

time_t g_time_end = 0;

int
tssim_cb_resultinfo_timeofsquare_expiretime (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    if (g_time_end > 0) {
        if (g_time_end < time (NULL)) {
            fprintf (stdout, "Time expired!\n");
            return -1;
        }
    }
    return tssim_cb_resultinfo_timeofsquare (userdata, idx_base, idx_test, pstpos, temperature, idx_created, ptc, plist_points_ok);
}

int
main (int argc, char *argv[])
{
    int c;
    struct option longopts[]  = {
        { "input",      1, 0, 'i' },
        { "outprefix",  1, 0, 'o' },
        { "temperature",1, 0, 't' },
        { "percentage", 1, 0, 'p' },
        { "expire",     1, 0, 'e' },
        { "inserthole", 0, 0, 'h' },
        { "seqrandom",  0, 0, 's' },
        //{ "parseall",   0, 0, 'a' },
        { "checkresult",0, 0, 'c' },
        { "version",    0, 0, 'V' },
        { "help",       0, 0, 501 },
        { "verbose",    0, 0, 'v' },
        { "recorditems",0, 0, 'r' },
        { "algorithm",  0, 0, 'g' },
#if DEBUG
        { "dbglevel",   1, 0, 'd' },
        { "dbgcatlog",  1, 0, 'a' },
#endif
        { 0,            0, 0,  0  }
    };
    int ret = 0;
    char *fn_in = NULL;
    char *percstr = NULL;
    char fname[MAX_PATH];
    size_t temperature = 0;
    size_t expiretime = 0; // the abort time after the start in seconds, 0: no affection
    double percentage = 0.0; // 0 - 100; 0 means the first emergence of the target supertile
    char flg_inserthole = 0;
    char flg_seqrandom = 0;
    char flg_checkresult = 0;
    char flg_recorditems = 0;
    int algorithm = 0;
    //char flg_parseall = 0;  // 0: do not test all of the posible combined positions, just use rand() to get the position and test
    tssiminfo_t *ptsim = NULL;
    tssim_chkall_info_t chkallinfo;
    //tssim_chkall_info_t *pchkallinfo = &chkallinfo;
    sortedlist_t sl_percent; // store the percentages
    size_t i;
#if DEBUG
    char flg_testcase = 0;
#define GETOPT_ARG "i:o:t:p:e:d:a:g:brhascVLv"
#else
#define GETOPT_ARG "i:o:t:p:e:g:rhascVLv"
#endif
    while ((c = getopt_long( argc, argv, GETOPT_ARG, longopts, NULL )) != EOF) {
        switch (c) {
        case 'i': fn_in = optarg; break;
        case 'o': g_fn_out = optarg; break;
        case 't': temperature = atoi (optarg); break;
        case 'p': percstr = optarg; break;
        case 'e':
            expiretime = atoi (optarg);
            if (expiretime > 0) {
                g_time_end = time(NULL) + expiretime;
            }
            break;
        case 'h': flg_inserthole = 1; break;
        case 's': flg_seqrandom = 1; break;
        //case 'a': flg_parseall = 1; break;
        case 'c': flg_checkresult = 1; break;
        case 'V': version (stdout); exit(1); break;
        case 'r': flg_recorditems = 1; break;
        case 'g': algorithm = tssim_algo_cstr2type (optarg); break;
        case 'v': break;
#if DEBUG
        case 'd': set_debug_level (atoi (optarg)); break;
        case 'a': set_debug_catlog (atoi (optarg)); break;
        case 'b': flg_testcase = 1; break;
#endif
        default:
        case 501: help (stdout, argv[0]); exit(1); break;
        }
    }
    if (optind == argc) {
        if ((NULL == fn_in) || (NULL == g_fn_out)) {
            version (stderr);
            fprintf (stderr, "Error: no input and output file.\n" );
            fprintf (stderr, "Use --help for help\n" );
            exit(1);
        }
    }

    init_seed ();

#ifdef MEMWATCH
    mwAutoCheck (/*1=ON*/1);
    mwSetOutFunc (myOutFunc);
#endif
    //int atexit(void (*function)(void));
    atexit (do_atexit);
    sig_proc ();

    init_tilesim ();
    ptsim = &g_tsim;
    CHECK ();

    slist_init (&sl_percent, sizeof (double), NULL, slist_cb_comp_double, slist_cb_swap_double);
    if (NULL == percstr) {
        percentage = 0.0;
        slist_store (&sl_percent, &percentage);
    } else {
        char *p = percstr - 1;
        for (; p != NULL; p = strchr(p, ',')) {
            p ++;
            percentage = atof (p);
            //fprintf (stderr, "[tscli] load percentage: '%s' = %f\n", p, percentage);
            if ((percentage < 0.0) || (percentage > 100.0)) {
                version (stderr);
                fprintf (stderr, "Parameter error: percentage should be in range [0.0 ~ 100.0].\n" );
                ret = 1;
                goto end_tscli;
            }
            slist_store (&sl_percent, &percentage);
        }
    }

    //fprintf (stderr, "[tscli] load file: %s\n", fn_in);
    if (ts_sim_load_datafile (ptsim, fn_in) < 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read data file");
        goto end_tscli;
    }
    CHECK ();

    if (flg_inserthole) {
        TSIM_MESHTEST_MODE (ptsim, 1);
    }
    if (flg_recorditems) {
        TSIM_RECORD_MODE (ptsim, 1);
    }
    TSIM_SIM_ALGO (ptsim, algorithm);

#if DEBUG
    if (flg_testcase) {
        if (tssim_testcase (ptsim) < 0) {
            fprintf (stderr, "Error in test case\n");
        } else {
            fprintf (stderr, "Test case OK!\n");
        }
        goto end_tscli;
    }
#endif

    // time step testing
    memset (&(chkallinfo), 0, sizeof (chkallinfo));
    chkallinfo.cur_max = ma_size (&(ptsim->countlist));
    chkallinfo.ptsim = ptsim;
    chkallinfo.num_supertiles = ts_get_total_supertiles (ptsim);

    ptsim->userdata = &chkallinfo;
    ptsim->cb_notifyfail = tssim_cb_notifyfail_chkall_mynew; //tssim_cb_notifyfail_chkall;
    ptsim->cb_resultinfo = tssim_cb_resultinfo_timeofsquare_expiretime;

    if (flg_seqrandom) {
        ptsim->cb_getdata = tssim_cb_getdata_timeofsquare;
    }
    //if (flg_parseall == 0) {
    //    ptsim->cb_findmergepos = tssim_cb_findmergepos_randomly;
    //}

    for (i = 0; i < slist_size (&sl_percent); i ++) {
        slist_data (&sl_percent, i, &percentage);
        CHECK ();

        fprintf (stderr, "[tscli] test percentage: %f\n", percentage);

        chkallinfo.target_percent = percentage;
        ptsim->birth_target = 0;

        ts_simulate_main (ptsim, "tscli");
        CHECK ();

        sprintf (fname, "%s_P%f.xml", g_fn_out, percentage);
        ts_sim_save_data_xml (ptsim, fname);
        CHECK ();
    }

    if ((0 == ret) && flg_checkresult) {
        fprintf (stderr, "**** Check the remains ****\n");
        memset (&(chkallinfo), 0, sizeof (chkallinfo));
        chkallinfo.cur_max = ma_size (&(ptsim->countlist));
        chkallinfo.ptsim = ptsim;
        chkallinfo.num_supertiles = ts_get_total_supertiles (ptsim);
        // check the remains:
        ptsim->cb_notifyfail = NULL;
        ptsim->cb_getdata    = tssim_cb_getdata_chkall;
        ptsim->cb_resultinfo = tssim_cb_resultinfo_chkall;
        ts_simulate_main (ptsim, "tsclichk");
        CHECK ();
    }

end_tscli:
    clean_all ();
    slist_clear (&sl_percent, NULL);
    return 0;
}
