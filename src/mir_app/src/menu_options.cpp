/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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

MIR_APP_DLL(void) Menu_SetVisible(TMO_IntMenuItem *pimi, bool bVisible)
{
	if ((pimi = MO_GetIntMenuItem(pimi)) == nullptr)
		return;

	char szModule[256], menuItemName[256];
	mir_snprintf(szModule, "%s_Items", pimi->parent->pszName);
	bin2hex(&pimi->mi.uid, sizeof(pimi->mi.uid), menuItemName);

	ptrW wszValue(db_get_wsa(0, szModule, menuItemName, L"1;;;"));
	wszValue[0] = bVisible ? '1' : '0';
	db_set_ws(0, szModule, menuItemName, wszValue);

	Menu_ShowItem(pimi, bVisible);
}

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
	LIST<TMO_IntMenuItem> m_arDeleted;

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
			auto *iod = (MenuItemOptData*)tvi.lParam;
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

		for (auto &it : arItems) {
			if (it != arItems[0] && it->pos - lastpos >= SEPARATORPOSITIONINTERVAL) {
				MenuItemOptData *sep = new MenuItemOptData();
				sep->id = -1;
				sep->name = mir_wstrdup(STR_SEPARATOR);
				sep->pos = it->pos - 1;

				tvis.item.lParam = (LPARAM)sep;
				tvis.item.pszText = sep->name;
				tvis.item.iImage = tvis.item.iSelectedImage = 1;
				tvis.item.cChildren = 0;
				m_menuItems.InsertItem(&tvis);
			}

			tvis.item.lParam = (LPARAM)it;
			tvis.item.pszText = it->name;
			tvis.item.iImage = tvis.item.iSelectedImage = it->bShow;
			tvis.item.cChildren = it->pimi->submenu.first != nullptr;

			HTREEITEM hti = m_menuItems.InsertItem(&tvis);
			if (bIsFirst) {
				if (hRoot == nullptr)
					m_menuItems.SelectItem(hti);
				bIsFirst = false;
			}

			if (it->pimi->submenu.first != nullptr) {
				BuildTreeInternal(pszModule, bReread, it->pimi->submenu.first, hti);
				m_menuItems.Expand(hti, TVE_EXPAND);
			}

			lastpos = it->pos;
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

		m_menuItems.SetDraw(false);
		m_menuItems.DeleteAllItems();

		BuildTreeInternal(szModule, bReread, pmo->m_items.first, nullptr);

		m_menuItems.SetDraw(true);

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
	CCtrlEdit m_customName, m_service, m_module;
	CCtrlButton m_btnInsSeparator, m_btnInsMenu, m_btnReset, m_btnSet, m_btnDefault, m_btnDelete;
	CCtrlBase m_warning;

public:
	CGenMenuOptionsPage() :
		CDlgBase(g_plugin, IDD_OPT_GENMENU),
		m_arDeleted(1),
		m_menuItems(this, IDC_MENUITEMS),
		m_menuObjects(this, IDC_MENUOBJECTS),
		m_radio1(this, IDC_RADIO1),
		m_radio2(this, IDC_RADIO2),
		m_enableIcons(this, IDC_DISABLEMENUICONS),
		m_btnInsSeparator(this, IDC_INSERTSEPARATOR),
		m_btnInsMenu(this, IDC_INSERTSUBMENU),
		m_btnReset(this, IDC_RESETMENU),
		m_btnSet(this, IDC_GENMENU_SET),
		m_btnDelete(this, IDC_GENMENU_DELETE),
		m_btnDefault(this, IDC_GENMENU_DEFAULT),
		m_customName(this, IDC_GENMENU_CUSTOMNAME),
		m_service(this, IDC_GENMENU_SERVICE),
		m_module(this, IDC_GENMENU_MODULE),
		m_warning(this, IDC_NOTSUPPORTWARNING)
	{
		m_btnSet.OnClick = Callback(this, &CGenMenuOptionsPage::btnSet_Clicked);
		m_btnReset.OnClick = Callback(this, &CGenMenuOptionsPage::btnReset_Clicked);
		m_btnInsSeparator.OnClick = Callback(this, &CGenMenuOptionsPage::btnInsSep_Clicked);
		m_btnInsMenu.OnClick = Callback(this, &CGenMenuOptionsPage::btnInsMenu_Clicked);
		m_btnDefault.OnClick = Callback(this, &CGenMenuOptionsPage::btnDefault_Clicked);
		m_btnDelete.OnClick = Callback(this, &CGenMenuOptionsPage::btnDelete_Clicked);

		m_menuObjects.OnSelChange = Callback(this, &CGenMenuOptionsPage::onMenuObjectChanged);

		m_menuItems.SetFlags(MTREE_CHECKBOX | MTREE_DND);
		m_menuItems.OnSelChanged = Callback(this, &CGenMenuOptionsPage::onMenuItemChanged);
		m_menuItems.OnBeginDrag = Callback(this, &CGenMenuOptionsPage::onMenuItemBeginDrag);

		m_customName.SetSilent();
		m_service.SetSilent();
		m_module.SetSilent();
	}

	//---- init dialog -------------------------------------------
	bool OnInitDialog() override
	{
		iInitMenuValue = db_get_b(0, "CList", "MoveProtoMenus", TRUE);
		
		if (iInitMenuValue)
			m_radio2.SetState(true);
		else
			m_radio1.SetState(true);

		m_enableIcons.SetState(!bIconsDisabled);

		//---- init menu object list --------------------------------------
		for (auto &p : g_menus)
			if (p->id != (int)hStatusMenuObject && p->m_bUseUserDefinedItems)
				m_menuObjects.AddString(TranslateW(p->ptszDisplayName), p->id);
		
		m_menuObjects.SetCurSel(0);
		RebuildCurrent();
		return true;
	}

	bool OnApply() override
	{
		bIconsDisabled = m_enableIcons.GetState() == 0;
		db_set_b(0, "CList", "DisableMenuIcons", bIconsDisabled);
		SaveTree();

		for (auto &pimi : m_arDeleted)
			Menu_RemoveItem(pimi);

		int iNewMenuValue = !m_radio1.GetState();
		if (iNewMenuValue != iInitMenuValue) {
			db_set_b(0, "CList", "MoveProtoMenus", iNewMenuValue);

			RebuildProtoMenus();
			iInitMenuValue = iNewMenuValue;
		}
		RebuildCurrent();
		return true;
	}

	void OnDestroy() override
	{
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

		TVINSERTSTRUCT tvis = {};
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

		TVINSERTSTRUCT tvis = {};
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

	void btnDelete_Clicked(CCtrlButton *)
	{
		HTREEITEM hti = m_menuItems.GetSelection();
		if (hti == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_PARAM;
		tvi.hItem = hti;
		m_menuItems.GetItem(&tvi);

		MenuItemOptData *iod = (MenuItemOptData *)tvi.lParam;
		if (!(iod->pimi->mi.flags & CMIF_CUSTOM))
			return;

		if (IDYES == MessageBoxW(m_hwnd, TranslateT("Do you really want to delete this menu item?"), TranslateT("Miranda"), MB_YESNO | MB_ICONQUESTION)) {
			m_arDeleted.insert(iod->pimi);
			m_menuItems.DeleteItem(hti);
			delete iod;
			NotifyChange();
		}
	}

	void onMenuObjectChanged(void*)
	{
		m_bInitialized = false;
		RebuildCurrent();
		m_bInitialized = true;
	}

	void onMenuItemChanged(void*)
	{
		m_customName.SetTextA("");
		m_service.SetTextA("");
		m_module.SetTextA("");

		m_btnInsMenu.Disable();
		m_btnDefault.Disable();
		m_btnSet.Disable();
		m_btnDelete.Disable();
		m_customName.Disable();

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

		const CMPluginBase *pPlugin = iod->pimi->mi.pPlugin;
		m_module.SetTextA(pPlugin == nullptr ? "" : pPlugin->getInfo().shortName);

		m_btnInsMenu.Enable(iod->pimi->mi.root == nullptr);
		m_btnDefault.Enable(mir_wstrcmp(iod->name, iod->defname) != 0);
		m_btnDelete.Enable(iod->pimi->mi.flags & CMIF_CUSTOM);
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
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.szTitle.a = LPGEN("Menus");
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CGenMenuOptionsPage();
	g_plugin.addOptions(wParam, &odp);
	
	return ProtocolOrderOptInit(wParam, 0);
}
