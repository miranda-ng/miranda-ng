// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#pragma warning( disable : 4503 4786 )


// Windows
#define _WIN32_WINNT 0x501			// for QueueUserAPC
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


// STL
#include <queue>		// for	conference.cpp, messaging.cpp
#include <string>		// for	conference.cpp, userlist.cpp
#include <map>			// for	messaging.cpp
#include <fstream>		// for	userlist.cpp
#include <iostream>		// for	userlist.cpp


// Glib
//
// Subset of libs and includes from Glib v 2.26 from:
// http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.26/glib-dev_2.26.1-1_win32.zip
// http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.26/glib-dev_2.26.1-1_win64.zip
//
//
// dll files needed in main Miranda directory:
//
// x32
// libglib-2.0-0.dll   from http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.26/glib_2.26.1-1_win32.zip
// intl.dll            from http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip
//
// x64
// libglib-2.0-0.dll   from http://ftp.gnome.org/pub/gnome/binaries/win64/glib/2.26/glib_2.26.1-1_win64.zip
// libintl-8.dll       from http://ftp.gnome.org/pub/gnome/binaries/win64/dependencies/gettext-runtime_0.18.1.1-2_win64.zip
// 
#include <glib.h>		//for	meanwhile and session.cpp


// Menwhile
// Sources of Meanwhile v 1.0.2 from
// http://meanwhile.sourceforge.net/
extern "C" {
#include <mw_session.h>
#include <mw_cipher.h>
#include <mw_st_list.h>
//#include <mw_util.h>
#include <mw_service.h>
#include <mw_channel.h>
#include <mw_srvc_im.h>
#include <mw_srvc_aware.h>
#include <mw_srvc_resolve.h>
#include <mw_srvc_store.h>
#include <mw_srvc_place.h>
#include <mw_srvc_ft.h>
#include <mw_srvc_conf.h>
#include <mw_error.h>
#include <mw_message.h>
};


// Miranda
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protoint.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_message.h>
#include <statusmodes.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_ignore.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_utils.h>
#include <m_idle.h>
#include <m_addcontact.h>
#include <m_popup.h>
#include <m_chat.h>
#include <m_genmenu.h>
#include <m_icolib.h>

