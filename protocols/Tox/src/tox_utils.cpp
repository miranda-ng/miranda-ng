#include "common.h"

TOX_USERSTATUS CToxProto::MirandaToToxStatus(int status)
{
	TOX_USERSTATUS userstatus = TOX_USERSTATUS_NONE;
	switch (status)
	{
	case ID_STATUS_AWAY:
		userstatus = TOX_USERSTATUS_AWAY;
		break;
	case ID_STATUS_OCCUPIED:
		userstatus = TOX_USERSTATUS_BUSY;
		break;
	}
	return userstatus;
}

int CToxProto::ToxToMirandaStatus(TOX_USERSTATUS userstatus)
{
	int status = ID_STATUS_OFFLINE;
	switch (userstatus)
	{
	case TOX_USERSTATUS_NONE:
		status = ID_STATUS_ONLINE;
		break;
	case TOX_USERSTATUS_AWAY:
		status = ID_STATUS_AWAY;
		break;
	case TOX_USERSTATUS_BUSY:
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
	//return ::GetFileAttributes(fileName) != DWORD(-1)
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		FindClose(hFind);
		return true;
	}
	return false;
}