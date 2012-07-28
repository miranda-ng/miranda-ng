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

#pragma once

extern COptPage g_MsgTreePage;

class CMsgTree
{
public:
	CMsgTree(HWND hTreeView); // warning: it changes GWLP_USERDATA of the parent window
	~CMsgTree();

	CBaseTreeItem* GetSelection(); // returns NULL if there's nothing selected
	bool SetSelection(int ID, int Flags); // set ID = -1 to unselect; ID specifies an order of an item if Flags = MTSS_BYORDER
	int GetDefMsg(int iMode);
	void SetDefMsg(int iMode, int ID);
	void Save();
	void UpdateItem(int ID); // updates item title, and expanded/collapsed state for groups; set SetModified to TRUE to change Modified flag of the tree
	bool DeleteSelectedItem(); // returns true if the item was deleted
	CTreeItem* AddCategory();
	CTreeItem* AddMessage();
	CBaseTreeItem* GetNextItem(int Flags, CBaseTreeItem* Item); // Item is 'int ID' if MTGN_BYID flag is set; returns CBaseTreeItem* or NULL
	void EnsureVisible(HTREEITEM hItem) {TreeView_EnsureVisible(hTreeView, hItem);}
	bool GetModified() {return MsgTreePage.GetModified();}
	void SetModified(bool Modified) {MsgTreePage.SetModified(Modified);}

	friend LRESULT CALLBACK ParentSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	friend LRESULT CALLBACK MsgTreeSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

private:
	COptItem_TreeCtrl* GetTreeCtrl() {return (COptItem_TreeCtrl*)MsgTreePage.Find(GetDlgCtrlID(hTreeView));}

	HWND hTreeView;
	WNDPROC OrigTreeViewProc;
	WNDPROC OrigParentProc;
	COptPage MsgTreePage;
	HTREEITEM hDragItem;
	HTREEITEM hPrevDropTarget;
	int UpdateLock;
	HIMAGELIST hImageList;
};

// SetSelection constants
#define MTSS_BYID 0
#define MTSS_BYORDER 1

// GetNextItem constants
#define MTGN_ROOT 0
#define MTGN_CHILD 1
#define MTGN_PARENT 2
#define MTGN_NEXT 3
#define MTGN_PREV 4
#define MTGN_BYID 0x8000 // means that lParam is ID of an item, not a pointer to its data


// notifications

typedef struct
{
	NMHDR hdr;
	CBaseTreeItem* ItemOld;
	CBaseTreeItem* ItemNew;
} NMMSGTREE;
typedef NMMSGTREE* PNMMSGTREE;

#define MTN_FIRST (0U - 100U)

#define MTN_SELCHANGED (MTN_FIRST - 0) // lParam = &NMMSGTREE; ItemOld and ItemNew contain old and new item info
#define MTN_BEGINDRAG (MTN_FIRST - 1) // lParam = &NMMSGTREE; ItemOld points to the item info; return TRUE to cancel dragging
#define MTN_ENDDRAG (MTN_FIRST - 2) // lParam = &NMMSGTREE; ItemOld points to the item info; ItemNew points to the item on which ItemOld was dropped; return TRUE to cancel moving
#define MTN_NEWCATEGORY (MTN_FIRST - 3) // lParam = &NMMSGTREE; ItemNew points to new category info
#define MTN_NEWMESSAGE (MTN_FIRST - 4) // lParam = &NMMSGTREE; ItemNew points to new message info
#define MTN_ITEMRENAMED (MTN_FIRST - 5) // lParam = &NMMSGTREE; ItemNew points to renamed item info
#define MTN_DELETEITEM (MTN_FIRST - 6) // lParam = &NMMSGTREE; ItemOld points to the item info; return TRUE to prevent item from being deleted
#define MTN_DEFMSGCHANGED (MTN_FIRST - 7) // lParam = &NMMSGTREE; ItemOld and ItemNew point to old and new default item info correspondingly


void LoadMsgTreeModule();
