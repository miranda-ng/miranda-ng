/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

#define STR_SEPARATOR L"-----------------------------------"

extern bool bIconsDisabled;
extern int DefaultImageListColorDepth;
void RebuildProtoMenus();

/////////////////////////////////////////////////////////////////////////////////////////

struct MenuItemOptData : public MZeroedObject
{
	~MenuItemOptData() {}

	int    pos;

	ptrW   name;
	ptrW   defname;
	
	bool   bShow;
	int    id;

	TMO_IntMenuItem *pimi;
};

static int SortMenuItems(const MenuItemOptData *p1, const MenuItemOptData *p2)
{
	if (p1->pos < p2->pos) return -1;
	if (p1->pos > p2->pos) return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CGenMenuOptionsPage : public CDlgBase
{
	int iInitMenuValue;

	wchar_t idstr[100];

	void SaveTreeInternal(MenuItemOptData *pParent, HTREEITEM hRootItem, const char *szModule)
	{
		TVITEMEX tvi;
		tvi.hItem = hRootItem;
		tvi.cchTextMax = _countof(idstr);
		tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE;
		tvi.pszText = idstr;

		int count = 0, customOrder = 0;
		int runtimepos = 100;

		char pszParent[33];
		if (pParent == nullptr)
			pszParent[0] = 0;
		else
			bin2hex(&pParent->pimi->mi.uid, sizeof(MUUID), pszParent);

		while (tvi.hItem != nullptr) {
			m_menuItems.GetItem(&tvi);
			MenuItemOptData *iod = (MenuItemOptData*)tvi.lParam;
			if (TMO_IntMenuItem *pimi = iod->pimi) {
				if (pimi->mi.uid != miid_last) {
					char menuItemName[256];
					bin2hex(&pimi->mi.uid, sizeof(pimi->mi.uid), menuItemName);

					int visible = tvi.iImage != 0;
					wchar_t *ptszCustomName;
					if (iod->name != nullptr && iod->defname != nullptr && mir_wstrcmp(iod->name, iod->defname) != 0)
						ptszCustomName = iod->name;
					else
						ptszCustomName = L"";

					CMStringW tszValue(FORMAT, L"%d;%d;%S;%s", visible, runtimepos, pszParent, ptszCustomName);
					db_set_ws(0, szModule, menuItemName, tszValue);

					if (pimi->mi.flags & CMIF_CUSTOM)
						db_set_s(0, szModule, CMStringA(FORMAT, "Custom%d", customOrder++), menuItemName);						
				}

				HTREEITEM hChild = m_menuItems.GetChild(tvi.hItem);
				if (hChild != nullptr)
					SaveTreeInternal(iod, hChild, szModule);

				runtimepos += 100;
			}

			if (iod->name && !mir_wstrcmp(iod->name, STR_SEPARATOR) && tvi.iImage)
				runtimepos += SEPARATORPOSITIONINTERVAL;

			tvi.hItem = m_menuItems.GetNextSibling(tvi.hItem);
			count++;
		}
	}

	void SaveTree()
	{
		int MenuObjectId;
		if (!GetCurrentMenuObjectID(MenuObjectId))
			return;

		TIntMenuObject *pmo = GetMenuObjbyId(MenuObjectId);
		if (pmo == nullptr)
			return;

		char szModule[256];
		mir_snprintf(szModule, "%s_Items", pmo->pszName);
		db_delete_module(0, szModule);
		SaveTreeInternal(nullptr, m_menuItems.GetRoot(), szModule);
		db_set_b(0, szModule, "MenuFormat", 1);
	}

	void FreeTreeData()
	{
		HTREEITEM hItem = m_menuItems.GetRoot();
		while (hItem != nullptr) {
			TVITEMEX tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = hItem;
			m_menuItems.GetItem(&tvi);
			delete (MenuItemOptData *)tvi.lParam;

			tvi.lParam = 0;
			m_menuItems.SetItem(&tvi);

			hItem = m_menuItems.GetNextSibling(hItem);
		}
	}

	void RebuildCurrent()
	{
		int MenuObjectID;
		if (GetCurrentMenuObjectID(MenuObjectID))
			BuildTree(MenuObjectID, true);
	}

	void BuildTreeInternal(const char *pszModule, bool bReread, TMO_IntMenuItem *pFirst, HTREEITEM hRoot)
	{
		LIST<MenuItemOptData> arItems(10, SortMenuItems);

		for (TMO_IntMenuItem *p = pFirst; p != nullptr; p = p->next) {
			// filter out items whose presence & position might not be changed
			if (p->mi.flags & CMIF_SYSTEM)
				continue;

			MenuItemOptData *PD = new MenuItemOptData();
			PD->pimi = p;
			PD->defname = mir_wstrdup(GetMenuItemText(p));
			PD->name = mir_wstrdup((bReread && p->ptszCustomName != nullptr) ? p->ptszCustomName : PD->defname);
			PD->bShow = (p->mi.flags & CMIF_HIDDEN) == 0;
			PD->pos = (bReread) ? p->mi.position : p->originalPosition;
			PD->id = p->iCommand;
			arItems.insert(PD);
		}

		int lastpos = 0;
		bool bIsFirst = TRUE;

		TVINSERTSTRUCT tvis;
		tvis.hParent = hRoot;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

		for (int i = 0; i < arItems.getCount(); i++) {
			MenuItemOptData *PD = arItems[i];
			if (i > 0 && PD->pos - lastpos >= SEPARATORPOSITIONINTERVAL) {
				MenuItemOptData *sep = new MenuItemOptData();
				sep->id = -1;
				sep->name = mir_wstrdup(STR_SEPARATOR);
				sep->pos = PD->pos - 1;

				tvis.item.lParam = (LPARAM)sep;
				tvis.item.pszText = sep->name;
				tvis.item.iImage = tvis.item.iSelectedImage = 1;
				tvis.item.cChildren = 0;
				m_menuItems.InsertItem(&tvis);
			}

			tvis.item.lParam = (LPARAM)PD;
			tvis.item.pszText = PD->name;
			tvis.item.iImage = tvis.item.iSelectedImage = PD->bShow;
			tvis.item.cChildren = PD->pimi->submenu.first != nullptr;

			HTREEITEM hti = m_menuItems.InsertItem(&tvis);
			if (bIsFirst) {
				if (hRoot == nullptr)
					m_menuItems.SelectItem(hti);
				bIsFirst = false;
			}

			if (PD->pimi->submenu.first != nullptr) {
				BuildTreeInternal(pszModule, bReread, PD->pimi->submenu.first, hti);
				m_menuItems.Expand(hti, TVE_EXPAND);
			}

			lastpos = PD->pos;
		}
	}

	bool BuildTree(int MenuObjectId, bool bReread)
	{
		FreeTreeData();

		TIntMenuObject *pmo = GetMenuObjbyId(MenuObjectId);
		if (pmo == nullptr || pmo->m_items.first == nullptr)
			return false;

		char szModule[256];
		mir_snprintf(szModule, "%s_Items", pmo->pszName);

		if (bReread) // no need to reread database on reset
			MO_RecursiveWalkMenu(pmo->m_items.first, Menu_LoadFromDatabase, szModule);

		m_menuItems.SendMsg(WM_SETREDRAW, FALSE, 0);
		m_menuItems.DeleteAllItems();

		BuildTreeInternal(szModule, bReread, pmo->m_items.first, nullptr);

		m_menuItems.SendMsg(WM_SETREDRAW, TRUE, 0);

		m_warning.Show(!pmo->m_bUseUserDefinedItems);
		m_menuItems.Enable(pmo->m_bUseUserDefinedItems);
		m_btnInsSeparator.Enable(pmo->m_bUseUserDefinedItems);
		m_btnInsMenu.Enable(pmo->m_bUseUserDefinedItems);
		return 1;
	}

	bool GetCurrentMenuObjectID(int &result)
	{
		int iItem = m_menuObjects.GetCurSel();
		if (iItem == -1)
			return false;

		result = (int)m_menuObjects.GetItemData(iItem);
		return true;
	}

	CCtrlListBox m_menuObjects;
	CCtrlTreeView m_menuItems;
	CCtrlCheck m_radio1, m_radio2, m_enableIcons;
	CCtrlEdit m_customName, m_service;
	CCtrlButton m_btnInsSeparator, m_btnInsMenu, m_btnReset, m_btnSet, m_btnDefault;
	CCtrlBase m_warning;

public:
	CGenMenuOptionsPage() :
		CDlgBase(g_hInst, IDD_OPT_GENMENU),
		m_menuItems(this, IDC_MENUITEMS),
		m_menuObjects(this, IDC_MENUOBJECTS),
		m_radio1(this, IDC_RADIO1),
		m_radio2(this, IDC_RADIO2),
		m_enableIcons(this, IDC_DISABLEMENUICONS),
		m_btnInsSeparator(this, IDC_INSERTSEPARATOR),
		m_btnInsMenu(this, IDC_INSERTSUBMENU),
		m_btnReset(this, IDC_RESETMENU),
		m_btnSet(this, IDC_GENMENU_SET),
		m_btnDefault(this, IDC_GENMENU_DEFAULT),
		m_customName(this, IDC_GENMENU_CUSTOMNAME),
		m_service(this, IDC_GENMENU_SERVICE),
		m_warning(this, IDC_NOTSUPPORTWARNING)
	{
		m_btnSet.OnClick = Callback(this, &CGenMenuOptionsPage::btnSet_Clicked);
		m_btnReset.OnClick = Callback(this, &CGenMenuOptionsPage::btnReset_Clicked);
		m_btnInsSeparator.OnClick = Callback(this, &CGenMenuOptionsPage::btnInsSep_Clicked);
		m_btnInsMenu.OnClick = Callback(this, &CGenMenuOptionsPage::btnInsMenu_Clicked);
		m_btnDefault.OnClick = Callback(this, &CGenMenuOptionsPage::btnDefault_Clicked);

		m_menuObjects.OnSelChange = Callback(this, &CGenMenuOptionsPage::onMenuObjectChanged);

		m_menuItems.SetFlags(MTREE_CHECKBOX | MTREE_DND /*| MTREE_MULTISELECT*/);
		m_menuItems.OnSelChanged = Callback(this, &CGenMenuOptionsPage::onMenuItemChanged);
		m_menuItems.OnBeginDrag = Callback(this, &CGenMenuOptionsPage::onMenuItemBeginDrag);

		m_customName.SetSilent();
		m_service.SetSilent();
	}

	//---- init dialog -------------------------------------------
	virtual void OnInitDialog()
	{
		iInitMenuValue = db_get_b(0, "CList", "MoveProtoMenus", TRUE);
		
		HIMAGELIST himlCheckBoxes = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 2, 2);
		ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_NOTICK);
		ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_TICK);
		m_menuItems.SetImageList(himlCheckBoxes, TVSIL_NORMAL);

		if (iInitMenuValue)
			m_radio2.SetState(true);
		else
			m_radio1.SetState(true);

		m_enableIcons.SetState(!bIconsDisabled);

		//---- init menu object list --------------------------------------
		for (int i = 0; i < g_menus.getCount(); i++) {
			TIntMenuObject *p = g_menus[i];
			if (p->id != (int)hStatusMenuObject && p->m_bUseUserDefinedItems)
				m_menuObjects.AddString(TranslateW(p->ptszDisplayName), p->id);
		}
		
		m_menuObjects.SetCurSel(0);
		RebuildCurrent();
	}

	virtual void OnApply()
	{
		bIconsDisabled = m_enableIcons.GetState() == 0;
		db_set_b(0, "CList", "DisableMenuIcons", bIconsDisabled);
		SaveTree();

		int iNewMenuValue = !m_radio1.GetState();
		if (iNewMenuValue != iInitMenuValue) {
			db_set_b(0, "CList", "MoveProtoMenus", iNewMenuValue);

			RebuildProtoMenus();
			iInitMenuValue = iNewMenuValue;
		}
		RebuildCurrent();
	}

	virtual void OnDestroy()
	{
		ImageList_Destroy(m_menuItems.GetImageList(TVSIL_NORMAL));
		FreeTreeData();
	}

	void btnInsSep_Clicked(CCtrlButton*)
	{
		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
		tvi.hItem = hti;
		if (!m_menuItems.GetItem(&tvi))
			return;

		MenuItemOptData *PD = new MenuItemOptData();
		PD->id = -1;
		PD->name = mir_wstrdup(STR_SEPARATOR);
		PD->pos = ((MenuItemOptData *)tvi.lParam)->pos - 1;

		TVINSERTSTRUCT tvis = { 0 };
		tvis.item.lParam = (LPARAM)PD;
		tvis.item.pszText = PD->name;
		tvis.item.iImage = tvis.item.iSelectedImage = 1;
		tvis.hInsertAfter = hti;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		m_menuItems.InsertItem(&tvis);

		NotifyChange();
	}

	void btnInsMenu_Clicked(CCtrlButton*)
	{
		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT;
		tvi.hItem = hti;
		if (!m_menuItems.GetItem(&tvi))
			return;

		MenuItemOptData *curData = (MenuItemOptData*)tvi.lParam;

		TMO_MenuItem mi = {};
		UuidCreate((UUID*)&mi.uid);
		mi.flags = CMIF_CUSTOM;
		mi.name.a = LPGEN("New submenu");
		mi.position = curData->pos - 1;
		TMO_IntMenuItem *pimi = Menu_AddItem(curData->pimi->parent->id, &mi, nullptr);

		MenuItemOptData *PD = new MenuItemOptData();
		PD->id = -1;
		PD->name = mir_wstrdup(pimi->mi.name.w);
		PD->pos = pimi->mi.position;
		PD->pimi = pimi;

		TVINSERTSTRUCT tvis = { 0 };
		tvis.item.lParam = (LPARAM)PD;
		tvis.item.pszText = PD->name;
		tvis.item.iImage = tvis.item.iSelectedImage = 1;
		tvis.hInsertAfter = hti;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		m_menuItems.InsertItem(&tvis);

		NotifyChange();
	}

	void btnReset_Clicked(CCtrlButton*)
	{
		int MenuObjectID;
		if (GetCurrentMenuObjectID(MenuObjectID)) {
			BuildTree(MenuObjectID, false);
			NotifyChange();
		}
	}

	void btnDefault_Clicked(CCtrlButton*)
	{
		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvi.hItem = hti;
		m_menuItems.GetItem(&tvi);

		MenuItemOptData *iod = (MenuItemOptData *)tvi.lParam;
		if (iod->name && wcsstr(iod->name, STR_SEPARATOR))
			return;

		iod->name = mir_wstrdup(iod->defname);
		m_customName.SetText(iod->defname);

		tvi.mask = TVIF_TEXT;
		tvi.pszText = iod->name;
		m_menuItems.SetItem(&tvi);
		NotifyChange();
	}

	void btnSet_Clicked(CCtrlButton*)
	{
		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvi.hItem = hti;
		m_menuItems.GetItem(&tvi);

		MenuItemOptData *iod = (MenuItemOptData *)tvi.lParam;
		if (iod->name && wcsstr(iod->name, STR_SEPARATOR))
			return;

		iod->name = m_customName.GetText();

		tvi.mask = TVIF_TEXT;
		tvi.pszText = iod->name;
		m_menuItems.SetItem(&tvi);
		NotifyChange();
	}

	void onMenuObjectChanged(void*)
	{
		m_initialized = false;
		RebuildCurrent();
		m_initialized = true;
	}

	void onMenuItemChanged(void*)
	{
		m_customName.SetTextA("");
		m_service.SetTextA("");

		m_btnInsMenu.Enable(false);
		m_btnDefault.Enable(false);
		m_btnSet.Enable(false);
		m_customName.Enable(false);

		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvi.hItem = hti;
		m_menuItems.GetItem(&tvi);
		if (tvi.lParam == 0)
			return;

		MenuItemOptData *iod = (MenuItemOptData *)tvi.lParam;
		if (iod->name && wcsstr(iod->name, STR_SEPARATOR))
			return;

		m_customName.SetText(iod->name);

		if (iod->pimi->mi.uid != miid_last) {
			char szText[100];
			bin2hex(&iod->pimi->mi.uid, sizeof(iod->pimi->mi.uid), szText);
			m_service.SetTextA(szText);
		}

		m_btnInsMenu.Enable(iod->pimi->mi.root == nullptr);
		m_btnDefault.Enable(mir_wstrcmp(iod->name, iod->defname) != 0);
		m_btnSet.Enable(true);
		m_customName.Enable(true);
	}

	void onMenuItemBeginDrag(CCtrlTreeView::TEventInfo *evt)
	{
		MenuItemOptData *p = (MenuItemOptData*)evt->nmtv->itemNew.lParam;
		if (p->pimi != nullptr)
			if (p->pimi->mi.flags & CMIF_UNMOVABLE)
				evt->nmhdr->code = 0; // reject an attempt to change item's position
	}
};

int GenMenuOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -1000000000;
	odp.szTitle.a = LPGEN("Menus");
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CGenMenuOptionsPage();
	Options_AddPage(wParam, &odp);
	
	return ProtocolOrderOptInit(wParam, 0);
}
