#include "stdafx.h"

static int hMenuObject;
static HANDLE hEventPreBuildMenu;
static HGENMENU hmiHistory, hmiCopy, hmiSaveAs, hmiDownload;

HMENU NSMenu_Build(ItemData *item)
{
	if (item->m_bOfflineFile) {
		Menu_ModifyItem(hmiCopy, (item->m_bOfflineDownloaded) ? TranslateT("Copy file name") : TranslateT("Copy URL"));
		Menu_ShowItem(hmiSaveAs, true);
		Menu_ShowItem(hmiDownload, !item->m_bOfflineDownloaded);
	}
	else {
		Menu_ShowItem(hmiSaveAs, false);
		Menu_ShowItem(hmiDownload, false);
	}

	NotifyEventHooks(hEventPreBuildMenu, item->hContact, (LPARAM)&item->dbe);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hMenuObject);
	return hMenu;
}

bool NSMenu_Process(int iCommand, NewstoryListData *data)
{
	return Menu_ProcessCommandById(iCommand, LPARAM(data)) != 0;
}

static INT_PTR NSMenuHelper(WPARAM wParam, LPARAM lParam)
{
	auto *pData = (NewstoryListData *)lParam;
	
	switch (wParam) {
	case 1:
		SendMessage(pData->hwnd, NSM_COPY, 0, 0);
		break;

	case 8:
		SendMessage(pData->hwnd, NSM_COPY, 1, 0);
		break;

	case 2:
		pData->BeginEditItem(pData->caret, false);
		break;

	case 3:
		pData->DeleteItems();
		break;

	case 4:
		SendMessage(pData->hwnd, NSM_SELECTITEMS, 0, pData->totalCount - 1);
		break;

	case 5:
		SendMessage(pData->hwnd, NSM_DOWNLOAD, 0, OFD_SAVEAS | OFD_RUN);
		break;

	case 6:
		SendMessage(pData->hwnd, NSM_DOWNLOAD, 0, OFD_DOWNLOAD);
		break;

	case 7:
		pData->ToggleBookmark();
		break;

	default:
		if (auto *pDlg = pData->pMsgDlg) {
			PostMessage(pDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
			Chat_DoEventHook(pDlg->getChat(), GC_USER_LOGMENU, nullptr, nullptr, wParam);
		}
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
		mi.name.a = LPGEN("User history");
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
	hmiCopy = Menu_AddNewStoryMenuItem(&mi, 1);

	mi.position = 100001;
	mi.name.a = LPGEN("Copy text");
	hmiCopy = Menu_AddNewStoryMenuItem(&mi, 8);

	mi.position = 100002;
	mi.name.a = LPGEN("Save as");
	hmiSaveAs = Menu_AddNewStoryMenuItem(&mi, 5);

	mi.position = 100003;
	mi.name.a = LPGEN("Download");
	hmiDownload = Menu_AddNewStoryMenuItem(&mi, 6);

	mi.position = 200000;
	mi.name.a = LPGEN("Edit");
	Menu_AddNewStoryMenuItem(&mi, 2);

	mi.position = 200001;
	mi.name.a = LPGEN("Delete");
	Menu_AddNewStoryMenuItem(&mi, 3);

	mi.position = 200002;
	mi.name.a = LPGEN("Toggle bookmark");
	Menu_AddNewStoryMenuItem(&mi, 7);

	mi.position = 300000;
	mi.name.a = LPGEN("Select all");
	Menu_AddNewStoryMenuItem(&mi, 4);
}
