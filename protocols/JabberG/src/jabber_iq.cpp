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

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_ibb.h"
#include "jabber_rc.h"

static int CompareIqs(const CJabberIqInfo *p1, const CJabberIqInfo *p2)
{
	return p1->GetPriority() - p2->GetPriority();
}

static int ComparePermanent(const CJabberIqPermanentInfo *p1, const CJabberIqPermanentInfo *p2)
{
	return p1->getPriority() - p2->getPriority();
}

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberIqManager class

CJabberIqManager::CJabberIqManager(CJabberProto *proto) :
	ppro(proto),
	m_arIqs(10, CompareIqs),
	m_arHandlers(10, ComparePermanent)
{
}

CJabberIqManager::~CJabberIqManager()
{
	ExpireAll();
}

void CJabberIqManager::FillPermanentHandlers()
{
	// version requests (XEP-0092)
	AddPermanentHandler(&CJabberProto::OnIqRequestVersion, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_VERSION, FALSE, "query");

	// last activity (XEP-0012)
	AddPermanentHandler(&CJabberProto::OnIqRequestLastActivity, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_LAST_ACTIVITY, FALSE, "query");

	// ping requests (XEP-0199)
	AddPermanentHandler(&CJabberProto::OnIqRequestPing, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PING, FALSE, "ping");

	// entity time (XEP-0202)
	AddPermanentHandler(&CJabberProto::OnIqRequestTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME, FALSE, "time");

	// entity time (XEP-0090)
	AddPermanentHandler(&CJabberProto::OnIqProcessIqOldTime, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_ENTITY_TIME_OLD, FALSE, "query");

	// privacy lists (XEP-0016)
	AddPermanentHandler(&CJabberProto::OnIqRequestPrivacyLists, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR, JABBER_FEAT_PRIVACY_LISTS, FALSE, "query");

	// in band bytestreams (XEP-0047)
	AddPermanentHandler(&CJabberProto::OnFtHandleIbbIq, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE | JABBER_IQ_PARSE_CHILD_TAG_NAME | JABBER_IQ_PARSE_CHILD_TAG_XMLNS, JABBER_FEAT_IBB, FALSE, nullptr);

	// socks5-bytestreams (XEP-0065)
	AddPermanentHandler(&CJabberProto::FtHandleBytestreamRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_BYTESTREAMS, FALSE, "query");

	// session initiation (XEP-0095)
	AddPermanentHandler(&CJabberProto::OnSiRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_SI, FALSE, "si");

	// roster push requests
	AddPermanentHandler(&CJabberProto::OnRosterPushRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_IQ_ROSTER, FALSE, "query");

	// OOB file transfers
	AddPermanentHandler(&CJabberProto::OnIqRequestOOB, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_OOB, FALSE, "query");

	// disco#items requests (XEP-0030, XEP-0050)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoItemsRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_ITEMS, FALSE, "query");

	// disco#info requests (XEP-0030, XEP-0050, XEP-0115)
	AddPermanentHandler(&CJabberProto::OnHandleDiscoInfoRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_DISCO_INFO, FALSE, "query");

	// ad-hoc commands (XEP-0050) for remote controlling (XEP-0146)
	AddPermanentHandler(&CJabberProto::HandleAdhocCommandRequest, JABBER_IQ_TYPE_SET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_COMMANDS, FALSE, "command");

	// http auth (XEP-0070)
	AddPermanentHandler(&CJabberProto::OnIqHttpAuth, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_HTTP_AUTH, FALSE, "confirm");

	// XEP-0231 support
	AddPermanentHandler(&CJabberProto::FtHandleCidRequest, JABBER_IQ_TYPE_GET, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO | JABBER_IQ_PARSE_ID_STR | JABBER_IQ_PARSE_CHILD_TAG_NODE, JABBER_FEAT_BITS, FALSE, "data");
}

void CJabberIqManager::ExpireInfo(CJabberIqInfo *pInfo)
{
	if (!pInfo)
		return;

	if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
		pInfo->m_szFrom = pInfo->m_szReceiver;
	if ((pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (pInfo->m_szFrom))
		pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom);

	ppro->debugLogA("Expiring iq id %d, sent to %s", pInfo->m_nIqId, pInfo->m_szReceiver ? pInfo->m_szReceiver : "server");

	pInfo->m_nIqType = JABBER_IQ_TYPE_FAIL;
	(ppro->*(pInfo->m_pHandler))(nullptr, pInfo);

	delete pInfo;
}

void CJabberIqManager::CheckExpired()
{
	while (auto *pInfo = DetachExpired())
		ExpireInfo(pInfo);
}

void CJabberIqManager::ExpireIq(int nIqId)
{
	while (CJabberIqInfo *pInfo = DetachInfo(nIqId))
		ExpireInfo(pInfo);
}

bool CJabberIqManager::ExpireByUserData(void *pUserData)
{
	bool bRet = false;
	while (CJabberIqInfo *pInfo = DetachInfo(pUserData)) {
		ExpireInfo(pInfo);
		bRet = true;
	}
	return bRet;
}

void CJabberIqManager::ExpireAll()
{
	while (CJabberIqInfo *pInfo = DetachInfo())
		ExpireInfo(pInfo);
}

CJabberIqInfo* CJabberIqManager::AddHandler(JABBER_IQ_HANDLER pHandler, int nIqType, const char *szReceiver, void *pUserData, int iPriority)
{
	CJabberIqInfo *pInfo = new CJabberIqInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_nIqId = ppro->SerialNext();
	pInfo->m_nIqType = nIqType;
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

	for (auto &it : m_arIqs) {
		if (it == pInfo) {
			m_arIqs.removeItem(&it);
			lck.unlock();
			ExpireInfo(pInfo); // must expire it to allow the handler to free m_pUserData if necessary
			return true;
		}
	}

	return false;
}

bool CJabberIqManager::HandleIq(int nIqId, const TiXmlElement *pNode)
{
	if (nIqId == -1 || pNode == nullptr)
		return false;

	const char *szType = XmlGetAttr(pNode, "type");
	if (!szType)
		return false;

	int nIqType = JABBER_IQ_TYPE_FAIL;
	if (!mir_strcmpi(szType, "result"))
		nIqType = JABBER_IQ_TYPE_RESULT;
	else if (!mir_strcmpi(szType, "error"))
		nIqType = JABBER_IQ_TYPE_ERROR;
	else
		return false;

	CJabberIqInfo *pInfo = DetachInfo(nIqId);
	if (pInfo == nullptr)
		return false;

	do {
		pInfo->m_nIqType = nIqType;
		if (nIqType == JABBER_IQ_TYPE_RESULT) {
			if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NODE)
				pInfo->m_pChildNode = XmlFirstChild(pNode);

			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_NAME))
				pInfo->m_szChildTagName = pInfo->m_pChildNode->Name();
			if (pInfo->m_pChildNode && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_CHILD_TAG_XMLNS))
				pInfo->m_szChildTagXmlns = XmlGetAttr(pNode, "xmlns");
		}

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
			pInfo->m_szTo = XmlGetAttr(pNode, "to");

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
			pInfo->m_szFrom = XmlGetAttr(pNode, "from");
		if (pInfo->m_szFrom && (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT))
			pInfo->m_hContact = ppro->HContactFromJID(pInfo->m_szFrom);

		if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_ID_STR)
			pInfo->m_szId = XmlGetAttr(pNode, "id");

		if (pInfo->m_pHandler)
			(ppro->*(pInfo->m_pHandler))(pNode, pInfo);
		delete pInfo;
	}
	while ((pInfo = DetachInfo(nIqId)) != nullptr);
	return true;
}

bool CJabberIqManager::HandleIqPermanent(const TiXmlElement *pNode)
{
	for (auto &pInfo : m_arHandlers) {
		// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		const char *szType = XmlGetAttr(pNode, "type");
		if (!szType)
			return false;

		CJabberIqInfo iqInfo;
		iqInfo.m_nIqType = JABBER_IQ_TYPE_FAIL;
		if (!mir_strcmpi(szType, "get"))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_GET;
		else if (!mir_strcmpi(szType, "set"))
			iqInfo.m_nIqType = JABBER_IQ_TYPE_SET;
		else
			return false;

		if (!(pInfo->m_nIqTypes & iqInfo.m_nIqType))
			continue;

		auto *pFirstChild = XmlFirstChild(pNode);
		if (!pFirstChild || !pFirstChild->Name())
			return false;

		const char *szTagName = pFirstChild->Name();
		const char *szXmlns = XmlGetAttr(pFirstChild, "xmlns");

		if ((!pInfo->m_szXmlns || (szXmlns && !mir_strcmp(pInfo->m_szXmlns, szXmlns))) &&
			(!pInfo->m_szTag || !mir_strcmp(pInfo->m_szTag, szTagName))) {
			// node suits handler criteria, call the handler
			iqInfo.m_pChildNode = pFirstChild;
			iqInfo.m_szChildTagName = szTagName;
			iqInfo.m_szChildTagXmlns = szXmlns;
			iqInfo.m_szId = XmlGetAttr(pNode, "id");
			iqInfo.m_pUserData = pInfo->m_pUserData;

			if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
				iqInfo.m_szTo = XmlGetAttr(pNode, "to");

			if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_FROM)
				iqInfo.m_szFrom = XmlGetAttr(pNode, "from");

			if ((pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_HCONTACT) && (iqInfo.m_szFrom))
				iqInfo.m_hContact = ppro->HContactFromJID(iqInfo.m_szFrom);

			ppro->debugLogA("Handling iq id %s, type %s, from %s", iqInfo.m_szId, szType, iqInfo.m_szFrom);
			if ((ppro->*(pInfo->m_pHandler))(pNode, &iqInfo))
				return true;
		}
	}

	return false;
}

CJabberIqInfo* CJabberIqManager::DetachInfo()
{
	mir_cslock lck(m_cs);

	CJabberIqInfo *pInfo = m_arIqs[0];
	if (pInfo)
		m_arIqs.remove(0);
	return pInfo;
}

CJabberIqInfo* CJabberIqManager::DetachInfo(int nIqId)
{
	mir_cslock lck(m_cs);

	for (auto &it : m_arIqs)
		if (it->m_nIqId == nIqId)
			return m_arIqs.removeItem(&it);

	return nullptr;
}

CJabberIqInfo* CJabberIqManager::DetachInfo(void *pUserData)
{
	mir_cslock lck(m_cs);

	for (auto &it : m_arIqs)
		if (it->m_pUserData == pUserData)
			return m_arIqs.removeItem(&it);

	return nullptr;
}

CJabberIqInfo* CJabberIqManager::DetachExpired()
{
	uint32_t dwCurrentTime = GetTickCount();

	mir_cslock lck(m_cs);

	for (auto &it : m_arIqs)
		if (dwCurrentTime - it->m_dwRequestTime > it->m_dwTimeout)
			return m_arIqs.removeItem(&it);

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

CJabberIqPermanentInfo* CJabberIqManager::AddPermanentHandler(
	JABBER_PERMANENT_IQ_HANDLER pHandler,
	int nIqTypes,
	uint32_t dwParamsToParse,
	const char *szXmlns,
	BOOL bAllowPartialNs,
	const char *szTag,
	void *pUserData,
	IQ_USER_DATA_FREE_FUNC pUserDataFree,
	int iPriority)
{
	CJabberIqPermanentInfo *pInfo = new CJabberIqPermanentInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_nIqTypes = nIqTypes ? nIqTypes : JABBER_IQ_TYPE_ANY;
	pInfo->m_szXmlns = mir_strdup(szXmlns);
	pInfo->m_bAllowPartialNs = bAllowPartialNs;
	pInfo->m_szTag = mir_strdup(szTag);
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
