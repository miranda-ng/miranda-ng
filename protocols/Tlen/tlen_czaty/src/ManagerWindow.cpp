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
#include "ManagerWindow.h"
#include "Utils.h"
#include "HelperDialog.h"

static INT_PTR CALLBACK ManagerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK ChatRoomsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK MyRoomsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

ManagerWindow *	ManagerWindow::list = NULL;
bool ManagerWindow::released = false;
CRITICAL_SECTION ManagerWindow::mutex;

void ManagerWindow::init() {
	released = false;
	InitializeCriticalSection(&mutex);
}

void ManagerWindow::release() {
	released = true;
	for (ManagerWindow *ptr2, *ptr = list; ptr != NULL; ptr=ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
//		SendMessage(ptr->getHWND(), WM_CLOSE, 0, 0);
	}
	DeleteCriticalSection(&mutex);
}

ManagerWindow::ManagerWindow(const char *module)
{
	next = prev = NULL;
	groups = NULL;
	rooms = NULL;
	myRooms = NULL;
	currentGroup = NULL;
	isStarted = 0;
	currentTab = 0;
	currentRoomsPage = 0;
	lastRoomsPage = 0;
	userNickList = userRoomList = 0;
	this->module = NULL;
	this->moduleName = NULL;
	Utils::copyString(&this->module, module);
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	EnterCriticalSection(&mutex);
	setNext(list);
	if (next != NULL) {
		next->setPrev(this);
	}
	list = this;
	LeaveCriticalSection(&mutex);
}

ManagerWindow::~ManagerWindow()
{
	if (!released) {
		EnterCriticalSection(&mutex);
		if (getPrev() != NULL) {
			getPrev()->setNext(next);
		} else {
			list = getNext();
		}
		if (getNext() != NULL) {
			getNext()->setPrev(prev);
		}
		LeaveCriticalSection(&mutex);
	}
	if (hWnd != NULL) {
		EndDialog(hWnd, 0);
	}
	if (hEvent != NULL) {
		CloseHandle(hEvent);
	}
	while (groups != NULL) {
		ChatGroup *group = groups;
		groups = groups->getListNext();
		delete group;
	}
	while (rooms != NULL) {
		ChatRoom *room = rooms;
		rooms = rooms->getNext();
		delete room;
	}
	while (myRooms != NULL) {
		ChatRoom *room = myRooms;
		myRooms = myRooms->getNext();
		delete room;
	}
	if (module != NULL) {
		delete module;
	}
	if (moduleName != NULL) {
		delete moduleName;
	}
}
void ManagerWindow::setPrev(ManagerWindow *prev)
{
	this->prev = prev;
}
ManagerWindow * ManagerWindow::getPrev()
{
	return prev;
}
void ManagerWindow::setNext(ManagerWindow *next)
{
	this->next = next;
}
ManagerWindow * ManagerWindow::getNext()
{
	return next;
}
void ManagerWindow::setHWND(HWND h)
{
	hWnd=h;
}
HWND ManagerWindow::getHWND()
{
	return hWnd;
}
void ManagerWindow::setChatRoomsTabHWND(HWND h)
{
	hChatRoomsTabWnd=h;
}
HWND ManagerWindow::getChatRoomsTabHWND()
{
	return hChatRoomsTabWnd;
}
void ManagerWindow::setMyRoomsTabHWND(HWND h)
{
	hMyRoomsTabWnd=h;
}
HWND ManagerWindow::getMyRoomsTabHWND()
{
	return hMyRoomsTabWnd;
}
void ManagerWindow::setMyNicksTabHWND(HWND h)
{
	hMyNicksTabWnd=h;
}
HWND ManagerWindow::getMyNicksTabHWND()
{
	return hMyNicksTabWnd;
}
const char *ManagerWindow::getModule()
{
	return module;
}
const char *ManagerWindow::getModuleName()
{
	return moduleName;
}
void ManagerWindow::setModuleName(const char *moduleName)
{
	Utils::copyString(&this->moduleName, moduleName);
}
ChatGroup *ManagerWindow::getCurrentGroup()
{
	return currentGroup;
}
void ManagerWindow::setCurrentGroup(ChatGroup *group)
{
	currentGroup = group;
}
HANDLE ManagerWindow::getEvent()
{
	return hEvent;
}
int ManagerWindow::getCurrentTab()
{
	return currentTab;
}
void ManagerWindow::setCurrentRoomsPage(int currentRoomsPage)
{
	this->currentRoomsPage = currentRoomsPage;
}
int ManagerWindow::getCurrentRoomsPage()
{
	return currentRoomsPage;
}
void ManagerWindow::setCurrentTab(int currentTab)
{
	this->currentTab = currentTab;
}
void ManagerWindow::setLastRoomsPage(int lastRoomsPage)
{
	this->lastRoomsPage = lastRoomsPage;
}
int ManagerWindow::isLastRoomsPage()
{
	return lastRoomsPage;
}
int ManagerWindow::isUserNickListReceived()
{
	return userNickList;
}
int ManagerWindow::isUserRoomListReceived()
{
	return userRoomList;
}
static void __cdecl StartThread(void *vManager)
{
	OleInitialize(NULL);
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHATMANAGER), NULL, ManagerDlgProc, (LPARAM) vManager);
	OleUninitialize();
}
int ManagerWindow::start()
{
	if (isStarted) {
		ShowWindow(hWnd, SW_SHOW);
		return 0;
	}
	isStarted = 1;
	Utils::forkThread((void (__cdecl *)(void *))StartThread, 0, (void *) this);
	WaitForSingleObject(hEvent, INFINITE);
	return 0;
}
void ManagerWindow::queryResultGroups(MUCCQUERYRESULT *queryResult)
{
	TVINSERTSTRUCT tvis;
	ChatGroup * par = NULL;
	ChatGroup * group;
	if (queryResult->pszParent != NULL) {
		par = findGroup(queryResult->pszParent);
		if (par != NULL) {
			if (par->getChild() != NULL) return;
		}
	}
	for (int i=0; i<queryResult->iItemsNum; i++) {
		group = new ChatGroup();
		group->setId(queryResult->pItems[i].pszID);
		group->setName(queryResult->pItems[i].pszName);
		if (par != NULL) {
			par->addChild(group);
			//group->setParent(par);
			tvis.hParent = par->getTreeItem();
		} else {
			tvis.hParent = NULL;
		}
		tvis.hInsertAfter = TVI_SORT;
		tvis.item.mask = TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM;
		LPTSTR lps1 = Utils::mucc_mir_a2t(queryResult->pItems[i].pszName);
		tvis.item.pszText = lps1;
		tvis.item.cChildren = queryResult->pItems[i].iCount;
		tvis.item.lParam = (LPARAM) group;
		group->setTreeItem(TreeView_InsertItem(GetDlgItem(hWnd, IDC_GROUP), &tvis));
		group->setListNext(groups);
		groups = group;
		if (par != NULL) {
			TreeView_Expand(GetDlgItem(hWnd, IDC_GROUP), par->getTreeItem(), TVE_EXPAND);
		}
		Utils::mucc_mir_free(lps1);
	}
}

void ManagerWindow::queryResultRooms(MUCCQUERYRESULT *queryResult)
{
	ChatRoom *room;
	while (rooms != NULL) {
		room = rooms;
		rooms = rooms->getNext();
		delete room;
	}
	TCHAR str[100];
	if (getCurrentGroup() == NULL || strcmp(getCurrentGroup()->getId(), queryResult->pszParent) || queryResult->iPage != getCurrentRoomsPage()) return;
	setLastRoomsPage(queryResult->iLastPage);
	if (queryResult->iLastPage) {
		EnableWindow(GetDlgItem(getChatRoomsTabHWND(), IDC_NEXT), FALSE);
	} else {
		EnableWindow(GetDlgItem(getChatRoomsTabHWND(), IDC_NEXT), TRUE);
	}
	if (queryResult->iPage>0) {
		EnableWindow(GetDlgItem(getChatRoomsTabHWND(), IDC_PREV), TRUE);
	} else {
		EnableWindow(GetDlgItem(getChatRoomsTabHWND(), IDC_PREV), FALSE);
	}
	for (int i=0; i<queryResult->iItemsNum; i++) {
		LVITEM lvItem;
		room = new ChatRoom();
		room->setId(queryResult->pItems[i].pszID);
		room->setName(queryResult->pItems[i].pszName);
		room->setFlags(queryResult->pItems[i].dwFlags);
		room->setNext(rooms);
		rooms=room;
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = 0;
		lvItem.iItem = ListView_GetItemCount(GetDlgItem(getChatRoomsTabHWND(), IDC_ROOM));
		LPTSTR lps1 = Utils::mucc_mir_a2t(queryResult->pItems[i].pszName);
		lvItem.pszText = lps1;
		lvItem.lParam = (LPARAM) room;
		ListView_InsertItem(GetDlgItem(getChatRoomsTabHWND(), IDC_ROOM), &lvItem);
		lvItem.iSubItem = 1;
		ListView_InsertItem(GetDlgItem(getChatRoomsTabHWND(), IDC_ROOM), &lvItem);
		_stprintf(str, _TEXT("%d"), queryResult->pItems[i].iCount);
		ListView_SetItemText(GetDlgItem(getChatRoomsTabHWND(), IDC_ROOM), lvItem.iItem, 1, str);
		Utils::mucc_mir_free(lps1);
	}
}

void ManagerWindow::queryResultUserRooms(MUCCQUERYRESULT *queryResult)
{
	ChatRoom *room;
	ListView_DeleteAllItems(GetDlgItem(getMyRoomsTabHWND(), IDC_LIST));
	while (myRooms != NULL) {
		room = myRooms;
		myRooms = myRooms->getNext();
		delete room;
	}
	for (int i=0; i<queryResult->iItemsNum; i++) {
		LVITEM lvItem;
		room = new ChatRoom();
		room->setId(queryResult->pItems[i].pszID);
		room->setName(queryResult->pItems[i].pszName);
		room->setNext(myRooms);
		myRooms=room;
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = 0;
		lvItem.iItem = ListView_GetItemCount(GetDlgItem(getMyRoomsTabHWND(), IDC_LIST));
		LPTSTR lps1 = Utils::mucc_mir_a2t(queryResult->pItems[i].pszName);
		lvItem.pszText = lps1;
		lvItem.lParam = (LPARAM) room;
		ListView_InsertItem(GetDlgItem(getMyRoomsTabHWND(), IDC_LIST), &lvItem);
		Utils::mucc_mir_free(lps1);
	}
	userRoomList = 1;
}

void ManagerWindow::queryResultUserNick(MUCCQUERYRESULT *queryResult)
{
	ListView_DeleteAllItems(GetDlgItem(getMyNicksTabHWND(), IDC_LIST));
	for (int i=0; i<queryResult->iItemsNum; i++) {
		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 0;
		lvItem.iItem = ListView_GetItemCount(GetDlgItem(getMyNicksTabHWND(), IDC_LIST));
		LPTSTR lps1 = Utils::mucc_mir_a2t(queryResult->pItems[i].pszName);
		lvItem.pszText = lps1;
		ListView_InsertItem(GetDlgItem(getMyNicksTabHWND(), IDC_LIST), &lvItem);
		Utils::mucc_mir_free(lps1);
	}
	userNickList = 1;
}

ChatGroup *ManagerWindow::findGroup(const char *id)
{
	ChatGroup *group;
	for (group = groups;group != NULL; group=group->getListNext()) {
		if (!strcmp(group->getId(), id)) break;
	}
	return group;
}


static INT_PTR CALLBACK ChatRoomsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND lv;
	LVCOLUMN lvCol;
	ManagerWindow *manager;
	manager = (ManagerWindow *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		manager = (ManagerWindow *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) manager);
		TranslateDialogDefault(hwndDlg);
		manager->setChatRoomsTabHWND(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_PREV, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_NEXT, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_SEARCH, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_PREV, BM_SETIMAGE, IMAGE_ICON, (LPARAM) muccIcon[MUCC_IDI_PREV]);
		SendDlgItemMessage(hwndDlg, IDC_NEXT, BM_SETIMAGE, IMAGE_ICON, (LPARAM) muccIcon[MUCC_IDI_NEXT]);
		SendDlgItemMessage(hwndDlg, IDC_SEARCH, BM_SETIMAGE, IMAGE_ICON, (LPARAM) muccIcon[MUCC_IDI_SEARCH]);

		lv = GetDlgItem(hwndDlg, IDC_ROOM);
		ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvCol.pszText = TranslateT("Name");
		lvCol.cx = 270;
		lvCol.iSubItem = 0;
		ListView_InsertColumn(lv, 0, &lvCol);
		lvCol.pszText = TranslateT("Persons");
		lvCol.cx = 50;
		lvCol.iSubItem = 1;
		ListView_InsertColumn(lv, 1, &lvCol);
		lvCol.pszText = _TEXT("");//Translate("");
		lvCol.cx = 46;
		lvCol.iSubItem = 2;
		ListView_InsertColumn(lv, 2, &lvCol);
		return TRUE;
	case WM_NOTIFY:
		switch (wParam) {
			case IDC_ROOM:
				switch (((LPNMHDR)lParam)->code) {
					case LVN_COLUMNCLICK:
						{
							LPNMLISTVIEW pnmlv = (LPNMLISTVIEW) lParam;
							if (pnmlv->iSubItem >= 0 && pnmlv->iSubItem <= 1) {
		//						if (pnmlv->iSubItem == sortColumn)
		//							sortAscending = !sortAscending;
		//						else {
		//						sortAscending = TRUE;
		//							sortColumn = pnmlv->iSubItem;
		//						}
		//						ListView_SortItems(GetDlgItem(hwndDlg, IDC_ROOM), GroupchatCompare, sortColumn);
							}
						}
						break;
					case NM_DBLCLK:
						{
							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							LVITEM item;
							item.iItem = lpnmitem->iItem;
							item.iSubItem = 0;
							item.mask = LVIF_PARAM;
							if (ListView_GetItem(GetDlgItem(hwndDlg, IDC_ROOM), &item)) {
								MUCCEVENT event;
								ChatRoom *room = (ChatRoom *)item.lParam;
								Utils::log("entering room %s ", room->getId());
								event.cbSize = sizeof(MUCCEVENT);
								event.iType = MUCC_EVENT_JOIN;
								event.pszModule = manager->getModule();
								event.pszID = room->getId();
								event.pszName = room->getName();
								event.dwFlags = room->getFlags();
								event.pszNick = NULL;
								HelperDialog::joinDlg(&event);
							}
						}
						break;
				}
				break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_NEXT:
				if (!manager->isLastRoomsPage()) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_NEXT), FALSE);
					manager->setCurrentRoomsPage(manager->getCurrentRoomsPage()+1);
					SendMessage(GetParent(hwndDlg), WM_MUCC_REFRESH_ROOMS, 0, 0);
				}
				break;
			case IDC_PREV:
				if (manager->getCurrentRoomsPage()>0) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_NEXT), FALSE);
					manager->setCurrentRoomsPage(manager->getCurrentRoomsPage()-1);
				}
				SendMessage(GetParent(hwndDlg), WM_MUCC_REFRESH_ROOMS, 0, 0);
				break;
			case IDC_SEARCH:
				{
					MUCCEVENT event;
					event.cbSize = sizeof(MUCCEVENT);
					event.iType = MUCC_EVENT_JOIN;
					event.dwFlags = MUCC_EF_ROOM_NAME;
					event.pszModule = manager->getModule();
					event.pszID = NULL;
					event.pszName = NULL;
					event.pszNick = NULL;
					HelperDialog::joinDlg(&event);
					break;
				}
		}
		break;
	case WM_MEASUREITEM:
		if (wParam == IDC_ROOM) {
			MEASUREITEMSTRUCT *lpMis = (MEASUREITEMSTRUCT *) lParam;
			lpMis->itemHeight = GetSystemMetrics(SM_CYSMICON);
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_ROOM) {
			RECT rc;
			int w, x, col;
			DRAWITEMSTRUCT *lpDis = (DRAWITEMSTRUCT *) lParam;
			ChatRoom *room = NULL;
//			if (ListView_GetItem(GetDlgItem(hwndDlg, IDC_ROOM), &item)) {
				room = (ChatRoom *)lpDis->itemData;// (ChatRoom *)item.lParam;
//			}
			TCHAR text[256];
			switch (lpDis->itemAction) {
				default:
				case ODA_SELECT:
				case ODA_DRAWENTIRE:
					if (lpDis->itemState & ODS_SELECTED) {
						HBRUSH hBrush = CreateSolidBrush(RGB(0xC2, 0xC8, 0xDA));//0xDAC8C2);
						FillRect(lpDis->hDC, &(lpDis->rcItem), hBrush);//(HBRUSH) (COLOR_HIGHLIGHT+1));
						DeleteObject(hBrush);
						SetTextColor(lpDis->hDC, 0);
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					else {
//						HBRUSH hBrush = CreateSolidBrush(RGB(0xC2, 0xC8, 0xDA));//0xDAC8C2);
						FillRect(lpDis->hDC, &(lpDis->rcItem), (HBRUSH) (COLOR_WINDOW+1));
//						DeleteObject(hBrush);
						SetTextColor(lpDis->hDC, RGB(0, 0, 0));//GetSysColor(COLOR_WINDOWTEXT));
//						SetBkColor(lpDis->hDC, GetSysColor(COLOR_WINDOW+1));
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					x = lpDis->rcItem.left;
					for (col=0;col<3;col++) {
						ListView_GetItemText(GetDlgItem(hwndDlg, IDC_ROOM), lpDis->itemID, col, text, sizeof(text)/sizeof(TCHAR));
						w = ListView_GetColumnWidth(GetDlgItem(hwndDlg, IDC_ROOM), col);
						rc.left = x;
						rc.top = lpDis->rcItem.top;
						rc.bottom = lpDis->rcItem.bottom;
						rc.right = x+w;
						if (col == 0) {
							rc.left+=2;
							rc.right-=2;
							DrawText(lpDis->hDC, text, (int)_tcslen(text), &rc, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
						}
						else if (col < 2) {
							DrawText(lpDis->hDC, text, (int)_tcslen(text), &rc, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
						} else {
							if (room->getFlags()&MUCC_EF_ROOM_MODERATED) {
								DrawIconEx(lpDis->hDC, x, rc.top, muccIcon[MUCC_IDI_R_MODERATED], GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
							}
							if (room->getFlags()&MUCC_EF_ROOM_MEMBERS_ONLY) {
								DrawIconEx(lpDis->hDC, x+12+0*GetSystemMetrics(SM_CXSMICON), rc.top, muccIcon[MUCC_IDI_R_MEMBERS], GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
							}
							if (!(room->getFlags()&MUCC_EF_ROOM_NICKNAMES)) {
								DrawIconEx(lpDis->hDC, x+24+0*GetSystemMetrics(SM_CXSMICON), rc.top, muccIcon[MUCC_IDI_R_ANONYMOUS], GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
							}
						}
						x += w;
					}
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK MyRoomsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND lv;
	LVCOLUMN lvCol;
	ManagerWindow *manager;
	manager = (ManagerWindow *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		manager = (ManagerWindow *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) manager);
		TranslateDialogDefault(hwndDlg);
		manager->setMyRoomsTabHWND(hwndDlg);
		lv = GetDlgItem(hwndDlg, IDC_LIST);
		ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);
		GetClientRect(lv, &rc);
		lvCol.mask = LVCF_WIDTH;
		lvCol.cx = rc.right - rc.left;
		ListView_InsertColumn(lv, 0, &lvCol);
		return TRUE;
	case WM_NOTIFY:
		switch (wParam) {
		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case NM_DBLCLK:
				{
					LVITEM item;
					item.iItem = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LIST));
					if (item.iItem >= 0) {
						item.iSubItem = 0;
						item.mask = LVIF_PARAM;
						if (ListView_GetItem(GetDlgItem(hwndDlg, IDC_LIST), &item)) {
							ChatRoom *room = (ChatRoom *)item.lParam;
							Utils::log("entering room %s (%s)", room->getId(), room->getId());
							MUCCEVENT muce;
							muce.cbSize = sizeof(MUCCEVENT);
							muce.iType = MUCC_EVENT_JOIN;
							muce.pszModule = manager->getModule();
							muce.pszID = room->getId();
							muce.pszName = room->getName();
							muce.dwFlags = MUCC_EF_ROOM_NICKNAMES;
							muce.pszNick = NULL;
							HelperDialog::joinDlg(&muce);
						}
					}
				}
				break;
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_REFRESH:
				SendMessage(GetParent(hwndDlg), WM_MUCC_REFRESH_USER_ROOMS, 0, 0);
				break;
			case IDC_REGISTER:
				{
					MUCCEVENT muce;
					if (manager->getCurrentGroup() != NULL) {
						TCHAR name[256];
						GetDlgItemText(hwndDlg, IDC_NAME, name, 255);
						if (_tcslen(name) != 0) {
							int flags = 0;
							if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_PUBLIC)) {
								flags |= MUCC_EF_ROOM_PUBLIC;
							}
							if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_PERMANENT)) {
								flags |= MUCC_EF_ROOM_PERMANENT;
							}
							if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MEMBERS)) {
								flags |= MUCC_EF_ROOM_MEMBERS_ONLY;
							}
							if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_NICKNAMES)) {
								flags |= MUCC_EF_ROOM_NICKNAMES;
							}
							muce.cbSize = sizeof(MUCCEVENT);
							muce.iType = MUCC_EVENT_REGISTER_ROOM;
							muce.pszModule = manager->getModule();
							muce.pszID = manager->getCurrentGroup()->getId();
							char* lps1 = Utils::mucc_mir_t2a(name);
							muce.pszName = lps1;
							muce.pszNick = NULL;
							muce.dwFlags = flags;
							NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
							Utils::mucc_mir_free(lps1);
						}
					} else {
						muce.pszText = "Please select a group first.";
						HelperDialog::errorDlg(&muce);
					}
				}
				break;
		}
		break;
	case WM_MEASUREITEM:
		if (wParam == IDC_LIST) {
			MEASUREITEMSTRUCT *lpMis = (MEASUREITEMSTRUCT *) lParam;
			lpMis->itemHeight = 16;//GetSystemMetrics(SM_CYSMICON);
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_LIST) {
			TCHAR text[256];
			DRAWITEMSTRUCT *lpDis = (DRAWITEMSTRUCT *) lParam;
			switch (lpDis->itemAction) {
				default:
				case ODA_SELECT:
				case ODA_DRAWENTIRE:
					if (lpDis->itemState & ODS_SELECTED) {
						HBRUSH hBrush = CreateSolidBrush(RGB(0xC2, 0xC8, 0xDA));//0xDAC8C2);
						FillRect(lpDis->hDC, &(lpDis->rcItem), hBrush);//(HBRUSH) (COLOR_HIGHLIGHT+1));
						DeleteObject(hBrush);
						SetTextColor(lpDis->hDC, 0);
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					else {
						FillRect(lpDis->hDC, &(lpDis->rcItem), (HBRUSH) (COLOR_WINDOW+1));
						SetTextColor(lpDis->hDC, RGB(0, 0, 0));//GetSysColor(COLOR_WINDOWTEXT));
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LIST), lpDis->itemID, 0, text, sizeof(text)/sizeof(TCHAR));
					DrawText(lpDis->hDC, text, (int)_tcslen(text), &(lpDis->rcItem), DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
				break;
			}
		}
		break;

	}
	return FALSE;
}

static INT_PTR CALLBACK MyNicksDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND lv;
	LVCOLUMN lvCol;
	ManagerWindow *manager;
	manager = (ManagerWindow *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		manager = (ManagerWindow *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) manager);
		TranslateDialogDefault(hwndDlg);
		manager->setMyNicksTabHWND(hwndDlg);
		lv = GetDlgItem(hwndDlg, IDC_LIST);
		ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT);
		GetClientRect(lv, &rc);
		lvCol.mask = LVCF_WIDTH;
		lvCol.cx = rc.right - rc.left;
		ListView_InsertColumn(lv, 0, &lvCol);
		return TRUE;
	case WM_NOTIFY:
		switch (wParam) {
			case IDC_LIST:
				break;

		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_REFRESH:
				SendMessage(GetParent(hwndDlg), WM_MUCC_REFRESH_USER_NICKS, 0, 0);
				break;
			case IDC_REGISTER:
				{
					char nick[256];
					GetDlgItemTextA(hwndDlg, IDC_NICK, nick, 255);
					if (strlen(nick) != 0) {
						MUCCEVENT muce;
						muce.cbSize = sizeof(MUCCEVENT);
						muce.iType = MUCC_EVENT_REGISTER_NICK;
						muce.pszModule = manager->getModule();
						muce.pszNick = nick;
						NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
					}
				}
				break;
			case IDC_REMOVE:
				{
					LVITEM item;
					item.iItem = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LIST));
					if (item.iItem >= 0) {
						TCHAR text[256];
						item.iSubItem = 0;
						item.mask = LVIF_TEXT;
						item.pszText = text;
						item.cchTextMax = sizeof(text);
						if (ListView_GetItem(GetDlgItem(hwndDlg, IDC_LIST), &item)) {
							MUCCEVENT muce;
							muce.cbSize = sizeof(MUCCEVENT);
							muce.iType = MUCC_EVENT_REMOVE_NICK;
							muce.pszModule = manager->getModule();
							char* lps2 = Utils::mucc_mir_t2a(text);
							muce.pszNick = lps2;
							NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
							Utils::mucc_mir_free(lps2);
						}
					}
				}
		}
		break;
	case WM_MEASUREITEM:
		if (wParam == IDC_LIST) {
			MEASUREITEMSTRUCT *lpMis = (MEASUREITEMSTRUCT *) lParam;
			lpMis->itemHeight = 16;//GetSystemMetrics(SM_CYSMICON);
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_LIST) {
			TCHAR text[256];
			DRAWITEMSTRUCT *lpDis = (DRAWITEMSTRUCT *) lParam;
			switch (lpDis->itemAction) {
				default:
				case ODA_SELECT:
				case ODA_DRAWENTIRE:
					if (lpDis->itemState & ODS_SELECTED) {
						HBRUSH hBrush = CreateSolidBrush(RGB(0xC2, 0xC8, 0xDA));//0xDAC8C2);
						FillRect(lpDis->hDC, &(lpDis->rcItem), hBrush);//(HBRUSH) (COLOR_HIGHLIGHT+1));
						DeleteObject(hBrush);
						SetTextColor(lpDis->hDC, 0);
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					else {
						FillRect(lpDis->hDC, &(lpDis->rcItem), (HBRUSH) (COLOR_WINDOW+1));
						SetTextColor(lpDis->hDC, RGB(0, 0, 0));//GetSysColor(COLOR_WINDOWTEXT));
						SetBkMode(lpDis->hDC, TRANSPARENT);
					}
					ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LIST), lpDis->itemID, 0, text, sizeof(text)/sizeof(TCHAR));
					DrawText(lpDis->hDC, text, (int)_tcslen(text), &(lpDis->rcItem), DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK ManagerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd, tc;
	TCITEM tci;
	char text[128];
//	HIMAGELIST hImageList;
	ManagerWindow *manager;
	MUCCEVENT muce;
	int dlgWidth, dlgHeight, tabPos;
	RECT rc2;

	manager = (ManagerWindow *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (manager == NULL && msg != WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		// lParam is the initial conference server (if any)
		manager = (ManagerWindow *)lParam;
		manager->setHWND(hwndDlg);
		sprintf(text, "%s Chats", manager->getModuleName());
		SetWindowTextA(hwndDlg, text);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) manager);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) muccIcon[MUCC_IDI_CHAT]);

		TranslateDialogDefault(hwndDlg);
//		sortColumn = -1;
		manager->setCurrentTab(0);
		tc = GetDlgItem(hwndDlg, IDC_TABS);
		tci.mask = TCIF_TEXT;
		// Public rooms tab
		tci.pszText = TranslateT("Chat rooms");
		TabCtrl_InsertItem(tc, 0, &tci);
		// Create room tab
		tci.pszText = TranslateT("My rooms");
		TabCtrl_InsertItem(tc, 1, &tci);
		tci.pszText = TranslateT("My nicknames");
		TabCtrl_InsertItem(tc, 2, &tci);
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TAB_CHATROOMS), hwndDlg, ChatRoomsDlgProc, (LPARAM) manager);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(hwnd, SW_SHOW);
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TAB_MYROOMS), hwndDlg, MyRoomsDlgProc, (LPARAM) manager);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_TAB_MYNICKNAMES), hwndDlg, MyNicksDlgProc, (LPARAM) manager);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_PREV), FALSE);
		EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_NEXT), FALSE);

		GetWindowRect(manager->getMyRoomsTabHWND(), &rc2);
		rc2.right-=rc2.left;
		rc2.bottom-=rc2.top;
		SetWindowPos(hwndDlg, NULL, 0, 0, rc2.right+300, 118+rc2.bottom, SWP_NOMOVE | SWP_NOZORDER);
		TreeView_SetItemHeight(GetDlgItem(hwndDlg, IDC_GROUP), 16);
//		hImageList=ImageList_Create(100, 100, ILC_COLOR32, 0, 10);
//		ListView_SetImageList(GetDlgItem(hwndDlg, IDC_ROOM), hImageList, LVSIL_SMALL);
		SetEvent(manager->getEvent());
		muce.cbSize = sizeof(MUCCEVENT);
		muce.iType = MUCC_EVENT_QUERY_GROUPS;
		muce.pszModule = manager->getModule();
		muce.pszID = NULL;
		NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		return TRUE;
	case WM_SIZING:
			RECT *rc, rc2;
			rc = (RECT *)lParam;
			GetWindowRect(manager->getMyRoomsTabHWND(), &rc2);
			rc2.right-=rc2.left;
			rc2.bottom-=rc2.top;
			if (rc->right-rc->left<(rc2.right+80)) {
				if (wParam == WMSZ_RIGHT || wParam == WMSZ_BOTTOMRIGHT || wParam == WMSZ_TOPRIGHT) {
					rc->right=rc->left+rc2.right+80;
				} else {
					rc->left=rc->right-rc2.right-80;
				}
			}
			if (rc->bottom-rc->top<118+rc2.bottom || rc->bottom-rc->top>118+rc2.bottom) {
				if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT) {
					rc->bottom=rc->top+118+rc2.bottom;
				} else {
					rc->top=rc->bottom-118-rc2.bottom;
				}
			}
			return TRUE;
	case WM_SIZE:
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			HDWP hdwp;
			dlgWidth = LOWORD(lParam);
			dlgHeight = HIWORD(lParam);
			GetWindowRect(manager->getMyRoomsTabHWND(), &rc2);
			rc2.right-=rc2.left;
			rc2.bottom-=rc2.top;
			tabPos = dlgWidth - rc2.right - 15;
			hdwp = BeginDeferWindowPos(6);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_TABS), 0, tabPos, 10, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			hdwp = DeferWindowPos(hdwp, manager->getChatRoomsTabHWND(), 0, tabPos+3, 45, 371, 292, SWP_NOZORDER | SWP_NOSIZE);
			hdwp = DeferWindowPos(hdwp, manager->getMyRoomsTabHWND()  , 0, tabPos+3, 45, 371, 292, SWP_NOZORDER | SWP_NOSIZE);
			hdwp = DeferWindowPos(hdwp, manager->getMyNicksTabHWND()  , 0, tabPos+3, 45, 371, 292, SWP_NOZORDER | SWP_NOSIZE);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_GROUP), 0, 0, 0, tabPos-16, dlgHeight-40, SWP_NOZORDER | SWP_NOMOVE);
			hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDCLOSE), 0, dlgWidth-88, dlgHeight-34, 0, 0 , SWP_NOZORDER | SWP_NOSIZE);
			EndDeferWindowPos(hdwp);
		}
		break;
	case WM_NOTIFY:
		switch (wParam) {
		case IDC_GROUP:
			switch (((LPNMHDR)lParam)->code) {
				case TVN_ITEMEXPANDING:
				{
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;
					if (pnmtv->action == TVE_EXPAND) {
						TVITEM item = (TVITEM) pnmtv->itemNew;
						ChatGroup *group = (ChatGroup *) item.lParam;// manager->findGroup(item.hItem);
						if (group != NULL) {
							if (group->getChild() == NULL) {
								muce.cbSize = sizeof(MUCCEVENT);
								muce.iType = MUCC_EVENT_QUERY_GROUPS;
								muce.pszModule = manager->getModule();
								muce.pszID = group->getId();
								NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
							}
						}
					}
				}
				break;
				case TVN_SELCHANGED:
				{
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) lParam;
					TVITEM item = (TVITEM) pnmtv->itemNew;
					ChatGroup *group = (ChatGroup *) item.lParam;// manager->findGroup(item.hItem);
					if (group != NULL) {
						EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_PREV), FALSE);
						EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_NEXT), FALSE);
						ListView_DeleteAllItems(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_ROOM));
						manager->setCurrentGroup(group);
						manager->setCurrentRoomsPage(0);
						muce.cbSize = sizeof(MUCCEVENT);
						muce.iType = MUCC_EVENT_QUERY_ROOMS;
						muce.pszModule = manager->getModule();
						muce.pszID = (char *)group->getId();
						muce.dwData = 0;
						NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
					}
				}
				break;
				case NM_CUSTOMDRAW:
				{
					if (((LPNMHDR)lParam)->idFrom == IDC_GROUP) {
						LPNMTVCUSTOMDRAW pCustomDraw = (LPNMTVCUSTOMDRAW)lParam;
						switch (pCustomDraw->nmcd.dwDrawStage) {
							case CDDS_PREPAINT:
								SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,CDRF_NOTIFYITEMDRAW);
								return TRUE;
							case CDDS_ITEMPREPAINT:
								{
									if (pCustomDraw->nmcd.uItemState & CDIS_FOCUS) { // selected (CDIS_FOCUS | CDIS_SELECTED
										pCustomDraw->clrText = 0;
										pCustomDraw->clrTextBk = 0xDAC8C2;
									}
									else {
										pCustomDraw->clrText = 0;
									}
							//	HBRUSH hBr;
							//	hBr = CreateSolidBrush(pCustomDraw->clrTextBk);//g_LogOptions.crUserListBGColor ) ;
							//	FillRect(pCustomDraw->nmcd.hdc, &rc, hBr);
							//	DeleteObject(hBr);
								SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT, CDRF_NEWFONT);
								return TRUE;
							}
						}
					}
				}
				break;
			}
			break;
		case IDC_TABS:
			switch (((LPNMHDR) lParam)->code) {
			case TCN_SELCHANGE:
				switch (manager->getCurrentTab()) {
					case 0:
						ShowWindow(manager->getChatRoomsTabHWND(), SW_HIDE);
						break;
					case 1:
						ShowWindow(manager->getMyRoomsTabHWND(), SW_HIDE);
						break;
					case 2:
						ShowWindow(manager->getMyNicksTabHWND(), SW_HIDE);
						break;
				}
				manager->setCurrentTab(TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS)));
				switch (manager->getCurrentTab()) {
				case 0:
					ShowWindow(manager->getChatRoomsTabHWND(), SW_SHOW);
					break;
				case 1:
					ShowWindow(manager->getMyRoomsTabHWND(), SW_SHOW);
					if (!manager->isUserRoomListReceived()) {
						SendMessage(hwndDlg, WM_MUCC_REFRESH_USER_ROOMS, 0, 0);
					}
					break;
				case 2:
					ShowWindow(manager->getMyNicksTabHWND(), SW_SHOW);
					if (!manager->isUserNickListReceived()) {
						SendMessage(hwndDlg, WM_MUCC_REFRESH_USER_NICKS, 0, 0);
					}
					break;
				}
				break;
			}
			break;
		}
		break;
	case WM_MUCC_REFRESH_USER_ROOMS:
		muce.cbSize = sizeof(MUCCEVENT);
		muce.iType = MUCC_EVENT_QUERY_USER_ROOMS;
		muce.pszModule = manager->getModule();
		muce.pszID = NULL;
		NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		break;
	case WM_MUCC_REFRESH_USER_NICKS:
		muce.cbSize = sizeof(MUCCEVENT);
		muce.iType = MUCC_EVENT_QUERY_USER_NICKS;
		muce.pszModule = manager->getModule();
		muce.pszID = NULL;
		NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		break;
	case WM_MUCC_REFRESH_ROOMS:
		{
			ChatGroup *group = manager->getCurrentGroup();
			if (manager->getCurrentGroup() != NULL) {
				EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_PREV), FALSE);
				EnableWindow(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_NEXT), FALSE);
				ListView_DeleteAllItems(GetDlgItem(manager->getChatRoomsTabHWND(), IDC_ROOM));
				/*
				 * TODO: delete rooms here !!!
				 */
				muce.cbSize = sizeof(MUCCEVENT);
				muce.iType = MUCC_EVENT_QUERY_ROOMS;
				muce.pszModule = manager->getModule();
				muce.pszID = (char *)group->getId();
				muce.dwData = manager->getCurrentRoomsPage();
				NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDCLOSE:
				ShowWindow(hwndDlg, SW_HIDE);
				break;
		}
		break;
	case WM_CLOSE:
		ShowWindow(hwndDlg, SW_HIDE);
		break;

	}
	return FALSE;
}

ManagerWindow* ManagerWindow::getWindow(const char *module) {
	ManagerWindow *ptr;
	EnterCriticalSection(&mutex);
	for (ptr=list;ptr != NULL;ptr=ptr->getNext()) {
		if (strcmp(ptr->getModule(), module) == 0) {
			break;
		}
	}
	LeaveCriticalSection(&mutex);
	if (ptr == NULL) {
		ptr = new ManagerWindow(module);
	}
	return ptr;
}

