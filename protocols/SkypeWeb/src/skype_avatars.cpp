#include "common.h"

INT_PTR CSkypeProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = 96;
		((POINT*)lParam)->y = 96;
		return 0;

	case AF_PROPORTION:
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED:
	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}

	return 0;
}

void CSkypeProto::ReloadAvatarInfo(MCONTACT hContact)
{
	if (!hContact) {
		CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
		return;
	}
	PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
	AI.hContact = hContact;
	SvcGetAvatarInfo(0, (LPARAM)&AI);
}

void CSkypeProto::OnReceiveAvatar(const NETLIBHTTPREQUEST *response, void *arg)
{
	if (response == NULL)
		return;
	MCONTACT hContact = (MCONTACT)arg;
	if (response->resultCode != 200)
		return;

	PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
	GetAvatarFileName(hContact, AI.filename, SIZEOF(AI.filename));
	AI.format = ProtoGetBufferFormat(response->pData);

	FILE *out = _tfopen(AI.filename, _T("wb"));
	if (out == NULL) {
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &AI, 0);
		return;
	}

	fwrite(response->pData, 1, response->dataLength, out);
	fclose(out);
	setByte(hContact, "NeedNewAvatar", 0);
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &AI, 0);
}

void CSkypeProto::OnSentAvatar(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;
}

INT_PTR CSkypeProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT* AI = (PROTO_AVATAR_INFORMATIONT*)lParam;

	ptrA szUrl(getStringA(AI->hContact, "AvatarUrl"));
	if (szUrl == NULL)
		return GAIR_NOAVATAR;

	TCHAR tszFileName[MAX_PATH];
	GetAvatarFileName(AI->hContact, tszFileName, SIZEOF(tszFileName));
	_tcsncpy(AI->filename, tszFileName, SIZEOF(AI->filename));

	AI->format = ProtoGetAvatarFormat(AI->filename);

	if (::_taccess(AI->filename, 0) == 0 && !getBool(AI->hContact, "NeedNewAvatar", 0))
		return GAIR_SUCCESS;

	if (IsOnline()) {
		PushRequest(new GetAvatarRequest(szUrl), &CSkypeProto::OnReceiveAvatar, (void*)AI->hContact);
		debugLogA("Requested to read an avatar from '%s'", szUrl);
		return GAIR_WAITFOR;
	}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CSkypeProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	debugLogA("CSkypeProto::SvcGetMyAvatar");
	PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
	AI.hContact = NULL;
	if (SvcGetAvatarInfo(0, (LPARAM)&AI) != GAIR_SUCCESS)
		return 1;

	TCHAR* buf = (TCHAR*)wParam;
	int size = (int)lParam;

	_tcsncpy(buf, AI.filename, size);
	buf[size - 1] = 0;

	return 0;
}

void CSkypeProto::GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen)
{
	int tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(pszDest);

	pszDest[tPathLen++] = '\\';

	const TCHAR* szFileType = _T(".jpg");
	ptrA username(getStringA(hContact, "Skypename"));
	mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%s%s"), _A2T(username), szFileType);
}

void CSkypeProto::SetAvatarUrl(MCONTACT hContact, CMString &tszUrl)
{
	CMString oldUrl(getTStringA(hContact, "AvatarUrl"));

	if (tszUrl == oldUrl)
		return;

	if (tszUrl.IsEmpty()) {
		delSetting(hContact, "AvatarUrl");
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
	}
	else {
		setTString(hContact, "AvatarUrl", tszUrl.GetBuffer());
		setByte(hContact,"NeedNewAvatar", 1);
		PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
		AI.hContact = hContact;
		GetAvatarFileName(AI.hContact, AI.filename, SIZEOF(AI.filename));
		AI.format = ProtoGetAvatarFormat(AI.filename);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&AI, 0);
	}
}

INT_PTR CSkypeProto::SvcSetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR *path = (TCHAR*)lParam;
	TCHAR avatarPath[MAX_PATH];
	GetAvatarFileName(NULL, avatarPath, SIZEOF(avatarPath));
	if (path != NULL)
	{
		if (!CopyFile(path, avatarPath, FALSE))
		{
			debugLogA("CSkypeProto::SetMyAvatar: failed to copy new avatar to avatar cache");
			return -1;
		}

		size_t length;
		char *data;
		FILE *hFile = _tfopen(path, L"rb");
		if (!hFile)
		{
			debugLogA("CSkypeProto::SetMyAvatar: failed to open avatar file");
			return -1;
		}

		fseek(hFile, 0, SEEK_END);
		length = ftell(hFile);
		rewind(hFile);

		data = (char*)mir_alloc(length);
		size_t read = fread(data, sizeof(char), length, hFile);
		if (read != length)
		{
			fclose(hFile);
			debugLogA("CSkypeProto::SvcSetMyAvatar: failed to read avatar file");
			return -1;
		}
		fclose(hFile);


		ptrA token(getStringA("TokenSecret"));
		ptrA skypename(getStringA("Skypename"));
		PushRequest(new SetAvatarRequest(token, skypename, data, length), &CSkypeProto::OnSentAvatar);
	}
	else
	{
		if (IsFileExists(avatarPath))
		{
			DeleteFile(avatarPath);
		}
	}

	return 0;
}
