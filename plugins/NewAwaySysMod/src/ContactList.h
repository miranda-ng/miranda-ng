/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Miranda's built-in CListControl is very slow and it has too limited functionality... 8( So i had to use my own control instead.

#pragma once

typedef TMyArray<HTREEITEM> TREEITEMARRAY;
typedef TREEITEMARRAY* PTREEITEMARRAY;


#define CLC_EXTRAICON_EMPTY 0xFF
#define CLC_ROOT TVI_ROOT

#define MAXEXTRAICONS 16

class CCLItemData // internal CCList's class
{
public:
	CCLItemData(MCONTACT hContact = INVALID_CONTACT_ID): hContact(hContact) {memset(ExtraIcons, CLC_EXTRAICON_EMPTY, sizeof(ExtraIcons));};

	BYTE ExtraIcons[MAXEXTRAICONS];
	MCONTACT hContact;
	LPARAM lParam;
};

typedef TMyArray<CCLItemData> TREEITEMDATAARRAY;
typedef TREEITEMDATAARRAY* PTREEITEMDATAARRAY;

class CCList
{
public:
	CCList(HWND hTreeView);
	~CCList();

	HTREEITEM AddContact(MCONTACT hContact);
	HTREEITEM AddGroup(TCString GroupName);
	HTREEITEM AddInfo(TCString Title, HTREEITEM hParent, HTREEITEM hInsertAfter, LPARAM lParam = NULL, HICON hIcon = NULL);
	void SetInfoIcon(HTREEITEM hItem, HICON hIcon);
	int GetExtraImage(HTREEITEM hItem, int iColumn); // returns iImage, or CLC_EXTRAICON_EMPTY
	void SetExtraImage(HTREEITEM hItem, int iColumn, int iImage); // set iImage to CLC_EXTRAICON_EMPTY to reset image
	void SetExtraImageList(HIMAGELIST hImgList);
	int GetItemType(HTREEITEM hItem); // returns a MCLCIT_ (see below)
	HTREEITEM GetNextItem(DWORD Flags, HTREEITEM hItem);
	void SortContacts();
	MCONTACT GethContact(HTREEITEM hItem); // returns hContact, hGroup or hInfo
	HTREEITEM HitTest(LPPOINT pt, PDWORD hitFlags); // pt is relative to control; returns hItem or NULL
	void EnsureVisible(HTREEITEM hItem) {TreeView_EnsureVisible(hTreeView, hItem); InvalidateRect(hTreeView, NULL, false);} // sometimes horizontal scrollbar position changes too, so we must redraw extra icons - that's why here is InvalidateRect. TODO: try to find a way to invalidate it on _every_ horizontal scrollbar position change, instead of just here - unfortunately the scrollbar doesn't notify the tree control of its position change through WM_HSCROLL in some cases
	int SelectItem(HTREEITEM hItem) {return TreeView_SelectItem(hTreeView, hItem);}
	void SetItemParam(HTREEITEM hItem, LPARAM lParam) {GetItemData(hItem).lParam = lParam;}
	LPARAM GetItemParam(HTREEITEM hItem) {return GetItemData(hItem).lParam;}
	PTREEITEMARRAY GetSelection() {return &SelectedItems;}
	void SetRedraw(bool bRedraw) {SendMessage(hTreeView, WM_SETREDRAW, bRedraw, 0);}

	friend LRESULT CALLBACK ParentSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	friend LRESULT CALLBACK ContactListSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	friend int CALLBACK CompareItemsCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	int Array_SetItemState(HTREEITEM hItem, bool bSelected);
	CCLItemData& GetItemData(HTREEITEM hItem);
	HTREEITEM TreeView_GetLastChild(HWND hTreeView, HTREEITEM hItem);
	HTREEITEM FindContact(MCONTACT hContact); // returns NULL if not found
	void SelectGroups(HTREEITEM hCurItem, bool bSelected);
	DWORD GetItemTypeAsCLGNFlag(HTREEITEM hItem); // returns MCLGN_CONTACT, MCLGN_GROUP or MCLGN_INFO

	HWND hTreeView;
	WNDPROC OrigTreeViewProc;
	WNDPROC OrigParentProc;
	TREEITEMDATAARRAY Items; // array that stores info for every tree item; array items are left even if corresponding tree item was deleted, so treeitem's lParam can be used as an index of an item in this array.
	TREEITEMARRAY SelectedItems; // contains HTREEITEMs of all selected items
	HIMAGELIST ExtraImageList;
};


// HitTest constants
#define MCLCHT_ABOVE 0x0001 // above the client area
#define MCLCHT_BELOW 0x0002 // below the client area
#define MCLCHT_TOLEFT 0x0004 // to the left of the client area
#define MCLCHT_TORIGHT 0x0008 // to the right of the client area
#define MCLCHT_NOWHERE 0x0010 // in the client area, but below the last item
#define MCLCHT_ONITEMINDENT 0x0020 // to the left of an item icon
#define MCLCHT_ONITEMICON 0x0040
#define MCLCHT_ONITEMLABEL 0x0080
#define MCLCHT_ONITEMRIGHT 0x0100 // in the area to the right of an item
#define MCLCHT_ONITEMEXTRA 0x0200 // on an extra icon, HIBYTE(HIWORD(hitFlags)) says which
#define MCLCHT_ONITEM (MCLCHT_ONITEMICON | MCLCHT_ONITEMLABEL)

// item types
#define MCLCIT_GROUP 0
#define MCLCIT_CONTACT 1
#define MCLCIT_INFO 3

// GetNextItem constants
#define MCLGN_ROOT 0
#define MCLGN_LAST 1
#define MCLGN_CHILD 2
#define MCLGN_LASTCHILD 3
#define MCLGN_PARENT 4
#define MCLGN_NEXT 5
#define MCLGN_PREV 6
// flags for use with MCLGN_NEXT and MCLGN_PREV:
#define MCLGN_CONTACT 0x20 // you need to specify at least one of these three! otherwise GetNextItem will not find anything
#define MCLGN_GROUP 0x40
#define MCLGN_INFO 0x80
#define MCLGN_ANY (MCLGN_CONTACT | MCLGN_GROUP | MCLGN_INFO)
#define MCLGN_MULTILEVEL 0x100 // walk through items of different levels too (ex.: MCLGN_NEXT | MCLGN_MULTILEVEL)
#define MCLGN_NOTCHILD (0x200 | MCLGN_MULTILEVEL) // when this flag is set, child items of specified hItem are ignored. for example, MCLGN_NEXT | MCLGN_CONTACT | MCLGN_NOTCHILD retrieves next contact that is not a child of hItem.
// GetNextItem(MCLGN_PREV | MCLGN_ALL | MCLGN_NOTCHILD, CLM_GETNEXTITEM(MCLGN_NEXT | MCLGN_ALL | MCLGN_NOTCHILD, hItem) === hItem


// notifications

typedef struct
{
	NMHDR hdr;
	PTREEITEMARRAY OldSelection, NewSelection;
} NMCLIST;
typedef NMCLIST* PNMCLIST;

#define MCLN_FIRST (0U - 100U)

#define MCLN_SELCHANGED (MCLN_FIRST - 20) // lParam = &NMCLIST; OldSelection and NewSelection contain selection info


void LoadCListModule();
