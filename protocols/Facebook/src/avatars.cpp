/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

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
	WCHAR wszPath[MAX_PATH];
	mir_snwprintf(wszPath, MAX_PATH, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(wszPath);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(wszPath);

	CMStringW id(getMStringW(hContact, DBKEY_ID));
	mir_snwprintf(pwszFileName, MAX_PATH, L"%s\\%s.jpg", wszPath, id.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl FacebookProto::AvatarsUpdate(void *)
{
	NETLIBHTTPREQUEST req = {};
	req.cbSize = sizeof(req);
	req.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.requestType = REQUEST_GET;

	const char *szParams = m_bUseBigAvatars ? "width=200&height=200" : "width=80&height=80";

	for (auto &cc : AccContacts()) {
		if (Miranda_IsTerminated())
			break;

		if (!getByte(cc, "UpdateNeeded"))
			continue;

		delSetting(cc, "UpdateNeeded");

		CMStringA szUrl(FORMAT, "https://graph.facebook.com/%s/picture?%s", getMStringA(cc, DBKEY_ID).c_str(), szParams);
		req.szUrl = szUrl.GetBuffer();
	
		NETLIBHTTPREQUEST *pReply = Netlib_HttpTransaction(m_hNetlibUser, &req);
		if (pReply == nullptr) {
			debugLogA("Failed to retrieve avatar from url: %s", szUrl.c_str());
			continue;
		}

		PROTO_AVATAR_INFORMATION ai;
		ai.hContact = cc;
		ai.format = PA_FORMAT_UNKNOWN;
		GetAvatarFilename(cc, ai.filename);

		bool bSuccess = false;
		if (pReply->resultCode == 200 && pReply->pData && pReply->dataLength) {
			for (int i = 0; i < pReply->headersCount; i++)
				if (!mir_strcmp(pReply->headers[i].szName, "Content-Type")) {
					ai.format = ProtoGetAvatarFormatByMimeType(pReply->headers[i].szValue);
					break;
				}

			if (ai.format != PA_FORMAT_UNKNOWN) {
				FILE *fout = _wfopen(ai.filename, L"wb");
				if (fout) {
					fwrite(pReply->pData, 1, pReply->dataLength, fout);
					fclose(fout);
					bSuccess = true;
				}
				else debugLogA("Error saving avatar to file %S", ai.filename);
			}
			else debugLogA("unknown avatar mime type");
		}
		else debugLogA("Error %d reading avatar from url: %s", pReply->resultCode, szUrl.c_str());

		ProtoBroadcastAck(cc, ACKTYPE_AVATAR, bSuccess ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&ai);

		Netlib_FreeHttpRequest(pReply);
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


/*
bool FacebookProto::GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &pai, std::string *url)
{
	ptrA id(getStringA(pai.hContact, FACEBOOK_KEY_ID));
	if (id == nullptr)
		return false;

	if (url) {
		*url = FACEBOOK_URL_PICTURE;
		utils::text::replace_first(url, "%s", std::string(id));
	}

	std::wstring filename = GetAvatarFolder() + L'\\' + std::wstring(_A2T(id)) + L".jpg";

	wcsncpy_s(pai.filename, filename.c_str(), _TRUNCATE);
	pai.format = ProtoGetAvatarFormat(pai.filename);
	return true;
}

void FacebookProto::CheckAvatarChange(MCONTACT hContact, const std::string &image_url)
{
	std::wstring::size_type pos = image_url.rfind("/");

	// Facebook contacts always have some avatar - keep avatar in database even if we have loaded empty one (e.g. for 'On Mobile' contacts)
	if (image_url.empty() || pos == std::wstring::npos)
		return;

	// Get name of image
	std::string image_name = image_url.substr(pos + 1);

	// Remove eventual parameters from name
	pos = image_name.rfind("?");
	if (pos != std::wstring::npos)
		image_name = image_name.substr(0, pos);

	// Append our parameters to allow comparing for avatar/settings change
	if (getBool(FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS))
		image_name += "?big";

	// Check for avatar change
	ptrA old_name(getStringA(hContact, FACEBOOK_KEY_AVATAR));
	bool update_required = (old_name == nullptr || image_name.compare(old_name) != 0);

	// TODO: Remove this in some newer version
	if (old_name == nullptr) {
		// Remove AvatarURL value, which was used in previous versions of plugin
		delSetting(hContact, "AvatarURL");
	}

	if (update_required)
		setString(hContact, FACEBOOK_KEY_AVATAR, image_name.c_str());

	if (!hContact) {
		PROTO_AVATAR_INFORMATION ai = { 0 };
		if (GetAvatarInfo(update_required ? GAIF_FORCE : 0, (LPARAM)&ai) != GAIR_WAITFOR)
			ReportSelfAvatarChanged();
	}
	else if (update_required) {
		db_set_b(hContact, "ContactPhoto", "NeedUpdate", 1);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, nullptr);
	}
}

void FacebookProto::UpdateAvatarWorker(void *)
{
	HNETLIBCONN nlc = nullptr;

	debugLogA("*** UpdateAvatarWorker");

	std::string params = getBool(FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS) ? "?width=200&height=200" : "?width=80&height=80";

	for (;;) {
		std::string url;
		PROTO_AVATAR_INFORMATION ai = { 0 };
		ai.hContact = avatar_queue[0];

		if (Miranda_IsTerminated()) {
			debugLogA("*** Terminating avatar update early: %s", url.c_str());
			break;
		}

		if (GetDbAvatarInfo(ai, &url)) {
			debugLogA("*** Updating avatar: %s", url.c_str());
			bool success = facy.save_url(url + params, ai.filename, nlc);

			if (ai.hContact)
				ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, success ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&ai);
			else if (success)
				ReportSelfAvatarChanged();
		}

		mir_cslock s(avatar_lock_);
		avatar_queue.erase(avatar_queue.begin());
		if (avatar_queue.empty())
			break;
	}
	Netlib_CloseHandle(nlc);
}

std::wstring FacebookProto::GetAvatarFolder()
{
	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%s", VARSW(L"%miranda_avatarcache%"), m_tszUserName);
	return path;
}

INT_PTR FacebookProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = -1;
		((POINT*)lParam)->y = -1;
		break;

	case AF_FORMATSUPPORTED:
		res = (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF);
		break;

	case AF_DELAYAFTERFAIL:
		res = 10 * 60 * 1000;
		break;

	case AF_ENABLED:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		res = 1;
		break;
	}

	return res;
}

INT_PTR FacebookProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;
	if (GetDbAvatarInfo(*pai, nullptr)) {
		bool fileExist = _waccess(pai->filename, 0) == 0;

		bool needLoad;
		if (pai->hContact)
			needLoad = (wParam & GAIF_FORCE) && (!fileExist || db_get_b(pai->hContact, "ContactPhoto", "NeedUpdate", 0));
		else
			needLoad = (wParam & GAIF_FORCE) || !fileExist;

		if (needLoad) {
			debugLogA("*** Starting avatar request thread for %s", _T2A(pai->filename));
			mir_cslock s(avatar_lock_);

			if (std::find(avatar_queue.begin(), avatar_queue.end(), pai->hContact) == avatar_queue.end()) {
				bool is_empty = avatar_queue.empty();
				avatar_queue.push_back(pai->hContact);
				if (is_empty)
					ForkThread(&FacebookProto::UpdateAvatarWorker, nullptr);
			}
			return GAIR_WAITFOR;
		}
		else if (fileExist)
			return GAIR_SUCCESS;

	}
	return GAIR_NOAVATAR;
}

INT_PTR FacebookProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	debugLogA("*** GetMyAvatar");

	if (!wParam || !lParam)
		return -3;

	wchar_t* buf = (wchar_t*)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	switch (GetAvatarInfo(0, (LPARAM)&ai)) {
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
*/