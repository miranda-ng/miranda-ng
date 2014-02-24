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

#include "Common.h"
#include "ContactList.h"
#include "Properties.h"

#define EXTRAICON_XSTEP (GetSystemMetrics(SM_CXSMICON) + 1)

static HANDLE hCLWindowList;

static int CLContactDeleted(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hCLWindowList, INTM_CONTACTDELETED, wParam, lParam);
	return 0;
}

static int CLContactIconChanged(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hCLWindowList, INTM_ICONCHANGED, wParam, lParam);
	return 0;
}

static int CLIconsChanged(WPARAM, LPARAM)
{
	WindowList_Broadcast(hCLWindowList, INTM_INVALIDATE, 0, 0);
	return 0;
}

void LoadCListModule()
{
	hCLWindowList = WindowList_Create();
	HookEvent(ME_DB_CONTACT_DELETED, CLContactDeleted);
	HookEvent(ME_CLIST_CONTACTICONCHANGED, CLContactIconChanged);
	HookEvent(ME_SKIN_ICONSCHANGED, CLIconsChanged);
}


static LRESULT CALLBACK ParentSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CCList *dat = CWndUserData(hWnd).GetCList();
	switch (Msg) {
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR)lParam;
			if (pnmh->hwndFrom == dat->hTreeView) {
				switch (pnmh->code) {
				case TVN_ITEMEXPANDED: // just set an appropriate group image
					{
						LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
						TVITEM tvItem;
						tvItem.hItem = pnmtv->itemNew.hItem;
						tvItem.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
						tvItem.iImage = tvItem.iSelectedImage = (pnmtv->itemNew.state & TVIS_EXPANDED) ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
						TreeView_SetItem(dat->hTreeView, &tvItem);
					}
					break;
				case TVN_SELCHANGED:
					{
						LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
						TREEITEMARRAY OldSelection = dat->SelectedItems;
						for (int i = 0; i < dat->SelectedItems.GetSize(); i++) {
							if (dat->SelectedItems[i] != pnmtv->itemNew.hItem) {
								TreeView_SetItemState(dat->hTreeView, dat->SelectedItems[i], 0, TVIS_SELECTED);
							}
						}
						dat->SelectedItems.RemoveAll();
						if (pnmtv->itemNew.hItem) {
							dat->SelectedItems.AddElem(pnmtv->itemNew.hItem);
							dat->SelectGroups(pnmtv->itemNew.hItem, true);
						}
						NMCLIST nm;
						nm.hdr.code = MCLN_SELCHANGED;
						nm.hdr.hwndFrom = dat->hTreeView;
						nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
						nm.OldSelection = &OldSelection;
						nm.NewSelection = &dat->SelectedItems;
						SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&nm);
					}
					break;

				case TVN_DELETEITEM:
					if (dat->Items.GetSize()) { // if Items size = 0, then this TVN_DELETEITEM came after WM_DESTROY, so there is no need to do anything
						LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
						TREEITEMARRAY OldSelection = dat->SelectedItems;
						int Index = dat->SelectedItems.Find(pnmtv->itemOld.hItem);
						if (Index != -1)
							dat->SelectedItems.RemoveElem(Index);

						// find an item to pass to SelectGroups()
						HTREEITEM hItem = TreeView_GetNextSibling(dat->hTreeView, pnmtv->itemOld.hItem);
						if (!hItem) {
							hItem = TreeView_GetPrevSibling(dat->hTreeView, pnmtv->itemOld.hItem);
							if (!hItem)
								hItem = TreeView_GetParent(dat->hTreeView, pnmtv->itemOld.hItem);
						}
						if (hItem) // if it wasn't one of the root items
							dat->SelectGroups(hItem, dat->SelectedItems.Find(hItem) != -1);

						NMCLIST nm;
						nm.hdr.code = MCLN_SELCHANGED;
						nm.hdr.hwndFrom = dat->hTreeView;
						nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
						nm.OldSelection = &OldSelection;
						nm.NewSelection = &dat->SelectedItems;
						SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&nm);
						dat->Items[pnmtv->itemOld.lParam].hContact = INVALID_CONTACT_ID;
					}
					break;

				case NM_CUSTOMDRAW:
					LPNMTVCUSTOMDRAW lpNMCD = (LPNMTVCUSTOMDRAW)lParam;
					switch (lpNMCD->nmcd.dwDrawStage) {
					case CDDS_PREPAINT: // the control is about to start painting
						return CDRF_NOTIFYITEMDRAW; // instruct the control to return information when it draws items
					case CDDS_ITEMPREPAINT:
						return CDRF_NOTIFYPOSTPAINT;
					case CDDS_ITEMPOSTPAINT:
						RECT rc;
						if (TreeView_GetItemRect(dat->hTreeView, (HTREEITEM)lpNMCD->nmcd.dwItemSpec, &rc, false)) {
							for (int i = 0; i < MAXEXTRAICONS; i++) {
								BYTE nIndex = dat->Items[lpNMCD->nmcd.lItemlParam].ExtraIcons[i];
								if (nIndex != CLC_EXTRAICON_EMPTY) {
									ImageList_DrawEx(dat->ExtraImageList, nIndex, lpNMCD->nmcd.hdc, rc.right - EXTRAICON_XSTEP * (i + 1), rc.top, 0, 0, /*GetSysColor(COLOR_WINDOW)*/CLR_NONE, CLR_NONE, ILD_NORMAL);
								}
							}
						}
						break;
					}
				}
				break;
			}
		}
	}
	return CallWindowProc(dat->OrigParentProc, hWnd, Msg, wParam, lParam);
}


static LRESULT CALLBACK ContactListSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	TVITEM tvi;
	CCList *dat = CWndUserData(GetParent(hWnd)).GetCList();

	switch (Msg) {
	case INTM_CONTACTDELETED: // wParam = (HANDLE)hContact
		{
			HTREEITEM hItem = dat->FindContact(wParam);
			if (hItem)
				TreeView_DeleteItem(hWnd, hItem);
		}
		break;

	case INTM_ICONCHANGED: // wParam = (HANDLE)hContact, lParam = IconID
		tvi.hItem = dat->FindContact(wParam);
		if (tvi.hItem) {
			tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvi.iImage = tvi.iSelectedImage = lParam;
			TreeView_SetItem(hWnd, &tvi);
			dat->SortContacts();
			InvalidateRect(hWnd, NULL, false);
		}
		break;

	case INTM_INVALIDATE:
		InvalidateRect(hWnd, NULL, true);
		break;

	case WM_RBUTTONDOWN:
		SetFocus(hWnd);
		{
			TVHITTESTINFO hitTest;
			hitTest.pt.x = (short)LOWORD(lParam);
			hitTest.pt.y = (short)HIWORD(lParam);
			TreeView_HitTest(hWnd, &hitTest);
			if (hitTest.hItem && hitTest.flags & TVHT_ONITEM)
				TreeView_SelectItem(hWnd, hitTest.hItem);
		}
		return DefWindowProc(hWnd, Msg, wParam, lParam);

	case WM_LBUTTONDOWN:
		{
			POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
			DWORD hitFlags;
			HTREEITEM hItem = dat->HitTest(&pt, &hitFlags);
			if (!hItem)
				break;

			if (hitFlags & MCLCHT_ONITEMICON) {
				if (TreeView_GetChild(hWnd, hItem)) { // if it's a group, then toggle its state
					NMTREEVIEW nmtv;
					nmtv.hdr.hwndFrom = hWnd;
					nmtv.hdr.idFrom = GetDlgCtrlID(hWnd);
					nmtv.hdr.code = TVN_ITEMEXPANDING;
					nmtv.action = TVE_TOGGLE;
					nmtv.itemNew.hItem = hItem;
					nmtv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
					TreeView_GetItem(hWnd, &nmtv.itemNew);
					nmtv.ptDrag = pt;
					if (SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nmtv))
						return 0;

					HTREEITEM hOldSelItem = TreeView_GetSelection(hWnd);
					TreeView_Expand(hWnd, hItem, TVE_TOGGLE);
					HTREEITEM hNewSelItem = TreeView_GetSelection(hWnd);
					if (hNewSelItem != hOldSelItem) {
						TreeView_SetItemState(hWnd, hOldSelItem, (dat->SelectedItems.Find(hOldSelItem) == -1) ? 0 : TVIS_SELECTED, TVIS_SELECTED);
						TreeView_SetItemState(hWnd, hNewSelItem, (dat->SelectedItems.Find(hNewSelItem) == -1) ? 0 : TVIS_SELECTED, TVIS_SELECTED);
					}
					nmtv.hdr.code = TVN_ITEMEXPANDED;
					TreeView_GetItem(hWnd, &nmtv.itemNew);
					SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nmtv);
					return 0;
				}
			}
			if (hitFlags & MCLCHT_ONITEM) {
				if (wParam & MK_CONTROL) {
					SetFocus(hWnd);
					TREEITEMARRAY OldSelection = dat->SelectedItems;
					int nIndex = dat->SelectedItems.Find(hItem);
					if (nIndex == -1) {
						TreeView_SetItemState(hWnd, hItem, TVIS_SELECTED, TVIS_SELECTED);
						dat->SelectedItems.AddElem(hItem);
					}
					else {
						TreeView_SetItemState(hWnd, hItem, 0, TVIS_SELECTED);
						dat->SelectedItems.RemoveElem(nIndex);
					}
					dat->SelectGroups(hItem, nIndex == -1);
					NMCLIST nm;
					nm.hdr.code = MCLN_SELCHANGED;
					nm.hdr.hwndFrom = hWnd;
					nm.hdr.idFrom = GetDlgCtrlID(hWnd);
					nm.OldSelection = &OldSelection;
					nm.NewSelection = &dat->SelectedItems;
					SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nm);
					return 0;
				}
				// if it was a click on the selected item and there's need to do something in this case, then send SELCHANGED notification by ourselves, as the tree control doesn't do anything
				if (hItem == TreeView_GetSelection(hWnd) && (dat->SelectedItems.GetSize() != 1 || (dat->SelectedItems.GetSize() == 1 && dat->SelectedItems[0] != hItem))) {
					TreeView_SetItemState(hWnd, hItem, TVIS_SELECTED, TVIS_SELECTED);
					NMTREEVIEW nm = { 0 };
					nm.hdr.code = TVN_SELCHANGED;
					nm.hdr.hwndFrom = hWnd;
					nm.hdr.idFrom = GetDlgCtrlID(hWnd);
					nm.itemOld.hItem = TreeView_GetSelection(hWnd);
					nm.itemOld.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
					TreeView_GetItem(hWnd, &nm.itemOld);
					nm.itemNew = nm.itemOld;
					SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nm);
				}
			}
		}
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		for (int i = 0; i < dat->SelectedItems.GetSize(); i++) {
			RECT rc;
			if (TreeView_GetItemRect(hWnd, dat->SelectedItems[i], &rc, false))
				InvalidateRect(hWnd, &rc, false);
		}
		break;

	case WM_SIZE:
	case WM_HSCROLL:
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_MEASUREITEM:
		if (!wParam) // if the message was sent by a menu
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		break;

	case WM_DRAWITEM:
		if (!wParam) // if the message was sent by a menu
			return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
		break;

	case WM_CONTEXTMENU:
		{
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			HTREEITEM hItem = NULL;
			if (pt.x == -1 && pt.y == -1) {
				if (dat->SelectedItems.GetSize() == 1) {
					hItem = dat->SelectedItems[0];
					TreeView_EnsureVisible(hWnd, hItem);
					RECT rc;
					TreeView_GetItemRect(hWnd, hItem, &rc, true);
					pt.x = rc.left;
					pt.y = rc.bottom;
				}
			}
			else {
				DWORD hitFlags;
				ScreenToClient(hWnd, &pt);
				hItem = dat->HitTest(&pt, &hitFlags);
				if (!(hitFlags & MCLCHT_ONITEM))
					hItem = NULL;
			}
			if (hItem) {
				MCONTACT hContact = dat->GetItemData(hItem).hContact;
				if (IsHContactContact(hContact)) {
					HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);
					if (hMenu) {
						ClientToScreen(hWnd, &pt);
						CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL), MPCF_CONTACTMENU), hContact);
						DestroyMenu(hMenu);
						return 0;
					}
				}
			}
		}
		break;

	case WM_DESTROY:
		if (dat->ExtraImageList)
			ImageList_Destroy(dat->ExtraImageList);
		dat->SelectedItems.RemoveAll();
		dat->Items.RemoveAll();
		break;
	}
	return CallWindowProc(dat->OrigTreeViewProc, hWnd, Msg, wParam, lParam);
}

CCList::CCList(HWND hTreeView) : 
	hTreeView(hTreeView), 
	ExtraImageList(NULL)
{
	CWndUserData(GetParent(hTreeView)).SetCList(this);
	OrigTreeViewProc = (WNDPROC)SetWindowLongPtr(hTreeView, GWLP_WNDPROC, (LONG_PTR)ContactListSubclassProc);
	OrigParentProc = (WNDPROC)SetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC, (LONG_PTR)ParentSubclassProc);
	TreeView_SetImageList(hTreeView, CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0), TVSIL_NORMAL);
	WindowList_Add(hCLWindowList, hTreeView, NULL);
	TreeView_SetIndent(hTreeView, 5); // doesn't set it less than the initial value on my system, and i guess it's because of icons... but who knows - maybe it will work somewhere
}

CCList::~CCList()
{
	WindowList_Remove(hCLWindowList, hTreeView);
	_ASSERT(GetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC) == (LONG_PTR)ParentSubclassProc); // we won't allow anyone to change our WNDPROC. otherwise we're not sure that we're setting the right WNDPROC back
	SetWindowLongPtr(hTreeView, GWLP_WNDPROC, (LONG_PTR)OrigTreeViewProc);
	SetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC, (LONG_PTR)OrigParentProc);
	CWndUserData(GetParent(hTreeView)).SetCList(NULL);
}

// adds a new contact if it doesn't exist yet; returns its hItem
HTREEITEM CCList::AddContact(MCONTACT hContact)
{
	_ASSERT(IsHContactContact(hContact));
	HTREEITEM hContactItem = FindContact(hContact);
	if (hContactItem)
		return hContactItem;

	TVINSERTSTRUCT tvIns;
	ZeroMemory(&tvIns, sizeof(tvIns));
	tvIns.hParent = AddGroup(db_get_s(hContact, "CList", "Group", _T("")));
	tvIns.item.pszText = pcli->pfnGetContactDisplayName(hContact, 0);
	tvIns.hInsertAfter = TVI_ROOT;
	tvIns.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvIns.item.iImage = tvIns.item.iSelectedImage = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);
	tvIns.item.lParam = Items.AddElem(CCLItemData(hContact));
	return TreeView_InsertItem(hTreeView, &tvIns);
}

struct sGroupEnumData
{
	HANDLE hGroup;
	TCString GroupName;
};

int GroupEnum(const char *szSetting, LPARAM lParam)
{
	sGroupEnumData *GroupEnumData = (sGroupEnumData*)lParam;
	TCString GroupName = db_get_s(NULL, "CListGroups", szSetting, _T(" "));
	if (!lstrcmp(GroupEnumData->GroupName, &GroupName[1]))
		GroupEnumData->hGroup = (HANDLE)(atol(szSetting) | HCONTACT_ISGROUP);
	return 0;
}

// adds a new group if it doesn't exist yet; returns its hItem
HTREEITEM CCList::AddGroup(TCString GroupName)
{
	if (GroupName == _T(""))
		return TVI_ROOT;

	sGroupEnumData GroupEnumData;
	GroupEnumData.GroupName = GroupName;
	GroupEnumData.hGroup = NULL;
	DBCONTACTENUMSETTINGS dbEnum;
	ZeroMemory(&dbEnum, sizeof(dbEnum));
	dbEnum.lParam = (LPARAM)&GroupEnumData;
	dbEnum.pfnEnumProc = GroupEnum;
	dbEnum.szModule = "CListGroups";
	CallService(MS_DB_CONTACT_ENUMSETTINGS, NULL, (LPARAM)&dbEnum);
	if (!GroupEnumData.hGroup) // means there is no such group in the groups list
		return NULL;

	HTREEITEM hGroupItem = FindContact((MCONTACT)GroupEnumData.hGroup);
	if (hGroupItem)
		return hGroupItem; // exists already, just return its handle

	TVINSERTSTRUCT tvIns = { 0 };
	tvIns.hParent = TVI_ROOT;
	tvIns.item.pszText = _tcsrchr(GroupName, '\\');
	if (tvIns.item.pszText) {
		TCString ParentGroupName(_T(""));
		tvIns.hParent = AddGroup(ParentGroupName.DiffCat(GroupName, tvIns.item.pszText));
		tvIns.item.pszText++;
	}
	else tvIns.item.pszText = GroupName;

	tvIns.hInsertAfter = TVI_ROOT;
	tvIns.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvIns.item.state = tvIns.item.stateMask = TVIS_BOLD | TVIS_EXPANDED;
	tvIns.item.iImage = tvIns.item.iSelectedImage = IMAGE_GROUPOPEN;
	tvIns.item.lParam = Items.AddElem(CCLItemData((MCONTACT)GroupEnumData.hGroup));
	return TreeView_InsertItem(hTreeView, &tvIns);
}

HTREEITEM CCList::AddInfo(TCString Title, HTREEITEM hParent, HTREEITEM hInsertAfter, LPARAM lParam, HICON hIcon)
{
	TVINSERTSTRUCT tvi = { 0 };
	tvi.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tvi.item.pszText = Title;
	tvi.hParent = hParent;
	tvi.hInsertAfter = hInsertAfter;
	tvi.item.lParam = Items.AddElem(CCLItemData());
	Items[tvi.item.lParam].lParam = lParam;
	tvi.item.state = tvi.item.stateMask = TVIS_BOLD | TVIS_EXPANDED;
	if (hIcon) {
		HIMAGELIST iml = TreeView_GetImageList(hTreeView, TVSIL_NORMAL);
		tvi.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.iImage = tvi.item.iSelectedImage = ImageList_AddIcon(iml, hIcon); // we don't check for duplicate icons, but i think that's ok, judging that the check will require some pretty significant amount of additional coding
		TreeView_SetImageList(hTreeView, iml, TVSIL_NORMAL);
	}
	return TreeView_InsertItem(hTreeView, &tvi);
}

void CCList::SetInfoIcon(HTREEITEM hItem, HICON hIcon)
{
	_ASSERT(hItem && hIcon && GetItemType(hItem) == MCLCIT_INFO);
	TVITEM tvi = { 0 };
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.hItem = hItem;
	HIMAGELIST iml = TreeView_GetImageList(hTreeView, TVSIL_NORMAL);
	tvi.iImage = tvi.iSelectedImage = ImageList_AddIcon(iml, hIcon); // again, we don't check for duplicate icons
	TreeView_SetImageList(hTreeView, iml, TVSIL_NORMAL);
	TreeView_SetItem(hTreeView, &tvi);
}

static int CALLBACK CompareItemsCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CCList *dat = (CCList*)lParamSort;
	if (IsHContactInfo(dat->Items[lParam1].hContact)) // Info items precede all other items
		return (IsHContactInfo(dat->Items[lParam2].hContact)) ? 0 : -1;

	if (IsHContactInfo(dat->Items[lParam2].hContact))
		return 1;

	if (IsHContactGroup(dat->Items[lParam1].hContact)) { // groups precedes contacts
		if (IsHContactGroup(dat->Items[lParam2].hContact))
			return (unsigned)dat->Items[lParam1].hContact - (unsigned)dat->Items[lParam2].hContact;
		return -1;
	}

	if (IsHContactGroup(dat->Items[lParam2].hContact))
		return 1;
	
	return CallService(MS_CLIST_CONTACTSCOMPARE, dat->Items[lParam1].hContact, dat->Items[lParam2].hContact);
}

void CCList::SortContacts()
{
	TVSORTCB tvSort = { 0 };
	tvSort.lpfnCompare = CompareItemsCallback;
	tvSort.hParent = TVI_ROOT;
	tvSort.lParam = (LPARAM)this;
	while (tvSort.hParent) {
		TreeView_SortChildrenCB(hTreeView, &tvSort, 0);
		tvSort.hParent = GetNextItem(MCLGN_NEXT | MCLGN_GROUP | MCLGN_MULTILEVEL, tvSort.hParent);
	}
}

int CCList::GetExtraImage(HTREEITEM hItem, int iColumn) // returns iImage, or CLC_EXTRAICON_EMPTY
{
	_ASSERT(iColumn < MAXEXTRAICONS);
	return GetItemData(hItem).ExtraIcons[iColumn];
}

void CCList::SetExtraImage(HTREEITEM hItem, int iColumn, int iImage) // set iImage to CLC_EXTRAICON_EMPTY to reset image
{
	_ASSERT(iColumn < MAXEXTRAICONS);
	GetItemData(hItem).ExtraIcons[iColumn] = iImage;
	RECT rc;
	if (TreeView_GetItemRect(hTreeView, hItem, &rc, false))
		InvalidateRect(hTreeView, &rc, true);
}

void CCList::SetExtraImageList(HIMAGELIST hImgList)
{
	ExtraImageList = hImgList;
	InvalidateRect(hTreeView, NULL, false);
}

int CCList::GetItemType(HTREEITEM hItem) // returns a MCLCIT_ (see below)
{
	MCONTACT hContact = GetItemData(hItem).hContact;
	return (IsHContactInfo(hContact)) ? MCLCIT_INFO : ((IsHContactGroup(hContact)) ? MCLCIT_GROUP : MCLCIT_CONTACT);
}

DWORD CCList::GetItemTypeAsCLGNFlag(HTREEITEM hItem)
{
	MCONTACT hContact = GetItemData(hItem).hContact;
	return (IsHContactInfo(hContact)) ? MCLGN_INFO : ((IsHContactGroup(hContact)) ? MCLGN_GROUP : MCLGN_CONTACT);
}

HTREEITEM CCList::GetNextItem(DWORD Flags, HTREEITEM hItem)
{
	switch (Flags & ~(MCLGN_MULTILEVEL | MCLGN_NOTCHILD | MCLGN_ANY)) {
	case MCLGN_ROOT:
		return TreeView_GetRoot(hTreeView);

	case MCLGN_LAST:
		{
			HTREEITEM hNextItem = TVI_ROOT;
			do {
				hItem = hNextItem;
				hNextItem = TreeView_GetLastChild(hTreeView, hNextItem);
			} while (hNextItem);
			return (hItem == TVI_ROOT) ? NULL : hItem;
		}

	case MCLGN_CHILD:
		return TreeView_GetChild(hTreeView, hItem);

	case MCLGN_LASTCHILD:
		return TreeView_GetLastChild(hTreeView, hItem);

	case MCLGN_PARENT:
		return TreeView_GetParent(hTreeView, hItem);

	case MCLGN_NEXT:
		do {
			if (Flags & MCLGN_MULTILEVEL) {
				HTREEITEM hNextItem = NULL;
				if ((Flags & MCLGN_NOTCHILD) != MCLGN_NOTCHILD)
					hNextItem = TreeView_GetChild(hTreeView, hItem);

				if (!hNextItem) {
					hNextItem = TreeView_GetNextSibling(hTreeView, hItem);
					while (!hNextItem) { // move back until we find next sibling of the item or one of its parents
						hItem = TreeView_GetParent(hTreeView, hItem);
						if (!hItem) // means it was the root, there are no items left.
							break; // returns NULL as the next item

						hNextItem = TreeView_GetNextSibling(hTreeView, hItem);
					}
				}
				hItem = hNextItem;
			}
			else hItem = TreeView_GetNextSibling(hTreeView, hItem);
	
			Flags &= ~(MCLGN_NOTCHILD & ~MCLGN_MULTILEVEL); // clear MCLGN_NOTCHILD flag
		}
			while (hItem && !(Flags & GetItemTypeAsCLGNFlag(hItem)));
		return hItem;

	case MCLGN_PREV:
		do {
			if (Flags & MCLGN_MULTILEVEL) {
				HTREEITEM hNextItem = TreeView_GetPrevSibling(hTreeView, hItem);
				if (hNextItem) {
					if ((Flags & MCLGN_NOTCHILD) != MCLGN_NOTCHILD) {
						while (hNextItem) {
							hItem = hNextItem;
							hNextItem = TreeView_GetLastChild(hTreeView, hItem);
						}
					}
					else hItem = hNextItem;
				}
				else hItem = TreeView_GetParent(hTreeView, hItem);
			}
			else hItem = TreeView_GetPrevSibling(hTreeView, hItem);

			Flags &= ~(MCLGN_NOTCHILD & ~MCLGN_MULTILEVEL); // clear MCLGN_NOTCHILD flag
		}
			while (hItem && !(Flags & GetItemTypeAsCLGNFlag(hItem)));
		return hItem;

	default:
		_ASSERT(0);
	}
	return NULL;
}

MCONTACT CCList::GethContact(HTREEITEM hItem) // returns hContact, hGroup or hInfo
{
	MCONTACT hContact = GetItemData(hItem).hContact;
	if (IsHContactContact(hContact))
		return hContact;
	if (IsHContactGroup(hContact))
		return hContact & ~HCONTACT_ISGROUP;
	return hContact & ~HCONTACT_ISINFO;
}

HTREEITEM CCList::HitTest(LPPOINT pt, PDWORD hitFlags) // pt is relative to control; returns hItem or NULL
{
	TVHITTESTINFO hti;
	hti.pt = *pt;
	TreeView_HitTest(hTreeView, &hti);
	*hitFlags = 0;
	if (hti.flags & TVHT_ABOVE)
		*hitFlags |= MCLCHT_ABOVE;

	if (hti.flags & TVHT_BELOW)
		*hitFlags |= MCLCHT_BELOW;

	if (hti.flags & TVHT_TOLEFT)
		*hitFlags |= MCLCHT_TOLEFT;

	if (hti.flags & TVHT_TORIGHT)
		*hitFlags |= MCLCHT_TORIGHT;

	if (hti.flags & TVHT_NOWHERE)
		*hitFlags |= MCLCHT_NOWHERE;

	if (hti.flags & TVHT_ONITEMINDENT)
		*hitFlags |= MCLCHT_ONITEMINDENT;

	if (hti.flags & (TVHT_ONITEMICON | TVHT_ONITEMSTATEICON))
		*hitFlags |= MCLCHT_ONITEMICON;

	if (hti.flags & TVHT_ONITEMLABEL)
		*hitFlags |= MCLCHT_ONITEMLABEL;

	if (hti.flags & TVHT_ONITEMRIGHT)
		*hitFlags |= MCLCHT_ONITEMRIGHT;

	if (hti.flags & (TVHT_ONITEMINDENT | TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
		// extraicon tests
		RECT rc;
		if (TreeView_GetItemRect(hTreeView, hti.hItem, &rc, false)) {
			int nIndex = (rc.right - pt->x - 1) / EXTRAICON_XSTEP;
			if (nIndex >= 0 && nIndex < MAXEXTRAICONS && GetItemData(hti.hItem).ExtraIcons[nIndex] != CLC_EXTRAICON_EMPTY)
				*hitFlags |= MCLCHT_ONITEMEXTRA | (nIndex << 24);
		}
	}
	return hti.hItem;
}

int CCList::Array_SetItemState(HTREEITEM hItem, bool bSelected)
{
	_ASSERT(hItem);
	int nIndex = SelectedItems.Find(hItem);
	if (nIndex == -1 && bSelected)
		return SelectedItems.AddElem(hItem);

	if (nIndex != -1 && !bSelected) {
		SelectedItems.RemoveElem(nIndex);
		return -1;
	}
	return nIndex;
}

CCLItemData& CCList::GetItemData(HTREEITEM hItem)
{
	_ASSERT(hItem && hItem != INVALID_HANDLE_VALUE);
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = hItem;
	int Res = TreeView_GetItem(hTreeView, &tvi);
	_ASSERT(Res);
	return Items[tvi.lParam];
}

HTREEITEM CCList::TreeView_GetLastChild(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM hPrevItem = TreeView_GetChild(hTreeView, hItem);
	hItem = hPrevItem;
	while (hItem) { // find last sibling
		hPrevItem = hItem;
		hItem = TreeView_GetNextSibling(hTreeView, hPrevItem);
	}
	return hPrevItem;
}

HTREEITEM CCList::FindContact(MCONTACT hContact)
{
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = TreeView_GetRoot(hTreeView);
	while (tvi.hItem) {
		TreeView_GetItem(hTreeView, &tvi);
		if (Items[tvi.lParam].hContact == hContact) {
			return tvi.hItem;
		}
		tvi.hItem = GetNextItem(MCLGN_NEXT | MCLGN_ANY | MCLGN_MULTILEVEL, tvi.hItem);
	}
	return NULL;
}

void CCList::SelectGroups(HTREEITEM hCurItem, bool bSelected)
{
	// select/deselect all child items
	HTREEITEM hItem = TreeView_GetChild(hTreeView, hCurItem);
	HTREEITEM hLimitItem = GetNextItem(MCLGN_NEXT | MCLGN_ANY | MCLGN_NOTCHILD, hCurItem);
	while (hItem && hItem != hLimitItem) {
		TreeView_SetItemState(hTreeView, hItem, bSelected ? TVIS_SELECTED : 0, TVIS_SELECTED);
		Array_SetItemState(hItem, bSelected);
		hItem = GetNextItem(MCLGN_NEXT | MCLGN_ANY | MCLGN_MULTILEVEL, hItem);
	}
	// select/deselect all parent groups
	hCurItem = TreeView_GetParent(hTreeView, hCurItem);
	if (bSelected) {
		while (hCurItem) { // select until we'll find an unselected item or until we'll reach the root
			hItem = TreeView_GetChild(hTreeView, hCurItem);
			while (hItem) { // walk through all siblings
				if (!(TreeView_GetItemState(hTreeView, hItem, TVIS_SELECTED) & TVIS_SELECTED))
					break;

				hItem = TreeView_GetNextSibling(hTreeView, hItem);
			}
			if (hItem) // means there was at least one unselected item
				break;

			TreeView_SetItemState(hTreeView, hCurItem, TVIS_SELECTED, TVIS_SELECTED);
			Array_SetItemState(hCurItem, true);
			hCurItem = TreeView_GetParent(hTreeView, hCurItem);
		}
	}

	while (hCurItem) { // and deselect all remaining parent groups
		TreeView_SetItemState(hTreeView, hCurItem, 0, TVIS_SELECTED);
		Array_SetItemState(hCurItem, false);
		hCurItem = TreeView_GetParent(hTreeView, hCurItem);
	}
}
