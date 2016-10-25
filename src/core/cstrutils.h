/******************************************************************************
 * Name:        cstrutils.h
 * Purpose:     some functions for handling the string
 * Author:      Yunhui Fu
 * Created:     2009-10-15
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef _CSTR_UTILS_H
#define _CSTR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

char * strrstr_len (const char *haystack, size_t haystack_len, const char * needle);
char * cstr_stripblank (char *pbuf);
char * cstr_trim (char *buf);

#ifdef __cplusplus
}
#endif

#endif /* _CSTR_UTILS_H */
