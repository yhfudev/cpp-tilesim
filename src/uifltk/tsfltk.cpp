/******************************************************************************
 * Name:        tscli.c
 * Purpose:     Supertile Self-assembly Simulator GUI(fltk) version
 * Author:      Yunhui Fu
 * Created:     2009-03-01
 * Modified by:
 * RCS-ID:      $Id: $
 * Copyright:   (c) 2009 Yunhui Fu
 * Licence:     GPL licence version 3
 ******************************************************************************/

#include <string.h> // memset()

//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Box.H>

#include "tilestruct.h"
#include "glutfont.h"

class MyWindow : public Fl_Gl_Window
{
public:
    MyWindow(int width, int height, char* title);
    virtual ~MyWindow();
    virtual void draw();
private:
    void InitializeGL();

    void InitTileSim (void);
    void CleanTileSim (void);
    // the current supertile to be show
    memarr_t    m_tilelist; // 各个 tile 的属性
    memarr_t    m_countlist; // 各个 tile 的个数
    memarr_t    m_birthlist; // 各个 tile 的birth
    char        m_flg_inited;
    int         m_idx_cur; // 当前显示的 tile 序列号
    tssiminfo_t m_tsim;
};

MyWindow::MyWindow(int width, int height, char* title) : Fl_Gl_Window(width, height, title)
{
    mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
    m_flg_inited = 0;
    m_idx_cur = 0;
    InitTileSim ();
}

MyWindow::~MyWindow()
{
    CleanTileSim ();
}

void MyWindow::InitializeGL()
{
    glClearColor(.1f, .1f, .1f, 1);
    glEnable(GL_DEPTH_TEST);

    glutfont_init ();
}

void MyWindow::draw()
{
   static bool firstTime = true;
   if (firstTime)
   {
      InitializeGL();
      firstTime = false;
   }// if

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);      // clear the color and depth buffer

   // view transformations
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-1, 1, -1, 1, 1, 100);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   //gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

   // draw something
   //DrawCube();
}

void
MyWindow::InitTileSim (void)
{
    if (0 == m_flg_inited) {
        m_flg_inited = 1;
        ma_init (&m_tilelist, sizeof (tstilecomb_t));
        ma_init (&m_countlist, sizeof (size_t));
        ma_init (&m_birthlist, sizeof (size_t));

        memset (&m_tsim, 0, sizeof (m_tsim));
        m_tsim.ptilelist = &m_tilelist;
        m_tsim.pcountlist = &m_countlist;
        m_tsim.pbirthlist = &m_birthlist;
    } else {
        ma_rmalldata (&m_tilelist, (memarr_cb_destroy_t)tstc_clear);
        ma_rmalldata (&m_countlist, NULL);
        ma_rmalldata (&m_birthlist, NULL);
        tssiminfo_clean (&m_tsim);
    }

    m_idx_cur = 0;
}

void
MyWindow::CleanTileSim (void)
{
    ma_clear (&m_tilelist, (memarr_cb_destroy_t)tstc_clear);
    ma_clear (&m_countlist, NULL);
    ma_clear (&m_birthlist, NULL);
    tssiminfo_clean (&m_tsim);
}

int main (int argc, char ** argv)
{

   MyWindow myWindow(400, 400, "Supertile Simulation");
   myWindow.show(argc, argv);

   Fl::run();

  return(Fl::run());
}
