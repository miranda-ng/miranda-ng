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

const char *getName(const TD::usernames *pName)
{
	return (pName == nullptr) ? TranslateU("none") : pName->editable_username_.c_str();
}

CMStringW TG_USER::getDisplayName() const
{
	if (!wszFirstName.IsEmpty())
		return (wszLastName.IsEmpty()) ? wszFirstName : wszFirstName + L" " + wszLastName;

	return wszNick;
}

TG_USER* CTelegramProto::GetSender(const TD::MessageSender *pSender)
{
	switch (pSender->get_id()) {
	case TD::messageSenderChat::ID:
		return FindChat(((TD::messageSenderChat *)pSender)->chat_id_);

	case TD::messageSenderUser::ID:
		return FindUser(((TD::messageSenderUser *)pSender)->user_id_);
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CTelegramProto::CheckSearchUser(TG_USER *pUser)
{
	auto pSearchId = std::find(m_searchIds.begin(), m_searchIds.end(), pUser->chatId);
	if (pSearchId == m_searchIds.end())
		return false;

	ReportSearchUser(pUser);

	m_searchIds.erase(pSearchId);
	if (m_searchIds.empty())
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, this);
	return true;
}

void CTelegramProto::ReportSearchUser(TG_USER *pUser)
{
	CMStringW wszId(FORMAT, L"%lld", pUser->id), wszNick, wszLastName, wszFirstName;

	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = wszId.GetBuffer();

	if (pUser->hContact != INVALID_CONTACT_ID) {
		wszNick = getMStringW(pUser->hContact, "Nick");
		wszLastName = getMStringW(pUser->hContact, "LastName");
		wszFirstName = getMStringW(pUser->hContact, "FirstName");

		psr.nick.w = wszNick.GetBuffer();
		psr.lastName.w = wszLastName.GetBuffer();
		psr.firstName.w = wszFirstName.GetBuffer();
	}
	else {
		psr.firstName.w = pUser->wszFirstName.GetBuffer();
		psr.lastName.w = pUser->wszLastName.GetBuffer();
		psr.nick.w = pUser->wszNick.GetBuffer();
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, this, (LPARAM)&psr);
}

/////////////////////////////////////////////////////////////////////////////////////////

int64_t CTelegramProto::GetId(MCONTACT hContact)
{
	return _atoi64(getMStringA(hContact, DBKEY_ID));
}

void CTelegramProto::SetId(MCONTACT hContact, int64_t id)
{
	char szId[100];
	_i64toa(id, szId, 10);
	setString(hContact, DBKEY_ID, szId);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str)
{
	if (str.empty())
		delSetting(hContact, pszSetting);
	else
		setUString(hContact, pszSetting, str.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Users

TG_USER* CTelegramProto::FindChat(int64_t id)
{
	auto *tmp = (TG_USER *)_alloca(sizeof(TG_USER));
	tmp->chatId = id;
	return m_arChats.find(tmp);
}

TG_USER* CTelegramProto::FindUser(int64_t id)
{
	return m_arUsers.find((TG_USER *)&id);
}

TG_USER* CTelegramProto::AddFakeUser(int64_t id, bool bIsChat)
{
	auto *pu = FindUser(id);
	if (pu == nullptr) {
		pu = new TG_USER(id, INVALID_CONTACT_ID, bIsChat);
		m_arUsers.insert(pu);
		if (!bIsChat)
			m_arChats.insert(pu);
	}
	return pu;
}

TG_USER* CTelegramProto::AddUser(int64_t id, bool bIsChat)
{
	auto *pUser = FindUser(id);
	if (pUser != nullptr)
		if (pUser->hContact != INVALID_CONTACT_ID)
			return pUser;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	SetId(hContact, id);

	if (bIsChat) {
		Clist_SetGroup(hContact, TranslateT("Chat rooms"));
		setByte(hContact, "ChatRoom", 1);
	}
	else if (mir_wstrlen(m_wszDefaultGroup))
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	if (pUser == nullptr) {
		pUser = new TG_USER(id, hContact, bIsChat);
		m_arUsers.insert(pUser);
		if (!bIsChat)
			m_arChats.insert(pUser);
	}
	else {
		pUser->hContact = hContact;
		setWString(hContact, "Nick", pUser->wszNick);
		setWString(hContact, "FirstName", pUser->wszFirstName);
		setWString(hContact, "LastName", pUser->wszLastName);
	}

	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popups

void CTelegramProto::InitPopups(void)
{
	g_plugin.addPopupOption(CMStringW(FORMAT, TranslateT("%s error notifications"), m_tszUserName), m_bUsePopups);

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	wchar_t desc[256];
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Errors"));

	POPUPCLASS ppc = {};
	ppc.flags = PCF_UNICODE;
	ppc.pszName = name;
	ppc.pszDescription.w = desc;
	ppc.hIcon = IcoLib_GetIconByHandle(m_hProtoIcon);
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	IcoLib_ReleaseIcon(ppc.hIcon);
}

void CTelegramProto::Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle)
{
	if (!m_bUsePopups)
		return;

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	CMStringW wszTitle(szTitle);
	if (hContact == 0) {
		wszTitle.Insert(0, L": ");
		wszTitle.Insert(0, m_tszUserName);
	}

	POPUPDATACLASS ppd = {};
	ppd.szTitle.w = wszTitle;
	ppd.szText.w = szMsg;
	ppd.pszClassName = name;
	ppd.hContact = hContact;
	Popup_AddClass(&ppd);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CTelegramProto::GetGcUserId(TG_USER *pUser, const TD::message *pMsg, char *dest)
{
	if (pUser->isGroupChat) {
		if (auto *pSender = GetSender(pMsg->sender_id_.get())) {
			_i64toa(pSender->id, dest, 10);
			if (pUser->m_si && !pSender->wszFirstName.IsEmpty())
				g_chatApi.UM_AddUser(pUser->m_si, Utf2T(dest), pSender->getDisplayName(), ID_STATUS_ONLINE);
			return true;
		}
	}

	*dest = 0;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CTelegramProto::GetMessageFile(
	TG_FILE_REQUEST::Type fileType,
	TG_USER *pUser,
	const TD::file *pFile,
	const char *pszFileName,
	const std::string &caption,
	const char *pszId,
	const char *pszUserId,
	time_t timestamp)
{
	if (pFile->get_id() != TD::file::ID) {
		debugLogA("Document contains unsupported type %d, exiting", pFile->get_id());
		return false;
	}

	auto *pRequest = new TG_FILE_REQUEST(fileType, pFile->id_, pFile->remote_->id_.c_str());
	pRequest->m_fileName = Utf2T(pszFileName);
	pRequest->m_fileSize = pFile->size_;
	{
		mir_cslock lck(m_csFiles);
		m_arFiles.insert(pRequest);
	}

	PROTORECVFILE pre = {};
	pre.dwFlags = PRFF_UTF | PRFF_SILENT;
	pre.fileCount = 1;
	pre.timestamp = timestamp;
	pre.files.a = &pszFileName;
	pre.lParam = (LPARAM)pRequest;
	pre.szId = pszId;
	pre.szUserId = pszUserId;

	if (!caption.empty())
		pre.descr.a = caption.c_str();
	ProtoChainRecvFile(pUser->hContact, &pre);
	return true;
}

CMStringA CTelegramProto::GetMessageSticker(const TD::file *pFile, const char *pwszExtension)
{
	auto *pFileId = pFile->remote_->unique_id_.c_str();

	auto *pRequest = new TG_FILE_REQUEST(TG_FILE_REQUEST::AVATAR, pFile->id_, pFileId);
	pRequest->m_destPath = GetAvatarPath() + L"\\Stickers";
	CreateDirectoryW(pRequest->m_destPath, 0);

	pRequest->m_fileName.Format(L"STK{%S}.%S", pFileId, pwszExtension);
	{
		mir_cslock lck(m_csFiles);
		m_arFiles.insert(pRequest);
	}

	SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, true));
	return CMStringA(FORMAT, "STK{%s}", pFileId);
}

/////////////////////////////////////////////////////////////////////////////////////////

static const TD::photoSize* GetBiggestPhoto(const TD::photo *pPhoto)
{
	const char *types[] = {"y", "x", "m", "s"};
	for (auto *pType : types)
		for (auto &it : pPhoto->sizes_)
			if (it->type_ == pType)
				return it.get();

	return nullptr;
}

CMStringA CTelegramProto::GetMessageText(TG_USER *pUser, const TD::message *pMsg)
{
	const TD::MessageContent *pBody = pMsg->content_.get();

	char szId[100], szUserId[100], *pszUserId = nullptr;
	_i64toa(pMsg->id_, szId, 10);
	if (GetGcUserId(pUser, pMsg, szUserId))
		pszUserId = szUserId;

	switch (pBody->get_id()) {
	case TD::messagePhoto::ID:
		{
			auto *pDoc = (TD::messagePhoto *)pBody;
			auto *pPhoto = GetBiggestPhoto(pDoc->photo_.get());
			if (pPhoto == nullptr) {
				debugLogA("cannot find photo, exiting");
				break;
			}

			CMStringA fileName(FORMAT, "%s (%d x %d)", TranslateU("Picture"), pPhoto->width_, pPhoto->height_);
			GetMessageFile(TG_FILE_REQUEST::PICTURE, pUser, pPhoto->photo_.get(), fileName, pDoc->caption_->text_, szId, pszUserId, pMsg->date_);
		}
		break;

	case TD::messageVideo::ID:
		{
			auto *pDoc = (TD::messageVideo *)pBody;
			auto *pVideo = pDoc->video_.get();
			CMStringA fileName(FORMAT, "%s (%d x %d, %d %s)", TranslateU("Video"), pVideo->width_, pVideo->height_, pVideo->duration_, TranslateU("seconds"));
			std::string caption = fileName.c_str();
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_;
			}
			GetMessageFile(TG_FILE_REQUEST::VIDEO, pUser, pVideo->video_.get(), pVideo->file_name_.c_str(), caption, szId, pszUserId, pMsg->date_);
		}
		break;

	case TD::messageVoiceNote::ID:
		{
			auto *pDoc = (TD::messageVoiceNote *)pBody;
			CMStringA fileName(FORMAT, "%s (%d %s)", TranslateU("Voice note"), pDoc->voice_note_->duration_, TranslateU("seconds"));
			GetMessageFile(TG_FILE_REQUEST::VOICE, pUser, pDoc->voice_note_->voice_.get(), fileName, pDoc->caption_->text_, szId, pszUserId, pMsg->date_);
		}
		break;

	case TD::messageDocument::ID:
		{
			auto *pDoc = (TD::messageDocument *)pBody;
			GetMessageFile(TG_FILE_REQUEST::FILE, pUser, pDoc->document_->document_.get(), pDoc->document_->file_name_.c_str(), pDoc->caption_->text_, szId, pszUserId, pMsg->date_);
		}
		break;

	case TD::messageAnimatedEmoji::ID:
		if (m_bSmileyAdd) {
			auto *pSticker = ((TD::messageAnimatedEmoji *)pBody)->animated_emoji_->sticker_.get();
			if (pSticker->type_->get_id() != TD::stickerTypeRegular::ID)
				break;

			const char *pwszFileExt;
			switch (pSticker->thumbnail_->format_->get_id()) {
			case TD::thumbnailFormatGif::ID: pwszFileExt = "gif"; break;
			case TD::thumbnailFormatPng::ID: pwszFileExt = "png"; break;
			case TD::thumbnailFormatTgs::ID: pwszFileExt = "tga"; break;
			case TD::thumbnailFormatJpeg::ID: pwszFileExt = "jpg"; break;
			case TD::thumbnailFormatWebm::ID: pwszFileExt = "webm"; break;
			case TD::thumbnailFormatWebp::ID: pwszFileExt = "webp"; break;
			default:pwszFileExt = "jpeg"; break;
			}

			return GetMessageSticker(pSticker->thumbnail_->file_.get(), pwszFileExt);
		}
		break;

	case TD::messageSticker::ID:
		if (m_bSmileyAdd) {
			auto *pSticker = ((TD::messageSticker *)pBody)->sticker_.get();
			if (pSticker->type_->get_id() != TD::stickerTypeRegular::ID)
				break;

			const char *pwszFileExt;
			switch (pSticker->format_->get_id()) {
			case TD::stickerFormatTgs::ID: pwszFileExt = "tga"; break;
			case TD::stickerFormatWebm::ID: pwszFileExt = "webm"; break;
			case TD::stickerFormatWebp::ID: pwszFileExt = "webp"; break;
			default:pwszFileExt = "jpeg"; break;
			}

			return GetMessageSticker(pSticker->thumbnail_->file_.get(), pwszFileExt);
		}
		else debugLogA("SmileyAdd plugin isn't installed, skipping sticker");
		break;

	case TD::messageText::ID:
		auto pText = ((TD::messageText *)pBody)->text_.get();
		if (pText->get_id() == TD::formattedText::ID)
			return CMStringA(((TD::formattedText *)pText)->text_.c_str());
		break;
	}

	return CMStringA();
}
