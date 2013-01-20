/*

Miranda IM: the free IM client for Microsoft* Windows*
Copyright 2000-2009 Miranda ICQ/IM project, 

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//#define _CRT_SECURE_NO_WARNINGS
#define WINVER			0x0700
#define _WIN32_WINNT	0x0700
#define _WIN32_IE		0x0601

#define OEMRESOURCE
#define MIRANDA_VER		0x0A00

// Windows includes
#include <windows.h>
#include <Windowsx.h>

// Standard includes
#include <shlobj.h>
//#include <uxtheme.h>
#include <commctrl.h>
#include <stdio.h>
#include <share.h>
#include <time.h>
#include <process.h>
#include <map>
#include <string>
#include <Shlwapi.h>
#include <Wingdi.h>
#include <gdiplus.h>
using namespace std;

// Miranda NG SDK includes
#include <win2k.h>
#include <msapi/vsstyle.h>
#include <msapi/vssym32.h>
#include <newpluginapi.h>	// This must be included first
#include <m_utils.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_file.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_png.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_system.h>
#include <m_popup.h>

// plugins SDK
#include <m_folders.h>
#include <m_HTTPServer.h>
#include <m_ftpfile.h>
#include <m_popup2.h>
#include "icons.h"		//from uiex icon pack

// Project resources
#include "m_sendss.h"
#include "mir_string.h"
#include "mir_icolib.h"
#include "ctrl_button.h"
#include "dlg_msgbox.h"
#include "resource.h"
#include "version.h"

#ifdef ComboBox_SelectItemData
 // use Workaround for MS bug ComboBox_SelectItemData;
 #undef ComboBox_SelectItemData
#endif

#define UM_CLOSING	WM_USER+1
#define UM_EVENT	WM_USER+2
#define UM_TAB1		WM_USER+11

// Generic Message Box for Errors
#define MSGERROR(text) MessageBox(NULL, text, _T("SendSS"), MB_OK | MB_ICONERROR)
#define MSGINFO	(text) MessageBox(NULL, text, _T("SendSS"), MB_OK | MB_ICONINFORMATION)

typedef struct _MGLOBAL {
	DWORD		mirandaVersion;					// mirandaVersion
	BOOLEAN		PopUpExist			: 1;		// Popup or MS_POPUP_ADDPOPUP exist
	BOOLEAN		PopUpActionsExist	: 1;		// Popup++ or MS_POPUP_REGISTERACTIONS exist
	BOOLEAN		PluginHTTPExist		: 1;		// HTTPServer or MS_HTTP_ACCEPT_CONNECTIONS exist
	BOOLEAN		PluginFTPExist		: 1;		// FTPFile or MS_FTPFILE_SHAREFILE exist

} MGLOBAL, *LPMGLOBAL;

//---------------------------------------------------------------------------
#define ERROR_TITLE		TranslateT("SendScreenshot - Error")

// Miranda Database Key
#define SZ_SENDSS		"SendSS"
#define MODNAME			"SendSS"

extern HINSTANCE		hInst;
extern MGLOBAL			myGlobals;
extern HANDLE			hNetlibUser;

#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))

template<class _Elem>
std::basic_string<_Elem> replace(const std::basic_string<_Elem> & Origninal, const std::basic_string<_Elem> & What, const std::basic_string<_Elem> & With)
{
	std::basic_string<_Elem> res;
	size_t l = 0;
	size_t p = 0;

	for (size_t p = Origninal.find(What.c_str(), 0); p != std::basic_string<_Elem>::npos; p = Origninal.find(What.c_str(), l))
	{
		if (l != p)
			res.append(Origninal.c_str() + l, p - l);
		res.append(With);
		l = p + What.length();
	}
	if (l < Origninal.length())
		res.append(Origninal.c_str() + l);

	return res;
}

#endif

/*************************************************************
 *	Uinfobuttonclass module
 */
 
// button styles
#define MBS_DEFBUTTON		0x00001000L			// default button
#define MBS_PUSHBUTTON		0x00002000L			// toggle button
#define MBS_FLAT			0x00004000L			// flat button
#define MBS_DOWNARROW		0x00008000L			// has arrow on the right

#define MBF_UNICODE			1
#ifdef _UNICODE
 #define MBF_TCHAR			MBF_UNICODE
#else
 #define MBF_TCHAR			0
#endif

// BUTTONADDTOOLTIP
// use lParam=MBF_UNICODE to set unicode tooltips
// for lParam=0 the string is interpreted as ansi

// message to explicitly translate the buttons text,
// as it is not done by default translation routine
// wParam=lParam=NULL
#define BUTTONTRANSLATE		(WM_USER+6)

/* UserInfo/MsgBox v0.1.0.4+
Slightly modified version of MButtonClass, to draw both text and icon in a button control
*/
#define UINFOBUTTONCLASS	_T("UInfoButtonClass")

