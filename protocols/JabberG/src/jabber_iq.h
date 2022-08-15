/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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

typedef void (CJabberProto::*JABBER_IQ_HANDLER)(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);
typedef bool (CJabberProto::*JABBER_PERMANENT_IQ_HANDLER)(const TiXmlElement *iqNode, CJabberIqInfo *pInfo);

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

	int           m_nIqId;
	uint32_t      m_dwParamsToParse;
	uint32_t      m_dwRequestTime;
	uint32_t      m_dwTimeout;
	char*         m_szReceiver;
	int           m_iPriority;
	void*         m_pUserData;
	int           m_nIqType;
	const char*   m_szFrom;
	const char*   m_szChildTagXmlns;
	const char*   m_szChildTagName;
	const TiXmlElement *m_pChildNode;
	MCONTACT      m_hContact;
	const char*   m_szTo;
	const char*   m_szId;

public:
	__forceinline CJabberIqInfo()
	{	memset(this, 0, sizeof(*this));
	}
	__forceinline ~CJabberIqInfo()
	{	mir_free(m_szReceiver);
	}

	__forceinline void SetReceiver(const char *szReceiver) { replaceStr(m_szReceiver, szReceiver); }
	__forceinline void SetParamsToParse(uint32_t dwParamsToParse) { m_dwParamsToParse = dwParamsToParse; }
	__forceinline void SetTimeout(uint32_t dwTimeout) { m_dwTimeout = dwTimeout; }

	__forceinline int         GetIqId() const { return m_nIqId; }
	__forceinline uint32_t    GetRequestTime() const { return m_dwRequestTime; }
	__forceinline int         GetIqType() const { return m_nIqType; }	
	__forceinline void*       GetUserData() const {	return m_pUserData; }
	__forceinline const char* GetFrom() const {	return m_szFrom; }
	__forceinline const char* GetTo() const { return m_szTo; }
	__forceinline const char* GetIdStr() const { return m_szId; }
	__forceinline MCONTACT    GetHContact() const { return m_hContact; }
	__forceinline const char* GetChildNodeName() const { return m_szChildTagName; }
	__forceinline const char* GetReceiver() const { return m_szReceiver; }
	__forceinline int         GetPriority() const { return m_iPriority; }

	__forceinline const TiXmlElement *GetChildNode() const { return m_pChildNode; }

	char* GetCharIqType()
	{
		switch (m_nIqType) {
			case JABBER_IQ_TYPE_SET: return "set";
			case JABBER_IQ_TYPE_GET: return "get";
			case JABBER_IQ_TYPE_ERROR: return "error";
			case JABBER_IQ_TYPE_RESULT: return "result";
		}
		return nullptr;
	}
};

class CJabberIqPermanentInfo : public MZeroedObject
{
	friend class CJabberIqManager;

	JABBER_PERMANENT_IQ_HANDLER m_pHandler;
	uint32_t m_dwParamsToParse;
	int m_nIqTypes;
	char *m_szXmlns;
	char *m_szTag;
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
	uint32_t  m_dwLastUsedHandle = 0;

	LIST<CJabberIqInfo> m_arIqs;
	OBJLIST<CJabberIqPermanentInfo> m_arHandlers;

	CJabberIqInfo* DetachInfo();
	CJabberIqInfo* DetachInfo(int nIqId);
	CJabberIqInfo* DetachInfo(void *pUserData);
	CJabberIqInfo* DetachExpired();

	void ExpireInfo(CJabberIqInfo *pInfo);

public:
	CJabberIqManager(CJabberProto *proto);
	~CJabberIqManager();

	// fucking params, maybe just return CJabberIqRequestInfo pointer ?
	CJabberIqInfo* AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType, const char *szReceiver, void *pUserData, int iPriority);
	CJabberIqPermanentInfo* AddPermanentHandler(JABBER_PERMANENT_IQ_HANDLER pHandler, int nIqTypes, uint32_t dwParamsToParse, const char *szXmlns, BOOL bAllowPartialNs, const char *szTag, void *pUserData = nullptr, IQ_USER_DATA_FREE_FUNC pUserDataFree = nullptr, int iPriority = JH_PRIORITY_DEFAULT);

	// returns TRUE when pInfo found, or FALSE otherwise
	bool DeletePermanentHandler(CJabberIqPermanentInfo *pInfo);
	bool DeleteHandler(CJabberIqInfo *pInfo);

	bool HandleIq(int nIqId, const TiXmlElement *pNode);
	bool HandleIqPermanent(const TiXmlElement *pNode);

	void CheckExpired(void);
	void ExpireIq(int nIqId);
	bool ExpireByUserData(void *pUserData);
	void ExpireAll();
	void FillPermanentHandlers();
};

#endif
