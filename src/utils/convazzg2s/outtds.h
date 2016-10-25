/******************************************************************************
 * Name:        outtds.h
 * Purpose:     output tile information data to .tds file
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _OUTTDS_H
#define _OUTTDS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ZZCOUV_COLOR_IN   "rgb(160,  32, 240)"
#define ZZCOUV_COLOR_OUT  "rgb(139, 191, 255)"
#define ZZCOUV_COLOR_ENC  "rgb( 94, 255,   0)"
#define ZZCOUV_COLOR_ENCI "rgb(204, 255, 139)"
#define ZZCOUV_COLOR_DEC  "rgb(255, 156,  45)"
#define ZZCOUV_COLOR_DECI "rgb(255, 200,  45)"
#define ZZCOUV_COLOR_UNKNOWN "yellow"

typedef int (* zzconv_cb_output_t) (FILE *fpout, const char *name, const char *label, const char *color, const char *gN, const char *gE, const char *gS, const char *gW, const char *gF, const char *gB);

extern int zzconv_output_tds (FILE *fpout, const char *name, const char *label, const char *color, const char *gN, const char *gE, const char *gS, const char *gW, const char *gF, const char *gB);
extern int zzconv_output_tds_ewmirror (FILE *fpout, const char *name, const char *label, const char *color, const char *gN, const char *gE, const char *gS, const char *gW, const char *gF, const char *gB);

#define zzconv_output_buf_rgb(buffer, group) sprintf (buffer, "rgb(%d,%d,%d)", ((group >> 16) & 0xFF), ((group >> 8) & 0xFF), (group & 0xFF))

#ifdef __cplusplus
}
#endif

#endif /* _OUTTDS_H */
