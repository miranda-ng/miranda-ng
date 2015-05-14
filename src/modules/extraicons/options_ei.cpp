/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-15 Miranda NG project

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"

#include "extraicons.h"

#define ICON_SIZE 				16

int SortFunc(const ExtraIcon *p1, const ExtraIcon *p2);

struct intlist
{
	intlist() : count(0), data(0) {}
	~intlist() { mir_free(data); }

	void add(int val)
	{
		data = (int*)mir_realloc(data, sizeof(int)*(count + 1));
		data[count++] = val;
	}

	int count;
	int *data;
};

static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	intlist *a = (intlist*)lParam1;
	intlist *b = (intlist*)lParam2;
	return SortFunc(registeredExtraIcons[a->data[0] - 1], registeredExtraIcons[b->data[0] - 1]);
}

// Functions //////////////////////////////////////////////////////////////////////////////////////

BOOL ScreenToClient(HWND hWnd, LPRECT lpRect)
{
	POINT pt;
	pt.x = lpRect->left;
	pt.y = lpRect->top;

	BOOL ret = ScreenToClient(hWnd, &pt);
	if (!ret)
		return ret;

	lpRect->left = pt.x;
	lpRect->top = pt.y;

	pt.x = lpRect->right;
	pt.y = lpRect->bottom;

	ret = ScreenToClient(hWnd, &pt);

	lpRect->right = pt.x;
	lpRect->bottom = pt.y;

	return ret;
}

static void RemoveExtraIcons(int slot)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		Clist_SetExtraIcon(hContact, slot, INVALID_HANDLE_VALUE);
}

class CExtraIconOptsDlg : public CDlgBase
{
	intlist* Tree_GetIDs(HTREEITEM hItem)
	{
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		m_tree.GetItem(&tvi);
		return (intlist*)tvi.lParam;
	}

	HTREEITEM Tree_AddExtraIcon(BaseExtraIcon *extra, bool selected, HTREEITEM hAfter = TVI_LAST)
	{
		intlist *ids = new intlist();
		ids->add(extra->getID());

		TVINSERTSTRUCT tvis = { 0 };
		tvis.hInsertAfter = hAfter;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.iSelectedImage = tvis.item.iImage = extra->getID();
		tvis.item.lParam = (LPARAM)ids;
		tvis.item.pszText = (LPTSTR)extra->getDescription();
		tvis.item.state = INDEXTOSTATEIMAGEMASK(selected ? 2 : 1);
		return m_tree.InsertItem(&tvis);
	}

	HTREEITEM Tree_AddExtraIconGroup(intlist &group, bool selected, HTREEITEM hAfter = TVI_LAST)
	{
		intlist *ids = new intlist();
		CMString desc;
		int img = 0;
		for (int i = 0; i < group.count; i++) {
			BaseExtraIcon *extra = registeredExtraIcons[group.data[i] - 1];
			ids->add(extra->getID());

			if (img == 0 && !IsEmpty(extra->getDescIcon()))
				img = extra->getID();

			if (i > 0)
				desc += _T(" / ");
			desc += extra->getDescription();
		}

		TVINSERTSTRUCT tvis = { 0 };
		tvis.hInsertAfter = hAfter;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		tvis.item.iSelectedImage = tvis.item.iImage = img;
		tvis.item.lParam = (LPARAM)ids;
		tvis.item.pszText = (TCHAR*)desc.c_str();
		tvis.item.state = INDEXTOSTATEIMAGEMASK(selected ? 2 : 1);
		return m_tree.InsertItem(&tvis);
	}

	void GroupSelectedItems()
	{
		LIST<_TREEITEM> toRemove(1);
		intlist ids;
		bool selected = false;
		HTREEITEM hPlace = NULL;

		// Find items
		HTREEITEM hItem = m_tree.GetRoot();
		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
		while (hItem) {
			if (m_tree.IsSelected(hItem)) {
				if (hPlace == NULL)
					hPlace = hItem;

				tvi.hItem = hItem;
				m_tree.GetItem(&tvi);

				intlist *iids = (intlist*)tvi.lParam;
				for (int i = 0; i < iids->count; i++)
					ids.add(iids->data[i]);

				if ((tvi.state & INDEXTOSTATEIMAGEMASK(3)) == INDEXTOSTATEIMAGEMASK(2))
					selected = true;

				toRemove.insert(hItem);
			}

			hItem = m_tree.GetNextSibling(hItem);
		}

		if (hPlace != NULL) {
			// Add new
			HTREEITEM hNew = Tree_AddExtraIconGroup(ids, selected, hPlace);

			// Remove old
			for (int i = 0; i < toRemove.getCount(); i++) {
				delete Tree_GetIDs(toRemove[i]);
				m_tree.DeleteItem(toRemove[i]);
			}

			// Select
			m_tree.UnselectAll();
			m_tree.SelectItem(hNew);
		}
	}

	void UngroupSelectedItems()
	{
		HTREEITEM hItem = m_tree.GetSelection();
		if (hItem == NULL)
			return;

		intlist *ids = Tree_GetIDs(hItem);
		if (ids->count < 2)
			return;

		bool selected = m_tree.IsSelected(hItem);

		for (int i = ids->count - 1; i >= 0; i--) {
			BaseExtraIcon *extra = registeredExtraIcons[ids->data[i] - 1];
			Tree_AddExtraIcon(extra, selected, hItem);
		}

		delete ids;
		m_tree.DeleteItem(hItem);

		m_tree.UnselectAll();
	}

	int ShowPopup(int popup)
	{
		// Fix selection
		HTREEITEM hSelected = m_tree.GetDropHilight();
		HTREEITEM hItem = m_tree.GetRoot();
		while (hItem) {
			if (hItem != hSelected && m_tree.IsSelected(hItem))
				m_tree.DropHilite(hItem);

			hItem = m_tree.GetNextSibling(hItem);
		}

		HMENU menu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_OPT_POPUP));
		HMENU submenu = GetSubMenu(menu, popup);
		TranslateMenu(submenu);

		DWORD pos = GetMessagePos();
		int ret = TrackPopupMenu(submenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_LEFTALIGN, LOWORD(pos), HIWORD(pos), 0, m_hwnd, NULL);

		DestroyMenu(menu);

		// Revert selection
		hItem = m_tree.GetRoot();
		while (hItem) {
			if (hItem != hSelected && m_tree.IsSelected(hItem))
				m_tree.DropUnhilite(hItem);
			hItem = m_tree.GetNextSibling(hItem);
		}

		return ret;
	}

	CCtrlTreeView m_tree;

public:
	CExtraIconOptsDlg() :
		CDlgBase(hInst, IDD_EI_OPTIONS),
		m_tree(this, IDC_EXTRAORDER)
	{
		m_tree.SetFlags(MTREE_CHECKBOX | MTREE_DND | MTREE_MULTISELECT);
	}

	virtual void OnInitDialog()
	{
		int numSlots = GetNumberOfSlots();
		if (numSlots < (int)registeredExtraIcons.getCount()) {
			HWND label = GetDlgItem(m_hwnd, IDC_MAX_ICONS_L);
			SetWindowText(label, CMString(FORMAT, TranslateT("*only the first %d icons will be shown"), numSlots));
			ShowWindow(label, SW_SHOW);
		}

		int cx = GetSystemMetrics(SM_CXSMICON);
		HIMAGELIST hImageList = ImageList_Create(cx, cx, ILC_COLOR32 | ILC_MASK, 2, 2);

		HICON hBlankIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, cx, cx, 0);
		ImageList_AddIcon(hImageList, hBlankIcon);

		for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
			ExtraIcon *extra = registeredExtraIcons[i];

			HICON hIcon = Skin_GetIcon(extra->getDescIcon());
			if (hIcon == NULL)
				ImageList_AddIcon(hImageList, hBlankIcon);
			else {
				ImageList_AddIcon(hImageList, hIcon);
				Skin_ReleaseIcon(hIcon);
			}
		}
		m_tree.SetImageList(hImageList, TVSIL_NORMAL);
		DestroyIcon(hBlankIcon);

		for (int k = 0; k < extraIconsBySlot.getCount(); k++) {
			ExtraIcon *extra = extraIconsBySlot[k];

			if (extra->getType() == EXTRAICON_TYPE_GROUP) {
				ExtraIconGroup *group = (ExtraIconGroup *)extra;
				intlist ids;
				for (int j = 0; j < group->items.getCount(); j++)
					ids.add(group->items[j]->getID());
				Tree_AddExtraIconGroup(ids, extra->isEnabled());
			}
			else Tree_AddExtraIcon((BaseExtraIcon *)extra, extra->isEnabled());
		}

		TVSORTCB sort = { 0 };
		sort.hParent = NULL;
		sort.lParam = 0;
		sort.lpfnCompare = CompareFunc;
		m_tree.SortChildrenCB(&sort, 0);
	}

	virtual void OnApply()
	{
		// Store old slots
		int *oldSlots = new int[registeredExtraIcons.getCount()];
		int lastUsedSlot = -1;
		for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
			if (extraIconsByHandle[i] == registeredExtraIcons[i])
				oldSlots[i] = registeredExtraIcons[i]->getSlot();
			else
				// Remove old slot for groups to re-set images
				oldSlots[i] = -1;
			lastUsedSlot = MAX(lastUsedSlot, registeredExtraIcons[i]->getSlot());
		}
		lastUsedSlot = MIN(lastUsedSlot, GetNumberOfSlots());

		// Get user data and create new groups
		LIST<ExtraIconGroup> groups(1);

		BYTE pos = 0;
		int firstEmptySlot = 0;
		HTREEITEM ht = m_tree.GetRoot();
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		while (ht) {
			tvi.hItem = ht;
			m_tree.GetItem(&tvi);

			intlist*ids = (intlist*)tvi.lParam;
			if (ids == NULL || ids->count < 1)
				continue; // ???

			bool enabled = ((tvi.state & INDEXTOSTATEIMAGEMASK(3)) == INDEXTOSTATEIMAGEMASK(2));
			int slot = (enabled ? firstEmptySlot++ : -1);
			if (slot >= GetNumberOfSlots())
				slot = -1;

			if (ids->count == 1) {
				BaseExtraIcon *extra = registeredExtraIcons[ids->data[0] - 1];
				extra->setPosition(pos++);
				extra->setSlot(slot);
			}
			else {
				char name[128];
				mir_snprintf(name, SIZEOF(name), "__group_%d", groups.getCount());

				ExtraIconGroup *group = new ExtraIconGroup(name);

				for (int i = 0; i < ids->count; i++) {
					BaseExtraIcon *extra = registeredExtraIcons[ids->data[i] - 1];
					extra->setPosition(pos++);

					group->addExtraIcon(extra);
				}

				group->setSlot(slot);
				groups.insert(group);
			}

			ht = m_tree.GetNextSibling(ht);
		}

		// Store data
		for (int i = 0; i < registeredExtraIcons.getCount(); i++) {
			BaseExtraIcon *extra = registeredExtraIcons[i];

			char setting[512];
			mir_snprintf(setting, SIZEOF(setting), "Position_%s", extra->getName());
			db_set_w(NULL, MODULE_NAME, setting, extra->getPosition());

			mir_snprintf(setting, SIZEOF(setting), "Slot_%s", extra->getName());
			db_set_w(NULL, MODULE_NAME, setting, extra->getSlot());
		}

		CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)MODULE_NAME "Groups");
		db_set_w(NULL, MODULE_NAME "Groups", "Count", groups.getCount());
		for (int k = 0; k < groups.getCount(); k++) {
			ExtraIconGroup *group = groups[k];

			char setting[512];
			mir_snprintf(setting, SIZEOF(setting), "%d_count", k);
			db_set_w(NULL, MODULE_NAME "Groups", setting, (WORD)group->items.getCount());

			for (int j = 0; j < group->items.getCount(); j++) {
				BaseExtraIcon *extra = group->items[j];

				mir_snprintf(setting, SIZEOF(setting), "%d_%d", k, j);
				db_set_s(NULL, MODULE_NAME "Groups", setting, extra->getName());
			}
		}

		// Clean removed slots
		for (int j = firstEmptySlot; j <= lastUsedSlot; j++)
			RemoveExtraIcons(j);

		// Apply icons to new slots
		RebuildListsBasedOnGroups(groups);
		for (int n = 0; n < extraIconsBySlot.getCount(); n++) {
			ExtraIcon *extra = extraIconsBySlot[n];
			if (extra->getType() != EXTRAICON_TYPE_GROUP)
				if (oldSlots[((BaseExtraIcon *)extra)->getID() - 1] == extra->getSlot())
					continue;

			if (extra->isEnabled())
				extra->applyIcons();
		}

		delete[] oldSlots;
	}

	virtual void OnDestroy()
	{
		HTREEITEM hItem = m_tree.GetRoot();
		while (hItem) {
			delete Tree_GetIDs(hItem);
			hItem = m_tree.GetNextSibling(hItem);
		}

		ImageList_Destroy(m_tree.GetImageList(TVSIL_NORMAL));
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom == IDC_EXTRAORDER && lpnmhdr->code == NM_RCLICK) {
				HTREEITEM hSelected = m_tree.GetDropHilight();
				if (hSelected != NULL && !m_tree.IsSelected(hSelected)) {
					m_tree.UnselectAll();
					m_tree.SelectItem(hSelected);
				}

				int sels = m_tree.GetNumSelected();
				if (sels > 1) {
					if (ShowPopup(0) == ID_GROUP) {
						GroupSelectedItems();
						NotifyChange();
					}
				}
				else if (sels == 1) {
					HTREEITEM hItem = m_tree.GetSelection();
					intlist *ids = Tree_GetIDs(hItem);
					if (ids->count > 1) {
						if (ShowPopup(1) == ID_UNGROUP) {
							UngroupSelectedItems();
							NotifyChange();
						}
					}
				}
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam, LPARAM lParam)
{
	if (GetNumberOfSlots() < 1)
		return 0;

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.pszGroup = LPGEN("Contact list");
	odp.pszTitle = LPGEN("Extra icons");
	odp.pszTab = LPGEN("General");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new CExtraIconOptsDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}
