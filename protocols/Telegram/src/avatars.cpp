/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

	ptrW wszPath(getWStringA(pai->hContact, DBKEY_AVATAR_PATH));
	if (wszPath == nullptr)
		return GAIR_NOAVATAR;

	pai->format = getByte(pai->hContact, DBKEY_AVATAR_TYPE, PA_FORMAT_JPEG);
	wcsncpy_s(pai->filename, wszPath, _TRUNCATE);

	if (::_waccess(pai->filename, 0) == 0)
		return GAIR_SUCCESS;

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CTelegramProto::SvcGetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

INT_PTR CTelegramProto::SvcSetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ProcessFile(TD::updateFile *pObj)
{
	if (auto *pFile = pObj->file_.get()) {
		if (!pFile->local_->is_downloading_completed_)
			return;

		for (auto &it : m_arUsers) {
			if (it->szAvatarHash == pFile->remote_->unique_id_.c_str()) {
				PROTO_AVATAR_INFORMATION pai;
				wcsncpy_s(pai.filename, Utf2T(pFile->local_->path_.c_str()), _TRUNCATE);
				pai.hContact = it->hContact;
				pai.format = ProtoGetAvatarFileFormat(pai.filename);

				setByte(pai.hContact, DBKEY_AVATAR_TYPE, pai.format);
				setWString(pai.hContact, DBKEY_AVATAR_PATH, pai.filename);

				ProtoBroadcastAck(it->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &pai);
				break;
			}
		}
	}
}
