/******************************************************************************
 * Name:        datasql.c
 * Purpose:     store the simulation data into database
 * Author:      Yunhui Fu
 * Created:     2008-09-18
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#if USE_DBRECORD
#include <stdio.h>
#include <string.h> // memset()
#include <assert.h>
#include <dbi/dbi.h>

#include "pfutils.h"
#include "datasql.h"

///////////////////////////////////////////////////////////////////////////
// basic database ussage
void *
tsim_dbisqlite3_conn (const char *path/*"./userdata/"*/, const char *dbname /*"tilesim.udb"*/)
{
    dbi_conn conn = dbi_conn_new ("sqlite3");
    if (NULL == conn) {
        return NULL;
    }
    dbi_conn_set_option (conn, "sqlite3_dbdir", path);
    dbi_conn_set_option (conn, "dbname",        dbname);
    if (dbi_conn_connect (conn) < 0) {
        dbi_conn_close (conn);
        return NULL;
    }
    return conn;
}

int
tsim_dbi_init (void)
{
    //return dbi_initialize (NULL);
    return dbi_initialize ("/usr/lib/dbd");
}

void
tsim_dbi_shutdown (void)
{
    dbi_shutdown ();
}

void
tsim_dbi_disconn (dbi_conn conn)
{
    dbi_conn_close (conn);
}

static int
tsim_dbi_lnk_isok (dbi_conn conn)
{
    if (conn) {
        return 1;
    }
    return 0;
}
/*
数据库存储流程：

在导入(import)时：
  需提供该sim的名称
  让用户选择是否记录合并源历史、每次步骤历史、缓存计算时间长的合并测试等（存入数据库）。
如果用户选择使用历史：
  导入初始数据到数据库
在运行时
  根据选项：保存每次合并历史，保存合并步骤等，计算时间长的测试

*/
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
#define MY_DEBUG 1
static int
_tsim_insert_supertile (dbi_conn conn, size_t idb, size_t idstile, size_t cnt, tstilecomb_t *ptc)
{
    dbi_result result;
    size_t j;
    tstilecombitem_t tci;

    result = dbi_conn_queryf (conn, "INSERT INTO supertile (id, idb, quantity/*, name*/) VALUES (%d, %d, %d);",
        idstile, idb, cnt);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    dbi_result_free (result);
    for (j = 0; j < slist_size (&(ptc->tbuf)); j ++) {
        slist_data (&(ptc->tbuf), j, &tci);
        result = dbi_conn_queryf (conn, "INSERT INTO tileitem (id, idb, idstile, idtile, rotnum, x, y, z/*, name*/) VALUES (%d, %d, %d, %d, %d, %d, %d, 0);",
            j, idb, idstile, tci.idtile, tci.rotnum, tci.x, tci.y);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);
    }
    return 0;
}

// save the base information of simulation to database
// call this function immediately after loading the xml file!
int
tsim_save_baseinfo_dbi (dbi_conn conn, const char *name, tssiminfo_t *ptsim)
{
    dbi_result result;
    char buf[50];
    time_t curtime;
    struct tm mytm;
    char *pcs_name  = NULL;
    char *pcs_birth = NULL;
    size_t i;
    size_t j;
    size_t cnt;
    tstilecomb_t *ptc;
    tstilecombitem_t tci;
#ifdef __MINGW32_VERSION
#define gmtime_r(a,b) gmtime(a)
#endif
    curtime = time (NULL);
    dbi_conn_quote_string_copy (conn, name, &pcs_name);
    strftime (buf, sizeof (buf) - 1, "%Y-%m-%d %H:%M:%S", gmtime_r (&curtime, &mytm));
    dbi_conn_quote_string_copy (conn, buf, &pcs_birth);

    result = dbi_conn_queryf (conn, "INSERT INTO tilesimbucket (birth, temperature, rotatable, name) VALUES (DATETIME(%s), %d, %d, %s);",
        pcs_birth, ptsim->temperature, (ptsim->flg_norotate?0:1), pcs_name);

    free (pcs_birth);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        free (pcs_name);
        return -1;
    }
    dbi_result_free (result);
    result = dbi_conn_queryf (conn, "SELECT id FROM tilesimbucket WHERE name=%s;",
        pcs_name);
    free (pcs_name);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    if (dbi_result_get_numrows (result) < 1) {
#if MY_DEBUG
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        dbi_result_free (result);
        return -1;
    }
    assert (1 == dbi_result_get_numrows (result));
    ptsim->id = 0;
    ptsim->flg_usedb = 1;
    if (dbi_result_next_row (result)) {
        ptsim->id = dbi_result_get_ulonglong (result, "id");
    }
    dbi_result_free (result);
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        result = dbi_conn_queryf (conn, "INSERT INTO glue (id, idb, strength/*, name*/) VALUES (%d, %d, %d);",
            i, ptsim->id, ptsim->pgluevec[i]);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);
    }
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        result = dbi_conn_queryf (conn, "INSERT INTO tile (id, idb, idgnorth, idgeast, idgsouth, idgwest/*, idgfront, idgback, name*/) VALUES (%d, %d, %d, %d, %d, %d);",
            i, ptsim->id, ptsim->ptilevec[i].glueN, ptsim->ptilevec[i].glueE, ptsim->ptilevec[i].glueS, ptsim->ptilevec[i].glueW);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);
    }
    for (i = 0; i < ma_size (&(ptsim->tilelist)); i ++) {
#if 0
        ma_data (&(ptsim->countlist), i, &cnt);
        ptc = ma_data_lock (&(ptsim->tilelist), i);
        _tsim_insert_supertile (conn, ptsim->id, i, cnt, ptc);
        ma_data_unlock (&(ptsim->tilelist), i);
#else
        ma_data (&(ptsim->countlist), i, &cnt);
        result = dbi_conn_queryf (conn, "INSERT INTO supertile (id, idb, quantity/*, name*/) VALUES (%d, %d, %d);",
            i, ptsim->id, cnt);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);

        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), i);
        for (j = 0; j < slist_size (&(ptc->tbuf)); j ++) {
            slist_data (&(ptc->tbuf), j, &tci);
            result = dbi_conn_queryf (conn, "INSERT INTO tileitem (id, idb, idstile, idtile, rotnum, x, y, z/*, name*/) VALUES (%d, %d, %d, %d, %d, %d, %d, 0);",
                j, ptsim->id, i, tci.idtile, tci.rotnum, tci.x, tci.y);
            if (! result) {
#if MY_DEBUG
                //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
                const char *errmsg = NULL;
                int errdbi = dbi_conn_error (conn, &errmsg);
                printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
                break;
            }
            dbi_result_free (result);
        }
        ma_data_unlock (&(ptsim->tilelist), i);
#endif
    }

    return 0;
}
#if MY_DEBUG
static const char * mydbi_get_type_idx (dbi_result result, int idx)
{
    switch (dbi_result_get_field_type_idx(result, idx)) {
    case DBI_TYPE_INTEGER:
        return ("int");
        break;
    case DBI_TYPE_DECIMAL:
        return ("decimal");
        break;
    case DBI_TYPE_STRING:
        return ("str");
        break;
    case DBI_TYPE_BINARY:
        return ("bin");
        break;
    case DBI_TYPE_DATETIME:
        return ("date");
        break;
    }
    return "unknown";
}
static const char * mydbi_get_type (dbi_result result, const char *fieldname)
{
    switch (dbi_result_get_field_type (result, fieldname)) {
    case DBI_TYPE_INTEGER:
        return ("int");
        break;
    case DBI_TYPE_DECIMAL:
        return ("decimal");
        break;
    case DBI_TYPE_STRING:
        return ("str");
        break;
    case DBI_TYPE_BINARY:
        return ("bin");
        break;
    case DBI_TYPE_DATETIME:
        return ("date");
        break;
    }
    return "unknown";
}
#endif

// walk around the bug in libdbi:
static size_t
mydbi_get_int (dbi_result result, const char *fieldname)
{
    switch (dbi_result_get_field_type (result, fieldname)) {
    case DBI_TYPE_INTEGER:
    case DBI_TYPE_DECIMAL:
        return dbi_result_get_ulonglong (result, fieldname);
    case DBI_TYPE_STRING:
        return atoi (dbi_result_get_string (result, fieldname));
    case DBI_TYPE_BINARY:
    case DBI_TYPE_DATETIME:
    default:
        break;
    }
    printf ("ERROR: unknown type: %s\n", mydbi_get_type(result, fieldname));
    assert (0);
    return 0;
}

// retrive the base information of one simulation from the database.
// the ptsim was initialized
int
tsim_load_baseinfo_dbi (dbi_conn conn, const char *name, tssiminfo_t *ptsim)
{
    dbi_result result;
    dbi_result result_sitem;
    char *pcs_name = NULL;
    size_t cnt;
    size_t idstile;
    size_t idxinc;
    tstilecomb_t *ptc;
    tstilecombitem_t tci;
    size_t i;
    size_t j;

    assert (0 == ptsim->num_gluevec);
    assert (0 == ptsim->num_tilevec);
    assert (NULL == ptsim->pgluevec);
    assert (NULL == ptsim->ptilevec);
    assert (NULL != &(ptsim->tilelist));
    assert (NULL != &(ptsim->countlist));
    assert (NULL != &(ptsim->tilelist));
    assert (NULL != &(ptsim->countlist));
    assert (0 == ma_size (&(ptsim->tilelist)));
    assert (0 == ma_size (&(ptsim->countlist)));

    dbi_conn_quote_string_copy (conn, name, &pcs_name);

    result = dbi_conn_queryf (conn, "SELECT id, temperature, rotatable/*, birth, name*/ FROM tilesimbucket WHERE name=%s;", pcs_name);
    free (pcs_name);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
        //printf ("result->field_types[1]:%d, %d", result->field_types[1], DBI_TYPE_INTEGER);
#endif
        return -1;
    }
    if (dbi_result_get_numrows (result) < 1) {
        dbi_result_free (result);
        return -1;
    }
    assert (1 == dbi_result_get_numrows (result));
    if (dbi_result_next_row (result)) {
        ptsim->id     = dbi_result_get_ulonglong (result, "id");
#if MY_DEBUG
        if (0 != dbi_conn_error_flag (conn)) {
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
        }
#endif
#if MY_DEBUG
//printf ("type of 1: %s\n", mydbi_get_type_idx(result, 1));printf ("type of 2: %s\n", mydbi_get_type_idx(result, 2));printf ("type of 3: %s\n", mydbi_get_type_idx(result, 3));
#endif
        ptsim->temperature = mydbi_get_int (result, "temperature");
#if MY_DEBUG
        if (0 != dbi_conn_error_flag (conn)) {
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
        }
#endif
        ptsim->flg_norotate = (0 == dbi_result_get_char (result, "rotatable")?1:0);
#if MY_DEBUG
        if (0 != dbi_conn_error_flag (conn)) {
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
        }
#endif
        //ptsim->name   = dbi_result_get_string (result, "name");
        //ptsim->birth  = dbi_result_get_datetime (result, "birth");
    }
    dbi_result_free (result);

    result = dbi_conn_queryf (conn, "SELECT id, strength FROM glue WHERE glue.idb=%d ORDER BY id;", ptsim->id);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    ptsim->num_gluevec = dbi_result_get_numrows (result);
    if (ptsim->num_gluevec < 1) {
        dbi_result_free (result);
        return -1;
    }
    ptsim->pgluevec    = (size_t *)malloc (ptsim->num_gluevec * sizeof (size_t));
    assert (NULL != ptsim->pgluevec);
    memset (ptsim->pgluevec, 0, ptsim->num_gluevec * sizeof (size_t));
    for (i = 0; i < ptsim->num_gluevec; i ++) {
        if (dbi_result_next_row (result)) {
            assert (i == dbi_result_get_ulonglong (result, "id"));
            ptsim->pgluevec[i] = mydbi_get_int (result, "strength");
        }
    }
    dbi_result_free (result);

    // the tile vector
    result = dbi_conn_queryf (conn, "SELECT id, idgnorth, idgeast, idgsouth, idgwest/*, idgfront, idgback*/ FROM tile WHERE tile.idb=%d ORDER BY id;", ptsim->id);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    ptsim->num_tilevec = dbi_result_get_numrows (result);
    if (ptsim->num_tilevec < 1) {
        dbi_result_free (result);
        return -1;
    }

    ptsim->ptilevec    = (tstile_t *)malloc (ptsim->num_tilevec * sizeof (tstile_t));
    assert (NULL != ptsim->ptilevec);
    memset (ptsim->ptilevec, 0, ptsim->num_tilevec * sizeof (tstile_t));
    for (i = 0; i < ptsim->num_tilevec; i ++) {
        if (dbi_result_next_row (result)) {
            assert (i == dbi_result_get_ulonglong (result, "id"));
            ptsim->ptilevec[i].glueN = mydbi_get_int (result, "idgnorth");
            ptsim->ptilevec[i].glueE = mydbi_get_int (result, "idgeast");
            ptsim->ptilevec[i].glueS = mydbi_get_int (result, "idgsouth");
            ptsim->ptilevec[i].glueW = mydbi_get_int (result, "idgwest");
            //ptsim->ptilevec[i].glueF = mydbi_get_int (result, "idgfront");
            //ptsim->ptilevec[i].glueB = mydbi_get_int (result, "idgback");
        }
    }
    dbi_result_free (result);

    // the supertile vector
    result = dbi_conn_queryf (conn, "SELECT id, quantity/*, name*/ FROM supertile WHERE supertile.idb=%d ORDER BY id;", ptsim->id);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    //printf ("here {ln:%d;fn:"__FILE__"}\n", __LINE__);
    memset (&tci, 0, sizeof (tci));
    for (i = 0; dbi_result_next_row (result); i ++) {
        idstile = dbi_result_get_ulonglong (result, "id");
        cnt = mydbi_get_int (result, "quantity");
        ma_insert (&(ptsim->countlist), ma_size (&(ptsim->countlist)), &cnt);
        idxinc = ma_inc (&(ptsim->tilelist));
        ptc = (tstilecomb_t *)ma_data_lock (&(ptsim->tilelist), idxinc);
        tstc_init (ptc);
        result_sitem  = dbi_conn_queryf (conn, "SELECT id, idtile, rotnum, x, y/*, z*/ FROM tileitem WHERE tileitem.idb=%d AND tileitem.idstile=%d ORDER BY id;",
                ptsim->id, idstile);
        if (! result_sitem) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            ma_data_unlock (&(ptsim->tilelist), idxinc);
            return -1;
        }
        ptc->id = i;
        for (j = 0; dbi_result_next_row (result_sitem); j ++) {
            tci.idtile = mydbi_get_int (result_sitem, "idtile");
            tci.rotnum = mydbi_get_int (result_sitem, "rotnum");
            tci.x = mydbi_get_int (result_sitem, "x");
            tci.y = mydbi_get_int (result_sitem, "y");
            //tci.z = dbi_result_get_ulonglong (result, "z");
            tstc_additem (ptc, &tci);
        }
        ma_data_unlock (&(ptsim->tilelist), idxinc);
        dbi_result_free (result_sitem);
        assert (j == dbi_result_get_numrows (result_sitem));
    }
    assert (i == dbi_result_get_numrows (result));
    dbi_result_free (result);

    return 0;
}

// get the combination position information from database
// return < 0: if the infomation is not accessable; the caller should call tstc_mesh_test() again to calculate the positions.
// return == 0: if the information is in database; plist_points_ok will contains all of the positions (as the tstc_mesh_test() returned); if the items in the list is zero, then the two supertile could not combine.
int
tssim_cb_findmergepos_dbi (void *pdbmi, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok)
{
    dbmergeinfo_t *dbmergeinfo = (dbmergeinfo_t *)pdbmi;
    dbi_result result;
    tsstpos_t stpos;
    int x;

    assert (idx_base >= idx_test);
    assert (NULL != plist_points_ok);
    assert (0 == ma_size (plist_points_ok));

    // the tile vector
    result = dbi_conn_queryf (dbmergeinfo->conn, "SELECT id, rotnum, x, y/*, z*/ FROM stilecombineposition WHERE idb=%d AND idpstbase=%d AND idpsttest=%d AND temperature=%d ORDER BY id;",
                dbmergeinfo->id, idx_base, idx_test, temperature);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    if (dbi_result_get_numrows (result) < 1) {
        return -1;
    }
    if (dbi_result_get_numrows (result) == 1) {
        x = dbi_result_get_longlong (result, "x");
        if (x < 0) {
            return 0;
        }
        stpos.rotnum = mydbi_get_int (result, "rotnum");
        stpos.x = mydbi_get_int (result, "x");
        stpos.y = mydbi_get_int (result, "y");
        //stpos.z = dbi_result_get_ulonglong (result, "z");
        ma_insert (plist_points_ok, ma_size(plist_points_ok), &stpos);
    }

    for (; dbi_result_next_row (result); ) {
        stpos.rotnum = mydbi_get_int (result, "rotnum");
        stpos.x = mydbi_get_int (result, "x");
        stpos.y = mydbi_get_int (result, "y");
        //stpos.z = dbi_result_get_ulonglong (result, "z");
        ma_insert (plist_points_ok, ma_size(plist_points_ok), &stpos);
    }
    dbi_result_free (result);
    return 0;
}

int
tssim_cb_storemergepos_dbi (void *pdbmi, size_t idx_base, size_t idx_test, size_t temperature, memarr_t *plist_points_ok)
{
    dbmergeinfo_t *dbmergeinfo = (dbmergeinfo_t *)pdbmi;
    dbi_result result;
    size_t i;
    tsstpos_t stpos;

    for (i = 0; i < ma_size (plist_points_ok); i ++) {
        ma_data (plist_points_ok, i, &stpos);
        result = dbi_conn_queryf (dbmergeinfo->conn, "INSERT INTO stilecombineposition (idb, temperature, idpstbase, idpsttest, rotnum, x, y/*, z*/) VALUES (%d, %d, %d, %d, %d, %d, %d);",
            dbmergeinfo->id, temperature, idx_base, idx_test, stpos.rotnum, stpos.x, stpos.y/*, stpos.z*/);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);
    }
    return 0;
}

// idx_base, idx_test: are the index of the base supertile and test supertile seperately.
// idx_created: the index of the new supertile in the pcountlist/ptilelist, it may be the index of existed supertile.
// pstpos: the position of the test supertile, which is selected by the simulation system, could not be NULL
// ptc: if the type of the created supertile is exist, then ptc==NULL, otheriwse is the detail of the created supertile
// plist_points_ok: the list of the positions that could place the test supertile. it may be NULL.
// return < 0: error, the simulation should exit
// return >= 0: ok, continue next simulation item
// the callback function could check the items of result list.
// one case is that the caller want to show some image in the user screen.
int
tssim_cb_resultinfo_record_history_dbi (void *pdbmi, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    dbmergeinfo_t *dbmergeinfo = (dbmergeinfo_t *)pdbmi;
    dbi_result result;

    // 将 idx_base, idx_test 所对应的supertile记录的quantity更新(每个都减1)
    result = dbi_conn_queryf (dbmergeinfo->conn, "UPDATE supertile SET quantity=quantity-1 WHERE idb='%d' AND id='%d';", dbmergeinfo->id, idx_base);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    dbi_result_free (result);
    result = dbi_conn_queryf (dbmergeinfo->conn, "UPDATE supertile SET quantity=quantity-1 WHERE idb='%d' AND id='%d';", dbmergeinfo->id, idx_test);
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    dbi_result_free (result);

    // 如果 ptc 为 NULL 表示没有形成新类型的 supertile
    if (NULL == ptc) {
        result = dbi_conn_queryf (dbmergeinfo->conn, "UPDATE stilehistory SET times=times+1 WHERE idb=%d AND idstile=%d AND idpstbase=%d AND idpsttest=%d AND rotnum=%d AND x=%d AND y=%d AND z=%d;", dbmergeinfo->id, idx_created, idx_base, idx_test, pstpos->rotnum, pstpos->x, pstpos->y, /*pstpos->z*/0);
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);

    } else {
        _tsim_insert_supertile (dbmergeinfo->conn, dbmergeinfo->id, idx_created, 1, ptc);

        result = dbi_conn_queryf (dbmergeinfo->conn, "INSERT INTO stilehistory (idb, idstile, times, idpstbase, idpsttest, rotnum, x, y, z) VALUES (%d, %d, 1, %d, %d, %d, %d, %d, 0);",
            dbmergeinfo->id, idx_created, idx_base, idx_test, pstpos->rotnum, pstpos->x, pstpos->y
            );
        if (! result) {
#if MY_DEBUG
            //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
            const char *errmsg = NULL;
            int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
            printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
            return -1;
        }
        dbi_result_free (result);
    }

#if 1 /* record all of the steps of simulation */

    result = dbi_conn_queryf (dbmergeinfo->conn, "INSERT INTO stilestephistory (idb, idpstbase, idpsttest, rotnum, x, y, z) VALUES (%d, %d, %d, %d, %d, %d, 0);",
        dbmergeinfo->id, idx_base, idx_test, pstpos->rotnum, pstpos->x, pstpos->y
        );
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    dbi_result_free (result);

#endif
    return 0;
}

// get the step information of the simulation
// idx: the begin index of the steps
// cnt: how many steps will be accepted by the callback function
// cb, userdata: the callback function and the userdata pointer
int
tsim_fetch_history_steps (void * pdbmi, size_t idx, size_t cnt, tsim_cb_getitem_histstep_t cb, void *userdata)
{
    dbmergeinfo_t *dbmergeinfo = (dbmergeinfo_t *)pdbmi;
    dbi_result result;

    result = dbi_conn_queryf (dbmergeinfo->conn, "SELECT id, idpstbase, idpsttest, rotnum, x, y, z FROM stilestephistory WHERE idb='%d' ORDER BY id LIMIT %d, %d;",
        dbmergeinfo->id, idx, cnt
        );
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    for (; dbi_result_next_row (result); ) {
        cb (userdata,
            dbi_result_get_ulonglong (result, "id"),
            mydbi_get_int (result, "idpstbase"),
            mydbi_get_int (result, "idpsttest"),
            mydbi_get_int (result, "rotnum"),
            mydbi_get_int (result, "x"),
            mydbi_get_int (result, "y"),
            mydbi_get_int (result, "z")
            );
    }
    dbi_result_free (result);
    return 0;
}

// get the parents information of each supertile
// idx: the begin index of the steps
// cnt: how many steps will accept by the callback function
// cb, userdata: the callback function and the userdata pointer
int
tsim_fetch_history_parents (void * pdbmi, size_t idstile, size_t idx, size_t cnt, tsim_cb_getitem_histparents_t cb, void *userdata)
{
    dbmergeinfo_t *dbmergeinfo = (dbmergeinfo_t *)pdbmi;
    dbi_result result;

    result = dbi_conn_queryf (dbmergeinfo->conn, "SELECT id, idpstbase, idpsttest, rotnum, x, y, z FROM stilehistory WHERE idb='%d' AND idstile='%d' ORDER BY id LIMIT %d, %d;",
        dbmergeinfo->id, idstile, idx, cnt
        );
    if (! result) {
#if MY_DEBUG
        //int dbi_conn_error(dbi_conn Conn, const char **errmsg_dest);
        const char *errmsg = NULL;
        int errdbi = dbi_conn_error (dbmergeinfo->conn, &errmsg);
        printf ("Err: %d: %s {%d,%s}\n", errdbi, errmsg, __LINE__, __FILE__);
#endif
        return -1;
    }
    for (; dbi_result_next_row (result); ) {
        cb (userdata,
            dbi_result_get_ulonglong (result, "id"),
            mydbi_get_int (result, "idpstbase"),
            mydbi_get_int (result, "idpsttest"),
            mydbi_get_int (result, "rotnum"),
            mydbi_get_int (result, "x"),
            mydbi_get_int (result, "y"),
            mydbi_get_int (result, "z"),
            mydbi_get_int (result, "times")
            );
    }
    dbi_result_free (result);
    return 0;
}
#endif /* USE_DBRECORD */
