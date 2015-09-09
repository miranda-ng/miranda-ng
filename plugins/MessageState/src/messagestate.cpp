#include "stdafx.h"

void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time)
{
	if (hContact && arMonitoredWindows.getIndex((HANDLE)hContact) != -1)
	{
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULENAME;
		sid.dwId = 1;
		sid.flags = MBF_TCHAR;

		CMString tszTooltip;

		switch (type)
		{
		case ICON_HIDDEN:
			{
				sid.flags |= MBF_HIDDEN;
				break;
			}
		case ICON_READ:
			{
				sid.hIcon = IcoLib_GetIcon("read_icon");
				
				if (db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME_TYPE, -1) == MRD_TYPE_READTIME)
				{
					TCHAR ttime[64];
					_locale_t locale = _create_locale(LC_ALL, "");
					_tcsftime_l(ttime, _countof(ttime), _T("%X %x"), localtime(&time), locale);
					_free_locale(locale);
					tszTooltip.Format(L"%s %s", TranslateT("Last message read at"), ttime);
				}
				else
				{
					tszTooltip = TranslateT("Last message read (unknown time)");
				}
				break;
			}
		case ICON_UNREAD:
			{
				sid.hIcon = IcoLib_GetIcon("unread_icon");
				tszTooltip = TranslateT("Last message is not read");
				break;
			}
		case ICON_FAILED:
			{
				sid.hIcon = IcoLib_GetIcon("fail_icon");
				tszTooltip = TranslateT("Last message was not sent.");
				break;
			}
		case ICON_NOSENT:
			{
				sid.hIcon = IcoLib_GetIcon("nosent_icon");
				tszTooltip = TranslateT("Sending...");
				break;
			}
		default:
			return;
		}

		sid.tszTooltip = tszTooltip.GetBuffer();

		Srmm_ModifyIcon(hContact, &sid);
	}
}

int IconsUpdate(WPARAM hContact, LONGLONG readtime)
{
	if (hContact == NULL) return 1;

	LONGLONG lasttime = GetLastSentMessageTime(hContact);
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
	if (pAck && pAck->type == ACKTYPE_MESSAGE && CheckProtoSupport(pAck->szModule))
	{
		if (pAck->result == ACKRESULT_SUCCESS)
		{
			SetSRMMIcon(pAck->hContact, ICON_UNREAD);
			ExtraIconsApply(pAck->hContact, 0);
		}
		else if (pAck->result == ACKRESULT_FAILED)
		{
			SetSRMMIcon(pAck->hContact, ICON_FAILED);
		}
	}
	return 0;
}

int	OnEventFilterAdd(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if (FLAG_CONTAINS(dbei->flags, DBEF_SENT) && CheckProtoSupport(dbei->szModule) && db_get_b(hContact, "Tab_SRMsg", "no_ack", 0))
		SetSRMMIcon(hContact, ICON_NOSENT);
	ExtraIconsApply(hContact, 0);
	return 0;
}

int OnSrmmWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL)
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		const char *szProto = GetContactProto(event->hContact);
		if (CheckProtoSupport(szProto))
		{
			arMonitoredWindows.insert((HANDLE)event->hContact);
			IconsUpdate(event->hContact, db_get_dw(event->hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0));
		}
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSE)
		arMonitoredWindows.remove(event->hContact);

	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowEvent);
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnEventFilterAdd);
	
	// IcoLib support
	for (size_t i = 0; i < _countof(Icons); i++)
		Icon_Register(g_hInst, MODULENAME, &Icons[i], 1);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 1;
	Srmm_AddIcon(&sid);

	InitClistExtraIcon();
	
	return 0;
}