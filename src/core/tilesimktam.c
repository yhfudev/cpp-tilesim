/******************************************************************************
 * Name:        tilesimktam.c
 * Purpose:     simulator for kTAM tile set self-assembly
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "pfrandom.h"
#include "tilestruct.h"
#include "tilesimatam.h"

#if USE_TILESIM_ATAM

#define tstc_data_lock(p, i) slist_data_lock(&((p)->tbuf), i)
#define tstc_data_unlock(p, i) slist_data_unlock(&((p)->tbuf), i)
#define tstc_data(p, i, pdata) slist_data(&((p)->tbuf), i, pdata)

typedef struct _ts_roffbidx_t {
    double roffb; // the r_{off,b} value of the tile at the position pos in the supertile
    tsposition_t pos; // the position of the tile in the supertile
} ts_roffbidx_t;

#if DEBUG
static void
slist_cb_iter_potpos (void * userdata, size_t idx, void *data)
{
    tsposition_t *ptsp = (tsposition_t *)data;
    printf ("[%d](x=%d,y=%d,z=%d); ", idx, ptsp->x, ptsp->y, ptsp->z);
}

void
potpos_dump (sortedlist_t *ppotposlist)
{
    printf ("dump ppotposlist:\n");
    slist_foreach (ppotposlist, slist_cb_iter_potpos, NULL);
    printf ("\n");
}

static void
slist_cb_iter_roffbidx (void * userdata, size_t idx, void *data)
{
    ts_roffbidx_t *ptsp = (ts_roffbidx_t *)data;
    printf ("[%d]roffb=%f,(x=%d,y=%d,z=%d); ", idx, ptsp->roffb, ptsp->pos.x, ptsp->pos.y, ptsp->pos.z);
}

void
roffbidx_dump (sortedlist_t *proffbidx)
{
    printf ("dump roffbidx:\n");
    slist_foreach (proffbidx, slist_cb_iter_roffbidx, NULL);
    printf ("\n");
}
#endif

//////////////////////////////////////////
int
slist_cb_comp_roffbidx (void * userdata, void * data1, void * data2)
{
    return tspos_cmp (&(((ts_roffbidx_t *)data1)->pos), &(((ts_roffbidx_t *)data2)->pos));
}

int
slist_cb_swap_roffbidx (void *userdata, void * data1, void * data2)
{
    ts_roffbidx_t * ptci1 = (ts_roffbidx_t *)data1;
    ts_roffbidx_t * ptci2 = (ts_roffbidx_t *)data2;
    ts_roffbidx_t tmp;
    memmove (&tmp, ptci1, sizeof (tmp));
    memmove (ptci1, ptci2, sizeof (tmp));
    memmove (ptci2, &tmp, sizeof (tmp));
    return 0;
}

// proffblist: ts_roffbidx_t
static int
calculate_roffb_ktam (tssiminfo_t *ptsim, tstilecombitem_t *ptstci_cur, sortedlist_t *proffblist)
{
    double strength;
    
    ts_roffbidx_t roffbidx;

    memmove (&(roffbidx.pos), &(ptstci_cur->pos), sizeof (ptstci_cur->pos));
    strength = (double) ts_atam_chk_strength (ptsim, ptstci_cur->idtile, &(ptstci_cur->pos));
    roffbidx.roffb = ptsim->ratek * exp (0.0 - ptsim->Gse * strength);//r_{off,b} = k_a  \times e^{-b * G_{se}}
    if (0 == strcmp (ptsim->ptilevec[ptstci_cur->idtile].name, "SEED")) {
        roffbidx.roffb = 0;
    }
    slist_store (proffblist, &roffbidx);

    return 0;
}

// find all of the pottential position near the tile (index is idx) and save it to p_ret_pos
static int
find_potential_position_adjacent2tile_ktam (tssiminfo_t *ptsim, tstilecomb_t *ptarget, size_t idx, sortedlist_t *p_ret_pos)
{
    size_t i;
    size_t j;
    tstilecombitem_t tstci_cur;
    tstilecombitem_t tstci_chk;
    size_t maxdir = ORI_MAX;
    if (ptsim->flg_is2d) {
        maxdir = 4;
    }

    tstc_data(ptsim->ptarget, idx, &tstci_cur);

    for (i = 0; i < maxdir; i ++) {
        memmove (&tstci_chk, &tstci_cur, sizeof (tstci_chk));
        tspos_step2dir_atam (&(tstci_chk.pos), i);
        // check for blank position near the current tile
        if (slist_find (&((ptarget)->tbuf), (&tstci_chk), slist_cb_comp_heter_tstilecomb_hastile, NULL) >= 0) {
            continue;
        }
        // store it
        slist_store (p_ret_pos, &(tstci_chk.pos));
    }
    return 0;
}

// remove the positions around pos, if the position is not near any tile in ptarget.
static int
update_potential_position_after_remove_tile (tssiminfo_t *ptsim, sortedlist_t *ppotposlist, tsposition_t *ppos)
{
    size_t i;
    size_t j;
    size_t idx;
    tstilecombitem_t tstci_cur;
    tstilecombitem_t tstci_chk;
    tstilecombitem_t tstci_adjacent;
    size_t total_tiles;
    size_t maxdir = ORI_MAX;
    if (ptsim->flg_is2d) {
        maxdir = 4;
    }

    // if ppos is occupied by a tile, then it dont't need to be checked.
    memmove (&(tstci_cur.pos), ppos, sizeof (*ppos));
    if (slist_find (&(ptsim->ptarget->tbuf), (&tstci_cur), slist_cb_comp_heter_tstilecomb_hastile, NULL) >= 0) {
        return 0;
    }

    memmove (&tstci_chk, &tstci_cur, sizeof (tstci_chk));
    for (i = 0; i <= maxdir; i ++) {
        // check for blank position near the current tile
        if (slist_find (&((ptsim->ptarget)->tbuf), (&tstci_chk), slist_cb_comp_heter_tstilecomb_hastile, NULL) < 0) {
            total_tiles = 0;
            for (j = 0; j < maxdir; j ++) {
                memmove (&tstci_adjacent, &tstci_chk, sizeof (tstci_chk));
                tspos_step2dir_atam (&(tstci_adjacent.pos), j);
                // check for exist tiles near the blank position
                if (slist_find (&((ptsim->ptarget)->tbuf), (&tstci_adjacent), slist_cb_comp_heter_tstilecomb_hastile, &idx) < 0) {
                    continue;
                }
                break;
            }
            if (j < maxdir) {
                // have adjacent tile
                // store it
                slist_store (ppotposlist, &(tstci_chk.pos));
            } else {
                // remove it
                if (slist_find (ppotposlist, &(tstci_chk.pos), NULL, &idx) >= 0) {
                    slist_rmdata (ppotposlist, idx, NULL);
                }
            }
        }
        memmove (&tstci_chk, &tstci_cur, sizeof (tstci_chk));
        tspos_step2dir_atam (&(tstci_chk.pos), i);
    }
    return 0;
}

// idx_roff: the tile to be removed is at (proffblist[idx_roff].pos)
static int
remove_tile (tssiminfo_t *ptsim, sortedlist_t *ppotposlist, sortedlist_t *proffblist, size_t idx_roff)
{
    size_t idx;
    tstilecombitem_t tstci_cur;
    ts_roffbidx_t roffbidx;

    slist_data (proffblist, idx_roff, &roffbidx);

    // remove the tile from ptsim->ptarget
    memset (&tstci_cur, 0, sizeof (tstci_cur));
    memmove (&(tstci_cur.pos), &(roffbidx.pos), sizeof (roffbidx.pos));
    if (slist_find (&(ptsim->ptarget->tbuf), (&tstci_cur), slist_cb_comp_heter_tstilecomb_hastile, &idx) < 0) {
        return -1;
    }
    // check if the tile is SEED?
    slist_rmdata (&(ptsim->ptarget->tbuf), idx, &tstci_cur);
    if (0 == strcmp (ptsim->ptilevec[tstci_cur.idtile].name, "SEED")) {
        tstc_additem (ptsim->ptarget, &tstci_cur);
        // it's the seed
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Try to remove SEED!");
        return -1;
    }

    // remove the tile.pos from proffblist
    slist_rmdata (proffblist, idx_roff, NULL);

    // add this site to ppotposlist, remove others?
    return update_potential_position_after_remove_tile (ptsim, ppotposlist, &(roffbidx.pos));
}

// place one tile at the position ppos
// return 0 if OK, return < 0 if failed.
static int
place_tile_at (tssiminfo_t *ptsim, sortedlist_t *ppotposlist, sortedlist_t *proffblist, size_t idx_tile, tsposition_t *ppos)
{
    tstilecombitem_t tstci;
    size_t idx_pp;
    tstilecombitem_t tstci_cur;

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

    memset (&tstci, 0, sizeof (tstci));
    memmove (&(tstci.pos), ppos, sizeof (*ppos));
    tstci.idtile = idx_tile;
    tstc_additem (ptsim->ptarget, &tstci);

    // remove the pos from potposlist
    slist_rmdata (ppotposlist, idx_pp, NULL);

    // check the potential positions around the one added.
    if (0 == slist_find (&((ptsim->ptarget)->tbuf), &(tstci), slist_cb_comp_heter_tstilecomb_hastile, &idx_pp)) {
        find_potential_position_adjacent2tile_ktam (ptsim, ptsim->ptarget, idx_pp, ppotposlist);
    }

    tstci_cur.idtile = idx_tile;
    memmove (&(tstci_cur.pos), ppos, sizeof (*ppos));
    calculate_roffb_ktam (ptsim, &tstci_cur, proffblist);

    // record this step?
    return 0;
}

int
tilesim_ktam (tssiminfo_t *ptsim, const char *name_sim)
{
    int ret = 0;
    size_t idx;
    size_t i;
    void *hashref;
    size_t cnt_fail;
    tsposition_t pos;
    memarr_t lst_place;
    tstilecombitem_t *ptstci;
    tstilecombitem_t tstci;
    tstilecombitem_t tstci_cur;
    char flg_idx_sequence = 0;
    sortedlist_t potposlist;  // tsposition_t: the list of all of the positions which the tile could attached to. Potential positions
    sortedlist_t roffblist;   // ts_roffbidx_t; the r_{off,b} value for each tile in the supertile
    ts_roffbidx_t *ptsroi;
    double rand_val;
    double ron_all;
    double roff_all;
    double ron;
#if DEBUG
    ts_roffbidx_t rofbidx_tmp;
#endif

    if (NULL == ptsim) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "NULL parameter!");
        return -1;
    }
    if (NULL == ptsim->ptarget) {
        ptsim->ptarget = tstc_create ();
        if (NULL == ptsim->ptarget) {
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "tstc_create() error");
            return -1;
        }
    }
    if (TSTC_NUM_TILE (ptsim->ptarget) < 1) {
        // place the seed to the supertile
        // find the seed tile
        for (i = 0; i < ptsim->num_tilevec; i ++) {
            if ((NULL != ptsim->ptilevec[i].name) && (0 == strcmp ("SEED", ptsim->ptilevec[i].name))) {
                break;
            }
        }
        if (i >= ptsim->num_tilevec) {
            // not found
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Not found SEED tile!");
            return -1;
        }
        memset (&tstci, 0, sizeof (tstci));
        tstci.idtile = i;
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
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "hashref init error");
        return -1;
    }
    if (slist_init (&roffblist, sizeof (ts_roffbidx_t), NULL, slist_cb_comp_roffbidx, slist_cb_swap_roffbidx) < 0) {
        hash_revert_clear (hashref);
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "roffblist init error");
        return -1;
    }
    if (slist_init (&potposlist, sizeof (tsposition_t), NULL, slist_cb_comp_tspos, slist_cb_swap_tspos) < 0) {
        hash_revert_clear (hashref);
        slist_clear (&roffblist, NULL);
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "potposlist init error");
        return -1;
    }
    ron = ptsim->ratek * exp (0.0 - ptsim->Gmc);//r_{on} = k_a  \times e^{-G_{mc}}

    // re-calculate the potential positions
    slist_rmalldata (&roffblist, NULL);
    slist_rmalldata (&potposlist, NULL);
    assert (TSTC_NUM_TILE (ptsim->ptarget) > 0);
    for (i = 0; i < TSTC_NUM_TILE (ptsim->ptarget); i ++) {
        // for each of the positions adjacent to it
        find_potential_position_adjacent2tile_ktam (ptsim, ptsim->ptarget, i, &potposlist);
        // calculate the r_{off,b} of each tile
        tstc_data (ptsim->ptarget, i, &tstci_cur);
        calculate_roffb_ktam (ptsim, &tstci_cur, &roffblist);
    }
    if (slist_size (&potposlist) < 1) {
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "can not get blank positions.");
        ret = 0;
        goto end_tilesim_atam;
    }

#if DEBUG
    assert (slist_size (&roffblist) > 0);
    assert (slist_size (&potposlist) > 0);
    roffbidx_dump (&roffblist);
    potpos_dump (&potposlist);
#endif // DEBUG

    for (; slist_size (&roffblist) > 0;) {
        assert (slist_size (&potposlist) > 0);
        ron_all = ron * (double)slist_size (&potposlist);
        roff_all = 0.0;
        for (i = 0; i < slist_size (&roffblist); i ++) {
            ptsroi = (ts_roffbidx_t *) slist_data_lock (&roffblist, i);
            roff_all = roff_all + ptsroi->roffb;
            slist_data_unlock (&roffblist, i);
        }
        roff_all = my_drrand (ron_all + roff_all);
        if (ron_all <= roff_all) {
            // use r_{off,b}
            // check the tile to be removed.
            //ron_all = 0.0;
            for (i = 0; i < slist_size (&roffblist); i ++) {
                ptsroi = (ts_roffbidx_t *) slist_data_lock (&roffblist, i);
                ron_all = ron_all + ptsroi->roffb;
#if DEBUG
                memmove (&rofbidx_tmp, ptsroi, sizeof (rofbidx_tmp));
#endif
                slist_data_unlock (&roffblist, i);
                if (ron_all >= roff_all) {
                    break;
                }
            }
            if (i >= slist_size (&roffblist)) {
                i = my_irrand (slist_size (&roffblist));
#if DEBUG
                slist_data (&roffblist, i, &rofbidx_tmp);
#endif
            }
            remove_tile (ptsim, &potposlist, &roffblist, i);
#if DEBUG
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "----- remove tile %d: ", i);
            slist_cb_iter_roffbidx (NULL, i, &rofbidx_tmp); printf ("\n");
            roffbidx_dump (&roffblist);
            potpos_dump (&potposlist);
#endif
        } else {
            // use r_{on}
            // select one position
            idx = my_irrand (slist_size (&potposlist));
            slist_data (&potposlist, idx, &pos);
            do {
                idx = my_irrand (ptsim->num_tilevec);
                assert (idx < ptsim->num_tilevec);
                idx = hash_revert (hashref, idx, ptsim->num_tilevec - 1);
            } while (0 == strcmp (ptsim->ptilevec[idx].name, "SEED"));

            if (place_tile_at (ptsim, &potposlist, &roffblist, idx, &pos) < 0) {
                // error
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "unable to place tile");
                break;
            }
#if DEBUG
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "+++++ add tile %d: ", idx);
            roffbidx_dump (&roffblist);
            potpos_dump (&potposlist);
#endif
            // TODO: notify the caller
            if (ptsim->cb_resultinfo) {
                memset (&tstci, 0, sizeof (tstci));
                memmove (&(tstci.pos), &pos, sizeof (pos));
                //tstci.idtile = i;
#if USE_THREEDIMENTION
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "tile[%d] @ (%d,%d,%d)rot[%d]", idx, tstci.pos.x, tstci.pos.y, tstci.pos.z, tstci.rotnum);
#else
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "tile[%d] @ (%d,%d)rot[%d]", idx, tstci.pos.x, tstci.pos.y, tstci.rotnum);
#endif
                if (ptsim->cb_resultinfo (ptsim->userdata, ptsim->num_tilevec, idx, &tstci, ptsim->temperature, ptsim->num_tilevec, ptsim->ptarget, NULL) < 0) {
                    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "break by caller's cb_resultinfo");
                    break;
                }
            }
        }
    }

end_tilesim_atam:
    hash_revert_clear (hashref);
    slist_clear (&roffblist, NULL);
    slist_clear (&potposlist, NULL);
    return ret;
}

#endif /* USE_TILESIM_ATAM */
