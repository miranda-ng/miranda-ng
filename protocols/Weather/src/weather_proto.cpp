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
	hUpdateMutex(CreateMutex(nullptr, FALSE, nullptr))
{
	HookProtoEvent(ME_OPT_INITIALISE, &CWeatherProto::OptInit);
	HookProtoEvent(ME_CLIST_DOUBLECLICKED, &CWeatherProto::BriefInfoEvt);
	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CWeatherProto::BuildContactMenu);

	InitMwin();

	// reset the weather data at startup for individual contacts
	EraseAllInfo();

	// load options and set defaults
	LoadOptions();

	// menu items
	InitMenuItems();

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

	CloseHandle(hUpdateMutex);
}

void CWeatherProto::OnModulesLoaded()
{
	// timer for the first update
	m_impl.m_start.Start(5000);  // first update is 5 sec after load

	// weather user detail
	HookProtoEvent(ME_USERINFO_INITIALISE, &CWeatherProto::UserInfoInit);
}

void CWeatherProto::OnShutdown()
{
	m_impl.m_update.Stop();

	SaveOptions(); // save options once more
	
	WindowList_Broadcast(hWindowList, WM_CLOSE, 0, 0);
	WindowList_Broadcast(hDataWindowList, WM_CLOSE, 0, 0);
	SendMessage(hWndSetup, WM_CLOSE, 0, 0);
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
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH;

	case PFLAGNUM_5: /* this is PFLAGNUM_5 change when alpha SDK is released */
		return PF2_INVISIBLE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Station ID");
	}
	return 0;
}
