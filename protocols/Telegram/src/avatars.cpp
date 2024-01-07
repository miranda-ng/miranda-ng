/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

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

INT_PTR CTelegramProto::SvcGetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

INT_PTR CTelegramProto::SvcSetMyAvatar(WPARAM, LPARAM)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Cloud file downloader

void CTelegramProto::OnGetFileInfo(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::file::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::chats::ID);
		return;
	}

	auto *pFile = (TD::file*)response.object.get();

	auto *ft = (TG_FILE_REQUEST *)pUserInfo;
	ft->m_fileId = pFile->id_;
	ft->m_uniqueId = pFile->remote_->unique_id_.c_str();

	SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, false));
}

void CTelegramProto::OnGetFileLink(td::ClientManager::Response &response)
{
	if (!response.object)
		return;

}

void __cdecl CTelegramProto::OfflineFileThread(void *pParam)
{
	auto *ofd = (OFDTHREAD *)pParam;

	DB::EventInfo dbei(ofd->hDbEvent);
	if (dbei && !strcmp(dbei.szModule, m_szModuleName) && dbei.eventType == EVENTTYPE_FILE) {
		if (!ofd->bCopy) {
			auto *ft = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, 0, "");
			ft->ofd = ofd;
			m_arFiles.insert(ft);

			DB::FILE_BLOB blob(dbei);
			SendQuery(new TD::getRemoteFile(blob.getUrl(), 0), &CTelegramProto::OnGetFileInfo, ft);
		}
	}
	else delete ofd;
}

INT_PTR __cdecl CTelegramProto::SvcOfflineFile(WPARAM param, LPARAM)
{
	ForkThread((MyThreadFunc)&CTelegramProto::OfflineFileThread, (void *)param);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Cloud file pre-creator

void CTelegramProto::OnReceiveOfflineFile(DB::FILE_BLOB &blob, void *pHandle)
{
	if (auto *ft = (TG_FILE_REQUEST *)pHandle) {
		blob.setUrl(ft->m_uniqueId.GetBuffer());
		blob.setSize(ft->m_fileSize);
	}
}

void CTelegramProto::OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB &blob, void *hTransfer)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;

	dbei.szId = ft->m_uniqueId;
	if (!ft->m_szUserId.IsEmpty())
		dbei.szUserId = ft->m_szUserId;

	auto *p = wcsrchr(ft->m_fileName, '\\');
	if (p == nullptr)
		p = ft->m_fileName;
	else
		p++;
	blob.setName(p);

	blob.setUrl("boo");
	blob.complete(ft->m_fileSize);
	blob.setLocalName(ft->m_fileName);
}

/////////////////////////////////////////////////////////////////////////////////////////

TG_FILE_REQUEST* CTelegramProto::FindFile(const char *pszUniqueId)
{
	mir_cslock lck(m_csFiles);

	for (auto &it : m_arFiles)
		if (it->m_uniqueId == pszUniqueId)
			return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// handles file info updates

void CTelegramProto::ProcessFile(TD::updateFile *pObj)
{
	auto *pFile = pObj->file_.get();
	if (pFile == nullptr)
		return;

	if (!pFile->local_->is_downloading_completed_) {
		if (auto *F = FindFile(pFile->remote_->unique_id_.c_str())) {
			if (F->m_type != F->AVATAR && F->ofd) {
				DBVARIANT dbv = { DBVT_DWORD };
				dbv.dVal = pFile->local_->downloaded_size_;
				db_event_setJson(F->ofd->hDbEvent, "ft", &dbv);
			}
		}
		return;
	}

	Utf2T wszExistingFile(pFile->local_->path_.c_str());

	if (auto *F = FindFile(pFile->remote_->unique_id_.c_str())) {
		if (F->m_type == F->AVATAR) {
			CMStringW wszFullName = F->m_destPath;
			if (!wszFullName.IsEmpty())
				wszFullName += L"\\";
			wszFullName += F->m_fileName;

			if (F->m_fileName.Right(5).MakeLower() == L".webp") {
				if (auto *pImage = FreeImage_LoadU(FIF_WEBP, wszExistingFile)) {
					wszFullName.Truncate(wszFullName.GetLength() - 5);
					wszFullName += L".png";
					FreeImage_SaveU(FIF_PNG, pImage, wszFullName);
					FreeImage_Unload(pImage);
				}
			}
			else MoveFileW(wszExistingFile, wszFullName);

			SmileyAdd_LoadContactSmileys(SMADD_FILE, m_szModuleName, wszFullName);

			mir_cslock lck(m_csFiles);
			m_arFiles.remove(F);
			delete F;
		}
		else { // FILE, PICTURE, VIDEO, VOICE
			if (F->ofd == nullptr)
				return;

			DBVARIANT dbv = { DBVT_DWORD };
			dbv.dVal = pFile->local_->downloaded_size_;
			db_event_setJson(F->ofd->hDbEvent, "ft", &dbv);

			CMStringW wszFullName(F->ofd->wszPath);
			int idxSlash = wszFullName.ReverseFind('\\') + 1;
			if (wszFullName.Find('.', idxSlash) == -1) {
				auto *pSlash = strrchr(pFile->local_->path_.c_str(), '\\');
				if (!pSlash)
					pSlash = pFile->local_->path_.c_str();
				else
					pSlash++;

				if (strchr(pSlash, '.')) {
					dbv.type = DBVT_UTF8;
					dbv.pszVal = (char *)pSlash;
					db_event_setJson(F->ofd->hDbEvent, "f", &dbv);

					wszFullName.Truncate(idxSlash);
					wszFullName.Append(Utf2T(pSlash));
					F->ofd->ResetFileName(wszFullName); // resulting ofd->wszPath may differ from wszFullName
				}
				else {
					int iFormat = ProtoGetAvatarFileFormat(wszExistingFile);
					if (iFormat != PA_FORMAT_UNKNOWN) {
						wszFullName.AppendChar('.');
						wszFullName.Append(ProtoGetAvatarExtension(iFormat));
						F->ofd->ResetFileName(wszFullName);
					}
				}
			}

			MoveFileW(wszExistingFile, F->ofd->wszPath);
			F->ofd->Finish();

			mir_cslock lck(m_csFiles);
			m_arFiles.remove(F);
			delete F;
		}
		return;
	}

	for (auto &it : m_arOwnMsg) {
		if (it->tmpFileId == pFile->id_) {
			if (!pFile->remote_->id_.empty()) {
				if (auto hDbEvent = db_event_getById(m_szModuleName, it->szMsgId)) {
					DBVARIANT dbv = { DBVT_UTF8 };
					dbv.pszVal = (char *)pFile->remote_->id_.c_str();
					db_event_setJson(hDbEvent, "u", &dbv);
				}
			}
			return;
		}
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
