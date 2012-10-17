/*

MUCC Group Chat GUI Plugin for Miranda NG
Copyright (C) 2004  Piotr Piastucki

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

#include "ChatContainer.h"
#include "Utils.h"
#include "Options.h"

#define DM_CREATECHILD		(WM_USER+10)
#define DM_ADDCHILD			(WM_USER+11)
#define DM_ACTIVATECHILD	(WM_USER+12)
#define DM_CHANGECHILDDATA	(WM_USER+13)
#define DM_REMOVECHILD		(WM_USER+14)

#define DM_SETUNREAD		(WM_USER+15)
#define DM_FLASHWINDOW		(WM_USER+16)

#define TIMERID_FLASHWND     1
#define TIMEOUT_FLASHWND     900

ChatContainer *	ChatContainer::list = NULL;
bool ChatContainer::released = false;
CRITICAL_SECTION ChatContainer::mutex;

//BOOL CALLBACK ContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void __cdecl StartThread(void *vContainer);

void ChatContainer::release() {
	released = true;
	for (ChatContainer *ptr2, *ptr = list; ptr != NULL; ptr=ptr2) {
		ptr2 = ptr->getNext();
		SendMessage(ptr->getHWND(), WM_CLOSE, 0, 0);
	}
	DeleteCriticalSection(&mutex);
}

void ChatContainer::init() {
	released = false;
	InitializeCriticalSection(&mutex);
}

int ChatContainer::getDefaultOptions() {
	return FLAG_USE_TABS;
}

ChatContainer * ChatContainer::getWindow() {
	ChatContainer *ptr;
	EnterCriticalSection(&mutex);
	if (list == NULL || !(Options::getChatContainerOptions() & ChatContainer::FLAG_USE_TABS)) {
		ptr = new ChatContainer();
	} else {
		ptr = list;
	}
	LeaveCriticalSection(&mutex);
	return ptr;
}

ChatContainer::ChatContainer() {
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hWnd = NULL;
	prev = next =NULL;
	active = NULL;
	childCount = 0;
	nFlash = 0;
	nFlashMax = 3;
	Utils::forkThread((void (__cdecl *)(void *))StartThread, 0, (void *) this);
	WaitForSingleObject(hEvent, INFINITE);
	EnterCriticalSection(&mutex);
	setNext(list);
	if (next != NULL) {
		next->setPrev(this);
	}
	list = this;
	LeaveCriticalSection(&mutex);
}

ChatContainer::~ChatContainer() {
	if (!released) {
		EnterCriticalSection(&mutex);
		if (getPrev() != NULL) {
			getPrev()->setNext(next);
		} else if (list == this) {
			list = getNext();
		}
		if (getNext() != NULL) {
			getNext()->setPrev(prev);
		}
		LeaveCriticalSection(&mutex);
	}
	if (hEvent != NULL) {
		CloseHandle(hEvent);
	}
}

void ChatContainer::setHWND(HWND hWnd) {
	this->hWnd = hWnd;
}

HWND ChatContainer::getHWND() {
	return hWnd;
}

HANDLE ChatContainer::getEvent() {
	return hEvent;
}

ChatContainer * ChatContainer::getNext() {
	return next;
}

void ChatContainer::setNext(ChatContainer * next) {
	this->next = next;
}

ChatContainer * ChatContainer::getPrev() {
	return prev;
}

void ChatContainer::setPrev(ChatContainer * prev) {
	this->prev = prev;
}

void ChatContainer::show(bool bShow) {
	ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);
}

ChatWindow * ChatContainer::getActive() {
	return active;
}

int ChatContainer::getFlash() {
	return nFlash;
}

int ChatContainer::getFlashMax() {
	return nFlashMax;
}

int ChatContainer::getFlashTimeout() {
	return TIMEOUT_FLASHWND;
}

void ChatContainer::setFlash(int n) {
	nFlash = n;
}

void ChatContainer::activateChild(ChatWindow *window) {
	RECT rcChild;
	getChildWindowRect(&rcChild);
	if (window != NULL) {
		SetWindowPos(window->getHWND(), HWND_TOP, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, SWP_NOSIZE);
	}
	if (window != active) {
		ChatWindow *prev = active;
		active = window;
		SendMessage(hWnd, WM_SIZE, 0, 0);
		ShowWindow(active->getHWND(), SW_SHOW);
//		SendMessage(active->getHWND(), DM_UPDATETITLE, 0, 0);
		if (prev != NULL) {
			ShowWindow(prev->getHWND(), SW_HIDE);
		}
		SetWindowTextA(hWnd, window->getRoomName());
	}
	TCITEM tci;
	tci.mask = TCIF_IMAGE;
	tci.iImage = -1;
	TabCtrl_SetItem(GetDlgItem(hWnd, IDC_TABS), getChildTab(window), &tci);
	SendMessage(active->getHWND(), WM_ACTIVATE, WA_ACTIVE, 0);
	SetFocus(active->getHWND());
}


void ChatContainer::addChild(ChatWindow *child) {
	TCITEM tci;
	int tabId;
	HWND hwndTabs = GetDlgItem(hWnd, IDC_TABS);
	childCount++;
	tci.mask = TCIF_TEXT | TCIF_PARAM;
	LPTSTR lps1 = Utils::mucc_mir_a2t(child->getRoomName());
	tci.pszText = lps1;
	tci.lParam = (LPARAM) child;
	tabId = TabCtrl_InsertItem(hwndTabs, childCount-1, &tci);
	TabCtrl_SetCurSel(hwndTabs, tabId);
	Utils::mucc_mir_free(lps1);
	activateChild(child);
	SendMessage(hWnd, WM_SIZE, 0, 0);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
}

void ChatContainer::changeChildData(ChatWindow *child) {
	int tabId;
	HWND hwndTabs = GetDlgItem(hWnd, IDC_TABS);
	tabId = getChildTab(child);
	if (tabId >=0) {
		TCITEM tci;
		tci.mask = TCIF_TEXT;
		LPTSTR lps1 = Utils::mucc_mir_a2t(child->getRoomName());
		tci.pszText = lps1;
		TabCtrl_SetItem(hwndTabs, childCount-1, &tci);
		Utils::mucc_mir_free(lps1);
	}
	if (child == active) {
		SetWindowTextA(hWnd, child->getRoomName());
	}
}


void ChatContainer::removeChild(ChatWindow *child) {
	HWND hwndTabs = GetDlgItem(hWnd, IDC_TABS);
	int iSel = getChildTab(child);
	if (iSel >= 0) {
		TabCtrl_DeleteItem(hwndTabs, iSel);
	}
	childCount--;
	if (childCount > 0) {
		TCITEM tci;
		if (iSel == childCount) iSel--;
		TabCtrl_SetCurSel(hwndTabs, iSel);
		tci.mask = TCIF_PARAM;
		if (TabCtrl_GetItem(hwndTabs, iSel, &tci)) {
			child = (ChatWindow *)tci.lParam;
			activateChild(child);
		}
	} else {//if (!released) {
		SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
}

void ChatContainer::setUnread(ChatWindow *child, int unread) {
	if (!unread || child != active) {
		TCITEM tci;
		tci.mask = TCIF_IMAGE;
		if (unread) {
			tci.iImage = 0;
		} else {
			tci.iImage = -1;
		}
		TabCtrl_SetItem(GetDlgItem(hWnd, IDC_TABS), getChildTab(child), &tci);
	}
}


void ChatContainer::getChildWindowRect(RECT *rcChild)
{
	RECT rc, rcTabs; //rcStatus, 
	HWND hwndTabs = GetDlgItem(hWnd, IDC_TABS);
	int l = TabCtrl_GetItemCount(hwndTabs);
	GetClientRect(hWnd, &rc);
	GetClientRect(hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(hwndTabs, FALSE, &rcTabs);
//	GetWindowRect(dat->hwndStatus, &rcStatus);
	rcChild->left = 0;
	rcChild->right = rc.right;
	if (l > 1) {
		rcChild->top = rcTabs.top - 1;
	} else {
		rcChild->top = 0;
	}
	rcChild->bottom = rc.bottom - rc.top;// - (rcStatus.bottom - rcStatus.top);
}

ChatWindow * ChatContainer::getChildFromTab(int tabId) {
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	TabCtrl_GetItem(GetDlgItem(hWnd, IDC_TABS), tabId, &tci);
	return (ChatWindow *)tci.lParam;
}

int ChatContainer::getChildTab(ChatWindow *child)  {
	TCITEM tci;
	int l, i;
	HWND hwndTabs = GetDlgItem(hWnd, IDC_TABS);
	l = TabCtrl_GetItemCount(hwndTabs);
	for (i = 0; i < l; i++) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(hwndTabs, i, &tci);
		if (child == (ChatWindow *) tci.lParam) {
			return i;
		}
	}
	return -1;

}

HWND ChatContainer::remoteCreateChild(DLGPROC proc, ChatWindow *ptr) {
	return (HWND) SendMessage(hWnd, DM_CREATECHILD, (WPARAM)proc, (LPARAM) ptr);
}

void ChatContainer::remoteAddChild(ChatWindow *ptr) {
	SendMessage(hWnd, DM_ADDCHILD, (WPARAM)0, (LPARAM) ptr);
}

void ChatContainer::remoteChangeChildData(ChatWindow *ptr) {
	SendMessage(hWnd, DM_CHANGECHILDDATA, (WPARAM)0, (LPARAM) ptr);
}

void ChatContainer::remoteRemoveChild(ChatWindow *ptr) {
	SendMessage(hWnd, DM_REMOVECHILD, (WPARAM)0, (LPARAM) ptr);
}

void ChatContainer::remoteSetUnread(ChatWindow *ptr, int unread) {
	SendMessage(hWnd, DM_SETUNREAD, (WPARAM)unread, (LPARAM) ptr);
}

void ChatContainer::remoteFlashWindow() {
	SendMessage(hWnd, DM_FLASHWINDOW, 0, 0);
}

INT_PTR CALLBACK ContainerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ChatContainer *container;
	container = (ChatContainer *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (container == NULL && msg != WM_INITDIALOG) return FALSE;
	switch (msg) {
		case WM_INITDIALOG:
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) muccIcon[MUCC_IDI_CHAT]);
			container = (ChatContainer *) lParam;
			container->setHWND(hwndDlg);
			TabCtrl_SetImageList(GetDlgItem(hwndDlg, IDC_TABS), hImageList);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) container);
			ShowWindow(hwndDlg, SW_SHOW);
			SetEvent(container->getEvent());
			return TRUE;
		case WM_GETMINMAXINFO:
			MINMAXINFO *mmi;
			RECT rcChild, rcWindow;
			mmi = (MINMAXINFO *) lParam;
			GetWindowRect(hwndDlg, &rcWindow);
			container->getChildWindowRect(&rcChild);
			mmi->ptMinTrackSize.x = 380;
			mmi->ptMinTrackSize.y = 130 + (rcWindow.bottom - rcWindow.top) - (rcChild.bottom - rcChild.top);
			return FALSE;
		case WM_SIZE:
			if (IsIconic(hwndDlg) || wParam == SIZE_MINIMIZED) break;
			{
				RECT rc, rcChild, rcWindow;
				GetClientRect(hwndDlg, &rc);
				HWND hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
				MoveWindow(hwndTabs, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top) - 0,	FALSE);
				RedrawWindow(hwndTabs, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
				container->getChildWindowRect(&rcChild);
				if ((rcChild.bottom-rcChild.top) < 130 || (rcChild.right-rcChild.left) < 380) {
					GetWindowRect(hwndDlg, &rcWindow);
					if ((rcChild.bottom-rcChild.top) < 130) {
						rcWindow.bottom = rcWindow.top + 130 + (rcWindow.bottom - rcWindow.top) - (rcChild.bottom - rcChild.top);
					} 
					if ((rcChild.right-rcChild.left) < 380) {
						rcWindow.right = rcWindow.left + 380;
					}
					MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
					container->getChildWindowRect(&rcChild);
				}
				if (container->getActive() != NULL) {
					MoveWindow(container->getActive()->getHWND(), rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
				}
			}
			return TRUE;
		case DM_CREATECHILD:
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_GROUPCHAT_LOG), hwndDlg, (DLGPROC) wParam, (LPARAM) lParam));
			return TRUE;
		case DM_ADDCHILD:
			container->addChild((ChatWindow *) lParam);
			return TRUE;
		case DM_REMOVECHILD:
			container->removeChild((ChatWindow *) lParam);
			return TRUE;
		case DM_CHANGECHILDDATA:
			container->removeChild((ChatWindow *) lParam);
			return TRUE;
		case DM_SETUNREAD:
			container->setUnread((ChatWindow *) lParam, (int)wParam);
			return TRUE;
		case DM_FLASHWINDOW:
			if (GetActiveWindow() != hwndDlg && GetForegroundWindow() != hwndDlg) {
				container->setFlash(0);
				SetTimer(hwndDlg, TIMERID_FLASHWND, container->getFlashTimeout(), NULL);
			}
			return TRUE;
		case WM_NOTIFY:
			{
				NMHDR* pNMHDR = (NMHDR*) lParam;
				switch (pNMHDR->code) {
				case TCN_SELCHANGE:
					{
						TCITEM tci = {0};
						HWND hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
						int iSel = TabCtrl_GetCurSel(hwndTabs);
						tci.mask = TCIF_PARAM;
						if (TabCtrl_GetItem(hwndTabs, iSel, &tci)) {
							ChatWindow * chatWindow = (ChatWindow *) tci.lParam;
							container->activateChild(chatWindow);
						}
					}
					break;
				case NM_CLICK:
					{
						FILETIME ft;
						TCHITTESTINFO thinfo;
						int tabId;
						HWND hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
						GetSystemTimeAsFileTime(&ft);
						GetCursorPos(&thinfo.pt);
						ScreenToClient(hwndTabs, &thinfo.pt);
						tabId = TabCtrl_HitTest(hwndTabs, &thinfo);
						if (tabId != -1 && tabId == container->lastClickTab &&
							(ft.dwLowDateTime - container->lastClickTime) < (GetDoubleClickTime() * 10000)) {
							SendMessage(container->getChildFromTab(tabId)->getHWND(), WM_CLOSE, 0, 0);
							container->lastClickTab = -1;
						} else {
							container->lastClickTab = tabId;
						}
						container->lastClickTime = ft.dwLowDateTime;
					}
					break;
				}

			}
			break;
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_ACTIVE)
				break;
		case WM_MOUSEACTIVATE:
			if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
				FlashWindow(hwndDlg, FALSE);
			}
			/*
			if (container->getActive() != NULL) {
				container->setUnread(container->getActive(), 0);
				SendMessage(container->getActive()->getHWND(), WM_ACTIVATE, WA_ACTIVE, 0);
			}*/
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			return FALSE;
		case WM_TIMER:
			if (wParam == TIMERID_FLASHWND) {
				if ((container->getFlash() > container->getFlashMax()) || (GetActiveWindow() == hwndDlg) || (GetForegroundWindow() == hwndDlg)) {
					KillTimer(hwndDlg, TIMERID_FLASHWND);
					FlashWindow(hwndDlg, FALSE);
				} else if (container->getFlash() < container->getFlashMax()) {
					FlashWindow(hwndDlg, TRUE);
					container->setFlash(container->getFlash()+1);
				}
			}
			break;
		case WM_DESTROY:
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)0);
			delete container;
			return TRUE;

	}
	return FALSE;
}


static void __cdecl StartThread(void *vContainer) {
	OleInitialize(NULL);
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GROUPCHAT_CONTAINER), NULL, ContainerDlgProc, (LPARAM) vContainer);
	//MessageBox(NULL, "ChatContainer dies.", "MW", MB_OK);
	OleUninitialize();

}
