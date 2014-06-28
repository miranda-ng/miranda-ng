/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

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

#define WINVER			0x0700
#define _WIN32_WINNT	0x0700
#define _WIN32_IE		0x0601

#define _CRT_SECURE_NO_WARNINGS
#define OEMRESOURCE

#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <time.h>
#include <Shlwapi.h>
#include <gdiplus.h>
#include <mapi.h>
#include <UxTheme.h>

#include <map>
#include <string>
using namespace std;

#ifdef ComboBox_SelectItemData
 // use Workaround for MS bug ComboBox_SelectItemData;
 #undef ComboBox_SelectItemData
#endif

#include <win2k.h>
#include <msapi/vsstyle.h>
#include <msapi/vssym32.h>
#include <newpluginapi.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_popup.h>
#include <m_icolib.h>

#include <m_folders.h>
#include <m_HTTPServer.h>
#include <m_ftpfile.h>
#include <m_sendss.h>
#include <m_userinfoex.h>
#include <m_dropbox.h>

#include "mir_string.h"
#include "mir_icolib.h"
#include "ctrl_button.h"
#include "dlg_msgbox.h"
#include "resource.h"
#include "version.h"
#include "main.h"
#include "CSend.h"
#include "CSendEmail.h"
#include "CSendFile.h"
#include "CSendFTPFile.h"
#include "CSendHTTPServer.h"
#include "CSendDropbox.h"
#include "CSendHost_ImageShack.h"
#include "CSendHost_uploadpie.h"
#include "CSendHost_imgur.h"
#include "DevKey.h"
#include "UMainForm.h"
#include "UAboutForm.h"
#include "Utils.h"

#define UM_CLOSING	WM_USER+1
#define UM_EVENT	WM_USER+2

// Generic Message Box for Errors
#define MSGERROR(text) MessageBox(NULL, text, _T("SendSS"), MB_OK | MB_ICONERROR)
#define MSGINFO	(text) MessageBox(NULL, text, _T("SendSS"), MB_OK | MB_ICONINFORMATION)

typedef struct _MGLOBAL {
	DWORD		mirandaVersion;					// mirandaVersion
	BOOLEAN		PopupExist			: 1;		// Popup or MS_POPUP_ADDPOPUP exist
	BOOLEAN		PopupActionsExist	: 1;		// Popup++ or MS_POPUP_REGISTERACTIONS exist
	BOOLEAN		PluginHTTPExist		: 1;		// HTTPServer or MS_HTTP_ACCEPT_CONNECTIONS exist
	BOOLEAN		PluginFTPExist		: 1;		// FTPFile or MS_FTPFILE_SHAREFILE exist
	BOOLEAN		PluginDropboxExist	: 1;		// Dropbox or MS_DROPBOX_SEND_FILE exists

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
#define MIR_FREE(p)			{if (PtrIsValid(p)){mir_free((void*)p);(p)=NULL;}}

template<class _Elem>
std::basic_string<_Elem> replace(const std::basic_string<_Elem> & Origninal, const std::basic_string<_Elem> & What, const std::basic_string<_Elem> & With)
{
	std::basic_string<_Elem> res;
	size_t l = 0;
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
