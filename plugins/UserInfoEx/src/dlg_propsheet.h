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

struct CPsHdr;
class CPsTree;

/***********************************************************************************************************
 * Tree item
 ***********************************************************************************************************/

class CPsTreeItem 
{
	int               _idDlg = 0;     // resource id of the property page
	HINSTANCE         _hInst = 0;     // instance handle of the owning plugin dll
	CUserInfoPageDlg *_pDialog = 0;   // pointer to a page dialog
	HWND              _hWnd = 0;      // window handle for the property page if shown jet
	uint32_t          _dwFlags = 0;   // some flags
	int               _iPosition = 0; // initiating position if custom (used for sorting)
	MCONTACT          _hContact = 0;  // contact the page is accociated with (may be a meta subcontact if details dialog is shown for a meta contact)
	const char*       _pszProto = 0;  // protocol the page is accociated with (is the contact's protocol if _hContact is not NULL)
	const char*       _pszPrefix = 0; // pointer to the dialog owning contact's protocol
	INT_PTR           _initParam = 0;
					      
	HTREEITEM         _hItem = 0;     // handle to the treeview item if visible (NULL if this item is hidden)
	CPsTreeItem*      _pParent = 0;   // owning tree item
	int               _iImage = -1;   // index of treeview item's image
	uint8_t           _bState = 0;    // initial state of this treeitem
	char*             _pszName = 0;   // original name, given by plugin (not customized)
	wchar_t*          _ptszLabel = 0; // string to setting in db holding information about this treeitem

	LPCSTR GlobalName();

	int Icon(HIMAGELIST hIml, USERINFOPAGE *uip, bool bInitIconsOnly);
	int ItemLabel(bool bReadDBValue);
	int Name(const wchar_t *pszTitle, bool bIsUnicode);
	HICON	ProtoIcon();

public:
	CPsTreeItem();
	~CPsTreeItem();

	int Create(CPsHdr* pPsh, USERINFOPAGE *uip);
	void Rename(const LPTSTR pszLabel);

	__inline LPSTR Name() const { return _pszName; }
	__inline LPCSTR Proto() const { return _pszProto; }
	__inline LPTSTR Label() const { return _ptszLabel; }
	__inline MCONTACT hContact() const { return _hContact; }

	__inline HWND Wnd() const { return _hWnd; }
	__inline int DlgId() const { return _idDlg; }
	__inline HINSTANCE Inst() const { return _hInst; }
	__inline CUserInfoPageDlg* Dialog() const { return _pDialog; }

	__inline int Image() const { return _iImage; }
	__inline int Pos() const { return _iPosition; }
	__inline uint8_t State() const { return _bState; }
	__inline HTREEITEM Hti() const { return _hItem; }
	__inline void Hti(HTREEITEM hti) { _hItem = hti; }
	__inline CPsTreeItem* Parent() const { return _pParent; }
	__inline void Parent(CPsTreeItem *pParent) { _pParent = pParent; }

	__inline uint32_t Flags() const { return _dwFlags; }
	__inline void Flags(uint32_t dwFlags) { _dwFlags = dwFlags; }
	__inline void AddFlags(uint32_t dwFlags) { _dwFlags |= dwFlags; }
	__inline void RemoveFlags(uint32_t dwFlags) { _dwFlags &= ~dwFlags; }

	uint8_t HasName(const LPCSTR pszName) const;

	LPCSTR PropertyKey(LPCSTR pszProperty);
	LPCSTR GlobalPropertyKey(LPCSTR pszProperty);
	LPCSTR IconKey();

	LPSTR ParentItemName();
	HWND CreateWnd(LPPS pPs);
	
	uint16_t DBSaveItemState(LPCSTR pszGroup, int iItemPosition, UINT iState, uint32_t dwFlags);

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

class CPsTree
{
	OBJLIST<CPsTreeItem> _pages;

	HWND _hWndTree = nullptr;
	HIMAGELIST _hImages = nullptr;
	int _curItem = -1;
	uint32_t _dwFlags = 0;
	HWND _hLabelEdit = nullptr;
	HTREEITEM _hDragItem = nullptr;
	bool _isDragging = false;
	LPPS _pPs = nullptr;

	uint16_t	SaveItemsState(LPCSTR pszGroup, HTREEITEM hRootItem, int& iItem);

public:
	CPsTree(LPPS pPs);
	~CPsTree();
	
	__inline void BeginDrag(HTREEITEM hDragItem) { _isDragging = true; _hDragItem = hDragItem; }
	__inline void EndDrag() { _isDragging = false; _hDragItem = nullptr; }
	__inline bool IsDragging() const { return _isDragging; }
	__inline HTREEITEM DragItem() const { return _hDragItem; }

	__inline uint32_t Flags() const { return _dwFlags; }
	__inline void Flags(uint32_t dwFlags) { _dwFlags = dwFlags; }
	__inline void AddFlags(uint32_t dwFlags) { _dwFlags |= dwFlags; }
	__inline void RemoveFlags(uint32_t dwFlags) { _dwFlags &= ~dwFlags; }

	__inline int NumItems() const { return _pages.getCount(); }
	__inline HWND Window() const { return _hWndTree; }
	__inline HIMAGELIST ImageList() const { return _hImages; }
	__inline uint8_t IsIndexValid(const int index) const { return (index >= 0 && index < _pages.getCount()); }
	
	__inline CPsTreeItem* TreeItem(int index) const { return (IsIndexValid(index) ? &_pages[index] : nullptr); };
	__inline HTREEITEM TreeItemHandle(int index) const { return (IsIndexValid(index) ? _pages[index].Hti() : nullptr); };

	__inline int CurrentItemIndex() const { return _curItem; };
	__inline CPsTreeItem* CurrentItem() const { return TreeItem(CurrentItemIndex()); };

	CPsTreeItem* AddDummyItem(const char *pszGroup);
	uint8_t Create(HWND hWndTree, CPsHdr *pPsh);
	uint8_t InitTreeItems(LPWORD needWidth);
	void Remove(HINSTANCE);

	void HideItem(const int iPageIndex);
	HTREEITEM ShowItem(CPsTreeItem *pti, LPWORD needWidth);

	HTREEITEM MoveItem(HTREEITEM hItem, HTREEITEM hInsertAfter, uint8_t bAsChild = FALSE);
	void SaveState();
	void DBResetState();

	int FindItemIndexByHandle(HTREEITEM hItem);
	int FindItemIndexByName(LPCSTR pszName);

	CPsTreeItem *FindItemByName(LPCSTR pszName);
	CPsTreeItem *FindItemByHandle(HTREEITEM hItem);
	HTREEITEM    FindItemHandleByName(LPCSTR pszName);
	CPsTreeItem *FindItemByResource(HINSTANCE hInst, int idDlg);

	int  BeginLabelEdit(HTREEITEM hItem);
	int  EndLabelEdit(const uint8_t bSave);
	void PopupMenu();

	void OnIconsChanged();
	uint8_t OnInfoChanged();
	uint8_t OnSelChanging();
	void OnSelChanged(LPNMTREEVIEW lpnmtv);
	void OnCancel();
	int  OnApply();
};

/***********************************************************************************************************
 * common stuff
 ***********************************************************************************************************/

// internal flags for the PSP structure
#define PSPF_CHANGED            4096
#define PSPF_PROTOPREPENDED     8192 // the first token of the title is the protocol

#define PSF_PROTOPAGESONLY        64 // load only contact's protocol pages
#define PSF_PROTOPAGESONLY_INIT  128

class CPsUpload;

struct CPsHdr
{
	MCONTACT _hContact = 0;    // handle to the owning contact
	LPCSTR   _pszProto = 0;    // owning contact's protocol 
	LPCSTR   _pszPrefix = 0;   // name prefix for treeitem settings
	CPsTreeItem** _pPages = 0; // the pages
	uint16_t     _numPages = 0;    // number of pages
	uint32_t    _dwFlags = 0;     // some option flags
	HIMAGELIST _hImages = 0;   // the imagelist with all tree item icons
	LIST<wchar_t> _ignore;     // list of skipped items when adding metasubcontacts pages
	int      _nSubContact = 0; // index of a current subcontact

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
	char pszProto[MAXMODULELABELLENGTH];

	HANDLE hProtoAckEvent;  // eventhook for protocol acks
	HANDLE hSettingChanged; // eventhook searching for changed contact information
	HANDLE hIconsChanged;   // eventhook for changed icons in icolib
	HANDLE hModuleUnloaded; // eventhook for unloading modules

	HFONT hCaptionFont;
	HFONT hBoldFont;
	RECT rcDisplay;
	uint8_t updateAnimFrame;
	CHAR szUpdating[64];
	uint32_t dwFlags;

	TAckInfo	*infosUpdated;
	int nSubContacts;
	
	// controls
	HWND hDlg;
	CPsTree *pTree;
	CPsUpload *pUpload;
};

void DlgContactInfoInitTreeIcons();
void DlgContactInfoLoadModule();
void DlgContactInfoUnLoadModule();
