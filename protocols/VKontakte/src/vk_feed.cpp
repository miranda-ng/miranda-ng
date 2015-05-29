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

		setTString(hContact, "Nick", TranslateT("VKontakte"));
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
	T2Utf pszBody(tszBody);

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = tTime;
	recv.szMessage = pszBody;
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

void CVkProto::CreateVkUserInfoList(OBJLIST<CVkUserInfo> &vkUsers, const JSONNode &jnResponse)
{
	debugLogA("CVkProto::CreateVkUserInfoList");
	if (!jnResponse)
		return;
	
	const JSONNode &jnProfiles = jnResponse["profiles"];
	
	if (!jnProfiles.isnull())
		for (auto it = jnProfiles.begin(); it != jnProfiles.end(); ++it) {
			const JSONNode &jnProfile = (*it);
			if (!jnProfile["id"])
				continue;
			LONG UserId =  jnProfile["id"].as_int();		

			CMString tszNick(jnProfile["first_name"].as_mstring());
			tszNick.AppendChar(' ');
			tszNick += jnProfile["last_name"].as_mstring();
			CMString tszLink = _T("https://vk.com/");
			CMString tszScreenName(jnProfile["screen_name"].as_mstring());
			if (tszScreenName.IsEmpty())
				tszScreenName.AppendFormat(_T("id%d"), UserId);
			tszLink += tszScreenName;
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, false, tszNick, tszLink, FindUser(UserId));
			vkUsers.insert(vkUser);
		}
	
	const JSONNode &jnGroups = jnResponse["groups"];
	if (!jnGroups.isnull())		
		for (auto it = jnGroups.begin(); it != jnGroups.end(); ++it) {
			const JSONNode &jnProfile = (*it);
			if (!jnProfile["id"])
				continue;
			LONG UserId = - jnProfile["id"].as_int();

			CMString tszNick(jnProfile["name"].as_mstring());
			CMString tszLink = _T("https://vk.com/");
			tszLink += jnProfile["screen_name"].as_mstring();
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, true, tszNick, tszLink);
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

	LONG iSourceId = jnItem["source_id"].isnull() ? jnItem["owner_id"].as_int() : jnItem["source_id"].as_int();
	LONG iPostId = jnItem["post_id"].as_int();
	CMString tszText(jnItem["text"].as_mstring());

	vkNewsItem->tszType = jnItem["type"].as_mstring();
	vkNewsItem->vkUser = GetVkUserInfo(iSourceId, vkUsers);
	vkNewsItem->bIsGroup = vkNewsItem->vkUser->m_bIsGroup;
	vkNewsItem->tDate = jnItem["date"].as_int();

	if (!tszText.IsEmpty())
		tszText += _T("\n");

	debugLog(_T("CVkProto::GetVkNewsItem %d %d %s <%s>"), iSourceId, iPostId, vkNewsItem->tszType, tszText);

	if (vkNewsItem->tszType == _T("photo_tag")) {
		bPostLink = false;
		const JSONNode &jnPhotos = jnItem["photo_tags"];
		if (!jnPhotos.isnull()) {			
			const JSONNode &jnPhotoItems = jnPhotos["items"];
			if (!jnPhotoItems.isnull()) {				
				tszText = TranslateT("User was tagged in these photos:");
				for (auto it = jnPhotoItems.begin(); it != jnPhotoItems.end(); ++it)
					tszText += _T("\n") + GetVkPhotoItem((*it), m_iBBCForNews);
			}
		}
	}
	else if (vkNewsItem->tszType == _T("photo") || vkNewsItem->tszType == _T("wall_photo")) {
		bPostLink = false;
		const JSONNode &jnPhotos = jnItem["photos"];
		int i = 0;
		if (!jnPhotos.isnull()) {
			const JSONNode &jnPhotoItems = jnPhotos["items"];
			if (!jnPhotoItems.isnull())
				for (auto it = jnPhotoItems.begin(); it != jnPhotoItems.end(); ++it) {
					const JSONNode &jnPhotoItem = (*it);
					tszText += GetVkPhotoItem(jnPhotoItem, m_iBBCForNews) + _T("\n");
					if (i == 0 && vkNewsItem->tszType == _T("wall_photo")) {						
						if (!jnPhotoItem["post_id"].isnull()) {
							bPostLink = true;
							iPostId = jnPhotoItem["post_id"].as_int();
							break; // max 1 wall_photo when photo post_id !=0
						}					
					}
					i++;
				}
		}
	} 
	else if (vkNewsItem->tszType == _T("post") || vkNewsItem->tszType.IsEmpty()) {
		bPostLink = true;
		const JSONNode &jnRepost = jnItem["copy_history"];
		if (!jnRepost.isnull()) {
			CVKNewsItem *vkRepost = GetVkNewsItem((*jnRepost.begin()), vkUsers, true);
			vkRepost->tszText.Replace(_T("\n"), _T("\n\t"));
			tszText += vkRepost->tszText;
			tszText += _T("\n");
			vkNewsItem->bIsRepost = true;
			delete vkRepost;
		}

		const JSONNode &jnAttachments = jnItem["attachments"];
		if (!jnAttachments.isnull()){
			if (!tszText.IsEmpty())
				tszText.AppendChar(_T('\n'));
			tszText += GetAttachmentDescr(jnAttachments, m_bUseBBCOnAttacmentsAsNews ? m_iBBCForNews : m_iBBCForAttachments);
		}
	}

	CMString tszResFormat;	

	if (!isRepost)
		tszResFormat = Translate("News from %s\n%s");
	else {
		tszResFormat = Translate("\tRepost from %s\n%s");
		bPostLink = false;
	}
		
	vkNewsItem->tszText.AppendFormat(tszResFormat, 
		SetBBCString(vkNewsItem->vkUser->m_tszUserNick, m_iBBCForNews, vkbbcUrl, 
		vkNewsItem->vkUser->m_tszLink), tszText);
	
	vkNewsItem->tszId.AppendFormat(_T("%d_%d"), vkNewsItem->vkUser->m_UserId, iPostId);
	if (bPostLink) {
		vkNewsItem->tszLink = CMString(_T("https://vk.com/wall")) + vkNewsItem->tszId;
		vkNewsItem->tszText.AppendChar(_T('\n'));
		vkNewsItem->tszText += SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNewsItem->tszLink);
	}

	debugLog(_T("CVkProto::GetVkNewsItem %d %d <%s> <%s>"), iSourceId, iPostId, vkNewsItem->tszText, tszText);

	return vkNewsItem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

CMString CVkProto::GetVkFeedback(const JSONNode &jnFeedback, VKObjType vkFeedbackType, OBJLIST<CVkUserInfo> &vkUsers, CVkUserInfo *vkUser)
{
	debugLogA("CVkProto::GetVkFeedback");
	CMString tszRes;
	if (!jnFeedback || !vkFeedbackType)
		return tszRes;

	CMString tszFormat;
	LONG iUserId = 0;

	if (vkFeedbackType == vkComment) {
		iUserId = jnFeedback["from_id"].as_int();
		tszFormat = _T("%s %%s %%s\n%s");		
	}
	else if (vkFeedbackType == vkPost) {		
		iUserId = jnFeedback["owner_id "].as_int();
		tszFormat = _T("%s %%s %%s\n%s");		
	}
	else if (vkFeedbackType == VKObjType::vkUsers || vkFeedbackType == vkCopy) {
		const JSONNode &jnUsers = jnFeedback["items"];
		
		CMString tszUsers;
		for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
			const JSONNode &jnUserItem = (*it);
			if (!jnUserItem["from_id"])
				continue;
			iUserId = jnUserItem["from_id"].as_int();			
			vkUser = GetVkUserInfo(iUserId, vkUsers);
			if (!tszUsers.IsEmpty())
				tszUsers += _T(", ");
			tszUsers += SetBBCString(vkUser->m_tszUserNick, m_iBBCForNews, vkbbcUrl, vkUser->m_tszLink);
		}
		tszRes.AppendFormat(_T("%s %%s %%s"), tszUsers);
		vkUser = NULL;
		iUserId = 0;
	}

	if (iUserId) {
		vkUser = GetVkUserInfo(iUserId, vkUsers);
		CMString tszText(jnFeedback["text"].as_mstring());
		tszText.Replace(_T("%"), _T("%%"));
		tszRes.AppendFormat(tszFormat, SetBBCString(vkUser->m_tszUserNick, m_iBBCForNews, vkbbcUrl, vkUser->m_tszLink), ClearFormatNick(tszText));
	}

	return tszRes;
}

CVKNewsItem* CVkProto::GetVkParent(const JSONNode &jnParent, VKObjType vkParentType, LPCTSTR ptszReplyText, LPCTSTR ptszReplyLink)
{
	debugLogA("CVkProto::GetVkParent");
	CMString tszRes;
	if (!jnParent || !vkParentType)
		return NULL;

	CVKNewsItem * vkNotificationItem = new CVKNewsItem();
			
	if (vkParentType == vkPhoto) {
		CMString tszPhoto = GetVkPhotoItem(jnParent, m_iBBCForNews);
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/photo%s"), vkNotificationItem->tszId);
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), tszPhoto);

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI));
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink));
	}
	else if (vkParentType == vkVideo) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMString tszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/video%s"), vkNotificationItem->tszId);
		
		CMString tszText(jnParent["text"].as_mstring());
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Video description:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText, m_iBBCForNews, vkbbcI));

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI));
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(tszTitle, m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink));
	}
	else if (vkParentType == vkPost) {
		LONG iOwnerId = jnParent["from_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/wall%s%s"), vkNotificationItem->tszId, ptszReplyLink ? ptszReplyLink : _T(""));
		
		CMString tszText(jnParent["text"].as_mstring());
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Post text:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText, m_iBBCForNews, vkbbcI));

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI));

		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(TranslateT("Link"), m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink));
	}
	else if (vkParentType == vkTopic) {
		LONG iOwnerId = jnParent["owner_id"].as_int();
		LONG iId = jnParent["id"].as_int();
		CMString tszTitle(jnParent["title"].as_mstring());
		vkNotificationItem->tszId.AppendFormat(_T("%d_%d"), iOwnerId, iId);
		vkNotificationItem->tszLink.AppendFormat(_T("https://vk.com/topic%s%s"), 
		vkNotificationItem->tszId, ptszReplyLink ? ptszReplyLink : _T(""));
		
		CMString tszText(jnParent["text"].as_mstring());
		ClearFormatNick(tszText);
		
		if (!tszText.IsEmpty())
			vkNotificationItem->tszText.AppendFormat(_T("\n%s: %s"), SetBBCString(TranslateT("Topic text:"), m_iBBCForNews, vkbbcB), SetBBCString(tszText, m_iBBCForNews, vkbbcI));

		if (ptszReplyText)
			vkNotificationItem->tszText.AppendFormat(_T("\n>> %s"), SetBBCString(ptszReplyText, m_iBBCForNews, vkbbcI));
		
		vkNotificationItem->tszText.AppendFormat(_T("\n%s"), SetBBCString(tszTitle, m_iBBCForNews, vkbbcUrl, vkNotificationItem->tszLink));
	}
	else if (vkParentType == vkComment) {
		CMString tszText(jnParent["text"].as_mstring());
		ClearFormatNick(tszText);

		const JSONNode &jnPhoto = jnParent["photo"];
		if (!jnPhoto.isnull()) {
			delete vkNotificationItem;
			return GetVkParent(jnPhoto, vkPhoto, tszText);
		}

		const JSONNode &jnVideo = jnParent["video"];
		if (!jnVideo.isnull()) {
			delete vkNotificationItem;
			return GetVkParent(jnVideo, vkVideo, tszText);
		}

		LONG iId = jnParent["id"].as_int();

		const JSONNode &jnPost = jnParent["post"];
		if (!jnPost.isnull()) {
			CMString tszRepl;
			tszRepl.AppendFormat(_T("?reply=%d"), iId);		
			delete vkNotificationItem;
			return GetVkParent(jnPost, vkPost, tszText, tszRepl);
		}

		const JSONNode &jnTopic = jnParent["topic"];
		if (!jnTopic.isnull()) {
			CMString tszRepl;		
			tszRepl.AppendFormat(_T("?reply=%d"), iId);
			delete vkNotificationItem;
			return GetVkParent(jnTopic, vkTopic, tszText, tszRepl);
		}
	}

	return vkNotificationItem;
}

CVKNewsItem* CVkProto::GetVkNotificationsItem(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkNotificationsItem");
	if (!jnItem)
		return NULL;

	CMString tszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString tszNotificationTranslate = SpanVKNotificationType(tszType, vkFeedbackType, vkParentType);
		
	const JSONNode &jnFeedback = jnItem["feedback"];
	if (!jnFeedback)
		return NULL;

	CVkUserInfo *vkUser = NULL;
	CMString tszFeedback = GetVkFeedback(jnFeedback, vkFeedbackType, vkUsers, vkUser);

	const JSONNode &jnParent = jnItem["parent"];
	if (!jnParent)
		return NULL;

	CVKNewsItem* vkNotification = GetVkParent(jnParent, vkParentType);
	if (!vkNotification)
		return NULL;
	
	if (vkNotification && !tszFeedback.IsEmpty()) {
		CMString tszNotificaton;
		tszNotificaton.AppendFormat(tszFeedback, tszNotificationTranslate, vkNotification->tszText);
		vkNotification->tszText = tszNotificaton;
		vkNotification->tszType = tszType;
		vkNotification->tDate = jnItem["date"].as_int();
		vkNotification->vkFeedbackType = vkFeedbackType;
		vkNotification->vkParentType = vkParentType;
		vkNotification->vkUser = vkUser;
		return vkNotification;
	}
	
	delete vkNotification;
	return NULL;
}

CVKNewsItem* CVkProto::GetVkGroupInvates(const JSONNode &jnItem, OBJLIST<CVkUserInfo> &vkUsers)
{
	debugLogA("CVkProto::GetVkGroupInvates");
	if (!jnItem)
		return NULL;

	CMString tszType(jnItem["type"].as_mstring());
	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString tszNotificationTranslate = SpanVKNotificationType(tszType, vkFeedbackType, vkParentType);
	
	if (!jnItem["id"])
		return NULL;

	LONG iGroupId = jnItem["id"].as_int();
	CMString tszId;
	tszId.AppendFormat(_T("%d,"), iGroupId);
	CMString tszIds = ptrT(db_get_tsa(NULL, m_szModuleName, "InviteGroupIds"));

	if (tszIds.Find(tszId, 0) != -1)
		return NULL;
		
	LONG iUserId = jnItem["invited_by"] ? 0 : jnItem["invited_by"].as_int();
	CVKNewsItem *vkNotification = new CVKNewsItem();
	vkNotification->tDate = time(NULL);
	vkNotification->vkUser = GetVkUserInfo(iUserId, vkUsers);
	vkNotification->tszType = tszType;
	vkNotification->tszId = tszId;
	vkNotification->vkFeedbackType = vkFeedbackType;
	vkNotification->vkParentType = vkParentType;

	CMString tszGroupName, tszGLink;
	CMString tszGName = jnItem["name"].as_mstring();
	tszGLink.AppendFormat(_T("https://vk.com/%s"), jnItem["screen_name"].as_mstring());
	tszGroupName = SetBBCString(tszGName, m_iBBCForNews, vkbbcUrl, tszGLink);

	CMString tszUsers = SetBBCString(vkNotification->vkUser->m_tszUserNick, m_iBBCForNews, vkbbcUrl, vkNotification->vkUser->m_tszLink);

	vkNotification->tszText.AppendFormat(_T("%s %s %s"), tszUsers, tszNotificationTranslate, tszGroupName);
	
	tszIds += tszId;
	setTString("InviteGroupIds", tszIds);	

	return vkNotification;	
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
		<< CHAR_PARAM("filters", szFilter)
		<< CHAR_PARAM("source_ids", szSource)
		<< VER_API);
}

static int sttCompareVKNewsItems(const CVKNewsItem *p1, const CVKNewsItem *p2)
{	
	int compareId = p1->tszId.Compare(p2->tszId);
	LONG compareDate = (LONG)p1->tDate - (LONG)p2->tDate;

	return compareId ? compareDate : 0;
}

static int sttCompareVKNotificationItems(const CVKNewsItem *p1, const CVKNewsItem *p2)
{
	int compareType = p1->tszType.Compare(p2->tszType);
	int compareId = p1->tszId.Compare(p2->tszId);
	LONG compareDate = (LONG)p1->tDate - (LONG)p2->tDate;

	return compareType ? compareDate : (compareId ? compareDate : 0);
}

void CVkProto::OnReceiveUnreadNews(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNews %d", reply->resultCode);
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
	if (!jnItems.isnull())
		for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
			CVKNewsItem *vkNewsItem = GetVkNewsItem((*it), vkUsers);
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

	CMString code;
	code.AppendFormat(_T("return{\"notifications\":API.notifications.get({\"count\": 100, \"start_time\":%d})%s"),
		(LONG)(tLastNotificationsTime + 1),
		m_bNotificationFilterInvites ? _T(",\"groupinvates\":API.groups.getInvites({\"extended\":1})};") : _T("};"));

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveUnreadNotifications)
		<< TCHAR_PARAM("code", code)		
		<< VER_API);	
}

bool CVkProto::FilterNotification(CVKNewsItem* vkNotificationItem, bool& isCommented)
{
	isCommented = false;
	if (vkNotificationItem->vkParentType == vkNull)
		return false;

	if (vkNotificationItem->tszType == _T("mention_comments")
		|| vkNotificationItem->tszType == _T("mention_comment_photo")
		|| vkNotificationItem->tszType == _T("mention_comment_video")){
		isCommented = true;
		return m_bNotificationFilterMentions;
	}

	bool result = (vkNotificationItem->vkFeedbackType == vkUsers && m_bNotificationFilterLikes);
	result = (vkNotificationItem->vkFeedbackType == vkCopy && m_bNotificationFilterReposts) || result;
	result = (vkNotificationItem->vkFeedbackType == vkComment && m_bNotificationFilterComments) || result;
	result = (vkNotificationItem->vkParentType == vkInvite && m_bNotificationFilterInvites) || result;

	isCommented = (vkNotificationItem->vkFeedbackType == vkComment);
	
	return result;
}

void CVkProto::NotificationMarkAsViewed()
{
	debugLogA("CVkProto::NotificationMarkAsViewed");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/notifications.markAsViewed.json", true, &CVkProto::OnReceiveSmth)
		<< VER_API);
}

void CVkProto::OnReceiveUnreadNotifications(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUnreadNotifications %d", reply->resultCode);
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
	
	if (!jnNotifications.isnull()) {
		const JSONNode &jnItems = jnNotifications["items"];

		if (!jnItems.isnull())
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

	if (!jnGroupInvates.isnull()) {
		const JSONNode &jnItems = jnGroupInvates["items"];

		if (!jnItems.isnull())
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
	for (int i = 0; i < vkNotification.getCount(); i++)
		if (FilterNotification(&vkNotification[i], bNotificationComment)) {			
			AddFeedEvent(vkNotification[i].tszText, vkNotification[i].tDate);
			bNotificationCommentAdded = bNotificationComment || bNotificationCommentAdded;
		}

	setDword("LastNotificationsTime", time(NULL));
	if (m_bNotificationsMarkAsViewed && bNotificationCommentAdded)
		NotificationMarkAsViewed();

	vkNotification.destroy();
	vkUsers.destroy();
}

void CVkProto::RetrieveUnreadEvents()
{
	debugLogA("CVkProto::RetrieveUnreadEvents");
	if (!IsOnline() || (!m_bNotificationsEnabled && !m_bNewsEnabled))
		return;

	time_t tLastNotificationsTime = getDword("LastNotificationsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNotificationsTime - m_iNotificationsInterval * 60 >= -3 && m_bNotificationsEnabled)
		RetrieveUnreadNotifications(tLastNotificationsTime);

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNewsTime - m_iNewsInterval * 60 >= -3 && m_bNewsEnabled)
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