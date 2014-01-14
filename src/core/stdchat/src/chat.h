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
#include <m_database.h>
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

#include "m_ieview.h"
#include "m_smileyadd.h"

#include "resource.h"
#include "version.h"
#include "richutil.h"

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	BOOL TabsEnable;
	BOOL TabsAtBottom;
	BOOL TabCloseOnDblClick;
	BOOL TabRestore;
};

extern GlobalLogSettings g_Settings;
extern SESSION_INFO g_TabSession;

extern HINSTANCE g_hInst;

struct CREOleCallback : public IRichEditOleCallback
{
	CREOleCallback() : refCount(0) {}
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

struct COLORCHOOSER
{
	MODULEINFO*   pModule;
	int           xPosition;
	int           yPosition;
	HWND          hWndTarget;
	BOOL          bForeground;
	SESSION_INFO *si;
};

extern BOOL SmileyAddInstalled;

//main.c

void LoadIcons(void);
void LoadLogIcons(void);

//colorchooser.c
INT_PTR CALLBACK DlgProcColorToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//log.c
void   Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO *si, BOOL bRedraw, BOOL bPhaseTwo);
void   ValidateFilename (TCHAR * filename);
TCHAR* MakeTimeStamp(TCHAR* pszStamp, time_t time);
char*  Log_CreateRtfHeader(MODULEINFO * mi);

//window.c
INT_PTR CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth);

//options.c
int    OptionsInit(void);
int    OptionsUnInit(void);
void   AddIcons(void);
HICON  LoadIconEx(char* pszIcoLibName, BOOL big);

// services.c
void   ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground);
//tools.c
TCHAR*        RemoveFormatting(const TCHAR* pszText);
BOOL          DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
int           GetColorIndex(const char* pszModule, COLORREF cr);
void          CheckColorsInModule(const char* pszModule);
const TCHAR*  my_strstri(const TCHAR* s1, const TCHAR* s2) ;
int           GetRichTextLength(HWND hwnd);
BOOL          IsHighlighted(SESSION_INFO *si, const TCHAR* pszText);
UINT          CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText);
void          DestroyGCMenu(HMENU *hMenu, int iIndex);
BOOL          DoEventHookAsync(HWND hwnd, const TCHAR *pszID, const char* pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, DWORD dwItem);
BOOL          DoEventHook(const TCHAR *pszID, const char* pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, DWORD dwItem);
BOOL          IsEventSupported(int eventType);

// message.c
char*         Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si);
TCHAR*        DoRtfToTags( char* pszRtfText, SESSION_INFO *si);

BOOL TabM_AddTab(const TCHAR *pszID, const char* pszModule);
BOOL TabM_RemoveAll(void);

#pragma comment(lib,"comctl32.lib")

#endif
