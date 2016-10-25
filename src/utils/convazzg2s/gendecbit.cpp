/******************************************************************************
 * Name:        gendecbit.cpp
 * Purpose:     generate tile set for one bits of the decode tile set
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>

#include <sstream>

#include "gendecbit.h"
#include "outtds.h"

// the times of the '0' tiles to be build
#define NUM_DUP_2D1T 8

// glue_input: the name of the input glue
// glue_output_0: the name of the output glue at position 0
// glue_output_1: the name of the output glue at position 1
int
gen_3d1t_dectile_1bit_2left (FILE *fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, zzconv_cb_output_t cb_output)
{
    size_t idx;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];

    assert ((NULL == glue_output_0) || ((void *)0x05 < glue_output_0));
    assert ((NULL == glue_output_1) || ((void *)0x05 < glue_output_1));

    if ((NULL == glue_output_0) && (NULL == glue_output_1)) {
        return -1;
    }
    idx = 0;
    sprintf (label, "");
    sprintf (name, "%sT_00", tilename, idx);
    sprintf (glue_in, "%sG_00", tilename);
    sprintf (glue_out, "%sG_01", tilename);
    cb_output (fpout, name, label, ZZCOUV_COLOR_DECI,
            /*N*/((NULL == glue_output_0)?NULL:glue_in),
            /*E*/glue_input,
            /*S*/((NULL == glue_output_1)?NULL:glue_out),
            /*W*/NULL,
            /*F*/NULL,
            /*B*/NULL);
    idx = 2;
    if (NULL != glue_output_0) {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, 0);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DEC,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DECI,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/glue_out,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%s", glue_output_0);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DECI,
                /*N*/glue_in,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
    }
    if (NULL != glue_output_1) {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DEC,
                /*N*/glue_in,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DECI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%s", glue_output_1);
        cb_output (fpout, name, label, ZZCOUV_COLOR_DECI,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
    }
    return 0;
}

int
gen_2d1t_dectile_1bit_2left (FILE * fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k, zzconv_cb_output_t cb_output)
{
    size_t i;
    size_t idx_glue; // the index of the glue
    size_t idx_tile; // the index of the current tile to be created
    size_t idx_out0; // the glue output 0 of the last tile sets
    size_t idx_out1; // the glue output 1 of the last tile sets
    char idxbuf[200];
    std::ostringstream buff;
    std::string label;
    std::string name;
    std::string glue_in;
    std::string glue_out;
    std::string glue_0;

    assert ((NULL == glue_output_0) || ((void *)0x05 < glue_output_0));
    assert ((NULL == glue_output_1) || ((void *)0x05 < glue_output_1));

    if ((NULL == glue_output_0) && (NULL == glue_output_1)) {
        return -1;
    }
    idx_glue = 0;
    idx_tile = 0;
    label = "";

    idx_out0 = 0;
    idx_out1 = 0;
    for (i = 0; i < k; i ++) {
        if (idx_out0 > 0) {
            label = "-0-";
            if (idx_out0 > 0) {
                sprintf (idxbuf, "%02X", idx_out0);
                glue_in = std::string (tilename) + "G_" + idxbuf;
                sprintf (idxbuf, "%02X", idx_tile);
                name = std::string (tilename) + "T_" + idxbuf;
            } else {
                glue_in = glue_input;
                name = tilename;
            }
            sprintf (idxbuf, "%02X", idx_glue + 1);
            glue_out = std::string (tilename) + "G_" + idxbuf;
            cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DEC,
                    /*N*/NULL,
                    /*E*/glue_in.c_str(),
                    /*S*/NULL,
                    /*W*/glue_out.c_str (),
                    /*F*/NULL,
                    /*B*/NULL);
            idx_glue ++; idx_tile ++;
            sprintf (idxbuf, "%02X", idx_tile);
            name = std::string (tilename) + "T_" + idxbuf;
            sprintf (idxbuf, "%02X", idx_glue);
            glue_in = std::string (tilename) + "G_" + idxbuf;
            idx_out0 = idx_glue + 1;
            if (i + 1 >= k) {
                if (NULL == glue_output_0) {
                    glue_out = std::string (tilename) + "G_ERR";
                } else {
                    glue_out = glue_output_0;
                }
            } else {
                sprintf (idxbuf, "%02X", idx_glue + 1);
                glue_out = std::string (tilename) + "G_" + idxbuf;
            }
            cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DECI,
                    /*N*/NULL,
                    /*E*/glue_in.c_str(),
                    /*S*/NULL,
                    /*W*/((0 == glue_out.size())?NULL:glue_out.c_str ()),
                    /*F*/NULL,
                    /*B*/NULL);
            idx_glue ++; idx_tile ++;
        }
        // construct the 'T' tile
        label = "T";
        sprintf (idxbuf, "%02X", idx_tile);
        name = std::string (tilename) + "T_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue + 1);
        glue_out = std::string (tilename) + "G_" + idxbuf;
        if (idx_out1 > 0) {
            sprintf (idxbuf, "%02X", idx_out1);
            glue_in = std::string (tilename) + "G_" + idxbuf;
        } else {
            glue_in = glue_input;
            name = tilename;
        }
        sprintf (idxbuf, "%02X", idx_glue + 2);
        glue_0 = std::string (tilename) + "G_" + idxbuf;
        cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DECI,
                /*N*/NULL,
                /*E*/((0 == i)?glue_input:glue_in.c_str ()),
                /*S*/glue_0.c_str(), // '0'
                /*W*/glue_out.c_str (), // '1'
                /*F*/NULL,
                /*B*/NULL);
        idx_glue ++; idx_tile ++;
        // construct the '1' tile
        label = "1?";
        sprintf (idxbuf, "%02X", idx_tile);
        name = std::string (tilename) + "T_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue);
        glue_in = std::string (tilename) + "G_" + idxbuf;
        idx_out1 = idx_glue + 1;
        assert (idx_out1 > 0);
        if (i + 1 >= k) {
            if (NULL == glue_output_1) {
                glue_out = std::string (tilename) + "G_ERR";
            } else {
                glue_out = glue_output_1;
            }
        } else {
            sprintf (idxbuf, "%02X", idx_out1);
            glue_out = std::string (tilename) + "G_" + idxbuf;
        }
        cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DEC,
                /*N*/NULL,
                /*E*/glue_in.c_str(),
                /*S*/NULL,
                /*W*/glue_out.c_str (),
                /*F*/NULL,
                /*B*/NULL);
        idx_glue ++; idx_tile ++;
#if DEBUG
{
size_t m;
size_t idx_glue_reset = idx_glue;
for (m = 0; m < NUM_DUP_2D1T; m ++) {
idx_glue = idx_glue_reset;
#else

#endif
        // construct the '0' tiles
        label = "0";
        sprintf (idxbuf, "%02X", idx_tile);
        name = std::string (tilename) + "T_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue);
        glue_in = std::string (tilename) + "G_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue + 1);
        glue_out = std::string (tilename) + "G_" + idxbuf;
        cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DEC,
                /*N*/glue_in.c_str(),
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_out.c_str (),
                /*F*/NULL,
                /*B*/NULL);
        idx_glue ++; idx_tile ++;
        sprintf (idxbuf, "%02X", idx_tile);
        name = std::string (tilename) + "T_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue);
        glue_in = std::string (tilename) + "G_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue + 1);
        glue_out = std::string (tilename) + "G_" + idxbuf;
        cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DECI,
                /*N*/glue_out.c_str (),
                /*E*/glue_in.c_str(),
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx_glue ++; idx_tile ++;
        sprintf (idxbuf, "%02X", idx_tile);
        name = std::string (tilename) + "T_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue);
        glue_in = std::string (tilename) + "G_" + idxbuf;
        sprintf (idxbuf, "%02X", idx_glue + 1);
        if (i + 1 >= k) {
            if (NULL == glue_output_0) {
                glue_out = std::string (tilename) + "G_ERR";
            } else {
                glue_out = glue_output_0;
            }
        } else {
            if (idx_out0 < 1) {
                idx_out0 = idx_glue + 1;
            } else {
                sprintf (idxbuf, "%02X", idx_out0);
            }
            glue_out = std::string (tilename) + "G_" + idxbuf;
        }
        cb_output (fpout, name.c_str(), label.c_str(), ZZCOUV_COLOR_DECI,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in.c_str(),
                /*W*/glue_out.c_str (),
                /*F*/NULL,
                /*B*/NULL);
        idx_glue ++; idx_tile ++;
#if DEBUG
} // m
}
#endif

    }
    return 0;
}

int
gen_dectile_1bit_2west (FILE *fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k)
{
    assert ((NULL == glue_output_0) || ((void *)0x05 < glue_output_0));
    assert ((NULL == glue_output_1) || ((void *)0x05 < glue_output_1));

    if (k > 0) {
        // is zig-zag 2D temperature 1
        return gen_2d1t_dectile_1bit_2left (fpout, tilename, glue_input, glue_output_0, glue_output_1, k, zzconv_output_tds);
    }
    return gen_3d1t_dectile_1bit_2left (fpout, tilename, glue_input, glue_output_0, glue_output_1, zzconv_output_tds);
}

int
gen_dectile_1bit_2east (FILE *fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k)
{
    assert ((NULL == glue_output_0) || ((void *)0x05 < glue_output_0));
    assert ((NULL == glue_output_1) || ((void *)0x05 < glue_output_1));

    if (k > 0) {
        // is zig-zag 2D temperature 1
        return gen_2d1t_dectile_1bit_2left (fpout, tilename, glue_input, glue_output_0, glue_output_1, k, zzconv_output_tds_ewmirror);
    }
    return gen_3d1t_dectile_1bit_2left (fpout, tilename, glue_input, glue_output_0, glue_output_1, zzconv_output_tds_ewmirror);
}
