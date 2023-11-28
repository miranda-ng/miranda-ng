/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

enum
{
	MENU_COPY, MENU_COPYTEXT, MENU_COPYURL, MENU_OPENFOLDER, MENU_QUOTE,
	MENU_SAVEAS, MENU_DOWNLOAD,
	MENU_EDIT, MENU_DELETE,
	MENU_SELECTALL, MENU_BOOKMARK,
};

static int hMenuObject;
static HANDLE hEventPreBuildMenu;
static HGENMENU hmiHistory, hmiOpenFolder, hmiCopyUrl, hmiSaveAs, hmiDownload, hmiQuote;
static HGENMENU hmiCopy, hmiCopyText, hmiEdit, hmiBookmark, hmiDelete;

HMENU NSMenu_Build(NewstoryListData *data, ItemData *item)
{
	bool bNotProtected = true;
	if (auto *szProto = Proto_GetBaseAccountName(item->hContact))
		bNotProtected = db_get_b(item->hContact, szProto, "Protected") == 0;

	Menu_ShowItem(hmiCopy, bNotProtected);
	Menu_ShowItem(hmiCopyText, bNotProtected);

	Menu_ShowItem(hmiQuote, bNotProtected && data->pMsgDlg != nullptr);
	Menu_ShowItem(hmiSaveAs, false);
	Menu_ShowItem(hmiCopyUrl, false);
	Menu_ShowItem(hmiDownload, false);
	Menu_ShowItem(hmiOpenFolder, false);

	bool bShowEventActions;
	if (item != nullptr) {
		if (item->m_bOfflineFile) {
			Menu_ModifyItem(hmiCopyUrl, (item->m_bOfflineDownloaded) ? TranslateT("Copy file path") : TranslateT("Copy URL"));
			Menu_ShowItem(hmiCopyUrl, true);
			Menu_ShowItem(hmiSaveAs, true);
			Menu_ShowItem(hmiDownload, !item->m_bOfflineDownloaded);
			Menu_ShowItem(hmiOpenFolder, item->m_bOfflineDownloaded);
		}

		bShowEventActions = item->hEvent != 0;

		DB::EventInfo dbei(item->hEvent);
		NotifyEventHooks(hEventPreBuildMenu, item->hContact, (LPARAM)&dbei);
	}
	else {
		bShowEventActions = false;

		DB::EventInfo dbei;
		NotifyEventHooks(hEventPreBuildMenu, 0, (LPARAM)&dbei);
	}

	Menu_ShowItem(hmiEdit, bShowEventActions);
	Menu_ShowItem(hmiDelete, bShowEventActions);
	Menu_ShowItem(hmiBookmark, bShowEventActions);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hMenuObject);
	return hMenu;
}

bool NSMenu_Process(int iCommand, NewstoryListData *data)
{
	if (Menu_ProcessCommandById(iCommand, LPARAM(data)))
		return true;

	if (auto *pDlg = data->pMsgDlg) {
		PostMessage(pDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
		if (Chat_DoEventHook(pDlg->getChat(), GC_USER_LOGMENU, nullptr, nullptr, iCommand))
			return true;
	}

	return false;
}

static INT_PTR NSMenuHelper(WPARAM wParam, LPARAM lParam)
{
	auto *pData = (NewstoryListData *)lParam;

	switch (wParam) {
	case MENU_COPY:
		pData->Copy(false);
		break;

	case MENU_COPYTEXT:
		pData->Copy(true);
		break;

	case MENU_COPYURL:
		pData->CopyUrl();
		break;

	case MENU_OPENFOLDER:
		pData->OpenFolder();
		break;

	case MENU_QUOTE:
		pData->Quote();
		break;

	case MENU_EDIT:
		pData->BeginEditItem();
		break;

	case MENU_DELETE:
		pData->DeleteItems();
		break;

	case MENU_SELECTALL:
		SendMessage(pData->m_hwnd, NSM_SELECTITEMS, 0, pData->totalCount - 1);
		break;

	case MENU_SAVEAS:
		pData->Download(OFD_SAVEAS | OFD_RUN);
		break;

	case MENU_DOWNLOAD:
		pData->Download(OFD_DOWNLOAD);
		break;

	case MENU_BOOKMARK:
		pData->ToggleBookmark();
		PostMessage(GetParent(pData->m_hwnd), UM_BOOKMARKS, 0, 0);
		break;
	}

	return 0;
}

static INT_PTR NSMenuAddService(WPARAM wParam, LPARAM lParam)
{
	auto *pmi = (TMO_MenuItem *)wParam;
	if (pmi == nullptr)
		return 0;

	auto *mmep = (NSMenuExecParam *)mir_calloc(sizeof(NSMenuExecParam));
	if (mmep == nullptr)
		return 0;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	mmep->iParam = lParam;

	HGENMENU hNewItem = Menu_AddItem(hMenuObject, pmi, mmep);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return (INT_PTR)hNewItem;
}

static INT_PTR NSMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (auto *pParam = (NSMenuExecParam *)wParam)
		CallService(pParam->szServiceName, pParam->iParam, lParam);

	return 0;
}

static INT_PTR NSMenuFreeOwnerData(WPARAM, LPARAM lParam)
{
	if (auto *param = (NSMenuExecParam *)lParam) {
		mir_free(param->szServiceName);
		mir_free(param);
	}
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hmiHistory, db_event_first(hContact) != 0);
	return 0;
}

void InitMenus()
{
	{	// Contact menu items
		CMenuItem mi(&g_plugin);
		SET_UID(mi, 0xc20d7a69, 0x7607, 0x4aad, 0xa7, 0x42, 0x10, 0x86, 0xfb, 0x32, 0x49, 0x21);
		mi.name.a = LPGEN("History");
		mi.position = 1000090000;
		mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_NEWSTORY);
		hmiHistory = Menu_AddContactMenuItem(&mi);
		CreateServiceFunction(mi.pszService, svcShowNewstory);

		HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);

		// Main menu items
		SET_UID(mi, 0x1848519e, 0x639d, 0x497a, 0xa5, 0x37, 0x6b, 0x76, 0x17, 0x2a, 0x6, 0xd9);
		mi.name.a = LPGEN("Global search");
		mi.position = 500060001;
		mi.pszService = "NewStory/GlobalSearch";
		Menu_AddMainMenuItem(&mi);
		CreateServiceFunction(mi.pszService, svcGlobalSearch);
	}

	// Init history item's menu
	CreateServiceFunction("NSMenu/Helper", NSMenuHelper);
	CreateServiceFunction("NSMenu/AddService", NSMenuAddService);
	CreateServiceFunction("NSMenu/ExecService", NSMenuExecService);
	CreateServiceFunction("NSMenu/FreeOwnerData", NSMenuFreeOwnerData);

	hEventPreBuildMenu = CreateHookableEvent(ME_NS_PREBUILDMENU);

	hMenuObject = Menu_AddObject("NSMenu", "NewStory item menu", nullptr, "NSMenu/ExecService");
	Menu_ConfigureObject(hMenuObject, MCO_OPT_USERDEFINEDITEMS, INT_PTR(FALSE));
	Menu_ConfigureObject(hMenuObject, MCO_OPT_FREE_SERVICE, INT_PTR("NSMenu/FreeOwnerData"));
	Menu_ConfigureObject(hMenuObject, MCO_OPT_ONADD_SERVICE, INT_PTR("NSMenu/OnAddService"));

	CMenuItem mi(&g_plugin);
	mi.pszService = "NSMenu/Helper";

	mi.position = 100000;
	mi.name.a = LPGEN("Copy");
	hmiCopy = Menu_AddNewStoryMenuItem(&mi, MENU_COPY);

	mi.position++;
	mi.name.a = LPGEN("Copy text");
	hmiCopyText = Menu_AddNewStoryMenuItem(&mi, MENU_COPYTEXT);

	mi.position++;
	mi.name.a = LPGEN("Copy URL");
	hmiCopyUrl = Menu_AddNewStoryMenuItem(&mi, MENU_COPYURL);

	mi.position++;
	mi.name.a = LPGEN("Show in folder");
	hmiOpenFolder = Menu_AddNewStoryMenuItem(&mi, MENU_OPENFOLDER);

	mi.position++;
	mi.name.a = LPGEN("Quote");
	hmiQuote = Menu_AddNewStoryMenuItem(&mi, MENU_QUOTE);

	mi.position++;
	mi.name.a = LPGEN("Save as");
	hmiSaveAs = Menu_AddNewStoryMenuItem(&mi, MENU_SAVEAS);

	mi.position++;
	mi.name.a = LPGEN("Download");
	hmiDownload = Menu_AddNewStoryMenuItem(&mi, MENU_DOWNLOAD);

	mi.position = 200000;
	mi.name.a = LPGEN("Edit");
	hmiEdit = Menu_AddNewStoryMenuItem(&mi, MENU_EDIT);

	mi.position++;
	mi.name.a = LPGEN("Delete");
	hmiDelete = Menu_AddNewStoryMenuItem(&mi, MENU_DELETE);

	mi.position++;
	mi.name.a = LPGEN("Toggle bookmark");
	hmiBookmark = Menu_AddNewStoryMenuItem(&mi, MENU_BOOKMARK);

	mi.position = 300000;
	mi.name.a = LPGEN("Select all");
	Menu_AddNewStoryMenuItem(&mi, MENU_SELECTALL);
}
