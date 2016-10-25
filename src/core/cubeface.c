/******************************************************************************
 * Name:        cubeface.c
 * Purpose:     generate the static tables for changing the cube faces
 * Author:      Yunhui Fu
 * Created:     2009-08-13
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "pfdebug.h"
#include "cubeface.h"

#define USE_IMPORTENCODE 0

/*

*. the cube:
    +---+
    | 1 |
+---+---+---+
| 4 | 0 | 2 |
+---+---+---+
    | 3 |
    +---+
    | 5 |
    +---+

*. the positions: [0] ~ [23]
sequence: {center,up,right,down,left}

[ 0] ~ [ 3]: {0,1,2,3,4}, {0,4,1,2,3}, {0,3,4,1,2}, {0,2,3,4,1}
[ 4] ~ [ 7]: {1,0,4,5,2}, {1,2,0,4,5}, {1,5,2,0,4}, {1,4,5,2,0}
[ 8] ~ [11]: {2,0,1,5,3}, ...
[12] ~ [15]: {3,0,2,4,5}, ...
[16] ~ [19]: {4,0,3,5,1}, ...
[20] ~ [23]: {5,1,4,3,2}, ...

*. the opposite:
            0  1  2  3  4  5
[0] ~ [5]: {5, 3, 4, 1, 2, 0}

the rotation table
    |NONE| up | down | left | right | rot 90 | rot -90 |
----+----+----+------+------+-------+--------+---------+
  0 |  0 | 12 |    6 |   11 |    17 |      1 |       3 |
  1 |  1 |  ? |    ? |    ? |     ? |      ? |       ? |
  2 |  2 |  ? |    ? |    ? |     ? |      ? |       ? |
  3 |  3 |  ? |    ? |    ? |     ? |      ? |       ? |
  4 |  4 |  ? |    ? |    ? |     ? |      ? |       ? |
  5 |  5 |  ? |    ? |    ? |     ? |      ? |       ? |
  6 |  6 |  ? |    ? |    ? |     ? |      ? |       ? |
  7 |  7 |  ? |    ? |    ? |     ? |      ? |       ? |
... |... |... |  ... |  ... |   ... |    ... |     ... |
 23 | 23 |  ? |    ? |   ?  |     ? |      ? |       ? |
*/

int g_cubeface_encode0[CUBE_FACE_NUM] = CUBE_FACE_ENC_LIST;

static const int g_cubeface_rotchg[CUBE_ROT_MAX][CUBE_FACE_NUM] = {
    {0, 1, 2, 3, 4, 5}, /*non-rotation*/
    {3, 0, 2, 5, 4, 1}, /*up*/
    {1, 5, 2, 0, 4, 3}, /*down*/
    {2, 1, 5, 3, 0, 4}, /*left*/
    {4, 1, 0, 3, 5, 2}, /*right*/
    {0, 4, 1, 2, 3, 5}, /*p90*/
    {0, 2, 3, 4, 1, 5}, /*m90*/
};

// this array is used for trun all of the side to the front once.
// 要将编号为 i 的某面翻到正前方，需要经过 rot4pos[i][0] 方向的翻转共 rot4pos[i][1] 次
static const int rot4pos[CUBE_FACE_NUM][2] = {
    //rot_dir, rot_num
    {CUBE_ROT_NONE,  1},
    {CUBE_ROT_DOWN,  1},
    {CUBE_ROT_LEFT,  1},
    {CUBE_ROT_UP,    1},
    {CUBE_ROT_RIGHT, 1},
    {CUBE_ROT_DOWN,  2},
};

// 从其他面切换到当前面需要的旋转和上翻操作
// 数组每项第一个记录是顺时针旋转次数，第二个记录是上翻次数
// 数组一共有六个面的数据
// 当前面在数组最前面，即旋转次数和上翻次数都为0
// 顺序分别是 前、上、右、下、左、后。
const size_t g_cubeface_cur2all[CUBE_FACE_NUM][2] = {
    // 顺时针旋转(CUBE_ROT_P90)次数，下翻(CUBE_ROT_DOWN)次数
    {0, 0}, // 当前面
    {0, 1}, // 上面
    {1, 3}, // 右
    {0, 3}, // 下
    {1, 1}, // 左
    {0, 2}, // 背
};

/////////////////////////////////////////////////////////////////////////////////////////
//int g_cubeface_rotpos[24][CUBE_FACE_NUM];
//int g_cubeface_rottab[24][CUBE_ROT_MAX];
//int g_cubeface_map_cur2all[24][24][3];
int g_cubeface_rotpos[24][CUBE_FACE_NUM] = {
    { 0, 1, 2, 3, 4, 5},
    { 0, 4, 1, 2, 3, 5},
    { 0, 3, 4, 1, 2, 5},
    { 0, 2, 3, 4, 1, 5},
    { 1, 0, 4, 5, 2, 3},
    { 1, 2, 0, 4, 5, 3},
    { 1, 5, 2, 0, 4, 3},
    { 1, 4, 5, 2, 0, 3},
    { 2, 0, 1, 5, 3, 4},
    { 2, 3, 0, 1, 5, 4},
    { 2, 5, 3, 0, 1, 4},
    { 2, 1, 5, 3, 0, 4},
    { 3, 0, 2, 5, 4, 1},
    { 3, 4, 0, 2, 5, 1},
    { 3, 5, 4, 0, 2, 1},
    { 3, 2, 5, 4, 0, 1},
    { 4, 0, 3, 5, 1, 2},
    { 4, 1, 0, 3, 5, 2},
    { 4, 5, 1, 0, 3, 2},
    { 4, 3, 5, 1, 0, 2},
    { 5, 1, 4, 3, 2, 0},
    { 5, 2, 1, 4, 3, 0},
    { 5, 3, 2, 1, 4, 0},
    { 5, 4, 3, 2, 1, 0},
}; /* end of g_cubeface_rotpos */
int g_cubeface_rottab[24][CUBE_ROT_MAX] = {
// CUBE_ROT_NONE, CUBE_ROT_UP, CUBE_ROT_DOWN, CUBE_ROT_LEFT, CUBE_ROT_RIGHT, CUBE_ROT_P90, CUBE_ROT_M90
    {  0, 12,  6, 11, 17,  1,  3,},
    {  1,  8, 18,  7, 13,  2,  0,},
    {  2,  4, 14, 19,  9,  3,  1,},
    {  3, 16, 10, 15,  5,  0,  2,},
    {  4, 20,  2, 16,  8,  5,  7,},
    {  5, 17,  9,  3, 21,  6,  4,},
    {  6,  0, 22, 10, 18,  7,  5,},
    {  7, 11, 19, 23,  1,  4,  6,},
    {  8, 21,  1,  4, 12,  9, 11,},
    {  9,  5, 13,  2, 22, 10,  8,},
    { 10,  3, 23, 14,  6, 11,  9,},
    { 11, 15,  7, 20,  0,  8, 10,},
    { 12, 22,  0,  8, 16, 13, 15,},
    { 13,  9, 17,  1, 23, 14, 12,},
    { 14,  2, 20, 18, 10, 15, 13,},
    { 15, 19, 11, 21,  3, 12, 14,},
    { 16, 23,  3, 12,  4, 17, 19,},
    { 17, 13,  5,  0, 20, 18, 16,},
    { 18,  1, 21,  6, 14, 19, 17,},
    { 19,  7, 15, 22,  2, 16, 18,},
    { 20, 14,  4, 17, 11, 21, 23,},
    { 21, 18,  8,  5, 15, 22, 20,},
    { 22,  6, 12,  9, 19, 23, 21,},
    { 23, 10, 16, 13,  7, 20, 22,},
}; /* end of g_cubeface_rottab */
/*what will do when one state converted to another state, the rotation sequence: P90,UP and P90*/
/*当前面为某状态时，要变到某个指定状态，需要P90,UP,P90各自多少转
同时，g_cubeface_map_cur2all[0][rotnum] 表示的是翻转 rotnum 的方法，即在一个cube 记录的状态 rotnum 是 在初始状态翻转的在第 rotnum 个方法所抵达的状态
*/
int g_cubeface_map_cur2all[24][24][3] = {
  {
    {0,0,0},{0,0,1},{0,0,2},{0,0,3},{0,1,2},{0,1,3},{0,1,0},{0,1,1},
    {1,3,0},{1,3,1},{1,3,2},{1,3,3},{0,3,0},{0,3,1},{0,3,2},{0,3,3},
    {1,1,2},{1,1,3},{1,1,0},{1,1,1},{0,2,2},{0,2,3},{0,2,0},{0,2,1},
  },
  {
    {0,0,3},{0,0,0},{0,0,1},{0,0,2},{1,3,0},{1,3,1},{1,3,2},{1,3,3},
    {0,3,0},{0,3,1},{0,3,2},{0,3,3},{1,1,2},{1,1,3},{1,1,0},{1,1,1},
    {0,1,2},{0,1,3},{0,1,0},{0,1,1},{0,2,3},{0,2,0},{0,2,1},{0,2,2},
  },
  {
    {0,0,2},{0,0,3},{0,0,0},{0,0,1},{0,3,0},{0,3,1},{0,3,2},{0,3,3},
    {1,1,2},{1,1,3},{1,1,0},{1,1,1},{0,1,2},{0,1,3},{0,1,0},{0,1,1},
    {1,3,0},{1,3,1},{1,3,2},{1,3,3},{0,2,0},{0,2,1},{0,2,2},{0,2,3},
  },
  {
    {0,0,1},{0,0,2},{0,0,3},{0,0,0},{1,1,2},{1,1,3},{1,1,0},{1,1,1},
    {0,1,2},{0,1,3},{0,1,0},{0,1,1},{1,3,0},{1,3,1},{1,3,2},{1,3,3},
    {0,3,0},{0,3,1},{0,3,2},{0,3,3},{0,2,1},{0,2,2},{0,2,3},{0,2,0},
  },
  {
    {0,1,2},{0,1,3},{0,1,0},{0,1,1},{0,0,0},{0,0,1},{0,0,2},{0,0,3},
    {1,1,3},{1,1,0},{1,1,1},{1,1,2},{0,2,2},{0,2,3},{0,2,0},{0,2,1},
    {1,3,3},{1,3,0},{1,3,1},{1,3,2},{0,3,0},{0,3,1},{0,3,2},{0,3,3},
  },
  {
    {1,3,0},{1,3,1},{1,3,2},{1,3,3},{0,0,3},{0,0,0},{0,0,1},{0,0,2},
    {0,1,3},{0,1,0},{0,1,1},{0,1,2},{0,2,3},{0,2,0},{0,2,1},{0,2,2},
    {0,3,3},{0,3,0},{0,3,1},{0,3,2},{1,1,2},{1,1,3},{1,1,0},{1,1,1},
  },
  {
    {0,3,0},{0,3,1},{0,3,2},{0,3,3},{0,0,2},{0,0,3},{0,0,0},{0,0,1},
    {1,3,1},{1,3,2},{1,3,3},{1,3,0},{0,2,0},{0,2,1},{0,2,2},{0,2,3},
    {1,1,1},{1,1,2},{1,1,3},{1,1,0},{0,1,2},{0,1,3},{0,1,0},{0,1,1},
  },
  {
    {1,1,2},{1,1,3},{1,1,0},{1,1,1},{0,0,1},{0,0,2},{0,0,3},{0,0,0},
    {0,3,1},{0,3,2},{0,3,3},{0,3,0},{0,2,1},{0,2,2},{0,2,3},{0,2,0},
    {0,1,1},{0,1,2},{0,1,3},{0,1,0},{1,3,0},{1,3,1},{1,3,2},{1,3,3},
  },
  {
    {0,1,3},{0,1,0},{0,1,1},{0,1,2},{1,3,3},{1,3,0},{1,3,1},{1,3,2},
    {0,0,0},{0,0,1},{0,0,2},{0,0,3},{1,1,3},{1,1,0},{1,1,1},{1,1,2},
    {0,2,2},{0,2,3},{0,2,0},{0,2,1},{0,3,3},{0,3,0},{0,3,1},{0,3,2},
  },
  {
    {1,3,1},{1,3,2},{1,3,3},{1,3,0},{0,3,3},{0,3,0},{0,3,1},{0,3,2},
    {0,0,3},{0,0,0},{0,0,1},{0,0,2},{0,1,3},{0,1,0},{0,1,1},{0,1,2},
    {0,2,3},{0,2,0},{0,2,1},{0,2,2},{1,1,1},{1,1,2},{1,1,3},{1,1,0},
  },
  {
    {0,3,1},{0,3,2},{0,3,3},{0,3,0},{1,1,1},{1,1,2},{1,1,3},{1,1,0},
    {0,0,2},{0,0,3},{0,0,0},{0,0,1},{1,3,1},{1,3,2},{1,3,3},{1,3,0},
    {0,2,0},{0,2,1},{0,2,2},{0,2,3},{0,1,1},{0,1,2},{0,1,3},{0,1,0},
  },
  {
    {1,1,3},{1,1,0},{1,1,1},{1,1,2},{0,1,1},{0,1,2},{0,1,3},{0,1,0},
    {0,0,1},{0,0,2},{0,0,3},{0,0,0},{0,3,1},{0,3,2},{0,3,3},{0,3,0},
    {0,2,1},{0,2,2},{0,2,3},{0,2,0},{1,3,3},{1,3,0},{1,3,1},{1,3,2},
  },
  {
    {0,1,0},{0,1,1},{0,1,2},{0,1,3},{0,2,2},{0,2,3},{0,2,0},{0,2,1},
    {1,3,3},{1,3,0},{1,3,1},{1,3,2},{0,0,0},{0,0,1},{0,0,2},{0,0,3},
    {1,1,3},{1,1,0},{1,1,1},{1,1,2},{0,3,2},{0,3,3},{0,3,0},{0,3,1},
  },
  {
    {1,3,2},{1,3,3},{1,3,0},{1,3,1},{0,2,3},{0,2,0},{0,2,1},{0,2,2},
    {0,3,3},{0,3,0},{0,3,1},{0,3,2},{0,0,3},{0,0,0},{0,0,1},{0,0,2},
    {0,1,3},{0,1,0},{0,1,1},{0,1,2},{1,1,0},{1,1,1},{1,1,2},{1,1,3},
  },
  {
    {0,3,2},{0,3,3},{0,3,0},{0,3,1},{0,2,0},{0,2,1},{0,2,2},{0,2,3},
    {1,1,1},{1,1,2},{1,1,3},{1,1,0},{0,0,2},{0,0,3},{0,0,0},{0,0,1},
    {1,3,1},{1,3,2},{1,3,3},{1,3,0},{0,1,0},{0,1,1},{0,1,2},{0,1,3},
  },
  {
    {1,1,0},{1,1,1},{1,1,2},{1,1,3},{0,2,1},{0,2,2},{0,2,3},{0,2,0},
    {0,1,1},{0,1,2},{0,1,3},{0,1,0},{0,0,1},{0,0,2},{0,0,3},{0,0,0},
    {0,3,1},{0,3,2},{0,3,3},{0,3,0},{1,3,2},{1,3,3},{1,3,0},{1,3,1},
  },
  {
    {0,1,1},{0,1,2},{0,1,3},{0,1,0},{1,1,3},{1,1,0},{1,1,1},{1,1,2},
    {0,2,2},{0,2,3},{0,2,0},{0,2,1},{1,3,3},{1,3,0},{1,3,1},{1,3,2},
    {0,0,0},{0,0,1},{0,0,2},{0,0,3},{0,3,1},{0,3,2},{0,3,3},{0,3,0},
  },
  {
    {1,3,3},{1,3,0},{1,3,1},{1,3,2},{0,1,3},{0,1,0},{0,1,1},{0,1,2},
    {0,2,3},{0,2,0},{0,2,1},{0,2,2},{0,3,3},{0,3,0},{0,3,1},{0,3,2},
    {0,0,3},{0,0,0},{0,0,1},{0,0,2},{1,1,3},{1,1,0},{1,1,1},{1,1,2},
  },
  {
    {0,3,3},{0,3,0},{0,3,1},{0,3,2},{1,3,1},{1,3,2},{1,3,3},{1,3,0},
    {0,2,0},{0,2,1},{0,2,2},{0,2,3},{1,1,1},{1,1,2},{1,1,3},{1,1,0},
    {0,0,2},{0,0,3},{0,0,0},{0,0,1},{0,1,3},{0,1,0},{0,1,1},{0,1,2},
  },
  {
    {1,1,1},{1,1,2},{1,1,3},{1,1,0},{0,3,1},{0,3,2},{0,3,3},{0,3,0},
    {0,2,1},{0,2,2},{0,2,3},{0,2,0},{0,1,1},{0,1,2},{0,1,3},{0,1,0},
    {0,0,1},{0,0,2},{0,0,3},{0,0,0},{1,3,1},{1,3,2},{1,3,3},{1,3,0},
  },
  {
    {0,2,2},{0,2,3},{0,2,0},{0,2,1},{0,1,0},{0,1,1},{0,1,2},{0,1,3},
    {1,1,0},{1,1,1},{1,1,2},{1,1,3},{0,3,2},{0,3,3},{0,3,0},{0,3,1},
    {1,3,2},{1,3,3},{1,3,0},{1,3,1},{0,0,0},{0,0,1},{0,0,2},{0,0,3},
  },
  {
    {0,2,3},{0,2,0},{0,2,1},{0,2,2},{1,3,2},{1,3,3},{1,3,0},{1,3,1},
    {0,1,0},{0,1,1},{0,1,2},{0,1,3},{1,1,0},{1,1,1},{1,1,2},{1,1,3},
    {0,3,2},{0,3,3},{0,3,0},{0,3,1},{0,0,3},{0,0,0},{0,0,1},{0,0,2},
  },
  {
    {0,2,0},{0,2,1},{0,2,2},{0,2,3},{0,3,2},{0,3,3},{0,3,0},{0,3,1},
    {1,3,2},{1,3,3},{1,3,0},{1,3,1},{0,1,0},{0,1,1},{0,1,2},{0,1,3},
    {1,1,0},{1,1,1},{1,1,2},{1,1,3},{0,0,2},{0,0,3},{0,0,0},{0,0,1},
  },
  {
    {0,2,1},{0,2,2},{0,2,3},{0,2,0},{1,1,0},{1,1,1},{1,1,2},{1,1,3},
    {0,3,2},{0,3,3},{0,3,0},{0,3,1},{1,3,2},{1,3,3},{1,3,0},{1,3,1},
    {0,1,0},{0,1,1},{0,1,2},{0,1,3},{0,0,1},{0,0,2},{0,0,3},{0,0,0},
  },
};

/*当前面为某状态时，要变到某个指定状态，经过的 rotnum 是多少，该 rotnum 是指在 g_cubeface_map_cur2all[0][idx][...]的索引*/
/*本数组是根据g_cubeface_map_cur2all 生成的*/
int g_cubeface_rotnum_cur2all[24][24] = {
{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,},
{  3,  0,  1,  2,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,  4,  5,  6,  7, 21, 22, 23, 20,},
{  2,  3,  0,  1, 12, 13, 14, 15, 16, 17, 18, 19,  4,  5,  6,  7,  8,  9, 10, 11, 22, 23, 20, 21,},
{  1,  2,  3,  0, 16, 17, 18, 19,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 23, 20, 21, 22,},
{  4,  5,  6,  7,  0,  1,  2,  3, 17, 18, 19, 16, 20, 21, 22, 23, 11,  8,  9, 10, 12, 13, 14, 15,},
{  8,  9, 10, 11,  3,  0,  1,  2,  5,  6,  7,  4, 21, 22, 23, 20, 15, 12, 13, 14, 16, 17, 18, 19,},
{ 12, 13, 14, 15,  2,  3,  0,  1,  9, 10, 11,  8, 22, 23, 20, 21, 19, 16, 17, 18,  4,  5,  6,  7,},
{ 16, 17, 18, 19,  1,  2,  3,  0, 13, 14, 15, 12, 23, 20, 21, 22,  7,  4,  5,  6,  8,  9, 10, 11,},
{  5,  6,  7,  4, 11,  8,  9, 10,  0,  1,  2,  3, 17, 18, 19, 16, 20, 21, 22, 23, 15, 12, 13, 14,},
{  9, 10, 11,  8, 15, 12, 13, 14,  3,  0,  1,  2,  5,  6,  7,  4, 21, 22, 23, 20, 19, 16, 17, 18,},
{ 13, 14, 15, 12, 19, 16, 17, 18,  2,  3,  0,  1,  9, 10, 11,  8, 22, 23, 20, 21,  7,  4,  5,  6,},
{ 17, 18, 19, 16,  7,  4,  5,  6,  1,  2,  3,  0, 13, 14, 15, 12, 23, 20, 21, 22, 11,  8,  9, 10,},
{  6,  7,  4,  5, 20, 21, 22, 23, 11,  8,  9, 10,  0,  1,  2,  3, 17, 18, 19, 16, 14, 15, 12, 13,},
{ 10, 11,  8,  9, 21, 22, 23, 20, 15, 12, 13, 14,  3,  0,  1,  2,  5,  6,  7,  4, 18, 19, 16, 17,},
{ 14, 15, 12, 13, 22, 23, 20, 21, 19, 16, 17, 18,  2,  3,  0,  1,  9, 10, 11,  8,  6,  7,  4,  5,},
{ 18, 19, 16, 17, 23, 20, 21, 22,  7,  4,  5,  6,  1,  2,  3,  0, 13, 14, 15, 12, 10, 11,  8,  9,},
{  7,  4,  5,  6, 17, 18, 19, 16, 20, 21, 22, 23, 11,  8,  9, 10,  0,  1,  2,  3, 13, 14, 15, 12,},
{ 11,  8,  9, 10,  5,  6,  7,  4, 21, 22, 23, 20, 15, 12, 13, 14,  3,  0,  1,  2, 17, 18, 19, 16,},
{ 15, 12, 13, 14,  9, 10, 11,  8, 22, 23, 20, 21, 19, 16, 17, 18,  2,  3,  0,  1,  5,  6,  7,  4,},
{ 19, 16, 17, 18, 13, 14, 15, 12, 23, 20, 21, 22,  7,  4,  5,  6,  1,  2,  3,  0,  9, 10, 11,  8,},
{ 20, 21, 22, 23,  6,  7,  4,  5, 18, 19, 16, 17, 14, 15, 12, 13, 10, 11,  8,  9,  0,  1,  2,  3,},
{ 21, 22, 23, 20, 10, 11,  8,  9,  6,  7,  4,  5, 18, 19, 16, 17, 14, 15, 12, 13,  3,  0,  1,  2,},
{ 22, 23, 20, 21, 14, 15, 12, 13, 10, 11,  8,  9,  6,  7,  4,  5, 18, 19, 16, 17,  2,  3,  0,  1,},
{ 23, 20, 21, 22, 18, 19, 16, 17, 14, 15, 12, 13, 10, 11,  8,  9,  6,  7,  4,  5,  1,  2,  3,  0,},
};

/*
  cube_rot: one of CUBE_ROT_{UP|DOWN|LEFT|RIGHT|P90|M90}
*/
static int
cube_rot (int cube_origin[CUBE_FACE_NUM], int rot, int cube_out[CUBE_FACE_NUM])
{
    int i;
    assert ((rot >= CUBE_ROT_NONE) && (rot < CUBE_ROT_MAX));
    assert (cube_origin != cube_out);
    assert (!((cube_out < cube_origin) && (cube_origin < cube_out + CUBE_FACE_NUM)));
    assert (!((cube_origin < cube_out) && (cube_out < cube_origin + CUBE_FACE_NUM)));
    for (i = 0; i < CUBE_FACE_NUM; i ++) {
        cube_out[i] = cube_origin[g_cubeface_rotchg[rot][i]];
    }
    return 0;
}

// 查找正上方为最小编号的顺时针旋转位置并返回其位置的数据
static int
find_clockwise_first (int cube_origin[CUBE_FACE_NUM], int * pcube_rot, int cube_out[CUBE_FACE_NUM])
{
    int cube_out_tmp1[CUBE_FACE_NUM];
    int cube_out_tmp2[CUBE_FACE_NUM];
    int *cube_out_tmpp1 = cube_out_tmp1;
    int *cube_out_tmpp2 = cube_out_tmp2;
    int i;
    int curidx;
    int curval;
    assert (NULL != pcube_rot);
    curidx = 1;
    curval = cube_origin[1];
    for (i = 2; i < 5; i ++) {
        if (cube_origin[i] < curval) {
            curidx = i;
            curval = cube_origin[i];
        }
    }
    *pcube_rot = curidx;
    if (curidx > 1) {
        memmove (cube_out_tmpp1, cube_origin, sizeof (cube_out_tmp1));
        for (i = 0; i < curidx - 1; i ++) {
            cube_rot (cube_out_tmpp1, CUBE_ROT_M90, cube_out_tmpp2);
            /*swap the pointer*/
            SWAP (int *, cube_out_tmpp1, cube_out_tmpp2);
        }
        memmove (cube_out, cube_out_tmpp1, sizeof (cube_out_tmp1));
    } else {
        memmove (cube_out, cube_origin, sizeof (cube_out_tmp1));
    }
    return 0;
}

// 各个状态可以经过 若干次P90 和 若干次DOWN 再若干次P90 而变化到任意一个状态
// 求出这些 “若干次” 的具体值，是所有的状态变化到其他状态的表格。
void
fill_mapcur2all (void)
{
    size_t i,j;
    size_t cur_state, ret_state;
    for (cur_state = 0; cur_state < 24; cur_state ++) {
        for (i = 0; i < CUBE_FACE_NUM; i ++) {
            for (j = 0; j < 4; j ++) {
                GET_CUBEFACE_4ROT_P90_DOWN_P90(g_cubeface_cur2all[i][0], g_cubeface_cur2all[i][1], j, cur_state, ret_state);
                assert (cur_state < 24);
                assert (ret_state < 24);
                g_cubeface_map_cur2all[cur_state][ret_state][0] = g_cubeface_cur2all[i][0];
                g_cubeface_map_cur2all[cur_state][ret_state][1] = g_cubeface_cur2all[i][1];
                g_cubeface_map_cur2all[cur_state][ret_state][2] = j;
            }
        }
        for (i = 0; i < 24; i ++) {
            for (j = 0; j < 24; j ++) {
                if (0 == memcmp (g_cubeface_map_cur2all[cur_state][i], g_cubeface_map_cur2all[0][j], 3 * sizeof(int))) {
                    g_cubeface_rotnum_cur2all[cur_state][i] = j;
                }
            }
        }
    }
}

#if DEBUG
// 测试用的数组
/* 各个 [0]状态 在某个变换([1]P90,[2]DOWN,[3]P90)下所达到的状态
如 g_cubeface_dbg_rotser2rotn[0][3 % 4][2 % 4][1 % 4] 表示的是在0状态下，经过 3次P90 而后 2次DOWN 最后1次P90到达的状态
*/
int g_cubeface_dbg_rotser2rotnum[24][4][4][4] = {
  {
    {{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},},
    {{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},},
    {{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},},
    {{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},},
  },
  {
    {{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},},
    {{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},},
    {{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},},
    {{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},},
  },
  {
    {{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},},
    {{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},},
    {{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},},
    {{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},},
  },
  {
    {{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},},
    {{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},},
    {{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},},
    {{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},},
  },
  {
    {{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},},
    {{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},},
    {{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},},
    {{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},},
  },
  {
    {{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},},
    {{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},},
    {{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},},
    {{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},},
  },
  {
    {{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},},
    {{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},},
    {{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},},
    {{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},},
  },
  {
    {{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},},
    {{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},},
    {{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},},
    {{  6,  7,  4,  5,},{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},},
  },
  {
    {{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},},
    {{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},},
    {{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},},
    {{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},},
  },
  {
    {{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},},
    {{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},},
    {{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},},
    {{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},},
  },
  {
    {{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},},
    {{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},},
    {{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},},
    {{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},},
  },
  {
    {{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},},
    {{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},},
    {{  9, 10, 11,  8,},{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},},
    {{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},},
  },
  {
    {{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},},
    {{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},},
    {{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},},
    {{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},},
  },
  {
    {{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},},
    {{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},},
    {{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},},
    {{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},},
  },
  {
    {{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},},
    {{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},},
    {{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},},
    {{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},},
  },
  {
    {{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},{ 19, 16, 17, 18,},},
    {{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},{ 22, 23, 20, 21,},},
    {{ 13, 14, 15, 12,},{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},},
    {{ 14, 15, 12, 13,},{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},},
  },
  {
    {{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},},
    {{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},},
    {{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},},
    {{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},},
  },
  {
    {{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},},
    {{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},},
    {{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},},
    {{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},},
  },
  {
    {{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},},
    {{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},},
    {{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},},
    {{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},},
  },
  {
    {{ 19, 16, 17, 18,},{ 15, 12, 13, 14,},{ 11,  8,  9, 10,},{  7,  4,  5,  6,},},
    {{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},{ 23, 20, 21, 22,},},
    {{ 17, 18, 19, 16,},{  5,  6,  7,  4,},{  9, 10, 11,  8,},{ 13, 14, 15, 12,},},
    {{ 18, 19, 16, 17,},{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},},
  },
  {
    {{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},},
    {{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},},
    {{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},},
    {{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},},
  },
  {
    {{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},},
    {{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},},
    {{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},},
    {{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},},
  },
  {
    {{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},},
    {{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},},
    {{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},},
    {{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},},
  },
  {
    {{ 23, 20, 21, 22,},{ 16, 17, 18, 19,},{  3,  0,  1,  2,},{ 10, 11,  8,  9,},},
    {{ 20, 21, 22, 23,},{  4,  5,  6,  7,},{  2,  3,  0,  1,},{ 14, 15, 12, 13,},},
    {{ 21, 22, 23, 20,},{  8,  9, 10, 11,},{  1,  2,  3,  0,},{ 18, 19, 16, 17,},},
    {{ 22, 23, 20, 21,},{ 12, 13, 14, 15,},{  0,  1,  2,  3,},{  6,  7,  4,  5,},},
  },
};

// 生成测试用的数组
static void
fill_rotser2rotnum (void)
{
    size_t i;
    size_t j;
    size_t k;
    size_t m;

    for (m = 0; m < 24; m ++)
        for (i = 0; i < 4; i ++)
        for (j = 0; j < 4; j ++)
        for (k = 0; k < 4; k ++)
        {
            GET_CUBEFACE_4ROT_P90_DOWN_P90 ( i, j, k, m, g_cubeface_dbg_rotser2rotnum[m][i][j][k]);
        }
}
#else
#define fill_rotser2rotnum()
#endif

// （之前用户有可能改变了g_cubeface_rotpos）更新其他表格
void
rottab_gen_update (void)
{
    int cube1[CUBE_FACE_NUM];
    int cube2[CUBE_FACE_NUM];
    int * cube1p = cube1;
    int * cube2p = cube2;
    int i;
    int j;
    int k;

    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "BEGIN ....");

    // 各个状态分别经过一次 UP、DOWN、LEFT、RIGHT、P90、M90 会变化到哪个状态
    for (i = 0; i < 24; i ++) {
        memmove (cube1p, &(g_cubeface_rotpos[i]), sizeof(g_cubeface_encode0));
        g_cubeface_rottab[i][0] = i;
        for (j = CUBE_ROT_UP; j < CUBE_ROT_MAX; j ++) {
            cube_rot (cube1p, j, cube2p);
            //fprintf (fp_out, "cube2p:"); out_one_cube (fp_out, cube2p);
            // find the idx of the position
            for (k = 0; k < 24; k ++) {
                int m;
                for (m = 0; m < CUBE_FACE_NUM; m ++) {
                    if (g_cubeface_rotpos[k][m] != cube2p[m]) {
                        break;
                    }
                }
                if (m >= CUBE_FACE_NUM) {
                    break;
                }
            }
            if (k >= 24) {
                // error!
                assert (0);
                break;
            }
            g_cubeface_rottab[i][j] = k;
        }
    }
    fill_mapcur2all ();
    fill_rotser2rotnum ();
}

// 给所有表格设置缺省值
void
rottab_gen_default (void)
{
    int front[CUBE_FACE_NUM];
    int cube1[CUBE_FACE_NUM];
    int cube2[CUBE_FACE_NUM];
    int * cube1p = cube1;
    int * cube2p = cube2;
    int i;
    int j;
    int k;
    int rotnum;

    // find the id of the front face
    for (i = 0; i < sizeof(rot4pos) / sizeof(int[2]); i ++) {
        memmove (cube1p, g_cubeface_encode0, sizeof (g_cubeface_encode0));
        for (j = 0; j < rot4pos[i][1]; j ++) {
            cube_rot (cube1p, rot4pos[i][0], cube2p);
            SWAP (int *, cube1p, cube2p);
        }
        assert ((0 <= cube1p[0]) && (cube1p[0] < 6));
        assert (i < CUBE_FACE_NUM);
        front[cube1p[0]] = i;
    }

    // 生成24个状态各个对应的编码顺序
    // 这里的g_cubeface_rotpos可以由用户指定，但是这里设定成缺省值
    k = 0;
    for (i = 0; i < sizeof(rot4pos) / sizeof(int[2]); i ++) {
        memmove (cube1p, g_cubeface_encode0, sizeof (g_cubeface_encode0));
        for (j = 0; j < rot4pos[front[i]][1]; j ++) {
            cube_rot (cube1p, rot4pos[front[i]][0], cube2p);
            SWAP (int *, cube1p, cube2p);
        }
        find_clockwise_first (cube1p, &rotnum, cube2p);
        SWAP (int *, cube1p, cube2p);
        for (j = 0; j < 4; j ++) {
            //out_one_cube (fp_out, cube2p);
            memmove (&(g_cubeface_rotpos[k]), cube1p, sizeof(g_cubeface_encode0));
            k ++;
            cube_rot (cube1p, CUBE_ROT_P90, cube2p);
            SWAP (int *, cube1p, cube2p);
        }
    }
    //fprintf (fp_out, "//k=%d\n", k);
    assert (k == 24);

    rottab_gen_update ();
}

// cube 内部编码 到ORI_XXX 的对照
const int g_cubeface_map2ori[CUBE_FACE_NUM] = {
    ORI_FRONT, ORI_NORTH, ORI_EAST, ORI_SOUTH, ORI_WEST, ORI_BACK,
};

// ORI_XXX 到 cube 内部编码的对照
const int g_cubeface_ori2map[CUBE_FACE_NUM] = {
    /* ORI_NORTH */ 1,
    /* ORI_EAST */  2,
    /* ORI_SOUTH */ 3,
    /* ORI_WEST */  4,
    /* ORI_FRONT */ 0,
    /* ORI_BACK */  5,
};

// use g_cubeface_map2ori

/*
<encoding3d>
    <facetenc>F,N,E,S,W,B</faceenc>
    <status id=0>0,1,2,3,4,5</status>
    <status id=1>0,4,1,2,3,5</status>
    <status id=2>0,3,4,1,2,5</status>
    <status id=3>0,2,3,4,1,5</status>
    ...
    <status id=23>5,4,3,2,1,0</status>
</encoding3d>
*/
static int
name2ori (char *name)
{
    int idx;
    idx = -1;
    if (0 == strcmp (name, "N")) {
        idx = ORI_NORTH;
    } else if (0 == strcmp (name, "E")) {
        idx = ORI_EAST;
    } else if (0 == strcmp (name, "S")) {
        idx = ORI_SOUTH;
    } else if (0 == strcmp (name, "W")) {
        idx = ORI_WEST;
    } else if (0 == strcmp (name, "F")) {
        idx = ORI_FRONT;
    } else if (0 == strcmp (name, "B")) {
        idx = ORI_BACK;
    }
    return idx;
}

// import and export the encoding system of 3D model
int
tssim_read_encoding_3d (xmlDocPtr doc, xmlNodePtr cur_child0)
{
#if USE_IMPORTENCODE
    xmlNodePtr cur_child;
    xmlChar *key;
    int xmlface2ori[ORI_MAX];
    int numlist[6];
    char buf[10];
    char *pcur;
    char *pnext;
    char flg_haveenc = 0;
    int ret;
    int id;
    int num;
    size_t i;

    assert (ORI_MAX == 6);
    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "reading encoded 3d status ....");
    memset (xmlface2ori, 0, sizeof (xmlface2ori));
    ret = 0;
    for (cur_child = cur_child0; NULL != cur_child; cur_child = cur_child->next) {
        if (! xmlStrcmp (cur_child->name, (const xmlChar *)"facetenc")) {
            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            pnext = (char *) key;
            for (i = 0; i < ORI_MAX - 1; i ++) {
                pcur = pnext;
                pnext = strchr (pnext, ',');
                if (NULL == pnext) {
                    // error
                    ret = -1;
                    break;
                }
                if (pnext - pcur >= sizeof (buf)) {
                    // error
                    ret = -1;
                    break;
                }
                memmove (buf, pcur, pnext - pcur);
                buf [pnext - pcur] = 0;
                num = name2ori (buf);
                if (num < 0) {
                    // error
                    ret = -1;
                    break;
                }
                xmlface2ori[i] = num;
                pnext ++;
            }
            if (i < ORI_MAX - 1) {
                // error
                ret = -1;
                xmlFree (key);
                break;
            }
            num = name2ori (pnext);
            xmlFree (key);
            if (num < 0) {
                // error
                ret = -1;
                break;
            }
            xmlface2ori[i] = num;
            flg_haveenc = 1;
            break;
        }
    }
    if (0 == flg_haveenc) {
        // error
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "not found 'faceenc', return -1 ....");
        return -1;
    }
    assert (ret >= 0);
    for (cur_child = cur_child0; NULL != cur_child; cur_child = cur_child->next) {
        if (! xmlStrcmp (cur_child->name, (const xmlChar *)"status")) {
            key = xmlGetProp (cur_child, (const xmlChar *)"id");
            id = atoi ((const char *)key);
            xmlFree (key);

            key = xmlNodeListGetString (doc, cur_child->xmlChildrenNode, 1);
            pnext = (char *)key;
            for (i = 0; i < ORI_MAX - 1; i ++) {
                numlist[i] = atoi (pnext);
                pnext = strchr (pnext, ',');
                if (NULL == pnext) {
                    // error
                    break;
                }
                pnext ++;
            }
            if (i < ORI_MAX - 1) {
                // error
                ret = -1;
                xmlFree (key);
                break;
            }
            numlist[i] = atoi (pnext);
            xmlFree (key);
            for (i = 0; i < ORI_MAX; i ++) {
                assert (numlist[i] < ORI_MAX);
                assert (xmlface2ori[numlist[i]] < ORI_MAX);
                assert (g_cubeface_ori2map[xmlface2ori[numlist[i]]] < ORI_MAX);
                // 译码
                g_cubeface_rotpos[id][i] = g_cubeface_ori2map[xmlface2ori[numlist[i]]];
            }
        }
    }
    if (ret < 0) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "return -1 ....");
        return -1;
    }
    // 更新表格
    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "rottab_gen_update() ....");
    rottab_gen_update ();

    return ret;
#else
    return 0;
#endif /* USE_IMPORTENCODE */
}

static const char g_ori2ch[CUBE_FACE_NUM] = {'N', 'E', 'S', 'W', 'F', 'B'};
void
tssim_save_encoding_3d (FILE *fp_xml)
{
#if USE_IMPORTENCODE
    size_t i, j;

    assert (ORI_MAX == 6);
    fprintf (fp_xml, "  <encoding3d>\n");
    fprintf (fp_xml, "    <facetenc>");
    i = 0;
    fprintf (fp_xml, "%c", g_ori2ch[g_cubeface_map2ori[i]]);
    i ++;
    for (; i < ORI_MAX; i ++) {
        fprintf (fp_xml, ",%c", g_ori2ch[g_cubeface_map2ori[i]]);
    }
    fprintf (fp_xml, "</facetenc>\n");
    for (i = 0; i < 24; i ++) {
        fprintf (fp_xml, "    <status id='%d'>", i);
        j = 0;
        fprintf (fp_xml, "%d", g_cubeface_rotpos[i][j]);
        j ++;
        for (; j < ORI_MAX; j ++) {
            fprintf (fp_xml, ",%d", g_cubeface_rotpos[i][j]);
        }
        fprintf (fp_xml, "</status>\n");
    }
    fprintf (fp_xml, "  </encoding3d>\n");
#endif /* USE_IMPORTENCODE */
}
