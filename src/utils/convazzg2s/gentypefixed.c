/******************************************************************************
 * Name:        gentypefixed.c
 * Purpose:     generate the tiles of type FW/FE
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
gen_3d1t_fixed_tile_to_left (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    size_t idx = 0;
    char val_pre;
    char val_next;
    char val_cur;
    char color[50];
#if DEBUG
size_t encode_val = INIT_VAL_ENC;
if (NULL == encode_val_array) {
    encode_val_array = (unsigned char *)&encode_val;
} else {
    encode_val = *((size_t *)encode_val_array);
}
#endif

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL != glue_input) {
        if (strlen(glue_input) < 1) {
            glue_input = NULL;
        }
    }
    if (NULL != glue_output) {
        if (strlen(glue_output) < 1) {
            glue_output = NULL;
        }
    }

    idx = 0;
    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    // LSB
    //val_cur = encode_val & 0x01;
    val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1);

    // the input pipe
    if (val_cur == 0) {
        //sprintf (name, "%sT_%02X", tilename, idx);
        //sprintf (glue_in, "%sG_%02X", tilename, idx);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/glue_out,
                /*B*/NULL);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/glue_in);
        idx ++;
    } else {
        //sprintf (name, "%sT_%02X", tilename, idx);
        //sprintf (glue_in, "%sG_%02X", tilename, idx);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/NULL,
                /*E*/glue_input,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
    }
    // the bits in calculation structure
    val_pre = 0;
    for (i = 0; i < encode_size; i ++) {
        if (i + 1 >= encode_size) {
            val_next = 0;
        } else {
            //val_next = (encode_val >> (i + 1)) & 0x01;
            val_next = NBO_GET_BIT (encode_val_array, encode_size - 1 - i - 1);
        }
        //val_cur = (encode_val >> i) & 0x01;
        val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1 - i);
        if (val_cur == val_pre) {
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
        } else {
            if (0 == val_cur) {
                sprintf (name, "%sT_%02X", tilename, idx);
                sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                        /*N*/NULL,
                        /*E*/glue_in,
                        /*S*/NULL,
                        /*W*/NULL,
                        /*F*/glue_out,
                        /*B*/NULL);
                idx ++;
            } else {
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
                        /*E*/NULL,
                        /*S*/glue_in,
                        /*W*/NULL,
                        /*F*/glue_out,
                        /*B*/NULL);
                idx ++;
            }
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/NULL,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/glue_in);
            idx ++;
        }
        // the second part of the bit
        if (val_cur == val_next) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENC,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        } else {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENC,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/NULL,
                    /*F*/NULL,
                    /*B*/glue_out);
            idx ++;
            if (0 == val_cur) {
                sprintf (name, "%sT_%02X", tilename, idx);
                sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                        /*N*/NULL,
                        /*E*/NULL,
                        /*S*/NULL,
                        /*W*/glue_out,
                        /*F*/glue_in,
                        /*B*/NULL);
                idx ++;
            } else {
                sprintf (name, "%sT_%02X", tilename, idx);
                sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                        /*N*/NULL,
                        /*E*/NULL,
                        /*S*/glue_out,
                        /*W*/NULL,
                        /*F*/glue_in,
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
                        /*W*/glue_out,
                        /*F*/NULL,
                        /*B*/NULL);
                idx ++;
            }
        }
        val_pre = val_cur;
    }

    // the tiles in the output pipe
    // MSB
    if (val_pre == 0) {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/glue_out);
        idx ++;
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        //sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/glue_in,
                /*B*/NULL);
        idx ++;
    } else {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        //sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_output,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
    }

    return 0;
}

int
gen_2d1t_fixed_tile_to_left (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k, zzconv_cb_output_t cb_output)
{
    size_t i;
    size_t j;
    char label[50];
    char name[50];
    char glue_in[50];
    char glue_out[50];
    size_t idx = 0;
    char val_pre;
    char val_next;
    char val_cur;
    char color[50];
#if DEBUG
size_t encode_val = INIT_VAL_ENC;
if (NULL == encode_val_array) {
    encode_val_array = (unsigned char *)&encode_val;
} else {
    encode_val = *((size_t *)encode_val_array);
}
#endif

    if (encode_size < 1) {
        return -1;
    }
    if (NULL == tilename) {
        tilename = "";
    }
    if (NULL != glue_input) {
        if (strlen(glue_input) < 1) {
            glue_input = NULL;
        }
    }
    if (NULL != glue_output) {
        if (strlen(glue_output) < 1) {
            glue_output = NULL;
        }
    }

    idx = 0;
    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    //LSB
    //val_cur = encode_val & 0x01;
    val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1);

    // the input pipe
    //sprintf (name, "%sT_%02X", tilename, idx);
    //sprintf (glue_in, "%sG_%02X", tilename, idx);
    sprintf (glue_out, "%sG_%02X", tilename, idx);
    cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
            /*N*/NULL,
            /*E*/glue_input,
            /*S*/NULL,
            /*W*/glue_out,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;

    // the bits in calculation structure
    val_pre = 0;
    for (i = 0; i < encode_size; i ++) {
        if (i + 1 >= encode_size) {
            val_next = 0;
        } else {
            //val_next = (encode_val >> (i + 1)) & 0x01;
            val_next = NBO_GET_BIT (encode_val_array, encode_size - 1 - i - 1);
        }
        //val_cur = (encode_val >> i) & 0x01;
        val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1 - i);
        if (val_cur == val_pre) {
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
        } else {
            if (0 == val_cur) {
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
            } else {
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
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                        /*N*/NULL,
                        /*E*/NULL,
                        /*S*/glue_in,
                        /*W*/glue_out,
                        /*F*/NULL,
                        /*B*/NULL);
                idx ++;
            }
        }
        // the middle tiles
        for (j = 0; j < k * 2 - 2; j ++) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:((0 == (j % 2))?ZZCOUV_COLOR_ENC:ZZCOUV_COLOR_ENCI),
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        // the second part of the bit
        if (val_cur == val_next) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENC,
                    /*N*/NULL,
                    /*E*/glue_in,
                    /*S*/NULL,
                    /*W*/glue_out,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        } else {
            if (0 == val_cur) {
                sprintf (name, "%sT_%02X", tilename, idx);
                sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENC,
                        /*N*/NULL,
                        /*E*/glue_in,
                        /*S*/NULL,
                        /*W*/glue_out,
                        /*F*/NULL,
                        /*B*/NULL);
                idx ++;
            } else {
                sprintf (name, "%sT_%02X", tilename, idx);
                sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
                sprintf (glue_out, "%sG_%02X", tilename, idx);
                cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENC,
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
                        /*S*/NULL,
                        /*W*/glue_out,
                        /*F*/NULL,
                        /*B*/NULL);
                idx ++;
            }
        }
        val_pre = val_cur;
    }

    // the tiles in the output pipe
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    //sprintf (glue_out, "%sG_%02X", tilename, idx);
    cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_OUT,
            /*N*/NULL,
            /*E*/glue_in,
            /*S*/NULL,
            /*W*/glue_output,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;

    return 0;
}

int
gen_fixed_tile_to_west (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_fixed_tile_to_left (fpout, group, tilename, glue_input, glue_output, encode_val_array, encode_size, k, zzconv_output_tds);
    }
    return gen_3d1t_fixed_tile_to_left (fpout, group, tilename, glue_input, glue_output, encode_val_array, encode_size, zzconv_output_tds);
}

int
gen_fixed_tile_to_east (FILE *fpout, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k)
{
    if (k > 0) {
        return gen_2d1t_fixed_tile_to_left (fpout, group, tilename, glue_input, glue_output, encode_val_array, encode_size, k, zzconv_output_tds_ewmirror);
    }
    return gen_3d1t_fixed_tile_to_left (fpout, group, tilename, glue_input, glue_output, encode_val_array, encode_size, zzconv_output_tds_ewmirror);
}
