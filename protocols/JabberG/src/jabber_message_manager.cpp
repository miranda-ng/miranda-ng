/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
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

static int CompareItems(const CJabberMessagePermanentInfo *p1, const CJabberMessagePermanentInfo *p2)
{
	return p1->getPriority() - p2->getPriority();
}

CJabberMessageManager::CJabberMessageManager(CJabberProto *proto) :
	m_arHandlers(1, &CompareItems)
{
	ppro = proto;
}

CJabberMessageManager::~CJabberMessageManager()
{
}

void CJabberMessageManager::FillPermanentHandlers()
{
	AddPermanentHandler(&CJabberProto::OnMessageError, JABBER_MESSAGE_TYPE_ERROR, JABBER_MESSAGE_PARSE_FROM | JABBER_MESSAGE_PARSE_HCONTACT, nullptr, FALSE, "error");
	AddPermanentHandler(&CJabberProto::OnMessageIbb, 0, 0, JABBER_FEAT_IBB, FALSE, "data");
	AddPermanentHandler(&CJabberProto::OnMessagePubsubEvent, 0, 0, JABBER_FEAT_PUBSUB_EVENT, FALSE, "event");
	AddPermanentHandler(&CJabberProto::OnMessageGroupchat, JABBER_MESSAGE_TYPE_GROUPCHAT, JABBER_MESSAGE_PARSE_FROM, nullptr, FALSE, nullptr);
}

bool CJabberMessageManager::HandleMessagePermanent(const TiXmlElement *node, ThreadData *pThreadData)
{
	for (auto &it : m_arHandlers) {
		// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		CJabberMessageInfo messageInfo;

		const char *szType = XmlGetAttr(node, "type");
		if (szType) {
			if (!mir_strcmpi(szType, "normal"))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;
			else if (!mir_strcmpi(szType, "error"))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_ERROR;
			else if (!mir_strcmpi(szType, "chat"))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_CHAT;
			else if (!mir_strcmpi(szType, "groupchat"))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_GROUPCHAT;
			else if (!mir_strcmpi(szType, "headline"))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_HEADLINE;
			else
				return false;
		}
		else messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;

		if (it->m_nMessageTypes & messageInfo.m_nMessageType) {
			// enumerate all children and see whether this node suits handler criteria
			for (auto *child : TiXmlEnum(node)) {
				const char *szTagName = child->Name();
				const char *szXmlns = XmlGetAttr(child, "xmlns");

				if ((!it->m_szXmlns || (szXmlns && !mir_strcmp(it->m_szXmlns, szXmlns))) && (!it->m_szTag || !mir_strcmp(it->m_szTag, szTagName))) {
					// node suits handler criteria, call the handler
					messageInfo.m_hChildNode = child;
					messageInfo.m_szChildTagName = szTagName;
					messageInfo.m_szChildTagXmlns = szXmlns;
					messageInfo.m_pUserData = it->m_pUserData;
					messageInfo.m_szFrom = XmlGetAttr(node, "from"); // is necessary for ppro->debugLogA() below, that's why we must parse it even if JABBER_MESSAGE_PARSE_FROM flag is not set

					if (it->m_dwParamsToParse & JABBER_MESSAGE_PARSE_ID_STR)
						messageInfo.m_szId = XmlGetAttr(node, "id");

					if (it->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
						messageInfo.m_szTo = XmlGetAttr(node, "to");

					if (it->m_dwParamsToParse & JABBER_MESSAGE_PARSE_HCONTACT)
						messageInfo.m_hContact = ppro->HContactFromJID(messageInfo.m_szFrom);

					if (messageInfo.m_szFrom)
						ppro->debugLogA("Handling message from %s", messageInfo.m_szFrom);
					if ((ppro->*(it->m_pHandler))(node, pThreadData, &messageInfo))
						return true;
				}
			}
		}
	}

	return false;
}

CJabberMessagePermanentInfo* CJabberMessageManager::AddPermanentHandler(
	JABBER_PERMANENT_MESSAGE_HANDLER pHandler,
	int nMessageTypes,
	uint32_t dwParamsToParse,
	const char *szXmlns,
	BOOL bAllowPartialNs,
	const char *szTag,
	void *pUserData,
	MESSAGE_USER_DATA_FREE_FUNC pUserDataFree,
	int iPriority)
{
	CJabberMessagePermanentInfo* pInfo = new CJabberMessagePermanentInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_nMessageTypes = nMessageTypes ? nMessageTypes : JABBER_MESSAGE_TYPE_ANY;
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

bool CJabberMessageManager::DeletePermanentHandler(CJabberMessagePermanentInfo *pInfo)
{
	mir_cslock lck(m_cs);
	return m_arHandlers.remove(pInfo) == 1;
}
