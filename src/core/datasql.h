/******************************************************************************
 * Name:        datasql.h
 * Purpose:     store the simulation data into database
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _DATA_SQL_H
#define _DATA_SQL_H

#include <dbi/dbi.h>
#include "tilestruct.h"

#if USE_DBRECORD
_PSF_BEGIN_EXTERN_C

extern int tsim_dbi_init (void);
extern void tsim_dbi_shutdown (void);
extern void tsim_dbi_disconn (dbi_conn conn);

extern int tsim_save_baseinfo_dbi (dbi_conn conn, const char *name, tssiminfo_t *ptsim);
extern int tsim_load_baseinfo_dbi (dbi_conn conn, const char *name, tssiminfo_t *ptsim);

typedef struct _dbmergeinfo_t {
    size_t id; // id of the bucket
    void *conn; /* dbi_conn * */
} dbmergeinfo_t;

extern int tssim_cb_findmergepos_dbi (void *pdbmi, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok);

extern int tssim_cb_storemergepos_dbi (void *pdbmi, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok);

extern int tssim_cb_resultinfo_record_history_dbi (void *pdbmi, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok);

// the callback function for the tsim_fetch_history_steps()
// id: the index of history information item in the database
// idpstbase: the id of the base supertile.
// idpsttest: the id of the test supertile.
// rotnum: the rotate number of the test supertile
// x, y, z: the position of the test supertile
typedef int (* tsim_cb_getitem_histstep_t) (void * userdata, size_t id, size_t idpstbase, size_t idpsttest, size_t rotnum, size_t x, size_t y, size_t z);
extern int tsim_fetch_history_steps (void *pdbmi, size_t idx, size_t cnt, tsim_cb_getitem_histstep_t cb, void *userdata);

// the callback function for the tsim_fetch_history_steps()
// id: the index of history information item in the database
// idpstbase: the id of the base supertile.
// idpsttest: the id of the test supertile.
// rotnum: the rotate number of the test supertile
// x, y, z: the position of the test supertile
// times: the times of the same two type supertile combined
typedef int (* tsim_cb_getitem_histparents_t) (void * userdata, size_t id, size_t idpstbase, size_t idpsttest, size_t rotnum, size_t x, size_t y, size_t z, size_t times);
extern int tsim_fetch_history_parents (void * pdbmi, size_t idstile, size_t idx, size_t cnt, tsim_cb_getitem_histparents_t cb, void *userdata);

_PSF_END_EXTERN_C
#endif /* USE_DBRECORD */
#endif /* _DATA_SQL_H */
