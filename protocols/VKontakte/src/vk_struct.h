/*
Copyright (c) 2013-21 Miranda NG team (https://miranda-ng.org)

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

	void Redirect(NETLIBHTTPREQUEST*);

	int m_iRetry;
	int m_iErrorCode;
	RequestPriority m_priority;
	static ULONG m_reqCount;
	ULONG m_reqNum;
	bool m_bApiReq;
	bool bNeedsRestart, bIsMainConn;
};

struct CVkFileUploadParam : public MZeroedObject {
	enum VKFileType { typeInvalid, typeImg, typeAudio, typeAudioMsg, typeDoc, typeNotSupported };
	wchar_t *FileName;
	wchar_t *Desc;
	char *atr;
	char *fname;
	MCONTACT hContact;
	VKFileType filetype;

	CVkFileUploadParam(MCONTACT _hContact, const wchar_t *_desc, wchar_t **_files);
	~CVkFileUploadParam();
	VKFileType GetType();
	__forceinline bool IsAccess() { return ::_waccess(FileName, 0) == 0; }
	__forceinline char* atrName() { GetType();  return atr; }
	__forceinline char* fileName() { GetType();  return fname; }
};

struct CVkSendMsgParam : public MZeroedObject
{
	CVkSendMsgParam(MCONTACT _hContact, int _iMsgID = 0, int _iCount = 0) :
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
	int iMsgID;
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
	CVkChatMessage(int _id) :
		m_mid(_id),
		m_uid(0),
		m_date(0),
		m_bHistory(false),
		m_bIsAction(false)
	{}

	int m_mid, m_uid, m_date;
	bool m_bHistory, m_bIsAction;
	ptrW m_wszBody;
};

struct CVkChatUser : public MZeroedObject
{
	CVkChatUser(LONG _id) :
		m_uid(_id),
		m_bDel(false),
		m_bUnknown(false)
	{}

	LONG m_uid;
	bool m_bDel, m_bUnknown;
	ptrW m_wszNick;
};

struct CVkChatInfo : public MZeroedObject
{
	CVkChatInfo(int _id) :
		m_users(10, NumericKeySortT),
		m_msgs(10, NumericKeySortT),
		m_iChatId(_id),
		m_iAdminId(0),
		m_bHistoryRead(0),
		m_hContact(INVALID_CONTACT_ID)
	{}

	int m_iChatId, m_iAdminId;
	bool m_bHistoryRead;
	ptrW m_wszTopic, m_wszId;
	MCONTACT m_hContact;
	OBJLIST<CVkChatUser> m_users;
	OBJLIST<CVkChatMessage> m_msgs;

	CVkChatUser* GetUserById(LPCWSTR);
	CVkChatUser* GetUserById(int user_id);
};

struct CVkUserInfo : public MZeroedObject {
	CVkUserInfo(LONG _UserId) :
		m_UserId(_UserId),
		m_bIsGroup(false)
	{}

	CVkUserInfo(LONG _UserId, bool _bIsGroup, const CMStringW& _wszUserNick, const CMStringW& _wszLink, MCONTACT _hContact = 0) :
		m_UserId(_UserId),
		m_bIsGroup(_bIsGroup),
		m_wszUserNick(_wszUserNick),
		m_wszLink(_wszLink),
		m_hContact(_hContact)
	{}

	LONG m_UserId;
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

enum VKBBCType : BYTE { vkbbcB, vkbbcI, vkbbcS, vkbbcU, vkbbcCode, vkbbcImg, vkbbcUrl, vkbbcSize, vkbbcColor };
enum BBCSupport : BYTE { bbcNo, bbcBasic, bbcAdvanced };

struct CVKBBCItem {
	VKBBCType vkBBCType;
	BBCSupport vkBBCSettings;
	wchar_t *pwszTempate;
};

struct CVKChatContactTypingParam {
	CVKChatContactTypingParam(int pChatId, int pUserId) :
		m_ChatId(pChatId),
		m_UserId(pUserId)
	{}

	int m_ChatId;
	LONG m_UserId;
};

struct CVKInteres {
	const char *szField;
	wchar_t *pwszTranslate;
};

struct CVKLang {
	wchar_t *szCode;
	wchar_t *szDescription;
};

enum MarkMsgReadOn : BYTE { markOnRead, markOnReceive, markOnReply, markOnTyping };
enum SyncHistoryMetod : BYTE { syncOff, syncAuto, sync1Days, sync3Days };
enum MusicSendMetod : BYTE { sendNone, sendStatusOnly, sendBroadcastOnly, sendBroadcastAndStatus };
enum IMGBBCSypport : BYTE { imgNo, imgFullSize, imgPreview130, imgPreview604 };

struct CVKSync {
	const wchar_t *type;
	SyncHistoryMetod data;
};

struct CVKMarkMsgRead {
	const wchar_t *type;
	MarkMsgReadOn data;
};

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
	CMOption<BYTE> bLoadLastMessageOnMsgWindowsOpen;
	CMOption<BYTE> bLoadOnlyFriends;
	CMOption<BYTE> bServerDelivery;
	CMOption<BYTE> bHideChats;
	CMOption<BYTE> bMesAsUnread;
	CMOption<BYTE> bUseLocalTime;
	CMOption<BYTE> bReportAbuse;
	CMOption<BYTE> bClearServerHistory;
	CMOption<BYTE> bRemoveFromFrendlist;
	CMOption<BYTE> bRemoveFromCList;
	CMOption<BYTE> bPopUpSyncHistory;
	CMOption<BYTE> iMarkMessageReadOn;
	CMOption<BYTE> bStikersAsSmileys;
	CMOption<BYTE> bUseStikersAsStaticSmileys;
	CMOption<BYTE> bUserForceInvisibleOnActivity;
	CMOption<BYTE> iMusicSendMetod;
	CMOption<BYTE> bPopupContactsMusic;
	CMOption<BYTE> iSyncHistoryMetod;
	CMOption<BYTE> bNewsEnabled;
	CMOption<BYTE> iMaxLoadNewsPhoto;
	CMOption<BYTE> bNotificationsEnabled;
	CMOption<BYTE> bNotificationsMarkAsViewed;
	CMOption<BYTE> bSpecialContactAlwaysEnabled;
	CMOption<BYTE> iIMGBBCSupport;
	CMOption<BYTE> iBBCForNews;
	CMOption<BYTE> iBBCForAttachments;
	CMOption<BYTE> bUseBBCOnAttacmentsAsNews;
	CMOption<BYTE> bNewsAutoClearHistory;
	CMOption<BYTE> bNewsFilterPosts;
	CMOption<BYTE> bNewsFilterPhotos;
	CMOption<BYTE> bNewsFilterTags;
	CMOption<BYTE> bNewsFilterWallPhotos;
	CMOption<BYTE> bNewsSourceFriends;
	CMOption<BYTE> bNewsSourceGroups;
	CMOption<BYTE> bNewsSourcePages;
	CMOption<BYTE> bNewsSourceFollowing;
	CMOption<BYTE> bNewsSourceIncludeBanned;
	CMOption<BYTE> bNewsSourceNoReposts;
	CMOption<BYTE> bNotificationFilterComments;
	CMOption<BYTE> bNotificationFilterLikes;
	CMOption<BYTE> bNotificationFilterReposts;
	CMOption<BYTE> bNotificationFilterMentions;
	CMOption<BYTE> bNotificationFilterInvites;
	CMOption<BYTE> bNotificationFilterAcceptedFriends;
	CMOption<BYTE> bUseNonStandardNotifications;
	CMOption<BYTE> bShortenLinksForAudio;
	CMOption<BYTE> bAddMessageLinkToMesWAtt;
	CMOption<BYTE> bSplitFormatFwdMsg;
	CMOption<BYTE> bSyncReadMessageStatusFromServer;
	CMOption<BYTE> bLoadFullCList;
	CMOption<BYTE> bSendVKLinksAsAttachments;
	CMOption<BYTE> bLoadSentAttachments;
	CMOption<BYTE> bShowVkDeactivateEvents;
	CMOption<BYTE> bStickerBackground;
	CMOption<BYTE> bFilterAudioMessages;

	CMOption<BYTE> bShowProtoMenuItem0;
	CMOption<BYTE> bShowProtoMenuItem1;
	CMOption<BYTE> bShowProtoMenuItem2;
	CMOption<BYTE> bShowProtoMenuItem3;
	CMOption<BYTE> bShowProtoMenuItem4;
	CMOption<BYTE> bShowProtoMenuItem5;
	CMOption<BYTE> bShowProtoMenuItem6;

	CMOption<DWORD> iReqAuthTimeLater;
	CMOption<DWORD> iNewsInterval;
	CMOption<DWORD> iNotificationsInterval;
	CMOption<DWORD> iNewsAutoClearHistoryInterval;
	CMOption<DWORD> iInvisibleInterval;
	CMOption<DWORD> iMaxFriendsCount;
	CMOption<DWORD> iStickerSize;

	CMOption<wchar_t*> pwszDefaultGroup;
	CMOption<wchar_t*> pwszVKLang;

	CVKOptions(PROTO_INTERFACE *proto);

	__forceinline BBCSupport BBCForNews() { return (BBCSupport)(BYTE)iBBCForNews; };
	__forceinline BBCSupport BBCForAttachments() { return (BBCSupport)(BYTE)iBBCForAttachments; };

};

struct CVKDeactivateEvent {
	wchar_t *wszType;
	char *szDescription;
};

enum VKContactType : BYTE { vkContactNormal, vkContactSelf, vkContactMUCUser };
enum VKMesType : BYTE { vkALL, vkIN, vkOUT };

struct CVKImageSizeItem {
	CMStringW wszUrl;
	int iSizeH, iSizeW;

	CVKImageSizeItem() :
		iSizeH(0),
		iSizeW(0)
	{}
};