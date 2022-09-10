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

	for (TMO_IntMenuItem *pimi = pParent; pimi != nullptr; pimi = pimi->next) {
		Netlib_Logf(nullptr, "%sMenu item %08p [%08p]: %S", temp, pimi, pimi->mi.root, pimi->mi.name.w);

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
		if (db_get_b(0, "CList", "MoveProtoMenus", true))
			id = hStatusMenuObject;
		else
			id = hMainMenuObject;
	}

	return g_menus.find((TIntMenuObject*)&id);
}

LPTSTR GetMenuItemText(TMO_IntMenuItem *pimi)
{
	if (pimi->mi.flags & CMIF_KEEPUNTRANSLATED)
		return pimi->mi.name.w;

	return TranslateW_LP(pimi->mi.name.w, pimi->mi.pPlugin);
}

///////////////////////////////////////////////////////////////////////////////

typedef HANDLE(STDAPICALLTYPE* pfnBeginBufferedPaint)(HDC, RECT*, BP_BUFFERFORMAT, BP_PAINTPARAMS*, HDC*);
static pfnBeginBufferedPaint beginBufferedPaint;

typedef HRESULT(STDAPICALLTYPE* pfnEndBufferedPaint)(HANDLE, BOOL);
static pfnEndBufferedPaint endBufferedPaint;

HBITMAP ConvertIconToBitmap(HIMAGELIST hIml, int iconId)
{
	if (!beginBufferedPaint) {
		HMODULE hThemeAPI = GetModuleHandleA("uxtheme.dll");
		beginBufferedPaint = (pfnBeginBufferedPaint)GetProcAddress(hThemeAPI, "BeginBufferedPaint");
		endBufferedPaint = (pfnEndBufferedPaint)GetProcAddress(hThemeAPI, "EndBufferedPaint");
	}

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biWidth = g_iIconSX;
	bmi.bmiHeader.biHeight = g_iIconSY;

	HDC hdc = CreateCompatibleDC(nullptr);
	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);

	BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BP_PAINTPARAMS paintParams = { 0 };
	paintParams.cbSize = sizeof(paintParams);
	paintParams.dwFlags = BPPF_ERASE;
	paintParams.pBlendFunction = &bfAlpha;

	HDC hdcBuffer;
	RECT rcIcon = { 0, 0, g_iIconSX, g_iIconSY };
	HANDLE hPaintBuffer = beginBufferedPaint(hdc, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
	if (hPaintBuffer) {
		ImageList_Draw(hIml, iconId, hdc, 0, 0, ILD_TRANSPARENT);
		endBufferedPaint(hPaintBuffer, TRUE);
	}

	SelectObject(hdc, hbmpOld);
	DeleteDC(hdc);

	return hbmp;
}

///////////////////////////////////////////////////////////////////////////////

TMO_IntMenuItem* MO_RecursiveWalkMenu(TMO_IntMenuItem *parent, pfnWalkFunc func, void* param)
{
	if (parent == nullptr)
		return nullptr;

	TMO_IntMenuItem *pnext;
	for (TMO_IntMenuItem *pimi = parent; pimi != nullptr; pimi = pnext) {
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

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_MeasureItem(LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return FALSE;

	MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT*)lParam;
	if (mis == nullptr)
		return FALSE;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem((HGENMENU)mis->itemData);
	if (pimi == nullptr)
		return FALSE;

	if (pimi->iconId == -1)
		return FALSE;

	mis->itemWidth = max(0, g_iIconSX - GetSystemMetrics(SM_CXMENUCHECK) + 4);
	mis->itemHeight = g_iIconSY + 2;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_DrawItem(LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return FALSE;

	DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
	if (dis == nullptr)
		return FALSE;

	TMO_IntMenuItem *pimi;
	{
		mir_cslock lck(csMenuHook);
		pimi = MO_GetIntMenuItem((HGENMENU)dis->itemData);
		if (pimi == nullptr || pimi->iconId == -1)
			return FALSE;
	}

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
	for (auto &p : g_menus)
		delete p;
	g_menus.destroy();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(BOOL) Menu_ProcessHotKey(int hMenuObject, int key)
{
	if (!bIsGenMenuInited)
		return -1;

	TIntMenuObject *pmo;
	{
		mir_cslock lck(csMenuHook);
		pmo = GetMenuObjbyId(hMenuObject);
		if (pmo == nullptr)
			return FALSE;
	}

	for (TMO_IntMenuItem *pimi = pmo->m_items.first; pimi != nullptr; pimi = pimi->next) {
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
	if (pThis == nullptr)
		return nullptr;

	if (db_get_b(0, "CList", "MoveProtoMenus", TRUE)) {
		if (pThis->m_hmiMainMenu != nullptr) {
			Menu_RemoveItem(pThis->m_hmiMainMenu);
			pThis->m_hmiMainMenu = nullptr;
		}
		return Menu_GetProtocolMenu(pThis->m_szModuleName);
	}

	TIntMenuObject *pmo = GetMenuObjbyId(hMainMenuObject);
	if (pmo == nullptr)
		return nullptr;

	mir_cslock lck(csMenuHook);
	for (TMO_IntMenuItem *p = pmo->m_items.first; p != nullptr; p = p->next)
		if (!mir_strcmp(p->pszUniqName, pThis->m_szModuleName))
			return p;

	// create protocol root in the main menu
	CMenuItem mi(&g_plugin);
	mi.name.w = pThis->m_tszUserName;
	mi.position = 500090000;
	mi.flags = CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
	mi.hIcolibItem = pThis->m_hProtoIcon;

	char szUid[33];
	if (db_get_static(0, pThis->m_szModuleName, "AM_MenuId", szUid, _countof(szUid))) {
		UUID id;
		UuidCreate(&id);
		bin2hex(&id, sizeof(id), szUid);
		db_set_s(0, pThis->m_szModuleName, "AM_MenuId", szUid);
	}
	hex2bin(szUid, &mi.uid, sizeof(mi.uid));

	return pThis->m_hmiMainMenu = Menu_AddMainMenuItem(&mi);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_GetItemInfo(HGENMENU hMenuItem, TMO_MenuItem &pInfo)
{
	if (!bIsGenMenuInited)
		return -1;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	mir_cslock lck(csMenuHook);
	if (pimi == nullptr)
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
		return nullptr;

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenu);
	mir_cslock lck(csMenuHook);
	return (pimi) ? MO_RecursiveWalkMenu(pimi, FindDefaultItem, nullptr) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void Menu_SetItemFlags(HGENMENU hMenuItem, bool bSet, int mask)
{
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	if (pimi == nullptr)
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

MIR_APP_DLL(int) Menu_ModifyItem(HGENMENU hMenuItem, const wchar_t *ptszName, HANDLE hIcolib, int iFlags)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);

	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	if (pimi == nullptr)
		return -1;

	if (ptszName != nullptr)
		replaceStrW(pimi->mi.name.w, ptszName);

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
			if (hIcon != nullptr) {
				pimi->hIcolibItem = hIcolibItem;
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, hIcon);
				IcoLib_ReleaseIcon(hIcon);
			}
			else pimi->iconId = -1, pimi->hIcolibItem = nullptr;
		}
		else {
			pimi->mi.hIcon = (HICON)hIcolib;
			if (hIcolib != nullptr)
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, (HICON)hIcolib);
			else
				pimi->iconId = -1;	  //fixme, should remove old icon & shuffle all iconIds
		}
		if (pimi->hBmp) {
			DeleteObject(pimi->hBmp);
			pimi->hBmp = nullptr;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void*) Menu_GetItemData(HGENMENU hMenuItem)
{
	if (!bIsGenMenuInited)
		return nullptr;

	mir_cslock lck(csMenuHook);
	TMO_IntMenuItem *pimi = MO_GetIntMenuItem(hMenuItem);
	return (pimi) ? pimi->pUserData : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

TMO_IntMenuItem *MO_GetIntMenuItem(HGENMENU wParam)
{
	TMO_IntMenuItem *result = (TMO_IntMenuItem*)wParam;
	if (result == nullptr)
		return nullptr;

	__try {
		if (result->signature != MENUITEM_SIGNATURE)
			result = nullptr;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		result = nullptr;
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
		if (pmo == nullptr)
			return -1;

		pimi = MO_RecursiveWalkMenu(pmo->m_items.first, FindMenuByCommand, &command);
	}

	return (pimi) ? Menu_ProcessCommand(pimi, lParam) : -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(BOOL) Menu_ProcessCommandById(int command, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return false;

	for (auto &p : g_menus)
		if (TMO_IntMenuItem *pimi = MO_RecursiveWalkMenu(p->m_items.first, FindMenuByCommand, &command))
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
		if ((pimi = MO_GetIntMenuItem(hMenuItem)) == nullptr)
			return false;
	}

	LPCSTR srvname = pimi->parent->ExecService;
	CallServiceSync(srvname, (WPARAM)pimi->pUserData, lParam);
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
	if (pimi == nullptr)
		return -1;

	switch (iOption) {
	case MCI_OPT_UNIQUENAME:
		replaceStr(pimi->pszUniqName, (char*)value);
		return 0;

	case MCI_OPT_HOTKEY:
		pimi->hotKey = (uint32_t)value;
		return 0;

	case MCI_OPT_EXECPARAM:
		pimi->execParam = value;
		return 0;

	case MCI_OPT_UID:
		UuidFromStringA((RPC_CSTR)value, (UUID*)&pimi->mi.uid);
		return 0;

	case MCI_OPT_DISABLED:
		pimi->customVisible = false;
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
	if (pmo != nullptr) {
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
			pmo->m_bUseUserDefinedItems = value != 0;
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Menu_AddObject(LPCSTR szName, LPCSTR szDisplayName, LPCSTR szCheckService, LPCSTR szExecService)
{
	if (szName == nullptr || szDisplayName == nullptr)
		return 0;

	mir_cslock lck(csMenuHook);

	TIntMenuObject *p = new TIntMenuObject();
	p->id = NextObjectId++;
	p->pszName = mir_strdup(szName);
	p->ptszDisplayName = mir_a2u(szDisplayName);
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
	if (pmo == nullptr)
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
	if (pimi == nullptr)
		return -1;

	if (pimi->submenu.first) {
		MO_RecursiveWalkMenu(pimi->submenu.first, FreeMenuItem, nullptr);
		pimi->submenu.first = nullptr;
	}

	pimi->owner->remove(pimi);
	pimi->signature = 0; // invalidate all future calls to that object
	pimi->parent->freeItem(pimi);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct KillMenuItemsParam
{
	KillMenuItemsParam(CMPluginBase *_pPlugin) :
		pPlugin(_pPlugin),
		arItems(10)
	{
	}

	CMPluginBase *pPlugin;
	LIST<TMO_IntMenuItem> arItems;
};

int KillMenuItems(TMO_IntMenuItem *pimi, KillMenuItemsParam* param)
{
	if (pimi->mi.pPlugin == param->pPlugin)
		param->arItems.insert(pimi);
	return FALSE;
}

void KillModuleMenus(CMPluginBase *pPlugin)
{
	if (!bIsGenMenuInited)
		return;

	KillMenuItemsParam param(pPlugin);

	mir_cslock lck(csMenuHook);
	for (auto &p : g_menus)
		MO_RecursiveWalkMenu(p->m_items.first, (pfnWalkFunc)KillMenuItems, &param);

	for (auto &p : param.arItems)
		Menu_RemoveItem(p);
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
		for (auto &p : g_menus)
			MO_RecursiveWalkMenu(p->m_items.first, PackMenuItems, nullptr);
	}

	return NextObjectMenuItemId++;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new submenu
// Returns a handle to the newly created root item or nullptr

static int FindRoot(TMO_IntMenuItem *pimi, void *param)
{
	if (pimi->mi.name.w != nullptr)
		if (pimi->submenu.first && !mir_wstrcmp(pimi->mi.name.w, (wchar_t*)param))
			return TRUE;

	return FALSE;
}

MIR_APP_DLL(HGENMENU) Menu_CreateRoot(int hMenuObject, LPCTSTR ptszName, int position, HANDLE hIcoLib, HPLUGIN pPlugin)
{
	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == nullptr)
		return nullptr;

	TMO_IntMenuItem *oldroot = MO_RecursiveWalkMenu(pmo->m_items.first, FindRoot, (void*)ptszName);
	if (oldroot != nullptr)
		return oldroot;

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = hIcoLib;
	mi.pPlugin = pPlugin;
	mi.name.w = (wchar_t*)ptszName;
	mi.position = position;
	return Menu_AddItem(hMenuObject, &mi, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Adds new menu item
// Returns a handle to the newly created item or nullptr

MIR_APP_DLL(HGENMENU) Menu_AddItem(int hMenuObject, TMO_MenuItem *pmi, void *pUserData)
{
	if (!bIsGenMenuInited || pmi == nullptr)
		return nullptr;

	mir_cslock lck(csMenuHook);
	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == nullptr)
		return nullptr;

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
		p->mi.name.w = mir_wstrdup(pmi->name.w);
	else
		p->mi.name.w = mir_a2u(pmi->name.a);

	if (pmi->hIcon != nullptr && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged(pmi->hIcon);
		if (hIcolibItem != nullptr) {
			HICON hIcon = IcoLib_GetIconByHandle(hIcolibItem, false);
			p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, hIcon);
			p->hIcolibItem = hIcolibItem;
			IcoLib_ReleaseIcon(hIcon);
		}
		else p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, pmi->hIcon);
	}

	TMO_IntMenuItem *pRoot = (p->mi.root != nullptr) ? MO_GetIntMenuItem(p->mi.root) : nullptr;
	if (pRoot) {
		p->owner = &pRoot->submenu;

		// if parent menu has no icon, copy our icon there
		if (pRoot->iconId == -1)
			pRoot->iconId = p->iconId;
		
		// if parent menu has no uid, copy our id instead
		if (pmi->uid != miid_last && pRoot->mi.uid == miid_last) {
			char szUid[100];
			bin2hex(&pmi->uid, sizeof(pmi->uid), szUid);
			Netlib_Logf(nullptr, "[MENU]: fake UUID added to menu item %s", szUid);

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
			if (pimi != nullptr)
				if (pimi->mi.position <= mi->position)
					return i + 1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static uint32_t GetMenuItemType(HMENU hMenu, int uItem)
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
	if (pimi == nullptr)
		return;

	// check for separator before
	if (uItem) {
		UINT fType = GetMenuItemTypeData(hMenu, uItem - 1, p);
		if (p != nullptr && fType != MFT_SEPARATOR) {
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
		if (p != nullptr && fType != MFT_SEPARATOR)
			if ((p->mi.position / SEPARATORPOSITIONINTERVAL) != (pimi->mi.position / SEPARATORPOSITIONINTERVAL))
				InsertSeparator(hMenu, uItem);
	}

	// create local copy *lpmii so we can change some flags
	MENUITEMINFO mii = *lpmii;

	int count = GetMenuItemCount(hMenu);
	if (count != 0 && (count % 33) == 0 && pimi->mi.root != nullptr) {
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
		mir_snprintf(menuItemName, "{%s}", (char*)_T2A(pmi->mi.name.w));
	else
		mir_snprintf(menuItemName, "{%s}", pmi->mi.name.w);

	// check if it visible
	mir_snprintf(szSetting, "%s_visible", menuItemName);
	pmi->customVisible = db_get_b(0, (char*)szModule, szSetting, 1) != 0;
	if (pmi->customVisible)
		pmi->mi.flags &= ~CMIF_HIDDEN;
	else
		pmi->mi.flags |= CMIF_HIDDEN;

	// mi.name.w
	mir_snprintf(szSetting, "%s_name", menuItemName);
	wchar_t *tszCustomName = db_get_wsa(0, (char*)szModule, szSetting);
	if (tszCustomName != nullptr) {
		mir_free(pmi->ptszCustomName);
		pmi->ptszCustomName = tszCustomName;
	}

	mir_snprintf(szSetting, "%s_pos", menuItemName);
	int pos = db_get_dw(0, (char*)szModule, szSetting, -1);
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
		const wchar_t *ptszName = (pmi->ptszCustomName != nullptr) ? pmi->ptszCustomName : L"";
		
		char szRootUid[33];
		if (pmi->mi.root == nullptr)
			szRootUid[0] = 0;
		else
			bin2hex(&pmi->mi.root->mi.uid, sizeof(MUUID), szRootUid);

		CMStringW szNewValue(FORMAT, L"%d;%d;%S;%s", bVisible, pmi->mi.position, szRootUid, ptszName);
		db_set_ws(0, (char*)szModule, menuItemName, szNewValue);

		Netlib_Logf(nullptr, "MENU[%s] => %s, %d, %d", menuItemName, pmi->pszUniqName, bVisible, pmi->mi.position);
	}
	return 0;
}

static void CALLBACK sttUpdateMenuService()
{
	for (auto &pmo : g_menus) {
		if (!pmo->m_bUseUserDefinedItems)
			continue;
		
		char szModule[256];
		mir_snprintf(szModule, "%s_Items", pmo->pszName);

		// was a menu converted?
		if (db_get_b(0, szModule, "MenuFormat", 0) == 0) { // no
			// read old settings
			MO_RecursiveWalkMenu(pmo->m_items.first, sttReadOldItem, szModule);

			// wipe out old trash, write new data & compatibility flag
			db_delete_module(0, szModule);
			db_set_b(0, szModule, "MenuFormat", true);
			MO_RecursiveWalkMenu(pmo->m_items.first, sttDumpItem, szModule);
		}
		else { // yes, menu is already converted, simply load its data
			for (int j = 0;; j++) {
				char szSetting[100];
				mir_snprintf(szSetting, "Custom%d", j);
				ptrA szCustomMenu(db_get_sa(0, szModule, szSetting));
				if (mir_strlen(szCustomMenu) != 32)
					break;

				TMO_MenuItem mi = {};
				mi.flags = CMIF_CUSTOM;
				mi.name.a = LPGEN("New submenu");
				mi.position = 500050000;
				uint8_t *p = (uint8_t*)&mi.uid;
				for (int k = 0; k < sizeof(MUUID); k++) {
					int tmp;
					sscanf(&szCustomMenu[k*2], "%02x", &tmp);
					p[k] = tmp;
				}
				Menu_AddItem(pmo->id, &mi, nullptr);
			}

			MO_RecursiveWalkMenu(pmo->m_items.first, Menu_LoadFromDatabase, szModule);
		}
	}
}

void ScheduleMenuUpdate()
{
	Miranda_WaitOnHandle(sttUpdateMenuService);
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
	ptrW szValue(db_get_wsa(0, (char*)szModule, menuItemName));
	if (szValue == nullptr)
		return 0;

	wchar_t *ptszToken = szValue, *pDelim = wcschr(szValue, ';');
	int bVisible = true, pos = 0;
	wchar_t tszCustomName[201]; tszCustomName[0] = 0;
	MUUID customRoot = {};
	for (int i = 0; i < 4; i++) {
		if (pDelim)
			*pDelim = 0;
		
		switch (i) {
		case 0: bVisible = _wtoi(ptszToken); break;
		case 1: pos = _wtoi(ptszToken); break;
		case 2:
			hex2binW(ptszToken, &customRoot, sizeof(customRoot));
			if (customRoot == pimi->mi.uid) // prevent a loop
				memset(&customRoot, 0, sizeof(customRoot));
			break;
		}

		ptszToken = pDelim + 1;
		if ((pDelim = wcschr(ptszToken, ';')) == nullptr) {
			if (i == 2 && *ptszToken != 0)
				wcsncpy_s(tszCustomName, ptszToken, _TRUNCATE);
			break;
		}
	}

	pimi->mi.position = pos;
	pimi->customVisible = bVisible != 0;
	if (bVisible)
		pimi->mi.flags &= ~CMIF_HIDDEN;
	else
		pimi->mi.flags |= CMIF_HIDDEN;
	
	replaceStrW(pimi->ptszCustomName, tszCustomName[0] ? tszCustomName : nullptr);

	MUUID currentUid;
	if (pimi->mi.root == nullptr)
		memset(&currentUid, 0, sizeof(currentUid));
	else
		memcpy(&currentUid, &pimi->mi.root->mi.uid, sizeof(currentUid));
		
	if (currentUid != customRoot) { // need to move menu item to another root
		TMO_LinkedList *pNew;
		if (customRoot != miid_last) {
			TMO_IntMenuItem *p = MO_RecursiveWalkMenu(pmo->m_items.first, sttFindMenuItemByUid, &customRoot);
			if (p == nullptr)
				return 0;

			if (p == pimi) { // prevent a loop
				Netlib_Logf(nullptr, "MENU: preventing endless loop in %s", menuItemName);
				return 0;
			}

			pimi->mi.root = p;
			pNew = &p->submenu;
		}
		else {
			pimi->mi.root = nullptr;
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
	if (pRootMenu == nullptr)
		return nullptr;

	char szModule[256];
	TIntMenuObject *pmo = pRootMenu->parent;
	mir_snprintf(szModule, "%s_Items", pmo->pszName);

	if (pRootMenu->mi.root == nullptr)
		while (GetMenuItemCount(hMenu) > 0)
			DeleteMenu(hMenu, 0, MF_BYPOSITION);

	for (TMO_IntMenuItem *pmi = pRootMenu; pmi != nullptr; pmi = pmi->next) {
		TMO_MenuItem *mi = &pmi->mi;
		if (mi->flags & CMIF_HIDDEN)
			continue;

		if (pmo->CheckService != nullptr) {
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
				if (pmi->hBmp == nullptr)
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

		mii.dwTypeData = (pmi->ptszCustomName) ? pmi->ptszCustomName : mi->name.w;

		// it's a submenu
		if (pmi->submenu.first) {
			mii.fMask |= MIIM_SUBMENU;
			mii.hSubMenu = CreatePopupMenu();

#ifdef PUTPOSITIONSONMENU
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				wchar_t str[256];
				mir_snwprintf(str, L"%s (%d, id %x)", mi->name.a, mi->position, mii.dwItemData);
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
				wchar_t str[256];
				mir_snwprintf(str, L"%s (%d, id %x)", mi->name.a, mi->position, mii.dwItemData);
				mii.dwTypeData = str;
			}
#endif

			if (pmo->onAddService != nullptr)
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
		return nullptr;

	mir_cslock lck(csMenuHook);

	TIntMenuObject *pmo = GetMenuObjbyId(hMenuObject);
	if (pmo == nullptr)
		return nullptr;

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
		for (auto &p : g_menus)
			if (hStatusMenuObject != p->id) //skip status menu
				MO_RecursiveWalkMenu(p->m_items.first, MO_ReloadIcon, nullptr);
	}

	Menu_ReloadProtoMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// register all icons from all menus in IcoLib

static int MO_RegisterIcon(TMO_IntMenuItem *pmi, void*)
{
	wchar_t *descr = pmi->mi.name.w;
	if (!descr || pmi->hIcolibItem != nullptr || pmi->mi.uid == miid_last)
		return FALSE;

	HICON hIcon = ImageList_GetIcon(pmi->parent->m_hMenuIcons, pmi->iconId, 0);

	wchar_t sectionName[256];
	mir_snwprintf(sectionName, LPGENW("Menu icons") L"/%s", pmi->parent->ptszDisplayName);

	char iconame[256], uname[100];
	bin2hex(&pmi->mi.uid, sizeof(pmi->mi.uid), uname);
	mir_snprintf(iconame, "genmenu_%s_%s", pmi->parent->pszName, uname);

	// remove '&'
	if (descr) {
		descr = NEWWSTR_ALLOCA(descr);

		for (wchar_t *p = descr; *p; p++) {
			if ((p = wcschr(p, '&')) == nullptr)
				break;

			memmove(p, p + 1, sizeof(wchar_t)*(mir_wstrlen(p + 1) + 1));
			if (*p == '\0')
				p++;
		}
	}

	SKINICONDESC sid = {};
	sid.flags = SIDF_UNICODE;
	sid.section.w = sectionName;
	sid.pszName = iconame;
	sid.description.w = descr;
	sid.hDefaultIcon = hIcon;
	pmi->hIcolibItem = IcoLib_AddIcon(&sid, pmi->mi.pPlugin);

	Safe_DestroyIcon(hIcon);
	if (hIcon = IcoLib_GetIcon(iconame)) {
		ImageList_ReplaceIcon(pmi->parent->m_hMenuIcons, pmi->iconId, hIcon);
		IcoLib_ReleaseIcon(hIcon);
	}

	return FALSE;
}

static void CALLBACK RegisterAllIconsInIconLib()
{
	for (auto &p : g_menus) {
		if (hStatusMenuObject == p->id) //skip status menu
			continue;

		MO_RecursiveWalkMenu(p->m_items.first, MO_RegisterIcon, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Static services

int InitGenMenu()
{
	bIconsDisabled = db_get_b(0, "CList", "DisableMenuIcons", 0) != 0;
	bIsGenMenuInited = true;

	HookEvent(ME_OPT_INITIALISE, GenMenuOptInit);
	HookEvent(ME_SKIN_ICONSCHANGED, OnIconLibChanges);

	Miranda_WaitOnHandle(RegisterAllIconsInIconLib);
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
	MO_RecursiveWalkMenu(m_items.first, FreeMenuItem, nullptr);

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
	mir_free(p->mi.name.w);
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
	TMO_IntMenuItem *pPrev = nullptr;
	for (TMO_IntMenuItem *p = first; p != nullptr; p = p->next) {
		if (p == pItem) {
			if (first == pItem) first = pItem->next;
			if (last == pItem) last = pPrev;
			if (pPrev)
				pPrev->next = pItem->next;
			pItem->next = nullptr;
			return;
		}
		pPrev = p;
	}
}
