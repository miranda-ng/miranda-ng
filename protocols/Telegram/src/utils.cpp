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

char* getSender(const TD::MessageSender *pSender, char *pDest, size_t cbSize)
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

void CTelegramProto::UpdateString(MCONTACT hContact, const char *pszSetting, const std::string &str)
{
	if (str.empty())
		delSetting(hContact, pszSetting);
	else
		setUString(hContact, pszSetting, str.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Users

TG_USER* CTelegramProto::FindUser(int64_t id)
{
	if (auto *pCache = m_arUsers.find((TG_USER *)&id))
		return pCache;

	if (id < 0) {
		id = -id;
		if (auto *pCache = m_arUsers.find((TG_USER *)&id))
			return pCache;
	}

	return nullptr;
}

TG_USER* CTelegramProto::AddUser(int64_t id, bool bIsChat)
{
	auto *pUser = FindUser(id);
	if (pUser != nullptr)
		return pUser;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	char szId[100];
	_i64toa(id, szId, 10);
	setString(hContact, DBKEY_ID, szId);

	if (bIsChat) {
		Clist_SetGroup(hContact, TranslateT("Chat rooms"));
		setByte(hContact, "ChatRoom", 1);
	}
	else if (mir_wstrlen(m_wszDefaultGroup))
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	pUser = new TG_USER(id, hContact, bIsChat);
	m_arUsers.insert(pUser);
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

CMStringA CTelegramProto::GetMessageText(TD::MessageContent *pBody)
{
	switch (pBody->get_id()) {
	case TD::messageSticker::ID:
		if (m_bSmileyAdd) {
			auto pSticker = ((TD::messageSticker *)pBody)->sticker_.get();
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
