/******************************************************************************
 * Name:        cubeface.h
 * Purpose:     generate the static tables for changing the cube faces
 * Author:      Yunhui Fu
 * Created:     2009-08-13
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#ifndef CUBEFACE_H
#define CUBEFACE_H

// xml parser
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "pfutils.h"
_PSF_BEGIN_EXTERN_C

#define ORI_NORTH 0
#define ORI_EAST  1
#define ORI_SOUTH 2
#define ORI_WEST  3

#if USE_THREEDIMENTION
#define ORI_FRONT 4
#define ORI_BACK  5
#define ORI_MAX   6
#else
#define ORI_FRONT ORI_SOUTH /* used in g_slide_face_xyz */
#define ORI_BACK  ORI_NORTH /* used in g_slide_face_xyz */
#define ORI_MAX   4
#endif

#if USE_THREEDIMENTION
#define TILE_STATUS_MAX 24
#else
#define TILE_STATUS_MAX 4
#endif

#define CUBE_FACE_NUM 6

#define CUBE_ROT_NONE  0x00
#define CUBE_ROT_UP    0x01
#define CUBE_ROT_DOWN  0x02
#define CUBE_ROT_LEFT  0x03
#define CUBE_ROT_RIGHT 0x04
#define CUBE_ROT_P90   0x05 /* +90 clockwise */
#define CUBE_ROT_M90   0x06 /* -90 clockwise */
#define CUBE_ROT_MAX   0x07

#define CUBE_ENCODE 0
#if (CUBE_ENCODE == 0)
#define CUBE_GRAPH \
"    +---+\n"\
"    | 1 |\n"\
"+---+---+---+\n"\
"| 4 | 0 | 2 |\n"\
"+---+---+---+\n"\
"    | 3 |\n"\
"    +---+\n"\
"    | 5 |\n"\
"    +---+\n"

#define CUBE_FACE_ENC_LIST {0, 1, 2, 3, 4, 5}

#else
    // if you encode the cube as this:
    //        +------+
    //        | (1)3 |
    // +------+------+------+
    // | (4)0 | (0)1 | (2)2 |
    // +------+------+------+
    //        | (3)4 |
    //        +------+
    //        | (5)5 |
    //        +------+
    // then the g_cubeface_encode0 is:
    //int g_cubeface_encode0[CUBE_FACE_NUM] = {1, 3, 2, 4, 0, 5};
#define CUBE_GRAPH \
"    +---+\n"\
"    | 3 |\n"\
"+---+---+---+\n"\
"| 0 | 1 | 2 |\n"\
"+---+---+---+\n"\
"    | 4 |\n"\
"    +---+\n"\
"    | 5 |\n"\
"    +---+\n"

#define CUBE_FACE_ENC_LIST {1, 3, 2, 4, 0, 5}

#endif
extern int g_cubeface_encode0[CUBE_FACE_NUM];

// 各个状态的当前对应位置的编码
extern int g_cubeface_rotpos[24][CUBE_FACE_NUM];

// 各个状态分别进行一次 NONE,UP,DOWN,LEFT,RIGHT,P90,M90 后的状态
extern int g_cubeface_rottab[24][CUBE_ROT_MAX];

// 为了统一，使用连续 CUBE_ROT_P90,CUBE_ROT_DOWN,CUBE_ROT_P90 三次变换，将当前状态转换到任意其他24个状态
// 当前状态要切换到指定其他24个状态之一所需的步骤：g_cubeface_map_cur2all[cur_state][new_state][0~2],
// 3项对应的翻转步骤次数分别对应 P90,DOWN,P90
extern int g_cubeface_map_cur2all[24][24][3];

// 从当前状态切换到其他状态需要经过的步骤在g_cubeface_map_cur2all[0][idx] 的索引idx
extern int g_cubeface_rotnum_cur2all[24][24];

// 从当前面切换到其他6个面之一需要的步骤，[0]-P90, [1]-UP
extern const size_t g_cubeface_cur2all[CUBE_FACE_NUM][2];

#if DEBUG
// 测试用的数组
// 各个 [0]状态 在某个变换([1]P90,[2]DOWN,[3]P90)下所达到的状态
extern int g_cubeface_dbg_rotser2rotnum[24][4][4][4];
#endif

/*立方体先在 num_p90_1 次顺时针旋转，而后再 num_down次 下翻，最后 num_p90_2 次顺时针旋转 后，其位置状态*/
#define GET_CUBEFACE_4ROT_P90_DOWN_P90(num_p90_1, num_down, num_p90_2, cur_state, ret_state) \
{ \
    size_t iii; \
    ret_state = cur_state; \
    for (iii = 0; iii < (num_p90_1); iii ++) { \
        ret_state = g_cubeface_rottab[ret_state][CUBE_ROT_P90]; \
        assert ((0 <= ret_state) && (ret_state < 24)); \
    } \
    for (iii = 0; iii < (num_down); iii ++) { \
        ret_state = g_cubeface_rottab[ret_state][CUBE_ROT_DOWN]; \
        assert ((0 <= ret_state) && (ret_state < 24)); \
    } \
    for (iii = 0; iii < (num_p90_2); iii ++) { \
        ret_state = g_cubeface_rottab[ret_state][CUBE_ROT_P90]; \
        assert ((0 <= ret_state) && (ret_state < 24)); \
    } \
}

// （之前用户有可能改变了g_cubeface_rotpos）更新其他表格
extern void rottab_gen_update (void);

// 根据 g_cubeface_encode0 填充 g_cubeface_rotpos,g_cubeface_rottab,g_cubeface_map_cur2all
extern void rottab_gen_default (void);


// cube 内部编码 到ORI_XXX 的对照
extern const int g_cubeface_map2ori[CUBE_FACE_NUM];
// ORI_XXX 到 cube 内部编码的对照
extern const int g_cubeface_ori2map[CUBE_FACE_NUM];

extern void tssim_save_encoding_3d (FILE *fp_xml);
extern int tssim_read_encoding_3d (xmlDocPtr doc, xmlNodePtr cur_child0);

#define SWAP(type,p1,p2) \
    { type tmp; tmp=p1; p1=p2; p2=tmp; }

_PSF_END_EXTERN_C
#endif /* CUBEFACE_H */
