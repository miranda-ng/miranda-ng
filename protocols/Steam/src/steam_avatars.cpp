#include "common.h"

wchar_t * CSteamProto::GetAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	ptrW steamId(db_get_wsa(hContact, m_szModuleName, "SteamID"));
	if (hContact != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, steamId);
	else if (steamId != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, steamId);
	else
		return NULL;

	return mir_wstrdup(path);
}

INT_PTR CSteamProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW *)lParam;

	if (ptrA(getStringA(pai->hContact, "AvatarUrl")))
		return GAIR_NOAVATAR;

	ptrA steamId(getStringA(pai->hContact, "SteamID"));
	if (steamId)
	{
		ptrW path(GetAvatarFilePath(pai->hContact));
		if (path && !_waccess(path, 0))
		{
			wcsncpy(pai->filename, path, SIZEOF(pai->filename));
			pai->format = PA_FORMAT_JPEG;
			return GAIR_SUCCESS;
		}
	}

	return GAIR_NOAVATAR;
}

INT_PTR CSteamProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case AF_MAXSIZE:
	{
		POINT *size = (POINT*)lParam;
		if (size)
		{
			size->x = 184;
			size->y = 184;
		}
	}
	break;
	
	case AF_PROPORTION:
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED:
		return lParam == PA_FORMAT_JPEG;

	case AF_ENABLED:
		return 1;
	
	/*case AF_DONTNEEDDELAYS:
		return 1;*/

	/*case AF_MAXFILESIZE:
		// server accepts images of 32000 bytees, not bigger
		return 32000;*/
	
	/*case AF_DELAYAFTERFAIL:
		// do not request avatar again if server gave an error
		return 1;// * 60 * 60 * 1000; // one hour*/
	
	/*case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		// avatars can be fetched all the time (server only operation)
		return 1;*/
	}

	return 0;
}

INT_PTR CSteamProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		return -2;

	ptrW path(GetAvatarFilePath(NULL));
	if (path && !_waccess(path, 0))
	{
		wcsncpy((wchar_t *)wParam, path, (int)lParam);
		return 0;
	}

	return -1;
}