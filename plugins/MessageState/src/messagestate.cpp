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

bool CheckProtoSupport(const char *szProto)
{
	if (szProto == NULL) return false;

	DWORD caps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	return FLAG_CONTAINS(caps, PF4_READNOTIFY);
}

void SetSRMMIcon(MCONTACT hContact, SRMM_ICON_TYPE type, time_t time = 0)
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
		case ICON_READED:
			{
				sid.hIcon = Skin_GetIcon("read_icon");
				TCHAR ttime[64];
				_locale_t locale = _create_locale(LC_ALL, "");
				_tcsftime_l(ttime, SIZEOF(ttime), _T("%X %x"), localtime(&time), locale);
				_free_locale(locale);
				CMString tooltip(FORMAT, L"%s %s",  TranslateT("Last message readed at"), ttime);
				sid.tszTooltip = mir_tstrdup(tooltip.GetBuffer());
				break;
			}
		case ICON_UNREADED:
			{
				sid.hIcon = Skin_GetIcon("unread_icon");
				sid.tszTooltip = TranslateT("Last message is not readed");
				break;
			}
		case ICON_FAILED:
			{
				sid.hIcon = Skin_GetIcon("fail_icon");
				sid.tszTooltip = TranslateT("Last message send failed");
				break;
			}
		case ICON_NOSENT:
			{
				sid.hIcon = Skin_GetIcon("nosent_icon");
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
		SetSRMMIcon(hContact, (readtime >= lasttime) ? ICON_READED : ICON_UNREADED, readtime);
	}
	else 
		SetSRMMIcon(hContact, ICON_HIDDEN);
	return 0;
}

int OnProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && pAck->type == ACKTYPE_MESSAGE && CheckProtoSupport(GetContactProto(pAck->hContact)))
	{
		if (pAck->result == ACKRESULT_SUCCESS)
		{
			SetSRMMIcon(pAck->hContact, ICON_UNREADED);
		}
		else if (pAck->result == ACKRESULT_FAILED)
		{
			SetSRMMIcon(pAck->hContact, ICON_FAILED);
		}
	}
	return 0;
}

int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{	
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;	
	if (CheckProtoSupport(GetContactProto(hContact)) && cws && cws->szSetting && !mir_strcmpi(cws->szSetting, DBKEY_MESSAGE_READ_TIME))
		IconsUpdate(hContact, cws->value.dVal);
	return 0;
}

int	OnEventFilterAdd(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if (FLAG_CONTAINS(dbei->flags, DBEF_SENT) && CheckProtoSupport(dbei->szModule) && db_get_b(hContact, "Tab_SRMsg", "no_ack", 0))
		SetSRMMIcon(hContact, ICON_NOSENT);
	return 0;
}

int OnSrmmWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL)
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		char *szProto = GetContactProto(event->hContact);
		if (CheckProtoSupport(szProto))
		{
			arMonitoredWindows.insert((HANDLE)event->hContact);
			IconsUpdate(event->hContact, db_get_dw(event->hContact, szProto, DBKEY_MESSAGE_READ_TIME, 0));
		}
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSE)
		arMonitoredWindows.remove(event->hContact);

	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowEvent);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnEventFilterAdd);
	// IcoLib support
	for (size_t i = 0; i < SIZEOF(Icons); i++)
		Icon_Register(g_hInst, MODULENAME, &Icons[i], 1);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 1;
	Srmm_AddIcon(&sid);

	return 0;
}