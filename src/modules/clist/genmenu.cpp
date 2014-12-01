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

	for (PMO_IntMenuItem pimi = pParent; pimi != NULL; pimi = pimi->next) {
		Netlib_Logf(NULL, "%sMenu item %08p [%08p]: %S", temp, pimi, pimi->mi.root, pimi->mi.ptszName);

		PMO_IntMenuItem submenu = pimi->submenu.first;
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

int GetMenuObjbyId(const int id)
{
	for (int i=0; i < g_menus.getCount(); i++)
		if (g_menus[i]->id == id)
			return i;

	return -1;
}

LPTSTR GetMenuItemText(PMO_IntMenuItem pimi)
{
	if (pimi->mi.flags & CMIF_KEEPUNTRANSLATED)
		return pimi->mi.ptszName;

	return TranslateTH(pimi->mi.hLangpack, pimi->mi.ptszName);
}

///////////////////////////////////////////////////////////////////////////////

PMO_IntMenuItem MO_RecursiveWalkMenu(PMO_IntMenuItem parent, pfnWalkFunc func, void* param)
{
	if (parent == NULL)
		return FALSE;

	PMO_IntMenuItem pnext;
	for (PMO_IntMenuItem pimi = parent; pimi != NULL; pimi = pnext) {
		PMO_IntMenuItem submenu = pimi->submenu.first;
		pnext = pimi->next;
		if (func(pimi, param)) // it can destroy the menu item
			return pimi;

		if (submenu) {
			PMO_IntMenuItem res = MO_RecursiveWalkMenu(submenu, func, param);
			if (res)
				return res;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//wparam = 0
//lparam = LPMEASUREITEMSTRUCT
int MO_MeasureMenuItem(LPMEASUREITEMSTRUCT mis)
{
	// prevent win9x from ugly menus displaying when there is no icon
	mis->itemWidth = 0;
	mis->itemHeight = 0;

	if (!bIsGenMenuInited)
		return -1;

	if (mis == NULL)
		return FALSE;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)mis->itemData);
	if (pimi == NULL)
		return FALSE;

	if (pimi->iconId == -1)
		return FALSE;

	mis->itemWidth = max(0, GetSystemMetrics(SM_CXSMICON)-GetSystemMetrics(SM_CXMENUCHECK)+4);
	mis->itemHeight = GetSystemMetrics(SM_CYSMICON)+2;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//wparam = 0
//lparam = LPDRAWITEMSTRUCT
int MO_DrawMenuItem(LPDRAWITEMSTRUCT dis)
{
	if (!bIsGenMenuInited)
		return -1;

	if (dis == NULL)
		return FALSE;

	mir_cslock lck(csMenuHook);

	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)dis->itemData);
	if (pimi == NULL || pimi->iconId == -1)
		return FALSE;

	int y = (dis->rcItem.bottom - dis->rcItem.top - GetSystemMetrics(SM_CYSMICON))/2+1;
	if (dis->itemState & ODS_SELECTED) {
		if (dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 2; rc.right = GetSystemMetrics(SM_CXSMICON)+2;
			rc.top = y; rc.bottom = rc.top+GetSystemMetrics(SM_CYSMICON)+2;
			FillRect(dis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_SELECTED);
		}
		else ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_DEFAULT, ILD_FOCUS);
	}
	else {
		if (dis->itemState & ODS_CHECKED) {
			RECT rc;
			rc.left = 0; rc.right = GetSystemMetrics(SM_CXSMICON)+4;
			rc.top = y-2; rc.bottom = rc.top + GetSystemMetrics(SM_CYSMICON)+4;
			DrawEdge(dis->hDC, &rc, BDR_SUNKENOUTER, BF_RECT);
			InflateRect(&rc, -1, -1);
			COLORREF menuCol = GetSysColor(COLOR_MENU);
			COLORREF hiliteCol = GetSysColor(COLOR_3DHIGHLIGHT);
			HBRUSH hBrush = CreateSolidBrush(RGB((GetRValue(menuCol)+GetRValue(hiliteCol))/2, (GetGValue(menuCol)+GetGValue(hiliteCol))/2, (GetBValue(menuCol)+GetBValue(hiliteCol))/2));
			FillRect(dis->hDC, &rc, GetSysColorBrush(COLOR_MENU));
			DeleteObject(hBrush);
			ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, GetSysColor(COLOR_MENU), ILD_BLEND50);
		}
		else ImageList_DrawEx(pimi->parent->m_hMenuIcons, pimi->iconId, dis->hDC, 2, y, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}
	return TRUE;
}

int MO_RemoveAllObjects()
{
	for (int i=0; i < g_menus.getCount(); i++)
		delete g_menus[i];
	g_menus.destroy();
	return 0;
}

//wparam = MenuObjectHandle
INT_PTR MO_RemoveMenuObject(WPARAM wParam, LPARAM)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	int objidx = GetMenuObjbyId((int)wParam);
	if (objidx == -1)
		return -1;

	delete g_menus[objidx];
	g_menus.remove(objidx);
	return 0;
}

//wparam = MenuObjectHandle
//lparam = vKey
INT_PTR MO_ProcessHotKeys(HANDLE menuHandle, INT_PTR vKey)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	int objidx = GetMenuObjbyId((int)menuHandle);
	if (objidx == -1)
		return FALSE;

	for (PMO_IntMenuItem pimi = g_menus[objidx]->m_items.first; pimi != NULL; pimi = pimi->next) {
		if (pimi->mi.hotKey == 0) continue;
		if (HIWORD(pimi->mi.hotKey) != vKey) continue;
		if (!(LOWORD(pimi->mi.hotKey) & MOD_ALT) != !(GetKeyState(VK_MENU) & 0x8000)) continue;
		if (!(LOWORD(pimi->mi.hotKey) & MOD_CONTROL) != !(GetKeyState(VK_CONTROL) & 0x8000)) continue;
		if (!(LOWORD(pimi->mi.hotKey) & MOD_SHIFT) != !(GetKeyState(VK_SHIFT) & 0x8000)) continue;

		MO_ProcessCommand(pimi, 0);
		return TRUE;
	}

	return FALSE;
}

INT_PTR MO_GetProtoRootMenu(WPARAM wParam, LPARAM lParam)
{
	char *szProto = (char*)wParam;
	if (szProto == NULL)
		return 0;

	if (db_get_b(NULL, "CList", "MoveProtoMenus", TRUE))
		return (INT_PTR)cli.pfnGetProtocolMenu(szProto);

	int objidx = GetMenuObjbyId((int)hMainMenuObject);
	if (objidx == -1)
		return NULL;

	mir_cslock lck(csMenuHook);

	TIntMenuObject* pmo = g_menus[objidx];
	for (PMO_IntMenuItem p = pmo->m_items.first; p != NULL; p = p->next)
		if (!mir_strcmp(p->UniqName, szProto))
			return (INT_PTR)p;

	return NULL;
}

//wparam = MenuItemHandle
//lparam = PMO_MenuItem
INT_PTR MO_GetMenuItem(WPARAM wParam, LPARAM lParam)
{
	PMO_MenuItem mi = (PMO_MenuItem)lParam;
	if (!bIsGenMenuInited || mi == NULL)
		return -1;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)wParam);
	mir_cslock lck(csMenuHook);
	if (pimi == NULL)
		return -1;

	*mi = pimi->mi;
	return 0;
}

static int FindDefaultItem(PMO_IntMenuItem pimi, void*)
{
	if (pimi->mi.flags & (CMIF_GRAYED | CMIF_HIDDEN))
		return FALSE;

	return (pimi->mi.flags & CMIF_DEFAULT) ? TRUE : FALSE;
}

INT_PTR MO_GetDefaultMenuItem(WPARAM wParam, LPARAM)
{
	if (!bIsGenMenuInited)
		return -1;

	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)wParam);
	mir_cslock lck(csMenuHook);
	return (pimi) ? (INT_PTR)MO_RecursiveWalkMenu(pimi, FindDefaultItem, NULL) : NULL;
}

//wparam MenuItemHandle
//lparam PMO_MenuItem
int MO_ModifyMenuItem(PMO_IntMenuItem menuHandle, PMO_MenuItem pmi)
{
	int oldflags;

	if (!bIsGenMenuInited || pmi == NULL || pmi->cbSize != sizeof(TMO_MenuItem))
		return -1;

	mir_cslock lck(csMenuHook);

	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)menuHandle);
	if (pimi == NULL)
		return -1;

	if (pmi->flags & CMIM_NAME) {
		FreeAndNil((void**)&pimi->mi.pszName);

		if (pmi->flags & CMIF_UNICODE)
			pimi->mi.ptszName = mir_tstrdup(pmi->ptszName);
		else
			pimi->mi.ptszName = mir_a2t(pmi->pszName);
	}

	if (pmi->flags & CMIM_FLAGS) {
		oldflags = (pimi->mi.flags & CMIF_ROOTHANDLE);
		pimi->mi.flags = (pmi->flags & ~CMIM_ALL) | oldflags;
	}

	if ((pmi->flags & CMIM_ICON) && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged(pmi->hIcon);
		if (hIcolibItem) {
			HICON hIcon = IcoLib_GetIconByHandle(hIcolibItem, false);
			if (hIcon != NULL) {
				pimi->hIcolibItem = hIcolibItem;
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, hIcon);
				IcoLib_ReleaseIcon(hIcon, 0);
			}
			else pimi->iconId = -1, pimi->hIcolibItem = NULL;
		}
		else {
			pimi->mi.hIcon = pmi->hIcon;
			if (pmi->hIcon != NULL)
				pimi->iconId = ImageList_ReplaceIcon(pimi->parent->m_hMenuIcons, pimi->iconId, pmi->hIcon);
			else
				pimi->iconId = -1;	  //fixme, should remove old icon & shuffle all iconIds
		}
		if (pimi->hBmp) {
			DeleteObject(pimi->hBmp);
			pimi->hBmp = NULL;
		}
	}

	if (pmi->flags & CMIM_HOTKEY)
		pimi->mi.hotKey = pmi->hotKey;

	return 0;
}

//wparam MenuItemHandle
//return ownerdata useful to free ownerdata before delete menu item,
//NULL on error.
INT_PTR MO_MenuItemGetOwnerData(WPARAM wParam, LPARAM)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)wParam);
	return (pimi) ? (INT_PTR)pimi->mi.ownerdata : -1;
}

PMO_IntMenuItem MO_GetIntMenuItem(HGENMENU wParam)
{
	PMO_IntMenuItem result = (PMO_IntMenuItem)wParam;
	if (result == NULL || wParam == (HGENMENU)0xffff1234 || wParam == HGENMENU_ROOT)
		return NULL;

	__try
	{
		if (result->signature != MENUITEM_SIGNATURE)
			result = NULL;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		result = NULL;
	}

	return result;
}

//LOWORD(wparam) menuident

static int FindMenuByCommand(PMO_IntMenuItem pimi, void* pCommand)
{
	return (pimi->iCommand == (int)pCommand);
}

int MO_ProcessCommandBySubMenuIdent(int menuID, int command, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	PMO_IntMenuItem pimi;
	{
		mir_cslock lck(csMenuHook);
		int objidx = GetMenuObjbyId(menuID);
		if (objidx == -1)
			return -1;

		pimi = MO_RecursiveWalkMenu(g_menus[objidx]->m_items.first, FindMenuByCommand, (void*)command);
	}

	return (pimi) ? MO_ProcessCommand(pimi, lParam) : -1;
}

INT_PTR MO_ProcessCommandByMenuIdent(WPARAM wParam, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	PMO_IntMenuItem pimi;
	{
		mir_cslock lck(csMenuHook);
		for (int i=0; i < g_menus.getCount(); i++)
			if ((pimi = MO_RecursiveWalkMenu(g_menus[i]->m_items.first, FindMenuByCommand, (void*)wParam)) != NULL)
				break;
	}

	return (pimi) ? MO_ProcessCommand(pimi, lParam) : FALSE;
}

int MO_ProcessCommand(PMO_IntMenuItem aHandle, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	PMO_IntMenuItem pimi;
	{
		mir_cslock lck(csMenuHook);
		if ((pimi = MO_GetIntMenuItem(aHandle)) == NULL)
			return -1;
	}

	LPCSTR srvname = pimi->parent->ExecService;
	void *ownerdata = pimi->mi.ownerdata;
	CallService(srvname, (WPARAM)ownerdata, lParam);
	return 1;
}

int MO_SetOptionsMenuItem(PMO_IntMenuItem aHandle, int setting, INT_PTR value)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);
	PMO_IntMenuItem pimi = MO_GetIntMenuItem(aHandle);
	if (pimi == NULL)
		return -1;

	if (setting == OPT_MENUITEMSETUNIQNAME) {
		mir_free(pimi->UniqName);
		pimi->UniqName = mir_strdup((char*)value);
	}

	return 1;
}

int MO_SetOptionsMenuObject(HANDLE handle, int setting, INT_PTR value)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);

	int pimoidx = GetMenuObjbyId((int)handle);
	int res = pimoidx != -1;
	if (res) {
		TIntMenuObject* pmo = g_menus[pimoidx];

		switch (setting) {
		case OPT_MENUOBJECT_SET_ONADD_SERVICE:
			FreeAndNil((void**)&pmo->onAddService);
			pmo->onAddService = mir_strdup((char*)value);
			break;

		case OPT_MENUOBJECT_SET_FREE_SERVICE:
			FreeAndNil((void**)&pmo->FreeService);
			pmo->FreeService = mir_strdup((char*)value);
			break;

		case OPT_MENUOBJECT_SET_CHECK_SERVICE:
			FreeAndNil((void**)&pmo->CheckService);
			pmo->CheckService = mir_strdup((char*)value);
			break;

		case OPT_USERDEFINEDITEMS:
			pmo->m_bUseUserDefinedItems = (BOOL)value;
			break;
		}
	}

	return res;
}

//wparam = LPCSTR szDisplayName;
//lparam = PMenuParam;
//result = MenuObjectHandle
INT_PTR MO_CreateNewMenuObject(WPARAM wParam, LPARAM lParam)
{
	TMenuParam *pmp = (TMenuParam *)lParam;
	if (!bIsGenMenuInited || pmp == NULL)
		return -1;

	mir_cslock lck(csMenuHook);

	TIntMenuObject* p = new TIntMenuObject();
	p->id = NextObjectId++;
	p->pszName = mir_strdup(pmp->name);
	p->ptszDisplayName = mir_a2t(LPCSTR(wParam));
	p->CheckService = mir_strdup(pmp->CheckService);
	p->ExecService = mir_strdup(pmp->ExecService);
	p->m_hMenuIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 15, 100);
	g_menus.insert(p);
	return p->id;
}

//wparam = MenuItemHandle
//lparam = 0

static int FreeMenuItem(TMO_IntMenuItem* pimi, void*)
{
	pimi->parent->freeItem(pimi);
	return FALSE;
}

static int FindParent(TMO_IntMenuItem* pimi, void* p)
{
	return pimi->next == p;
}

INT_PTR MO_RemoveMenuItem(WPARAM wParam, LPARAM)
{
	mir_cslock lck(csMenuHook);
	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)wParam);
	if (pimi == NULL)
		return -1;

	if (pimi->submenu.first) {
		MO_RecursiveWalkMenu(pimi->submenu.first, FreeMenuItem, NULL);
		pimi->submenu.first = NULL;
	}

	PMO_IntMenuItem prev = MO_RecursiveWalkMenu(pimi->owner->first, FindParent, pimi);
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

int KillMenuItems(PMO_IntMenuItem pimi, KillMenuItemsParam* param)
{
	if (pimi->hLangpack == param->hLangpack)
		param->arItems.insert(pimi);
	return FALSE;
}

void KillModuleMenus(int hLangpack)
{
	if (!bIsGenMenuInited)
		return;

	KillMenuItemsParam param(hLangpack);

	mir_cslock lck(csMenuHook);
	for (int i=0; i < g_menus.getCount(); i++)
		MO_RecursiveWalkMenu(g_menus[i]->m_items.first, (pfnWalkFunc)KillMenuItems, &param);

	for (int k=0; k < param.arItems.getCount(); k++)
		MO_RemoveMenuItem((WPARAM)param.arItems[k], 0);
}

///////////////////////////////////////////////////////////////////////////////
// we presume that this function is being called inside csMenuHook only

static int PackMenuItems(PMO_IntMenuItem pimi, void*)
{
	pimi->iCommand = NextObjectMenuItemId++;
	return FALSE;
}

static int GetNextObjectMenuItemId()
{
	// if menu commands are exausted, pack the menu array
	if (NextObjectMenuItemId >= CLISTMENUIDMAX) {
		NextObjectMenuItemId = CLISTMENUIDMIN;
		for (int i=0; i < g_menus.getCount(); i++)
			MO_RecursiveWalkMenu(g_menus[i]->m_items.first, PackMenuItems, NULL);
	}

	return NextObjectMenuItemId++;
}

//wparam = MenuObjectHandle
//lparam = PMO_MenuItem
//return MenuItemHandle
PMO_IntMenuItem MO_AddNewMenuItem(HANDLE menuobjecthandle, PMO_MenuItem pmi)
{
	if (!bIsGenMenuInited || pmi == NULL || pmi->cbSize != sizeof(TMO_MenuItem))
		return NULL;

	//old mode
	if (!(pmi->flags & CMIF_ROOTHANDLE))
		return MO_AddOldNewMenuItem(menuobjecthandle, pmi);

	mir_cslock lck(csMenuHook);
	int objidx = GetMenuObjbyId((int)menuobjecthandle);
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
		p->mi.ptszName = mir_tstrdup(pmi->ptszName);
	else
		p->mi.ptszName = mir_a2u(pmi->pszName);

	if (pmi->hIcon != NULL && !bIconsDisabled) {
		HANDLE hIcolibItem = IcoLib_IsManaged(pmi->hIcon);
		if (hIcolibItem != NULL) {
			HICON hIcon = IcoLib_GetIconByHandle(hIcolibItem, false);
			p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, hIcon);
			p->hIcolibItem = hIcolibItem;
			IcoLib_ReleaseIcon(hIcon, 0);
		}
		else p->iconId = ImageList_AddIcon(pmo->m_hMenuIcons, pmi->hIcon);
	}

	if (p->mi.root == HGENMENU_ROOT)
		p->mi.root = NULL;

	PMO_IntMenuItem pRoot = (p->mi.root != NULL) ? MO_GetIntMenuItem(p->mi.root) : NULL;
	if (pRoot)
		p->owner = &pRoot->submenu;
	else
		p->owner = &pmo->m_items;

	if (!p->owner->first)
		p->owner->first = p;
	if (p->owner->last)
		p->owner->last->next = p;
	p->owner->last = p;
	return p;
}

//wparam = MenuObjectHandle
//lparam = PMO_MenuItem

int FindRoot(PMO_IntMenuItem pimi, void* param)
{
	if (pimi->mi.pszName != NULL)
		if (pimi->submenu.first && !_tcscmp(pimi->mi.ptszName, (TCHAR*)param))
			return TRUE;

	return FALSE;
}

PMO_IntMenuItem MO_AddOldNewMenuItem(HANDLE menuobjecthandle, PMO_MenuItem pmi)
{
	if (!bIsGenMenuInited || pmi == NULL)
		return NULL;

	int objidx = GetMenuObjbyId((int)menuobjecthandle);
	if (objidx == -1)
		return NULL;

	if (pmi->cbSize != sizeof(TMO_MenuItem))
		return NULL;

	if (pmi->flags & CMIF_ROOTHANDLE)
		return NULL;

	//is item with popup or not
	if (pmi->root == 0) {
		//yes, this without popup
		pmi->root = NULL; //first level
	}
	else { // no, search for needed root and create it if need
		TCHAR* tszRoot;
		if (pmi->flags & CMIF_UNICODE)
			tszRoot = mir_tstrdup((TCHAR*)pmi->root);
		else
			tszRoot = mir_a2t((char*)pmi->root);

		PMO_IntMenuItem oldroot = MO_RecursiveWalkMenu(g_menus[objidx]->m_items.first, FindRoot, tszRoot);
		mir_free(tszRoot);

		if (oldroot == NULL) {
			//not found, creating root
			TMO_MenuItem tmi = *pmi;
			tmi.flags |= CMIF_ROOTHANDLE;
			tmi.ownerdata = 0;
			tmi.root = NULL;
			//copy pszPopupName
			tmi.ptszName = (TCHAR*)pmi->root;
			if ((oldroot = MO_AddNewMenuItem(menuobjecthandle, &tmi)) != NULL)
				MO_SetOptionsMenuItem(oldroot, OPT_MENUITEMSETUNIQNAME, (INT_PTR)pmi->root);
		}
		pmi->root = oldroot;

		//popup will be created in next commands
	}
	pmi->flags |= CMIF_ROOTHANDLE;
	//add popup(root allready exists)
	return MO_AddNewMenuItem(menuobjecthandle, pmi);
}

static int WhereToPlace(HMENU hMenu, PMO_MenuItem mi)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_SUBMENU | MIIM_DATA;
	for (int i = GetMenuItemCount(hMenu)-1; i >= 0; i--) {
		GetMenuItemInfo(hMenu, i, TRUE, &mii);
		if (mii.fType != MFT_SEPARATOR) {
			PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
			if (pimi != NULL)
				if (pimi->mi.position <= mi->position)
					return i+1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD GetMenuItemType(HMENU hMenu, int uItem)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_TYPE;
	GetMenuItemInfo(hMenu, uItem, TRUE, &mii);
	return mii.fType;
}

static UINT GetMenuItemTypeData(HMENU hMenu, int uItem, PMO_IntMenuItem& p)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_DATA | MIIM_TYPE;
	GetMenuItemInfo(hMenu, uItem, TRUE, &mii);
	p = MO_GetIntMenuItem((HGENMENU)mii.dwItemData);
	return mii.fType;
}

static void InsertSeparator(HMENU hMenu, int uItem)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	InsertMenuItem(hMenu, uItem, TRUE, &mii);
}

static void InsertMenuItemWithSeparators(HMENU hMenu, int uItem, MENUITEMINFO *lpmii)
{
	PMO_IntMenuItem pimi = MO_GetIntMenuItem((HGENMENU)lpmii->dwItemData), p;
	if (pimi == NULL)
		return;

	//check for separator before
	if (uItem) {
		UINT fType = GetMenuItemTypeData(hMenu, uItem-1, p);
		if (p != NULL && fType != MFT_SEPARATOR) {
			if ((p->mi.position / SEPARATORPOSITIONINTERVAL) != (pimi->mi.position / SEPARATORPOSITIONINTERVAL)) {
				//but might be supposed to be after the next one instead
				if (!(uItem < GetMenuItemCount(hMenu) && GetMenuItemType(hMenu, uItem) == MFT_SEPARATOR))
					InsertSeparator(hMenu, uItem);
				uItem++;
			}
		}
	}

	//check for separator after
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
// wparam started hMenu
// lparam ListParam*
// result hMenu

INT_PTR MO_BuildMenu(WPARAM wParam, LPARAM lParam)
{
	if (!bIsGenMenuInited)
		return -1;

	mir_cslock lck(csMenuHook);

	ListParam *lp = (ListParam*)lParam;
	int pimoidx = GetMenuObjbyId((int)lp->MenuObjectHandle);
	if (pimoidx == -1)
		return 0;

	#if defined(_DEBUG)
		// DumpMenuItem(g_menus[pimoidx]->m_items.first);
	#endif

	return (INT_PTR)BuildRecursiveMenu((HMENU)wParam, g_menus[pimoidx]->m_items.first, (ListParam*)lParam);
}

#ifdef _DEBUG
#define PUTPOSITIONSONMENU
#endif

void GetMenuItemName(PMO_IntMenuItem pMenuItem, char* pszDest, size_t cbDestSize)
{
	if (pMenuItem->UniqName)
		mir_snprintf(pszDest, cbDestSize, "{%s}", pMenuItem->UniqName);
	else if (pMenuItem->mi.flags & CMIF_UNICODE)
		mir_snprintf(pszDest, cbDestSize, "{%s}", (char*)_T2A(pMenuItem->mi.ptszName));
	else
		mir_snprintf(pszDest, cbDestSize, "{%s}", pMenuItem->mi.pszName);
}

HMENU BuildRecursiveMenu(HMENU hMenu, PMO_IntMenuItem pRootMenu, ListParam *param)
{
	if (param == NULL || pRootMenu == NULL)
		return NULL;

	TIntMenuObject* pmo = pRootMenu->parent;

	int rootlevel = (param->rootlevel == -1) ? 0 : param->rootlevel;

	ListParam localparam = *param;

	while (rootlevel == 0 && GetMenuItemCount(hMenu) > 0)
		DeleteMenu(hMenu, 0, MF_BYPOSITION);

	for (PMO_IntMenuItem pmi = pRootMenu; pmi != NULL; pmi = pmi->next) {
		PMO_MenuItem mi = &pmi->mi;
		if (mi->cbSize != sizeof(TMO_MenuItem))
			continue;

		if (mi->flags & CMIF_HIDDEN)
			continue;

		if (pmo->CheckService != NULL) {
			TCheckProcParam CheckParam;
			CheckParam.lParam = param->lParam;
			CheckParam.wParam = param->wParam;
			CheckParam.MenuItemOwnerData = mi->ownerdata;
			CheckParam.MenuItemHandle = pmi;
			if (CallService(pmo->CheckService, (WPARAM)&CheckParam, 0) == FALSE)
				continue;
		}

		/**************************************/
		if (rootlevel == 0 && mi->root == NULL && pmo->m_bUseUserDefinedItems) {
			char DBString[256];
			DBVARIANT dbv = { 0 };
			int pos;
			char MenuNameItems[256];
			mir_snprintf(MenuNameItems, SIZEOF(MenuNameItems), "%s_Items", pmo->pszName);

			char menuItemName[256];
			GetMenuItemName(pmi, menuItemName, sizeof(menuItemName));

			// check if it visible
			mir_snprintf(DBString, SIZEOF(DBString), "%s_visible", menuItemName);
			if (db_get_b(NULL, MenuNameItems, DBString, -1) == -1)
				db_set_b(NULL, MenuNameItems, DBString, 1);

			pmi->OverrideShow = TRUE;
			if (!db_get_b(NULL, MenuNameItems, DBString, 1)) {
				pmi->OverrideShow = FALSE;
				continue;  // find out what value to return if not getting added
			}

			// mi.pszName
			mir_snprintf(DBString, SIZEOF(DBString), "%s_name", menuItemName);
			if (!db_get_ts(NULL, MenuNameItems, DBString, &dbv)) {
				if (_tcslen(dbv.ptszVal) > 0)
					replaceStrT(pmi->CustomName, dbv.ptszVal);
				db_free(&dbv);
			}

			mir_snprintf(DBString, SIZEOF(DBString), "%s_pos", menuItemName);
			if ((pos = db_get_dw(NULL, MenuNameItems, DBString, -1)) == -1) {
				db_set_dw(NULL, MenuNameItems, DBString, mi->position);
				if (pmi->submenu.first)
					mi->position = 0;
			}
			else mi->position = pos;
		}

		/**************************************/

		if (rootlevel != (int)pmi->mi.root)
			continue;

		int i = WhereToPlace(hMenu, mi);

		MENUITEMINFO mii = { sizeof(mii) };
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

		mii.dwTypeData = (pmi->CustomName) ? pmi->CustomName : mi->ptszName;

		// it's a submenu
		if (pmi->submenu.first) {
			mii.fMask |= MIIM_SUBMENU;
			mii.hSubMenu = CreatePopupMenu();

			#ifdef PUTPOSITIONSONMENU
				if (GetKeyState(VK_CONTROL) & 0x8000) {
					TCHAR str[256];
					mir_sntprintf(str, SIZEOF(str), _T("%s (%d, id %x)"), mi->pszName, mi->position, mii.dwItemData);
					mii.dwTypeData = str;
				}
			#endif

			InsertMenuItemWithSeparators(hMenu, i, &mii);
			localparam.rootlevel = LPARAM(pmi);
			BuildRecursiveMenu(mii.hSubMenu, pmi->submenu.first, &localparam);
		}
		else {
			mii.wID = pmi->iCommand;

			#ifdef PUTPOSITIONSONMENU
				if (GetKeyState(VK_CONTROL) & 0x8000) {
					TCHAR str[256];
					mir_sntprintf(str, SIZEOF(str), _T("%s (%d, id %x)"), mi->pszName, mi->position, mii.dwItemData);
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
// iconlib in menu

static int MO_ReloadIcon(PMO_IntMenuItem pmi, void*)
{
	if (pmi->hIcolibItem) {
		HICON newIcon = IcoLib_GetIconByHandle(pmi->hIcolibItem, false);
		if (newIcon)
			ImageList_ReplaceIcon(pmi->parent->m_hMenuIcons, pmi->iconId, newIcon);

		IcoLib_ReleaseIcon(newIcon, 0);
	}

	return FALSE;
}

int OnIconLibChanges(WPARAM, LPARAM)
{
	{
		mir_cslock lck(csMenuHook);
		for (int mo=0; mo < g_menus.getCount(); mo++)
			if ((int)hStatusMenuObject != g_menus[mo]->id) //skip status menu
				MO_RecursiveWalkMenu(g_menus[mo]->m_items.first, MO_ReloadIcon, 0);
	}

	cli.pfnReloadProtoMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//

static int MO_RegisterIcon(PMO_IntMenuItem pmi, void*)
{
	TCHAR *uname = (pmi->UniqName) ? mir_a2t(pmi->UniqName) : mir_tstrdup(pmi->CustomName),
		*descr = GetMenuItemText(pmi);

	if (!uname && !descr)
		return FALSE;

	if (!pmi->hIcolibItem) {
		HICON hIcon = ImageList_GetIcon(pmi->parent->m_hMenuIcons, pmi->iconId, 0);

		TCHAR sectionName[256];
		mir_sntprintf(sectionName, SIZEOF(sectionName), LPGENT("Menu icons") _T("/%s"), TranslateTS(pmi->parent->ptszDisplayName));

		char iconame[256];
		mir_snprintf(iconame, SIZEOF(iconame), "genmenu_%s_%s", pmi->parent->pszName, uname && *uname ? uname : descr);

		// remove '&'
		if (descr) {
			descr = NEWTSTR_ALLOCA(descr);

			for (TCHAR *p = descr; *p; p++) {
				if ((p = _tcschr(p, '&')) == NULL)
					break;

				memmove(p, p+1, sizeof(TCHAR)*(_tcslen(p+1)+1));
				if (*p == '\0')
					p++;
			}
		}

		SKINICONDESC sid = { sizeof(sid) };
		sid.flags = SIDF_TCHAR;
		sid.ptszSection = sectionName;
		sid.pszName = iconame;
		sid.ptszDescription = descr;
		sid.hDefaultIcon = hIcon;
		pmi->hIcolibItem = IcoLib_AddNewIcon(0, &sid);

		Safe_DestroyIcon(hIcon);
		if (hIcon = Skin_GetIcon(iconame)) {
			ImageList_ReplaceIcon(pmi->parent->m_hMenuIcons, pmi->iconId, hIcon);
			IcoLib_ReleaseIcon(hIcon, 0);
		}
	}

	mir_free(uname);
	return FALSE;
}

int RegisterAllIconsInIconLib()
{
	//register all icons
	for (int mo=0; mo < g_menus.getCount(); mo++) {
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

		PMO_IntMenuItem pimi = (PMO_IntMenuItem)MO_GetDefaultMenuItem((WPARAM)g_menus[ iMenuID ]->m_items.first, 0);
		if (pimi != NULL) {
			MO_ProcessCommand(pimi, hContact);
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
	posttimerid = SetTimer((HWND)NULL, 0, 5, (TIMERPROC)PostRegisterIcons);
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconLibChanges);
	return 0;
}

static INT_PTR SRVMO_SetOptionsMenuObject(WPARAM, LPARAM lParam)
{
	lpOptParam lpop = (lpOptParam)lParam;
	if (lpop == NULL)
		return 0;

	return MO_SetOptionsMenuObject(lpop->Handle, lpop->Setting, lpop->Value);
}

static INT_PTR SRVMO_SetOptionsMenuItem(WPARAM, LPARAM lParam)
{
	lpOptParam lpop = (lpOptParam)lParam;
	if (lpop == NULL)
		return 0;

	return MO_SetOptionsMenuItem((PMO_IntMenuItem)lpop->Handle, lpop->Setting, lpop->Value);
}

int InitGenMenu()
{
	CreateServiceFunction(MO_BUILDMENU, MO_BuildMenu);

	CreateServiceFunction(MO_PROCESSCOMMAND, (MIRANDASERVICE)MO_ProcessCommand);
	CreateServiceFunction("MO/CreateNewMenuObject", MO_CreateNewMenuObject);
	CreateServiceFunction(MO_REMOVEMENUITEM, MO_RemoveMenuItem);
	CreateServiceFunction(MO_ADDNEWMENUITEM, (MIRANDASERVICE)MO_AddNewMenuItem);
	CreateServiceFunction(MO_MENUITEMGETOWNERDATA, MO_MenuItemGetOwnerData);
	CreateServiceFunction(MO_MODIFYMENUITEM, (MIRANDASERVICE)MO_ModifyMenuItem);
	CreateServiceFunction(MO_GETMENUITEM, MO_GetMenuItem);
	CreateServiceFunction(MO_GETDEFAULTMENUITEM, MO_GetDefaultMenuItem);
	CreateServiceFunction(MO_PROCESSCOMMANDBYMENUIDENT, MO_ProcessCommandByMenuIdent);
	CreateServiceFunction(MO_PROCESSHOTKEYS, (MIRANDASERVICE)MO_ProcessHotKeys);
	CreateServiceFunction(MO_REMOVEMENUOBJECT, MO_RemoveMenuObject);
	CreateServiceFunction(MO_GETPROTOROOTMENU, MO_GetProtoRootMenu);

	CreateServiceFunction(MO_SRV_SETOPTIONSMENUOBJECT, SRVMO_SetOptionsMenuObject);
	CreateServiceFunction(MO_SETOPTIONSMENUITEM, SRVMO_SetOptionsMenuItem);

	bIconsDisabled = db_get_b(NULL, "CList", "DisableMenuIcons", 0) != 0;

	bIsGenMenuInited = true;

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE,       GenMenuOptInit);
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
	FreeAndNil((void**)&p->mi.pszName);
	FreeAndNil((void**)&p->UniqName);
	FreeAndNil((void**)&p->CustomName);
	if (p->hBmp) DeleteObject(p->hBmp);
	mir_free(p);
}
