/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

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
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact);
		return;
	}

	if (!hContact) {
		hContact = FindUser(VK_FEED_USER, true);

		setWString(hContact, "Nick", TranslateT("VKontakte"));
		CMString wszUrl = L"https://vk.com/press/Simple.png";
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

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = vkNewsItem.tDate;
	recv.szMessage = pszBody;
	recv.lParam = 0;
	recv.pCustomData = NULL;
	recv.cbCustomDataSize = 0;

	if (m_vkOptions.bUseNonStandardNotifications) {
		recv.flags = PREF_CREATEREAD;
		MsgPopup(hContact, vkNewsItem.wszPopupText, vkNewsItem.wszPopupTitle);
	}

	ProtoChainRecvMsg(hContact, &recv);
}

void CVkProto::AddCListEvent(bool bNews)
{
	SkinPlaySound("VKNewsFeed");

	if (!m_vkOptions.bUseNonStandardNotifications)
		return;

	MCONTACT hContact = FindUser(VK_FEED_USER, true);

	CLISTEVENT cle = {};
	cle.hIcon = IcoLib_GetIconByHandle(GetIconHandle(IDI_NOTIFICATION));
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_URGENT | CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = NULL;
	wchar_t toolTip[255];
	mir_snwprintf(toolTip, bNews ? TranslateT("New news") : TranslateT("New notifications"));
	cle.ptszTooltip = toolTip;
	pcli->pfnAddEvent(&cle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVkUserInfo* CVkProto::GetVkUserInfo(LONG iUserId, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkUserInfo %d", iUserId);
	if (iUserId == 0)
		return NULL;

	bool bIsGroup = (iUserId < 0);
	CVkUserInfo *vkUser = vkUsers.find((CVkUserInfo *)&iUserId);

	if (vkUser == NULL) {
		CMString wszNick = TranslateT("Unknown");
		CMString wszLink(L"https://vk.com/");
		if (iUserId) {
			wszLink += bIsGroup ? "club" : "id";
			wszLink.AppendFormat(L"%d", bIsGroup ? -iUserId : iUserId);
		}
		vkUser = new CVkUserInfo(iUserId, bIsGroup, wszNick, wszLink, bIsGroup ? NULL : FindUser(iUserId));
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
		for (auto it = jnProfiles.begin(); it != jnProfiles.end(); ++it) {
			const JSONNode &jnProfile = (*it);
			if (!jnProfile["id"])
				continue;
			LONG UserId =  jnProfile["id"].as_int();

			CMString wszNick(jnProfile["first_name"].as_mstring());
			wszNick.AppendChar(' ');
			wszNick += jnProfile["last_name"].as_mstring();
			CMString wszLink = L"https://vk.com/";
			CMString wszScreenName(jnProfile["screen_name"].as_mstring());
			if (wszScreenName.IsEmpty())
				wszScreenName.AppendFormat(L"id%d", UserId);
			wszLink += wszScreenName;
			CVkUserInfo *vkUser = new CVkUserInfo(UserId, false, wszNick, wszLink, FindUser(UserId));
			vkUsers.insert(vkUser);
		}

	const JSONNode &jnGroups = jnResponse["groups"];
	if (jnGroups)		
		for (auto it = jnGroups.begin(); it != jnGroups.end(); ++it) {
			const JSONNode &jnProfile = (*it);
			if (!jnProfile["id"])
				continue;
			LONG UserId = - jnProfile["id"].as_int();

			CMString wszNick(jnProfile["name"].as_mstring());
			CMString wszLink = L"https://vk.com/";
			wszLink += jnProfile["screen_name"].as_mstring();
			CVkUserInfo *vkUser = new CVkUserInfo(UserId, true, wszNick, wszLink);
			vkUsers.insert(vkUser);
		}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVKNewsItem* CVkProto::GetVkNewsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers, bool isRepost)
{
	bool bPostLink = true;
	CVKNewsItem *vkNewsItem = new CVKNewsItem();
	if (!jnItem)
		return vkNewsItem;

	LONG iSourceId = !jnItem["source_id"] ? jnItem["owner_id"].as_int() : jnItem["source_id"].as_int();
	LONG iPostId = jnItem["post_id"].as_int();
	CMString wszText(jnItem["text"].as_mstring());
	CMString wszPopupText(wszText);

	vkNewsItem->wszType = jnItem["type"].as_mstring();
	vkNewsItem->vkUser = GetVkUserInfo(iSourceId, vkUsers);
	vkNewsItem->bIsGroup = vkNewsItem->vkUser->m_bIsGroup;
	vkNewsItem->tDate = jnItem["date"].as_int();

	if (!wszText.IsEmpty())
		wszText += L"\n";

	debugLog(L"CVkProto::GetVkNewsItem %d %d %s", iSourceId, iPostId, vkNewsItem->wszType);

	if (vkNewsItem->wszType == L"photo_tag") {
		bPostLink = false;
		const JSONNode &jnPhotos = jnItem["photo_tags"];
		if (jnPhotos) {			
			const JSONNode &jnPhotoItems = jnPhotos["items"];
			if (jnPhotoItems) {				
				wszText = TranslateT("User was tagged in these photos:");
				wszPopupText = wszText + TranslateT("(photos)");
				for (auto it = jnPhotoItems.begin(); it != jnPhotoItems.end(); ++it)
					wszText += L"\n" + GetVkPhotoItem((*it), m_vkOptions.BBCForNews());
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
				for (auto it = jnPhotoItems.begin(); it != jnPhotoItems.end(); ++it) {
					const JSONNode &jnPhotoItem = (*it);
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

	CMString wszResFormat, wszTitleFormat;

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
		vkNewsItem->vkUser->m_wszLink), wszText);
	vkNewsItem->wszPopupTitle.AppendFormat(wszTitleFormat, vkNewsItem->vkUser->m_wszUserNick);
	vkNewsItem->wszPopupText = wszPopupText;

	vkNewsItem->wszId.AppendFormat(L"%d_%d", vkNewsItem->vkUser->m_UserId, iPostId);
	if (bPostLink) {
		vkNewsItem->wszLink = CMString(L"https://vk.com/wall") + vkNewsItem->wszId;
		vkNewsItem->wszText.AppendChar('\n');
		vkNewsItem->wszText += SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNewsItem->wszLink);
	}

	debugLog(L"CVkProto::GetVkNewsItem %d %d <\n%s\n>", iSourceId, iPostId, vkNewsItem->wszText);

	return vkNewsItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CMString CVkProto::GetVkFeedback(const JSONNode &jnFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser)
{
	debugLogA("CVkProto::GetVkFeedback");
	CMString wszRes;
	if (!jnFeedback || !vkFeedbackType)
		return wszRes;

	CMString wszFormat;
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
		
		CMString wszUsers;
		for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
			const JSONNode &jnUserItem = (*it);
			if (!jnUserItem["from_id"])
				continue;
			iUserId = jnUserItem["from_id"].as_int();
			vkUser = GetVkUserInfo(iUserId, vkUsers);
			if (!wszUsers.IsEmpty())
				wszUsers += L", ";
			wszUsers += SetBBCString(vkUser->m_wszUserNick, m_vkOptions.BBCForNews(), vkbbcUrl, vkUser->m_wszLink);
		}
		wszRes.AppendFormat(L"%s %%s %%s", wszUsers);
		vkUser = NULL;
		iUserId = 0;
	}

	if (iUserId) {
		vkUser = GetVkUserInfo(iUserId, vkUsers);
		CMString wszText(jnFeedback["text"].as_mstring());
		wszText.Replace(L"%", L"%%");
		wszRes.AppendFormat(wszFormat, SetBBCString(vkUser->m_wszUserNick, m_vkOptions.BBCForNews(), vkbbcUrl, vkUser->m_wszLink), ClearFormatNick(wszText));
	}

	return wszRes;
}

CVKNewsItem* CVkProto::GetVkParent(const JSONNode &jnParent, VKObjType vkParentType, LPCWSTR pwszReplyText, LPCWSTR pwszReplyLink)
{
	debugLogA("CVkProto::GetVkParent");
	CMString wszRes;
	if (!jnParent || !vkParentType)
		return NULL;

	CVKNewsItem *vkNotificationItem = new CVKNewsItem();

	if (vkParentType == vkPhoto) {
		CMString wszPhoto = GetVkPhotoItem(jnParent, m_vkOptions.BBCForNews());
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/photo%s", vkNotificationItem->wszId);
		vkNotificationItem->wszText.AppendFormat(L"\n%s", wszPhoto);

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI));
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink));
	}
	else if (vkParentType == vkVideo) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMString wszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/video%s", vkNotificationItem->wszId);

		CMString wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty())
			vkNotificationItem->wszText.AppendFormat(L"\n%s: %s", SetBBCString(TranslateT("Video description:"), m_vkOptions.BBCForNews(), vkbbcB), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI));

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI));
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(wszTitle, m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink));
	}
	else if (vkParentType == vkPost) {
		LONG iToId = jnParent["to_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iToId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/wall%s%s", vkNotificationItem->wszId, pwszReplyLink ? pwszReplyLink : L"");

		CMString wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty()) {
			vkNotificationItem->wszText.AppendFormat(L"\n%s: %s", SetBBCString(TranslateT("Post text:"), m_vkOptions.BBCForNews(), vkbbcB), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI));
			vkNotificationItem->wszPopupText.AppendFormat(L"%s: %s", TranslateT("Post text:"), wszText);
		}

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI));
			if (!vkNotificationItem->wszPopupText.IsEmpty())
				vkNotificationItem->wszPopupText += L"\n";
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(TranslateT("Link"), m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink));
	}
	else if (vkParentType == vkTopic) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMString wszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->wszId.AppendFormat(L"%d_%d", iOwnerId, iId);
		vkNotificationItem->wszLink.AppendFormat(L"https://vk.com/topic%s%s", 
			vkNotificationItem->wszId, pwszReplyLink ? pwszReplyLink : L"");

		CMString wszText(jnParent["text"].as_mstring());
		ClearFormatNick(wszText);

		if (!wszText.IsEmpty()) {
			vkNotificationItem->wszText.AppendFormat(L"\n%s %s", SetBBCString(TranslateT("Topic text:"), m_vkOptions.BBCForNews(), vkbbcB), SetBBCString(wszText, m_vkOptions.BBCForNews(), vkbbcI));
			vkNotificationItem->wszPopupText.AppendFormat(L"%s %s", TranslateT("Topic text:"), wszText);
		}

		if (pwszReplyText) {
			vkNotificationItem->wszText.AppendFormat(L"\n>> %s", SetBBCString(pwszReplyText, m_vkOptions.BBCForNews(), vkbbcI));
			if (!vkNotificationItem->wszPopupText.IsEmpty())
				vkNotificationItem->wszPopupText += L"\n";
			vkNotificationItem->wszPopupText.AppendFormat(L">> %s", pwszReplyText);
		}

		vkNotificationItem->wszText.AppendFormat(L"\n%s", SetBBCString(wszTitle, m_vkOptions.BBCForNews(), vkbbcUrl, vkNotificationItem->wszLink));
	}
	else if (vkParentType == vkComment) {
		CMString wszText(jnParent["text"].as_mstring());
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
			CMString wszRepl(FORMAT, L"?reply=%d", iId);
			delete vkNotificationItem;
			return GetVkParent(jnPost, vkPost, wszText, wszRepl);
		}

		const JSONNode &jnTopic = jnParent["topic"];
		if (jnTopic) {
			CMString wszRepl(FORMAT, L"?reply=%d", iId);
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
		return NULL;

	CMString wszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString wszNotificationTranslate = SpanVKNotificationType(wszType, vkFeedbackType, vkParentType);

	const JSONNode &jnFeedback = jnItem["feedback"];
	const JSONNode &jnParent = jnItem["parent"];

	if (m_vkOptions.bNotificationFilterAcceptedFriends && wszType == L"friend_accepted" && jnFeedback && vkFeedbackType == VKObjType::vkUsers) {
		OnFriendAccepted(jnFeedback);
		return NULL;
	}

	if (!jnFeedback || !jnParent)
		return NULL;

	CVkUserInfo *vkUser = NULL;
	CMString wszFeedback = GetVkFeedback(jnFeedback, vkFeedbackType, vkUsers, vkUser);
	CVKNewsItem *vkNotification = GetVkParent(jnParent, vkParentType);

	if (!vkNotification)
		return NULL;

	if (vkNotification && !wszFeedback.IsEmpty()) {
		CMString wszNotificaton;
		wszNotificaton.AppendFormat(wszFeedback, wszNotificationTranslate, vkNotification->wszText);
		vkNotification->wszText = wszNotificaton;

		wszFeedback = RemoveBBC(wszFeedback);
		int idx = wszFeedback.Find(L" %s %s");

		vkNotification->wszPopupTitle.AppendFormat(L"%s %s", wszFeedback.Mid(0, idx), wszNotificationTranslate);
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
	return NULL;
}

void CVkProto::OnFriendAccepted(const JSONNode & jnFeedback)
{
	const JSONNode &jnUsers = jnFeedback["items"];

	for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
		const JSONNode &jnUserItem = (*it);
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
		return NULL;

	CMString wszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString wszNotificationTranslate = SpanVKNotificationType(wszType, vkFeedbackType, vkParentType);

	if (!jnItem["id"])
		return NULL;

	LONG iGroupId = jnItem["id"].as_int();
	CMString wszId(FORMAT, L"%d,", iGroupId);
	CMString wszIds(ptrW(db_get_wsa(NULL, m_szModuleName, "InviteGroupIds")));

	if (wszIds.Find(wszId, 0) != -1)
		return NULL;
		
	LONG iUserId = !jnItem["invited_by"] ? 0 : jnItem["invited_by"].as_int();
	CVKNewsItem *vkNotification = new CVKNewsItem();
	vkNotification->tDate = time(NULL);
	vkNotification->vkUser = GetVkUserInfo(iUserId, vkUsers);
	vkNotification->wszType = wszType;
	vkNotification->wszId = wszId;
	vkNotification->vkFeedbackType = vkFeedbackType;
	vkNotification->vkParentType = vkParentType;

	CMString wszGroupName;
	CMString wszGName = jnItem["name"].as_mstring();
	CMString wszGLink(FORMAT, L"https://vk.com/%s", jnItem["screen_name"].as_mstring());
	wszGroupName = SetBBCString(wszGName, m_vkOptions.BBCForNews(), vkbbcUrl, wszGLink);

	CMString wszUsers = SetBBCString(iUserId ? vkNotification->vkUser->m_wszUserNick : TranslateT("Unknown"), m_vkOptions.BBCForNews(), vkbbcUrl, iUserId ? vkNotification->vkUser->m_wszLink : L"https://vk.com/");

	vkNotification->wszText.AppendFormat(L"%s %s %s", wszUsers, wszNotificationTranslate, wszGroupName);
	vkNotification->wszPopupTitle.AppendFormat(L"%s %s %s", iUserId ? vkNotification->vkUser->m_wszUserNick : TranslateT("Unknown"), wszNotificationTranslate, wszGName);

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

	time_t tLastNewsReqTime = getDword("LastNewsReqTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNewsReqTime < 3 * 60)
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

	setDword("LastNewsReqTime", (DWORD)time(NULL));
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
	db_unset(NULL, m_szModuleName, "LastNewsReqTime");
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
		for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
			CVKNewsItem *vkNewsItem = GetVkNewsItem((*it), vkUsers);
			if (!vkNewsItem)
				continue;
			CVKNewsItem *vkNewsFoundItem = vkNews.find(vkNewsItem);
			if (vkNewsFoundItem == NULL)
				vkNews.insert(vkNewsItem);
			else if (vkNewsFoundItem->wszType == L"wall_photo" && vkNewsItem->wszType == L"post") {
				vkNews.remove(vkNewsFoundItem);
				vkNews.insert(vkNewsItem);
			}
			else 
				delete vkNewsItem;
		}

	bool bNewsAdded = false;
	for (int i = 0; i < vkNews.getCount(); i++)
		if (!(m_vkOptions.bNewsSourceNoReposts && vkNews[i].bIsRepost)) {
			AddFeedEvent(vkNews[i]);
			bNewsAdded = true;
		}

	if (bNewsAdded)
		AddCListEvent(true);
	
	setDword("LastNewsTime", time(NULL));

	vkNews.destroy();
	vkUsers.destroy();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUnreadNotifications(time_t tLastNotificationsTime)
{
	debugLogA("CVkProto::RetrieveUnreadNotifications");
	if (!IsOnline())
		return;

	time_t tLastNotificationsReqTime = getDword("LastNotificationsReqTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNotificationsReqTime < 3 * 60)
		return;

	CMString code(FORMAT, L"return{\"notifications\":API.notifications.get({\"count\": 100, \"start_time\":%d})%s",
		(LONG)(tLastNotificationsTime + 1),
		m_vkOptions.bNotificationFilterInvites ? L",\"groupinvates\":API.groups.getInvites({\"extended\":1})};" : L"};");

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveUnreadNotifications)
		<< WCHAR_PARAM("code", code)		);

	setDword("LastNotificationsReqTime", (DWORD)time(NULL));
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
	db_unset(NULL, m_szModuleName, "LastNotificationsReqTime");
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
			for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
				CVKNewsItem *vkNotificationItem = GetVkNotificationsItem((*it), vkUsers);
				if (!vkNotificationItem)
					continue;
				if (vkNotification.find(vkNotificationItem) == NULL)
					vkNotification.insert(vkNotificationItem);
				else
					delete vkNotificationItem;
			}
	}

	if (jnGroupInvates) {
		const JSONNode &jnItems = jnGroupInvates["items"];

		if (jnItems)
			for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
				CVKNewsItem *vkNotificationItem = GetVkGroupInvates((*it), vkUsers);
				if (!vkNotificationItem)
					continue;
				if (vkNotification.find(vkNotificationItem) == NULL)
					vkNotification.insert(vkNotificationItem);
				else
					delete vkNotificationItem;
			}
	}

	bool bNotificationCommentAdded = false;
	bool bNotificationComment = false;
	bool bNotificationAdded = false;
	for (int i = 0; i < vkNotification.getCount(); i++)
		if (FilterNotification(&vkNotification[i], bNotificationComment)) {
			AddFeedEvent(vkNotification[i]);
			bNotificationAdded = true;
			bNotificationCommentAdded = bNotificationComment || bNotificationCommentAdded;
		}

	if (bNotificationAdded)
		AddCListEvent(false);

	setDword("LastNotificationsTime", time(NULL));
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

	time_t tLastNotificationsTime = getDword("LastNotificationsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNotificationsTime - m_vkOptions.iNotificationsInterval * 60 >= -3 && m_vkOptions.bNotificationsEnabled)
		RetrieveUnreadNotifications(tLastNotificationsTime);

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNewsTime - m_vkOptions.iNewsInterval * 60 >= -3 && m_vkOptions.bNewsEnabled)
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

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	RetrieveUnreadNews(tLastNewsTime);

	return 0;
}

void CVkProto::NewsClearHistory()
{
	debugLogA("CVkProto::NewsClearHistory");
	MCONTACT hContact = FindUser(VK_FEED_USER);
	if (hContact == NULL || !m_vkOptions.bNewsAutoClearHistory)
		return;

	time_t tTime = time(NULL) - m_vkOptions.iNewsAutoClearHistoryInterval;
	MEVENT hDBEvent = db_event_first(hContact);
	while (hDBEvent) {
		MEVENT hDBEventNext = db_event_next(hContact, hDBEvent);
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDBEvent, &dbei);
		if (dbei.timestamp < tTime)
			db_event_delete(hContact, hDBEvent);
		hDBEvent = hDBEventNext;
	}
}