/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
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
#include "genmenu.h"
#include "plugins.h"

static bool bIsGenMenuInited;
bool bIconsDisabled;
static mir_cs csMenuHook;

static int NextObjectId = 0x100, NextObjectMenuItemId = CLISTMENUIDMIN;

#if defined(_DEBUG)
static void DumpMenuItem(TMO_IntMenuItem* pParent, int level = 0)
{
	char temp[30];
	memset(temp, '\t', level);
	temp[level] = 0;

	for (TMO_IntMenuItem *pimi = pParent; pimi != NULL; pimi = pimi->next) {
		Netlib_Logf(NULL, "%sMenu item %08p [%08p]: %S", temp, pimi, pimi->mi.root, pimi->mi.name.t);

		TMO_IntMenuItem *submenu = pimi->submenu.first;
		if (submenu)
			DumpMenuItem(submenu, level + 1);
	}
}

#endif

static int CompareMenus(const TIntMenuObject *p1, const TIntMenuObject *p2)
{
	return mir_strcmp(p1->pszName, p2->pszName);
}

LIST<TIntMenuObject> g_menus(10, NumericKeySortT);

TIntMenuObject* GetMenuObjbyId(int id)
{
	switch (id) {
	case MO_MAIN:    id = hMainMenuObject;    break;
	case MO_CONTACT: id = hContactMenuObject; break;
	case MO_STATUS:  id = hStatusMenuObject;  break;
	case MO_PROTO:
		if (db_get_b(NULL, "CList", "MoveProtoMenus", true))
			id = hStatusMenuObject;
		else
			id = hMainMenuObject;
	}

	return g_menus.find((TIntMenuObject*)&id);
}

LPTSTR GetMenuItemText(TMO_IntMenuItem *pimi)
{
	if (pimi->mi.flags & CMIF_KEEPUNTRANSLATED)
		return pimi->mi.name.t;

	return TranslateTH(pimi->mi.hLangpack, pimi->mi.name.t);
}

///////////////////////////////////////////////////////////////////////////////

TMO_IntMenuItem* MO_RecursiveWalkMenu(TMO_IntMenuItem *parent, pfnWalkFunc func, void* param)
{
	if (parent == NULL)
		return NULL;

	TMO_IntMenuItem *pnext;
	for (TMO_IntMenuItem *pimi = parent; pimi != NULL; pimi = pnext) {
		TMO_IntMenuItem *submenu = pimi->submenu.first;
		pnext = pimi->next;
		if (func(pimi, param)) // it can destroy the menu item
			return pimi;

		if (submenu) {
			TMO_IntMenuItem *res = MO_RecursiveWalkMenu(submenu, func, param);
			if (res)
				return res;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_MeasureItem(MEASUREITEMSTRUCT *mis)
{
	if (!bIsGenMenuInited)
		return FALSE;

	if (mis == NULL)
		return FALSE;

	// prevent win9x from ugly menus displaying when there is no icon
	mis->itemWidth = 0;
	mis->itemHeight = 0;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)mis->itemData);
	if (pimi == NULL)
		return FALSE;

	if (pimi->iconId == -1)
		return FALSE;

	mis->itemWidth = max(0, g_iIconSX - GetSystemMetrics(SM_CXMENUCHECK) + 4);
	mis->itemHeight = g_iIconSY + 2;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_DrawItem(DRAWITEMSTRUCT *dis)
{
	if (!bIsGenMenuInited)
		return FALSE;

	if (dis == NULL)
		return FALSE;

	mir_cslock lck(csMenuHook);

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)dis->itemData);
	if (pimi == NULL || pimi->iconId == -1)
		return FALSE;

	int y = (dis->rcItem.bottom - dis->rcItem.top - g_iIconSY) / 2 + 1;
	if (dis->itemState & ODS_SELECTED) {
		if (dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 2; rc.right = g_iIconSX + 2;
			rc.top = y; rc.bottom = rc.top + g_iIconSY + 2;
			FillRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_SELECTED);
		}
		else ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_FOCUS);
	}
	else {
		if (dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 0; rc.right = g_iIconSX + 4;
			rc.top = y - 2; rc.bottom = rc.top + g_iIconSY + 4;
			DrawEdge(dis->hDC, &rc, BDR_SUNKENOUTER, BF_RECT);
			InflateRect(&rc, -1, -1);
			COLORREF menuCol = GetSysColor(COLOR_MENU);
			COLORREF hiliteCol = GetSysColor(COLOR_3DHIGHLIGHT);
			HBRUSH hBrush = CreateSolidBrush(RGB((GetRValue(menuCol) + GetRValue(hiliteCol)) / 2, (GetGValue(menuCol) + GetGValue(hiliteCol)) / 2, (GetBValue(menuCol) + GetBValue(hiliteCol)) / 2));
			FillRect(dis->hDC, &rc, GetSysColorBrush(COLOR_MENU));
			DeleteObject(hBrush);
			ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, GetSysColor(COLOR_MENU), ILD_BLEND50);
		}
		else ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int MO_RemoveAllObjects()
{
	for (int i = 0; i < g_menus.getCount(); i++)
		delete g_menus[i];
	g_menus.destroy();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(BOOL) Menu_ProcessHotKey(int hMenuObject, int key)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == NULL)
		return FALSE;

	for (TMO_IntMenuItem *pimi = pmo->m_items.first; pimi != NULL; pimi = pimi->next) {
		if (pimi->hotKey == 0) continue;
		if (HIWORD(pimi->hotKey) != key) continue;
		if (!(LOWORD(pimi->hotKey) & MOD_ALT) != !(GetKeyState(VK_MENU) & 0x8000)) continue;
		if (!(LOWORD(pimi->hotKey) & MOD_CONTROL) != !(GetKeyState(VK_CONTROL) & 0x8000)) continue;
		if (!(LOWORD(pimi->hotKey) & MOD_SHIFT) != !(GetKeyState(VK_SHIFT) & 0x8000)) continue;

		Menu_ProcessCommand(pimi, 0);
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HGENMENU) Menu_GetProtocolRoot(PROTO_INTERFACE *pThis)
{
	if (pThis == NULL)
		return 0;

	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE)) {
		if (pThis->m_hMainMenuItem != NULL) {
			Menu_RemoveItem(pThis->m_hMainMenuItem);
			pThis->m_hMainMenuItem = NULL;
		}
		return cli.pfnGetProtocolMenu(pThis->m_szModuleName);
	}

	TIntMenuObject *pmo = GetMenuObjbyId(hMainMenuObject);
	if (pmo == NULL)
		return NULL;

	mir_cslock lck(csMenuHook);
	for (TMO_IntMenuItem *p = pmo->m_items.first; p != NULL; p = p->next)
		if (!mir_strcmp(p->pszUniqName, pThis->m_szModuleName))
			return p;

	// create protocol root in the main menu
	CMenuItem mi;
	mi.name.t = pThis->m_tszUserName;
	mi.position = 500090000;
	mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
	mi.hIcolibItem = pThis->m_hProtoIcon;

	char szUid[33];
	if (db_get_static(NULL, pThis->m_szModuleName, "AM_MenuId", szUid, _countof(szUid))) {
		UUID id;
		UuidCreate(&id);
		bin2hex(&id, sizeof(id), szUid);
		db_set_s(NULL, pThis->m_szModuleName, "AM_MenuId", szUid);
	}
	hex2bin(szUid, &mi.uid, sizeof(mi.uid));

	return pThis->m_hMainMenuItem = Menu_AddMainMenuItem(&mi);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_GetItemInfo(HGENMENU hMenuItem, TMO_MenuItem &pInfo)
{
	if (!bIsGenMenuInited)
		return -1;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	mir_cslock lck(csMenuHook);
	if (pimi == NULL)
		return -1;

	pInfo = pimi->mi;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int FindDefaultItem(TMO_IntMenuItem *pimi, void*)
{
	if (pimi->mi.flags & (CMIF_GRAYED | CMIF_HIDDEN))
		return FALSE;

	return (pimi->mi.flags & CMIF_DEFAULT) ? TRUE : FALSE;
}

MIR_APP_DLL(HGENMENU) Menu_GetDefaultItem(HGENMENU hMenu)
{
	if (!bIsGenMenuInited)
		return NULL;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenu);
	mir_cslock lck(csMenuHook);
	return (pimi) ? MO_RecursiveWalkMenu(pimi, FindDefaultItem, NULL) : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void Menu_SetItemFlags(HGENMENU hMenuItem, bool bSet, int mask)
{
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	if (pimi == NULL)
		return;

	int flags = pimi->mi.flags;
	if (bSet)
		flags |= mask;
	else
		flags &= ~mask;

	if (!pimi->customVisible)
		flags |= CMIF_HIDDEN;

	// we allow to set only first 3 bits
	mir_cslock lck(csMenuHook);
	pimi->mi.flags = flags | (pimi->mi.flags & 0xFFFFFFF8);
}

MIR_APP_DLL(void) Menu_EnableItem(HGENMENU hMenuItem, bool bEnable)
{
	Menu_SetItemFlags(hMenuItem, !bEnable, CMIF_GRAYED);
}

MIR_APP_DLL(void) Menu_ShowItem(HGENMENU hMenuItem, bool bShow)
{
	Menu_SetItemFlags(hMenuItem, !bShow, CMIF_HIDDEN);
}

MIR_APP_DLL(void) Menu_SetChecked(HGENMENU hMenuItem, bool bSet)
{
	Menu_SetItemFlags(hMenuItem, bSet, CMIF_CHECKED);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_ModifyItem(HGENMENU hMenuItem, const TCHAR *ptszName, HANDLE hIcolib, int iFlags)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	if (pimi == NULL)
		return -1;

	if (ptszName != NULL)
		replaceStrT(pimi->mi.name.t, ptszName);

	if (iFlags != -1) {
		Menu_SetItemFlags(hMenuItem, true, iFlags);
		int oldflags = (pimi->mi.flags & 0xFFFFFFF8);
		if (!pimi->customVisible)
			oldflags |= CMIF_HIDDEN;
		pimi->mi.flags = (iFlags & 0x07) | oldflags;
	}

	if (hIcolib != INVALID_HANDLE_VALUE && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged((HICON)hIcolib);
		if (hIcolibItem) {
			HICON hIcon = IcoLib_GetIconByHandle(hIcolibItem, false);
			if (hIcon != NULL) {
				pimi->hIcolibItem = hIcolibItem;
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, hIcon);
				IcoLib_ReleaseIcon(hIcon);
			}
			else pimi->iconId = -1, pimi->hIcolibItem = NULL;
		}
		else {
			pimi->mi.hIcon = (HICON)hIcolib;
			if (hIcolib != NULL)
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, (HICON)hIcolib);
			else
				pimi->iconId = -1;	  //fixme, should remove old icon & shuffle all iconIds
		}
		if (pimi->hBmp) {
			DeleteObject(pimi->hBmp);
			pimi->hBmp = NULL;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void*) Menu_GetItemData(HGENMENU hMenuItem)
{
	if (!bIsGenMenuInited)
		return NULL;

	mir_cslock lck(csMenuHook);
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	return (pimi) ? pimi->pUserData : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

TMO_IntMenuItem *MO_GetIntMenuItem(HGENMENU wParam)
{
	TMO_IntMenuItem *result = (TMO_IntMenuItem*)wParam;
	if (result == NULL)
		return NULL;

	__try {
		if (result->signature != MENUITEM_SIGNATURE)
			result = NULL;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		result = NULL;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int FindMenuByCommand(TMO_IntMenuItem *pimi, void *pCommand)
{
	return (pimi->iCommand == *(int*)pCommand);
}

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	TMO_IntMenuItem *pimi;
	{
		mir_cslock lck(csMenuHook);
		TIntMenuObject *pmo = GetMenuObjbyId(menuID);
		if (pmo == NULL)
			return -1;

		pimi = MO_RecursiveWalkMenu(pmo->m_items.first, FindMenuByCommand, &command);
	}

	return (pimi) ? Menu_ProcessCommand(pimi, lParam) : -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_ProcessCommandById(int command, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	for (int i = 0; i < g_menus.getCount(); i++)
		if (TMO_IntMenuItem *pimi = MO_RecursiveWalkMenu(g_menus[i]->m_items.first, FindMenuByCommand, &command))
			return Menu_ProcessCommand(pimi, lParam);

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_ProcessCommand(HGENMENU hMenuItem, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return false;

	TMO_IntMenuItem *pimi;
	{
		mir_cslock lck(csMenuHook);
		if ((pimi = MO_GetIntMenuItem(hMenuItem)) == NULL)
			return false;
	}

	LPCSTR srvname = pimi->parent->ExecService;
	CallService(srvname, (WPARAM)pimi->pUserData, lParam);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "Rpcrt4.lib")

MIR_APP_DLL(int) Menu_ConfigureItem(HGENMENU hItem, int iOption, INT_PTR value)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hItem);
	if (pimi == NULL)
		return -1;

	switch (iOption) {
	case MCI_OPT_UNIQUENAME:
		replaceStr(pimi->pszUniqName, (char*)value);
		return 0;

	case MCI_OPT_HOTKEY:
		pimi->hotKey = (DWORD)value;
		return 0;

	case MCI_OPT_EXECPARAM:
		pimi->execParam = value;
		return 0;

	case MCI_OPT_UID:
		UuidFromStringA((RPC_CSTR)value, (UUID*)&pimi->mi.uid);
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_ConfigureObject(int hMenuObject, int setting, INT_PTR value)
{
	if (!bIsGenMenuInited)
		return false;

	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo != NULL) {
		switch (setting) {
		case MCO_OPT_ONADD_SERVICE:
			replaceStr(pmo->onAddService, (char*)value);
			return true;

		case MCO_OPT_FREE_SERVICE:
			replaceStr(pmo->FreeService, (char*)value);
			return true;

		case MCO_OPT_CHECK_SERVICE:
			replaceStr(pmo->CheckService, (char*)value);
			return true;

		case MCO_OPT_USERDEFINEDITEMS:
			pmo->m_bUseUserDefinedItems = (BOOL)value;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_AddObject(LPCSTR szName, LPCSTR szDisplayName, LPCSTR szCheckService, LPCSTR szExecService)
{
	if (szName == NULL || szDisplayName == NULL)
		return NULL;

	mir_cslock lck(csMenuHook);

	TIntMenuObject *p = new TIntMenuObject();
	p->id = NextObjectId++;
	p->pszName = mir_strdup(szName);
	p->ptszDisplayName = mir_a2t(szDisplayName);
	p->CheckService = mir_strdup(szCheckService);
	p->ExecService = mir_strdup(szExecService);
	p->m_hMenuIcons = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 15, 100);
	g_menus.insert(p);
	return p->id;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_RemoveObject(int hMenuObject)
{
	if (!bIsGenMenuInited || hMenuObject == 0)
		return -1;

	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == NULL)
		return -1;

	g_menus.remove(pmo);
	delete pmo;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// wparam = MenuItemHandle
// lparam = 0

static int FreeMenuItem(TMO_IntMenuItem* pimi, void*)
{
	pimi->parent->freeItem(pimi);
	return FALSE;
}

MIR_APP_DLL(int) Menu_RemoveItem(HGENMENU hMenuItem)
{
	mir_cslock lck(csMenuHook);
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	if (pimi == NULL)
		return -1;

	if (pimi->submenu.first) {
		MO_RecursiveWalkMenu(pimi->submenu.first, FreeMenuItem, NULL);
		pimi->submenu.first = NULL;
	}

	pimi->owner->remove(pimi);
	pimi->signature = 0; // invalidate all future calls to that object
	pimi->parent->freeItem(pimi);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct KillMenuItemsParam
{
	KillMenuItemsParam(int _hLangpack) :
		hLangpack(_hLangpack),
		arItems(10)
	{
	}

	int hLangpack;
	LIST<TMO_IntMenuItem> arItems;
};

int KillMenuItems(TMO_IntMenuItem *pimi, KillMenuItemsParam* param)
{
	if (pimi->mi.hLangpack == param->hLangpack)
		param->arItems.insert(pimi);
	return FALSE;
}

MIR_APP_DLL(void) KillModuleMenus(int _hLang)
{
	if (!bIsGenMenuInited)
		return;

	KillMenuItemsParam param(_hLang);

	mir_cslock lck(csMenuHook);
	for (int i = 0; i < g_menus.getCount(); i++)
		MO_RecursiveWalkMenu(g_menus[i]->m_items.first, (pfnWalkFunc)KillMenuItems, &param);

	for (int k = 0; k < param.arItems.getCount(); k++)
		Menu_RemoveItem(param.arItems[k]);
}

///////////////////////////////////////////////////////////////////////////////
// we presume that this function is being called inside csMenuHook only

static int PackMenuItems(TMO_IntMenuItem *pimi, void*)
{
	pimi->iCommand = NextObjectMenuItemId++;
	return FALSE;
}

static int GetNextObjectMenuItemId()
{
	// if menu commands are exausted, pack the menu array
	if (NextObjectMenuItemId >= CLISTMENUIDMAX) {
		NextObjectMenuItemId = CLISTMENUIDMIN;
		for (int i = 0; i < g_menus.getCount(); i++)
			MO_RecursiveWalkMenu(g_menus[i]->m_items.first, PackMenuItems, NULL);
	}

	return NextObjectMenuItemId++;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new submenu
// Returns a handle to the newly created root item or NULL

static int FindRoot(TMO_IntMenuItem *pimi, void *param)
{
	if (pimi->mi.name.t != NULL)
		if (pimi->submenu.first && !mir_tstrcmp(pimi->mi.name.t, (TCHAR*)param))
			return TRUE;

	return FALSE;
}

MIR_APP_DLL(HGENMENU) Menu_CreateRoot(int hMenuObject, LPCTSTR ptszName, int position, HANDLE hIcoLib, int _hLang)
{
	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == NULL)
		return NULL;

	TMO_IntMenuItem *oldroot = MO_RecursiveWalkMenu(pmo->m_items.first, FindRoot, (void*)ptszName);
	if (oldroot != NULL)
		return oldroot;

	CMenuItem mi;
	mi.flags = CMIF_TCHAR;
	mi.hIcolibItem = hIcoLib;
	mi.hLangpack = _hLang;
	mi.name.t = (TCHAR*)ptszName;
	mi.position = position;
	return Menu_AddItem(hMenuObject, &mi, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new menu item
// Returns a handle to the newly created item or NULL

MIR_APP_DLL(HGENMENU) Menu_AddItem(int hMenuObject, TMO_MenuItem *pmi, void *pUserData)
{
	if (!bIsGenMenuInited || pmi == NULL)
		return NULL;

	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == NULL)
		return NULL;

	TMO_IntMenuItem* p = (TMO_IntMenuItem*)mir_calloc(sizeof(TMO_IntMenuItem));
	p->parent = pmo;
	p->signature = MENUITEM_SIGNATURE;
	p->iCommand = GetNextObjectMenuItemId();
	p->mi = *pmi;
	p->customVisible = true;
	p->iconId = -1;
	p->originalPosition = pmi->position;
	p->pUserData = pUserData;

	if (pmi->flags & CMIF_UNICODE)
		p->mi.name.t = mir_tstrdup(pmi->name.t);
	else
		p->mi.name.t = mir_a2u(pmi->name.a);

	if (pmi->hIcon != NULL && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged(pmi->hIcon);
		if (hIcolibItem != NULL) {
			HICON hIcon = IcoLib_GetIconByHandle(hIcolibItem, false);
			p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, hIcon);
			p->hIcolibItem = hIcolibItem;
			IcoLib_ReleaseIcon(hIcon);
		}
		else p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, pmi->hIcon);
	}

	TMO_IntMenuItem *pRoot = (p->mi.root != NULL) ? MO_GetIntMenuItem(p->mi.root) : NULL;
	if (pRoot) {
		p->owner = &pRoot->submenu;

		// if parent menu has no icon, copy our icon there
		if (pRoot->iconId == -1)
			pRoot->iconId = p->iconId;
		
		// if parent menu has no uid, copy our id instead
		if (pmi->uid != miid_last && pRoot->mi.uid == miid_last) {
			char szUid[100];
			bin2hex(&pmi->uid, sizeof(pmi->uid), szUid);
			Netlib_Logf("[MENU]: fake UUID added to menu item %s", szUid);

			pRoot->mi.uid = pmi->uid;
			pRoot->mi.uid.d[7]--; // and make it slightly different
		}
	}
	else p->owner = &pmo->m_items;

	p->owner->insert(p);
	return p;
}

/////////////////////////////////////////////////////////////////////////////////////////
// wparam = MenuObjectHandle
// lparam = PMO_MenuItem

static int WhereToPlace(HMENU hMenu, TMO_MenuItem *mi)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for (int i = GetMenuItemCount(hMenu) - 1; i >= 0; i--) {
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.fType != MFT_SEPARATOR) {
			TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
			if (pimi != NULL)
				if (pimi->mi.position <= mi->position)
					return i + 1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD GetMenuItemType(HMENU hMenu, int uItem)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	GetMenuItemInfo(hMenu, uItem, TRUE, &mii);
	return mii.fType;
}

static UINT GetMenuItemTypeData(HMENU hMenu, int uItem, TMO_IntMenuItem* &p)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA | MIIM_TYPE;
	GetMenuItemInfo(hMenu, uItem, TRUE, &mii);
	p = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
	return mii.fType;
}

static void InsertSeparator(HMENU hMenu, int uItem)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, uItem, TRUE, &mii);
}

static void InsertMenuItemWithSeparators(HMENU hMenu, int uItem, MENUITEMINFO *lpmii)
{
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)lpmii->dwItemData), *p;
	if (pimi == NULL)
		return;

	// check for separator before
	if (uItem) {
		UINT fType = GetMenuItemTypeData(hMenu, uItem - 1, p);
		if (p != NULL && fType != MFT_SEPARATOR) {
			if ((p->mi.position / SEPARATORPOSITIONINTERVAL) != (pimi->mi.position / SEPARATORPOSITIONINTERVAL)) {
				// but might be supposed to be after the next one instead
				if (!(uItem < GetMenuItemCount(hMenu) && GetMenuItemType(hMenu, uItem) == MFT_SEPARATOR))
					InsertSeparator(hMenu, uItem);
				uItem++;
			}
		}
	}

	// check for separator after
	if (uItem < GetMenuItemCount(hMenu)) {
		UINT fType = GetMenuItemTypeData(hMenu, uItem, p);
		if (p != NULL && fType != MFT_SEPARATOR)
			if ((p->mi.position / SEPARATORPOSITIONINTERVAL) != (pimi->mi.position / SEPARATORPOSITIONINTERVAL))
				InsertSeparator(hMenu, uItem);
	}

	// create local copy *lpmii so we can change some flags
	MENUITEMINFO mii = *lpmii;

	int count = GetMenuItemCount(hMenu);
	if (count != 0 && (count % 33) == 0 && pimi->mi.root != NULL) {
		if (!(mii.fMask & MIIM_FTYPE))
			mii.fType = 0;
		mii.fMask |= MIIM_FTYPE;
		mii.fType |= MFT_MENUBARBREAK;
	}

	if (!pimi->ptszCustomName)
		mii.dwTypeData = GetMenuItemText(pimi);

	InsertMenuItem(hMenu, uItem, TRUE, &mii);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttReadOldItem(TMO_IntMenuItem *pmi, void *szModule)
{
	char menuItemName[200], szSetting[256];
	if (pmi->pszUniqName)
		mir_snprintf(menuItemName, "{%s}", pmi->pszUniqName);
	else if (pmi->mi.flags & CMIF_UNICODE)
		mir_snprintf(menuItemName, "{%s}", (char*)_T2A(pmi->mi.name.t));
	else
		mir_snprintf(menuItemName, "{%s}", pmi->mi.name.t);

	// check if it visible
	mir_snprintf(szSetting, "%s_visible", menuItemName);
	pmi->customVisible = db_get_b(NULL, (char*)szModule, szSetting, 1) != 0;
	if (pmi->customVisible)
		pmi->mi.flags &= ~CMIF_HIDDEN;
	else
		pmi->mi.flags |= CMIF_HIDDEN;

	// mi.name.t
	mir_snprintf(szSetting, "%s_name", menuItemName);
	TCHAR *tszCustomName = db_get_tsa(NULL, (char*)szModule, szSetting);
	if (tszCustomName != NULL) {
		mir_free(pmi->ptszCustomName);
		pmi->ptszCustomName = tszCustomName;
	}

	mir_snprintf(szSetting, "%s_pos", menuItemName);
	int pos = db_get_dw(NULL, (char*)szModule, szSetting, -1);
	if (pos == -1) {
		if (pmi->submenu.first)
			pmi->mi.position = 0;
	}
	else pmi->mi.position = pos;
	return 0;
}

static int sttDumpItem(TMO_IntMenuItem *pmi, void *szModule)
{
	if (pmi->mi.uid != miid_last) {
		char menuItemName[200];
		bin2hex(&pmi->mi.uid, sizeof(pmi->mi.uid), menuItemName);

		int bVisible = (pmi->mi.flags & CMIF_HIDDEN) == 0;
		TCHAR *ptszName = (pmi->ptszCustomName != NULL) ? pmi->ptszCustomName : _T("");
		
		char szRootUid[33];
		if (pmi->mi.root == NULL)
			szRootUid[0] = 0;
		else
			bin2hex(&pmi->mi.root->mi.uid, sizeof(MUUID), szRootUid);

		CMString szNewValue(FORMAT, _T("%d;%d;%S;%s"), bVisible, pmi->mi.position, szRootUid, ptszName);
		db_set_ts(NULL, (char*)szModule, menuItemName, szNewValue);

		Netlib_Logf(NULL, "MENU[%s] => %s, %d, %d", menuItemName, pmi->pszUniqName, bVisible, pmi->mi.position);
	}
	return 0;
}

static INT_PTR sttUpdateMenuService(WPARAM wParam, LPARAM)
{
	CallService(MS_SYSTEM_REMOVEWAIT, wParam, 0);
	CloseHandle((HANDLE)wParam);

	for (int i = 0; i < g_menus.getCount(); i++) {						 
		TIntMenuObject *pmo = g_menus[i];
		if (!pmo->m_bUseUserDefinedItems)
			continue;
		
		char szModule[256];
		mir_snprintf(szModule, "%s_Items", pmo->pszName);

		// was a menu converted?
		if (db_get_b(NULL, szModule, "MenuFormat", 0) == 0) { // no
			// read old settings
			MO_RecursiveWalkMenu(pmo->m_items.first, sttReadOldItem, szModule);

			// wipe out old trash, write new data & compatibility flag
			CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)szModule);
			db_set_b(NULL, szModule, "MenuFormat", true);
			MO_RecursiveWalkMenu(pmo->m_items.first, sttDumpItem, szModule);
		}
		else { // yes, menu is already converted, simply load its data
			for (int i = 0;; i++) {
				char szSetting[100];
				mir_snprintf(szSetting, "Custom%d", i);
				ptrA szCustomMenu(db_get_sa(NULL, szModule, szSetting));
				if (mir_strlen(szCustomMenu) != 32)
					break;

				TMO_MenuItem mi = {};
				mi.flags = CMIF_CUSTOM;
				mi.name.a = LPGEN("New submenu");
				mi.position = 500050000;
				BYTE *p = (BYTE*)&mi.uid;
				for (int i = 0; i < sizeof(MUUID); i++) {
					int tmp;
					sscanf(&szCustomMenu[i*2], "%02x", &tmp);
					p[i] = tmp;
				}
				Menu_AddItem(pmo->id, &mi, NULL);
			}

			MO_RecursiveWalkMenu(pmo->m_items.first, Menu_LoadFromDatabase, szModule);
		}
	}
	return 0;
}

#define MS_MENU_UPDATE "System/Genmenu/Update"

void ScheduleMenuUpdate()
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	CreateServiceFunction(MS_MENU_UPDATE, sttUpdateMenuService);
	CallService(MS_SYSTEM_WAITONHANDLE, (WPARAM)hEvent, (LPARAM)MS_MENU_UPDATE);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int sttFindMenuItemByUid(TMO_IntMenuItem *pimi, void *pUid)
{
	return 0 == memcmp(&pimi->mi.uid, pUid, sizeof(MUUID));
}

int Menu_LoadFromDatabase(TMO_IntMenuItem *pimi, void *szModule)
{
	if ((pimi->mi.flags & CMIF_SYSTEM) || pimi->mi.uid == miid_last)
		return 0;

	char menuItemName[256];
	bin2hex(&pimi->mi.uid, sizeof(pimi->mi.uid), menuItemName);

	TIntMenuObject *pmo = pimi->parent;
	ptrT szValue(db_get_tsa(NULL, (char*)szModule, menuItemName));
	if (szValue == NULL)
		return 0;

	TCHAR *ptszToken = szValue, *pDelim = _tcschr(szValue, ';');
	int bVisible = true, pos = 0;
	TCHAR tszCustomName[201]; tszCustomName[0] = 0;
	MUUID customRoot = {};
	for (int i = 0; i < 4; i++) {
		if (pDelim)
			*pDelim = 0;
		
		switch (i) {
		case 0: bVisible = _ttoi(ptszToken); break;
		case 1: pos = _ttoi(ptszToken); break;
		case 2:
			hex2binT(ptszToken, &customRoot, sizeof(customRoot));
			if (customRoot == pimi->mi.uid) // prevent a loop
				memset(&customRoot, 0, sizeof(customRoot));
			break;
		}

		ptszToken = pDelim + 1;
		if ((pDelim = _tcschr(ptszToken, ';')) == NULL) {
			if (i == 2 && *ptszToken != 0)
				_tcsncpy_s(tszCustomName, ptszToken, _TRUNCATE);
			break;
		}
	}

	pimi->mi.position = pos;
	pimi->customVisible = bVisible != 0;
	if (bVisible)
		pimi->mi.flags &= ~CMIF_HIDDEN;
	else
		pimi->mi.flags |= CMIF_HIDDEN;
	
	replaceStrT(pimi->ptszCustomName, tszCustomName[0] ? tszCustomName : NULL);

	MUUID currentUid;
	if (pimi->mi.root == NULL)
		memset(&currentUid, 0, sizeof(currentUid));
	else
		memcpy(&currentUid, &pimi->mi.root->mi.uid, sizeof(currentUid));
		
	if (currentUid != customRoot) { // need to move menu item to another root
		TMO_LinkedList *pNew;
		if (customRoot != miid_last) {
			TMO_IntMenuItem *p = MO_RecursiveWalkMenu(pmo->m_items.first, sttFindMenuItemByUid, &customRoot);
			if (p == NULL)
				return 0;

			if (p == pimi) { // prevent a loop
				Netlib_Logf(NULL, "MENU: preventing endless loop in %s", menuItemName);
				return 0;
			}

			pimi->mi.root = p;
			pNew = &p->submenu;
		}
		else {
			pimi->mi.root = NULL;
			pNew = &pmo->m_items;
		}

		// relink menu item
		pimi->owner->remove(pimi);
		pNew->insert(pimi);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define PUTPOSITIONSONMENU
#endif

static HMENU BuildRecursiveMenu(HMENU hMenu, TMO_IntMenuItem *pRootMenu, WPARAM wParam, LPARAM lParam)
{
	if (pRootMenu == NULL)
		return NULL;

	char szModule[256];
	TIntMenuObject *pmo = pRootMenu->parent;
	mir_snprintf(szModule, "%s_Items", pmo->pszName);

	if (pRootMenu->mi.root == NULL)
		while (GetMenuItemCount(hMenu) > 0)
			DeleteMenu(hMenu, 0, MF_BYPOSITION);

	for (TMO_IntMenuItem *pmi = pRootMenu; pmi != NULL; pmi = pmi->next) {
		TMO_MenuItem *mi = &pmi->mi;
		if (mi->flags & CMIF_HIDDEN)
			continue;

		if (pmo->CheckService != NULL) {
			TCheckProcParam CheckParam;
			CheckParam.wParam = wParam;
			CheckParam.lParam = lParam;
			CheckParam.MenuItemOwnerData = pmi->pUserData;
			CheckParam.MenuItemHandle = pmi;
			if (CallService(pmo->CheckService, (WPARAM)&CheckParam, 0) == false)
				continue;
		}

		int i = WhereToPlace(hMenu, mi);

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.dwItemData = (LPARAM)pmi;
		mii.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
		if (pmi->iconId != -1) {
			mii.fMask |= MIIM_BITMAP;
			if (IsWinVerVistaPlus() && IsThemeActive()) {
				if (pmi->hBmp == NULL)
					pmi->hBmp = ConvertIconToBitmap(pmi->parent->m_hMenuIcons, pmi->iconId);
				mii.hbmpItem = pmi->hBmp;
			}
			else mii.hbmpItem = HBMMENU_CALLBACK;
		}

		mii.fMask |= MIIM_STATE;
		mii.fState = ((pmi->mi.flags & CMIF_GRAYED) ? MFS_GRAYED : MFS_ENABLED);
		mii.fState |= ((pmi->mi.flags & CMIF_CHECKED) ? MFS_CHECKED : MFS_UNCHECKED);
		if (pmi->mi.flags & CMIF_DEFAULT)
			mii.fState |= MFS_DEFAULT;

		mii.dwTypeData = (pmi->ptszCustomName) ? pmi->ptszCustomName : mi->name.t;

		// it's a submenu
		if (pmi->submenu.first) {
			mii.fMask |= MIIM_SUBMENU;
			mii.hSubMenu = CreatePopupMenu();

#ifdef PUTPOSITIONSONMENU
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				TCHAR str[256];
				mir_sntprintf(str, _T("%s (%d, id %x)"), mi->name.a, mi->position, mii.dwItemData);
				mii.dwTypeData = str;
			}
#endif

			InsertMenuItemWithSeparators(hMenu, i, &mii);
			BuildRecursiveMenu(mii.hSubMenu, pmi->submenu.first, wParam, lParam);
		}
		else {
			mii.wID = pmi->iCommand;

#ifdef PUTPOSITIONSONMENU
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				TCHAR str[256];
				mir_sntprintf(str, _T("%s (%d, id %x)"), mi->name.a, mi->position, mii.dwItemData);
				mii.dwTypeData = str;
			}
#endif

			if (pmo->onAddService != NULL)
				if (CallService(pmo->onAddService, (WPARAM)&mii, (LPARAM)pmi) == FALSE)
					continue;

			InsertMenuItemWithSeparators(hMenu, i, &mii);
		}
	}

	return hMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////
// wparam started hMenu
// lparam ListParam*
// result hMenu

EXTERN_C MIR_APP_DLL(HMENU) Menu_Build(HMENU parent, int hMenuObject, WPARAM wParam, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return NULL;

	mir_cslock lck(csMenuHook);

	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == NULL)
		return NULL;

#if defined(_DEBUG)
	// DumpMenuItem(pmo->m_items.first);
#endif

	return BuildRecursiveMenu(parent, pmo->m_items.first, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// iconlib in menu

static int MO_ReloadIcon(TMO_IntMenuItem *pmi, void*)
{
	if (pmi->hIcolibItem) {
		HICON newIcon = IcoLib_GetIconByHandle(pmi->hIcolibItem, false);
		if (newIcon)
			ImageList_ReplaceIcon(pmi->parent->m_hMenuIcons, pmi->iconId, newIcon);

		IcoLib_ReleaseIcon(newIcon);
	}

	return FALSE;
}

int OnIconLibChanges(WPARAM, LPARAM)
{
	{
		mir_cslock lck(csMenuHook);
		for (int mo = 0; mo < g_menus.getCount(); mo++)
			if (hStatusMenuObject != g_menus[mo]->id) //skip status menu
				MO_RecursiveWalkMenu(g_menus[mo]->m_items.first, MO_ReloadIcon, 0);
	}

	cli.pfnReloadProtoMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int MO_RegisterIcon(TMO_IntMenuItem *pmi, void*)
{
	TCHAR *descr = GetMenuItemText(pmi);
	if (!descr || pmi->hIcolibItem != NULL || pmi->mi.uid == miid_last)
		return FALSE;

	HICON hIcon = ImageList_GetIcon(pmi->parent->m_hMenuIcons, pmi->iconId, 0);

	TCHAR sectionName[256];
	mir_sntprintf(sectionName, LPGENT("Menu icons") _T("/%s"), TranslateTS(pmi->parent->ptszDisplayName));

	char iconame[256], uname[100];
	bin2hex(&pmi->mi.uid, sizeof(pmi->mi.uid), uname);
	mir_snprintf(iconame, "genmenu_%s_%s", pmi->parent->pszName, uname);

	// remove '&'
	if (descr) {
		descr = NEWTSTR_ALLOCA(descr);

		for (TCHAR *p = descr; *p; p++) {
			if ((p = _tcschr(p, '&')) == NULL)
				break;

			memmove(p, p + 1, sizeof(TCHAR)*(mir_tstrlen(p + 1) + 1));
			if (*p == '\0')
				p++;
		}
	}

	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_TCHAR;
	sid.section.t = sectionName;
	sid.pszName = iconame;
	sid.description.t = descr;
	sid.hDefaultIcon = hIcon;
	pmi->hIcolibItem = IcoLib_AddIcon(&sid, 0);

	Safe_DestroyIcon(hIcon);
	if (hIcon = IcoLib_GetIcon(iconame)) {
		ImageList_ReplaceIcon(pmi->parent->m_hMenuIcons, pmi->iconId, hIcon);
		IcoLib_ReleaseIcon(hIcon);
	}

	return FALSE;
}

int RegisterAllIconsInIconLib()
{
	// register all icons
	for (int mo = 0; mo < g_menus.getCount(); mo++) {
		if (hStatusMenuObject == g_menus[mo]->id) //skip status menu
			continue;

		MO_RecursiveWalkMenu(g_menus[mo]->m_items.first, MO_RegisterIcon, 0);
	}

	return 0;
}

int TryProcessDoubleClick(MCONTACT hContact)
{
	TIntMenuObject *pmo = GetMenuObjbyId(hContactMenuObject);
	if (pmo != NULL) {
		NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

		TMO_IntMenuItem *pimi = Menu_GetDefaultItem(pmo->m_items.first);
		if (pimi != NULL) {
			Menu_ProcessCommand(pimi, hContact);
			return 0;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Static services

int posttimerid;

static VOID CALLBACK PostRegisterIcons(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(0, posttimerid);
	RegisterAllIconsInIconLib();
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	posttimerid = SetTimer(NULL, 0, 5, (TIMERPROC)PostRegisterIcons);
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconLibChanges);
	return 0;
}

int InitGenMenu()
{
	bIconsDisabled = db_get_b(NULL, "CList", "DisableMenuIcons", 0) != 0;
	bIsGenMenuInited = true;

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, GenMenuOptInit);
	return 0;
}

int UninitGenMenu()
{
	if (bIsGenMenuInited) {
		mir_cslock lck(csMenuHook);
		MO_RemoveAllObjects();
		bIsGenMenuInited = false;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

TIntMenuObject::TIntMenuObject()
{
}

TIntMenuObject::~TIntMenuObject()
{
	MO_RecursiveWalkMenu(m_items.first, FreeMenuItem, NULL);

	mir_free(FreeService);
	mir_free(onAddService);
	mir_free(CheckService);
	mir_free(ExecService);
	mir_free(ptszDisplayName);
	mir_free(pszName);

	ImageList_Destroy(m_hMenuIcons);
}

void TIntMenuObject::freeItem(TMO_IntMenuItem *p)
{
	if (FreeService)
		CallService(FreeService, (WPARAM)p, (LPARAM)p->pUserData);

	p->signature = 0;
	mir_free(p->mi.name.t);
	mir_free(p->pszUniqName);
	mir_free(p->ptszCustomName);
	if (p->hBmp) DeleteObject(p->hBmp);
	mir_free(p);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TMO_LinkedList::insert(TMO_IntMenuItem *pItem)
{
	pItem->owner = this;
	if (!first)
		first = pItem;
	if (last)
		last->next = pItem;
	last = pItem;
}

void TMO_LinkedList::remove(TMO_IntMenuItem *pItem)
{
	TMO_IntMenuItem *pPrev = NULL;
	for (TMO_IntMenuItem *p = first; p != NULL; p = p->next) {
		if (p == pItem) {
			if (first == pItem) first = pItem->next;
			if (last == pItem) last = pPrev;
			if (pPrev)
				pPrev->next = pItem->next;
			pItem->next = NULL;
			return;
		}
		pPrev = p;
	}
}
