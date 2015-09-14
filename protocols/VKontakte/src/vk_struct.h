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
#pragma once

typedef void (CVkProto::*VK_REQUEST_HANDLER)(NETLIBHTTPREQUEST*, struct AsyncHttpRequest*);

struct AsyncHttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	enum RequestPriority { rpLow, rpMedium, rpHigh };

	AsyncHttpRequest();
	AsyncHttpRequest(CVkProto*, int iRequestType, LPCSTR szUrl, bool bSecure, VK_REQUEST_HANDLER pFunc, RequestPriority rpPriority = rpMedium);
	~AsyncHttpRequest();

	void AddHeader(LPCSTR, LPCSTR);
	void Redirect(NETLIBHTTPREQUEST*);

	CMStringA m_szUrl;
	CMStringA m_szParam;
	VK_REQUEST_HANDLER m_pFunc;
	void *pUserInfo;
	int m_iRetry;
	int m_iErrorCode;
	RequestPriority m_priority;
	static ULONG m_reqCount;
	ULONG m_reqNum;
	bool m_bApiReq;
	bool bExpUrlEncode;
	bool bNeedsRestart, bIsMainConn;
};

struct PARAM
{
	LPCSTR szName;
	__forceinline PARAM(LPCSTR _name) : szName(_name)
	{}
};

struct INT_PARAM : public PARAM
{
	int iValue;
	__forceinline INT_PARAM(LPCSTR _name, int _value) :
		PARAM(_name), iValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const INT_PARAM&);

struct CHAR_PARAM : public PARAM
{
	LPCSTR szValue;
	__forceinline CHAR_PARAM(LPCSTR _name, LPCSTR _value) :
		PARAM(_name), szValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const CHAR_PARAM&);

struct TCHAR_PARAM : public PARAM
{
	LPCTSTR tszValue;
	__forceinline TCHAR_PARAM(LPCSTR _name, LPCTSTR _value) :
		PARAM(_name), tszValue(_value)
	{}
};
AsyncHttpRequest* operator<<(AsyncHttpRequest*, const TCHAR_PARAM&);

struct CVkFileUploadParam : public MZeroedObject {
	enum VKFileType { typeInvalid, typeImg, typeAudio, typeDoc, typeNotSupported };
	TCHAR* FileName;
	TCHAR* Desc;
	char* atr;
	char* fname;
	MCONTACT hContact;
	VKFileType filetype;

	CVkFileUploadParam(MCONTACT _hContact, const TCHAR* _desc, TCHAR** _files);
	~CVkFileUploadParam();
	VKFileType GetType();
	__forceinline bool IsAccess() { return ::_taccess(FileName, 0) == 0; }
	__forceinline char* atrName() { GetType();  return atr; }
	__forceinline char* fileName() { GetType();  return fname; }
};

struct CVkSendMsgParam : public MZeroedObject
{
	CVkSendMsgParam(MCONTACT _hContact, int _iMsgID = 0, int _iCount = 0) :
		hContact(_hContact),
		iMsgID(_iMsgID),
		iCount(_iCount),
		pFUP(NULL)
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
	ptrT m_tszBody;
};

struct CVkChatUser : public MZeroedObject
{
	CVkChatUser(int _id) :
		m_uid(_id),
		m_bDel(false),
		m_bUnknown(false)
	{}

	int m_uid;
	bool m_bDel, m_bUnknown;
	ptrT m_tszNick;
};

struct CVkChatInfo : public MZeroedObject
{
	CVkChatInfo(int _id) :
		m_users(10, NumericKeySortT),
		m_msgs(10, NumericKeySortT),
		m_chatid(_id),
		m_admin_id(0),
		m_bHistoryRead(0),
		m_hContact(INVALID_CONTACT_ID)
	{}

	int m_chatid, m_admin_id;
	bool m_bHistoryRead;
	ptrT m_tszTopic, m_tszId;
	MCONTACT m_hContact;
	OBJLIST<CVkChatUser> m_users;
	OBJLIST<CVkChatMessage> m_msgs;

	CVkChatUser* GetUserById(LPCTSTR);
};

struct CVkUserInfo : public MZeroedObject {
	CVkUserInfo(LONG _UserId) :
		m_UserId(_UserId),
		m_bIsGroup(false)
	{}

	CVkUserInfo(LONG _UserId, bool _bIsGroup, CMString& _tszUserNick, CMString& _tszLink, MCONTACT _hContact = NULL) :
		m_UserId(_UserId),
		m_bIsGroup(_bIsGroup),
		m_tszUserNick(_tszUserNick),
		m_tszLink(_tszLink),
		m_hContact(_hContact)
	{}

	LONG m_UserId;
	MCONTACT m_hContact;
	CMString m_tszUserNick;
	CMString m_tszLink;
	bool m_bIsGroup;
};

enum VKObjType { vkNull, vkPost, vkPhoto, vkVideo, vkComment, vkTopic, vkUsers, vkCopy, vkInvite };

struct CVKNotification {
	TCHAR *ptszType;
	VKObjType vkParent, vkFeedback;
	TCHAR *ptszTranslate;
};

struct CVKNewsItem : public MZeroedObject {
	CVKNewsItem() :
		tDate(NULL),
		vkUser(NULL),
		bIsGroup(false),
		bIsRepost(false),
		vkFeedbackType(vkNull),
		vkParentType(vkNull)
	{}

	CMString tszId;
	time_t tDate;
	CVkUserInfo *vkUser;
	CMString tszText;
	CMString tszLink;
	CMString tszType;
	VKObjType vkFeedbackType, vkParentType;
	bool bIsGroup;
	bool bIsRepost;
};

enum VKBBCType { vkbbcB, vkbbcI, vkbbcS, vkbbcU, vkbbcImg, vkbbcUrl, vkbbcSize, vkbbcColor };
enum BBCSupport { bbcNo, bbcBasic, bbcAdvanced };

struct CVKBBCItem {
	VKBBCType vkBBCType;
	BBCSupport vkBBCSettings;
	TCHAR *ptszTempate;
};