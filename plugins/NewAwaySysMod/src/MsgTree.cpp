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

#include "stdafx.h"
#include "MsgTree.h"
#include "Properties.h"

#define UM_MSGTREE_INIT (WM_USER + 0x2103)
#define UM_MSGTREE_UPDATE (WM_USER + 0x2104)

#define MSGTREE_TIMER_ID 0x2103
#define MSGTREE_DRAGANDDROP_GROUPEXPANDTIME 600

#define IMGLIST_NEWMESSAGE 0
#define IMGLIST_NEWCATEGORY 1
#define IMGLIST_DELETE 2

struct
{
	int DBSetting, Status, MenuItemID;
}
SettingsList[] =
{
	{ IDS_MESSAGEDLG_DEF_ONL,  ID_STATUS_ONLINE,     IDR_MSGTREEMENU_DEF_ONL },
	{ IDS_MESSAGEDLG_DEF_AWAY, ID_STATUS_AWAY,       IDR_MSGTREEMENU_DEF_AWAY },
	{ IDS_MESSAGEDLG_DEF_NA,   ID_STATUS_NA,         IDR_MSGTREEMENU_DEF_NA },
	{ IDS_MESSAGEDLG_DEF_OCC,  ID_STATUS_OCCUPIED,   IDR_MSGTREEMENU_DEF_OCC },
	{ IDS_MESSAGEDLG_DEF_DND,  ID_STATUS_DND,        IDR_MSGTREEMENU_DEF_DND },
	{ IDS_MESSAGEDLG_DEF_FFC,  ID_STATUS_FREECHAT,   IDR_MSGTREEMENU_DEF_FFC },
	{ IDS_MESSAGEDLG_DEF_INV,  ID_STATUS_INVISIBLE,  IDR_MSGTREEMENU_DEF_INV },
};

static MWindowList hMTWindowList;
static WNDPROC g_OrigEditProc;

void LoadMsgTreeModule()
{
	hMTWindowList = WindowList_Create();
}

static LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_GETDLGCODE:
		return CallWindowProc(g_OrigEditProc, hWnd, Msg, wParam, lParam) | DLGC_WANTALLKEYS;
	}
	return CallWindowProc(g_OrigEditProc, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK CMsgTree::ParentSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CMsgTree *dat = CWndUserData(hWnd).GetMsgTree();
	switch (Msg) {
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == dat->hTreeView) {
			switch (((LPNMHDR)lParam)->code) {
			case TVN_BEGINDRAG:
				{
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
					NMMSGTREE nm = {};
					COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
					int Order = TreeCtrl->hItemToOrder(pnmtv->itemNew.hItem);
					_ASSERT(Order != -1);
					if (Order != -1) {
						nm.ItemOld = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
						nm.hdr.code = MTN_BEGINDRAG;
						nm.hdr.hwndFrom = dat->hTreeView;
						nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
						if (!SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&nm)) {
							SetCapture(hWnd);
							dat->hPrevDropTarget = dat->hDragItem = pnmtv->itemNew.hItem;
							SetFocus(dat->hTreeView);
							TreeView_SelectItem(dat->hTreeView, dat->hDragItem);
						}
					}
				}
				break;

			case TVN_SELCHANGED:
				if (dat->UpdateLock)
					return 0;
				else {
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
					NMMSGTREE nm = {};
					COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
					if (pnmtv->itemOld.hItem) {
						int Order = TreeCtrl->IDToOrder(pnmtv->itemOld.lParam);
						if (Order != -1) {
							nm.ItemOld = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
						}
					}
					if (pnmtv->itemNew.hItem) {
						int Order = TreeCtrl->IDToOrder(pnmtv->itemNew.lParam);
						if (Order != -1) {
							nm.ItemNew = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
						}
					}
					nm.hdr.code = MTN_SELCHANGED;
					nm.hdr.hwndFrom = dat->hTreeView;
					nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
					SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&nm);
				}
				break;

			case TVN_BEGINLABELEDIT:
				if (dat->GetTreeCtrl()->IDToOrder(((LPNMTVDISPINFO)lParam)->item.lParam) < 0)
					return true; // cancel editing

				g_OrigEditProc = (WNDPROC)SetWindowLongPtr(TreeView_GetEditControl(dat->hTreeView), GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
				break;

			case TVN_ENDLABELEDIT:
				{
					LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
					if (ptvdi->item.pszText) {
						COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
						int Order = TreeCtrl->IDToOrder(ptvdi->item.lParam);
						if (Order >= 0) {
							TreeCtrl->m_value[Order].Title = ptvdi->item.pszText;
							TreeCtrl->SetModified(true);
							NMMSGTREE nm = {};
							nm.ItemNew = &TreeCtrl->m_value[Order];
							nm.hdr.code = MTN_ITEMRENAMED;
							nm.hdr.hwndFrom = dat->hTreeView;
							nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
							SendMessage(GetParent(dat->hTreeView), WM_NOTIFY, 0, (LPARAM)&nm);
							return true; // commit new text
						}
					}
				}
				break;

			case NM_CLICK:
			case NM_RCLICK:
				{
					TVHITTESTINFO hitTest;
					hitTest.pt.x = (short)LOWORD(GetMessagePos());
					hitTest.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(dat->hTreeView, &hitTest.pt);
					TreeView_HitTest(dat->hTreeView, &hitTest);
					if (hitTest.hItem) {
						if (TreeView_GetSelection(dat->hTreeView) == hitTest.hItem) // make sure TVN_SELCHANGED notification is sent always, even if previous selected item was the same as new
							TreeView_SelectItem(dat->hTreeView, NULL);
						TreeView_SelectItem(dat->hTreeView, hitTest.hItem);
					}
				}
				break;

			case NM_CUSTOMDRAW:
				NMTVCUSTOMDRAW *lpNMCD = (NMTVCUSTOMDRAW*)lParam;
				switch (lpNMCD->nmcd.dwDrawStage) {
				case CDDS_PREPAINT: // the control is about to start painting
					return CDRF_NOTIFYITEMDRAW; // instruct the control to return information when it draws items
				case CDDS_ITEMPREPAINT:
					return CDRF_NOTIFYPOSTPAINT;
				case CDDS_ITEMPOSTPAINT:
					RECT rc;
					TreeView_GetItemRect(lpNMCD->nmcd.hdr.hwndFrom, (HTREEITEM)lpNMCD->nmcd.dwItemSpec, &rc, true);
					int iSize = GetSystemMetrics(SM_CXSMICON);
					int x = rc.left - iSize - 5;
					for (auto &it: SettingsList) {
						if (lpNMCD->nmcd.lItemlParam == (LPARAM)dat->MsgTreePage.GetValue(it.DBSetting)) {
							DrawIconEx(lpNMCD->nmcd.hdc, x, rc.top, Skin_LoadProtoIcon(nullptr, it.Status), iSize, iSize, 0, GetSysColorBrush(COLOR_WINDOW), DI_NORMAL);
							x -= iSize + 1;
						}
					}
				}
			}
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->hDragItem) {
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hWnd, &hti.pt);
			ScreenToClient(dat->hTreeView, &hti.pt);
			TreeView_HitTest(dat->hTreeView, &hti);
			if (hti.hItem) {
				TreeView_SelectDropTarget(dat->hTreeView, hti.hItem);
				SetTimer(hWnd, MSGTREE_TIMER_ID, MSGTREE_DRAGANDDROP_GROUPEXPANDTIME, nullptr);
			}
			else {
				if (hti.flags & TVHT_ABOVE)
					SendMessage(dat->hTreeView, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW)
					SendMessage(dat->hTreeView, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

				TreeView_SelectDropTarget(dat->hTreeView, NULL);
				KillTimer(hWnd, MSGTREE_TIMER_ID);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (dat->hDragItem) {
			TreeView_SelectDropTarget(dat->hTreeView, NULL);
			KillTimer(hWnd, MSGTREE_TIMER_ID);
			ReleaseCapture();
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hWnd, &hti.pt);
			ScreenToClient(dat->hTreeView, &hti.pt);
			TreeView_HitTest(dat->hTreeView, &hti);
			if (hti.hItem && dat->hDragItem != hti.hItem) {
				NMMSGTREE nm = {};
				COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
				int OrderOld = TreeCtrl->hItemToOrder(dat->hDragItem);
				int OrderNew = TreeCtrl->hItemToOrder(hti.hItem);
				_ASSERT(OrderOld != -1 && OrderNew != -1);
				nm.ItemOld = (OrderOld <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(OrderOld)] : (CBaseTreeItem*)&TreeCtrl->m_value[OrderOld];
				nm.ItemNew = (OrderNew <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(OrderNew)] : (CBaseTreeItem*)&TreeCtrl->m_value[OrderNew];
				nm.hdr.code = MTN_ENDDRAG;
				nm.hdr.hwndFrom = dat->hTreeView;
				nm.hdr.idFrom = GetDlgCtrlID(dat->hTreeView);
				if (!SendMessage(hWnd, WM_NOTIFY, 0, (LPARAM)&nm)) {
					dat->UpdateLock++;
					dat->GetTreeCtrl()->MoveItem(hWnd, dat->hDragItem, hti.hItem);
					dat->UpdateLock--;
				}
			}
			dat->hDragItem = nullptr;
		}
		break;

	case WM_TIMER:
		if (wParam == MSGTREE_TIMER_ID) {
			KillTimer(hWnd, MSGTREE_TIMER_ID);
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(GetMessagePos());
			hti.pt.y = (short)HIWORD(GetMessagePos());
			ScreenToClient(dat->hTreeView, &hti.pt);
			TreeView_HitTest(dat->hTreeView, &hti);
			if (hti.hItem && dat->hDragItem != hti.hItem && TreeView_GetChild(dat->hTreeView, hti.hItem)) // target is a group and is not the same item that we're dragging
				TreeView_Expand(dat->hTreeView, hti.hItem, TVE_EXPAND);
		}
	}
	return CallWindowProc(dat->OrigParentProc, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK MsgTreeSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CMsgTree *dat = CWndUserData(GetParent(hWnd)).GetMsgTree();
	switch (Msg) {
	case UM_MSGTREE_UPDATE: // returns TRUE if updated
		{
			bool m_bModified = dat->MsgTreePage.GetModified();
			TCString WndTitle;
			if (m_bModified) {
				WndTitle.GetBuffer(256);
				HWND hCurWnd = hWnd;
				do {
					hCurWnd = GetParent(hCurWnd);
				} while (hCurWnd && !GetWindowText(hCurWnd, WndTitle, 256));
				WndTitle.ReleaseBuffer();
			}
			if (!m_bModified || MessageBox(GetParent(hWnd), TCString(TranslateT("You've made changes to multiple message trees at a time.\r\nDo you want to leave changes in \"")) + WndTitle + TranslateT("\" dialog?\r\nPress Yes to leave changes in this dialog, or No to discard its changes and save changes of the other message tree instead."), WndTitle + L" - " + TranslateT("New Away System"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
				COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
				TCString OldTitle, OldMsg, NewTitle, NewMsg;
				int OldOrder = TreeCtrl->IDToOrder(TreeCtrl->GetSelectedItemID(GetParent(hWnd)));
				if (OldOrder != -1) {
					CBaseTreeItem* ItemOld = (OldOrder <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(OldOrder)] : (CBaseTreeItem*)&TreeCtrl->m_value[OldOrder];
					OldTitle = ItemOld->Title;
					if (!(ItemOld->Flags & TIF_ROOTITEM))
						OldMsg = ((CTreeItem*)ItemOld)->User_Str1;
				}
				dat->UpdateLock++;
				dat->MsgTreePage.DBToMemToPage();
				dat->UpdateLock--;
				NMMSGTREE nm = {};
				int Order = TreeCtrl->IDToOrder(TreeCtrl->GetSelectedItemID(GetParent(hWnd)));
				if (Order != -1) {
					nm.ItemNew = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
					NewTitle = nm.ItemNew->Title;
					if (!(nm.ItemNew->Flags & TIF_ROOTITEM))
						NewMsg = ((CTreeItem*)nm.ItemNew)->User_Str1;
				}
				if (OldTitle.IsEmpty())
					OldTitle = L""; // to be sure that NULL will be equal to "" in the latter comparisons
				if (OldMsg.IsEmpty())
					OldMsg = L"";
				if (NewTitle.IsEmpty())
					NewTitle = L"";
				if (NewMsg.IsEmpty())
					NewMsg = L"";

				if (OldTitle != (const wchar_t*)NewTitle || OldMsg != (const wchar_t*)NewMsg) {
					// probably it's better to leave nm.ItemOld = NULL, to prevent accidental rewriting of it with old data from an edit control etc.
					nm.hdr.code = MTN_SELCHANGED;
					nm.hdr.hwndFrom = hWnd;
					nm.hdr.idFrom = GetDlgCtrlID(hWnd);
					SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nm);
				}
				return true;
			}
		}
		return false;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_DELETE:
			dat->DeleteSelectedItem();
			break;
		case VK_INSERT:
			dat->AddMessage();
			break;
		}
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

			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
		break;

	case WM_CONTEXTMENU:
		{
			TVHITTESTINFO ht;
			ht.pt.x = GET_X_LPARAM(lParam);
			ht.pt.y = GET_Y_LPARAM(lParam);
			TVITEM tvi = { 0 };
			if (ht.pt.x == -1 && ht.pt.y == -1) { // use selected item 
				if (tvi.hItem = TreeView_GetSelection(hWnd)) {
					TreeView_EnsureVisible(hWnd, tvi.hItem);
					RECT rc;
					TreeView_GetItemRect(hWnd, tvi.hItem, &rc, true);
					ht.pt.x = rc.left;
					ht.pt.y = rc.bottom;
				}
			}
			else {
				ScreenToClient(hWnd, &ht.pt);
				TreeView_HitTest(hWnd, &ht);
				if (ht.hItem && ht.flags & TVHT_ONITEM) {
					tvi.hItem = ht.hItem;
				}
			}
			if (tvi.hItem) {
				COptItem_TreeCtrl *TreeCtrl = dat->GetTreeCtrl();
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_GetItem(hWnd, &tvi);
				int Order = TreeCtrl->IDToOrder(tvi.lParam);
				if (Order >= 0) {
					HMENU hMenu;
					if (TreeCtrl->m_value[Order].Flags & TIF_GROUP)
						hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MSGTREE_CATEGORYMENU));
					else
						hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MSGTREE_MESSAGEMENU));

					_ASSERT(hMenu);
					HMENU hPopupMenu = GetSubMenu(hMenu, 0);
					TranslateMenu(hPopupMenu);
					ClientToScreen(hWnd, &ht.pt);
					struct
					{
						int ItemID, IconID;
					}
					MenuItems[] = {
						IDM_MSGTREEMENU_NEWMESSAGE, IMGLIST_NEWMESSAGE,
						IDM_MSGTREEMENU_NEWCATEGORY, IMGLIST_NEWCATEGORY,
						IDM_MSGTREEMENU_DELETE, IMGLIST_DELETE
					};
					MENUITEMINFO mii = { 0 };
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_BITMAP | MIIM_DATA | MIIM_STATE | MIIM_CHECKMARKS;
					mii.hbmpItem = HBMMENU_CALLBACK;
					for (auto &it: MenuItems) { // set icons
						mii.dwItemData = it.IconID;
						SetMenuItemInfo(hPopupMenu, it.ItemID, false, &mii);
					}
					mii.fMask = MIIM_STATE;
					mii.fState = MFS_CHECKED;
					for (auto &it: SettingsList) // set checkmarks
						if (TreeCtrl->m_value[Order].ID == (int)dat->MsgTreePage.GetValue(it.DBSetting))
							SetMenuItemInfo(hPopupMenu, it.MenuItemID, false, &mii);

					int MenuResult = TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, ht.pt.x, ht.pt.y, 0, hWnd, nullptr);
					switch (MenuResult) {
					case IDM_MSGTREEMENU_NEWMESSAGE:
						dat->AddMessage();
						break;
					case IDM_MSGTREEMENU_NEWCATEGORY:
						dat->AddCategory();
						break;
					case IDM_MSGTREEMENU_RENAME:
						TreeView_EditLabel(hWnd, tvi.hItem);
						break;
					case IDM_MSGTREEMENU_DELETE:
						dat->DeleteSelectedItem();
						break;
					case IDR_MSGTREEMENU_DEF_ONL:
					case IDR_MSGTREEMENU_DEF_AWAY:
					case IDR_MSGTREEMENU_DEF_NA:
					case IDR_MSGTREEMENU_DEF_OCC:
					case IDR_MSGTREEMENU_DEF_DND:
					case IDR_MSGTREEMENU_DEF_FFC:
					case IDR_MSGTREEMENU_DEF_INV:
						for (auto &it: SettingsList) {
							if (it.MenuItemID == MenuResult) {
								dat->SetDefMsg(it.Status, tvi.lParam);
								break;
							}
						}
					}
					DestroyMenu(hMenu);
					return 0;
				}
			}
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
			if (lpmi->CtlType == ODT_MENU) {
				lpmi->itemWidth = max(0, GetSystemMetrics(SM_CXSMICON) - GetSystemMetrics(SM_CXMENUCHECK) + 4);
				lpmi->itemHeight = GetSystemMetrics(SM_CYSMICON) + 2;
				return true;
			}
		}
		break;
	
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU) {
				ImageList_DrawEx(dat->hImageList, dis->itemData, dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2 + 1, 0, 0, GetSysColor(COLOR_WINDOW), CLR_NONE, ILD_NORMAL);
				return true;
			}
		} 
		break;
	}
	return CallWindowProc(dat->OrigTreeViewProc, hWnd, Msg, wParam, lParam);
}

CMsgTree::CMsgTree(HWND hTreeView) : MsgTreePage(g_MsgTreePage), hTreeView(hTreeView), hDragItem(nullptr), hPrevDropTarget(nullptr), UpdateLock(0)
{
	CWndUserData(GetParent(hTreeView)).SetMsgTree(this);
	OrigParentProc = (WNDPROC)SetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC, (LONG_PTR)ParentSubclassProc);
	OrigTreeViewProc = (WNDPROC)SetWindowLongPtr(hTreeView, GWLP_WNDPROC, (LONG_PTR)MsgTreeSubclassProc);
	MsgTreePage.SetWnd(GetParent(hTreeView));
	COptItem_TreeCtrl* TreeCtrl = (COptItem_TreeCtrl*)MsgTreePage.Find(IDV_MSGTREE);
	TreeCtrl->SetDlgItemID(GetDlgCtrlID(hTreeView));
	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 5, 2);
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_NEWMESSAGE));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_NEWCATEGORY));
	ImageList_AddIcon(hImageList, g_plugin.getIcon(IDI_DELETE));
	MsgTreePage.DBToMemToPage();
	if (!g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_RECENTMSGSCOUNT)) // show "Recent messages" group only when RECENTMSGSCOUNT is not set to 0.
		TreeView_DeleteItem(hTreeView, TreeCtrl->RootItems[g_Messages_RecentRootID].hItem);

	WindowList_Add(hMTWindowList, hTreeView, NULL);
}

CMsgTree::~CMsgTree()
{
	_ASSERT(GetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC) == (LONG_PTR)ParentSubclassProc); // we won't allow anyone to change our WNDPROC. otherwise we're not sure that we're setting the right WNDPROC back
	SetWindowLongPtr(hTreeView, GWLP_WNDPROC, (GetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC) == (LONG_PTR)ParentSubclassProc) ? (LONG_PTR)OrigTreeViewProc : (LONG_PTR)DefDlgProc); // yeah, if that crazy Help plugin substituted MY WndProc again, he won't get his WndProc back.. he-he >:)
	SetWindowLongPtr(GetParent(hTreeView), GWLP_WNDPROC, (LONG_PTR)OrigParentProc);
	CWndUserData(GetParent(hTreeView)).SetMsgTree(nullptr);
	WindowList_Remove(hMTWindowList, hTreeView);
	ImageList_Destroy(hImageList);
}

CBaseTreeItem* CMsgTree::GetSelection() // returns NULL if there's nothing selected
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	int Order = TreeCtrl->IDToOrder(TreeCtrl->GetSelectedItemID(GetParent(hTreeView)));
	if (Order == -1)
		return nullptr;
	return (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
}

bool CMsgTree::SetSelection(int ID, int Flags) // set ID = -1 to unselect; returns TRUE on unselect and on successful select
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	int Order = (Flags & MTSS_BYORDER) ? ID : TreeCtrl->IDToOrder(ID);
	if (Order == -1 && ID != -1)
		return false;

	TreeView_SelectItem(hTreeView, (Order == -1) ? nullptr : ((Order <= TREECTRL_ROOTORDEROFFS) ? TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)].hItem : TreeCtrl->m_value[Order].hItem));
	return true;
}

int CMsgTree::GetDefMsg(int iMode)
{
	for (auto &it: SettingsList)
		if (it.Status == iMode)
			return (int)MsgTreePage.GetValue(it.DBSetting);

	return 0;
}

void CMsgTree::SetDefMsg(int iMode, int ID)
{
	for (auto &it: SettingsList) {
		if (it.Status == iMode) {
			if ((int)MsgTreePage.GetValue(it.DBSetting) != ID) {
				RECT rc;
				COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
				int OrderOld = TreeCtrl->IDToOrder((int)MsgTreePage.GetValue(it.DBSetting));
				if (OrderOld >= 0 && TreeView_GetItemRect(hTreeView, TreeCtrl->m_value[OrderOld].hItem, &rc, false))
					InvalidateRect(hTreeView, &rc, true); // refresh icons of previous default tree item

				int OrderNew = TreeCtrl->IDToOrder(ID);
				if (OrderNew >= 0 && TreeView_GetItemRect(hTreeView, TreeCtrl->m_value[OrderNew].hItem, &rc, false))
					InvalidateRect(hTreeView, &rc, true); // refresh new default item icons

				MsgTreePage.SetValue(it.DBSetting, ID);
				NMMSGTREE nm = { 0 };
				if (OrderOld >= 0)
					nm.ItemOld = &TreeCtrl->m_value[OrderOld];
				if (OrderNew >= 0)
					nm.ItemNew = &TreeCtrl->m_value[OrderNew];
				nm.hdr.code = MTN_DEFMSGCHANGED;
				nm.hdr.hwndFrom = hTreeView;
				nm.hdr.idFrom = GetDlgCtrlID(hTreeView);
				SendMessage(GetParent(hTreeView), WM_NOTIFY, 0, (LPARAM)&nm);
			}
			break;
		}
	}
}

void CMsgTree::Save()
{
	if (MsgTreePage.GetModified()) {
		MsgTreePage.PageToMemToDB();
		WindowList_BroadcastAsync(hMTWindowList, UM_MSGTREE_UPDATE, 0, 0);
	}
}

void CMsgTree::UpdateItem(int ID) // updates item title, and expanded/collapsed state for groups
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	int Order = TreeCtrl->IDToOrder(ID);
	if (Order != -1) {
		CBaseTreeItem* TreeItem = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
		TCString NewTitle;
		TVITEM tvi;
		tvi.mask = TVIF_HANDLE | TVIF_TEXT;
		tvi.hItem = TreeItem->hItem;
		tvi.pszText = NewTitle.GetBuffer(TREEITEMTITLE_MAXLEN);
		tvi.cchTextMax = TREEITEMTITLE_MAXLEN;
		TreeView_GetItem(hTreeView, &tvi);
		if (TreeItem->Title != (const wchar_t*)tvi.pszText) {
			TreeCtrl->SetModified(true);
			NMMSGTREE nm = {};
			nm.ItemNew = TreeItem;
			nm.hdr.code = MTN_ITEMRENAMED;
			nm.hdr.hwndFrom = hTreeView;
			nm.hdr.idFrom = GetDlgCtrlID(hTreeView);
			SendMessage(GetParent(hTreeView), WM_NOTIFY, 0, (LPARAM)&nm);
		}
		tvi.mask = TVIF_HANDLE | TVIF_TEXT;
		tvi.pszText = TreeItem->Title;
		TreeView_SetItem(hTreeView, &tvi);
		TreeView_Expand(hTreeView, tvi.hItem, (TreeItem->Flags & TIF_EXPANDED) ? TVE_EXPAND : TVE_COLLAPSE);
	}
}

bool CMsgTree::DeleteSelectedItem() // returns true if the item was deleted
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	int Order = TreeCtrl->IDToOrder(TreeCtrl->GetSelectedItemID(GetParent(hTreeView)));
	_ASSERT(Order >= 0);
	CTreeItem *SelectedItem = &TreeCtrl->m_value[Order];

	//NightFox: fix for langpack and fix cut char space in text
	if (MessageBox(GetParent(hTreeView),
		((SelectedItem->Flags & TIF_GROUP) ?
			TranslateT("Do you really want to delete this category with its messages?") :
			TranslateT("Do you really want to delete this message?")),
		TranslateT("New Away System"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		NMMSGTREE nm = {};
		nm.ItemOld = SelectedItem;
		nm.hdr.code = MTN_DELETEITEM;
		nm.hdr.hwndFrom = hTreeView;
		nm.hdr.idFrom = GetDlgCtrlID(hTreeView);
		if (!SendMessage(GetParent(hTreeView), WM_NOTIFY, 0, (LPARAM)&nm)) {
			TreeCtrl->Delete(GetParent(hTreeView), TreeCtrl->GetSelectedItemID(GetParent(hTreeView)));
			return true;
		}
	}
	return false;
}

CTreeItem* CMsgTree::AddCategory()
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	CTreeItem* TreeItem = TreeCtrl->InsertItem(GetParent(hTreeView), CTreeItem(L"", 0, 0, TIF_GROUP));
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	tvi.hItem = TreeItem->hItem;
	TreeItem->Title = tvi.pszText = TranslateT("New category");
	TreeView_SetItem(hTreeView, &tvi);
	TreeView_EditLabel(hTreeView, TreeItem->hItem);
	NMMSGTREE nm = {};
	nm.ItemNew = TreeItem;
	nm.hdr.code = MTN_NEWCATEGORY;
	nm.hdr.hwndFrom = hTreeView;
	nm.hdr.idFrom = GetDlgCtrlID(hTreeView);
	SendMessage(GetParent(hTreeView), WM_NOTIFY, 0, (LPARAM)&nm);
	return TreeItem;
}

CTreeItem* CMsgTree::AddMessage()
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	CTreeItem* TreeItem = TreeCtrl->InsertItem(GetParent(hTreeView), CTreeItem(L"", 0, 0));
	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	tvi.hItem = TreeItem->hItem;
	TreeItem->Title = tvi.pszText = TranslateT("New message");
	TreeView_SetItem(hTreeView, &tvi);
	TreeView_EditLabel(hTreeView, TreeItem->hItem);
	NMMSGTREE nm = {};
	nm.ItemNew = TreeItem;
	nm.hdr.code = MTN_NEWMESSAGE;
	nm.hdr.hwndFrom = hTreeView;
	nm.hdr.idFrom = GetDlgCtrlID(hTreeView);
	SendMessage(GetParent(hTreeView), WM_NOTIFY, 0, (LPARAM)&nm);
	return TreeItem;
}

CBaseTreeItem* CMsgTree::GetNextItem(int Flags, CBaseTreeItem *Item) // Item is 'int ID' if MTGN_BYID flag is set; returns CBaseTreeItem* or NULL
{
	COptItem_TreeCtrl *TreeCtrl = GetTreeCtrl();
	CBaseTreeItem *TreeItem = Item;
	if (Flags & MTGN_BYID) {
		int Order = TreeCtrl->IDToOrder((INT_PTR)Item);
		_ASSERT(Order != -1);
		TreeItem = (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
	}

	int TVFlag = 0;
	switch (Flags & ~MTGN_BYID) {
		case MTGN_ROOT: TVFlag = TVGN_ROOT; break;
		case MTGN_CHILD: TVFlag = TVGN_CHILD; break;
		case MTGN_PARENT: TVFlag = TVGN_PARENT; break;
		case MTGN_NEXT: TVFlag = TVGN_NEXT; break;
		case MTGN_PREV: TVFlag = TVGN_PREVIOUS; break;
		default: _ASSERT(0);
	}
	
	int Order = TreeCtrl->hItemToOrder(TreeView_GetNextItem(hTreeView, TreeItem ? TreeItem->hItem : nullptr, TVFlag));
	if (Order == -1)
		return nullptr;

	return (Order <= TREECTRL_ROOTORDEROFFS) ? (CBaseTreeItem*)&TreeCtrl->RootItems[ROOT_ORDER_TO_INDEX(Order)] : (CBaseTreeItem*)&TreeCtrl->m_value[Order];
}
