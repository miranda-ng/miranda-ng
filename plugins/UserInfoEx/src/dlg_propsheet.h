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

#pragma once

typedef struct TPropSheet	PS, *LPPS;

class CPsHdr;
class CPsTree;

/***********************************************************************************************************
 * Tree item
 ***********************************************************************************************************/

class CPsTreeItem 
{
	int				_idDlg;			// resource id of the property page
	LPDLGTEMPLATE	_pTemplate;		// locked template for the property page
	HINSTANCE		_hInst;			// instance handle of the owning plugin dll
	DLGPROC			_pfnDlgProc;	// dialog procedure for the property page
	HWND			_hWnd;			// window handle for the property page if shown jet
	DWORD			_dwFlags;		// some flags
	int				_iPosition;		// initiating position if custom (used for sorting)
	LPARAM			_initParam;
	MCONTACT			_hContact;		// contact the page is accociated with (may be a meta subcontact if details dialog is shown for a meta contact)
	LPCSTR			_pszProto;		// protocol the page is accociated with (is the contact's protocol if _hContact is not NULL)
	LPCSTR			_pszPrefix;		// pointer to the dialog owning contact's protocol

	HTREEITEM		_hItem;			// handle to the treeview item if visible (NULL if this item is hidden)
	int				_iParent;		// index of the owning tree item
	int				_iImage;		// index of treeview item's image
	BYTE			_bState;		// initial state of this treeitem
	LPSTR			_pszName;		// original name, given by plugin (not customized)
	LPTSTR			_ptszLabel;		// string to setting in db holding information about this treeitem

	LPCSTR	GlobalName();

	int		Name(LPTSTR pszTitle, const BYTE bIsUnicode);
	int		ItemLabel(const BYTE bReadDBValue);

	HICON	ProtoIcon();
	int		Icon(HIMAGELIST hIml, OPTIONSDIALOGPAGE *odp, BYTE bInitIconsOnly);

public:
	CPsTreeItem();
	~CPsTreeItem();
	int Create(CPsHdr* pPsh, OPTIONSDIALOGPAGE *odp);

	__inline LPSTR Name() const { return _pszName; };
	__inline LPCSTR Proto() const { return _pszProto; };
	__inline LPTSTR Label() const { return _ptszLabel; };
	void Rename( const LPTSTR pszLabel );
	__inline MCONTACT hContact() const { return _hContact; };

	__inline HWND Wnd() const { return _hWnd; };
	__inline int DlgId() const { return _idDlg; };
	__inline HINSTANCE Inst() const { return _hInst; };

	__inline int Image() const { return _iImage; };
	__inline int Pos() const { return _iPosition; };
	__inline BYTE State() const { return _bState; };
	__inline HTREEITEM Hti() const { return _hItem; };
	__inline void Hti(HTREEITEM hti) { _hItem = hti; };
	__inline int Parent() const { return _iParent; };
	__inline void Parent(const int iParent) { _iParent = iParent; };

	__inline DWORD Flags() const { return _dwFlags; };
	__inline void Flags(DWORD dwFlags) { _dwFlags = dwFlags; };
	__inline void AddFlags(DWORD dwFlags) { _dwFlags |= dwFlags; };
	__inline void RemoveFlags(DWORD dwFlags) { _dwFlags &= ~dwFlags; };

	BYTE HasName(const LPCSTR pszName) const;

	LPCSTR PropertyKey(LPCSTR pszProperty);
	LPCSTR GlobalPropertyKey(LPCSTR pszProperty);
	LPCSTR IconKey();

	LPSTR ParentItemName();
	HWND CreateWnd(LPPS pPs);
	
	WORD DBSaveItemState(LPCSTR pszGroup, int iItemPosition, UINT iState, DWORD dwFlags);

	// notification handlers
	void OnInfoChanged();
	void OnPageIconsChanged();
	void OnIconsChanged(CPsTree *pTree);
};


/***********************************************************************************************************
 * Tree control
 ***********************************************************************************************************/

#define MAX_TINAME			64

// internal setting strings
#define SET_LASTITEM		"LastItem"
#define SET_ITEM_LABEL		"label"
#define SET_ITEM_GROUP		"group"
#define SET_ITEM_STATE		"state"
#define SET_ITEM_POS		"pos"
#define TREE_ROOTITEM		"<ROOT>"

// treeview states
#define DBTVIS_INVISIBLE	0
#define DBTVIS_NORMAL		1
#define DBTVIS_EXPANDED		2

// flags for the treeview
#define PSTVF_SORTTREE		0x00010000
#define PSTVF_GROUPS		0x00020000
#define PSTVF_LABEL_CHANGED	0x00040000
#define PSTVF_POS_CHANGED	0x00080000
#define PSTVF_STATE_CHANGED	0x00100000
#define PSTVF_INITICONS		0x00200000		// flag that indicates the AddPage handler to load only basic stuff

class CPsTree {
	HWND					_hWndTree;
	HIMAGELIST				_hImages;
	CPsTreeItem**			_pItems;
	int						_curItem;
	int						_numItems;
	DWORD					_dwFlags;
	HWND					_hLabelEdit;
	HTREEITEM				_hDragItem;
	BYTE					_isDragging;
	LPPS					_pPs;

	WORD	SaveItemsState(LPCSTR pszGroup, HTREEITEM hRootItem, int& iItem);

public:
	CPsTree(LPPS pPs);
	~CPsTree();
	
	__inline void BeginDrag(HTREEITEM hDragItem) { _isDragging = TRUE; _hDragItem = hDragItem; };
	__inline void EndDrag() { _isDragging = FALSE; _hDragItem = NULL; };
	__inline BYTE IsDragging() const { return _isDragging; };
	__inline HTREEITEM DragItem() const { return _hDragItem; };

	__inline DWORD Flags() const { return _dwFlags; };
	__inline void Flags(DWORD dwFlags) { _dwFlags = dwFlags; };
	__inline void AddFlags(DWORD dwFlags) { _dwFlags |= dwFlags; };
	__inline void RemoveFlags(DWORD dwFlags) { _dwFlags &= ~dwFlags; };

	__inline int NumItems() const { return _numItems; };
	__inline HWND Window() const { return _hWndTree; };
	__inline HIMAGELIST ImageList() const { return _hImages; };
	__inline BYTE IsIndexValid(const int index) const { return (index >= 0 && index < _numItems); };
	
	__inline CPsTreeItem* TreeItem(int index) const { return (IsIndexValid(index) ? _pItems[index] : NULL); };
	__inline HTREEITEM TreeItemHandle(int index) const { return (IsIndexValid(index) ? _pItems[index]->Hti() : NULL); };

	__inline int CurrentItemIndex() const { return _curItem; };
	__inline CPsTreeItem* CurrentItem() const { return TreeItem(CurrentItemIndex()); };

	int				AddDummyItem(LPCSTR pszGroup);
	BYTE			Create(HWND hWndTree, CPsHdr* pPsh);
	BYTE			InitTreeItems(LPWORD needWidth);
	
	void			HideItem(const int iPageIndex);
	HTREEITEM ShowItem(const int iPageIndex, LPWORD needWidth);

	HTREEITEM MoveItem(HTREEITEM hItem, HTREEITEM hInsertAfter, BYTE bAsChild = FALSE);
	void			SaveState();
	void			DBResetState();

	int				FindItemIndexByHandle(HTREEITEM hItem);
	int				FindItemIndexByName(LPCSTR pszName);

	CPsTreeItem*	FindItemByName(LPCSTR pszName);
	CPsTreeItem*	FindItemByHandle(HTREEITEM hItem);
	HTREEITEM		FindItemHandleByName(LPCSTR pszName);
	CPsTreeItem*	FindItemByResource(HINSTANCE hInst, int idDlg);

	int				BeginLabelEdit( HTREEITEM hItem );
	int				EndLabelEdit( const BYTE bSave );
	void			PopupMenu();
	
	void			OnIconsChanged();
	BYTE			OnInfoChanged();
	BYTE			OnSelChanging();
	void			OnSelChanged(LPNMTREEVIEW lpnmtv);
	void			OnCancel();
	int				OnApply();
};

/***********************************************************************************************************
 * common stuff
 ***********************************************************************************************************/

// internal flags for the PSP structure
#define PSPF_CHANGED				4096
#define PSPF_PROTOPREPENDED			8192	// the first token of the title is the protocol

#define PSF_PROTOPAGESONLY			64		// load only contact's protocol pages
#define PSF_PROTOPAGESONLY_INIT		128

class CPsUpload;

class CPsHdr 
{
public:
	DWORD			_dwSize;		// size of this class in bytes
	MCONTACT		_hContact;		// handle to the owning contact
	LPCSTR			_pszProto;		// owning contact's protocol 
	LPCSTR			_pszPrefix;		// name prefix for treeitem settings
	CPsTreeItem**	_pPages;		// the pages
	WORD			_numPages;		// number of pages
	DWORD			_dwFlags;		// some option flags
	HIMAGELIST		_hImages;		// the imagelist with all tree item icons
	LIST<TCHAR>		_ignore;		// list of to skipp items when adding metasubcontacts pages
	int				_nSubContact;	// index of a current subcontact

	CPsHdr();
	~CPsHdr();
	
	void Free_pPages();

};

struct TAckInfo 
{
	MCONTACT hContact;
	LPINT acks;
	int count;
};

struct TPropSheet 
{
	// dialogs owner
	MCONTACT	hContact;
	CHAR pszProto[MAXMODULELABELLENGTH];

	HANDLE hProtoAckEvent;		// eventhook for protocol acks
	HANDLE hSettingChanged;	// eventhook searching for changed contact information
	HANDLE hIconsChanged;		// eventhook for changed icons in icolib
	HFONT hCaptionFont;
	HFONT hBoldFont;
	RECT rcDisplay;
	BYTE updateAnimFrame;
	CHAR szUpdating[64];
	DWORD dwFlags;

	TAckInfo	*infosUpdated;
	int nSubContacts;
	
	// controls
	HWND hDlg;
	CPsTree *pTree;
	CPsUpload *pUpload;
};

void	DlgContactInfoInitTreeIcons();
void	DlgContactInfoLoadModule();
void	DlgContactInfoUnLoadModule();
