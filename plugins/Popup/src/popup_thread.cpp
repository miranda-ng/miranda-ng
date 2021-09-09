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
*/

#include "stdafx.h"
#include <process.h>

// globals
static int    gIdleRequests = 0;
static bool   gTerminating = false;
static HWND   gHwndManager = nullptr;
static int    gLockCount = 0;
static volatile int nPopups = 0;
static HANDLE hThread = nullptr;

static LIST<PopupWnd2> popupList(3);

// forwards
enum
{
	//  message id's
	UTM_PT_FIRST = WM_USER + 1607,
	UTM_STOP_THREAD,
	UTM_ADD_WINDOW,
	UTM_UPDATE_WINDOW,
	UTM_REMOVE_WINDOW,
	UTM_REQUEST_IDLE,
	UTM_LOCK_QUEUE,
	UTM_UNLOCK_QUEUE,
	UTM_REQUEST_REMOVE,
	UTM_AVATAR_CHANGED
};

bool UpdatePopupPosition(PopupWnd2 *prev, PopupWnd2 *wnd)
{
	if (!wnd) return false;
	if (!PopupOptions.ReorderPopups && wnd->isPositioned()) return false;

	int POPUP_SPACING = PopupOptions.spacing;

	POINT pos = { 0 };
	SIZE prevSize = { 0 };
	if (prev)
		prevSize = prev->getSize();

	// we have only one monitor (cant check it together with 1.if)
	RECT rc;
	if (GetSystemMetrics(SM_CMONITORS) == 1)
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	else { // Multimonitor stuff (we have more then 1)
		HWND hWnd;
		if (PopupOptions.Monitor == MN_MIRANDA)
			hWnd = g_clistApi.hwndContactList;
		else //  PopupOptions.Monitor == MN_ACTIVE
			hWnd = GetForegroundWindow();

		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

		MONITORINFOEX mnti;
		mnti.cbSize = sizeof(MONITORINFOEX);
		if (GetMonitorInfo(hMonitor, &mnti) == TRUE)
			memcpy(&rc, &(mnti.rcWork), sizeof(RECT));
		else
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	}

	rc.left += PopupOptions.gapLeft - POPUP_SPACING;
	rc.right -= PopupOptions.gapRight - POPUP_SPACING;
	rc.top += PopupOptions.gapTop - POPUP_SPACING;
	rc.bottom -= PopupOptions.gapBottom - POPUP_SPACING;
	if (PopupOptions.Spreading == SPREADING_VERTICAL) {
		switch (PopupOptions.Position) {
		case POS_UPPERLEFT:
			pos.x = rc.left + POPUP_SPACING;
			pos.y = (prev ? (prev->getPosition().y + prev->getSize().cy) : rc.top) + POPUP_SPACING;
			break;
		case POS_LOWERLEFT:
			pos.x = rc.left + POPUP_SPACING;
			pos.y = (prev ? prev->getPosition().y : rc.bottom) - wnd->getSize().cy - POPUP_SPACING;
			break;
		case POS_LOWERRIGHT:
			pos.x = rc.right - wnd->getSize().cx - POPUP_SPACING;
			pos.y = (prev ? prev->getPosition().y : rc.bottom) - wnd->getSize().cy - POPUP_SPACING;
			break;
		case POS_UPPERRIGHT:
			pos.x = rc.right - wnd->getSize().cx - POPUP_SPACING;
			pos.y = (prev ? (prev->getPosition().y + prev->getSize().cy) : rc.top) + POPUP_SPACING;
			break;
		}
	}
	else {
		switch (PopupOptions.Position) {
		case POS_UPPERLEFT:
			pos.x = (prev ? (prev->getPosition().x + prev->getSize().cx) : rc.left) + POPUP_SPACING;
			pos.y = rc.top + POPUP_SPACING;
			break;
		case POS_LOWERLEFT:
			pos.x = (prev ? (prev->getPosition().x + prev->getSize().cx) : rc.left) + POPUP_SPACING;
			pos.y = rc.bottom - wnd->getSize().cy - POPUP_SPACING;
			break;
		case POS_LOWERRIGHT:
			pos.x = (prev ? prev->getPosition().x : rc.right) - wnd->getSize().cx - POPUP_SPACING;
			pos.y = rc.bottom - wnd->getSize().cy - POPUP_SPACING;
			break;
		case POS_UPPERRIGHT:
			pos.x = (prev ? prev->getPosition().x : rc.right) - wnd->getSize().cx - POPUP_SPACING;
			pos.y = rc.top + POPUP_SPACING;
			break;
		}
	}
	wnd->setPosition(pos);
	return true;
}

void RepositionPopups()
{
	PopupWnd2 *prev = nullptr;
	if (PopupOptions.ReorderPopups) {
		for (auto &it : popupList) {
			UpdatePopupPosition(prev, it);
			prev = it;
		}
	}
}

static LRESULT CALLBACK PopupThreadManagerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PopupWnd2 *wnd = nullptr;
	if (message == UTM_ADD_WINDOW || message == UTM_UPDATE_WINDOW || message == UTM_REMOVE_WINDOW || message == UTM_REQUEST_REMOVE)
		if (!(wnd = (PopupWnd2 *)lParam))
			return 0;

	switch (message) {
	case UTM_STOP_THREAD:
		gTerminating = true;
		if (g_plugin.getByte("FastExit", 0))
			for (auto &it : popupList)
				PUDeletePopup(it->getHwnd());
		PostQuitMessage(0);
		break;

	case UTM_ADD_WINDOW:
		if (gTerminating)
			break;
		UpdatePopupPosition(popupList.getCount() ? popupList[popupList.getCount() - 1] : nullptr, wnd);
		popupList.insert(wnd);
		++nPopups;
		wnd->callMethodAsync(&PopupWnd2::m_show, 0);
		break;

	case UTM_UPDATE_WINDOW:
		RepositionPopups();
		break;

	case UTM_REQUEST_REMOVE:
		if ((PopupOptions.LeaveHovered && gLockCount) || (wnd && wnd->isLocked()))
			wnd->updateTimer();
		else
			PostMessage(wnd->getHwnd(), WM_CLOSE, 0, 0);
		break;

	case UTM_REMOVE_WINDOW:
		for (auto &it : popupList.rev_iter())
			if (it == wnd)
				popupList.removeItem(&it);

		RepositionPopups();
		--nPopups;
		delete wnd;
		break;

	case UTM_LOCK_QUEUE:
		++gLockCount;
		break;

	case UTM_UNLOCK_QUEUE:
		if (--gLockCount < 0)
			gLockCount = 0;
		break;

	case UTM_AVATAR_CHANGED:
		for (auto &p : popupList)
			if (p->getContact() == wParam)
				SendMessage(p->getHwnd(), UM_AVATARCHANGED, 0, 0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//  thread func
static unsigned __stdcall PopupThread(void *)
{
	Thread_SetName("Popup: PopupThread");
	MThreadLock threadLock(hThread);

	//  Create manager window
	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = PopupThreadManagerWndProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = g_plugin.getInst();
	wcl.hIcon = nullptr;
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = nullptr;
	wcl.lpszClassName = L"PopupThreadManagerWnd";
	wcl.hIconSm = Skin_LoadIcon(SKINICON_OTHER_POPUP);
	g_wndClass.cPopupThreadManagerWnd = RegisterClassExW(&wcl);

	gHwndManager = CreateWindow(L"PopupThreadManagerWnd", nullptr, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, nullptr, g_plugin.getInst(), nullptr);
	SetWindowPos(gHwndManager, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(gHwndManager); gHwndManager = nullptr;
	UnregisterClassW(wcl.lpszClassName, g_plugin.getInst());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// interface

static int sttAvatarChanged(WPARAM wParam, LPARAM)
{
	PostMessage(gHwndManager, UTM_AVATAR_CHANGED, wParam, 0);
	return 0;
}

void LoadPopupThread()
{
	unsigned threadId;
	hThread = mir_forkthreadex(PopupThread, nullptr, &threadId);

	HookEvent(ME_AV_AVATARCHANGED, sttAvatarChanged);
}

void StopPopupThread()
{
	PostMessage(gHwndManager, UTM_STOP_THREAD, 0, 0);
}

void UnloadPopupThread()
{
	//  We won't waint for thread to exit, Miranda's thread unwind mechanism will do that for us.
	if (hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	for (auto &it : popupList)
		delete it;
	popupList.destroy();
}

void PopupThreadLock()
{
	PostMessage(gHwndManager, UTM_LOCK_QUEUE, 0, 0);
}

void PopupThreadUnlock()
{
	PostMessage(gHwndManager, UTM_UNLOCK_QUEUE, 0, 0);
}

bool PopupThreadIsFull()
{
	return nPopups >= PopupOptions.MaxPopups;
}

bool PopupThreadAddWindow(PopupWnd2 *wnd)
{
	PostMessage(gHwndManager, UTM_ADD_WINDOW, 0, (LPARAM)wnd);
	return true;
}

bool PopupThreadRemoveWindow(PopupWnd2 *wnd)
{
	PostMessage(gHwndManager, UTM_REMOVE_WINDOW, 0, (LPARAM)wnd);
	return true;
}

bool PopupThreadUpdateWindow(PopupWnd2 *wnd)
{
	PostMessage(gHwndManager, UTM_UPDATE_WINDOW, 0, (LPARAM)wnd);
	return true;
}

bool PopupThreadRequestRemoveWindow(PopupWnd2 *wnd)
{
	PostMessage(gHwndManager, UTM_REQUEST_REMOVE, 0, (LPARAM)wnd);
	return true;
}
