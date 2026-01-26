/*
Copyright (C) 2012-26 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMStringW CTelegramProto::GetAvatarFilename(MCONTACT hContact)
{
	CMStringW wszResult(GetAvatarPath());

	const wchar_t *szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_JPEG));
	wszResult.AppendFormat(L"\\%lld%s", GetId(hContact), szFileType);
	return wszResult;
}

INT_PTR CTelegramProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT *)lParam)->x = 160;
		((POINT *)lParam)->y = 160;
		break;

	case AF_MAXFILESIZE:
		return 32000;

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

INT_PTR CTelegramProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	auto *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	CMStringW wszPath(GetAvatarFilename(pai->hContact));
	pai->format = getByte(pai->hContact, DBKEY_AVATAR_TYPE, PA_FORMAT_JPEG);
	wcsncpy_s(pai->filename, wszPath, _TRUNCATE);

	if (::_waccess(pai->filename, 0) == 0)
		return GAIR_SUCCESS;

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CTelegramProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	auto wszFileName(GetAvatarFilename(0));
	mir_wstrncpy((wchar_t *)wParam, wszFileName, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnAvatarSet(td::ClientManager::Response&, void *pUserInfo)
{
	ptrW pwszFileName((wchar_t *)pUserInfo);
	DeleteFileW(pwszFileName);
}

INT_PTR CTelegramProto::SvcSetMyAvatar(WPARAM, LPARAM lParam)
{
	auto *pwszFileName = (const wchar_t *)lParam;
	if (ProtoGetAvatarFileFormat(pwszFileName) != PA_FORMAT_JPEG) {
		Popup(0, TranslateT("Avatar file must be a picture in JPEG format"), TranslateT("Error setting avatar"));
		return 1;
	}

	TD::object_ptr<TD::InputFile> localFile(new TD::inputFileLocal(T2Utf(pwszFileName).get()));
	TD::object_ptr<TD::InputChatPhoto> photo(new TD::inputChatPhotoStatic(std::move(localFile)));
	SendQuery(new TD::setProfilePhoto(std::move(photo), true), &CTelegramProto::OnAvatarSet, mir_wstrdup(pwszFileName));
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Extracts a photo/avatar to a file

void CTelegramProto::ProcessAvatar(const TD::file *pFile, TG_USER *pUser)
{
	if (pUser->hContact == INVALID_CONTACT_ID)
		return;

	auto remoteId = pFile->remote_->unique_id_;
	auto storedId = getMStringA(pUser->hContact, DBKEY_AVATAR_HASH);
	auto wszFileName = GetAvatarFilename(pUser->hContact);
	if (remoteId != storedId.c_str() || _waccess(wszFileName, 0)) {
		if (!remoteId.empty()) {
			pUser->szAvatarHash = remoteId.c_str();
			setString(pUser->hContact, DBKEY_AVATAR_HASH, remoteId.c_str());
			SendQuery(new TD::downloadFile(pFile->id_, 5, 0, 0, false));
		}
		else delSetting(pUser->hContact, DBKEY_AVATAR_HASH);
	}
}
