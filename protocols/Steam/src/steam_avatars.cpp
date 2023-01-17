#include "stdafx.h"

wchar_t* CSteamProto::GetAvatarFilePath(MCONTACT hContact)
{
	ptrA steamId(getStringA(hContact, "SteamID"));
	if (steamId == NULL)
		return nullptr;

	CMStringW wszPath(GetAvatarPath());
	wszPath.AppendFormat(L"\\%S.jpg", steamId.get());
	return wszPath.Detach();
}

bool CSteamProto::GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &pai)
{
	ptrW path(GetAvatarFilePath(pai.hContact));
	if (!path)
		return false;

	wcsncpy_s(pai.filename, path, _TRUNCATE);
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

	if (!hContact) {
		PROTO_AVATAR_INFORMATION ai = { 0 };
		if (GetAvatarInfo(update_required ? GAIF_FORCE : 0, (LPARAM)& ai) != GAIR_WAITFOR)
			ReportSelfAvatarChanged();
	}
	else if (update_required) {
		db_set_b(hContact, "ContactPhoto", "NeedUpdate", 1);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr, 0);
	}
}

INT_PTR CSteamProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	ptrA avatarUrl(getStringA(pai->hContact, "AvatarUrl"));
	if (!avatarUrl)
		return GAIR_NOAVATAR;

	if (GetDbAvatarInfo(*pai)) {
		bool fileExist = _waccess(pai->filename, 0) == 0;

		bool needLoad;
		if (pai->hContact)
			needLoad = (wParam & GAIF_FORCE) && (!fileExist || db_get_b(pai->hContact, "ContactPhoto", "NeedUpdate", 0));
		else
			needLoad = (wParam & GAIF_FORCE) || !fileExist;

		if (needLoad) {
			PushRequest(new GetAvatarRequest(avatarUrl), &CSteamProto::OnGotAvatar, (void *)pai->hContact);
			return GAIR_WAITFOR;
		}
		if (fileExist)
			return GAIR_SUCCESS;
	}

	return GAIR_NOAVATAR;
}

INT_PTR CSteamProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		{
			POINT *size = (POINT *)lParam;
			if (size) {
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

	wchar_t *buf = (wchar_t *)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	switch (GetAvatarInfo(0, (LPARAM)& ai)) {
	case GAIR_SUCCESS:
		wcsncpy(buf, ai.filename, size);
		buf[size - 1] = 0;
		return 0;

	case GAIR_WAITFOR:
		return -1;

	default:
		return -2;
	}
}
