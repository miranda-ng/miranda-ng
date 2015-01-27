/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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
	bool bSpecialContact = m_bNewsEnabled || m_bNotificationsEnabled || m_bSpecialContactAlwaysEnabled;
	
	MCONTACT hContact = FindUser(VK_FEED_USER);
	if (!bSpecialContact) {
		if (hContact)
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		return;
	}
	
	if (!hContact) {
		hContact = FindUser(VK_FEED_USER, true);
		CMString tszNick = TranslateT("VKontakte");

		setTString(hContact, "Nick", tszNick.GetBuffer());
		CMString tszUrl = _T("https://vk.com/press/Simple.png");
		SetAvatarUrl(hContact, tszUrl);
		ReloadAvatarInfo(hContact);

		setTString(hContact, "domain", _T("feed"));
		setTString(hContact, "Homepage", _T("https://vk.com/feed"));
	} 
	
	if (getWord(hContact, "Status", 0) != ID_STATUS_ONLINE)
		setWord(hContact, "Status", ID_STATUS_ONLINE);
	SetMirVer(hContact, 7);

}

void CVkProto::AddFeedEvent(CMString& tszBody, time_t tTime)
{
	if (tszBody.IsEmpty()) {
		debugLogA("CVkProto::AddFeedEvent %d", tTime);
		return;
	}
	
	MCONTACT hContact = FindUser(VK_FEED_USER, true);
	ptrT ptszBody(mir_tstrdup(tszBody.GetBuffer()));
	PROTORECVEVENT recv = { 0 };

	recv.flags = PREF_TCHAR;
	recv.timestamp = tTime;
	recv.tszMessage = ptszBody;
	recv.lParam = 0;
	recv.pCustomData = NULL;
	recv.cbCustomDataSize = 0;

	ProtoChainRecvMsg(hContact, &recv);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVkUserInfo* CVkProto::GetVkUserInfo(LONG iUserId, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkUserInfo %d", iUserId);
	if (iUserId == 0)
		return NULL;

	bool bIsGroup = (iUserId < 0);
	CVkUserInfo * vkUser = vkUsers.find((CVkUserInfo *)&iUserId);

	if (vkUser == NULL) {
		CMString tszNick = TranslateT("Unknown");
		CMString tszLink = _T("https://vk.com/");
		if (iUserId) {
			tszLink += bIsGroup ? "club" : "id";
			tszLink.AppendFormat(_T("%d"), bIsGroup ? -iUserId : iUserId);
		}
		vkUser = new CVkUserInfo(iUserId, bIsGroup, tszNick, tszLink, bIsGroup ? NULL : FindUser(iUserId));
		vkUsers.insert(vkUser);
	}

	return vkUser;
}

void CVkProto::CreateVkUserInfoList(OBJLIST<CVkUserInfo> &vkUsers, JSONNODE *pResponse)
{
	debugLogA("CVkProto::CreateVkUserInfoList");
	if (pResponse == NULL)
		return;
	
	JSONNODE *pProfiles = json_get(pResponse, "profiles");
	JSONNODE *pProfile;
	if (pProfiles != NULL)
		for (size_t i = 0; (pProfile = json_at(pProfiles, i)) != NULL; i++) {
			LONG UserId = json_as_int(json_get(pProfile, "id"));
			if (!UserId)
				continue;

			CMString tszNick = json_as_CMString(json_get(pProfile, "first_name"));
			tszNick.AppendChar(' ');
			tszNick += json_as_CMString(json_get(pProfile, "last_name"));
			CMString tszLink = _T("https://vk.com/");
			tszLink += json_as_CMString(json_get(pProfile, "screen_name"));
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, false, tszNick, tszLink, FindUser(UserId));
			vkUsers.insert(vkUser);
		}
	
	JSONNODE *pGroups = json_get(pResponse, "groups");
	if (pGroups != NULL)		
		for (size_t i = 0; (pProfile = json_at(pGroups, i)) != NULL; i++) {
			LONG UserId = -json_as_int(json_get(pProfile, "id"));
			if (!UserId)
				continue;

			CMString tszNick = json_as_CMString(json_get(pProfile, "name"));
			CMString tszLink = _T("https://vk.com/");
			tszLink += json_as_CMString(json_get(pProfile, "screen_name"));
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, true, tszNick, tszLink);
			vkUsers.insert(vkUser);
		}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CVKNewsItem* CVkProto::GetVkNewsItem(JSONNODE *pItem, OBJLIST<CVkUserInfo> &vkUsers, bool isRepost)
{
	//debugLogA("CVkProto::GetVkNewsItem");
	bool bPostLink = true;
	CVKNewsItem *vkNewsItem = new CVKNewsItem();
	if (pItem == NULL)
		return vkNewsItem;

	LONG iSourceId = json_as_int(json_get(pItem, "source_id"));
	iSourceId = iSourceId ? iSourceId : json_as_int(json_get(pItem, "owner_id"));
	LONG iPostId = json_as_int(json_get(pItem, "post_id"));
	CMString tszText = json_as_CMString(json_get(pItem, "text"));

	vkNewsItem->tszType = json_as_CMString(json_get(pItem, "type"));
	vkNewsItem->vkUser = GetVkUserInfo(iSourceId, vkUsers);
	vkNewsItem->bIsGroup = vkNewsItem->vkUser->m_bIsGroup;
	vkNewsItem->tDate = json_as_int(json_get(pItem, "date"));

	if (!tszText.IsEmpty())
		tszText += _T("\n");

	debugLog(_T("CVkProto::GetVkNewsItem %d %d %s <%s>"), iSourceId, iPostId, vkNewsItem->tszType.GetBuffer(), tszText.GetBuffer());

	if (vkNewsItem->tszType == _T("photo_tag")) {
		bPostLink = false;
		JSONNODE *pPhotos = json_get(pItem, "photo_tags");
		if (pPhotos) {			
			JSONNODE *pPhotoItems = json_get(pPhotos, "items");
			if (pPhotoItems) {
				JSONNODE *pPhotoItem;
				tszText = TranslateT("User was tagged in these photos:");
				for (size_t i = 0; (pPhotoItem = json_at(pPhotoItems, i)) != NULL; i++)
					tszText += _T("\n") + GetVkPhotoItem(pPhotoItem, m_iBBCForNews);
			}
		}
	}
	else if (vkNewsItem->tszType == _T("photo") || vkNewsItem->tszType == _T("wall_photo")) {
		bPostLink = false;
		JSONNODE *pPhotos = json_get(pItem, "photos");
		if (pPhotos) {
			JSONNODE *pPhotoItems = json_get(pPhotos, "items"), *pPhotoItem;
			if (pPhotoItems)		
				for (size_t i = 0; (pPhotoItem = json_at(pPhotoItems, i)) != NULL; i++) {					
					tszText += GetVkPhotoItem(pPhotoItem, m_iBBCForNews) + _T("\n");
					if (i == 0 && vkNewsItem->tszType == _T("wall_photo")) {
						LONG iPhotoPostId = json_as_int(json_get(pPhotoItem, "post_id"));
						if (iPhotoPostId) {
							bPostLink = true;
							iPostId = iPhotoPostId;
							break; // max 1 wall_photo when photo post_id !=0
						}					
					}
				}
		}
	} 
	else if (vkNewsItem->tszType == _T("post") || vkNewsItem->tszType.IsEmpty()) {
		bPostLink = true;
		JSONNODE * pRepost = json_get(pItem, "copy_history");
		if (pRepost) {
			CVKNewsItem *vkRepost = GetVkNewsItem(json_at(pRepost, 0), vkUsers, true);		
			vkRepost->tszText.Replace(_T("\n"), _T("\n\t"));
			tszText += vkRepost->tszText;
			tszText += _T("\n");
			vkNewsItem->bIsRepost = true;
			delete vkRepost;
		}

		JSONNODE *pAttachments = json_get(pItem, "attachments");
		if (pAttachments)
			tszText += GetAttachmentDescr(pAttachments, m_bUseBBCOnAttacmentsAsNews ? m_iBBCForNews : m_iBBCForAttachments);
	}

	CMString tszResFormat;	

	if (!isRepost)
		tszResFormat = Translate("News from %s\n%s");
	else {
		tszResFormat = Translate("\tRepost from %s\n%s");
		bPostLink = false;
	}
		
	vkNewsItem->tszText.AppendFormat(tszResFormat, 
		SetBBCString(vkNewsItem->vkUser->m_tszUserNick.GetBuffer(), m_iBBCForNews, vkbbcUrl, 
		vkNewsItem->vkUser->m_tszLink.GetBuffer()), tszText.GetBuffer());
	
	vkNewsItem->tszId.AppendFormat(_T("%d_%d"), vkNewsItem->vkUser->m_UserId, iPostId);
	if (bPostLink) {
		vkNewsItem->tszLink = CMString(_T("https://vk.com/wall")) + vkNewsItem->tszId;
		vkNewsItem->tszText.AppendChar(_T('\n'));
		vkNewsItem->tszText += SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNewsItem->tszLink.GetBuffer());
	}

	debugLog(_T("CVkProto::GetVkNewsItem %d %d <%s> <%s>"), iSourceId, iPostId, vkNewsItem->tszText.GetBuffer(), tszText.GetBuffer());

	return vkNewsItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CMString CVkProto::GetVkFeedback(JSONNODE *pFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser)
{
	debugLogA("CVkProto::GetVkFeedback");
	CMString tszRes;
	if (!pFeedback || !vkFeedbackType)
		return tszRes;

	CMString tszFormat;
	LONG iUserId = 0;

	if (vkFeedbackType == vkComment) {
		iUserId = json_as_int(json_get(pFeedback, "from_id"));
		tszFormat = _T("%s %%s %%s\n%s");		
	}
	else if (vkFeedbackType == vkPost) {		
		iUserId = json_as_int(json_get(pFeedback, "owner_id "));
		tszFormat = _T("%s %%s %%s\n%s");		
	}
	else if (vkFeedbackType == VKObjType::vkUsers || vkFeedbackType == vkCopy) {
		JSONNODE *pUsers = json_get(pFeedback, "items"), *pUserItem;
		
		CMString tszUsers;
		for (int i = 0; (pUserItem = json_at(pUsers, i)) != NULL; i++) {
			iUserId = json_as_int(json_get(pUserItem, "from_id"));
			if (iUserId == 0)
				continue;
			vkUser = GetVkUserInfo(iUserId, vkUsers);
			if (!tszUsers.IsEmpty())
				tszUsers += _T(", ");
			tszUsers += SetBBCString(vkUser->m_tszUserNick.GetBuffer(), m_iBBCForNews, vkbbcUrl, vkUser->m_tszLink.GetBuffer());
		}
		tszRes.AppendFormat(_T("%s %%s %%s"), tszUsers.GetBuffer());
		vkUser = NULL;
		iUserId = 0;
	}

	if (iUserId) {
		vkUser = GetVkUserInfo(iUserId, vkUsers);
		CMString tszText = json_as_CMString(json_get(pFeedback, "text"));
		tszRes.AppendFormat(tszFormat, SetBBCString(vkUser->m_tszUserNick.GetBuffer(), m_iBBCForNews, vkbbcUrl, vkUser->m_tszLink.GetBuffer()), ClearFormatNick(tszText).GetBuffer());
	}

	return tszRes;
}

CVKNewsItem* CVkProto::GetVkParent(JSONNODE *pParent, VKObjType vkParentType, TCHAR *ptszReplyText, TCHAR *ptszReplyLink)
{
	debugLogA("CVkProto::GetVkParent");
	CMString tszRes;
	if (!pParent || !vkParentType)
		return NULL;

	CVKNewsItem * vkNotificationItem = new CVKNewsItem();
			
	if (vkParentType == vkPhoto) {
		CMString tszPhoto = GetVkPhotoItem(pParent, m_iBBCForNews);
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/photo%s"), vkNotificationItem->tszId.GetBuffer());
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), tszPhoto.GetBuffer());

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI).GetBuffer());
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink.GetBuffer()).GetBuffer());
	}
	else if (vkParentType == vkVideo) {
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszTitle = json_as_CMString(json_get(pParent, "title"));
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/video%s"), vkNotificationItem->tszId.GetBuffer());
		
		CMString tszText = json_as_CMString(json_get(pParent, "text"));
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Video description:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText.GetBuffer(), m_iBBCForNews, vkbbcI).GetBuffer());

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI).GetBuffer());
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(tszTitle.GetBuffer(), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink.GetBuffer()).GetBuffer());
	}
	else if (vkParentType == vkPost) {
		LONG iOwnerId = json_as_int(json_get(pParent, "from_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/wall%s%s"), vkNotificationItem->tszId.GetBuffer(), ptszReplyLink ? ptszReplyLink : _T(""));
		
		CMString tszText = json_as_CMString(json_get(pParent, "text"));
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Post text:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText.GetBuffer(), m_iBBCForNews, vkbbcI).GetBuffer());

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI).GetBuffer());

		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink.GetBuffer()).GetBuffer());
	}
	else if (vkParentType == vkTopic) {
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszTitle = json_as_CMString(json_get(pParent, "title"));
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/topic%s%s"), 
		vkNotificationItem->tszId.GetBuffer(), ptszReplyLink ? ptszReplyLink : _T(""));
		
		CMString tszText = json_as_CMString(json_get(pParent, "text"));
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Topic text:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText.GetBuffer(), m_iBBCForNews, vkbbcI).GetBuffer());

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI).GetBuffer());
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(tszTitle.GetBuffer(), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink.GetBuffer()).GetBuffer());
	}
	else if (vkParentType == vkComment) {
		CMString tszText = json_as_CMString(json_get(pParent, "text"));
		ClearFormatNick(tszText);

		JSONNODE *pNode = json_get(pParent, "photo");
		if (pNode) {
			delete vkNotificationItem;
			return GetVkParent(pNode, vkPhoto, tszText.IsEmpty() ? NULL : tszText.GetBuffer());
		}

		pNode = json_get(pParent, "video");
		if (pNode) {
			delete vkNotificationItem;
			return GetVkParent(pNode, vkVideo, tszText.IsEmpty() ? NULL : tszText.GetBuffer());
		}

		LONG iId = json_as_int(json_get(pParent, "id"));

		pNode = json_get(pParent, "post");
		if (pNode) {
			CMString tszRepl;
			tszRepl.AppendFormat(_T("?reply=%d"), iId);		
			delete vkNotificationItem;
			return GetVkParent(pNode, vkPost, tszText.IsEmpty() ? NULL : tszText.GetBuffer(), tszRepl.GetBuffer());
		}

		pNode = json_get(pParent, "topic");
		if (pNode) {
			CMString tszRepl;		
			tszRepl.AppendFormat(_T("?reply=%d"), iId);
			delete vkNotificationItem;
			return GetVkParent(pNode, vkTopic, tszText.IsEmpty() ? NULL : tszText.GetBuffer(), tszRepl.GetBuffer());
		}
	}

	return vkNotificationItem;
}

CVKNewsItem* CVkProto::GetVkNotificationsItem(JSONNODE *pItem, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkNotificationsItem");
	if (pItem == NULL)
		return NULL;

	CMString tszType = json_as_CMString(json_get(pItem, "type"));
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString tszNotificationTranslate = SpanVKNotificationType(tszType, vkFeedbackType, vkParentType);
		
	JSONNODE *pFeedback = json_get(pItem, "feedback");
	if (!pFeedback)
		return NULL;
	CVkUserInfo *vkUser = NULL;
	CMString tszFeedback = GetVkFeedback(pFeedback, vkFeedbackType, vkUsers, vkUser);

	JSONNODE *pParent = json_get(pItem, "parent");
	if (!pParent)
		return NULL;

	CVKNewsItem* vkNotification = GetVkParent(pParent, vkParentType);
	if (!vkNotification)
		return NULL;
	
	if (vkNotification && !tszFeedback.IsEmpty()) {
		CMString tszNotificaton;
		tszNotificaton.AppendFormat(tszFeedback, tszNotificationTranslate.GetBuffer(), vkNotification->tszText.GetBuffer());
		vkNotification->tszText = tszNotificaton;
		vkNotification->tszType = tszType;
		vkNotification->tDate = json_as_int(json_get(pItem, "date"));
		vkNotification->vkFeedbackType = vkFeedbackType;
		vkNotification->vkParentType = vkParentType;
		vkNotification->vkUser = vkUser;
		return vkNotification;
	}
	
	delete vkNotification;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUnreadNews(time_t tLastNewsTime)
{
	debugLogA("CVkProto::RetrieveUnreadNews");
	if (!IsOnline())
		return;
		
	CMStringA szFilter;
	szFilter = m_bNewsFilterPosts ? "post" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_bNewsFilterPhotos ? "photo" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_bNewsFilterTags ? "photo_tag" : "";

	szFilter += szFilter.IsEmpty() ? "" : ",";
	szFilter += m_bNewsFilterWallPhotos ? "wall_photo" : "";

	if (szFilter.IsEmpty()) {
		debugLogA("CVkProto::RetrieveUnreadNews szFilter empty");
		return;
	}

	CMStringA szSource;
	szSource = m_bNewsSourceFriends ? "friends" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_bNewsSourceGroups ? "groups" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_bNewsSourcePages ? "pages" : "";

	szSource += szSource.IsEmpty() ? "" : ",";
	szSource += m_bNewsSourceFollowing ? "following" : "";

	if (szSource.IsEmpty()) {
		debugLogA("CVkProto::RetrieveUnreadNews szSource empty");
		return;
	}
			
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/newsfeed.get.json", true, &CVkProto::OnReceiveUnreadNews)
		<< INT_PARAM("count", 100)
		<< INT_PARAM("return_banned", m_bNewsSourceIncludeBanned ? 1 : 0)
		<< INT_PARAM("max_photos", m_iMaxLoadNewsPhoto)
		<< INT_PARAM("start_time", tLastNewsTime + 1)
		<< CHAR_PARAM("filters", szFilter.GetBuffer())
		<< CHAR_PARAM("source_ids", szSource.GetBuffer())
		<< VER_API);
}

static int sttCompareVKNewsItems(const CVKNewsItem *p1, const CVKNewsItem *p2)
{
	return p1->tszId.Compare(p2->tszId) ? (LONG)p1->tDate - (LONG)p2->tDate : 0;
}

void CVkProto::OnReceiveUnreadNews(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNews %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	OBJLIST<CVkUserInfo> vkUsers(5, NumericKeySortT);
	CreateVkUserInfoList(vkUsers, pResponse);

	JSONNODE *pItems = json_get(pResponse, "items");
	JSONNODE *pItem;

	OBJLIST<CVKNewsItem> vkNews(5, sttCompareVKNewsItems);
	if (pItems != NULL)
		for (int i = 0; (pItem = json_at(pItems, i)) != NULL; i++) {	
			CVKNewsItem *vkNewsItem = GetVkNewsItem(pItem, vkUsers);
			if (!vkNewsItem)
				continue;
			if (vkNews.find(vkNewsItem) == NULL)
				vkNews.insert(vkNewsItem);
			else
				delete vkNewsItem;
		}

	for (int i = 0; i < vkNews.getCount(); i++)
		if (!(m_bNewsSourceNoReposts && vkNews[i].bIsRepost))
			AddFeedEvent(vkNews[i].tszText, vkNews[i].tDate);
	
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
	
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/notifications.get.json", true, &CVkProto::OnReceiveUnreadNotifications)
		<< INT_PARAM("count", 100)
		<< INT_PARAM("start_time", tLastNotificationsTime + 1)
		<< VER_API);
}

bool CVkProto::FilterNotification(CVKNewsItem* vkNotificationItem)
{
	if (vkNotificationItem->vkParentType == vkNull)
		return false;

	if (vkNotificationItem->tszType == _T("mention_comments")
		|| vkNotificationItem->tszType == _T("mention_comment_photo")
		|| vkNotificationItem->tszType == _T("mention_comment_video"))
		return m_bNotificationFilterMentions;

	bool result = (vkNotificationItem->vkFeedbackType == vkUsers && m_bNotificationFilterLikes);
	result = (vkNotificationItem->vkFeedbackType == vkCopy && m_bNotificationFilterReposts) || result;
	result = (vkNotificationItem->vkFeedbackType == vkComment && m_bNotificationFilterComments) || result;
	return result;
}

void CVkProto::OnReceiveUnreadNotifications(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNotifications %d", reply->resultCode);
	if (reply->resultCode != 200)
		return;

	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	if (pResponse == NULL)
		return;

	OBJLIST<CVkUserInfo> vkUsers(5, NumericKeySortT);
	CreateVkUserInfoList(vkUsers, pResponse);

	JSONNODE *pItems = json_get(pResponse, "items");
	JSONNODE *pItem;

	OBJLIST<CVKNewsItem> vkNotification(5, sttCompareVKNewsItems);
	if (pItems != NULL)
		for (int i = 0; (pItem = json_at(pItems, i)) != NULL; i++) {	
			CVKNewsItem *vkNotificationItem = GetVkNotificationsItem(pItem, vkUsers);
			if (!vkNotificationItem)
				continue;
			if (vkNotification.find(vkNotificationItem) == NULL)
				vkNotification.insert(vkNotificationItem);
			else
				delete vkNotificationItem;
		}

	for (int i = 0; i < vkNotification.getCount(); i++)
		if (FilterNotification(&vkNotification[i]))
			AddFeedEvent(vkNotification[i].tszText, vkNotification[i].tDate);
	
	setDword("LastNotificationsTime", time(NULL));
	vkNotification.destroy();
	vkUsers.destroy();
}

void CVkProto::RetrieveUnreadEvents()
{
	debugLogA("CVkProto::RetrieveUnreadEvents");
	if (!IsOnline() || (!m_bNotificationsEnabled && !m_bNewsEnabled))
		return;

	time_t tLastNotificationsTime = getDword("LastNotificationsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNotificationsTime >= m_iNotificationsInterval * 60 && m_bNotificationsEnabled)
		RetrieveUnreadNotifications(tLastNotificationsTime);

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNewsTime >= m_iNewsInterval * 60 && m_bNewsEnabled)
		RetrieveUnreadNews(tLastNewsTime);

	NewsClearHistory();
}

INT_PTR CVkProto::SvcLoadVKNews(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcLoadVKNews");
	if (!IsOnline())
		return 1;

	if (!m_bNewsEnabled && !m_bNotificationsEnabled) {
		m_bSpecialContactAlwaysEnabled = true; 
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
	if (hContact == NULL || !m_bNewsAutoClearHistory)
		return;

	time_t tTime = time(NULL) - m_iNewsAutoClearHistoryInterval;
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