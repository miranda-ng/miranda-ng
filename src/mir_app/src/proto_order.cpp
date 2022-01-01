/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "clc.h"

struct ProtocolData
{
	char *RealName;
	int enabled;
};

int isProtoSuitable(PROTO_INTERFACE* ppi)
{
	if (ppi == nullptr)
		return TRUE;

	return ppi->GetCaps(PFLAGNUM_2, 0) & ~ppi->GetCaps(PFLAGNUM_5, 0);
}

bool CheckProtocolOrder(void)
{
	bool changed = false;
	int id = 0;

	for (;;) {
		// Find account with this id
		bool bFound = false;
		for (auto &pa : g_arAccounts)
			if (pa->iOrder == id) {
				bFound = true;
				break;
			}

		// Account with id not found
		if (!bFound) {
			// Check if this is skipped id, if it is decrement all other ids
			for (auto &pa : g_arAccounts) {
				if (pa->iOrder < 1000000 && pa->iOrder > id) {
					--pa->iOrder;
					bFound = true;
				}
			}
			if (!bFound)
				break;
			
			changed = true;
		}
		else id++;
	}

	if (id < g_arAccounts.getCount()) {
		// Remove huge ids
		for (auto &pa : g_arAccounts)
			if (pa->iOrder >= 1000000)
				pa->iOrder = id++;

		changed = true;
	}

	if (id < g_arAccounts.getCount()) {
		// Remove duplicate ids
		for (int i = 0; i < g_arAccounts.getCount(); i++) {
			bool found = false;
			for (int j = 0; j < g_arAccounts.getCount(); j++) {
				if (g_arAccounts[j]->iOrder == i) {
					if (found) g_arAccounts[j]->iOrder = id++;
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
	if (!pa->IsEnabled())
		return false;

	PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(pa->szProtoName);
	return (pd != nullptr && pd->type == PROTOTYPE_PROTOWITHACCS);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CProtocolOrderOpts : public CDlgBase
{
	void FillTree()
	{
		m_order.DeleteAllItems();

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

		for (int i = 0; i < g_arAccounts.getCount(); i++) {
			int idx = Clist_GetAccountIndex(i);
			if (idx == -1)
				continue;

			PROTOACCOUNT *pa = g_arAccounts[idx];
			if (!ProtoToInclude(pa))
				continue;

			ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
			PD->RealName = pa->szModuleName;
			PD->enabled = pa->IsEnabled() && isProtoSuitable(pa->ppro);

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
		CDlgBase(g_plugin, IDD_OPT_PROTOCOLORDER),
		m_order(this, IDC_PROTOCOLORDER),
		m_btnReset(this, IDC_RESETPROTOCOLDATA),
		m_bDragging(false),
		m_hDragItem(nullptr)
	{
		m_btnReset.OnClick = Callback(this, &CProtocolOrderOpts::onReset_Click);

		m_order.SetFlags(MTREE_CHECKBOX | MTREE_DND);
		m_order.OnDeleteItem = Callback(this, &CProtocolOrderOpts::onOrder_DeleteItem);
	}

	bool OnInitDialog() override
	{
		FillTree();
		return true;
	}

	bool OnApply() override
	{
		// assume all g_arAccounts are disabled
		for (auto &it : g_arAccounts)
			it->iOrder = -1;

		int idx = 0;

		// scan chosen g_arAccounts and apply the order
		TVITEMEX tvi;
		tvi.hItem = m_order.GetRoot();
		tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_IMAGE;
		while (tvi.hItem != nullptr) {
			m_order.GetItem(&tvi);

			if (tvi.lParam != 0) {
				ProtocolData *ppd = (ProtocolData*)tvi.lParam;
				PROTOACCOUNT *pa = Proto_GetAccount(ppd->RealName);
				if (pa != nullptr) {
					pa->iOrder = idx++;
					if (ppd->enabled)
						pa->bIsVisible = tvi.iImage != 0;
				}
			}

			tvi.hItem = m_order.GetNextSibling(tvi.hItem);
		}

		// all g_arAccounts in the rest are disabled, so order doesn't matter
		for (auto &it : g_arAccounts)
			if (it->iOrder == -1)
				it->iOrder = idx++;

		WriteDbAccounts();
		Menu_ReloadProtoMenus();
		Clist_TrayIconIconsChanged();
		Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
		Clist_Broadcast(INTM_INVALIDATE, 0, 0);
		return true;
	}

	void onReset_Click(CCtrlButton*)
	{
		for (int i = 0; i < g_arAccounts.getCount(); i++)
			g_arAccounts[i]->iOrder = i;

		FillTree();
		NotifyChange();
	}

	void onOrder_DeleteItem(CCtrlTreeView::TEventInfo *env)
	{
		NMTREEVIEW *pnmtv = env->nmtv;
		if (pnmtv)
			mir_free((ProtocolData*)pnmtv->itemOld.lParam);
	}
};

int ProtocolOrderOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -10000000;
	odp.szTitle.a = LPGEN("Accounts");
	odp.szGroup.a = LPGEN("Contact list");
	odp.pDialog = new CProtocolOrderOpts();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
