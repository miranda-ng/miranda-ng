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

void CTelegramProto::KillFile(TG_FILE_REQUEST *ft)
{
	{	mir_cslock lck(m_csFiles);
		m_arFiles.remove(ft);
	}
	delete ft;
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
		auto *ft = FindFile(pRemote->unique_id_);
		if (ft && ft->m_type != ft->AVATAR && ft->ofd) {
			DBVARIANT dbv = { DBVT_DWORD };
			dbv.dVal = pLocal->downloaded_size_;
			db_event_setJson(ft->ofd->hDbEvent, "ft", &dbv);
		}
		return;
	}

	// file upload is not completed, skip it
	if (pRemote->is_uploading_active_) {
		if (auto *ft = FindFile(pFile->id_))
			ShowFileProgress(pFile, ft);
		return;
	}
	else { // look for preliminary uploaded file
		if (pRemote->unique_id_.empty() && pFile->size_ == pRemote->uploaded_size_)
			if (auto *ft = FindFile(pFile->id_)) {
				ShowFileProgress(pFile, ft);

				// move to the next file
				AdvanceToNextFile(ft);
			}
	}

	Utf2T wszExistingFile(pLocal->path_.c_str());

	if (auto *ft = FindFile(pRemote->unique_id_)) {
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
						ft->ofd->ResetFileName(wszFullName, !ft->m_bRecv); // resulting ofd->wszPath may differ from wszFullName
					}
					else {
						int iFormat = ProtoGetAvatarFileFormat(wszExistingFile);
						if (iFormat != PA_FORMAT_UNKNOWN) {
							wszFullName.AppendChar('.');
							wszFullName.Append(ProtoGetAvatarExtension(iFormat));
							ft->ofd->ResetFileName(wszFullName, !ft->m_bRecv);
						}
					}
				}

				MoveFileW(wszExistingFile, ft->ofd->wszPath);
				ft->ofd->Finish();
			}
		}

		KillFile(ft);
		return;
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
	auto *pMessage = (TD::message *)response.object.get();

	CMStringA szFileName, szCaption;
	if (auto *pFile = GetContentFile(pMessage->content_.get(), ft->m_type, szFileName, szCaption)) {
		ft->m_fileName = Utf2T(szFileName);
		ft->m_wszDescr = Utf2T(szCaption);
		ft->m_fileId = pFile->id_;
		ft->m_uniqueId = pFile->remote_->unique_id_.c_str();

		SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, false));
	}
	else KillFile(ft);
}

void __cdecl CTelegramProto::OfflineFileThread(void *pParam)
{
	auto *ofd = (OFDTHREAD *)pParam;

	DB::EventInfo dbei(ofd->hDbEvent);
	if (dbei && !strcmp(dbei.szModule, m_szModuleName) && dbei.eventType == EVENTTYPE_FILE) {
		if (!ofd->bCopy) {
			auto *ft = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, 0, "");
			ft->ofd = ofd;
			{
				mir_cslock lck(m_csFiles);
				m_arFiles.insert(ft);
			}

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

void CTelegramProto::OnReceiveOfflineFile(DB::EventInfo &, DB::FILE_BLOB &blob)
{
	if (auto *ft = (TG_FILE_REQUEST *)blob.getUserInfo()) {
		blob.setUrl(ft->m_uniqueId.GetBuffer());
		blob.setSize(ft->m_fileSize);
		delete ft;
	}
}

void CTelegramProto::OnSendOfflineFile(DB::EventInfo &dbei, DB::FILE_BLOB& /*blob*/, void* /*hTransfer*/)
{
	dbei.bTemporary = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

TG_FILE_REQUEST *CTelegramProto::FindFile(const std::string &pszUniqueId)
{
	if (pszUniqueId.empty())
		return nullptr;

	mir_cslock lck(m_csFiles);

	for (auto &it : m_arFiles)
		if (it->m_uniqueId.c_str() == pszUniqueId)
			return it;

	return nullptr;
}

TG_FILE_REQUEST *CTelegramProto::FindFile(int id)
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
	fts.totalFiles = ft->m_arFiles.getCount();
	for (auto &it : ft->m_arFiles) {
		fts.totalBytes += it->m_iFileSize;
		if (ft->m_arFiles.indexOf(&it) < ft->m_iCurrFile)
			fts.totalProgress += it->m_iFileSize;
	}
	fts.currentFileSize = pFile->size_;
	fts.currentFileProgress = pFile->remote_->uploaded_size_;
	fts.totalProgress += fts.currentFileProgress;
	ProtoBroadcastAck((UINT_PTR)ft->m_hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&fts);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::OnSendFile(td::ClientManager::Response &, void *pUserInfo)
{
	auto *ft = (TG_FILE_REQUEST *)pUserInfo;
	ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft);

	KillFile(ft);
}

void CTelegramProto::OnUploadFile(td::ClientManager::Response &response, void *pUserInfo)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::file::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::file::ID);
		return;
	}

	auto *pFile = (TD::file *)response.object.get();
	auto *ft = (TG_FILE_REQUEST *)pUserInfo;
	ft->m_arFiles[ft->m_iCurrFile].m_fileId = ft->m_fileId = pFile->id_;

	// remote file might be already cached & reused, so just advance to the next file
	if (pFile->remote_->is_uploading_completed_ && !pFile->remote_->is_uploading_active_)
		AdvanceToNextFile(ft);
}

void CTelegramProto::AdvanceToNextFile(TG_FILE_REQUEST *ft)
{
	ft->m_iCurrFile++;

	if (ft->m_iCurrFile < ft->m_arFiles.getCount())
		ProtoBroadcastAck(ft->m_hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft);
	
	InitNextFileSend(ft);
}

void CTelegramProto::InitNextFileSend(TG_FILE_REQUEST *ft)
{
	// all files are uploaded, create a message with an embedded remote file & send it
	int iCount = ft->m_arFiles.getCount();
	if (ft->m_iCurrFile >= iCount) {
		TD::sendMessage *pMessage = nullptr;
		TD::sendMessageAlbum *pAlbum = nullptr;

		auto *pUser = FindUser(GetId(ft->m_hContact));
		if (pUser == nullptr) {
			debugLogA("request from unknown contact %d, ignored", ft->m_hContact);
			return;
		}

		if (iCount == 1) {
			pMessage = new TD::sendMessage();
			pMessage->chat_id_ = pUser->chatId;
		}
		else {
			pAlbum = new TD::sendMessageAlbum();
			pAlbum->chat_id_ = pUser->chatId;
		}

		for (auto &it : ft->m_arFiles) {
			auto iFileType = (m_bCompressFiles) ? AutoDetectType(it->m_wszFileName) : TG_FILE_REQUEST::FILE;

			auto caption = formatBbcodes(T2Utf(ft->m_wszDescr));
			TD::object_ptr<TD::InputMessageContent> pPart;
			auto remoteFile = TD::make_object<TD::inputFileId>(it->m_fileId);

			if (iFileType == TG_FILE_REQUEST::PICTURE) {
				auto pContent = TD::make_object<TD::inputMessagePhoto>();
				pContent->photo_ = std::move(remoteFile);
				pContent->thumbnail_ = 0;
				pContent->caption_ = std::move(caption);
				pContent->height_ = 0;
				pContent->width_ = 0;
				pPart = std::move(pContent);
			}
			else if (iFileType == TG_FILE_REQUEST::VOICE) {
				auto pContent = TD::make_object<TD::inputMessageAudio>();
				pContent->audio_ = std::move(remoteFile);
				pContent->caption_ = std::move(caption);
				pContent->duration_ = 0;
				pPart = std::move(pContent);
			}
			else if (iFileType == TG_FILE_REQUEST::VIDEO) {
				auto pContent = TD::make_object<TD::inputMessageVideo>();
				pContent->video_ = std::move(remoteFile);
				pContent->caption_ = std::move(caption);
				pContent->duration_ = 0;
				pContent->height_ = 0;
				pContent->width_ = 0;
				pPart = std::move(pContent);
			}
			else {
				auto pContent = TD::make_object<TD::inputMessageDocument>();
				pContent->document_ = std::move(remoteFile);
				pContent->caption_ = std::move(caption);
				pContent->thumbnail_ = 0;
				pPart = std::move(pContent);
			}

			if (pMessage)
				pMessage->input_message_content_ = std::move(pPart);
			else
				pAlbum->input_message_contents_.push_back(std::move(pPart));
		}

		if (pMessage)
			SendQuery(pMessage, &CTelegramProto::OnSendFile, ft);
		else
			SendQuery(pAlbum, &CTelegramProto::OnSendFile, ft);
		return;
	}

	auto &pFile = ft->m_arFiles[ft->m_iCurrFile];
	auto localFile = makeFile(pFile.m_wszFileName);
	auto iFileType = (m_bCompressFiles) ? AutoDetectType(pFile.m_wszFileName) : TG_FILE_REQUEST::FILE;

	ft->m_fileName = pFile.m_wszFileName;
	ft->m_fileSize = pFile.m_iFileSize;

	TD::object_ptr<TD::FileType> fileType;
	switch (iFileType) {
	case TG_FILE_REQUEST::PICTURE:
		fileType = TD::make_object<TD::fileTypePhoto>();
		break;
	case TG_FILE_REQUEST::VOICE:
		fileType = TD::make_object<TD::fileTypeAudio>();
		break;
	case TG_FILE_REQUEST::VIDEO:
		fileType = TD::make_object<TD::fileTypeVideo>();
		break;
	default:
		fileType = TD::make_object<TD::fileTypeDocument>();
		break;
	}
	SendQuery(new TD::preliminaryUploadFile(std::move(localFile), std::move(fileType), 1), &CTelegramProto::OnUploadFile, ft);
}

HANDLE CTelegramProto::SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles)
{
	auto *pUser = FindUser(GetId(hContact));
	if (pUser == nullptr) {
		debugLogA("request from unknown contact %d, ignored", hContact);
		return nullptr;
	}

	auto *ft = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, 0, 0);
	ft->m_hContact = hContact;
	ft->m_wszDescr = szDescription;

	for (int i = 0; ppszFiles[i] != 0; i++) {
		struct _stat statbuf;
		if (_wstat(ppszFiles[0], &statbuf)) {
			debugLogW(L"'%s' is an invalid filename", ppszFiles[i]);
			continue;
		}

		ft->m_arFiles.insert(new TG_FILE_REQUEST::Upload(ppszFiles[i], statbuf.st_size));
	}

	if (!ft->m_arFiles.getCount()) {
		debugLogA("No files to be sent");
		delete ft;
		return nullptr;
	}

	if (ft->m_arFiles.getCount() > 10) {
		Popup(hContact, TranslateT("Too many files to be sent"), TranslateT("Error"));
		delete ft;
		return nullptr;
	}

	{
		mir_cslock lck(m_csFiles);
		m_arFiles.insert(ft);
	}

	InitNextFileSend(ft);
	return ft;
}

HANDLE CTelegramProto::FileAllow(MCONTACT, HANDLE hTransfer, const wchar_t* /*szPath*/)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	if (!isRunning()) {
		KillFile(ft);
		return 0;
	}

	return ft;
}

int CTelegramProto::FileCancel(MCONTACT, HANDLE hTransfer)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	for (auto &it : ft->m_arFiles)
		if (it->m_fileId != 0)
			SendQuery(new TD::cancelPreliminaryUploadFile(it->m_fileId));
	return 0;
}

int CTelegramProto::FileDeny(MCONTACT, HANDLE hTransfer, const wchar_t* /*szReason*/)
{
	auto *ft = (TG_FILE_REQUEST *)hTransfer;
	KillFile(ft);
	return 0;
}
