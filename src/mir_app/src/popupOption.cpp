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
#include "plugins.h"

struct MPopupOption
{
	const char *m_descr;
	CMPluginBase *m_plugin;
	CMOption<bool> &pVal;
};

static OBJLIST<MPopupOption> g_arOptions(1);

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addPopupOption(const char *pszDescr, CMOption<bool> &pVal)
{
	MPopupOption tmp = { pszDescr, this, pVal };
	g_arOptions.insert(new MPopupOption(tmp));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CPopupOptionsDlg : public CDlgBase
{
	CCtrlListView m_tree;

public:
	CPopupOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_POPUPOPTION),
		m_tree(this, IDC_TREE)
	{}

	bool OnInitDialog() override
	{
		m_tree.SetExtendedListViewStyleEx(0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iSubItem = 0;

		for (auto &it : g_arOptions) {
			_A2T tmp(it->m_descr);
			lvi.pszText = TranslateW_LP(tmp, it->m_plugin);
			lvi.lParam = LPARAM(it);
	
			int iRow = m_tree.InsertItem(&lvi);
			m_tree.SetItemState(iRow, it->pVal ? 0x2000 : 0x1000, LVIS_STATEIMAGEMASK);
		}

		return true;
	}

	bool OnApply() override
	{
		int iRows = m_tree.GetItemCount();
		for (int i = 0; i < iRows; i++) {
			LVITEM lvi;
			lvi.iItem = i;
			lvi.mask = LVIF_STATE | LVIF_PARAM;
			m_tree.GetItem(&lvi);

			auto *p = (MPopupOption *)lvi.lParam;
			p->pVal = lvi.state == 0x2000;
		}
		return true;
	}
};

int PopupOptionsInit(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.szGroup.a = LPGEN("Popups");
	odp.szTitle.a = LPGEN("Enabled items");
	odp.pDialog = new CPopupOptionsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
