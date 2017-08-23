/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_ibb.h"
#include "jabber_rc.h"

static int CompareIqs(const CJabberIqInfo *p1, const CJabberIqInfo *p2)
{	return p1->GetPriority() - p2->GetPriority();
}

static int ComparePermanent(const CJabberIqPermanentInfo *p1, const CJabberIqPermanentInfo *p2)
{	return p1->getPriority() - p2->getPriority();
}

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberIqManager class

CJabberIqManager::CJabberIqManager(CJabberProto *proto) :
	m_arIqs(10, CompareIqs),
	m_arHandlers(10, ComparePermanent),
	m_bExpirerThreadShutdownRequest(false)
{
	m_dwLastUsedHandle = 0;
	m_hExpirerThread = nullptr;
	ppro = proto;
}

CJabberIqManager::~CJabberIqManager()
{
	ExpireAll();
}

bool CJabberIqManager::Start()
{
	if (m_hExpirerThread || m_bExpirerThreadShutdownRequest)
		return false;

	m_hExpirerThread = ppro->ForkThreadEx(&CJabberProto::ExpirerThread, this, 0);
	if (!m_hExpirerThread)
		return false;

	return true;
}

void CJabberIqManager::Shutdown()
{
	if (m_bExpirerThreadShutdownRequest || !m_hExpirerThread)
		return;

	m_bExpirerThreadShutdownRequest = TRUE;

	WaitForSingleObject(m_hExpirerThread, INFINITE);
	CloseHandle(m_hExpirerThread);
	m_hExpirerThread = nullptr;
}

void CJabberIqManager::FillPermanentHandlers()
{
	// version requests (XEP-0092)
	AddPermanentHandler(&CJabberProto::OnIqRequestVersion, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_VERSION, FALSE, L"query");

	// last activity (XEP-0012)
	AddPermanentHandler(&CJabberProto::OnIqRequestLastActivity, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_LAST_ACTIVITY, FALSE, L"query");

	// ping requests (XEP-0199)
	AddPermanentHandler(&CJabberProto::OnIqRequestPing, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PING, FALSE, L"ping");

	// entity time (XEP-0202)
	AddPermanentHandler(&CJabberProto::OnIqRequestTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME, FALSE, L"time");

	// entity time (XEP-0090)
	AddPermanentHandler(&CJabberProto::OnIqProcessIqOldTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME_OLD, FALSE, L"query");

	// old avatars support (deprecated XEP-0008)
	AddPermanentHandler(&CJabberProto::OnIqRequestAvatar, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_AVATAR, FALSE, L"query");

	// privacy lists (XEP-0016)
	AddPermanentHandler(&CJabberProto::OnIqRequestPrivacyLists, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PRIVACY_LISTS, FALSE, L"query");

	// in band bytestreams (XEP-0047)
	AddPermanentHandler(&CJabberProto::OnFtHandleIbbIq, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_CHILD_TAG_NAME | JABBER_IQ_PARSE_CHILD_TAG_XMLNS, JABBER_FEAT_IBB, FALSE, nullptr);

	// socks5-bytestreams (XEP-0065)
	AddPermanentHandler(&CJabberProto::FtHandleBytestreamRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_BYTESTREAMS, FALSE, L"query");

	// session initiation (XEP-0095)
	AddPermanentHandler(&CJabberProto::OnSiRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_SI, FALSE, L"si");

	// roster push requests
	AddPermanentHandler(&CJabberProto::OnRosterPushRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_IQ_ROSTER, FALSE, L"query");

	// OOB file transfers
	AddPermanentHandler(&CJabberProto::OnIqRequestOOB, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_OOB, FALSE, L"query");

	// disco#items requests (XEP-0030, XEP-0050)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoItemsRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_ITEMS, FALSE, L"query");

	// disco#info requests (XEP-0030, XEP-0050, XEP-0115)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoInfoRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_INFO, FALSE, L"query");

	// ad-hoc commands (XEP-0050) for remote controlling (XEP-0146)
	AddPermanentHandler(&CJabberProto::HandleAdhocCommandRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_COMMANDS, FALSE, L"command");

	// http auth (XEP-0070)
	AddPermanentHandler(&CJabberProto::OnIqHttpAuth, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_HTTP_AUTH, FALSE, L"confirm");
}

void __cdecl CJabberProto::ExpirerThread(void* pParam)
{
	Thread_SetName("Jabber: ExpirerThread");
	CJabberIqManager *pManager = (CJabberIqManager *)pParam;
	pManager->ExpirerThread();
}

void CJabberIqManager::ExpirerThread()
{
	Thread_SetName("Jabber: ExpirerThread");

	while (!m_bExpirerThreadShutdownRequest) {
		CJabberIqInfo *pInfo = DetouchExpired();
		if (!pInfo) {
			for (int i=0; !m_bExpirerThreadShutdownRequest && (i < 10); i++)
				Sleep(50);

			// -1 thread :)
			ppro->m_adhocManager.ExpireSessions();
			continue;
		}
		ExpireInfo(pInfo);
	}

	if (!m_bExpirerThreadShutdownRequest) {
		CloseHandle(m_hExpirerThread);
		m_hExpirerThread = nullptr;
	}
}

void CJabberIqManager::ExpireInfo(CJabberIqInfo *pInfo)
{
	if (!pInfo)
		return;

	if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
		pInfo->m_szFrom = pInfo->m_szReceiver;
	if ((pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (pInfo->m_szFrom))
		pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom);

	ppro->debugLogW(L"Expiring iq id %d, sent to %s", pInfo->m_nIqId, pInfo->m_szReceiver ? pInfo->m_szReceiver : L"server");

	pInfo->m_nIqType = JABBER_IQ_TYPE_FAIL;
	(ppro->*(pInfo->m_pHandler))(nullptr, pInfo);

	delete pInfo;
}

void CJabberIqManager::ExpireIq(int nIqId)
{
	while (CJabberIqInfo *pInfo = DetouchInfo(nIqId))
		ExpireInfo(pInfo);
}
	
bool CJabberIqManager::ExpireByUserData(void *pUserData)
{
	bool bRet = false;
	while (CJabberIqInfo *pInfo = DetouchInfo(pUserData)) {
		ExpireInfo(pInfo);
		bRet = true;
	}
	return bRet;
}

void CJabberIqManager::ExpireAll()
{
	while (CJabberIqInfo *pInfo = DetouchInfo())
		ExpireInfo(pInfo);
}

CJabberIqInfo* CJabberIqManager::AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType, const wchar_t *szReceiver, DWORD dwParamsToParse, int nIqId, void *pUserData, int iPriority)
{
	CJabberIqInfo *pInfo = new CJabberIqInfo();
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

	mir_cslock lck(m_cs);
	m_arIqs.insert(pInfo);
	return pInfo;
}

bool CJabberIqManager::DeleteHandler(CJabberIqInfo *pInfo)
{
	// returns TRUE when pInfo found, or FALSE otherwise
	mir_cslockfull lck(m_cs);

	for (int i = 0; i < m_arIqs.getCount(); i++) {
		if (m_arIqs[i] == pInfo) {
			m_arIqs.remove(i);
			lck.unlock();
			ExpireInfo(pInfo); // must expire it to allow the handler to free m_pUserData if necessary
			return true;
		}
	}

	return false;
}

bool CJabberIqManager::HandleIq(int nIqId, HXML pNode)
{
	if (nIqId == -1 || pNode == nullptr)
		return false;

	const wchar_t *szType = XmlGetAttrValue(pNode, L"type");
	if (!szType)
		return false;

	int nIqType = JABBER_IQ_TYPE_FAIL;
	if (!mir_wstrcmpi(szType, L"result"))
		nIqType = JABBER_IQ_TYPE_RESULT;
	else if (!mir_wstrcmpi(szType, L"error"))
		nIqType = JABBER_IQ_TYPE_ERROR;
	else
		return false;

	CJabberIqInfo *pInfo = DetouchInfo(nIqId);
	if (pInfo == nullptr)
		return false;

	do {
		pInfo->m_nIqType = nIqType;
		if (nIqType == JABBER_IQ_TYPE_RESULT) {
			if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NODE)
				pInfo->m_pChildNode = XmlGetChild(pNode , 0);

			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NAME))
				pInfo->m_szChildTagName = (wchar_t*)XmlGetName(pInfo->m_pChildNode);
			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_XMLNS))
				pInfo->m_szChildTagXmlns = (wchar_t*)XmlGetAttrValue(pNode, L"xmlns");
		}

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
			pInfo->m_szTo = (wchar_t*)XmlGetAttrValue(pNode, L"to");

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
			pInfo->m_szFrom = (wchar_t*)XmlGetAttrValue(pNode, L"from");
		if (pInfo->m_szFrom && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT))
			pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom);

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_ID_STR)
			pInfo->m_szId = (wchar_t*)XmlGetAttrValue(pNode, L"id");

		(ppro->*(pInfo->m_pHandler))(pNode, pInfo);
		delete pInfo;
	}
		while ((pInfo = DetouchInfo(nIqId)) != nullptr);
	return true;
}

bool CJabberIqManager::HandleIqPermanent(HXML pNode)
{
	for (int i = 0; i < m_arHandlers.getCount(); i++) {
		CJabberIqPermanentInfo &pInfo = m_arHandlers[i];
		// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		const wchar_t *szType = XmlGetAttrValue(pNode, L"type");
		if (!szType)
			return FALSE;

		CJabberIqInfo iqInfo;
		iqInfo.m_nIqType = JABBER_IQ_TYPE_FAIL;
		if (!mir_wstrcmpi(szType, L"get"))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_GET;
		else if (!mir_wstrcmpi(szType, L"set"))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_SET;
		else
			return FALSE;

		if (!(pInfo.m_nIqTypes & iqInfo.m_nIqType))
			continue;

		HXML pFirstChild = XmlGetChild(pNode , 0);
		if (!pFirstChild || !XmlGetName(pFirstChild))
			return FALSE;

		const wchar_t *szTagName = XmlGetName(pFirstChild);
		const wchar_t *szXmlns = XmlGetAttrValue(pFirstChild, L"xmlns");

		if ((!pInfo.m_szXmlns || (szXmlns && !mir_wstrcmp(pInfo.m_szXmlns, szXmlns))) &&
			 (!pInfo.m_szTag || !mir_wstrcmp(pInfo.m_szTag, szTagName)))
		{
			// node suits handler criteria, call the handler
			iqInfo.m_pChildNode = pFirstChild;
			iqInfo.m_szChildTagName = (wchar_t*)szTagName;
			iqInfo.m_szChildTagXmlns = (wchar_t*)szXmlns;
			iqInfo.m_szId = (wchar_t*)XmlGetAttrValue(pNode, L"id");
			iqInfo.m_pUserData = pInfo.m_pUserData;

			if (pInfo.m_dwParamsToParse & JABBER_IQ_PARSE_TO)
				iqInfo.m_szTo = (wchar_t*)XmlGetAttrValue(pNode, L"to");

			if (pInfo.m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
				iqInfo.m_szFrom = (wchar_t*)XmlGetAttrValue(pNode, L"from");

			if ((pInfo.m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (iqInfo.m_szFrom))
				iqInfo.m_hContact = ppro->HContactFromJID(iqInfo.m_szFrom);

			ppro->debugLogW(L"Handling iq id %s, type %s, from %s", iqInfo.m_szId, szType, iqInfo.m_szFrom);
			if ((ppro->*(pInfo.m_pHandler))(pNode, &iqInfo))
				return true;
		}
	}

	return false;
}

CJabberIqInfo* CJabberIqManager::DetouchInfo()
{
	mir_cslock lck(m_cs);
	
	CJabberIqInfo *pInfo = m_arIqs[0];
	if (pInfo)
		m_arIqs.remove(0);
	return pInfo;
}

CJabberIqInfo* CJabberIqManager::DetouchInfo(int nIqId)
{
	mir_cslock lck(m_cs);

	for (int i = 0; i < m_arIqs.getCount(); i++) {
		CJabberIqInfo *pInfo = m_arIqs[i];
		if (pInfo->m_nIqId == nIqId) {
			m_arIqs.remove(i);
			return pInfo;
		}
	}
	return nullptr;
}

CJabberIqInfo* CJabberIqManager::DetouchInfo(void *pUserData)
{
	mir_cslock lck(m_cs);

	for (int i = 0; i < m_arIqs.getCount(); i++) {
		CJabberIqInfo *pInfo = m_arIqs[i];
		if (pInfo->m_pUserData == pUserData) {
			m_arIqs.remove(i);
			return pInfo;
		}
	}
	return nullptr;
}

CJabberIqInfo* CJabberIqManager::DetouchExpired()
{
	DWORD dwCurrentTime = GetTickCount();

	mir_cslock lck(m_cs);

	for (int i = 0; i < m_arIqs.getCount(); i++) {
		CJabberIqInfo *pInfo = m_arIqs[i];
		if (dwCurrentTime - pInfo->m_dwRequestTime > pInfo->m_dwTimeout) {
			m_arIqs.remove(i);
			return pInfo;
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

CJabberIqPermanentInfo* CJabberIqManager::AddPermanentHandler(
	JABBER_PERMANENT_IQ_HANDLER pHandler,
	int nIqTypes,
	DWORD dwParamsToParse,
	const wchar_t *szXmlns,
	BOOL bAllowPartialNs,
	const wchar_t *szTag,
	void *pUserData,
	IQ_USER_DATA_FREE_FUNC pUserDataFree,
	int iPriority)
{
	CJabberIqPermanentInfo *pInfo = new CJabberIqPermanentInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_nIqTypes = nIqTypes ? nIqTypes : JABBER_IQ_TYPE_ANY;
	pInfo->m_szXmlns = mir_wstrdup(szXmlns);
	pInfo->m_bAllowPartialNs = bAllowPartialNs;
	pInfo->m_szTag = mir_wstrdup(szTag);
	pInfo->m_dwParamsToParse = dwParamsToParse;
	pInfo->m_pUserData = pUserData;
	pInfo->m_pUserDataFree = pUserDataFree;
	pInfo->m_iPriority = iPriority;

	mir_cslock lck(m_cs);
	m_arHandlers.insert(pInfo);
	return pInfo;
}

// returns TRUE when pInfo found, or FALSE otherwise
bool CJabberIqManager::DeletePermanentHandler(CJabberIqPermanentInfo *pInfo)
{		
	mir_cslock lck(m_cs);
	return m_arHandlers.remove(pInfo) == 1;
}
