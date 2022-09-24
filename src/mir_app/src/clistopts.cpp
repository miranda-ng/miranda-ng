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

#define MODULENAME "CList"

CMOption<bool> Clist::UseGroups(MODULENAME, "UseGroups", true);
CMOption<bool> Clist::HideOffline(MODULENAME, "HideOffline", false);
CMOption<bool> Clist::ConfirmDelete(MODULENAME, "ConfirmDelete", true);
CMOption<bool> Clist::HideEmptyGroups(MODULENAME, "HideEmptyGroups", false);
CMOption<bool> Clist::DisableIconBlink(MODULENAME, "NoIconBlink", false);
CMOption<bool> Clist::RemoveTempContacts(MODULENAME, "RemoveTempContacts", true);

CMOption<bool> Clist::Tray1Click(MODULENAME, "Tray1Click", IsWinVer7Plus());
CMOption<bool> Clist::TrayAlwaysStatus(MODULENAME, "AlwaysStatus", false);

CMOption<bool>  Clist::FilterSearch("CLC", "FilterSearch", false);
CMOption<uint32_t> Clist::OfflineModes("CLC", "OfflineModes", MODEF_OFFLINE);

struct
{
	uint32_t style;
	wchar_t *szDescr;
}
static const offlineValues[] =
{
	{ MODEF_OFFLINE, LPGENW("Offline") },
	{ PF2_ONLINE,    LPGENW("Online") },
	{ PF2_SHORTAWAY, LPGENW("Away") },
	{ PF2_LONGAWAY,  LPGENW("Not available") },
	{ PF2_LIGHTDND,  LPGENW("Occupied") },
	{ PF2_HEAVYDND,  LPGENW("Do not disturb") },
	{ PF2_FREECHAT,  LPGENW("Free for chat") },
	{ PF2_INVISIBLE, LPGENW("Invisible") }
};

/////////////////////////////////////////////////////////////////////////////////////////

class ClistCommonOptsDlg : public CDlgBase
{
	CCtrlCheck chkUseGroups, chkHideOffline, chkConfirmDelete, chkHideEmptyGroups, chkRemoveTempContacts, chkEnableIconBlink, chkFilterSearch;
	CCtrlCheck chkAlwaysStatus, chkOneClick;
	CCtrlTreeView hideStatuses;

public:
	ClistCommonOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CLIST),
		hideStatuses(this, IDC_HIDEOFFLINEOPTS),
		chkOneClick(this, IDC_ONECLK),
		chkUseGroups(this, IDC_USEGROUPS),
		chkHideOffline(this, IDC_HIDEOFFLINE),
		chkFilterSearch(this, IDC_FILTER_SEARCH),
		chkAlwaysStatus(this, IDC_ALWAYSSTATUS),
		chkConfirmDelete(this, IDC_CONFIRMDELETE), 
		chkHideEmptyGroups(this, IDC_HIDEEMPTYGROUPS), 
		chkEnableIconBlink(this, IDC_ENABLE_ICON_BLINK),
		chkRemoveTempContacts(this, IDC_REMOVETEMP)
	{
		CreateLink(chkOneClick, Clist::Tray1Click);
		CreateLink(chkUseGroups, Clist::UseGroups);
		CreateLink(chkHideOffline, Clist::HideOffline);
		CreateLink(chkFilterSearch, Clist::FilterSearch);
		CreateLink(chkAlwaysStatus, Clist::TrayAlwaysStatus);
		CreateLink(chkConfirmDelete, Clist::ConfirmDelete);
		CreateLink(chkHideEmptyGroups, Clist::HideEmptyGroups);
		CreateLink(chkRemoveTempContacts, Clist::RemoveTempContacts);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(hideStatuses.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(hideStatuses.GetHwnd(), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		chkEnableIconBlink.SetState(!Clist::DisableIconBlink);

		int style = Clist::OfflineModes;

		TVINSERTSTRUCT tvis;
		tvis.hParent = nullptr;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
		for (auto &it : offlineValues) {
			tvis.item.lParam = it.style;
			tvis.item.pszText = TranslateW(it.szDescr);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			tvis.item.state = INDEXTOSTATEIMAGEMASK((style & it.style) != 0 ? 2 : 1);
			hideStatuses.InsertItem(&tvis);
		}
		return true;
	}

	bool OnApply() override
	{
		uint32_t flags = 0;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE;
		tvi.hItem = hideStatuses.GetRoot();
		while (tvi.hItem) {
			hideStatuses.GetItem(&tvi);
			if ((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2)
				flags |= tvi.lParam;
			tvi.hItem = hideStatuses.GetNextSibling(tvi.hItem);
		}
		Clist::OfflineModes = flags;

		Clist::DisableIconBlink = !chkEnableIconBlink.IsChecked();

		Clist_ClcOptionsChanged();
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
