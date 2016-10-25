/******************************************************************************
 * Name:        zzconv.cpp
 * Purpose:     Convert the arbitrary zig-zag tile set from 2D2T to 2D1T
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stddef.h> // size_t
#include <stdlib.h>
#include <assert.h>

#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <stack>
#include <map>

#include "outtds.h"
#include "zzconv.h"
#include "zzconv_inter.h"
#include "gentileset.h"

zzconv_t *
zzconv_create (zigzag_info_t *pzzi)
{
    size_t i;
    size_t enc;
    std::map<std::string, size_t>::iterator itstrn;
    glue_enc_t::iterator itge;
    zzconv_t *pzzc;

    if (NULL == pzzi) {
        return NULL;
    }
    pzzc = new zzconv_t;
    if (NULL == pzzc) {
        return NULL;
    }
    pzzc->k = 0;
    pzzc->rotnum = 0; // not used
    pzzc->maxbits = 0;
    pzzc->ref_pzzi = pzzi;
    pzzc->fpout = NULL;

    // create the strength map of all of the gules
    for (i = 0; i < pzzi->sz_glues; i ++) {
        //std::cerr << "add '" << pzzi->glues[i].name << "' ..." << std::endl;
        pzzc->glue_strength[pzzi->glues[i].name] = pzzi->glues[i].strength;
    }
#ifdef SHOW_MSG
    for (i = 0; i < pzzi->sz_glues; i ++) {
        assert (pzzi->glues[i].strength == pzzc->glue_strength[pzzi->glues[i].name]);
    }
    for (i = 0, itstrn = pzzc->glue_strength.begin(); itstrn != pzzc->glue_strength.end(); itstrn ++, i ++) {
        std::cerr << "pzzc->glue_strength['" << itstrn->first << "']=" << itstrn->second << "," << std::endl;
    }
#endif
    // encoding the glues of tiles at the position N,S
    enc = 0;
#ifdef SHOW_MSG
    std::cerr << "tile size: " << pzzi->sz_tiles << std::endl;
#endif
    for (i = 0; i < pzzi->sz_tiles; i ++) {
#ifdef SHOW_MSG
        std::cerr << "search '" << pzzi->tiles[i].gN << "' ...";
#endif
        itstrn = pzzc->glue_strength.find (pzzi->tiles[i].gN);
        if ((itstrn != pzzc->glue_strength.end ()) && (itstrn->second == 1)) {
            itge = pzzc->glue_info.find (itstrn->first);
            if (itge == pzzc->glue_info.end ()) {
                glue_info_t gi;
                gi.strength = itstrn->second;
                gi.encoding = enc;
                enc ++;
                pzzc->glue_info[itstrn->first] = gi;
#ifdef SHOW_MSG
                std::cerr << "glue '" << itstrn->first << "' = enc(" << gi.encoding << ")" << std::endl;
                std::cerr << "OK" << std::endl;
            } else {
                std::cerr << "(Exist: '" << itstrn->first << "')" << std::endl;
#endif
            }

#ifdef SHOW_MSG
        } else {
            std::cerr << "IGNORE!";
            if (itstrn != pzzc->glue_strength.end ()) {
                std::cerr << "'" << pzzi->tiles[i].gN << "'={stren=" << itstrn->second << "})";
            }
            std::cerr << std::endl;
#endif
        }
        itstrn = pzzc->glue_strength.find (pzzi->tiles[i].gS);
        if ((itstrn != pzzc->glue_strength.end ()) && (itstrn->second == 1)) {
            itge = pzzc->glue_info.find (itstrn->first);
            if (itge == pzzc->glue_info.end ()) {
                glue_info_t gi;
                gi.strength = itstrn->second;
                gi.encoding = enc;
                enc ++;
                pzzc->glue_info[itstrn->first] = gi;
                //std::cerr << "glue '" << itstrn->first << "' = enc(" << gi.encoding << ")" << std::endl;
            }
        }
    }

    // get the max number of bit
    pzzc->maxbits = 1;
    for (i = 0x01; ((i << pzzc->maxbits) < enc); pzzc->maxbits ++) {
    }
    std::cerr << "enc=" << enc << "; m_maxbits=" << pzzc->maxbits << std::endl;
    // a simple way to revert the code: encode=0x01^maxbits - 1 - encode
    // TODO: check the number 1 bit in the code
    enc = (0x01 << pzzc->maxbits) - 1;
    for (itge = pzzc->glue_info.begin(); itge != pzzc->glue_info.end(); itge ++) {
        //std::cerr << "chg enc from " << itge->second.encoding << " to " << enc - itge->second.encoding << std::endl;
        itge->second.encoding = enc - itge->second.encoding;
    }
#if 1 // SHOW_MSG
    std::cerr << "glues in glue map:" << std::endl;
    for (i = 0, itge = pzzc->glue_info.begin(); itge != pzzc->glue_info.end(); itge ++, i ++) {
        std::cerr << "(" << i << ") '" << itge->first << "'={stren=" << itge->second.strength << ",enc=";
        std::cerr << itge->second.encoding << "=0x";
        std::cerr.setf(std::ios::hex);
        std::cerr << itge->second.encoding;
        std::cerr.unsetf(std::ios::hex);
        std::cerr << "},\n";
    }
#endif

    return pzzc;
}

int
zzconv_destroy (zzconv_t *pzzc)
{
    assert (NULL != pzzc);
    delete pzzc;
    return 0;
}

int
zzconv_conv_categories_to_3d1t (zzconv_t *pzzc, FILE *fpout)
{
    pzzc->k = 0;
    pzzc->fpout = fpout;

    gen_tileset_dec (pzzc);
    gen_tileset_enc (pzzc);
    return 0;
}

int
zzconv_conv_categories_to_2d1t (zzconv_t *pzzc, FILE *fpout, size_t k)
{
    pzzc->k = k;
    pzzc->fpout = fpout;

    gen_tileset_dec (pzzc);
    gen_tileset_enc (pzzc);
    return 0;
}
