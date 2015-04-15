#include "common.h"

int CToxProto::OnModulesLoaded(WPARAM, LPARAM)
{
	CToxProto::InitIcons();
	CToxProto::InitMenus();
	CToxProto::InitCustomDbEvents();

	hProfileFolderPath = FoldersRegisterCustomPathT("Tox", Translate("Profiles folder"), MIRANDA_USERDATAT);

	return 0;
}

void CToxProto::InitCustomDbEvents()
{
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = MODULE;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = DB_EVENT_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_CALL;
	dbEventType.descr = Translate("Audio call");
	dbEventType.eventIcon = GetIconHandle("audio_call");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_RING;
	dbEventType.descr = Translate("Audio ring");
	dbEventType.eventIcon = GetIconHandle("audio_ring");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_START;
	dbEventType.descr = Translate("Audio start");
	dbEventType.eventIcon = GetIconHandle("audio_start");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_END;
	dbEventType.descr = Translate("Audio end");
	dbEventType.eventIcon = GetIconHandle("audio_end");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}

int CToxProto::OnDbEventAdded(WPARAM hContact, LPARAM hEvent)
{
	DWORD dwSignature;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD);
	dbei.pBlob = (PBYTE)&dwSignature;
	db_event_get(hEvent, &dbei);
	if (dbei.flags & (DBEF_SENT | DBEF_READ) || dbei.eventType <= DB_EVENT_ACTION || dwSignature == 0)
		return 0;

	DBEVENTTYPEDESCR *dbEventType = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)MODULE, dbei.eventType);
	if (dbEventType == NULL)
		return 0;

	CLISTEVENT cle = { sizeof(cle) };
	cle.flags |= CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.hIcon = Skin_GetIconByHandle(dbEventType->eventIcon);

	TCHAR szTooltip[256];
	mir_sntprintf(szTooltip, SIZEOF(szTooltip), _T("%s %s %s"), _A2T(dbEventType->descr), TranslateT("from"), pcli->pfnGetContactDisplayName(hContact, 0));
	cle.ptszTooltip = szTooltip;

	char szService[256];
	switch (dbei.eventType)
	{
	case DB_EVENT_AUDIO_CALL:
		mir_snprintf(szService, SIZEOF(szService), "%s/Audio/Call", GetContactProto(hContact));
		break;

	case DB_EVENT_AUDIO_RING:
		mir_snprintf(szService, SIZEOF(szService), "%s/Audio/Ring", GetContactProto(hContact));
		break;

	case DB_EVENT_AUDIO_START:
		mir_snprintf(szService, SIZEOF(szService), "%s/Audio/Start", GetContactProto(hContact));
		break;

	case DB_EVENT_AUDIO_END:
		mir_snprintf(szService, SIZEOF(szService), "%s/Audio/End", GetContactProto(hContact));
		break;

	default:
		return 0;
	}
	cle.pszService = szService;

	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

	return 0;
}