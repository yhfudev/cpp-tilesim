/******************************************************************************
 * Name:        gentypeenc.c
 * Purpose:     generate the tiles of type DWW1, DWW2, TEW1, TEW2, TWW2, DEE1, DEE2, TWE1, TWE2, TEE2
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

#define INTYPE_C1 0x01
#define INTYPE_C2 0x02
#define INTYPE_T2 0x03

#define OUTTYPE_C1 0x01
#define OUTTYPE_C2 0x02

int m_caltype_io[][2] = {
    {0,0},
    /*GENCAL_TYPE_DXX1*/{INTYPE_C1, OUTTYPE_C1},
    /*GENCAL_TYPE_DXX2*/{INTYPE_C1, OUTTYPE_C2},
    /*GENCAL_TYPE_TYX1*/{INTYPE_C2, OUTTYPE_C1},
    /*GENCAL_TYPE_TYX2*/{INTYPE_C2, OUTTYPE_C2},
    /*GENCAL_TYPE_TXX2*/{INTYPE_T2, OUTTYPE_C2},
};

// encode_val_array: the encoding array, network sequence, iaw: MSB is in the first byte
int
gen_3d1t_enctile_type_2left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, zzconv_cb_output_t cb_output)
{
    size_t i;
    char label[50] = "";
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
    assert (type < NUM_TYPE (m_caltype_io, int[2]));

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
    if (NULL == glue_output) {
        glue_output = "";
    }
    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (m_caltype_io[type][0]) {
    case INTYPE_C1:
    case INTYPE_T2:
        if (INTYPE_C1 == m_caltype_io[type][0]) {
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, tilename, label, (group != 0)?color:(group != 0)?color:ZZCOUV_COLOR_IN,
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
        } else {
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_out,
                    /*S*/glue_input,
                    /*W*/NULL,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        i = (INTYPE_C1 == m_caltype_io[type][0]) ? encode_size * 2 - 1: encode_size * 2 - 2;
        for (; i > 0; i --) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_out,
                    /*S*/NULL,
                    /*W*/glue_in,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }

        // the input pipe (at the corner)
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

        break;
    case INTYPE_C2:
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    }

    // the first bit: LSB
    // LSB
    assert (encode_size > 0);
    //val_cur = encode_val & 0x01;
    val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1);
    if (encode_size < 2) {
        val_next = 0;
    } else {
        //val_next = (encode_val >> 1) & 0x01;
        val_next = NBO_GET_BIT (encode_val_array, encode_size - 2);
    }
    if (val_cur == 0x01) {
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
    }
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
    // second part of the first bit
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

    // the bits in calculation structure
    //val_pre = encode_val & 0x01;
    val_pre = NBO_GET_BIT (encode_val_array, encode_size - 1);
    assert (val_pre < 2);
    for (i = 1; i < encode_size; i ++) {
        if (i + 1 >= encode_size) {
            val_next = 0;
        } else {
            //val_next = (encode_val >> (i + 1)) & 0x01;
            val_next = NBO_GET_BIT (encode_val_array, encode_size - 1 - i - 1);
        }
        //val_cur = (encode_val >> i) & 0x01;
        val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1 - i);

        assert ((0 <= val_pre) && (val_pre < 2));
        assert ((0 <= val_cur) && (val_cur < 2));
        assert ((0 <= val_next) && (val_next < 2));

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
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
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
        assert ((0 <= val_pre) && (val_pre < 2));
        assert ((0 <= val_cur) && (val_cur < 2));
        assert ((0 <= val_next) && (val_next < 2));
    }

    assert ((0 <= val_pre) && (val_pre < 2));
    assert ((0 <= val_cur) && (val_cur < 2));
    assert ((0 <= val_next) && (val_next < 2));
    // 05. the tiles in the output pipe
    // MSB
    //assert (((encode_val >> (encode_size - 1)) & 0x01) == val_pre);
    assert (NBO_GET_BIT (encode_val_array, 0) == val_pre);
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
    }
    switch (m_caltype_io[type][1]) {
    case OUTTYPE_C1:
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        break;
    case OUTTYPE_C2:
        sprintf (glue_out, "%s", glue_output);
        break;
    }
    //assert (((encode_val >> (encode_size - 1)) & 0x01) == val_pre);
    assert (NBO_GET_BIT (encode_val_array, 0) == val_pre);
    if (val_pre == 1) {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        //sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ((OUTTYPE_C2 == m_caltype_io[type][1])?(group != 0)?color:ZZCOUV_COLOR_OUT:(group != 0)?color:ZZCOUV_COLOR_ENCI),
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
        //sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ((OUTTYPE_C2 == m_caltype_io[type][1])?(group != 0)?color:ZZCOUV_COLOR_OUT:(group != 0)?color:ZZCOUV_COLOR_ENCI),
                /*N*/NULL,
                /*E*/NULL,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/glue_in,
                /*B*/NULL);
        idx ++;
    }

    switch (m_caltype_io[type][1]) {
    case OUTTYPE_C1:
        // 04. the tiles in the output pipe
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

        // 02~03. the tiles in the output pipe
        for (i = 0; i < 2; i ++) {
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
        }

        // 01. the tiles in the output pipe
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

        // 00. the output tile
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
    }

    return 0;
}

int
gen_2d1t_enctile_type_2left (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t encode_size, size_t k, zzconv_cb_output_t cb_output)
{
    size_t i;
    size_t j;
    char label[50] = "";
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

    assert (type < NUM_TYPE (m_caltype_io, int[2]));

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
    if (NULL == glue_output) {
        glue_output = "";
    }
    sprintf (label, "");
    zzconv_output_buf_rgb (color, group);

    switch (m_caltype_io[type][0]) {
    case INTYPE_C1:
    case INTYPE_T2:
        if (INTYPE_C1 == m_caltype_io[type][0]) {
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                    /*N*/glue_out,
                    /*E*/glue_input,
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
        } else {
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                    /*N*/NULL,
                    /*E*/glue_out,
                    /*S*/glue_input,
                    /*W*/NULL,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }
        i = (INTYPE_C1 == m_caltype_io[type][0]) ? encode_size * k * 2 - 1: encode_size * k * 2 - 2;
        for (; i > 0; i --) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, (group != 0)?color:ZZCOUV_COLOR_ENCI,
                    /*N*/NULL,
                    /*E*/glue_out,
                    /*S*/NULL,
                    /*W*/glue_in,
                    /*F*/NULL,
                    /*B*/NULL);
            idx ++;
        }

        // the input pipe (at the corner)
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

        break;
    case INTYPE_C2:
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, tilename, label, (group != 0)?color:ZZCOUV_COLOR_IN,
                /*N*/glue_out,
                /*E*/NULL,
                /*S*/glue_input,
                /*W*/NULL,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
        break;
    }

    // the first bit: LSB
    // LSB
    //val_cur = encode_val & 0x01;
    val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1);
    if (encode_size < 2) {
        val_next = 0;
    } else {
        //val_next = (encode_val >> 1) & 0x01;
        val_next = NBO_GET_BIT (encode_val_array, encode_size - 2);
    }
    if (val_cur == 0x01) {
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
    }
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
    // the middle tiles
    for (i = 0; i < k * 2 - 2; i ++) {
        sprintf (name, "%sT_%02X", tilename, idx);
        sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        cb_output (fpout, name, label, ((0 == (i % 2))?(group != 0)?color:ZZCOUV_COLOR_ENC:(group != 0)?color:ZZCOUV_COLOR_ENCI),
                /*N*/NULL,
                /*E*/glue_in,
                /*S*/NULL,
                /*W*/glue_out,
                /*F*/NULL,
                /*B*/NULL);
        idx ++;
    }
    // second part of the first bit
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

    // the bits in calculation structure
    //val_pre = encode_val & 0x01;
    val_pre = NBO_GET_BIT (encode_val_array, encode_size - 1);
    assert (val_pre < 2);
    for (i = 1; i < encode_size; i ++) {
        if (i + 1 >= encode_size) {
            val_next = 0;
        } else {
            //val_next = (encode_val >> (i + 1)) & 0x01;
            val_next = NBO_GET_BIT (encode_val_array, encode_size - 1 - i - 1);
        }
        //val_cur = (encode_val >> i) & 0x01;
        val_cur = NBO_GET_BIT (encode_val_array, encode_size - 1 - i);

        assert ((0 <= val_pre) && (val_pre < 2));
        assert ((0 <= val_cur) && (val_cur < 2));
        assert ((0 <= val_next) && (val_next < 2));

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
        for (j = k * 2 - 2; j > 0; j --) {
            sprintf (name, "%sT_%02X", tilename, idx);
            sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
            sprintf (glue_out, "%sG_%02X", tilename, idx);
            cb_output (fpout, name, label, ((0 == (j % 2))?(group != 0)?color:ZZCOUV_COLOR_ENC:(group != 0)?color:ZZCOUV_COLOR_ENCI),
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
        assert ((0 <= val_pre) && (val_pre < 2));
        assert ((0 <= val_cur) && (val_cur < 2));
        assert ((0 <= val_next) && (val_next < 2));
    }

    assert ((0 <= val_pre) && (val_pre < 2));
    assert ((0 <= val_cur) && (val_cur < 2));
    assert ((0 <= val_next) && (val_next < 2));
    // 05. the tiles in the output pipe
    switch (m_caltype_io[type][1]) {
    case OUTTYPE_C1:
        sprintf (glue_out, "%sG_%02X", tilename, idx);
        break;
    case OUTTYPE_C2:
        sprintf (glue_out, "%s", glue_output);
        break;
    }
    // MSB
    //assert (((encode_val >> (encode_size - 1)) & 0x01) == val_pre);
    assert (NBO_GET_BIT (encode_val_array, 0) == val_pre);
    sprintf (name, "%sT_%02X", tilename, idx);
    sprintf (glue_in, "%sG_%02X", tilename, idx - 1);
    //sprintf (glue_out, "%sG_%02X", tilename, idx);
    cb_output (fpout, name, label, ((OUTTYPE_C2 == m_caltype_io[type][1])?(group != 0)?color:ZZCOUV_COLOR_OUT:(group != 0)?color:ZZCOUV_COLOR_ENCI),
            /*N*/NULL,
            /*E*/glue_in,
            /*S*/NULL,
            /*W*/glue_out,
            /*F*/NULL,
            /*B*/NULL);
    idx ++;

    switch (m_caltype_io[type][1]) {
    case OUTTYPE_C1:
        // 02. the tiles in the output pipe
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

        // 01. the tiles in the output pipe
        for (i = 0; i < 1; i ++) {
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
        }

        // 00. the output tile
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
    }

    return 0;
}

int
gen_enc_tile_to_west (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t maxbits, size_t k)
{
    if (k > 0) {
        return gen_2d1t_enctile_type_2left (fpout, type, group, tilename, glue_input, glue_output, encode_val_array, maxbits, k, zzconv_output_tds);
    }
    return gen_3d1t_enctile_type_2left (fpout, type, group, tilename, glue_input, glue_output, encode_val_array, maxbits, zzconv_output_tds);
}

int
gen_enc_tile_to_east (FILE *fpout, int type, size_t group, char *tilename, const char *glue_input, const char *glue_output, unsigned char * encode_val_array, size_t maxbits, size_t k)
{
    if (k > 0) {
        return gen_2d1t_enctile_type_2left (fpout, type, group, tilename, glue_input, glue_output, encode_val_array, maxbits, k, zzconv_output_tds_ewmirror);
    }
    return gen_3d1t_enctile_type_2left (fpout, type, group, tilename, glue_input, glue_output, encode_val_array, maxbits, zzconv_output_tds_ewmirror);
}
