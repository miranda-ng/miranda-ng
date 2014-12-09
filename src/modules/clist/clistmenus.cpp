/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#pragma hdrstop

#include "m_hotkeys.h"

#include "clc.h"
#include "genmenu.h"

#define MS_CLIST_HKSTATUS "Clist/HK/SetStatus"

#define FIRSTCUSTOMMENUITEMID	30000
#define MENU_CUSTOMITEMMAIN		0x80000000
//#define MENU_CUSTOMITEMCONTEXT	0x40000000
//#define MENU_CUSTOMITEMFRAME	0x20000000

typedef struct  {
	WORD id;
	int iconId;
	CLISTMENUITEM mi;
}
	CListIntMenuItem, *lpCListIntMenuItem;

//new menu sys
HANDLE hMainMenuObject = 0;
HANDLE hContactMenuObject = 0;
HANDLE hStatusMenuObject = 0;
int UnloadMoveToGroup(void);

int statustopos(int status);
void Proto_SetStatus(const char *szProto, unsigned status);

bool prochotkey;

HANDLE hPreBuildMainMenuEvent, hStatusModeChangeEvent, hPreBuildContactMenuEvent;

static HMENU hMainMenu, hStatusMenu = 0;
static const int statusModeList[MAX_STATUS_COUNT] =
{
	ID_STATUS_OFFLINE, ID_STATUS_ONLINE, ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED,
	ID_STATUS_DND, ID_STATUS_FREECHAT, ID_STATUS_INVISIBLE, ID_STATUS_ONTHEPHONE, ID_STATUS_OUTTOLUNCH
};

static const int skinIconStatusList[MAX_STATUS_COUNT] =
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

PMO_IntMenuItem* hStatusMainMenuHandles;
int  hStatusMainMenuHandlesCnt;

typedef struct
{
	int protoindex;
	int protostatus[MAX_STATUS_COUNT];
	PMO_IntMenuItem menuhandle[MAX_STATUS_COUNT];
}
tStatusMenuHandles, *lpStatusMenuHandles;

lpStatusMenuHandles hStatusMenuHandles;
int hStatusMenuHandlesCnt;

//mainmenu exec param(ownerdata)
struct MainMenuExecParam
{
	char *szServiceName;
	TCHAR *szMenuName;
	int Param1;
};

//contactmenu exec param(ownerdata)
//also used in checkservice
struct ContactMenuExecParam
{
	char *szServiceName;
	char *pszContactOwner;//for check proc
	int param;
};

struct BuildContactParam
{
	char *szProto;
	int isOnList;
	int isOnline;
};

struct StatusMenuExecParam
{
	char *proto;            //This is unique protoname
	int protoindex;
	int status;

	BOOL custom;
	char *svc;
	HANDLE hMenuItem;
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

/////////////////////////////////////////////////////////////////////////////////////////
// MAIN MENU

static INT_PTR BuildMainMenu(WPARAM, LPARAM)
{
	ListParam param = { 0 };
	param.MenuObjectHandle = hMainMenuObject;

	NotifyEventHooks(hPreBuildMainMenuEvent, 0, 0);

	CallService(MO_BUILDMENU, (WPARAM)hMainMenu, (LPARAM)&param);
	DrawMenuBar((HWND)CallService("CLUI/GetHwnd", 0, 0));
	return (INT_PTR)hMainMenu;
}

static INT_PTR AddMainMenuItem(WPARAM, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return 0;

	MainMenuExecParam *mmep = (MainMenuExecParam*)mir_alloc(sizeof(MainMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	mmep->szMenuName = tmi.ptszName;
	tmi.ownerdata = mmep;

	PMO_IntMenuItem pimi = MO_AddNewMenuItem(hMainMenuObject, &tmi);

	char* name;
	bool needFree = false;

	if (mi->pszService)
		name = mi->pszService;
	else if (mi->flags & CMIF_UNICODE) {
		name = mir_t2a(mi->ptszName);
		needFree = true;
	}
	else name = mi->pszName;

	MO_SetOptionsMenuItem(pimi, OPT_MENUITEMSETUNIQNAME, (INT_PTR)name);
	if (needFree) mir_free(name);

	return (INT_PTR)pimi;
}

int MainMenuCheckService(WPARAM, LPARAM)
{
	return 0;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR MainMenuExecService(WPARAM wParam, LPARAM lParam)
{
	MainMenuExecParam *mmep = (MainMenuExecParam*)wParam;
	if (mmep != NULL) {
		// bug in help.c, it used wparam as parent window handle without reason.
		if (!mir_strcmp(mmep->szServiceName, "Help/AboutCommand"))
			mmep->Param1 = 0;

		CallService(mmep->szServiceName, mmep->Param1, lParam);
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

static INT_PTR AddContactMenuItem(WPARAM, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return 0;

	if (!(mi->flags & CMIF_ROOTHANDLE)) {
		//old system
		tmi.flags |= CMIF_ROOTHANDLE;
		tmi.root = NULL;
	}

	//owner data
	ContactMenuExecParam *cmep = (ContactMenuExecParam*)mir_calloc(sizeof(ContactMenuExecParam));
	cmep->szServiceName = mir_strdup(mi->pszService);
	if (mi->pszContactOwner != NULL)
		cmep->pszContactOwner = mir_strdup(mi->pszContactOwner);
	cmep->param = mi->popupPosition;
	tmi.ownerdata = cmep;

	//may be need to change how UniqueName is formed?
	PMO_IntMenuItem menuHandle = MO_AddNewMenuItem(hContactMenuObject, &tmi);
	char buf[256];
	if (mi->pszService)
		mir_snprintf(buf, SIZEOF(buf), "%s/%s", (mi->pszContactOwner) ? mi->pszContactOwner : "", (mi->pszService) ? mi->pszService : "");
	else if (mi->ptszName) {
		if (tmi.flags & CMIF_UNICODE)
			mir_snprintf(buf, SIZEOF(buf), "%s/NoService/%s", (mi->pszContactOwner) ? mi->pszContactOwner : "", _T2A(mi->ptszName));
		else
			mir_snprintf(buf, SIZEOF(buf), "%s/NoService/%s", (mi->pszContactOwner) ? mi->pszContactOwner : "", mi->ptszName);
	}
	else buf[0] = '\0';
	if (buf[0]) MO_SetOptionsMenuItem(menuHandle, OPT_MENUITEMSETUNIQNAME, (INT_PTR)buf);
	return (INT_PTR)menuHandle;
}

static INT_PTR BuildContactMenu(WPARAM hContact, LPARAM)
{
	NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

	char *szProto = GetContactProto(hContact);

	BuildContactParam bcp;
	bcp.szProto = szProto;
	bcp.isOnList = (db_get_b(hContact, "CList", "NotOnList", 0) == 0);
	bcp.isOnline = (szProto != NULL && ID_STATUS_OFFLINE != db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE));

	ListParam param = { 0 };
	param.MenuObjectHandle = hContactMenuObject;
	param.wParam = (WPARAM)&bcp;

	HMENU hMenu = CreatePopupMenu();
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);

	return (INT_PTR)hMenu;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR ContactMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		ContactMenuExecParam *cmep = (ContactMenuExecParam*)wParam;
		//call with wParam = (MCONTACT)hContact, lparam = popupposition
		CallService(cmep->szServiceName, lParam, cmep->param);
	}
	return 0;
}

//true - ok, false ignore
INT_PTR ContactMenuCheckService(WPARAM wParam, LPARAM)
{
	PCheckProcParam pcpp = (PCheckProcParam)wParam;
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
		if (strcmp(cmep->pszContactOwner, bcp->szProto)) return FALSE;
	}

	TMO_MenuItem mi;
	if (MO_GetMenuItem((WPARAM)pcpp->MenuItemHandle, (LPARAM)&mi) == 0) {
		if (mi.flags & CMIF_HIDDEN) return FALSE;
		if (mi.flags & CMIF_NOTONLIST  && bcp->isOnList) return FALSE;
		if (mi.flags & CMIF_NOTOFFLIST && !bcp->isOnList) return FALSE;
		if (mi.flags & CMIF_NOTONLINE  && bcp->isOnline) return FALSE;
		if (mi.flags & CMIF_NOTOFFLINE && !bcp->isOnline) return FALSE;
	}
	return TRUE;
}

INT_PTR FreeOwnerDataContactMenu(WPARAM, LPARAM lParam)
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

BOOL FindMenuHandleByGlobalID(HMENU hMenu, PMO_IntMenuItem id, MenuItemData* itdat)
{
	if (!itdat)
		return FALSE;

	MENUITEMINFO mii = { sizeof(mii) };
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

		PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
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
	PCheckProcParam pcpp = (PCheckProcParam)wParam;
	if (!pcpp)
		return TRUE;

	PMO_IntMenuItem timi = MO_GetIntMenuItem(pcpp->MenuItemHandle);
	if (!timi)
		return TRUE;

	StatusMenuExecParam *smep = (StatusMenuExecParam*)pcpp->MenuItemOwnerData;
	if (smep && !smep->status && smep->custom) {
		if (wildcmp(smep->svc, "*XStatus*")) {
			int XStatus;
			CUSTOM_STATUS cs = { sizeof(cs) };
			cs.flags = CSSF_MASK_STATUS;
			cs.status = &XStatus;
			if (CallProtoServiceInt(NULL, smep->proto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) != 0)
				XStatus = 0;

			char buf[255];
			mir_snprintf(buf, SIZEOF(buf), "*XStatus%d", XStatus);

			bool check = wildcmp(smep->svc, buf) != 0;
			bool reset = wildcmp(smep->svc, "*XStatus0") != 0;

			if (check)
				timi->mi.flags |= CMIF_CHECKED;
			else
				timi->mi.flags &= ~CMIF_CHECKED;

			if (reset || check) {
				PMO_IntMenuItem timiParent = MO_GetIntMenuItem(timi->mi.root);
				if (timiParent) {
					CLISTMENUITEM mi2 = { sizeof(mi2) };
					mi2.flags = CMIM_NAME | CMIF_TCHAR;
					mi2.ptszName = TranslateTH(timi->mi.hLangpack, timi->mi.hIcon ? timi->mi.ptszName : LPGENT("Custom status"));

					timiParent = MO_GetIntMenuItem(timi->mi.root);

					MenuItemData it = { 0 };

					if (FindMenuHandleByGlobalID(hStatusMenu, timiParent, &it)) {
						MENUITEMINFO mi = { 0 };
						TCHAR d[100];
						GetMenuString(it.OwnerMenu, it.position, d, SIZEOF(d), MF_BYPOSITION);

						mi.cbSize = sizeof(mi);
						mi.fMask = MIIM_STRING | MIIM_STATE;
						if (timi->iconId != -1) {
							mi.fMask |= MIIM_BITMAP;
							if (IsWinVerVistaPlus() && IsThemeActive()) {
								if (timi->hBmp == NULL)
									timi->hBmp = ConvertIconToBitmap(NULL, timi->parent->m_hMenuIcons, timi->iconId);
								mi.hbmpItem = timi->hBmp;
							}
							else mi.hbmpItem = HBMMENU_CALLBACK;
						}

						mi.fState |= (check && !reset ? MFS_CHECKED : MFS_UNCHECKED);
						mi.dwTypeData = mi2.ptszName;
						SetMenuItemInfo(it.OwnerMenu, it.position, TRUE, &mi);
					}

					Menu_ModifyItem(timi->mi.root, &mi2);
					timiParent->iconId = timi->iconId;
					if (timiParent->hBmp) DeleteObject(timiParent->hBmp);
					timiParent->hBmp = NULL;
				}
			}
		}
	}
	else if (smep && smep->status && !smep->custom) {
		int curProtoStatus = (smep->proto) ? CallProtoServiceInt(NULL, smep->proto, PS_GETSTATUS, 0, 0) : cli.pfnGetAverageMode(NULL);
		if (smep->status == curProtoStatus)
			timi->mi.flags |= CMIF_CHECKED;
		else
			timi->mi.flags &= ~CMIF_CHECKED;
	}
	else if ((!smep || smep->proto) && timi->mi.pszName) {
		int curProtoStatus = 0;
		BOOL IconNeedDestroy = FALSE;
		char* prot;
		if (smep)
			prot = smep->proto;
		else {
			char *prn = mir_u2a(timi->mi.ptszName);
			prot = NEWSTR_ALLOCA(prn);
			if (prn) mir_free(prn);
		}
		if (Proto_GetAccount(prot) == NULL)
			return TRUE;

		if ((curProtoStatus = CallProtoServiceInt(NULL, prot, PS_GETSTATUS, 0, 0)) == CALLSERVICE_NOTFOUND)
			curProtoStatus = 0;

		if (curProtoStatus >= ID_STATUS_OFFLINE && curProtoStatus < ID_STATUS_IDLE)
			timi->mi.hIcon = LoadSkinProtoIcon(prot, curProtoStatus);
		else {
			timi->mi.hIcon = (HICON)CallProtoServiceInt(NULL, prot, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);
			if (timi->mi.hIcon == (HICON)CALLSERVICE_NOTFOUND)
				timi->mi.hIcon = NULL;
			else
				IconNeedDestroy = TRUE;
		}

		if (timi->mi.hIcon) {
			timi->mi.flags |= CMIM_ICON;
			MO_ModifyMenuItem(timi, &timi->mi);
			if (IconNeedDestroy) {
				DestroyIcon(timi->mi.hIcon);
				timi->mi.hIcon = NULL;
			}
			else IcoLib_ReleaseIcon(timi->mi.hIcon, 0);
		}
	}

	return TRUE;
}

INT_PTR StatusMenuExecService(WPARAM wParam, LPARAM)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)wParam;
	if (smep != NULL) {
		if (smep->custom) {
			if (smep->svc && *smep->svc)
				CallService(smep->svc, 0, (LPARAM)smep->hMenuItem);
		}
		else {
			if (smep->status == 0 && smep->protoindex != 0 && smep->proto != NULL) {
				PMO_IntMenuItem pimi;
				char *prot = smep->proto;
				char szHumanName[64] = { 0 };
				PROTOACCOUNT *acc = Proto_GetAccount(smep->proto);
				bool bIsLocked = !Proto_IsAccountLocked(acc);
				db_set_b(NULL, prot, "LockMainStatus", bIsLocked);

				CallProtoServiceInt(NULL, smep->proto, PS_GETNAME, (WPARAM)SIZEOF(szHumanName), (LPARAM)szHumanName);
				pimi = MO_GetIntMenuItem((HGENMENU)smep->protoindex);
				PMO_IntMenuItem root = (PMO_IntMenuItem)pimi->mi.root;
				mir_free(pimi->mi.pszName);
				mir_free(root->mi.pszName);
				if (bIsLocked) {
					TCHAR buf[256];
					pimi->mi.flags |= CMIF_CHECKED;
					if (cli.bDisplayLocked) {
						mir_sntprintf(buf, SIZEOF(buf), TranslateT("%s (locked)"), acc->tszAccountName);
						pimi->mi.ptszName = mir_tstrdup(buf);
						root->mi.ptszName = mir_tstrdup(buf);
					}
					else {
						pimi->mi.ptszName = mir_tstrdup(acc->tszAccountName);
						root->mi.ptszName = mir_tstrdup(acc->tszAccountName);
					}
				}
				else {
					pimi->mi.ptszName = mir_tstrdup(acc->tszAccountName);
					root->mi.ptszName = mir_tstrdup(acc->tszAccountName);
					pimi->mi.flags &= ~CMIF_CHECKED;
				}
				if (cli.hwndStatus)
					InvalidateRect(cli.hwndStatus, NULL, TRUE);
			}
			else if (smep->proto != NULL) {
				Proto_SetStatus(smep->proto, smep->status);
				NotifyEventHooks(hStatusModeChangeEvent, smep->status, (LPARAM)smep->proto);
			}
			else {
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
		}
	}

	return 0;
}

INT_PTR FreeOwnerDataStatusMenu(WPARAM, LPARAM lParam)
{
	StatusMenuExecParam *smep = (StatusMenuExecParam*)lParam;
	if (smep != NULL) {
		FreeAndNil((void**)&smep->proto);
		FreeAndNil((void**)&smep->svc);
		FreeAndNil((void**)&smep);
	}

	return (0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Other menu functions

static INT_PTR ShowHideMenuItem(WPARAM wParam, LPARAM lParam)
{
	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)wParam);
	if (pimi == NULL)
		return 1;

	TMO_MenuItem tmi = { sizeof(tmi) };
	tmi.flags = CMIM_FLAGS + pimi->mi.flags;
	if (lParam)
		tmi.flags &= ~CMIF_HIDDEN;
	else
		tmi.flags |= CMIF_HIDDEN;

	return MO_ModifyMenuItem((PMO_IntMenuItem)wParam, &tmi);
}

//wparam MenuItemHandle
static INT_PTR ModifyCustomMenuItem(WPARAM wParam, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return 0;

	return MO_ModifyMenuItem((PMO_IntMenuItem)wParam, &tmi);
}

INT_PTR MenuProcessCommand(WPARAM wParam, LPARAM lParam)
{
	WORD cmd = LOWORD(wParam);

	if (HIWORD(wParam) & MPCF_MAINMENU) {
		int hst = LOWORD(wParam);
		if (hst >= ID_STATUS_OFFLINE && hst <= ID_STATUS_OUTTOLUNCH) {
			int pos = statustopos(hst);
			if (pos != -1 && hStatusMainMenuHandles != NULL)
				return MO_ProcessCommand(hStatusMainMenuHandles[pos], lParam);
		}
	}

	if (!(cmd >= CLISTMENUIDMIN && cmd <= CLISTMENUIDMAX))
		return 0; // DO NOT process ids outside from clist menu id range		v0.7.0.27+

	//process old menu sys
	if (HIWORD(wParam) & MPCF_CONTACTMENU)
		return MO_ProcessCommandBySubMenuIdent((int)hContactMenuObject, LOWORD(wParam), lParam);

	//unknown old menu
	return MO_ProcessCommandByMenuIdent(LOWORD(wParam), lParam);
}

BOOL FindMenuHanleByGlobalID(HMENU hMenu, PMO_IntMenuItem id, MenuItemData* itdat)
{
	if (!itdat)
		return FALSE;

	BOOL inSub = FALSE;

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for (int i = GetMenuItemCount(hMenu) - 1; i >= 0; i--) {
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.fType == MFT_SEPARATOR)
			continue;

		if (mii.hSubMenu)
			inSub = FindMenuHanleByGlobalID(mii.hSubMenu, id, itdat);
		if (inSub)
			return inSub;

		PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
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
		MO_ProcessHotKeys(hStatusMenuObject, vKey) ||
		MO_ProcessHotKeys(hMainMenuObject, vKey);

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

static INT_PTR MeasureMenuItem(WPARAM, LPARAM lParam)
{
	return MO_MeasureMenuItem((LPMEASUREITEMSTRUCT)lParam);
}

static INT_PTR DrawMenuItem(WPARAM, LPARAM lParam)
{
	return MO_DrawMenuItem((LPDRAWITEMSTRUCT)lParam);
}

int RecursiveDeleteMenu(HMENU hMenu)
{
	int cnt = GetMenuItemCount(hMenu);
	for (int i = 0; i < cnt; i++) {
		HMENU submenu = GetSubMenu(hMenu, 0);
		if (submenu) DestroyMenu(submenu);
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
	}
	return 0;
}

static INT_PTR MenuGetMain(WPARAM, LPARAM)
{
	RecursiveDeleteMenu(hMainMenu);
	BuildMainMenu(0, 0);
	return (INT_PTR)hMainMenu;
}

static INT_PTR BuildStatusMenu(WPARAM, LPARAM)
{
	ListParam param = { 0 };
	param.MenuObjectHandle = hStatusMenuObject;

	RecursiveDeleteMenu(hStatusMenu);
	CallService(MO_BUILDMENU, (WPARAM)hStatusMenu, (LPARAM)&param);
	return (INT_PTR)hStatusMenu;
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
		CallService(MO_REMOVEMENUOBJECT, (WPARAM)hStatusMenuObject, 0);
		mir_free(hStatusMainMenuHandles);
		mir_free(hStatusMenuHandles);
	}

	hStatusMenuObject = MO_CreateMenuObject("StatusMenu", LPGEN("Status menu"), "StatusMenuCheckService", "StatusMenuExecService");
	MO_SetOptionsMenuObject(hStatusMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataStatusMenu");

	hStatusMainMenuHandles = (PMO_IntMenuItem*)mir_calloc(SIZEOF(statusModeList) * sizeof(PMO_IntMenuItem*));
	hStatusMainMenuHandlesCnt = SIZEOF(statusModeList);

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

		//adding root
		TMO_MenuItem tmi = { 0 };
		tmi.cbSize = sizeof(tmi);
		tmi.flags = CMIF_TCHAR | CMIF_ROOTHANDLE | CMIF_KEEPUNTRANSLATED;
		tmi.position = pos++;
		tmi.hIcon = ic = (HICON)CallProtoServiceInt(NULL, pa->szModuleName, PS_LOADICON, PLI_PROTOCOL | PLIF_SMALL, 0);

		if (Proto_IsAccountLocked(pa) && cli.bDisplayLocked) {
			mir_sntprintf(tbuf, SIZEOF(tbuf), TranslateT("%s (locked)"), pa->tszAccountName);
			tmi.ptszName = tbuf;
		}
		else tmi.ptszName = pa->tszAccountName;

		//owner data
		StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->proto = mir_strdup(pa->szModuleName);
		tmi.ownerdata = smep;

		PMO_IntMenuItem rootmenu = MO_AddNewMenuItem(hStatusMenuObject, &tmi);

		memset(&tmi, 0, sizeof(tmi));
		tmi.cbSize = sizeof(tmi);
		tmi.flags = CMIF_TCHAR | CMIF_ROOTHANDLE | CMIF_KEEPUNTRANSLATED;
		tmi.root = rootmenu;
		tmi.position = pos++;
		tmi.hIcon = ic;

		//owner data
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->proto = mir_strdup(pa->szModuleName);
		tmi.ownerdata = smep;

		if (Proto_IsAccountLocked(pa))
			tmi.flags |= CMIF_CHECKED;

		if ((tmi.flags & CMIF_CHECKED) && cli.bDisplayLocked) {
			mir_sntprintf(tbuf, SIZEOF(tbuf), TranslateT("%s (locked)"), pa->tszAccountName);
			tmi.ptszName = tbuf;
		}
		else tmi.ptszName = pa->tszAccountName;

		PMO_IntMenuItem menuHandle = MO_AddNewMenuItem(hStatusMenuObject, &tmi);
		((StatusMenuExecParam*)tmi.ownerdata)->protoindex = (int)menuHandle;
		MO_ModifyMenuItem(menuHandle, &tmi);

		cli.menuProtos = (MenuProto*)mir_realloc(cli.menuProtos, sizeof(MenuProto)*(cli.menuProtoCount + 1));
		memset(&(cli.menuProtos[cli.menuProtoCount]), 0, sizeof(MenuProto));
		cli.menuProtos[cli.menuProtoCount].pMenu = rootmenu;
		cli.menuProtos[cli.menuProtoCount].szProto = mir_strdup(pa->szModuleName);

		cli.menuProtoCount++;

		char buf[256];
		mir_snprintf(buf, SIZEOF(buf), "RootProtocolIcon_%s", pa->szModuleName);
		MO_SetOptionsMenuItem(menuHandle, OPT_MENUITEMSETUNIQNAME, (INT_PTR)buf);

		DestroyIcon(ic);
		pos += 500000;

		for (int j = 0; j < SIZEOF(statusModeList); j++) {
			if (!(flags & statusModePf2List[j]))
				continue;

			// adding
			memset(&tmi, 0, sizeof(tmi));
			tmi.cbSize = sizeof(tmi);
			tmi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
			if (statusModeList[j] == ID_STATUS_OFFLINE)
				tmi.flags |= CMIF_CHECKED;
			tmi.root = rootmenu;
			tmi.position = pos++;
			tmi.ptszName = cli.pfnGetStatusModeDescription(statusModeList[j], GSMDF_UNTRANSLATED);
			tmi.hIcon = LoadSkinProtoIcon(pa->szModuleName, statusModeList[j]);

			// owner data
			StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->custom = FALSE;
			smep->status = statusModeList[j];
			smep->protoindex = i;
			smep->proto = mir_strdup(pa->szModuleName);
			tmi.ownerdata = smep;

			hStatusMenuHandles[i].protoindex = i;
			hStatusMenuHandles[i].protostatus[j] = statusModeList[j];
			hStatusMenuHandles[i].menuhandle[j] = MO_AddNewMenuItem(hStatusMenuObject, &tmi);

			char buf[256];
			mir_snprintf(buf, SIZEOF(buf), "ProtocolIcon_%s_%s", pa->szModuleName, tmi.pszName);
			MO_SetOptionsMenuItem(hStatusMenuHandles[i].menuhandle[j], OPT_MENUITEMSETUNIQNAME, (INT_PTR)buf);

			IcoLib_ReleaseIcon(tmi.hIcon, 0);
		}
	}

	NotifyEventHooks(cli.hPreBuildStatusMenuEvent, 0, 0);
	int pos = 200000;

	// add to root menu
	for (int j = 0; j < SIZEOF(statusModeList); j++) {
		for (int i = 0; i < accounts.getCount(); i++) {
			PROTOACCOUNT *pa = accounts[i];
			if (!bHideStatusMenu && !cli.pfnGetProtocolVisibility(pa->szModuleName))
				continue;

			DWORD flags = pa->ppro->GetCaps(PFLAGNUM_2, 0) & ~pa->ppro->GetCaps(PFLAGNUM_5, 0);
			if (!(flags & statusModePf2List[j]))
				continue;

			TMO_MenuItem tmi = { sizeof(tmi) };
			tmi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;
			if (statusModeList[j] == ID_STATUS_OFFLINE)
				tmi.flags |= CMIF_CHECKED;

			tmi.hIcon = LoadSkinIcon(skinIconStatusList[j]);
			tmi.position = pos++;
			tmi.hotKey = MAKELPARAM(MOD_CONTROL, '0' + j);

			//owner data
			StatusMenuExecParam *smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
			smep->status = statusModeList[j];
			tmi.ownerdata = smep;
			{
				TCHAR buf[256], hotkeyName[100];
				WORD hotKey = GetHotkeyValue(statusHotkeys[j]);
				HotkeyToName(hotkeyName, SIZEOF(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
				mir_sntprintf(buf, SIZEOF(buf), _T("%s\t%s"),
					cli.pfnGetStatusModeDescription(statusModeList[j], 0), hotkeyName);
				tmi.ptszName = buf;
				tmi.hotKey = MAKELONG(HIBYTE(hotKey), LOBYTE(hotKey));
				hStatusMainMenuHandles[j] = MO_AddNewMenuItem(hStatusMenuObject, &tmi);
			}

			char buf[256];
			mir_snprintf(buf, SIZEOF(buf), "Root2ProtocolIcon_%s_%s", pa->szModuleName, tmi.pszName);
			MO_SetOptionsMenuItem(hStatusMainMenuHandles[j], OPT_MENUITEMSETUNIQNAME, (INT_PTR)buf);

			IcoLib_ReleaseIcon(tmi.hIcon, 0);
			break;
		}
	}

	BuildStatusMenu(0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttRebuildHotkeys(WPARAM, LPARAM)
{
	TMO_MenuItem tmi = { sizeof(tmi) };
	tmi.flags = CMIM_HOTKEY | CMIM_NAME | CMIF_TCHAR;

	for (int j = 0; j < SIZEOF(statusModeList); j++) {
		TCHAR buf[256], hotkeyName[100];
		WORD hotKey = GetHotkeyValue(statusHotkeys[j]);
		HotkeyToName(hotkeyName, SIZEOF(hotkeyName), HIBYTE(hotKey), LOBYTE(hotKey));
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\t%s"), cli.pfnGetStatusModeDescription(statusModeList[j], 0), hotkeyName);
		tmi.ptszName = buf;
		tmi.hotKey = MAKELONG(HIBYTE(hotKey), LOBYTE(hotKey));
		MO_ModifyMenuItem(hStatusMainMenuHandles[j], &tmi);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int statustopos(int status)
{
	for (int j = 0; j < SIZEOF(statusModeList); j++)
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

	TMO_MenuItem tmi = { sizeof(tmi) };
	if (overallStatus >= ID_STATUS_OFFLINE) {
		int pos = statustopos(cli.currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		// reset all current possible checked statuses
		for (int pos2 = 0; pos2 < hStatusMainMenuHandlesCnt; pos2++) {
			if (pos2 >= 0 && pos2 < hStatusMainMenuHandlesCnt) {
				tmi.flags = CMIM_FLAGS | CMIF_ROOTHANDLE;
				MO_ModifyMenuItem(hStatusMainMenuHandles[pos2], &tmi);
			}
		}

		cli.currentStatusMenuItem = overallStatus;
		pos = statustopos(cli.currentStatusMenuItem);
		if (pos >= 0 && pos < hStatusMainMenuHandlesCnt) {
			tmi.flags = CMIM_FLAGS | CMIF_ROOTHANDLE | CMIF_CHECKED;
			MO_ModifyMenuItem(hStatusMainMenuHandles[pos], &tmi);
		}
	}
	else {
		int pos = statustopos(cli.currentStatusMenuItem);
		if (pos == -1)
			pos = 0;

		if (pos >= 0 && pos < hStatusMainMenuHandlesCnt) {
			tmi.flags = CMIM_FLAGS | CMIF_ROOTHANDLE;
			MO_ModifyMenuItem(hStatusMainMenuHandles[pos], &tmi);
		}

		cli.currentStatusMenuItem = 0;
	}

	for (int i = 0; i < accounts.getCount(); i++) {
		if (!mir_strcmp(accounts[i]->szModuleName, ack->szModule)) {
			if (((int)ack->hProcess >= ID_STATUS_OFFLINE || (int)ack->hProcess == 0) && (int)ack->hProcess < ID_STATUS_OFFLINE + SIZEOF(statusModeList)) {
				int pos = statustopos((int)ack->hProcess);
				if (pos == -1)
					pos = 0;
				for (pos = 0; pos < SIZEOF(statusModeList); pos++) {
					tmi.flags = CMIM_FLAGS | CMIF_ROOTHANDLE;
					MO_ModifyMenuItem(hStatusMenuHandles[i].menuhandle[pos], &tmi);
				}
			}

			if (ack->lParam >= ID_STATUS_OFFLINE && ack->lParam < ID_STATUS_OFFLINE + SIZEOF(statusModeList)) {
				int pos = statustopos((int)ack->lParam);
				if (pos >= 0 && pos < SIZEOF(statusModeList)) {
					tmi.flags = CMIM_FLAGS | CMIF_ROOTHANDLE | CMIF_CHECKED;
					MO_ModifyMenuItem(hStatusMenuHandles[i].menuhandle[pos], &tmi);
				}
			}
			break;
		}
	}

	//BuildStatusMenu(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int fnConvertMenu(CLISTMENUITEM *mi, TMO_MenuItem *pmi)
{
	if (mi == NULL || pmi == NULL)
		return FALSE;

	if (mi->cbSize != sizeof(CLISTMENUITEM))
		return FALSE;

	memset(pmi, 0, sizeof(TMO_MenuItem));
	pmi->cbSize = sizeof(TMO_MenuItem);
	pmi->root = mi->hParentMenu;
	pmi->flags = mi->flags;
	pmi->hIcon = mi->hIcon;
	pmi->hotKey = mi->hotKey;
	pmi->pszName = mi->pszName;
	pmi->position = mi->position;
	pmi->hLangpack = mi->hLangpack;
	return TRUE;
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

static INT_PTR AddStatusMenuItem(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;

	TMO_MenuItem tmi;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return 0;

	// for new style menus the pszPopupName contains the root menu handle
	PMO_IntMenuItem pRoot = NULL;
	if (mi->flags & CMIF_ROOTHANDLE)
		pRoot = MO_GetIntMenuItem(mi->hParentMenu);

	// for old style menus the pszPopupName really means the popup name
	else {
		MenuProto *mp = FindProtocolMenu(mi->pszContactOwner);
		if (mp && mi->pszPopupName) {
			if (mp->pMenu) {
				TCHAR *ptszName = (mi->flags & CMIF_UNICODE) ? mir_tstrdup(mi->ptszPopupName) : mir_a2t(mi->pszPopupName);
				pRoot = MO_RecursiveWalkMenu(mp->pMenu->submenu.first, FindRoot, ptszName);
				mir_free(ptszName);
			}
			if (pRoot == NULL) {
				TMO_MenuItem tmi = { 0 };
				tmi.cbSize = sizeof(tmi);
				tmi.flags = (mi->flags & CMIF_UNICODE) | CMIF_ROOTHANDLE;
				tmi.position = 1001;
				tmi.root = mp->pMenu;
				tmi.hIcon = NULL;
				tmi.pszName = mi->pszPopupName;
				pRoot = MO_AddNewMenuItem(hStatusMenuObject, &tmi);
			}

			tmi.flags |= CMIF_ROOTHANDLE;
			tmi.root = pRoot;
		}
	}

	if (wParam) {
		int *res = (int*)wParam;
		*res = (int)pRoot;
	}

	// owner data
	StatusMenuExecParam *smep = NULL;
	if (mi->pszService) {
		smep = (StatusMenuExecParam*)mir_calloc(sizeof(StatusMenuExecParam));
		smep->custom = TRUE;
		smep->svc = mir_strdup(mi->pszService);
		{
			char *buf = mir_strdup(mi->pszService);
			int i = 0;
			while (buf[i] != '\0' && buf[i] != '/') i++;
			buf[i] = '\0';
			smep->proto = mir_strdup(buf);
			mir_free(buf);
		}
		tmi.ownerdata = smep;
	}

	PMO_IntMenuItem menuHandle = MO_AddNewMenuItem(hStatusMenuObject, &tmi);
	if (smep)
		smep->hMenuItem = menuHandle;

	char buf[MAX_PATH + 64];
	char *p = (pRoot) ? mir_t2a(pRoot->mi.ptszName) : NULL;
	mir_snprintf(buf, SIZEOF(buf), "%s/%s", (p) ? p : "", mi->pszService ? mi->pszService : "");
	mir_free(p);

	MO_SetOptionsMenuItem(menuHandle, OPT_MENUITEMSETUNIQNAME, (INT_PTR)buf);
	return (INT_PTR)menuHandle;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR HotkeySetStatus(WPARAM wParam, LPARAM lParam)
{
	return SetStatusMode(lParam, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL MENU

static INT_PTR AddProtoMenuItem(WPARAM wParam, LPARAM lParam)
{
	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE))
		return AddStatusMenuItem(wParam, lParam);

	return AddMainMenuItem(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitCustomMenus(void)
{
	CreateServiceFunction("MainMenuExecService", MainMenuExecService);

	CreateServiceFunction("ContactMenuExecService", ContactMenuExecService);
	CreateServiceFunction("ContactMenuCheckService", ContactMenuCheckService);

	CreateServiceFunction("StatusMenuExecService", StatusMenuExecService);
	CreateServiceFunction("StatusMenuCheckService", StatusMenuCheckService);

	//free services
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataMainMenu", FreeOwnerDataMainMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataContactMenu", FreeOwnerDataContactMenu);
	CreateServiceFunction("CLISTMENUS/FreeOwnerDataStatusMenu", FreeOwnerDataStatusMenu);

	CreateServiceFunction(MS_CLIST_SETSTATUSMODE, SetStatusMode);

	CreateServiceFunction("CList/AddMainMenuItem", AddMainMenuItem);
	CreateServiceFunction("CList/AddStatusMenuItem", AddStatusMenuItem);
	CreateServiceFunction(MS_CLIST_MENUGETMAIN, MenuGetMain);
	CreateServiceFunction(MS_CLIST_MENUBUILDMAIN, BuildMainMenu);

	CreateServiceFunction("CList/AddContactMenuItem", AddContactMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDCONTACT, BuildContactMenu);

	CreateServiceFunction(MS_CLIST_SHOWHIDEMENUITEM, ShowHideMenuItem);
	CreateServiceFunction(MS_CLIST_MODIFYMENUITEM, ModifyCustomMenuItem);
	CreateServiceFunction(MS_CLIST_MENUMEASUREITEM, MeasureMenuItem);
	CreateServiceFunction(MS_CLIST_MENUDRAWITEM, DrawMenuItem);

	CreateServiceFunction(MS_CLIST_MENUGETSTATUS, BuildStatusMenu);
	CreateServiceFunction(MS_CLIST_MENUPROCESSCOMMAND, MenuProcessCommand);
	CreateServiceFunction(MS_CLIST_MENUPROCESSHOTKEY, MenuProcessHotkey);

	CreateServiceFunction("CList/AddProtoMenuItem", AddProtoMenuItem);

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
	hMainMenuObject = MO_CreateMenuObject("MainMenu", LPGEN("Main menu"), 0, "MainMenuExecService");
	MO_SetOptionsMenuObject(hMainMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetOptionsMenuObject(hMainMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataMainMenu");

	// contact menu
	hContactMenuObject = MO_CreateMenuObject("ContactMenu", LPGEN("Contact menu"), "ContactMenuCheckService", "ContactMenuExecService");
	MO_SetOptionsMenuObject(hContactMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetOptionsMenuObject(hContactMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, (INT_PTR)"CLISTMENUS/FreeOwnerDataContactMenu");

	// initialize hotkeys
	CreateServiceFunction(MS_CLIST_HKSTATUS, HotkeySetStatus);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.ptszSection = _T("Status");
	hkd.dwFlags = HKD_TCHAR;
	for (int i = 0; i < SIZEOF(statusHotkeys); i++) {
		char szName[30];
		mir_snprintf(szName, SIZEOF(szName), "StatusHotKey_%d", i);
		hkd.pszName = szName;
		hkd.lParam = statusModeList[i];
		hkd.ptszDescription = fnGetStatusModeDescription(hkd.lParam, 0);
		hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, '0' + i) | HKF_MIRANDA_LOCAL;
		hkd.pszService = MS_CLIST_HKSTATUS;
		statusHotkeys[i] = Hotkey_Register(&hkd);
	}

	HookEvent(ME_HOTKEYS_CHANGED, sttRebuildHotkeys);

	// add exit command to menu

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 0x7fffffff;
	mi.pszService = "CloseAction";
	mi.pszName = LPGEN("E&xit");
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_EXIT);
	AddMainMenuItem(0, (LPARAM)&mi);

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

	if (hMainMenuObject) CallService(MO_REMOVEMENUOBJECT, (WPARAM)hMainMenuObject, 0);
	if (hStatusMenuObject) CallService(MO_REMOVEMENUOBJECT, (WPARAM)hMainMenuObject, 0);

	UnloadMoveToGroup();
	FreeMenuProtos();

	DestroyMenu(hMainMenu);
	DestroyMenu(hStatusMenu);
}
