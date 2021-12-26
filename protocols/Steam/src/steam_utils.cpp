#include "stdafx.h"

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
		return ID_STATUS_FREECHAT;
	case PersonaState::LookingToTrade:
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
