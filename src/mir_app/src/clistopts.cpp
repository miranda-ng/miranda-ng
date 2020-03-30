/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org),
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

#define MODULENAME "CList"

CMOption<bool> Clist::UseGroups(MODULENAME, "UseGroups", true);
CMOption<bool> Clist::HideOffline(MODULENAME, "HideOffline", false);
CMOption<bool> Clist::ConfirmDelete(MODULENAME, "ConfirmDelete", true);
CMOption<bool> Clist::HideEmptyGroups(MODULENAME, "HideEmptyGroups", false);
CMOption<bool> Clist::DisableIconBlink(MODULENAME, "NoIconBlink", false);
CMOption<bool> Clist::RemoveTempContacts(MODULENAME, "RemoveTempContacts", true);

class ClistCommonOptsDlg : public CDlgBase
{
	CCtrlCheck chkUseGroups, chkHideOffline, chkConfirmDelete, chkHideEmptyGroups, chkRemoveTempContacts, chkDisableIconBlink;

public:
	ClistCommonOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CLIST),
		chkUseGroups(this, IDC_USEGROUPS),
		chkHideOffline(this, IDC_HIDEOFFLINE),
		chkConfirmDelete(this, IDC_CONFIRMDELETE), 
		chkHideEmptyGroups(this, IDC_HIDEEMPTYGROUPS), 
		chkDisableIconBlink(this, IDC_DISABLEICONBLINK),
		chkRemoveTempContacts(this, IDC_REMOVETEMP)		
	{
		CreateLink(chkUseGroups, Clist::UseGroups);
		CreateLink(chkHideOffline, Clist::HideOffline);
		CreateLink(chkConfirmDelete, Clist::ConfirmDelete);
		CreateLink(chkHideEmptyGroups, Clist::HideEmptyGroups);
		CreateLink(chkDisableIconBlink, Clist::DisableIconBlink);
		CreateLink(chkRemoveTempContacts, Clist::RemoveTempContacts);
	}

	bool OnApply() override
	{
		Clist_LoadContactTree();
		Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
		return true;
	}
};

int ClcOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -200000000;
	odp.szGroup.a = LPGEN("Contact list");
	odp.szTitle.a = LPGEN("Common");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new ClistCommonOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
