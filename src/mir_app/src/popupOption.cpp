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
	MPopupOption(CMPluginBase *pPlugin, const char *pszDescr, CMOption<bool> &pVal) :
		m_plugin(pPlugin),
		m_val(pVal),
		m_descr(pszDescr)
	{}

	MPopupOption(CMPluginBase *pPlugin, const wchar_t *pwszDescr, CMOption<bool> &pVal) :
		m_plugin(pPlugin),
		m_val(pVal),
		m_descr(pwszDescr)
	{}

	CMPluginBase *m_plugin;
	CMOption<bool> &m_val;
	CMStringW m_descr;
};

static OBJLIST<MPopupOption> g_arOptions(1);

/////////////////////////////////////////////////////////////////////////////////////////

int CMPluginBase::addPopupOption(const char *pszDescr, CMOption<bool> &pVal)
{
	g_arOptions.insert(new MPopupOption(this, pszDescr, pVal));
	return 0;
}

int CMPluginBase::addPopupOption(const wchar_t *pwszDescr, CMOption<bool> &pVal)
{
	g_arOptions.insert(new MPopupOption(this, pwszDescr, pVal));
	return 0;
}

void KillModulePopups(CMPluginBase *pPlugin)
{
	for (auto &it : g_arOptions.rev_iter())
		if (it->m_plugin == pPlugin)
			g_arOptions.remove(g_arOptions.indexOf(&it));
}

/////////////////////////////////////////////////////////////////////////////////////////

class CPopupOptionsDlg : public CDlgBase
{
	CCtrlListView m_list;

public:
	CPopupOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_POPUPOPTION),
		m_list(this, IDC_TREE)
	{}

	bool OnInitDialog() override
	{
		m_list.SetExtendedListViewStyleEx(0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

		LVITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.iSubItem = 0;

		for (auto &it : g_arOptions) {
			lvi.pszText = TranslateW_LP(it->m_descr, it->m_plugin);
			lvi.lParam = LPARAM(it);
	
			int iRow = m_list.InsertItem(&lvi);
			m_list.SetItemState(iRow, it->m_val ? 0x2000 : 0x1000, LVIS_STATEIMAGEMASK);
		}

		return true;
	}

	bool OnApply() override
	{
		int iRows = m_list.GetItemCount();

		for (int i = 0; i < iRows; i++) {
			auto *p = (MPopupOption *)m_list.GetItemData(i);
			p->m_val = m_list.GetItemState(i, LVIS_STATEIMAGEMASK) == 0x2000;
		}
		return true;
	}
};

int PopupOptionsInit(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.szGroup.a = LPGEN("Popups");
	odp.szTitle.a = LPGEN("Events");
	odp.pDialog = new CPopupOptionsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
