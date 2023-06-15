#include "stdafx.h"

static int64_t getRandomInt()
{
	int64_t ret;
	Utils_GetRandom(&ret, sizeof(ret));
	return (ret >= 0) ? ret : -ret;
}

void CSteamProto::WSSend(EMsg msgType, const ProtobufCppMessage &msg)
{
	CMsgProtoBufHeader hdr;
	hdr.has_client_sessionid = hdr.has_steamid = hdr.has_jobid_source = hdr.has_jobid_target = true;

	switch (msgType) {
	case EMsg::ClientHello:
		hdr.jobid_source = -1;
		break;

	default:
		hdr.jobid_source = getRandomInt();
		break;
	}
	
	hdr.jobid_target = -1;

	WSSendHeader(msgType, hdr, msg);
}

void CSteamProto::WSSendHeader(EMsg msgType, const CMsgProtoBufHeader &hdr, const ProtobufCppMessage &msg)
{
	uint32_t hdrLen = (uint32_t)protobuf_c_message_get_packed_size(&hdr);
	MBinBuffer hdrbuf(hdrLen);
	protobuf_c_message_pack(&hdr, (uint8_t *)hdrbuf.data());
	hdrbuf.appendBefore(&hdrLen, sizeof(hdrLen));

	uint32_t type = (uint32_t)msgType;
	type |= STEAM_PROTOCOL_MASK;
	hdrbuf.appendBefore(&type, sizeof(type));
	Netlib_Dump(m_hServerConn, hdrbuf.data(), hdrbuf.length(), true, 0);

	MBinBuffer body(protobuf_c_message_get_packed_size(&msg));
	protobuf_c_message_pack(&msg, body.data());
	Netlib_Dump(m_hServerConn, body.data(), body.length(), true, 0);

	hdrbuf.append(body);
	WebSocket_SendBinary(m_hServerConn, hdrbuf.data(), hdrbuf.length());
}

void CSteamProto::WSSendService(const char *pszServiceName, const ProtobufCppMessage &msg, MsgCallback pCallback)
{
	CMsgProtoBufHeader hdr;
	hdr.has_client_sessionid = hdr.has_steamid = hdr.has_jobid_source = hdr.has_jobid_target = true;
	hdr.jobid_source = getRandomInt();
	hdr.jobid_target = -1;
	hdr.target_job_name = (char*)pszServiceName;
	hdr.realm = 1; hdr.has_realm = true;

	if (pCallback) {
		mir_cslock lck(m_csRequests);
		m_arRequests.insert(new ProtoRequest(hdr.jobid_source, pCallback));
	}

	WSSendHeader(EMsg::ServiceMethodCallFromClientNonAuthed, hdr, msg);
}

/////////////////////////////////////////////////////////////////////////////////////////

int64_t CSteamProto::GetId(MCONTACT hContact, const char *pszSetting)
{
	return _atoi64(getMStringA(hContact, pszSetting));
}

void CSteamProto::SetId(MCONTACT hContact, const char *pszSetting, int64_t id)
{
	char szId[100];
	_i64toa(id, szId, 10);
	setString(hContact, pszSetting, szId);
}

/////////////////////////////////////////////////////////////////////////////////////////

int64_t CSteamProto::GetId(const char *pszSetting)
{
	return _atoi64(getMStringA(pszSetting));
}

void CSteamProto::SetId(const char *pszSetting, int64_t id)
{
	char szId[100];
	_i64toa(id, szId, 10);
	setString(pszSetting, szId);
}

/////////////////////////////////////////////////////////////////////////////////////////

uint16_t CSteamProto::SteamToMirandaStatus(PersonaState state)
{
	switch (state) {
	case PersonaState::Offline:
		return ID_STATUS_OFFLINE;
	case PersonaState::Online:
		return ID_STATUS_ONLINE;
	case PersonaState::Busy:
		return ID_STATUS_DND;
	case PersonaState::Away:
		return ID_STATUS_AWAY;
	case PersonaState::Snooze:
		return ID_STATUS_NA;
	case PersonaState::LookingToPlay:
	case PersonaState::LookingToTrade:
		return ID_STATUS_FREECHAT;
	case PersonaState::Invisible:
		return ID_STATUS_INVISIBLE;
	default:
		return ID_STATUS_ONLINE;
	}
}

PersonaState CSteamProto::MirandaToSteamState(int status)
{
	switch (status) {
	case ID_STATUS_OFFLINE:
		return PersonaState::Offline;
	case ID_STATUS_ONLINE:
		return PersonaState::Online;
	case ID_STATUS_DND:
		return PersonaState::Busy;
	case ID_STATUS_AWAY:
		return PersonaState::Away;
	case ID_STATUS_NA:
		return PersonaState::Snooze;
	case ID_STATUS_FREECHAT:
		return PersonaState::LookingToPlay;
	case ID_STATUS_INVISIBLE:
		return PersonaState::Invisible;
	default:
		return PersonaState::Online;
	}
}

void CSteamProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
		return;

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = IcoLib_GetIcon(MODULE"_main");

		if (!PUAddPopupW(&ppd))
			return;
	}

	MessageBox(nullptr, message, caption, MB_OK | flags);
}

void CSteamProto::ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(_A2W(MODULE), message, flags, hContact);
}

INT_PTR CSteamProto::OnGetEventTextChatStates(WPARAM pEvent, LPARAM datatype)
{
	// Retrieves a chat state description from an event
	DBEVENTINFO *dbei = (DBEVENTINFO *)pEvent;
	if (dbei->cbBlob > 0 && dbei->pBlob[0] == STEAM_DB_EVENT_CHATSTATES_GONE)
		return (datatype == DBVT_WCHAR)
		? (INT_PTR)mir_wstrdup(TranslateT("closed chat session"))
		: (INT_PTR)mir_strdup(Translate("closed chat session"));

	return NULL;
}
