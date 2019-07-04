/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

FacebookProto::FacebookProto(const char *proto_name, const wchar_t *username) :
	PROTO<FacebookProto>(proto_name, username)
{
	// to upgrade previous settings
	if (getByte("Compatibility") < 1) {
		setByte("Compatibility", 1);
		delSetting(DBKEY_DEVICE_ID);
	}

	m_szDeviceID = getMStringA(DBKEY_DEVICE_ID);
	if (m_szDeviceID.IsEmpty()) {
		UUID deviceId;
		UuidCreate(&deviceId);
		RPC_CSTR szId;
		UuidToStringA(&deviceId, &szId);
		m_szDeviceID = szId;
		setString(DBKEY_DEVICE_ID, m_szDeviceID);
		RpcStringFreeA(&szId);
	}

	m_szClientID = getMStringA(DBKEY_CLIENT_ID);
	if (m_szClientID.IsEmpty()) {
		for (int i = 0; i < 20; i++) {
			int c = rand() % 62;
			if (c >= 0 && c < 26)
				c += 'a';
			else if (c >= 26 && c < 52)
				c += 'A' - 26;
			else if (c >= 52 && c < 62)
				c += '0' - 52;
			m_szClientID.AppendChar(c);
		}
		setString(DBKEY_CLIENT_ID, m_szClientID);
	}

	m_uid = _atoi64(getMStringA(DBKEY_ID));

	// Create standard network connection
	wchar_t descr[512];
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

FacebookProto::~FacebookProto()
{
}

void FacebookProto::OnModulesLoaded()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR FacebookProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		{
			DWORD_PTR flags = PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES;

			if (getByte(DBKEY_SET_MIRANDA_STATUS))
				return flags |= PF1_MODEMSG;
			else
				return flags |= PF1_MODEMSGRECV;
		}

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_INVISIBLE | PF2_IDLE;

	case PFLAGNUM_3:
		if (getByte(DBKEY_SET_MIRANDA_STATUS))
			return PF2_ONLINE; // | PF2_SHORTAWAY;
		else
			return 0;

	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_READNOTIFY;

	case PFLAG_MAXLENOFMESSAGE:
		return FACEBOOK_MESSAGE_LIMIT;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR) "Facebook ID";
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int FacebookProto::SetStatus(int iNewStatus)
{
	if (iNewStatus != ID_STATUS_OFFLINE && IsStatusConnecting(m_iStatus)) {
		debugLogA("=== Status is already connecting, no change");
		return 0;
	}

	// Routing statuses not supported by Facebook
	switch (iNewStatus) {
	case ID_STATUS_ONLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:
		break;

	case ID_STATUS_NA:
		iNewStatus = ID_STATUS_AWAY;
		break;
	default:
		iNewStatus = getByte(DBKEY_MAP_STATUSES) ? ID_STATUS_INVISIBLE : ID_STATUS_AWAY;
		break;
	}

	if (m_iStatus == iNewStatus) {
		debugLogA("=== Statuses are same, no change");
		return 0;
	}

	m_invisible = (iNewStatus == ID_STATUS_INVISIBLE);
	m_iDesiredStatus = iNewStatus;

	int iOldStatus = m_iStatus;

	// log off & free all resources
	if (iNewStatus == ID_STATUS_OFFLINE) {
		OnLoggedOut();

		m_iStatus = ID_STATUS_OFFLINE;
	}
	else if (m_iStatus == ID_STATUS_OFFLINE) { // we gonna connect
		debugLogA("*** Beginning SignOn process");

		m_iStatus = ID_STATUS_CONNECTING;

		ForkThread(&FacebookProto::ServerThread);
	}
	else {
		// SetServerStatus(iNewStatus);

		m_iStatus = iNewStatus;
	}

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	return 0;
}
