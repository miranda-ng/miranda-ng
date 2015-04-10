/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _CHAT_H_
#define _CHAT_H_

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501

#include <shlobj.h>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <ole2.h>
#include <richole.h>
#include <commdlg.h>
#include <shellapi.h>
#include <Initguid.h>
#include <Oleacc.h>
#include <Uxtheme.h>

#include <malloc.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_button.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_clui.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_popup.h>
#include <m_chat_int.h>
#include <m_core.h>
#include <m_string.h>
#include <m_fontservice.h>

#include "m_ieview.h"
#include "m_smileyadd.h"

#include "resource.h"
#include "version.h"

#define EM_SUBCLASSED          (WM_USER+200)
#define EM_ACTIVATE            (WM_USER+201)

struct TABLIST
{
	TCHAR *pszID;
	char *pszModule;
	TABLIST *next;
};

struct MODULEINFO : public GCModuleInfoBase
{
	int OnlineIconIndex;
	int OfflineIconIndex;
};

struct SESSION_INFO : public GCSessionInfoBase
{
	int iX, iY;
	IAccPropServices* pAccPropServicesForNickList;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase {};

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	int   iX, iY;
	bool  bTabsEnable, TabsAtBottom, TabCloseOnDblClick, TabRestore;
	
	HFONT MessageAreaFont;
	COLORREF MessageAreaColor;
};

extern GlobalLogSettings g_Settings;
extern SESSION_INFO g_TabSession;
extern CHAT_MANAGER saveCI;
extern TABLIST *g_TabList;
extern HMENU g_hMenu;
extern HIMAGELIST hIconsList;

extern HINSTANCE g_hInst;
extern BOOL SmileyAddInstalled, PopupInstalled;

struct CREOleCallback : public IRichEditOleCallback
{
	CREOleCallback() : refCount(0), nextStgId(0), pictStg(NULL) {}
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;

 	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * lplpObj);
	STDMETHOD_(ULONG,AddRef) (THIS);
	STDMETHOD_(ULONG,Release) (THIS);

	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(GetNewStorage) (LPSTORAGE FAR * lplpstg);
	STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME FAR * lplpFrame, LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(ShowContainerUI) (BOOL fShow);
	STDMETHOD(QueryInsertObject) (LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp);
	STDMETHOD(DeleteObject) (LPOLEOBJECT lpoleobj);
	STDMETHOD(QueryAcceptData) (LPDATAOBJECT lpdataobj, CLIPFORMAT FAR * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
	STDMETHOD(GetClipboardData) (CHARRANGE FAR * lpchrg, DWORD reco, LPDATAOBJECT FAR * lplpdataobj);
	STDMETHOD(GetDragDropEffect) (BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect);
	STDMETHOD(GetContextMenu) (WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu) ;
};

extern CREOleCallback reOleCallback;

//main.c
void LoadIcons(void);

// log.c
void Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO *si, BOOL bRedraw, BOOL bPhaseTwo);
void ValidateFilename (TCHAR * filename);
char* Log_CreateRtfHeader(MODULEINFO * mi);

// window.c
INT_PTR CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth);

SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si);
SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si);

// options.c
int OptionsInitialize(WPARAM wParam, LPARAM lParam);
void AddIcons(void);
HICON LoadIconEx(char* pszIcoLibName, BOOL big);

// services.c
void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground);

// tools.c
int  GetColorIndex(const char* pszModule, COLORREF cr);
void CheckColorsInModule(const char* pszModule);
int  GetRichTextLength(HWND hwnd);
UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText);
void DestroyGCMenu(HMENU *hMenu, int iIndex);
bool LoadMessageFont(LOGFONT *lf, COLORREF *colour);

// message.c
char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si);

BOOL TabM_AddTab(const TCHAR *pszID, const char* pszModule);
BOOL TabM_RemoveAll(void);

#pragma comment(lib,"comctl32.lib")

#endif
