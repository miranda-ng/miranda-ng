/*
   Authorization State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2006-2010 by Thief

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

class COptionsDialog : public CDlgBase
{
	CCtrlCheck m_chkAuthIcon;
	CCtrlCheck m_chkGrantIcon;
	CCtrlCheck m_chkMenuItem;
	CCtrlCheck m_chkOnlyForRecent;

public:
	COptionsDialog() : 
		CDlgBase(g_plugin, IDD_AUTHSTATE_OPT),
		m_chkAuthIcon(this, IDC_AUTHICON),
		m_chkGrantIcon(this, IDC_GRANTICON),
		m_chkMenuItem(this, IDC_ENABLEMENUITEM),
		m_chkOnlyForRecent(this, IDC_ICONSFORRECENT)
	{
		CreateLink(m_chkAuthIcon, g_plugin.bUseAuthIcon);
		CreateLink(m_chkGrantIcon, g_plugin.bUseGrantIcon);
		CreateLink(m_chkMenuItem, g_plugin.bContactMenuItem);
		CreateLink(m_chkOnlyForRecent, g_plugin.bIconsForRecentContacts);
	}

	bool OnApply() override
	{
		for (auto &hContact : Contacts())
			onExtraImageApplying((WPARAM)hContact, 0);
		return true;
	}
};

int onOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Icons");
	odp.szTitle.a = LPGEN("Auth state");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptionsDialog();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
