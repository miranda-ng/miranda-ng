#include "common.h"

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