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

#define VK_EVENTTYPE_FEED 10001
static char* szImageTypes[] = { "photo_2560", "photo_1280", "photo_807", "photo_604", "photo_256", "photo_130", "photo_128", "photo_75", "photo_64" };

void CVkProto::AddFeedSpecialUser()
{
	MCONTACT hContact = FindUser(VK_FEED_USER, m_bNewsEnabled);
	if (!m_bNewsEnabled) {
		if (hContact)
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		return;
	}

	CMString tszNick = TranslateT("VKontakte");		
	setTString(hContact, "Nick", tszNick.GetBuffer());
	CMString  url = _T("https://vk.com/press/Simple.png");
	SetAvatarUrl(hContact, url);
	ReloadAvatarInfo(hContact);
	
	if (getWord(hContact, "Status", 0) != ID_STATUS_ONLINE)
		setWord(hContact, "Status", ID_STATUS_ONLINE);
	SetMirVer(hContact, 7);

	setTString(hContact, "domain", _T("feed"));
	setTString(hContact, "Homepage", _T("https://vk.com/feed"));
}

void CVkProto::AddFeedEvent(CMString& tszBody, time_t tTime)
{
	
	MCONTACT hContact = FindUser(VK_FEED_USER, true);
	ptrT ptszBody;
	PROTORECVEVENT recv = { 0 };

	ptszBody = mir_tstrdup(tszBody.GetBuffer());
	recv.flags = PREF_TCHAR;
	recv.timestamp = tTime;
	recv.tszMessage = ptszBody;
	recv.lParam = 0;
	recv.pCustomData = NULL;
	recv.cbCustomDataSize = 0;

	ProtoChainRecvMsg(hContact, &recv);
}

void CVkProto::RetrieveUnreadNews()
{
	debugLogA("CVkProto::RetrieveUnreadNews");
	if (!IsOnline() || !m_bNewsEnabled)
		return;

	time_t tLastNewsTime = getDword("LastNewsTime", time(NULL) - 24 * 60 * 60);
	if (time(NULL) - tLastNewsTime <= m_iNewsInterval * 60 )
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/newsfeed.get.json", true, &CVkProto::OnReceiveUnreadNews)
		<< INT_PARAM("count", 100)
		<< INT_PARAM("return_banned", 0)
		<< INT_PARAM("max_photos", 100)
		<< INT_PARAM("start_time", tLastNewsTime + 1)
		<< CHAR_PARAM("filters", "post,photo,photo_tag,wall_photo")
		<< VER_API);
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
			CVkUserInfo * vkUser = new CVkUserInfo(UserId, false, tszNick, tszLink);
			debugLog(_T("CVkProto::CreateVkUserInfoList %d %d %s %s"), UserId, false, tszNick.GetBuffer(), tszLink.GetBuffer());
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
			debugLog(_T("CVkProto::CreateVkUserInfoList %d %d %s %s"), UserId, true, tszNick.GetBuffer(), tszLink.GetBuffer());
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
	tszRes.AppendFormat(_T("%s: %s (%dx%d)"), TranslateT("Photo"), ptszLink, iWidth, iHeight);
	if (m_bAddImgBbc)
		tszRes.AppendFormat(_T("\n\t[img]%s[/img]"), ptszLink);
	CMString tszText = json_as_string(json_get(pPhoto, "text"));
	if (!tszText.IsEmpty())
		tszRes += "\n" + tszText;
	
	return tszRes;
}

CMString CVkProto::GetVkNewsItem(JSONNODE *pItem, OBJLIST<CVkUserInfo> &vkUsers, time_t &tDate)
{
	debugLogA("CVkProto::GetVkNewsItem");
	bool m_bBBCOnNews = true; //!!!
	bool bPostLink = true;
	CMString tszRes;
	if (pItem == NULL)
		return tszRes;

	CMString tszType = json_as_string(json_get(pItem, "type"));

	LONG iSourceId = json_as_int(json_get(pItem, "source_id"));
	iSourceId = iSourceId ? iSourceId : json_as_int(json_get(pItem, "owner_id"));

	bool bIsGroup = (iSourceId < 0);
	CVkUserInfo * vkUser = vkUsers.find((CVkUserInfo *)&iSourceId);

	if (vkUser == NULL) {
		CMString tszNick = TranslateT("Unknown");
		CMString tszLink = _T("https://vk.com/");
		if (iSourceId){
			tszLink += bIsGroup ? "club" : "id";
			tszLink.AppendFormat(_T("%d"), bIsGroup ? -iSourceId : iSourceId);
		}
		vkUser = new CVkUserInfo(iSourceId, bIsGroup, tszNick, tszLink);
		vkUsers.insert(vkUser);
	}

	LONG iPostId = json_as_int(json_get(pItem, "post_id"));
	tDate = json_as_int(json_get(pItem, "date"));
	CMString tszText = json_as_string(json_get(pItem, "text"));
	if (!tszText.IsEmpty())
		tszText += _T("\n");

	if (tszType == _T("photo_tag")){
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
	else if (tszType == _T("photo") || tszType == _T("wall_photo")){
		bPostLink = false;
		JSONNODE *pPhotos = json_get(pItem, "photos");
		if (pPhotos){
			JSONNODE *pPhotoItems = json_get(pPhotos, "items");
			if (pPhotoItems){
				JSONNODE *pPhotoItem;				
				for (size_t i = 0; (pPhotoItem = json_at(pPhotoItems, i)) != NULL; i++){
					
					tszText += GetVkPhotoItem(pPhotoItem) + _T("\n");
					if (i == 0 && tszType == _T("wall_photo")){
						iPostId = json_as_int(json_get(pPhotoItem, "post_id"));
						bPostLink = true;
						break; // Max 1 wall_photo
					}
				}
			}
		}
	} else if (tszType == _T("post") || tszType.IsEmpty()) {
		bPostLink = true;
		JSONNODE * pRepost = json_get(pItem, "copy_history");
		if (pRepost){
			time_t tRDate;
			CMString tszRepostText = GetVkNewsItem(json_at(pRepost, 0), vkUsers, tRDate);		
			tszRepostText.Replace(_T("\n"), _T("\n\t"));
			tszText += tszRepostText;
			tszText += _T("\n");
		}

		JSONNODE *pAttachments = json_get(pItem, "attachments");
		if (pAttachments)
			tszText += GetAttachmentDescr(pAttachments);
	}

	CMString tszResFormat;	
	CMString tszBBCIn, tszBBCOut;
	CMString tszUrl;
	
	if (iPostId)
		tszResFormat = Translate("News from %s%s%s (%s)\n%s");
	else {
		tszResFormat = Translate("\tRepost from %s%s%s (%s)\n%s");
		bPostLink = false;
	}

	tszBBCIn = m_bBBCOnNews ? "[b]" : "[";
	tszBBCOut = m_bBBCOnNews ? "[/b]" : "]";
	tszRes.AppendFormat(tszResFormat, tszBBCIn.GetBuffer(), vkUser->m_tszUserNick.GetBuffer(), tszBBCOut.GetBuffer(), vkUser->m_tszLink.GetBuffer(), tszText.GetBuffer()); 
	
	if (bPostLink) {
		tszUrl.AppendFormat(_T("%d_%d"), vkUser->m_UserId, iPostId);
		tszUrl = CMString(_T("https://vk.com/wall")) + tszUrl;
		tszRes.AppendFormat(TranslateT("\nNews link: %s"), tszUrl.GetBuffer());
	}

	return tszRes;
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

	if (pItems != NULL)
		for (int i = 0; (pItem = json_at(pItems, i)) != NULL; i++){
			time_t tDate;	
			CMString tszText = GetVkNewsItem(pItem, vkUsers, tDate);
			AddFeedEvent(tszText, tDate);
		}
		

	setDword("LastNewsTime", time(NULL));
	vkUsers.destroy();
}
