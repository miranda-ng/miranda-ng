/*
Fingerprint NG (client version) icons module for Miranda NG

Copyright © 2006-26 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//Start of header
#include "stdafx.h"

struct
{
	int idCtrl, defValue;
	LPCSTR szSetName;
}
static settings[] =
{
	{ IDC_GROUP_MIRANDA, TRUE, "GroupMiranda" },
	{ IDC_GROUP_MIRANDA_VERSION, FALSE, "GroupMirandaVersion" },
	{ IDC_GROUP_MIRANDA_PACKS, TRUE, "GroupMirandaPacks" },

	{ IDC_GROUP_MULTI, TRUE, "GroupMulti" },
	{ IDC_GROUP_GG, TRUE, "GroupGG" },
	{ IDC_GROUP_IRC, TRUE, "GroupIRC" },
	{ IDC_GROUP_JABBER, TRUE, "GroupJabber" },
	{ IDC_GROUP_RSS, TRUE, "GroupRSS" },
	{ IDC_GROUP_WEATHER, TRUE, "GroupWeather" },
	{ IDC_GROUP_VK, TRUE, "GroupVK" },

	{ IDC_GROUP_OTHER_PROTOS, TRUE, "GroupOtherProtos" },
	{ IDC_GROUP_OTHERS, TRUE, "GroupOthers" },

	{ IDC_GROUP_OVERLAYS_RESOURCE, TRUE, "GroupOverlaysResource" },
	{ IDC_GROUP_OVERLAYS_PLATFORM, TRUE, "GroupOverlaysPlatform" },
	{ IDC_GROUP_OVERLAYS_UNICODE, TRUE, "GroupOverlaysUnicode" },
	{ IDC_GROUP_OVERLAYS_PROTO, TRUE, "GroupOverlaysProtos" },
	{ IDC_GROUP_OVERLAYS_SECURITY, TRUE, "GroupOverlaysSecurity" },
	{ IDC_STATUSBAR, TRUE, "StatusBarIcon" }
};

class COptDialog : public CDlgBase
{
public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_DIALOG)
	{}

	bool OnInitDialog() override
	{
		for (auto &it : settings)
			if (auto *pCtrl = (CCtrlCheck *)FindControl(it.idCtrl))
				pCtrl->SetState(g_plugin.getByte(it.szSetName, it.defValue));

		return true;
	}

	bool OnApply() override
	{
		for (auto &it : settings)
			if (auto *pCtrl = (CCtrlCheck *)FindControl(it.idCtrl))
				g_plugin.setByte(it.szSetName, pCtrl->GetState());

		ClearFI();
		RegisterIcons();

		for (auto &hContact : Contacts())
			OnExtraImageApply(hContact, 0);
		return true;
	}
};

int OnOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Customize");
	odp.szTitle.a = LPGEN("Icons");
	odp.szTab.a = LPGEN("Fingerprint");
	odp.pDialog = new COptDialog;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
