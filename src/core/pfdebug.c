/******************************************************************************
 * Name:        pfdebug.c
 * Purpose:     debug functions
 * Author:      Yunhui Fu
 * Created:     2009-08-22
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <stdio.h>
#include "pfdebug.h"

#ifdef DEBUG
int g_debug_level = PFDBG_LEVEL_LOG;
int g_debug_catlog = PFDBG_CATLOG_APP;

void
set_debug_level (int level)
{
    if (PFDBG_LEVEL_ALL == level) {
        level = PFDBG_LEVEL_LOG;
    }
    g_debug_level = level;
}

void
set_debug_catlog (int catlog)
{
    g_debug_catlog = catlog;
}

typedef struct _dbg_val_info_t {
    int val;
    const char *desc;
} dbg_val_info_t;

#define DBG_VAL_PAIR_CATLOG(name) {PFDBG_CATLOG_##name, #name}
dbg_val_info_t g_dbg_catlog_info[] = {
    DBG_VAL_PAIR_CATLOG(ALL),
    DBG_VAL_PAIR_CATLOG(APP),
    DBG_VAL_PAIR_CATLOG(3DBASE),
    DBG_VAL_PAIR_CATLOG(TSTC),
    DBG_VAL_PAIR_CATLOG(TSSIM),
};

#define DBG_VAL_PAIR_LEVEL(name) {PFDBG_LEVEL_##name, #name}
dbg_val_info_t g_dbg_level_info[] = {
    DBG_VAL_PAIR_LEVEL(ALL),
    DBG_VAL_PAIR_LEVEL(ERROR),
    DBG_VAL_PAIR_LEVEL(WARNING),
    DBG_VAL_PAIR_LEVEL(INFO),
    DBG_VAL_PAIR_LEVEL(DEBUG),
    DBG_VAL_PAIR_LEVEL(LOG),
};

void
show_all_dbgval (FILE * fpout, dbg_val_info_t *list, size_t num, const char *line_prefix)
{
    size_t i;
    for (i = 0; i < num; i ++) {
        fprintf (fpout, "%s%d - %s\n", line_prefix, g_dbg_catlog_info[i].val, g_dbg_catlog_info[i].desc);
    }
}

void
show_all_dbgcatlog (FILE *fpout, const char *line_prefix)
{
    show_all_dbgval (fpout, g_dbg_catlog_info, (sizeof(g_dbg_catlog_info)/sizeof(g_dbg_catlog_info[0])), line_prefix);
}

void
show_all_dbglevel (FILE *fpout, const char *line_prefix)
{
    show_all_dbgval (fpout, g_dbg_level_info, (sizeof(g_dbg_level_info)/sizeof(g_dbg_level_info[0])), line_prefix);
}

#endif
