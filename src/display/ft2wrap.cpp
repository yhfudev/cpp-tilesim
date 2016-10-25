/*
    A quick and simple opengl font library that uses GNU freetype2, written
    and distributed as part of a tutorial for nehe.gamedev.net.
    Sven Olsen, 2003
*/

//MSVC will spit out all sorts of useless warnings if
//you create vectors of strings, this pragma gets rid of them.
//#pragma warning(disable: 4786) 

#include <assert.h>
#include <wchar.h>

//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

//OpenGL Headers 
//#include <windows.h>        //(the GL headers need it)
#include <GL/gl.h>
#include <GL/glu.h>

//Some STL headers
#include <iostream>
#include <vector>
#include <string>
#include <map>

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>

//Include our header file.
#include "ft2wrap.h"

#define NUM_TEXTURE 128

typedef struct _gltr_mapitem_t {
    GLuint listID;
    GLuint texID;
    GLfloat width;
} gltr_mapitem_t;

typedef std::map<wchar_t, gltr_mapitem_t> gltr_map_t;
//This holds all of the information related to any
//freetype font that we want to create.  
typedef struct _gltr_t {
    GLfloat h;              ///< Holds the height of the font.

    FT_Library ftlib;
    FT_Face    ftface;
    gltr_map_t wc2item;
} gltr_t;

///This function gets the first power of 2 >= the
///int that we pass it.
inline int next_p2 ( int a )
{
    int rval=1;
    while (rval<a) rval<<=1;
    return rval;
}

int
divPow2 (int val, int pow)
{
    if (pow < 1) return val;
    return val >> (pow);
}

///Create a display list coresponding to the give character.
static void
make_dlist ( FT_Face face, wchar_t ch, GLuint list_base, GLuint tex_base, GLfloat *pret_width )
{
    //The first thing we do is get FreeType to render our character
    //into a bitmap.  This actually requires a couple of FreeType commands:

    //Load the Glyph for our character.
    if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
        throw std::runtime_error("FT_Load_Glyph failed");

    //Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph ))
        throw std::runtime_error("FT_Get_Glyph failed");

    //Convert the glyph to a bitmap.
    FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    //This reference will make accessing the bitmap easier
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;

    //Use our helper function to get the widths of
    //the bitmap data that we will need in order to create
    //our texture.
    int width = next_p2( bitmap.width );
    int height = next_p2( bitmap.rows );

    //Allocate memory for the texture data.
    GLubyte* expanded_data = new GLubyte[ 2 * width * height];

// Here We Fill In The Data For The Expanded Bitmap.
    // Notice That We Are Using A Two Channel Bitmap (One
    // Channel For Luminosity And One For Alpha Values).
    //
    // We Make All The Luminosity Values White, And Use The Freeytpe Generated Bitmap
    // To Set Up The Alpha Values. Given The Blend Function That We're Going To Use,
    // This Will Make OpenGL Render The Font Properly.
    //
    // We Use The ?: Operator To Say That The Alpha Value Which We Use
    // Will Be 0 If We Are In The Padding Zone, And Whatever
    // Is The FreeType Bitmap Otherwise.
     for(int j=0; j <height;j++) for(int i=0; i < width; i++) {
        expanded_data[2*(i+j*width)] = 255;
        expanded_data[2*(i+j*width)+1] = (i>=bitmap.width || j>=bitmap.rows) ? 
            0 : bitmap.buffer[i + bitmap.width*j];
    }

    //Now we just setup some texture paramaters.
    glBindTexture( GL_TEXTURE_2D, tex_base);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    //Here we actually create the texture itself, notice
    //that we are using GL_LUMINANCE_ALPHA to indicate that
    //we are using 2 channel data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
          0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

    //With the texture created, we don't need to expanded data anymore
    delete [] expanded_data;

    //So now we can create the display list
    glNewList(list_base,GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, tex_base);

    glPushMatrix();

    //first we need to move over a little so that
    //the character has the right amount of space
    //between it and the one before it.
    glTranslatef(bitmap_glyph->left,0,0);

    //Now we move down a little in the case that the
    //bitmap extends past the bottom of the line 
    //(this is only true for characters like 'g' or 'y'.
    glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

    //Now we need to account for the fact that many of
    //our textures are filled with empty padding space.
    //We figure what portion of the texture is used by 
    //the actual character and store that information in 
    //the x and y variables, then when we draw the
    //quad, we will only reference the parts of the texture
    //that we contain the character itself.
    float    x=(float)bitmap.width / (float)width,
            y=(float)bitmap.rows / (float)height;

    if (pret_width) {
        // store char width
        // note .5f
        *pret_width = x;
    }

    //Here we draw the texturemaped quads.
    //The bitmap that we got from FreeType was not 
    //oriented quite like we would like it to be,
    //so we need to link the texture to the quad
    //so that the result will be properly aligned.
    glBegin(GL_QUADS);
    glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
    glTexCoord2d(0,y); glVertex2f(0,0);
    glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
    glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
    glEnd();
    glPopMatrix();
    glTranslatef(face->glyph->advance.x >> 6 ,0,0);

    //increment the raster position as if we were a bitmap font.
    //(only needed if you want to calculate text length)
    //glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);

    //Finnish the display list
    glEndList();
}

GLuint
gltr_getgl_listid (void *pref, const wchar_t c)
{
    gltr_t *pft2wf = (gltr_t *)pref;
    gltr_mapitem_t mapitem;
    gltr_map_t::iterator it = pft2wf->wc2item.find (c);
    if (it != pft2wf->wc2item.end ()) {
        return it->second.listID;
    }
    mapitem.listID = glGenLists (1);
    glGenTextures (1, &(mapitem.texID));
    make_dlist (pft2wf->ftface, c, mapitem.listID, mapitem.texID, &(mapitem.width));
    pft2wf->wc2item[c] = mapitem;
    return mapitem.listID;
}

void *
gltr_create (const char * fname, unsigned int h)
{
    gltr_t *pft2wf;
    pft2wf = new gltr_t;
    if (NULL == pft2wf) {
        return NULL;
    }

    pft2wf->h=h;

    //Create and initilize a freetype font library.
    if (FT_Init_FreeType( &(pft2wf->ftlib) )) {
        //throw std::runtime_error("FT_Init_FreeType failed");
        delete (pft2wf);
        return NULL;
    }

    //The object in which Freetype holds information on a given
    //font is called a "face".

    //This is where we load in the font information from the file.
    //Of all the places where the code might die, this is the most likely,
    //as FT_New_Face will die if the font file does not exist or is somehow broken.
    if (FT_New_Face( pft2wf->ftlib, fname, 0, &(pft2wf->ftface) )) {
        // fail back:
#ifdef _WIN32
        if (FT_New_Face( pft2wf->ftlib, "C:\\Windows\\Fonts\\arial.ttf", 0, &(pft2wf->ftface) ))
        if (FT_New_Face( pft2wf->ftlib, "C:\\Windows\\Fonts\\consola.ttf", 0, &(pft2wf->ftface) ))
        if (FT_New_Face( pft2wf->ftlib, "C:\\Windows\\Fonts\\couri.ttf", 0, &(pft2wf->ftface) )) {
#else
        if (FT_New_Face( pft2wf->ftlib, "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc", 0, &(pft2wf->ftface) ))
        if (FT_New_Face( pft2wf->ftlib, "/usr/share/fonts/truetype/mswin/simhei.ttf", 0, &(pft2wf->ftface) ))
        if (FT_New_Face( pft2wf->ftlib, "/usr/share/fonts/truetype/freefont/FreeSans.ttf", 0, &(pft2wf->ftface) )) {
#endif
            //throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
            FT_Done_FreeType (pft2wf->ftlib);
            delete (pft2wf);
            return NULL;
        }
    }

    //For some twisted reason, Freetype measures font size
    //in terms of 1/64ths of pixels.  Thus, to make a font
    //h pixels high, we need to request a size of h*64.
    //(h << 6 is just a prettier way of writting h*64)
    FT_Set_Char_Size( pft2wf->ftface, h << 6, h << 6, 96, 96);

    //Here we ask opengl to allocate resources for
    //all the textures and displays lists which we
    //are about to create.  

    //This is where we actually create each of the fonts display lists.
    for(wchar_t i = 0; i < NUM_TEXTURE; i ++) gltr_getgl_listid ((void *)pft2wf, i);

    return (void *)pft2wf;
}

void
gltr_destroy (void *pref)
{
    gltr_t *pft2wf = (gltr_t *)pref;
    assert (NULL != pft2wf);

    for (gltr_map_t::iterator it = pft2wf->wc2item.begin (); it != pft2wf->wc2item.end (); ++it) {
        glDeleteLists (it->second.listID,1);
        glDeleteTextures (1, &(it->second.texID));
    }

    FT_Face face = pft2wf->ftface;
    //We don't need the face information now that the display
    //lists have been created, so we free the assosiated resources.
    // TODO:: work out stupid freetype crashz0rs
    try {
        static int foo = 0;
        if(face && foo < 1) {
            foo++;
            FT_Done_Face (pft2wf->ftface);
            face = 0;
        }
    } catch(...) {
        face = 0;
    }

    //Ditto for the library.
    FT_Done_FreeType(pft2wf->ftlib);
    delete pft2wf;
}

/// A fairly straight forward function that pushes
/// a projection matrix that will make object world 
/// coordinates identical to window coordinates.
inline void pushScreenCoordinateMatrix() {
    GLint    viewport[4];

    glPushAttrib (GL_TRANSFORM_BIT);

    glGetIntegerv (GL_VIEWPORT, viewport);
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    gluOrtho2D (viewport[0], viewport[2], viewport[1], viewport[3]);

    glPopAttrib();
}

/// Pops the projection matrix without changing the current
/// MatrixMode.
inline void pop_projection_matrix() {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

///Much like Nehe's glPrint function, but modified to work
///with freetype fonts.
void
gltr_print (void *pref, float x, float y, const wchar_t *text)
{
    gltr_t *pft2wf = (gltr_t *)pref;

    float h = pft2wf->h / .63f;                        //We make the height about 1.5* that of

    //Here is some code to split the text that we have been
    //given into a set of lines.  
    //This could be made much neater by using
    //a regular expression library such as the one avliable from
    //boost.org (I've only done it out by hand to avoid complicating
    //this tutorial with unnecessary library dependencies).
    const wchar_t *start_line=text;
    std::vector<std::wstring> lines;
    const wchar_t *c;
    for(c=text;*c;c++) {
        if(*c=='\n') {
            std::wstring line;
            for(const wchar_t *n=start_line;n<c;n++) line.append(1,*n);
            lines.push_back(line);
            start_line=c+1;
        }
    }
    if(start_line) {
        std::wstring line;
        const wchar_t *n;
        for (n=start_line;n<c;n++) line.append(1,*n);
        lines.push_back(line);
    }

    // We want a coordinate system where things coresponding to window pixels.
    pushScreenCoordinateMatrix();

    //GLfloat color[4];
    //glGetFloatv(GL_CURRENT_COLOR, color);

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);    
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glListBase(0u);

    float modelview_matrix[16];    
    glGetFloatv (GL_MODELVIEW_MATRIX, modelview_matrix);

    //This is where the text display actually happens.
    //For each line of text we reset the modelview matrix
    //so that the line's text will start in the correct position.
    //Notice that we need to reset the matrix, rather than just translating
    //down by h. This is because when each character is
    //draw it modifies the current matrix so that the next character
    //will be drawn immediatly after it.  
    for(int i=0;i<lines.size();i++) {
        glPushMatrix ();
        glLoadIdentity ();
        glTranslatef (x, y - h * i, 0);
        glMultMatrixf (modelview_matrix);

        glListBase (0u);
        for(unsigned int j = 0; j < lines[i].length(); j ++) {
            //glCallList (gltr_getgl_listid ((void *)pft2wf, lines[i][j]));
            gltr_draw_wchar (pft2wf, lines[i][j]);
        }
    //  The commented out raster position stuff can be useful if you need to
    //  know the length of the text that you are creating.
    //  If you decide to use it make sure to also uncomment the glBitmap command
    //  in make_dlist().
    //    glRasterPos2f(0,0);
        //glCallLists (lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
    //    float rpos[4];
    //    glGetFloatv(GL_CURRENT_RASTER_POSITION ,rpos);
    //    float len=x-rpos[0];

        glPopMatrix();
    }

    glPopAttrib();

    //glColor4fv(color);

    pop_projection_matrix();
}

#ifdef WIN32
#define vswprintf _vsnwprintf
#endif

void
gltr_printf (void *pref, float x, float y, const wchar_t *fmt, ...)
{
    gltr_t *pft2wf = (gltr_t *)pref;

    wchar_t text[250];                           // Holds Our String
    va_list ap;                                  // Pointer To List Of Arguments

    if (fmt == NULL) {                           // If There's No Text
        *text=0;                                 // Do Nothing
    } else {
        va_start(ap, fmt);                       // Parses The String For Variables
        vswprintf (text, 250, fmt, ap);          // And Converts Symbols To Actual Numbers
        va_end(ap);                              // Results Are Stored In Text
    }
    gltr_print (pref, x, y, text);
}

GLfloat
gltr_get_adjusted_width (void *pref, const wchar_t* str)
{
    gltr_t *pft2wf = (gltr_t *)pref;
    GLfloat w = 0.0f;

    assert (NULL != pref);

    for (unsigned int i = 0; i < wcslen(str); i ++) {
        gltr_map_t::iterator it = pft2wf->wc2item.find (str[i]);
        if (it == pft2wf->wc2item.end ()) {
            gltr_getgl_listid ((void *)pft2wf, str[i]);
            it = pft2wf->wc2item.find (str[i]);
        }
        if (it != pft2wf->wc2item.end ()) {
            w += it->second.width;
        }
    }

    return w;
}

GLfloat
gltr_get_height (void *pref)
{
    return ((gltr_t *)pref)->h;
}
