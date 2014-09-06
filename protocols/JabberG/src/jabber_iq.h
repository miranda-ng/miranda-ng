/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _JABBER_IQ_H_
#define _JABBER_IQ_H_

#include "jabber_xml.h"

class CJabberIqInfo;

struct CJabberProto;
typedef void (*IQ_USER_DATA_FREE_FUNC)(void *pUserData);

// 2 minutes, milliseconds
#define JABBER_DEFAULT_IQ_REQUEST_TIMEOUT		120000

typedef void (CJabberProto::*JABBER_IQ_HANDLER)(HXML iqNode, CJabberIqInfo *pInfo);
typedef BOOL (CJabberProto::*JABBER_PERMANENT_IQ_HANDLER)(HXML iqNode, CJabberIqInfo *pInfo);

#define JABBER_IQ_PARSE_CHILD_TAG_NODE  (1)
#define JABBER_IQ_PARSE_CHILD_TAG_NAME  ((1<<1)|JABBER_IQ_PARSE_CHILD_TAG_NODE)
#define JABBER_IQ_PARSE_CHILD_TAG_XMLNS ((1<<2)|JABBER_IQ_PARSE_CHILD_TAG_NODE)
#define JABBER_IQ_PARSE_FROM             (1<<3)
#define JABBER_IQ_PARSE_HCONTACT        ((1<<4)|JABBER_IQ_PARSE_FROM)
#define JABBER_IQ_PARSE_TO               (1<<5)
#define JABBER_IQ_PARSE_ID_STR           (1<<6)

#define JABBER_IQ_PARSE_DEFAULT (JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_CHILD_TAG_NAME | JABBER_IQ_PARSE_CHILD_TAG_XMLNS)

class CJabberIqInfo
{
protected:
	friend class CJabberIqManager;
	JABBER_IQ_HANDLER m_pHandler;
	CJabberIqInfo *m_pNext;

	int    m_nIqId;
	DWORD  m_dwParamsToParse;
	DWORD  m_dwRequestTime;
	DWORD  m_dwTimeout;
	TCHAR *m_szReceiver;
	int    m_iPriority;

public:
	void  *m_pUserData;
	int    m_nIqType;
	TCHAR *m_szFrom;
	TCHAR *m_szChildTagXmlns;
	TCHAR *m_szChildTagName;
	HXML   m_pChildNode;
	MCONTACT m_hContact;
	TCHAR *m_szTo;
	TCHAR *m_szId;

public:
	__forceinline CJabberIqInfo()
	{
		memset(this, 0, sizeof(*this));
	}
	__forceinline ~CJabberIqInfo()
	{
		mir_free(m_szReceiver);
	}

	__forceinline void SetReceiver(const TCHAR *szReceiver)
	{	replaceStrT(m_szReceiver, szReceiver);
	}
	__forceinline TCHAR* GetReceiver()
	{	return m_szReceiver;
	}
	__forceinline void SetParamsToParse(DWORD dwParamsToParse)
	{	m_dwParamsToParse = dwParamsToParse;
	}
	__forceinline void SetTimeout(DWORD dwTimeout)
	{	m_dwTimeout = dwTimeout;
	}
	__forceinline int GetIqId()
	{	return m_nIqId;
	}
	__forceinline DWORD GetRequestTime()
	{	return m_dwRequestTime;
	}
	__forceinline int GetIqType()
	{	return m_nIqType;
	}
	__forceinline void* GetUserData()
	{	return m_pUserData;
	}
	__forceinline TCHAR* GetFrom()
	{	return m_szFrom;
	}
	__forceinline TCHAR* GetTo()
	{	return m_szTo;
	}
	__forceinline TCHAR* GetIdStr()
	{	return m_szId;
	}
	__forceinline MCONTACT GetHContact()
	{	return m_hContact;
	}
	__forceinline HXML GetChildNode()
	{	return m_pChildNode;
	}
	__forceinline TCHAR* GetChildNodeName()
	{	return m_szChildTagName;
	}
	
	char* GetCharIqType()
	{
		switch (m_nIqType) {
			case JABBER_IQ_TYPE_SET: return "set";
			case JABBER_IQ_TYPE_GET: return "get";
			case JABBER_IQ_TYPE_ERROR: return "error";
			case JABBER_IQ_TYPE_RESULT: return "result";
		}
		return NULL;
	}
};

class CJabberIqPermanentInfo : public MZeroedObject
{
	friend class CJabberIqManager;

	JABBER_PERMANENT_IQ_HANDLER m_pHandler;
	DWORD m_dwParamsToParse;
	int m_nIqTypes;
	TCHAR *m_szXmlns;
	TCHAR *m_szTag;
	BOOL m_bAllowPartialNs;
	void *m_pUserData;
	IQ_USER_DATA_FREE_FUNC m_pUserDataFree;
	int m_iPriority;

public:
	~CJabberIqPermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
		mir_free(m_szXmlns);
		mir_free(m_szTag);
	}

	__forceinline int getPriority() const { return m_iPriority; }
};

class CJabberIqManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	DWORD m_dwLastUsedHandle;
	CJabberIqInfo *m_pIqs; // list of iqs ordered by priority
	HANDLE m_hExpirerThread;
	BOOL m_bExpirerThreadShutdownRequest;

	OBJLIST<CJabberIqPermanentInfo> m_arHandlers;

	CJabberIqInfo* DetachInfo(int nIqId);
	CJabberIqInfo* DetachInfo(void *pUserData);
	CJabberIqInfo* DetachExpired();

	void ExpireInfo(CJabberIqInfo *pInfo, void *pUserData = NULL);
	
	// inserts pInfo at a place determined by pInfo->m_iPriority
	bool InsertIq(CJabberIqInfo *pInfo);

public:
	CJabberIqManager(CJabberProto* proto);
	~CJabberIqManager();

	bool Start();
	void Shutdown();

	// fucking params, maybe just return CJabberIqRequestInfo pointer ?
	CJabberIqInfo* AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType, const TCHAR *szReceiver, DWORD dwParamsToParse, int nIqId, void *pUserData, int iPriority);
	CJabberIqPermanentInfo* AddPermanentHandler(JABBER_PERMANENT_IQ_HANDLER pHandler, int nIqTypes, DWORD dwParamsToParse, const TCHAR *szXmlns, BOOL bAllowPartialNs, const TCHAR *szTag, void *pUserData = NULL, IQ_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT);

	// returns TRUE when pInfo found, or FALSE otherwise
	bool DeletePermanentHandler(CJabberIqPermanentInfo *pInfo);
	bool DeleteHandler(CJabberIqInfo *pInfo);

	bool HandleIq(int nIqId, HXML pNode);
	bool HandleIqPermanent(HXML pNode);

	bool ExpireIq(int nIqId);
	void ExpirerThread(void);
	bool ExpireByUserData(void *pUserData);
	bool ExpireAll(void *pUserData = NULL);
	void FillPermanentHandlers();
};

#endif
