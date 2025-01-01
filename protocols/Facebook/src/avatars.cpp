/*

Facebook plugin for Miranda NG
Copyright © 2019-25 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////

void FacebookProto::GetAvatarFilename(MCONTACT hContact, wchar_t *pwszFileName)
{
	CMStringW wszPath(GetAvatarPath());
	CMStringW id(getMStringW(hContact, DBKEY_ID));
	mir_snwprintf(pwszFileName, MAX_PATH, L"%s\\%s.jpg", wszPath.c_str(), id.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl FacebookProto::AvatarsUpdate(void *)
{
	MHttpRequest req(REQUEST_GET);
	req.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;

	CMStringA szParams((m_bUseBigAvatars) ? "type=large" : "type=normal");
	szParams.AppendFormat("&access_token=%s", m_szAuthToken.c_str());

	for (auto &cc : AccContacts()) {
		if (Miranda_IsTerminated())
			break;

		if (!getByte(cc, "UpdateNeeded"))
			continue;

		delSetting(cc, "UpdateNeeded");
		req.m_szUrl.Format("https://graph.facebook.com/%s/picture?%s", getMStringA(cc, DBKEY_ID).c_str(), szParams.c_str());
	
		PROTO_AVATAR_INFORMATION ai;
		ai.hContact = cc;
		ai.format = PA_FORMAT_UNKNOWN;
		GetAvatarFilename(cc, ai.filename);

		NLHR_PTR pReply(Netlib_DownloadFile(m_hNetlibUser, &req, ai.filename));
		if (pReply == nullptr) {
			debugLogA("Failed to retrieve avatar from url: %s", req.m_szUrl.c_str());
			continue;
		}

		if (pReply->resultCode == 200) {
			if (auto *pszHdr = pReply->FindHeader("Content-Type"))
				ai.format = ProtoGetAvatarFormatByMimeType(pszHdr);

			if (ai.format == PA_FORMAT_UNKNOWN)
				debugLogA("unknown avatar mime type");

			ProtoBroadcastAck(cc, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
		}
		else {
			debugLogA("Error %d reading avatar from url: %s", pReply->resultCode, req.m_szUrl.c_str());
			ProtoBroadcastAck(cc, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
		}
	}
}

INT_PTR FacebookProto::GetAvatarInfo(WPARAM flags, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;
	GetAvatarFilename(pai->hContact, pai->filename);

	bool bFileExist = _waccess(pai->filename, 0) == 0;

	// if we still need to load an avatar
	if ((flags & GAIF_FORCE) || !bFileExist) {
		setByte(pai->hContact, "UpdateNeeded", 1);
		ForkThread(&FacebookProto::AvatarsUpdate);
		return GAIR_WAITFOR;
	}
	
	return (bFileExist) ? GAIR_SUCCESS : GAIR_NOAVATAR;
}

INT_PTR FacebookProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT *)lParam)->x = ((POINT *)lParam)->y = 128;
		break;

	case AF_FORMATSUPPORTED:
		res = lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_JPEG;
		break;

	case AF_ENABLED:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}

	return res;
}
