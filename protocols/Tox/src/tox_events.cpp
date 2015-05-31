#include "stdafx.h"

int CToxProto::OnModulesLoaded(WPARAM, LPARAM)
{
	CToxProto::InitIcons();
	CToxProto::InitMenus();

	hProfileFolderPath = FoldersRegisterCustomPathT("Tox", Translate("Profiles folder"), MIRANDA_USERDATAT);

	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
	{
		CreateServiceFunction(MODULE "/ParseUri", CToxProto::ParseToxUri);
		AssocMgr_AddNewUrlTypeT("tox:", TranslateT("Tox URI scheme"), g_hInstance, IDI_TOX, MODULE "/ParseUri", 0);
	}

	return 0;
}

void CToxProto::InitCustomDbEvents()
{
	DBEVENTTYPEDESCR dbEventType = { sizeof(dbEventType) };
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = DB_EVENT_ACTION;
	dbEventType.descr = Translate("Action");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);

	dbEventType.eventType = DB_EVENT_CALL;
	dbEventType.descr = Translate("Call");
	dbEventType.eventIcon = GetIconHandle("audio_start");
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&dbEventType);
}