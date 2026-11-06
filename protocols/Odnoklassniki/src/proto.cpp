/*
Copyright (C) 2026 Miranda NG team

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

COdnoklassnikiProto::COdnoklassnikiProto(const char *szModuleName, const wchar_t *wszUserName) :
	PROTO<COdnoklassnikiProto>(szModuleName, wszUserName),
	m_wszLogin(this, "Login", L""),
	m_iPollInterval(this, "PollInterval", 15)
{
	m_hPollEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateProtoService(PS_OFFLINEFILE, &COdnoklassnikiProto::SvcOfflineFile);

	HookProtoEvent(ME_OPT_INITIALISE, &COdnoklassnikiProto::OnOptionsInit);
	HookProtoEvent(ME_GC_EVENT, &COdnoklassnikiProto::GcEventHook);

	// group chats keep their messages in the database, like private ones
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_DATABASE | GC_PERSISTENT;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);
}

COdnoklassnikiProto::~COdnoklassnikiProto()
{
	CloseHandle(m_hPollEvent);
}

void COdnoklassnikiProto::OnModulesLoaded()
{
	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void COdnoklassnikiProto::OnShutdown()
{
	m_bTerminated = true;
	SetEvent(m_hPollEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////
// contacts: private ones are keyed by the OK user id, group chats by the conversation id

MCONTACT COdnoklassnikiProto::FindUser(const char *uid)
{
	for (auto &hContact : AccContacts())
		if (!isChatRoom(hContact) && getMStringA(hContact, DB_KEY_ID) == uid)
			return hContact;
	return 0;
}

MCONTACT COdnoklassnikiProto::AddUser(const char *uid, bool bTemporary)
{
	MCONTACT hContact = FindUser(uid);
	if (hContact) {
		if (!bTemporary && !Contact::OnList(hContact))
			Contact::PutOnList(hContact);
		return hContact;
	}

	hContact = db_add_contact(m_szModuleName, bTemporary ? DBAC_NOTINLIST : 0);
	setString(hContact, DB_KEY_ID, uid);

	CMStringW wszName(GetUserName(uid));
	if (!wszName.IsEmpty())
		setWString(hContact, "Nick", wszName);
	return hContact;
}

MCONTACT COdnoklassnikiProto::FindChat(const char *cnvId)
{
	for (auto &hContact : AccContacts())
		if (isChatRoom(hContact) && getMStringA(hContact, DB_KEY_CONV) == cnvId)
			return hContact;
	return 0;
}

// a one-to-one conversation id is base64("PRIVATE:<peer uid>")
CMStringA COdnoklassnikiProto::GetConversationId(MCONTACT hContact)
{
	if (isChatRoom(hContact))
		return getMStringA(hContact, DB_KEY_CONV);

	CMStringA szUid(getMStringA(hContact, DB_KEY_ID));
	if (szUid.IsEmpty())
		return CMStringA();

	CMStringA szRaw("PRIVATE:" + szUid);
	return ptrA(mir_base64_encode(szRaw.c_str(), szRaw.GetLength())).get();
}

CMStringW COdnoklassnikiProto::GetUserName(const char *uid)
{
	mir_cslock lck(m_csNames);
	auto it = m_names.find(uid);
	return (it == m_names.end()) ? CMStringW() : it->second;
}

/////////////////////////////////////////////////////////////////////////////////////////
// capabilities

INT_PTR COdnoklassnikiProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SERVERMSGID;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("User ID");
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// status: OK has no presence of our own to set, so there is online (polling) and offline

int COdnoklassnikiProto::SetStatus(int iNewStatus)
{
	if (iNewStatus != ID_STATUS_OFFLINE)
		iNewStatus = ID_STATUS_ONLINE;

	int iOldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_bTerminated = true;
		SetEvent(m_hPollEvent);
		if (!m_bOnline && m_iStatus != ID_STATUS_OFFLINE && !IsStatusConnecting(m_iStatus)) {
			m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		}
		return 0;
	}

	if (m_bOnline || IsStatusConnecting(m_iStatus))
		return 0;

	if (mir_wstrlen(ptrW(m_wszLogin)) == 0 || (getMStringA("Password").IsEmpty() && getMStringA(DB_KEY_TOKEN).IsEmpty())) {
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_BADUSERID);
		return 0;
	}

	m_bTerminated = false;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	ForkThread(&COdnoklassnikiProto::ServerThread);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// messages

// returns the new message id, or an empty string with szError filled in
CMStringA COdnoklassnikiProto::SendMessage(const CMStringA &cnvId, const char *pszText, const CMStringA &szPhotoToken, CMStringA &szError)
{
	char szUuid[16], szUuidText[33];
	Utils_GetRandom(szUuid, sizeof(szUuid));
	bin2hex(szUuid, sizeof(szUuid), szUuidText);

	std::map<CMStringA, CMStringA> params;
	params["cnv_id"] = cnvId;
	params["text"] = pszText ? pszText : "";
	params["uuid"] = szUuidText;
	if (!szPhotoToken.IsEmpty())
		params["attachments"] = "{\"attachments\":[{\"type\":\"UPLOADED_PHOTO\",\"token\":\"" + szPhotoToken + "\"}]}";

	auto reply = Call("api/messagesV2/send", params);
	if (!reply) {
		szError = reply.error;
		return CMStringA();
	}

	CMStringA szId(reply.root["message_id"].as_string().c_str());
	if (szId.IsEmpty())
		szId = reply.root["id"].as_string().c_str();
	return szId;
}

void COdnoklassnikiProto::SendMsgThread(void *param)
{
	auto *p = (SendParam *)param;

	CMStringA szError, cnvId(GetConversationId(p->hContact));
	CMStringA szId = cnvId.IsEmpty() ? CMStringA() : SendMessage(cnvId, p->text, CMStringA(), szError);

	if (!isChatRoom(p->hContact)) {
		if (szId.IsEmpty()) {
			CMStringW wszError(FORMAT, TranslateT("The message was not sent: %S"), szError.IsEmpty() ? "unknown error" : szError.c_str());
			ProtoBroadcastAck(p->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)p->seq, (LPARAM)wszError.c_str());
		}
		else ProtoBroadcastAck(p->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)p->seq, (LPARAM)szId.c_str());
	}
	else if (szId.IsEmpty())
		debugLogA("message to chat %s was not sent: %s", cnvId.c_str(), szError.c_str());

	// a chat shows the message once the server echoes it back, so poll at once
	SetEvent(m_hPollEvent);
	delete p;
}

int COdnoklassnikiProto::SendMsg(MCONTACT hContact, MEVENT, const char *pszText)
{
	int seq = InterlockedIncrement(&m_msgSeq);
	if (!m_bOnline) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)TranslateT("You are offline"));
		return seq;
	}

	ForkThread(&COdnoklassnikiProto::SendMsgThread, new SendParam{ hContact, seq, pszText });
	return seq;
}
