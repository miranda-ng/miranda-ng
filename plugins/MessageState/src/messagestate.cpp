#include "stdafx.h"

IconItem Icons[] =
{
	{ LPGEN("Unread message icon"), "unread_icon", IDI_UNREAD },
	{ LPGEN("Read message icon"), "read_icon", IDI_READ },
	{ LPGEN("Failed sending icon"), "fail_icon", IDI_FAIL },
	{ LPGEN("Sending message icon"), "nosent_icon", IDI_NOSENT },
	{ LPGEN("Unread clist extra icon"), "clist_unread_icon", IDI_EXTRA },
};

const wchar_t* Tooltips[] =
{
	LPGENW("Last message is not read"),
	LPGENW("Last message read"),
	LPGENW("Last message was not sent"),
	LPGENW("Sending...")
};

void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time)
{
	MCONTACT hActualContact;
	if (db_mc_isMeta(hContact))
		hActualContact = db_mc_getSrmmSub(hContact);
	else
		hActualContact = hContact;

	if (type != ICON_HIDDEN) {
		const wchar_t *pwszToolTip;
		if (type == ICON_READ) {
			if (g_plugin.getDword(hActualContact, DBKEY_MESSAGE_READ_TIME_TYPE, -1) == MRD_TYPE_READTIME) {
				wchar_t buf[100];
				wcsftime(buf, _countof(buf), TranslateT("Last message read at %X %x"), localtime(&time));
				pwszToolTip = buf;
			}
			else pwszToolTip = TranslateT("Last message read (unknown time)");
		}
		else pwszToolTip = TranslateW(Tooltips[type]);
		
		Srmm_ModifyIcon(hContact, MODULENAME, 1, IcoLib_GetIconByHandle(Icons[type].hIcolib), pwszToolTip);
	}
	else Srmm_SetIconFlags(hContact, MODULENAME, 1, MBF_HIDDEN);
}

int IconsUpdate(MCONTACT hContact)
{
	MCONTACT hActualContact;
	if (db_mc_isMeta(hContact))
		hActualContact = db_mc_getSrmmSub(hContact);
	else
		hActualContact = hContact;

	time_t readtime = g_plugin.getDword(hActualContact, DBKEY_MESSAGE_READ_TIME, 0);
	time_t lasttime = GetLastSentMessageTime(hActualContact);
	if (lasttime != -1 && readtime != 0)
		SetSRMMIcon(hContact, HasUnread(hActualContact) ? ICON_UNREAD : ICON_READ, readtime);
	else
		SetSRMMIcon(hContact, ICON_HIDDEN);

	ExtraIconsApply(hContact, 0);
	return 0;
}

static int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && (pAck->type == ACKTYPE_MESSAGE || pAck->type == ACKTYPE_FILE) && CheckProtoSupport(pAck->szModule)) {
		if (pAck->result == ACKRESULT_SUCCESS)
			SetSRMMIcon(pAck->hContact, ICON_UNREAD);
		else if (pAck->result == ACKRESULT_FAILED)
			SetSRMMIcon(pAck->hContact, ICON_FAILED);

		ExtraIconsApply(pAck->hContact, 0);
	}
	return 0;
}

static int OnEventFilterAdd(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if ((dbei->flags & DBEF_SENT) && CheckProtoSupport(dbei->szModule) && db_get_b(hContact, "Tab_SRMsg", "no_ack", 0))
		SetSRMMIcon(hContact, ICON_NOSENT);
	return 0;
}

static int OnSrmmWindowOpened(WPARAM, LPARAM lParam)
{
	auto *pEvent = (MessageWindowEventData*)lParam;
	if (pEvent->uType == MSG_WINDOW_EVT_OPENING)
		IconsUpdate(pEvent->hContact);
	return 0;
}

static int OnMetaChanged(WPARAM hContact, LPARAM)
{
	IconsUpdate(hContact);
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnEventFilterAdd);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaChanged);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, OnMetaChanged);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened);

	g_plugin.registerIcon(MODULENAME, Icons);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 1;
	Srmm_AddIcon(&sid, &g_plugin);

	InitClistExtraIcon();

	for (auto &hContact : Contacts())
		IconsUpdate(hContact);

	return 0;
}
