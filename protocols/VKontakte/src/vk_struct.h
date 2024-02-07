/*
Copyright (c) 2013-24 Miranda NG team (https://miranda-ng.org)

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
#pragma once

struct AsyncHttpRequest : public MTHttpRequest<CVkProto>
{
	enum RequestPriority { rpLow, rpMedium, rpHigh };

	AsyncHttpRequest();
	AsyncHttpRequest(CVkProto*, int iRequestType, LPCSTR szUrl, bool bSecure, MTHttpRequestHandler pFunc, RequestPriority rpPriority = rpMedium);

	void Redirect(MHttpResponse*);

	int m_iRetry;
	int m_iErrorCode;
	RequestPriority m_priority;
	static ULONG m_uReqCount;
	ULONG m_uReqNum;
	bool m_bApiReq;
	bool bNeedsRestart, bIsMainConn;
};

struct CVkFileUploadParam : public MZeroedObject {
	enum VKFileType { typeInvalid, typeImg, typeAudio, typeAudioMsg, typeDoc, typeNotSupported };
	wchar_t *wszFileName;
	wchar_t *wszDesc;
	char *szAtr;
	char *szFname;
	MCONTACT hContact;
	VKFileType vkFileType;

	CVkFileUploadParam(MCONTACT _hContact, const wchar_t *_desc, wchar_t **_files);
	~CVkFileUploadParam();
	VKFileType GetType();
	__forceinline bool IsAccess() { return ::_waccess(wszFileName, 0) == 0; }
	__forceinline char* atrName() { GetType();  return szAtr; }
	__forceinline char* fileName() { GetType();  return szFname; }
};

struct CVkSendMsgParam : public MZeroedObject
{
	CVkSendMsgParam(MCONTACT _hContact, VKMessageID_t _iMsgID = 0, int _iCount = 0) :
		hContact(_hContact),
		iMsgID(_iMsgID),
		iCount(_iCount),
		pFUP(nullptr)
	{}

	CVkSendMsgParam(MCONTACT _hContact, CVkFileUploadParam *_pFUP) :
		hContact(_hContact),
		iMsgID(-1),
		iCount(0),
		pFUP(_pFUP)
	{}

	MCONTACT hContact;
	VKMessageID_t iMsgID;
	int iCount;
	CVkFileUploadParam *pFUP;
};

struct CVkDBAddAuthRequestThreadParam : public MZeroedObject
{
	CVkDBAddAuthRequestThreadParam(MCONTACT _hContact, bool _bAdded) :
		hContact(_hContact),
		bAdded(_bAdded)
	{}

	MCONTACT hContact;
	bool bAdded;
};

struct CVkChatMessage : public MZeroedObject
{
	CVkChatMessage(VKUserID_t _id) :
		m_iMessageId(_id),
		m_iUserId(0),
		m_tDate(0),
		m_bHistory(false),
		m_bIsAction(false)
	{}

	VKMessageID_t m_iMessageId;
	VKUserID_t m_iUserId; 
	time_t m_tDate;
	bool m_bHistory, m_bIsAction;
	ptrW m_wszBody;
};

struct CVkChatUser : public MZeroedObject
{
	CVkChatUser(VKUserID_t _id) :
		m_iUserId(_id),
		m_bDel(false),
		m_bUnknown(false)
	{}

	VKUserID_t m_iUserId;
	bool m_bDel, m_bUnknown;
	ptrW m_wszNick;
};

struct CVkChatInfo : public MZeroedObject
{
	CVkChatInfo(VKUserID_t _id) :
		m_users(10, NumericKeySortT),
		m_msgs(10, NumericKeySortT),
		m_iChatId(_id)
	{}

	VKUserID_t m_iChatId;
	VKUserID_t m_iAdminId = 0;
	bool m_bHistoryRead = false;
	ptrW m_wszTopic;
	SESSION_INFO *m_si = nullptr;
	OBJLIST<CVkChatUser> m_users;
	OBJLIST<CVkChatMessage> m_msgs;

	CVkChatUser* GetUserById(LPCWSTR);
	CVkChatUser* GetUserById(VKUserID_t iUserId);
};

struct CVkUserInfo : public MZeroedObject {
	CVkUserInfo(VKUserID_t _UserId) :
		m_UserId(_UserId),
		m_bIsGroup(false)
	{}

	CVkUserInfo(VKUserID_t _UserId, bool _bIsGroup, const CMStringW& _wszUserNick, const CMStringW& _wszLink, MCONTACT _hContact = 0) :
		m_UserId(_UserId),
		m_bIsGroup(_bIsGroup),
		m_wszUserNick(_wszUserNick),
		m_wszLink(_wszLink),
		m_hContact(_hContact)
	{}

	VKUserID_t m_UserId;
	MCONTACT m_hContact;
	CMStringW m_wszUserNick;
	CMStringW m_wszLink;
	bool m_bIsGroup;
};

enum VKObjType { vkNull, vkPost, vkPhoto, vkVideo, vkComment, vkTopic, vkUsers, vkCopy, vkInvite };

struct CVKNotification {
	wchar_t *pwszType;
	VKObjType vkParent, vkFeedback;
	wchar_t *pwszTranslate;
};

struct CVKNewsItem : public MZeroedObject {
	CVKNewsItem() :
		tDate(0),
		vkUser(nullptr),
		bIsGroup(false),
		bIsRepost(false),
		vkFeedbackType(vkNull),
		vkParentType(vkNull)
	{}

	CMStringW wszId;
	time_t tDate;
	CVkUserInfo *vkUser;
	CMStringW wszText;
	CMStringW wszLink;
	CMStringW wszType;
	CMStringW wszPopupTitle;
	CMStringW wszPopupText;
	VKObjType vkFeedbackType, vkParentType;
	bool bIsGroup;
	bool bIsRepost;
};

enum VKBBCType : uint8_t { vkbbcB, vkbbcI, vkbbcS, vkbbcU, vkbbcCode, vkbbcImg, vkbbcUrl, vkbbcSize, vkbbcColor };
enum BBCSupport : uint8_t { bbcNo, bbcBasic, bbcAdvanced };

struct CVKBBCItem {
	VKBBCType vkBBCType;
	BBCSupport vkBBCSettings;
	wchar_t *pwszTempate;
};

struct CVKChatContactTypingParam {
	CVKChatContactTypingParam(VKUserID_t pChatId, VKUserID_t pUserId) :
		m_ChatId(pChatId),
		m_UserId(pUserId)
	{}

	VKUserID_t m_ChatId;
	VKUserID_t m_UserId;
};

struct CVKInteres {
	const char *szField;
	wchar_t *pwszTranslate;
};

struct CVKLang {
	wchar_t *szCode;
	wchar_t *szDescription;
};

enum MarkMsgReadOn : uint8_t { markOnRead, markOnReceive, markOnReply, markOnTyping };
enum SyncHistoryMetod : uint8_t { syncOff, syncAuto, sync1Days, sync3Days };
enum MusicSendMetod : uint8_t { sendNone, sendStatusOnly, sendBroadcastOnly, sendBroadcastAndStatus };
enum IMGBBCSypport : uint8_t { imgNo, imgFullSize, imgPreview130, imgPreview604 };

struct CVkCookie
{
	CVkCookie(const CMStringA& name, const CMStringA& value, const CMStringA& domain) :
		m_name(name),
		m_value(value),
		m_domain(domain)
	{}

	CMStringA m_name, m_value, m_domain;
};

struct CVKOptions {
	CMOption<uint8_t> bLoadLastMessageOnMsgWindowsOpen;
	CMOption<uint8_t> bLoadOnlyFriends;
	CMOption<uint8_t> bServerDelivery;
	CMOption<uint8_t> bHideChats;
	CMOption<uint8_t> bMesAsUnread;
	CMOption<uint8_t> bUseLocalTime;
	CMOption<uint8_t> bReportAbuse;
	CMOption<uint8_t> bClearServerHistory;
	CMOption<uint8_t> bRemoveFromFrendlist;
	CMOption<uint8_t> bRemoveFromCList;
	CMOption<uint8_t> bPopUpSyncHistory;
	CMOption<uint8_t> iMarkMessageReadOn;
	CMOption<uint8_t> bStikersAsSmileys;
	CMOption<uint8_t> bUseStikersAsStaticSmileys;
	CMOption<uint8_t> bUserForceInvisibleOnActivity;
	CMOption<uint8_t> iMusicSendMetod;
	CMOption<uint8_t> bPopupContactsMusic;
	CMOption<uint8_t> iSyncHistoryMetod;
	CMOption<uint8_t> bNewsEnabled;
	CMOption<uint8_t> iMaxLoadNewsPhoto;
	CMOption<uint8_t> bNotificationsEnabled;
	CMOption<uint8_t> bNotificationsMarkAsViewed;
	CMOption<uint8_t> bSpecialContactAlwaysEnabled;
	CMOption<uint8_t> iIMGBBCSupport;
	CMOption<uint8_t> iBBCForNews;
	CMOption<uint8_t> iBBCForAttachments;
	CMOption<uint8_t> bUseBBCOnAttacmentsAsNews;
	CMOption<uint8_t> bNewsAutoClearHistory;
	CMOption<uint8_t> bNewsFilterPosts;
	CMOption<uint8_t> bNewsFilterPhotos;
	CMOption<uint8_t> bNewsFilterTags;
	CMOption<uint8_t> bNewsFilterWallPhotos;
	CMOption<uint8_t> bNewsSourceFriends;
	CMOption<uint8_t> bNewsSourceGroups;
	CMOption<uint8_t> bNewsSourcePages;
	CMOption<uint8_t> bNewsSourceFollowing;
	CMOption<uint8_t> bNewsSourceIncludeBanned;
	CMOption<uint8_t> bNewsSourceNoReposts;
	CMOption<uint8_t> bNotificationFilterComments;
	CMOption<uint8_t> bNotificationFilterLikes;
	CMOption<uint8_t> bNotificationFilterReposts;
	CMOption<uint8_t> bNotificationFilterMentions;
	CMOption<uint8_t> bNotificationFilterInvites;
	CMOption<uint8_t> bNotificationFilterAcceptedFriends;
	CMOption<uint8_t> bUseNonStandardNotifications;
	CMOption<uint8_t> bShortenLinksForAudio;
	CMOption<uint8_t> bAddMessageLinkToMesWAtt;
	CMOption<uint8_t> bShowReplyInMessage;
	CMOption<uint8_t> bSplitFormatFwdMsg;
	CMOption<uint8_t> bSyncReadMessageStatusFromServer;
	CMOption<uint8_t> bLoadFullCList;
	CMOption<uint8_t> bSendVKLinksAsAttachments;
	CMOption<uint8_t> bLoadSentAttachments;
	CMOption<uint8_t> bShowVkDeactivateEvents;
	CMOption<uint8_t> bStickerBackground;
	CMOption<uint8_t> bFilterAudioMessages;

	CMOption<uint8_t> bShowProtoMenuItem0;
	CMOption<uint8_t> bShowProtoMenuItem1;
	CMOption<uint8_t> bShowProtoMenuItem2;
	CMOption<uint8_t> bShowProtoMenuItem3;
	CMOption<uint8_t> bShowProtoMenuItem4;
	CMOption<uint8_t> bShowProtoMenuItem5;
	CMOption<uint8_t> bShowProtoMenuItem6;

	CMOption<uint32_t> iReqAuthTimeLater;
	CMOption<uint32_t> iNewsInterval;
	CMOption<uint32_t> iNotificationsInterval;
	CMOption<uint32_t> iNewsAutoClearHistoryInterval;
	CMOption<uint32_t> iInvisibleInterval;
	CMOption<uint32_t> iMaxFriendsCount;
	CMOption<uint32_t> iStickerSize;

	CMOption<wchar_t*> pwszDefaultGroup;
	CMOption<wchar_t*> pwszVKLang;

	CVKOptions(PROTO_INTERFACE *proto);

	__forceinline BBCSupport BBCForNews() { return (BBCSupport)(uint8_t)iBBCForNews; };
	__forceinline BBCSupport BBCForAttachments() { return (BBCSupport)(uint8_t)iBBCForAttachments; };

};

struct CVKDeactivateEvent {
	wchar_t *wszType;
	char *szDescription;
};

enum VKContactType : uint8_t { 
	vkContactNormal		= 0x01, 
	vkContactSelf		= 0x02,
	vkContactMUCUser	= 0x04,
	vkContactGroupUser	= 0x08
};
enum VKMesType : uint8_t { vkALL, vkIN, vkOUT };

struct CVKImageSizeItem {
	CMStringW wszUrl;
	int iSizeH, iSizeW;

	CVKImageSizeItem() :
		iSizeH(0),
		iSizeW(0)
	{}
};

enum VKPeerType : uint8_t { vkPeerError = 0 , vkPeerUser, vkPeerGroup, vkPeerMUC, vkPeerFeed};