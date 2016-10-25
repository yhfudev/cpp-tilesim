/******************************************************************************
 * Name:        pfdebug.h
 * Purpose:     debug functions
 * Author:      Yunhui Fu
 * Created:     2009-08-22
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef PFDEBUG_H
#define PFDEBUG_H

#define PFDBG_LEVEL_ALL     0
#define PFDBG_LEVEL_ERROR   1
#define PFDBG_LEVEL_WARNING 2
#define PFDBG_LEVEL_INFO    3
#define PFDBG_LEVEL_DEBUG   4
#define PFDBG_LEVEL_LOG     5

#define PFDBG_CATLOG_ALL    0
#define PFDBG_CATLOG_APP    1
#define PFDBG_CATLOG_USERBEGIN 2

#define PFDBG_CATLOG_3DBASE (PFDBG_CATLOG_USERBEGIN + 0)
#define PFDBG_CATLOG_TSTC   (PFDBG_CATLOG_USERBEGIN + 1) /* tstilecomb_t */
#define PFDBG_CATLOG_TSMESH (PFDBG_CATLOG_USERBEGIN + 2)
#define PFDBG_CATLOG_TSSIM  (PFDBG_CATLOG_USERBEGIN + 3)
#define PFDBG_CATLOG_XMLDATA  (PFDBG_CATLOG_USERBEGIN + 4)
#define PFDBG_CATLOG_OPENGL   (PFDBG_CATLOG_USERBEGIN + 5)
#define PFDBG_CATLOG_TESTCASE (PFDBG_CATLOG_USERBEGIN + 6)

#include <stdio.h>

#ifdef DEBUG
extern int g_debug_level;
extern int g_debug_catlog;

extern void set_debug_level(int level);
extern void set_debug_catlog(int catlog);

extern void show_all_dbgcatlog (FILE *fpout, const char *line_prefix);
extern void show_all_dbglevel (FILE *fpout, const char *line_prefix);

#define DBGMSG(catlog, level, ...) \
    if ((level <= PFDBG_LEVEL_WARNING) || ((g_debug_catlog == PFDBG_CATLOG_ALL || g_debug_catlog == catlog) && (level <= g_debug_level))) { \
        fprintf(stderr, "[%s()]\t", __func__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\t{%d," __FILE__ "}\n", __LINE__); \
    }

#define DEBUG_CODE(code) {code}
#else

#define DBGOUT_PFDBG_LEVEL_ALL(...) fprintf(stderr, __VA_ARGS__),fprintf(stderr, "\n")
#define DBGOUT_PFDBG_LEVEL_ERROR   DBGOUT_PFDBG_LEVEL_ALL
#define DBGOUT_PFDBG_LEVEL_WARNING DBGOUT_PFDBG_LEVEL_ALL
#define DBGOUT_PFDBG_LEVEL_INFO(...)
#define DBGOUT_PFDBG_LEVEL_DEBUG(...)
#define DBGOUT_PFDBG_LEVEL_LOG(...)

#define DBGOUT_0 DBGOUT_PFDBG_LEVEL_ALL
#define DBGOUT_1 DBGOUT_PFDBG_LEVEL_ERROR
#define DBGOUT_2 DBGOUT_PFDBG_LEVEL_WARNING
#define DBGOUT_3 DBGOUT_PFDBG_LEVEL_INFO
#define DBGOUT_4 DBGOUT_PFDBG_LEVEL_DEBUG
#define DBGOUT_5 DBGOUT_PFDBG_LEVEL_LOG

#define DBGMSG(catlog, level, ...) DBGOUT_##level (__VA_ARGS__)

#define set_debug_level(level)
#define set_debug_catlog(catlog)

#define DEBUG_CODE(code)
#endif

#endif /* PFDEBUG_H */
