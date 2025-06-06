/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

#define OEMRESOURCE

#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <time.h>
#include <Shlwapi.h>
#include <gdiplus.h>
#include <mapi.h>
#include <UxTheme.h>
#include <io.h>

#include <map>
#include <memory>
#include <string>
using namespace std;

#include <msapi/vsstyle.h>
#include <msapi/vssym32.h>

#include <newpluginapi.h>
#include <m_button.h>
#include <m_chat_int.h>
#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_hotkeys.h>
#include <m_imgsrvc.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_json.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_message.h>

#include <m_folders.h>
#include <m_HTTPServer.h>
#include <m_ftpfile.h>
#include <m_sendss.h>
#include <m_userinfoex.h>
#include <m_cloudfile.h>

#include "ctrl_button.h"
#include "dlg_msgbox.h"
#include "resource.h"
#include "version.h"
#include "CSend.h"
#include "CSendEmail.h"
#include "CSendFile.h"
#include "CSendFTPFile.h"
#include "CSendHTTPServer.h"
#include "CSendCloduFile.h"
#include "CSendHost_ImageShack.h"
#include "CSendHost_uploadpie.h"
#include "CSendHost_imgur.h"
#include "UMainForm.h"
#include "Utils.h"

#define UM_CLOSING	WM_USER+1
#define UM_EVENT	WM_USER+2

// Generic Message Box for Errors
#define MSGERROR(text) MessageBox(NULL, text, L"SendSS", MB_OK | MB_ICONERROR)
#define MSGINFO	(text) MessageBox(NULL, text, L"SendSS", MB_OK | MB_ICONINFORMATION)

typedef struct _MGLOBAL {
	uint32_t   mirandaVersion;          // mirandaVersion
	BOOLEAN PopupActionsExist   : 1; // Popup+ or MS_POPUP_REGISTERACTIONS exist
	BOOLEAN PluginHTTPExist     : 1;	// HTTPServer or MS_HTTP_ACCEPT_CONNECTIONS exist
	BOOLEAN PluginFTPExist      : 1; // FTPFile or MS_FTPFILE_UPLOAD exist
	BOOLEAN PluginCloudFileExist: 1;	// CloudFile or MS_CLOUDFILE_UPLOAD exists

} MGLOBAL, *LPMGLOBAL;

/////////////////////////////////////////////////////////////////////////////////////////

#define ERROR_TITLE TranslateT("SendScreenshot - Error")

// Miranda Database Key
#define MODULENAME "SendSS"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern ATOM g_clsTargetHighlighter;
extern MGLOBAL g_myGlobals;
extern HNETLIBUSER g_hNetlibUser;

enum
{
	ICO_MAIN = 0,
	ICO_MAINXS,
	ICO_TARGET,
	ICO_MONITOR,
	ICO_END_,
	ICO_BTN_HELP = 0,
	ICO_BTN_FOLDER,
	ICO_BTN_DESC,
	ICO_BTN_DESCON,
	ICO_BTN_DEL,
	ICO_BTN_DELON,
	ICO_BTN_ARROWL,
	ICO_BTN_ARROWR,
	ICO_BTN_UPDATE,
	ICO_BTN_OK,
	ICO_BTN_CANCEL,
	ICO_BTN_EDIT,
	ICO_BTN_EDITON,
	ICO_BTN_COPY,
	ICO_BTN_BBC,
	ICO_BTN_BBCLNK,
	ICO_BTN_DOWNARROW,
	ICO_BTN_END_,
};
#define GetIconHandle(ico) ICONS[ico].hIcolib
#define GetIcon(ico) IcoLib_GetIconByHandle(GetIconHandle(ico))
extern IconItem ICONS[ICO_END_];
#define GetIconBtnHandle(ico) ICONS_BTN[ico].hIcolib
#define GetIconBtn(ico) IcoLib_GetIconByHandle(GetIconBtnHandle(ico))
extern IconItem ICONS_BTN[ICO_BTN_END_];

#define PtrIsValid(p)		(((p)!=0)&&(((HANDLE)(p))!=INVALID_HANDLE_VALUE))
#define MIR_FREE(p)			{if (PtrIsValid(p)){mir_free((void*)p);(p)=NULL;}}
#ifdef _DEBUG
#	define DBGMSG(str,...) do{char tmp[1024];sprintf(tmp,str,##__VA_ARGS__);OutputDebugStringA(tmp);}while(0)
#else
#	define DBGMSG(str,...)
#endif

void ComboBox_SelectItem(HWND hCombo, LPARAM data);

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
