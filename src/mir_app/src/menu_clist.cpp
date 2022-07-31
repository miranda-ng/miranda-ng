/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "stdafx.h"
#pragma hdrstop

#include "m_hotkeys.h"
#include "m_history.h"

#include "clc.h"
#include "genmenu.h"

void InitGroupMenus(void);
void InitFramesMenus(void);
void InitProtoMenus(void);
void InitTrayMenus(void);

#define MS_CLIST_HKSTATUS "Clist/HK/SetStatus"

#define FIRSTCUSTOMMENUITEMID	30000

#define MENU_CUSTOMITEMMAIN   0x80000000

// new menu sys
int hMainMenuObject = 0, hContactMenuObject = 0, hStatusMenuObject = 0;
int currentStatusMenuItem;

void Proto_SetStatus(const char *szProto, unsigned status);

OBJLIST<MenuProto> g_menuProtos(1);

bool prochotkey;

HANDLE hPreBuildMainMenuEvent, hStatusModeChangeEvent, hPreBuildContactMenuEvent, hPreBuildStatusMenuEvent;

HMENU hMainMenu, hStatusMenu;

MStatus g_statuses[MAX_STATUS_COUNT] =
{
	{ ID_STATUS_OFFLINE, SKINICON_STATUS_OFFLINE, -1 },
	{ ID_STATUS_ONLINE, SKINICON_STATUS_ONLINE, PF2_ONLINE },
	{ ID_STATUS_AWAY, SKINICON_STATUS_AWAY, PF2_SHORTAWAY },
	{ ID_STATUS_NA, SKINICON_STATUS_NA, PF2_LONGAWAY },
	{ ID_STATUS_OCCUPIED, SKINICON_STATUS_OCCUPIED, PF2_LIGHTDND },
	{ ID_STATUS_DND, SKINICON_STATUS_DND, PF2_HEAVYDND },
	{ ID_STATUS_FREECHAT, SKINICON_STATUS_FREE4CHAT, PF2_FREECHAT },
	{ ID_STATUS_INVISIBLE, SKINICON_STATUS_INVISIBLE, PF2_INVISIBLE },
};

static int statustopos(int status)
{
	for (auto &it : g_statuses)
		if (status == it.iStatus)
			return int(&it - g_statuses);

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// service functions

static int RecursiveDeleteMenu(HMENU hMenu)
{
	int cnt = GetMenuItemCount(hMenu);
	for (int i = 0; i < cnt; i++) {
		HMENU submenu = GetSubMenu(hMenu, 0);
		if (submenu) DestroyMenu(submenu);
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MAIN MENU

struct MainMenuExecParam
{
	char *szServiceName;
	wchar_t *szMenuName;
	TMO_IntMenuItem *pimi;
};

MIR_APP_DLL(HMENU) Menu_GetMainMenu(void)
{
	RecursiveDeleteMenu(hMainMenu);

	NotifyEventHooks(hPreBuildMainMenuEvent, 0, 0);

	Menu_Build(hMainMenu, hMainMenuObject);
	DrawMenuBar(g_clistApi.hwndContactList);
	return hMainMenu;
}

MIR_APP_DLL(HGENMENU) Menu_AddMainMenuItem(TMO_MenuItem *pmi)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)mir_alloc(sizeof(MainMenuExecParam));
	if (mmep == nullptr)
		return nullptr;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	mmep->szMenuName = pmi->name.w;

	TMO_IntMenuItem *pimi = Menu_AddItem(hMainMenuObject, pmi, mmep);
	if (pimi == nullptr)
		return nullptr;

	mmep->pimi = pimi;

	const char* name;
	bool needFree = false;

	if (pmi->pszService)
		name = pmi->pszService;
	else if (pmi->flags & CMIF_UNICODE) {
		name = mir_u2a(pmi->name.w);
		needFree = true;
	}
	else name = pmi->name.a;

	Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, name);
	if (needFree)
		mir_free((void*)name);

	return pimi;
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
static INT_PTR MainMenuExecService(WPARAM wParam, LPARAM lParam)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)wParam;
	if (mmep != nullptr) {
		CallService(mmep->szServiceName, mmep->pimi->execParam, lParam);
	}
	return 1;
}

static INT_PTR FreeOwnerDataMainMenu(WPARAM, LPARAM lParam)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)lParam;
	if (mmep != nullptr) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CONTACT MENU

struct ContactMenuExecParam
{
	char *szServiceName;
	char *pszContactOwner;
	TMO_IntMenuItem *pimi;
};

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HGENMENU) Menu_AddContactMenuItem(TMO_MenuItem *pmi, const char *pszProto)
{
	// owner data
	ContactMenuExecParam *cmep = (ContactMenuExecParam*)mir_calloc(sizeof(ContactMenuExecParam));
	cmep->szServiceName = mir_strdup(pmi->pszService);
	if (pszProto != nullptr)
		cmep->pszContactOwner = mir_strdup(pszProto);

	// may be need to change how UniqueName is formed?
	TMO_IntMenuItem *pimi = Menu_AddItem(hContactMenuObject, pmi, cmep);
	if (pimi == nullptr)
		return nullptr;
	
	cmep->pimi = pimi;

	if (pszProto == nullptr)
		pszProto = "";

	char buf[256];
	if (pmi->pszService) {
		mir_snprintf(buf, "%s/%s", pszProto, (pmi->pszService) ? pmi->pszService : "");
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);
	}
	else if (pmi->name.w) {
		if (pmi->flags & CMIF_UNICODE)
			mir_snprintf(buf, "%s/NoService/%S", pszProto, pmi->name.w);
		else
			mir_snprintf(buf, "%s/NoService/%s", pszProto, pmi->name.a);
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);
	}
	return pimi;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct BuildContactParam
{
	char *szProto;
	int isOnList;
	int isOnline;
};

EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildContactMenu(MCONTACT hContact)
{
	NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

	char *szProto = Proto_GetBaseAccountName(hContact);

	BuildContactParam bcp;
	bcp.szProto = szProto;
	bcp.isOnList = Contact::OnList(hContact);
	bcp.isOnline = (szProto != nullptr && ID_STATUS_OFFLINE != db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE));

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hContactMenuObject, (WPARAM)&bcp);
	return hMenu;
}

// true - ok, false ignore
static INT_PTR ContactMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == nullptr)
		return FALSE;

	BuildContactParam *bcp = (BuildContactParam*)pcpp->wParam;
	if (bcp == nullptr)
		return FALSE;

	ContactMenuExecParam *cmep = (ContactMenuExecParam*)pcpp->MenuItemOwnerData;
	if (cmep == nullptr) //this is root...build it
		return TRUE;

	if (cmep->pszContactOwner != nullptr) {
		if (bcp->szProto == nullptr) return FALSE;
		if (mir_strcmp(cmep->pszContactOwner, bcp->szProto)) return FALSE;
	}

	TMO_MenuItem &mi = pcpp->MenuItemHandle->mi;
	if (mi.flags & CMIF_HIDDEN) return FALSE;
	if (mi.flags & CMIF_NOTONLIST  && bcp->isOnList) return FALSE;
	if (mi.flags & CMIF_NOTOFFLIST && !bcp->isOnList) return FALSE;
	if (mi.flags & CMIF_NOTONLINE  && bcp->isOnline) return FALSE;
	if (mi.flags & CMIF_NOTOFFLINE && !bcp->isOnline) return FALSE;
	return TRUE;
}

// called with:
// wparam - ContactMenuExecParam*
// lparam - lparam from winproc
static INT_PTR ContactMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		ContactMenuExecParam *cmep = (ContactMenuExecParam*)wParam;
		if (cmep->pszContactOwner && cmep->szServiceName && cmep->szServiceName[0] == '/')
			ProtoCallService(cmep->pszContactOwner, cmep->szServiceName, lParam, cmep->pimi->execParam);
		else
			CallService(cmep->szServiceName, lParam, cmep->pimi->execParam);
	}
	return 0;
}

// called with:
// lparam - ContactMenuExecParam*
static INT_PTR FreeOwnerDataContactMenu(WPARAM, LPARAM lParam)
{
	ContactMenuExecParam *cmep = (ContactMenuExecParam*)lParam;
	if (cmep != nullptr) {
		mir_free(cmep->szServiceName);
		mir_free(cmep->pszContactOwner);
		mir_free(cmep);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// STATUS MENU

struct StatusMenuExecParam
{
	char *szProto;  // This is unique protoname
	HGENMENU pimi;
	int status;

	BOOL custom;
	char *svc;
	HANDLE hMenuItem;
};

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HGENMENU) Menu_AddStatusMenuItem(TMO_MenuItem *pmi, const char *pszProto)
{
	TMO_IntMenuItem *pRoot = MO_GetIntMenuItem(pmi->root);

	// owner data
	StatusMenuExecParam *smep = nullptr;
	if (pmi->pszService) {
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->custom = TRUE;
		smep->svc = mir_strdup(pmi->pszService);
		smep->szProto = mir_strdup(pszProto);
	}

	TMO_IntMenuItem *pimi = Menu_AddItem(hStatusMenuObject, pmi, smep);
	if (pimi == nullptr)
		return nullptr;
	
	if (smep)
		smep->hMenuItem = pimi;

	char buf[MAX_PATH + 64];
	char *p = (pRoot) ? mir_u2a(pRoot->mi.name.w) : nullptr;
	mir_snprintf(buf, "%s/%s", (p) ? p : "", pmi->pszService ? pmi->pszService : "");
	mir_free(p);

	Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);
	return pimi;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HMENU) Menu_GetStatusMenu()
{
	RecursiveDeleteMenu(hStatusMenu);

	Menu_Build(hStatusMenu, hStatusMenuObject);
	return hStatusMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct MenuItemData
{
	HMENU OwnerMenu;
	int position;
};

static BOOL FindMenuHandleByGlobalID(HMENU hMenu, TMO_IntMenuItem *id, MenuItemData* itdat)
{
	if (!itdat)
		return FALSE;

	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for (int i = GetMenuItemCount(hMenu) - 1; i >= 0; i--) {
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.fType == MFT_SEPARATOR)
			continue;

		BOOL inSub = FALSE;
		if (mii.hSubMenu)
			inSub = FindMenuHandleByGlobalID(mii.hSubMenu, id, itdat);
		if (inSub)
			return inSub;

		TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
		if (pimi != nullptr) {
			if (pimi == id) {
				itdat->OwnerMenu = hMenu;
				itdat->position = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

static INT_PTR StatusMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (!pcpp)
		return TRUE;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(pcpp->MenuItemHandle);
	if (!pimi)
		return TRUE;

	StatusMenuExecParam *smep = (StatusMenuExecParam*)pcpp->MenuItemOwnerData;
	if (smep && !smep->status && smep->custom) {
		if (wildcmp(smep->svc, "*XStatus*")) {
			int XStatus;
			CUSTOM_STATUS cs = { sizeof(cs) };
			cs.flags = CSSF_MASK_STATUS;
			cs.status = &XStatus;
			if (CallProtoServiceInt(0, smep->szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) != 0)
				XStatus = 0;

			char buf[255];
			mir_snprintf(buf, "*XStatus%d", XStatus);

			bool check = wildcmp(smep->svc, buf) != 0;
			bool reset = wildcmp(smep->svc, "*XStatus0") != 0;

			if (check)
				pimi->mi.flags |= CMIF_CHECKED;
			else
				pimi->mi.flags &= ~CMIF_CHECKED;

			if (reset || check) {
				TMO_IntMenuItem *timiParent = MO_GetIntMenuItem(pimi->mi.root);
				if (timiParent) {
					LPTSTR ptszName = TranslateW_LP(pimi->mi.hIcolibItem ? pimi->mi.name.w : LPGENW("Custom status"), pimi->mi.pPlugin);

					timiParent = MO_GetIntMenuItem(pimi->mi.root);

					MenuItemData it = {};
					if (FindMenuHandleByGlobalID(hStatusMenu, timiParent, &it)) {
						wchar_t d[100];
						GetMenuString(it.OwnerMenu, it.position, d, _countof(d), MF_BYPOSITION);

						MENUITEMINFO mii = {};
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_STRING | MIIM_STATE;
						if (pimi->iconId != -1) {
							mii.fMask |= MIIM_BITMAP;
							if (IsWinVerVistaPlus() && IsThemeActive()) {
								if (pimi->hBmp == nullptr)
									pimi->hBmp = ConvertIconToBitmap(pimi->parent->m_hMenuIcons, pimi->iconId);
								mii.hbmpItem = pimi->hBmp;
							}
							else mii.hbmpItem = HBMMENU_CALLBACK;
						}

						mii.fState |= (check && !reset ? MFS_CHECKED : MFS_UNCHECKED);
						mii.dwTypeData = ptszName;
						SetMenuItemInfo(it.OwnerMenu, it.position, TRUE, &mii);
					}

					Menu_ModifyItem(pimi->mi.root, ptszName);

					timiParent->iconId = pimi->iconId;
					if (timiParent->hBmp) DeleteObject(timiParent->hBmp);
					timiParent->hBmp = nullptr;
				}
			}
		}
	}
	else if (smep && smep->status && !smep->custom) {
		int curProtoStatus = (smep->szProto) ? Proto_GetStatus(smep->szProto) : Proto_GetAverageStatus();
		if (smep->status == curProtoStatus)
			pimi->mi.flags |= CMIF_CHECKED;
		else
			pimi->mi.flags &= ~CMIF_CHECKED;
	}
	else if ((!smep || smep->szProto) && pimi->mi.name.a) {
		char* prot;
		if (smep)
			prot = smep->szProto;
		else {
			char *prn = mir_u2a(pimi->mi.name.w);
			prot = NEWSTR_ALLOCA(prn);
			if (prn) mir_free(prn);
		}
		PROTOACCOUNT *pa = Proto_GetAccount(prot);
		if (pa == nullptr)
			return TRUE;

		if (pa->iRealStatus >= ID_STATUS_OFFLINE && pa->iRealStatus < ID_STATUS_IDLE)
			pimi->mi.hIcolibItem = Skin_LoadProtoIcon(prot, pa->iRealStatus);
		else
			pimi->mi.hIcolibItem = Skin_LoadProtoIcon(prot, ID_STATUS_ONLINE);

		if (pimi->mi.hIcolibItem) {
			Menu_ModifyItem(pimi, nullptr, pimi->mi.hIcolibItem);
			IcoLib_ReleaseIcon((HICON)pimi->mi.hIcolibItem);
		}
	}

	return TRUE;
}

static INT_PTR StatusMenuExecService(WPARAM wParam, LPARAM)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)wParam;
	if (smep == nullptr)
		return 0;

	if (smep->custom) {
		if (smep->svc && *smep->svc) {
			if (smep->szProto && *smep->svc == '/')
				ProtoCallService(smep->szProto, smep->svc, 0, (LPARAM)smep->hMenuItem);
			else
				CallService(smep->svc, 0, (LPARAM)smep->hMenuItem);
		}
		return 0;
	}

	if (smep->status == 0 && smep->pimi != nullptr && smep->szProto != nullptr) {
		char *prot = smep->szProto;
		char szHumanName[64] = { 0 };
		PROTOACCOUNT *acc = Proto_GetAccount(smep->szProto);
		acc->bIsLocked = !acc->bIsLocked;
		db_set_b(0, prot, "LockMainStatus", acc->bIsLocked);

		CallProtoServiceInt(0, smep->szProto, PS_GETNAME, _countof(szHumanName), (LPARAM)szHumanName);

		TMO_IntMenuItem *pimi = MO_GetIntMenuItem(smep->pimi);
		if (pimi == nullptr)
			return 0;

		TMO_IntMenuItem *root = (TMO_IntMenuItem*)pimi->mi.root;
		wchar_t buf[256], *ptszName;
		if (acc->bIsLocked) {
			pimi->mi.flags |= CMIF_CHECKED;
			mir_snwprintf(buf, TranslateT("%s (locked)"), acc->tszAccountName);
			ptszName = buf;
		}
		else {
			ptszName = acc->tszAccountName;
			pimi->mi.flags &= ~CMIF_CHECKED;
		}
		replaceStrW(pimi->mi.name.w, ptszName);
		replaceStrW(root->mi.name.w, ptszName);

		if (g_clistApi.hwndStatus)
			InvalidateRect(g_clistApi.hwndStatus, nullptr, TRUE);
		return 0;
	}
	
	if (smep->szProto != nullptr) {
		Proto_SetStatus(smep->szProto, smep->status);
		NotifyEventHooks(hStatusModeChangeEvent, smep->status, (LPARAM)smep->szProto);
		return 0;
	}

	int MenusProtoCount = 0;

	for (auto &pa : g_arAccounts)
		if (pa->IsVisible())
			MenusProtoCount++;

	g_clistApi.currentDesiredStatusMode = smep->status;

	for (auto &pa : g_arAccounts) {
		if (!pa->IsEnabled())
			continue;
		if (MenusProtoCount > 1 && pa->IsLocked())
			continue;

		Proto_SetStatus(pa->szModuleName, g_clistApi.currentDesiredStatusMode);
	}
	NotifyEventHooks(hStatusModeChangeEvent, g_clistApi.currentDesiredStatusMode, 0);
	db_set_w(0, "CList", "Status", (uint16_t)g_clistApi.currentDesiredStatusMode);
	return 1;
}

static INT_PTR FreeOwnerDataStatusMenu(WPARAM, LPARAM lParam)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)lParam;
	if (smep != nullptr) {
		mir_free(smep->szProto);
		mir_free(smep->svc);
		mir_free(smep);
	}

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Global menu functions

MIR_APP_DLL(BOOL) Clist_MenuProcessCommand(int menu_id, int flags, MCONTACT hContact)
{
	if (flags & MPCF_MAINMENU) {
		if (menu_id >= ID_STATUS_OFFLINE && menu_id <= ID_STATUS_MAX) {
			int pos = statustopos(menu_id);
			if (pos != -1)
				return Menu_ProcessCommand(g_statuses[pos].hStatusMenu, hContact);
		}
	}

	if (!(menu_id >= CLISTMENUIDMIN && menu_id <= CLISTMENUIDMAX))
		return false; // DO NOT process ids outside from clist menu id range		v0.7.0.27+

	// process old menu sys
	if (flags & MPCF_CONTACTMENU)
		return MO_ProcessCommandBySubMenuIdent(hContactMenuObject, menu_id, hContact);

	// unknown old menu
	return Menu_ProcessCommandById(menu_id, hContact);
}

MIR_APP_DLL(BOOL) Clist_MenuProcessHotkey(unsigned vKey)
{
	prochotkey = true;

	bool res =
		Menu_ProcessHotKey(hStatusMenuObject, vKey) ||
		Menu_ProcessHotKey(hMainMenuObject, vKey);

	prochotkey = false;

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Other menu functions

static BOOL FindMenuHanleByGlobalID(HMENU hMenu, TMO_IntMenuItem *id, MenuItemData* itdat)
{
	if (!itdat)
		return FALSE;

	BOOL inSub = FALSE;

	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for (int i = GetMenuItemCount(hMenu) - 1; i >= 0; i--) {
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.fType == MFT_SEPARATOR)
			continue;

		if (mii.hSubMenu)
			inSub = FindMenuHanleByGlobalID(mii.hSubMenu, id, itdat);
		if (inSub)
			return inSub;

		TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
		if (pimi != nullptr) {
			if (pimi == id) {
				itdat->OwnerMenu = hMenu;
				itdat->position = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

MIR_APP_DLL(void) Clist_SetStatusMode(int iStatus)
{
	prochotkey = true;
	Clist_MenuProcessCommand(iStatus, MPCF_MAINMENU, 0);
	prochotkey = false;
}

MIR_APP_DLL(bool) Clist_GetProtocolVisibility(const char *szModuleName)
{
	if (szModuleName == nullptr)
		return false;

	PROTOACCOUNT *pa = Proto_GetAccount(szModuleName);
	return (pa) ? pa->IsVisible() : false;
}

MIR_APP_DLL(int) Clist_GetAccountIndex(int Pos)
{
	for (auto &it : g_arAccounts)
		if (it->iOrder == Pos)
			return g_arAccounts.indexOf(&it);

	return -1;
}

void RebuildMenuOrder(void)
{
	// clear statusmenu
	RecursiveDeleteMenu(hStatusMenu);

	// status menu
	if (hStatusMenuObject != 0)
		Menu_RemoveObject(hStatusMenuObject);

	hStatusMenuObject = Menu_AddObject("StatusMenu", LPGEN("Status menu"), "StatusMenuCheckService", "StatusMenuExecService");
	Menu_ConfigureObject(hStatusMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataStatusMenu");

	g_menuProtos.destroy();

	for (int s = 0; s < g_arAccounts.getCount(); s++) {
		int i = Clist_GetAccountIndex(s);
		if (i == -1)
			continue;

		PROTOACCOUNT *pa = g_arAccounts[i];
		if (!pa->IsVisible())
			continue;

		uint32_t flags = pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0);
		HICON ic;
		wchar_t tbuf[256];
		int pos = 0;

		// adding root
		CMenuItem mi(&g_plugin);
		mi.flags = CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
		mi.position = pos++;
		mi.hIcon = ic = (HICON)CallProtoServiceInt(0, pa->szModuleName, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);

		if (pa->IsLocked()) {
			mir_snwprintf(tbuf, TranslateT("%s (locked)"), pa->tszAccountName);
			mi.name.w = tbuf;
		}
		else mi.name.w = pa->tszAccountName;

		// owner data
		StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->szProto = mir_strdup(pa->szModuleName);
		TMO_IntMenuItem *rootmenu = Menu_AddItem(hStatusMenuObject, &mi, smep);

		memset(&mi, 0, sizeof(mi));
		mi.flags = CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
		mi.root = rootmenu;
		mi.position = pos++;
		mi.hIcon = ic;

		// owner data
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->szProto = mir_strdup(pa->szModuleName);

		if (pa->IsLocked())
			mi.flags |= CMIF_CHECKED;

		if ((mi.flags & CMIF_CHECKED)) {
			mir_snwprintf(tbuf, TranslateT("%s (locked)"), pa->tszAccountName);
			mi.name.w = tbuf;
		}
		else mi.name.w = pa->tszAccountName;

		TMO_IntMenuItem *pimi = Menu_AddItem(hStatusMenuObject, &mi, smep);
		smep->pimi = pimi;
		Menu_ModifyItem(pimi, mi.name.w, mi.hIcon, mi.flags);

		MenuProto *pMenu = new MenuProto();
		pMenu->hIcon = nullptr;
		pMenu->pMenu = rootmenu;
		pMenu->szProto = mir_strdup(pa->szModuleName);
		g_menuProtos.insert(pMenu);

		char buf[256];
		mir_snprintf(buf, "RootProtocolIcon_%s", pa->szModuleName);
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);

		DestroyIcon(ic);
		pos += 500000;

		for (int j = 0; j < _countof(g_statuses); j++) {
			if (!(flags & g_statuses[j].Pf2flag))
				continue;

			// adding
			memset(&mi, 0, sizeof(mi));
			mi.flags = CMIF_UNICODE;
			if (g_statuses[j].iStatus == ID_STATUS_OFFLINE)
				mi.flags |= CMIF_CHECKED;
			mi.root = rootmenu;
			mi.position = pos++;
			mi.name.w = Clist_GetStatusModeDescription(g_statuses[j].iStatus, GSMDF_UNTRANSLATED);
			mi.hIcon = Skin_LoadProtoIcon(pa->szModuleName, g_statuses[j].iStatus);

			// owner data
			smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->custom = FALSE;
			smep->status = g_statuses[j].iStatus;
			smep->pimi = (HGENMENU)i;
			smep->szProto = mir_strdup(pa->szModuleName);

			pa->protoindex = i;
			pa->menuhandle[j] = Menu_AddItem(hStatusMenuObject, &mi, smep);

			mir_snprintf(buf, "ProtocolIcon_%s_%s", pa->szModuleName, mi.name.a);
			Menu_ConfigureItem(pa->menuhandle[j], MCI_OPT_UNIQUENAME, buf);

			IcoLib_ReleaseIcon(mi.hIcon);
		}
	}

	NotifyEventHooks(hPreBuildStatusMenuEvent, 0, 0);
	int pos = 200000;

	// add to root menu
	for (auto &it : g_statuses) {
		for (auto &pa : g_arAccounts) {
			if (!pa->IsVisible())
				continue;

			uint32_t flags = pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0);
			if (!(flags & it.Pf2flag))
				continue;

			CMenuItem mi(&g_plugin);
			mi.flags = CMIF_UNICODE;
			if (it.iStatus == ID_STATUS_OFFLINE)
				mi.flags |= CMIF_CHECKED;

			mi.hIcon = Skin_LoadIcon(it.iSkinIcon);
			mi.position = pos++;

			// owner data
			StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->status = it.iStatus;
			{
				wchar_t buf[256], hotkeyName[100];
				uint16_t hotKey = GetHotkeyValue(it.iHotKey);
				HotkeyToName(hotkeyName, _countof(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
				mir_snwprintf(buf, L"%s\t%s", Clist_GetStatusModeDescription(it.iStatus, 0), hotkeyName);
				mi.name.w = buf;
				it.hStatusMenu = Menu_AddItem(hStatusMenuObject, &mi, smep);
				
				it.hStatusMenu->hotKey = hotKey;
			}

			char buf[256];
			mir_snprintf(buf, "Root2ProtocolIcon_%s_%s", pa->szModuleName, mi.name.a);
			Menu_ConfigureItem(it.hStatusMenu, MCI_OPT_UNIQUENAME, buf);

			IcoLib_ReleaseIcon(mi.hIcon);
			break;
		}
	}

	Menu_GetStatusMenu();
}

/////////////////////////////////////////////////////////////////////////////////////////

void BuildProtoMenus()
{
	for (auto &pa : g_arAccounts) {
		if (!pa->IsVisible())
			continue;

		if (pa->ppro)
			pa->ppro->OnBuildProtoMenu();
	}
}

void RebuildProtoMenus()
{
	RebuildMenuOrder();
	BuildProtoMenus();
}

MIR_APP_DLL(void) Menu_ReloadProtoMenus(void)
{
	RebuildMenuOrder();
	if (db_get_b(0, "CList", "MoveProtoMenus", true))
		BuildProtoMenus();
	g_clistApi.pfnCluiProtocolStatusChanged(0, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttRebuildHotkeys(WPARAM, LPARAM)
{
	for (auto &it : g_statuses) {
		if (it.hStatusMenu == nullptr)
			continue;

		wchar_t buf[256], hotkeyName[100];
		uint16_t hotKey = GetHotkeyValue(it.iHotKey);
		HotkeyToName(hotkeyName, _countof(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
		mir_snwprintf(buf, L"%s\t%s", Clist_GetStatusModeDescription(it.iStatus, 0), hotkeyName);
		Menu_ModifyItem(it.hStatusMenu, buf);

		it.hStatusMenu->hotKey = MAKELONG(HIBYTE(hotKey), LOBYTE(hotKey));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int MenuProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS) return 0;
	if (ack->result != ACKRESULT_SUCCESS) return 0;
	if (Clist_GetProtocolVisibility(ack->szModule) == 0) return 0;

	int overallStatus = Proto_GetAverageStatus();
	if (overallStatus >= ID_STATUS_OFFLINE) {
		int pos = statustopos(currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		// reset all current possible checked statuses
		for (auto &it : g_statuses)
			Menu_ModifyItem(it.hStatusMenu, nullptr, INVALID_HANDLE_VALUE, 0);

		currentStatusMenuItem = overallStatus;
		pos = statustopos(currentStatusMenuItem);
		if (pos >= 0 && pos < _countof(g_statuses))
			Menu_SetChecked(g_statuses[pos].hStatusMenu, true);
	}
	else {
		int pos = statustopos(currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		if (pos >= 0 && pos < _countof(g_statuses))
			Menu_ModifyItem(g_statuses[pos].hStatusMenu, nullptr, INVALID_HANDLE_VALUE, 0);

		currentStatusMenuItem = 0;
	}

	for (auto &pa : g_arAccounts) {
		if (!mir_strcmp(pa->szModuleName, ack->szModule)) {
			int iOldStatus = (INT_PTR)ack->hProcess;
			if ((iOldStatus >= ID_STATUS_OFFLINE || iOldStatus == 0) && iOldStatus < ID_STATUS_OFFLINE + _countof(g_statuses)) {
				int pos = statustopos(iOldStatus);
				if (pos == -1)
					pos = 0;
				for (pos = 0; pos < _countof(g_statuses); pos++)
					Menu_ModifyItem(pa->menuhandle[pos], nullptr, INVALID_HANDLE_VALUE, 0);
			}

			if (ack->lParam >= ID_STATUS_OFFLINE && ack->lParam < ID_STATUS_OFFLINE + _countof(g_statuses)) {
				int pos = statustopos((int)ack->lParam);
				if (pos >= 0 && pos < _countof(g_statuses))
					Menu_SetChecked(pa->menuhandle[pos], true);
			}
			break;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static MenuProto* FindProtocolMenu(const char *proto)
{
	for (auto &it : g_menuProtos)
		if (it->pMenu && !mir_strcmpi(it->szProto, proto))
			return it;

	if (g_menuProtos.getCount() == 1)
		if (!mir_strcmpi(g_menuProtos[0].szProto, proto))
			return &g_menuProtos[0];

	return nullptr;
}

MIR_APP_DLL(HGENMENU) Menu_GetProtocolMenu(const char *proto)
{
	MenuProto *mp = FindProtocolMenu(proto);
	return (mp) ? mp->pMenu : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR HotkeySetStatus(WPARAM, LPARAM lParam)
{
	Clist_SetStatusMode(lParam);
	return 0;
}

static INT_PTR ShowHide(WPARAM, LPARAM)
{
	g_clistApi.pfnShowHide();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL MENU

MIR_APP_DLL(HGENMENU) Menu_AddProtoMenuItem(TMO_MenuItem *mi, const char *pszProto)
{
	if (mi == nullptr)
		return nullptr;

	if (db_get_b(0, "CList", "MoveProtoMenus", TRUE))
		return Menu_AddStatusMenuItem(mi, pszProto);

	char szService[100];
	if (pszProto && mi->pszService && *mi->pszService == '/') {
		strncpy_s(szService, pszProto, _TRUNCATE);
		strncat_s(szService, mi->pszService, _TRUNCATE);
		mi->pszService = szService;
	}
	return Menu_AddMainMenuItem(mi);
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitCustomMenus(void)
{
	CreateServiceFunction("MainMenuExecService", MainMenuExecService);

	CreateServiceFunction("ContactMenuExecService", ContactMenuExecService);
	CreateServiceFunction("ContactMenuCheckService", ContactMenuCheckService);

	CreateServiceFunction("StatusMenuExecService", StatusMenuExecService);
	CreateServiceFunction("StatusMenuCheckService", StatusMenuCheckService);

	// free services
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataMainMenu", FreeOwnerDataMainMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataContactMenu", FreeOwnerDataContactMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataStatusMenu", FreeOwnerDataStatusMenu);

	hPreBuildContactMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDCONTACTMENU);
	hPreBuildMainMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDMAINMENU);
	hPreBuildStatusMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDSTATUSMENU);
	hStatusModeChangeEvent = CreateHookableEvent(ME_CLIST_STATUSMODECHANGE);

	HookEvent(ME_PROTO_ACK, MenuProtoAck);

	hMainMenu = CreatePopupMenu();
	hStatusMenu = CreatePopupMenu();

	// new menu sys
	InitGenMenu();

	// main menu
	hMainMenuObject = Menu_AddObject("MainMenu", LPGEN("Main menu"), nullptr, "MainMenuExecService");
	Menu_ConfigureObject(hMainMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hMainMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataMainMenu");

	// contact menu
	hContactMenuObject = Menu_AddObject("ContactMenu", LPGEN("Contact menu"), "ContactMenuCheckService", "ContactMenuExecService");
	Menu_ConfigureObject(hContactMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hContactMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataContactMenu");

	// other menus
	InitGroupMenus();
	InitFramesMenus();
	InitProtoMenus();
	InitTrayMenus();

	// initialize hotkeys
	CreateServiceFunction(MS_CLIST_HKSTATUS, HotkeySetStatus);
	CreateServiceFunction(MS_CLIST_SHOWHIDE, ShowHide);

	HOTKEYDESC hkd = {};
	hkd.szSection.w = L"Status";
	hkd.dwFlags = HKD_UNICODE;
	for (int i = 0; i < _countof(g_statuses); i++) {
		char szName[30];
		mir_snprintf(szName, "StatusHotKey_%d", i);
		hkd.pszName = szName;
		hkd.lParam = g_statuses[i].iStatus;
		hkd.szDescription.w = Clist_GetStatusModeDescription(hkd.lParam, GSMDF_UNTRANSLATED);
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, '0' + i) | HKF_MIRANDA_LOCAL;
		hkd.pszService = MS_CLIST_HKSTATUS;
		g_statuses[i].iHotKey = g_plugin.addHotkey(&hkd);
	}

	HookEvent(ME_HOTKEYS_CHANGED, sttRebuildHotkeys);
	HookEvent(ME_LANGPACK_CHANGED, sttRebuildHotkeys);

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x9d6d4bb1, 0x5207, 0x481a, 0x80, 0x47, 0x67, 0x58, 0x8e, 0xb6, 0x8f, 0xff);
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.a = LPGEN("System history");
	mi.position = 500060000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	Menu_AddMainMenuItem(&mi);

	// add exit command to menu
	SET_UID(mi, 0x707c8962, 0xc33f, 0x4893, 0x8e, 0x36, 0x30, 0xb1, 0x7c, 0xd8, 0x61, 0x40);
	mi.position = 0x7fffffff;
	mi.pszService = "CloseAction";
	mi.name.a = LPGEN("E&xit");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	Menu_AddMainMenuItem(&mi);

	currentStatusMenuItem = ID_STATUS_OFFLINE;
	g_clistApi.currentDesiredStatusMode = ID_STATUS_OFFLINE;
}

void UninitCustomMenus(void)
{
	Menu_RemoveObject(hMainMenuObject);
	Menu_RemoveObject(hStatusMenuObject);

	g_menuProtos.destroy();

	DestroyMenu(hMainMenu);
	DestroyMenu(hStatusMenu);
}
