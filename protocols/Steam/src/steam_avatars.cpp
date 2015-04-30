#include "stdafx.h"

TCHAR* CSteamProto::GetAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	ptrA steamId(getStringA(hContact, "SteamID"));
	if (steamId != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, _A2T(steamId));
	else
		return NULL;

	return mir_tstrdup(path);
}

bool CSteamProto::GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &pai)
{
	ptrT path(GetAvatarFilePath(pai.hContact));
	if (!path)
		return false;

	_tcsncpy_s(pai.filename, path, _TRUNCATE);
	pai.format = PA_FORMAT_JPEG;

	return true;
}

void CSteamProto::CheckAvatarChange(MCONTACT hContact, std::string avatarUrl)
{
	if (avatarUrl.empty())
		return;

	// Check for avatar change
	ptrA oldAvatarUrl(getStringA(hContact, "AvatarUrl"));
	bool update_required = (!oldAvatarUrl || avatarUrl.compare(oldAvatarUrl));

	if (update_required)
		setString(hContact, "AvatarUrl", avatarUrl.c_str());

	if (!hContact)
	{
		PROTO_AVATAR_INFORMATIONT pai = { sizeof(pai) };
		if (GetAvatarInfo(update_required ? GAIF_FORCE : 0, (LPARAM)&pai) != GAIR_WAITFOR)
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	}
	else if (update_required)
	{
		db_set_b(hContact, "ContactPhoto", "NeedUpdate", 1);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
	}
}

INT_PTR CSteamProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATIONT* pai = (PROTO_AVATAR_INFORMATIONT*)lParam;

	ptrA avatarUrl(getStringA(pai->hContact, "AvatarUrl"));
	if (!avatarUrl)
		return GAIR_NOAVATAR;

	if (GetDbAvatarInfo(*pai))
	{
		bool fileExist = _taccess(pai->filename, 0) == 0;

		bool needLoad;
		if (pai->hContact)
			needLoad = (wParam & GAIF_FORCE) && (!fileExist || db_get_b(pai->hContact, "ContactPhoto", "NeedUpdate", 0));
		else
			needLoad = (wParam & GAIF_FORCE) || !fileExist;

		if (needLoad)
		{
			PushRequest(
				new SteamWebApi::GetAvatarRequest(avatarUrl),
				&CSteamProto::OnGotAvatar,
				(void*)pai->hContact,
				ARG_NO_FREE);

			return GAIR_WAITFOR;
		}
		else if (fileExist)
			return GAIR_SUCCESS;

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
	
	case AF_DELAYAFTERFAIL:
		// request avatar again in one hour if server gave an error
		return 60 * 60 * 1000;

	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		// avatars can be fetched all the time (server only operation)
		return 1;
	}

	return 0;
}

INT_PTR CSteamProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return -3;

	TCHAR* buf = (TCHAR*)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai) };
	switch (GetAvatarInfo(0, (LPARAM)&ai)) {
	case GAIR_SUCCESS:
		_tcsncpy(buf, ai.filename, size);
		buf[size - 1] = 0;
		return 0;

	case GAIR_WAITFOR:
		return -1;

	default:
		return -2;
	}
}