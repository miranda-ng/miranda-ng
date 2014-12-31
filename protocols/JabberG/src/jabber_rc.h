/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

XEP-0146 support for Miranda IM

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

#ifndef _JABBER_RC_H_
#define _JABBER_RC_H_

class CJabberAdhocSession;

#define JABBER_ADHOC_HANDLER_STATUS_EXECUTING            1
#define JABBER_ADHOC_HANDLER_STATUS_COMPLETED            2
#define JABBER_ADHOC_HANDLER_STATUS_CANCEL               3
#define JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION       4
typedef int (CJabberProto::*JABBER_ADHOC_HANDLER)(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession);

// 5 minutes to fill out form :)
#define JABBER_ADHOC_SESSION_EXPIRE_TIME                 300000

class CJabberAdhocSession
{
protected:
	CMString m_szSessionId;
	CJabberAdhocSession* m_pNext;
	DWORD m_dwStartTime;

	void* m_pUserData;
	BOOL m_bAutofreeUserData;

	DWORD m_dwStage;
public:
	CJabberProto *ppro;
	CJabberAdhocSession(CJabberProto* global);
	~CJabberAdhocSession()
	{
		if (m_bAutofreeUserData && m_pUserData)
			mir_free(m_pUserData);
		if (m_pNext)
			delete m_pNext;
	}
	CJabberAdhocSession* GetNext()
	{
		return m_pNext;
	}
	CJabberAdhocSession* SetNext(CJabberAdhocSession *pNext)
	{
		CJabberAdhocSession *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	DWORD GetSessionStartTime()
	{
		return m_dwStartTime;
	}
	LPCTSTR GetSessionId()
	{
		return m_szSessionId;
	}
	BOOL SetUserData(void* pUserData, BOOL bAutofree = FALSE)
	{
		if (m_bAutofreeUserData && m_pUserData)
			mir_free(m_pUserData);
		m_pUserData = pUserData;
		m_bAutofreeUserData = bAutofree;
		return TRUE;
	}
	DWORD SetStage(DWORD dwStage)
	{
		DWORD dwRetVal = m_dwStage;
		m_dwStage = dwStage;
		return dwRetVal;
	}
	DWORD GetStage()
	{
		return m_dwStage;
	}
};

class CJabberAdhocNode;
class CJabberAdhocNode
{
protected:
	TCHAR *m_szJid;
	TCHAR *m_szNode;
	TCHAR *m_szName;
	CJabberAdhocNode* m_pNext;
	JABBER_ADHOC_HANDLER m_pHandler;
	CJabberProto *m_pProto;
public:
	CJabberAdhocNode(CJabberProto* pProto, TCHAR* szJid, TCHAR* szNode, TCHAR* szName, JABBER_ADHOC_HANDLER pHandler)
	{
		memset(this, 0, sizeof(CJabberAdhocNode));
		replaceStrT(m_szJid, szJid);
		replaceStrT(m_szNode, szNode);
		replaceStrT(m_szName, szName);
		m_pHandler = pHandler;
		m_pProto = pProto;
	}
	~CJabberAdhocNode()
	{
		mir_free(m_szJid);
		mir_free(m_szNode);
		mir_free(m_szName);

		if (m_pNext)
			delete m_pNext;
	}
	CJabberAdhocNode* GetNext()
	{
		return m_pNext;
	}
	CJabberAdhocNode* SetNext(CJabberAdhocNode *pNext)
	{
		CJabberAdhocNode *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	TCHAR* GetJid()
	{
		return m_szJid;
	}
	TCHAR* GetNode()
	{
		return m_szNode;
	}
	TCHAR* GetName()
	{
		return m_szName;
	}
	BOOL CallHandler(HXML iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
	{
		if (m_pHandler == NULL)
			return FALSE;
		return (m_pProto->*m_pHandler)(iqNode, pInfo, pSession);
	}
};

class CJabberAdhocManager
{
protected:
	CJabberProto *m_pProto;
	CJabberAdhocNode* m_pNodes;
	CJabberAdhocSession* m_pSessions;
	mir_cs m_cs;

	CJabberAdhocSession* FindSession(const TCHAR *szSession)
	{
		CJabberAdhocSession* pSession = m_pSessions;
		while (pSession) {
			if (!_tcscmp(pSession->GetSessionId(), szSession))
				return pSession;
			pSession = pSession->GetNext();
		}
		return NULL;
	}

	CJabberAdhocSession* AddNewSession()
	{
		CJabberAdhocSession* pSession = new CJabberAdhocSession(m_pProto);
		if (!pSession)
			return NULL;

		pSession->SetNext(m_pSessions);
		m_pSessions = pSession;

		return pSession;
	}

	CJabberAdhocNode* FindNode(const TCHAR *szNode)
	{
		CJabberAdhocNode* pNode = m_pNodes;
		while (pNode) {
			if (!_tcscmp(pNode->GetNode(), szNode))
				return pNode;
			pNode = pNode->GetNext();
		}
		return NULL;
	}

	BOOL RemoveSession(CJabberAdhocSession* pSession)
	{
		if (!m_pSessions)
			return FALSE;

		if (pSession == m_pSessions) {
			m_pSessions = m_pSessions->GetNext();
			pSession->SetNext(NULL);
			delete pSession;
			return TRUE;
		}

		CJabberAdhocSession* pTmp = m_pSessions;
		while (pTmp->GetNext()) {
			if (pTmp->GetNext() == pSession) {
				pTmp->SetNext(pSession->GetNext());
				pSession->SetNext(NULL);
				delete pSession;
				return TRUE;
			}
			pTmp = pTmp->GetNext();
		}
		return FALSE;
	}

	BOOL _ExpireSession(DWORD dwExpireTime)
	{
		if (!m_pSessions)
			return FALSE;

		CJabberAdhocSession* pSession = m_pSessions;
		if (pSession->GetSessionStartTime() < dwExpireTime) {
			m_pSessions = pSession->GetNext();
			pSession->SetNext(NULL);
			delete pSession;
			return TRUE;
		}

		while (pSession->GetNext()) {
			if (pSession->GetNext()->GetSessionStartTime() < dwExpireTime) {
				CJabberAdhocSession* pRetVal = pSession->GetNext();
				pSession->SetNext(pSession->GetNext()->GetNext());
				pRetVal->SetNext(NULL);
				delete pRetVal;
				return TRUE;
			}
			pSession = pSession->GetNext();
		}
		return FALSE;
	}

public:
	CJabberAdhocManager(CJabberProto* pProto)
	{
		m_pProto = pProto;
		m_pNodes = NULL;
		m_pSessions = NULL;
	}
	~CJabberAdhocManager()
	{
		delete m_pNodes;
		delete m_pSessions;
	}

	BOOL FillDefaultNodes();
	BOOL AddNode(TCHAR* szJid, TCHAR* szNode, TCHAR* szName, JABBER_ADHOC_HANDLER pHandler)
	{
		CJabberAdhocNode* pNode = new CJabberAdhocNode(m_pProto, szJid, szNode, szName, pHandler);
		if (!pNode)
			return FALSE;

		mir_cslock lck(m_cs);
		if (!m_pNodes)
			m_pNodes = pNode;
		else {
			CJabberAdhocNode* pTmp = m_pNodes;
			while (pTmp->GetNext())
				pTmp = pTmp->GetNext();
			pTmp->SetNext(pNode);
		}
		return TRUE;
	}

	CJabberAdhocNode* GetFirstNode()
	{
		return m_pNodes;
	}

	BOOL HandleItemsRequest(HXML iqNode, CJabberIqInfo *pInfo, const TCHAR *szNode);
	BOOL HandleInfoRequest(HXML iqNode, CJabberIqInfo *pInfo, const TCHAR *szNode);
	BOOL HandleCommandRequest(HXML iqNode, CJabberIqInfo *pInfo, const TCHAR *szNode);

	BOOL ExpireSessions()
	{
		mir_cslock lck(m_cs);
		DWORD dwExpireTime = GetTickCount() - JABBER_ADHOC_SESSION_EXPIRE_TIME;
		while (_ExpireSession(dwExpireTime))
			;
		return TRUE;
	}
};

#endif //_JABBER_RC_H_
