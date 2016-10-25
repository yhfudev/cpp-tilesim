/******************************************************************************
 * Name:        glutfont.h
 * Purpose:     show message in OpenGL scene
 * Author:      Yunhui Fu
 * Created:     2008-11-15
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2008 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/
#ifndef GLUTFONT_H
#define GLUTFONT_H
_PSF_BEGIN_EXTERN_C
void glutfont_init (void);
void glutfont_clear (void);
void glutfont_textout (int x, int y, char *s);
void glutfont_print (const char *fmt, ...);
_PSF_END_EXTERN_C
#endif /* GLUTFONT_H */
