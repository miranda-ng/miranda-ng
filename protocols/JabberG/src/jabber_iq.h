/*

Jabber Protocol Plugin for Miranda IM
Copyright (C) 2002-04  Santithorn Bunchua
Copyright (C) 2005-12  George Hazan
Copyright (C) 2007     Maxim Mluhov
Copyright (C) 2012-13  Miranda NG Project

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

typedef enum {
	IQ_PROC_NONE,
	IQ_PROC_GETAGENTS,
	IQ_PROC_GETREGISTER,
	IQ_PROC_SETREGISTER,
	IQ_PROC_GETVCARD,
	IQ_PROC_SETVCARD,
	IQ_PROC_GETSEARCH,
	IQ_PROC_GETSEARCHFIELDS,
	IQ_PROC_BROWSEROOMS,
	IQ_PROC_DISCOROOMSERVER,
	IQ_PROC_DISCOAGENTS,
	IQ_PROC_DISCOBOOKMARKS,
	IQ_PROC_SETBOOKMARKS,
	IQ_PROC_DISCOCOMMANDS,
	IQ_PROC_EXECCOMMANDS,
} JABBER_IQ_PROCID;

struct CJabberProto;
typedef void (CJabberProto::*JABBER_IQ_PFUNC)(HXML iqNode);
typedef void (*IQ_USER_DATA_FREE_FUNC)(void *pUserData);

typedef struct {
	TCHAR *xmlns;
	JABBER_IQ_PFUNC func;
	BOOL allowSubNs;		// e.g. #info in disco#info
} JABBER_IQ_XMLNS_FUNC;

// 2 minutes, milliseconds
#define JABBER_DEFAULT_IQ_REQUEST_TIMEOUT		120000

typedef void (CJabberProto::*JABBER_IQ_HANDLER)(HXML iqNode, CJabberIqInfo* pInfo);
typedef BOOL (CJabberProto::*JABBER_PERMANENT_IQ_HANDLER)(HXML iqNode, CJabberIqInfo* pInfo);

#define JABBER_IQ_PARSE_CHILD_TAG_NODE			(1)
#define JABBER_IQ_PARSE_CHILD_TAG_NAME			((1<<1)|JABBER_IQ_PARSE_CHILD_TAG_NODE)
#define JABBER_IQ_PARSE_CHILD_TAG_XMLNS			((1<<2)|JABBER_IQ_PARSE_CHILD_TAG_NODE)
#define JABBER_IQ_PARSE_FROM					(1<<3)
#define JABBER_IQ_PARSE_HCONTACT				((1<<4)|JABBER_IQ_PARSE_FROM)
#define JABBER_IQ_PARSE_TO						(1<<5)
#define JABBER_IQ_PARSE_ID_STR					(1<<6)

#define JABBER_IQ_PARSE_DEFAULT					(JABBER_IQ_PARSE_CHILD_TAG_NODE|JABBER_IQ_PARSE_CHILD_TAG_NAME|JABBER_IQ_PARSE_CHILD_TAG_XMLNS)

class CJabberIqInfo
{
protected:
	friend class CJabberIqManager;
	JABBER_IQ_HANDLER m_pHandler;
	CJabberIqInfo* m_pNext;

	int m_nIqId;
	DWORD m_dwParamsToParse;
	DWORD m_dwRequestTime;
	DWORD m_dwTimeout;
	TCHAR *m_szReceiver;
	int m_iPriority;
public:
	void *m_pUserData;
public:// parsed data
	int m_nIqType;
	TCHAR *m_szFrom;
	TCHAR *m_szChildTagXmlns;
	TCHAR *m_szChildTagName;
	HXML   m_pChildNode;
	HANDLE m_hContact;
	TCHAR *m_szTo;
	TCHAR *m_szId;
public:
	CJabberIqInfo()
	{
		ZeroMemory(this, sizeof(CJabberIqInfo));
	}
	~CJabberIqInfo()
	{
		if (m_szReceiver)
			mir_free(m_szReceiver);
	}
	void SetReceiver(const TCHAR *szReceiver)
	{
		replaceStrT(m_szReceiver, szReceiver);
	}
	TCHAR* GetReceiver()
	{
		return m_szReceiver;
	}
	void SetParamsToParse(DWORD dwParamsToParse)
	{
		m_dwParamsToParse = dwParamsToParse;
	}
	void SetTimeout(DWORD dwTimeout)
	{
		m_dwTimeout = dwTimeout;
	}
	int GetIqId()
	{
		return m_nIqId;
	}
	DWORD GetRequestTime()
	{
		return m_dwRequestTime;
	}
	int GetIqType()
	{
		return m_nIqType;
	}
	void* GetUserData()
	{
		return m_pUserData;
	}
	TCHAR* GetFrom()
	{
		return m_szFrom;
	}
	TCHAR* GetTo()
	{
		return m_szTo;
	}
	TCHAR* GetIdStr()
	{
		return m_szId;
	}
	HANDLE GetHContact()
	{
		return m_hContact;
	}
	HXML GetChildNode()
	{
		return m_pChildNode;
	}
	TCHAR* GetChildNodeName()
	{
		return m_szChildTagName;
	}
	char* GetCharIqType()
	{
		switch (m_nIqType)
		{
		case JABBER_IQ_TYPE_SET: return "set";
		case JABBER_IQ_TYPE_GET: return "get";
		case JABBER_IQ_TYPE_ERROR: return "error";
		case JABBER_IQ_TYPE_RESULT: return "result";
		}
		return NULL;
	}
};

class CJabberIqPermanentInfo
{
	friend class CJabberIqManager;

	CJabberIqPermanentInfo* m_pNext;

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
	CJabberIqPermanentInfo()
	{
		ZeroMemory(this, sizeof(CJabberIqPermanentInfo));
	}
	~CJabberIqPermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
		mir_free(m_szXmlns);
		mir_free(m_szTag);
	}
};

class CJabberIqManager
{
protected:
	CJabberProto *ppro;
	CRITICAL_SECTION m_cs;
	DWORD m_dwLastUsedHandle;
	CJabberIqInfo* m_pIqs; // list of iqs ordered by priority
	HANDLE m_hExpirerThread;
	BOOL m_bExpirerThreadShutdownRequest;

	CJabberIqPermanentInfo* m_pPermanentHandlers;

	CJabberIqInfo* DetachInfo(int nIqId)
	{
		if ( !m_pIqs)
			return NULL;

		CJabberIqInfo* pInfo = m_pIqs;
		if (m_pIqs->m_nIqId == nIqId)
		{
			m_pIqs = pInfo->m_pNext;
			pInfo->m_pNext = NULL;
			return pInfo;
		}

		while (pInfo->m_pNext)
		{
			if (pInfo->m_pNext->m_nIqId == nIqId)
			{
				CJabberIqInfo* pRetVal = pInfo->m_pNext;
				pInfo->m_pNext = pInfo->m_pNext->m_pNext;
				pRetVal->m_pNext = NULL;
				return pRetVal;
			}
			pInfo = pInfo->m_pNext;
		}
		return NULL;
	}
	CJabberIqInfo* DetachInfo(void *pUserData)
	{
		if ( !m_pIqs)
			return NULL;

		CJabberIqInfo* pInfo = m_pIqs;
		if (m_pIqs->m_pUserData == pUserData)
		{
			m_pIqs = pInfo->m_pNext;
			pInfo->m_pNext = NULL;
			return pInfo;
		}

		while (pInfo->m_pNext)
		{
			if (pInfo->m_pNext->m_pUserData == pUserData)
			{
				CJabberIqInfo* pRetVal = pInfo->m_pNext;
				pInfo->m_pNext = pInfo->m_pNext->m_pNext;
				pRetVal->m_pNext = NULL;
				return pRetVal;
			}
			pInfo = pInfo->m_pNext;
		}
		return NULL;
	}
	CJabberIqInfo* DetachExpired()
	{
		if ( !m_pIqs)
			return NULL;

		DWORD dwCurrentTime = GetTickCount();

		CJabberIqInfo* pInfo = m_pIqs;
		if (dwCurrentTime - pInfo->m_dwRequestTime > pInfo->m_dwTimeout)
		{
			m_pIqs = pInfo->m_pNext;
			pInfo->m_pNext = NULL;
			return pInfo;
		}

		while (pInfo->m_pNext)
		{
			if (dwCurrentTime - pInfo->m_pNext->m_dwRequestTime > pInfo->m_pNext->m_dwTimeout)
			{
				CJabberIqInfo* pRetVal = pInfo->m_pNext;
				pInfo->m_pNext = pInfo->m_pNext->m_pNext;
				pRetVal->m_pNext = NULL;
				return pRetVal;
			}
			pInfo = pInfo->m_pNext;
		}
		return NULL;
	}
	void ExpireInfo(CJabberIqInfo* pInfo, void *pUserData = NULL);
	BOOL InsertIq(CJabberIqInfo* pInfo)
	{ // inserts pInfo at a place determined by pInfo->m_iPriority
		Lock();
		if ( !m_pIqs)
			m_pIqs = pInfo;
		else
		{
			if (m_pIqs->m_iPriority > pInfo->m_iPriority) {
				pInfo->m_pNext = m_pIqs;
				m_pIqs = pInfo;
			} else
			{
				CJabberIqInfo* pTmp = m_pIqs;
				while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
					pTmp = pTmp->m_pNext;
				pInfo->m_pNext = pTmp->m_pNext;
				pTmp->m_pNext = pInfo;
			}
		}
		Unlock();
		return TRUE;
	}
public:
	CJabberIqManager(CJabberProto* proto)
	{
		InitializeCriticalSection(&m_cs);
		m_dwLastUsedHandle = 0;
		m_pIqs = NULL;
		m_hExpirerThread = NULL;
		m_pPermanentHandlers = NULL;
		ppro = proto;
	}
	~CJabberIqManager()
	{
		ExpireAll();
		Lock();
		CJabberIqPermanentInfo *pInfo = m_pPermanentHandlers;
		while (pInfo)
		{
			CJabberIqPermanentInfo *pTmp = pInfo->m_pNext;
			delete pInfo;
			pInfo = pTmp;
		}
		m_pPermanentHandlers = NULL;
		Unlock();
		DeleteCriticalSection(&m_cs);
	}
	BOOL Start();
	BOOL Shutdown()
	{
		if (m_bExpirerThreadShutdownRequest || !m_hExpirerThread)
			return TRUE;

		m_bExpirerThreadShutdownRequest = TRUE;

		WaitForSingleObject(m_hExpirerThread, INFINITE);
		CloseHandle(m_hExpirerThread);
		m_hExpirerThread = NULL;

		return TRUE;
	}
	void Lock()
	{
		EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
	// fucking params, maybe just return CJabberIqRequestInfo pointer ?
	CJabberIqInfo* AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType = JABBER_IQ_TYPE_GET, const TCHAR *szReceiver = NULL, DWORD dwParamsToParse = 0, int nIqId = -1, void *pUserData = NULL, int iPriority = JH_PRIORITY_DEFAULT);
	CJabberIqPermanentInfo* AddPermanentHandler(JABBER_PERMANENT_IQ_HANDLER pHandler, int nIqTypes, DWORD dwParamsToParse, const TCHAR *szXmlns, BOOL bAllowPartialNs, const TCHAR *szTag, void *pUserData = NULL, IQ_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT)
	{
		CJabberIqPermanentInfo* pInfo = new CJabberIqPermanentInfo();
		if ( !pInfo)
			return NULL;

		pInfo->m_pHandler = pHandler;
		pInfo->m_nIqTypes = nIqTypes ? nIqTypes : JABBER_IQ_TYPE_ANY;
		replaceStrT(pInfo->m_szXmlns, szXmlns);
		pInfo->m_bAllowPartialNs = bAllowPartialNs;
		replaceStrT(pInfo->m_szTag, szTag);
		pInfo->m_dwParamsToParse = dwParamsToParse;
		pInfo->m_pUserData = pUserData;
		pInfo->m_pUserDataFree = pUserDataFree;
		pInfo->m_iPriority = iPriority;

		Lock();
		if ( !m_pPermanentHandlers)
			m_pPermanentHandlers = pInfo;
		else
		{
			if (m_pPermanentHandlers->m_iPriority > pInfo->m_iPriority) {
				pInfo->m_pNext = m_pPermanentHandlers;
				m_pPermanentHandlers = pInfo;
			} else
			{
				CJabberIqPermanentInfo* pTmp = m_pPermanentHandlers;
				while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
					pTmp = pTmp->m_pNext;
				pInfo->m_pNext = pTmp->m_pNext;
				pTmp->m_pNext = pInfo;
			}
		}
		Unlock();

		return pInfo;
	}
	BOOL DeletePermanentHandler(CJabberIqPermanentInfo *pInfo)
	{ // returns TRUE when pInfo found, or FALSE otherwise
		Lock();
		if ( !m_pPermanentHandlers)
		{
			Unlock();
			return FALSE;
		}
		if (m_pPermanentHandlers == pInfo) // check first item
		{
			m_pPermanentHandlers = m_pPermanentHandlers->m_pNext;
			delete pInfo;
			Unlock();
			return TRUE;
		} else
		{
			CJabberIqPermanentInfo* pTmp = m_pPermanentHandlers;
			while (pTmp->m_pNext)
			{
				if (pTmp->m_pNext == pInfo)
				{
					pTmp->m_pNext = pTmp->m_pNext->m_pNext;
					delete pInfo;
					Unlock();
					return TRUE;
				}
				pTmp = pTmp->m_pNext;
			}
		}
		Unlock();
		return FALSE;
	}
	BOOL DeleteHandler(CJabberIqInfo *pInfo)
	{ // returns TRUE when pInfo found, or FALSE otherwise
		Lock();
		if ( !m_pIqs)
		{
			Unlock();
			return FALSE;
		}
		if (m_pIqs == pInfo) // check first item
		{
			m_pIqs = m_pIqs->m_pNext;
			Unlock();
			ExpireInfo(pInfo); // must expire it to allow the handler to free m_pUserData if necessary
			delete pInfo;
			return TRUE;
		} else
		{
			CJabberIqInfo* pTmp = m_pIqs;
			while (pTmp->m_pNext)
			{
				if (pTmp->m_pNext == pInfo)
				{
					pTmp->m_pNext = pTmp->m_pNext->m_pNext;
					Unlock();
					ExpireInfo(pInfo); // must expire it to allow the handler to free m_pUserData if necessary
					delete pInfo;
					return TRUE;
				}
				pTmp = pTmp->m_pNext;
			}
		}
		Unlock();
		return FALSE;
	}
	BOOL HandleIq(int nIqId, HXML pNode);
	BOOL HandleIqPermanent(HXML pNode);
	BOOL ExpireIq(int nIqId)
	{
		Lock();
		CJabberIqInfo* pInfo = DetachInfo(nIqId);
		Unlock();
		if (pInfo)
		{
			ExpireInfo(pInfo);
			delete pInfo;
			return TRUE;
		}
		return FALSE;
	}
	void ExpirerThread(void);
	BOOL ExpireByUserData(void *pUserData)
	{
		BOOL bRetVal = FALSE;
		while (1)
		{
			Lock();
			CJabberIqInfo* pInfo = DetachInfo(pUserData);
			Unlock();
			if ( !pInfo)
				break;
			ExpireInfo(pInfo, NULL);
			delete pInfo;
			bRetVal = TRUE;
		}
		return bRetVal;
	}
	BOOL ExpireAll(void *pUserData = NULL)
	{
		while (1)
		{
			Lock();
			CJabberIqInfo* pInfo = m_pIqs;
			if (pInfo)
				m_pIqs = m_pIqs->m_pNext;
			Unlock();
			if ( !pInfo)
				break;
			pInfo->m_pNext = NULL;
			ExpireInfo(pInfo, pUserData);
			delete pInfo;
		}
		return TRUE;
	}
	BOOL FillPermanentHandlers();
};

#endif
