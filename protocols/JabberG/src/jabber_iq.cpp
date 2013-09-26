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

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_ibb.h"
#include "jabber_rc.h"

void CJabberProto::IqInit()
{
	InitializeCriticalSection(&m_csIqList);
	m_ppIqList = NULL;
	m_nIqCount = 0;
	m_nIqAlloced = 0;
}

void CJabberProto::IqUninit()
{
	if (m_ppIqList) mir_free(m_ppIqList);
	m_ppIqList = NULL;
	m_nIqCount = 0;
	m_nIqAlloced = 0;
	DeleteCriticalSection(&m_csIqList);
}

void CJabberProto::IqRemove(int index)
{
	mir_cslock lck(m_csIqList);
	if (index>=0 && index<m_nIqCount) {
		memmove(m_ppIqList+index, m_ppIqList+index+1, sizeof(JABBER_IQ_FUNC)*(m_nIqCount-index-1));
		m_nIqCount--;
	}
}

void CJabberProto::IqExpire()
{
	time_t expire = time(NULL) - 120;	// 2 minute

	mir_cslock lck(m_csIqList);
	for (int i=0; i < m_nIqCount; ) {
		if (m_ppIqList[i].requestTime < expire)
			IqRemove(i);
		else
			i++;
	}
}

JABBER_IQ_PFUNC CJabberProto::JabberIqFetchFunc(int iqId)
{
	mir_cslock lck(m_csIqList);
	IqExpire();

	int i;
	for (i=0; i<m_nIqCount && m_ppIqList[i].iqId!=iqId; i++);
	if (i < m_nIqCount) {
		JABBER_IQ_PFUNC res = m_ppIqList[i].func;
		IqRemove(i);
		return res;
	}
	return NULL;
}

void CJabberProto::IqAdd(unsigned int iqId, JABBER_IQ_PROCID procId, JABBER_IQ_PFUNC func)
{
	int i;

	mir_cslock lck(m_csIqList);
	Log("IqAdd id=%d, proc=%d, func=0x%x", iqId, procId, func);
	if (procId == IQ_PROC_NONE)
		i = m_nIqCount;
	else
		for (i=0; i<m_nIqCount && m_ppIqList[i].procId!=procId; i++);

	if (i >= m_nIqCount && m_nIqCount >= m_nIqAlloced) {
		m_nIqAlloced = m_nIqCount + 8;
		m_ppIqList = (JABBER_IQ_FUNC *)mir_realloc(m_ppIqList, sizeof(JABBER_IQ_FUNC)*m_nIqAlloced);
	}

	if (m_ppIqList != NULL) {
		m_ppIqList[i].iqId = iqId;
		m_ppIqList[i].procId = procId;
		m_ppIqList[i].func = func;
		m_ppIqList[i].requestTime = time(NULL);
		if (i == m_nIqCount)
			m_nIqCount++;
	}
}

BOOL CJabberIqManager::FillPermanentHandlers()
{
	// Google Shared Status (http://code.google.com/apis/talk/jep_extensions/shared_status.html)
	AddPermanentHandler(&CJabberProto::OnIqSetGoogleSharedStatus, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR, _T("google:shared-status"), FALSE, _T("query"));

	// version requests (XEP-0092)
	AddPermanentHandler(&CJabberProto::OnIqRequestVersion, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_VERSION, FALSE, _T("query"));

	// last activity (XEP-0012)
	AddPermanentHandler(&CJabberProto::OnIqRequestLastActivity, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_LAST_ACTIVITY, FALSE, _T("query"));

	// ping requests (XEP-0199)
	AddPermanentHandler(&CJabberProto::OnIqRequestPing, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PING, FALSE, _T("ping"));

	// entity time (XEP-0202)
	AddPermanentHandler(&CJabberProto::OnIqRequestTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME, FALSE, _T("time"));

	// entity time (XEP-0090)
	AddPermanentHandler(&CJabberProto::OnIqProcessIqOldTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME_OLD, FALSE, _T("query"));

	// old avatars support (deprecated XEP-0008)
	AddPermanentHandler(&CJabberProto::OnIqRequestAvatar, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_AVATAR, FALSE, _T("query"));

	// privacy lists (XEP-0016)
	AddPermanentHandler(&CJabberProto::OnIqRequestPrivacyLists, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PRIVACY_LISTS, FALSE, _T("query"));

	// in band bytestreams (XEP-0047)
	AddPermanentHandler(&CJabberProto::OnFtHandleIbbIq, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_CHILD_TAG_NAME | JABBER_IQ_PARSE_CHILD_TAG_XMLNS, JABBER_FEAT_IBB, FALSE, NULL);

	// socks5-bytestreams (XEP-0065)
	AddPermanentHandler(&CJabberProto::FtHandleBytestreamRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_BYTESTREAMS, FALSE, _T("query"));

	// session initiation (XEP-0095)
	AddPermanentHandler(&CJabberProto::OnSiRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_SI, FALSE, _T("si"));

	// roster push requests
	AddPermanentHandler(&CJabberProto::OnRosterPushRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_IQ_ROSTER, FALSE, _T("query"));

	// OOB file transfers
	AddPermanentHandler(&CJabberProto::OnIqRequestOOB, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_OOB, FALSE, _T("query"));

	// disco#items requests (XEP-0030, XEP-0050)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoItemsRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_ITEMS, FALSE, _T("query"));

	// disco#info requests (XEP-0030, XEP-0050, XEP-0115)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoInfoRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_INFO, FALSE, _T("query"));

	// ad-hoc commands (XEP-0050) for remote controlling (XEP-0146)
	AddPermanentHandler(&CJabberProto::HandleAdhocCommandRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_COMMANDS, FALSE, _T("command"));

	// http auth (XEP-0070)
	AddPermanentHandler(&CJabberProto::OnIqHttpAuth, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_HTTP_AUTH, FALSE, _T("confirm"));

	return TRUE;
}

BOOL CJabberIqManager::Start()
{
	if (m_hExpirerThread || m_bExpirerThreadShutdownRequest)
		return FALSE;

	m_hExpirerThread = ppro->ForkThreadEx(&CJabberProto::ExpirerThread, this, 0);
	if ( !m_hExpirerThread)
		return FALSE;

	return TRUE;
}

void __cdecl CJabberProto::ExpirerThread(void* pParam)
{
	CJabberIqManager *pManager = (CJabberIqManager *)pParam;
	pManager->ExpirerThread();
}

void CJabberIqManager::ExpirerThread()
{
	while (!m_bExpirerThreadShutdownRequest)
	{
		Lock();
		CJabberIqInfo* pInfo = DetachExpired();
		Unlock();
		if ( !pInfo)
		{
			for (int i=0; !m_bExpirerThreadShutdownRequest && (i < 10); i++)
				Sleep(50);

			// -1 thread :)
			ppro->m_adhocManager.ExpireSessions();
			continue;
		}
		ExpireInfo(pInfo);
		delete pInfo;
	}

	if ( !m_bExpirerThreadShutdownRequest) {
		CloseHandle(m_hExpirerThread);
		m_hExpirerThread = NULL;
	}
}

void CJabberIqManager::ExpireInfo(CJabberIqInfo* pInfo, void*)
{
	if ( !pInfo)
		return;

	if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
		pInfo->m_szFrom = pInfo->m_szReceiver;
	if ((pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (pInfo->m_szFrom))
		pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom , 3);

	ppro->Log("Expiring iq id %d, sent to %S", pInfo->m_nIqId, pInfo->m_szReceiver ? pInfo->m_szReceiver : _T("server"));

	pInfo->m_nIqType = JABBER_IQ_TYPE_FAIL;
	(ppro->*(pInfo->m_pHandler))(NULL, pInfo);
}

CJabberIqInfo* CJabberIqManager::AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType, const TCHAR *szReceiver, DWORD dwParamsToParse, int nIqId, void *pUserData, int iPriority)
{
	CJabberIqInfo* pInfo = new CJabberIqInfo();
	if ( !pInfo)
		return NULL;

	pInfo->m_pHandler = pHandler;
	if (nIqId == -1)
		nIqId = ppro->SerialNext();
	pInfo->m_nIqId = nIqId;
	pInfo->m_nIqType = nIqType;
	pInfo->m_dwParamsToParse = dwParamsToParse;
	pInfo->m_pUserData = pUserData;
	pInfo->m_dwRequestTime = GetTickCount();
	pInfo->m_dwTimeout = JABBER_DEFAULT_IQ_REQUEST_TIMEOUT;
	pInfo->m_iPriority = iPriority;
	pInfo->SetReceiver(szReceiver);

	InsertIq(pInfo);

	return pInfo;
}

BOOL CJabberIqManager::HandleIq(int nIqId, HXML pNode)
{
	if (nIqId == -1 || pNode == NULL)
		return FALSE;

	const TCHAR *szType = xmlGetAttrValue(pNode, _T("type"));
	if ( !szType)
		return FALSE;

	int nIqType = JABBER_IQ_TYPE_FAIL;
	if ( !_tcsicmp(szType, _T("result")))
		nIqType = JABBER_IQ_TYPE_RESULT;
	else if ( !_tcsicmp(szType, _T("error")))
		nIqType = JABBER_IQ_TYPE_ERROR;
	else
		return FALSE;

	Lock();
	CJabberIqInfo* pInfo = DetachInfo(nIqId);
	Unlock();
	if (pInfo)
	{
		pInfo->m_nIqType = nIqType;
		if (nIqType == JABBER_IQ_TYPE_RESULT) {
			if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NODE)
				pInfo->m_pChildNode = xmlGetChild(pNode , 0);

			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NAME))
				pInfo->m_szChildTagName = (TCHAR*)xmlGetName(pInfo->m_pChildNode);
			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_XMLNS))
				pInfo->m_szChildTagXmlns = (TCHAR*)xmlGetAttrValue(pNode, _T("xmlns"));
		}

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
			pInfo->m_szTo = (TCHAR*)xmlGetAttrValue(pNode, _T("to"));

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
			pInfo->m_szFrom = (TCHAR*)xmlGetAttrValue(pNode, _T("from"));
		if (pInfo->m_szFrom && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT))
			pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom, 3);

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_ID_STR)
			pInfo->m_szId = (TCHAR*)xmlGetAttrValue(pNode, _T("id"));

		(ppro->*(pInfo->m_pHandler))(pNode, pInfo);
		delete pInfo;
		return TRUE;
	}
	return FALSE;
}

BOOL CJabberIqManager::HandleIqPermanent(HXML pNode)
{
	BOOL bStopHandling = FALSE;
	Lock();
	CJabberIqPermanentInfo *pInfo = m_pPermanentHandlers;
	while (pInfo) {
		// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		const TCHAR *szType = xmlGetAttrValue(pNode, _T("type"));
		if ( !szType)
			break;

		CJabberIqInfo iqInfo;

		iqInfo.m_nIqType = JABBER_IQ_TYPE_FAIL;
		if ( !_tcsicmp(szType, _T("get")))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_GET;
		else if ( !_tcsicmp(szType, _T("set")))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_SET;
		else
			break;

		if (pInfo->m_nIqTypes & iqInfo.m_nIqType)
		{
			HXML pFirstChild = xmlGetChild(pNode , 0);
			if ( !pFirstChild || !xmlGetName(pFirstChild))
				break;

			const TCHAR *szTagName = xmlGetName(pFirstChild);
			const TCHAR *szXmlns = xmlGetAttrValue(pFirstChild, _T("xmlns"));

			if ((!pInfo->m_szXmlns || (szXmlns && !_tcscmp(pInfo->m_szXmlns, szXmlns))) &&
			(!pInfo->m_szTag || !_tcscmp(pInfo->m_szTag, szTagName))) {
			// node suits handler criteria, call the handler
				iqInfo.m_pChildNode = pFirstChild;
				iqInfo.m_szChildTagName = (TCHAR*)szTagName;
				iqInfo.m_szChildTagXmlns = (TCHAR*)szXmlns;
				iqInfo.m_szId = (TCHAR*)xmlGetAttrValue(pNode, _T("id"));
				iqInfo.m_pUserData = pInfo->m_pUserData;

				if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
					iqInfo.m_szTo = (TCHAR*)xmlGetAttrValue(pNode, _T("to"));

				if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
					iqInfo.m_szFrom = (TCHAR*)xmlGetAttrValue(pNode, _T("from"));

				if ((pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (iqInfo.m_szFrom))
					iqInfo.m_hContact = ppro->HContactFromJID(iqInfo.m_szFrom, 3);

				ppro->Log("Handling iq id %S, type %S, from %S", iqInfo.m_szId, szType, iqInfo.m_szFrom);
				if ((ppro->*(pInfo->m_pHandler))(pNode, &iqInfo)) {
					bStopHandling = TRUE;
					break;
				}
			}
		}

		pInfo = pInfo->m_pNext;
	}
	Unlock();

	return bStopHandling;
}
