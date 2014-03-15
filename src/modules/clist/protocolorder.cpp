/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"
#include "clc.h"

typedef struct tagProtocolData
{
	char *RealName;
	int show, enabled;
}
	ProtocolData;

struct ProtocolOrderData
{
	int dragging;
	HTREEITEM hDragItem;
};

typedef struct {
	char* protoName;
	int   visible;
}
	tempProtoItem;

int isProtoSuitable(PROTO_INTERFACE* ppi)
{
	if (ppi == NULL)
		return TRUE;

	return ppi->GetCaps(PFLAGNUM_2, 0) & ~ppi->GetCaps(PFLAGNUM_5, 0);
}

bool CheckProtocolOrder(void)
{
	bool changed = false;
	int i, id = 0;

	for (;;) {
		// Find account with this id
		for (i=0; i < accounts.getCount(); i++)
			if (accounts[i]->iOrder == id) break;

		// Account with id not found
		if (i == accounts.getCount()) {
			// Check if this is skipped id, if it is decrement all other ids
			bool found = false;
			for (i=0; i < accounts.getCount(); i++) {
				if (accounts[i]->iOrder < 1000000 && accounts[i]->iOrder > id) {
					--accounts[i]->iOrder;
					found = true;
				}
			}
			if (found) changed = true;
			else break;
		}
		else id++;
	}

	if (id < accounts.getCount()) {
		// Remove huge ids
		for (i=0; i < accounts.getCount(); i++)
			if (accounts[i]->iOrder >= 1000000)
				accounts[i]->iOrder = id++;

		changed = true;
	}

	if (id < accounts.getCount()) {
		// Remove duplicate ids
		for (i=0; i < accounts.getCount(); i++) {
			bool found = false;
			for (int j = 0; j < accounts.getCount(); j++) {
				if (accounts[j]->iOrder == i) {
					if (found) accounts[j]->iOrder = id++;
					else found = true;
				}
			}
		}
		changed = true;
	}

	return changed;
}

static bool ProtoToInclude(PROTOACCOUNT *pa)
{
	if (!Proto_IsAccountEnabled(pa))
		return false;

	PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(pa->szProtoName);
	return (pd != NULL && pd->type == PROTOTYPE_PROTOCOL);
}

static int FillTree(HWND hwnd)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;

	TreeView_DeleteAllItems(hwnd);
	if (accounts.getCount() == 0)
		return FALSE;

	for (int i=0; i < accounts.getCount(); i++) {
		int idx = cli.pfnGetAccountIndexByPos(i);
		if (idx == -1)
			continue;

		PROTOACCOUNT *pa = accounts[idx];
		if (!ProtoToInclude(pa))
			continue;

		ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
		PD->RealName = pa->szModuleName;
		PD->enabled = Proto_IsAccountEnabled(pa) && isProtoSuitable(pa->ppro);
		PD->show = PD->enabled ? pa->bIsVisible : 100;

		tvis.item.lParam = (LPARAM)PD;
		tvis.item.pszText = pa->tszAccountName;
		tvis.item.iImage = tvis.item.iSelectedImage = PD->show;
		TreeView_InsertItem(hwnd, &tvis);
	}

	return 0;
}

INT_PTR CALLBACK ProtocolOrderOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndProtoOrder = GetDlgItem(hwndDlg, IDC_PROTOCOLORDER);
	struct ProtocolOrderData *dat = (ProtocolOrderData*)GetWindowLongPtr(hwndProtoOrder, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		dat = (ProtocolOrderData*)mir_calloc(sizeof(ProtocolOrderData));
		SetWindowLongPtr(hwndProtoOrder, GWLP_USERDATA, (LONG_PTR)dat);
		dat->dragging = 0;

		SetWindowLongPtr(hwndProtoOrder, GWL_STYLE, GetWindowLongPtr(hwndProtoOrder, GWL_STYLE) | TVS_NOHSCROLL);
		{
			HIMAGELIST himlCheckBoxes = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32|ILC_MASK, 2, 2);
			ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_NOTICK);
			ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_TICK);
			TreeView_SetImageList(hwndProtoOrder, himlCheckBoxes, TVSIL_NORMAL);
		}

		FillTree(hwndProtoOrder);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_RESETPROTOCOLDATA && HIWORD(wParam) == BN_CLICKED) {
			for (int i=0; i < accounts.getCount(); i++)
				accounts[i]->iOrder = i;

			FillTree(hwndProtoOrder);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			if (((LPNMHDR)lParam)->code == PSN_APPLY) {
				int idx = 0;

				TVITEM tvi;
				tvi.hItem = TreeView_GetRoot(hwndProtoOrder);
				tvi.cchTextMax = 32;
				tvi.mask = TVIF_PARAM | TVIF_HANDLE;

				while (tvi.hItem != NULL) {
					TreeView_GetItem(hwndProtoOrder, &tvi);

					if (tvi.lParam != 0) {
						ProtocolData* ppd = (ProtocolData*)tvi.lParam;
						PROTOACCOUNT *pa = Proto_GetAccount(ppd->RealName);
						if (pa != NULL) {
							while (idx < accounts.getCount() && !ProtoToInclude(accounts[idx])) idx++;
							pa->iOrder = idx++;
							if (ppd->enabled)
								pa->bIsVisible = ppd->show != 0;
						}
					}

					tvi.hItem = TreeView_GetNextSibling(hwndProtoOrder, tvi.hItem);
				}

				WriteDbAccounts();
				cli.pfnReloadProtoMenus();
				cli.pfnTrayIconIconsChanged();
				cli.pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
				cli.pfnClcBroadcast(INTM_INVALIDATE, 0, 0);
			}
			break;

		case IDC_PROTOCOLORDER:
			switch (((LPNMHDR)lParam)->code) {
			case TVN_DELETEITEMA:
				{
					NMTREEVIEWA * pnmtv = (NMTREEVIEWA *) lParam;
					if (pnmtv && pnmtv->itemOld.lParam)
						mir_free((ProtocolData*)pnmtv->itemOld.lParam);
				}
				break;

			case TVN_BEGINDRAGA:
				SetCapture(hwndDlg);
				dat->dragging = 1;
				dat->hDragItem = ((LPNMTREEVIEW)lParam)->itemNew.hItem;
				TreeView_SelectItem(hwndProtoOrder, dat->hDragItem);
				break;

			case NM_CLICK:
				{
					TVHITTESTINFO hti;
					hti.pt.x = (short)LOWORD(GetMessagePos());
					hti.pt.y = (short)HIWORD(GetMessagePos());
					ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
					if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
						if (hti.flags & TVHT_ONITEMICON) {
							TVITEMA tvi;
							tvi.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
							tvi.hItem = hti.hItem;
							TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);

							ProtocolData *pData = (ProtocolData*)tvi.lParam;
							if (pData->enabled) {
								tvi.iImage = tvi.iSelectedImage = !tvi.iImage;
								pData->show = tvi.iImage;
								TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
							}
						}
					}
				}
			}
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->dragging) {
			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(hwndProtoOrder, &hti.pt);
			TreeView_HitTest(hwndProtoOrder, &hti);
			if (hti.flags & (TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= TreeView_GetItemHeight(hwndProtoOrder) / 2;
				TreeView_HitTest(hwndProtoOrder, &hti);
				if (!(hti.flags & TVHT_ABOVE))
					TreeView_SetInsertMark(hwndProtoOrder, hti.hItem, 1);
				else
					TreeView_SetInsertMark(hwndProtoOrder, it, 0);
			}
			else {
				if (hti.flags&TVHT_ABOVE) SendMessage(hwndProtoOrder, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags&TVHT_BELOW) SendMessage(hwndProtoOrder, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				TreeView_SetInsertMark(hwndProtoOrder, NULL, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (dat->dragging) {
			TreeView_SetInsertMark(hwndProtoOrder, NULL, 0);
			dat->dragging = 0;
			ReleaseCapture();

			TVHITTESTINFO hti;
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(hwndDlg, &hti.pt);
			ScreenToClient(hwndProtoOrder, &hti.pt);
			hti.pt.y -= TreeView_GetItemHeight(hwndProtoOrder) / 2;
			TreeView_HitTest(hwndProtoOrder, &hti);
			if (dat->hDragItem == hti.hItem) break;
			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;

			TVITEM tvi;
			tvi.mask = TVIF_HANDLE|TVIF_PARAM;
			tvi.hItem = dat->hDragItem;
			TreeView_GetItem(hwndProtoOrder, &tvi);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST)) {
				TVINSERTSTRUCT tvis;
				TCHAR name[128];
				ProtocolData * lpOldData;
				tvis.item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
				tvis.item.stateMask = 0xFFFFFFFF;
				tvis.item.pszText = name;
				tvis.item.cchTextMax = SIZEOF(name);
				tvis.item.hItem = dat->hDragItem;
				tvis.item.iImage = tvis.item.iSelectedImage = ((ProtocolData *)tvi.lParam)->show;
				TreeView_GetItem(hwndProtoOrder, &tvis.item);

				//the pointed lParam will be freed inside TVN_DELETEITEM
				//so lets substitute it with 0
				lpOldData = (ProtocolData *)tvis.item.lParam;
				tvis.item.lParam = 0;
				TreeView_SetItem(hwndProtoOrder, &tvis.item);
				tvis.item.lParam = (LPARAM)lpOldData;

				//now current item contain lParam = 0 we can delete it. the memory will be kept.
				TreeView_DeleteItem(hwndProtoOrder, dat->hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				TreeView_SelectItem(hwndProtoOrder, TreeView_InsertItem(hwndProtoOrder, &tvis));
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
			}
		}
		break;

	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(hwndProtoOrder, TVSIL_NORMAL));
		mir_free(dat);
		break;
	}
	return FALSE;
}
