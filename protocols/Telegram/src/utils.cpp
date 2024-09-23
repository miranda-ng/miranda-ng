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

enum class BBCODE
{
	BOLD, ITALIC, STRIKE, UNDERLINE, URL, CODE
};

struct
{
	BBCODE type;
	const wchar_t *begin, *end;
	unsigned len1, len2;
}
static bbCodes[] =
{
	{ BBCODE::BOLD,      L"[b]",    L"[/b]",    3, 4 },
	{ BBCODE::ITALIC,    L"[i]",    L"[/i]",    3, 4 },
	{ BBCODE::STRIKE,    L"[s]",    L"[/s]",    3, 4 },
	{ BBCODE::UNDERLINE, L"[u]",    L"[/u]",    3, 4 },
	{ BBCODE::URL,       L"[url]",  L"[/url]",  5, 6 },
	{ BBCODE::CODE,      L"[code]", L"[/code]", 6, 7 },
};

TD::object_ptr<TD::formattedText> formatBbcodes(const char *pszText)
{
	auto res = TD::make_object<TD::formattedText>();
	if (mir_strlen(pszText)) {
		std::wstring str = Utf2T(pszText).get();
		for (auto &it : bbCodes) {
			while (true) {
				int i1 = (int)str.find(it.begin);
				if (i1 == str.npos)
					break;

				int i2 = (int)str.find(it.end, i1);
				if (i2 == str.npos)
					break;

				for (auto &jt : res->entities_) {
					if (jt->offset_ > i1)
						jt->offset_ -= it.len1;
					if (jt->offset_ > i2)
						jt->offset_ -= it.len2;
				}

				str.erase(i2, it.len2); i2 -= it.len1;
				str.erase(i1, it.len1);

				TD::object_ptr<TD::TextEntityType> pNew;
				switch (it.type) {
				case BBCODE::URL: pNew = TD::make_object<TD::textEntityTypeUrl>(); break;
				case BBCODE::CODE: pNew = TD::make_object<TD::textEntityTypeCode>(); break;
				case BBCODE::BOLD: pNew = TD::make_object<TD::textEntityTypeBold>(); break;
				case BBCODE::ITALIC: pNew = TD::make_object<TD::textEntityTypeItalic>(); break;
				case BBCODE::STRIKE: pNew = TD::make_object<TD::textEntityTypeStrikethrough>(); break;
				case BBCODE::UNDERLINE: pNew = TD::make_object<TD::textEntityTypeUnderline>(); break;
				}

				res->entities_.push_back(TD::make_object<TD::textEntity>(TD::int32(i1), TD::int32(i2 - i1), std::move(pNew)));
			}
		}
		res->text_ = T2Utf(str.c_str()).get();
	}
	
	return res;
}

CMStringA CTelegramProto::GetFormattedText(TD::object_ptr<TD::formattedText> &pText)
{
	if (pText->get_id() == TD::formattedText::ID) {
		CMStringW ret(Utf2T(pText->text_.c_str()));
		unsigned offset = 0;

		for (auto &it : pText->entities_) {
			int iCode;
			switch (it->type_->get_id()) {
			case TD::textEntityTypeBold::ID: iCode = 0; break;
			case TD::textEntityTypeItalic::ID: iCode = 1; break;
			case TD::textEntityTypeStrikethrough::ID: iCode = 2; break;
			case TD::textEntityTypeUnderline::ID: iCode = 3; break;
			case TD::textEntityTypeCode::ID: iCode = 5; break;
			case TD::textEntityTypeUrl::ID:
				if (!m_bUrlPreview)
					continue;
				iCode = 4;
				break;
			default:
				continue;
			}

			auto &bb = bbCodes[iCode];
			ret.Insert(offset + it->offset_ + it->length_, bb.end);
			ret.Insert(offset + it->offset_, bb.begin);
			offset += bb.len1 + bb.len2;
		}
		return T2Utf(ret).get();
	}
	
	return "";
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA msg2id(TD::int53 chatId, TD::int53 msgId)
{
	return CMStringA(FORMAT, "%lld_%lld", chatId, msgId);
}

CMStringA msg2id(const TD::message *pMsg)
{
	return CMStringA(FORMAT, "%lld_%lld", pMsg->chat_id_, pMsg->id_);
}

TD::int53 dbei2id(const DBEVENTINFO &dbei)
{
	if (dbei.szId == nullptr)
		return -1;

	auto *p = strchr(dbei.szId, '_');
	return _atoi64(p ? p + 1 : dbei.szId);
}

/////////////////////////////////////////////////////////////////////////////////////////

const char *getName(const TD::usernames *pName)
{
	return (pName == nullptr) ? TranslateU("none") : pName->editable_username_.c_str();
}

TD::object_ptr<TD::inputFileLocal> makeFile(const wchar_t *pwszFilename)
{
	std::string szPath = T2Utf(pwszFilename);
	return TD::make_object<TD::inputFileLocal>(std::move(szPath));
}

TG_FILE_REQUEST::Type AutoDetectType(const wchar_t *pwszFilename)
{
	if (ProtoGetAvatarFileFormat(pwszFilename) != PA_FORMAT_UNKNOWN)
		return TG_FILE_REQUEST::PICTURE;

	CMStringW path(pwszFilename);
	int idx = path.ReverseFind('.');
	if (idx == -1 || path.Find('\\', idx) != -1)
		return TG_FILE_REQUEST::FILE;

	auto wszExt = path.Right(path.GetLength() - idx);
	wszExt.MakeLower();
	if (wszExt == L"mp4" || wszExt == L"webm")
		return TG_FILE_REQUEST::VIDEO;
	else if (wszExt == L"mp3" || wszExt == "ogg" || wszExt == "oga" || wszExt == "wav")
		return TG_FILE_REQUEST::VOICE;

	return TG_FILE_REQUEST::FILE;
}

CMStringW TG_USER::getDisplayName() const
{
	if (hContact != 0) {
		if (hContact != INVALID_CONTACT_ID)
			return Clist_GetContactDisplayName(hContact, 0);

		if (!wszFirstName.IsEmpty())
			return (wszLastName.IsEmpty()) ? wszFirstName : wszFirstName + L" " + wszLastName;
	}

	return wszNick;
}

void CTelegramProto::RemoveFromClist(TG_USER *pUser)
{
	Contact::RemoveFromList(pUser->hContact);

	if (pUser->isForum) {
		if (MGROUP hGroup = Clist_GroupExists(ptrW(Clist_GetGroup(pUser->hContact))))
			Clist_GroupDelete(hGroup, true);

		for (auto &cc : AccContacts())
			if (pUser->id == GetId(cc, DBKEY_OWNER))
				Contact::RemoveFromList(cc);
	}
}

int CTelegramProto::GetDefaultMute(const TG_USER *pUser)
{
	if (pUser->isGroupChat)
		return (pUser->isChannel) ? m_iDefaultMuteChannel : m_iDefaultMuteGroup;
	return m_iDefaultMutePrivate;
}

MCONTACT CTelegramProto::GetRealContact(const TG_USER *pUser)
{
	return (pUser->hContact != 0) ? pUser->hContact : m_iSavedMessages;
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

int64_t CTelegramProto::GetId(MCONTACT hContact, const char *pszSetting)
{
	return _atoi64(getMStringA(hContact, pszSetting));
}

void CTelegramProto::SetId(MCONTACT hContact, int64_t id, const char *pszSetting)
{
	char szId[100];
	_i64toa(id, szId, 10);
	setString(hContact, pszSetting, szId);
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
		setByte(hContact, "ChatRoom", 1);
	}
	else if (mir_wstrlen(m_wszDefaultGroup))
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	if (pUser == nullptr) {
		pUser = new TG_USER(id, hContact, bIsChat);
		m_arUsers.insert(pUser);
	}
	else {
		pUser->hContact = hContact;
		setWString(hContact, "Nick", pUser->wszNick);
		if (!pUser->isGroupChat) {
			setWString(hContact, "FirstName", pUser->wszFirstName);
			setWString(hContact, "LastName", pUser->wszLastName);
		}
		else pUser->bStartChat = true;
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

			CMStringW wszDisplayName(pSender->getDisplayName());
			if (pUser->m_si && !pSender->wszFirstName.IsEmpty())
				g_chatApi.UM_AddUser(pUser->m_si, Utf2T(dest), wszDisplayName, ID_STATUS_ONLINE);
			
			mir_strncpy(dest, T2Utf(wszDisplayName), 100);
			return true;
		}
	}

	*dest = 0;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CTelegramProto::GetMessageFile(const EmbeddedFile &F, TG_FILE_REQUEST::Type iType, const TD::file *pFile, const char *pszFileName, const char *pszCaption)
{
	if (pFile->get_id() != TD::file::ID) {
		debugLogA("Document contains unsupported type %d, exiting", pFile->get_id());
		return false;
	}

	auto *pRequest = new TG_FILE_REQUEST(iType, pFile->id_, pFile->remote_->id_.c_str());
	pRequest->m_fileName = Utf2T(pszFileName);
	pRequest->m_fileSize = pFile->size_;
	pRequest->m_bRecv = !F.pMsg->is_outgoing_;
	pRequest->m_hContact = GetRealContact(F.pUser);

	if (mir_strlen(pszCaption))
		F.szBody += pszCaption;

	char szReplyId[100];

	DB::EventInfo dbei(db_event_getById(m_szModuleName, F.pszId));
	dbei.flags = DBEF_TEMPORARY;
	dbei.timestamp = F.pMsg->date_;
	dbei.szId = F.pszId;
	dbei.szUserId = F.pszUser;
	if (F.pMsg->is_outgoing_)
		dbei.flags |= DBEF_SENT;
	if (!F.pUser->bInited || F.bRead)
		dbei.flags |= DBEF_READ;
	if (F.pMsg->reply_to_message_id_) {
		_i64toa(F.pMsg->reply_to_message_id_, szReplyId, 10);
		dbei.szReplyId = szReplyId;
	}

	if (dbei) {
		DB::FILE_BLOB blob(dbei);
		OnReceiveOfflineFile(dbei, blob);
		blob.write(dbei);
		db_event_edit(dbei.getEvent(), &dbei, true);
		delete pRequest;
	}
	else ProtoChainRecvFile(pRequest->m_hContact, DB::FILE_BLOB(pRequest, pszFileName, F.szBody), dbei);
	
	F.szBody.Empty();
	return true;
}

CMStringA CTelegramProto::GetMessagePreview(const TD::file *pFile)
{
	auto *pFileId = pFile->remote_->unique_id_.c_str();

	auto *pRequest = new TG_FILE_REQUEST(TG_FILE_REQUEST::AVATAR, pFile->id_, pFileId);
	pRequest->m_destPath = GetPreviewPath();
	CreateDirectoryTreeW(pRequest->m_destPath);

	pRequest->m_fileName.Format(L"{%S}.jpg", pFileId);
	{
		mir_cslock lck(m_csFiles);
		m_arFiles.insert(pRequest);
	}

	SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, true));
	return T2Utf(pRequest->m_destPath + L"\\" + pRequest->m_fileName).get();
}

CMStringA CTelegramProto::GetMessageSticker(const TD::file *pFile, const char *pwszExtension)
{
	auto *pFileId = pFile->remote_->unique_id_.c_str();

	auto *pRequest = new TG_FILE_REQUEST(TG_FILE_REQUEST::AVATAR, pFile->id_, pFileId);
	pRequest->m_isSmiley = true;
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
	const char *types[] = { "y", "x", "m", "s" };
	for (auto *pType : types)
		for (auto &it : pPhoto->sizes_)
			if (it->type_ == pType)
				return it.get();

	return nullptr;
}

static bool checkStickerType(uint32_t ID)
{
	switch (ID) {
	case TD::stickerTypeRegular::ID:
	case TD::stickerFullTypeRegular::ID:
		return true;
	default:
		return false;
	}	
}

CMStringA CTelegramProto::GetMessageText(TG_USER *pUser, const TD::message *pMsg, bool bSkipJoin, bool bRead)
{
	const TD::MessageContent *pBody = pMsg->content_.get();

	char szUserId[100], *pszUserId = nullptr;
	auto szMsgId(msg2id(pMsg));
	if (GetGcUserId(pUser, pMsg, szUserId))
		pszUserId = szUserId;

	CMStringA ret;

	if (auto *pForward = pMsg->forward_info_.get()) {
		CMStringW wszNick;
		switch (pForward->origin_->get_id()) {
		case TD::messageForwardOriginUser::ID:
			if (auto *p = FindUser(((TD::messageForwardOriginUser *)pForward->origin_.get())->sender_user_id_))
				wszNick = p->getDisplayName();
			break;
		case TD::messageForwardOriginChat::ID:
			if (auto *p = FindChat(((TD::messageForwardOriginChat *)pForward->origin_.get())->sender_chat_id_))
				wszNick = p->getDisplayName();
			break;
		case TD::messageForwardOriginHiddenUser::ID:
			if (auto *p = (TD::messageForwardOriginHiddenUser *)pForward->origin_.get())
				wszNick = Utf2T(p->sender_name_.c_str());
			break;
		case TD::messageForwardOriginChannel::ID:
			if (auto *p = FindChat(((TD::messageForwardOriginChannel *)pForward->origin_.get())->chat_id_))
				wszNick = p->getDisplayName();
			break;
		default:
			wszNick = TranslateT("Unknown");
		}

		wchar_t wszDate[100];
		TimeZone_PrintTimeStamp(0, pForward->date_, L"d t", wszDate, _countof(wszDate), 0);
		CMStringW wszForward(FORMAT, L">%s %s %s\r\n", wszDate, wszNick.c_str(), TranslateT("wrote"));
		ret.Insert(0, T2Utf(wszForward));
	}

	EmbeddedFile embed(ret);
	embed.pUser = pUser;
	embed.bRead = bRead;
	embed.pszId= szMsgId;
	embed.pszUser = szUserId;
	embed.pMsg = pMsg;

	switch (pBody->get_id()) {
	case TD::messageChatUpgradeTo::ID:
		if (auto *pUgrade = (TD::messageChatUpgradeTo *)pBody) {
			MCONTACT hContact = pUser->hContact;
			m_arChats.remove(pUser);
			m_arUsers.remove(pUser);
			SetId(hContact, pUgrade->supergroup_id_);
			pUser = new TG_USER(pUgrade->supergroup_id_, hContact, true);
			m_arUsers.insert(pUser);
		}
		break;

	case TD::messageChatAddMembers::ID:
		if (!bSkipJoin)
			if (auto *pDoc = (TD::messageChatAddMembers *)pBody)
				for (auto &it : pDoc->member_user_ids_)
					GcChangeMember(pUser, pszUserId, it, true);
		break;

	case TD::messageChatDeleteMember::ID:
		if (!bSkipJoin)
			if (auto *pDoc = (TD::messageChatDeleteMember *)pBody)
				GcChangeMember(pUser, pszUserId, pDoc->user_id_, false);
		break;

	case TD::messageChatChangeTitle::ID:
		if (auto *pDoc = (TD::messageChatChangeTitle *)pBody) {
			if (pUser->m_si)
				Chat_ChangeSessionName(pUser->m_si, Utf2T(pDoc->title_.c_str()));
			else
				setUString(pUser->hContact, "Nick", pDoc->title_.c_str());

			ret.AppendFormat(TranslateU("Chat name was changed to %s"), pDoc->title_.c_str());
		}
		break;

	case TD::messagePhoto::ID:
		if (auto *pDoc = (TD::messagePhoto *)pBody) {
			auto *pPhoto = GetBiggestPhoto(pDoc->photo_.get());
			if (pPhoto == nullptr) {
				debugLogA("cannot find photo, exiting");
				break;
			}

			CMStringA fileName(FORMAT, "%s (%d x %d)", TranslateU("Picture"), pPhoto->width_, pPhoto->height_);
			GetMessageFile(embed, TG_FILE_REQUEST::PICTURE, pPhoto->photo_.get(), fileName, pDoc->caption_->text_.c_str());
		}
		break;

	case TD::messageAudio::ID:
		if (auto *pDoc = (TD::messageAudio *)pBody) {
			auto *pAudio = pDoc->audio_.get();
			CMStringA fileName(FORMAT, "%s (%d %s)", TranslateU("Audio"), pAudio->duration_, TranslateU("seconds"));
			std::string caption = fileName.c_str();
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_;
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pAudio->audio_.get(), pAudio->file_name_.c_str(), caption.c_str());
		}
		break;

	case TD::messageVideo::ID:
		if (auto *pDoc = (TD::messageVideo *)pBody) {
			auto *pVideo = pDoc->video_.get();
			CMStringA fileName(FORMAT, "%s (%d x %d, %d %s)", TranslateU("Video"), pVideo->width_, pVideo->height_, pVideo->duration_, TranslateU("seconds"));
			std::string caption = fileName.c_str();
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_;
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pVideo->video_.get(), pVideo->file_name_.c_str(), caption.c_str());
		}
		break;

	case TD::messageAnimation::ID:
		if (auto *pDoc = (TD::messageAnimation *)pBody) {
			auto *pVideo = pDoc->animation_.get();
			CMStringA fileName(FORMAT, "%s (%d x %d, %d %s)", TranslateU("Video"), pVideo->width_, pVideo->height_, pVideo->duration_, TranslateU("seconds"));
			std::string caption = fileName.c_str();
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_;
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pVideo->animation_.get(), pVideo->file_name_.c_str(), caption.c_str());
		}
		break;

	case TD::messageVoiceNote::ID:
		if (auto *pDoc = (TD::messageVoiceNote *)pBody) {
			CMStringA fileName(FORMAT, "%s (%d %s)", TranslateU("Voice message"), pDoc->voice_note_->duration_, TranslateU("seconds"));
			GetMessageFile(embed, TG_FILE_REQUEST::VOICE, pDoc->voice_note_->voice_.get(), fileName, pDoc->caption_->text_.c_str());
		}
		break;

	case TD::messageDocument::ID:
		if (auto *pDoc = (TD::messageDocument *)pBody)
			GetMessageFile(embed, TG_FILE_REQUEST::FILE, pDoc->document_->document_.get(), pDoc->document_->file_name_.c_str(), pDoc->caption_->text_.c_str());
		break;

	case TD::messageAnimatedEmoji::ID:
		if (auto *pObj = (TD::messageAnimatedEmoji *)pBody) {
			if (m_bSmileyAdd) {
				if (auto *pAnimated = pObj->animated_emoji_.get()) {
					if (auto *pSticker = pAnimated->sticker_.get()) {
						if (!checkStickerType(pSticker->full_type_->get_id())) {
							debugLogA("You received a sticker of unsupported type %d, ignored", pSticker->full_type_->get_id());
							break;
						}

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

						ret = GetMessageSticker(pSticker->thumbnail_->file_.get(), pwszFileExt);
						break;
					}
				}
			}
			ret += pObj->emoji_.c_str();
		}
		break;

	case TD::messageSticker::ID:
		if (auto *pSticker = ((TD::messageSticker *)pBody)->sticker_.get()) {
			if (!checkStickerType(pSticker->full_type_->get_id())) {
				debugLogA("You received a sticker of unsupported type %d, ignored", pSticker->full_type_->get_id());
				break;
			}
			
			if (m_bSmileyAdd) {
				const char *pwszFileExt;
				switch (pSticker->format_->get_id()) {
				case TD::stickerFormatTgs::ID: pwszFileExt = "tga"; break;
				case TD::stickerFormatWebm::ID: pwszFileExt = "webm"; break;
				case TD::stickerFormatWebp::ID: pwszFileExt = "webp"; break;
				default:pwszFileExt = "jpeg"; break;
				}

				ret = GetMessageSticker(pSticker->thumbnail_->file_.get(), pwszFileExt);
			}
			else ret.AppendFormat("%s: %s", TranslateU("SmileyAdd plugin required to support stickers"), pSticker->emoji_.c_str());
		}
		break;

	case TD::messageInvoice::ID:
		if (auto *pInvoice = ((TD::messageInvoice *)pBody)) {
			ret.Format("%s: %.2lf %s", TranslateU("You received an invoice"), double(pInvoice->total_amount_)/100.0, pInvoice->currency_.c_str());
			if (!pInvoice->title_.empty())
				ret.AppendFormat("\r\n%s: %s", TranslateU("Title"), pInvoice->title_.c_str());
			if (auto pszText = GetFormattedText(pInvoice->description_))
				ret.AppendFormat("\r\n%s", pszText.c_str());
		}
		break;

	case TD::messageText::ID:
		if (auto *pText = ((TD::messageText *)pBody)) {
			ret = GetFormattedText(pText->text_);

			if (auto *pWeb = pText->web_page_.get()) {
				if (!pWeb->embed_url_.empty() && m_bUrlPreview)
					ret.AppendFormat("\r\n[url]%s[/url]", pWeb->embed_url_.c_str());

				if (pWeb->photo_) {
					const TD::photoSize *pSize = nullptr;
					for (auto &it : pWeb->photo_->sizes_)
						if (it->type_ == "m")
							pSize = it.get();

					if (pSize == nullptr)
						pSize = pWeb->photo_->sizes_[0].get();

					if (auto szText = GetMessagePreview(pSize->photo_.get()))
						ret.AppendFormat("\r\n[img=%s][/img]", szText.c_str());
				}

				if (auto szText = GetFormattedText(pWeb->description_))
					ret.AppendFormat("\r\n%s", szText.c_str());
			}
		}
		break;
	}

	return ret;
}
