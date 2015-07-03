/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "clc.h"
#include "genmenu.h"

void InitGroupMenus(void);
void InitFramesMenus(void);
void InitTrayMenus(void);

#define MS_CLIST_HKSTATUS "Clist/HK/SetStatus"

#define FIRSTCUSTOMMENUITEMID	30000
#define MENU_CUSTOMITEMMAIN		0x80000000
//#define MENU_CUSTOMITEMCONTEXT	0x40000000
//#define MENU_CUSTOMITEMFRAME	0x20000000

typedef struct  {
	WORD id;
	int iconId;
	TMO_MenuItem mi;
}
	CListIntMenuItem, *lpCListIntMenuItem;

// new menu sys
int hMainMenuObject = 0, hContactMenuObject = 0, hStatusMenuObject = 0;
int UnloadMoveToGroup(void);

int statustopos(int status);
void Proto_SetStatus(const char *szProto, unsigned status);

bool prochotkey;

HANDLE hPreBuildMainMenuEvent, hStatusModeChangeEvent, hPreBuildContactMenuEvent;

HMENU hMainMenu, hStatusMenu;
const int statusModeList[MAX_STATUS_COUNT] =
{
	ID_STATUS_OFFLINE, ID_STATUS_ONLINE, ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED,
	ID_STATUS_DND, ID_STATUS_FREECHAT, ID_STATUS_INVISIBLE, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH
};

const int skinIconStatusList[MAX_STATUS_COUNT] =
{
	SKINICON_STATUS_OFFLINE, SKINICON_STATUS_ONLINE, SKINICON_STATUS_AWAY, SKINICON_STATUS_NA, SKINICON_STATUS_OCCUPIED,
	SKINICON_STATUS_DND, SKINICON_STATUS_FREE4CHAT, SKINICON_STATUS_INVISIBLE, SKINICON_STATUS_ONTHEPHONE, SKINICON_STATUS_OUTTOLUNCH
};

static const int statusModePf2List[MAX_STATUS_COUNT] =
{
	0xFFFFFFFF, PF2_ONLINE, PF2_SHORTAWAY, PF2_LONGAWAY, PF2_LIGHTDND,
	PF2_HEAVYDND, PF2_FREECHAT, PF2_INVISIBLE, PF2_ONTHEPHONE, PF2_OUTTOLUNCH
};

static INT_PTR statusHotkeys[MAX_STATUS_COUNT];

TMO_IntMenuItem **hStatusMainMenuHandles;
int  hStatusMainMenuHandlesCnt;

struct tStatusMenuHandles
{
	int protoindex;
	int protostatus[MAX_STATUS_COUNT];
	TMO_IntMenuItem *menuhandle[MAX_STATUS_COUNT];
};

tStatusMenuHandles *hStatusMenuHandles;
int hStatusMenuHandlesCnt;

struct BuildContactParam
{
	char *szProto;
	int isOnList;
	int isOnline;
};

struct MenuItemData
{
	HMENU OwnerMenu;
	int position;
};

/////////////////////////////////////////////////////////////////////////////////////////
// service functions

void FreeMenuProtos(void)
{
	if (cli.menuProtos) {
		for (int i = 0; i < cli.menuProtoCount; i++)
			mir_free(cli.menuProtos[i].szProto);
		mir_free(cli.menuProtos);
		cli.menuProtos = NULL;
	}
	cli.menuProtoCount = 0;
}

//////////////////////////////////////////////////////////////////////////

int fnGetAverageMode(int *pNetProtoCount)
{
	int netProtoCount = 0, averageMode = 0;

	for (int i = 0; i < accounts.getCount(); i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (cli.pfnGetProtocolVisibility(pa->szModuleName) == 0 || Proto_IsAccountLocked(pa))
			continue;

		netProtoCount++;

		if (averageMode == 0)
			averageMode = CallProtoServiceInt(NULL, pa->szModuleName, PS_GETSTATUS, 0, 0);
		else if (averageMode > 0 && averageMode != CallProtoServiceInt(NULL, pa->szModuleName, PS_GETSTATUS, 0, 0)) {
			averageMode = -1;
			if (pNetProtoCount == NULL)
				break;
		}
	}

	if (pNetProtoCount) *pNetProtoCount = netProtoCount;
	return averageMode;
}

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
	TCHAR *szMenuName;
	TMO_IntMenuItem *pimi;
};

MIR_APP_DLL(HMENU) Menu_BuildMainMenu(void)
{
	NotifyEventHooks(hPreBuildMainMenuEvent, 0, 0);

	Menu_Build(hMainMenu, hMainMenuObject);
	DrawMenuBar(cli.hwndContactList);
	return hMainMenu;
}

MIR_APP_DLL(HMENU) Menu_GetMainMenu(void)
{
	RecursiveDeleteMenu(hMainMenu);
	return Menu_BuildMainMenu();
}

MIR_APP_DLL(HGENMENU) Menu_AddMainMenuItem(TMO_MenuItem *pmi)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)mir_alloc(sizeof(MainMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	mmep->szMenuName = pmi->name.t;

	TMO_IntMenuItem *pimi = Menu_AddItem(hMainMenuObject, pmi, mmep);
	if (pimi == NULL)
		return NULL;

	mmep->pimi = pimi;

	const char* name;
	bool needFree = false;

	if (pmi->pszService)
		name = pmi->pszService;
	else if (pmi->flags & CMIF_UNICODE) {
		name = mir_t2a(pmi->name.t);
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
INT_PTR MainMenuExecService(WPARAM wParam, LPARAM lParam)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)wParam;
	if (mmep != NULL) {
		CallService(mmep->szServiceName, mmep->pimi->execParam, lParam);
	}
	return 1;
}

INT_PTR FreeOwnerDataMainMenu(WPARAM, LPARAM lParam)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)lParam;
	if (mmep != NULL) {
		FreeAndNil((void**)&mmep->szServiceName);
		FreeAndNil((void**)&mmep);
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
	if (pszProto != NULL)
		cmep->pszContactOwner = mir_strdup(pszProto);

	// may be need to change how UniqueName is formed?
	TMO_IntMenuItem *pimi = Menu_AddItem(hContactMenuObject, pmi, cmep);
	if (pimi == NULL)
		return NULL;
	
	cmep->pimi = pimi;

	if (pszProto == NULL)
		pszProto = "";

	char buf[256];
	if (pmi->pszService) {
		mir_snprintf(buf, "%s/%s", pszProto, (pmi->pszService) ? pmi->pszService : "");
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);
	}
	else if (pmi->name.t) {
		if (pmi->flags & CMIF_UNICODE)
			mir_snprintf(buf, "%s/NoService/%S", pszProto, pmi->name.t);
		else
			mir_snprintf(buf, "%s/NoService/%s", pszProto, pmi->name.a);
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);
	}
	return pimi;
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(HMENU) Menu_BuildContactMenu(MCONTACT hContact)
{
	NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

	char *szProto = GetContactProto(hContact);

	BuildContactParam bcp;
	bcp.szProto = szProto;
	bcp.isOnList = (db_get_b(hContact, "CList", "NotOnList", 0) == 0);
	bcp.isOnline = (szProto != NULL && ID_STATUS_OFFLINE != db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE));

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hContactMenuObject, (WPARAM)&bcp);
	return hMenu;
}

// true - ok, false ignore
static INT_PTR ContactMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == NULL)
		return FALSE;

	BuildContactParam *bcp = (BuildContactParam*)pcpp->wParam;
	if (bcp == NULL)
		return FALSE;

	ContactMenuExecParam *cmep = (ContactMenuExecParam*)pcpp->MenuItemOwnerData;
	if (cmep == NULL) //this is root...build it
		return TRUE;

	if (cmep->pszContactOwner != NULL) {
		if (bcp->szProto == NULL) return FALSE;
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
	if (cmep != NULL) {
		FreeAndNil((void**)&cmep->szServiceName);
		FreeAndNil((void**)&cmep->pszContactOwner);
		FreeAndNil((void**)&cmep);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// STATUS MENU

struct StatusMenuExecParam
{
	char *szProto;  // This is unique protoname
	int protoindex;
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
	StatusMenuExecParam *smep = NULL;
	if (pmi->pszService) {
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->custom = TRUE;
		smep->svc = mir_strdup(pmi->pszService);
		smep->szProto = mir_strdup(pszProto);
	}

	TMO_IntMenuItem *pimi = Menu_AddItem(hStatusMenuObject, pmi, smep);
	if (pimi == NULL)
		return NULL;
	
	if (smep)
		smep->hMenuItem = pimi;

	char buf[MAX_PATH + 64];
	char *p = (pRoot) ? mir_t2a(pRoot->mi.name.t) : NULL;
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

BOOL FindMenuHandleByGlobalID(HMENU hMenu, TMO_IntMenuItem *id, MenuItemData* itdat)
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
		if (pimi != NULL) {
			if (pimi == id) {
				itdat->OwnerMenu = hMenu;
				itdat->position = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

INT_PTR StatusMenuCheckService(WPARAM wParam, LPARAM)
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
			if (CallProtoServiceInt(NULL, smep->szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) != 0)
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
					LPTSTR ptszName = TranslateTH(pimi->mi.hLangpack, pimi->mi.hIcolibItem ? pimi->mi.name.t : LPGENT("Custom status"));

					timiParent = MO_GetIntMenuItem(pimi->mi.root);

					MenuItemData it = { 0 };
					if (FindMenuHandleByGlobalID(hStatusMenu, timiParent, &it)) {
						TCHAR d[100];
						GetMenuString(it.OwnerMenu, it.position, d, _countof(d), MF_BYPOSITION);

						MENUITEMINFO mii = { 0 };
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_STRING | MIIM_STATE;
						if (pimi->iconId != -1) {
							mii.fMask |= MIIM_BITMAP;
							if (IsWinVerVistaPlus() && IsThemeActive()) {
								if (pimi->hBmp == NULL)
									pimi->hBmp = ConvertIconToBitmap(NULL, pimi->parent->m_hMenuIcons, pimi->iconId);
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
					timiParent->hBmp = NULL;
				}
			}
		}
	}
	else if (smep && smep->status && !smep->custom) {
		int curProtoStatus = (smep->szProto) ? CallProtoServiceInt(NULL, smep->szProto, PS_GETSTATUS, 0, 0) : cli.pfnGetAverageMode(NULL);
		if (smep->status == curProtoStatus)
			pimi->mi.flags |= CMIF_CHECKED;
		else
			pimi->mi.flags &= ~CMIF_CHECKED;
	}
	else if ((!smep || smep->szProto) && pimi->mi.name.a) {
		int curProtoStatus = 0;
		BOOL IconNeedDestroy = FALSE;
		char* prot;
		if (smep)
			prot = smep->szProto;
		else {
			char *prn = mir_u2a(pimi->mi.name.t);
			prot = NEWSTR_ALLOCA(prn);
			if (prn) mir_free(prn);
		}
		if (Proto_GetAccount(prot) == NULL)
			return TRUE;

		if ((curProtoStatus = CallProtoServiceInt(NULL, prot, PS_GETSTATUS, 0, 0)) == CALLSERVICE_NOTFOUND)
			curProtoStatus = 0;

		if (curProtoStatus >= ID_STATUS_OFFLINE && curProtoStatus < ID_STATUS_IDLE)
			pimi->mi.hIcolibItem = Skin_LoadProtoIcon(prot, curProtoStatus);
		else {
			pimi->mi.hIcolibItem = (HICON)CallProtoServiceInt(NULL, prot, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (pimi->mi.hIcolibItem == (HICON)CALLSERVICE_NOTFOUND)
				pimi->mi.hIcolibItem = NULL;
			else
				IconNeedDestroy = TRUE;
		}

		if (pimi->mi.hIcolibItem) {
			Menu_ModifyItem(pimi, NULL, pimi->mi.hIcolibItem);
			if (IconNeedDestroy) {
				DestroyIcon((HICON)pimi->mi.hIcolibItem);
				pimi->mi.hIcolibItem = NULL;
			}
			else IcoLib_ReleaseIcon((HICON)pimi->mi.hIcolibItem);
		}
	}

	return TRUE;
}

INT_PTR StatusMenuExecService(WPARAM wParam, LPARAM)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)wParam;
	if (smep == NULL)
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

	if (smep->status == 0 && smep->protoindex != 0 && smep->szProto != NULL) {
		char *prot = smep->szProto;
		char szHumanName[64] = { 0 };
		PROTOACCOUNT *acc = Proto_GetAccount(smep->szProto);
		bool bIsLocked = !Proto_IsAccountLocked(acc);
		db_set_b(NULL, prot, "LockMainStatus", bIsLocked);

		CallProtoServiceInt(NULL, smep->szProto, PS_GETNAME, _countof(szHumanName), (LPARAM)szHumanName);

		TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)smep->protoindex);
		if (pimi == NULL)
			return 0;

		TMO_IntMenuItem *root = (TMO_IntMenuItem*)pimi->mi.root;
		TCHAR buf[256], *ptszName;
		if (bIsLocked) {
			pimi->mi.flags |= CMIF_CHECKED;
			if (cli.bDisplayLocked) {
				mir_sntprintf(buf, TranslateT("%s (locked)"), acc->tszAccountName);
				ptszName = buf;
			}
			else ptszName = acc->tszAccountName;
		}
		else {
			ptszName = acc->tszAccountName;
			pimi->mi.flags &= ~CMIF_CHECKED;
		}
		replaceStrT(pimi->mi.name.t, ptszName);
		replaceStrT(root->mi.name.t, ptszName);

		if (cli.hwndStatus)
			InvalidateRect(cli.hwndStatus, NULL, TRUE);
		return 0;
	}
	
	if (smep->szProto != NULL) {
		Proto_SetStatus(smep->szProto, smep->status);
		NotifyEventHooks(hStatusModeChangeEvent, smep->status, (LPARAM)smep->szProto);
		return 0;
	}

	int MenusProtoCount = 0;

	for (int i = 0; i < accounts.getCount(); i++)
		if (cli.pfnGetProtocolVisibility(accounts[i]->szModuleName))
			MenusProtoCount++;

	cli.currentDesiredStatusMode = smep->status;

	for (int j = 0; j < accounts.getCount(); j++) {
		PROTOACCOUNT *pa = accounts[j];
		if (!Proto_IsAccountEnabled(pa))
			continue;
		if (MenusProtoCount > 1 && Proto_IsAccountLocked(pa))
			continue;

		Proto_SetStatus(pa->szModuleName, cli.currentDesiredStatusMode);
	}
	NotifyEventHooks(hStatusModeChangeEvent, cli.currentDesiredStatusMode, 0);
	db_set_w(NULL, "CList", "Status", (WORD)cli.currentDesiredStatusMode);
	return 1;
}

INT_PTR FreeOwnerDataStatusMenu(WPARAM, LPARAM lParam)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)lParam;
	if (smep != NULL) {
		mir_free(smep->szProto);
		mir_free(smep->svc);
		mir_free(smep);
	}

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Other menu functions

INT_PTR MenuProcessCommand(WPARAM wParam, LPARAM lParam)
{
	WORD cmd = LOWORD(wParam);

	if (HIWORD(wParam) & MPCF_MAINMENU) {
		int hst = LOWORD(wParam);
		if (hst >= ID_STATUS_OFFLINE && hst <= ID_STATUS_OUTTOLUNCH) {
			int pos = statustopos(hst);
			if (pos != -1 && hStatusMainMenuHandles != NULL)
				return Menu_ProcessCommand(hStatusMainMenuHandles[pos], lParam);
		}
	}

	if (!(cmd >= CLISTMENUIDMIN && cmd <= CLISTMENUIDMAX))
		return 0; // DO NOT process ids outside from clist menu id range		v0.7.0.27+

	//process old menu sys
	if (HIWORD(wParam) & MPCF_CONTACTMENU)
		return MO_ProcessCommandBySubMenuIdent((int)hContactMenuObject, LOWORD(wParam), lParam);

	//unknown old menu
	return Menu_ProcessCommandById(LOWORD(wParam), lParam);
}

BOOL FindMenuHanleByGlobalID(HMENU hMenu, TMO_IntMenuItem *id, MenuItemData* itdat)
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
		if (pimi != NULL) {
			if (pimi == id) {
				itdat->OwnerMenu = hMenu;
				itdat->position = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}

static INT_PTR MenuProcessHotkey(WPARAM vKey, LPARAM)
{
	prochotkey = true;

	bool res =
		Menu_ProcessHotKey(hStatusMenuObject, vKey) ||
		Menu_ProcessHotKey(hMainMenuObject, vKey);

	prochotkey = false;

	return res;
}

static int MenuIconsChanged(WPARAM, LPARAM)
{
	//just rebuild menu
	RebuildMenuOrder();
	cli.pfnCluiProtocolStatusChanged(0, 0);
	return 0;
}

static INT_PTR SetStatusMode(WPARAM wParam, LPARAM)
{
	prochotkey = true;
	MenuProcessCommand(MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), 0);
	prochotkey = false;
	return 0;
}

int fnGetProtocolVisibility(const char *accName)
{
	if (accName) {
		PROTOACCOUNT *pa = Proto_GetAccount(accName);
		if (pa && pa->bIsVisible && Proto_IsAccountEnabled(pa) && pa->ppro) {
			PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(pa->szProtoName);
			if (pd == NULL || pd->type != PROTOTYPE_PROTOCOL)
				return FALSE;

			return (pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0));
		}
	}

	return FALSE;
}

int fnGetProtoIndexByPos(PROTOCOLDESCRIPTOR **proto, int protoCnt, int Pos)
{
	char buf[10];
	_itoa(Pos, buf, 10);

	DBVARIANT dbv;
	if (!db_get_s(NULL, "Protocols", buf, &dbv)) {
		for (int p = 0; p < protoCnt; p++) {
			if (mir_strcmp(proto[p]->szName, dbv.pszVal) == 0) {
				db_free(&dbv);
				return p;
			}
		}

		db_free(&dbv);
	}

	return -1;
}

int fnGetAccountIndexByPos(int Pos)
{
	for (int i = 0; i < accounts.getCount(); i++)
		if (accounts[i]->iOrder == Pos)
			return i;

	return -1;
}

void RebuildMenuOrder(void)
{
	BYTE bHideStatusMenu = db_get_b(NULL, "CLUI", "DontHideStatusMenu", 0); // cool perversion, though

	//clear statusmenu
	RecursiveDeleteMenu(hStatusMenu);

	//status menu
	if (hStatusMenuObject != 0) {
		Menu_RemoveObject(hStatusMenuObject);
		mir_free(hStatusMainMenuHandles);
		mir_free(hStatusMenuHandles);
	}

	hStatusMenuObject = Menu_AddObject("StatusMenu", LPGEN("Status menu"), "StatusMenuCheckService", "StatusMenuExecService");
	Menu_ConfigureObject(hStatusMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataStatusMenu");

	hStatusMainMenuHandles = (TMO_IntMenuItem**)mir_calloc(_countof(statusModeList) * sizeof(TMO_IntMenuItem*));
	hStatusMainMenuHandlesCnt = _countof(statusModeList);

	hStatusMenuHandles = (tStatusMenuHandles*)mir_calloc(sizeof(tStatusMenuHandles)*accounts.getCount());
	hStatusMenuHandlesCnt = accounts.getCount();

	FreeMenuProtos();

	for (int s = 0; s < accounts.getCount(); s++) {
		int i = cli.pfnGetAccountIndexByPos(s);
		if (i == -1)
			continue;

		PROTOACCOUNT *pa = accounts[i];
		int pos = 0;
		if (!bHideStatusMenu && !cli.pfnGetProtocolVisibility(pa->szModuleName))
			continue;

		DWORD flags = pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0);
		HICON ic;
		TCHAR tbuf[256];

		// adding root
		CMenuItem mi;
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.position = pos++;
		mi.hIcon = ic = (HICON)CallProtoServiceInt(NULL, pa->szModuleName, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);

		if (Proto_IsAccountLocked(pa) && cli.bDisplayLocked) {
			mir_sntprintf(tbuf, _countof(tbuf), TranslateT("%s (locked)"), pa->tszAccountName);
			mi.name.t = tbuf;
		}
		else mi.name.t = pa->tszAccountName;

		// owner data
		StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->szProto = mir_strdup(pa->szModuleName);
		TMO_IntMenuItem *rootmenu = Menu_AddItem(hStatusMenuObject, &mi, smep);

		memset(&mi, 0, sizeof(mi));
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.root = rootmenu;
		mi.position = pos++;
		mi.hIcon = ic;

		// owner data
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->szProto = mir_strdup(pa->szModuleName);

		if (Proto_IsAccountLocked(pa))
			mi.flags |= CMIF_CHECKED;

		if ((mi.flags & CMIF_CHECKED) && cli.bDisplayLocked) {
			mir_sntprintf(tbuf, _countof(tbuf), TranslateT("%s (locked)"), pa->tszAccountName);
			mi.name.t = tbuf;
		}
		else mi.name.t = pa->tszAccountName;

		TMO_IntMenuItem *pimi = Menu_AddItem(hStatusMenuObject, &mi, smep);
		smep->protoindex = (int)pimi;
		Menu_ModifyItem(pimi, mi.name.t, mi.hIcon, mi.flags);

		cli.menuProtos = (MenuProto*)mir_realloc(cli.menuProtos, sizeof(MenuProto)*(cli.menuProtoCount + 1));
		memset(&(cli.menuProtos[cli.menuProtoCount]), 0, sizeof(MenuProto));
		cli.menuProtos[cli.menuProtoCount].pMenu = rootmenu;
		cli.menuProtos[cli.menuProtoCount].szProto = mir_strdup(pa->szModuleName);

		cli.menuProtoCount++;

		char buf[256];
		mir_snprintf(buf, "RootProtocolIcon_%s", pa->szModuleName);
		Menu_ConfigureItem(pimi, MCI_OPT_UNIQUENAME, buf);

		DestroyIcon(ic);
		pos += 500000;

		for (int j = 0; j < _countof(statusModeList); j++) {
			if (!(flags & statusModePf2List[j]))
				continue;

			// adding
			memset(&mi, 0, sizeof(mi));
			mi.flags = CMIF_TCHAR;
			if (statusModeList[j] == ID_STATUS_OFFLINE)
				mi.flags |= CMIF_CHECKED;
			mi.root = rootmenu;
			mi.position = pos++;
			mi.name.t = cli.pfnGetStatusModeDescription(statusModeList[j], GSMDF_UNTRANSLATED);
			mi.hIcon = Skin_LoadProtoIcon(pa->szModuleName, statusModeList[j]);

			// owner data
			StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->custom = FALSE;
			smep->status = statusModeList[j];
			smep->protoindex = i;
			smep->szProto = mir_strdup(pa->szModuleName);

			hStatusMenuHandles[i].protoindex = i;
			hStatusMenuHandles[i].protostatus[j] = statusModeList[j];
			hStatusMenuHandles[i].menuhandle[j] = Menu_AddItem(hStatusMenuObject, &mi, smep);

			char buf[256];
			mir_snprintf(buf, "ProtocolIcon_%s_%s", pa->szModuleName, mi.name.a);
			Menu_ConfigureItem(hStatusMenuHandles[i].menuhandle[j], MCI_OPT_UNIQUENAME, buf);

			IcoLib_ReleaseIcon(mi.hIcon);
		}
	}

	NotifyEventHooks(cli.hPreBuildStatusMenuEvent, 0, 0);
	int pos = 200000;

	// add to root menu
	for (int j = 0; j < _countof(statusModeList); j++) {
		for (int i = 0; i < accounts.getCount(); i++) {
			PROTOACCOUNT *pa = accounts[i];
			if (!bHideStatusMenu && !cli.pfnGetProtocolVisibility(pa->szModuleName))
				continue;

			DWORD flags = pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0);
			if (!(flags & statusModePf2List[j]))
				continue;

			CMenuItem mi;
			mi.flags = CMIF_TCHAR;
			if (statusModeList[j] == ID_STATUS_OFFLINE)
				mi.flags |= CMIF_CHECKED;

			mi.hIcon = Skin_LoadIcon(skinIconStatusList[j]);
			mi.position = pos++;

			// owner data
			StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->status = statusModeList[j];
			{
				TCHAR buf[256], hotkeyName[100];
				WORD hotKey = GetHotkeyValue(statusHotkeys[j]);
				HotkeyToName(hotkeyName, _countof(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
				mir_sntprintf(buf, _T("%s\t%s"), cli.pfnGetStatusModeDescription(statusModeList[j], 0), hotkeyName);
				mi.name.t = buf;
				hStatusMainMenuHandles[j] = Menu_AddItem(hStatusMenuObject, &mi, smep);
				
				hStatusMainMenuHandles[j]->hotKey = hotKey;
			}

			char buf[256];
			mir_snprintf(buf, "Root2ProtocolIcon_%s_%s", pa->szModuleName, mi.name.a);
			Menu_ConfigureItem(hStatusMainMenuHandles[j], MCI_OPT_UNIQUENAME, buf);

			IcoLib_ReleaseIcon(mi.hIcon);
			break;
		}
	}

	Menu_GetStatusMenu();
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttRebuildHotkeys(WPARAM, LPARAM)
{
	for (int j = 0; j < _countof(statusModeList); j++) {
		TCHAR buf[256], hotkeyName[100];
		WORD hotKey = GetHotkeyValue(statusHotkeys[j]);
		HotkeyToName(hotkeyName, _countof(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
		mir_sntprintf(buf, _T("%s\t%s"), cli.pfnGetStatusModeDescription(statusModeList[j], 0), hotkeyName);
		Menu_ModifyItem(hStatusMainMenuHandles[j], buf);

		hStatusMainMenuHandles[j]->hotKey = MAKELONG(HIBYTE(hotKey), LOBYTE(hotKey));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int statustopos(int status)
{
	for (int j = 0; j < _countof(statusModeList); j++)
		if (status == statusModeList[j])
			return j;

	return -1;
}

static int MenuProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS) return 0;
	if (ack->result != ACKRESULT_SUCCESS) return 0;
	if (hStatusMainMenuHandles == NULL) return 0;
	if (cli.pfnGetProtocolVisibility(ack->szModule) == 0) return 0;

	int overallStatus = cli.pfnGetAverageMode(NULL);

	if (overallStatus >= ID_STATUS_OFFLINE) {
		int pos = statustopos(cli.currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		// reset all current possible checked statuses
		for (int pos2 = 0; pos2 < hStatusMainMenuHandlesCnt; pos2++)
			if (pos2 >= 0 && pos2 < hStatusMainMenuHandlesCnt)
				Menu_ModifyItem(hStatusMainMenuHandles[pos2], NULL, INVALID_HANDLE_VALUE, 0);

		cli.currentStatusMenuItem = overallStatus;
		pos = statustopos(cli.currentStatusMenuItem);
		if (pos >= 0 && pos < hStatusMainMenuHandlesCnt)
			Menu_SetChecked(hStatusMainMenuHandles[pos], true);
	}
	else {
		int pos = statustopos(cli.currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		if (pos >= 0 && pos < hStatusMainMenuHandlesCnt)
			Menu_ModifyItem(hStatusMainMenuHandles[pos], NULL, INVALID_HANDLE_VALUE, 0);

		cli.currentStatusMenuItem = 0;
	}

	for (int i = 0; i < accounts.getCount(); i++) {
		if (!mir_strcmp(accounts[i]->szModuleName, ack->szModule)) {
			if (((int)ack->hProcess >= ID_STATUS_OFFLINE || (int)ack->hProcess == 0) && (int)ack->hProcess < ID_STATUS_OFFLINE + _countof(statusModeList)) {
				int pos = statustopos((int)ack->hProcess);
				if (pos == -1)
					pos = 0;
				for (pos = 0; pos < _countof(statusModeList); pos++)
					Menu_ModifyItem(hStatusMenuHandles[i].menuhandle[pos], NULL, INVALID_HANDLE_VALUE, 0);
			}

			if (ack->lParam >= ID_STATUS_OFFLINE && ack->lParam < ID_STATUS_OFFLINE + _countof(statusModeList)) {
				int pos = statustopos((int)ack->lParam);
				if (pos >= 0 && pos < _countof(statusModeList))
					Menu_SetChecked(hStatusMenuHandles[i].menuhandle[pos], true);
			}
			break;
		}
	}

	//BuildStatusMenu(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static MenuProto* FindProtocolMenu(const char *proto)
{
	for (int i = 0; i < cli.menuProtoCount; i++)
		if (cli.menuProtos[i].pMenu && !mir_strcmpi(cli.menuProtos[i].szProto, proto))
			return &cli.menuProtos[i];

	if (cli.menuProtoCount == 1)
		if (!mir_strcmpi(cli.menuProtos[0].szProto, proto))
			return &cli.menuProtos[0];

	return NULL;
}

HGENMENU fnGetProtocolMenu(const char* proto)
{
	MenuProto *mp = FindProtocolMenu(proto);
	return (mp) ? mp->pMenu : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR HotkeySetStatus(WPARAM, LPARAM lParam)
{
	return SetStatusMode(lParam, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL MENU

MIR_APP_DLL(HGENMENU) Menu_AddProtoMenuItem(TMO_MenuItem *mi, const char *pszProto)
{
	if (mi == NULL)
		return NULL;

	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE))
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

	CreateServiceFunction(MS_CLIST_SETSTATUSMODE, SetStatusMode);

	CreateServiceFunction(MS_CLIST_MENUPROCESSCOMMAND, MenuProcessCommand);
	CreateServiceFunction(MS_CLIST_MENUPROCESSHOTKEY, MenuProcessHotkey);

	hPreBuildContactMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDCONTACTMENU);
	hPreBuildMainMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDMAINMENU);
	cli.hPreBuildStatusMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDSTATUSMENU);
	hStatusModeChangeEvent = CreateHookableEvent(ME_CLIST_STATUSMODECHANGE);

	HookEvent(ME_PROTO_ACK, MenuProtoAck);

	hMainMenu = CreatePopupMenu();
	hStatusMenu = CreatePopupMenu();

	hStatusMainMenuHandles = NULL;
	hStatusMainMenuHandlesCnt = 0;

	hStatusMenuHandles = NULL;
	hStatusMenuHandlesCnt = 0;

	// new menu sys
	InitGenMenu();

	// main menu
	hMainMenuObject = Menu_AddObject("MainMenu", LPGEN("Main menu"), 0, "MainMenuExecService");
	Menu_ConfigureObject(hMainMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hMainMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataMainMenu");

	// contact menu
	hContactMenuObject = Menu_AddObject("ContactMenu", LPGEN("Contact menu"), "ContactMenuCheckService", "ContactMenuExecService");
	Menu_ConfigureObject(hContactMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hContactMenuObject, MCO_OPT_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataContactMenu");

	// other menus
	InitGroupMenus();
	InitFramesMenus();
	InitTrayMenus();

	// initialize hotkeys
	CreateServiceFunction(MS_CLIST_HKSTATUS, HotkeySetStatus);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.ptszSection = _T("Status");
	hkd.dwFlags = HKD_TCHAR;
	for (int i = 0; i < _countof(statusHotkeys); i++) {
		char szName[30];
		mir_snprintf(szName, _countof(szName), "StatusHotKey_%d", i);
		hkd.pszName = szName;
		hkd.lParam = statusModeList[i];
		hkd.ptszDescription = fnGetStatusModeDescription(hkd.lParam, 0);
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, '0' + i) | HKF_MIRANDA_LOCAL;
		hkd.pszService = MS_CLIST_HKSTATUS;
		statusHotkeys[i] = Hotkey_Register(&hkd);
	}

	HookEvent(ME_HOTKEYS_CHANGED, sttRebuildHotkeys);

	// add exit command to menu
	CMenuItem mi;
	mi.position = 0x7fffffff;
	mi.pszService = "CloseAction";
	mi.name.a = LPGEN("E&xit");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	Menu_AddMainMenuItem(&mi);

	cli.currentStatusMenuItem = ID_STATUS_OFFLINE;
	cli.currentDesiredStatusMode = ID_STATUS_OFFLINE;

	HookEvent(ME_SKIN_ICONSCHANGED, MenuIconsChanged);
}

void UninitCustomMenus(void)
{
	mir_free(hStatusMainMenuHandles);
	hStatusMainMenuHandles = NULL;

	mir_free(hStatusMenuHandles);
	hStatusMenuHandles = NULL;

	Menu_RemoveObject(hMainMenuObject);
	Menu_RemoveObject(hMainMenuObject);

	UnloadMoveToGroup();
	FreeMenuProtos();

	DestroyMenu(hMainMenu);
	DestroyMenu(hStatusMenu);
}
