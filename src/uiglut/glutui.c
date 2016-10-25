/******************************************************************************
 * Name:        tscli.c
 * Purpose:     Supertile Self-assembly Simulator GUI(fltk) version
 * Author:      Yunhui Fu
 * Created:     2009-03-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdlib.h> /* exit() */
#include <assert.h>

#include <string.h> // memset
#include <stdio.h> // printf
#include <getopt.h>

#include <GL/glut.h>

#include "pfutils.h"
#include "bitset.h"
#include "tilestruct.h"

#include "tileog.h"
#include "tilepresent.h"

//#define NUM_TYPE(p,t) (sizeof(p)/sizeof(t))

#define AUTHOR "Yunhui Fu"
#define TITLE  "TileSim"

#define COLOR_BACK 0.5, 0.5, 0.5

char        flg_inited = 0;
tssiminfo_t g_tsim;

#if USE_PRESENTATION
tsim_test_pres_t g_thinfo;
#endif

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
    //return 0;
}

void
cb_mouse (int button, int state, int x, int y)
{
    /* hack for 2 button mouse */
    if (button == GLUT_LEFT_BUTTON && glutGetModifiers() & GLUT_ACTIVE_SHIFT)
        button = GLUT_MIDDLE_BUTTON;
    ogl_on_mouse (button, state, x, y);

#if 0
    switch (button) {
    case GLUT_LEFT_BUTTON:
        switch (state) {
        case GLUT_DOWN:
            break;
        }
        break;
    //case GLUT_RIGHT_BUTTON:
    //    break;
    default:
        break;
    }
#endif
}

void
cb_motion (int x, int y) {
    //printf ("mouse motion: (%d,%d)\n", x, y);
    ogl_on_mouse_motion (x, y);
    //glutPostRedisplay ();
}

void
cb_display (void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if USE_PRESENTATION
    if (g_thinfo.flg_running) {
        ts_simulate_test_presentation_draw (&g_thinfo);
    } else
#endif
    {
        ogl_draw_current_tilesim (&g_tsim, 1);
        ogl_drawother ();
    }

    glutSwapBuffers ();
}

void
cb_keyboard (unsigned char key, int x, int y)
{
    int glutMod = glutGetModifiers();
    ogl_on_keyboard (key, glutMod);
    switch (key) {
    case 27: /* 'ESC' */
    case 'q':
        exit (0);
        break;
    case 'p': /* Previous item */
    case '<':
    case ',':
        ogl_current_page_dec (&g_tsim);
        glutPostRedisplay ();
        break;
    case 'n': /* Next item */
    case '>':
    case '.':
        ogl_current_page_inc (&g_tsim);
        glutPostRedisplay ();
        break;
    default:
        printf ("received key: %d\n", key);
        break;
    }
}

void
cb_reshape (int w, int h)
{
    ogl_reshape (w, h);
}

void
cb_idle (void)
{
#if USE_PRESENTATION
    if (g_thinfo.flg_running) {
        glutPostRedisplay ();
    }
#endif
}

#define TN_TST "testdatame"

static char * g_infile_list [] = {
    "testdata1a",
    "testdata1b",
    "testdata2a",
    "testdata2b",
    "testdata3a",
    "testdata3b",
    "testdata4a",
    "testdata4b",
};

static void
get_filename (int idx, char *ret_in, char *ret_out)
{
    sprintf (ret_in,  "%s.xml", g_infile_list [idx % NUM_TYPE (g_infile_list, char *)]);
    sprintf (ret_out, "%s_output.xml", g_infile_list [idx % NUM_TYPE (g_infile_list, char *)]);
}

static char * m_dir_list [] = {
    "test/",
    "../test/",
    "../../test/",
};

void
open_4sim (tssiminfo_t *ptsim, int idx)
{
    char buf1[300];
    char buf2[300];
    char *p1;
    char *p2;
    int i;
    int ret;

    assert (NULL != ptsim);
    for (i = 0; i < NUM_TYPE (m_dir_list, char *); i ++) {
        sprintf (buf1, "%s", m_dir_list[i]);
        p1 = buf1 + strlen (buf1);
        sprintf (buf2, "%s", m_dir_list[i]);
        p2 = buf2 + strlen (buf2);
        get_filename (idx, p1, p2);

        if (ts_sim_load_data_xml (ptsim, buf1) < 0) {
            DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
            continue;
        }

        ret = ts_simulate (ptsim, "glut");

        ts_sim_save_data_xml (ptsim, buf2);

        if (ret >= 0) {
            break;
        }
    }
    if (i >= NUM_TYPE (m_dir_list, char *)) {
        // error
        return;
    }
}

void
open_4run (tssiminfo_t *ptsim, int idx)
{
    char buf1[300];
    int i;

    assert (NULL != ptsim);
    for (i = 0; i < NUM_TYPE (m_dir_list, char *); i ++) {
        sprintf (buf1, "%s%s.xml", m_dir_list[i], g_infile_list [idx % NUM_TYPE (g_infile_list, char *)]);
        if (ts_sim_load_data_xml (ptsim, buf1) >= 0) {
            break;
        }
    }
    if (i >= NUM_TYPE (m_dir_list, char *)) {
        // error
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
        return;
    }
}

void
open_4show (tssiminfo_t *ptsim, int idx)
{
    char buf1[300];
    int i;

    assert (NULL != ptsim);
    for (i = 0; i < NUM_TYPE (m_dir_list, char *); i ++) {
        sprintf (buf1, "%s%s_output.xml", m_dir_list[i], g_infile_list [idx % NUM_TYPE (g_infile_list, char *)]);
        if (ts_sim_load_data_xml (ptsim, buf1) >= 0) {
            break;
        }
    }
    if (i >= NUM_TYPE (m_dir_list, char *)) {
        // error
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
        return;
    }
}

void
cb_menu_file (int choice)
{
    if (choice >= NUM_TYPE (g_infile_list, char *)) {
        return;
    }
    init_tilesim ();
    open_4sim (&g_tsim, choice);
}

void
cb_menu_file_origin (int choice)
{
    if (choice >= NUM_TYPE (g_infile_list, char *)) {
        return;
    }
    init_tilesim ();
    open_4run (&g_tsim, choice);
}

void
cb_menu_file_result (int choice)
{
    if (choice >= NUM_TYPE (g_infile_list, char *)) {
        return;
    }
    init_tilesim ();
    open_4show (&g_tsim, choice);
}

#if USE_DBRECORD
extern void ts_simulate_test_db (void);
#endif
#if USE_PRESENTATION
extern void ts_simulate_test_presentation (void);
int
tssim_test_cb_present_updatedisplay (void)
{
    //glutPostRedisplay ();
}
#endif

void
cb_menu_main (int choice)
{
    switch (choice) {
    case 0: /* "Run" */
        ts_simulate (&g_tsim, "glutrun");
        break;

    case 1: /* "Exit" */
        exit (0);
        break;
#if USE_DBRECORD
    case 2: /* "Test database" */
        ts_simulate_test_db ();
        break;
#endif
#if USE_PRESENTATION
    case 3: /* "Test presentation" */
        if (1 == g_thinfo.flg_running) {
            break;
        }
        ts_simulate_test_presentation_init (&g_thinfo, tssim_test_cb_present_updatedisplay, 100000);
        ts_simulate_test_presentation_new_thread (&g_thinfo);
        break;
#endif
    default:
        break;
    }
}

void
add_menu (void)
{
    int sm_main;

    int i;
    int sm_file;
    int sm_file_origin;
    int sm_file_result;
    sm_file = glutCreateMenu (cb_menu_file);
    for (i = 0; i < NUM_TYPE (g_infile_list, char *); i ++) {
        glutAddMenuEntry (g_infile_list[i], i);
    }
    sm_file_origin = glutCreateMenu (cb_menu_file_origin);
    for (i = 0; i < NUM_TYPE (g_infile_list, char *); i ++) {
        glutAddMenuEntry (g_infile_list[i], i);
    }
    sm_file_result = glutCreateMenu (cb_menu_file_result);
    for (i = 0; i < NUM_TYPE (g_infile_list, char *); i ++) {
        glutAddMenuEntry (g_infile_list[i], i);
    }

    sm_main = glutCreateMenu (cb_menu_main);
    glutAddSubMenu ("Load & Run",  sm_file);
    glutAddSubMenu ("Load Origin",  sm_file_origin);
    glutAddSubMenu ("Load Result", sm_file_result);
#if USE_DBRECORD
    glutAddMenuEntry ("Test database", 2);
#endif
#if USE_PRESENTATION
    glutAddMenuEntry ("Test presentation", 3);
#endif
    glutAddMenuEntry ("Run",  0);
    glutAddMenuEntry ("Exit", 1);

    glutAttachMenu (GLUT_RIGHT_BUTTON);
    glutPostRedisplay ();
}

static void init_parameter (int argc, char * argv[]);

void
init_system (int argc, char * argv[])
{
    init_tilesim ();
    init_parameter (argc, argv);

    ogl_initgl ((ogl_cb_postredisplay_t)glutPostRedisplay);
    glClearColor (COLOR_BACK, 0.0);
}

int
main (int argc, char * argv[])
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (100, 100);
    glutInitWindowSize (640, 480);
    glutCreateWindow (TITLE " - " AUTHOR);

    glutDisplayFunc (cb_display);
    glutKeyboardFunc (cb_keyboard);
    glutMouseFunc (cb_mouse);
    glutMotionFunc (cb_motion);
    glutPassiveMotionFunc (cb_motion);
    glutReshapeFunc (cb_reshape);
    glutIdleFunc (cb_idle);
    //TwGLUTModifiersFunc(glutGetModifiers);

    add_menu ();

    init_system (argc, argv);

    glutMainLoop ();
    ogl_clear ();
    return 0;
}

static void
version (FILE *out_stream)
{
    fprintf( out_stream, "tsglut\n");
    fprintf( out_stream, "Copyright 2009 Yunhui Fu (yhfudev@gmail.com)\n\n" );
}

static void
help (FILE *out_stream, const char *progname)
{
    static const char *help_msg[] = {
        "Glut version of the Supertile Self-assembly Simulator",
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

static void
init_parameter (int argc, char * argv[])
{
    int c;
    struct option longopts[]  = {
        { "dbglevel",   1, 0, 'd' },
        { "dbgcatlog",  1, 0, 'a' },
        { "version",    0, 0, 'V' },
        { "help",       0, 0, 501 },
        { "verbose",    0, 0, 'v' },
        { "dbglevel",   1, 0, 'd' },
        { "dbgcatlog",  1, 0, 'a' },
        { 0,            0, 0,  0  }
    };
    while ((c = getopt_long( argc, argv, "i:o:t:p:d:a:hascVLvx", longopts, NULL )) != EOF) {
        switch (c) {
        case 'V': version (stdout); exit(1); break;
        case 'v': break;
        case 'd': set_debug_level (atoi (optarg)); break;
        case 'a': set_debug_catlog (atoi (optarg)); break;
        default:
        case 501: help (stdout, argv[0]); exit(1); break;
        }
    }

}
