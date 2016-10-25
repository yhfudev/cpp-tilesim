/******************************************************************************
 * Name:        outtds.c
 * Purpose:     output tile information data to .tds file
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#include <stdio.h>
#include "outtds.h"

int
zzconv_output_tds (FILE *fpout, const char *name, const char *label, const char *color, const char *gN, const char *gE, const char *gS, const char *gW, const char *gF, const char *gB)
{
    fprintf (fpout, "TILENAME %s\n", ((NULL == name)?"":name));
    fprintf (fpout, "LABEL %s\n", ((NULL == label)?"":label));
    fprintf (fpout, "NORTHBIND %d\n", ((NULL == gN)?0:1));
    fprintf (fpout, "EASTBIND %d\n",  ((NULL == gE)?0:1));
    fprintf (fpout, "SOUTHBIND %d\n", ((NULL == gS)?0:1));
    fprintf (fpout, "WESTBIND %d\n",  ((NULL == gW)?0:1));
    if (gF || gB) {
        fprintf (fpout, "UPBIND %d\n",    ((NULL == gF)?0:1));
        fprintf (fpout, "DOWNBIND %d\n",  ((NULL == gB)?0:1));
    }
    fprintf (fpout, "NORTHLABEL %s\n", ((NULL == gN)?"":gN));
    fprintf (fpout, "EASTLABEL %s\n",  ((NULL == gE)?"":gE));
    fprintf (fpout, "SOUTHLABEL %s\n", ((NULL == gS)?"":gS));
    fprintf (fpout, "WESTLABEL %s\n",  ((NULL == gW)?"":gW));
    if (gF || gB) {
        fprintf (fpout, "UPLABEL %s\n",    ((NULL == gF)?"":gF));
        fprintf (fpout, "DOWNLABEL %s\n",  ((NULL == gB)?"":gB));
    }
    if (color) {
        fprintf (fpout, "TILECOLOR %s\n", color);
    }
    fprintf (fpout, "CREATE\n\n");
    return 0;
}

int
zzconv_output_tds_ewmirror (FILE *fpout, const char *name, const char *label, const char *color, const char *gN, const char *gE, const char *gS, const char *gW, const char *gF, const char *gB)
{
    return zzconv_output_tds (fpout, name, label, color, gN, gW, gS, gE, gF, gB);
}
