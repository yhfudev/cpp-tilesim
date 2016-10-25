/////////////////////////////////////////////////////////////////////////////
// Name:        tilesim.cpp
// Purpose:     GUI of tilesim
// Author:      Yunhui Fu (yhfudev@gmail.com)
// Modified by: Yunhui Fu, 2011-01-21
// Created:     10/28/2008
// RCS-ID:      $Id: $
// Copyright:   (c) Yunhui Fu
// Licence:     GPL licence version 3
/////////////////////////////////////////////////////////////////////////////

// The version of tilesim
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

// The default model (aTAM)
#define DEFAULG_ALG TSIM_ALGO_ATAM

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#ifdef __DARWIN__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif
#include <GL/glut.h>

extern "C"
{
//#include "trackball.h"
}

#include "pfrandom.h"
#include "tilestruct.h"

#include "tswx.h"

enum {
    TS_CMD_RUN = wxID_HIGHEST + 1,
    TS_CMD_MODEL_2HAND = wxID_HIGHEST + 2,
    TS_CMD_MODEL_ATAM = wxID_HIGHEST + 3,
    TS_CMD_MODEL_KTAM = wxID_HIGHEST + 4,
};

// ---------------------------------------------------------------------------

#if USE_PRESENTATION
static void
mysleep (size_t millisec)
{
    /*if (millisec < 1000000) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = millisec;
        select (0, NULL, NULL, NULL, &tv);
        return;
    }*/
    wxThread::Sleep (millisec/1000);
}
#endif

/* check for thread */

static int
tssim_cb_notifyfail_chkallthread_null (void *userdata, size_t cnt_fail)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);

#if DEBUG
    pthinfo->mutex.Unlock(); /* unlock for main gui */
    mysleep (100 /* millisec */);
    pthinfo->mutex.Lock(); /* unlock for main gui */
#endif
    if (pthinfo->flg_quit) {
        fprintf (stderr, "Recieve quit command\n");
        pthinfo->flg_quit = 0;
        return -1;
    }
    return 0;
}

// 当测试两个 supertile 合并失败的次数连续达到1000次的时候需要终止测试，返回程序
static int
tssim_cb_notifyfail_chkallthread (void *userdata, size_t cnt_fail)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    if (cnt_fail > 1000) {
        //return -1;
    }
#if DEBUG
    pthinfo->mutex.Unlock(); /* unlock for main gui */
    mysleep (100 /* millisec */);
    pthinfo->mutex.Lock(); /* unlock for main gui */
#endif
    if (pthinfo->flg_quit) {
        fprintf (stderr, "Recieve quit command\n");
        pthinfo->flg_quit = 0;
        return -1;
    }
    return tssim_cb_notifyfail_chkall (&(pthinfo->chkallinfo), cnt_fail);
}

static int
tssim_cb_resultinfo_chkallthread_single (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    int ret = 0;

    pthinfo->flg_paint = 1;
    pthinfo->mutex.Unlock(); /* unlock for main gui */
    if (pthinfo->flg_quit) {
        fprintf (stderr, "Recieve quit command\n");
        pthinfo->flg_quit = 0;
        ret = -1;
    }
    pthinfo->mutex.Lock();
    return ret;
}

static int
tssim_cb_resultinfo_chkallthread (void *userdata, size_t idx_base, size_t idx_test, tsstpos_t *pstpos, size_t temperature, size_t idx_created, tstilecomb_t *ptc, memarr_t *plist_points_ok)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    int ret = 0;
    size_t num_created;

    if (pthinfo->algorithm != TSIM_ALGO_2HATAM) {
        return tssim_cb_resultinfo_chkallthread_single (userdata, idx_base, idx_test, pstpos, temperature, idx_created, ptc, plist_points_ok);
    }
#if USE_PRESENTATION
    tstilecomb_t *ptc_base;
    tstilecomb_t *ptc_test;
    tstilecombitem_t info_test;
    assert (NULL != pstpos);
    memmove (&info_test, pstpos, sizeof(info_test));
    ptc_base = (tstilecomb_t *)ma_data_lock (&(pthinfo->resultinfo.psim->tilelist), idx_base);
    ptc_test = (tstilecomb_t *)ma_data_lock (&(pthinfo->resultinfo.psim->tilelist), idx_test);
    tssim_cb_initadheredect_ogshow ((void *)(&(pthinfo->resultinfo)), ptc_base, ptc_test, pthinfo->resultinfo.psim->temperature, &info_test);
#endif
    pthinfo->flg_paint = 1;

    pthinfo->mutex.Unlock(); /* unlock for main gui */
    if (pthinfo->flg_quit) {
        fprintf (stderr, "Recieve quit command\n");
        pthinfo->flg_quit = 0;
        ret = -1;
    }
    if (NULL != ptc) {
        pthinfo->chkallinfo.num_supertiles --;
    }

    if ((NULL != pthinfo->chkallinfo.ptsim->ptarget) && (slist_size (&(pthinfo->chkallinfo.ptsim->ptarget->tbuf)) > 0) && (NULL != ptc)) {
        tssim_chkall_info_t *ptci = &(pthinfo->chkallinfo);
        if (NULL == ptc) {
            ptc = (tstilecomb_t *) ma_data_lock (&(ptci->ptsim->tilelist), idx_created);
            assert (NULL != ptc);
            if (tstc_is_equal (ptci->ptsim->ptarget, ptc, ptci->ptsim->flg_norotate)) {
                ma_data (&(ptci->ptsim->countlist), idx_created, &num_created);
                if (num_created * slist_size (&(ptc->tbuf)) > ((ptci->ptsim->num_tiles) / 2) ) {
                    fprintf (stderr, "----------- target supertile 50% birth = %d\n", ptci->ptsim->year_current);
                    if ((ptci->ptsim->birth_target < 1) && (ptci->ptsim->ptarget)) {
                        ptci->ptsim->birth_target = ptci->ptsim->year_current;
                    }
                    ret = -1;
                }
            }
            ma_data_unlock (&(ptci->ptsim->tilelist), idx_created);
#if 0
        } else {
            if (tstc_is_equal (ptci->ptsim->ptarget, ptc, ptci->ptsim->flg_norotate)) {
                // new item, check it
                fprintf (stderr, "----------- find the target !!! birth = %d\n", ptci->ptsim->year_current);
                //ret = -1;
            }
#endif
        }
    }
#if USE_PRESENTATION
    if (pthinfo->flg_presentation) {
        if (pthinfo->delay_millisec != 0) {
            // delay
            mysleep (pthinfo->delay_millisec * 20);
        }
    }
#endif
    pthinfo->mutex.Lock();

#if USE_PRESENTATION
    tssim_cb_clearadheredect_ogshow ((void *)(&(pthinfo->resultinfo)));
    ma_data_unlock (&(pthinfo->resultinfo.psim->tilelist), idx_base);
    ma_data_unlock (&(pthinfo->resultinfo.psim->tilelist), idx_test);
#endif

    return ret;
}

static int
tssim_cb_getdata_chkallthread (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    return tssim_cb_getdata_chkall (&(pthinfo->chkallinfo), pcountlist, pyear_cur, pidx_base, pidx_test);
}

static int
tssim_cb_getdata_timeofsquare_thread (void *userdata, const memarr_t *pcountlist, size_t *pyear_cur, size_t *pidx_base, size_t *pidx_test)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    return tssim_cb_getdata_timeofsquare (&(pthinfo->chkallinfo), pcountlist, pyear_cur, pidx_base, pidx_test);
}

#if USE_PRESENTATION
#define THD_CHK_PAUSE(pthinfo) \
while ((pthinfo)->flg_pause) { \
    (pthinfo)->mutex.Unlock(); \
    mysleep (300000); \
    (pthinfo)->mutex.Lock(); \
}

int
tssim_cb_initadheredect_chkallthread (void *userdata, tstilecomb_t *tc_base, tstilecomb_t *tc_test, int temperature, tstilecombitem_t * pinfo_test)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        THD_CHK_PAUSE (pthinfo);
        return tssim_cb_initadheredect_ogshow (&(pthinfo->adhereinfo), tc_base, tc_test, temperature, pinfo_test);
    }
    return 0;
}

int
tssim_cb_adherepos_chkallthread (void *userdata, char flg_canadhere, tstilecombitem_t * pposinfo)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        THD_CHK_PAUSE (pthinfo);
        tssim_cb_adherepos_ogshow (&(pthinfo->adhereinfo), flg_canadhere, pposinfo);
        pthinfo->flg_paint = 1;

        pthinfo->mutex.Unlock(); /* unlock for main gui */
        if (pthinfo->cb_update) {
            pthinfo->cb_update ();
        }

        if (pthinfo->delay_millisec != 0) {
            mysleep (pthinfo->delay_millisec);
        }

        pthinfo->mutex.Lock();
    }
    return 0;
}

int
tssim_cb_clearadheredect_chkallthread (void *userdata)
{
    tsim_thread_t *pthinfo = (tsim_thread_t *)userdata;
    assert (NULL != pthinfo);
    if (pthinfo->flg_presentation) {
        THD_CHK_PAUSE (pthinfo);
        return tssim_cb_clearadheredect_ogshow (&(pthinfo->adhereinfo));
    }
    return 0;
}
#endif

int
ts_simulate_thread (tssiminfo_t *ptsim, tsim_thread_t *pthinfo)
{
    assert (NULL != ptsim);
    assert (NULL != pthinfo);

    pthinfo->mutex.Lock();
    pthinfo->flg_running = 1;

#if USE_PRESENTATION
    memset (&(pthinfo->adhereinfo), 0, sizeof (pthinfo->adhereinfo));
    memset (&(pthinfo->resultinfo), 0, sizeof (pthinfo->resultinfo));
    pthinfo->adhereinfo.psim = ptsim;
    pthinfo->resultinfo.psim = ptsim;
    ptsim->cb_initadheredect  = tssim_cb_initadheredect_chkallthread;
    ptsim->cb_adherepos       = tssim_cb_adherepos_chkallthread;
    ptsim->cb_clearadheredect = tssim_cb_clearadheredect_chkallthread;
#endif

    memset (&(pthinfo->chkallinfo), 0, sizeof (pthinfo->chkallinfo));
    pthinfo->chkallinfo.idx_base = pthinfo->chkallinfo.cur_max + 1;
    pthinfo->chkallinfo.ptsim = ptsim;
    pthinfo->chkallinfo.num_supertiles = ts_get_total_supertiles (ptsim);

    printf ("**** Start Simulation (thread)****\n");
    ptsim->cb_getdata       = NULL;
    ptsim->cb_selectone     = NULL;
    ptsim->cb_resultinfo    = tssim_cb_resultinfo_chkallthread;
    ptsim->cb_notifyfail    = tssim_cb_notifyfail_chkallthread;
    ptsim->cb_findmergepos  = NULL;
    ptsim->cb_storemergepos = NULL;
    ptsim->userdata = pthinfo;

    // time step testing
    ptsim->cb_getdata    = tssim_cb_getdata_timeofsquare_thread;

    //if (0 == ptsim->flg_single_supertile_mode) {
        ts_simulate_main (ptsim, "tswx");
    if (0) {//} else {
        printf ("**** Check the remains (thread)****\n");
        memset (&(pthinfo->chkallinfo), 0, sizeof (pthinfo->chkallinfo));
        pthinfo->chkallinfo.cur_max = ma_size (&(ptsim->countlist));
        pthinfo->chkallinfo.ptsim = ptsim;
        pthinfo->chkallinfo.num_supertiles = ts_get_total_supertiles (ptsim);
        // check the remains:
        ptsim->cb_notifyfail = NULL;
        ptsim->cb_getdata    = tssim_cb_getdata_chkallthread;
        ptsim->cb_resultinfo = tssim_cb_resultinfo_chkallthread;
        ptsim->cb_notifyfail    = tssim_cb_notifyfail_chkallthread_null;
        ts_simulate_main (ptsim, "tswxchk");
    }

    pthinfo->flg_running = 0;
    pthinfo->mutex.Unlock();

    printf ("**** End of Simulation (thread)****\n");
    return 0;
}

// ---------------------------------------------------------------------------

class MyThread : public wxThread
{
public:
    MyThread(MyFrame *frame, TestGLCanvas *canvas);
    virtual ~MyThread();

    // thread execution starts here
    virtual void *Entry();

    // write something to the text control in the main frame
    void WriteText(const wxString& text)
    {
        //m_frame->LogThreadMessage(text);
    }

public:
    MyFrame *m_frame;
    TestGLCanvas *m_canvas;
};

MyThread::MyThread(MyFrame *frame, TestGLCanvas *canvas)
        : wxThread(wxTHREAD_DETACHED)
{
    m_frame = frame;
    m_canvas = canvas;
}

MyThread::~MyThread()
{
}

void *MyThread::Entry()
{
    m_canvas->Run ();
    return NULL;
}

// ---------------------------------------------------------------------------
// MyApp
// ---------------------------------------------------------------------------

// `Main program' equivalent, creating windows and returning main app frame
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    wxImage::AddHandler( new wxPNGHandler );
    wxImage::AddHandler( new wxJPEGHandler );

    // Create the main frame window
    wxSize size(500, 400);
    MyFrame *frame = new MyFrame(NULL, wxT("Supertile Simulation System"),
        wxDefaultPosition, size);

//#if wxUSE_ZLIB
//    if (wxFileExists(wxT("penguin.dxf.gz")))
//        frame->GetCanvas()->LoadDXF(wxT("penguin.dxf.gz"));
//#else
//    if (wxFileExists(wxT("penguin.dxf")))
//        frame->GetCanvas()->LoadDXF(wxT("penguin.dxf"));
//#endif

    /* Show the frame */
    frame->Show(true);

    return true;
}

bool
MyApp::Initialize (int& argc, wxChar **argv)
{
    flg_inserthole = false;
    flg_recorditems = false;
    algorithm = DEFAULG_ALG;

    int argcOrig = argc;
    for ( int i = 0; i < argcOrig; i++ ) {
        if (0 == wxStrcmp (argv[i], wxT("-d"))) {
            if (i < (argcOrig - 1)) {
                argv[i++] = NULL;
                set_debug_level (wxAtoi (argv[i]));
                argv[i] = NULL;
                argc -= 2;
            }
        } else if (0 == wxStrcmp (argv[i], wxT("-a"))) {
            if (i < (argcOrig - 1)) {
                argv[i++] = NULL;
                set_debug_catlog (wxAtoi (argv[i]));
                argv[i] = NULL;
                argc -= 2;
            }
        } else if (0 == wxStrcmp (argv[i], wxT("-h"))) {
            flg_inserthole = true;
            argv[i] = NULL;
            argc --;
        } else if (0 == wxStrcmp (argv[i], wxT("-r"))) {
            flg_recorditems = true;
            argv[i] = NULL;
            argc --;
        } else if (0 == wxStrcmp (argv[i], wxT("-g"))) {
            if (i < (argcOrig - 1)) {
                argv[i++] = NULL;
                wxString algo = argv[i];
                algorithm = tssim_algo_cstr2type (algo.utf8_str ());
                if (algorithm < 0) {
                    //printf ("unsupport algorithm: '%s'\n", algo.utf8_str ());
                    exit (1);
                } else {
                    //printf ("change to algorithm: (%d)'%s'\n", algorithm, algo.utf8_str ());
                }
                argv[i] = NULL;
                argc -= 2;
            }
        }
    }
    if (argc != argcOrig) {
        // remove the argumens we consumed
        for ( int i = 0; i < argc; i++ ) {
            while (! argv[i]) {
                memmove(argv + i, argv + i + 1, (argcOrig - i) * sizeof (argv[0]));
            }
        }
    }

    if (! wxApp::Initialize(argc, argv)) {
        return false;
    }

    return true;
}

IMPLEMENT_APP(MyApp)

// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------
#define MID_OPEN    1002
#define MID_EXEC    1003
#define MID_STOP    1004
#define MID_BACK    1005
#define MID_FORWARD 1006
#if USE_PRESENTATION
#define MID_SWPRES  1007
#define MID_PAUSE   1008
#endif
#define MID_TBJUMPINPUT 1009 /* jump to the page user input */
#define MID_GOTARGET    1010 /* search the item equal to target supertile */
#define MID_SNAPSHOT    1011 /* snapshot the OpenGL screen to a .eps file */

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_OPEN, MyFrame::OnMenuFileOpen)
    EVT_MENU(TS_CMD_RUN, MyFrame::OnMenuFileRun)
    EVT_MENU(TS_CMD_MODEL_ATAM, MyFrame::OnMenuModelAtam)
    EVT_MENU(TS_CMD_MODEL_KTAM, MyFrame::OnMenuModelKtam)
    EVT_MENU(TS_CMD_MODEL_2HAND, MyFrame::OnMenuModel2hand)
    EVT_MENU(wxID_SAVEAS, MyFrame::OnMenuFileSaveas)
    EVT_MENU(wxID_EXIT, MyFrame::OnMenuFileExit)
    EVT_MENU(wxID_HELP, MyFrame::OnMenuHelpAbout)
    EVT_TOOL(MID_OPEN, MyFrame::OnMenuFileOpen)
	EVT_TOOL(MID_EXEC, MyFrame::OnClkTbExec )
	EVT_TOOL(MID_STOP, MyFrame::OnClkTbStop )
	EVT_TOOL(MID_BACK, MyFrame::OnClkTbBack )
	EVT_TOOL(MID_FORWARD, MyFrame::OnClkTbForward )
	EVT_TEXT(MID_TBJUMPINPUT, MyFrame::OnTextEnterJump)
	EVT_TEXT_ENTER(MID_TBJUMPINPUT, MyFrame::OnTextEnterJump)
	EVT_TOOL(MID_GOTARGET, MyFrame::OnClkTbGotarget )
	EVT_TOOL(MID_SNAPSHOT, MyFrame::OnClkTbSnapshot )
#if USE_PRESENTATION
    EVT_MENU(MID_SWPRES, MyFrame::OnClkTbPresw)
	EVT_TOOL(MID_PAUSE, MyFrame::OnClkTbPause )
#endif
END_EVENT_TABLE()

#include "../pixmaps/bt_docopen.xpm"
#include "../pixmaps/bt_run.xpm"
#include "../pixmaps/bt_runstop.xpm"
#include "../pixmaps/bt_pageprevious.xpm"
#include "../pixmaps/bt_pagenext.xpm"
#include "../pixmaps/bt_targetsearch.xpm"
#if USE_PRESENTATION
#include "../pixmaps/bt_presentationpause.xpm"
#include "../pixmaps/bt_presentation.xpm"
#endif // USE_PRESENTATION
#include "../pixmaps/bt_digikam.xpm"

// MyFrame constructor
MyFrame::MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{
    //SetIcon(wxIcon((char **)sample_xpm));

    // Make the "File" menu
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_OPEN, wxT("&Open..."));
    fileMenu->Append(TS_CMD_RUN, wxT("&Run..."));
    fileMenu->Append(wxID_SAVEAS, wxT("Save&as..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxT("E&xit\tALT-X"));

    // Make the "Model" menu
    wxMenu *modelMenu = new wxMenu;
    modelMenu->AppendRadioItem (TS_CMD_MODEL_ATAM, wxT("aTAM"));
    modelMenu->AppendRadioItem (TS_CMD_MODEL_KTAM, wxT("kTAM"));
    modelMenu->AppendRadioItem (TS_CMD_MODEL_2HAND, wxT("2handed"));

    // Make the "Help" menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_HELP, wxT("&About..."));

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(modelMenu, wxT("&Model"));
    menuBar->Append(helpMenu, wxT("&Help"));
    this->SetMenuBar(menuBar);

    wxToolBar* m_toolBar1;
    wxStatusBar* statusBar;
	m_toolBar1 = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	m_toolBar1->AddTool( MID_OPEN,    wxT("tool"), wxBitmap( bt_docopen_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Open a XML data file"), wxT("Open a XML data file") );

	m_toolBar1->AddSeparator();
	m_toolBar1->AddTool( MID_SNAPSHOT,   wxT("tool"), wxBitmap( bt_digikam_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Snapshot the OpenGL screen"), wxT("Snapshot the OpenGL screen") );

	m_toolBar1->AddSeparator();
	m_toolBar1->AddTool( MID_EXEC,    wxT("tool"), wxBitmap( bt_run_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Excute the simulation"), wxT("Excute the simultion") );
	m_toolBar1->AddTool( MID_STOP,    wxT("tool"), wxBitmap( bt_runstop_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Stop the simulation"), wxT("Stop the simulation") );
	m_toolBar1->AddTool( MID_BACK,    wxT("tool"), wxBitmap( bt_pageprevious_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Back"), wxT("Back") );
	m_toolBar1->AddTool( MID_FORWARD, wxT("tool"), wxBitmap( bt_pagenext_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Forward"), wxT("Forward") );

	m_toolBar1->AddSeparator();
	m_toolBar1->AddTool( MID_GOTARGET,   wxT("tool"), wxBitmap( bt_targetsearch_xpm ), wxNullBitmap, wxITEM_NORMAL, wxT("Search the target"), wxT("Search the target") );

	wxStaticText* pstaticText1 = new wxStaticText( m_toolBar1, wxID_ANY, wxT("JumpTo："), wxDefaultPosition, wxDefaultSize, 0 );
	pstaticText1->Wrap( -1 );
	m_toolBar1->AddControl( pstaticText1 );
	m_textCtrlJumpNum = new wxTextCtrl( m_toolBar1, MID_TBJUMPINPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_toolBar1->AddControl( m_textCtrlJumpNum );

#if USE_PRESENTATION
	m_toolBar1->AddSeparator();
	m_toolBar1->AddTool( MID_SWPRES,  wxT("tool"), wxBitmap( bt_presentation_xpm ), wxNullBitmap, wxITEM_CHECK, wxT("Presentation"), wxT("Presentation") );
	m_toolBar1->AddTool( MID_PAUSE,   wxT("tool"), wxBitmap( bt_presentationpause_xpm ), wxNullBitmap, wxITEM_CHECK, wxT("Pause"), wxT("Pause presentation") );
#endif

	m_toolBar1->Realize();

	statusBar = this->CreateStatusBar( 2, wxST_SIZEGRIP, wxID_ANY );

    this->Show(true);

    //init_seed ();

    m_canvas = new TestGLCanvas(this, wxID_ANY, wxDefaultPosition,
        GetClientSize(), wxSUNKEN_BORDER);
}

// File|Open... command
void MyFrame::OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) )
{
    wxString filename = wxFileSelector(wxT("Choose TileSim Data File"), wxT(""), wxT(""), wxT(""),
        wxT("TileSim (*.xml)|*.xml|TAS (*.tdp)|*.tdp|All files (*.*)|*.*"),
        wxFD_OPEN);
    if (!filename.IsEmpty())
    {
        m_canvas->LoadXML (filename);
        this->SetStatusText (filename.AfterLast('/'));
        m_canvas->Refresh (false);
    }
}

void MyFrame::OnMenuFileRun(wxCommandEvent& event)
{
    MyThread *thread = new MyThread(this, m_canvas);
    if ( thread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError(wxT("Can't create thread!"));
    }
    thread->Run ();
}

void MyFrame::OnMenuFileSaveas(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(wxT("Choose TileSim Data File"), wxT(""), wxT(""), wxT(""),
        wxT("TileSim (*.xml)|*.xml|All files (*.*)|*.*"),
        wxFD_SAVE);
    if (!filename.IsEmpty())
    {
        m_canvas->SaveXML (filename);
    }
}

void MyFrame::OnTextEnterJump (wxCommandEvent& event)
{
    wxString word = m_textCtrlJumpNum->GetValue();
    word.Trim ();
    long page = 0;
    word.ToLong (&page);
    m_canvas->ShowPage (page);
}

void MyFrame::OnClkTbGotarget (wxCommandEvent& event)
{
    m_canvas->SearchTarget ();
}

void MyFrame::OnClkTbSnapshot (wxCommandEvent& event)
{
    // snapshot the OpenGL screen
    m_canvas->SnapshotCurrent ();
}

// File|Exit command
void MyFrame::OnMenuFileExit( wxCommandEvent& WXUNUSED(event) )
{
    // true is to force the frame to close
    Close(true);
}

// Help|About... command
void MyFrame::OnMenuHelpAbout( wxCommandEvent& WXUNUSED(event) )
{
    wxString msg = wxT("Suptertile Simulation System (c) Yunhui Fu");
    msg << wxT("\nVersion: ") << VERSION_MAJOR << wxT(".") << VERSION_MINOR << wxT("; Build Date: ") << wxT(__DATE__);
    wxMessageBox(msg);
}

// ---------------------------------------------------------------------------
// TestGLCanvas
// ---------------------------------------------------------------------------

BEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_SIZE(TestGLCanvas::OnSize)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(TestGLCanvas::OnEraseBackground)
    EVT_MOUSE_EVENTS(TestGLCanvas::OnMouse)
    EVT_KEY_DOWN(TestGLCanvas::OnKeyUp)
    EVT_IDLE(TestGLCanvas::OnIdle)
END_EVENT_TABLE()

TestGLCanvas::TestGLCanvas(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
    : wxGLCanvas(parent, id, NULL, pos, size,
                 style | wxFULL_REPAINT_ON_RESIZE, name)
{
    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);

    // Make the new context current (activate it for use) with this canvas.
    SetCurrent(*m_glRC);

    m_gl_initialized = false;

    m_flg_inited = 0;
    m_idx_cur = 0;
    InitTileSim ();

    m_thinfo.flg_quit = 0;
    m_thinfo.flg_running = 0;
    m_thinfo.algorithm = DEFAULG_ALG;
#if USE_PRESENTATION
    m_thinfo.flg_presentation = 0;
    m_thinfo.flg_pause = 0;
    m_thinfo.flg_paint = 0;
    m_thinfo.cb_update = NULL;
    m_thinfo.delay_millisec = 600000;//150000;//100000;
#endif
}

TestGLCanvas::~TestGLCanvas()
{
    CancelRun ();
    delete m_glRC;
    ogl_clear ();
    CleanTileSim ();
}


void TestGLCanvas::SetModel (int model) {
    MyApp &app = wxGetApp ();
    app.algorithm = model;
    TSIM_SIM_ALGO (&m_tsim, app.algorithm);
    m_thinfo.algorithm = app.algorithm;
}

void
TestGLCanvas::CmdLnParam (void)
{
    MyApp &app = wxGetApp ();
    if (app.flg_inserthole) {
        TSIM_MESHTEST_MODE (&m_tsim, 1);
    }
    if (app.flg_recorditems) {
        TSIM_RECORD_MODE (&m_tsim, 1);
    }
    TSIM_SIM_ALGO (&m_tsim, app.algorithm);
    m_thinfo.algorithm = app.algorithm;
}

void
TestGLCanvas::InitTileSim (void)
{
    if (0 == m_flg_inited) {
        m_flg_inited = 1;
        tssiminfo_init (&m_tsim);
        CmdLnParam ();
    } else {
        tssiminfo_reset (&m_tsim);
    }
    m_idx_cur = 0;
}

void
TestGLCanvas::CleanTileSim (void)
{
    tssiminfo_clear (&m_tsim);
}

// 只画 supertile
void TestGLCanvas::draw_screen_onlyst (char flg_draw_info)
{
    // Render the graphics and swap the buffers.
    m_thinfo.mutex.Lock ();
    //m_thinfo.flg_paint = 0;
#if USE_PRESENTATION
#define PRES_ISSHOW_RESULT(p) (NULL != (p)->tc_base)
    if (m_thinfo.flg_presentation && m_thinfo.flg_running) {
        if (PRES_ISSHOW_RESULT (&(m_thinfo.resultinfo))) {
            tssim_adhere_display_ogshow (&(m_thinfo.resultinfo), "Result:");
        } else if (PRES_ISSHOW_RESULT (&(m_thinfo.adhereinfo))) {
            tssim_adhere_display_ogshow (&(m_thinfo.adhereinfo), "Testing:");
        }
    } else
#endif
    {
        ogl_draw_current_tilesim (&m_tsim, flg_draw_info);
    }
    m_thinfo.mutex.Unlock ();

}

void TestGLCanvas::draw_screen (void)
{
    // Initialize OpenGL
    if (!m_gl_initialized)
    {
        ogl_initgl ((ogl_cb_postredisplay_t)glFlush);
        m_gl_initialized = true;
    }
    //if ((wxGetApp ().flg_singlest) && (m_tsim.idx_last > 0) && (m_thinfo.flg_running)) {
        //ogl_jump2page (&m_tsim, m_tsim.idx_last);
    //}
    if ((wxGetApp ().algorithm != TSIM_ALGO_2HATAM) && (m_thinfo.flg_running)) {
        ogl_jump2page (&m_tsim, TS_NUMTYPE_SUPERTILE(&m_tsim) + 1);
    }

    ResetProjectionMode ();

    draw_screen_onlyst (1);
    ogl_drawother ();

    // Flush
    glFlush();
}

void TestGLCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    SetCurrent(*m_glRC);

    // must always be here
    wxPaintDC dc(this);

    draw_screen ();

    // Swap
    SwapBuffers();
}

void TestGLCanvas::OnSize(wxSizeEvent& WXUNUSED(event))
{
    // Reset the OpenGL view aspect.
    // This is OK only because there is only one canvas that uses the context.
    // See the cube sample for that case that multiple canvases are made current with one context.
    ResetProjectionMode();
}

void TestGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    // Do nothing, to avoid flashing on MSW
}

void TestGLCanvas::LoadXML(const wxString& filename)
{
    if (m_thinfo.flg_running) {
        // could not load the data when runing
        return;
    }
    if (filename == wxT("")) {
        return;
    }
    InitTileSim ();
    if (ts_sim_load_datafile (&(this->m_tsim), filename.ToUTF8 ()) < 0) {
        DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "Error in read xml data file");
    }
    CmdLnParam ();

}

void TestGLCanvas::SaveXML(const wxString& filename)
{
    if (m_thinfo.flg_running) {
        // could not save the data when runing
        return;
    }
    if (filename == wxT("")) {
        return;
    }
    ts_sim_save_data_xml (&(this->m_tsim), filename.ToUTF8 ());
}

void TestGLCanvas::Run (void)
{
    if (m_thinfo.flg_running) {
        return;
    }
    MyApp &app = wxGetApp ();
    TSIM_SIM_ALGO (&m_tsim, app.algorithm);
    m_thinfo.algorithm = app.algorithm;
    ts_simulate_thread (&(this->m_tsim), &m_thinfo);
}

void TestGLCanvas::CancelRun (void)
{
    int cnt = 5;
    fprintf (stderr, "Start of CancelRun ...\n");
    m_thinfo.flg_quit = 1;
    //while (m_thinfo.flg_running && cnt > 0) {
    //    cnt --;
    //    wxThread::Sleep (200);
    //}
    //m_thinfo.flg_quit = 0;
    if (m_thinfo.flg_running) {
        // fail
    }
}

void TestGLCanvas::OnMouse(wxMouseEvent& event)
{
    if (event.Dragging() || event.Moving())
    {
        //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse dragging/moving: %d, %d", event.GetX(), event.GetY());
        ogl_on_mouse_motion (event.GetX(), event.GetY());

        /* orientation has changed, redraw mesh */
        Refresh (false);
    } else {
        if (event.m_leftDown) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse left down: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_LEFT_BUTTON, GLUT_DOWN, event.GetX(), event.GetY());
        } else if (event.m_middleDown) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse middle down: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_MIDDLE_BUTTON, GLUT_DOWN, event.GetX(), event.GetY());
        } else if (event.m_rightDown) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse right down: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_RIGHT_BUTTON, GLUT_DOWN, event.GetX(), event.GetY());
        } else if (event.LeftUp()) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse left up: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_LEFT_BUTTON, GLUT_UP, event.GetX(), event.GetY());
        } else if (event.MiddleUp()) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse middle up: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_MIDDLE_BUTTON, GLUT_UP, event.GetX(), event.GetY());
        } else if (event.RightUp()) {
            //DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "mouse right up: %d, %d", event.GetX(), event.GetY());
            ogl_on_mouse (GLUT_RIGHT_BUTTON, GLUT_UP, event.GetX(), event.GetY());
        }
    }
}

void TestGLCanvas::OnKeyUp(wxKeyEvent& event)
{
    int kmod;
    switch (event.GetModifiers()) {
    case wxMOD_CONTROL:
        kmod = GLUT_ACTIVE_CTRL;
        break;
    case wxMOD_SHIFT:
        kmod = GLUT_ACTIVE_SHIFT;
        break;
    case wxMOD_ALT:
        kmod = GLUT_ACTIVE_ALT;
        break;
    }
    DBGMSG (PFDBG_CATLOG_APP, PFDBG_LEVEL_ERROR, "ogl_on_keyboard (%d, %d)", event.GetKeyCode(), kmod);
    ogl_on_keyboard (event.GetKeyCode (), kmod);
    switch (event.GetKeyCode ()) {
    case 27: /* 'ESC' */
    case 'q':
        exit (0);
        break;
    case 'p': /* Previous item */
    case '<':
    case ',':
        ogl_current_page_dec (&m_tsim);
        Refresh(false);
        break;
    case 'n': /* Next item */
    case '>':
    case '.':
        ogl_current_page_inc (&m_tsim);
        Refresh(false);
        break;
    }
}

void TestGLCanvas::ResetProjectionMode()
{
    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent(*m_glRC);

    //int w, h; GetClientSize(&w, &h);
    const wxSize ClientSize = GetClientSize();

    // It's up to the application code to update the OpenGL viewport settings.
    // In order to avoid extensive context switching, consider doing this in
    // OnPaint() rather than here, though.
    ogl_reshape (ClientSize.GetWidth(), ClientSize.GetHeight());
}

#include <stdio.h> // FILE *tmpfile(void);
#include <stdlib.h> // int mkstemp(char *template);

#include <wx/clipbrd.h> // wxTheClipboard

#include "gl2ps.h"

#if defined(_WIN32)
#include <fcntl.h>
#define _S_IREAD 256
#define _S_IWRITE 128
int mkstemp (char *tmpl)
{
    int ret;
    mktemp (tmpl);
    ret = open (tmpl, O_RDWR|O_BINARY|O_CREAT|O_EXCL|_O_SHORT_LIVED, _S_IREAD|_S_IWRITE);
    return ret;
}
#else
//The mkstemp() function generates a unique temporary filename from template. The last six characters of template must be XXXXXX and these are replaced with a string that makes the filename unique. The file is then created with mode read/write and permissions 0666 (glibc 2.0.6 and earlier), 0600 (glibc 2.0.7 and later). Since it will be modified, template must not be a string constant, but should be declared as a character array. The file is opened with the open(2) O_EXCL flag, guaranteeing that when mkstemp() returns successfully we are the only user.
static FILE *
my_mktmpfile (char *cstr_template)
{
    FILE *fp;
    int fd = mkstemp (cstr_template);
    if (fd < 0) {
        return NULL;
    }
    fp = fdopen (fd, "wb+");
    if (NULL == fp) {
        close (fd);
    }
    return fp;
}
#endif

static int
my_tmpfile_date (const wxChar *suff, wxString &buff)
{
    wxDateTime now = wxDateTime::Now();
    buff = now.Format (wxT("sss_%Y-%m-%d_%H.%M.%S"), wxDateTime::CET);
    buff += suff;
    return 0;
}

/* Create a Screenshot of the current 3D view.
 *  Output file format is png or jpeg, or image is copied on clipboard
 */
void TestGLCanvas::Snap2Png (bool fmt_is_jpeg, bool flg_copy2screen)
{
    wxString FullFileName;
    if (fmt_is_jpeg) {
        my_tmpfile_date (wxT(".jpg"), FullFileName);
    } else {
        my_tmpfile_date (wxT(".png"), FullFileName);
    }

    draw_screen_onlyst (0);

    wxSize     image_size = GetClientSize();
    wxClientDC dc( this );
    wxBitmap   bitmap( image_size.x, image_size.y );
    wxMemoryDC memdc;
    memdc.SelectObject( bitmap );
    memdc.Blit( 0, 0, image_size.x, image_size.y, &dc, 0, 0 );
    memdc.SelectObject( wxNullBitmap );

    if (flg_copy2screen) {
        wxBitmapDataObject* dobjBmp = new wxBitmapDataObject;
        dobjBmp->SetBitmap( bitmap );
        if( wxTheClipboard->Open() )
        {
            if( !wxTheClipboard->SetData( dobjBmp ) )
                wxLogError( _T( "Failed to copy image to clipboard" ) );
            wxTheClipboard->Flush();    /* the data on clipboard
                                         *  will stay available after the application exits */
            wxTheClipboard->Close();
        }
    } else {
        wxImage image = bitmap.ConvertToImage();

        if( !image.SaveFile( FullFileName,
                             fmt_is_jpeg ? wxBITMAP_TYPE_JPEG : wxBITMAP_TYPE_PNG ) )
            wxLogError( wxT( "Can't save file" ) );

        image.Destroy();
    }
}

void TestGLCanvas::Snap2Eps (void)
{
    FILE *fp;
    char fname[200];
    int state = GL2PS_OVERFLOW;
    int buffsize = 0;
    wxString FullFileName;
    if (my_tmpfile_date (wxT(".eps"), FullFileName) < 0) {
        return;
    }
    strncpy (fname, FullFileName.utf8_str (), sizeof (fname) / sizeof(char) - 1);

    fp = fopen (fname, "wb+");
    if (NULL == fp) {
        printf ("Error in open file: %s\n", fname);
        return;
    }
    while (state == GL2PS_OVERFLOW) {
        buffsize += 1024*1024;
        gl2psBeginPage ("Snapshot of SSS", SSSVER_COPYRIGHT, NULL, GL2PS_EPS, GL2PS_SIMPLE_SORT,
            GL2PS_DRAW_BACKGROUND | GL2PS_USE_CURRENT_VIEWPORT,
            GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp, fname);
        draw_screen_onlyst (0);
        state = gl2psEndPage ();
    }
    fclose (fp);
    printf ("out to '%s', Done!\n", fname);
}

void TestGLCanvas::SnapshotCurrent (void)
{
    Snap2Png (false, false);
    //Snap2Eps ();
}
