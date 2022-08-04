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

/**
 * name:	BoldGroupTitlesEnumChildren()
 * desc:	set font of groupboxes to bold
 *
 * return:	0
 **/
BOOL CALLBACK BoldGroupTitlesEnumChildren(HWND hWnd, LPARAM lParam)
{
	 wchar_t szClass[64];
	 GetClassName(hWnd, szClass, 64);
	 if (!mir_wstrcmp(szClass, L"Button") && (GetWindowLongPtr(hWnd, GWL_STYLE) & 0x0F) == BS_GROUPBOX)
			SendMessage(hWnd, WM_SETFONT, lParam, NULL);
	 return TRUE;
}

/**
 * name:	CPsTreeItem
 * class:	CPsTreeItem
 * desc:	default constructor
 * param:	pPsh		- propertysheet's init structure
 *			odp			- optiondialogpage structure with the info about the item to add
 * return: nothing
 **/
CPsTreeItem::CPsTreeItem()
{
}

/**
 * name:	~CPsTreeItem
 * class:	CPsTreeItem
 * desc:	default destructor
 * param:	none
 * return:	nothing
 **/
CPsTreeItem::~CPsTreeItem()
{
	if (_hWnd)		DestroyWindow(_hWnd);
	if (_pszName)	mir_free(_pszName);
	if (_ptszLabel)	mir_free(_ptszLabel);
	if (_pszProto)	mir_free((LPVOID)_pszProto);
}

/**
 * name:	PropertyKey
 * class:	CPsTreeItem
 * desc:	merge the treeitem's unique name with a prefix to form a setting string
 * param:	pszProperty - the prefix to add
 * return:	pointer to the setting string
 **/
LPCSTR CPsTreeItem::PropertyKey(LPCSTR pszProperty)
{
	static CHAR pszSetting[MAXSETTING];
	mir_snprintf(pszSetting, "{%s\\%s}_%s", _pszPrefix, _pszName, pszProperty);
	return pszSetting;
}

/**
 * name:	GlobalName
 * class:	CPsTreeItem
 * desc:	return item name without prepended protocol name
 * param:	nothing
 * return:	item name without protocol name
 **/
LPCSTR CPsTreeItem::GlobalName()
{
	LPSTR pgn = nullptr;
	
	if (_dwFlags & PSPF_PROTOPREPENDED) {
		pgn = mir_strchr(_pszName, '\\');
		if (pgn && pgn[1])
			pgn++;
	}
	return (!pgn || !*pgn) ?_pszName : pgn;
}

/**
 * name:	GlobalPropertyKey
 * class:	CPsTreeItem
 * desc:	merge the treeitem's unique name with a prefix to form a setting string
 * param:	pszProperty - the prefix to add
 * return:	pointer to the setting string
 **/
LPCSTR CPsTreeItem::GlobalPropertyKey(LPCSTR pszProperty)
{
	static CHAR pszSetting[MAXSETTING];
	mir_snprintf(pszSetting, "{Global\\%s}_%s", GlobalName(), pszProperty);
	return pszSetting;
}

/**
 * name:	IconKey
 * class:	CPsTreeItem
 * desc:	merge the treeitem's unique name with a prefix to form a setting string
 * param:	pszProperty			- the prefix to add
 * return:	pointer to the setting string
 **/
LPCSTR CPsTreeItem::IconKey()
{
	LPCSTR pszIconName = GlobalName();
	if (pszIconName)
	{
		static CHAR pszSetting[MAXSETTING];
		mir_snprintf(pszSetting, MODULENAME"_{%s}", pszIconName);
		return pszSetting;
	}
	return nullptr;
}

/**
 * name:	ParentItemName()
 * class:	CPsTreeItem
 * desc:	returns the unique name for the parent item
 * param:	nothing
 * return:	length of group name
 **/
LPSTR CPsTreeItem::ParentItemName()
{
	// try to read the parent item from the database
	DBVARIANT dbv;
	if (!DB::Setting::GetAString(0, MODULENAME, PropertyKey(SET_ITEM_GROUP), &dbv))
		return dbv.pszVal;

	const CHAR* p = mir_strrchr(_pszName, '\\');
	if (p) {
		int cchGroup = p - _pszName + 1;
		return mir_strncpy((LPSTR)mir_alloc(cchGroup), _pszName, cchGroup);
	}
	return nullptr;
}

/**
 * name:	SetName
 * class:	CPsTreeItem
 * desc:	set the unique name for this item from a given title as it comes with OPTIONDIALOGPAGE
 * param:	ptszTitle		- the title which is the base for the unique name
 *			bIsUnicode		- if TRUE the title is unicode
 * return:	0 on success, 1 to 4 indicating the failed operation
 **/
int CPsTreeItem::Name(const wchar_t *ptszTitle, bool bIsUnicode)
{
	// convert title to utf8
	_pszName = (bIsUnicode) ? mir_utf8encodeW((LPWSTR)ptszTitle) : mir_utf8encode((LPSTR)ptszTitle);
	if (_pszName) {
		// convert disallowed characters
		for (uint32_t i = 0; _pszName[i] != 0; i++) {
			switch (_pszName[i]) {
			case '{': _pszName[i] = '('; break;
			case '}': _pszName[i] = ')'; break;
			}
		}
	}
	return _pszName == nullptr;
}

/**
 * name:	HasName
 * class:	CPsTreeItem
 * desc:	returns true, if current item has the name provided by the parameter
 * params:	pszName	- the name to match the item with
 * return:	nothing
 **/
uint8_t	CPsTreeItem::HasName(const LPCSTR pszName) const
{ 
	return !mir_strcmpi(_pszName, pszName); 
};

/**
 * name:	Rename
 * class:	CPsTreeItem
 * desc:	Rename label for the treeitem
 * params:	pszLabel	- the desired new label
 * return:	nothing
 **/
void CPsTreeItem::Rename(const LPTSTR pszLabel)
{
	if (pszLabel && *pszLabel) {
		LPTSTR pszDup = mir_wstrdup(pszLabel);
		if (pszDup) {
			if (_ptszLabel)
				mir_free(_ptszLabel);
			_ptszLabel =  pszDup;
			// convert disallowed characters
			while(*pszDup) {
				switch(*pszDup) {
					case '{': *pszDup = '('; break;
					case '}': *pszDup = ')'; break;
				}
				pszDup++;
			}
			AddFlags(PSTVF_LABEL_CHANGED);
		}
	}
}

/**
 * name:	ItemLabel
 * class:	CPsTreeItem
 * desc:	returns the label for a given item. The returned value must be freed after use!
 * param:	pszName		- uniquely identifiing string for a propertypage encoded with utf8 (e.g.: {group\item})
 * return:	Label in a newly allocated piece of memory
 **/
int CPsTreeItem::ItemLabel(bool bReadDBValue)
{
	// try to get custom label from database
	DBVARIANT dbv;
	if (!bReadDBValue || DB::Setting::GetWString(0, MODULENAME, GlobalPropertyKey(SET_ITEM_LABEL), &dbv) || (_ptszLabel = dbv.pwszVal) == nullptr) {
		// extract the name
		char *pszName = mir_strrchr(_pszName, '\\');
		if (pszName && pszName[1])
			pszName++;
		else
			pszName = _pszName;

		ptrW ptszLabel(mir_utf8decodeW(pszName));
		if (ptszLabel)
			replaceStrW(_ptszLabel, TranslateW(ptszLabel));
	}
	// return nonezero if label is invalid
	return _ptszLabel == nullptr;
}

/**
 * name:	ProtoIcon
 * class:	CPsTreeItem
 * desc:	check if current tree item name is a protocol name and return its icon if so
 * params:	none
 * return:	nothing
 **/
HICON CPsTreeItem::ProtoIcon()
{
	if (!_pszName)
		return nullptr;

	for (auto &pa : Accounts()) {
		if (!mir_wstrncmpi(pa->tszAccountName, _A2T(_pszName), mir_wstrlen(pa->tszAccountName))) {
			CHAR szIconID[MAX_PATH];
			mir_snprintf(szIconID, "core_status_%s1", pa->szModuleName);
			HICON hIco = IcoLib_GetIcon(szIconID);
			if (!hIco)
				hIco = (HICON)CallProtoService(pa->szModuleName, PS_LOADICON, PLI_PROTOCOL, NULL);

			return hIco;
		}
	}
	return nullptr;
}

/**
 * name:	Icon
 * class:	CPsTreeItem
 * desc:	load the icon, add to icolib if required and add to imagelist of treeview
 * params:	hIml			- treeview's imagelist to add the icon to
 *			odp				- pointer to USERINFOPAGE providing the information about the icon to load
 *			hDefaultIcon	- default icon to use
 * return: nothing
 **/
int CPsTreeItem::Icon(HIMAGELIST hIml, USERINFOPAGE *uip, bool bInitIconsOnly)
{
	// check parameter
	if (!_pszName || !uip)
		return 1;

	// load the icon if no icolib is installed or creating the required settingname failed
	LPCSTR pszIconName = IconKey();
	HANDLE hIcoLib = nullptr;

	// use icolib to handle icons
	HICON hIcon = IcoLib_GetIcon(pszIconName);
	if (!hIcon) {
		bool bNeedFree = false;
		ptrW pwszSection(mir_a2u(SECT_TREE));

		SKINICONDESC sid = {};
		sid.flags = SIDF_ALL_UNICODE;
		sid.pszName = (LPSTR)pszIconName;
		sid.description.w = _ptszLabel;
		sid.section.w = pwszSection;

		// the item to insert brings along an icon?
		if (uip->flags & ODPF_ICON) {
			// is it uinfoex item?
			if (uip->pPlugin == &g_plugin) {
				// the pszGroup holds the iconfile for items added by uinfoex
				sid.defaultFile.w = uip->szGroup.w;

				// icon library exists?
				if (sid.defaultFile.w)
					sid.iDefaultIndex = uip->dwInitParam;
				// no valid icon library
				else {
					bNeedFree = true;
					sid.hDefaultIcon = ImageList_GetIcon(hIml, 0, ILD_NORMAL);
					sid.iDefaultIndex = -1;
				}
			}
			else hIcoLib = (HANDLE)uip->dwInitParam;
		}
		// no icon to add, use default
		else {
			sid.iDefaultIndex = -1;
			sid.hDefaultIcon = ProtoIcon();
			if (!sid.hDefaultIcon)
				sid.hDefaultIcon = ImageList_GetIcon(hIml, 0, ILD_NORMAL), bNeedFree = true;
		}
		// add file to icolib
		if (!hIcoLib)
			g_plugin.addIcon(&sid);

		if (!bInitIconsOnly)
			hIcon = (hIcoLib) ? IcoLib_GetIconByHandle(hIcoLib) : IcoLib_GetIcon(pszIconName);
		if (bNeedFree)
			DestroyIcon(sid.hDefaultIcon);
	}
	
	if (!bInitIconsOnly && hIml) {
		// set custom icon to image list
		if (hIcon) return ((_iImage = ImageList_AddIcon(hIml, hIcon)) == -1);
		_iImage = 0;
	}
	else _iImage = -1;

	if (hIcoLib)
		IcoLib_ReleaseIcon(hIcon);
	return 0;
}

/**
 * name:	OnAddPage
 * class:	CPsTreeItem
 * desc:	inits the treeitem's attributes
 * params:	pPsh	- pointer to the property page's header structure
 *			odp		- USERINFOPAGE structure with the information about the page to add
 * return:	0 on success, 1 on failure
 **/
int CPsTreeItem::Create(CPsHdr *pPsh, USERINFOPAGE *uip)
{
	// check parameter
	if (!pPsh || !uip || !PtrIsValid(uip->pPlugin))
		return 1;

	// instance value
	_hInst = uip->pPlugin->getInst();
	_dwFlags = uip->flags;
	_initParam = uip->dwInitParam;

	// init page owning contact
	_hContact = pPsh->_hContact;
	_pszProto = mir_strdup(pPsh->_pszProto);

	// global settings prefix for current contact (is dialog owning contact's protocol by default)
	_pszPrefix = (pPsh->_pszPrefix) ? pPsh->_pszPrefix : "Owner";

	CMStringW wszTitle;
	if (_dwFlags & ODPF_USERINFOTAB) {
		wszTitle.Append(uip->szGroup.w);
		wszTitle.AppendChar('\\');
	}

	wszTitle.Append(uip->szTitle.w);
	if (pPsh->_dwFlags & PSF_PROTOPAGESONLY)
		wszTitle.AppendFormat(L" %d", pPsh->_nSubContact + 1);

	// set the unique utf8 encoded name for the item
	if (int err = Name(wszTitle, (_dwFlags & ODPF_UNICODE) == ODPF_UNICODE)) {
		MsgErr(nullptr, LPGENW("Creating unique name for a page failed with %d and error code %d"), err, GetLastError());
		return 1;
	}

	// read label from database or create it
	if (int err = ItemLabel(TRUE)) {
		MsgErr(nullptr, LPGENW("Creating the label for a page failed with %d and error code %d"), err, GetLastError());
		return 1;
	}

	// load icon for the item
	Icon(pPsh->_hImages, uip, (pPsh->_dwFlags & PSTVF_INITICONS) == PSTVF_INITICONS);
			
	// the rest is not needed if only icons are loaded
	if (pPsh->_dwFlags & PSTVF_INITICONS)
		return 0;

	// load custom order
	if (!(pPsh->_dwFlags & PSTVF_SORTTREE)) {
		_iPosition = g_plugin.getByte(PropertyKey(SET_ITEM_POS), uip->position);
		if ((_iPosition < 0) || (_iPosition > 0x800000A))
			_iPosition = 0;
	}
	// read visibility state
	_bState = g_plugin.getByte(PropertyKey(SET_ITEM_STATE), DBTVIS_EXPANDED);

	// fetch dialog 
	_pDialog = uip->pDialog;
	return 0;
}

/**
 * name:	DBSaveItemState
 * class:	CPsTreeItem
 * desc:	saves the current treeitem to database
 * param:	pszGroup			- name of the parent item
 *			iItemPosition		- iterated index to remember the order of the tree
 *			iState				- expanded|collapsed|invisible
 *			dwFlags				- tells what to save
 * return:	handle to new (moved) treeitem if successful or NULL otherwise
 **/
uint16_t CPsTreeItem::DBSaveItemState(LPCSTR pszGroup, int iItemPosition, UINT iState, uint32_t dwFlags)
{
	uint16_t numErrors = 0;

	// save group
	if ((dwFlags & PSTVF_GROUPS) && (dwFlags & PSTVF_POS_CHANGED))
		numErrors += db_set_utf(0, MODULENAME, PropertyKey(SET_ITEM_GROUP), (LPSTR)pszGroup);

	// save label
	if ((dwFlags & PSTVF_LABEL_CHANGED) && (_dwFlags & PSTVF_LABEL_CHANGED))
		g_plugin.setWString(GlobalPropertyKey(SET_ITEM_LABEL), Label());

	// save position
	if ((dwFlags & PSTVF_POS_CHANGED) && !(dwFlags & PSTVF_SORTTREE))
		g_plugin.setByte(PropertyKey(SET_ITEM_POS), iItemPosition);

	// save state
	if (dwFlags & PSTVF_STATE_CHANGED)
		g_plugin.setByte(PropertyKey(SET_ITEM_STATE), _hItem ? ((iState & TVIS_EXPANDED) ? DBTVIS_EXPANDED : DBTVIS_NORMAL) : DBTVIS_INVISIBLE);

	RemoveFlags(PSTVF_STATE_CHANGED|PSTVF_LABEL_CHANGED|PSTVF_POS_CHANGED);
	return numErrors;
}

/**
 * name:	CreateWnd
 * class:	CPsTreeItem
 * desc:	create the dialog for the propertysheet page
 * params:	pPs		- propertysheet's datastructure
 *			hDlg	- windowhandle of the propertysheet
 * return:	windowhandle of the dialog if successful
 **/
HWND CPsTreeItem::CreateWnd(LPPS pPs)
{
	if (pPs && !_hWnd && _pDialog) {
		_pDialog->SetParent(pPs->hDlg);
		_pDialog->SetContact(_hContact);
		_pDialog->Create();
		_hWnd = _pDialog->GetHwnd();
		if (_hWnd != nullptr) {
			// force child window (mainly for AIM property page)
			SetWindowLongPtr(_hWnd, GWL_STYLE, (GetWindowLongPtr(_hWnd, GWL_STYLE) & ~(WS_POPUP|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME)) | WS_CHILD);
			SetWindowLongPtr(_hWnd, GWL_EXSTYLE, GetWindowLongPtr(_hWnd, GWL_EXSTYLE) & ~(WS_EX_APPWINDOW|WS_EX_STATICEDGE|WS_EX_CLIENTEDGE));

			// move dialog into the display area
			SetWindowPos(_hWnd, HWND_TOP, 
				pPs->rcDisplay.left,	pPs->rcDisplay.top,
				pPs->rcDisplay.right - pPs->rcDisplay.left,	
				pPs->rcDisplay.bottom - pPs->rcDisplay.top,	FALSE);
			// set bold titles
			if (_dwFlags & ODPF_BOLDGROUPS)
				EnumChildWindows(_hWnd, BoldGroupTitlesEnumChildren, (LPARAM)pPs->hBoldFont);
						
			// some initial notifications
			OnInfoChanged();
			OnPageIconsChanged();
			return _hWnd;
		}
	}
	return nullptr;
}

/***********************************************************************************************************
 * public event handlers
 ***********************************************************************************************************/

/**
 * name:	OnInfoChanged
 * class:	CPsTreeItem
 * desc:	Notifies the propertypage of changed contact information
 * params:	none
 * return:	nothing
 **/
void CPsTreeItem::OnInfoChanged()
{
	if (_hWnd)
		_pDialog->OnRefresh();
}

/**
 * name:	OnPageIconsChanged
 * class:	CPsTreeItem
 * desc:	Notifies the propertypage of changed icons in icolib
 * params:	none
 * return:	nothing
 **/
void CPsTreeItem::OnPageIconsChanged()
{
	if (_hWnd && _hInst == g_plugin.getInst()) {
		auto *pDlg = (PSPBaseDlg *)_pDialog;
		pDlg->OnIconsChanged();
	}
}

/**
 * name:	OnIconsChanged
 * class:	CPsTreeItem
 * desc:	Handles reloading icons if changed by icolib
 * params:	none
 * return:	nothing
 **/
void CPsTreeItem::OnIconsChanged(CPsTree *pTree)
{
	HICON hIcon;
	RECT rc;

	// update tree item icons
	if (pTree->ImageList() && (hIcon = IcoLib_GetIcon(IconKey())) != nullptr) {
		_iImage = (_iImage > 0)
			? ImageList_ReplaceIcon(pTree->ImageList(), _iImage, hIcon)
			: ImageList_AddIcon(pTree->ImageList(), hIcon);
		
		if (_hItem && TreeView_GetItemRect(pTree->Window(), _hItem, &rc, 0))
			InvalidateRect(pTree->Window(), &rc, TRUE);
	}
	// update pages icons
	OnPageIconsChanged();
}
