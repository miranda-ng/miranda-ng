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

TCHAR* CToxProto::ToxErrorToString(TOX_ERR_NEW error)
{
	switch (error)
	{
	case TOX_ERR_NEW_NULL:
		return TranslateT("One of the arguments is not valid or ");
	case TOX_ERR_NEW_MALLOC:
		return TranslateT("Unable to allocate enough memory");
	case TOX_ERR_NEW_PORT_ALLOC:
		return TranslateT("Unable to bind to a port");
	case TOX_ERR_NEW_PROXY_BAD_TYPE:
		return TranslateT("The proxy type is not valid");
	case TOX_ERR_NEW_PROXY_BAD_HOST:
		return TranslateT("The proxy host is not valid");
	case TOX_ERR_NEW_PROXY_BAD_PORT:
		return TranslateT("The proxy port is not valid");
	case TOX_ERR_NEW_PROXY_NOT_FOUND:
		return TranslateT("The proxy address could not be resolved");
	case TOX_ERR_NEW_LOAD_ENCRYPTED:
		return TranslateT("The profile is encrypted");
	case TOX_ERR_NEW_LOAD_BAD_FORMAT:
		return TranslateT("The data format is not valid");
	default:
		return TranslateT("Unknown error");
	}
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
		ppd.lchIcon = IcoLib_GetIcon("Tox_main");

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

MEVENT CToxProto::AddEventToDb(MCONTACT hContact, WORD type, DWORD timestamp, DWORD flags, PBYTE pBlob, size_t cbBlob)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = this->m_szModuleName;
	dbei.timestamp = timestamp;
	dbei.eventType = type;
	dbei.cbBlob = (DWORD)cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

INT_PTR CToxProto::ParseToxUri(WPARAM, LPARAM lParam)
{
	TCHAR *uri = (TCHAR*)lParam;
	if (mir_tstrlen(uri) <= 4)
		return 1;

	if (Accounts.getCount() == 0)
		return 1;

	CToxProto *proto = NULL;
	for (int i = 0; i < Accounts.getCount(); i++)
	{
		if (Accounts[i]->IsOnline())
		{
			proto = Accounts[i];
			break;
		}
	}
	if (proto == NULL)
		return 1;

	if (_tcschr(uri, _T('@')) != NULL)
		return 1;

	PROTOSEARCHRESULT psr = { sizeof(psr) };
	psr.flags = PSR_UTF8;
	psr.id.a = mir_t2a(&uri[4]);

	ADDCONTACTSTRUCT acs = { HANDLE_SEARCHRESULT };
	acs.szProto = proto->m_szModuleName;
	acs.psr = &psr;

	CallService(MS_ADDCONTACT_SHOW, 0, (LPARAM)&acs);
	return 0;
}