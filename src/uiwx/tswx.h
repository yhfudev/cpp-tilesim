/////////////////////////////////////////////////////////////////////////////
// Name:        tilesim.h
// Purpose:     GUI of tilesim
// Author:      Yunhui Fu (yhfudev@gmail.com)
// Modified by:
// Created:     10/28/2008
// RCS-ID:      $Id: $
// Copyright:   (c) Yunhui Fu
// Licence:     LGPL licence
/////////////////////////////////////////////////////////////////////////////
#ifndef _TILESIM_GUI_H_
#define _TILESIM_GUI_H_

#include <wx/defs.h>
#include <wx/app.h>
#include <wx/menu.h>
#include <wx/dcclient.h>
#include <wx/wfstream.h>

#include <wx/glcanvas.h>

#include "tilestruct.h"
#include "tileog.h"

// ---------------------------------------------------------------------------
typedef int (* tssim_cb_presentupdate_t) (void);
/* check for thread */
typedef struct _tsim_thread_t {
    char flg_quit;   /*set by outer, read by thread*/
    char flg_running; /*set by this thread, read by outer*/
    int algorithm;
    wxMutex mutex; // locker of this structure.

    tssim_chkall_info_t chkallinfo;

#if USE_PRESENTATION
    char flg_presentation;
    char flg_pause;
    char flg_paint;
    tssim_adhere_info_t adhereinfo;
    tssim_adhere_info_t resultinfo; // for showing result
    tssim_cb_presentupdate_t cb_update; // callback: update the display
    size_t delay_millisec; // 每步显示的间隔时间
#endif
} tsim_thread_t;

// Define a new application type
class MyApp : public wxApp
{
public:
    bool flg_inserthole;
    bool flg_recorditems;
    int algorithm;
    virtual bool OnInit();
    virtual bool Initialize (int& argc, wxChar **argv);
};

class TestGLCanvas : public wxGLCanvas
{
public:
    TestGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxString& name = wxT("TestGLCanvas"));

    virtual ~TestGLCanvas();

    void SetModel (int model);
    void LoadXML(const wxString& filename);
    void SaveXML(const wxString& filename);
    void Run (void);
    void CancelRun (void);
    void SearchTarget (void) {
        ogl_set_target_tilesim (&m_tsim);
        Refresh ();
    }
    void ShowPage (size_t page) {
        ogl_jump2page (&m_tsim, page);
        Refresh ();
    }
    void ShowNext (void) {
        ogl_current_page_inc (&m_tsim);
        Refresh();
    }
    void ShowPrev (void) {
        ogl_current_page_dec (&m_tsim);
        Refresh();
    }
#if USE_PRESENTATION
    void SwPresent (void) {
        if (m_thinfo.flg_presentation) {
            m_thinfo.flg_presentation = 0;
        } else {
            m_thinfo.flg_presentation = 1;
        }
    }
    void Pause (void) {
        if (m_thinfo.flg_pause) {
            m_thinfo.flg_pause = 0;
        } else {
            m_thinfo.flg_pause = 1;
        }
    }
#endif
    void OnIdle (wxIdleEvent& event) {
        if (m_thinfo.flg_running
          && (! m_thinfo.flg_pause)
#if USE_PRESENTATION
          //&& m_thinfo.flg_presentation
#endif
        )
        {
            if (m_thinfo.flg_paint) {
                m_thinfo.mutex.Lock ();
                m_thinfo.flg_paint = 0;
                m_thinfo.mutex.Unlock ();
                // post paint event
                wxCommandEvent eventCustom(wxEVT_PAINT);
                wxPostEvent(this, eventCustom);
            }
            event.RequestMore();
        }
        event.Skip();
    }
    void SnapshotCurrent (void); // snapshot the current OpenGL screen

protected:
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouse(wxMouseEvent& event);
    void OnKeyUp(wxKeyEvent& event);

private:
    void draw_screen_onlyst (char flg_draw_info);
    void draw_screen (void);
    void ResetProjectionMode();

    wxGLContext* m_glRC;
    bool m_gl_initialized;

    tsim_thread_t m_thinfo;

    void CmdLnParam (void);
    void InitTileSim (void);
    void CleanTileSim (void);
    // the current supertile to be show
    char        m_flg_inited;
    int         m_idx_cur; // 当前显示的 tile 序列号
    tssiminfo_t m_tsim;
    
    void Snap2Png (bool fmt_is_jpeg, bool flg_copy2screen);
    void Snap2Eps (void);

    DECLARE_NO_COPY_CLASS(TestGLCanvas)
    DECLARE_EVENT_TABLE()
};

// Define a new frame type
class MyFrame : public wxFrame
{
public:
    MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
            const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);

    void OnMenuFileOpen(wxCommandEvent& event);
    void OnMenuFileRun(wxCommandEvent& event);
    void OnMenuFileSaveas(wxCommandEvent& event);
    void OnMenuFileExit(wxCommandEvent& event);
    void OnMenuHelpAbout(wxCommandEvent& event);
    void OnClkTbExec( wxCommandEvent& event ){ OnMenuFileRun(event); }
    void OnClkTbStop( wxCommandEvent& event ){ m_canvas->CancelRun(); }
    void OnClkTbBack( wxCommandEvent& event ){ m_canvas->ShowPrev (); }
    void OnClkTbForward( wxCommandEvent& event ){ m_canvas->ShowNext (); }
#if USE_PRESENTATION
    void OnClkTbPresw( wxCommandEvent& event ){ m_canvas->SwPresent (); }
    void OnClkTbPause( wxCommandEvent& event ){ m_canvas->Pause (); }
#endif
    void OnTextEnterJump( wxCommandEvent& event );
    void OnClkTbGotarget( wxCommandEvent& event );
    void OnClkTbSnapshot( wxCommandEvent& event );

    void OnMenuModelAtam (wxCommandEvent& event) { m_canvas->SetModel (TSIM_ALGO_ATAM); }
    void OnMenuModelKtam (wxCommandEvent& event) { m_canvas->SetModel (TSIM_ALGO_KTAM); }
    void OnMenuModel2hand (wxCommandEvent& event) { m_canvas->SetModel (TSIM_ALGO_2HATAM); }

    void SetCanvas(TestGLCanvas *canvas) { m_canvas = canvas; }
    TestGLCanvas *GetCanvas() { return m_canvas; }

private:
    TestGLCanvas *m_canvas;
    wxTextCtrl* m_textCtrlJumpNum;

    DECLARE_EVENT_TABLE()
};

#endif // #ifndef _TILESIM_GUI_H_
