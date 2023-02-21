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

const char* getSender(const TD::MessageSender *pSender, char *pDest, size_t cbSize)
{
	switch (pSender->get_id()) {
	case TD::messageSenderChat::ID:
		_i64toa_s(((TD::messageSenderChat *)pSender)->chat_id_, pDest, cbSize, 10);
		break;
	case TD::messageSenderUser::ID:
		_i64toa_s(((TD::messageSenderUser *)pSender)->user_id_, pDest, cbSize, 10);
		break;
	default:
		*pDest = 0;
	}
	return pDest;
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
	CMStringW wszId(FORMAT, L"%lld", pUser->id);

	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = wszId.GetBuffer();

	if (pUser->hContact != INVALID_CONTACT_ID) {
		CMStringW wszNick = getMStringW(pUser->hContact, "Nick");
		CMStringW wszLastName = getMStringW(pUser->hContact, "LastName");
		CMStringW wszFirstName = getMStringW(pUser->hContact, "FirstName");

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

	pUser = new TG_USER(id, hContact, bIsChat);
	m_arUsers.insert(pUser);
	if (!bIsChat)
		m_arChats.insert(pUser);
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

CMStringA CTelegramProto::GetMessageText(TG_USER *pUser, TD::MessageContent *pBody)
{
	switch (pBody->get_id()) {
	case TD::messageDocument::ID:
		{
			auto *pDoc = ((TD::messageDocument *)pBody);
			auto *pFile = pDoc->document_->document_.get();

			if (pFile->get_id() != TD::file::ID) {
				debugLogA("Document contains unsupported type %d, exiting", pDoc->document_->get_id());
				break;
			}
			
			auto *pFileId = pFile->remote_->unique_id_.c_str();

			CMStringW wszDest(GetProtoFolder() + L"\\tmpfiles");
			CreateDirectoryW(wszDest, 0);
			wszDest.AppendFormat(L"\\%s", Utf2T(pDoc->document_->file_name_.c_str()).get());

			auto *pRequest = new TG_FILE_REQUEST(TG_FILE_REQUEST::FILE, pFileId, wszDest);
			m_arFiles.insert(pRequest);

			auto *pszFileName = pDoc->document_->file_name_.c_str();

			PROTORECVFILE pre = {};
			pre.fileCount = 1;
			pre.timestamp = time(0);
			pre.files.a = &pszFileName;
			pre.lParam = (LPARAM)pRequest;
			if (!pDoc->caption_->text_.empty())
				pre.descr.a = pDoc->caption_->text_.c_str();
			ProtoChainRecvFile(pUser->hContact, &pre);
		}
		break;

	case TD::messageSticker::ID:
		if (m_bSmileyAdd) {
			auto *pSticker = ((TD::messageSticker *)pBody)->sticker_.get();
			if (pSticker->type_->get_id() != TD::stickerTypeRegular::ID)
				break;

			auto *pFile = pSticker->thumbnail_->file_.get();
			auto *pFileId = pFile->remote_->unique_id_.c_str();

			const char *pwszFileExt;
			switch (pSticker->format_->get_id()) {
			case TD::stickerFormatTgs::ID: pwszFileExt = "tga"; break;
			case TD::stickerFormatWebm::ID: pwszFileExt = "webm"; break;
			case TD::stickerFormatWebp::ID: pwszFileExt = "webp"; break;
			default:pwszFileExt = "jpeg"; break;
			}

			CMStringW wszDest(GetAvatarPath() + L"\\Stickers");
			CreateDirectoryW(wszDest, 0);
			wszDest.AppendFormat(L"\\STK{%S}.%S", pFileId, pwszFileExt);

			m_arFiles.insert(new TG_FILE_REQUEST(TG_FILE_REQUEST::AVATAR, pFileId, wszDest));

			SendQuery(new TD::downloadFile(pFile->id_, 10, 0, 0, true));
			return CMStringA(FORMAT, "STK{%s}", pFileId);
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
