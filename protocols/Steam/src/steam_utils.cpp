#include "stdafx.h"

uint64_t getRandomInt()
{
	uint64_t ret;
	Utils_GetRandom(&ret, sizeof(ret));
	return ret & INT64_MAX;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool IsNull(const ProtobufCBinaryData &buf)
{
	for (auto i = 0; i < buf.len; i++)
		if (buf.data[i] != 0)
			return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer createMachineID(const char *accName)
{
	uint8_t hashOut[MIR_SHA1_HASH_SIZE];
	char hashHex[MIR_SHA1_HASH_SIZE * 2 + 1];

	CMStringA _bb3 = CMStringA("SteamUser Hash BB3 ") + accName;
	CMStringA _ff2 = CMStringA("SteamUser Hash FF2 ") + accName;
	CMStringA _3b3 = CMStringA("SteamUser Hash 3B3 ") + accName;

	MBinBuffer ret;
	uint8_t c = 0;
	ret.append(&c, 1);
	ret.append("MessageObject", 14);

	c = 1;
	ret.append(&c, 1);
	ret.append("BB3", 4);
	mir_sha1_hash((uint8_t *)_bb3.c_str(), _bb3.GetLength(), hashOut);
	bin2hex(hashOut, sizeof(hashOut), hashHex);
	ret.append(hashHex, 41);

	ret.append(&c, 1);
	ret.append("FF2", 4);
	mir_sha1_hash((uint8_t *)_ff2.c_str(), _ff2.GetLength(), hashOut);
	bin2hex(hashOut, sizeof(hashOut), hashHex);
	ret.append(hashHex, 41);

	ret.append(&c, 1);
	ret.append("3B3", 4);
	mir_sha1_hash((uint8_t *)_3b3.c_str(), _3b3.GetLength(), hashOut);
	bin2hex(hashOut, sizeof(hashOut), hashHex);
	ret.append(hashHex, 41);

	ret.append("\x08\x08", 2);
	return ret;
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
// Statuses

int SteamToMirandaStatus(PersonaState state)
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

PersonaState MirandaToSteamState(int status)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Popups

void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
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

void ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(_A2W(MODULE), message, flags, hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

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
