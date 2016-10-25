/******************************************************************************
 * Name:        tilesimatam.c
 * Purpose:     simulator for aTAM tile set self-assembly
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pfrandom.h"
#include "tilestruct.h"
#include "tilesimatam.h"

#define NO_NEG_POSITION 0

#if USE_TILESIM_ATAM

#define tstc_data_lock(p, i) slist_data_lock(&((p)->tbuf), i)
#define tstc_data_unlock(p, i) slist_data_unlock(&((p)->tbuf), i)
#define tstc_data(p, i, pdata) slist_data(&((p)->tbuf), i, pdata)

// the opposite direction
const int g_tile_glue_opp_direction[ORI_MAX] = {
    /*ORI_NORTH*/ ORI_SOUTH,
    /*ORI_EAST*/  ORI_WEST,
    /*ORI_SOUTH*/ ORI_NORTH,
    /*ORI_WEST*/  ORI_EAST,
#if USE_THREEDIMENTION
    /*ORI_FRONT*/ ORI_BACK,
    /*ORI_BACK*/  ORI_FRONT,
#endif
};

int
tspos_step2dir_atam (tsposition_t *ptp, int dir)
{
    assert (NULL != ptp);
    dir = dir % ORI_MAX;
    switch (dir) {
    case ORI_NORTH:
        ptp->y ++;
        break;
    case ORI_EAST:
        ptp->x ++;
        break;
    case ORI_SOUTH:
        ptp->y --;
        break;
    case ORI_WEST:
        ptp->x --;
        break;
#if USE_THREEDIMENTION
    case ORI_FRONT:
        ptp->z --;
        break;
    case ORI_BACK:
        ptp->z ++;
        break;
#endif
    }
    return 0;
}

// if the position is moved, then return 1
int
tstc_nomalize_atam (tstilecomb_t *ptc)
{
    return tstc_nomalize (ptc);
}

// find all of the pottential position near the tile (index is idx) and save it to p_ret_pos
static int
find_potential_position_adjacent2tile (tssiminfo_t *ptsim, tstilecomb_t *ptarget, size_t idx, sortedlist_t *p_ret_pos)
{
    size_t i;
    size_t j;
    tstilecombitem_t *ptstci;
    tstilecombitem_t tstci_cur;
    tstilecombitem_t tstci_chk;
    tstilecombitem_t tstci_glue;
    int glue;
    size_t total_strength;
    size_t maxdir = ORI_MAX;
    if (ptsim->flg_is2d) {
        maxdir = 4;
    }
    //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "ORI_MAX=%d; maxdir=%d", ORI_MAX, maxdir);

    tstc_data (ptsim->ptarget, idx, &tstci_cur);

    for (i = 0; i < maxdir; i ++) {
        memmove (&tstci_chk, &tstci_cur, sizeof (tstci_chk));
        tspos_step2dir_atam (&(tstci_chk.pos), i);
        // check for blank position near the current tile
        if (slist_find (&((ptarget)->tbuf), (&tstci_chk), slist_cb_comp_heter_tstilecomb_hastile, NULL) >= 0) {
            continue;
        }
        total_strength = 0;
        for (j = 0; j < maxdir; j ++) {
            memmove (&tstci_glue, &tstci_chk, sizeof (tstci_chk));
            tspos_step2dir_atam (&(tstci_glue.pos), j);
            // check for exist tiles near the blank position
            if (slist_find (&((ptarget)->tbuf), (&tstci_glue), slist_cb_comp_heter_tstilecomb_hastile, &idx) < 0) {
                continue;
            }
            // check the glue strength
            ptstci = (tstilecombitem_t *) tstc_data_lock (ptsim->ptarget, idx);
            if (NULL == ptstci) {
                continue;
            }
            // get the glue
            glue = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptstci, g_tile_glue_opp_direction[j], 0);
            tstc_data_unlock (ptsim->ptarget, idx);
            if ((glue < 0) || (glue >= ptsim->num_gluevec)) {
                // error
                break;
            }
            total_strength += ptsim->pgluevec[glue];
        }
        if (total_strength >= ptsim->temperature) {
            // store it
            slist_store (p_ret_pos, &(tstci_chk.pos));
        }
    }
    return 0;
}

// if the tile(idx_tile) place at position(ppos), what's the strengh?
size_t
ts_atam_chk_strength (tssiminfo_t *ptsim, size_t idx_tile, tsposition_t *ppos)
{
    size_t i;
    size_t idx;
    int glue;
    tstilecombitem_t tstci_chk;
    tstilecombitem_t tstci_glue;
    tstilecombitem_t * ptstci;
    size_t total_strength;
    size_t maxdir = ORI_MAX;
    if (ptsim->flg_is2d) {
        maxdir = 4;
    }

    assert ((0 <= idx_tile) && (idx_tile < ptsim->num_tilevec));
    total_strength = 0;
    memset (&tstci_chk, 0, sizeof (tstci_chk));
    memmove (&(tstci_chk.pos), ppos, sizeof (*ppos));
    for (i = 0; i < maxdir; i ++) {
        memmove (&tstci_glue, &tstci_chk, sizeof (tstci_chk));
        tspos_step2dir_atam (&(tstci_glue.pos), i);
        // check for exist tiles near the blank position
        if (slist_find (&(ptsim->ptarget->tbuf), (&tstci_glue), slist_cb_comp_heter_tstilecomb_hastile, &idx) < 0) {
            continue;
        }
        // check the glue strength
        ptstci = (tstilecombitem_t *) tstc_data_lock (ptsim->ptarget, idx);
        if (NULL == ptstci) {
            continue;
        }
        // get the glue
        glue = tile_get_glue (ptsim->ptilevec, ptsim->num_tilevec, ptstci, g_tile_glue_opp_direction[i], 0);
        tstc_data_unlock (ptsim->ptarget, idx);
        if ((glue < 0) || (glue >= ptsim->num_gluevec)) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_LOG, "record error,i=%d,maxglue=%d, glue: %d", i, ptsim->num_gluevec, glue);
            break;
        }
        //assert ((0 <= ptsim->ptilevec[idx_tile].glues[i]) && (ptsim->ptilevec[idx_tile].glues[i] < ptsim->num_gluevec));
        if (ptsim->ptilevec[idx_tile].glues[i] >= ptsim->num_gluevec) {
            continue;
        }
        if (ptsim->ptilevec[idx_tile].glues[i] == glue) {
            // add 
            total_strength += ptsim->pgluevec[glue];
        }
    }
    //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "total_strength=%d", total_strength);
    return total_strength;
}

typedef struct _it_potpos_t {
    tssiminfo_t *ptsim;
    size_t idx_tile;
    memarr_t *p_ret_pos;
} it_potpos_t;

static void
slist_cb_iter_potpos (void * userdata, size_t idx, void *data)
{
    it_potpos_t *pipp = (it_potpos_t *)userdata;
    tsposition_t *ppos = (tsposition_t *)data;
    size_t total_strength;

    total_strength = ts_atam_chk_strength (pipp->ptsim, pipp->idx_tile, ppos);
    if (total_strength >= pipp->ptsim->temperature) {
        ma_append (pipp->p_ret_pos, ppos);
    }
}

// try to place a tile to the target supertile
// ppotposlist: sortedlist_t of tsposition_t
// p_ret_pos: the positions which the tile can be placed.
static int
find_pos_4_tile (tssiminfo_t *ptsim, sortedlist_t *ppotposlist, size_t idx_tile, memarr_t *p_ret_pos)
{
    it_potpos_t ipp;
    ipp.ptsim = ptsim;
    ipp.idx_tile = idx_tile;
    ipp.p_ret_pos = p_ret_pos;
    slist_foreach (ppotposlist, slist_cb_iter_potpos, &ipp);
    return 0;
}

// place one tile at the position ppos
// return 0 if OK, return < 0 if failed.
static int
place_tile_at (tssiminfo_t *ptsim, sortedlist_t *ppotposlist, size_t idx_tile, tsposition_t *ppos)
{
    tstilecombitem_t tstci;
    size_t idx_pp;
    size_t total_strength;

    if (tstc_hastile (ptsim->ptarget, ppos)) {
        // error: exist tile at the position
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "the position is occupied!");
        return -1;
    }
    // check the ppos, if the ppos is valid. (use potposlist)
    if (slist_find (ppotposlist, ppos, NULL, &idx_pp) < 0) {
        // error: the position is not list in potposlist
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "the position is invalid!");
        assert (0);
        return -1;
    }
    // check the glue strength
    total_strength = ts_atam_chk_strength (ptsim, idx_tile, ppos);
    if (total_strength < ptsim->temperature) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "week strength!");
        return -1;
    }

    memset (&tstci, 0, sizeof (tstci));
    memmove (&(tstci.pos), ppos, sizeof (*ppos));
    tstci.idtile = idx_tile;
    tstc_additem (ptsim->ptarget, &tstci);

#if NO_NEG_POSITION
if (tstc_nomalize_atam (ptsim->ptarget) > 0) {
    size_t i;
    slist_rmalldata (ppotposlist, NULL);
    for (i = 0; i < TSTC_NUM_TILE (ptsim->ptarget); i ++) {
        // for each of the positions adjacent to it
        find_potential_position_adjacent2tile (ptsim, ptsim->ptarget, i, ppotposlist);
    }
} else {
#endif
    // remove the pos from potposlist
    slist_rmdata (ppotposlist, idx_pp, NULL);

    // check the potential positions around the one added.
    if (0 == slist_find (&((ptsim->ptarget)->tbuf), &(tstci), slist_cb_comp_heter_tstilecomb_hastile, &idx_pp)) {
        find_potential_position_adjacent2tile (ptsim, ptsim->ptarget, idx_pp, ppotposlist);
    }
#if NO_NEG_POSITION
}
#endif
    // record this step?
    return 0;
}

int
tilesim_atam (tssiminfo_t *ptsim, const char *name_sim)
{
    int ret = 0;
    size_t ii;
    size_t idx;
    size_t idx_real;
    void *hashref;
    size_t cnt_fail;
    tsposition_t pos;
    memarr_t lst_place;
    tstilecombitem_t *ptstci;
    tstilecombitem_t tstci;
    char flg_idx_sequence = 0;
    sortedlist_t potposlist;  // tsposition_t: the list of all of the positions which the tile could attached to. Potential positions

    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "tilesim_atam('%s') ....", name_sim);
    if (NULL == ptsim) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "NULL parameter!");
        return -1;
    }
    if (NULL == ptsim->ptarget) {
        ptsim->ptarget = tstc_create ();
        if (NULL == ptsim->ptarget) {
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "error in malloc for target!");
            return -1;
        }
    }
    if (TSTC_NUM_TILE (ptsim->ptarget) < 1) {
        // place the seed to the supertile
        // find the seed tile
        for (ii = 0; ii < ptsim->num_tilevec; ii ++) {
            if ((NULL != ptsim->ptilevec[ii].name) && (0 == strcmp ("SEED", ptsim->ptilevec[ii].name))) {
                break;
            }
        }
        if (ii >= ptsim->num_tilevec) {
            // not found
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Not found SEED tile!");
            return -1;
        }
        memset (&tstci, 0, sizeof (tstci));
        tstci.idtile = ii;
        tstc_additem (ptsim->ptarget, &tstci);
        if (ptsim->cb_resultinfo) {
#if USE_THREEDIMENTION
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') tile[%d] @ (%d,%d,%d)rot[%d]", name_sim, tstci.idtile, tstci.pos.x, tstci.pos.y, tstci.pos.z, tstci.rotnum);
#else
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') tile[%d] @ (%d,%d)rot[%d]", name_sim, tstci.idtile, tstci.pos.x, tstci.pos.y, tstci.rotnum);
#endif
            if (ptsim->cb_resultinfo (ptsim->userdata, ptsim->num_tilevec, tstci.idtile, &tstci, ptsim->temperature, ptsim->num_tilevec, ptsim->ptarget, NULL) < 0) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "break by caller's cb_resultinfo");
                return 0;
            }
        }
    }

    assert (NULL != ptsim->ptarget);
    tstc_nomalize_atam (ptsim->ptarget);

    if (hash_revert_init (ptsim->num_tilevec - 1, &hashref) < 0) {
        return -1;
    }
    ma_init (&lst_place, sizeof (tsposition_t));

    if (slist_init (&potposlist, sizeof (tsposition_t), NULL, slist_cb_comp_tspos, slist_cb_swap_tspos) < 0) {
        ma_clear (&lst_place, NULL);
        hash_revert_clear (hashref);
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "error in init potpos!");
        return -1;
    }
    // re-calculate the potential positions
    slist_rmalldata (&potposlist, NULL);
    for (ii = 0; ii < TSTC_NUM_TILE (ptsim->ptarget); ii ++) {
        // for each of the positions adjacent to it
        find_potential_position_adjacent2tile (ptsim, ptsim->ptarget, ii, &potposlist);
    }
    if (slist_size (&potposlist) < 1) {
        ret = 0;
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "not site for tile!");
        goto end_tilesim_atam;
    }

    cnt_fail = 0; // the times of failure;
    for (ii = my_irrand (ptsim->num_tilevec); cnt_fail < ptsim->num_tilevec; ii ++) {
        ii = ii % ptsim->num_tilevec;
        if (0 == flg_idx_sequence) {
            idx_real = hash_revert (hashref, ii, ptsim->num_tilevec - 1);
        } else {
            idx_real = ii;
        }
        // for each of the tiles in the ptilevec
        ma_rmalldata (&lst_place, NULL);
        if (find_pos_4_tile (ptsim, &potposlist, idx_real, &lst_place) < 0) {
            cnt_fail ++;
            continue;
        }
        if (ma_size (&lst_place) < 1) {
            cnt_fail ++;
            continue;
        }
        // select one position
        idx = my_irrand (ma_size (&lst_place));
        ma_data (&lst_place, idx, &pos);
        if (place_tile_at (ptsim, &potposlist, idx_real, &pos) < 0) {
            // error
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "palce tile error, idx=%d", idx_real);
            break;
        }
        // TODO: notify the caller
        if (ptsim->cb_resultinfo) {
            memset (&tstci, 0, sizeof (tstci));
            memmove (&(tstci.pos), &pos, sizeof (pos));
            //tstci.idtile = i;
#if USE_THREEDIMENTION
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') tile[%d] @ (%d,%d,%d)rot[%d]", name_sim, idx_real, tstci.pos.x, tstci.pos.y, tstci.pos.z, tstci.rotnum);
#else
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "('%s') tile[%d] @ (%d,%d)rot[%d]", name_sim, idx_real, tstci.pos.x, tstci.pos.y, tstci.rotnum);
#endif
            if (ptsim->cb_resultinfo (ptsim->userdata, ptsim->num_tilevec, idx_real, &tstci, ptsim->temperature, ptsim->num_tilevec, ptsim->ptarget, NULL) < 0) {
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "break by caller's cb_resultinfo");
                break;
            }
        }
        cnt_fail = 0;
        if (0 == flg_idx_sequence) {
            ii = my_irrand (ptsim->num_tilevec);
        }
    }
#if DEBUG
    if (cnt_fail >= ptsim->num_tilevec) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "faile exceed! flag2d=%d", (int)(ptsim->flg_is2d));
    }
#endif
end_tilesim_atam:
    ma_clear (&lst_place, NULL);
    hash_revert_clear (hashref);

    slist_clear (&potposlist, NULL);
    return ret;
}

#endif /* USE_TILESIM_ATAM */
