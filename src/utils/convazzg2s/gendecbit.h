/******************************************************************************
 * Name:        gendecbit.h
 * Purpose:     generate tile set for one bits of the decode tile set
 * Author:      Yunhui Fu
 * Created:     2009-11-07
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _GENERATE_INFO_BIT_H
#define _GENERATE_INFO_BIT_H

#ifdef __cplusplus
extern "C" {
#endif

int gen_dectile_1bit_2west (FILE *fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k);
int gen_dectile_1bit_2east (FILE *fpout, char *tilename, char *glue_input, char *glue_output_0, char *glue_output_1, size_t k);

#ifdef __cplusplus
}
#endif

#endif /* _GENERATE_INFO_BIT_H */
