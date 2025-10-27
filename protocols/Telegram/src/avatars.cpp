/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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
// Cloud file downloader

void CTelegramProto::OnGetFileInfo(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::message::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::chats::ID);
		return;
	}

	auto *ft = (TG_FILE_REQUEST *)pUserInfo;
	auto *pMessage = (TD::message*)response.object.get();
	
	CMStringA szFileName, szCaption;
	if (auto *pFile = GetContentFile(pMessage->content_.get(), ft->m_type, szFileName, szCaption)) {
		ft->m_fileName = Utf2T(szFileName);
		ft->m_wszDescr = Utf2T(szCaption);
		ft->m_fileId = pFile->id_;
		ft->m_uniqueId = pFile->remote_->unique_id_.c_str();

		SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, false));
	}
	else delete ft;
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

			TD::int53 chatId, msgId;
			if (2 == sscanf(dbei.szId, "%lld_%lld", &chatId, &msgId))
				SendQuery(new TD::getMessage(chatId, msgId), &CTelegramProto::OnGetFileInfo, ft);
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

void CTelegramProto::OnReceiveOfflineFile(DB::EventInfo&, DB::FILE_BLOB &blob)
{
	if (auto *ft = (TG_FILE_REQUEST *)blob.getUserInfo()) {
		blob.setUrl(ft->m_uniqueId.GetBuffer());
		blob.setSize(ft->m_fileSize);
		delete ft;
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

TG_FILE_REQUEST* CTelegramProto::FindFile(int id)
{
	mir_cslock lck(m_csFiles);

	for (auto &it : m_arFiles)
		if (it->m_fileId == id)
			return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::ShowFileProgress(const TD::file *pFile, TG_FILE_REQUEST *ft)
{
	PROTOFILETRANSFERSTATUS fts = {};
	fts.hContact = ft->m_hContact;
	fts.totalFiles = 1;
	fts.totalBytes = fts.currentFileSize = pFile->size_;
	fts.totalProgress = fts.currentFileProgress = pFile->remote_->uploaded_size_;
	ProtoBroadcastAck((UINT_PTR)ft->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&fts);
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

/////////////////////////////////////////////////////////////////////////////////////////
// handles file info updates

void CTelegramProto::ProcessFile(TD::updateFile *pObj)
{
	auto *pFile = pObj->file_.get();
	if (pFile == nullptr)
		return;

	auto *pLocal = pFile->local_.get();
	auto *pRemote = pFile->remote_.get();
	if (pRemote == nullptr)
		return;

	if (!pLocal->is_downloading_completed_) {
		auto *ft = FindFile(pRemote->unique_id_.c_str());
		if (ft && ft->m_type != ft->AVATAR && ft->ofd) {
			DBVARIANT dbv = { DBVT_DWORD };
			dbv.dVal = pLocal->downloaded_size_;
			db_event_setJson(ft->ofd->hDbEvent, "ft", &dbv);
		}
		return;
	}

	// file upload is not completed, skip it
	if (pRemote->is_uploading_active_) {
		auto *ft = FindFile(pFile->id_);
		if (ft)
			ShowFileProgress(pFile, ft);
		return;
	}

	Utf2T wszExistingFile(pLocal->path_.c_str());

	if (auto *ft = FindFile(pRemote->unique_id_.c_str())) {
		if (ft->m_type == ft->AVATAR) {
			CMStringW wszFullName = ft->m_destPath;
			if (!wszFullName.IsEmpty())
				wszFullName += L"\\";
			wszFullName += ft->m_fileName;

			if (ft->m_fileName.Right(5).MakeLower() == L".webp") {
				if (auto *pImage = FreeImage_LoadU(FIF_WEBP, wszExistingFile)) {
					wszFullName.Truncate(wszFullName.GetLength() - 5);
					wszFullName += L".png";
					FreeImage_SaveU(FIF_PNG, pImage, wszFullName);
					FreeImage_Unload(pImage);
				}
			}
			else if (ft->m_fileName.Right(4).MakeLower() == L".tga") {
				if (auto *pImage = FreeImage_LoadU(FIF_TARGA, wszExistingFile)) {
					wszFullName.Truncate(wszFullName.GetLength() - 5);
					wszFullName += L".png";
					FreeImage_SaveU(FIF_PNG, pImage, wszFullName);
					FreeImage_Unload(pImage);
				}
			}
			else MoveFileW(wszExistingFile, wszFullName);

			if (ft->m_isSmiley)
				SmileyAdd_LoadContactSmileys(SMADD_FILE, m_szModuleName, wszFullName);
			else
				NS_NotifyFileReady(wszFullName);
		}
		else { // FILE, PICTURE, VIDEO, VOICE
			if (ft->ofd) {
				DBVARIANT dbv = { DBVT_DWORD };
				dbv.dVal = pLocal->downloaded_size_;
				db_event_setJson(ft->ofd->hDbEvent, "ft", &dbv);

				CMStringW wszFullName(ft->ofd->wszPath);
				int idxSlash = wszFullName.ReverseFind('\\') + 1;
				if (wszFullName.Find('.', idxSlash) == -1) {
					auto *pSlash = strrchr(pLocal->path_.c_str(), '\\');
					if (!pSlash)
						pSlash = pLocal->path_.c_str();
					else
						pSlash++;

					if (strchr(pSlash, '.')) {
						dbv.type = DBVT_UTF8;
						dbv.pszVal = (char *)pSlash;
						db_event_setJson(ft->ofd->hDbEvent, "f", &dbv);

						wszFullName.Truncate(idxSlash);
						wszFullName.Append(Utf2T(pSlash));
						ft->ofd->ResetFileName(wszFullName); // resulting ofd->wszPath may differ from wszFullName
					}
					else {
						int iFormat = ProtoGetAvatarFileFormat(wszExistingFile);
						if (iFormat != PA_FORMAT_UNKNOWN) {
							wszFullName.AppendChar('.');
							wszFullName.Append(ProtoGetAvatarExtension(iFormat));
							ft->ofd->ResetFileName(wszFullName);
						}
					}
				}

				MoveFileW(wszExistingFile, ft->ofd->wszPath);
				ft->ofd->Finish();
			}
		}

		mir_cslock lck(m_csFiles);
		m_arFiles.remove(ft);
		delete ft;
		return;
	}

	for (auto &it : m_arOwnMsg) {
		if (it->tmpFileId == pFile->id_) {
			if (!pRemote->id_.empty()) {
				if (auto hDbEvent = db_event_getById(m_szModuleName, it->szMsgId)) {
					DBVARIANT dbv = { DBVT_UTF8 };
					dbv.pszVal = (char *)pRemote->id_.c_str();
					db_event_setJson(hDbEvent, "u", &dbv);

					// file is uploaded to the server
					db_event_delivered(it->hContact, hDbEvent);
				}
			}

			if (auto *ft = FindFile(pFile->id_)) {
				// file being uploaded is finished
				ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);

				mir_cslock lck(m_csFiles);
				m_arFiles.remove(ft);
				delete ft;
			}
			return;
		}
	}

	for (auto &it : m_arUsers) {
		if (it->szAvatarHash == pRemote->unique_id_.c_str()) {
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
