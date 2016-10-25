/******************************************************************************
 * Name:        grouptiles.c
 * Purpose:     categorize the tiles of arbitrary zig-zag tile set.
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

#include "tilesimatam.h"
#include "zzconv.h"
#include "grouptiles.h"

#if DEBUG
#define CHK_TSIM_GLUEVEC(ptsim) \
    for (i = 0; i < (ptsim)->num_gluevec; i ++) { \
        assert (ptsim->pgluevec[i] < 3); \
    }
#else
#define CHK_TSIM_GLUEVEC(ptsim)
#endif

// load the data from zigzag_info_t to tssiminfo_t
int
data_tran_zzi2tsim (tssiminfo_t *ptsim, zigzag_info_t *pzzi)
{
    size_t i;

    tssiminfo_reset (ptsim);

    ptsim->pgluevec = (size_t *) malloc (sizeof (size_t) * pzzi->sz_glues);
    assert (NULL != ptsim->pgluevec);
    ptsim->num_gluevec = pzzi->sz_glues;
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        assert (pzzi->glues[i].strength < 3);
        ptsim->pgluevec[i] = pzzi->glues[i].strength;
    }
    CHK_TSIM_GLUEVEC(ptsim);
    ptsim->num_tilevec = pzzi->sz_tiles;
    ptsim->ptilevec = (tstile_t *) malloc (ptsim->num_tilevec * sizeof (tstile_t));
    assert (NULL != ptsim->ptilevec);
    memset (ptsim->ptilevec, 0, sizeof (ptsim->num_tilevec * sizeof (tstile_t)));
    for (i = 0; i < ptsim->num_tilevec; i ++) {
#if 0
        assert (0 == ptsim->ptilevec[i].glues[ORI_NORTH]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_EAST]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_SOUTH]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_WEST]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_FRONT]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_BACK]);
#endif
        ptsim->ptilevec[i].name = NULL;
        ptsim->ptilevec[i].label = NULL;
        if ((NULL != pzzi->tiles[i].name) && (strlen (pzzi->tiles[i].name) > 0)) {
            ptsim->ptilevec[i].name = strdup (pzzi->tiles[i].name);
        }
        if ((NULL != pzzi->tiles[i].label) && (strlen (pzzi->tiles[i].label) > 0)) {
            ptsim->ptilevec[i].label = strdup (pzzi->tiles[i].label);
        }
        ptsim->ptilevec[i].glues[ORI_NORTH] = gluearr_name2idx (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gN);
        ptsim->ptilevec[i].glues[ORI_EAST]  = gluearr_name2idx (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gE);
        ptsim->ptilevec[i].glues[ORI_SOUTH] = gluearr_name2idx (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gS);
        ptsim->ptilevec[i].glues[ORI_WEST]  = gluearr_name2idx (pzzi->glues, pzzi->sz_glues, pzzi->tiles[i].gW);
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_NORTH]) && (ptsim->ptilevec[i].glues[ORI_NORTH] < pzzi->sz_glues));
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_EAST])  && (ptsim->ptilevec[i].glues[ORI_EAST] < pzzi->sz_glues));
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_SOUTH]) && (ptsim->ptilevec[i].glues[ORI_SOUTH] < pzzi->sz_glues));
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_WEST])  && (ptsim->ptilevec[i].glues[ORI_WEST] < pzzi->sz_glues));
#if USE_THREEDIMENTION
        ptsim->ptilevec[i].glues[ORI_FRONT] = 0;
        ptsim->ptilevec[i].glues[ORI_BACK] = 0;
        assert (0 == ptsim->ptilevec[i].glues[ORI_FRONT]);
        assert (0 == ptsim->ptilevec[i].glues[ORI_BACK]);
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_FRONT]) && (ptsim->ptilevec[i].glues[ORI_FRONT] < pzzi->sz_glues));
        //assert ((0 <= ptsim->ptilevec[i].glues[ORI_BACK])  && (ptsim->ptilevec[i].glues[ORI_BACK] < pzzi->sz_glues));
#endif
    }
    ptsim->flg_is2d = 1;
    ptsim->flg_norotate = 1;
    ptsim->temperature = pzzi->temperature;
    TSIM_SIM_ALGO (ptsim, TSIM_ALGO_ATAM);

    CHK_TSIM_GLUEVEC (ptsim);
    return 0;
}

// return ORI_MAX if not adjacent
// return one of ORI_NORTH, ORI_EAST, ORI_SOUTH, ORI_WEST, ORI_FRONT, ORI_BACK
int
pos_which_adjacent (tsposition_t *ppos_base, tsposition_t *ppos_which)
{
    if (ppos_base->x != ppos_which->x) {
        if ((ppos_base->y != ppos_which->y)
#if USE_THREEDIMENTION
            || (ppos_base->z != ppos_which->z)
#endif
            ) {
            return ORI_MAX;
        }
        if (ppos_base->x + 1 == ppos_which->x) {
            return ORI_EAST;
        } else if (ppos_base->x == ppos_which->x + 1) {
            return ORI_WEST;
        }
        return ORI_MAX;
    }
    if (ppos_base->y != ppos_which->y) {
#if USE_THREEDIMENTION
        if (ppos_base->z != ppos_which->z) {
            return ORI_MAX;
        }
#endif
        if (ppos_base->y + 1 == ppos_which->y) {
            return ORI_NORTH;
        } else if (ppos_base->y == ppos_which->y + 1) {
            return ORI_SOUTH;
        }
        return ORI_MAX;
    }
#if USE_THREEDIMENTION
    if (ppos_base->z + 1 == ppos_which->z) {
        return ORI_BACK;
    } else if (ppos_base->z == ppos_which->z + 1) {
        return ORI_FRONT;
    }
#endif
    return ORI_MAX;
}

// the opposite direction
static const int m_opp_dir[ORI_MAX] = {
    /*ORI_NORTH*/ ORI_SOUTH,
    /*ORI_EAST*/  ORI_WEST,
    /*ORI_SOUTH*/ ORI_NORTH,
    /*ORI_WEST*/  ORI_EAST,
#if USE_THREEDIMENTION
    /*ORI_FRONT*/ ORI_BACK,
    /*ORI_BACK*/  ORI_FRONT,
#endif
};

typedef struct _zzidect_t {
    zigzag_info_t *pzzi;
    tssiminfo_t *ptsim;
    size_t max_steps; // the max steps to be simulated, 0: endless
    memarr_t steps; // list of tstilecombitem_t, each of the steps
    size_t num_known; // the number of the tiles which has known types
    tsposition_t pos_max; // the max value of x,y,z
    tsposition_t pos_min; // the min value of x,y,z
} zzidect_t;

#define SET_TILE_TYPE(pzzid, idx, newtype) \
    if ((TILETYPE_UNKNOWN < (pzzid)->pzzi->tiles[idx].type) && ((pzzid)->pzzi->tiles[idx].type < TILETYPE_MAX)) { \
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "The tile type is known, skip. oldtype=(%d)'%s', newtype=(%d)'%s'", (pzzid)->pzzi->tiles[idx].type, tiletype_val2cstr((pzzid)->pzzi->tiles[idx].type), newtype, tiletype_val2cstr(newtype));\
    } else { \
        (pzzid)->pzzi->tiles[idx].type = newtype; \
        (pzzid)->num_known ++; \
    }

int
tssim_cb_resultinfo_zzi_detection (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    int dir;
    size_t i;
    size_t idx_pre;
    tstilecombitem_t tstci;
    tstilecombitem_t *ptstci;
    zzidect_t *pzzid;

    pzzid = (zzidect_t *)userdata;
    assert (NULL != pzzid);
    assert (NULL != pstpos);
    CHK_TSIM_GLUEVEC(pzzid->ptsim);

    if (0 != pzzid->max_steps) {
        if (pzzid->max_steps < ma_size (&(pzzid->steps))) {
            // exceed the max steps
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Exceed max steps");
            return -1;
        }
#if DEBUG
        printf ("steps: %d/%d\n", ma_size (&(pzzid->steps)), pzzid->max_steps);
    } else {
        printf ("steps: %d/(infinite)\n", ma_size (&(pzzid->steps)));
#endif
    }
    // the idx_test is the index of the tile of ptsim->ptilevec
    memset (&tstci, 0, sizeof (tstilecombitem_t));
    memmove (&(tstci.pos), &(pstpos->pos), sizeof (tstci.pos));
    tstci.idtile = idx_test;
    ma_append (&(pzzid->steps), &tstci);

    //printf ("pos (%d,%d,%d): ", pstpos->pos.x, pstpos->pos.y, pstpos->pos.z);
    //printf ("max (%d,%d,%d)=>", pzzid->pos_max.x, pzzid->pos_max.y, pzzid->pos_max.z);
    UPDATE_MAX_VAL (pzzid->pos_max, pstpos->pos);
    //printf ("(%d,%d,%d)\n", pzzid->pos_max.x, pzzid->pos_max.y, pzzid->pos_max.z);
    UPDATE_MIN_VAL (pzzid->pos_min, pstpos->pos);

    // check if this tile has known type.
    if ((TILETYPE_UNKNOWN < pzzid->pzzi->tiles[tstci.idtile].type) && (pzzid->pzzi->tiles[tstci.idtile].type < TILETYPE_MAX)) {
        if (pzzid->num_known >= pzzid->pzzi->sz_tiles) {
            // all of the tiles' type are known, so can safe to quit the testing
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "DONE! All of tile is known, so safe quit");
            return -1;
        }
        //DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Known type: [id=%d] %d", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].type);
        return 0;
    }
    CHK_TSIM_GLUEVEC(pzzid->ptsim);

    pzzid->pzzi->idx_last = idx_test;

    if (ma_size (&(pzzid->steps)) < 2) {
        // the first one
        assert (ma_size (&(pzzid->steps)) == 1);
        if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] > 1) {
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] < 2);
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] < 2);
            SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DWN2);
        } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] > 1) {
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] < 2);
            SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_FE);
        } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] > 1) {
            assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_FW);
        } else {
            //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] > 1);
            SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TWE1);
        }
        assert ((NULL != pzzid) && (NULL != pzzid->pzzi) && (NULL != pzzid->pzzi->tiles) && (tstci.idtile < pzzid->pzzi->sz_tiles) && (NULL != pzzid->pzzi->tiles[tstci.idtile].name));
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
        memmove (&(pzzid->pos_max), &(tstci.pos), sizeof (pzzid->pos_max));
        memmove (&(pzzid->pos_min), &(tstci.pos), sizeof (pzzid->pos_min));
        return 0;
    }

    // search the tile that is adjacent to this tile
    for (i = ma_size (&(pzzid->steps)) - 1; i > 0; i --) {
        ptstci = (tstilecombitem_t *) ma_data_lock (&(pzzid->steps), i - 1);
        assert (NULL != ptstci);
        idx_pre = ptstci->idtile;
        dir = pos_which_adjacent (&(tstci.pos), &(ptstci->pos));
        if ((0 < dir) && (dir < ORI_MAX)) {
            // if two tiles are attached each other
            if (pzzid->ptsim->ptilevec[tstci.idtile].glues[dir] == pzzid->ptsim->ptilevec[ptstci->idtile].glues[m_opp_dir[dir]]) {
                ma_data_unlock (&(pzzid->steps), i - 1);
                break;
            }
        }
        ma_data_unlock (&(pzzid->steps), i - 1);
    }
    if (i < 1) {
        // not found
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "Not found previous tile for tile[%d] '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name);
        return 0;
    }
    // found
    switch (dir) {
    case ORI_EAST:
        // DWW1, DWN2, SWN2, FW, DWW2
        // check the glue strength
        //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[dir]] > 0);
        if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[dir]] > 1) {
            // SWN2, FW
            // check the glue strength at the North and West
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_SWN2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
            } else {
                //                          assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] > 1);
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_FW);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
            }
        } else {
            // DWW1, DWN2, DWW2
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DWN2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] < 2);
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DWW2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            } else {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DWW1);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            }
        }
        break;
    case ORI_SOUTH:
        // TEW1, TWE1, TWW2, TEE2, TEW2, TWE2
        //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] > 1);
        // should check the previous tile
        assert (pzzid->ptsim->ptilevec[idx_pre].glues[ORI_NORTH] == pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]);
        //assert ((TILETYPE_UNKNOWN < pzzid->pzzi->tiles[idx_pre].type) && (pzzid->pzzi->tiles[idx_pre].type < TILETYPE_MAX));
        switch (pzzid->pzzi->tiles[idx_pre].type) {
        case TILETYPE_DEN2:
        case TILETYPE_SEN2:
            //TEW1;TEE2;TEW2
            // check current tile's E,W strength
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TEE2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] < 2);
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TEW2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            } else {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TEW1);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            }
            break;
        case TILETYPE_DWN2:
        case TILETYPE_SWN2:
            //TWE1;TWW2;TWE2
            // check current tile's E,W strength
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TWE2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] < 2);
                assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_WEST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TWW2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            } else {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_TWE1);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] < 2);
            }
            break;
        default:
            DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "Un-handled type: %d '%s'", pzzid->pzzi->tiles[idx_pre].type, (tiletype_val2cstr(pzzid->pzzi->tiles[idx_pre].type)?tiletype_val2cstr(pzzid->pzzi->tiles[idx_pre].type):""));
            break;
        }
        break;
    case ORI_WEST:
        // DEE1, DEN2, SEN2, FE, DEE2, 
        // check the glue strength
        //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[dir]] > 0);
        if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[dir]] > 1) {
            // SEN2, FE
            // check the glue strength at the North and West
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_SEN2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
            } else {
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] > 1);
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_FE);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
            }
        } else {
            // DEE1, DEN2, DEE2
            if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_NORTH]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DEN2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] < 2);
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            } else if (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_EAST]] > 1) {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DEE2);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            } else {
                SET_TILE_TYPE (pzzid, tstci.idtile, TILETYPE_DEE1);
                DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "Detected: tile[%d] '%s' is type '%s'", tstci.idtile, pzzid->pzzi->tiles[tstci.idtile].name, tiletype_val2cstr(pzzid->pzzi->tiles[tstci.idtile].type));
                //assert (pzzid->ptsim->pgluevec[pzzid->ptsim->ptilevec[tstci.idtile].glues[ORI_SOUTH]] < 2);
            }
        }
        break;
    case ORI_NORTH:
        // imposible?
#if 0
    case ORI_FRONT:
    case ORI_BACK:
#endif
    default:
        // error
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "ERROR in detect tile(%d)", tstci.idtile);
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_ERROR, "dir=%d; i=%d", dir, i);
        return -1;
        break;
    }
    if (pzzid->num_known >= pzzid->pzzi->sz_tiles) {
        // all of the tiles' type are known, so can safe to quit the testing
        DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_WARNING, "DONE! All of tile is known, so safe quit");
        return -1;
    }
    return 0;
}

// pbczz: (IN) the glue list and tile list, the item of the tiles have a invalid value of type
//        (OUT) the type value is set in the pbczz->tiles[i]
int
zzi_group_tiles (zigzag_info_t *pzzi, const char *name, size_t max_steps)
{
    zzidect_t zzid;
    tssiminfo_t tsim;
    tssiminfo_t *ptsim = &tsim;
    tssiminfo_init (ptsim);

    data_tran_zzi2tsim (ptsim, pzzi);

    memset (&zzid, 0, sizeof (zzid));
    ma_init (&(zzid.steps), sizeof (tstilecombitem_t));
    zzid.pzzi = pzzi;
    zzid.ptsim = ptsim;
    zzid.max_steps = max_steps;
    zzid.num_known = 0;
    //ma_rmalldata (&(zzid.steps), NULL);
    ptsim->userdata = &zzid;
    ptsim->cb_resultinfo = tssim_cb_resultinfo_zzi_detection;

    tilesim_atam (ptsim, name);

    //printf ("max(x,y,z)=(%d,%d,%d)\n", zzid.pos_max.x, zzid.pos_max.y, zzid.pos_max.z);
    //printf ("min(x,y,z)=(%d,%d,%d)\n", zzid.pos_min.x, zzid.pos_min.y, zzid.pos_min.z);
    pzzi->pos_max_x = zzid.pos_max.x - zzid.pos_min.x + 1;
    pzzi->pos_max_y = zzid.pos_max.y - zzid.pos_min.y + 1;
    assert (zzid.pos_min.x <= 0);
    pzzi->pos_seed_x = 0 - zzid.pos_min.x;
    pzzi->tiles_filled = ma_size (&(zzid.steps));
    //printf ("pos_max_x=%d; pos_max_y=%d; pos_seed_x=%d\n", pzzi->pos_max_x, pzzi->pos_max_y, pzzi->pos_seed_x);
    if (ma_size (&(zzid.steps)) > 1) {
        tstilecombitem_t *ptsci;
        size_t idx_steplast;
        size_t i;

        ptsci = (tstilecombitem_t *) ma_data_lock (&(zzid.steps), ma_size (&(zzid.steps)) - 1);
        idx_steplast = ptsci->idtile;
        // try to get the last tile id
        for (i = 0; i < ma_size (&(zzid.steps)); i ++) {
            ptsci = (tstilecombitem_t *) ma_data_lock (&(zzid.steps), i);
            assert (NULL != ptsci);
            if (ptsci->idtile == pzzi->idx_last) {
                if (idx_steplast > ptsci->idtile) {
                    pzzi->idx_last = idx_steplast;
                }
                ma_data_unlock (&(zzid.steps), i);
                break;
            }
            if (ptsci->idtile == idx_steplast) {
                ma_data_unlock (&(zzid.steps), i);
                break;
            }
            ma_data_unlock (&(zzid.steps), i);
        }
    }

    //printf ("pos_max_x=%d; pos_max_y=%d; pos_seed_x=%d; idx_last=%d\n", pzzi->pos_max_x, pzzi->pos_max_y, pzzi->pos_seed_x, pzzi->idx_last);

    ma_clear (&(zzid.steps), NULL);
    tssiminfo_clear (ptsim);
}

// pzzi: the tiles with the correct type information
// return 0 on OK, <0 on error
int
test_grouptiles (zigzag_info_t *pzzi, const char *name)
{
    int ret = 0;
    size_t i;
    zigzag_info_t zzi_dup;

    // duplicate the data
    memmove (&zzi_dup, pzzi, sizeof (zzi_dup));
    zzi_dup.tiles = NULL;
    zzi_dup.tiles = (tile_t *) malloc (sizeof (tile_t) * zzi_dup.sz_tiles);
    assert (NULL != zzi_dup.tiles);
    memmove (zzi_dup.tiles, pzzi->tiles, sizeof (tile_t) * zzi_dup.sz_tiles);
    for (i = 0; i < zzi_dup.sz_tiles; i ++) {
        zzi_dup.tiles[i].type = TILETYPE_UNKNOWN;
    }

    DBGMSG (PFDBG_CATLOG_TSSIM, PFDBG_LEVEL_DEBUG, "zzi_group_tiles() ...");
    zzi_group_tiles (&zzi_dup, name, 1000);

    // check the result
    ret = zzi_comapre (pzzi, &zzi_dup, 1);
    free (zzi_dup.tiles);
    return ret;
}
