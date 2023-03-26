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

CMStringW CTelegramProto::GetAvatarFilename(MCONTACT hContact)
{
	CMStringW wszResult(GetAvatarPath());

	const wchar_t *szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_JPEG));
	wszResult.AppendFormat(L"\\%s%s", getMStringW(hContact, DBKEY_ID).c_str(), szFileType);
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

INT_PTR CTelegramProto::SvcGetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

INT_PTR CTelegramProto::SvcSetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

TG_FILE_REQUEST* CTelegramProto::PopFile(const char *pszUniqueId)
{
	mir_cslock lck(m_csFiles);

	for (auto &it : m_arFiles)
		if (it->m_uniqueId == pszUniqueId)
			return it;

	return nullptr;
}

void CTelegramProto::ProcessFile(TD::updateFile *pObj)
{
	if (auto *pFile = pObj->file_.get()) {
		if (!pFile->local_->is_downloading_completed_)
			return;

		Utf2T wszExistingFile(pFile->local_->path_.c_str());

		if (auto *F = PopFile(pFile->remote_->unique_id_.c_str())) {
			CMStringW wszFullName = F->m_destPath + L"\\" + F->m_fileName;

			if (F->m_type == F->AVATAR) {
				if (F->m_fileName.Right(5).MakeLower() == L".webp") {
					if (auto *pImage = FreeImage_LoadU(FIF_WEBP, wszExistingFile)) {
						wszFullName.Truncate(wszFullName.GetLength() - 5);
						wszFullName += L".png";
						FreeImage_SaveU(FIF_PNG, pImage, wszFullName);
						FreeImage_Unload(pImage);
					}
				}
				else MoveFileW(wszExistingFile, wszFullName);
					
				SMADD_CONT cont = { 1, m_szModuleName, wszFullName };
				CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));

				mir_cslock lck(m_csFiles);
				m_arFiles.remove(F);
			}
			else { // FILE
				F->pfts.currentFileProgress = pFile->local_->downloaded_size_;
				ProtoBroadcastAck(F->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, F, (LPARAM)&F->pfts);

				if (pFile->local_->is_downloading_completed_) {
					MoveFileW(wszExistingFile, wszFullName);
					ProtoBroadcastAck(F->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, F);

					mir_cslock lck(m_csFiles);
					m_arFiles.remove(F);
				}
			}

			delete F;
			return;
		}

		for (auto &it : m_arUsers) {
			if (it->szAvatarHash == pFile->remote_->unique_id_.c_str()) {
	
				PROTO_AVATAR_INFORMATION pai;
				pai.hContact = it->hContact;
				pai.format = ProtoGetAvatarFileFormat(wszExistingFile);
				setByte(pai.hContact, DBKEY_AVATAR_TYPE, pai.format);

				CMStringW wszAvatarPath(GetAvatarFilename(it->hContact));
				wcsncpy_s(pai.filename, wszAvatarPath, _TRUNCATE);

				MoveFileW(wszExistingFile, wszAvatarPath);

				ProtoBroadcastAck(it->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &pai);
				break;
			}
		}
	}
}
