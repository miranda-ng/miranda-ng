#include "stdafx.h"

LONGLONG GetLastSentMessageTime(MCONTACT hContact)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent))
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		if (FLAG_CONTAINS(dbei.flags, DBEF_SENT))
			return dbei.timestamp;
	}
	return -1;
}

void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time)
{
	if (hContact && arMonitoredWindows.getIndex((HANDLE)hContact) != -1)
	{
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULENAME;
		sid.dwId = 1;
		sid.flags = MBF_TCHAR;

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
				CMString tooltip;
				if (db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME_TYPE, -1) == MRD_TYPE_READTIME)
				{
					TCHAR ttime[64];
					_locale_t locale = _create_locale(LC_ALL, "");
					_tcsftime_l(ttime, _countof(ttime), _T("%X %x"), localtime(&time), locale);
					_free_locale(locale);
					tooltip.Format(L"%s %s", TranslateT("Last message read at"), ttime);
				}
				else
				{
					tooltip = TranslateT("Last message read (unknown time)");
				}
				sid.tszTooltip = tooltip.Detach();
				break;
			}
		case ICON_UNREAD:
			{
				sid.hIcon = IcoLib_GetIcon("unread_icon");
				sid.tszTooltip = TranslateT("Last message is not read");
				break;
			}
		case ICON_FAILED:
			{
				sid.hIcon = IcoLib_GetIcon("fail_icon");
				sid.tszTooltip = TranslateT("Last message was not sent.");
				break;
			}
		case ICON_NOSENT:
			{
				sid.hIcon = IcoLib_GetIcon("nosent_icon");
				sid.tszTooltip = TranslateT("Sending...");
				break;
			}
		default:
			return;
		}

		Srmm_ModifyIcon(hContact, &sid);
	}
}

int IconsUpdate(WPARAM hContact, LONGLONG readtime)
{
	if (hContact == NULL) return 1;

	LONGLONG lasttime = GetLastSentMessageTime(hContact);
	if (lasttime != -1 && readtime != 0)
	{
		SetSRMMIcon(hContact, (readtime >= lasttime) ? ICON_READ : ICON_UNREAD, readtime);
	}
	else 
		SetSRMMIcon(hContact, ICON_HIDDEN);
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

INT_PTR UpdateService(WPARAM hContact, LPARAM lParam)
{
	MessageReadData *mrd = (MessageReadData*)lParam;
	if (mrd->dw_lastTime > db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0))
	{
		db_set_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, mrd->dw_lastTime);
		db_set_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME_TYPE, mrd->iTimeType);
		IconsUpdate(hContact, mrd->dw_lastTime);
		ExtraIconsApply(hContact, 0);
	}
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