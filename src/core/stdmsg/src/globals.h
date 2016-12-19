/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef SRMM_GLOBALS_H
#define SRMM_GLOBALS_H

struct GlobalMessageData
{
	bool bShowButtons;
	bool bSendButton;
	bool bShowTyping;
	bool bShowTypingWin;
	bool bShowTypingTray;
	bool bShowTypingClist;
	bool bShowIcons;
	bool bShowTime;
	bool bShowDate;
	bool bShowAvatar;
	bool bHideNames;
	bool bShowSecs;
	bool bShowReadChar;
	bool bSendOnEnter;
	bool bSendOnDblEnter;
	bool bAutoClose;
	bool bAutoMin;
	bool bTypingUnknown;
	bool bCtrlSupport;
	bool bShowFormat;
	bool bSavePerContact;
	bool bUseStatusWinIcon;
	bool bDoNotStealFocus;
	bool bCascade;
	bool bDeleteTempCont;

	DWORD openFlags;
	DWORD msgTimeout;
	DWORD nFlashMax;
	int iGap;
};

void InitGlobals();
void ReloadGlobals();

extern GlobalMessageData g_dat;

/////////////////////////////////////////////////////////////////////////////////////////

struct CREOleCallback : public IRichEditOleCallback
{
	CREOleCallback() : refCount(0), nextStgId(0), pictStg(NULL) {}
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;

	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * lplpObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(GetNewStorage)(LPSTORAGE FAR * lplpstg);
	STDMETHOD(GetInPlaceContext)(LPOLEINPLACEFRAME FAR * lplpFrame, LPOLEINPLACEUIWINDOW FAR * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(ShowContainerUI)(BOOL fShow);
	STDMETHOD(QueryInsertObject)(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp);
	STDMETHOD(DeleteObject)(LPOLEOBJECT lpoleobj);
	STDMETHOD(QueryAcceptData)(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
	STDMETHOD(GetClipboardData)(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR * lplpdataobj);
	STDMETHOD(GetDragDropEffect)(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect);
	STDMETHOD(GetContextMenu)(WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE FAR * lpchrg, HMENU FAR * lphmenu);
};

#endif
