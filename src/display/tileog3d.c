/******************************************************************************
 * Name:        tileog3d.c
 * Purpose:     Display the supertiles (3D mode) by OpenGL
 * Author:      Yunhui Fu
 * Created:     2009-08-22
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <string.h> // memset
#include <assert.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glext.h> // GL_MULTISAMPLE_ARB
#include <GL/glu.h>
#include <GL/glut.h>

#define USE_ANTTWEAKBAR 1

#if USE_ANTTWEAKBAR
#include "AntTweakBar.h"
#endif

#include "ft2wrap.h"

#include "pfutils.h"
#include "pfdebug.h"
#include "tilestruct.h"
#include "glutfont.h"

#include "tileog.h"

#define ALPHA_VAL 0.85

static GLfloat g_color_table [][4] = {
    {0.40, 0.40, 0.40, ALPHA_VAL}, /* the color of the body of tile */
    {0.70, 0.70, 0.70, ALPHA_VAL}, /* black */
    {1.00, 1.00, 1.00, ALPHA_VAL}, /* white*/
    {1.00, 0.00, 0.60, ALPHA_VAL}, /* purple */
    {0.00, 0.00, 1.00, ALPHA_VAL}, /* blue */
    {1.00, 0.80, 0.00, ALPHA_VAL}, /* orange */
    {0.39, 1.00, 0.13, ALPHA_VAL}, /* green */
    {1.00, 0.00, 0.00, ALPHA_VAL}, /* red */
    {1.00, 0.00, 1.00, ALPHA_VAL}, /* pink */
    {0.70, 0.70, 0.70, ALPHA_VAL}, /* grey */
    {0.40, 1.00, 1.00, ALPHA_VAL}, /* lightblue */
    {0.60, 0.40, 0.20, ALPHA_VAL}, /* darkbrown */
    {1.00, 1.00, 0.13, ALPHA_VAL}, /* yellow */
    {0.60, 1.00, 0.60, ALPHA_VAL}, /* lightgreen */
    {0.8, 0.8, 0.8, ALPHA_VAL},
    {0.8, 0.1, 0.6, ALPHA_VAL},
    {0.2, 0.1, 0.7, ALPHA_VAL},
    {0.6, 0.0, 0.6, ALPHA_VAL},
    {0.8, 0.2, 0.8, ALPHA_VAL},
    {0.1, 0.7, 0.2, ALPHA_VAL},
    {0.7, 0.2, 0.3, ALPHA_VAL},
    {0.2, 0.8, 0.5, ALPHA_VAL},
    {0.8, 0.5, 0.4, ALPHA_VAL},
    {0.5, 0.1, 0.9, ALPHA_VAL},
    {0.40, 0.40, 0.40, ALPHA_VAL}, /* the color of the body of tile */
};

int g_maxx = 0;
int g_maxy = 0;
void * g_gltr = NULL;
size_t g_page_cur = 0;
ogl_cb_postredisplay_t g_cb_postredisplay = NULL; // post redisplay event back to system

#if USE_ANTTWEAKBAR
GLfloat g_Zoom = 1.0f;
GLfloat g_Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };
char g_rotable = 0;
char g_is2d = 0;
size_t g_tempreture = 1;
size_t g_birth = 0;
size_t g_count = 0;

size_t g_page_max = 0;
TwBar *g_twbar = NULL;

void
ogl_update_parameter (tssiminfo_t *ptsim)
{
    size_t maxval = g_page_max - 1;
    TwSetParam (g_twbar, "Page", "max", TW_PARAM_INT32, 1, &maxval);

    g_is2d = ptsim->flg_is2d;
    g_tempreture = ptsim->temperature;
    g_page_max = TS_NUMTYPE_SUPERTILE(ptsim);
    if (ptsim->algorithm != TSIM_ALGO_2HATAM) {
        g_page_max ++;
    }
}

void TW_CALL twcb_setval_pagecur (const void *value, void *clientData)
{
    g_page_cur = *(const size_t *)value;
}
void TW_CALL twcb_getval_pagecur (void *value, void *clientData)
{
    *(size_t *)value = g_page_cur;
}

// Routine to convert a quaternion to a 4x4 matrix
// ( input: quat = float[4]  output: mat = float[4*4] )
void ConvertQuaternionToMatrix(const float *quat, float *mat)
{
    float yy2 = 2.0f * quat[1] * quat[1];
    float xy2 = 2.0f * quat[0] * quat[1];
    float xz2 = 2.0f * quat[0] * quat[2];
    float yz2 = 2.0f * quat[1] * quat[2];
    float zz2 = 2.0f * quat[2] * quat[2];
    float wz2 = 2.0f * quat[3] * quat[2];
    float wy2 = 2.0f * quat[3] * quat[1];
    float wx2 = 2.0f * quat[3] * quat[0];
    float xx2 = 2.0f * quat[0] * quat[0];
    mat[0*4+0] = - yy2 - zz2 + 1.0f;
    mat[0*4+1] = xy2 + wz2;
    mat[0*4+2] = xz2 - wy2;
    mat[0*4+3] = 0;
    mat[1*4+0] = xy2 - wz2;
    mat[1*4+1] = - xx2 - zz2 + 1.0f;
    mat[1*4+2] = yz2 + wx2;
    mat[1*4+3] = 0;
    mat[2*4+0] = xz2 + wy2;
    mat[2*4+1] = yz2 - wx2;
    mat[2*4+2] = - xx2 - yy2 + 1.0f;
    mat[2*4+3] = 0;
    mat[3*4+0] = mat[3*4+1] = mat[3*4+2] = 0;
    mat[3*4+3] = 1;
}

#else
#define ogl_update_parameter(a)
#endif

void
ogl_jump2page (tssiminfo_t *ptsim, size_t page)
{
    if (TS_NUMTYPE_SUPERTILE(ptsim) < 1) {
        if (ptsim->algorithm != TSIM_ALGO_2HATAM) {
            if (page > 0) {
                page = 1;
            } else {
                page = 0;
            }
        } else {
            page = 0;
        }
    } else if (page + 1 > TS_NUMTYPE_SUPERTILE(ptsim)) {
        if (ptsim->algorithm != TSIM_ALGO_2HATAM) {
            page = TS_NUMTYPE_SUPERTILE(ptsim);
        } else {
            page = TS_NUMTYPE_SUPERTILE(ptsim) - 1;
        }
    }
    g_page_cur = page;
    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Change to page: %d", g_page_cur);
}

void
ogl_current_page_inc (tssiminfo_t *ptsim)
{
    size_t maxtile = TS_NUMTYPE_SUPERTILE(ptsim);
    if (ptsim->algorithm != TSIM_ALGO_2HATAM) {
        maxtile ++;
    }
    if (g_page_cur + 1 < maxtile) {
        g_page_cur ++;
    } else {
        g_page_cur = maxtile - 1;
    }
    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Change to page: %d", g_page_cur);
}

void
ogl_current_page_dec (tssiminfo_t *ptsim)
{
    if (g_page_cur > 0) {
        g_page_cur --;
    } else {
        g_page_cur = 0;
    }
    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Change to page: %d", g_page_cur);
}

#define COLOR_BACK 0.5, 0.5, 0.5

#define CUBE_WIDTH 3.0
#define RATIO_BOARD 0.1

/*
    5        6
    +--------+
   /        /|
  /        / |
1+--------+2 |
 |        |  +7
 |        | /
 |        |/
0+--------+3
*/
GLfloat g_cube_vertices[][4] = {
    {(-CUBE_WIDTH/2),(-CUBE_WIDTH/2), (CUBE_WIDTH/2)},
    { (CUBE_WIDTH/2),(-CUBE_WIDTH/2), (CUBE_WIDTH/2)},
    { (CUBE_WIDTH/2), (CUBE_WIDTH/2), (CUBE_WIDTH/2)},
    {(-CUBE_WIDTH/2), (CUBE_WIDTH/2), (CUBE_WIDTH/2)},
    {(-CUBE_WIDTH/2),(-CUBE_WIDTH/2),(-CUBE_WIDTH/2)},
    { (CUBE_WIDTH/2),(-CUBE_WIDTH/2),(-CUBE_WIDTH/2)},
    { (CUBE_WIDTH/2), (CUBE_WIDTH/2),(-CUBE_WIDTH/2)},
    {(-CUBE_WIDTH/2), (CUBE_WIDTH/2),(-CUBE_WIDTH/2)},
};

/*
board_x:
    5  6
    +--+
   /  /|
  /  / |
1+--+2 |
 |  |  +7
 |  | /
 |  |/
0+--+3

board_y:
    5        6
    +--------+
   /        /|
  /        / +7
1+--------+2/
 |        |/
0+--------+3

*/

// glTranslatef() + glRotatef()
GLfloat g_front2facet[][7] = {
    // Tx, Ty, Tz,  Ra, Rx, Ry, Rz
    /*North*/ {   0.0, (1+RATIO_BOARD)*CUBE_WIDTH/2, (1+RATIO_BOARD)*CUBE_WIDTH/2,
                 90.0, 1.0, 0.0, 0.0 },
    /*East*/  { (1+RATIO_BOARD)*CUBE_WIDTH/2, 0.0, 0.0,
                -90.0, 0.0, 1.0, 0.0 },
    /*South*/ {   0.0, (1+RATIO_BOARD)*CUBE_WIDTH/2, (1+RATIO_BOARD)*CUBE_WIDTH/2,
                -90.0, 1.0, 0.0, 0.0 },
    /*West*/  {   0.0, 0.0, (1+RATIO_BOARD)*CUBE_WIDTH/2,
                 90.0, 0.0, 1.0, 0.0 },
    /*Front*/ {   0.0, 0.0, 0.0,
                  0.0, 0.0, 0.0, 0.0 },
    /*Back*/  { (1+RATIO_BOARD)*CUBE_WIDTH, 0.0, (1+RATIO_BOARD)*CUBE_WIDTH,
                180.0, 0.0, 1.0, 0.0 },
};

#if 1
#define FONT_TRANS(dir, pref, wcs_msg)
#else
// 字符串字体长度为GLfloat lenfont
#define FONT_TRANS(dir, pref, wcs_msg) \
  { \
    size_t i; \
    GLfloat lenfont; \
    glPushMatrix (); \
    lenfont = gltr_get_adjusted_width (pref, wcs_msg); \
    if (lenfont < gltr_get_height (pref)) { \
        lenfont = gltr_get_height (pref); \
    } \
    if (lenfont > CUBE_WIDTH) { \
        glScalef (CUBE_WIDTH/lenfont, CUBE_WIDTH/lenfont, CUBE_WIDTH/lenfont); \
    } \
    glTranslatef (g_front2facet[dir][0], g_front2facet[dir][1], g_front2facet[dir][2]); \
    glRotatef (g_front2facet[dir][3], g_front2facet[dir][4], g_front2facet[dir][5], g_front2facet[dir][6]); \
    /*gltr_printf (pref, 0, 0, wcs_msg); */ \
    glEnable (GL_TEXTURE_2D); \
    glColor4f (1.0, 0.0, 0.0, ALPHA_VAL); \
    for (i = 0; i < wcslen (wcs_msg); i ++) { gltr_draw_wchar (pref, wcs_msg[i]); break; }\
    glPopMatrix (); \
  }
#endif

#if 0
// 使用单层的glue
#define GLQUAD(idx_color, posxyz, idx_1, idx_2, idx_3, idx_4) \
    glColor4fv (g_color_table[idx_color]); \
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "set color[%d]={%f,%f,%f,%f} ...", idx_color, g_color_table[idx_color][0], g_color_table[idx_color][1], g_color_table[idx_color][2], g_color_table[idx_color][3]); \
    glVertex3f (g_cube_vertices[idx_1][0] + posxyz[0], g_cube_vertices[idx_1][1] + posxyz[1], g_cube_vertices[idx_1][2] + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_2][0] + posxyz[0], g_cube_vertices[idx_2][1] + posxyz[1], g_cube_vertices[idx_2][2] + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_3][0] + posxyz[0], g_cube_vertices[idx_3][1] + posxyz[1], g_cube_vertices[idx_3][2] + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_4][0] + posxyz[0], g_cube_vertices[idx_4][1] + posxyz[1], g_cube_vertices[idx_4][2] + posxyz[2])

#define GLCUBE(color_tile, colors6, posxyz) \
    GLQUAD (colors6[0], posxyz, 2, 3, 7, 6); /*east*/ \
    GLQUAD (colors6[1], posxyz, 1, 2, 6, 5); /*north*/ \
    GLQUAD (colors6[2], posxyz, 0, 1, 5, 4); /*west*/ \
    GLQUAD (colors6[3], posxyz, 0, 4, 7, 3); /*south*/ \
    GLQUAD (colors6[4], posxyz, 0, 3, 2, 1); /*front*/ \
    GLQUAD (colors6[5], posxyz, 4, 5, 6, 7)  /*back*/

#else
// 使用双层厚度的 glue显示
#define GLQUAD(cube_color, posxyz, ratio_x, ratio_y, ratio_z, trans_x, trans_y, trans_z, idx_1, idx_2, idx_3, idx_4) \
    glColor4fv (cube_color); \
    /*DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "set color[%d]={%f,%f,%f,%f} ...", idx_color, g_color_table[idx_color][0], g_color_table[idx_color][1], g_color_table[idx_color][2], g_color_table[idx_color][3]);*/ \
    glVertex3f (g_cube_vertices[idx_1][0] * ratio_x + trans_x + posxyz[0], g_cube_vertices[idx_1][1] * ratio_y + trans_y + posxyz[1], g_cube_vertices[idx_1][2] * ratio_z + trans_z + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_2][0] * ratio_x + trans_x + posxyz[0], g_cube_vertices[idx_2][1] * ratio_y + trans_y + posxyz[1], g_cube_vertices[idx_2][2] * ratio_z + trans_z + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_3][0] * ratio_x + trans_x + posxyz[0], g_cube_vertices[idx_3][1] * ratio_y + trans_y + posxyz[1], g_cube_vertices[idx_3][2] * ratio_z + trans_z + posxyz[2]); \
    glVertex3f (g_cube_vertices[idx_4][0] * ratio_x + trans_x + posxyz[0], g_cube_vertices[idx_4][1] * ratio_y + trans_y + posxyz[1], g_cube_vertices[idx_4][2] * ratio_z + trans_z + posxyz[2])

#define GLCUBE(color_tile, colors6, posxyz, pref, wcs_msg) \
    GLQUAD (color_tile, posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 2, 3, 7, 6); \
    GLQUAD (g_color_table[colors6[ORI_NORTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 1, 2, 6, 5); \
    GLQUAD (g_color_table[colors6[ORI_NORTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 1, 5, 4); \
    GLQUAD (g_color_table[colors6[ORI_NORTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 4, 7, 3); \
    GLQUAD (g_color_table[colors6[ORI_NORTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 3, 2, 1); \
    GLQUAD (g_color_table[colors6[ORI_NORTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 4, 5, 6, 7); \
    GLQUAD (g_color_table[colors6[ORI_EAST]],  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 2, 3, 7, 6); \
    GLQUAD (color_tile,  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 1, 2, 6, 5); \
    GLQUAD (g_color_table[colors6[ORI_EAST]],  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 1, 5, 4); \
    GLQUAD (g_color_table[colors6[ORI_EAST]],  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 4, 7, 3); \
    GLQUAD (g_color_table[colors6[ORI_EAST]],  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 3, 2, 1); \
    GLQUAD (g_color_table[colors6[ORI_EAST]],  posxyz, RATIO_BOARD, 1.0, 1.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 4, 5, 6, 7); \
    GLQUAD (g_color_table[colors6[ORI_FRONT]], posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 2, 3, 7, 6); \
    GLQUAD (g_color_table[colors6[ORI_FRONT]], posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 1, 2, 6, 5); \
    GLQUAD (g_color_table[colors6[ORI_FRONT]], posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 1, 5, 4); \
    GLQUAD (g_color_table[colors6[ORI_FRONT]], posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 4, 7, 3); \
    GLQUAD (color_tile, posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 3, 2, 1); \
    GLQUAD (g_color_table[colors6[ORI_FRONT]], posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 4, 5, 6, 7); \
    GLQUAD (g_color_table[colors6[ORI_SOUTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 2, 3, 7, 6); \
    GLQUAD (g_color_table[colors6[ORI_SOUTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 1, 2, 6, 5); \
    GLQUAD (color_tile, posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 1, 5, 4); \
    GLQUAD (g_color_table[colors6[ORI_SOUTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 4, 7, 3); \
    GLQUAD (g_color_table[colors6[ORI_SOUTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0, 3, 2, 1); \
    GLQUAD (g_color_table[colors6[ORI_SOUTH]], posxyz, 1.0, RATIO_BOARD, 1.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 4, 5, 6, 7); \
    GLQUAD (g_color_table[colors6[ORI_WEST]],  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 2, 3, 7, 6); \
    GLQUAD (g_color_table[colors6[ORI_WEST]],  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 1, 2, 6, 5); \
    GLQUAD (g_color_table[colors6[ORI_WEST]],  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 1, 5, 4); \
    GLQUAD (color_tile,  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 4, 7, 3); \
    GLQUAD (g_color_table[colors6[ORI_WEST]],  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 0, 3, 2, 1); \
    GLQUAD (g_color_table[colors6[ORI_WEST]],  posxyz, RATIO_BOARD, 1.0, 1.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0.0, 0.0, 4, 5, 6, 7); \
    GLQUAD (g_color_table[colors6[ORI_BACK]],  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 2, 3, 7, 6); \
    GLQUAD (g_color_table[colors6[ORI_BACK]],  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 1, 2, 6, 5); \
    GLQUAD (g_color_table[colors6[ORI_BACK]],  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 1, 5, 4); \
    GLQUAD (g_color_table[colors6[ORI_BACK]],  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 4, 7, 3); \
    GLQUAD (g_color_table[colors6[ORI_BACK]],  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 0, 3, 2, 1); \
    GLQUAD (color_tile,  posxyz, 1.0, 1.0, RATIO_BOARD, 0.0, 0.0, 0.0 - (1 + RATIO_BOARD)*(CUBE_WIDTH/2), 4, 5, 6, 7); \
    FONT_TRANS (ORI_NORTH,      pref, wcs_msg); \
    FONT_TRANS (ORI_EAST,       pref, wcs_msg); \
    FONT_TRANS (ORI_SOUTH,      pref, wcs_msg); \
    FONT_TRANS (ORI_WEST,       pref, wcs_msg); \
    FONT_TRANS (4/*ORI_FRONT*/, pref, wcs_msg); \
    FONT_TRANS (5/*ORI_BACK*/,  pref, wcs_msg)


#endif

wchar_t
get_val_utf82uni (uint8_t *pstart, uint8_t **pret_nextpos)
{
    size_t cntleft;
    wchar_t retval = 0;

    if (0 == (0x80 & *pstart)) {
        return *pstart;
    }

	if (((*pstart & 0xE0) ^ 0xC0) == 0) {
        cntleft = 1;
        retval = *pstart & ~0xE0;
    } else if (((*pstart & 0xF0) ^ 0xE0) == 0) {
        cntleft = 2;
        retval = *pstart & ~0xF0;
    } else if (((*pstart & 0xF8) ^ 0xF0) == 0) {
        cntleft = 3;
        retval = *pstart & ~0xF8;
    } else if (((*pstart & 0xFC) ^ 0xF8) == 0) {
        cntleft = 4;
        retval = *pstart & ~0xFC;
    } else if (((*pstart & 0xFE) ^ 0xFC) == 0) {
        cntleft = 5;
        retval = *pstart & ~0xFE;
	} else {
	    // encoding error
        cntleft = 0;
        retval = 0;
    }
    pstart ++;
    for (; cntleft > 0; cntleft --) {
        retval <<= 6;
        retval |= *pstart & 0x3F;
        pstart ++;
    }
    if (pret_nextpos) {
        *pret_nextpos = pstart;
    }
    return retval;
}

// return the number of the wchar_t converted.
// return -1 on error
int
utf8_to_wcs (uint8_t *utf8str, wchar_t *buf, size_t maxnum)
{
    size_t i;
    for (i = 0; i < maxnum; i ++) {
        buf[i] = get_val_utf82uni (utf8str, &utf8str);
        if (0 == buf[i]) {
            break;
        }
    }
    if (i >= maxnum) {
        buf[maxnum - 1] = 0;
        i = maxnum - 1;
    }
    return i;
}

#define CUBES_INTERVAL (CUBE_WIDTH * 1.3)
static int
ogl_draw_tile (int groupid, tsposition_t *ppos, tssiminfo_t *ptsim, tstilecombitem_t *ptci)
{
    // 注意，不能在此内调用 glRotate 和 glTranslate, 而应该在此函数之前调用 glRotate 和 glTranslate
    size_t i;
    int color_list[ORI_MAX];
    int glueid;
    tsposition_t pos_real;
    GLfloat pos_trans[3];
    GLfloat color_tile[4];
    wchar_t label[10] = L"龍";

    assert (NULL != ppos);
    assert (NULL != ptsim);
    assert (NULL != ptci);
    memset (color_tile, 0, sizeof (color_tile));
    memmove (&pos_real, ppos, sizeof (pos_real));
    //DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "ppos(%d,%d,%d) ", pos_real.x, pos_real.y, pos_real.z);
    TSPOS_ADDSELF (pos_real, ptci->pos);
    pos_trans[0] = (GLfloat)pos_real.x * CUBES_INTERVAL + /*平移到原点*/(CUBE_WIDTH / 2);
    pos_trans[1] = (GLfloat)pos_real.y * CUBES_INTERVAL + /*平移到原点*/(CUBE_WIDTH / 2);
    pos_trans[2] = 0.0 - (GLfloat)pos_real.z * CUBES_INTERVAL - /*平移到原点*/(CUBE_WIDTH / 2);;

    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "tile id: %d", ptci->idtile);
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "tile rotnum: %d", ptci->rotnum);
    DBGTS_SHOWPOS (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "tile pos", ptci->pos);
    for (i = 0; i < ORI_MAX; i ++) {
        glueid = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptci, i, 0);
        if (glueid < 0) {
            DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "Unable to find the gule for side: %d", i);
            return -1;
        }
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "color_list[%d]=%d", i, glueid);
        color_list[i] = (glueid + 1) % (NUM_TYPE(g_color_table, GLfloat[4]));
    }
    i = ptsim->ptilevec[ptci->idtile].group;
    if (i > 255) {
        color_tile[0] = ((GLfloat) ((i >> 16) & 0xFF)) / 256.0;
        color_tile[1] = ((GLfloat) ((i >> 8) & 0xFF)) / 256.0;
        color_tile[2] = ((GLfloat) (i & 0xFF)) / 256.0;
        color_tile[3] = ALPHA_VAL;
    } else {
        memmove (color_tile, g_color_table[(i + 1) % (NUM_TYPE(g_color_table, GLfloat[4]))], sizeof (GLfloat[4]));
    }
    //DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "color_tile=(%f,%f,%f)", color_tile[0], color_tile[1], color_tile[2]);
    // get the message:
    if (ptsim->ptilevec[ptci->idtile].label) {
        utf8_to_wcs ((uint8_t *)(ptsim->ptilevec[ptci->idtile].label), label, sizeof (label) / sizeof (wchar_t));
    }
    //glDisable (GL_TEXTURE_2D); glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    glPushMatrix ();
    glBegin (GL_QUADS);
        GLCUBE (color_tile, color_list, pos_trans, g_gltr, label);
    glEnd ();
    glPopMatrix ();
    return 0;
}

// 绘制 supertile
static void
ogl_draw_supertile (int groupid, tsposition_t *ppos, tssiminfo_t *ptsim, tstilecomb_t *ptc)
{
    // 注意，不能在此内调用 glRotate 和 glTranslate, 而应该在此函数之前调用 glRotate 和 glTranslate
    size_t i;
    tstilecombitem_t tci;

    assert (NULL != ppos);
    assert (NULL != ptsim);
    assert (NULL != ptc);
    ogl_update_parameter (ptsim);

    for (i = 0; i < slist_size (&(ptc->tbuf)); i ++) {
        //DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "draw tile %d", i);
        slist_data (&(ptc->tbuf), i, &tci);
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "ogl_draw_tile(%d) ...", i);
        if (ogl_draw_tile (0, ppos, ptsim, &tci) < 0) {
            DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "Unable to draw tile %d", i);
        }
    }
}

// 绘制 supertile 列表中指定 idx 的 supertile
// idx_in: the index of the ptsim->tilelstidx
void
ogl_draw_tilesim (tssiminfo_t *ptsim, size_t idx_in, char flg_draw_info)
{
    // 注意，不能在此内调用 glRotate 和 glTranslate, 而应该在此函数之前调用 glRotate 和 glTranslate
    size_t idx_cur;
    tsposition_t pos;
    tstilecomb_t *ptc = NULL;

    memset (&pos, 0, sizeof (pos));
    if ((ptsim->algorithm != TSIM_ALGO_2HATAM) && (idx_in >= TS_NUMTYPE_SUPERTILE(ptsim))) {
        ptc = ptsim->ptarget;
    } else {
        if (idx_in >= TS_NUMTYPE_SUPERTILE(ptsim)) {
            return;
        }
        idx_cur = idx_in;
        if (ptsim->flg_shrink_record) {
            if (TS_UNSORTED_IDX (ptsim, idx_in, &idx_cur) < 0) {
                DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Unable to find the supertile: %d", idx_in);
            }
        }
        //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_LOG, "draw the supertile: (%d) %d", idx_in, idx_cur);

        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "draw supertile %d", idx_cur);
        if (idx_cur >= ma_size (&(ptsim->tilelist))) {
            DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "Unable to find supertile %d", (int)idx_cur);
            return;
        }

        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idx_cur);
        assert (NULL != ptc);
    }
    if (NULL == ptc) {
        return;
    }
#if USE_ANTTWEAKBAR
    TwDefine (" ControlBar visible=true ");
    TwDefine (" ControlBar/Birth visible=true ");
    TwDefine (" ControlBar/Count visible=true ");
    TwDefine (" ControlBar/Page visible=true ");
    TwDefine (" ControlBar/PageMax visible=true ");
#endif

    ogl_reset_view (g_maxx, g_maxy, &(ptc->maxpos));

    ogl_draw_supertile (0, &pos, ptsim, ptc);

    ma_data_unlock (&(ptsim->tilelist), idx_cur);

if (flg_draw_info) {
#define OGL_CORD2SCRN_X(maxx, x) (x)
#define OGL_CORD2SCRN_Y(maxy, y) (y)
    size_t birth;
    size_t cnt;
    int ret;

    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->countlist)));
    assert (ma_size (&(ptsim->tilelist)) == ma_size (&(ptsim->birthlist)));

    birth = 0;
    cnt = 0;
    if (idx_cur < ma_size (&(ptsim->birthlist))) {
        ret = ma_data (&(ptsim->birthlist), idx_cur, &birth);
        assert (ret >= 0);
        ret = ma_data (&(ptsim->countlist), idx_cur, &cnt);
        assert (ret >= 0);
    }
    g_birth = birth;
    g_count = cnt;

    assert (NUM_TYPE(g_color_table, GLfloat[4]) > 9);
    glPushMatrix ();
    glLoadIdentity ();
    glColor4fv (g_color_table[9]); /*light grey*/
    //glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16));
    //glutfont_print ("%s;%s;TEMP=%d;PAGE=% 3d/%d;BIRTH=%d;CNT=%d"
    gltr_printf (g_gltr, OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16), L"%s;%s;TEMP=%d;BIRTH=%d;CNT=%d;PAGE=% 3d/%d"
        , (ptsim->flg_is2d?"2D":"3D")
        , (ptsim->flg_norotate?"NonRotat":"Rotatable")
        , ptsim->temperature
        , birth
        , cnt
        , idx_in
        , TS_NUMTYPE_SUPERTILE(ptsim) - 1
        );
    glPopMatrix();
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
    if (ptsim->algorithm != TSIM_ALGO_2HATAM) {
        pos = TS_NUMTYPE_SUPERTILE(ptsim);
    } else {
        if (ts_sim_search_target (ptsim, &pos) < 0) {
            DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "The target not found!");
            return -1;
        }
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "found the target @ %d", pos);
    }
    g_page_cur = pos;
    return 0;
}

#if ! USE_ANTTWEAKBAR
// 鼠标模拟轨迹球
#define GLTRAN_OBJ_NONE      0
#define GLTRAN_OBJ_ANGLE     1
#define GLTRAN_OBJ_TRANSLATE 2
#define GLTRAN_OBJ_PICK      3

#define GLTRAN_ACT_ANGLE(pgt, x,y) {(pgt)->active = GLTRAN_OBJ_ANGLE; (pgt)->ox=x; (pgt)->oy=y;}
#define GLTRAN_ACT_PICK(pgt, x,y) {(pgt)->active = GLTRAN_OBJ_PICK; (pgt)->ox=x; (pgt)->oy=y;}
#define GLTRAN_ACT_TRANSLATE(pgt, x,y) {(pgt)->active = GLTRAN_OBJ_TRANSLATE; (pgt)->ox=x; (pgt)->oy=y;}

typedef struct _gl_trans_t {
    int active;
    int winWidth;
    int winHeight;
    int ox;
    int oy;
    GLfloat objangle[2];
    GLfloat objpos[3];
} gl_trans_t;

#define X 0
#define Y 1
#define Z 2

int
gltran_init (gl_trans_t *pgt)
{
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_INFO, "This function should be called only once!");
    assert (NULL != pgt);
    memset (pgt, 0, sizeof (*pgt));

    return 0;
}

// 重置。在显示窗口改变时用
int
gltran_reset (gl_trans_t *pgt, int xw, int yh)
{
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "here");
    assert (NULL != pgt);
    pgt->winWidth = xw;
    pgt->winHeight = yh;
}

// 鼠标移动时操作
int
gltran_mousesim_motion (gl_trans_t *pgt, int x, int y)
{
    int ret = 0;
    switch(pgt->active) {
    case GLTRAN_OBJ_ANGLE:
        pgt->objangle[X] = 360.0 * (x - pgt->winWidth / 2) / (GLfloat)(pgt->winWidth);
        pgt->objangle[Y] = 360.0 * (y - pgt->winHeight / 2)/ (GLfloat)(pgt->winHeight);
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "objangle(x,y) set to (%f,%f)", pgt->objangle[X], pgt->objangle[Y]);
        ret = 1;
        break;
    case GLTRAN_OBJ_PICK:
        pgt->objpos[X] = 100.0 * (x - pgt->winWidth / 2) / (GLfloat)(pgt->winWidth);
        pgt->objpos[Y] = 100.0 * (pgt->winHeight / 2 - y) / (GLfloat)(pgt->winHeight);
        ret = 1;
        break;
    case GLTRAN_OBJ_TRANSLATE:
        pgt->objpos[Z] += x - pgt->ox;
        pgt->ox = x;
        pgt->oy = y;
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "objpos set to (%d,%d,%d)", pgt->objpos[X], pgt->objpos[Y], pgt->objpos[Z]);
        ret = 1;
        break;
    }
    return ret;
}

// 作图像变换。在绘图前调用
// place after the call to
//    glMatrixMode(GL_MODELVIEW);
//    glPushMatrix();
//    gltran_rotatran (&g_ogl_mouse);
void
gltran_rotatran (gl_trans_t *pgt)
{
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glTranslatef(%d,%d,%d) ..."
        , pgt->objpos[X]
        , pgt->objpos[Y]
        , pgt->objpos[Z]
        );
    glTranslatef (pgt->objpos[X], pgt->objpos[Y], pgt->objpos[Z]); /* translate object */
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glRotatef(%f,0,1,0) ..."
        , pgt->objangle[X]
        );
    glRotatef (pgt->objangle[X], 0.f, 1.f, 0.f); /* rotate object */
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glRotatef(%f,1,0,0) ..."
        , pgt->objangle[Y]
        );
    glRotatef (pgt->objangle[Y], 1.f, 0.f, 0.f);
}

#undef X
#undef Y
#undef Z

gl_trans_t g_ogl_mouse;
//void ogl_mousesim_angle (int x, int y)
#define ogl_mousesim_angle(x,y) GLTRAN_ACT_ANGLE (&g_ogl_mouse, x, y)
//void ogl_mousesim_pick (int x, int y)
#define ogl_mousesim_pick(x,y) GLTRAN_ACT_PICK (&g_ogl_mouse, x, y)
//void ogl_mousesim_translate (int x, int y)
#define ogl_mousesim_translate(x,y) GLTRAN_ACT_TRANSLATE (&g_ogl_mouse, x, y)
#define ogl_mousesim_none() (g_ogl_mouse.active=GLTRAN_OBJ_NONE)

//void ogl_mousesim_motion (int x, int y)
#define ogl_mousesim_motion(x,y) gltran_mousesim_motion (&g_ogl_mouse, x, y)
#endif // ! USE_ANTTWEAKBAR

int
ogl_reset_view (int w, int h, tsposition_t *pmaxpos)
{
    GLfloat viewratio;
    size_t maxdis;
    g_maxx = w;
    g_maxy = h;
#if ! USE_ANTTWEAKBAR
    gltran_reset (&g_ogl_mouse, w, h);
#endif

    glViewport (0, 0, (GLsizei)g_maxx, (GLsizei)g_maxy);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

#if USE_THREEDIMENTION
{
    GLfloat param_gltranslatef[3];
    GLfloat param_glfrustum[6];

    viewratio = (GLfloat)g_maxx / (GLfloat)g_maxy;
    maxdis = 1;
    if (maxdis < pmaxpos->x) maxdis = pmaxpos->x;
    if (maxdis < pmaxpos->y) maxdis = pmaxpos->y;
    if (maxdis < pmaxpos->z) maxdis = pmaxpos->z;
    //maxdis = 1;
    // x
    param_gltranslatef[0] = 0.;// - (CUBES_INTERVAL * (pmaxpos->x - 1) + CUBE_WIDTH) / 2;
    // y
    param_gltranslatef[1] = 0.;// - (CUBES_INTERVAL * (pmaxpos->y - 1) + CUBE_WIDTH) / 2;
    // z
    param_gltranslatef[2] = 0.0 - CUBES_INTERVAL * (GLfloat) maxdis * 2.0 * 5;

    // xmin/left
    param_glfrustum[0] = 0.0 - (CUBES_INTERVAL * (/*pmaxpos->x*/2 - 1) + CUBE_WIDTH) * 1.5 / 2.0;
    // xmax/right
    param_glfrustum[1] = (CUBES_INTERVAL * (/*pmaxpos->x*/2 - 1) + CUBE_WIDTH) * 1.5  / 2.0; //CUBES_INTERVAL * pmaxpos->x;
    // ymin/bottom
    param_glfrustum[2] = 0.0 - (CUBES_INTERVAL * (/*pmaxpos->y*/2 - 1) + CUBE_WIDTH) * 1.5  / 2.0; //0.0 - CUBES_INTERVAL;
    // ymax/top
    param_glfrustum[3] = (CUBES_INTERVAL * (/*pmaxpos->y*/2 - 1) + CUBE_WIDTH) * 1.5  / 2.0; //CUBES_INTERVAL * pmaxpos->y;
    // near
    param_glfrustum[4] = 0.0 - 0.1 - param_gltranslatef[2] / 10.0; if (param_glfrustum[4] < 0.1) param_glfrustum[4] = 0.1;
    // far
    //param_glfrustum[5] = param_glfrustum[4] + CUBES_INTERVAL * /*pmaxpos->z*/2 * 1.5 * 100 + 1.0;
    param_glfrustum[5] = param_glfrustum[4] - param_gltranslatef[2];

    //if (viewratio > 1)
    {
        param_glfrustum[0] = viewratio * param_glfrustum[0];
        param_glfrustum[1] = viewratio * param_glfrustum[1];
    //} else {
        //param_glfrustum[2] = viewratio * param_glfrustum[2];
        //param_glfrustum[3] = viewratio * param_glfrustum[3];
    }

    //gluPerspective (45.0f, (GLfloat)w/h, 1.0, 100.0);
    //glFrustum (-2,2,-2,2,1,100);
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glFrustum (%f,%f,%f,%f,%f,%f)", param_glfrustum[0], param_glfrustum[1], param_glfrustum[2], param_glfrustum[3], param_glfrustum[4], param_glfrustum[5]);
    glFrustum (param_glfrustum[0], param_glfrustum[1], param_glfrustum[2], param_glfrustum[3], param_glfrustum[4], param_glfrustum[5]);

    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glTranslatef(%f,%f,%f) ...", param_gltranslatef[0], param_gltranslatef[1], param_gltranslatef[2]);
    glTranslatef (param_gltranslatef[0], param_gltranslatef[1], param_gltranslatef[2]);
}
#else
    glOrtho (0.0, w, 0.0, h, -1.0, 1.0);
#endif

    // clear
    glClearColor (COLOR_BACK, 0.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

#if ! USE_ANTTWEAKBAR
    gltran_rotatran (&g_ogl_mouse);
#endif

#if USE_ANTTWEAKBAR
    //glPushMatrix ();
{
    GLfloat mat[4*4]; // rotation matrix
    ConvertQuaternionToMatrix (g_Rotation, mat);
    glMultMatrixf (mat);
}

    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "glScalef(%f, %f, %f);", g_Zoom, g_Zoom, g_Zoom);
    glScalef (g_Zoom, g_Zoom, g_Zoom);
    //glPopMatrix ();
#endif // USE_ANTTWEAKBAR
#if 1
    //glRotatef (45.0, 1, 0, 0);
    //glRotatef (45.0, 0, 1, 0);
    //glRotatef (45.0, 0, 0, 1);
    // translate the center of the cube to (0,0,0)
    glTranslatef (0.0 - (CUBES_INTERVAL * (pmaxpos->x - 1) + CUBE_WIDTH) / 2.0
        , 0.0 - (CUBES_INTERVAL * (pmaxpos->y - 1) + CUBE_WIDTH) / 2.0
        , (CUBES_INTERVAL * (pmaxpos->z - 1) + CUBE_WIDTH) / 2.0
        );
#endif

}

// include the glViewport(), glMatrixMode (GL_PROJECTION)
int
ogl_reset_projection (int w, int h)
{
    tsposition_t maxpos;
    maxpos.x = 2;
    maxpos.y = 2;
    maxpos.z = 2;
    return ogl_reset_view (w, h, &maxpos);
}

#if 1
#define ogl_set_lights()
#else
static void
ogl_set_lights (void)
{
#if 0
    double  light;
    GLfloat light_color[4];

    /* set viewing projection */
    light_color[3] = 1.0;
    GLfloat Z_axis_pos[4]    = { 0.0, 0.0, 3.0, 0.0 };
    GLfloat lowZ_axis_pos[4] = { 0.0, 0.0, -3.0, 0.5 };

    /* activate light */
    light = 1.0;
    light_color[0] = light_color[1] = light_color[2] = light;
    glLightfv( GL_LIGHT0, GL_POSITION, Z_axis_pos );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, light_color );
    light = 0.3;
    light_color[0] = light_color[1] = light_color[2] = light;
    glLightfv( GL_LIGHT1, GL_POSITION, lowZ_axis_pos );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, light_color );
    glEnable( GL_LIGHT0 );      // White spot on Z axis
    glEnable( GL_LIGHT1 );      // White spot on Z axis ( bottom)
    glEnable( GL_LIGHTING );

#else
    static const GLfloat light0_pos[4]   = { -50.0f, 50.0f, 0.0f, 0.0f };

    // white light
    static const GLfloat light0_color[4] = { 0.6f, 0.6f, 0.6f, 1.0f };

    static const GLfloat light1_pos[4]   = {  50.0f, 50.0f, 0.0f, 0.0f };

    // cold blue light
    static const GLfloat light1_color[4] = { 0.4f, 0.4f, 1.0f, 1.0f };

    /* light */
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_color);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_color);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
#endif
}
#endif

void
ogl_initgl (ogl_cb_postredisplay_t cb_postredisplay)
{
    g_cb_postredisplay = cb_postredisplay;

#if ! USE_ANTTWEAKBAR
    gltran_init (&g_ogl_mouse);
#endif

    /* remove back faces */
    glDisable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);

    /* blend */
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* speedups */
    glEnable (GL_DITHER);
    glEnable (GL_POLYGON_SMOOTH);
    //glEnable(GL_LINE_SMOOTH);
    glShadeModel (GL_SMOOTH);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint (GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

    glEnable(GL_MULTISAMPLE_ARB);

    ogl_set_lights ();

    glutfont_init ();
    g_gltr = gltr_create ("font.ttf", 16);
    if (NULL == g_gltr) {
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "Error in load fonts!");
    }

#if USE_ANTTWEAKBAR
    DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_LOG, "TwInit() ...");
    if (TwInit(TW_OPENGL, NULL)) {
        // Create a tweak bar
        assert (NULL == g_twbar);
        g_twbar = TwNewBar("ControlBar");
        TwDefine (" GLOBAL help='Control in OpenGL.' "); // Message added to the help bar.
        TwDefine (" ControlBar size='200 300' color='96 216 224' "); // change default tweak bar size and color

        //"%s;%s;TEMP=%d;PAGE=% 3d/%d;BIRTH=%d;CNT=%d"
        TwAddVarRO (g_twbar, "Dimension",  TW_TYPE_BOOL8,  &g_is2d,    " true=2D false=3D ");
        TwAddVarRO (g_twbar, "Rotatable",    TW_TYPE_BOOL8,  &g_rotable, " true=Ture false=False ");
        TwAddVarRO (g_twbar, "Tempreture", TW_TYPE_UINT32, &g_tempreture,    "");
        TwAddVarRO (g_twbar, "Birth",      TW_TYPE_UINT32, &g_birth, "");
        TwAddVarRO (g_twbar, "Count",      TW_TYPE_UINT32, &g_count, "");

        TwAddSeparator (g_twbar, NULL, "");
        TwAddVarCB (g_twbar, "Page",       TW_TYPE_UINT32, twcb_setval_pagecur, twcb_getval_pagecur, NULL, " min=0 step=1 ");
        TwAddVarRO (g_twbar, "PageMax",    TW_TYPE_UINT32, &g_page_max, "");
        TwAddSeparator (g_twbar, NULL, "");

        // Add 'g_Zoom' to 'bar': this is a modifable (RW) variable of type TW_TYPE_FLOAT. Its key shortcuts are [z] and [Z].
        TwAddVarRW(g_twbar, "Zoom", TW_TYPE_FLOAT, &g_Zoom,
            " min=0.1 max=5.5 step=0.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size).'  group='adjust' ");

        // Add 'g_Rotation' to 'bar': this is a variable of type TW_TYPE_QUAT4F which defines the object's orientation
        TwAddVarRW(g_twbar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation,
            " label='Object rotation' open help='Change the object orientation.' group='adjust' ");

        TwDefine (" ControlBar visible=false ");
        TwDefine (" TW_HELP visible=false ");
    } else {
        DBGMSG (PFDBG_CATLOG_OPENGL, PFDBG_LEVEL_ERROR, "Error in TwInit");
    }
#endif // USE_ANTTWEAKBAR
}

void
ogl_clear (void)
{
    gltr_destroy (g_gltr);
#if USE_ANTTWEAKBAR
    TwTerminate();
#endif // USE_ANTTWEAKBAR
}

void
ogl_drawother (void)
{
#if USE_ANTTWEAKBAR
    TwDraw ();
#endif
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
    //TwMouseMotion (x, y);
#else
    switch (state) {
    case GLUT_DOWN:
        switch(button) {
        case GLUT_LEFT_BUTTON: /* move the light */
            ogl_mousesim_angle (x, y);
            break;
        case GLUT_RIGHT_BUTTON: /* move the polygon */
            ogl_mousesim_pick (x, y);
            break;
        case GLUT_MIDDLE_BUTTON:
            ogl_mousesim_translate (x, y);
            break;
        }
        break;
    case GLUT_UP:
        ogl_mousesim_none ();
        break;
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
#else // USE_ANTTWEAKBAR
    // mousesim
    if (ogl_mousesim_motion (x, y)) {
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

#if USE_PRESENTATION

int
tssim_cb_initadheredect_ogshow (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test)
{
    tssim_adhere_info_t * pinfo = (tssim_adhere_info_t *) userdata;
    assert (NULL != userdata);
    pinfo->tc_base = tc_base;
    pinfo->tc_test = tc_test;
    memmove (&(pinfo->info_test), pinfo_test, sizeof (*pinfo_test));
    assert (pinfo->info_test.rotnum == (pinfo->info_test.rotnum % TILE_STATUS_MAX));
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

void
tssim_adhere_display_ogshow (tssim_adhere_info_t * pinfo, const char * msg)
{
    tsposition_t tptmp;
    tstilecomb_t tctmp;
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;

    assert (NULL != pinfo);
    assert (NULL != pinfo->psim);

    tstc_init (&tctmp);
    ptc_base = pinfo->tc_base;
    ptc_test = pinfo->tc_test;
    memset (&tptmp, 0, sizeof (&tptmp));
#define USE_OPENGL_ROTATE 0
#if USE_OPENGL_ROTATE
    // 使用 OpenGL 的旋转
    glPushMatrix ();
        glRoate ()
        glTranslatefv ()
        // display the test supertile
        ogl_draw_supertile (0, &tptmp, pinfo->psim, ptc_test);
    glPopMatrix ();

    glPushMatrix ();
        glTranslatef (a,b,c)
        // display the base supertile
        ogl_draw_supertile (0, &tptmp, pinfo->psim, ptc_base);
    glPopMatrix ();

#else
    if ((pinfo->info_test.rotnum) > 0) {
        // 将 test 旋转
        //memset (&tptmp, 0, sizeof (&tptmp));
        ptc_test = tstc_rotate3d (pinfo->tc_test, &tctmp, pinfo->info_test.rotnum, &tptmp);
    }
#if USE_ANTTWEAKBAR
    TwDefine (" ControlBar visible=true ");
    TwDefine (" ControlBar/Birth visible=false ");
    TwDefine (" ControlBar/Count visible=false ");
    TwDefine (" ControlBar/Page visible=false ");
    TwDefine (" ControlBar/PageMax visible=false ");
#endif

    memmove (&tptmp, &(ptc_base->maxpos), sizeof (tptmp));
    TSPOS_ADDSELF (tptmp, ptc_test->maxpos);
    TSPOS_ADDSELF (tptmp, ptc_test->maxpos);
    ogl_reset_view (g_maxx, g_maxy, &tptmp);

    // display the test supertile
    ogl_draw_supertile (0, &(pinfo->info_test.pos), pinfo->psim, ptc_test);

    // display the base supertile
    ogl_draw_supertile (0, &(ptc_test->maxpos), pinfo->psim, ptc_base);

#endif

    // display the tested glues
    // ...

    tstc_clear (&tctmp);

#define OGL_CORD2SCRN_X(maxx, x) (x)
#define OGL_CORD2SCRN_Y(maxy, y) (y)

    assert (NUM_TYPE(g_color_table, GLfloat[4]) > 9);

    glPushMatrix ();
    glLoadIdentity ();

    glColor4fv (g_color_table[9]); /*light grey*/

    //glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16));
    //glutfont_print ("%s;%s;TEMP=%d;"
    gltr_printf (g_gltr, OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy - 16 - 16), L"%s;%s;TEMP=%d;"
        , (pinfo->psim->flg_is2d?"2D":"3D")
        , (pinfo->psim->flg_norotate?"NonRotat":"Rotatable")
        , pinfo->psim->temperature);

    //glRasterPos2i (OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy / 2));
    //glutfont_print ("%s", msg);
    gltr_printf (g_gltr, OGL_CORD2SCRN_X(g_maxx, 5), OGL_CORD2SCRN_Y(g_maxy, g_maxy / 2), L"%s", msg);
    glPopMatrix ();

#if 0
    memset (&tci, 0, sizeof(tci));
    memset (&tptmp, 0, sizeof (&tptmp));
    for (i = 0; i < pinfo->psim->num_tilevec; i ++) {
        tci.idtile = i;
        tptmp.x = i * (SZ_EDGE + SZ_EDGE / 2);
        ogl_draw_tile (0, &tptmp, pinfo->psim, &tci);
    }

    ogl_draw_glueinfo (0, 0, pinfo->psim);
#endif

    ogl_drawother ();
}

#endif
