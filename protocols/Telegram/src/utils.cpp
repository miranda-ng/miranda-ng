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

enum class BBCODE
{
	BOLD, ITALIC, STRIKE, UNDERLINE, URL, CODE, QUOTE
};

struct Bbcode
{
	BBCODE type;
	const wchar_t *begin, *end;
	unsigned len1, len2;
}
static bbCodes[] =
{
	{ BBCODE::BOLD,      L"[b]",     L"[/b]",     3, 4 },
	{ BBCODE::ITALIC,    L"[i]",     L"[/i]",     3, 4 },
	{ BBCODE::STRIKE,    L"[s]",     L"[/s]",     3, 4 },
	{ BBCODE::UNDERLINE, L"[u]",     L"[/u]",     3, 4 },
	{ BBCODE::URL,       L"[url]",   L"[/url]",   5, 6 },
	{ BBCODE::CODE,      L"[code]",  L"[/code]",  6, 7 },
	{ BBCODE::QUOTE,     L"[quote]", L"[/quote]", 7, 8 },
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
				case BBCODE::QUOTE: pNew = TD::make_object<TD::textEntityTypeBlockQuote>(); break;
				case BBCODE::ITALIC: pNew = TD::make_object<TD::textEntityTypeItalic>(); break;
				case BBCODE::STRIKE: pNew = TD::make_object<TD::textEntityTypeStrikethrough>(); break;
				case BBCODE::UNDERLINE: pNew = TD::make_object<TD::textEntityTypeUnderline>(); break;
				}

				for (auto &jt : res->entities_) {
					if (i1 >= jt->offset_ && i1 < jt->offset_ + jt->length_)
						jt->length_ -= it.len1;
					if (i2 >= jt->offset_ && i2 < jt->offset_ + jt->length_)
						jt->length_ -= it.len2;
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
	CMStringW ret(Utf2T(pText->text_.c_str()));

	struct HistItem {
		HistItem(int _1, int _2, const Bbcode &_3) : start(_1), length(_2), l1(_3.len1), l2(_3.len2) {}
		int start, length, l1, l2;
	};
	std::vector<HistItem> history;

	for (auto &it : pText->entities_) {
		int iCode;
		switch (it->type_->get_id()) {
		case TD::textEntityTypeBold::ID: iCode = 0; break;
		case TD::textEntityTypeItalic::ID: iCode = 1; break;
		case TD::textEntityTypeStrikethrough::ID: iCode = 2; break;
		case TD::textEntityTypeUnderline::ID: iCode = 3; break;
		case TD::textEntityTypeTextUrl::ID: iCode = 4; break;
		case TD::textEntityTypeCode::ID: iCode = 5; break;
		case TD::textEntityTypeBlockQuote::ID: iCode = 6; break;
		default:
			continue;
		}

		int off1 = 0, off2 = 0;
		for (auto &h : history) {
			if (it->offset_ >= h.start)
				off1 += h.l1;
			if (it->offset_ + it->length_ > h.start)
				off2 += h.l1;
			if (it->offset_ >= h.start + h.length)
				off1 += h.l2;
			if (it->offset_ + it->length_ > h.start + h.length)
				off2 += h.l2;
		}

		auto &bb = bbCodes[iCode];
		HistItem histItem(it->offset_, it->length_, bb);
		ret.Insert(off2 + it->offset_ + it->length_, bb.end);
		ret.Insert(off1 + it->offset_, bb.begin);
		if (iCode == 4) {
			auto *pUrl = (TD::textEntityTypeTextUrl *)it->type_.get();
			Utf2T wszUrl(pUrl->url_.c_str());
			ret.Insert(off1 + it->offset_ + 4, wszUrl);
			ret.Insert(off1 + it->offset_ + 4, L"=");
			histItem.l1 += 1 + (int)mir_wstrlen(wszUrl);
		}
		history.push_back(histItem);
	}
	return T2Utf(ret).get();
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA msg2id(TD::int53 chatId, TD::int53 msgId)
{
	return CMStringA(FORMAT, "%lld_%lld", chatId, msgId);
}

CMStringA msg2id(const TD::message *pMsg)
{
	auto iChatId = pMsg->chat_id_;
	if (!iChatId && pMsg->sender_id_->get_id() == TD::messageSenderChat::ID)
		iChatId = ((TD::messageSenderChat *)pMsg->sender_id_.get())->chat_id_;

	return CMStringA(FORMAT, "%lld_%lld", iChatId, pMsg->id_);
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
	if (int iFormat = ProtoGetAvatarFileFormat(pwszFilename)) {
		if (iFormat != PA_FORMAT_GIF)
			return TG_FILE_REQUEST::PICTURE;

		if (auto *pBitmap = FreeImage_OpenMultiBitmapU(FIF_GIF, pwszFilename, FALSE, TRUE)) {
			int iPages = FreeImage_GetPageCount(pBitmap);
			FreeImage_CloseMultiBitmap(pBitmap);
			if (iPages <= 1)
				return TG_FILE_REQUEST::PICTURE;
		}
	}

	CMStringW path(pwszFilename);
	int idx = path.ReverseFind('.');
	if (idx == -1 || path.Find('\\', idx) != -1)
		return TG_FILE_REQUEST::FILE;

	auto wszExt = path.Right(path.GetLength() - idx - 1);
	wszExt.MakeLower();
	if (wszExt == L"mp4" || wszExt == L"webm" || wszExt == L"gif")
		return TG_FILE_REQUEST::VIDEO;
	
	if (wszExt == L"mp3" || wszExt == "ogg" || wszExt == "oga" || wszExt == "wav")
		return TG_FILE_REQUEST::VOICE;

	return TG_FILE_REQUEST::FILE;
}

TD::int53 getReplyId(const TD::MessageReplyTo *pReply)
{
	if (pReply) {
		switch (pReply->get_id()) {
		case TD::messageReplyToMessage::ID:
			return ((TD::messageReplyToMessage *)pReply)->message_id_;

		case TD::inputMessageReplyToExternalMessage::ID:
			return ((TD::inputMessageReplyToExternalMessage *)pReply)->message_id_;
		}
	}
	return 0;
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

void CTelegramProto::MarkRead(MCONTACT hContact, const CMStringA &szMaxId, bool bSent)
{
	for (MEVENT hEvent = db_event_firstUnread(hContact); hEvent; hEvent = db_event_next(hContact, hEvent)) {
		DB::EventInfo dbei(hEvent, false);
		if (!dbei || !dbei.szId)
			continue;

		if (dbei.szId > szMaxId)
			break;

		if (dbei.bSent != bSent)
			continue;

		if (!dbei.bRead)
			db_event_markRead(hContact, hEvent, true);
	}
}

int CTelegramProto::GetDefaultMute(const TG_USER *pUser)
{
	if (pUser->isGroupChat)
		return (pUser->isChannel) ? m_iDefaultMuteChannel : m_iDefaultMuteGroup;
	return m_iDefaultMutePrivate;
}

MCONTACT CTelegramProto::GetRealContact(const TG_USER *pUser, int64_t threadId)
{
	if (threadId)
		if (auto *pu = FindChat(pUser->chatId, threadId))
			return pu->hContact;

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

void CTelegramProto::CheckCompatibility()
{
	int iLevel = db_get_b(0, "Compatibility", m_szModuleName);

	if (iLevel < 1) {
		for (auto &cc : AccContacts())
			delSetting(cc, "Notes");
		delSetting("Notes");
	}

	db_set_b(0, "Compatibility", m_szModuleName, 1);
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

TG_SUPER_GROUP* CTelegramProto::FindSuperGroup(int64_t id)
{
	TG_SUPER_GROUP tmp(id, 0);
	return m_arSuperGroups.find(&tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Users

TG_USER* CTelegramProto::FindChat(int64_t id)
{
	auto *tmp = (TG_USER *)_alloca(sizeof(TG_USER));
	tmp->chatId = id;
	tmp->forumId = -1;
	return m_arChats.find(tmp);
}

TG_USER* CTelegramProto::FindChat(int64_t id, int64_t forumId)
{
	auto *tmp = (TG_USER *)_alloca(sizeof(TG_USER));
	tmp->chatId = id;
	tmp->forumId = forumId;
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
		if (pUser->wszNick[0] == '@')
			pUser->wszNick.Delete(0, 1);
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
	auto *pRequest = new TG_FILE_REQUEST(iType, pFile->id_, pFile->remote_->id_.c_str());
	pRequest->m_fileName = Utf2T(pszFileName);
	pRequest->m_fileSize = pFile->size_;
	pRequest->m_bRecv = !F.pMsg->is_outgoing_;
	pRequest->m_hContact = GetRealContact(F.pUser, F.pMsg->message_thread_id_);

	if (mir_strlen(pszCaption))
		F.szBody += pszCaption;

	char szReplyId[100];

	DB::EventInfo dbei(db_event_getById(m_szModuleName, F.pszId));
	dbei.bTemporary = true;
	dbei.szId = F.pszId;
	dbei.szUserId = F.pszUser;
	if (F.pMsg->date_)
		dbei.iTimestamp = F.pMsg->date_;
	if (F.pMsg->is_outgoing_) {
		dbei.bSent = true;
		if (F.pUser->id != m_iOwnId)
			dbei.bRead = true;
	}
	if (!F.pUser->bInited || F.bRead)
		dbei.bRead = true;
	if (auto iReplyId = getReplyId(F.pMsg->reply_to_.get())) {
		_i64toa(iReplyId, szReplyId, 10);
		dbei.szReplyId = szReplyId;
	}

	if (dbei) {
		if (!Ignore_IsIgnored(pRequest->m_hContact, IGNOREEVENT_FILE)) {
			DB::FILE_BLOB blob(dbei);
			blob.setDescr(Utf2T(pszCaption));
			OnReceiveOfflineFile(dbei, blob);
			blob.write(dbei);
			db_event_edit(dbei.getEvent(), &dbei, true);
		}
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

CMStringA CTelegramProto::GetMessageSticker(const TD::file *pFile, const TD::ThumbnailFormat *pFormat)
{
	auto *pFileId = pFile->remote_->unique_id_.c_str();

	auto *pRequest = new TG_FILE_REQUEST(TG_FILE_REQUEST::AVATAR, pFile->id_, pFileId);
	pRequest->m_isSmiley = true;
	pRequest->m_destPath = GetAvatarPath() + L"\\Stickers";
	CreateDirectoryW(pRequest->m_destPath, 0);

	const char *pszFileExt;
	switch (pFormat->get_id()) {
	case TD::thumbnailFormatGif::ID: pszFileExt = "gif"; break;
	case TD::thumbnailFormatPng::ID: pszFileExt = "png"; break;
	case TD::thumbnailFormatTgs::ID: pszFileExt = "tga"; break;
	case TD::thumbnailFormatJpeg::ID: pszFileExt = "jpg"; break;
	case TD::thumbnailFormatWebm::ID: pszFileExt = "webm"; break;
	case TD::thumbnailFormatWebp::ID: pszFileExt = "webp"; break;
	default:pszFileExt = "jpeg"; break;
	}

	pRequest->m_fileName.Format(L"STK{%S}.%S", pFileId, pszFileExt);
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
		case TD::messageOriginUser::ID:
			if (auto *p = FindUser(((TD::messageOriginUser *)pForward->origin_.get())->sender_user_id_))
				wszNick = p->getDisplayName();
			break;
		case TD::messageOriginChat::ID:
			if (auto *p = FindChat(((TD::messageOriginChat *)pForward->origin_.get())->sender_chat_id_))
				wszNick = p->getDisplayName();
			break;
		case TD::messageOriginHiddenUser::ID:
			if (auto *p = (TD::messageOriginHiddenUser *)pForward->origin_.get())
				wszNick = Utf2T(p->sender_name_.c_str());
			break;
		case TD::messageOriginChannel::ID:
			if (auto *p = FindChat(((TD::messageOriginChannel *)pForward->origin_.get())->chat_id_)) {
				auto str = p->getDisplayName();
				wszNick.Format(L"[url=https://t.me/%s]%s[/url]", str.c_str(), str.c_str());
			}
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
			CMStringA caption(FORMAT, "%s (%d %s)", TranslateU("Audio"), pAudio->duration_, TranslateU("seconds"));
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_.c_str();
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pAudio->audio_.get(), pAudio->file_name_.c_str(), caption);
		}
		break;

	case TD::messageVideo::ID:
		if (auto *pDoc = (TD::messageVideo *)pBody) {
			auto *pVideo = pDoc->video_.get();
			CMStringA caption(FORMAT, "%s (%d x %d, %d %s)", TranslateU("Video"), pVideo->width_, pVideo->height_, pVideo->duration_, TranslateU("seconds"));
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_.c_str();
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pVideo->video_.get(), pVideo->file_name_.c_str(), caption);
		}
		break;

	case TD::messageAnimation::ID:
		if (auto *pDoc = (TD::messageAnimation *)pBody) {
			auto *pVideo = pDoc->animation_.get();
			CMStringA caption(FORMAT, "%s (%d x %d, %d %s)", TranslateU("Video"), pVideo->width_, pVideo->height_, pVideo->duration_, TranslateU("seconds"));
			if (!pDoc->caption_->text_.empty()) {
				caption += " ";
				caption += pDoc->caption_->text_.c_str();
			}
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pVideo->animation_.get(), pVideo->file_name_.c_str(), caption.c_str());
		}
		break;

	case TD::messageVideoNote::ID:
		if (auto *pDoc = (TD::messageVideoNote *)pBody) {
			auto *pVideo = pDoc->video_note_.get();
			CMStringA fileName(FORMAT, "%s (%d %s)", TranslateU("Video note"), pVideo->duration_, TranslateU("seconds"));
			GetMessageFile(embed, TG_FILE_REQUEST::VIDEO, pVideo->video_.get(), fileName, "");
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

						ret = GetMessageSticker(pSticker->thumbnail_->file_.get(), pSticker->thumbnail_->format_.get());
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
				if (pSticker->thumbnail_.get()) {
					ret = GetMessageSticker(pSticker->thumbnail_->file_.get(), pSticker->thumbnail_->format_.get());
				}
				else {
					debugLogA("Strange sticker without preview");
					ret.Append(pSticker->emoji_.c_str());
				}
			}
			else ret.AppendFormat("%s: %s", TranslateU("SmileyAdd plugin required to support stickers"), pSticker->emoji_.c_str());
		}
		break;

	case TD::messageInvoice::ID:
		if (auto *pInvoice = ((TD::messageInvoice *)pBody)) {
			ret.Format("%s: %.2lf %s", TranslateU("You received an invoice"), double(pInvoice->total_amount_)/100.0, pInvoice->currency_.c_str());
			if (auto pszText = GetFormattedText(pInvoice->paid_media_caption_))
				ret.AppendFormat("\r\n%s", pszText.c_str());
		}
		break;

	case TD::messageText::ID:
		if (auto *pText = ((TD::messageText *)pBody)) {
			ret = GetFormattedText(pText->text_);

			auto *pWeb = pText->link_preview_.get();
			if (pWeb && m_bIncludePreviews) {
				CMStringA szDescr;

				if (!pWeb->site_name_.empty())
					szDescr.AppendFormat("%s\r\n", pWeb->site_name_.c_str());

				if (!pWeb->title_.empty())
					szDescr.AppendFormat("[b]%s[/b]\r\n", pWeb->title_.c_str());

				if (auto szText = GetFormattedText(pWeb->description_))
					szDescr.AppendFormat("%s\r\n", szText.c_str());

				if (pWeb->type_->get_id() == TD::linkPreviewTypePhoto::ID) {
					auto *pPhoto = ((TD::linkPreviewTypePhoto *)pWeb->type_.get())->photo_.get();
					const TD::photoSize *pSize = nullptr;
					for (auto &it : pPhoto->sizes_)
						if (it->type_ == "m")
							pSize = it.get();

					if (pSize == nullptr)
						pSize = pPhoto->sizes_[0].get();

					if (auto szText = GetMessagePreview(pSize->photo_.get()))
						szDescr.AppendFormat("[img=%s][/img]\r\n", szText.c_str());
				}

				if (!szDescr.IsEmpty())
					ret += "\r\n[quote]" + szDescr.Trim() + "[/quote]";
			}
		}
		break;
	}

	return ret;
}
