/******************************************************************************
 * Name:        gendec.cpp
 * Purpose:     generate the decode tile set
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <stack>
#include <set>
#include <vector>
#include <iostream>

#include "bintree.h"
#include "zzconv_inter.h"
#include "gendecbit.h"
#include "gentileset.h"

typedef int (* zzc_cb_gen_dectile_1bit_t) (FILE *fp, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k);

typedef struct _gendectile_t {
    FILE *fpout;
    size_t idx;
    size_t k; // used in zig-zag 2D temperature 1
    char *tilename;
    char *input;
    std::stack<size_t> gluein; // the glue interface of the each layer
    zzc_cb_gen_dectile_1bit_t cb_gen1bit; // gen_3d1t_dectile_1bit_2west or gen_3d1t_dectile_1bit_2east
} gendectile_t;

static void
bintree_cb_visit_output_infotile (bintree_node_t * root, void *userdata)
{
    gendectile_t *pgtt = (gendectile_t *)userdata;
    char glue_in[50];
    char glue_out1[50];
    char glue_out0[50];
    char *p1 = NULL;
    char *p0 = NULL;
    char *pi = NULL;
    size_t idx;
    //if (NULL != root->chd_left) {
        //sprintf (glue_out1, "%sG_%02X", tilename, idx);
    //}
    if (NULL != root->data) {
        pi = (char *)(root->data);
        sprintf (glue_in, "(%s,%s)", pgtt->input, pi);
        pi = glue_in;
    } else {
        assert (pgtt->gluein.size () > 0);
        idx = pgtt->gluein.top ();
        pgtt->gluein.pop ();
        sprintf (glue_in, "%sGI_%02X", pgtt->tilename, idx);
        pi = glue_in;
    }
    if (root->chd_right) {
        if (bintree_has_nochild (root->chd_right)) {
            assert (NULL != root->chd_right->data);
            p1 = (char *)(root->chd_right->data);
            sprintf (glue_out1, "(%s,%s)", pgtt->input, p1);
            p1 = glue_out1;
        } else {
            sprintf (glue_out1, "%sGI_%02X", pgtt->tilename, pgtt->idx);
            p1 = glue_out1;
            pgtt->gluein.push (pgtt->idx);
            pgtt->idx ++;
        }
    } else {
        p1 = NULL;
    }
    if (root->chd_left) {
        if (bintree_has_nochild (root->chd_left)) {
            assert (NULL != root->chd_left->data);
            p0 = (char *)(root->chd_left->data);
            sprintf (glue_out0, "(%s,%s)", pgtt->input, p0);
            p0 = glue_out0;
        } else {
            sprintf (glue_out0, "%sGI_%02X", pgtt->tilename, pgtt->idx);
            p0 = glue_out0;
            pgtt->gluein.push (pgtt->idx);
            pgtt->idx ++;
        }
    } else {
        p0 = NULL;
    }
    assert ((pgtt->cb_gen1bit == gen_dectile_1bit_2west) || (pgtt->cb_gen1bit == gen_dectile_1bit_2east));
    pgtt->cb_gen1bit (pgtt->fpout, pi, pi, p0, p1, pgtt->k);
}

#define TTS_WEST 0
#define TTS_EAST 1
// output information tile set by constructing binary tree
// input: the input glue name
// lst_output: the output glues list
// dir: the direction of the translation tile set, left or right
int
zzconv_build_dectileset (zzconv_t *pzzc, const char *tilename, char dir, const char *input, std::set<char *> *plst_output)
{
    bintree_node_t * root;
    gendectile_t gtt;

    if (NULL == pzzc) {
        return -1;
    }

    gtt.k = pzzc->k;
    gtt.idx = 0;
    gtt.tilename = (char *)tilename;
    gtt.input = (char *)input;
    gtt.fpout = pzzc->fpout;

    switch (dir) {
    case TTS_WEST:
        gtt.cb_gen1bit = gen_dectile_1bit_2west;
        break;
    case TTS_EAST:
        gtt.cb_gen1bit = gen_dectile_1bit_2east;
        break;
    default:
        return -1;
    }

    root = bintree_create ();
    assert (NULL != root);
    root->data = (void *)"-";
    glue_enc_t::iterator it;
    std::set<char *>::iterator itout;

    for (itout = plst_output->begin(); itout != plst_output->end(); itout ++) {
        it = pzzc->glue_info.find (*itout);
        if (it == pzzc->glue_info.end()) {
            std::cerr << "Error in finding the glue map" << std::endl;
            return -1;
        }
        bintree_construct_bincode (root, it->second.encoding, pzzc->maxbits, *itout);
    }
    bintree_traversal_preorder (root, bintree_cb_visit_output_infotile, (void *)(&gtt));
    bintree_destroy (root);
    return 0;
}

typedef struct _gluedir_t {
    char dir; // TTS_WEST, TTS_EAST
    size_t idx; // the index of the glue in the gluecategorylst
} gluedir_t;

int
gen_tileset_dec (zzconv_t *pzzc)
{
    std::vector<std::set<char *> *> gluecategorylst; // 以输入分组的 glue 列表组
    std::map<std::string, gluedir_t> gluename2diridx; // glue 对应的方向/gluecategorylst列表的索引
    std::map<std::string, gluedir_t>::iterator itg2i;
    std::string trantilename; // translation tile set name
    gluedir_t tgi;
    char *input = NULL;
    size_t i;
    size_t cnt = 0;

    for (i = 0; i < pzzc->ref_pzzi->sz_tiles; i ++) {
        switch (pzzc->ref_pzzi->tiles[i].type) {
        case TILETYPE_DWW1:
        case TILETYPE_DWW2:
        case TILETYPE_DWN2:
            input = pzzc->ref_pzzi->tiles[i].gE;
            tgi.dir = TTS_WEST;
            break;

        case TILETYPE_DEE1:
        case TILETYPE_DEE2:
        case TILETYPE_DEN2:
            input = pzzc->ref_pzzi->tiles[i].gW;
            tgi.dir = TTS_EAST;
            break;

        case TILETYPE_TEW1:
        case TILETYPE_TEW2:
        case TILETYPE_TWW2:
        case TILETYPE_TWE1:
        case TILETYPE_TWE2:
        case TILETYPE_TEE2:
        case TILETYPE_FE:
        case TILETYPE_FW:
        case TILETYPE_SWN2:
        case TILETYPE_SEN2:
        default:
            continue; // FIXME: continue to for()
            break;
        }
        itg2i = gluename2diridx.find (input);
        if (itg2i == gluename2diridx.end ()) {
            // it's the first occur of the glue type
            // add it to buf
            std::set<char *> *pnewglst = new std::set<char *>;
            gluecategorylst.push_back (pnewglst);
            tgi.idx = gluecategorylst.size() - 1;
            gluename2diridx[input] = tgi;
        } else {
            tgi.idx = itg2i->second.idx;
        }
        // find the list according to the input glue
        std::set<char *> *pglst = gluecategorylst[tgi.idx];
        // include the name of the south glue
        pglst->insert (pzzc->ref_pzzi->tiles[i].gS);
        cnt ++;
    }

    for (itg2i = gluename2diridx.begin(); itg2i != gluename2diridx.end(); itg2i ++) {
        trantilename = std::string ("T") + (itg2i->second.dir == TTS_WEST?"L":"R");
        trantilename += itg2i->first.c_str();
        zzconv_build_dectileset (pzzc, trantilename.c_str(), itg2i->second.dir, itg2i->first.c_str(),
            gluecategorylst[itg2i->second.idx]);
    }
//end_gendec:
    // delete the resource used in the coding
    std::vector<std::set<char *> *>::iterator itggl;
    for (itggl = gluecategorylst.begin(); itggl != gluecategorylst.end(); itggl ++) {
        delete *itggl;
    }
    return 0;
}
