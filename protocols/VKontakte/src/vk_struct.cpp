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
	if (auto *pszHdr = Netlib_GetHeader(nhr, "Location"))
		m_szUrl = pszHdr;
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

CVKOptions::CVKOptions(PROTO_INTERFACE* proto) :
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
	bStikersAsSmileys(proto, "StikersAsSmileys", false),
	bUseStikersAsStaticSmileys(proto, "UseStikersAsStaticSmileys", false),
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
	bFilterAudioMessages(proto, "FilterAudioMessages", false),

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

	iReqAuthTimeLater(proto, "ReqAuthTimeLater", 60 * 60 * 24),
	iNewsInterval(proto, "NewsInterval", 15),
	iNotificationsInterval(proto, "NotificationsInterval", 1),
	iNewsAutoClearHistoryInterval(proto, "NewsAutoClearHistoryInterval", 60 * 60 * 24 * 3),
	iInvisibleInterval(proto, "InvisibleInterval", 10),
	iMaxFriendsCount(proto, "MaxFriendsCount", 1000),
	iStickerSize(proto, "StickerSize", 128),

	pwszDefaultGroup(proto, "ProtoGroup", L"VKontakte"),
	pwszVKLang(proto, "VKLang", nullptr)

{
	// Note: Delete this code after next stable build
	int iStikersAsSmyles = db_get_b(0, proto->m_szModuleName, "StikersAsSmyles", -1);
	if (iStikersAsSmyles != -1) {
		bStikersAsSmileys = iStikersAsSmyles == 1;
		db_set_b(0, proto->m_szModuleName, "StikersAsSmileys", (uint8_t)iStikersAsSmyles);
		db_unset(0, proto->m_szModuleName, "StikersAsSmyles");
	}
	// Note
}