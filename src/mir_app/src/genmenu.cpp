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
#include "genmenu.h"

static bool bIsGenMenuInited;
bool bIconsDisabled;
static mir_cs csMenuHook;

static int NextObjectId = 0x100, NextObjectMenuItemId = CLISTMENUIDMIN;

#if defined(_DEBUG)
static void DumpMenuItem(TMO_IntMenuItem* pParent, int level = 0)
{
	char temp[ 30 ];
	memset(temp, '\t', level);
	temp[ level ] = 0;

	for (TMO_IntMenuItem *pimi = pParent; pimi != NULL; pimi = pimi->next) {
		Netlib_Logf(NULL, "%sMenu item %08p [%08p]: %S", temp, pimi, pimi->mi.root, pimi->mi.name.t);

		TMO_IntMenuItem *submenu = pimi->submenu.first;
		if (submenu)
			DumpMenuItem(submenu, level+1);
	}
}

#endif

static int CompareMenus(const TIntMenuObject* p1, const TIntMenuObject* p2)
{
	return mir_strcmp(p1->pszName, p2->pszName);
}

LIST<TIntMenuObject> g_menus(10, CompareMenus);

void FreeAndNil(void **p)
{
	if (p == NULL)
		return;

	if (*p != NULL) {
		mir_free(*p);
		*p = NULL;
	}
}

int GetMenuObjbyId(int id)
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

	for (int i = 0; i < g_menus.getCount(); i++)
		if (g_menus[i]->id == id)
			return i;

	return -1;
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
		return FALSE;

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

	return FALSE;
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
	int objidx = GetMenuObjbyId(hMenuObject);
	if (objidx == -1)
		return FALSE;

	for (TMO_IntMenuItem *pimi = g_menus[objidx]->m_items.first; pimi != NULL; pimi = pimi->next) {
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

MIR_APP_DLL(HGENMENU) Menu_GetProtocolRoot(const char *szProto)
{
	if (szProto == NULL)
		return 0;

	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE))
		return cli.pfnGetProtocolMenu(szProto);

	int objidx = GetMenuObjbyId((int)hMainMenuObject);
	if (objidx == -1)
		return NULL;

	mir_cslock lck(csMenuHook);

	TIntMenuObject* pmo = g_menus[objidx];
	for (TMO_IntMenuItem *p = pmo->m_items.first; p != NULL; p = p->next)
		if (!mir_strcmp(p->UniqName, szProto))
			return p;

	return NULL;
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

MIR_APP_DLL(int) Menu_ModifyItem(HGENMENU hMenuItem, const TCHAR *ptszName, HANDLE hIcon, int iFlags)
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
		// we allow to set only first 3 bits
		int oldflags = (pimi->mi.flags & 0xFFFFFFF8);
		pimi->mi.flags = (iFlags & 0x07) | oldflags;
	}

	if (hIcon != INVALID_HANDLE_VALUE && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged((HICON)hIcon);
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
			pimi->mi.hIcolibItem = (HICON)hIcon;
			if (hIcon != NULL)
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, (HICON)hIcon);
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
	return (pimi) ? pimi->mi.ownerdata : NULL;
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

static int FindMenuByCommand(TMO_IntMenuItem *pimi, void* pCommand)
{
	return (pimi->iCommand == (int)pCommand);
}

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	TMO_IntMenuItem *pimi;
	{
		mir_cslock lck(csMenuHook);
		int objidx = GetMenuObjbyId(menuID);
		if (objidx == -1)
			return -1;

		pimi = MO_RecursiveWalkMenu(g_menus[objidx]->m_items.first, FindMenuByCommand, (void*)command);
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
		if (TMO_IntMenuItem *pimi = MO_RecursiveWalkMenu(g_menus[i]->m_items.first, FindMenuByCommand, (void*)command))
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
	CallService(srvname, (WPARAM)pimi->mi.ownerdata, lParam);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
		replaceStr(pimi->UniqName, (char*)value);
		return 0;

	case MCI_OPT_HOTKEY:
		pimi->hotKey = (DWORD)value;
		return 0;

	case MCI_OPT_EXECPARAM:
		pimi->execParam = value;
		return 0;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_ConfigureObject(int hMenuObject, int setting, INT_PTR value)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);

	int pimoidx = GetMenuObjbyId(hMenuObject);
	int res = pimoidx != -1;
	if (res) {
		TIntMenuObject* pmo = g_menus[pimoidx];

		switch (setting) {
		case MCO_OPT_ONADD_SERVICE:
			FreeAndNil((void**)&pmo->onAddService);
			pmo->onAddService = mir_strdup((char*)value);
			break;

		case MCO_OPT_FREE_SERVICE:
			FreeAndNil((void**)&pmo->FreeService);
			pmo->FreeService = mir_strdup((char*)value);
			break;

		case MCO_OPT_CHECK_SERVICE:
			FreeAndNil((void**)&pmo->CheckService);
			pmo->CheckService = mir_strdup((char*)value);
			break;

		case MCO_OPT_USERDEFINEDITEMS:
			pmo->m_bUseUserDefinedItems = (BOOL)value;
			break;
		}
	}

	return res;
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
	int objidx = GetMenuObjbyId(hMenuObject);
	if (objidx == -1)
		return -1;

	delete g_menus[objidx];
	g_menus.remove(objidx);
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

static int FindParent(TMO_IntMenuItem* pimi, void* p)
{
	return pimi->next == p;
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

	TMO_IntMenuItem *prev = MO_RecursiveWalkMenu(pimi->owner->first, FindParent, pimi);
	if (prev)
		prev->next = pimi->next;
	if (pimi->owner->first == pimi)
		pimi->owner->first = pimi->next;
	if (pimi->owner->last == pimi)
		pimi->owner->last = prev;

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
	{}

	int hLangpack;
	LIST<TMO_IntMenuItem> arItems;
};

int KillMenuItems(TMO_IntMenuItem *pimi, KillMenuItemsParam* param)
{
	if (pimi->hLangpack == param->hLangpack)
		param->arItems.insert(pimi);
	return FALSE;
}

MIR_APP_DLL(void) KillModuleMenus(int hLangpack)
{
	if (!bIsGenMenuInited)
		return;

	KillMenuItemsParam param(hLangpack);

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

MIR_APP_DLL(HGENMENU) Menu_CreateRoot(int hMenuObject, LPCTSTR ptszName, int position, HANDLE hIcoLib, int hLang)
{
	mir_cslock lck(csMenuHook);
	int objidx = GetMenuObjbyId(hMenuObject);
	if (objidx == -1)
		return NULL;

	TMO_IntMenuItem *oldroot = MO_RecursiveWalkMenu(g_menus[objidx]->m_items.first, FindRoot, (void*)ptszName);
	if (oldroot != NULL)
		return oldroot;

	TMO_MenuItem tmi = { 0 };
	tmi.flags = CMIF_TCHAR;
	tmi.hIcolibItem = hIcoLib;
	tmi.hLangpack = hLang;
	tmi.name.t = (TCHAR*)ptszName;
	tmi.position = position;
	return Menu_AddItem(hMenuObject, &tmi);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new menu item
// Returns a handle to the newly created item or NULL

MIR_APP_DLL(HGENMENU) Menu_AddItem(int hMenuObject, TMO_MenuItem *pmi)
{
	if (!bIsGenMenuInited || pmi == NULL)
		return NULL;

	mir_cslock lck(csMenuHook);
	int objidx = GetMenuObjbyId(hMenuObject);
	if (objidx == -1)
		return NULL;

	TIntMenuObject* pmo = g_menus[objidx];

	TMO_IntMenuItem* p = (TMO_IntMenuItem*)mir_calloc(sizeof(TMO_IntMenuItem));
	p->parent = pmo;
	p->signature = MENUITEM_SIGNATURE;
	p->iCommand = GetNextObjectMenuItemId();
	p->mi = *pmi;
	p->iconId = -1;
	p->OverrideShow = TRUE;
	p->originalPosition = pmi->position;
	p->hLangpack = pmi->hLangpack;

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

		if (pRoot->iconId == -1)
			pRoot->iconId = p->iconId;
	}
	else p->owner = &pmo->m_items;

	if (!p->owner->first)
		p->owner->first = p;
	if (p->owner->last)
		p->owner->last->next = p;
	p->owner->last = p;
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

	if (!pimi->CustomName)
		mii.dwTypeData = GetMenuItemText(pimi);

	InsertMenuItem(hMenu, uItem, TRUE, &mii);
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define PUTPOSITIONSONMENU
#endif

void GetMenuItemName(TMO_IntMenuItem *pMenuItem, char* pszDest, size_t cbDestSize)
{
	if (pMenuItem->UniqName)
		mir_snprintf(pszDest, cbDestSize, "{%s}", pMenuItem->UniqName);
	else if (pMenuItem->mi.flags & CMIF_UNICODE)
		mir_snprintf(pszDest, cbDestSize, "{%s}", (char*)_T2A(pMenuItem->mi.name.t));
	else
		mir_snprintf(pszDest, cbDestSize, "{%s}", pMenuItem->mi.name.t);
}

static HMENU BuildRecursiveMenu(HMENU hMenu, TMO_IntMenuItem *pRootMenu, INT_PTR iRootLevel, WPARAM wParam, LPARAM lParam)
{
	if (pRootMenu == NULL)
		return NULL;

	TIntMenuObject* pmo = pRootMenu->parent;

	if (iRootLevel == 0)
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
			CheckParam.MenuItemOwnerData = mi->ownerdata;
			CheckParam.MenuItemHandle = pmi;
			if (CallService(pmo->CheckService, (WPARAM)&CheckParam, 0) == FALSE)
				continue;
		}

		/**************************************/
		if (iRootLevel == 0 && mi->root == NULL && pmo->m_bUseUserDefinedItems) {
			char DBString[256];
			DBVARIANT dbv = { 0 };
			int pos;
			char MenuNameItems[256];
			mir_snprintf(MenuNameItems, _countof(MenuNameItems), "%s_Items", pmo->pszName);

			char menuItemName[256];
			GetMenuItemName(pmi, menuItemName, sizeof(menuItemName));

			// check if it visible
			mir_snprintf(DBString, _countof(DBString), "%s_visible", menuItemName);
			if (db_get_b(NULL, MenuNameItems, DBString, -1) == -1)
				db_set_b(NULL, MenuNameItems, DBString, 1);

			pmi->OverrideShow = TRUE;
			if (!db_get_b(NULL, MenuNameItems, DBString, 1)) {
				pmi->OverrideShow = FALSE;
				continue;  // find out what value to return if not getting added
			}

			// mi.name.t
			mir_snprintf(DBString, _countof(DBString), "%s_name", menuItemName);
			if (!db_get_ts(NULL, MenuNameItems, DBString, &dbv)) {
				if (mir_tstrlen(dbv.ptszVal) > 0)
					replaceStrT(pmi->CustomName, dbv.ptszVal);
				db_free(&dbv);
			}

			mir_snprintf(DBString, _countof(DBString), "%s_pos", menuItemName);
			if ((pos = db_get_dw(NULL, MenuNameItems, DBString, -1)) == -1) {
				db_set_dw(NULL, MenuNameItems, DBString, mi->position);
				if (pmi->submenu.first)
					mi->position = 0;
			}
			else mi->position = pos;
		}

		/**************************************/

		if (iRootLevel != (INT_PTR)pmi->mi.root)
			continue;

		int i = WhereToPlace(hMenu, mi);

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.dwItemData = (LPARAM)pmi;
		mii.fMask = MIIM_DATA | MIIM_ID | MIIM_STRING;
		if (pmi->iconId != -1) {
			mii.fMask |= MIIM_BITMAP;
			if (IsWinVerVistaPlus() && IsThemeActive()) {
				if (pmi->hBmp == NULL)
					pmi->hBmp = ConvertIconToBitmap(NULL, pmi->parent->m_hMenuIcons, pmi->iconId);
				mii.hbmpItem = pmi->hBmp;
			}
			else mii.hbmpItem = HBMMENU_CALLBACK;
		}

		mii.fMask |= MIIM_STATE;
		mii.fState = ((pmi->mi.flags & CMIF_GRAYED) ? MFS_GRAYED : MFS_ENABLED);
		mii.fState |= ((pmi->mi.flags & CMIF_CHECKED) ? MFS_CHECKED : MFS_UNCHECKED);
		if (pmi->mi.flags & CMIF_DEFAULT)
			mii.fState |= MFS_DEFAULT;

		mii.dwTypeData = (pmi->CustomName) ? pmi->CustomName : mi->name.t;

		// it's a submenu
		if (pmi->submenu.first) {
			mii.fMask |= MIIM_SUBMENU;
			mii.hSubMenu = CreatePopupMenu();

#ifdef PUTPOSITIONSONMENU
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				TCHAR str[256];
				mir_sntprintf(str, _countof(str), _T("%s (%d, id %x)"), mi->name.a, mi->position, mii.dwItemData);
				mii.dwTypeData = str;
			}
#endif

			InsertMenuItemWithSeparators(hMenu, i, &mii);
			BuildRecursiveMenu(mii.hSubMenu, pmi->submenu.first, LPARAM(pmi), wParam, lParam);
		}
		else {
			mii.wID = pmi->iCommand;

#ifdef PUTPOSITIONSONMENU
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				TCHAR str[256];
				mir_sntprintf(str, _countof(str), _T("%s (%d, id %x)"), mi->name.a, mi->position, mii.dwItemData);
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

	int pimoidx = GetMenuObjbyId(hMenuObject);
	if (pimoidx == -1)
		return 0;

	#if defined(_DEBUG)
		// DumpMenuItem(g_menus[pimoidx]->m_items.first);
	#endif

	return BuildRecursiveMenu(parent, g_menus[pimoidx]->m_items.first, 0, wParam, lParam);
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
			if ((int)hStatusMenuObject != g_menus[mo]->id) //skip status menu
				MO_RecursiveWalkMenu(g_menus[mo]->m_items.first, MO_ReloadIcon, 0);
	}

	cli.pfnReloadProtoMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int MO_RegisterIcon(TMO_IntMenuItem *pmi, void*)
{
	TCHAR *uname = (pmi->UniqName) ? mir_a2t(pmi->UniqName) : mir_tstrdup(pmi->CustomName),
		*descr = GetMenuItemText(pmi);

	if (!uname && !descr)
		return FALSE;

	if (!pmi->hIcolibItem) {
		HICON hIcon = ImageList_GetIcon(pmi->parent->m_hMenuIcons, pmi->iconId, 0);

		TCHAR sectionName[256];
		mir_sntprintf(sectionName, _countof(sectionName), LPGENT("Menu icons") _T("/%s"), TranslateTS(pmi->parent->ptszDisplayName));

		char iconame[256];
		mir_snprintf(iconame, _countof(iconame), "genmenu_%s_%s", pmi->parent->pszName, uname && *uname ? uname : descr);

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
	}

	mir_free(uname);
	return FALSE;
}

int RegisterAllIconsInIconLib()
{
	// register all icons
	for (int mo = 0; mo < g_menus.getCount(); mo++) {
		if ((int)hStatusMenuObject == g_menus[mo]->id) //skip status menu
			continue;

		MO_RecursiveWalkMenu(g_menus[mo]->m_items.first, MO_RegisterIcon, 0);
	}

	return 0;
}

int TryProcessDoubleClick(MCONTACT hContact)
{
	int iMenuID = GetMenuObjbyId((int)hContactMenuObject);
	if (iMenuID != -1) {
		NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

		TMO_IntMenuItem *pimi = Menu_GetDefaultItem(g_menus[iMenuID]->m_items.first);
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

int UnitGenMenu()
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

	FreeAndNil((void**)&FreeService);
	FreeAndNil((void**)&onAddService);
	FreeAndNil((void**)&CheckService);
	FreeAndNil((void**)&ExecService);
	FreeAndNil((void**)&ptszDisplayName);
	FreeAndNil((void**)&pszName);

	ImageList_Destroy(m_hMenuIcons);
}

void TIntMenuObject::freeItem(TMO_IntMenuItem *p)
{
	if (FreeService)
		CallService(FreeService, (WPARAM)p, (LPARAM)p->mi.ownerdata);

	p->signature = 0;
	FreeAndNil((void**)&p->mi.name.t);
	FreeAndNil((void**)&p->UniqName);
	FreeAndNil((void**)&p->CustomName);
	if (p->hBmp) DeleteObject(p->hBmp);
	mir_free(p);
}
