/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

struct ProtocolData
{
	char *RealName;
	int enabled;
};

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
		for (i = 0; i < accounts.getCount(); i++)
			if (accounts[i]->iOrder == id) break;

		// Account with id not found
		if (i == accounts.getCount()) {
			// Check if this is skipped id, if it is decrement all other ids
			bool found = false;
			for (i = 0; i < accounts.getCount(); i++) {
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
		for (i = 0; i < accounts.getCount(); i++)
			if (accounts[i]->iOrder >= 1000000)
				accounts[i]->iOrder = id++;

		changed = true;
	}

	if (id < accounts.getCount()) {
		// Remove duplicate ids
		for (i = 0; i < accounts.getCount(); i++) {
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

/////////////////////////////////////////////////////////////////////////////////////////

class CProtocolOrderOpts : public CDlgBase
{
	void FillTree()
	{
		m_order.DeleteAllItems();

		TVINSERTSTRUCT tvis;
		tvis.hParent = NULL;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

		for (int i = 0; i < accounts.getCount(); i++) {
			int idx = cli.pfnGetAccountIndexByPos(i);
			if (idx == -1)
				continue;

			PROTOACCOUNT *pa = accounts[idx];
			if (!ProtoToInclude(pa))
				continue;

			ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
			PD->RealName = pa->szModuleName;
			PD->enabled = Proto_IsAccountEnabled(pa) && isProtoSuitable(pa->ppro);

			tvis.item.lParam = (LPARAM)PD;
			tvis.item.pszText = pa->tszAccountName;
			tvis.item.iImage = tvis.item.iSelectedImage = PD->enabled ? pa->bIsVisible : 100;
			m_order.InsertItem(&tvis);
		}
	}

	bool      m_bDragging;
	HTREEITEM m_hDragItem;

	CCtrlTreeView m_order;
	CCtrlButton m_btnReset;

public:
	CProtocolOrderOpts() :
		CDlgBase(hInst, IDD_OPT_PROTOCOLORDER),
		m_order(this, IDC_PROTOCOLORDER),
		m_btnReset(this, IDC_RESETPROTOCOLDATA),
		m_bDragging(false),
		m_hDragItem(NULL)
	{
		m_btnReset.OnClick = Callback(this, &CProtocolOrderOpts::onReset_Click);

		m_order.SetFlags(MTREE_CHECKBOX);
		m_order.OnBeginDrag = Callback(this, &CProtocolOrderOpts::onOrder_BeginDrag);
		m_order.OnDeleteItem = Callback(this, &CProtocolOrderOpts::onOrder_DeleteItem);
	}

	virtual void OnInitDialog()
	{
		HIMAGELIST himlCheckBoxes = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
		ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_NOTICK);
		ImageList_AddIcon_IconLibLoaded(himlCheckBoxes, SKINICON_OTHER_TICK);
		m_order.SetImageList(himlCheckBoxes, TVSIL_NORMAL);

		FillTree();
	}

	virtual void OnApply()
	{
		int idx = 0;

		TVITEMEX tvi;
		tvi.hItem = m_order.GetRoot();
		tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE;
		while (tvi.hItem != NULL) {
			m_order.GetItem(&tvi);

			if (tvi.lParam != 0) {
				ProtocolData *ppd = (ProtocolData*)tvi.lParam;
				PROTOACCOUNT *pa = Proto_GetAccount(ppd->RealName);
				if (pa != NULL) {
					while (idx < accounts.getCount() && !ProtoToInclude(accounts[idx]))
						idx++;
					pa->iOrder = idx++;
					if (ppd->enabled)
						pa->bIsVisible = tvi.iImage != 0;
				}
			}

			tvi.hItem = m_order.GetNextSibling(tvi.hItem);
		}

		WriteDbAccounts();
		cli.pfnReloadProtoMenus();
		cli.pfnTrayIconIconsChanged();
		cli.pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
		cli.pfnClcBroadcast(INTM_INVALIDATE, 0, 0);
	}

	virtual void OnDestroy()
	{
		ImageList_Destroy(m_order.GetImageList(TVSIL_NORMAL));
	}

	void onReset_Click(CCtrlButton*)
	{
		for (int i = 0; i < accounts.getCount(); i++)
			accounts[i]->iOrder = i;

		FillTree();
		NotifyChange();
	}

	void onOrder_DeleteItem(CCtrlTreeView::TEventInfo *env)
	{
		NMTREEVIEW *pnmtv = env->nmtv;
		if (pnmtv)
			mir_free((ProtocolData*)pnmtv->itemOld.lParam);
	}

	void onOrder_BeginDrag(CCtrlTreeView::TEventInfo *env)
	{
		SetCapture(m_hwnd);
		m_bDragging = true;
		m_hDragItem = env->nmtv->itemNew.hItem;
		m_order.SelectItem(m_hDragItem);
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		TVHITTESTINFO hti;

		switch (msg) {
		case WM_MOUSEMOVE:
			if (!m_bDragging)
				break;

			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(m_hwnd, &hti.pt);
			ScreenToClient(m_order.GetHwnd(), &hti.pt);
			m_order.HitTest(&hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= m_order.GetItemHeight() / 2;
				m_order.HitTest(&hti);
				if (!(hti.flags & TVHT_ABOVE))
					m_order.SetInsertMark(hti.hItem, 1);
				else
					m_order.SetInsertMark(it, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE) m_order.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) m_order.SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				m_order.SetInsertMark(NULL, 0);
			}
			break;

		case WM_LBUTTONUP:
			if (!m_bDragging)
				break;

			m_order.SetInsertMark(NULL, 0);
			m_bDragging = false;
			ReleaseCapture();

			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			ClientToScreen(m_hwnd, &hti.pt);
			ScreenToClient(m_order.GetHwnd(), &hti.pt);
			hti.pt.y -= m_order.GetItemHeight() / 2;
			m_order.HitTest(&hti);
			if (m_hDragItem == hti.hItem)
				break;
			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;

			TVITEMEX tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;
			tvi.hItem = m_hDragItem;
			m_order.GetItem(&tvi);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT) || (hti.hItem == TVI_FIRST)) {
				TCHAR name[128];
				TVINSERTSTRUCT tvis;
				tvis.itemex.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.itemex.stateMask = 0xFFFFFFFF;
				tvis.itemex.pszText = name;
				tvis.itemex.cchTextMax = SIZEOF(name);
				tvis.itemex.hItem = m_hDragItem;
				tvis.itemex.iImage = tvis.itemex.iSelectedImage = tvi.iImage;
				m_order.GetItem(&tvis.itemex);

				// the pointed lParam will be freed inside TVN_DELETEITEM
				// so lets substitute it with 0
				ProtocolData *lpOldData = (ProtocolData*)tvis.itemex.lParam;
				tvis.itemex.lParam = 0;
				m_order.SetItem(&tvis.itemex);

				// now current item contain lParam = 0 we can delete it. the memory will be kept.
				tvis.itemex.lParam = (LPARAM)lpOldData;
				m_order.DeleteItem(m_hDragItem);
				tvis.hParent = NULL;
				tvis.hInsertAfter = hti.hItem;
				m_order.SelectItem(m_order.InsertItem(&tvis));
				NotifyChange();
			}
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

int ProtocolOrderOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -10000000;
	odp.groupPosition = 1000000;
	odp.pszTitle = LPGEN("Accounts");
	odp.pszGroup = LPGEN("Contact list");
	odp.pDialog = new CProtocolOrderOpts();
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
