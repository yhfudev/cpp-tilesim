/******************************************************************************
 * Name:        convzzg2s.c
 * Purpose:     convert arbitrary zig-zag tile set at \tau=2 to \tau=1
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> // size_t
#include <string.h>
#include <getopt.h>

#include "cstrutils.h"
#include "zzconv.h"
#include "gentileset.h"
#include "gendecbit.h"
#include "gentypeenc.h"
#include "grouptiles.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

tm_tranfunc_t g_tmtran_tm_sierpinski[] = {
    {"q0", "0L", "q0", "0R", TM_TRAN_MOVE_LEFT},  // S1
    {"q0", "1L", "q1", "1R", TM_TRAN_MOVE_LEFT},  // S2
    {"q1", "0L", "q1", "1R", TM_TRAN_MOVE_LEFT},  // S3
    {"q1", "1L", "q0", "0R", TM_TRAN_MOVE_LEFT},  // S4
    {"q0", "0R", "q0", "0L", TM_TRAN_MOVE_RIGHT}, // S5
    {"q0", "1R", "q1", "1L", TM_TRAN_MOVE_RIGHT}, // S6
    {"q1", "0R", "q1", "1L", TM_TRAN_MOVE_RIGHT}, // S7
    {"q1", "1R", "q0", "0L", TM_TRAN_MOVE_RIGHT}, // S8

    {"q0",  "l", "q0",  "l", TM_TRAN_MOVE_LEFT},  // S9
    {"q1",  "l", "q1",  "l", TM_TRAN_MOVE_LEFT},  // S10
    {"q0",  "r", "q0",  "r", TM_TRAN_MOVE_RIGHT}, // S11
    {"q1",  "r", "q1",  "r", TM_TRAN_MOVE_RIGHT}, // S12
};

char *g_tmtape_tm_sierpinski[] = {
    "r",
    "1L", "1L", "1L", "1L",  "1L", "1L", "1L", "1L",
    "1L", "1L", "1L", "1L",  "1L", "1L", "1L", "1L",
    "1L", "1L", "1L", "1L",  "1L", "1L", "1L", "1L",
    "1L", "1L", "1L", "1L",  "1L", "1L", "1L", "1L",
    "l",
};

tm_info_t g_tminfo_tm_sierpinski = {
    NUM_TYPE(g_tmtran_tm_sierpinski, tm_tranfunc_t), g_tmtran_tm_sierpinski,
    NUM_TYPE(g_tmtape_tm_sierpinski, char *), g_tmtape_tm_sierpinski,
    "q0"
};

#if DEBUG
void
test_tmtf2zz (void)
{
    zigzag_info_t zzi;
    zzi_init (&zzi);
    zzi_tmi_to_zigzag2d2t (&zzi, &g_tminfo_tm_sierpinski);
    zzi_output_c (&zzi, "tm_sierpinski", stderr);
    zzi_output_tds_2d2t (&zzi, stdout);
    zzi_clear (&zzi);
}
#endif

glue_t g_lst_glues_arbitrary[] = {
    // let's ""=0 be the first one!
    {"",  0},
    {"N", 1},

    {"a", 1},
    {"b", 1},
    {"c", 1},
    {"d", 1},
    {"e", 1},
    {"x", 1},
    {"y", 1},
    {"r", 1},
    {"E1", 2},
    {"E2", 2},
    {"E3", 2},
    {"E4", 2},
    {"E5", 2},
    {"E6", 2},
    {"E7", 2},
    {"E8", 2},
    {"E9", 2},
    {"E10", 2},
    {"S1", 2},
    {"S2", 2},
    {"S3", 2},
    {"S4", 2},
    {"S5", 2},
    {"S6", 2},
    {"S7", 2},
    {"S8", 2},
    {"S9", 2},
    {"S10", 2},
    {"S11", 2},
    {"S12", 2},
};

tile_t g_lst_tiles_arbitrary[] = {
    // N,   E,   S,   W, name,label,type
    { "c", "S1",   "",   "",  "SEED", "", TILETYPE_FE},
    { "a", "S2",   "", "S1",  "2", "", TILETYPE_FE},
    {"E1",   "",   "", "S2",  "3", "", TILETYPE_SEN2},
    {  "",   "", "E1",  "x",  "4", "", TILETYPE_TEW1},
    { "a",  "x",  "a",  "x",  "5", "", TILETYPE_DWW1},
    { "b",  "x",  "c", "S3",  "6", "", TILETYPE_DWW2},
    { "d", "S3",   "", "S4",  "7", "", TILETYPE_FW},
    {"E2", "S4",   "",   "",  "8", "", TILETYPE_SWN2}, // ??
    { "N",  "r", "E2",   "",  "9", "", TILETYPE_TWE1}, // if the north is set to null, then decode tile can be random attached
    { "d",  "r",  "d",  "r", "10", "", TILETYPE_DEE1},
    { "b",  "r",  "b",  "r", "11", "", TILETYPE_DEE1},
    {"E3",   "",  "a",  "r", "12", "", TILETYPE_DEN2},
    { "e",   "", "E3",  "x", "13", "", TILETYPE_TEW1},
    { "b",  "x",  "b",  "x", "14", "", TILETYPE_DWW1},
    {"E4",  "x",  "d",   "", "15", "", TILETYPE_DWN2},
    { "c",  "r", "E4",   "", "16", "", TILETYPE_TWE1},
    //{ "b",  "r",  "b",  "r", "17", "", TILETYPE_DEE1}, // same as 11
    { "e", "S5",  "e",  "r", "18", "", TILETYPE_DEE2},
    { "a", "S6",   "", "S5", "19", "", TILETYPE_FE},
    {"E5",   "",   "", "S6", "20", "", TILETYPE_SEN2},
    {  "",   "", "E5",  "x", "21", "", TILETYPE_TEW1},
    //{ "a",  "x",  "a",  "x", "22", "", TILETYPE_DWW1}, // same as 5
    { "d",  "x",  "e",  "y", "23", "", TILETYPE_DWW1},
    { "b",  "y",  "b",  "y", "24", "", TILETYPE_DWW1},
    {"E6",  "y",  "c",   "", "25", "", TILETYPE_DWN2},
    {  "", "S7", "E6",   "", "26", "", TILETYPE_TWE2},
    {  "", "S8",   "", "S7", "27", "", TILETYPE_FE},
    {"E7",   "",   "", "S8", "28", "", TILETYPE_SEN2},
    {  "",   "", "E7", "S9", "29", "", TILETYPE_TEW2},
    {"E8", "S9",   "",   "", "30", "", TILETYPE_SWN2},
    {  "",   "", "E8","S10", "31", "", TILETYPE_TWW2},
    {"E9","S10",   "",   "", "32", "", TILETYPE_SWN2},
    {  "","S11", "E9",   "", "33", "", TILETYPE_TWE2},
    {"E10",  "",   "","S11", "34", "", TILETYPE_SEN2},
    {  "","S12","E10",   "", "35", "", TILETYPE_TEE2},
    {  "",   "",   "","S12", "36", "", TILETYPE_FE},
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_arbitrary = {
    NUM_TYPE(g_lst_tiles_arbitrary, tile_t), g_lst_tiles_arbitrary,
    NUM_TYPE(g_lst_glues_arbitrary, glue_t), g_lst_glues_arbitrary,
    2,
};

glue_t g_lst_glues_arbitrary2[] = {
    // let's ""=0 be the first one!
    {"",  0},
    {"N", 1},

    {"a", 1},
    {"b", 1},
    {"c", 1},
    {"r", 1},
    {"rr", 1},
    {"x", 1},
    {"y", 1},
    {"E1", 2},
    {"E2", 2},
    {"E3", 2},
    {"S1", 2},
    {"S2", 2},
    {"S3", 2},
    {"S4", 2},
    {"S5", 2},
    {"S6", 2},
};

tile_t g_lst_tiles_arbitrary2[] = {
    // N,   E,   S,   W, name,label,type
    { "r", "S1",   "",   "", "SEED", "", TILETYPE_FE},
    { "b", "S2",   "", "S1", "02", "", TILETYPE_FE},
    { "a", "S3",   "", "S2", "03", "", TILETYPE_FE},
    {"E1",   "",   "", "S3", "04", "", TILETYPE_SEN2},
    { "r",   "", "E1",  "x", "05", "", TILETYPE_TEW1},
    { "a",  "x",  "a",  "x", "06", "", TILETYPE_DWW1},
    { "b",  "x",  "b",  "x", "07", "", TILETYPE_DWW1},
    {"E2",  "x",  "r",   "", "08", "", TILETYPE_DWN2},
    { "N",  "y", "E2",   "", "09", "", TILETYPE_TWE1}, // if the north is set to null, then decode tile can be random attached
    { "c",  "y",  "b",  "y", "10", "", TILETYPE_DEE1},
    { "a",  "y",  "a",  "y", "11", "", TILETYPE_DEE1},
    {"E1",   "",  "r",  "y", "12", "", TILETYPE_DEN2},
    { "a",  "x",  "c", "S4", "15", "", TILETYPE_DWW2},
    { "c", "S4",   "", "S5", "16", "", TILETYPE_FW},
    {"E2", "S5",   "",   "", "17", "", TILETYPE_SWN2},
    {"rr",  "y",  "c",  "y", "19", "", TILETYPE_DEE1},
    {"E3",  "x", "rr",   "", "26", "", TILETYPE_DWN2},
    { "r", "S6", "E3",   "", "27", "", TILETYPE_TWE2},
    { "a", "S2",   "", "S6", "28", "", TILETYPE_FE},
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_arbitrary2 = {
    NUM_TYPE(g_lst_tiles_arbitrary2, tile_t), g_lst_tiles_arbitrary2,
    NUM_TYPE(g_lst_glues_arbitrary2, glue_t), g_lst_glues_arbitrary2,
    2,
};

glue_t g_lst_glues_robert[] = {
    // let's ""=0 be the first one!
    {"",  0},

    {"a", 1},
    {"b", 1},
    {"c", 1},
    {"d", 1},
    {"r", 1},
    {"x", 1},
    {"y", 1},
    {"E", 2},
    {"F", 2},
    {"NULL",  2},

    {"SA0_1", 2},
    {"SA1_2", 2},
    {"SA2_3", 2},

#if 0
    {"SB0_1", 2},
    {"SB1_2", 2},
    {"SB2_3", 2},
#endif
};

tile_t g_lst_tiles_robert[] = {
    // N,   E,   S,   W, name,label,type
    {"F", "y", "c",  "",  "L2", "", TILETYPE_DWN2},
    {"a", "y", "a", "y", "SL1", "", TILETYPE_DWW1},
    {"b", "x", "b", "x", "SL2", "", TILETYPE_DWW1},
    {"b", "x", "a", "y", "SL3", "", TILETYPE_DWW1},
    {"d",  "", "E", "x",  "R1", "", TILETYPE_TEW1},
    {"c", "r", "F",  "",  "L1", "", TILETYPE_TWE1},
    {"a", "r", "a", "r", "SR1", "", TILETYPE_DEE1},
    {"b", "r", "b", "r", "SR2", "", TILETYPE_DEE1},
    {"E",  "", "d", "r",  "R2", "", TILETYPE_DEN2},

    // the end of the tile set
    {"NULL", "x", "c",  "",  "END", "", TILETYPE_DWN2},

    { "E",      "", "", "SA0_1", "S0",   "0", TILETYPE_SEN2},
    { "c", "SA2_3", "",      "", "SEED", "0", TILETYPE_FE},
    { "a", "SA1_2", "", "SA2_3", "S2",   "0", TILETYPE_FE},
    { "a", "SA0_1", "", "SA1_2", "S1",   "0", TILETYPE_FE},
#if 0
    // another seed bar
    { "F",      "", "", "SB0_1", "SB0",   "0", TILETYPE_SEN2},
    { "c", "SB2_3", "",      "", "SEED_B", "0", TILETYPE_FE},
    { "a", "SB1_2", "", "SB2_3", "SB2",   "0", TILETYPE_FE},
    { "a", "SB0_1", "", "SB1_2", "SB1",   "0", TILETYPE_FE},
#endif
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_robert = {
    NUM_TYPE(g_lst_tiles_robert, tile_t), g_lst_tiles_robert,
    NUM_TYPE(g_lst_glues_robert, glue_t), g_lst_glues_robert,
    2,
};

glue_t g_lst_glues_winfree5[] = {
    // let's ""=0 be the first one!
    {"",  0},

    {"0", 1},
    {"1", 1},
    {"c", 1},
    {"n", 1},
    {"x", 1},
    {"*0", 1},
    {"*1", 1},
    {"0*", 1},
    {"1*", 1},
    {"ZZ1", 2},
    {"ZZ2", 2},
    {"FF1", 2},
    {"FZ1", 2},
    {"FZ2", 2},
    {"ZF1", 2},

    // glues in the seed bar
    {"SA0_1", 2},
    {"SA1_2", 2},
    {"SA2_3", 2},
    {"SA3_4", 2},
};

tile_t g_lst_tiles_winfree5[] = {
    // N,   E,   S,   W, name,label,type
    {"0", "n", "0", "n", "(n,0)", "0", TILETYPE_DWW1},
    {"1", "n", "1", "n", "(n,1)", "1", TILETYPE_DWW1},
    {"1", "c", "0", "n", "(c,0)", "1", TILETYPE_DWW1},
    {"0", "c", "1", "c", "(c,1)", "0", TILETYPE_DWW1},
    {"0", "x", "0", "x", "(x,0)", "0", TILETYPE_DEE1},
    {"1", "x", "1", "x", "(x,1)", "1", TILETYPE_DEE1},
    {"ZZ1", "n", "*0", "", "LA00", "0", TILETYPE_DWN2},
    {"FF1", "n", "*1", "", "LA01", "1", TILETYPE_DWN2},
    {"FF1", "c", "*0", "", "LA02", "1", TILETYPE_DWN2},
    {"ZZ2", "c", "*1", "", "LA03", "0", TILETYPE_DWN2},
    {"*0", "x", "ZZ1", "", "LA04", "0", TILETYPE_TWE1},
    {"*1", "x", "FF1", "", "LA05", "1", TILETYPE_TWE1},
    {"0*", "", "FZ1", "c", "RA00", "0", TILETYPE_TEW1},
    {"0*", "", "FZ2", "c", "RA01", "0", TILETYPE_TEW1},
    {"1*", "", "ZF1", "n", "RA02", "1", TILETYPE_TEW1},
    {"ZF1", "", "0*", "x", "RA03", "0", TILETYPE_DEN2},
    {"FZ2", "", "1*", "x", "RA04", "1", TILETYPE_DEN2},

    {"FZ1",      "", "", "SA0_1",  "SA0", "1", TILETYPE_SEN2},
    {  "0", "SA0_1", "", "SA1_2",  "SA1", "0", TILETYPE_FE},
    {  "0", "SA1_2", "", "SA2_3",  "SA2", "0", TILETYPE_FE},
    {  "1", "SA2_3", "", "SA3_4",  "SA3", "1", TILETYPE_FE},
    { "*0", "SA3_4", "",      "", "SEED", "0", TILETYPE_FE},
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_winfree5 = {
    NUM_TYPE(g_lst_tiles_winfree5, tile_t), g_lst_tiles_winfree5,
    NUM_TYPE(g_lst_glues_winfree5, glue_t), g_lst_glues_winfree5,
    2,
};

glue_t g_lst_glues_winfree6[] = {
    // let's ""=0 be the first one!
    {"",  0},

    {"00", 1},
    {"11", 1},
    {"*00", 1},
    {"*11", 1},
    {"00*", 1},
    {"11*", 1},
    {"n", 1},
    {"c", 1},
    {"x", 1},
    {"FFa", 2},
    {"ZZa", 2},
    {"FZa", 2},
    {"ZFa", 2},
    {"NULLa", 2},
    //{"ZZ2", 2},

    // glues in the seed bar
    {"SB0_1", 2},
    {"SB1_2", 2},
    {"SB2_3", 2},
    {"SB3_4", 2},
};

tile_t g_lst_tiles_winfree6[] = {
    // N,   E,   S,   W, name,label,type
    {"00", "n", "00", "n", "FB0", "0", TILETYPE_DWW1},
    {"11", "n", "11", "n", "FB1", "1", TILETYPE_DWW1},
    {"11", "c", "00", "n", "FB2", "1", TILETYPE_DWW1},
    {"00", "c", "11", "c", "FB3", "0", TILETYPE_DWW1},
    {"00", "x", "00", "x", "FB4", "0", TILETYPE_DEE1},
    {"11", "x", "11", "x", "FB5", "0", TILETYPE_DEE1},
    {"ZZa", "n", "*00", "", "LB0", "0", TILETYPE_DWN2},
    {"FFa", "n", "*11", "", "LB1", "1", TILETYPE_DWN2},
    {"FFa", "c", "*00", "", "LB2", "1", TILETYPE_DWN2},
    {"NULLa", "c", "*11", "", "LB3", "0", TILETYPE_DWN2},
    {"*00", "x", "ZZa", "", "LB4", "0", TILETYPE_TWE1},
    {"*11", "x", "FFa", "", "LB5", "1", TILETYPE_TWE1},
    {"00*", "", "FZa", "c", "RB0", "0", TILETYPE_TEW1},
    {"11*", "", "ZFa", "n", "RB1", "1", TILETYPE_TEW1},
    {"ZFa", "", "00*", "x", "RB2", "0", TILETYPE_DEN2},
    {"FZa", "", "11*", "x", "RB3", "0", TILETYPE_DEN2},

    {"ZFa",      "", "", "SB0_1", "SB0", "1", TILETYPE_SEN2},
    { "00", "SB0_1", "", "SB1_2", "SB1", "0", TILETYPE_FE},
    { "00", "SB1_2", "", "SB2_3", "SB2", "0", TILETYPE_FE},
    { "00", "SB2_3", "", "SB3_4", "SB3", "0", TILETYPE_FE},
    {"*00", "SB3_4", "",      "", "SEED", "0", TILETYPE_FE},
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_winfree6 = {
    NUM_TYPE(g_lst_tiles_winfree6, tile_t), g_lst_tiles_winfree6,
    NUM_TYPE(g_lst_glues_winfree6, glue_t), g_lst_glues_winfree6,
    2,
};

#if 0
// Optimal Self-Assembly of Counters at Temperature Two
glue_t g_lst_glues_optimal_bincnt[] = {
    // let's ""=0 be the first one!
    {"",  0},

    {"c", 1},
    {"s", 1},
    {"n", 1},
    {"u", 1},
    {"l", 1},
    {"z", 1},
    {"a", 2},
    {"b", 2},

    // glues in the seed bar
    {"S0_1", 2},
    {"S1_2", 2},
    {"S2_3", 2},
    //{"SB3_4", 2},
};

tile_t g_lst_tiles_optimal_bincnt[] = {
    // N,   E,   S,   W, name,label,type
    /* 0*/{"a", "s", "n", "c", "0M", "0", TILETYPE_DWN2},
    /* 1*/{"u", "s", "u", "s", "1S", "1", TILETYPE_DWW1},
    /* 2*/{"l",  "", "b", "s", "1E", "1", TILETYPE_TEW1},
    /* 3*/{"u", "c", "u", "c", "1C", "1", TILETYPE_DWW1},
    /* 4*/{"n", "c", "n", "c", "0C", "0", TILETYPE_DWW1},
    /* 5*/{"u", "z", "a", "c", "1M", "1", TILETYPE_TWE1},
    /* 6*/{"n", "z", "u", "z", "0Z", "0", TILETYPE_DEE1},
    /* 7*/{"b",  "", "l", "z", "0E", "0", TILETYPE_DEN2},

    /* 8*/{"n", "S0_1", "",     "", "SEED", "0", TILETYPE_FE},
    /* 9*/{"n", "S1_2", "", "S0_1", "S1", "0", TILETYPE_FE},
    /* 10*/{"n", "S2_3", "", "S1_2", "S2", "0", TILETYPE_FE},
    /* 11*/{"b",     "", "", "S2_3", "S3", "0", TILETYPE_SEN2},
    //{NULL, NULL, NULL, NULL, NULL, NULL, TILETYPE_NONE},
};

zigzag_info_t bczz_optimal_bincnt = {
    NUM_TYPE(g_lst_tiles_optimal_bincnt, tile_t), g_lst_tiles_optimal_bincnt,
    NUM_TYPE(g_lst_glues_optimal_bincnt, glue_t), g_lst_glues_optimal_bincnt,
    2,
};
#endif // 0

glue_t g_lst_glues_winfree6_triangle[] = {
    // let's ""=0 be the first one!
    {"",  0},

    {"00", 1},
    {"11", 1},
    {"*00", 1},
    {"*11", 1},
    {"00*", 1},
    {"11*", 1},
    {"n", 1},
    {"c", 1},
    {"x", 1},
    {"FFa", 2},
    {"ZZa", 2},
    {"FZa", 2},
    {"ZFa", 2},
    //{"NULLa", 2},
    //{"ZZ2", 2},
    // new added
    {"En*00", 2},
    {"En*11", 2},
    {"Ec*00", 2},
    //{"Ec*11", 2},
    {"Ex00*", 2},
    {"Ex11*", 2},
};

tile_t g_lst_tiles_winfree6_triangle[] = {
    // N,   E,   S,   W, name,label,type
    {"00", "n", "00", "n", "FB0", "0", TILETYPE_DWW1},
    {"11", "n", "11", "n", "FB1", "1", TILETYPE_DWW1},
    {"11", "c", "00", "n", "FB2", "1", TILETYPE_DWW1},
    {"00", "c", "11", "c", "FB3", "0", TILETYPE_DWW1},
    {"00", "x", "00", "x", "FB4", "0", TILETYPE_DEE1},
    {"11", "x", "11", "x", "FB5", "0", TILETYPE_DEE1},

    {"11", "n", "*00", "En*00", "LB0a", "0", TILETYPE_DWW2},
    {"ZZa", "En*00", "", "", "LB0b", "0", TILETYPE_SWN2},

    {"11", "n", "*11", "En*11", "LB1a", "1", TILETYPE_DWW2},
    {"FFa", "En*11", "", "", "LB1b", "1", TILETYPE_SWN2},

    {"11", "c", "*00", "Ec*00", "LB2a", "1", TILETYPE_DWW2},
    {"FFa", "Ec*00", "", "", "LB2b", "1", TILETYPE_SWN2},

    // the end of the zigzag bar:
    //{"11", "c", "*11", "Ec*11", "LB3a", "0", TILETYPE_DWW2},
    //{"NULLa", "Ec*11", "", "", "LB3b", "0", TILETYPE_SWN2},

    {"*00", "x", "ZZa", "", "LB4", "0", TILETYPE_TWE1},
    {"*11", "x", "FFa", "", "LB5", "1", TILETYPE_TWE1},
    {"00*", "", "FZa", "c", "RB0", "0", TILETYPE_TEW1},
    {"11*", "", "ZFa", "n", "RB1", "1", TILETYPE_TEW1},

    {"11", "Ex00*", "00*", "x", "RB2a", "0", TILETYPE_DEE2},
    {"ZFa", "", "", "Ex00*", "RB2b", "0", TILETYPE_SEN2},

    {"11", "Ex11*", "11*", "x", "RB3a", "0", TILETYPE_DEE2},
    {"FZa", "", "", "Ex11*", "RB3b", "0", TILETYPE_SEN2},

    { "*00", "Ex00*", "", "", "SEED", "0", TILETYPE_FE},
};

zigzag_info_t bczz_winfree6_triangle = {
    NUM_TYPE(g_lst_tiles_winfree6_triangle, tile_t), g_lst_tiles_winfree6_triangle,
    NUM_TYPE(g_lst_glues_winfree6_triangle, glue_t), g_lst_glues_winfree6_triangle,
    2,
};

int
change_stdout_to_file (const char *fname)
{
    int fd_out;
    fd_out = open (fname, O_CREAT | O_RDWR, 0666);
    if (fd_out < 0) {
        perror("open()");
        return -1;
    }
    close (1);
    dup2 (fd_out, 1);
    return fd_out;
}

int
convert_tds (const char *fn_in, const char *fn_out_prefix, size_t k, char flg_xgrow, size_t max_steps)
{
    int ret = 0;
    char *p;
    zigzag_info_t zzi;
    zzconv_t *pzzc;
    int fd_out;
    char newfname[250];
    FILE *fp;

    p = strrstr_len (fn_in, strlen(fn_in), ".tds");
    if (NULL != p) {
        memmove (newfname, fn_in, p - fn_in);
        newfname [p - fn_in] = 0;
        strcat (newfname, ".tdp");
        // TODO: check if the .tdp exist
        fp = fopen (newfname, "w");
        if (NULL != fp) {
            fprintf (fp, "%s\nTemperature=2\nSEED 0 0 0\n", fn_in);
            fclose (fp);
            fn_in = newfname;
        }
    }
    if (NULL == fn_out_prefix) {
        p = strrstr_len (fn_in, strlen(fn_in), ".");
        if (NULL == p) {
            p = (char *)fn_in + strlen (fn_in);
        }
    }
    zzi_init (&zzi);
    zzi_load_tdp (fn_in, &zzi);
    zzi_output_c (&zzi, (char *)fn_in, stdout);
    zzi_group_tiles (&zzi, fn_in, max_steps);
    zzi_output_c (&zzi, (char *)fn_in, stdout);

    pzzc = zzconv_create (&zzi);

    if (NULL == fn_out_prefix) {
        if (fn_in != newfname) {
            memmove (newfname, fn_in, p - fn_in);
        }
        newfname [p - fn_in] = 0;
    } else {
        strcpy (newfname, fn_out_prefix);
        p = newfname + strlen (fn_out_prefix);
    }
    strcat (newfname, "_2d1t.tdp");
    fp = fopen (newfname, "w");
    *p = 0;
    strcat (newfname, "_2d1t.tds");
    if (NULL != fp) {
        fprintf (fp, "%s\nTemperature=1\nSEED 0 0 0\n", newfname);
        fclose (fp);
    }
    fp = fopen (newfname, "w");
    if (NULL != fp) {
        ret = zzconv_conv_categories_to_2d1t (pzzc, fp, k);
        fclose (fp);
        if (NULL == fn_out_prefix) {
            if (fn_in != newfname) {
                memmove (newfname, fn_in, p - fn_in);
            }
            newfname [p - fn_in] = 0;
        } else {
            strcpy (newfname, fn_out_prefix);
            p = newfname + strlen (fn_out_prefix);
        }
        strcat (newfname, "_3d1t.tdp");
        fp = fopen (newfname, "w");
        *p = 0;
        strcat (newfname, "_3d1t.tds");
        if (NULL != fp) {
            fprintf (fp, "%s\nTemperature=1\nSEED 0 0 0\n", newfname);
            fclose (fp);
        }
        fp = fopen (newfname, "w");
        if (NULL != fp) {
            ret = zzconv_conv_categories_to_3d1t (pzzc, fp);
            fclose (fp);
        } else {
            perror("change_stdout_to_file()");
            //exit (0);
        }
    } else {
        perror("change_stdout_to_file()");
        //exit (0);
    }

    zzconv_destroy (pzzc);
    if (flg_xgrow) {
        *p = 0;
        strcat (newfname, ".tiles");
        fp = fopen (newfname, "w");
        if (NULL != fp) {
            zzi_output_xgrow_tiles (&zzi, fp);
            fclose (fp);
        }
        *p = 0;
        strcat (newfname, "_2d1t.tiles");
        fp = fopen (newfname, "w");
        if (NULL != fp) {
            size_t i;
            zigzag_info_t zzi_tmp;
            zzi_init (&zzi_tmp);
            *p = 0;
            strcat (newfname, "_2d1t.tdp");
            zzi_load_tdp (newfname, &zzi_tmp);
            // change the glue strengh from 1 to 2
            for (i = 0; i < zzi_tmp.sz_glues; i ++) {
                zzi_tmp.glues[i].strength = zzi_tmp.glues[i].strength * 2;
            }
            zzi_group_tiles (&zzi_tmp, newfname, max_steps);
            zzi_output_xgrow_tiles (&zzi_tmp, fp);
            zzi_clear (&zzi_tmp);
        }
        // xgrow don't support 3D. :-(
    }
    zzi_clear (&zzi);
    return ret;
}

int
self_test_item (zigzag_info_t *pzzi, size_t k, char flg_xgrow, size_t max_steps, const char *fn_out)
{
    FILE *fp;
    zzconv_t *pzzc;
    fp = fopen (fn_out, "w");
    if (NULL != fp) {
        zzi_output_tds_2d2t (pzzi, fp);
        fclose (fp);
    }
#if 0 // debug
{
    // temp file:
    fp = fopen ("temptemp.tdp", "w");
    if (NULL != fp) {
        fprintf (fp, "%s\nTemperature=2\nSEED 0 0 0\n", fn_out);
        fclose (fp);
{
    size_t temp;
    zigzag_info_t zzi;
    zzi_init (&zzi);
    zzi_load_tdp ("temptemp.tdp", &zzi, &temp);
    if (temp != 2) {
        fprintf (stderr, "ERR in zzi_load_tdp() '%s' , temp(%d) != 2\n", fn_out, temp);
        return -1;
    }
    // compare it:
#if 1
    if (zzi_comapre (pzzi, &zzi, 0) < 0) {
        fprintf (stderr, "ERR in zzi_load_tdp() '%s', body\n", fn_out);
        return -1;
    } else {
        fprintf (stderr, "zzi_load_tdp() '%s' OK!\n", fn_out);
    }
    zzi_group_tiles (&zzi, max_steps);
#endif
    if (zzi_comapre (pzzi, &zzi, 1) < 0) {
        fprintf (stderr, "ERR in group_tiles() '%s'\n", fn_out);
        return -1;
    } else {
        fprintf (stderr, "group_tiles() '%s' OK!\n", fn_out);
    }
    fprintf (stderr, "zzi_load_tdp '%s' ok!\n", fn_out);
    zzi_clear (&zzi);
}
    }
}
#endif
    convert_tds (fn_out, fn_out, k, flg_xgrow, max_steps);

    if (test_grouptiles (pzzi, fn_out) < 0) {
        fprintf (stderr, "ERR in test_grouptiles() '%s' \n", fn_out);
    } else {
        fprintf (stderr, "test_grouptiles() '%s' OK!\n", fn_out);
    }
    return 0;
}

int
output_examples (size_t k, char flg_xgrow)
{
    int ret = 0;

    fprintf (stderr, "output example: arbitrary ...\n");
    if (self_test_item (&bczz_arbitrary, k, flg_xgrow, 20000, "zzi_arbitrary.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: arbitrary2 ...\n");
    if (self_test_item (&bczz_arbitrary2, k, flg_xgrow, 20000, "zzi_arbitrary2.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: robert ...\n");
    if (self_test_item (&bczz_robert,    k, flg_xgrow, 20000, "zzi_robert.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: winfree5 ...\n");
    if (self_test_item (&bczz_winfree5,  k, flg_xgrow, 20000, "zzi_winfree5.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: winfree6 ...\n");
    if (self_test_item (&bczz_winfree6,  k, flg_xgrow, 20000, "zzi_winfree6.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: winfree6_triangle ...\n");
    if (self_test_item (&bczz_winfree6_triangle, k, flg_xgrow, 8000, "zzi_winfree6_triangle.tds") < 0) {
        return -1;
    }
    fprintf (stderr, "output example: TM sierpinski ...\n");
{
    zigzag_info_t zzi;
    zzi_init (&zzi);
    zzi_tmi_to_zigzag2d2t (&zzi, &g_tminfo_tm_sierpinski);
    zzi_output_c (&zzi, "tm_sierpinski", stderr);
    zzi_output_tds_2d2t (&zzi, stdout);
    if (self_test_item (&zzi,  k, flg_xgrow, 1000, "zzi_tm_sierpinski.tds") < 0) {
        ret = -1;
    }
    zzi_clear (&zzi);
}

    //fprintf (stderr, "output example: bczz_optimal_bincnt ...\n");
    //if (self_test_item (&bczz_optimal_bincnt, k, flg_xgrow, 20000, "zzi_optbincnt.tds") < 0) {
        //return -1;
    //}

    return ret;
}

static void
version (FILE *out_stream)
{
    fprintf( out_stream, "ZigZag t=1 convertor version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
    fprintf( out_stream, "Copyright 2009 Yunhui Fu (yhfudev@gmail.com)\n\n" );
}

static void
help (FILE *out_stream, const char *progname)
{
    static const char *help_msg[] = {
        "Command line version of Zigzag Convertor, Convert zigzag tiles from t=2 to t=1 ", 
        "",
        "-e --example               output some examples",
        "-i --input <file name>     the input file for convertion",
        "-o --outprefix <prefix>    specify the prefix of the output file",
        "-k --k <value>             specify k in 2d1t",
        "-1 --temp1                 convert 2d2t to 2d1t and 3d1t",
        "-s --maxsteps              the max steps to detect the types of tiles",
        "-w --xgrow                 also output to winfree's xgrow data file",
        "-V --version               display version information",
        "   --help                  display this help",
        "-v --verbose               be verbose",
        0 };
   const char **p = help_msg;

   fprintf (out_stream, "Usage: %s [options]\n", progname);
   while (*p) fprintf (out_stream, "%s\n", *p++);
}

int
main (int argc, char *argv[])
{
    int ret = 0;
    size_t max_steps = 0;
    const char *fn_in = "";
    const char *fn_out = NULL;
    char flg_example = 0;
    //char flg_2d = 0;
    char flg_temp1 = 0;
    char flg_xgrow = 0;
    size_t k = 5;
    int c;
    struct option longopts[]  = {
        //{ "2d",         0, 0, '2' },
        //{ "3d",         0, 0, '3' },
        { "example",    0, 0, 'e' },
        { "k",          1, 0, 'k' },
        { "maxsteps",   1, 0, 's' },
        { "temp1",      1, 0, '1' },
        { "xgrow",      1, 0, 'w' },
        { "input",      1, 0, 'i' },
        { "outprefix",  1, 0, 'o' },
        { "version",    0, 0, 'V' },
        { "help",       0, 0, 501 },
        { "verbose",    0, 0, 'v' },
        { 0,            0, 0,  0  },
    };

    while ((c = getopt_long( argc, argv, "i:o:k:s:1ewVLv", longopts, NULL )) != EOF) {
        switch (c) {
        //case '2': flg_3d = 1; break;
        //case '3': flg_3d = 1; break;
        case 'e': flg_example = 1; break;
        case 'i': fn_in = optarg; break;
        case 'o': fn_out = optarg; break;
        case 'k': k = atoi (optarg); break;
        case 's': max_steps = atoi (optarg); break;
        case '1': flg_temp1 = 1; break;
        case 'w': flg_xgrow = 1; break;
        case 'V': version (stdout); exit(1); break;
        case 'v': break;
        default:
        case 501: help (stdout, argv[0]); exit(1); break;
        }
    }
    //extern void test_revert (void);test_revert ();
    printf ("file in: '%s'; file out: '%s'\n", fn_in, fn_out);
    printf ("k=%d; max_steps=%d\n", k, max_steps);

    if (flg_example) {
        ret = output_examples (k, flg_xgrow);
        exit (ret);
    }
    if (NULL == fn_in) {
        printf ("Please specify the input file.\n");
    }
    if (flg_temp1) {
        convert_tds (fn_in, fn_out, k, flg_xgrow, max_steps);
    } else if (flg_xgrow) {
        zigzag_info_t zzi;
        FILE *fpout = stdout;
        if (fn_out) {
            fpout = fopen (fn_out, "w");
            if (NULL == fpout) {
                printf ("Error in open file: %s\n", fn_out);
                exit (1);
            }
        }
        // convert to another format?
        zzi_init (&zzi);
        zzi_load_tdp (fn_in, &zzi);
        zzi_group_tiles (&zzi, "test", 20000);
        zzi_output_xgrow_tiles (&zzi, fpout);
        zzi_clear (&zzi);
        if (NULL != fn_out) {
            fclose (fpout);
        }
    }

    return ret;
}

void
test_testbit (void)
{
    size_t val;
    unsigned char *p = (unsigned char *)&val;
    val = htonl (0x00000000);
    assert (0 == NBO_GET_BIT(p, 0));
    assert (0 == NBO_GET_BIT(p, 31));
    val = htonl (0x80000000);
    assert (1 == NBO_GET_BIT(p, 0));
    assert (0 == NBO_GET_BIT(p, 1));
    assert (0 == NBO_GET_BIT(p, 10));
    assert (0 == NBO_GET_BIT(p, 20));
    assert (0 == NBO_GET_BIT(p, 30));
    assert (0 == NBO_GET_BIT(p, 31));
    val = htonl (0x40000010);
    assert (0 == NBO_GET_BIT(p, 0));
    assert (1 == NBO_GET_BIT(p, 1));
    assert (0 == NBO_GET_BIT(p, 2));
    assert (0 == NBO_GET_BIT(p, 3));
    assert (0 == NBO_GET_BIT(p, 10));
    assert (0 == NBO_GET_BIT(p, 20));
    assert (0 == NBO_GET_BIT(p, 25));
    assert (0 == NBO_GET_BIT(p, 26));
    assert (1 == NBO_GET_BIT(p, 27));
    assert (0 == NBO_GET_BIT(p, 28));
    assert (0 == NBO_GET_BIT(p, 29));
    assert (0 == NBO_GET_BIT(p, 31));
}
