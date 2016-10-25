//  ---------------------------------------------------------------------------
//
//  @file       TwPrecomp.h
//  @brief      Precompiled header
//  @author     Philippe Decaudin - http://www.antisphere.com
//  @license    This file is part of the AntTweakBar library.
//              For conditions of distribution and use, see License.txt
//
//  notes:      Private header
//              TAB=4
//
//  ---------------------------------------------------------------------------


#if !defined ANT_TW_PRECOMP_INCLUDED
#define ANT_TW_PRECOMP_INCLUDED

#if defined(_MSC_VER) || defined(_WIN32) || defined(WIN32)
#define _WINDOWS 1
#else
#ifndef _MACOSX
#define _UNIX
#endif
#endif

#if defined _MSC_VER
#   pragma warning(disable: 4514)   // unreferenced inline function has been removed
#   pragma warning(disable: 4710)   // function not inlined
#   pragma warning(disable: 4786)   // template name truncated
#   pragma warning(disable: 4530)   // exceptions not handled
#   define _CRT_SECURE_NO_DEPRECATE // visual 8 secure crt warning
#endif

#include <stdio.h>
#include <assert.h>
#include <math.h>

/* __STRICT_ANSI__ buggers up MinGW, so disable it */
#ifdef __MINGW32__
# undef __STRICT_ANSI__
#endif
#include <float.h>

#if defined(_MSC_VER) && _MSC_VER<=1200
#   pragma warning(push, 3)
#endif
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#if defined(_MSC_VER) && _MSC_VER<=1200
#   pragma warning(pop)
#endif

#if defined(_UNIX)
#   define ANT_UNIX
#   include <X11/cursorfont.h>
#   define GLX_GLXEXT_LEGACY
#   include <GL/glx.h>
#   include <X11/Xatom.h>
#   include <unistd.h>
#   undef _WIN32
#   undef WIN32
#   undef _WIN64
#   undef WIN64
#   undef _WINDOWS
#   undef ANT_WINDOWS
#   undef ANT_OSX
#elif defined(_MACOSX)
#   define ANT_OSX
#   include <unistd.h>
#   include <Foundation/Foundation.h>
#   include <NSImage.h>
#   include <NSCursor.h>
#   undef _WIN32
#   undef WIN32
#   undef _WIN64
#   undef WIN64
#   undef _WINDOWS
#   undef ANT_WINDOWS
#   undef ANT_UNIX
#elif defined(_WINDOWS) || defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#   define ANT_WINDOWS
#   define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#   include <windows.h>
#   include <shellapi.h>
#endif

#if defined(ANT_OSX)
#	include <OpenGL/gl.h>
#else
#	include <GL/gl.h>  // must be included after windows.h
#endif
#define  ANT_OGL_HEADER_INCLUDED


#endif  // !defined ANT_TW_PRECOMP_INCLUDED
