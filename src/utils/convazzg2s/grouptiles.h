/******************************************************************************
 * Name:        grouptiles.h
 * Purpose:     categorize the tiles of arbitrary zig-zag tile set.
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GROUP_TILES_H
#define _GROUP_TILES_H

#include <stddef.h> // size_t

#include "zzinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int zzi_group_tiles (zigzag_info_t *pzzi, const char *name, size_t max_steps);

int test_grouptiles (zigzag_info_t *pzzi, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _GROUP_TILES_H */
