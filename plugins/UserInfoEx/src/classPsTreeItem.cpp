/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

/**
 * name:	BoldGroupTitlesEnumChildren()
 * desc:	set font of groupboxes to bold
 *
 * return:	0
 **/
BOOL CALLBACK BoldGroupTitlesEnumChildren(HWND hWnd, LPARAM lParam)
{
	 TCHAR szClass[64];
	 GetClassName(hWnd, szClass, 64);
	 if (!mir_tstrcmp(szClass, _T("Button")) && (GetWindowLongPtr(hWnd, GWL_STYLE) & 0x0F) == BS_GROUPBOX)
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
	_idDlg = NULL;
	_pTemplate = NULL;
	_hInst = NULL;
	_pfnDlgProc = NULL;
	_hWnd = NULL;
	_dwFlags = NULL;
	_hItem = NULL;			// handle to the treeview item
	_iParent = -1;			// index to the parent item
	_iImage = -1;			// index of treeview item's image
	_bState = NULL;			// initial state of this treeitem
	_pszName = NULL;		// original name, given by plugin (not customized)
	_ptszLabel = NULL;
	_pszProto = NULL;
	_pszPrefix = NULL;
	_hContact = NULL;
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
	mir_snprintf(pszSetting, SIZEOF(pszSetting), "{%s\\%s}_%s", _pszPrefix, _pszName, pszProperty);
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
	LPSTR pgn = NULL;
	
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
	mir_snprintf(pszSetting, SIZEOF(pszSetting), "{Global\\%s}_%s", GlobalName(), pszProperty);
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
		mir_snprintf(pszSetting, SIZEOF(pszSetting), MODNAME"_{%s}", pszIconName);
		return pszSetting;
	}
	return NULL;
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
	if (!DB::Setting::GetAString(NULL, MODNAME, PropertyKey(SET_ITEM_GROUP), &dbv))
		return dbv.pszVal;

	const CHAR* p = mir_strrchr(_pszName, '\\');
	if (p) {
		int cchGroup = p - _pszName + 1;
		return mir_strncpy((LPSTR)mir_alloc(cchGroup), _pszName, cchGroup);
	}
	return NULL;
}

/**
 * name:	SetName
 * class:	CPsTreeItem
 * desc:	set the unique name for this item from a given title as it comes with OPTIONDIALOGPAGE
 * param:	ptszTitle		- the title which is the base for the unique name
 *			bIsUnicode		- if TRUE the title is unicode
 * return:	0 on success, 1 to 4 indicating the failed operation
 **/
int CPsTreeItem::Name(LPTSTR ptszTitle, const BYTE bIsUnicode)
{
	// convert title to utf8
	_pszName = (bIsUnicode) ? mir_utf8encodeW((LPWSTR)ptszTitle) : mir_utf8encode((LPSTR)ptszTitle);
	if (_pszName)
	{
		// convert disallowed characters
		for (DWORD i = 0; _pszName[i] != 0; i++) 
		{
			switch (_pszName[i]) 
			{
				case '{': _pszName[i] = '('; break;
				case '}': _pszName[i] = ')'; break;
			}
		}
	}
	return _pszName == NULL;
}

/**
 * name:	HasName
 * class:	CPsTreeItem
 * desc:	returns true, if current item has the name provided by the parameter
 * params:	pszName	- the name to match the item with
 * return:	nothing
 **/
BYTE	CPsTreeItem::HasName(const LPCSTR pszName) const
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
		LPTSTR pszDup = mir_tstrdup(pszLabel);
		if (pszDup) {
			replaceStrT(_ptszLabel, pszDup);
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
int CPsTreeItem::ItemLabel(const BYTE bReadDBValue)
{
	DBVARIANT dbv;

	// clear existing
	if (_ptszLabel)
		mir_free(_ptszLabel);

	// try to get custom label from database
	if (!bReadDBValue || DB::Setting::GetTString(NULL, MODNAME, GlobalPropertyKey(SET_ITEM_LABEL), &dbv) || (_ptszLabel = dbv.ptszVal) == NULL) {
		// extract the name
		LPSTR pszName = mir_strrchr(_pszName, '\\');
		if (pszName && pszName[1])
			pszName++;
		else
			pszName = _pszName;

		LPTSTR ptszLabel = mir_utf8decodeT(pszName);
		if (ptszLabel) {
			_ptszLabel = mir_tstrdup(TranslateTS(ptszLabel));
			mir_free(ptszLabel);
		}
	}
	// return nonezero if label is invalid
	return _ptszLabel == NULL;
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
	PROTOACCOUNT **pa;
	int ProtoCount;
	if (!CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&ProtoCount, (LPARAM)&pa)) {
		if (_pszName) {
			for (int i = 0; i < ProtoCount; i++) {
				if (!mir_tcsnicmp(pa[i]->tszAccountName, _A2T(_pszName), mir_tstrlen(pa[i]->tszAccountName))) {
					CHAR szIconID[MAX_PATH];
					mir_snprintf(szIconID, SIZEOF(szIconID), "core_status_%s1", pa[i]->szModuleName);
					HICON hIco = Skin_GetIcon(szIconID);
					if (!hIco)
						hIco = (HICON)CallProtoService(pa[i]->szModuleName, PS_LOADICON, PLI_PROTOCOL, NULL);

					return hIco;
				}
			}
		}
	}
	return NULL;
}

/**
 * name:	Icon
 * class:	CPsTreeItem
 * desc:	load the icon, add to icolib if required and add to imagelist of treeview
 * params:	hIml			- treeview's imagelist to add the icon to
 *			odp				- pointer to OPTIONSDIALOGPAGE providing the information about the icon to load
 *			hDefaultIcon	- default icon to use
 * return: nothing
 **/
int CPsTreeItem::Icon(HIMAGELIST hIml, OPTIONSDIALOGPAGE *odp, BYTE bInitIconsOnly)
{
	HICON hIcon;

	// check parameter
	if (!_pszName || !odp)
		return 1;

	// load the icon if no icolib is installed or creating the required settingname failed
	LPCSTR pszIconName = IconKey();

	// use icolib to handle icons
	if (!(hIcon = Skin_GetIcon(pszIconName))) {
		bool bNeedFree = false;

		SKINICONDESC sid = { 0 };
		sid.flags = SIDF_ALL_TCHAR;
		sid.cx = GetSystemMetrics(SM_CXSMICON);
		sid.cy = GetSystemMetrics(SM_CYSMICON);
		sid.pszName = (LPSTR)pszIconName;
		sid.description.t = _ptszLabel;
		sid.section.t = LPGENT(SECT_TREE);

		// the item to insert brings along an icon?
		if (odp->flags & ODPF_ICON) {
			// is it uinfoex item?
			if (odp->hInstance == ghInst) {

				// the pszGroup holds the iconfile for items added by uinfoex
				sid.defaultFile.t = odp->ptszGroup;

				// icon library exists?
				if (sid.defaultFile.t)
					sid.iDefaultIndex = (INT_PTR)odp->hIcon;
				// no valid icon library
				else {
					bNeedFree = true;
					sid.hDefaultIcon = ImageList_GetIcon(hIml, 0, ILD_NORMAL);
					sid.iDefaultIndex = -1;
				}
			}
			// default icon is delivered by the page to add
			else {
				if (odp->hIcon)
					sid.hDefaultIcon = odp->hIcon;
				else
					sid.hDefaultIcon = ImageList_GetIcon(hIml, 0, ILD_NORMAL), bNeedFree = true;
				sid.iDefaultIndex = -1;
			}
		}
		// no icon to add, use default
		else {
			sid.iDefaultIndex = -1;
			sid.hDefaultIcon = ProtoIcon();
			if (!sid.hDefaultIcon)
				sid.hDefaultIcon = ImageList_GetIcon(hIml, 0, ILD_NORMAL), bNeedFree = true;
		}
		// add file to icolib
		Skin_AddIcon(&sid);

		if (!bInitIconsOnly)
			hIcon = Skin_GetIcon(pszIconName);
		if (bNeedFree)
			DestroyIcon(sid.hDefaultIcon);
	}
	
	if (!bInitIconsOnly && hIml) {
		// set custom icon to image list
		if (hIcon) return ((_iImage = ImageList_AddIcon(hIml, hIcon)) == -1);
		_iImage = 0;
	}
	else
		_iImage = -1;
	return 0;
}

/**
 * name:	OnAddPage
 * class:	CPsTreeItem
 * desc:	inits the treeitem's attributes
 * params:	pPsh	- pointer to the property page's header structure
 *			odp		- OPTIONSDIALOGPAGE structure with the information about the page to add
 * return:	0 on success, 1 on failure
 **/
int CPsTreeItem::Create(CPsHdr* pPsh, OPTIONSDIALOGPAGE *odp)
{
	int err;
	TCHAR szTitle[ MAXSETTING ];

	// check parameter
	if (pPsh && pPsh->_dwSize == sizeof(CPsHdr) && odp && PtrIsValid(odp->hInstance)) {
		// instance value
		_hInst = odp->hInstance;
		_dwFlags = odp->flags;
		_initParam = odp->dwInitParam;

		// init page owning contact
		_hContact = pPsh->_hContact;
		_pszProto = mir_strdup(pPsh->_pszProto);

		// global settings prefix for current contact (is dialog owning contact's protocol by default)
		_pszPrefix = (pPsh->_pszPrefix) ? pPsh->_pszPrefix : "Owner";

		if (pPsh->_dwFlags & PSF_PROTOPAGESONLY) {
			if (_dwFlags & ODPF_USERINFOTAB)
				mir_sntprintf(szTitle, _T("%s %d\\%s"), odp->ptszTitle, pPsh->_nSubContact+1, odp->ptszTab);
			else
				mir_sntprintf(szTitle, _T("%s %d"), odp->ptszTitle, pPsh->_nSubContact+1);
		}
		else {
			if (_dwFlags & ODPF_USERINFOTAB)
				mir_sntprintf(szTitle, _T("%s\\%s"), odp->ptszTitle, odp->ptszTab);
			else
				mir_tstrcpy(szTitle, odp->ptszTitle);
		}
		// set the unique utf8 encoded name for the item
		if (err = Name(szTitle, (_dwFlags & ODPF_UNICODE) == ODPF_UNICODE)) 
			MsgErr(NULL, LPGENT("Creating unique name for a page failed with %d and error code %d"), err, GetLastError());

		// read label from database or create it
		else if (err = ItemLabel(TRUE)) 
			MsgErr(NULL, LPGENT("Creating the label for a page failed with %d and error code %d"), err, GetLastError());
		else {
			// load icon for the item
			Icon(pPsh->_hImages, odp, (pPsh->_dwFlags & PSTVF_INITICONS) == PSTVF_INITICONS);
			
			// the rest is not needed if only icons are loaded
			if (pPsh->_dwFlags & PSTVF_INITICONS)
				return 0;

			// load custom order
			if (!(pPsh->_dwFlags & PSTVF_SORTTREE)) {
				_iPosition = (int)db_get_b(NULL, MODNAME, PropertyKey(SET_ITEM_POS), odp->position);
				if ((_iPosition < 0) && (_iPosition > 0x800000A))
					_iPosition = 0;
			}
			// read visibility state
			_bState =	db_get_b(NULL, MODNAME, PropertyKey(SET_ITEM_STATE), DBTVIS_EXPANDED);

			// error for no longer supported dialog template type
			if (((UINT_PTR)odp->pszTemplate & 0xFFFF0000)) 
				MsgErr(NULL, LPGENT("The dialog template type is no longer supported"));
			else {
				// fetch dialog resource id
				_idDlg = (INT_PTR)odp->pszTemplate;
				// dialog procedure
				_pfnDlgProc = odp->pfnDlgProc;

				// is dummy item?
				if (!_idDlg	&& !_pfnDlgProc)
					return 0;

				if (_idDlg	&& _pfnDlgProc) {
					// lock the property pages dialog resource
					_pTemplate = (DLGTEMPLATE*)LockResource(LoadResource(_hInst, FindResource(_hInst, (LPCTSTR)(UINT_PTR)_idDlg, RT_DIALOG)));
					if (_pTemplate)
						return 0;
				}
			}
		}
	}
	return 1;
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
WORD CPsTreeItem::DBSaveItemState(LPCSTR pszGroup, int iItemPosition, UINT iState, DWORD dwFlags)
{
	WORD numErrors = 0;

	// save group
	if ((dwFlags & PSTVF_GROUPS) && (dwFlags & PSTVF_POS_CHANGED))
		numErrors += db_set_utf(NULL, MODNAME, PropertyKey(SET_ITEM_GROUP), (LPSTR)pszGroup);

	// save label
	if ((dwFlags & PSTVF_LABEL_CHANGED) && (_dwFlags & PSTVF_LABEL_CHANGED))
		numErrors += db_set_ts(NULL, MODNAME, GlobalPropertyKey(SET_ITEM_LABEL), Label());

	// save position
	if ((dwFlags & PSTVF_POS_CHANGED) && !(dwFlags & PSTVF_SORTTREE))
		numErrors += db_set_b(NULL, MODNAME, PropertyKey(SET_ITEM_POS), iItemPosition);

	// save state
	if (dwFlags & PSTVF_STATE_CHANGED)
		numErrors += db_set_b(NULL, MODNAME, PropertyKey(SET_ITEM_STATE), 
			_hItem ? ((iState & TVIS_EXPANDED) ? DBTVIS_EXPANDED : DBTVIS_NORMAL) : DBTVIS_INVISIBLE);

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
	if (pPs && !_hWnd && _pTemplate && _pfnDlgProc) {
		_hWnd = CreateDialogIndirectParam(_hInst, _pTemplate, pPs->hDlg, _pfnDlgProc, (LPARAM)_hContact);
		if (_hWnd != NULL) {
			PSHNOTIFY pshn;
			pshn.hdr.code = PSN_PARAMCHANGED;
			pshn.hdr.hwndFrom = _hWnd;
			pshn.hdr.idFrom = 0;
			pshn.lParam = (LPARAM)_initParam;
			SendMessage(_hWnd, WM_NOTIFY, 0, (LPARAM)&pshn);

			// force child window (mainly for AIM property page)
			SetWindowLongPtr(_hWnd, GWL_STYLE, (GetWindowLongPtr(_hWnd, GWL_STYLE) & ~(WS_POPUP|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME)) | WS_CHILD);
			SetWindowLongPtr(_hWnd, GWL_EXSTYLE, GetWindowLongPtr(_hWnd, GWL_EXSTYLE) & ~(WS_EX_APPWINDOW|WS_EX_STATICEDGE|WS_EX_CLIENTEDGE));
			SetParent(_hWnd, pPs->hDlg);

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
	return NULL;
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
	if (_hWnd) {
		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_INFOCHANGED;
		pshn.hdr.hwndFrom = _hWnd;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)_hContact;
		if (PSP_CHANGED != SendMessage(_hWnd, WM_NOTIFY, 0, (LPARAM)&pshn))
			_dwFlags &= ~PSPF_CHANGED;
	}
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
	if (_hWnd) {
		PSHNOTIFY pshn;
		pshn.hdr.code = PSN_ICONCHANGED;
		pshn.hdr.hwndFrom = _hWnd;
		pshn.hdr.idFrom = 0;
		pshn.lParam = (LPARAM)_hContact;
		SendMessage(_hWnd, WM_NOTIFY, 0, (LPARAM)&pshn);
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
	if (pTree->ImageList() && (hIcon = Skin_GetIcon(IconKey())) != NULL) {
		_iImage = (_iImage > 0)
			? ImageList_ReplaceIcon(pTree->ImageList(), _iImage, hIcon)
			: ImageList_AddIcon(pTree->ImageList(), hIcon);
		
		if (_hItem && TreeView_GetItemRect(pTree->Window(), _hItem, &rc, 0))
			InvalidateRect(pTree->Window(), &rc, TRUE);
	}
	// update pages icons
	OnPageIconsChanged();
}
