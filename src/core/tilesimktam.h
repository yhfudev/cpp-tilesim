/******************************************************************************
 * Name:        tilesimktam.h
 * Purpose:     simulator for kTAM tile set self-assembly
 * Author:      Yunhui Fu
 * Created:     2009-11-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _TILE_SIMULATION_KTAM_H
#define _TILE_SIMULATION_KTAM_H

#include "pfutils.h"
#include "tilestruct.h"

_PSF_BEGIN_EXTERN_C

int tilesim_ktam (tssiminfo_t *ptsim, const char *name_sim);

_PSF_END_EXTERN_C
#endif /* _TILE_SIMULATION_KTAM_H */
