/*
Copyright © 2016-17 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMStringW CDiscordProto::GetAvatarFilename(MCONTACT hContact)
{
	CMStringW wszResult(FORMAT, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(wszResult);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(wszResult);

	wszResult.AppendChar('\\');

	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));
	wszResult.AppendFormat(L"%lld%s", getId(hContact, DB_KEY_ID), szFileType);
	return wszResult;
}

INT_PTR CDiscordProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = ((POINT*)lParam)->y = 128;
		break;

	case AF_PROPORTION:
		res = PIP_NONE;
		break;

	case AF_FORMATSUPPORTED:
		res = lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_JPEG;
		break;

	case AF_ENABLED:
		res = 1;
		break;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveAvatar(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.format = PA_FORMAT_UNKNOWN;
	ai.hContact = (MCONTACT)pReq->pUserInfo;

	if (reply->resultCode != 200) {
LBL_Error:
		ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai);
		return;
	}

	const char *pszMimeType = NULL;
	for (int i = 0; i < reply->headersCount; i++)
		if (!mir_strcmp(reply->headers[i].szName, "Content-Type")) {
			pszMimeType = reply->headers[i].szValue;
			break;
		}

	if (!mir_strcmp(pszMimeType, "image/jpeg"))
		ai.format = PA_FORMAT_JPEG;
	else if (!mir_strcmp(pszMimeType, "image/png"))
		ai.format = PA_FORMAT_PNG;
	else if (!mir_strcmp(pszMimeType, "image/gif"))
		ai.format = PA_FORMAT_GIF;
	else if (!mir_strcmp(pszMimeType, "image/bmp"))
		ai.format = PA_FORMAT_BMP;
	else {
		debugLogA("unknown avatar mime type: %s", pszMimeType);
		goto LBL_Error;
	}

	mir_wstrncpy(ai.filename, GetAvatarFilename(ai.hContact), _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == NULL) {
		debugLogA("cannot open avatar file %S for writing", ai.filename);
		goto LBL_Error;
	}

	fwrite(reply->pData, 1, reply->dataLength, out);
	fclose(out);

	if (ai.hContact)
		ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai);
	else
		CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
}

INT_PTR CDiscordProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	CMStringW wszFileName(GetAvatarFilename(pai->hContact));
	if (!wszFileName.IsEmpty()) {
		mir_wstrncpy(pai->filename, wszFileName, _countof(pai->filename));

		bool bFileExist = _waccess(wszFileName, 0) == 0;

		// if we still need to load an avatar
		if ((wParam & GAIF_FORCE) || !bFileExist) {
			ptrA szAvatarHash(getStringA(pai->hContact, DB_KEY_AVHASH));
			SnowFlake id = getId(pai->hContact, DB_KEY_ID);
			if (id == 0 || szAvatarHash == NULL)
				return GAIR_NOAVATAR;
		
			CMStringA szUrl(FORMAT, "https://cdn.discordapp.com/avatars/%lld/%s.jpg", id, szAvatarHash);
			AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveAvatar);
			pReq->pUserInfo = (void*)pai->hContact;
			Push(pReq);
			return GAIR_WAITFOR;
		}
		if (bFileExist)
			return GAIR_SUCCESS;
	}

	return GAIR_NOAVATAR;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return -3;

	wchar_t* buf = (wchar_t*)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATION ai = {};
	switch (GetAvatarInfo(0, (LPARAM)&ai)) {
	case GAIR_SUCCESS:
		wcsncpy_s(buf, size, ai.filename, _TRUNCATE);
		return 0;

	case GAIR_WAITFOR:
		return -1;
	}

	return -2;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	CMStringW wszFileName(GetAvatarFilename(NULL));

	const wchar_t *pwszFilename = (const wchar_t*)lParam;
	if (pwszFilename == NULL) { // remove my avatar file
		delSetting(DB_KEY_AVHASH);
		DeleteFile(wszFileName);
	}

	CMStringA szPayload("data:");

	int iFormat = ProtoGetAvatarFileFormat(pwszFilename);
	switch (iFormat) {
	case PA_FORMAT_BMP: szPayload.Append("image/bmp"); break;
	case PA_FORMAT_GIF: szPayload.Append("image/gif"); break;
	case PA_FORMAT_PNG: szPayload.Append("image/png"); break;
	case PA_FORMAT_JPEG: szPayload.Append("image/jpeg"); break;
	default:
		debugLogA("invalid file format for avatar %S: %d", pwszFilename, iFormat);
		return 1;
	}
	szPayload.Append(";base64,");
	FILE *in = _wfopen(pwszFilename, L"rb");
	if (in == NULL) {
		debugLogA("cannot open avatar file %S for reading", pwszFilename);
		return 2;
	}

	int iFileLength = _filelength(_fileno(in));
	ptrA szFileContents((char*)mir_alloc(iFileLength));
	fread(szFileContents, 1, iFileLength, in);
	fclose(in);
	szPayload.Append(ptrA(mir_base64_encode((BYTE*)szFileContents.get(), iFileLength)));

	JSONNode root; root << CHAR_PARAM("avatar", szPayload);
	Push(new AsyncHttpRequest(this, REQUEST_PATCH, "/users/@me", NULL, &root));
	return 0;
}
