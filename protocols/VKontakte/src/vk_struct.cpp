/*
Copyright (c) 2013-19 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////

ULONG AsyncHttpRequest::m_reqCount = 0;

AsyncHttpRequest::AsyncHttpRequest()
{
	m_bApiReq = true;
	AddHeader("Connection", "keep-alive");
	pUserInfo = nullptr;
	m_iRetry = MAX_RETRIES;
	m_iErrorCode = 0;
	bNeedsRestart = false;
	bIsMainConn = false;
	m_pFunc = nullptr;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpLow;
}

AsyncHttpRequest::AsyncHttpRequest(CVkProto *ppro, int iRequestType, LPCSTR _url, bool bSecure, MTHttpRequestHandler pFunc, RequestPriority rpPriority)
{
	m_bApiReq = true;
	bIsMainConn = false;
	AddHeader("Connection", "keep-alive");

	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = ((bSecure) ? "https://" : "http://") + CMStringA("api.vk.com");
		m_szUrl += _url;
		bIsMainConn = true;
	}
	else
		m_szUrl = _url;

	flags = VK_NODUMPHEADERS | NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (bSecure) {
		flags |= NLHRF_SSL;
		this << CHAR_PARAM("access_token", ppro->m_szAccessToken);
	}

	requestType = iRequestType;
	m_pFunc = pFunc;
	pUserInfo = nullptr;
	m_iRetry = MAX_RETRIES;
	m_iErrorCode = 0;
	bNeedsRestart = false;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpPriority;
}

void AsyncHttpRequest::Redirect(NETLIBHTTPREQUEST *nhr)
{
	for (int i = 0; i < nhr->headersCount; i++) {
		LPCSTR szValue = nhr->headers[i].szValue;
		if (!_stricmp(nhr->headers[i].szName, "Location"))
			m_szUrl = szValue;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkFileUploadParam::CVkFileUploadParam(MCONTACT _hContact, const wchar_t *_desc, wchar_t **_files) :
	hContact(_hContact),
	Desc(mir_wstrdup(_desc)),
	FileName(mir_wstrdup(_files[0])),
	atr(nullptr),
	fname(nullptr),
	filetype(typeInvalid)
{}

CVkFileUploadParam::~CVkFileUploadParam()
{
	mir_free(Desc);
	mir_free(FileName);
	mir_free(atr);
	mir_free(fname);
}

CVkFileUploadParam::VKFileType CVkFileUploadParam::GetType()
{
	if (filetype != typeInvalid)
		return filetype;

	if (atr)
		mir_free(atr);
	if (fname)
		mir_free(fname);

	wchar_t img[] = L".jpg .jpeg .png .bmp";
	wchar_t audio[] = L".mp3";
	wchar_t audiomsg[] = L".ogg";

	wchar_t DRIVE[3], DIR[256], FNAME[256], EXT[256];
	_wsplitpath(FileName, DRIVE, DIR, FNAME, EXT);

	T2Utf pszFNAME(FNAME), pszEXT(EXT);
	CMStringA fn(FORMAT, "%s%s", pszFNAME, pszEXT);
	fname = mir_strdup(fn);

	if (wlstrstr(img, EXT)) {
		filetype = CVkFileUploadParam::typeImg;
		atr = mir_strdup("photo");
	}
	else if (wlstrstr(audio, EXT)) {
		filetype = CVkFileUploadParam::typeAudio;
		atr = mir_strdup("file");
	}
	else if (wlstrstr(audiomsg, EXT)) {
		filetype = CVkFileUploadParam::typeAudioMsg;
		atr = mir_strdup("file");
	}
	else {
		filetype = CVkFileUploadParam::typeDoc;
		atr = mir_strdup("file");
	}

	return filetype;
}

/////////////////////////////////////////////////////////////////////////////////////////

CVkChatUser* CVkChatInfo::GetUserById(LPCWSTR pwszId)
{
	int user_id = _wtoi(pwszId);
	return m_users.find((CVkChatUser*)&user_id);
}

CVkChatUser* CVkChatInfo::GetUserById(int user_id)
{
	return m_users.find((CVkChatUser*)&user_id);
}

/////////////////////////////////////////////////////////////////////////////////////////

CVKOptions::CVKOptions(PROTO_INTERFACE *proto) :
	bLoadLastMessageOnMsgWindowsOpen(proto, "LoadLastMessageOnMsgWindowsOpen", true),
	bLoadOnlyFriends(proto, "LoadOnlyFriends", false),
	bServerDelivery(proto, "BsDirect", true),
	bHideChats(proto, "HideChats", true),
	bMesAsUnread(proto, "MesAsUnread", false),
	bUseLocalTime(proto, "UseLocalTime", false),
	bReportAbuse(proto, "ReportAbuseOnBanUser", false),
	bClearServerHistory(proto, "ClearServerHistoryOnBanUser", false),
	bRemoveFromFrendlist(proto, "RemoveFromFrendlistOnBanUser", false),
	bRemoveFromCList(proto, "RemoveFromClistOnBanUser", false),
	bPopUpSyncHistory(proto, "PopUpSyncHistory", false),
	iMarkMessageReadOn(proto, "MarkMessageReadOn", MarkMsgReadOn::markOnRead),
	bStikersAsSmyles(proto, "StikersAsSmyles", false),
	bUserForceInvisibleOnActivity(proto, "UserForceOnlineOnActivity", false),
	bNewsEnabled(proto, "NewsEnabled", false),
	iMaxLoadNewsPhoto(proto, "MaxLoadNewsPhoto", 5),
	bNotificationsEnabled(proto, "NotificationsEnabled", false),
	bNotificationsMarkAsViewed(proto, "NotificationsMarkAsViewed", true),
	bSpecialContactAlwaysEnabled(proto, "SpecialContactAlwaysEnabled", false),
	bUseBBCOnAttacmentsAsNews(proto, "UseBBCOnAttacmentsAsNews", true),
	bNewsAutoClearHistory(proto, "NewsAutoClearHistory", false),
	bNewsFilterPosts(proto, "NewsFilterPosts", true),
	bNewsFilterPhotos(proto, "NewsFilterPhotos", true),
	bNewsFilterTags(proto, "NewsFilterTags", true),
	bNewsFilterWallPhotos(proto, "NewsFilterWallPhotos", true),
	bNewsSourceFriends(proto, "NewsSourceFriends", true),
	bNewsSourceGroups(proto, "NewsSourceGroups", true),
	bNewsSourcePages(proto, "NewsSourcePages", true),
	bNewsSourceFollowing(proto, "NewsSourceFollowing", true),
	bNewsSourceIncludeBanned(proto, "NewsSourceIncludeBanned", false),
	bNewsSourceNoReposts(proto, "NewsSourceNoReposts", false),
	bNotificationFilterComments(proto, "NotificationFilterComments", true),
	bNotificationFilterLikes(proto, "NotificationFilterLikes", true),
	bNotificationFilterReposts(proto, "NotificationFilterReposts", true),
	bNotificationFilterMentions(proto, "NotificationFilterMentions", true),
	bNotificationFilterInvites(proto, "NotificationFilterInvites", true),
	bNotificationFilterAcceptedFriends(proto, "NotificationFilterAcceptedFriends", true),

	bSendVKLinksAsAttachments(proto, "SendVKLinksAsAttachments", true),
	bLoadSentAttachments(proto, "LoadSentAttachments", bSendVKLinksAsAttachments),
	bUseNonStandardNotifications(proto, "UseNonStandardNotifications", false),
	bShortenLinksForAudio(proto, "ShortenLinksForAudio", true),
	bAddMessageLinkToMesWAtt(proto, "AddMessageLinkToMesWAtt", true),
	bSplitFormatFwdMsg(proto, "SplitFormatFwdMsg", true),
	bSyncReadMessageStatusFromServer(proto, "SyncReadMessageStatusFromServer", false),
	bLoadFullCList(proto, "LoadFullCList", false),
	bShowVkDeactivateEvents(proto, "ShowVkDeactivateEvents", true),
	bStickerBackground(proto, "StickerBackground", false),

	bShowProtoMenuItem0(proto, "ShowProtoMenuItem0", true),
	bShowProtoMenuItem1(proto, "ShowProtoMenuItem1", true),
	bShowProtoMenuItem2(proto, "ShowProtoMenuItem2", true),
	bShowProtoMenuItem3(proto, "ShowProtoMenuItem3", true),
	bShowProtoMenuItem4(proto, "ShowProtoMenuItem4", true),
	bShowProtoMenuItem5(proto, "ShowProtoMenuItem5", true),
	bShowProtoMenuItem6(proto, "ShowProtoMenuItem6", true),

	iMusicSendMetod(proto, "MusicSendMetod", MusicSendMetod::sendBroadcastOnly),
	bPopupContactsMusic(proto, "PopupContactsMusic", false),
	iSyncHistoryMetod(proto, "SyncHistoryMetod", SyncHistoryMetod::syncOff),
	iIMGBBCSupport(proto, "IMGBBCSupport", IMGBBCSypport::imgNo),
	iBBCForNews(proto, "BBCForNews", BBCSupport::bbcBasic),
	iBBCForAttachments(proto, "BBCForAttachments", BBCSupport::bbcBasic),

	iNewsInterval(proto, "NewsInterval", 15),
	iNotificationsInterval(proto, "NotificationsInterval", 1),
	iNewsAutoClearHistoryInterval(proto, "NewsAutoClearHistoryInterval", 60 * 60 * 24 * 3),
	iInvisibleInterval(proto, "InvisibleInterval", 10),
	iMaxFriendsCount(proto, "MaxFriendsCount", 1000),
	iStickerSize(proto, "StickerSize", 128),

	pwszDefaultGroup(proto, "ProtoGroup", L"VKontakte"),
	pwszReturnChatMessage(proto, "ReturnChatMessage", TranslateT("I'm back")),
	pwszVKLang(proto, "VKLang", nullptr)

{
	// Note: Delete this code after next stable build
	int iAutoClean = db_get_b(0, proto->m_szModuleName, "AutoClean", -1);
	if (iAutoClean != -1) {
		bLoadOnlyFriends = (BYTE)iAutoClean;
		db_set_b(0, proto->m_szModuleName, "LoadOnlyFriends", bLoadOnlyFriends);
		db_unset(0, proto->m_szModuleName, "AutoClean");
	}
	// Note
}



////////////////////////////////////////////////////////////////////////////////////////////////////

void CVkUserItem::LoadFromUserProfile(const JSONNode &jnNode)
{
	if (!jnNode)
		return;

	Set(m_bIsGroup, false);
	Set(m_bIsUser, true);
	Set(m_bIsMUChat, false);
	Set(m_bIsEmail, false);

	Set(m_iUserId, jnNode["id"].as_int());
	Set(m_wszFirstName, jnNode["first_name"].as_mstring());
	Set(m_wszLastName, jnNode["last_name"].as_mstring());

	Set(m_bIsFriend, jnNode["is_friend"].as_bool());

	Set(m_wszUserNick, m_wszFirstName + L" " + m_wszLastName);

	m_bIsDeactivated = !Set(m_wszDeactivated, jnNode["deactivated"].as_mstring()).IsEmpty();

	Set(m_btGender, BYTE(jnNode["sex"].as_int() == 2 ? 'M' : 'F'));
	Set(m_btTimezone, BYTE(jnNode["timezone"].as_int() * -2));

	Set(m_wszBDate, jnNode["bdate"].as_mstring());
	Set(m_wszAvatarUrl, jnNode["photo_100"].as_mstring());

	Set(m_wszCellular, jnNode["mobile_phone"].as_mstring());
	Set(m_wszPhone, jnNode["home_phone"].as_mstring());

	const JSONNode &jnLastSeen = jnNode["last_seen"];
	if (jnLastSeen)
		Set(m_iLastSeen, jnLastSeen["time"].as_int());

	Set(m_iStatus, LONG((jnNode["online"].as_int() == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE));

	const JSONNode &jnCountry = jnNode["country"];
	if (jnCountry)
		Set(m_wszCountry, jnCountry["title"].as_mstring());

	const JSONNode &jnCity = jnNode["city"];
	if (jnCity)
		Set(m_wszCity, jnCity["title"].as_mstring());

	BYTE cMaritalStatus[] = { 0, 10, 11, 12, 20, 70, 50, 60, 80 };

	if (jnNode["relation"] && jnNode["relation"].as_int() < _countof(cMaritalStatus))
		Set(m_btMaritalStatus, cMaritalStatus[jnNode["relation"].as_int()]);

	Set(m_wszAbout, jnNode["about"].as_mstring());
	Set(m_wszDomain, jnNode["domain"].as_mstring());
}


void CVkUserItem::LoadFromConversation(const JSONNode &jnNode)
{
	if (!jnNode)
		return;

	Set(m_bIsGroup, false);
	Set(m_bIsUser, false);
	Set(m_bIsMUChat, false);
	Set(m_bIsEmail, false);

	Set(m_bIsFriend, false);

	const JSONNode &jnPeer = jnNode["peer"];

	CMStringW wszConversationType(jnPeer["type"].as_mstring());

	if (wszConversationType.IsEmpty())
		return;

	if (wszConversationType == L"user") {
		Set(m_bIsUser, true);
		Set(m_iUserId, jnPeer["id"].as_int());
	}
	else if (wszConversationType == L"chat") {
		Set(m_bIsMUChat, true);
		Set(m_iUserId, jnPeer["id"].as_int());
		Set(m_iChatId, jnPeer["local_id"].as_int());

		const JSONNode &jnChatS = jnNode["chat_settings"];

		Set(m_wszUserNick, jnChatS["title"].as_mstring());
		Set(m_iAdminId, jnChatS["owner_id"].as_int());
	}
	else if (wszConversationType == L"group") {
		Set(m_bIsGroup, true);
		Set(m_iUserId, jnPeer["id"].as_int());
	}

	Set(m_iLastMsgId, jnNode["last_message_id"].as_int());
}

void CVkUserItem::LoadFromGroup(const JSONNode & jnNode)
{
	if (!jnNode)
		return;

	Set(m_bIsGroup, true);
	Set(m_bIsUser, false);
	Set(m_bIsMUChat, false);
	Set(m_bIsEmail, false);

	Set(m_iUserId, -1 * jnNode["id"].as_int());

	Set(m_bIsFriend, jnNode["is_member"].as_bool());
	Set(m_wszUserNick, jnNode["name"].as_mstring());

	Set(m_wszAvatarUrl, jnNode["photo_100"].as_mstring());

	Set(m_wszDomain, jnNode["screen_name"].as_mstring());
}

MCONTACT CVkUserItem::WriteToDB(bool bForce, VKContactType vkContactType)
{
	if (!bForce && !m_bIsUpdated)
		return m_hContact;

// Work in process...

//	LONG m_iUserId;
//	LONG m_iChatId;
//	LONG m_iAdminId;

//	LONG m_iStatus;
//	LONG m_iLastSeen;
//	LONG m_iLastMsgId;

//	MCONTACT m_hContact;

//	CMStringW m_wszFirstName;
//	CMStringW m_wszLastName;
//	CMStringW m_wszUserNick;
//	CMStringW m_wszLink;
//	CMStringW m_wszDeactivated;
//	CMStringW m_wszCellular;
//	CMStringW m_wszPhone;
//	CMStringW m_wszCountry;
//	CMStringW m_wszCity;
//	CMStringW m_wszAbout;
//	CMStringW m_wszDomain;
//	CMStringW m_wszBDate;
//	CMStringW m_wszAvatarUrl;

//	BYTE m_btGender;
//	BYTE m_btTimezone;
//	BYTE m_btMaritalStatus;

//	bool m_bIsGroup;
//	bool m_bIsFriend;
//	bool m_bIsUser;
//	bool m_bIsEmail;
//	bool m_bIsMUChat;
//	bool m_bIsDeactivated;

//	bool m_bIsHidden;
//	bool m_bIsUpdated;

	return m_hContact;
}

CVkUserItem::CVkUserItem(CVkProto* vkProto, const JSONNode &jnNode, vkJSONNodeType vkJSONType)
{
	m_VK = vkProto;
	m_iUserId = 0;

	if (!jnNode)
		return;

	if (vkJSONTypeProfile == vkJSONType)
		LoadFromUserProfile(jnNode);
	else if (vkJSONTypeConversation == vkJSONType)
		LoadFromConversation(jnNode);
	else if (vkJSONTypeGroup == vkJSONType)
		LoadFromGroup(jnNode);


	m_hContact = m_bIsMUChat ? m_VK->FindChat(m_iChatId) : m_VK->FindUser(m_iUserId);
	m_hContact = m_hContact ? m_hContact : INVALID_CONTACT_ID;
}
