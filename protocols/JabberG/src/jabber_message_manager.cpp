/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
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

#include "jabber.h"
#include "jabber_message_manager.h"

CJabberMessageManager::CJabberMessageManager(CJabberProto *proto)
{
	InitializeCriticalSection(&m_cs);
	m_pPermanentHandlers = NULL;
	ppro = proto;
}
CJabberMessageManager::~CJabberMessageManager()
{
	CJabberMessagePermanentInfo *pInfo = m_pPermanentHandlers;
	while (pInfo) {
		CJabberMessagePermanentInfo *pTmp = pInfo->m_pNext;
		delete pInfo;
		pInfo = pTmp;
	}
	m_pPermanentHandlers = NULL;

	DeleteCriticalSection(&m_cs);
}

void CJabberMessageManager::FillPermanentHandlers()
{
	AddPermanentHandler(&CJabberProto::OnMessageError, JABBER_MESSAGE_TYPE_ERROR, JABBER_MESSAGE_PARSE_FROM | JABBER_MESSAGE_PARSE_HCONTACT, NULL, FALSE, _T("error"));
	AddPermanentHandler(&CJabberProto::OnMessageIbb, 0, 0, JABBER_FEAT_IBB, FALSE, _T("data"));
	AddPermanentHandler(&CJabberProto::OnMessagePubsubEvent, 0, 0, JABBER_FEAT_PUBSUB_EVENT, FALSE, _T("event"));
	AddPermanentHandler(&CJabberProto::OnMessageGroupchat, JABBER_MESSAGE_TYPE_GROUPCHAT, JABBER_MESSAGE_PARSE_FROM, NULL, FALSE, NULL);
}

bool CJabberMessageManager::HandleMessagePermanent(HXML node, ThreadData *pThreadData)
{
	mir_cslock lck(m_cs);

	CJabberMessagePermanentInfo *pInfo = m_pPermanentHandlers;
	while (pInfo) {
		// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		CJabberMessageInfo messageInfo;

		LPCTSTR szType = xmlGetAttrValue(node, _T("type"));
		if (szType) {
			if (!_tcsicmp(szType, _T("normal")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;
			else if (!_tcsicmp(szType, _T("error")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_ERROR;
			else if (!_tcsicmp(szType, _T("chat")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_CHAT;
			else if (!_tcsicmp(szType, _T("groupchat")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_GROUPCHAT;
			else if (!_tcsicmp(szType, _T("headline")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_HEADLINE;
			else
				return FALSE;
		}
		else messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;

		if (pInfo->m_nMessageTypes & messageInfo.m_nMessageType) {
			for (int i = xmlGetChildCount(node) - 1; i >= 0; i--) {
				// enumerate all children and see whether this node suits handler criteria
				HXML child = xmlGetChild(node, i);

				LPCTSTR szTagName = xmlGetName(child);
				LPCTSTR szXmlns = xmlGetAttrValue(child, _T("xmlns"));

				if ((!pInfo->m_szXmlns || (szXmlns && !_tcscmp(pInfo->m_szXmlns, szXmlns))) && (!pInfo->m_szTag || !_tcscmp(pInfo->m_szTag, szTagName))) {
					// node suits handler criteria, call the handler
					messageInfo.m_hChildNode = child;
					messageInfo.m_szChildTagName = szTagName;
					messageInfo.m_szChildTagXmlns = szXmlns;
					messageInfo.m_pUserData = pInfo->m_pUserData;
					messageInfo.m_szFrom = xmlGetAttrValue(node, _T("from")); // is necessary for ppro->debugLogA() below, that's why we must parse it even if JABBER_MESSAGE_PARSE_FROM flag is not set

					if (pInfo->m_dwParamsToParse & JABBER_MESSAGE_PARSE_ID_STR)
						messageInfo.m_szId = xmlGetAttrValue(node, _T("id"));

					if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
						messageInfo.m_szTo = xmlGetAttrValue(node, _T("to"));

					if (pInfo->m_dwParamsToParse & JABBER_MESSAGE_PARSE_HCONTACT)
						messageInfo.m_hContact = ppro->HContactFromJID(messageInfo.m_szFrom, 3);

					if (messageInfo.m_szFrom)
						ppro->debugLog(_T("Handling message from %s"), messageInfo.m_szFrom);
					if ((ppro->*(pInfo->m_pHandler))(node, pThreadData, &messageInfo))
						return TRUE;
				}
			}
		}
		pInfo = pInfo->m_pNext;
	}

	return FALSE;
}

CJabberMessagePermanentInfo* CJabberMessageManager::AddPermanentHandler(
	JABBER_PERMANENT_MESSAGE_HANDLER pHandler,
	int nMessageTypes,
	DWORD dwParamsToParse,
	const TCHAR *szXmlns,
	BOOL bAllowPartialNs,
	const TCHAR *szTag,
	void *pUserData,
	MESSAGE_USER_DATA_FREE_FUNC pUserDataFree,
	int iPriority)
{
	CJabberMessagePermanentInfo* pInfo = new CJabberMessagePermanentInfo();
	if (pInfo == NULL)
		return NULL;

	pInfo->m_pHandler = pHandler;
	pInfo->m_nMessageTypes = nMessageTypes ? nMessageTypes : JABBER_MESSAGE_TYPE_ANY;
	replaceStrT(pInfo->m_szXmlns, szXmlns);
	pInfo->m_bAllowPartialNs = bAllowPartialNs;
	replaceStrT(pInfo->m_szTag, szTag);
	pInfo->m_dwParamsToParse = dwParamsToParse;
	pInfo->m_pUserData = pUserData;
	pInfo->m_pUserDataFree = pUserDataFree;
	pInfo->m_iPriority = iPriority;

	mir_cslock lck(m_cs);
	if (!m_pPermanentHandlers)
		m_pPermanentHandlers = pInfo;
	else {
		if (m_pPermanentHandlers->m_iPriority > pInfo->m_iPriority) {
			pInfo->m_pNext = m_pPermanentHandlers;
			m_pPermanentHandlers = pInfo;
		}
		else {
			CJabberMessagePermanentInfo *pTmp = m_pPermanentHandlers;
			while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
				pTmp = pTmp->m_pNext;
			pInfo->m_pNext = pTmp->m_pNext;
			pTmp->m_pNext = pInfo;
		}
	}
	return pInfo;
}

bool CJabberMessageManager::DeletePermanentHandler(CJabberMessagePermanentInfo *pInfo)
{ 
	mir_cslock lck(m_cs);
	if (!m_pPermanentHandlers)
		return FALSE;

	if (m_pPermanentHandlers == pInfo) { // check first item
		m_pPermanentHandlers = m_pPermanentHandlers->m_pNext;
		delete pInfo;
		return TRUE;
	}

	CJabberMessagePermanentInfo *pTmp = m_pPermanentHandlers;
	while (pTmp->m_pNext) {
		if (pTmp->m_pNext == pInfo) {
			pTmp->m_pNext = pTmp->m_pNext->m_pNext;
			delete pInfo;
			return TRUE;
		}
		pTmp = pTmp->m_pNext;
	}
	return FALSE;
}
