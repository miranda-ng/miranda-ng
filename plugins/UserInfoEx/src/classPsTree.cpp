/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

/***********************************************************************************************************
 * construction and destruction
 ***********************************************************************************************************/

/**
 * name:	CPsTree
 * class:	CPsTree
 * desc:	constructor
 * param:	none
 * return:	none
 **/
CPsTree::CPsTree(LPPS pPs) :
	_pages(1)
{
	_curItem = -1;
	_dwFlags = 0;
	_hLabelEdit = nullptr;
	_hDragItem = nullptr;
	_isDragging = false;
	_pPs = pPs;
}

/**
 * name:	~CPsTree
 * class:	CPsTree
 * desc:	frees up all memory, used by the tree control
 * return:	nothing
 **/
CPsTree::~CPsTree()
{
	if (_hLabelEdit) {
		DestroyWindow(_hLabelEdit);
		_hLabelEdit = nullptr;
	}

	_pages.destroy();

	ImageList_Destroy(_hImages);
	_hImages = nullptr;
}

/**
 * name:	CPsTree
 * class:	CPsTree
 * desc:	constructor
 * param:	none
 * return:	none
 **/
uint8_t CPsTree::Create(HWND hWndTree, CPsHdr* pPsh)
{
	uint8_t rc;

	if (hWndTree && pPsh->_hImages && pPsh->_pPages && pPsh->_numPages)
	{
		_hWndTree = hWndTree;
		_hImages = pPsh->_hImages;
		_dwFlags = pPsh->_dwFlags;

		for (int i = 0; i < pPsh->_numPages; i++)
			_pages.insert(pPsh->_pPages[i]);
		pPsh->_pPages = nullptr;
		pPsh->_numPages = 0;

		TreeView_SetImageList(_hWndTree, _hImages, TVSIL_NORMAL);
		TreeView_SetItemHeight(_hWndTree, TreeView_GetItemHeight(_hWndTree) + 4);
		SetUserData(_hWndTree, this);
		rc = TRUE;
	}
	else
	{
		rc = FALSE;
	}
	return rc;
}

/**
 * name:	AddDummyItem
 * class:	CPsTree
 * desc:	insert an empty tree item group
 * param:	pszGroup	- utf8 encoded string of the item to add
 * return:	index of the new item or -1 if failed to add
 **/

CPsTreeItem* CPsTree::AddDummyItem(const char *pszGroup)
{
	if (!mir_strcmpi(pszGroup, TREE_ROOTITEM))
		return nullptr;
		
	CPsHdr psh;
	psh._hContact = _pPs->hContact;
	psh._pszProto = _pPs->pszProto;
	psh._hImages = _hImages;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE;
	uip.szTitle.w = mir_utf8decodeW(pszGroup);
	uip.pPlugin = &g_plugin;

	auto *p = new CPsTreeItem();
	p->Create(&psh, &uip);
	_pages.insert(p);
	
	return p;
}

void CPsTree::Remove(HINSTANCE hInst)
{
	bool bRemoved = false;

	for (auto &it : _pages.rev_iter()) {
		if (it->Inst() == hInst) {
			bRemoved = true;
			_pages.removeItem(&it);
		}
	}

	if (bRemoved) {
		TreeView_DeleteAllItems(_hWndTree);

		for (auto &it : _pages)
			it->Hti(nullptr);

		InitTreeItems(nullptr);
	}
}

/**
 * name:	InitTreeItems()
 * desc:	initialize the tree control's datastructure
 * param:	needWidth	- width to expand the tree by
 * return:	TRUE if initialization is ok, FALSE otherwise
 **/
uint8_t CPsTree::InitTreeItems(LPWORD needWidth)
{
	if (!_hWndTree || !_pages.getCount())
		return FALSE;

	DBVARIANT dbv;
	if (!DB::Setting::GetUString(0, MODULENAME, SET_LASTITEM, &dbv)) {
		_curItem = FindItemIndexByName(dbv.pszVal);
		db_free(&dbv);
	}

	// init the groups
	if ((_dwFlags & PSTVF_GROUPS) || (!_pPs->hContact && myGlobals.CanChangeDetails)) {
		// set treeview styles
		TreeView_SetIndent(_hWndTree, 3);
		SetWindowLongPtr(_hWndTree, GWL_STYLE, GetWindowLongPtr(_hWndTree, GWL_STYLE) | TVS_HASBUTTONS);

		// init the iParent member for all the items
		for (int i = 0; i < _pages.getCount(); i++) {
			auto &it = _pages[i];
			char *pszGroup = it.ParentItemName();
			if (pszGroup != nullptr) {
				auto *pParent = FindItemByName(pszGroup);

				// need to add an empty parent item
				if (pParent == nullptr)
					pParent = AddDummyItem(pszGroup);

				it.Parent(pParent);
				mir_free(pszGroup);
			}
		}
	}

	if (needWidth)
		*needWidth = 0;

	ShowWindow(_hWndTree, SW_HIDE);
	for (auto &it : _pages)
		if (it->State() != DBTVIS_INVISIBLE)
			ShowItem(it, needWidth);

	ShowWindow(_hWndTree, SW_SHOW);
	return TRUE;
}

/***********************************************************************************************************
 * finding items
 ***********************************************************************************************************/

/**
 * name:	FindItemIndexByHandle
 * class:	CPsTree
 * desc:	returns the treeitem with specified handle
 * param:	hItem		- handle of the treeview's treeitem
 * return:	HTREEITEM if item exists or NULL otherwise
 **/

int CPsTree::FindItemIndexByHandle(HTREEITEM hItem)
{
	for (auto &it : _pages)
		if (it && hItem == it->Hti())
			return _pages.indexOf(&it);

	return -1;
}

/**
 * name:	FindItemIndexByHandle
 * class:	CPsTree
 * desc:	returns the treeitem with specified handle
 * param:	hItem		- handle of the treeview's treeitem
 * return:	HTREEITEM if item exists or NULL otherwise
 **/

int CPsTree::FindItemIndexByName(LPCSTR pszName)
{
	for (auto &it : _pages)
		if (it && it->HasName(pszName))
			return _pages.indexOf(&it);

	return -1;
}

/**
 * name:	FindItemByHandle
 * class:	CPsTree
 * desc:	returns the treeitem with specified handle
 * param:	hItem		- handle of the treeview's treeitem
 * return:	HTREEITEM if item exists or NULL otherwise
 **/

CPsTreeItem* CPsTree::FindItemByHandle(HTREEITEM hItem)
{
	int i = FindItemIndexByHandle(hItem);
	if (i > -1)
		return &_pages[i];

	return nullptr;
}

/**
 * name:	FindItemByName
 * class:	CPsTree
 * desc:	returns the treeitem with specified name
 * param:	pszName		- name of the item to search for
 * return:	HTREEITEM if item exists or NULL otherwise
 **/
CPsTreeItem* CPsTree::FindItemByName(LPCSTR pszName)
{
	int i = FindItemIndexByName(pszName);
	if (i > -1) 
		return &_pages[i];

	return nullptr;
}

/**
 * name:	FindItemByHandle
 * class:	CPsTree
 * desc:	returns the treeitem with specified handle
 * param:	hItem		- handle of the treeview's treeitem
 * return:	HTREEITEM if item exists or NULL otherwise
 **/

CPsTreeItem* CPsTree::FindItemByResource(HINSTANCE hInst, int idDlg)
{
	for (auto &it : _pages)
		if (it && it->Inst() == hInst && it->DlgId() == idDlg) 
			return it;

	return nullptr;
}


/**
 * name:	FindItemHandleByName
 * class:	CPsTree
 * desc:	returns the treeitem with specified name
 * param:	pszName		- name of the item to search for
 * return:	HTREEITEM if item exists or NULL otherwise
 **/

HTREEITEM CPsTree::FindItemHandleByName(LPCSTR pszName)
{
	int i = FindItemIndexByName(pszName);
	if (i > -1)
		return _pages[i].Hti();

	return nullptr;
}

/***********************************************************************************************************
 * public methods
 ***********************************************************************************************************/


/**
 * name:	HideItem
 * class:	CPsTree
 * desc:	is called if icolib's icons have changed
 * param:	iPageIndex	- the index of the treeitem in the array.
 * return:	nothing
 **/

void CPsTree::HideItem(const int iPageIndex)
{
	if (IsIndexValid(iPageIndex)) {
		auto &p = _pages[iPageIndex];
		TreeView_DeleteItem(_hWndTree, p.Hti());
		p.Hti(nullptr);
		_dwFlags |= PSTVF_STATE_CHANGED;
	}
}

/**
 * name:	ShowItem
 * class:	CPsTree
 * desc:	displays on of the items in the treeview
 * param:	iPageIndex	- the index of the treeitem in the array.
 *			needWidth	- gives and takes the width, the treeview must have to show all items properly
 * return:	TRUE if item was added successfully, FALSE otherwise
 **/
HTREEITEM CPsTree::ShowItem(CPsTreeItem *pti, LPWORD needWidth)
{
	TVINSERTSTRUCT tvii;

	// check parameters
	if (!_hWndTree || !pti->Name() || !pti->Label()) {
		MsgErr(GetParent(_hWndTree), LPGENW("Due to a parameter error, one of the treeitems can't be added!"));
		return nullptr;
	}	
	
	// item is visible at the moment
	if ((tvii.itemex.hItem = pti->Hti()) == nullptr) {
		auto *pParent = pti->Parent();
		
		// init the rest of the treeitem
		tvii.hParent = (pParent) ? ShowItem(pParent, needWidth) : nullptr;
		tvii.hInsertAfter	= (_dwFlags & PSTVF_SORTTREE) ? TVI_SORT : TVI_LAST;
		tvii.itemex.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvii.itemex.pszText = pti->Label();
		tvii.itemex.state = pti->State() == DBTVIS_EXPANDED ? TVIS_EXPANDED : 0;
		tvii.itemex.stateMask = TVIS_EXPANDED;
		tvii.itemex.lParam = _pages.indexOf(pti);
		
		// set images
		if ((tvii.itemex.iImage = tvii.itemex.iSelectedImage = pti->Image()) != -1)
			tvii.itemex.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;

		// insert item into tree if set visible
		if ((tvii.itemex.hItem = TreeView_InsertItem(_hWndTree, &tvii)) == nullptr) {
			MsgErr(GetParent(_hWndTree), LPGENW("A fatal error occurred on adding a property sheet page!\nDialog creation aborted!"));
			return nullptr;
		}
		
		pti->Hti(tvii.itemex.hItem);

		// calculate width of treeview
		RECT rc;
		if (needWidth && TreeView_GetItemRect(_hWndTree, pti->Hti(), &rc, TRUE) && rc.right > *needWidth)
			*needWidth = (uint16_t)rc.right;
	}
	return tvii.itemex.hItem;
}

/**
 * name:	MoveItem()
 * class:	CPsTree
 * desc:	moves a treeitem and its children to a new position
 * param:	pPs					- datastructure of the propertysheetpage
 *			hItem				- the HTREEITEM to move
 *			hInsertAfter		- the HTREEITEM to insert hItem after
 *			bAsChild			- tells, whether to try to add hItem as child of hInsertAfter or not
 * return:	handle to new (moved) treeitem if successful or NULL otherwise
 **/
HTREEITEM CPsTree::MoveItem(HTREEITEM hItem, HTREEITEM hInsertAfter, uint8_t bAsChild)
{
	TVINSERTSTRUCT tvis;
	HTREEITEM hParent, hChild, hNewItem;

	if (!hItem || !hInsertAfter)
		return nullptr;
	if (hItem == hInsertAfter)
		return hItem;
	
	switch ((ULONG_PTR)hInsertAfter) {
		case TVI_ROOT:
		case TVI_FIRST:
		case TVI_LAST:
			hParent = NULL;
			bAsChild = FALSE;
			break;
		default:
			hParent = TreeView_GetParent(_hWndTree, hInsertAfter);
			break;
	}
	// do not move a parent next to its own children!
	if (hItem == hParent)
		return hItem;
	
	// get detailed information about the item to move
	int idx = FindItemIndexByHandle(hItem);
	if (idx < 0)
		return hItem;

	auto &pItem = _pages[idx];

	// item should be inserted as the first child of an existing root item
	if (bAsChild) { 
		tvis.hParent = hInsertAfter;
		tvis.hInsertAfter = (_dwFlags & PSTVF_SORTTREE) ? TVI_SORT : ((bAsChild == 2) ? TVI_LAST : TVI_FIRST);
	}
	// item should be inserted after an existing item
	else {
		tvis.hParent = hParent;
		tvis.hInsertAfter = hInsertAfter;
	}

	// don't move subitems of a protocol to root as this would mean them not to be unique anymore
	if (!_pPs->hContact && (pItem.Flags() & PSPF_PROTOPREPENDED) && !tvis.hParent)
		return hItem;
	
	// prepare the insert structure
	tvis.itemex.mask = TVIF_PARAM|TVIF_TEXT;
	tvis.itemex.state = TVIS_EXPANDED;
	tvis.itemex.stateMask = TVIS_EXPANDED;
	tvis.itemex.pszText = pItem.Label();
	tvis.itemex.lParam = idx;
	if ((tvis.itemex.iImage = tvis.itemex.iSelectedImage = pItem.Image()) >= 0)
		tvis.itemex.mask |= TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	// insert the item
	if (!(hNewItem = TreeView_InsertItem(_hWndTree, &tvis)))
		return hItem;
	// update handle pointer in the page structure
	pItem.Hti(hNewItem);
	// get the index of the parent
	pItem.Parent(FindItemByHandle(tvis.hParent));
	// move children
	hInsertAfter = hNewItem;
	while (hChild = TreeView_GetChild(_hWndTree, hItem))
		MoveItem(hChild, hInsertAfter, 2);

	// delete old tree
	TreeView_DeleteItem(_hWndTree, hItem);
	_dwFlags |= PSTVF_POS_CHANGED;

	TreeView_SelectItem(_hWndTree, hNewItem);
	TreeView_Expand(_hWndTree, hNewItem, TVE_EXPAND);
	return hNewItem;
}

/**
 * name:	SaveItemsState
 * class:	CPsTree
 * desc:	saves the tree's visible items to database
 * param:	pszGroup		- name of the parent item of the current subtree
 *			hRootItem		- the root of the current subtree
 *			iItem			- index of the current item for position saving
 * return:	0 on success or 1 otherwise
 **/

uint16_t CPsTree::SaveItemsState(LPCSTR pszGroup, HTREEITEM hRootItem, int& iItem)
{
	TVITEMEX tvi;
	tvi.mask = TVIF_CHILDREN|TVIF_STATE|TVIF_PARAM;
	tvi.state = 0;
	tvi.stateMask = TVIS_EXPANDED;
	tvi.lParam = (LPARAM)-1;

	// save all visible items
	uint16_t numErrors = 0;
	for (tvi.hItem = TreeView_GetChild(_hWndTree, hRootItem); TreeView_GetItem(_hWndTree, &tvi); tvi.hItem = TreeView_GetNextSibling(_hWndTree, tvi.hItem)) {
		auto &it = _pages[tvi.lParam];
		numErrors += it.DBSaveItemState(pszGroup, iItem++, tvi.state, _dwFlags);
		if (tvi.cChildren)
			numErrors += SaveItemsState(it.Name(), tvi.hItem, iItem);
	}
	return numErrors;
}

/**
 * name:	SaveState
 * class:	CPsTree
 * desc:	saves the current tree to database
 * param:	none
 * return:	nothing
 **/

void CPsTree::SaveState()
{
	CPsTreeItem *pti = CurrentItem();

	if (_hWndTree && (_dwFlags & (PSTVF_LABEL_CHANGED|PSTVF_POS_CHANGED|PSTVF_STATE_CHANGED))) {
		int iItem = 0;

		// save all visible items
		uint16_t numErrors = SaveItemsState(TREE_ROOTITEM, TVGN_ROOT, iItem);

		// save all invisible items of the current subtree
		for (auto &it : _pages) {
			if (!it->Hti()) {
				LPSTR pszGroup;

				if (!it->Parent() || !(pszGroup = it->Parent()->Name()))
					pszGroup = TREE_ROOTITEM;
				numErrors += it->DBSaveItemState(pszGroup, iItem++, DBTVIS_INVISIBLE, _dwFlags);
			}
		}
		// remove changed flags
		RemoveFlags(PSTVF_STATE_CHANGED|PSTVF_LABEL_CHANGED|PSTVF_POS_CHANGED);
	}

	// save current selected item
	if (pti) db_set_utf(0, MODULENAME, SET_LASTITEM, pti->Name());
	else g_plugin.delSetting(SET_LASTITEM);
}

/**
 * name:	DBResetState
 * class:	CPsTree
 * desc:	delete all treesettings from database
 * param:	pszGroup		- name of the parent item of the current subtree
 *			hRootItem		- the root of the current subtree
 *			iItem			- index of the current item for position saving
 * return: 0 on success or 1 otherwise
 **/

void CPsTree::DBResetState()
{
	DB::CEnumList	Settings;

	if (!Settings.EnumSettings(NULL, MODULENAME))
	{
		LPCSTR p = (_pPs->pszProto[0]) ? _pPs->pszProto : "Owner";
		size_t c = mir_strlen(p);

		for (auto &s : Settings)
			if (s && *s == '{' && !mir_strncmpi(s + 1, p, c)) 
				g_plugin.delSetting(s);

		// keep only these flags
		_dwFlags &= PSTVF_SORTTREE|PSTVF_GROUPS;
	}
}

/***********************************************************************************************************
 * public methods for handling label editing
 ***********************************************************************************************************/

/**
 * name:	TPropsheetTree_LabelEditProc()
 * desc:	subclussproc of the label editcontrol
 * return:	0
 **/
static LRESULT CALLBACK TPropsheetTree_LabelEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg) {
	case WM_KEYDOWN:
		switch(wParam) {
		case VK_RETURN:
			return ((CPsTree*)GetUserData(hwnd))->EndLabelEdit(TRUE);
		case VK_TAB:
		case VK_ESCAPE:
			return ((CPsTree*)GetUserData(hwnd))->EndLabelEdit(FALSE);
		}
		break;

	case WM_KILLFOCUS:
		((CPsTree*)GetUserData(hwnd))->EndLabelEdit(FALSE);
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS | mir_callNextSubclass(hwnd, TPropsheetTree_LabelEditProc, uMsg, wParam, lParam );
	}
	return mir_callNextSubclass(hwnd, TPropsheetTree_LabelEditProc, uMsg, wParam, lParam);
}

/**
 * name:	BeginLabelEdit
 * class:	CPsTree
 * desc:	begins the labeledit mode
 * param:	hItem		- handle of the treeitm whose label to edit
 * return:	0
 **/

int CPsTree::BeginLabelEdit(HTREEITEM hItem)
{
	CPsTreeItem* pti;

	// tree is readonly
	if (g_plugin.getByte(SET_PROPSHEET_READONLYLABEL, 0))
		return 0;

	// get item text
	if (!hItem && !(hItem = TreeView_GetSelection(_hWndTree)))
		return 0;
	if (pti = FindItemByHandle(hItem))
		{
		RECT rc, rcTree;

		// create the edit control
		GetClientRect(_hWndTree, &rcTree);
		TreeView_GetItemRect(_hWndTree, hItem, &rc, TRUE);
		_hLabelEdit = CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE,
						_T( "EDIT" ),
						pti->Label(),
						WS_VISIBLE|ES_AUTOHSCROLL|WS_CHILD,
						rc.left, rc.top,
						rcTree.right - rc.left, rc.bottom - rc.top,
						_hWndTree,
						nullptr,
						g_plugin.getInst(),
						nullptr );
		if (_hLabelEdit)
		{
			_hDragItem = hItem;
			SetUserData(_hLabelEdit, this);
			mir_subclassWindow(_hLabelEdit, TPropsheetTree_LabelEditProc);
			SendMessage(_hLabelEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0 );
			Edit_SetSel(_hLabelEdit, 0, -1);
			Edit_LimitText(_hLabelEdit, MAX_TINAME);
			SetFocus(_hLabelEdit);
			return 0;
		}
	}
	return 1;
}

/**
 * name:	EndLabelEdit
 * class:	CPsTree
 * desc:	exits the labeledit mode
 *			bSave		- tell whether to save changes or not
 * return:	0
 **/

int CPsTree::EndLabelEdit(const uint8_t bSave)
{
	wchar_t szEdit[MAX_TINAME];
	TVITEM tvi;
	uint16_t cchEdit;

	if (bSave && (cchEdit = Edit_GetText(_hLabelEdit, szEdit, MAX_TINAME)) > 0)
	{
		tvi.hItem = _hDragItem;
		tvi.mask = TVIF_TEXT|TVIF_HANDLE;
		tvi.pszText = szEdit;
		if (TreeView_SetItem(_hWndTree, &tvi))
		{
			CPsTreeItem* pti;
			if (pti = FindItemByHandle(_hDragItem))
			{
				pti->Rename(szEdit);
			}
			_dwFlags |= PSTVF_LABEL_CHANGED;
		}
	}
	DestroyWindow(_hLabelEdit);
	_hLabelEdit = nullptr;
	_hDragItem = nullptr;
	return 0;
}

/***********************************************************************************************************
 * public methods for handling other commands
 ***********************************************************************************************************/

void CPsTree::PopupMenu()
{
	HMENU hPopup;
	TVHITTESTINFO hti;
	POINT pt;
	int iItem;

	// init popup menu
	if (!(hPopup = CreatePopupMenu()))
		return;

	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STRING | MIIM_ID;

	// get cursor postion
	GetCursorPos(&pt);
	hti.pt = pt;
	ScreenToClient(_hWndTree, &hti.pt);

	TVITEM tvi = { 0 };
	tvi.mask = TVIF_PARAM | TVIF_CHILDREN;
	// find treeitem under cursor
	TreeView_HitTest(_hWndTree, &hti);
	if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
		tvi.hItem = hti.hItem;
		TreeView_GetItem(_hWndTree, &tvi);

		if (!g_plugin.getByte(SET_PROPSHEET_READONLYLABEL, FALSE)) {
			mii.dwTypeData = TranslateT("Rename Item");
			mii.wID = 32001;
			InsertMenuItem(hPopup, 0, FALSE, &mii);
		}

		// do not allow hiding groups
		if (tvi.cChildren) {
			mii.fMask |= MIIM_STATE;
			mii.fState = MFS_DISABLED;
		}
		mii.dwTypeData = TranslateT("Hide Item");
		mii.wID = 32000;
		InsertMenuItem(hPopup, 0, FALSE, &mii);
	}
	else {
		int i = 0;
		// add hidden items to menu
		mii.wID = 0;
		for (auto &it : _pages) {
			if (!it->Hti()) {
				mii.dwTypeData = it->Label();
				mii.wID = 100 + i;
				InsertMenuItem(hPopup, 0, FALSE, &mii);
				i++;
			}
		}
		// add headline
		if (mii.wID > 0) {
			mii.fMask |= MIIM_STATE;
			mii.fState = MFS_DISABLED;
			mii.dwTypeData = TranslateT("Show Items:");
			mii.wID = 0;
			InsertMenuItem(hPopup, 0, TRUE, &mii);
			mii.fMask |= MIIM_FTYPE;
			mii.fType = MFT_SEPARATOR;
			InsertMenuItem(hPopup, 1, TRUE, &mii);
			InsertMenuItem(hPopup, ++i, TRUE, &mii);
		}
		mii.fMask &= ~(MIIM_FTYPE | MIIM_STATE);
		mii.dwTypeData = TranslateT("Reset to defaults");
		mii.wID = 32004;
		InsertMenuItem(hPopup, ++i, TRUE, &mii);
	}
	// show the popup menu
	iItem = TrackPopupMenu(hPopup, TPM_RETURNCMD, pt.x, pt.y, 0, _hWndTree, nullptr);
	DestroyMenu(hPopup);

	switch (iItem) {
	case 32000: // hide the item
		HideItem(tvi.lParam);
		break;
	case 32001: // rename the item
		BeginLabelEdit(tvi.hItem);
		break;
	case 32004: // reset current tree
		DBResetState();
		break;
	default: // show a hidden item
		if ((iItem -= 100) >= 0 && ShowItem(&_pages[iItem], nullptr))
			AddFlags(PSTVF_STATE_CHANGED | PSTVF_POS_CHANGED);
		break;
	}
}

/***********************************************************************************************************
 * public event handlers
 ***********************************************************************************************************/

/**
 * name:	OnIconsChanged
 * class:	CPsTree
 * desc:	is called if icolib's icons have changed
 * param:	none
 * return:	nothing
 **/

void CPsTree::OnIconsChanged()
{
	for (auto &it : _pages)
		it->OnIconsChanged(this);
}

/**
 * name:	OnInfoChanged
 * class:	CPsTree
 * desc:	contact information have changed and pages need an update
 * param:	none
 * return:	TRUE if any page holds changed information
 **/

uint8_t CPsTree::OnInfoChanged()
{
	uint8_t bChanged = 0;

	for (auto &it : _pages) {
		if (it->Wnd() == nullptr)
			continue;

		it->Dialog()->SetContact(it->hContact());
		if (it->Dialog()->OnRefresh())
			bChanged |= 1;
		else
			it->RemoveFlags(PSPF_CHANGED);
	}
	return bChanged;
}

/**
 * name:	OnSelChanging
 * class:	CPsTree
 * desc:	the displayed page is up to change
 * param:	none
 * return:	nothing
 **/
uint8_t CPsTree::OnSelChanging()
{
	CPsTreeItem *pti = CurrentItem();

	if (pti != nullptr) {
		TreeView_SetItemState(_hWndTree, pti->Hti(), 0, TVIS_SELECTED);
		if (pti->Wnd() != nullptr) {
			PSHNOTIFY pshn;

			pshn.hdr.code = PSN_KILLACTIVE;
			pshn.hdr.hwndFrom = pti->Wnd();
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)pti->hContact();
			if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn)) {
				SetWindowLongPtr(_pPs->hDlg, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
 * name:	OnSelChanged
 * class:	CPsTree
 * desc:	it handles the change of displayed page
 * param:	lpnmtv	-	notification structure
 * return:	nothing
 **/
void CPsTree::OnSelChanged(LPNMTREEVIEW lpnmtv)
{
	CPsTreeItem *oldPage = CurrentItem();
	CPsTreeItem *pti;

	_curItem = (int)lpnmtv->itemNew.lParam;
	if (pti = CurrentItem()) {
		if (pti->Wnd() == nullptr) {
			pti->CreateWnd(_pPs);
		}
	}
	// hide old page even if new item has no valid one
	if (oldPage && oldPage->Wnd() != nullptr)
		ShowWindow(oldPage->Wnd(), SW_HIDE);
	if (pti)
		ShowWindow(pti->Wnd(), SW_SHOW);
	if (lpnmtv->action != TVC_BYMOUSE)
		SetFocus(_hWndTree);
}

/**
 * name:	OnCancel
 * class:	CPsTree
 * desc:	asks pages to reset their information
 * param:	none
 * return:	nothing
 **/
void CPsTree::OnCancel()
{
	PSHNOTIFY pshn;
	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_RESET;
	for (auto &it : _pages) {
		pshn.hdr.hwndFrom = it->Wnd();
		if (pshn.hdr.hwndFrom && (it->Flags() & PSPF_CHANGED)) {
			pshn.lParam = (LPARAM)it->hContact();
			SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
		}
	}
}

/**
 * name:	OnApply
 * class:	CPsTree
 * desc:	saves settings of all pages
 * param:	none
 * return:	0 if ready to continue, 1 if need to abort
 **/
int CPsTree::OnApply()
{
	CPsTreeItem *pti = CurrentItem();
	if (pti == nullptr)
		return 1;

	PSHNOTIFY pshn;
	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_KILLACTIVE;
	pshn.hdr.hwndFrom = pti->Wnd();
	if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn))
		return 1;
	// save everything to database
	pshn.hdr.code = PSN_APPLY;
	for (auto &it : _pages) {
		if (it && it->Wnd() && it->Flags() & PSPF_CHANGED) {
			pshn.lParam = (LPARAM)it->hContact();
			pshn.hdr.hwndFrom = it->Wnd();
			if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				if (pti = CurrentItem())
					ShowWindow(pti->Wnd(), SW_HIDE);
				_curItem = _pages.indexOf(&it);
				ShowWindow(it->Wnd(), SW_SHOW);
				return 1;
			}
			it->RemoveFlags(PSPF_CHANGED);
		}
	}
	return 0;
}
