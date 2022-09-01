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

#include "IcoLib.h"

struct TreeItem
{
	char *paramName;
	uint32_t value;
};

/////////////////////////////////////////////////////////////////////////////////////////

static HICON ExtractIconFromPath(const wchar_t *path, int cxIcon, int cyIcon)
{
	if (!path)
		return (HICON)nullptr;

	int n;
	wchar_t file[MAX_PATH], fileFull[MAX_PATH];
	mir_wstrncpy(file, path, _countof(file));
	wchar_t *comma = wcsrchr(file, ',');
	if (!comma)
		n = 0;
	else {
		n = _wtoi(comma + 1);
		*comma = 0;
	}
	PathToAbsoluteW(file, fileFull);

	// SHOULD BE REPLACED WITH GOOD ENOUGH FUNCTION
	HICON hIcon = nullptr;
	_ExtractIconEx(fileFull, n, cxIcon, cyIcon, &hIcon, LR_COLOR);
	return hIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconItem_GetIcon_Preview

static HICON IconItem_GetIcon_Preview(IcolibItem *item)
{
	HICON hIcon = nullptr;

	if (!item->temp_reset) {
		HICON hRefIcon = IcoLib_GetIconByHandle((HANDLE)item, false);
		hIcon = CopyIcon(hRefIcon);
		if (item->source_small && item->source_small->icon == hRefIcon)
			item->source_small->releaseIcon();
	}
	else {
		if (item->default_icon) {
			HICON hRefIcon = item->default_icon->getIcon();
			if (hRefIcon) {
				hIcon = CopyIcon(hRefIcon);
				if (item->default_icon->icon == hRefIcon)
					item->default_icon->releaseIcon();
			}
		}

		if (!hIcon && item->default_file) {
			item->default_icon->release();
			item->default_icon = GetIconSourceItem(item->default_file->file, item->default_indx, item->cx, item->cy);
			if (item->default_icon) {
				HICON hRefIcon = item->default_icon->getIcon();
				if (hRefIcon) {
					hIcon = CopyIcon(hRefIcon);
					if (item->default_icon->icon == hRefIcon)
						item->default_icon->releaseIcon();
				}
			}
		}

		if (!hIcon)
			return CopyIcon(hIconBlank);
	}
	return hIcon;
}

// User interface

static void __fastcall MySetCursor(wchar_t *nCursor)
{
	SetCursor(LoadCursor(nullptr, nCursor));
}

static wchar_t* OpenFileDlg(HWND hParent, const wchar_t *szFile, BOOL bAll)
{
	OPENFILENAME ofn = { 0 };
	wchar_t filter[512], *pfilter, file[MAX_PATH * 2];

	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = hParent;

	mir_wstrcpy(filter, TranslateT("Icon sets"));
	if (bAll)
		mir_wstrcat(filter, L" (*.dll;*.mir;*.icl;*.exe;*.ico)");
	else
		mir_wstrcat(filter, L" (*.dll;*.mir)");

	pfilter = filter + mir_wstrlen(filter) + 1;
	if (bAll)
		mir_wstrcpy(pfilter, L"*.DLL;*.MIR;*.ICL;*.EXE;*.ICO");
	else
		mir_wstrcpy(pfilter, L"*.DLL;*.MIR");

	pfilter += mir_wstrlen(pfilter) + 1;
	mir_wstrcpy(pfilter, TranslateT("All files"));
	mir_wstrcat(pfilter, L" (*)");
	pfilter += mir_wstrlen(pfilter) + 1;
	mir_wstrcpy(pfilter, L"*");
	pfilter += mir_wstrlen(pfilter) + 1;
	*pfilter = '\0';

	ofn.lpstrFilter = filter;
	ofn.lpstrDefExt = L"dll";
	mir_wstrncpy(file, szFile, _countof(file));
	ofn.lpstrFile = file;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
	ofn.nMaxFile = MAX_PATH * 2;

	if (!GetOpenFileName(&ofn))
		return nullptr;

	return mir_wstrdup(file);
}

/////////////////////////////////////////////////////////////////////////////////////////
// icon import dialog's window procedure

class CIconImportDlg : public CDlgBase
{
	HWND m_hwndDragOver = nullptr;
	int  m_iDragItem = 0, m_iDropHiLite = 0;
	bool m_bDragging = false;

	class CIcoLibOptsDlg *m_pParent;

	CCtrlHyperlink m_urlGetMore;
	CCtrlListView m_preview;
	CCtrlButton m_btnBrowse;
	CCtrlEdit m_iconSet;

public:
	CIconImportDlg(CIcoLibOptsDlg *_parent) :
		CDlgBase(g_plugin, IDD_ICOLIB_IMPORT),
		m_pParent(_parent),
		m_preview(this, IDC_PREVIEW),
		m_iconSet(this, IDC_ICONSET),
		m_btnBrowse(this, IDC_BROWSE),
		m_urlGetMore(this, IDC_GETMORE, "https://miranda-ng.org/")
	{
		m_btnBrowse.OnClick = Callback(this, &CIconImportDlg::OnBrowseClick);
		m_iconSet.OnChange = Callback(this, &CIconImportDlg::OnEditChange);
		m_preview.OnBeginDrag = Callback(this, &CIconImportDlg::OnBeginDragPreview);
	}

	bool OnInitDialog() override;
	void OnDestroy() override;

	virtual int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_ICONSET:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

		case IDC_BROWSE:
			return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;

		case IDC_PREVIEW:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

		case IDC_GETMORE:
			return RD_ANCHORX_CENTRE | RD_ANCHORY_BOTTOM;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP; // default
	}

	void OnEditChange(void*)
	{
		RebuildIconsPreview();
	}

	void OnBrowseClick(void*)
	{
		wchar_t str[MAX_PATH];
		m_iconSet.GetText(str, _countof(str));
		if (wchar_t *file = OpenFileDlg(GetParent(m_hwnd), str, TRUE)) {
			m_iconSet.SetText(file);
			mir_free(file);
		}
	}

	void OnBeginDragPreview(CCtrlListView::TEventInfo *evt)
	{
		SetCapture(m_hwnd);
		m_bDragging = true;
		m_iDragItem = evt->nmlv->iItem;
		m_iDropHiLite = -1;
		ImageList_BeginDrag(m_preview.GetImageList(LVSIL_NORMAL), m_iDragItem, g_iIconX / 2, g_iIconY / 2);

		POINT pt;
		GetCursorPos(&pt);

		RECT rc;
		GetWindowRect(m_hwnd, &rc);
		ImageList_DragEnter(m_hwnd, pt.x - rc.left, pt.y - rc.top);
		m_hwndDragOver = m_hwnd;
	}

	void RebuildIconsPreview()
	{
		MySetCursor(IDC_WAIT);
		m_preview.DeleteAllItems();
		HIMAGELIST hIml = m_preview.GetImageList(LVSIL_NORMAL);
		ImageList_RemoveAll(hIml);

		wchar_t filename[MAX_PATH], caption[64];
		m_iconSet.GetText(filename, _countof(filename));

		RECT rcPreview, rcGroup;
		GetWindowRect(m_preview.GetHwnd(), &rcPreview);
		GetWindowRect(GetDlgItem(m_hwnd, IDC_IMPORTMULTI), &rcGroup);
		// SetWindowPos(hPreview, 0, 0, 0, rcPreview.right-rcPreview.left, rcGroup.bottom-rcPreview.top, SWP_NOZORDER|SWP_NOMOVE);

		if (_waccess(filename, 0) != 0) {
			MySetCursor(IDC_ARROW);
			return;
		}

		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		int count = (int)_ExtractIconEx(filename, -1, 16, 16, nullptr, LR_DEFAULTCOLOR);
		for (int i = 0; i < count; lvi.iItem++, i++) {
			mir_snwprintf(caption, L"%d", i + 1);
			lvi.pszText = caption;

			HICON hIcon = nullptr;
			if (_ExtractIconEx(filename, i, 16, 16, &hIcon, LR_DEFAULTCOLOR) == 1) {
				lvi.iImage = ImageList_AddIcon(hIml, hIcon);
				DestroyIcon(hIcon);
				lvi.lParam = i;
				m_preview.InsertItem(&lvi);
			}
		}
		MySetCursor(IDC_ARROW);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override; // forward declaration
};

/////////////////////////////////////////////////////////////////////////////////////////
// IcoLib options window procedure

static int CALLBACK DoSortIconsFunc(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	return mir_wstrcmpi(iconList[lParam1]->getDescr(), iconList[lParam2]->getDescr());
}

static int CALLBACK DoSortIconsFuncByOrder(LPARAM lParam1, LPARAM lParam2, LPARAM)
{
	return iconList[lParam1]->orderID - iconList[lParam2]->orderID;
}

static void LoadSectionIcons(wchar_t *filename, SectionItem* sectionActive)
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s,", filename);
	size_t suffIndx = mir_wstrlen(path);

	mir_cslock lck(csIconList);

	for (auto &it : iconList) {
		if (it->default_file && it->section == sectionActive) {
			_itow(it->default_indx, path + suffIndx, 10);
			HICON hIcon = ExtractIconFromPath(path, it->cx, it->cy);
			if (!hIcon)
				continue;

			SafeDestroyIcon(it->temp_icon);
			it->temp_icon = hIcon;

			replaceStrW(it->temp_file, path);
			it->temp_reset = FALSE;
		}
	}
}

static void UndoChanges(int iconIndx, int cmd)
{
	IcolibItem *item = iconList[iconIndx];

	if (!item->temp_file && !item->temp_icon && item->temp_reset && cmd == ID_CANCELCHANGE)
		item->temp_reset = FALSE;
	else {
		replaceStrW(item->temp_file, nullptr);
		SafeDestroyIcon(item->temp_icon);
	}

	if (cmd == ID_RESET)
		item->temp_reset = TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CIcoLibOptsDlg : public CDlgBase
{
	friend class CIconImportDlg;

	CIconImportDlg *m_pDialog = nullptr;
	HTREEITEM m_hPrevItem = nullptr;

	CCtrlButton m_btnImport, m_btnLoadIcons;
	CCtrlTreeView m_categoryList;
	CCtrlListView m_preview;
	CCtrlHyperlink m_urlGetMore;

	HTREEITEM FindNamedTreeItemAt(HTREEITEM hItem, const wchar_t *name)
	{
		TVITEMEX tvi = { 0 };
		if (hItem)
			tvi.hItem = m_categoryList.GetChild(hItem);
		else
			tvi.hItem = m_categoryList.GetRoot();

		if (!name)
			return tvi.hItem;

		wchar_t str[MAX_PATH];
		tvi.mask = TVIF_TEXT;
		tvi.pszText = str;
		tvi.cchTextMax = _countof(str);

		while (tvi.hItem) {
			m_categoryList.GetItem(&tvi);

			if (!mir_wstrcmp(tvi.pszText, name))
				return tvi.hItem;

			tvi.hItem = m_categoryList.GetNextSibling(tvi.hItem);
		}
		return nullptr;
	}

	void RebuildTree()
	{
		if (!m_categoryList.GetHwnd())
			return;

		m_categoryList.SelectItem(nullptr);
		m_categoryList.DeleteAllItems();

		for (int indx = 0; indx < sectionList.getCount(); indx++) {
			int sectionLevel = 0;

			HTREEITEM hSection = nullptr;
			wchar_t itemName[1024];
			mir_wstrcpy(itemName, sectionList[indx]->name);
			wchar_t *sectionName = itemName;

			while (sectionName) {
				// allow multi-level tree
				wchar_t *pItemName = sectionName;
				HTREEITEM hItem;

				if (sectionName = wcschr(sectionName, '/')) {
					// one level deeper
					*sectionName = 0;
				}

				pItemName = TranslateW(pItemName);
				hItem = FindNamedTreeItemAt(hSection, pItemName);
				if (!sectionName || !hItem) {
					if (!hItem) {
						TVINSERTSTRUCT tvis = {};
						TreeItem *treeItem = (TreeItem *)mir_alloc(sizeof(TreeItem));
						treeItem->value = SECTIONPARAM_MAKE(indx, sectionLevel, sectionName ? 0 : SECTIONPARAM_HAVEPAGE);
						treeItem->paramName = mir_u2a(itemName);

						tvis.hParent = hSection;
						tvis.hInsertAfter = TVI_SORT;
						tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
						tvis.item.pszText = pItemName;
						tvis.item.lParam = (LPARAM)treeItem;
						tvis.item.state = tvis.item.stateMask = db_get_b(0, "SkinIconsUI", treeItem->paramName, TVIS_EXPANDED);
						hItem = m_categoryList.InsertItem(&tvis);
					}
					else {
						TVITEMEX tvi = { 0 };
						tvi.hItem = hItem;
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						m_categoryList.GetItem(&tvi);
						TreeItem *treeItem = (TreeItem *)tvi.lParam;
						treeItem->value = SECTIONPARAM_MAKE(indx, sectionLevel, SECTIONPARAM_HAVEPAGE);
					}
				}

				if (sectionName) {
					*sectionName = '/';
					sectionName++;
				}
				sectionLevel++;

				hSection = hItem;
			}
		}

		ShowWindow(m_categoryList.GetHwnd(), SW_SHOW);

		m_categoryList.SelectItem(FindNamedTreeItemAt(nullptr, nullptr));
	}

	int OpenPopupMenu()
	{
		POINT pt;
		GetCursorPos(&pt);
		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_ICOLIB_CONTEXT));
		HMENU hPopup = GetSubMenu(hMenu, 0);
		TranslateMenu(hPopup);
		int cmd = TrackPopupMenu(hPopup, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
		DestroyMenu(hMenu);
		return cmd;
	}

	void UpdateIconsPreview()
	{
		HIMAGELIST hIml = m_preview.GetImageList(LVSIL_NORMAL);

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_IMAGE | LVIF_PARAM;
		int count = m_preview.GetItemCount();

		for (int indx = 0; indx < count; indx++) {
			lvi.iItem = indx;
			m_preview.GetItem(&lvi);

			HICON hIcon;
			{
				mir_cslock lck(csIconList);
				hIcon = iconList[lvi.lParam]->temp_icon;
				if (!hIcon)
					hIcon = IconItem_GetIcon_Preview(iconList[lvi.lParam]);
			}

			if (hIcon)
				ImageList_ReplaceIcon(hIml, lvi.iImage, hIcon);
			if (hIcon != iconList[lvi.lParam]->temp_icon)
				SafeDestroyIcon(hIcon);
		}
		m_preview.RedrawItems(0, count);
	}

	void DoOptionsChanged()
	{
		UpdateIconsPreview();
		NotifyChange();
	}

	void DoIconsChanged()
	{
		UpdateIconsPreview();
		NotifyEventHooks(hIconsChangedEvent, 0, 0);
	}

	void LoadSubIcons(wchar_t *filename, HTREEITEM hItem)
	{
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		m_categoryList.GetItem(&tvi);

		TreeItem *treeItem = (TreeItem*)tvi.lParam;
		SectionItem *sectionActive = sectionList[SECTIONPARAM_INDEX(treeItem->value)];

		tvi.hItem = m_categoryList.GetChild(tvi.hItem);
		while (tvi.hItem) {
			LoadSubIcons(filename, tvi.hItem);
			tvi.hItem = m_categoryList.GetNextSibling(tvi.hItem);
		}

		if (SECTIONPARAM_FLAGS(treeItem->value) & SECTIONPARAM_HAVEPAGE)
			LoadSectionIcons(filename, sectionActive);
	}

	void UndoSubItemChanges(HTREEITEM hItem, int cmd)
	{
		TVITEMEX tvi = { 0 };
		tvi.mask = TVIF_HANDLE | TVIF_PARAM;
		tvi.hItem = hItem;
		m_categoryList.GetItem(&tvi);

		TreeItem *treeItem = (TreeItem *)tvi.lParam;
		if (SECTIONPARAM_FLAGS(treeItem->value) & SECTIONPARAM_HAVEPAGE) {
			mir_cslock lck(csIconList);

			for (int indx = 0; indx < iconList.getCount(); indx++)
				if (iconList[indx]->section == sectionList[SECTIONPARAM_INDEX(treeItem->value)])
					UndoChanges(indx, cmd);
		}

		for (hItem = m_categoryList.GetChild(tvi.hItem); hItem; hItem = m_categoryList.GetNextSibling(hItem))
			UndoSubItemChanges(hItem, cmd);
	}

public:
	CIcoLibOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_ICOLIB),
		m_preview(this, IDC_PREVIEW),
		m_btnImport(this, IDC_IMPORT),
		m_urlGetMore(this, IDC_GETMORE, "https://miranda-ng.org/tags/icons"),
		m_btnLoadIcons(this, IDC_LOADICONS),
		m_categoryList(this, IDC_CATEGORYLIST)
	{
		m_btnImport.OnClick = Callback(this, &CIcoLibOptsDlg::OnImport);
		m_btnLoadIcons.OnClick = Callback(this, &CIcoLibOptsDlg::OnLoadIcons);

		m_preview.OnGetInfoTip = Callback(this, &CIcoLibOptsDlg::OnGetInfoTip);
		m_preview.OnBuildMenu = Callback(this, &CIcoLibOptsDlg::OnPreviewMenu);
		
		m_categoryList.OnSelChanged = Callback(this, &CIcoLibOptsDlg::OnCategoryChanged);
		m_categoryList.OnDeleteItem = Callback(this, &CIcoLibOptsDlg::OnCategoryDeleted);
		m_categoryList.OnBuildMenu = Callback(this, &CIcoLibOptsDlg::OnTreeMenu);
	}

	bool OnInitDialog() override
	{
		// Reset temporary data & upload sections list
		{
			mir_cslock lck(csIconList);

			for (auto &it : iconList) {
				it->temp_file = nullptr;
				it->temp_icon = nullptr;
				it->temp_reset = false;
			}
			bNeedRebuild = false;
		}

		// Setup preview listview
		m_preview.SetUnicodeFormat(TRUE);
		m_preview.SetExtendedListViewStyleEx(LVS_EX_INFOTIP, LVS_EX_INFOTIP);
		m_preview.SetImageList(ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 0, 30), LVSIL_NORMAL);
		m_preview.SetIconSpacing(56, 67);

		RebuildTree();
		return true;
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_NOTIFY:
			if (bNeedRebuild) {
				bNeedRebuild = false;
				RebuildTree();
			}
			break;
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}

	void OnImport(void*)
	{
		m_pDialog = new CIconImportDlg(this);
		m_pDialog->Show();
		m_btnImport.Disable();
	}

	void OnLoadIcons(void*)
	{
		wchar_t filetmp[1] = { 0 };
		if (wchar_t *file = OpenFileDlg(m_hwnd, filetmp, FALSE)) {
			wchar_t filename[MAX_PATH];
			PathToRelativeW(file, filename);
			mir_free(file);

			MySetCursor(IDC_WAIT);
			LoadSubIcons(filename, m_categoryList.GetSelection());
			MySetCursor(IDC_ARROW);

			DoOptionsChanged();
		}
	}

	bool OnApply() override
	{
		{
			mir_cslock lck(csIconList);

			for (auto &it : iconList) {
				if (it->temp_reset) {
					db_unset(0, "SkinIcons", it->name);
					if (it->source_small != it->default_icon) {
						it->source_small->release();
						it->source_small = nullptr;
					}
				}
				else if (it->temp_file) {
					db_set_ws(0, "SkinIcons", it->name, it->temp_file);
					it->source_small->release();
					it->source_small = nullptr;
					SafeDestroyIcon(it->temp_icon);
				}
			}
		}

		DoIconsChanged();
		return true;
	}

	void OnDestroy() override
	{
		HTREEITEM hti = m_categoryList.GetRoot();
		while (hti != nullptr) {
			TVITEMEX tvi;
			tvi.mask = TVIF_STATE | TVIF_HANDLE | TVIF_CHILDREN | TVIF_PARAM;
			tvi.hItem = hti;
			tvi.stateMask = (uint32_t)-1;
			m_categoryList.GetItem(&tvi);

			if (tvi.cChildren > 0) {
				TreeItem *treeItem = (TreeItem *)tvi.lParam;
				if (tvi.state & TVIS_EXPANDED)
					db_set_b(0, "SkinIconsUI", treeItem->paramName, TVIS_EXPANDED);
				else
					db_set_b(0, "SkinIconsUI", treeItem->paramName, 0);
			}

			HTREEITEM ht = m_categoryList.GetChild(hti);
			if (ht == nullptr) {
				ht = m_categoryList.GetNextSibling(hti);
				while (ht == nullptr) {
					hti = m_categoryList.GetParent(hti);
					if (hti == nullptr)
						break;
					
					ht = m_categoryList.GetNextSibling(hti);
				}
			}

			hti = ht;
		}

		if (m_pDialog)
			m_pDialog->Close();

		mir_cslock lck(csIconList);
		for (auto &it : iconList) {
			replaceStrW(it->temp_file, nullptr);
			SafeDestroyIcon(it->temp_icon);
		}
	}

	void OnPreviewMenu(void*)
	{
		UINT count = m_preview.GetSelectedCount();
		if (count <= 0)
			return;

		int cmd = OpenPopupMenu();
		switch (cmd) {
		case ID_CANCELCHANGE:
		case ID_RESET:
			int itemIndx = -1;
			while ((itemIndx = m_preview.GetNextItem(itemIndx, LVNI_SELECTED)) != -1) {
				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = itemIndx;
				m_preview.GetItem(&lvi);

				UndoChanges(lvi.lParam, cmd);
			}

			DoOptionsChanged();
		}
	}

	void OnTreeMenu(void*)
	{
		int cmd = OpenPopupMenu();
		switch (cmd) {
		case ID_CANCELCHANGE:
		case ID_RESET:
			UndoSubItemChanges(m_categoryList.GetSelection(), cmd);
			DoOptionsChanged();
		}
	}

	void OnGetInfoTip(CCtrlListView::TEventInfo *evt)
	{
		NMLVGETINFOTIP *pInfoTip = evt->nmlvit;
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = pInfoTip->iItem;
		ListView_GetItem(pInfoTip->hdr.hwndFrom, &lvi);

		if (lvi.lParam < iconList.getCount()) {
			IcolibItem *item = iconList[lvi.lParam];
			if (item->temp_file)
				wcsncpy_s(pInfoTip->pszText, pInfoTip->cchTextMax, item->temp_file, _TRUNCATE);
			else if (item->default_file)
				mir_snwprintf(pInfoTip->pszText, pInfoTip->cchTextMax, L"%s, %d", item->default_file->file, item->default_indx);
		}
	}

	void OnCategoryChanged(CCtrlTreeView::TEventInfo *evt)
	{
		NMTREEVIEW *pnmtv = evt->nmtv;
		TVITEM tvi = pnmtv->itemNew;
		TreeItem *treeItem = (TreeItem *)tvi.lParam;
		if (treeItem) {
			SectionItem *sectionActive;
			if (SECTIONPARAM_FLAGS(treeItem->value) & SECTIONPARAM_HAVEPAGE)
				sectionActive = sectionList[SECTIONPARAM_INDEX(treeItem->value)];
			else
				sectionActive = nullptr;

			m_preview.Enable(sectionActive != nullptr);
			m_preview.DeleteAllItems();
			HIMAGELIST hIml = m_preview.GetImageList(LVSIL_NORMAL);
			ImageList_RemoveAll(hIml);

			if (sectionActive == nullptr)
				return;

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
			{
				mir_cslock lck(csIconList);

				for (auto &item : iconList) {
					if (item->section == sectionActive) {
						lvi.pszText = item->getDescr();
						HICON hIcon = item->temp_icon;
						if (!hIcon)
							hIcon = IconItem_GetIcon_Preview(item);
						lvi.iImage = ImageList_AddIcon(hIml, hIcon);
						lvi.lParam = iconList.indexOf(&item);
						m_preview.InsertItem(&lvi);
						if (hIcon != item->temp_icon)
							SafeDestroyIcon(hIcon);
					}
				}
			}

			if (sectionActive->flags & SIDF_SORTED)
				m_preview.SortItems(DoSortIconsFunc, 0);
			else
				m_preview.SortItems(DoSortIconsFuncByOrder, 0);
		}
	}

	void OnCategoryDeleted(CCtrlTreeView::TEventInfo *evt)
	{
		TreeItem *treeItem = (TreeItem *)(evt->nmtv->itemOld.lParam);
		if (treeItem) {
			mir_free(treeItem->paramName);
			mir_free(treeItem);
		}
	}

	void ChangeIcon(int iItem, const wchar_t *path)
	{
		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		lvi.iItem = iItem;
		m_preview.GetItem(&lvi);
		{
			mir_cslock lck(csIconList);

			IcolibItem *item = iconList[lvi.lParam];
			SafeDestroyIcon(item->temp_icon);

			replaceStrW(item->temp_file, path);
			item->temp_icon = (HICON)ExtractIconFromPath(path, item->cx, item->cy);
			item->temp_reset = false;
		}
		DoOptionsChanged();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

bool CIconImportDlg::OnInitDialog()
{
	m_preview.SetImageList(ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 0, 100), LVSIL_NORMAL);
	m_preview.SetIconSpacing(56, 67);

	RECT rcThis, rcParent;
	int cxScreen = GetSystemMetrics(SM_CXSCREEN);

	GetWindowRect(m_hwnd, &rcThis);
	GetWindowRect(m_pParent->GetHwnd(), &rcParent);
	OffsetRect(&rcThis, rcParent.right - rcThis.left, 0);
	OffsetRect(&rcThis, 0, rcParent.top - rcThis.top);
	GetWindowRect(GetParent(m_pParent->GetHwnd()), &rcParent);
	if (rcThis.right > cxScreen) {
		OffsetRect(&rcParent, cxScreen - rcThis.right, 0);
		OffsetRect(&rcThis, cxScreen - rcThis.right, 0);
		MoveWindow(GetParent(m_pParent->GetHwnd()), rcParent.left, rcParent.top, rcParent.right - rcParent.left, rcParent.bottom - rcParent.top, TRUE);
	}
	MoveWindow(m_hwnd, rcThis.left, rcThis.top, rcThis.right - rcThis.left, rcThis.bottom - rcThis.top, FALSE);
	GetClientRect(m_hwnd, &rcThis);
	SendMessage(m_hwnd, WM_SIZE, 0, MAKELPARAM(rcThis.right - rcThis.left, rcThis.bottom - rcThis.top));

	SHAutoComplete(m_iconSet.GetHwnd(), 1);
	m_iconSet.SetText(L"icons.dll");
	return true;
}

void CIconImportDlg::OnDestroy()
{
	m_pParent->m_btnImport.Enable();
}

INT_PTR CIconImportDlg::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_MOUSEMOVE:
		if (m_bDragging) {
			int onItem = 0;

			LVHITTESTINFO lvhti;
			lvhti.pt.x = (short)LOWORD(lParam); lvhti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(m_hwnd, &lvhti.pt);
			HWND hwndOver = WindowFromPoint(lvhti.pt);

			RECT rc;
			GetWindowRect(hwndOver, &rc);

			int dragX = lvhti.pt.x - rc.left, dragY = lvhti.pt.y - rc.top;
			if (hwndOver != m_hwndDragOver) {
				ImageList_DragLeave(m_hwndDragOver);
				m_hwndDragOver = hwndOver;
				ImageList_DragEnter(m_hwndDragOver, dragX, dragY);
			}

			ImageList_DragMove(dragX, dragY);
			CCtrlListView &mainList = m_pParent->m_preview;
			if (hwndOver == mainList.GetHwnd()) {
				ScreenToClient(mainList.GetHwnd(), &lvhti.pt);

				if (mainList.HitTest(&lvhti) != -1) {
					if (lvhti.iItem != m_iDropHiLite) {
						ImageList_DragLeave(m_hwndDragOver);
						if (m_iDropHiLite != -1)
							mainList.SetItemState(m_iDropHiLite, 0, LVIS_DROPHILITED);
						m_iDropHiLite = lvhti.iItem;
						mainList.SetItemState(m_iDropHiLite, LVIS_DROPHILITED, LVIS_DROPHILITED);
						UpdateWindow(mainList.GetHwnd());
						ImageList_DragEnter(m_hwndDragOver, dragX, dragY);
					}
					onItem = 1;
				}
			}

			if (!onItem && m_iDropHiLite != -1) {
				ImageList_DragLeave(m_hwndDragOver);
				mainList.SetItemState(m_iDropHiLite, 0, LVIS_DROPHILITED);
				UpdateWindow(mainList.GetHwnd());
				ImageList_DragEnter(m_hwndDragOver, dragX, dragY);
				m_iDropHiLite = -1;
			}
			MySetCursor(onItem ? IDC_ARROW : IDC_NO);
		}
		break;

	case WM_LBUTTONUP:
		if (m_bDragging) {
			ReleaseCapture();
			ImageList_EndDrag();
			m_bDragging = 0;
			if (m_iDropHiLite != -1) {
				wchar_t fullPath[MAX_PATH], filename[MAX_PATH];
				m_iconSet.GetText(fullPath, _countof(fullPath));
				PathToRelativeW(fullPath, filename);

				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = m_iDragItem;
				lvi.iSubItem = 0;
				m_preview.GetItem(&lvi);

				m_pParent->ChangeIcon(m_iDropHiLite, CMStringW(FORMAT, L"%s,%d", filename, (int)lvi.lParam));
				m_pParent->m_preview.SetItemState(m_iDropHiLite, 0, LVIS_DROPHILITED);
			}
		}
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// module initialization

int SkinOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -180000000;
	odp.pDialog = new CIcoLibOptsDlg();
	odp.szTitle.a = LPGEN("Icons");
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
