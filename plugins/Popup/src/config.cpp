/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/config.cpp $
Revision       : $Revision: 1651 $
Last change on : $Date: 2010-07-15 20:31:06 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

//===== General Plugin =====
HINSTANCE hInst;
HANDLE hMainThread;
//MNOTIFYLINK *notifyLink;	//deprecatet
PLUGINLINK *pluginLink;
HANDLE hSemaphore;
BOOL closing = FALSE;
MM_INTERFACE		mmi;
LIST_INTERFACE		li;
UTF8_INTERFACE		utfi;
MTEXT_INTERFACE		MText = {0};
HANDLE folderId;
BOOL gbPopupLoaded  = FALSE;
BOOL gbHppInstalled = FALSE;
LPCSTR gszMetaProto = "";

//===== Brushes, Colours and Fonts =====
HBITMAP hbmNoAvatar;

//===== Options =====
POPUPOPTIONS PopUpOptions;
//SKINELEMENT *skin;
//SKINELEMENT *w_skin;
//SKINELEMENT *n_skin;


//===== Plugin information =====

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	POPUP_DISPLAYNAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM),
	POPUP_DESCRIPTION,
	POPUP_AUTHOR,
	POPUP_EMAIL,
	POPUP_COPYRIGHT,
	POPUP_WEBPAGE,
	0,
	0,
	POPUP_UUID
};

// MLU layer for ansi release
#if !defined(_UNICODE)
BOOL		(WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL		(WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);
BOOL		(WINAPI *MyGetMonitorInfo)(HMONITOR hMonitor, LPMONITORINFO lpmi);
BOOL		(WINAPI *MyUpdateLayeredWindow)
				(HWND hwnd, HDC hdcDST, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc,
				COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
HMONITOR	(WINAPI *MyMonitorFromWindow)(HWND hWnd, DWORD dwFlags);
BOOL		(WINAPI *MyTransparentBlt)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
BOOL		(WINAPI *MyAlphaBlend)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

// gdi32.dll
BOOL		(WINAPI *MyGetTextExtentPoint32W)(HDC, LPCWSTR, int, LPSIZE);

//user32.dll
int			(WINAPI *MyDrawTextW)(HDC, LPCWSTR, int, LPRECT, UINT);
int			(WINAPI *MyDrawTextExW)(HDC, LPCWSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
BOOL		(WINAPI *MySetWindowTextW)(HWND, LPCWSTR);
LRESULT		(WINAPI *MySendMessageW)(HWND, UINT, WPARAM, LPARAM);
LRESULT		(WINAPI *MyCallWindowProcW)(WNDPROC, HWND, UINT, WPARAM, LPARAM);
HWND		(WINAPI *MyCreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

#endif

HRESULT		(WINAPI *MyDwmEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);

//====== Common Vars ========================

// common funcs
void LoadOptions() {
	ZeroMemory(&PopUpOptions, sizeof(PopUpOptions));
	#if defined(_DEBUG)
		PopUpOptions.debug = DBGetContactSettingByte(NULL, MODULNAME, "debug", FALSE);
	#endif

	//Load PopUp Options
	if(!OptionLoaded){
		LoadOption_General();
		LoadOption_Skins();
		LoadOption_Actions();
		LoadOption_AdvOpts();
	}
	Check_ReorderPopUps();
	OptionLoaded = true;
	return;
}

void PopUpPreview()
{
	TCHAR *lptzTitle1Eng = TranslateT("The Jabberwocky");
	TCHAR *lptzText1Eng  = TranslateTS(
		_T("`Twas brillig, and the slithy toves\r\n")
		_T("Did gyre and gimble in the wabe:\r\n")
		_T("All mimsy were the borogoves,\r\n")
		_T("And the mome raths outgrabe.\r\n")
		_T("\t[b][i]Lewis Carroll, 1855[/i][/b]")
		);

	TCHAR *lptzTitle2 = TranslateT("Test preview for the popup plugin settings. This is supposed to be long enough not to fit in one line...");
	TCHAR *lptzText2  = TranslateTS(
		_T("This is a special test preview for the popup ")
		_T("plugin settings. The text and title are quite ")
		_T("long so you can tweak your skin and plugin ")
		_T("settings to best fit your needs :)")
		);

	POPUPDATA2 ppd = {0};

	ZeroMemory(&ppd, sizeof(ppd));
	ppd.cbSize		= sizeof(ppd);
	ppd.flags		= PU2_TCHAR;

#if defined(_DEBUG)
	// test per-contact popups
	for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0); hContact; 
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		if (DBGetContactSettingDword(hContact, "ICQ", "UIN", 0) == 256771455)
		{
			ppd.lchContact = hContact;
			break;
		}
	}
#endif
	ppd.lptzTitle	= lptzTitle1Eng;
	ppd.lptzText	= lptzText1Eng;
	ppd.lchIcon		= LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	CallService(MS_POPUP_ADDPOPUP2, (WPARAM)&ppd, APF_NO_HISTORY);
	if (PopUpOptions.UseAnimations || PopUpOptions.UseEffect) Sleep((ANIM_TIME*2)/3); //Pause

	ZeroMemory(&ppd, sizeof(ppd));
	ppd.cbSize		= sizeof(ppd);
	ppd.flags		= PU2_TCHAR;
	ppd.lptzTitle	= lptzTitle2;
	ppd.lptzText	= lptzText2;
	ppd.lchIcon		= LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	ppd.hbmAvatar	= hbmNoAvatar;
#if defined(_DEBUG)
//remove testactions
	POPUPACTION actions[3];
	actions[0].cbSize	= sizeof(POPUPACTION);
	actions[0].flags	= PAF_ENABLED;
	lstrcpyA(actions[0].lpzTitle, "Popup Plus/Test action");
	actions[0].lchIcon	= IcoLib_GetIcon(ICO_POPUP_ON,0);
	actions[0].wParam	= actions[0].lParam = -1;

	actions[1].cbSize	= sizeof(POPUPACTION);
	actions[1].flags	= PAF_ENABLED;
	lstrcpyA(actions[1].lpzTitle, "Popup Plus/Second test action");
	actions[1].lchIcon	= IcoLib_GetIcon(ICO_ACT_CLOSE,0);
	actions[1].wParam	= actions[1].lParam = -1;

	actions[2].cbSize	= sizeof(POPUPACTION);
	actions[2].flags	= PAF_ENABLED;
	lstrcpyA(actions[2].lpzTitle, "Popup Plus/One more action");
	actions[2].lchIcon	= LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	actions[2].wParam	= actions[2].lParam = -1;

	ppd.lpActions	= actions;
	ppd.actionCount	= SIZEOF(actions);
#endif
	CallService(MS_POPUP_ADDPOPUP2, (WPARAM)&ppd, APF_NO_HISTORY);
	if (PopUpOptions.UseAnimations || PopUpOptions.UseEffect) Sleep((ANIM_TIME*2)/3); //Pause

	PUShowMessageT(TranslateT("This is a notification message"),	(DWORD)SM_NOTIFY|0x80000000);
	if (PopUpOptions.UseAnimations || PopUpOptions.UseEffect) Sleep((ANIM_TIME*2)/3); //Pause

	PUShowMessageT(TranslateT("This is a warning message"),			(DWORD)SM_WARNING|0x80000000);
	if (PopUpOptions.UseAnimations || PopUpOptions.UseEffect) Sleep((ANIM_TIME*2)/3); //Pause

	PUShowMessageT(TranslateT("This is an error message"),			(DWORD)SM_ERROR|0x80000000);
}