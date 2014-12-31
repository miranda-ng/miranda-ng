/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef modern_clui_h__
#define modern_clui_h__

#include "windowsX.h"
#define HANDLE_MESSAGE( _message, _fn)    \
	case (_message): return This->_fn((_message), (wParam), (lParam))

class CLUI
{
public:
	static HWND   m_hWnd;
	static CLUI * m_pCLUI;
	static BOOL   m_fMainMenuInited;

private:
	CLUI();			// is protected use InitClui to initialize instead

public:
	~CLUI();

	static HRESULT InitClui()         { m_pCLUI = new CLUI(); return S_OK; };
	static HWND&   ClcWnd()           { return pcli->hwndContactTree; }
	static HWND&   CluiWnd()          { return pcli->hwndContactList; }
	static CLUI *  GetClui()          { return m_pCLUI; }
	static BOOL    IsMainMenuInited() { return CLUI::m_fMainMenuInited; }

	CLINTERFACE void cliOnCreateClc();

	EVENTHOOK(OnEvent_ModulesLoaded);
	EVENTHOOK(OnEvent_ContactMenuPreBuild);
	EVENTHOOK(OnEvent_FontReload);

	SERVICE(Service_ShowMainMenu);
	SERVICE(Service_ShowStatusMenu);
	SERVICE(Service_Menu_ShowContactAvatar);
	SERVICE(Service_Menu_HideContactAvatar);

	static LRESULT CALLBACK cli_ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		CLUI * This = m_pCLUI;
		if (!m_hWnd) m_hWnd = hwnd;

		BOOL bHandled = FALSE;
		LRESULT lRes = This->PreProcessWndProc(msg, wParam, lParam, bHandled);
		if (bHandled) return lRes;

		switch (msg)
		{
			HANDLE_MESSAGE(WM_NCCREATE, OnNcCreate);
			HANDLE_MESSAGE(WM_CREATE, OnCreate);
			HANDLE_MESSAGE(UM_CREATECLC, OnCreateClc);
			HANDLE_MESSAGE(UM_SETALLEXTRAICONS, OnSetAllExtraIcons);
			HANDLE_MESSAGE(WM_INITMENU, OnInitMenu);
			HANDLE_MESSAGE(WM_SIZE, OnSizingMoving);
			HANDLE_MESSAGE(WM_SIZING, OnSizingMoving);
			HANDLE_MESSAGE(WM_MOVE, OnSizingMoving);
			HANDLE_MESSAGE(WM_EXITSIZEMOVE, OnSizingMoving);
			HANDLE_MESSAGE(WM_WINDOWPOSCHANGING, OnSizingMoving);
			HANDLE_MESSAGE(WM_DISPLAYCHANGE, OnSizingMoving);
			HANDLE_MESSAGE(WM_THEMECHANGED, OnThemeChanged);
			HANDLE_MESSAGE(WM_DWMCOMPOSITIONCHANGED, OnDwmCompositionChanged);
			HANDLE_MESSAGE(UM_UPDATE, OnUpdate);
			HANDLE_MESSAGE(WM_NCACTIVATE, OnNcPaint);
			HANDLE_MESSAGE(WM_PRINT, OnNcPaint);
			HANDLE_MESSAGE(WM_NCPAINT, OnNcPaint);
			HANDLE_MESSAGE(WM_ERASEBKGND, OnEraseBkgnd);
			HANDLE_MESSAGE(WM_PAINT, OnPaint);
			HANDLE_MESSAGE(WM_LBUTTONDOWN, OnLButtonDown);
			HANDLE_MESSAGE(WM_PARENTNOTIFY, OnParentNotify);
			HANDLE_MESSAGE(WM_SETFOCUS, OnSetFocus);
			HANDLE_MESSAGE(WM_TIMER, OnTimer);
			HANDLE_MESSAGE(WM_ACTIVATE, OnActivate);
			HANDLE_MESSAGE(WM_SETCURSOR, OnSetCursor);
			HANDLE_MESSAGE(WM_MOUSEACTIVATE, OnMouseActivate);
			HANDLE_MESSAGE(WM_NCLBUTTONDOWN, OnNcLButtonDown);
			HANDLE_MESSAGE(WM_NCLBUTTONDBLCLK, OnNcLButtonDblClk);
			HANDLE_MESSAGE(WM_NCHITTEST, OnNcHitTest);
			HANDLE_MESSAGE(WM_SHOWWINDOW, OnShowWindow);
			HANDLE_MESSAGE(WM_SYSCOMMAND, OnSysCommand);
			HANDLE_MESSAGE(WM_KEYDOWN, OnKeyDown);
			HANDLE_MESSAGE(WM_GETMINMAXINFO, OnGetMinMaxInfo);
			HANDLE_MESSAGE(WM_MOVING, OnMoving);
			HANDLE_MESSAGE(WM_NOTIFY, OnNotify);
			HANDLE_MESSAGE(WM_CONTEXTMENU, OnContextMenu);
			HANDLE_MESSAGE(WM_MEASUREITEM, OnMeasureItem);
			HANDLE_MESSAGE(WM_DRAWITEM, OnDrawItem);
			HANDLE_MESSAGE(WM_DESTROY, OnDestroy);
		default:
			return This->DefCluiWndProc(msg, wParam, lParam);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// METHODS
	//
private:
	HRESULT CreateCLC();
	HRESULT FillAlphaChannel(HDC hDC, RECT* prcParent);
	HRESULT SnappingToEdge(WINDOWPOS * lpWindowPos);
	HRESULT LoadDllsRuntime();
	HRESULT RegisterAvatarMenu();  // TODO move to CLC class
	HRESULT CreateCluiFrames();
	HRESULT CreateCLCWindow(const HWND parent);
	HRESULT CreateUIFrames();

	LRESULT DefCluiWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return corecli.pfnContactListWndProc(m_hWnd, msg, wParam, lParam);
	}

	// MessageMap
	LRESULT PreProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSizingMoving(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnThemeChanged(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDwmCompositionChanged(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdate(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInitMenu(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcPaint(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcCreate(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetAllExtraIcons(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreateClc(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnParentNotify(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetFocus(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnStatusBarUpdateTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnAutoAlphaTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSmoothAlphaTransitionTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDelayedSizingTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnBringOutTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnBringInTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateBringTimer(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnActivate(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseActivate(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcLButtonDblClk(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNcHitTest(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWindow(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSysCommand(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetMinMaxInfo(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMoving(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnListSizeChangeNotify(NMCLISTCONTROL *pnmc);
	LRESULT OnClickNotify(NMCLISTCONTROL *pnmc);
	LRESULT OnContextMenu(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMeasureItem(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDrawItem(UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	HMODULE m_hDwmapiDll;

	enum { SNAPTOEDGESENSIVITY = 30 };
};

#endif // modern_clui_h__