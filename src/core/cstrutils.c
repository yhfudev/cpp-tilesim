/******************************************************************************
 * Name:        cstrutils.c
 * Purpose:     some functions for handling the string
 * Author:      Yunhui Fu
 * Created:     2009-10-15
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <string.h>
#include "cstrutils.h"

char *
strrstr_len (const char *haystack, size_t haystack_len, const char * needle)
{
    char *p = NULL;
    char *pcur;
    for (;;) {
        // strstr(haystack, needle)
        pcur = strstr ((char *)haystack, needle);
        if (NULL == pcur) {
            break;
        }
        p = pcur;
        haystack = p + 1;
    }
    return p;
}

#define isspace2(c) (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

char *
cstr_trim (char *buf)
{
    size_t szbuf;
    char *pequ;
    char *pcmt;
    if ('\0' == *buf)
    {
        return buf;
    }

    /* 删除后面的空格和换行符 */
    pequ = buf + strlen (buf) - 1;
    while (isspace2(*pequ))
    {
        pequ --;
    }
    *(pequ + 1) = 0;
    /* 查找前面的空格 */
    pcmt = buf;
    while (isspace2(*pcmt))
    {
        pcmt ++;
    }
    if (pcmt > pequ)
    {
        return pequ;
    }
    szbuf = pequ + 1 - pcmt;

    memmove (buf, pcmt, szbuf + 1);
    return buf;
}

char *
cstr_stripblank (char *pbuf)
{
    int i;
    int len;
    char *buf = pbuf;
    len = strlen (buf);
    for (; *buf; buf ++) {
        if (isspace2(*buf)) {
            *buf = ' ';
            buf ++;
            len --;
            for (i = 0; isspace2(buf[i]); i ++);
            if (i > 0) {
                memmove (buf, buf + i, len - i + 1);
                len -= i;
            }
        }
        len --;
    }
    return pbuf;
}