#include "common.h"

char *CSkypeProto::MirandaToSkypeStatus(int status)
{
	char *result = "Online";
	switch (status)
	{
	case ID_STATUS_AWAY:
		return "Away";

	case ID_STATUS_DND:
		return "Busy";

	case ID_STATUS_IDLE:
		return "Idle";

	case ID_STATUS_INVISIBLE:
		return "Hidden";
	}
	return "Online";
}

void CSkypeProto::ShowNotification(const TCHAR *caption, const TCHAR *message, int flags, MCONTACT hContact)
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
		ppd.lchIcon = Skin_GetIcon("Skype_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void CSkypeProto::ShowNotification(const TCHAR *message, int flags, MCONTACT hContact)
{
	ShowNotification(_T(MODULE), message, flags, hContact);
}

bool CSkypeProto::IsFileExists(std::tstring path)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(path.c_str(), &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		FindClose(hFind);
		return true;
	}
	return false;
}

char *CSkypeProto::ContactUrlToName(const char *url)
{
	char *tempname = NULL;
	const char *start, *end;
	start = strstr(url, "/8:");

	if (!start)
		return NULL;
	start = start + 3;
	if ((end = strchr(start, '/'))) 
	{
		mir_free(tempname);
		tempname = mir_strndup(start, end - start);
		return tempname;
	}
	mir_free(tempname);
	tempname = mir_strdup(start);

	return tempname;
}
