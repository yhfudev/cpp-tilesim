/******************************************************************************
 * Name:        gentypeconn.c
 * Purpose:     generate the tiles of type DEN2, DWN2, SWN2, SEN2
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "outtds.h"
#include "zzconv.h"
#include "gentileset.h"

int
gen_3d1t_conn_tile_at_left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    char color[50];
    size_t idx = 0;

    assert (type == GENCONN_TYPE_DXN2 || type == GENCONN_TYPE_SXN2);

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL != glue_input) {
        if (strlen (glue_input) < 1) {
            glue_input = NULL;
        }
    }
    if (NULL != glue_output) {
        if (strlen (glue_output) < 1) {
            glue_output = NULL;
        }
    }

    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (type) {
    case GENCONN_TYPE_DXN2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/glue_out);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/glue_in,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/glue_out,
                /*S*/glue_in,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_in,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_SXN2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * 2 - 1; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    }
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    sprintf (glue_out, "%sG_%02X", tilename, idx);
    cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
            /*N*/glue_out,
            /*E*/NULL,
            /*S*/glue_in,
            /*W*/NULL,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
            /*N*/glue_output,
            /*E*/NULL,
            /*S*/glue_in,
            /*W*/NULL,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;
    return 0;
}

int
gen_3d1t_conn_tile_to_left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    char color[50];
    size_t idx = 0;

    assert (
           (type == GENCONN_TYPE_DXX1)
        || (type == GENCONN_TYPE_DXX2)
        || (type == GENCONN_TYPE_TYX1)
        || (type == GENCONN_TYPE_TYX2)
        || (type == GENCONN_TYPE_TXX2)
        || (type == GENCONN_TYPE_FX)
        );

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL != glue_input) {
        if (strlen (glue_input) < 1) {
            glue_input = NULL;
        }
    }
    if (NULL != glue_output) {
        if (strlen (glue_output) < 1) {
            glue_output = NULL;
        }
    }

    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (type) {
    case GENCONN_TYPE_DXX1:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_DXX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/glue_out);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/glue_in,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/glue_out,
                    /*E*/NULL,
                    /*S*/glue_in,
                    /*W*/NULL,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_TYX1:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/glue_out,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_in,
                /*E*/NULL,
                /*S*/glue_out,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_in,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/glue_out,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/glue_in);
        idx ++;
        break;

    case GENCONN_TYPE_TYX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_TXX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    case GENCONN_TYPE_FX:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    default:
        return -1;
    }
    return 0;
}

int
gen_2d1t_conn_tile_at_left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    char color[50];
    size_t idx = 0;

    assert (type == GENCONN_TYPE_DXN2 || type == GENCONN_TYPE_SXN2);

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL == glue_input) {
        glue_input = "";
    }
    if (NULL == glue_output) {
        glue_output = "";
    }

    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (type) {
    case GENCONN_TYPE_DXN2:
        sprintf (name, "%s", tilename);
        sprintf (glue_in, "%s", glue_input);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/glue_out,
                /*S*/glue_in,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_in,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_SXN2:
        sprintf (name, "%s", tilename);
        sprintf (glue_in, "%s", glue_input);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * k * 2 - 1; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    }
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    sprintf (glue_out, "%sG_%02X", tilename, idx);
    cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
            /*N*/glue_out,
            /*E*/NULL,
            /*S*/glue_in,
            /*W*/NULL,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
            /*N*/glue_output,
            /*E*/NULL,
            /*S*/glue_in,
            /*W*/NULL,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;
    return 0;
}

int
gen_2d1t_conn_tile_to_left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    char color[50];
    size_t idx = 0;

    assert (
           (type == GENCONN_TYPE_DXX1)
        || (type == GENCONN_TYPE_DXX2)
        || (type == GENCONN_TYPE_TYX1)
        || (type == GENCONN_TYPE_TYX2)
        || (type == GENCONN_TYPE_TXX2)
        || (type == GENCONN_TYPE_FX)
        );

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL != glue_input) {
        if (strlen (glue_input) < 1) {
            glue_input = NULL;
        }
    }
    if (NULL != glue_output) {
        if (strlen (glue_output) < 1) {
            glue_output = NULL;
        }
    }

    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (type) {
    case GENCONN_TYPE_DXX1:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_DXX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/glue_out,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < 1; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/glue_out,
                    /*E*/NULL,
                    /*S*/glue_in,
                    /*W*/NULL,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_TYX1:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * k * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/glue_out,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/glue_in,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_TYX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * k * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_TXX2:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/glue_out,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/glue_in,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    case GENCONN_TYPE_FX:
        sprintf (name, "%s", tilename);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        for (i = 0; i < encode_size * k * 2; i ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;

    default:
        return -1;
    }
    return 0;
}

int
gen_conn_tile_at_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_conn_tile_at_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, k, zzconv_output_tds);
    }
    return gen_3d1t_conn_tile_at_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, zzconv_output_tds);
}

int
gen_conn_tile_at_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_conn_tile_at_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, k, zzconv_output_tds_ewmirror);
    }
    return gen_3d1t_conn_tile_at_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, zzconv_output_tds_ewmirror);
}

int
gen_conn_tile_to_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_conn_tile_to_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, k, zzconv_output_tds);
    }
    return gen_3d1t_conn_tile_to_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, zzconv_output_tds);
}

int
gen_conn_tile_to_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_conn_tile_to_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, k, zzconv_output_tds_ewmirror);
    }
    return gen_3d1t_conn_tile_to_left (fpout, type, group, tilename, glue_input, glue_output, encode_size, zzconv_output_tds_ewmirror);
}
