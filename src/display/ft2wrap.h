#ifndef FREE_NEHE_H
#define FREE_NEHE_H

#ifdef __cplusplus
extern "C" {
#endif

// Text Renderer

//The init function will create a font of
//of the height h from the file fname.
extern void * gltr_create (const char * fname, unsigned int h);

//Free all the resources assosiated with the font.
extern void gltr_destroy (void *pref);

//The flagship function of the library - this thing will print
//out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text. 
extern void gltr_printf (void *pref, GLfloat x, GLfloat y, const wchar_t *fmt, ...) ;

extern GLfloat gltr_get_adjusted_width (void *pref, const wchar_t* str);
extern GLuint  gltr_getgl_listid (void *pref, const wchar_t c);

extern GLfloat gltr_get_height (void *pref);

#define gltr_draw_wchar(pref, wch) glCallList (gltr_getgl_listid ((void *)(pref), (wch)))

#ifdef __cplusplus
}
#endif


#endif