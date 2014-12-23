/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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

static char* szImageTypes[] = { "photo_2560", "photo_1280", "photo_807", "photo_604", "photo_256", "photo_130", "photo_128", "photo_75", "photo_64" };

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
		CMString  tszUrl = _T("https://vk.com/press/Simple.png");
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
		if (iUserId){
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
		for (size_t i = 0; (pProfile = json_at(pProfiles, i)) != NULL; i++){
			LONG UserId = json_as_int(json_get(pProfile, "id"));
			if (!UserId)
				continue;

			CMString tszNick = json_as_string(json_get(pProfile, "first_name"));
			tszNick.AppendChar(' ');
			tszNick += json_as_string(json_get(pProfile, "last_name"));
			CMString tszLink = _T("https://vk.com/");
			tszLink += json_as_string(json_get(pProfile, "screen_name"));
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, false, tszNick, tszLink, FindUser(UserId));
			vkUsers.insert(vkUser);
		}
	
	JSONNODE *pGroups = json_get(pResponse, "groups");
	if (pGroups != NULL)		
		for (size_t i = 0; (pProfile = json_at(pGroups, i)) != NULL; i++){
			LONG UserId = -json_as_int(json_get(pProfile, "id"));
			if (!UserId)
				continue;

			CMString tszNick = json_as_string(json_get(pProfile, "name"));
			CMString tszLink = _T("https://vk.com/");
			tszLink += json_as_string(json_get(pProfile, "screen_name"));
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, true, tszNick, tszLink);
			vkUsers.insert(vkUser);
		}
}

CMString CVkProto::GetVkPhotoItem(JSONNODE *pPhoto)
{
	CMString tszRes;

	if (pPhoto == NULL)
		return tszRes;

	ptrT ptszLink;
	for (int i = 0; i < SIZEOF(szImageTypes); i++) {
		JSONNODE *n = json_get(pPhoto, szImageTypes[i]);
		if (n != NULL) {
			ptszLink = json_as_string(n);
			break;
		}
	}

	int iWidth = json_as_int(json_get(pPhoto, "width"));
	int iHeight = json_as_int(json_get(pPhoto, "height"));

	tszRes.AppendFormat(_T("%s: %s (%dx%d)"), TranslateT("Photo"), ptszLink ? ptszLink : _T(""), iWidth, iHeight);
	if (m_bAddImgBbc)
		tszRes.AppendFormat(_T("\n\t[img]%s[/img]"), ptszLink);
	CMString tszText = json_as_string(json_get(pPhoto, "text"));
	if (!tszText.IsEmpty())
		tszRes += "\n" + tszText;
	
	return tszRes;
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
	CMString tszText = json_as_string(json_get(pItem, "text"));

	vkNewsItem->tszType = json_as_string(json_get(pItem, "type"));
	vkNewsItem->vkUser = GetVkUserInfo(iSourceId, vkUsers);
	vkNewsItem->bIsGroup = vkNewsItem->vkUser->m_bIsGroup;
	vkNewsItem->tDate = json_as_int(json_get(pItem, "date"));

	if (!tszText.IsEmpty())
		tszText += _T("\n");

	debugLog(_T("CVkProto::GetVkNewsItem %d %d %s <%s>"), iSourceId, iPostId, vkNewsItem->tszType.GetBuffer(), tszText.GetBuffer());

	if (vkNewsItem->tszType == _T("photo_tag")){
		bPostLink = false;
		JSONNODE *pPhotos = json_get(pItem, "photo_tags");
		if (pPhotos){			
			JSONNODE *pPhotoItems = json_get(pPhotos, "items");
			if (pPhotoItems){
				JSONNODE *pPhotoItem;
				tszText = TranslateT("User was tagged in these photos:");
				for (size_t i = 0; (pPhotoItem = json_at(pPhotoItems, i)) != NULL; i++)
					tszText += _T("\n") + GetVkPhotoItem(pPhotoItem);
			}
		}
	}
	else if (vkNewsItem->tszType == _T("photo") || vkNewsItem->tszType == _T("wall_photo")){
		bPostLink = false;
		JSONNODE *pPhotos = json_get(pItem, "photos");
		if (pPhotos){
			JSONNODE *pPhotoItems = json_get(pPhotos, "items"), *pPhotoItem;
			if (pPhotoItems)		
				for (size_t i = 0; (pPhotoItem = json_at(pPhotoItems, i)) != NULL; i++){					
					tszText += GetVkPhotoItem(pPhotoItem) + _T("\n");
					if (i == 0 && vkNewsItem->tszType == _T("wall_photo")){
						LONG iPhotoPostId = json_as_int(json_get(pPhotoItem, "post_id"));
						if (iPhotoPostId){
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
			tszText += GetAttachmentDescr(pAttachments);
	}

	CMString tszResFormat;	
	CMString tszBBCIn = m_bBBCOnNews ? _T("[b]") : _T("");
	CMString tszBBCOut = m_bBBCOnNews ? _T("[/b]") : _T("");

	if (!isRepost)
		tszResFormat = Translate("News from %s%s%s (%s)\n%s");
	else {
		tszResFormat = Translate("\tRepost from %s%s%s (%s)\n%s");
		bPostLink = false;
	}
		
	vkNewsItem->tszText.AppendFormat(tszResFormat, tszBBCIn.GetBuffer(), vkNewsItem->vkUser->m_tszUserNick.GetBuffer(), tszBBCOut.GetBuffer(),
		vkNewsItem->vkUser->m_tszLink.GetBuffer(), tszText.GetBuffer());
	
	vkNewsItem->tszId.AppendFormat(_T("%d_%d"), vkNewsItem->vkUser->m_UserId, iPostId);
	if (bPostLink) {
		vkNewsItem->tszLink = CMString(_T("https://vk.com/wall")) + vkNewsItem->tszId;
		vkNewsItem->tszText.AppendFormat(TranslateT("\nNews link: %s"), vkNewsItem->tszLink.GetBuffer());
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


	CMString tszBBCIn = m_bBBCOnNews ? _T("[b]") : _T("");
	CMString tszBBCOut = m_bBBCOnNews ? _T("[/b]") : _T("");
	CMString tszFormat;
	LONG iUserId = 0;

	if (vkFeedbackType == vkComment) {
		iUserId = json_as_int(json_get(pFeedback, "from_id"));
		tszFormat = _T("%s%s%s (%s) %%s %%s\n%s");		
	}
	else if (vkFeedbackType == vkPost) {		
		iUserId = json_as_int(json_get(pFeedback, "owner_id "));
		tszFormat = _T("%s%s%s (%s) %%s %%s\n%s");		
	}
	else if (vkFeedbackType == VKObjType::vkUsers || vkFeedbackType == vkCopy){
		JSONNODE *pUsers = json_get(pFeedback, "items"), *pUserItem;
		
		CMString tszUsers;
		for (int i = 0; (pUserItem = json_at(pUsers, i)) != NULL; i++){
			iUserId = json_as_int(json_get(pUserItem, "from_id"));
			if (iUserId == 0)
				continue;
			vkUser = GetVkUserInfo(iUserId, vkUsers);
			if (!tszUsers.IsEmpty())
				tszUsers += _T(", ");
			tszUsers.AppendFormat(_T("%s%s%s (%s)"), tszBBCIn.GetBuffer(), vkUser->m_tszUserNick.GetBuffer(), tszBBCOut.GetBuffer(), vkUser->m_tszLink.GetBuffer());
		}
		tszRes.AppendFormat(_T("%s %%s %%s"), tszUsers.GetBuffer());
		vkUser = NULL;
		iUserId = 0;
	}

	if (iUserId){
		vkUser = GetVkUserInfo(iUserId, vkUsers);
		CMString tszText = json_as_string(json_get(pFeedback, "text"));
		
		size_t iNameEnd = tszText.Find(_T("],")), iNameBeg = tszText.Find(_T("|"));
		if (iNameEnd != -1 && iNameBeg != -1 && iNameBeg < iNameEnd){
			CMString tszName = tszText.Mid(iNameBeg + 1, iNameEnd - iNameBeg - 1);
			CMString tszBody = tszText.Mid(iNameEnd + 2);
			if (!tszName.IsEmpty() && !tszBody.IsEmpty())
				tszText = tszName + _T(",") + tszBody;
		}
		
		tszRes.AppendFormat(tszFormat, tszBBCIn.GetBuffer(), vkUser->m_tszUserNick.GetBuffer(), tszBBCOut.GetBuffer(), vkUser->m_tszLink.GetBuffer(), tszText.GetBuffer());
	}

	return tszRes;
}

CMString CVkProto::GetVkParent(JSONNODE *pParent, VKObjType vkParentType)
{
	debugLogA("CVkProto::GetVkParent");
	CMString tszRes;
	if (!pParent || !vkParentType)
		return tszRes;
	
	CMString tszBBCIn = m_bBBCOnNews ? _T("[b]") : _T("");
	CMString tszBBCOut = m_bBBCOnNews ? _T("[/b]") : _T("");
	
	if (vkParentType == vkPhoto) {
		CMString tszPhoto = GetVkPhotoItem(pParent);
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszFormat = _T("\n%s\n%s%s%s: https://vk.com/photo%d_%d");
		tszRes.AppendFormat(tszFormat, tszPhoto.GetBuffer(), tszBBCIn.GetBuffer(), TranslateT("Link"), tszBBCOut.GetBuffer(), iOwnerId,  iId);
	}
	else if (vkParentType == vkVideo) {
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszTitle = json_as_string(json_get(pParent, "title"));

		CMString tszFormat = _T("\n%s%s%s\n%s%s%s: https://vk.com/video%d_%d");
		tszRes.AppendFormat(tszFormat, tszBBCIn.GetBuffer(), tszTitle.GetBuffer(), tszBBCOut.GetBuffer(), tszBBCIn.GetBuffer(), TranslateT("Link"), tszBBCOut.GetBuffer(), iOwnerId, iId);
	}
	else if (vkParentType == vkPost) {
		LONG iOwnerId = json_as_int(json_get(pParent, "from_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszFormat = _T("\n%s%s%s: https://vk.com/wall%d_%d");
		tszRes.AppendFormat(tszFormat, tszBBCIn.GetBuffer(), TranslateT("Link"), tszBBCOut.GetBuffer(), iOwnerId, iId);
	}
	else if (vkParentType == vkTopic) {
		LONG iOwnerId = json_as_int(json_get(pParent, "owner_id"));
		LONG iId = json_as_int(json_get(pParent, "id"));
		CMString tszTitle = json_as_string(json_get(pParent, "title"));

		CMString tszFormat = _T("%s%s%s\n%s%s%s: https://vk.com/topic%d_%d");	
		tszRes.AppendFormat(tszFormat, tszBBCIn.GetBuffer(), tszTitle.GetBuffer(), tszBBCOut.GetBuffer(), tszBBCIn.GetBuffer(), TranslateT("Link"), tszBBCOut.GetBuffer(), iOwnerId, iId);
	}
	else if (vkParentType == vkComment) {
		JSONNODE *pNode = json_get(pParent, "photo");
		if (pNode)
			return GetVkParent(pNode, vkPhoto);

		pNode = json_get(pParent, "video");
		if (pNode)
			return GetVkParent(pNode, vkVideo);

		LONG iId = json_as_int(json_get(pParent, "id"));

		pNode = json_get(pParent, "post");
		if (pNode){
			tszRes = GetVkParent(pNode, vkPost);
			tszRes.AppendFormat(_T("?reply=%d"), iId);
			return tszRes;
		}

		pNode = json_get(pParent, "topic");
		if (pNode){
			tszRes = GetVkParent(pNode, vkTopic);
			tszRes.AppendFormat(_T("?reply=%d"), iId);
			return tszRes;
		}
	}

	return tszRes;
}

CMString CVkProto::GetVkNotificationsItem(JSONNODE *pItem, OBJLIST<CVkUserInfo> &vkUsers, time_t &tDate)
{
	debugLogA("CVkProto::GetVkNotificationsItem");
	CMString tszRes;
	if (pItem == NULL)
		return tszRes;

	CMString tszType = json_as_string(json_get(pItem, "type"));
	tDate = json_as_int(json_get(pItem, "date"));

	VKObjType vkFeedbackType = vkNull, vkParentType = vkNull;
	CMString tszNotificationTranslate = SpanVKNotificationType(tszType, vkFeedbackType, vkParentType);
		
	JSONNODE *pFeedback = json_get(pItem, "feedback");
	if (!pFeedback)
		return tszRes;
	CVkUserInfo *vkUser = NULL;
	CMString tszFeedback = GetVkFeedback(pFeedback, vkFeedbackType, vkUsers, vkUser);

	JSONNODE *pParent = json_get(pItem, "parent");
	if (!pParent)
		return tszRes;
	CMString tszParent = GetVkParent(pParent, vkParentType);
	if (!tszParent.IsEmpty() && !tszFeedback.IsEmpty())
		tszRes.AppendFormat(tszFeedback, tszNotificationTranslate.GetBuffer(), tszParent.GetBuffer());
	return tszRes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveUnreadNews(time_t tLastNewsTime)
{
	debugLogA("CVkProto::RetrieveUnreadNews");
	if (!IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/newsfeed.get.json", true, &CVkProto::OnReceiveUnreadNews)
		<< INT_PARAM("count", 100)
		<< INT_PARAM("return_banned", 0)
		<< INT_PARAM("max_photos", 5)
		<< INT_PARAM("start_time", tLastNewsTime + 1)
		<< CHAR_PARAM("filters", "post,photo,photo_tag,wall_photo")
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
		for (int i = 0; (pItem = json_at(pItems, i)) != NULL; i++){	
			CVKNewsItem *vkNewsItem = GetVkNewsItem(pItem, vkUsers);
			if (vkNews.find(vkNewsItem) == NULL)
				vkNews.insert(vkNewsItem);
			else
				delete vkNewsItem;
		}

	for (int i = 0; i < vkNews.getCount(); i++)
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
		<< CHAR_PARAM("filters", "comments,likes,reposts")
		<< VER_API);
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

	if (pItems != NULL)
		for (int i = 0; (pItem = json_at(pItems, i)) != NULL; i++){
			time_t tDate;
			CMString tszText = GetVkNotificationsItem(pItem, vkUsers, tDate);
			AddFeedEvent(tszText, tDate);
		}

	setDword("LastNotificationsTime", time(NULL));
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

}

INT_PTR CVkProto::SvcLoadVKNews(WPARAM, LPARAM)
{
	if (!IsOnline())
		return 1;

	if (!m_bNewsEnabled && !m_bNotificationsEnabled){
		m_bSpecialContactAlwaysEnabled = true; 
		AddFeedSpecialUser();
	}

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	RetrieveUnreadNews(tLastNewsTime);

	return 0;
}