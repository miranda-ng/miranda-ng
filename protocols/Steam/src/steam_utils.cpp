#include "stdafx.h"

static bool sttIncludesSessionId(const ProtobufCppMessage &msg)
{
	if (!mir_strcmp(msg.descriptor->short_name, "CMsgClientHello"))
		return false;
	if (!mir_strcmp(msg.descriptor->short_name, "CMsgServiceMethodCallFromClientNonAuthed"))
		return false;
	return true;
}

void CSteamProto::WSSend(int msgType, const ProtobufCppMessage &msg)
{
	msgType |= STEAM_PROTOCOL_MASK;

	CMsgProtoBufHeader hdr;
	hdr.has_client_sessionid = hdr.has_steamid = hdr.has_jobid_source = hdr.has_jobid_target = true;
	if (sttIncludesSessionId(msg)) {
	}
	else {
		hdr.client_sessionid = 0;
		hdr.steamid = 0;
	}
	hdr.jobid_source = hdr.jobid_target = -1;

	unsigned hdrLen = (unsigned)protobuf_c_message_get_packed_size(&hdr);
	MBinBuffer hdrbuf(hdrLen);
	protobuf_c_message_pack(&hdr, (uint8_t *)hdrbuf.data());
	hdrbuf.appendBefore(&hdrLen, sizeof(hdrLen));
	hdrbuf.appendBefore(&msgType, sizeof(msgType));
	Netlib_Dump(m_hServerConn, hdrbuf.data(), hdrbuf.length(), true, 0);

	MBinBuffer body(protobuf_c_message_get_packed_size(&msg));
	protobuf_c_message_pack(&msg, body.data());
	Netlib_Dump(m_hServerConn, body.data(), body.length(), true, 0);

	hdrbuf.append(body);
	WebSocket_SendBinary(m_hServerConn, hdrbuf.data(), hdrbuf.length());
}

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
