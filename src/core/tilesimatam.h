/******************************************************************************
 * Name:        tilesimatam.h
 * Purpose:     simulator for aTAM tile set self-assembly
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _TILE_SIMULATION_ATAM_H
#define _TILE_SIMULATION_ATAM_H

#include "pfutils.h"
#include "tilestruct.h"

_PSF_BEGIN_EXTERN_C

int tilesim_atam (tssiminfo_t *ptsim, const char *name_sim);

int tspos_step2dir_atam (tsposition_t *ptp, int dir);
int tstc_nomalize_atam (tstilecomb_t *ptc);
size_t ts_atam_chk_strength (tssiminfo_t *ptsim, size_t idx_tile, tsposition_t *ppos);

extern const int g_tile_glue_opp_direction[ORI_MAX];

_PSF_END_EXTERN_C
#endif /* _TILE_SIMULATION_ATAM_H */
