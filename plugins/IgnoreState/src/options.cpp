/*
   IgnoreState plugin for Miranda-IM (www.miranda-im.org)
   (c) 2010 by Kildor

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

class COptDialog : public CDlgBase
{
	CCtrlTreeView m_tvFilter;
	CCtrlCheck m_chkIgnoreAll;

public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_IGNORE_OPT),
		m_tvFilter(this, IDC_FILTER),
		m_chkIgnoreAll(this, IDC_IGNORE_IGNOREALL)
	{
		m_chkIgnoreAll.OnChange = Callback(this, &COptDialog::OnIgnoreAllChange);
	}

	bool OnInitDialog() override
	{
		fill_filter();
		SetWindowLongPtr(m_tvFilter.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_tvFilter.GetHwnd(), GWL_STYLE) | TVS_NOHSCROLL);
		{
			HIMAGELIST himlButtonIcons = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
			m_tvFilter.SetImageList(himlButtonIcons, TVSIL_NORMAL);
			m_tvFilter.DeleteAllItems();

			for (int i = 2; i < nII; i++) { // we don`t need it IGNORE_ALL and IGNORE_MESSAGE
				TVINSERTSTRUCT tvis = {};
				int index = ImageList_AddIcon(himlButtonIcons, Skin_LoadIcon(ii[i].icon));
				tvis.hParent = nullptr;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
				tvis.item.lParam = ii[i].type;
				tvis.item.pszText = TranslateW(ii[i].name);
				tvis.item.iImage = tvis.item.iSelectedImage = index;
				HTREEITEM hti = m_tvFilter.InsertItem(&tvis);
				m_tvFilter.SetCheckState(hti, checkState(ii[i].type));
			}
		}

		m_chkIgnoreAll.SetState(bUseMirandaSettings);
		m_tvFilter.Enable(!bUseMirandaSettings);

		return true;
	}

	void OnIgnoreAllChange(CCtrlBase*)
	{
		m_tvFilter.Enable(!m_chkIgnoreAll.GetState());
	}

	bool OnApply() override
	{
		uint32_t flags = 0;
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TBIF_LPARAM;
		tvi.hItem = m_tvFilter.GetRoot(); //check ignore all
		while (tvi.hItem) {
			m_tvFilter.GetItem(&tvi);
			if (m_tvFilter.GetCheckState(tvi.hItem)) flags |= 1 << (tvi.lParam - 1);
			tvi.hItem = m_tvFilter.GetNextSibling(tvi.hItem);
		}
		g_plugin.setDword("Filter", flags);

		bUseMirandaSettings = m_chkIgnoreAll.GetState();
		g_plugin.setByte("UseMirandaSettings", bUseMirandaSettings);

		fill_filter();
		return true;
	}
};

int onOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Icons");
	odp.szTitle.a = LPGEN("Ignore State");
	odp.pDialog = new COptDialog;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
