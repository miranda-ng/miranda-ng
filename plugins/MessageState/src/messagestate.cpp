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
	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.dwId = 1;
	sid.flags = MBF_UNICODE;

	CMStringW tszTooltip;

	if (type != ICON_HIDDEN)
	{
		sid.hIcon = IcoLib_GetIconByHandle(Icons[type].hIcolib);

		if (type == ICON_READ)
		{
			if (db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME_TYPE, -1) == MRD_TYPE_READTIME)
			{
				wcsftime(tszTooltip.GetBuffer(64), 64, TranslateT("Last message read at %X %x"), localtime(&time));
				tszTooltip.ReleaseBuffer();
			}
			else
			{
				tszTooltip = TranslateT("Last message read (unknown time)");
			}
		}
		else 
			tszTooltip = TranslateW(Tooltips[type]);

		sid.tszTooltip = tszTooltip.GetBuffer();
	}
	else
	{
		sid.flags |= MBF_HIDDEN;
	}

	Srmm_ModifyIcon(hContact, &sid);
}

int IconsUpdate(MCONTACT hContact)
{
	time_t readtime = db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0);
	time_t lasttime = GetLastSentMessageTime(hContact);
	if (lasttime != -1 && readtime != 0)
	{
		SetSRMMIcon(hContact, HasUnread(hContact) ? ICON_UNREAD : ICON_READ , readtime);
	}
	else
	{
		SetSRMMIcon(hContact, ICON_HIDDEN);
	}

	ExtraIconsApply(hContact, 0);

	return 0;
}

int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && (pAck->type == ACKTYPE_MESSAGE || pAck->type == ACKTYPE_FILE) && CheckProtoSupport(pAck->szModule))
	{
		if (pAck->result == ACKRESULT_SUCCESS)     SetSRMMIcon(pAck->hContact, ICON_UNREAD);
		else if (pAck->result == ACKRESULT_FAILED) SetSRMMIcon(pAck->hContact, ICON_FAILED);

		ExtraIconsApply(pAck->hContact, 0);
	}
	return 0;
}

int	OnEventFilterAdd(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if ((dbei->flags & DBEF_SENT) && CheckProtoSupport(dbei->szModule) && db_get_b(hContact, "Tab_SRMsg", "no_ack", 0))
		SetSRMMIcon(hContact, ICON_NOSENT);
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnEventFilterAdd);
	
	Icon_Register(g_hInst, MODULENAME, Icons, _countof(Icons));

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 1;
	Srmm_AddIcon(&sid);

	InitClistExtraIcon();
	
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		IconsUpdate(hContact);

	return 0;
}