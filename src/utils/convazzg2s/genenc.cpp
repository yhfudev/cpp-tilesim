/******************************************************************************
 * Name:        genenc.cpp
 * Purpose:     generate all of the encode tiles
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

#include "zzconv_inter.h"
#include "gentileset.h"

int
gen_tileset_enc (zzconv_t *pzzc)
{
    std::string glue_in;
    std::string glue_out;
    glue_enc_t::iterator itge;
    size_t i;
    size_t enc = 0;
    unsigned char *penc = NULL;

    assert (NULL != pzzc);
    assert (NULL != pzzc->ref_pzzi);

    for (i = 0; i < pzzc->ref_pzzi->sz_tiles; i ++) {
        itge = pzzc->glue_info.find (pzzc->ref_pzzi->tiles[i].gN);
        enc = INIT_VAL_ENC;
        penc = NULL;
        if (itge != pzzc->glue_info.end()) {
            assert (sizeof (enc) * 8 >= pzzc->maxbits);
            enc = itge->second.encoding;
            enc <<= (sizeof (enc) * 8 - pzzc->maxbits);
            enc = htonl (enc);
            penc = (unsigned char *)&enc;
            if (itge->second.strength < 1) {
                penc = NULL;
            }
        }
        switch (pzzc->ref_pzzi->tiles[i].type) {
        case TILETYPE_DWW1:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gE + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = std::string("(") + pzzc->ref_pzzi->tiles[i].gW + "," + "-)";
            GEN_ENC_TILE_DWW1 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_DWW2:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gE + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = pzzc->ref_pzzi->tiles[i].gW;
            GEN_ENC_TILE_DWW2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_DWN2:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gE + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = pzzc->ref_pzzi->tiles[i].gN;
            GEN_CONN_TILE_DWN2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_DEE1:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gW + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = std::string("(") + pzzc->ref_pzzi->tiles[i].gE + "," + "-)";
            GEN_ENC_TILE_DEE1 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_DEE2:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gW + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = pzzc->ref_pzzi->tiles[i].gE;
            GEN_ENC_TILE_DEE2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_DEN2:
            glue_in = std::string("(") + pzzc->ref_pzzi->tiles[i].gW + "," + pzzc->ref_pzzi->tiles[i].gS + ")";
            glue_out = pzzc->ref_pzzi->tiles[i].gN;
            GEN_CONN_TILE_DEN2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TEW1:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = std::string("(") + pzzc->ref_pzzi->tiles[i].gW + "," + "-)";
            //std::cerr << "(TEW1) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TEW1 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TEW2:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = pzzc->ref_pzzi->tiles[i].gW;
            //std::cerr << "(TEW2) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TEW2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TWW2:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = pzzc->ref_pzzi->tiles[i].gW;
            //std::cerr << "(TWW2) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TWW2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TWE1:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = std::string("(") + pzzc->ref_pzzi->tiles[i].gE + "," + "-)";
            //std::cerr << "(TWE1) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TWE1 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TWE2:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = pzzc->ref_pzzi->tiles[i].gE;
            //std::cerr << "(TWE2) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TWE2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_TEE2:
            glue_in = pzzc->ref_pzzi->tiles[i].gS;
            glue_out = pzzc->ref_pzzi->tiles[i].gE;
            //std::cerr << "(TEE2) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_ENC_TILE_TEE2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_FE:
            glue_in = pzzc->ref_pzzi->tiles[i].gW;
            glue_out = pzzc->ref_pzzi->tiles[i].gE;
            //std::cerr << "(FE) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_FIXED_TILE_FE (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_FW:
            glue_in = pzzc->ref_pzzi->tiles[i].gE;
            glue_out = pzzc->ref_pzzi->tiles[i].gW;
            //std::cerr << "(FW) i=" << i << ", gN='" << pzzc->ref_pzzi->tiles[i].gN << "'" << std::endl;
            GEN_FIXED_TILE_FW (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), penc, pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_SWN2:
            glue_in = pzzc->ref_pzzi->tiles[i].gE;
            glue_out = pzzc->ref_pzzi->tiles[i].gN;
            GEN_CONN_TILE_SWN2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_SEN2:
            glue_in = pzzc->ref_pzzi->tiles[i].gW;
            glue_out = pzzc->ref_pzzi->tiles[i].gN;
            GEN_CONN_TILE_SEN2 (pzzc->fpout, pzzc->ref_pzzi->tiles[i].group, pzzc->ref_pzzi->tiles[i].name, glue_in.c_str(), glue_out.c_str(), pzzc->maxbits, pzzc->k);
            break;

        case TILETYPE_NONE:
        case TILETYPE_UNKNOWN:
            break;

        default:
            std::cerr << "Unknown type: " << pzzc->ref_pzzi->tiles[i].type << std::endl;
            break;
        }
    }
    return 0;
}
