/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

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

static int CompareUsers(const FacebookUser *p1, const FacebookUser *p2)
{
	if (p1->id == p2->id)
		return 0;

	return (p1->id < p2->id) ? -1 : 1;
}

static int CompareMessages(const COwnMessage *p1, const COwnMessage *p2)
{
	if (p1->msgId == p2->msgId)
		return 0;

	return (p1->msgId < p2->msgId) ? -1 : 1;
}

FacebookProto::FacebookProto(const char *proto_name, const wchar_t *username) :
	PROTO<FacebookProto>(proto_name, username),
	m_impl(*this),
	m_users(50, CompareUsers),
	arOwnMessages(1, CompareMessages),
	m_bLoadAll(this, "LoadAllContacts", false),
	m_bKeepUnread(this, "KeepUnread", false),
	m_bUseBigAvatars(this, "UseBigAvatars", true),
	m_bUseGroupchats(this, "UseGroupChats", true),
	m_bHideGroupchats(this, "HideGroupChats", true),
	m_bLoginInvisible(this, "LoginInvisible", false),
	m_wszDefaultGroup(this, "DefaultGroup", L"Facebook")
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
			uint32_t dwRandon;
			Utils_GetRandom(&dwRandon, sizeof(dwRandon));
			int c = dwRandon % 62;
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
	m_sid = _atoi64(getMStringA(DBKEY_SID));
	m_szSyncToken = getMStringA(DBKEY_SYNC_TOKEN);

	// Create standard network connection
	wchar_t descr[512];
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	db_set_resident(m_szModuleName, "UpdateNeeded");

	// Services
	CreateProtoService(PS_CREATEACCMGRUI, &FacebookProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETAVATARINFO, &FacebookProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &FacebookProto::GetAvatarCaps);

	// Events
	HookProtoEvent(ME_GC_EVENT, &FacebookProto::GroupchatEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &FacebookProto::GroupchatMenuHook);
	HookProtoEvent(ME_OPT_INITIALISE, &FacebookProto::OnOptionsInit);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &FacebookProto::OnMarkedRead);

	// Group chats
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);
}

FacebookProto::~FacebookProto()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// protocol events

void FacebookProto::OnContactAdded(MCONTACT hContact)
{
	__int64 userId = _atoi64(getMStringA(hContact, DBKEY_ID));
	if (userId && !FindUser(userId)) {
		mir_cslock lck(m_csUsers);
		m_users.insert(new FacebookUser(userId, hContact));
	}
}

void FacebookProto::OnModulesLoaded()
{
	VARSW wszCache(L"%miranda_avatarcache%");

	CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers\\*.png", wszCache.get(), m_szModuleName);
	SMADD_CONT cont = { 2, m_szModuleName, wszPath };
	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));

	wszPath.Format(L"%s\\%S\\Stickers\\*.webp", wszCache.get(), m_szModuleName);
	cont.path = wszPath;
	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));

	// contacts cache
	for (auto &cc : AccContacts()) {
		CMStringA szId(getMStringA(cc, DBKEY_ID));
		if (!szId.IsEmpty())
			m_users.insert(new FacebookUser(_atoi64(szId), cc, isChatRoom(cc)));
	}

	// Default group
	Clist_GroupCreate(0, m_wszDefaultGroup);
}

void FacebookProto::OnShutdown()
{
	if (m_mqttConn != nullptr)
		Netlib_Shutdown(m_mqttConn);
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT FacebookProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	if (!mir_wstrlen(psr->id.w))
		return 0;

	if (auto *pUser = FindUser(_wtoi64(psr->id.w)))
		return pUser->hContact;

	MCONTACT hContact = db_add_contact();
	setWString(hContact, DBKEY_ID, psr->id.w);
	Proto_AddToContact(hContact, m_szModuleName);
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR FacebookProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		{
			DWORD_PTR flags = PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ;

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
		return (INT_PTR) L"Facebook ID";
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int FacebookProto::SendMsg(MCONTACT hContact, int, const char *pszSrc)
{
	if (!m_bOnline) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)TranslateT("Protocol is offline or user isn't authorized yet"));
		return 1;
	}

	CMStringA userId(getMStringA(hContact, DBKEY_ID));

	__int64 msgId;
	Utils_GetRandom(&msgId, sizeof(msgId));
	msgId = abs(msgId);

	JSONNode root; root << CHAR_PARAM("body", pszSrc) << INT64_PARAM("msgid", msgId) << INT64_PARAM("sender_fbid", m_uid) << CHAR_PARAM("to", userId);
	MqttPublish("/send_message2", root);

	mir_cslock lck(m_csOwnMessages);
	arOwnMessages.insert(new COwnMessage(msgId, m_mid, hContact));
	return m_mid;
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
	case ID_STATUS_OFFLINE:
		break;

	default:
		iNewStatus = ID_STATUS_AWAY;
		break;
	}

	if (m_iStatus == iNewStatus) {
		debugLogA("=== Statuses are same, no change");
		return 0;
	}

	m_iDesiredStatus = iNewStatus;

	int iOldStatus = m_iStatus;

	// log off & free all resources
	if (iNewStatus == ID_STATUS_OFFLINE) {
		OnShutdown();

		m_iStatus = ID_STATUS_OFFLINE;
	}
	else if (m_iStatus == ID_STATUS_OFFLINE) { // we gonna connect
		debugLogA("*** Beginning SignOn process");

		m_iStatus = ID_STATUS_CONNECTING;

		ForkThread(&FacebookProto::ServerThread);
	}
	else m_iStatus = iNewStatus;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::UserIsTyping(MCONTACT hContact, int type)
{
	JSONNode root; root << INT_PARAM("state", type == PROTOTYPE_SELFTYPING_ON) << CHAR_PARAM("to", getMStringA(hContact, DBKEY_ID));
	MqttPublish("/typing", root);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Services

INT_PTR FacebookProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FACEBOOKACCOUNT),
		(HWND) lParam, FBAccountProc, (LPARAM) this);
}
