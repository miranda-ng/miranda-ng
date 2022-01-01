/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

void CVkProto::AddFeedSpecialUser()
{
	bool bSpecialContact = m_vkOptions.bNewsEnabled || m_vkOptions.bNotificationsEnabled || m_vkOptions.bSpecialContactAlwaysEnabled;

	MCONTACT hContact = FindUser(VK_FEED_USER);
	if (!bSpecialContact) {
		if (hContact)
			DeleteContact(hContact);
		return;
	}

	if (!hContact) {
		hContact = FindUser(VK_FEED_USER, true);

		setWString(hContact, "Nick", TranslateT("VKontakte"));
		CMStringW wszUrl = L"https://vk.com/press/Simple.png";
		SetAvatarUrl(hContact, wszUrl);
		ReloadAvatarInfo(hContact);

		setWString(hContact, "domain", L"feed");
		setWString(hContact, "Homepage", L"https://vk.com/feed");
	}

	if (getWord(hContact, "Status") != ID_STATUS_ONLINE)
		setWord(hContact, "Status", ID_STATUS_ONLINE);
	SetMirVer(hContact, 7);

}

void CVkProto::AddFeedEvent(CVKNewsItem& vkNewsItem)
{
	if (vkNewsItem.wszText.IsEmpty())
		return;

	MCONTACT hContact = FindUser(VK_FEED_USER, true);
	T2Utf pszBody(vkNewsItem.wszText);

	PROTORECVEVENT recv = {};
	recv.timestamp = vkNewsItem.tDate;
	recv.szMessage = pszBody;

	if (m_vkOptions.bUseNonStandardNotifications) {
		recv.flags = PREF_CREATEREAD;
		MsgPopup(hContact, vkNewsItem.wszPopupText, vkNewsItem.wszPopupTitle);
	}

	ProtoChainRecvMsg(hContact, &recv);
}

void CVkProto::AddCListEvent(bool bNews)
{
	Skin_PlaySound("VKNewsFeed");

	if (!m_vkOptions.bUseNonStandardNotifications)
		return;

	MCONTACT hContact = FindUser(VK_FEED_USER, true);

	CLISTEVENT cle = {};
	cle.hIcon = g_plugin.getIcon(IDI_NOTIFICATION);
	cle.pszService = MS_MSG_READMESSAGE;
	cle.flags = CLEF_URGENT | CLEF_UNICODE;
	cle.hContact = hContact;
	cle.hDbEvent = 0;
	wchar_t toolTip[255];
	mir_snwprintf(toolTip, bNews ? TranslateT("New news") : TranslateT("New notifications"));
	cle.szTooltip.w = toolTip;
	g_clistApi.pfnAddEvent(&cle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVkUserInfo* CVkProto::GetVkUserInfo(LONG iUserId, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkUserInfo %d", iUserId);

	bool bIsGroup = (iUserId < 0);
	CVkUserInfo *vkUser = vkUsers.find((CVkUserInfo *)&iUserId);

	if (vkUser == nullptr) {
		CMStringW wszNick = TranslateT("Unknown");
		CMStringW wszLink(L"https://vk.com/");
		if (iUserId) {
			wszLink += bIsGroup ? "club" : "id";
			wszLink.AppendFormat(L"%d", bIsGroup ? -iUserId : iUserId);
		}
		vkUser = new CVkUserInfo(iUserId, bIsGroup, wszNick, wszLink, bIsGroup ? 0 : FindUser(iUserId));
		vkUsers.insert(vkUser);
	}

	return vkUser;
}

void CVkProto::CreateVkUserInfoList(OBJLIST<CVkUserInfo> &vkUsers, const JSONNode &jnResponse)
{
	debugLogA("CVkProto::CreateVkUserInfoList");
	if (!jnResponse)
		return;

	const JSONNode &jnProfiles = jnResponse["profiles"];

	if (jnProfiles)
		for (auto  &jnProfile : jnProfiles) {
			if (!jnProfile["id"])
				continue;
			LONG UserId = jnProfile["id"].as_int();

			CMStringW wszNick(jnProfile["first_name"].as_mstring());
			wszNick.AppendChar(' ');
			wszNick += jnProfile["last_name"].as_mstring();
			CMStringW wszLink = L"https://vk.com/";
			CMStringW wszScreenName(jnProfile["screen_name"].as_mstring());
			if (wszScreenName.IsEmpty())
				wszScreenName.AppendFormat(L"id%d", UserId);
			wszLink += wszScreenName;
			CVkUserInfo *vkUser = new CVkUserInfo(UserId, false, wszNick, wszLink, FindUser(UserId));
			vkUsers.insert(vkUser);
		}

	const JSONNode &jnGroups = jnResponse["groups"];
	if (jnGroups)
		for (auto &jnProfile : jnGroups) {
			if (!jnProfile["id"])
				continue;
			LONG UserId = -jnProfile["id"].as_int();

			CMStringW wszNick(jnProfile["name"].as_mstring());
			CMStringW wszLink = L"https://vk.com/";
			wszLink += jnProfile["screen_name"].as_mstring();
			CVkUserInfo *vkUser = new CVkUserInfo(UserId, true, wszNick, wszLink);
			vkUsers.insert(vkUser);
		}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVKNewsItem* CVkProto::GetVkNewsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers, bool isRepost)
{
	if (!jnItem || jnItem["type"].as_mstring() == L"friends_recomm")
		return nullptr;

	bool bPostLink = true;
	CVKNewsItem *vkNewsItem = new CVKNewsItem();

	LONG iSourceId = !jnItem["source_id"] ? jnItem["owner_id"].as_int() : jnItem["source_id"].as_int();
	LONG iPostId = jnItem["post_id"].as_int();
	CMStringW wszText(jnItem["text"].as_mstring());
	CMStringW wszPopupText(wszText);

	vkNewsItem->wszType = jnItem["type"].as_mstring();
	vkNewsItem->vkUser = GetVkUserInfo(iSourceId, vkUsers);
	vkNewsItem->bIsGroup = vkNewsItem->vkUser->m_bIsGroup;
	vkNewsItem->tDate = jnItem["date"].as_int();

	if (!wszText.IsEmpty())
		wszText += L"\n";

	debugLogW(L"CVkProto::GetVkNewsItem %d %d %s", iSourceId, iPostId, vkNewsItem->wszType.c_str());

	if (vkNewsItem->wszType == L"photo_tag") {
		bPostLink = false;
		const JSONNode &jnPhotos = jnItem["photo_tags"];
		if (jnPhotos) {
			const JSONNode &jnPhotoItems = jnPhotos["items"];
			if (jnPhotoItems) {
				wszText = TranslateT("User was tagged in these photos:");
				wszPopupText = wszText + TranslateT("(photos)");
				for (auto &it : jnPhotoItems)
					wszText += L"\n" + GetVkPhotoItem(it, m_vkOptions.BBCForNews());
			}
		}
	}
	else if (vkNewsItem->wszType == L"photo" || vkNewsItem->wszType == L"wall_photo") {
		bPostLink = false;
		const JSONNode &jnPhotos = jnItem["photos"];
		int i = 0;
		if (jnPhotos) {
			const JSONNode &jnPhotoItems = jnPhotos["items"];
			if (jnPhotoItems) {
				wszPopupText += TranslateT("(photos)");
				for (auto &jnPhotoItem : jnPhotoItems) {
					wszText += GetVkPhotoItem(jnPhotoItem, m_vkOptions.BBCForNews()) + L"\n";
					if (i == 0 && vkNewsItem->wszType == L"wall_photo") {
						if (jnPhotoItem["post_id"]) {
							bPostLink = true;
							iPostId = jnPhotoItem["post_id"].as_int();
							break; // max 1 wall_photo when photo post_id !=0
						}
					}
					i++;
				}
			}
		}
	}
	else if (vkNewsItem->wszType == L"post" || vkNewsItem->wszType.IsEmpty()) {
		bPostLink = true;
		const JSONNode &jnRepost = jnItem["copy_history"];
		if (jnRepost) {
			CVKNewsItem *vkRepost = GetVkNewsItem((*jnRepost.begin()), vkUsers, true);
			if (vkRepost) {
				vkRepost->wszText.Replace(L"\n", L"\n\t");
				wszText += vkRepost->wszText;
				wszText += L"\n";

				wszPopupText += L"\t";
				wszPopupText += vkRepost->wszPopupTitle;
				wszPopupText += L"\n\t";
				wszPopupText += vkRepost->wszPopupText;
				vkNewsItem->bIsRepost = true;
				delete vkRepost;
			}
		}

		const JSONNode &jnAttachments = jnItem["attachments"];
		if (jnAttachments) {
			if (!wszText.IsEmpty())
				wszText.AppendChar('\n');
			if (!wszPopupText.IsEmpty())
				wszPopupText.AppendChar('\n');
			wszPopupText += TranslateT("(attachments)");
			wszText += GetAttachmentDescr(jnAttachments, m_vkOptions.bUseBBCOnAttacmentsAsNews ? m_vkOptions.BBCForNews() : m_vkOptions.BBCForAttachments());
		}
	}

	CMStringW wszResFormat, wszTitleFormat;

	if (!isRepost) {
		wszResFormat = Translate("News from %s\n%s");
		wszTitleFormat = Translate("News from %s");
	}
	else {
		wszResFormat = Translate("\tRepost from %s\n%s");
		wszTitleFormat = Translate("Repost from %s");
		bPostLink = false;
	}

	vkNewsItem->wszText.AppendFormat(wszResFormat,
		SetBBCString(vkNewsItem->vkUser->m_wszUserNick, m_vkOptions.BBCForNews(), vkbbcUrl,
			vkNewsItem->vkUser->m_wszLink).c_str(), wszText.c_str());
	vkNewsItem->wszPopupTitle.AppendFormat(wszTitleFormat, vkNewsItem->vkUser->m_wszUserNick.c_str());
	vkNewsItem->wszPopupText = wszPopupText;

	vkNewsItem->wszId.AppendFormat(L"%d_%d", vkNewsItem->vkUser->m_UserId, iPostId);
	if (bPostLink) {
		vkNewsItem->wszLink = CMStringW(L"https://vk.com/wall") + vkNewsItem->wszId;
		vkNewsItem->wszText.AppendChar('\n');
		vkNewsItem->wszText += SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNewsItem->wszLink);
	}

	debugLogW(L"CVkProto::GetVkNewsItem %d %d <\n%s\n>", iSourceId, iPostId, vkNewsItem->wszText.c_str());

	return vkNewsItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CMStringW CVkProto::GetVkFeedback(const JSONNode &jnFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser)
{
	debugLogA("CVkProto::GetVkFeedback");
	CMStringW wszRes;
	if (!jnFeedback || !vkFeedbackType)
		return wszRes;

	CMStringW wszFormat;
	LONG iUserId = 0;

	if (vkFeedbackType == vkComment) {
		iUserId = jnFeedback["from_id"].as_int();
		wszFormat = L"%s %%s %%s\n%s";
	}
	else if (vkFeedbackType == vkPost) {
		iUserId = jnFeedback["owner_id"].as_int();
		wszFormat = L"%s %%s %%s\n%s";
	}
	else if (vkFeedbackType == VKObjType::vkUsers || vkFeedbackType == vkCopy) {
		const JSONNode &jnUsers = jnFeedback["items"];

		CMStringW wszUsers;
		for (auto &jnUserItem : jnUsers) {
			if (!jnUserItem["from_id"])
				continue;
			iUserId = jnUserItem["from_id"].as_int();
			vkUser = GetVkUserInfo(iUserId, vkUsers);
			if (!wszUsers.IsEmpty())
				wszUsers += L", ";
			wszUsers += SetBBCString(vkUser->m_wszUserNick, m_vkOptions.BBCForNews(), vkbbcUrl, vkUser->m_wszLink);
		}
		wszRes.AppendFormat(L"%s %%s %%s", wszUsers.c_str());
		vkUser = nullptr;
		iUserId = 0;
	}

	if (iUserId) {
		vkUser = GetVkUserInfo(iUserId, vkUsers);
		CMStringW wszText(jnFeedback["text"].as_mstring());
		wszText.Replace(L"%", L"%%");
		wszRes.AppendFormat(wszFormat, SetBBCString(vkUser->m_wszUserNick, m_vkOptions.BBCForNews(), vkbbcUrl, vkUser->m_wszLink).c_str(), ClearFormatNick(wszText).c_str());
	}

	return wszRes;
}

CVKNewsItem* CVkProto::GetVkParent(const JSONNode &jnParent, VKObjType vkParentType, LPCWSTR pwszReplyText, LPCWSTR pwszReplyLink)
{
	debugLogA("CVkProto::GetVkParent");
	CMStringW wszRes;
	if (!jnParent || !vkParentType)
		return nullptr;

	CVKNewsItem *vkNotificationItem = new CVKNewsItem();

	if (vkParentType == vkPhoto) {
		CMStringW wszPhoto = GetVkPhotoItem(jnParent, m_vkOptions.BBCForNews());
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/photo%s", vkNotificationItem->wszId.c_str());
		vkNotificationItem->wszText.AppendFormat(L"\n%s", wszPhoto.c_str());

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink).c_str());
	}
	else if (vkParentType == vkVideo) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMStringW wszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/video%s", vkNotificationItem->wszId.c_str());

		CMStringW wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty())
			vkNotificationItem->wszText.AppendFormat(L"\n%s: %s", SetBBCString(TranslateT("Video description:"), m_vkOptions.BBCForNews(), vkbbcB).c_str(), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI).c_str());

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(wszTitle, m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink).c_str());
	}
	else if (vkParentType == vkPost) {
		LONG iToId = jnParent["to_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iToId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/wall%s%s", vkNotificationItem->wszId.c_str(), pwszReplyLink ? pwszReplyLink : L"");

		CMStringW wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty()) {
			vkNotificationItem->wszText.AppendFormat(L"\n%s: %s", SetBBCString(TranslateT("Post text:"), m_vkOptions.BBCForNews(), vkbbcB).c_str(), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			vkNotificationItem->wszPopupText.AppendFormat(L"%s: %s", TranslateT("Post text:"), wszText.c_str());
		}

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			if (!vkNotificationItem->wszPopupText.IsEmpty())
				vkNotificationItem->wszPopupText += L"\n";
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink).c_str());
	}
	else if (vkParentType == vkTopic) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMStringW wszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/topic%s%s",
			vkNotificationItem->wszId.c_str(), pwszReplyLink ? pwszReplyLink : L"");

		CMStringW wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty()) {
			vkNotificationItem->wszText.AppendFormat(L"\n%s %s", SetBBCString(TranslateT("Topic text:"), m_vkOptions.BBCForNews(), vkbbcB).c_str(), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			vkNotificationItem->wszPopupText.AppendFormat(L"%s %s", TranslateT("Topic text:"), wszText.c_str());
		}

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI).c_str());
			if (!vkNotificationItem->wszPopupText.IsEmpty())
				vkNotificationItem->wszPopupText += L"\n";
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(wszTitle, m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink).c_str());
	}
	else if (vkParentType == vkComment) {
		CMStringW wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		const JSONNode &jnPhoto = jnParent["photo"];
		if (jnPhoto) {
			delete vkNotificationItem;
			return GetVkParent(jnPhoto, vkPhoto, wszText);
		}

		const JSONNode &jnVideo = jnParent["video"];
		if (jnVideo) {
			delete vkNotificationItem;
			return GetVkParent(jnVideo, vkVideo, wszText);
		}

		LONG iId = jnParent["id"].as_int();

		const JSONNode &jnPost = jnParent["post"];
		if (jnPost) {
			CMStringW wszRepl(FORMAT, L"?reply=%d", iId);
			delete vkNotificationItem;
			return GetVkParent(jnPost, vkPost, wszText, wszRepl);
		}

		const JSONNode &jnTopic = jnParent["topic"];
		if (jnTopic) {
			CMStringW wszRepl(FORMAT, L"?reply=%d", iId);
			delete vkNotificationItem;
			return GetVkParent(jnTopic, vkTopic, wszText, wszRepl);
		}
	}

	return vkNotificationItem;
}

CVKNewsItem* CVkProto::GetVkNotificationsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkNotificationsItem");
	if (!jnItem)
		return nullptr;

	CMStringW wszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMStringW wszNotificationTranslate = SpanVKNotificationType(wszType, vkFeedbackType, vkParentType);

	const JSONNode &jnFeedback = jnItem["feedback"];
	const JSONNode &jnParent = jnItem["parent"];

	if (m_vkOptions.bNotificationFilterAcceptedFriends && wszType == L"friend_accepted" && jnFeedback && vkFeedbackType == VKObjType::vkUsers) {
		OnFriendAccepted(jnFeedback);
		return nullptr;
	}

	if (!jnFeedback || !jnParent)
		return nullptr;

	CVkUserInfo *vkUser = nullptr;
	CMStringW wszFeedback = GetVkFeedback(jnFeedback, vkFeedbackType, vkUsers, vkUser);
	CVKNewsItem *vkNotification = GetVkParent(jnParent, vkParentType);

	if (!vkNotification)
		return nullptr;

	if (!wszFeedback.IsEmpty()) {
		CMStringW wszNotificaton;
		wszNotificaton.AppendFormat(wszFeedback, wszNotificationTranslate.c_str(), vkNotification->wszText.c_str());
		vkNotification->wszText = wszNotificaton;

		wszFeedback = RemoveBBC(wszFeedback);
		int idx = wszFeedback.Find(L" %s %s");

		vkNotification->wszPopupTitle.AppendFormat(L"%s %s", wszFeedback.Mid(0, idx).c_str(), wszNotificationTranslate.c_str());
		if (wszFeedback.GetLength() > idx + 7) {
			if (!vkNotification->wszPopupText.IsEmpty())
				vkNotification->wszPopupText += L"\n>> ";
			vkNotification->wszPopupText += wszFeedback.Mid(idx + 7, wszFeedback.GetLength() - idx - 7);
		}

		vkNotification->wszType = wszType;
		vkNotification->tDate = jnItem["date"].as_int();
		vkNotification->vkFeedbackType = vkFeedbackType;
		vkNotification->vkParentType = vkParentType;
		vkNotification->vkUser = vkUser;
		return vkNotification;
	}

	delete vkNotification;
	return nullptr;
}

void CVkProto::OnFriendAccepted(const JSONNode & jnFeedback)
{
	const JSONNode &jnUsers = jnFeedback["items"];

	for (auto &jnUserItem : jnUsers) {
		if (!jnUserItem["from_id"])
			continue;

		LONG iUserId = jnUserItem["from_id"].as_int();
		MCONTACT hContact = FindUser(iUserId, true);

		RetrieveUserInfo(iUserId);
		CVkDBAddAuthRequestThreadParam *param = new CVkDBAddAuthRequestThreadParam(hContact, true);
		ForkThread(&CVkProto::DBAddAuthRequestThread, (void *)param);
	}
}

CVKNewsItem* CVkProto::GetVkGroupInvates(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkGroupInvates");
	if (!jnItem)
		return nullptr;

	CMStringW wszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMStringW wszNotificationTranslate = SpanVKNotificationType(wszType, vkFeedbackType, vkParentType);

	if (!jnItem["id"])
		return nullptr;

	LONG iGroupId = jnItem["id"].as_int();
	CMStringW wszId(FORMAT, L"%d,", iGroupId);
	CMStringW wszIds(ptrW(db_get_wsa(0, m_szModuleName, "InviteGroupIds")));

	if (wszIds.Find(wszId, 0) != -1)
		return nullptr;

	LONG iUserId = !jnItem["invited_by"] ? 0 : jnItem["invited_by"].as_int();
	CVKNewsItem *vkNotification = new CVKNewsItem();
	vkNotification->tDate = time(0);
	vkNotification->vkUser = GetVkUserInfo(iUserId, vkUsers);
	vkNotification->wszType = wszType;
	vkNotification->wszId = wszId;
	vkNotification->vkFeedbackType = vkFeedbackType;
	vkNotification->vkParentType = vkParentType;

	CMStringW wszGroupName;
	CMStringW wszGName = jnItem["name"].as_mstring();
	CMStringW wszGLink(FORMAT, L"https://vk.com/%s", jnItem["screen_name"].as_mstring().c_str());
	wszGroupName = SetBBCString(wszGName, m_vkOptions.BBCForNews(), vkbbcUrl, wszGLink);

	CMStringW wszUsers = SetBBCString(iUserId ? vkNotification->vkUser->m_wszUserNick : TranslateT("Unknown"), m_vkOptions.BBCForNews(),
		vkbbcUrl, iUserId ? vkNotification->vkUser->m_wszLink : L"https://vk.com/");

	vkNotification->wszText.AppendFormat(L"%s %s %s", wszUsers.c_str(), wszNotificationTranslate.c_str(), wszGroupName.c_str());
	vkNotification->wszPopupTitle.AppendFormat(L"%s %s %s", iUserId ? vkNotification->vkUser->m_wszUserNick.c_str() : TranslateT("Unknown"),
		wszNotificationTranslate.c_str(), wszGName.c_str());

	wszIds += wszId;
	setWString("InviteGroupIds", wszIds);

	return vkNotification;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUnreadNews(time_t tLastNewsTime)
{
	debugLogA("CVkProto::RetrieveUnreadNews");
	if (!IsOnline())
		return;

	time_t tLastNewsReqTime = getDword("LastNewsReqTime", time(0) - 24 * 60 * 60);
	if (time(0) - tLastNewsReqTime < 3 * 60)
		return;

	CMStringA szFilter;
	szFilter = m_vkOptions.bNewsFilterPosts ? "post" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_vkOptions.bNewsFilterPhotos ? "photo" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_vkOptions.bNewsFilterTags ? "photo_tag" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_vkOptions.bNewsFilterWallPhotos ? "wall_photo" : "";

	if (szFilter.IsEmpty()) {
		debugLogA("CVkProto::RetrieveUnreadNews szFilter empty");
		return;
	}

	CMStringA szSource;
	szSource = m_vkOptions.bNewsSourceFriends ? "friends" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_vkOptions.bNewsSourceGroups ? "groups" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_vkOptions.bNewsSourcePages ? "pages" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_vkOptions.bNewsSourceFollowing ? "following" : "";

	if (szSource.IsEmpty()) {
		debugLogA("CVkProto::RetrieveUnreadNews szSource empty");
		return;
	}

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/newsfeed.get.json", true, &CVkProto::OnReceiveUnreadNews)
		<< INT_PARAM("count", 100)
		<< INT_PARAM("return_banned", m_vkOptions.bNewsSourceIncludeBanned ? 1 : 0)
		<< INT_PARAM("max_photos", m_vkOptions.iMaxLoadNewsPhoto)
		<< INT_PARAM("start_time", tLastNewsTime + 1)
		<< CHAR_PARAM("filters", szFilter)
		<< CHAR_PARAM("source_ids", szSource));

	setDword("LastNewsReqTime", (uint32_t)time(0));
}

static int sttCompareVKNewsItems(const CVKNewsItem *p1, const CVKNewsItem *p2)
{
	int compareId = p1->wszId.Compare(p2->wszId);
	LONG compareUserId = p1->vkUser->m_UserId - p2->vkUser->m_UserId;
	LONG compareDate = (LONG)p1->tDate - (LONG)p2->tDate;

	return compareId ? (compareDate ? compareDate : compareUserId) : 0;
}

static int sttCompareVKNotificationItems(const CVKNewsItem *p1, const CVKNewsItem *p2)
{
	int compareType = p1->wszType.Compare(p2->wszType);
	int compareId = p1->wszId.Compare(p2->wszId);
	LONG compareDate = (LONG)p1->tDate - (LONG)p2->tDate;

	return compareType ? compareDate : (compareId ? compareDate : 0);
}

void CVkProto::OnReceiveUnreadNews(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNews %d", reply->resultCode);
	db_unset(0, m_szModuleName, "LastNewsReqTime");
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	OBJLIST<CVkUserInfo> vkUsers(5, NumericKeySortT);
	CreateVkUserInfoList(vkUsers, jnResponse);

	const JSONNode &jnItems = jnResponse["items"];

	OBJLIST<CVKNewsItem> vkNews(5, sttCompareVKNewsItems);
	if (jnItems)
		for (auto &it : jnItems) {
			CVKNewsItem *vkNewsItem = GetVkNewsItem(it, vkUsers);
			if (!vkNewsItem)
				continue;
			CVKNewsItem *vkNewsFoundItem = vkNews.find(vkNewsItem);
			if (vkNewsFoundItem == nullptr)
				vkNews.insert(vkNewsItem);
			else if (vkNewsFoundItem->wszType == L"wall_photo" && vkNewsItem->wszType == L"post") {
				vkNews.remove(vkNewsFoundItem);
				vkNews.insert(vkNewsItem);
			}
			else
				delete vkNewsItem;
		}

	bool bNewsAdded = false;
	for (auto &it : vkNews)
		if (!(m_vkOptions.bNewsSourceNoReposts && it->bIsRepost)) {
			AddFeedEvent(*it);
			bNewsAdded = true;
		}

	if (bNewsAdded)
		AddCListEvent(true);

	setDword("LastNewsTime", time(0));

	vkNews.destroy();
	vkUsers.destroy();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUnreadNotifications(time_t tLastNotificationsTime)
{
	debugLogA("CVkProto::RetrieveUnreadNotifications");
	if (!IsOnline())
		return;

	time_t tLastNotificationsReqTime = getDword("LastNotificationsReqTime", time(0) - 24 * 60 * 60);
	if (time(0) - tLastNotificationsReqTime < 3 * 60)
		return;

	CMStringW code(FORMAT, L"return{\"notifications\":API.notifications.get({\"count\": 100, \"start_time\":%d})%s",
		(LONG)(tLastNotificationsTime + 1),
		m_vkOptions.bNotificationFilterInvites ? L",\"groupinvates\":API.groups.getInvites({\"extended\":1})};" : L"};");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveUnreadNotifications)
		<< WCHAR_PARAM("code", code)
	);

	setDword("LastNotificationsReqTime", (uint32_t)time(0));
}

bool CVkProto::FilterNotification(CVKNewsItem* vkNotificationItem, bool& isCommented)
{
	isCommented = false;
	if (vkNotificationItem->vkParentType == vkNull)
		return false;

	if (vkNotificationItem->wszType == L"mention_comments"
		|| vkNotificationItem->wszType == L"mention_comment_photo"
		|| vkNotificationItem->wszType == L"mention_comment_video") {
		isCommented = true;
		return (m_vkOptions.bNotificationFilterMentions != 0);
	}

	bool result = (vkNotificationItem->vkFeedbackType == vkUsers && m_vkOptions.bNotificationFilterLikes);
	result = (vkNotificationItem->vkFeedbackType == vkCopy && m_vkOptions.bNotificationFilterReposts) || result;
	result = (vkNotificationItem->vkFeedbackType == vkComment && m_vkOptions.bNotificationFilterComments) || result;
	result = (vkNotificationItem->vkParentType == vkInvite && m_vkOptions.bNotificationFilterInvites) || result;

	isCommented = (vkNotificationItem->vkFeedbackType == vkComment);

	return result;
}

void CVkProto::NotificationMarkAsViewed()
{
	debugLogA("CVkProto::NotificationMarkAsViewed");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/notifications.markAsViewed.json", true, &CVkProto::OnReceiveSmth));
}

void CVkProto::OnReceiveUnreadNotifications(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNotifications %d", reply->resultCode);
	db_unset(0, m_szModuleName, "LastNotificationsReqTime");
	if (reply->resultCode != 200)
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	const JSONNode &jnNotifications = jnResponse["notifications"];
	const JSONNode &jnGroupInvates = jnResponse["groupinvates"];

	OBJLIST<CVkUserInfo> vkUsers(5, NumericKeySortT);
	OBJLIST<CVKNewsItem> vkNotification(5, sttCompareVKNotificationItems);

	CreateVkUserInfoList(vkUsers, jnNotifications);
	CreateVkUserInfoList(vkUsers, jnGroupInvates);

	if (jnNotifications) {
		const JSONNode &jnItems = jnNotifications["items"];

		if (jnItems)
			for (auto &it : jnItems) {
				CVKNewsItem *vkNotificationItem = GetVkNotificationsItem(it, vkUsers);
				if (!vkNotificationItem)
					continue;
				if (vkNotification.find(vkNotificationItem) == nullptr)
					vkNotification.insert(vkNotificationItem);
				else
					delete vkNotificationItem;
			}
	}

	if (jnGroupInvates) {
		const JSONNode &jnItems = jnGroupInvates["items"];

		if (jnItems)
			for (auto &it : jnItems) {
				CVKNewsItem *vkNotificationItem = GetVkGroupInvates(it, vkUsers);
				if (!vkNotificationItem)
					continue;
				if (vkNotification.find(vkNotificationItem) == nullptr)
					vkNotification.insert(vkNotificationItem);
				else
					delete vkNotificationItem;
			}
	}

	bool bNotificationCommentAdded = false;
	bool bNotificationComment = false;
	bool bNotificationAdded = false;
	for (auto &it : vkNotification)
		if (FilterNotification(it, bNotificationComment)) {
			AddFeedEvent(*it);
			bNotificationAdded = true;
			bNotificationCommentAdded = bNotificationComment || bNotificationCommentAdded;
		}

	if (bNotificationAdded)
		AddCListEvent(false);

	setDword("LastNotificationsTime", time(0));
	if (m_vkOptions.bNotificationsMarkAsViewed && bNotificationCommentAdded)
		NotificationMarkAsViewed();

	vkNotification.destroy();
	vkUsers.destroy();
}

void CVkProto::RetrieveUnreadEvents()
{
	debugLogA("CVkProto::RetrieveUnreadEvents");
	if (!IsOnline() || (!m_vkOptions.bNotificationsEnabled && !m_vkOptions.bNewsEnabled))
		return;

	time_t tLastNotificationsTime = getDword("LastNotificationsTime", time(0) - 24 * 60 * 60);
	if (time(0) - tLastNotificationsTime - m_vkOptions.iNotificationsInterval * 60 >= -3 && m_vkOptions.bNotificationsEnabled)
		RetrieveUnreadNotifications(tLastNotificationsTime);

	time_t tLastNewsTime = getDword("LastNewsTime", time(0) - 24 * 60 * 60);
	if (time(0) - tLastNewsTime - m_vkOptions.iNewsInterval * 60 >= -3 && m_vkOptions.bNewsEnabled)
		RetrieveUnreadNews(tLastNewsTime);

	NewsClearHistory();
}

INT_PTR CVkProto::SvcLoadVKNews(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcLoadVKNews");
	if (!IsOnline())
		return 1;

	if (!m_vkOptions.bNewsEnabled && !m_vkOptions.bNotificationsEnabled) {
		m_vkOptions.bSpecialContactAlwaysEnabled = true;
		AddFeedSpecialUser();
	}

	time_t tLastNewsTime = getDword("LastNewsTime", time(0) - 24 * 60 * 60);
	RetrieveUnreadNews(tLastNewsTime);

	return 0;
}

void CVkProto::NewsClearHistory()
{
	debugLogA("CVkProto::NewsClearHistory");
	MCONTACT hContact = FindUser(VK_FEED_USER);
	if (hContact == 0 || !m_vkOptions.bNewsAutoClearHistory)
		return;

	time_t tTime = time(0) - m_vkOptions.iNewsAutoClearHistoryInterval;

	DB::ECPTR pCursor(DB::Events(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.timestamp < tTime)
			pCursor.DeleteEvent();
	}
}