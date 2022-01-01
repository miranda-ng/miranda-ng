/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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
typedef int (CJabberProto::*JABBER_ADHOC_HANDLER)(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession);

// 5 minutes to fill out form :)
#define JABBER_ADHOC_SESSION_EXPIRE_TIME                 300000

class CJabberAdhocSession
{
protected:
	CMStringA m_szSessionId;
	CJabberAdhocSession* m_pNext = nullptr;
	uint32_t m_dwStartTime;

	void* m_pUserData = nullptr;
	BOOL m_bAutofreeUserData;

	uint32_t m_dwStage = 0;

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

	uint32_t GetSessionStartTime()
	{
		return m_dwStartTime;
	}

	const char* GetSessionId()
	{
		return m_szSessionId;
	}
	
	void SetUserData(void* pUserData, BOOL bAutofree = FALSE)
	{
		if (m_bAutofreeUserData && m_pUserData)
			mir_free(m_pUserData);
		m_pUserData = pUserData;
		m_bAutofreeUserData = bAutofree;
	}

	uint32_t SetStage(uint32_t dwStage)
	{
		uint32_t dwRetVal = m_dwStage;
		m_dwStage = dwStage;
		return dwRetVal;
	}

	uint32_t GetStage()
	{
		return m_dwStage;
	}
};

class CJabberAdhocNode
{
protected:
	char *m_szJid;
	char *m_szNode;
	char *m_szName;
	CJabberAdhocNode* m_pNext = nullptr;
	JABBER_ADHOC_HANDLER m_pHandler;
	CJabberProto *m_pProto;

public:
	CJabberAdhocNode(CJabberProto* pProto, const char *szJid, const char *szNode, const char *szName, JABBER_ADHOC_HANDLER pHandler) :
		m_pProto(pProto),
		m_pHandler(pHandler)
	{
		m_szJid = mir_strdup(szJid);
		m_szNode = mir_strdup(szNode);
		m_szName = mir_strdup(szName);
	}
	
	~CJabberAdhocNode()
	{
		mir_free(m_szJid);
		mir_free(m_szNode);
		mir_free(m_szName);

		if (m_pNext)
			delete m_pNext;
	}
	
	__inline CJabberAdhocNode* GetNext() const
	{
		return m_pNext;
	}
	
	__inline CJabberAdhocNode* SetNext(CJabberAdhocNode *pNext)
	{
		CJabberAdhocNode *pRetVal = m_pNext;
		m_pNext = pNext;
		return pRetVal;
	}
	
	__inline char* GetJid() const {
		return m_szJid;
	}
	__inline char* GetNode() const {
		return m_szNode;
	}
	__inline char* GetName() const {
		return m_szName;
	}
	
	bool CallHandler(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
	{
		if (m_pHandler == nullptr)
			return false;
		return (m_pProto->*m_pHandler)(iqNode, pInfo, pSession);
	}
};

class CJabberAdhocManager
{
protected:
	CJabberProto *m_pProto;
	CJabberAdhocNode* m_pNodes = nullptr;
	CJabberAdhocSession* m_pSessions = nullptr;
	mir_cs m_cs;

	CJabberAdhocSession* FindSession(const char *szSession)
	{
		CJabberAdhocSession *pSession = m_pSessions;
		while (pSession) {
			if (!mir_strcmp(pSession->GetSessionId(), szSession))
				return pSession;
			pSession = pSession->GetNext();
		}
		return nullptr;
	}

	CJabberAdhocSession* AddNewSession()
	{
		CJabberAdhocSession *pSession = new CJabberAdhocSession(m_pProto);
		if (!pSession)
			return nullptr;

		pSession->SetNext(m_pSessions);
		m_pSessions = pSession;

		return pSession;
	}

	CJabberAdhocNode* FindNode(const char *szNode)
	{
		CJabberAdhocNode* pNode = m_pNodes;
		while (pNode) {
			if (!mir_strcmp(pNode->GetNode(), szNode))
				return pNode;
			pNode = pNode->GetNext();
		}
		return nullptr;
	}

	bool RemoveSession(CJabberAdhocSession *pSession)
	{
		if (!m_pSessions)
			return false;

		if (pSession == m_pSessions) {
			m_pSessions = m_pSessions->GetNext();
			pSession->SetNext(nullptr);
			delete pSession;
			return true;
		}

		CJabberAdhocSession* pTmp = m_pSessions;
		while (pTmp->GetNext()) {
			if (pTmp->GetNext() == pSession) {
				pTmp->SetNext(pSession->GetNext());
				pSession->SetNext(nullptr);
				delete pSession;
				return true;
			}
			pTmp = pTmp->GetNext();
		}
		return false;
	}

	bool _ExpireSession(uint32_t dwExpireTime)
	{
		if (!m_pSessions)
			return false;

		CJabberAdhocSession *pSession = m_pSessions;
		if (pSession->GetSessionStartTime() < dwExpireTime) {
			m_pSessions = pSession->GetNext();
			pSession->SetNext(nullptr);
			delete pSession;
			return true;
		}

		while (pSession->GetNext()) {
			if (pSession->GetNext()->GetSessionStartTime() < dwExpireTime) {
				CJabberAdhocSession* pRetVal = pSession->GetNext();
				pSession->SetNext(pSession->GetNext()->GetNext());
				pRetVal->SetNext(nullptr);
				delete pRetVal;
				return true;
			}
			pSession = pSession->GetNext();
		}
		return false;
	}

public:
	CJabberAdhocManager(CJabberProto* pProto) :
		m_pProto(pProto)
	{		 
	}

	~CJabberAdhocManager()
	{
		delete m_pNodes;
		delete m_pSessions;
	}

	void FillDefaultNodes();
	
	void AddNode(const char *szJid, const char *szNode, const char *szName, JABBER_ADHOC_HANDLER pHandler)
	{
		CJabberAdhocNode* pNode = new CJabberAdhocNode(m_pProto, szJid, szNode, szName, pHandler);

		mir_cslock lck(m_cs);
		if (!m_pNodes)
			m_pNodes = pNode;
		else {
			CJabberAdhocNode* pTmp = m_pNodes;
			while (pTmp->GetNext())
				pTmp = pTmp->GetNext();
			pTmp->SetNext(pNode);
		}
	}

	CJabberAdhocNode* GetFirstNode()
	{
		return m_pNodes;
	}

	bool HandleItemsRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, const char *szNode);
	bool HandleInfoRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, const char *szNode);
	bool HandleCommandRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, const char *szNode);

	void ExpireSessions()
	{
		mir_cslock lck(m_cs);
		uint32_t dwExpireTime = GetTickCount() - JABBER_ADHOC_SESSION_EXPIRE_TIME;
		while (_ExpireSession(dwExpireTime))
			;
	}
};

#endif //_JABBER_RC_H_
