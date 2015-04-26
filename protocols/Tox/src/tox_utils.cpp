#include "stdafx.h"

TOX_USER_STATUS CToxProto::MirandaToToxStatus(int status)
{
	TOX_USER_STATUS userstatus = TOX_USER_STATUS_NONE;
	switch (status)
	{
	case ID_STATUS_AWAY:
		userstatus = TOX_USER_STATUS_AWAY;
		break;
	case ID_STATUS_OCCUPIED:
		userstatus = TOX_USER_STATUS_BUSY;
		break;
	}
	return userstatus;
}

int CToxProto::ToxToMirandaStatus(TOX_USER_STATUS userstatus)
{
	int status = ID_STATUS_OFFLINE;
	switch (userstatus)
	{
	case TOX_USER_STATUS_NONE:
		status = ID_STATUS_ONLINE;
		break;
	case TOX_USER_STATUS_AWAY:
		status = ID_STATUS_AWAY;
		break;
	case TOX_USER_STATUS_BUSY:
		status = ID_STATUS_OCCUPIED;
		break;
	}
	return status;
}

void CToxProto::ShowNotification(const TCHAR *caption, const TCHAR *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated())
	{
		return;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Tox_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void CToxProto::ShowNotification(const TCHAR *message, int flags, MCONTACT hContact)
{
	ShowNotification(_T(MODULE), message, flags, hContact);
}

bool CToxProto::IsFileExists(std::tstring path)
{
	return _taccess(path.c_str(), 0) == 0;
}

MEVENT CToxProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, PBYTE pBlob, DWORD cbBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}