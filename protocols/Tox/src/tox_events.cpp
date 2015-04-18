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
	dbEventType.descr = Translate("Outgoint call");
	dbEventType.eventIcon = GetIconHandle("audio_call");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_RING;
	dbEventType.descr = Translate("Incoming call");
	dbEventType.eventIcon = GetIconHandle("audio_ring");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_START;
	dbEventType.descr = Translate("Call started");
	dbEventType.eventIcon = GetIconHandle("audio_start");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_AUDIO_END;
	dbEventType.descr = Translate("Call ended");
	dbEventType.eventIcon = GetIconHandle("audio_end");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}