/******************************************************************************
 * Name:        tileog.c
 * Purpose:     Display the supertiles (2D mode) by OpenGL
 * Author:      Yunhui Fu
 * Created:     2009-08-22
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <string.h> // memset
#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "pfutils.h"
#include "pfdebug.h"
#include "tilestruct.h"
#include "glutfont.h"

#include "tileog.h"

#if USE_THREEDIMENTION
#include "tileog3d.c"

#else /* USE_THREEDIMENTION */

static unsigned char m_showcharlist[] = {
'0','1','2','3','4','5','6','7','8','9',
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
};
#define NM_IDX(idx) (m_showcharlist[(idx) % NUM_TYPE(m_showcharlist, unsigned char)])

static GLfloat g_color_table [][4] = {
    {0.40, 0.40, 0.40, 0.5}, /* the color of the body of tile */
    {0.00, 0.00, 0.00, 0.5}, /* black */
    {1.00, 1.00, 1.00, 0.5}, /* white*/
    {1.00, 0.00, 0.60, 0.5}, /* purple */
    {0.00, 0.00, 1.00, 0.5}, /* blue */
    {1.00, 0.80, 0.00, 0.5}, /* orange */
    {0.39, 1.00, 0.13, 0.5}, /* green */
    {1.00, 0.00, 0.00, 0.5}, /* red */
    {1.00, 0.00, 1.00, 0.5}, /* pink */
    {0.70, 0.70, 0.70, 0.5}, /* grey */
    {0.40, 1.00, 1.00, 0.5}, /* lightblue */
    {0.60, 0.40, 0.20, 0.5}, /* darkbrown */
    {1.00, 1.00, 0.13, 0.5}, /* yellow */
    {0.60, 1.00, 0.60, 0.5}, /* lightgreen */
    {0.8, 0.8, 0.8, 0.5},
    {0.8, 0.1, 0.6, 0.5},
    {0.2, 0.1, 0.7, 0.5},
    {0.6, 0.0, 0.6, 0.5},
    {0.8, 0.2, 0.8, 0.5},
    {0.1, 0.7, 0.2, 0.5},
    {0.7, 0.2, 0.3, 0.5},
    {0.2, 0.8, 0.5, 0.5},
    {0.8, 0.5, 0.4, 0.5},
    {0.5, 0.1, 0.9, 0.5},
    {0.40, 0.40, 0.40, 0.5}, /* the color of the body of tile */
};

#define OGL_CORD2SCRN_X(maxx, x) (x)
//#define OGL_CORD2SCRN_Y(maxy, y) (maxy - (y) - 1)
#define OGL_CORD2SCRN_Y(maxy, y) (y)

#define SZ_EDGE (20)
#define SZ_GLUE (2)

int g_maxx = 0;
int g_maxy = 0;
size_t g_page_cur = 0;

#define COLOR_BACK 0.5, 0.5, 0.5

// include the glViewport(), glMatrixMode (GL_PROJECTION)
int
ogl_reset_projection (int w, int h)
{
    g_maxx = w;
    g_maxy = h;

    glViewport (0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

#if USE_THREEDIMENTION
    //gluPerspective (45.0f, (GLfloat)w/h, 1.0, 100.0);
    glFrustum (-2, 2, -2, 2, 1, 100);
    glTranslatef (0, 0, -2);
#else
    glOrtho (0.0, w, 0.0, h, -1.0, 1.0);
#endif

    // clear
    glClearColor (COLOR_BACK, 0.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}

void
ogl_initgl (ogl_cb_postredisplay_t cb_postredisplay)
{
#if 0
    static const GLfloat light0_pos[4]   = { -50.0f, 50.0f, 0.0f, 0.0f };

    // white light
    static const GLfloat light0_color[4] = { 0.6f, 0.6f, 0.6f, 1.0f };

    static const GLfloat light1_pos[4]   = {  50.0f, 50.0f, 0.0f, 0.0f };

    // cold blue light
    static const GLfloat light1_color[4] = { 0.4f, 0.4f, 1.0f, 1.0f };
#endif
    /* remove back faces */
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    /* speedups */
    glEnable(GL_DITHER);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

#if 0
    /* light */
    //glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    //glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_color);
    //glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    //glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_color);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
    //glEnable(GL_LIGHTING);

    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);
#endif

    glutfont_init ();
}

void
ogl_clear (void)
{
}

void
ogl_drawother (void)
{
}

static void
ogl_draw_tile (int groupid, GLint x0, GLint y0, tssiminfo_t *ptsim, tstilecombitem_t *ptci)
{
    int glue_base;

    //GLint maxx = glutGet (GLUT_WINDOW_WIDTH);
    //GLint maxy = glutGet (GLUT_WINDOW_HEIGHT);

    x0 = x0 + ((GLint)(ptci->pos.x)) * SZ_EDGE;
    y0 = y0 + ((GLint)(ptci->pos.y)) * SZ_EDGE;

    glColor4fv (g_color_table[NUM_TYPE(g_color_table, GLfloat[4]) - 1 - (groupid % NUM_TYPE(g_color_table, GLfloat[4]))]);
    //glRecti (x0 + SZ_GLUE, y0 + SZ_GLUE, x0 + SZ_EDGE - SZ_GLUE, y0 + SZ_EDGE - SZ_GLUE);
    //glRecti (OGL_CORD2SCRN_X(g_maxx, x0), OGL_CORD2SCRN_Y(g_maxy, y0), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE));
    glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE));

    glue_base = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptci, 0, 0);
    glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
    glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE));

    glue_base = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptci, 1, 0);
    glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
    glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE));

    glue_base = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptci, 2, 0);
    glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
    glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE));

    glue_base = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptci, 3, 0);
    glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
    glRecti (OGL_CORD2SCRN_X(g_maxx, x0), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE));

    glColor4fv (g_color_table[2]); /*yellow*/
    glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE / 2 - 4), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE / 2 - 8));
    glutfont_print ("%c", NM_IDX(ptci->idtile));
}

static void
ogl_draw_glueinfo (GLint x0, GLint y0, tssiminfo_t *ptsim)
{
    size_t i;

    //GLint maxx = glutGet (GLUT_WINDOW_WIDTH);
    //GLint maxy = glutGet (GLUT_WINDOW_HEIGHT);

    // display all of values of glues
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        glColor4fv (g_color_table[(i + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
        glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, x0 + g_maxx - i * 20 - 40), OGL_CORD2SCRN_Y(g_maxy, y0 + g_maxy - 16));
        glutfont_print ("%d", ptsim->pgluevec[i]);
    }
}

// idx_in: the index of the ptsim->tilelstidx
void
ogl_draw_tilesim (tssiminfo_t *ptsim, size_t idx_in, char flg_draw_info)
{
    size_t i;
    size_t birth;
    size_t cnt;
    size_t idx_cur;
    tstilecomb_t *ptc;
    tstilecombitem_t tci;
    size_t centerx;
    size_t centery;
    int ret;

    if (idx_in >= TS_NUMTYPE_SUPERTILE(ptsim)) {
        return;
    }
    if (TS_UNSORTED_IDX (ptsim, idx_in, &idx_cur) < 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "unable to find the supertile: %d", idx_in);
    }
    //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_LOG, "draw the supertile: (%d) %d", idx_in, idx_cur);

    ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_cur);
    assert (NULL != ptc);

    centerx = 0;
    centery = 0;
    if (g_maxx > ptc->maxpos.x * SZ_EDGE) {
        centerx = (g_maxx - ptc->maxpos.x * SZ_EDGE) / 2;
    }
    if (g_maxy > ptc->maxpos.y * SZ_EDGE) {
        centery = (g_maxy - ptc->maxpos.y * SZ_EDGE) / 2;
    }

    for (i = 0; i < slist_size (&(ptc->tbuf)); i ++) {
        slist_data (&(ptc->tbuf), i, &tci);
        ogl_draw_tile (0, centerx, centery, ptsim, &tci);
    }
    ma_data_unlock (&(ptsim->tilelist), idx_cur);

if (flg_draw_info) {
    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->countlist)));
    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->birthlist)));

    birth = 0;
    ret = ma_data (&(ptsim->birthlist), idx_cur, &birth);
    assert (ret >= 0);
    ret = ma_data (&(ptsim->countlist), idx_cur, &cnt);
    assert (ret >= 0);

    assert (NUM_TYPE(g_color_table, GLfloat[4]) > 9);
    glColor4fv (g_color_table[9]); /*light grey*/
    //glRasterPos2i (0, 5);
    glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16));
    glutfont_print ("%s;%s;TEMP=%d;PAGE=% 3d/%d;BIRTH=%d;CNT=%d"
#if USE_THREEDIMENTION
        , (ptsim->flg_is2d?"2D":"3D")
#else
        , "2D"
#endif
        , (ptsim->flg_norotate?"NonRotat":"Rotatable")
        , ptsim->temperature
        , idx_in
        , TS_NUMTYPE_SUPERTILE(ptsim) - 1
        , birth
        , cnt);
}

if (flg_draw_info) {
    memset (&tci, 0, sizeof(tci));
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        tci.idtile = i;
        //ogl_draw_tile (0, g_maxx - SZ_EDGE, i * (SZ_EDGE + SZ_EDGE / 2), ptsim, &tci); // display the base tile at the right side of screen
        ogl_draw_tile (0, i * (SZ_EDGE + SZ_EDGE / 2), 0, ptsim, &tci);
    }

    ogl_draw_glueinfo (0,0,ptsim);
}

}

// can be used directly in glut
void
ogl_on_mouse (int button, int state, int x, int y)
{
#if USE_ANTTWEAKBAR
    //if (TwEventMouseButtonGLUT (button, state, x, y)) {
        // update the screen
        //if (g_cb_postredisplay) {
            //g_cb_postredisplay ();
        //}
        //return;
    //}
    TwMouseButtonID twbutton;
    TwMouseAction twaction;
    switch(button) {
    case GLUT_LEFT_BUTTON: /* move the light */
        twbutton = TW_MOUSE_LEFT;
        break;
    case GLUT_RIGHT_BUTTON: /* move the polygon */
        twbutton = TW_MOUSE_RIGHT;
        break;
    case GLUT_MIDDLE_BUTTON:
        twbutton = TW_MOUSE_MIDDLE;
        break;
    }
    switch (state) {
    case GLUT_DOWN:
        twaction = TW_MOUSE_PRESSED;
        break;
    case GLUT_UP:
        twaction = TW_MOUSE_RELEASED;
        break;
    }
    if (TwMouseButton (twaction, twbutton)) {
        if (g_cb_postredisplay) {
            g_cb_postredisplay ();
        }
    }
#endif // USE_ANTTWEAKBAR
}

void
ogl_on_mouse_motion (int x, int y)
{
#if USE_ANTTWEAKBAR
    if (TwMouseMotion (x, y)) {
        if (g_cb_postredisplay) {
            g_cb_postredisplay ();
        }
    }
#endif // USE_ANTTWEAKBAR
}

void
ogl_on_keyboard (unsigned char glutKey, int glutMod)
{
#if USE_ANTTWEAKBAR
    int kmod = 0;
    if( glutMod & GLUT_ACTIVE_SHIFT )
        kmod |= TW_KMOD_SHIFT;
    if( glutMod & GLUT_ACTIVE_CTRL )
        kmod |= TW_KMOD_CTRL;
    if( glutMod & GLUT_ACTIVE_ALT )
        kmod |= TW_KMOD_ALT;

    if( (kmod & TW_KMOD_CTRL) && (glutKey>0 && glutKey<27) )  // CTRL special case
        glutKey += 'a'-1;

    if (TwKeyPressed((int)glutKey, kmod)) {
        // update the screen
        if (g_cb_postredisplay) {
            g_cb_postredisplay ();
        }
    }
#endif // USE_ANTTWEAKBAR
}

#if USE_ANTTWEAKBAR
void
ogl_on_special ( int key, int x, int y )
{
    if (TwEventSpecialGLUT (key, x, y)) {
        // update the screen
        if (g_cb_postredisplay) {
            g_cb_postredisplay ();
        }
    }
}
#endif

void
ogl_reshape (int width, int height)
{
    g_maxx = width;
    g_maxy = height;

    ogl_reset_projection (width, height);

#if USE_ANTTWEAKBAR
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "TwWindowSize(%d, %d)", width, height);
    TwWindowSize (width, height);
#endif // USE_ANTTWEAKBAR
}

void
ogl_jump2page (tssiminfo_t *ptsim, size_t page)
{
    if (TS_NUMTYPE_SUPERTILE(ptsim) < 1) {
        page = 0;
    } else if (page + 1 >= TS_NUMTYPE_SUPERTILE(ptsim)) {
        page = TS_NUMTYPE_SUPERTILE(ptsim) - 1;
    }
    g_page_cur = page;
}

void
ogl_current_page_inc (tssiminfo_t *ptsim)
{
    if (g_page_cur + 1 < TS_NUMTYPE_SUPERTILE(ptsim)) {
        g_page_cur ++;
    } else {
        g_page_cur = TS_NUMTYPE_SUPERTILE(ptsim) - 1;
    }
}

void
ogl_current_page_dec (tssiminfo_t *ptsim)
{
    if (g_page_cur > 0) {
        g_page_cur --;
    } else {
        g_page_cur = 0;
    }
}

void
ogl_draw_current_tilesim (tssiminfo_t *ptsim, char flg_draw_info)
{
    ogl_draw_tilesim (ptsim, g_page_cur, flg_draw_info);
}

int
ogl_set_target_tilesim (tssiminfo_t *ptsim)
{
    size_t pos;
    if (ts_sim_search_target (ptsim, &pos) < 0) {
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "The target not found!");
        return -1;
    }
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "found the target @ %d", pos);
    g_page_cur = pos;
    return 0;
}

#if USE_PRESENTATION

int
tssim_cb_initadheredect_ogshow (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test)
{
    tssim_adhere_info_t * pinfo = (tssim_adhere_info_t *) userdata;
    assert (NULL != userdata);
    pinfo->tc_base = tc_base;
    pinfo->tc_test = tc_test;
    memmove (&(pinfo->info_test), pinfo_test, sizeof (*pinfo_test));
    assert (pinfo->info_test.rotnum == (pinfo->info_test.rotnum % 4));
    ma_init (&(pinfo->poslist), sizeof (tstilecombitem_t));
    return 0;
}

// flg_canadhere: 是否可以粘合，如果可以，则为1，否则为0
// pposinfo: 为 tstilecombitem_t 类型的数据指针，其中 x,y,z 表示 test 的位置(在预约好的放置方式上),rotnum 表示 glue的位置,在该方向上的 glue 可以和base上对应位置粘合。特别注意的是，其为相对于 test 被旋转后的位置。
int
tssim_cb_adherepos_ogshow (void *userdata, char flg_canadhere, tstilecombitem_t * pposinfo)
{
    tstilecombitem_t posinfo;
    tssim_adhere_info_t * pinfo = (tssim_adhere_info_t *) userdata;
    assert (NULL != userdata);

    memmove (&posinfo, pposinfo, sizeof (posinfo));
    // idtile被重定义为是否可以粘合的标记（0:不能粘合，1:可以粘合
    posinfo.idtile = flg_canadhere;
    ma_append (&(pinfo->poslist), &posinfo);
    return 0;
}

int
tssim_cb_clearadheredect_ogshow (void *userdata)
{
    tssim_adhere_info_t * pinfo = (tssim_adhere_info_t *) userdata;
    assert (NULL != userdata);

    ma_clear (&(pinfo->poslist), NULL);
    pinfo->tc_base = NULL;
    pinfo->tc_test = NULL;
    memset (&(pinfo->info_test), 0, sizeof (pinfo->info_test));
    return 0;
}

static void
ogl_draw_gluecircle (GLint x0, GLint y0, tssiminfo_t *ptsim, tstilecombitem_t *ptci)
{
    int glue_base;

    x0 = x0 + ((GLint)(ptci->pos.x)) * SZ_EDGE;
    y0 = y0 + ((GLint)(ptci->pos.y)) * SZ_EDGE;
    if (1 == ptci->idtile) {
        glue_base = 5;
    } else {
        glue_base = 2;
    }

    switch (ptci->rotnum) {
    case 0:
        glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
        glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE));
        break;

    case 1:
        glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
        glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE));
        break;

    case 2:
        glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
        glRecti (OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_EDGE - SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE));
        break;

    case 3:
        glColor4fv (g_color_table[(glue_base + 1) % NUM_TYPE(g_color_table, GLfloat[4])]);
        glRecti (OGL_CORD2SCRN_X(g_maxx, x0), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_GLUE), OGL_CORD2SCRN_X(g_maxx, x0 + SZ_GLUE), OGL_CORD2SCRN_Y(g_maxy, y0 + SZ_EDGE - SZ_GLUE));
        break;
    }
}

void
tssim_adhere_display_ogshow (tssim_adhere_info_t * pinfo, const char * msg)
{
    assert (NULL != pinfo);
    assert (NULL != pinfo->psim);
    tsposition_t tptmp;
    tstilecomb_t tctmp;
    tstilecombitem_t tcitmp;
    size_t i;
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;
    tstilecombitem_t tci;
    size_t centerx = 0;
    size_t centery = 0;

    tstc_init (&tctmp);
    ptc_test = pinfo->tc_test;
    if ((pinfo->info_test.rotnum) > 0) {
        memset (&tptmp, 0, sizeof (&tptmp));
        ptc_test = tstc_rotate3d (pinfo->tc_test, &tctmp, pinfo->info_test.rotnum, &tptmp);
    }
    ptc_base = pinfo->tc_base;

    centerx = 0;
    centery = 0;
    // find the center of the display
    if ((g_maxx / SZ_EDGE / 2) > ptc_test->maxpos.x + ptc_base->maxpos.x / 2) {
        centerx = (g_maxx / SZ_EDGE / 2) - ptc_test->maxpos.x - ptc_base->maxpos.x / 2;
    }
    if ((g_maxy / SZ_EDGE / 2) > ptc_test->maxpos.y + ptc_base->maxpos.y / 2) {
        centery = (g_maxy / SZ_EDGE / 2) - ptc_test->maxpos.y - ptc_base->maxpos.y / 2;
    }

    // display the base supertile
    for (i = 0; i < slist_size (&(ptc_base->tbuf)); i ++) {
        slist_data (&(ptc_base->tbuf), i, &tci);
        ogl_draw_tile (0, (centerx + ptc_test->maxpos.x) * SZ_EDGE, (centery + ptc_test->maxpos.y) * SZ_EDGE, pinfo->psim, &tci);
    }

    // display the test supertile
    for (i = 0; i < slist_size (&(ptc_test->tbuf)); i ++) {
        slist_data (&(ptc_test->tbuf), i, &tci);
        ogl_draw_tile (1, (centerx + pinfo->info_test.pos.x) * SZ_EDGE, (centery + pinfo->info_test.pos.y) * SZ_EDGE, pinfo->psim, &tci);
    }
    // display the tested glues
    for (i = 0; i < ma_size (&(pinfo->poslist)); i ++) {
        ma_data (&(pinfo->poslist), i, &tcitmp);
        ogl_draw_gluecircle (centerx * SZ_EDGE, centery * SZ_EDGE, pinfo->psim, &tcitmp);
    }

    tstc_clear (&tctmp);

    assert (NUM_TYPE(g_color_table, GLfloat[4]) > 9);
    glColor4fv (g_color_table[9]); /*light grey*/
    //glRasterPos2i (0, 5);
    glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16));
    glutfont_print ("%s;%s;TEMP=%d;"
#if USE_THREEDIMENTION
        , (pinfo->psim->flg_is2d?"2D":"3D")
#else
        , "2D"
#endif
        , (pinfo->psim->flg_norotate?"NonRotat":"Rotatable")
        , pinfo->psim->temperature);

    glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy / 2));
    glutfont_print ("%s", msg);

    memset (&tci, 0, sizeof(tci));
    for (i = 0; i < pinfo->psim->num_tilevec; i ++) {
        tci.idtile = i;
        //ogl_draw_tile (0, g_maxx - SZ_EDGE, i * (SZ_EDGE + SZ_EDGE / 2), pinfo->psim, &tci); // display the base tile at the right side of screen
        ogl_draw_tile (0, i * (SZ_EDGE + SZ_EDGE / 2), 0, pinfo->psim, &tci);
    }

    ogl_draw_glueinfo (0, 0, pinfo->psim);
}

#endif /* USE_PRESENTATION */

void ogl_mousesim_angle (int x, int y)
{
}

void
ogl_mousesim_pick (int x, int y)
{
}

void ogl_mousesim_translate (int x, int y)
{
}

void ogl_mousesim_motion (int x, int y)
{
}

#endif /* USE_THREEDIMENTION */
