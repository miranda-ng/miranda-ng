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

#include "commonheaders.h"

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
CPsTree::CPsTree(LPPS pPs)
{
	_hWndTree = NULL;
	_hImages = NULL;

	_pItems = NULL;
	_numItems = 0;
	_curItem = -1;
	_dwFlags = 0;
	_hLabelEdit = NULL;
	_hDragItem = NULL;
	_isDragging = FALSE;
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
	if (_hLabelEdit)
	{
		DestroyWindow(_hLabelEdit);
		_hLabelEdit = NULL;
	}
	if (_pItems) 
	{
		for (int i = 0; i < _numItems; i++) 
		{
			if (_pItems[i])
			{
				delete _pItems[i];
				_pItems[i] = NULL;
			}
		}
		MIR_FREE(_pItems);
		_pItems = NULL;
		_numItems = NULL;
	}
	ImageList_Destroy(_hImages);
	_hImages = NULL;
}

/**
 * name:	CPsTree
 * class:	CPsTree
 * desc:	constructor
 * param:	none
 * return:	none
 **/
BYTE CPsTree::Create(HWND hWndTree, CPsHdr* pPsh)
{
	BYTE rc;

	if (hWndTree && pPsh->_hImages && pPsh->_pPages && pPsh->_numPages)
	{
		_hWndTree = hWndTree;
		_hImages = pPsh->_hImages;
		_pItems = pPsh->_pPages;
		_numItems = pPsh->_numPages;
		_dwFlags = pPsh->_dwFlags;
		
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
int CPsTree::AddDummyItem(LPCSTR pszGroup)
{
	if (mir_stricmp(pszGroup, TREE_ROOTITEM)) 
	{
		CPsHdr psh;
		psh._hContact = _pPs->hContact;
		psh._pszProto = _pPs->pszProto;
		psh._hImages	= _hImages;
		psh._pPages	 = _pItems;
		psh._numPages = _numItems;

		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.hInstance = ghInst;
		odp.flags = ODPF_TCHAR;
		odp.ptszTitle = mir_utf8decodeT(pszGroup);
		
		INT_PTR rc = UserInfo_AddPage((WPARAM)&psh, &odp);
		mir_free(odp.ptszTitle);
		if (!rc) {
			_pItems = psh._pPages;
			_numItems = psh._numPages;
			return _numItems - 1;
		}
	}
	return -1;
}

/**
 * name:	InitTreeItems()
 * desc:	initialize the tree control's datastructure
 * param:	needWidth	- width to expand the tree by
 * return:	TRUE if initialization is ok, FALSE otherwise
 **/
BYTE CPsTree::InitTreeItems(LPWORD needWidth)
{
	int i;
	DBVARIANT dbv;

	if (!_hWndTree || !_pItems)
	{
		return FALSE;
	}

	if (!DB::Setting::GetUString(NULL, MODNAME, SET_LASTITEM, &dbv)) 
	{
		_curItem = FindItemIndexByName(dbv.pszVal);
		db_free(&dbv);
	}

	// init the groups
	if ((_dwFlags & PSTVF_GROUPS) || (!_pPs->hContact && myGlobals.CanChangeDetails)) 
	{
		LPSTR pszGroup;
		
		// set treeview styles
		TreeView_SetIndent(_hWndTree, 3);
		SetWindowLongPtr(_hWndTree, GWL_STYLE, GetWindowLongPtr(_hWndTree, GWL_STYLE)|TVS_HASBUTTONS);

		// init the iParent member for all the items
		for (i = 0; i < _numItems; i++) 
		{
			if (_pItems[i] && (pszGroup = _pItems[i]->ParentItemName()) != NULL) 
			{
				int iParent = FindItemIndexByName(pszGroup);
				
				// need to add an empty parent item
				if (iParent == -1) 
					iParent = AddDummyItem(pszGroup);

				_pItems[i]->Parent(iParent);
				mir_free(pszGroup);
			}
		}
	}

	if (needWidth)
	{
		*needWidth = 0;
	}
	ShowWindow(_hWndTree, SW_HIDE);
	for (i = 0; i < _numItems; i++)
	{
		if (_pItems[i]->State() != DBTVIS_INVISIBLE)
		{
			ShowItem(i, needWidth);
		}
	}
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
	int i;
	for (i = 0; i < _numItems; i++) 
	{
		if (_pItems[i] && hItem == _pItems[i]->Hti())
		{
			return i;
		}
	}
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
	int i;
	for (i = 0; i < _numItems; i++) 
	{
		if (_pItems[i] && _pItems[i]->HasName(pszName))
		{
			return i;
		}
	}
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
	int i;

	if ((i = FindItemIndexByHandle(hItem)) > -1)
	{
		return _pItems[i];
	}
	return NULL;
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
	int i;
	
	if ((i = FindItemIndexByName(pszName)) > -1) 
	{
		return _pItems[i];
	}
	return NULL;
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
	int i;
	for (i = 0; i < _numItems; i++) 
	{
		if (_pItems[i] && _pItems[i]->Inst() == hInst && _pItems[i]->DlgId() == idDlg) 
		{
			return _pItems[i];
		}
	}
	return NULL;
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
	int i;

	if ((i = FindItemIndexByName(pszName)) > -1)
	{
		return _pItems[i]->Hti();
	}
	return NULL;
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
	if (IsIndexValid(iPageIndex)) 
	{
		TreeView_DeleteItem(_hWndTree, _pItems[iPageIndex]->Hti());
		_pItems[iPageIndex]->Hti(0);
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
HTREEITEM CPsTree::ShowItem(const int iPageIndex, LPWORD needWidth)
{
	TVINSERTSTRUCT tvii;
	CPsTreeItem *pti;

	// check parameters
	if (!_hWndTree || 
		!IsIndexValid(iPageIndex) || 
		!(pti = _pItems[iPageIndex]) ||
		!pti->Name() ||
		!pti->Label())
	{
		MsgErr(GetParent(_hWndTree), LPGENT("Due to a parameter error, one of the treeitems can't be added!"));
		return NULL;
	}	
	// item is visible at the moment
	if ((tvii.itemex.hItem = pti->Hti()) == NULL)
	{
		RECT rc;
		const int iParent = pti->Parent();
		
		// init the rest of the treeitem
		tvii.hParent = IsIndexValid(iParent) ? ShowItem(iParent, needWidth) : NULL;
		tvii.hInsertAfter		= (_dwFlags & PSTVF_SORTTREE) ? TVI_SORT : TVI_LAST;
		tvii.itemex.mask		= TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
		tvii.itemex.pszText		= pti->Label();
		tvii.itemex.state		= pti->State() == DBTVIS_EXPANDED ? TVIS_EXPANDED : 0;
		tvii.itemex.stateMask	= TVIS_EXPANDED;
		tvii.itemex.lParam		= iPageIndex;
		// set images
		if ((tvii.itemex.iImage = tvii.itemex.iSelectedImage = pti->Image()) != -1)
		{
			tvii.itemex.mask |= TVIF_IMAGE|TVIF_SELECTEDIMAGE;
		}
		// insert item into tree if set visible
		if ((tvii.itemex.hItem = TreeView_InsertItem(_hWndTree, &tvii)) == NULL) 
		{
			MsgErr(GetParent(_hWndTree), LPGENT("A fatal error occurred on adding a property sheet page!\nDialog creation aborted!"));
			return NULL;
		}
		pti->Hti(tvii.itemex.hItem);
		// calculate width of treeview
		if (needWidth && TreeView_GetItemRect(_hWndTree, pti->Hti(), &rc, TRUE) && rc.right > *needWidth)
		{
			*needWidth = (WORD)rc.right;
		}
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
HTREEITEM CPsTree::MoveItem(HTREEITEM hItem, HTREEITEM hInsertAfter, BYTE bAsChild)
{
	TVINSERTSTRUCT tvis;
	HTREEITEM hParent, hChild, hNewItem;
	int iItemIndex;

	if (!hItem || !hInsertAfter)
		return NULL;
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
	if (FAILED(iItemIndex = FindItemIndexByHandle(hItem)))
		return hItem;
	
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
	if (!_pPs->hContact && (_pItems[iItemIndex]->Flags() & PSPF_PROTOPREPENDED) && !tvis.hParent)
		return hItem;
	
	// prepare the insert structure
	tvis.itemex.mask = TVIF_PARAM|TVIF_TEXT;
	tvis.itemex.state = TVIS_EXPANDED;
	tvis.itemex.stateMask = TVIS_EXPANDED;
	tvis.itemex.pszText = _pItems[iItemIndex]->Label();
	tvis.itemex.lParam = (LPARAM)iItemIndex;
	if ((tvis.itemex.iImage = tvis.itemex.iSelectedImage = _pItems[iItemIndex]->Image()) >= 0)
		tvis.itemex.mask |= TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	// insert the item
	if (!(hNewItem = TreeView_InsertItem(_hWndTree, &tvis)))
		return hItem;
	// update handle pointer in the page structure
	_pItems[iItemIndex]->Hti(hNewItem);
	// get the index of the parent
	_pItems[iItemIndex]->Parent(FindItemIndexByHandle(tvis.hParent));
	// move children
	hInsertAfter = hNewItem;
	while (hChild = TreeView_GetChild(_hWndTree, hItem)) {
		MoveItem(hChild, hInsertAfter, 2);
	}
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
WORD CPsTree::SaveItemsState(LPCSTR pszGroup, HTREEITEM hRootItem, int& iItem)
{
	TVITEMEX tvi;
	WORD numErrors = 0;

	tvi.mask = TVIF_CHILDREN|TVIF_STATE|TVIF_PARAM;
	tvi.state = 0;
	tvi.stateMask = TVIS_EXPANDED;
	tvi.lParam = (LPARAM)-1;

	// save all visible items
	for (tvi.hItem = TreeView_GetChild(_hWndTree, hRootItem);
			 TreeView_GetItem(_hWndTree, &tvi);
			 tvi.hItem = TreeView_GetNextSibling(_hWndTree, tvi.hItem)) 
	{
		numErrors += _pItems[tvi.lParam]->DBSaveItemState(pszGroup, iItem++, tvi.state, _dwFlags);
		if (tvi.cChildren) numErrors += SaveItemsState(_pItems[tvi.lParam]->Name(), tvi.hItem, iItem);
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
		SHORT i;
		int iItem = 0;

		// save all visible items
		WORD numErrors = SaveItemsState(TREE_ROOTITEM, TVGN_ROOT, iItem);

		// save all invisible items of the current subtree
		for (i = 0; i < _numItems; i++) {
			if (!_pItems[i]->Hti()) {
				LPSTR pszGroup;

				if (!IsIndexValid(_pItems[i]->Parent()) || !(pszGroup = _pItems[_pItems[i]->Parent()]->Name()))
					pszGroup = TREE_ROOTITEM;
				numErrors += _pItems[i]->DBSaveItemState(pszGroup, iItem++, DBTVIS_INVISIBLE, _dwFlags);
			}
		}
		// remove changed flags
		RemoveFlags(PSTVF_STATE_CHANGED|PSTVF_LABEL_CHANGED|PSTVF_POS_CHANGED);
	}

	// save current selected item
	if (pti) db_set_utf(NULL, MODNAME, SET_LASTITEM, pti->Name());
	else db_unset(NULL, MODNAME, SET_LASTITEM);
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

	if (!Settings.EnumSettings(NULL, MODNAME))
	{
		int i;
		LPSTR s;
		LPCSTR p;
		INT_PTR c;

		p = (_pPs->pszProto[0]) ? _pPs->pszProto : "Owner";
		c = mir_strlen(p);

		for (i = 0; i < Settings.getCount(); i++)
		{
			s = Settings[i];

			if (s && *s == '{' && !mir_strnicmp(s + 1, p, c)) 
			{
				db_unset(NULL, MODNAME, s);
			}
		}
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
	if (db_get_b(NULL, MODNAME, SET_PROPSHEET_READONLYLABEL, 0))
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
						NULL,
						ghInst,
						NULL );
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
int CPsTree::EndLabelEdit(const BYTE bSave)
{
	TCHAR szEdit[MAX_TINAME];
	TVITEM tvi;
	WORD cchEdit;

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
	_hLabelEdit = NULL;
	_hDragItem = NULL;
	return 0;
}

/***********************************************************************************************************
 * public methods for handling other commands
 ***********************************************************************************************************/

void CPsTree::PopupMenu()
{
	HMENU hPopup;
	MENUITEMINFO mii;
	TVHITTESTINFO hti;
	TVITEM tvi;
	POINT pt;
	int iItem, i;

	// init popup menu
	if (!(hPopup = CreatePopupMenu()))
		return;
	ZeroMemory(&mii, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STRING|MIIM_ID;

	// get cursor postion
	GetCursorPos(&pt);
	hti.pt = pt;
	ScreenToClient(_hWndTree, &hti.pt);

	tvi.mask = TVIF_PARAM|TVIF_CHILDREN;
	// find treeitem under cursor
	TreeView_HitTest(_hWndTree, &hti);
	if (hti.flags & (TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
		tvi.hItem = hti.hItem;
		TreeView_GetItem(_hWndTree, &tvi);

		if (!db_get_b(NULL, MODNAME, SET_PROPSHEET_READONLYLABEL, FALSE)) {
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
		// add hidden items to menu
		mii.wID = 0;
		for (i = 0; i < _numItems; i++) {
			if (!_pItems[i]->Hti()) {
				mii.dwTypeData = _pItems[i]->Label();
				mii.wID = 100 + i;
				InsertMenuItem(hPopup, 0, FALSE, &mii);
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
		mii.fMask &= ~(MIIM_FTYPE|MIIM_STATE);
		mii.dwTypeData = TranslateT("Reset to defaults");
		mii.wID = 32004;
		InsertMenuItem(hPopup, ++i, TRUE, &mii);
	}
	// show the popup menu
	iItem = TrackPopupMenu(hPopup, TPM_RETURNCMD, pt.x, pt.y, 0, _hWndTree, NULL);
	DestroyMenu(hPopup);
	
	switch (iItem) {
		// hide the item
		case 32000:
			HideItem(tvi.lParam);
			break;
		// rename the item
		case 32001:
			BeginLabelEdit(tvi.hItem);
			break;
		// reset current tree
		case 32004:
			DBResetState();
			break;
		// show a hidden item
		default:
			if ((iItem -= 100) >= 0 && ShowItem(iItem, NULL))
				AddFlags(PSTVF_STATE_CHANGED|PSTVF_POS_CHANGED);
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
	for (int i = 0; i < _numItems; i++) {
		_pItems[i]->OnIconsChanged(this);
	}
}

/**
 * name:	OnInfoChanged
 * class:	CPsTree
 * desc:	contact information have changed and pages need an update
 * param:	none
 * return:	TRUE if any page holds changed information
 **/
BYTE CPsTree::OnInfoChanged()
{
	PSHNOTIFY pshn;
	int i;
	BYTE bChanged = 0;

	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_INFOCHANGED;
	for (i = 0; i < _numItems; i++) {
		pshn.hdr.hwndFrom = _pItems[i]->Wnd();
		if (pshn.hdr.hwndFrom != NULL) {
			pshn.lParam = (LPARAM)_pItems[i]->hContact();
			SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn);
			if (PSP_CHANGED == GetWindowLongPtr(pshn.hdr.hwndFrom, DWLP_MSGRESULT))
				bChanged |= 1;
			else
				_pItems[i]->RemoveFlags(PSPF_CHANGED);
		}
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
BYTE CPsTree::OnSelChanging()
{	 
	CPsTreeItem *pti = CurrentItem();
	
	if (pti != NULL) {
		TreeView_SetItemState(_hWndTree, pti->Hti(), 0, TVIS_SELECTED);
		if (pti->Wnd() != NULL) {
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
		if (pti->Wnd() == NULL) {
			pti->CreateWnd(_pPs);
		}
	}
	// hide old page even if new item has no valid one
	if (oldPage && oldPage->Wnd() != NULL)
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
	int i;

	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_RESET;
	for (i = 0; i < _numItems; i++) {
		pshn.hdr.hwndFrom = _pItems[i]->Wnd();
		if (pshn.hdr.hwndFrom && (_pItems[i]->Flags() & PSPF_CHANGED)) {
			pshn.lParam = (LPARAM)_pItems[i]->hContact();
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
	PSHNOTIFY pshn;
	int i;

	if (pti == NULL)
		return 1;

	pshn.hdr.idFrom = 0;
	pshn.hdr.code = PSN_KILLACTIVE;
	pshn.hdr.hwndFrom = pti->Wnd();
	if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn))
		return 1;
	// save everything to database
	pshn.hdr.code = PSN_APPLY;
	for (i = 0; i < _numItems; i++) {
		if (_pItems[i] && _pItems[i]->Wnd() && _pItems[i]->Flags() & PSPF_CHANGED) {
			pshn.lParam = (LPARAM)_pItems[i]->hContact();
			pshn.hdr.hwndFrom = _pItems[i]->Wnd();
			if (SendMessage(pshn.hdr.hwndFrom, WM_NOTIFY, 0, (LPARAM)&pshn) == PSNRET_INVALID_NOCHANGEPAGE) {
				CPsTreeItem *pti;
				if (pti = CurrentItem())
					ShowWindow(pti->Wnd(), SW_HIDE);
				_curItem = i;
				ShowWindow(_pItems[i]->Wnd(), SW_SHOW);
				return 1;
			}
			_pItems[i]->RemoveFlags(PSPF_CHANGED);
		}
	}
	return 0;
}
