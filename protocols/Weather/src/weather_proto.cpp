/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CWeatherProto::CWeatherProto(const char *protoName, const wchar_t *userName) :
	PROTO<CWeatherProto>(protoName, userName),
	m_impl(*this),
	m_bPopups(m_szModuleName, "UsePopup", true),
	m_szApiKey(m_szModuleName, "ApiKey", L"")
{
	m_hProtoIcon = g_plugin.getIconHandle(IDI_ICON);

	CreateProtoService(PS_GETAVATARINFO, &CWeatherProto::GetAvatarInfoSvc);
	CreateProtoService(PS_GETADVANCEDSTATUSICON, &CWeatherProto::AdvancedStatusIconSvc);

	HookProtoEvent(ME_OPT_INITIALISE, &CWeatherProto::OptInit);
	HookProtoEvent(ME_CLIST_DOUBLECLICKED, &CWeatherProto::BriefInfoEvt);
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CWeatherProto::BuildContactMenu);

	InitMwin();

	// load options and set defaults
	LoadOptions();

	// reset the weather data at startup for individual contacts
	EraseAllInfo();

	// menu items
	InitMenuItems();

	// popup initialization
	CMStringW wszTitle(FORMAT, L"%s %s", m_tszUserName, TranslateT("notifications"));
	g_plugin.addPopupOption(wszTitle, m_bPopups);

	// netlib
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

CWeatherProto::~CWeatherProto()
{
	DestroyMwin();
	DestroyUpdateList();
}

void CWeatherProto::OnModulesLoaded()
{
	// timer for the first update
	m_impl.m_start.Start(5000);  // first update is 5 sec after load

	// weather user detail
	HookProtoEvent(ME_USERINFO_INITIALISE, &CWeatherProto::UserInfoInit);
	HookProtoEvent(ME_TTB_MODULELOADED, &CWeatherProto::OnToolbarLoaded);
}

int CWeatherProto::OnToolbarLoaded(WPARAM, LPARAM)
{
	CMStringA szName(FORMAT, "%s/Enabled", m_szModuleName);

	TTBButton ttb = {};
	ttb.name = LPGEN("Enable/disable auto update");
	ttb.pszService = szName.GetBuffer();
	ttb.pszTooltipUp = LPGEN("Auto Update Enabled");
	ttb.pszTooltipDn = LPGEN("Auto Update Disabled");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_ICON);
	ttb.hIconHandleDn = g_plugin.getIconHandle(IDI_DISABLED);
	ttb.dwFlags = (getByte("AutoUpdate", 1) ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
	hTBButton = g_plugin.addTTB(&ttb);
	return 0;
}

void CWeatherProto::OnShutdown()
{
	m_impl.m_update.Stop();

	SaveOptions(); // save options once more
}

/////////////////////////////////////////////////////////////////////////////////////////

int CWeatherProto::SetStatus(int new_status)
{
	// if we don't want to show status for default station
	if (m_iStatus != new_status) {
		int old_status = m_iStatus;
		m_iStatus = new_status != ID_STATUS_OFFLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		UpdateMenu(m_iStatus != ID_STATUS_OFFLINE);
		if (m_iStatus != ID_STATUS_OFFLINE)
			UpdateAll(FALSE, FALSE);
	}

	return 0;
}

// get capabilities protocol service function
INT_PTR CWeatherProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		// support search and visible list
		return PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_EXTSEARCH | PF1_MODEMSGRECV;

	case PFLAGNUM_2:
	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Coordinates");
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// nothing to do here because weather proto do not need to retrieve contact info form network
// so just return a 0

void CWeatherProto::AckThreadProc(void *param)
{
	Sleep(100);

	ProtoBroadcastAck((DWORD_PTR)param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1);
}

int CWeatherProto::GetInfo(MCONTACT hContact, int)
{
	ForkThread(&CWeatherProto::AckThreadProc, (void *)hContact);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CWeatherProto::GetAwayMsgThread(void *arg)
{
	Sleep(100);

	MCONTACT hContact = (DWORD_PTR)arg;
	ptrW wszStatus(db_get_wsa(hContact, "CList", "StatusMsg"));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, this, wszStatus);
}

HANDLE CWeatherProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CWeatherProto::GetAwayMsgThread, (void*)hContact);
	return this;
}
