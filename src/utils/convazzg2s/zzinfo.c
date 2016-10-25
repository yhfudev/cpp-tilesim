/******************************************************************************
 * Name:        zzinfo.c
 * Purpose:     functions of the tile set information, such as load data from file,
 *              convertions between the TM and zig-zag tiles.
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <string.h>
#include <assert.h>

#include "pfdebug.h"
#include "colortab.h"
#include "cstrutils.h"
#include "cubeface.h"
#include "tilestruct.h"

#include "grouptiles.h"
#include "zzinfo.h"

size_t
gluearr_name2val (glue_t *pglue, size_t sz_glues, const char *gluename)
{
    size_t i;
    if (NULL == gluename) {
        return 0;
    }
    for (i = 0; i < sz_glues; i ++) {
        //fprintf (stderr, "chk '%s' with glue '%s'\n", pglue[i].name, gluename);
        if (0 == strcmp (gluename, pglue[i].name)) {
            //fprintf (stderr, "FOUND! '%s' = %d\n",pglue[i].name, pglue[i].strength);
            return pglue[i].strength;
        }
    }
    return 0;
}

// return sz_glues on error
size_t
gluearr_name2idx (glue_t *pglue, size_t sz_glues, const char *gluename)
{
    size_t i;
    if (NULL == gluename) {
        return sz_glues;
    }
    for (i = 0; i < sz_glues; i ++) {
        //fprintf (stderr, "chk '%s' with glue '%s'\n", pglue[i].name, gluename);
        if (0 == strcmp (gluename, pglue[i].name)) {
            //fprintf (stderr, "FOUND! '%s' = %d\n",pglue[i].name, pglue[i].strength);
            return i;
        }
    }
    return sz_glues;
}

const char * m_tiletype_cstr[] = {
    /*TILETYPE_NONE*/"NONE",
    /*TILETYPE_UNKNOWN*/"UNKNOWN",
    /*TILETYPE_DWW1*/"DWW1",
    /*TILETYPE_DWW2*/"DWW2",
    /*TILETYPE_TEW1*/"TEW1",
    /*TILETYPE_TEW2*/"TEW2",
    /*TILETYPE_TWW2*/"TWW2",
    /*TILETYPE_DEE1*/"DEE1",
    /*TILETYPE_DEE2*/"DEE2",
    /*TILETYPE_TWE1*/"TWE1",
    /*TILETYPE_TWE2*/"TWE2",
    /*TILETYPE_TEE2*/"TEE2",
    /*TILETYPE_FE  */"FE",
    /*TILETYPE_FW  */"FW",
    /*TILETYPE_SWN2*/"SWN2",
    /*TILETYPE_SEN2*/"SEN2",
    /*TILETYPE_DWN2*/"DWN2",
    /*TILETYPE_DEN2*/"DEN2",
};

const char *
tiletype_val2cstr (int type)
{
    if (type >= NUM_TYPE(m_tiletype_cstr, const char *)) {
        return NULL;
    }
    return m_tiletype_cstr[type];
}

void
zzi_init (zigzag_info_t *pzzi)
{
    memset (pzzi, 0, sizeof (*pzzi));
}

void
zzi_clear (zigzag_info_t *pzzi)
{
    size_t i;
    for (i = 0; i < pzzi->sz_glues; i ++) {
        if (NULL != pzzi->glues[i].name) {
            free (pzzi->glues[i].name);
        }
    }
    for (i = 0; i < pzzi->sz_tiles; i ++) {
        if (NULL != pzzi->tiles[i].name) {
            free (pzzi->tiles[i].name);
        }
        if (NULL != pzzi->tiles[i].label) {
            free (pzzi->tiles[i].label);
        }
    }
    if (NULL != pzzi->glues) {
        free (pzzi->glues);
    }
    if (NULL != pzzi->tiles) {
        free (pzzi->tiles);
    }
}

int
zzi_copy (zigzag_info_t *pzzi_dest, zigzag_info_t *pzzi_src)
{
    size_t idx;
    size_t i;

    if ((NULL != pzzi_dest->glues) || (NULL != pzzi_dest->tiles)) {
        zzi_clear (pzzi_dest);
        zzi_init (pzzi_dest);
    }

    pzzi_dest->glues = (glue_t *) malloc (pzzi_src->sz_glues * sizeof (glue_t));
    if (NULL == pzzi_dest->glues) {
        return -1;
    }
    memset (pzzi_dest->glues, 0, (pzzi_dest->sz_glues * sizeof (glue_t)));
    pzzi_dest->sz_glues = pzzi_src->sz_glues;

    pzzi_dest->tiles = (tile_t *) malloc (pzzi_src->sz_tiles * sizeof (tile_t));
    if (NULL == pzzi_dest->tiles) {
        zzi_clear (pzzi_dest);
        return -1;
    }
    memset (pzzi_dest->tiles, 0, (pzzi_dest->sz_tiles * sizeof (tile_t)));
    pzzi_dest->sz_tiles = pzzi_src->sz_tiles;

    for (i = 0; i < pzzi_src->sz_glues; i ++) {
        pzzi_dest->glues[i].name = strdup (pzzi_src->glues[i].name);
        pzzi_dest->glues[i].strength = pzzi_src->glues[i].strength;
    }
    for (i = 0; i < pzzi_src->sz_tiles; i ++) {
        idx = gluearr_name2idx (pzzi_dest->glues, pzzi_dest->sz_glues, pzzi_src->tiles[i].gN);
        pzzi_dest->tiles[i].gN = pzzi_dest->glues[idx].name;
        idx = gluearr_name2idx (pzzi_dest->glues, pzzi_dest->sz_glues, pzzi_src->tiles[i].gE);
        pzzi_dest->tiles[i].gE = pzzi_dest->glues[idx].name;
        idx = gluearr_name2idx (pzzi_dest->glues, pzzi_dest->sz_glues, pzzi_src->tiles[i].gS);
        pzzi_dest->tiles[i].gS = pzzi_dest->glues[idx].name;
        idx = gluearr_name2idx (pzzi_dest->glues, pzzi_dest->sz_glues, pzzi_src->tiles[i].gW);
        pzzi_dest->tiles[i].gW = pzzi_dest->glues[idx].name;
        pzzi_dest->tiles[i].type = pzzi_src->tiles[i].type;
        pzzi_dest->tiles[i].name = NULL;
        if (NULL != pzzi_src->tiles[i].name) {
            pzzi_dest->tiles[i].name = strdup (pzzi_src->tiles[i].name);
        }
        pzzi_dest->tiles[i].label = NULL;
        if (NULL != pzzi_src->tiles[i].label) {
            pzzi_dest->tiles[i].label = strdup (pzzi_src->tiles[i].label);
        }
    }
    return 0;
}

int
zzi_comapre (zigzag_info_t *pzzi_old, zigzag_info_t *pzzi_new, char flg_chktype)
{
    size_t i;
    size_t j;

    if (pzzi_old->sz_glues != pzzi_new->sz_glues) {
        fprintf (stderr, "ERR: the size of the glue, old(%d)!=new(%d)\n", pzzi_old->sz_glues, pzzi_new->sz_glues);
        return -1;
    }
    if (pzzi_old->sz_tiles != pzzi_new->sz_tiles) {
        fprintf (stderr, "ERR: the size of the tile, old(%d)!=new(%d)\n", pzzi_old->sz_tiles, pzzi_new->sz_tiles);
        return -1;
    }
    // check the tile body
    for (i = 0; i < pzzi_new->sz_tiles; i ++) {
        if ((NULL == pzzi_new->tiles[i].gN)
            || (NULL == pzzi_new->tiles[i].gE)
            || (NULL == pzzi_new->tiles[i].gS)
            || (NULL == pzzi_new->tiles[i].gW)
            || (NULL == pzzi_new->tiles[i].name)) {
                fprintf (stderr, "ERR: tile[%d] glue NULL!\n", i);
                break;
        }
        for (j = 0; j < pzzi_old->sz_tiles; j ++) {
            if ((0 == strcmp (pzzi_new->tiles[i].gN, pzzi_old->tiles[j].gN))
                && (0 == strcmp (pzzi_new->tiles[i].gE, pzzi_old->tiles[j].gE))
                && (0 == strcmp (pzzi_new->tiles[i].gS, pzzi_old->tiles[j].gS))
                && (0 == strcmp (pzzi_new->tiles[i].gW, pzzi_old->tiles[j].gW)))
            {
                char flg_notequal = 1;
                if (pzzi_new->tiles[i].name != NULL) {
                    if (pzzi_old->tiles[j].name != NULL) {
                        if (0 == strcmp (pzzi_new->tiles[i].name, pzzi_old->tiles[j].name)) {
                            flg_notequal = 0;
                        }
                    }
                } else {
                    if (pzzi_old->tiles[j].name == NULL) {
                        flg_notequal = 0;
                    }
                }
                if (flg_notequal) {
                    continue;
                }
                if (0 == flg_notequal) {
                    break;
                }
            }
        }
        if (j >= pzzi_old->sz_tiles) {
            fprintf (stderr, "ERR: not found tile[%d]\n", i);
            break;
        }
    }
    if (i < pzzi_new->sz_tiles) {
        return -1;
    }
    // check the glue
    for (i = 0; i < pzzi_new->sz_glues; i ++) {
        if (NULL == pzzi_new->glues[i].name) {
            fprintf (stderr, "ERR: glue[%d].name NULL\n", i);
            break;
        }
        for (j = 0; j < pzzi_old->sz_glues; j ++) {
            if (0 == strcmp (pzzi_new->glues[i].name, pzzi_old->glues[j].name)) {
                break;
            }
        }
        if (j >= pzzi_old->sz_glues) {
            fprintf (stderr, "ERR: not found glue[%d].name '%s'\n", i, pzzi_new->glues[i].name);
            break;
        }
        if (pzzi_new->glues[i].strength != pzzi_old->glues[j].strength) {
            fprintf (stderr, "ERR: glue[%d].name '%s' strength(%d) != old's(%d) \n", i,
                pzzi_new->glues[i].name, pzzi_new->glues[i].strength, pzzi_old->glues[j].strength);
            break;
        }
    }
    if (i < pzzi_new->sz_glues) {
        return -1;
    }
    if (flg_chktype) {
        // check the result
        for (i = 0; i < pzzi_new->sz_tiles; i ++) {
            if (pzzi_new->tiles[i].type != pzzi_old->tiles[i].type) {
                fprintf (stderr, "ERR: given tile[%d].name '%s' .type(%d)'%s' != calculated type(%d)'%s'\n", i,
                    pzzi_old->tiles[i].name, pzzi_old->tiles[i].type, tiletype_val2cstr(pzzi_old->tiles[i].type),
                    pzzi_new->tiles[i].type, tiletype_val2cstr(pzzi_new->tiles[i].type));
                return -1;
            }
        }
    }
    return 0;
}

#if DEBUG
void
zzi_output_c (zigzag_info_t *pzzi, char *name, FILE *fpout)
{
    size_t i;
    if (NULL == name) {
        name = "";
    }
    fprintf (fpout, "glue_t g_lst_glues_%s[] = {\n", name);
    for (i = 0; i < pzzi->sz_glues; i ++) {
        fprintf (fpout, "    /*% d*/{\"%s\", %d},\n", i, ((NULL == pzzi->glues[i].name)?"":pzzi->glues[i].name), pzzi->glues[i].strength);
    }
    fprintf (fpout, "};\n");
    fprintf (fpout, "tile_t g_lst_tiles_%s[] = {\n", name);
    for (i = 0; i < pzzi->sz_tiles; i ++) {
        fprintf (fpout, "    /*% d*/{", i);
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].gN)?"":pzzi->tiles[i].gN));
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].gE)?"":pzzi->tiles[i].gE));
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].gS)?"":pzzi->tiles[i].gS));
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].gW)?"":pzzi->tiles[i].gW));
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].name)?"":pzzi->tiles[i].name));
        fprintf (fpout, "\"%s\", ", ((NULL == pzzi->tiles[i].label)?"":pzzi->tiles[i].label));
        fprintf (fpout, "TILETYPE_%s", tiletype_val2cstr (pzzi->tiles[i].type));
        fprintf (fpout, "},\n");
    }
    fprintf (fpout, "};\n");
}
#endif

static char * m_color_tiletype[] = {
    /*TILETYPE_NONE*/"white",
    /*TILETYPE_UNKNOWN*/"yellow",
    /*TILETYPE_DWW1*/"rgb( 94, 255,   0)",
    /*TILETYPE_DWW2*/"rgb(157, 223, 100)",
    /*TILETYPE_TEW1*/"rgb(160,  32, 240)",
    /*TILETYPE_TEW2*/"rgb(246, 114, 179)",
    /*TILETYPE_TWW2*/"rgb(162, 124, 234)",
    /*TILETYPE_DEE1*/"rgb(204, 255, 139)",
    /*TILETYPE_DEE2*/"rgb(157, 223, 100)",
    /*TILETYPE_TWE1*/"rgb(160,  32, 240)",
    /*TILETYPE_TWE2*/"rgb(246, 114, 179)",
    /*TILETYPE_TEE2*/"rgb(162, 124, 234)",
    /*TILETYPE_FE  */"rgb(139, 139, 139)",
    /*TILETYPE_FW  */"rgb(180, 180, 180)",
    /*TILETYPE_SWN2*/"rgb(236, 188, 212)",
    /*TILETYPE_SEN2*/"rgb(236, 188, 212)",
    /*TILETYPE_DWN2*/"rgb(139, 191, 255)",
    /*TILETYPE_DEN2*/"rgb(139, 191, 255)",
};

void
zzi_output_tds_tilecolor (FILE *fpout, size_t group, char type)
{
    if (group == 0) {
        if (m_color_tiletype[type % TILETYPE_MAX]) {
            fprintf (fpout, "TILECOLOR %s\n", m_color_tiletype[type % TILETYPE_MAX]);
        }
    } else {
        fprintf (fpout, "TILECOLOR rgb(%d,%d,%d)\n", ((group >> 16) & 0xFF), ((group >> 8) & 0xFF), (group & 0xFF));
    }
}

void
zzi_output_buf_tilecolor (char *buffer, size_t group, char type)
{
    if (group == 0) {
        if (m_color_tiletype[type % TILETYPE_MAX]) {
            sprintf (buffer, "%s", m_color_tiletype[type % TILETYPE_MAX]);
        }
    } else {
        sprintf (buffer, "rgb(%d,%d,%d)", ((group >> 16) & 0xFF), ((group >> 8) & 0xFF), (group & 0xFF));
    }
}

// output the tile set directly to one .tds file
void
zzi_output_tds_2d2t (zigzag_info_t *pzzi, FILE *fpout)
{
    size_t i;
    for (i = 0; i < pzzi->sz_tiles; i ++) {
        fprintf (fpout, "TILENAME %s\n", ((NULL == pzzi->tiles[i].name)?"":pzzi->tiles[i].name));
        fprintf (fpout, "LABEL %s\n",    ((NULL == pzzi->tiles[i].label)?"":pzzi->tiles[i].label));
        fprintf (fpout, "NORTHBIND %d\n", gluearr_name2val (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gN));
        fprintf (fpout, "EASTBIND %d\n",  gluearr_name2val (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gE));
        fprintf (fpout, "SOUTHBIND %d\n", gluearr_name2val (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gS));
        fprintf (fpout, "WESTBIND %d\n",  gluearr_name2val (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gW));
        fprintf (fpout, "NORTHLABEL %s\n", ((NULL == pzzi->tiles[i].gN)?"":pzzi->tiles[i].gN));
        fprintf (fpout, "EASTLABEL %s\n",  ((NULL == pzzi->tiles[i].gE)?"":pzzi->tiles[i].gE));
        fprintf (fpout, "SOUTHLABEL %s\n", ((NULL == pzzi->tiles[i].gS)?"":pzzi->tiles[i].gS));
        fprintf (fpout, "WESTLABEL %s\n",  ((NULL == pzzi->tiles[i].gW)?"":pzzi->tiles[i].gW));
        zzi_output_tds_tilecolor (fpout, pzzi->tiles[i].group, pzzi->tiles[i].type);
        fprintf (fpout, "CREATE\n\n");
    }
}

size_t
gluearr_name2idx_stripnull (glue_t *pglue, size_t sz_glues, const char *gluename)
{
    if (gluearr_name2val (pglue, sz_glues, gluename) == 0) {
        return 0;
    }
    return gluearr_name2idx (pglue, sz_glues, gluename) + 1;
}

void
zzi_output_xgrow_tiles (zigzag_info_t *pzzi, FILE *fpout)
{
    size_t i;
    size_t idx_seed = 0;
    size_t block = 1;

    fprintf (fpout, "%% Generated by convazzg2s automatically, author: Yunhui Fu\n");
    fprintf (fpout, "tile edges matches {{N E S W}*}\n");
    fprintf (fpout, "num tile types=%d\n", pzzi->sz_tiles);
    fprintf (fpout, "num binding types=%d\n", pzzi->sz_glues);

    fprintf (fpout, "tile edges={\n");
    for (i = 0; i < pzzi->sz_tiles; i ++) {
        if (idx_seed == 0) {
            if (0 == strcmp ("SEED", pzzi->tiles[i].name)) {
                idx_seed = i;
            }
        }
        fprintf (fpout, "{%d %d %d %d}\n"
            , gluearr_name2idx_stripnull (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gN)
            , gluearr_name2idx_stripnull (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gE)
            , gluearr_name2idx_stripnull (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gS)
            , gluearr_name2idx_stripnull (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gW)
            );
    }
    fprintf (fpout, "}\n");
    fflush (fpout);

    fprintf (fpout, "binding strengths={");
    for (i = 0; i < pzzi->sz_glues; i ++) {
        fprintf (fpout, "%d ", pzzi->glues[i].strength);
    }
    fprintf (fpout, "}\n");
    fflush (fpout);

    i = 1;
    for (; i < pzzi->pos_max_x + 2; i <<= 1);
    for (; i < pzzi->pos_max_y + 2; i <<= 1);
    if (i < 16) i = 16;
    if (i < 1024) {
        block = 1024 / i;
        if (block > 10) {
            block = 10;
        }
    }
    if (block > 10) {
        block = 10;
    }
    if (block < 1) {
        block = 1;
    }
    //printf ("block = %d; size = i", block, i);
    fprintf (fpout, "block=%d\n", block);
    fprintf (fpout, "size=%d\n", i);
    fprintf (fpout, "seed=%d,%d,%d\n", (i - 1) * 2 / 3, (i + pzzi->pos_seed_x) / 2, idx_seed + 1);
    fprintf (fpout, "%%untiltiles=%d\n", pzzi->idx_last + 1);
    fprintf (fpout, "%%smax=%d %% tiles_filled=%d\n", pzzi->tiles_filled * 2, pzzi->tiles_filled);
    fprintf (fpout, "%%update_rate=1000000\n");
    //fprintf (fpout, "%%Gmc=15\n%%Gse=7.8\n");
    fprintf (fpout, "%%Gmc=15\n%%Gse=10\n");
    fflush (fpout);
    //printf ("[zzi_output_xgrow_tiles()] pos_max_x=%d; pos_max_y=%d; pos_seed_x=%d; idx_last=%d\n", pzzi->pos_max_x, pzzi->pos_max_y, pzzi->pos_seed_x, pzzi->idx_last);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

static int
memarr_cb_destroy_cstr (void * data)
{
    char *str;
    str = *((char **)data);
    free (str);
    return 0;
}

static int
slist_cb_comp_heter_gluelabel (void *userdata, void * data_pin, void * data2)
{
    memarr_t *pma_str = (memarr_t *)userdata;
    size_t idx2 = *((size_t *)data2);
    char *str2;
    ma_data (pma_str, idx2, &str2);
    return strcmp ((char *)data_pin, str2);
}

// return data1 - data2
static int
slist_cb_comp_glue (void *userdata, void * data1, void * data2)
{
    size_t idx1 = *((size_t *)data1);
    size_t idx2 = *((size_t *)data2);
    memarr_t *pma_str = (memarr_t *)userdata;
    char *str1;
    char *str2;
    ma_data (pma_str, idx1, &str1);
    ma_data (pma_str, idx2, &str2);
    return strcmp (str1, str2);
}

// search one of the label, if not found, then add it to the list
// return the index of the label in list
static ssize_t
label_search_or_add (memarr_t *pma_str, sortedlist_t *psl_glue, const char *label)
{
    char *str;
    size_t idx;
    if (slist_find (psl_glue, (void *)label, slist_cb_comp_heter_gluelabel, &idx) < 0) {
        // not found
        str = strdup (label);
        assert (NULL != str);
        idx = ma_size (pma_str);
        ma_append (pma_str, &str);
        slist_store (psl_glue, &idx);
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "add label: '%s' to idx=%d", str, idx);
#if DEBUG
{
    char *tmp = NULL;
    assert (NULL != str);
    ma_data (pma_str, idx, &tmp);
    assert (tmp == str);
    assert (0 == strcmp (str, label));
    assert (ma_size (pma_str) == slist_size (psl_glue));
}
        //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "store glue '%s'", label);
#endif
    } else {
        // get the real idx
        slist_data (psl_glue, idx, &idx);
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "get label: '%s' at idx=%d", label, idx);
    }
    return idx;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// pzzi: the structure will be filled by the zzf_xxx functions
void
zzf_init (zigzag_form_t *pzzf, zigzag_info_t *pzzi)
{
    memset (pzzf, 0, sizeof (*pzzf));
    ma_init (&(pzzf->ma_val), sizeof (size_t));
    ma_init (&(pzzf->ma_cstr), sizeof (char *));
    slist_init (&(pzzf->sl_label), sizeof (size_t), &(pzzf->ma_cstr)/*userdata*/, slist_cb_comp_glue, slist_cb_swap_sizet);
    pzzf->pzzi = pzzi;
}

void
zzf_clear (zigzag_form_t *pzzf)
{
    //ma_clear (&ma_str, memarr_cb_destroy_cstr);
    ma_clear (&(pzzf->ma_cstr), NULL);
    ma_clear (&(pzzf->ma_val), NULL);
    slist_clear (&(pzzf->sl_label), NULL);
}

// label is strdup()ed, and transfer to pzzi, the caller will not free it.
ssize_t
zzf_feed_glue (zigzag_form_t *pzzf, const char *label, size_t strength)
{
    ssize_t idx_add;
    size_t tmp;
    idx_add = label_search_or_add (&(pzzf->ma_cstr), &(pzzf->sl_label), label);
    if (idx_add < 0) {
        return -1;
    }
    if (idx_add < ma_size (&(pzzf->ma_val))) {
        // check the value of the glue
        ma_data (&(pzzf->ma_val), idx_add, &tmp);
        if (tmp != strength) {
            // we get the same label and different glue value!
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Non-constant glue value! '%s': stored(%d), newadd(%d)", label, tmp, strength);
            return -1;
        }
        return idx_add;
    }
    ma_replace (&(pzzf->ma_val), idx_add, &strength);
    return idx_add;
}

// TODO: duplicated tile detection: the same name, the same sides, or both
// all of the glues of each sides should be feed to glue first!
// ptile->gN, ptile->gE, ptile->gS, ptile->gW are all strings which are already stored in pzzi
// ptile->name and ptile->label are referenced to a buffer, so the pzzi should strdup() it, and free it.
int
zzf_feed_tile (zigzag_form_t *pzzf, const tile_t *ptile)
{
    size_t idx_add;
    tile_t tile;
    assert (NULL != pzzf);
    assert (NULL != pzzf->pzzi);

    if (pzzf->sz_buf_tilevec < pzzf->pzzi->sz_tiles + 1) {
        void *newbuf;
        newbuf = realloc (pzzf->pzzi->tiles, (pzzf->pzzi->sz_tiles + 50) * sizeof (tile_t));
        pzzf->pzzi->tiles = (tile_t *)newbuf;
        pzzf->sz_buf_tilevec = pzzf->pzzi->sz_tiles + 50;
    }
    memset (&tile, 0, sizeof (tile));

    if (NULL != ptile->gN) {
        if (slist_find (&(pzzf->sl_label), ptile->gN, slist_cb_comp_heter_gluelabel, &idx_add) < 0) {
            return -1;
        }
        slist_data (&(pzzf->sl_label), idx_add, &idx_add);
        ma_data (&(pzzf->ma_cstr), idx_add, &(tile.gN));
        assert (0 == strcmp (ptile->gN, tile.gN));
    }

    if (NULL != ptile->gE) {
        if (slist_find (&(pzzf->sl_label), ptile->gE, slist_cb_comp_heter_gluelabel, &idx_add) < 0) {
            return -1;
        }
        slist_data (&(pzzf->sl_label), idx_add, &idx_add);
        ma_data (&(pzzf->ma_cstr), idx_add, &(tile.gE));
        assert (0 == strcmp (ptile->gE, tile.gE));
    }

    if (NULL != ptile->gS) {
        if (slist_find (&(pzzf->sl_label), ptile->gS, slist_cb_comp_heter_gluelabel, &idx_add) < 0) {
            return -1;
        }
        slist_data (&(pzzf->sl_label), idx_add, &idx_add);
        ma_data (&(pzzf->ma_cstr), idx_add, &(tile.gS));
        assert (0 == strcmp (ptile->gS, tile.gS));
    }

    if (NULL != ptile->gW) {
        if (slist_find (&(pzzf->sl_label), ptile->gW, slist_cb_comp_heter_gluelabel, &idx_add) < 0) {
            return -1;
        }
        slist_data (&(pzzf->sl_label), idx_add, &idx_add);
        ma_data (&(pzzf->ma_cstr), idx_add, &(tile.gW));
        assert (0 == strcmp (ptile->gW, tile.gW));
    }
    tile.name = NULL;
    if (NULL != ptile->name) {
        tile.name  = strdup (ptile->name);
    }
    tile.label = NULL;
    if (NULL != ptile->label) {
        tile.label = strdup (ptile->label);
    }
    tile.type = ptile->type;
    memmove (&(pzzf->pzzi->tiles[pzzf->pzzi->sz_tiles]), &tile, sizeof(tile));
    pzzf->pzzi->sz_tiles ++;
    return 0;
}

ssize_t
zzf_get_glue_idx (zigzag_form_t *pzzf, const char *gluename)
{
    size_t idx;
    if (slist_find (&(pzzf->sl_label), (void *)gluename, slist_cb_comp_heter_gluelabel, &idx) < 0) {
        return -1;
    }
    slist_data (&(pzzf->sl_label), idx, &idx);
    return idx;
}

int
zzf_feed_tile_glueidx (zigzag_form_t *pzzf, size_t ign, size_t ige, size_t igs, size_t igw, const char *name, const char *label, size_t group, size_t type)
{
    size_t idx_add;
    tile_t tile;
    assert (NULL != pzzf);
    assert (NULL != pzzf->pzzi);

    if (pzzf->sz_buf_tilevec < pzzf->pzzi->sz_tiles + 1) {
        void *newbuf;
        newbuf = realloc (pzzf->pzzi->tiles, (pzzf->pzzi->sz_tiles + 50) * sizeof (tile_t));
        pzzf->pzzi->tiles = (tile_t *)newbuf;
        pzzf->sz_buf_tilevec = pzzf->pzzi->sz_tiles + 50;
    }
    memset (&tile, 0, sizeof (tile));

    if (ma_data (&(pzzf->ma_cstr), ign, &(tile.gN)) < 0) {
        return -1;
    }

    if (ma_data (&(pzzf->ma_cstr), ige, &(tile.gE)) < 0) {
        return -1;
    }

    if (ma_data (&(pzzf->ma_cstr), igs, &(tile.gS)) < 0) {
        return -1;
    }

    if (ma_data (&(pzzf->ma_cstr), igw, &(tile.gW)) < 0) {
        return -1;
    }

    tile.name = NULL;
    if (NULL != name) {
        tile.name  = strdup (name);
    }
    tile.label = NULL;
    if (NULL != label) {
        tile.label = strdup (label);
    }
    tile.type = type;
    memmove (&(pzzf->pzzi->tiles[pzzf->pzzi->sz_tiles]), &tile, sizeof(tile));
    pzzf->pzzi->sz_tiles ++;
    return 0;
}

void
zzf_feed_end (zigzag_form_t *pzzf)
{
    size_t i;
    // save all of the glue information
    assert (ma_size (&(pzzf->ma_cstr)) == ma_size (&(pzzf->ma_val)));
    pzzf->pzzi->sz_glues = ma_size (&(pzzf->ma_val));
    pzzf->pzzi->glues = (glue_t *) malloc (pzzf->pzzi->sz_glues * sizeof (glue_t));
    for (i = 0; i < pzzf->pzzi->sz_glues; i ++) {
        ma_data (&(pzzf->ma_val), i, &(pzzf->pzzi->glues[i].strength));
        assert (pzzf->pzzi->glues[i].strength < 3);
        ma_data (&(pzzf->ma_cstr), i, &(pzzf->pzzi->glues[i].name));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

// load the data from the TAS data file
int
zzi_load_tds (const char *fn_tas, zigzag_info_t *pzzi)
{
    size_t num_line = 0;
    char str[200];
    FILE *fp_tas = NULL;
    int ret = 0;
    tile_t tile;
    size_t tmp;
    char flg_is2d = 1;
    char * tilename = NULL;
    char * tilelabel = NULL;
    size_t tilegroup = 0;
    char * label = NULL;
    memarr_t ma_str; // the C string list, the glue name
    memarr_t ma_val; // the glue value
    sortedlist_t sl_glue; // the sorted glue list by glue name, contains only the glue string index
    char flg_glue_val[ORI_MAX];
    size_t glue_val[ORI_MAX];
    size_t glue_labelidx[ORI_MAX]; // the label index in the ma_str
    size_t stridx_cur = 0; // the current max index of the glue, before processing the current record.
    ssize_t idx_add;
    size_t sz_buf_tilevec = 0;
    size_t i;
#if DEBUG
    char *glue_label[ORI_MAX]; // the reference of the label
#define DUP_GLUE_LABEL(dir, val) glue_label[dir]=strdup(val)
    memset (glue_label, 0, sizeof (glue_label));
#else
#define DUP_GLUE_LABEL(dir, val)
#endif

    assert (NULL != pzzi);
    assert (NULL == pzzi->tiles);
    assert (NULL == pzzi->glues);

    fp_tas = fopen (fn_tas, "r");
    if (NULL == fp_tas) {
        return -1;
    }

    ma_init (&ma_str, sizeof (char *));
    ma_init (&ma_val, sizeof (size_t));
    slist_init (&sl_glue, sizeof (size_t), &ma_str/*userdata*/, slist_cb_comp_glue, slist_cb_swap_sizet);

    stridx_cur = ma_size (&ma_str);
    memset (glue_val, 0, sizeof (glue_val));
    memset (glue_labelidx, 0, sizeof (glue_labelidx));
    memset (flg_glue_val, 0, sizeof (flg_glue_val));
    memset (str, 0, sizeof (str));

    // set the default label ""
    label = strdup ("");
    idx_add = label_search_or_add (&ma_str, &sl_glue, label);
    i = 0;
    ma_replace (&ma_val, idx_add, &i);
    for (i = 0; i < ORI_MAX; i ++) {
        glue_labelidx[i] = idx_add;
    }

    for (; fgets (str, sizeof (str), fp_tas) != NULL; ) {
        char *brks;
        char *word;
        num_line ++;
        // strip trailing '\n' if it exists
        cstr_trim (str);
        cstr_stripblank (str);
        if (strlen (str) < 1) {
            continue;
        }
        word = strtok_r (str, " \n", &brks);
        if (NULL == word) {
            continue;
        }
        //printf("\n%s", str);
        if (0 == strcmp (word, "CREATE")) {
            // save the tile
            // check the glue
            //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "CREATE");
            for (i = 0; i < ORI_MAX; i ++) {
                if (0 == flg_glue_val[i]) {
                    continue;
                }
                if ((size_t)-1 == glue_val[i]) {
                    glue_val[i] = 0;
                }
                if (glue_labelidx[i] >= stridx_cur) {
                    ma_replace (&ma_val, glue_labelidx[i], &(glue_val[i]));
                    ma_data (&ma_str, glue_labelidx[i], &label);
                    //DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "store glue '%s'=%d @ idx=%d; stridx_cur=%d", label, glue_val[i], glue_labelidx[i], stridx_cur);
                } else
#if 1 // DEBUG
                if (glue_val[i] > 0)
#endif
                {
                    // check the value of the glue
                    ma_data (&ma_val, glue_labelidx[i], &tmp);
                    if (tmp != glue_val[i]) {
                        // we get the same label and different glue value!
                        char *cstrtmp;
                        ma_data (&ma_str, glue_labelidx[i], &cstrtmp);
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Non-constant glue value! '%s': stored(%d), newadd(%d)", cstrtmp, tmp, glue_val[i]);
#if DEBUG
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Non-constant glue value! current label: '%s'", glue_label[i]);
#endif
                        break;
                    }
                }
#if DEBUG
                if (glue_label[i]) {
                    free (glue_label[i]);
                }
#endif
            }
            if (i < ORI_MAX) {
                // some error in checking the values
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_WARNING, "Some errors in the checking of glue values.");
                ret = -1;
                break;
            }
            assert (ma_size (&ma_str) == ma_size (&ma_val));
            // save to tilevec
            memset (&tile, 0, sizeof (tile));
            ma_data (&ma_str, glue_labelidx[ORI_NORTH], &(tile.gN));
            ma_data (&ma_str, glue_labelidx[ORI_EAST],  &(tile.gE));
            ma_data (&ma_str, glue_labelidx[ORI_SOUTH], &(tile.gS));
            ma_data (&ma_str, glue_labelidx[ORI_WEST],  &(tile.gW));

            assert (NULL != tile.gN);
            assert (NULL != tile.gE);
            assert (NULL != tile.gS);
            assert (NULL != tile.gW);
            // if the strength == 0, then set the glue name to NULL
            //ma_data (&ma_val, glue_labelidx[ORI_NORTH], &tmp); if (tmp < 1) tile.gN = NULL;
            //ma_data (&ma_val, glue_labelidx[ORI_EAST],  &tmp); if (tmp < 1) tile.gE = NULL;
            //ma_data (&ma_val, glue_labelidx[ORI_SOUTH], &tmp); if (tmp < 1) tile.gS = NULL;
            //ma_data (&ma_val, glue_labelidx[ORI_WEST],  &tmp); if (tmp < 1) tile.gW = NULL;

            tile.name = tilename;
            tilename = NULL;
            tile.label = tilelabel;
            tilelabel = NULL;
            tile.type = TILETYPE_UNKNOWN;
            tile.group = tilegroup;
            tilegroup = 0;
            assert (NULL != tile.name);
            if (sz_buf_tilevec < pzzi->sz_tiles + 1) {
                void *newbuf;
                newbuf = realloc (pzzi->tiles, (pzzi->sz_tiles + 50) * sizeof (tile_t));
                pzzi->tiles = (tile_t *)newbuf;
                sz_buf_tilevec = pzzi->sz_tiles + 50;
            }
            memmove (&(pzzi->tiles[pzzi->sz_tiles]), &tile, sizeof(tile));
            pzzi->sz_tiles ++;

            tilename = NULL;
            tilelabel = NULL;
            assert (ma_size (&ma_str) == ma_size (&ma_val));
            stridx_cur = ma_size (&ma_str);
            memset (&tile, 0, sizeof (tile));
            memset (glue_val, 0, sizeof (glue_val));
            memset (glue_labelidx, 0, sizeof (glue_labelidx));
            memset (flg_glue_val, 0, sizeof (flg_glue_val));
#if DEBUG
            memset (glue_label, 0, sizeof (glue_label));
#endif
            // set the default label ""
            idx_add = label_search_or_add (&ma_str, &sl_glue, "");
            for (i = 0; i < ORI_MAX; i ++) {
                glue_labelidx[i] = idx_add;
            }

        } else if (0 == strcmp (word, "TILENAME")) {
            label = strtok_r (NULL, " \n", &brks);
            if ((NULL != label) && (0 == strcmp (label, "SEED"))) {
                //flg_isseed = 1;
            }
            if (NULL != label) {
                cstr_trim (label);
                if (NULL != tilename) {
                    free (tilename);
                    tilename = NULL;
                }
                if (strlen (label) > 0) {
                    tilename = strdup (label);
                }
            }
        } else if (0 == strcmp (word, "LABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                if (NULL != tilelabel) {
                    free (tilelabel);
                    tilelabel = NULL;
                }
                if (strlen (label) > 0) {
                    tilelabel = strdup (label);
                }
            }
        } else if (0 == strcmp (word, "NORTHBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_NORTH] = 0;
            } else {
                glue_val[ORI_NORTH] = atoi (word);
            }
            flg_glue_val[ORI_NORTH] = 1;
        } else if (0 == strcmp (word, "EASTBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_EAST] = 0;
            } else {
                glue_val[ORI_EAST] = atoi (word);
            }
            flg_glue_val[ORI_EAST] = 1;
        } else if (0 == strcmp (word, "SOUTHBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_SOUTH] = 0;
            } else {
                glue_val[ORI_SOUTH] = atoi (word);
            }
            flg_glue_val[ORI_SOUTH] = 1;
        } else if (0 == strcmp (word, "WESTBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_WEST] = 0;
            } else {
                glue_val[ORI_WEST] = atoi (word);
            }
            flg_glue_val[ORI_WEST] = 1;
#if USE_THREEDIMENTION
        } else if (0 == strcmp (word, "UPBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_FRONT] = 0;
            } else {
                glue_val[ORI_FRONT] = atoi (word);
                flg_glue_val[ORI_FRONT] = 1;
            }
            flg_is2d = 0;
        } else if (0 == strcmp (word, "DOWNBIND")) {
            word = strtok_r (NULL, " \n", &brks);
            if (NULL == word) {
                glue_val[ORI_BACK] = 0;
            } else {
                glue_val[ORI_BACK] = atoi (word);
                flg_glue_val[ORI_BACK] = 1;
            }
            flg_is2d = 0;
#else
        } else if (0 == strcmp (word, "UPBIND")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to support 3D data");
            ret = -1;
            break;
        } else if (0 == strcmp (word, "DOWNBIND")) {
            DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to support 3D data");
            ret = -1;
            break;
#endif
        } else if (0 == strcmp (word, "NORTHLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_NORTH] = idx_add;
            DUP_GLUE_LABEL (ORI_NORTH, label);
        } else if (0 == strcmp (word, "EASTLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_EAST] = idx_add;
            DUP_GLUE_LABEL (ORI_EAST, label);
        } else if (0 == strcmp (word, "SOUTHLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_SOUTH] = idx_add;
            DUP_GLUE_LABEL (ORI_SOUTH, label);
        } else if (0 == strcmp (word, "WESTLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_WEST] = idx_add;
            DUP_GLUE_LABEL (ORI_WEST, label);
#if USE_THREEDIMENTION
        } else if (0 == strcmp (word, "UPLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                fprintf (stderr, "UPLABEL '%s'\n", label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_FRONT] = idx_add;
            DUP_GLUE_LABEL (ORI_FRONT, label);
        } else if (0 == strcmp (word, "DOWNLABEL")) {
            label = strtok_r (NULL, " \n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                fprintf (stderr, "DOWNLABEL '%s'\n", label);
            } else {
                label = "";
            }
            // find the label
            idx_add = label_search_or_add (&ma_str, &sl_glue, label);
            assert (idx_add >= 0);
            glue_labelidx[ORI_BACK] = idx_add;
            DUP_GLUE_LABEL (ORI_BACK, label);
#endif

        //} else if (0 == strcmp (word, "CONCENTRATION")) {
        } else if (0 == strcmp (word, "TILECOLOR")) {
            label = strtok_r (NULL, "\n", &brks);
            if (NULL != label) {
                cstr_trim (label);
                tilegroup = convert_color_value (label);
            }
        //} else if (0 == strcmp (word, "TEXTCOLOR")) {
        //} else if (0 == strcmp (word, "icon")) {
        }
    }
    if (ret < 0) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Error in line: %d @ file '%s'", num_line, fn_tas);
        goto end_ts_sim_load_data_tastds;
    }
#if USE_THREEDIMENTION
    //ptsim->flg_is2d = flg_is2d;
#endif

    // save all of the glue information
    assert (ma_size (&ma_str) == ma_size (&ma_val));
    pzzi->sz_glues = ma_size (&ma_val);
    pzzi->glues = (glue_t *) malloc (pzzi->sz_glues * sizeof (glue_t));
    for (i = 0; i < pzzi->sz_glues; i ++) {
        ma_data (&ma_val, i, &(pzzi->glues[i].strength));
        assert (pzzi->glues[i].strength < 3);
        ma_data (&ma_str, i, &(pzzi->glues[i].name));
    }

end_ts_sim_load_data_tastds:
    fclose (fp_tas);
    //ma_clear (&ma_str, memarr_cb_destroy_cstr);
    ma_clear (&ma_str, NULL);
    ma_clear (&ma_val, NULL);
    slist_clear (&sl_glue, NULL);
    return ret;
}

int
zzi_load_tdp (const char *fn_tas, zigzag_info_t *pzzi)
{
    size_t num_line = 0;
    int ret = 0;
    char str[200];
    FILE *fp_tas = NULL;

    pzzi->temperature = 2;

    fp_tas = fopen (fn_tas, "r");
    if (NULL == fp_tas) {
        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "Unable to open file: '%s'", fn_tas);
        return -1;
    }
    ret = 0;
    for (; (fgets (str, sizeof(str), fp_tas) != NULL) && (ret >= 0); ) {
        char *brks;
        char *word;
        num_line ++;
        // strip trailing '\n' if it exists
        cstr_trim (str);
        cstr_stripblank (str);
        if (strlen (str) < 1) {
            continue;
        }
        word = strtok_r (str, " =", &brks);
        if (0 == strcmp (word, "SEED")) {
            // skip
        } else if (0 == strcmp (word, "Temperature")) {
            word = strtok_r (NULL, " =", &brks);
            pzzi->temperature = atoi (word);
        } else {
            // check the file name
            if ((0 != strstr (word, ".tds")) || (0 != strstr (word, ".TDS"))) {
                char *p;
                p = strrstr_len (fn_tas, strlen(fn_tas), "/");
                if (NULL == p) {
                    ret = zzi_load_tds (word, pzzi);
                } else {
                    if (p - fn_tas < sizeof (str) - strlen(str)) {
                        assert (str == word);
                        memmove (word + (p - fn_tas) + 1, str, strlen(str) + 1);
                        memmove (word, fn_tas, p - fn_tas + 1);
                        ret = zzi_load_tds (word, pzzi);
                    } else {
                        DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_ERROR, "No enough buffer. p(0x%x)-fn_tas(0x%x)=%d", p, fn_tas, p - fn_tas);
                        ret = -1;
                    }
                }
            } else {
                DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_INFO, "Unknown keywords in line: %d @ file '%s'. skip it", num_line, fn_tas);
            }
        }
    }
    fclose (fp_tas);
    DBGMSG (PFDBG_CATLOG_XMLDATA, PFDBG_LEVEL_DEBUG, "return %d", ret);

    return ret;
}

// convert the turing machine state to zigzag tile set, include the type of each tile.
int
zzi_tmi_to_zigzag2d2t (zigzag_info_t *pzzi, tm_info_t *ptmi)
{
    size_t i;
    size_t j;
    size_t idx;
    memarr_t ma_cstr_state;
    memarr_t ma_cstr_ioval;
    sortedlist_t sl_state;
    sortedlist_t sl_ioval;
    char * cstr_state = NULL;
    char * cstr_ioval = NULL;
    zigzag_form_t zzf;
    tile_t tile;
    char buf_name[100];
    char buf_gn[100];
    char buf_ge[100];
    char buf_gs[100];
    char buf_gw[100];

    zzf_init (&zzf, pzzi);
    assert (NULL != zzf.pzzi);
    zzf_feed_glue (&zzf, "", 0);

    //ma_init (&ma_val, sizeof (size_t));
    ma_init (&ma_cstr_state, sizeof (char *));
    ma_init (&ma_cstr_ioval, sizeof (char *));
    slist_init (&sl_state, sizeof (size_t), &ma_cstr_state/*userdata*/, slist_cb_comp_glue, slist_cb_swap_sizet);
    slist_init (&sl_ioval, sizeof (size_t), &ma_cstr_ioval/*userdata*/, slist_cb_comp_glue, slist_cb_swap_sizet);

    for (i = 0; i < ptmi->sz_tranfunc; i ++) {
        // get all of the type of states
        assert (NULL != ptmi->tranfunc[i].state_in);
        assert (0 != strcmp ("", ptmi->tranfunc[i].state_in));
        assert (NULL != ptmi->tranfunc[i].state_out);
        assert (0 != strcmp ("", ptmi->tranfunc[i].state_out));
        label_search_or_add (&ma_cstr_state, &sl_state, ptmi->tranfunc[i].state_in);
        label_search_or_add (&ma_cstr_state, &sl_state, ptmi->tranfunc[i].state_out);
        zzf_feed_glue (&zzf, ptmi->tranfunc[i].state_in, 1);
        zzf_feed_glue (&zzf, ptmi->tranfunc[i].state_out, 1);

        // get all of the type of input/output (seed bar, state transfer table) => \Segma
        assert (NULL != ptmi->tranfunc[i].input);
        assert (0 != strcmp ("", ptmi->tranfunc[i].input));
        assert (NULL != ptmi->tranfunc[i].output);
        assert (0 != strcmp ("", ptmi->tranfunc[i].output));
        label_search_or_add (&ma_cstr_ioval, &sl_ioval, ptmi->tranfunc[i].input);
        label_search_or_add (&ma_cstr_ioval, &sl_ioval, ptmi->tranfunc[i].output);
        zzf_feed_glue (&zzf, ptmi->tranfunc[i].input, 1);
        zzf_feed_glue (&zzf, ptmi->tranfunc[i].output, 1);
    }
    for (i = 0; i < ptmi->sz_tape; i ++) {
        assert (NULL != ptmi->tape[i]);
        assert (0 != strcmp ("", ptmi->tape[i]));
        label_search_or_add (&ma_cstr_ioval, &sl_ioval, ptmi->tape[i]);
        zzf_feed_glue (&zzf, ptmi->tape[i], 1);
    }

    zzf_feed_glue (&zzf, "sltr", 1);
    zzf_feed_glue (&zzf, "srtl", 1);
    for (i = 0; i < slist_size (&sl_state); i ++) {
        slist_data (&sl_state, i, &idx);
        ma_data (&ma_cstr_state, idx, &cstr_state);

        // create side tiles: $ | \delta .*.q | + | \delta .*.q' | \times | Lside, Rside |$
        // left
        snprintf (buf_gn, sizeof (buf_gn) - 1, "sl{%s}", cstr_state);
        snprintf (buf_ge, sizeof (buf_ge) - 1, "%sr", cstr_state);
        zzf_feed_glue (&zzf, buf_gn, 2);
        zzf_feed_glue (&zzf, buf_ge, 1);
        snprintf (buf_name, sizeof (buf_name) - 1, "SL_%02X_d", i);
        tile.gN = buf_gn;
        tile.gE = buf_ge;
        tile.gS = "sltr";
        tile.gW = "";
        tile.name = buf_name;
        tile.label = NULL;
        tile.type = TILETYPE_DWN2;
        zzf_feed_tile (&zzf, &tile);

        snprintf (buf_name, sizeof (buf_name) - 1, "SL_%02X_u", i);
        tile.gN = "sltr";
        tile.gE = cstr_state;
        tile.gS = buf_gn;
        tile.gW = "";
        tile.name = buf_name;
        tile.label = NULL;
        tile.type = TILETYPE_TWE1;
        zzf_feed_tile (&zzf, &tile);

        // right
        snprintf (buf_gn, sizeof (buf_gn) - 1, "sr{%s}", cstr_state);
        zzf_feed_glue (&zzf, buf_gn, 2);
        snprintf (buf_name, sizeof (buf_name) - 1, "SR_%02X_d", i);
        tile.gN = buf_gn;
        tile.gE = "";
        tile.gS = "srtl";
        tile.gW = cstr_state;
        tile.name = buf_name;
        tile.label = NULL;
        tile.type = TILETYPE_DEN2;
        zzf_feed_tile (&zzf, &tile);

        snprintf (buf_gw, sizeof (buf_gw) - 1, "%sr", cstr_state);
        zzf_feed_glue (&zzf, buf_gw, 1);
        snprintf (buf_name, sizeof (buf_name) - 1, "SR_%02X_u", i);
        tile.gN = "srtl";
        tile.gE = "";
        tile.gS = buf_gn;
        tile.gW = buf_gw;
        tile.name = buf_name;
        tile.label = NULL;
        tile.type = TILETYPE_TEW1;
        zzf_feed_tile (&zzf, &tile);

        for (j = 0; j < slist_size (&sl_ioval); j ++) {
            slist_data (&sl_ioval, j, &idx);
            ma_data (&ma_cstr_ioval, idx, &cstr_ioval);
            // copy tiles: any alphabate in \Segma

            // generate tile: (cstr_ioval, cstr_state, cstr_ioval, cstr_state), (cstr_ioval, cstr_state +"r", cstr_ioval, cstr_state +"r")
            snprintf (buf_name, sizeof (buf_name) - 1, "Cr_%s_%s", cstr_state, cstr_ioval);
            tile.gN = cstr_ioval;
            tile.gE = cstr_state;
            tile.gS = cstr_ioval;
            tile.gW = cstr_state;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DEE1;
            zzf_feed_tile (&zzf, &tile);

            snprintf (buf_ge, sizeof (buf_ge) - 1, "%sr", cstr_state);
            zzf_feed_glue (&zzf, buf_ge, 1);
            snprintf (buf_name, sizeof (buf_name) - 1, "Cl_%s_%s", cstr_state, cstr_ioval);
            tile.gN = cstr_ioval;
            tile.gE = buf_ge;
            tile.gS = cstr_ioval;
            tile.gW = buf_ge;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DWW1;
            zzf_feed_tile (&zzf, &tile);

            // Right move auxiliary: head copy tiles $t_2$, ($ | \delta .L.q' | \cup | \delta .H.q' | $) to all of $|\Sigma|$, (a{H},q'r,a{H},q'r)
            snprintf (buf_ge, sizeof (buf_ge) - 1, "%sr", cstr_state);
            snprintf (buf_gs, sizeof (buf_gs) - 1, "%s{H}", cstr_ioval);
            zzf_feed_glue (&zzf, buf_ge, 1);
            zzf_feed_glue (&zzf, buf_gs, 1);
            snprintf (buf_name, sizeof (buf_name) - 1, "Cl_%s_%s", buf_ge, buf_gs);
            tile.gN = buf_gs;
            tile.gE = buf_ge;
            tile.gS = buf_gs;
            tile.gW = buf_ge;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DWW1;
            zzf_feed_tile (&zzf, &tile);

            // Right move auxiliary: trun up from left, $t_1$,  $ | \delta .L.q' | $ 对 所有 $|\Sigma|$, (a{H},q',a,q'{R})
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s{H}", cstr_ioval);
            snprintf (buf_gw, sizeof (buf_gw) - 1, "%s{R}", cstr_state);
            zzf_feed_glue (&zzf, buf_gn, 1);
            zzf_feed_glue (&zzf, buf_gw, 1);
            snprintf (buf_name, sizeof (buf_name) - 1, "Rt1_%s_%s", cstr_state, cstr_ioval);
            tile.gN = buf_gn;
            tile.gE = cstr_state;
            tile.gS = cstr_ioval;
            tile.gW = buf_gw;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DEE1;
            zzf_feed_tile (&zzf, &tile);

            // Left move auxiliary:  turn up from right, $t_2$, $ | \delta .R.q' | $ 对 所有 $|\Sigma|$, (a{H},q'r{L},a,q'r)
            snprintf (buf_name, sizeof (buf_name) - 1, "Lt2_%s_%s", cstr_state, cstr_ioval);
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s{H}", cstr_ioval);
            snprintf (buf_ge, sizeof (buf_ge) - 1, "%sr{L}", cstr_state);
            snprintf (buf_gw, sizeof (buf_gw) - 1, "%sr", cstr_state);
            zzf_feed_glue (&zzf, buf_gn, 1);
            zzf_feed_glue (&zzf, buf_ge, 1);
            zzf_feed_glue (&zzf, buf_gw, 1);
            tile.gN = buf_gn;
            tile.gE = buf_ge;
            tile.gS = cstr_ioval;
            tile.gW = buf_gw;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DWW1;
            zzf_feed_tile (&zzf, &tile);
        }
    }

    for (i = 0; i < ptmi->sz_tranfunc; i ++) {
        snprintf (buf_name, sizeof (buf_name) - 1, "s%02X", i);
        snprintf (buf_gs, sizeof (buf_gs) - 1, "%s{H}", ptmi->tranfunc[i].input);
        zzf_feed_glue (&zzf, buf_gs, 1);
        switch (ptmi->tranfunc[i].move) {
        case TM_TRAN_MOVE_RIGHT:
            // Right move: state transfer, $t_0$, $ | \delta .L | $ , (c',q,c{H},q'{L})
            snprintf (buf_ge, sizeof (buf_ge) - 1, "%s{R}", ptmi->tranfunc[i].state_out);
            zzf_feed_glue (&zzf, buf_ge, 1);
            tile.gN = ptmi->tranfunc[i].output;
            tile.gE = buf_ge;
            tile.gS = buf_gs;
            tile.gW = ptmi->tranfunc[i].state_in;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DEE1;
            zzf_feed_tile (&zzf, &tile);
            break;

        case TM_TRAN_MOVE_LEFT:
            // Left move: state transfer, $t_0$, $ | \delta .R | $ , (c'{R},q,c{H},q')
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s{L}", ptmi->tranfunc[i].output);
            zzf_feed_glue (&zzf, buf_gn, 1);
            tile.gN = buf_gn;
            tile.gE = ptmi->tranfunc[i].state_out;
            tile.gS = buf_gs;
            tile.gW = ptmi->tranfunc[i].state_in;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DEE1;
            zzf_feed_tile (&zzf, &tile);

            // Left move auxiliary:  trun Left from under, $t_1$, $ | \delta .R | $, (c',q'{R},c'{R},q')
            snprintf (buf_gs, sizeof (buf_gs) - 1, "%s{L}", ptmi->tranfunc[i].output);
            snprintf (buf_ge, sizeof (buf_ge) - 1, "%sr", ptmi->tranfunc[i].state_out);
            snprintf (buf_gw, sizeof (buf_gw) - 1, "%sr{L}", ptmi->tranfunc[i].state_out);
            zzf_feed_glue (&zzf, buf_gs, 1);
            zzf_feed_glue (&zzf, buf_ge, 1);
            zzf_feed_glue (&zzf, buf_gw, 1);
            snprintf (buf_name, sizeof (buf_name) - 1, "Lt1_%s_%s", buf_ge, buf_gs);
            tile.gN = ptmi->tranfunc[i].output;
            tile.gE = buf_ge;
            tile.gS = buf_gs;
            tile.gW = buf_gw;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DWW1;
            zzf_feed_tile (&zzf, &tile);
            break;

        case TM_TRAN_MOVE_NOMOVE:
            // No move: state transfer, $t_0$, $ | \delta .H | $ , (c'{H},q,c{H},q')
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s{H}", ptmi->tranfunc[i].output);
            zzf_feed_glue (&zzf, buf_gn, 1);
            tile.gN = buf_gn;
            tile.gE = ptmi->tranfunc[i].state_out;
            tile.gS = buf_gs;
            tile.gW = ptmi->tranfunc[i].state_in;
            tile.name = buf_name;
            tile.label = NULL;
            tile.type = TILETYPE_DEE1;
            zzf_feed_tile (&zzf, &tile);
            break;
        default:
            break;
        }
    }
    // seed bar:
    zzf_feed_glue (&zzf, "gl", 2);
    zzf_feed_glue (&zzf, "gr", 2);
    snprintf (buf_name, sizeof (buf_name) - 1, "SEED");
    tile.gN = "srtl";
    tile.gE = "";
    tile.gS = "";
    tile.gW = "gr";
    tile.name = buf_name;
    tile.label = NULL;
    tile.type = TILETYPE_FW;
    zzf_feed_tile (&zzf, &tile);

    assert (NULL != ptmi->q0);
    assert (0 != strcmp ("", ptmi->q0));
    snprintf (buf_gn, sizeof (buf_gn) - 1, "sl{%s}", ptmi->q0);
    snprintf (buf_name, sizeof (buf_name) - 1, "SEED_L");
    zzf_feed_glue (&zzf, buf_gn, 1);
    tile.gN = buf_gn;
    tile.gE = "gl";
    tile.gS = "";
    tile.gW = "";
    tile.name = buf_name;
    tile.label = NULL;
    tile.type = TILETYPE_SWN2;
    zzf_feed_tile (&zzf, &tile);

    for (i = 0; i < ptmi->sz_tape; i ++) {
        assert (NULL != ptmi->tape[i]);
        assert (0 != strcmp ("", ptmi->tape[i]));
        snprintf (buf_name, sizeof (buf_name) - 1, "SEED_%02X", i);
        if (0 == i) {
            snprintf (buf_ge, sizeof (buf_ge) - 1, "s%02X", i);
            snprintf (buf_gw, sizeof (buf_gw) - 1, "gl");
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s", ptmi->tape[i]);
        } else if (ptmi->sz_tape == i + 1) {
            snprintf (buf_ge, sizeof (buf_ge) - 1, "gr");
            snprintf (buf_gw, sizeof (buf_gw) - 1, "s%02X", i - 1);
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s{H}", ptmi->tape[i]);
            zzf_feed_glue (&zzf, buf_gn, 2);
        } else {
            snprintf (buf_ge, sizeof (buf_ge) - 1, "s%02X", i);
            snprintf (buf_gw, sizeof (buf_gw) - 1, "s%02X", i - 1);
            snprintf (buf_gn, sizeof (buf_gn) - 1, "%s", ptmi->tape[i]);
        }
        zzf_feed_glue (&zzf, buf_ge, 2);
        zzf_feed_glue (&zzf, buf_gw, 2);
        tile.gN = buf_gn;
        tile.gE = buf_ge;
        tile.gS = "";
        tile.gW = buf_gw;
        tile.name = buf_name;
        tile.label = NULL;
        tile.type = TILETYPE_FW;
        zzf_feed_tile (&zzf, &tile);
    }

    zzf_feed_end (&zzf);
    zzf_clear (&zzf);
    ma_clear (&ma_cstr_state, memarr_cb_destroy_cstr);
    ma_clear (&ma_cstr_state, memarr_cb_destroy_cstr);
    slist_clear (&sl_state, NULL);
    slist_clear (&sl_ioval, NULL);
    return 0;
}
