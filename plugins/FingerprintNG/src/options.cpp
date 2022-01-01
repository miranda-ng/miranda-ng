/*
Fingerprint NG (client version) icons module for Miranda NG

Copyright © 2006-22 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

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
	{ IDC_GROUP_ICQ, TRUE, "GroupICQ" },
	{ IDC_GROUP_IRC, TRUE, "GroupIRC" },
	{ IDC_GROUP_JABBER, TRUE, "GroupJabber" },
	{ IDC_GROUP_RSS, TRUE, "GroupRSS" },
	{ IDC_GROUP_WEATHER, TRUE, "GroupWeather" },
	{ IDC_GROUP_FACEBOOK, TRUE, "GroupFacebook" },
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
	CCtrlCheck m_chkMiranda, m_chkMirandaPacks, m_chkMirandaVer, m_chkOverRes, m_chkOverPlatf, m_chkOverProto, m_chkOverUnicode, m_chkOverSecur,
		m_chkFacebbok, m_chkGG, m_chkICQ, m_chkIRC, m_chkJabber, m_chkRSS, m_chkVK, m_chkWeather, m_chkMulti, m_chkOthersProto,
		m_chkOthers, m_chkStatusBar;

	void LoadDBCheckState(int idCtrl, LPCSTR szSetting, uint8_t bDef)
	{
		CCtrlCheck &item = *(CCtrlCheck*)FindControl(idCtrl);
		item.SetState(g_plugin.getByte(szSetting, bDef));
	}

	void StoreDBCheckState(int idCtrl, LPCSTR szSetting)
	{
		CCtrlCheck &item = *(CCtrlCheck*)FindControl(idCtrl);
		g_plugin.setByte(szSetting, item.GetState());
	}

public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_DIALOG),
		m_chkMiranda(this, IDC_GROUP_MIRANDA),
		m_chkMirandaPacks(this, IDC_GROUP_MIRANDA_PACKS),
		m_chkMirandaVer(this, IDC_GROUP_MIRANDA_VERSION),
		m_chkOverRes(this, IDC_GROUP_OVERLAYS_RESOURCE),
		m_chkOverPlatf(this, IDC_GROUP_OVERLAYS_PLATFORM),
		m_chkOverProto(this, IDC_GROUP_OVERLAYS_PROTO),
		m_chkOverUnicode(this, IDC_GROUP_OVERLAYS_UNICODE),
		m_chkOverSecur(this, IDC_GROUP_OVERLAYS_SECURITY),
		m_chkFacebbok(this, IDC_GROUP_FACEBOOK),
		m_chkGG(this, IDC_GROUP_GG),
		m_chkICQ(this, IDC_GROUP_ICQ),
		m_chkIRC(this, IDC_GROUP_IRC),
		m_chkJabber(this, IDC_GROUP_JABBER),
		m_chkRSS(this, IDC_GROUP_RSS),
		m_chkVK(this, IDC_GROUP_VK),
		m_chkWeather(this, IDC_GROUP_WEATHER),
		m_chkMulti(this, IDC_GROUP_MULTI),
		m_chkOthersProto(this, IDC_GROUP_OTHER_PROTOS),
		m_chkOthers(this, IDC_GROUP_OTHERS),
		m_chkStatusBar(this, IDC_STATUSBAR)
	{}

	bool OnInitDialog() override
	{
		for (auto &it : settings)
			LoadDBCheckState(it.idCtrl, it.szSetName, it.defValue);
		return true;
	}

	bool OnApply() override
	{
		for (auto &it : settings)
			StoreDBCheckState(it.idCtrl, it.szSetName);

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
	odp.szGroup.w = LPGENW("Icons");
	odp.szTitle.w = LPGENW("Fingerprint");
	odp.pDialog = new COptDialog;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
