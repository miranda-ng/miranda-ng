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
#include "version.h"

CJabberClientCapsManager g_clientCapsManager;

const JabberFeatCapPair g_JabberFeatCapPairs[] =
{
	{ JABBER_FEAT_DISCO_INFO,              JABBER_CAPS_DISCO_INFO,              LPGEN("Supports Service Discovery info") },
	{ JABBER_FEAT_DISCO_ITEMS,             JABBER_CAPS_DISCO_ITEMS,             LPGEN("Supports Service Discovery items list") },
	{ JABBER_FEAT_ENTITY_CAPS,             JABBER_CAPS_ENTITY_CAPS,             LPGEN("Can inform about its Jabber capabilities") },
	{ JABBER_FEAT_SI,                      JABBER_CAPS_SI,                      LPGEN("Supports stream initiation (e.g., for filetransfers)") },
	{ JABBER_FEAT_SI_FT,                   JABBER_CAPS_SI_FT,                   LPGEN("Supports stream initiation for file transfers") },
	{ JABBER_FEAT_BYTESTREAMS,             JABBER_CAPS_BYTESTREAMS,             LPGEN("Supports file transfers via SOCKS5 Bytestreams") },
	{ JABBER_FEAT_IBB,                     JABBER_CAPS_IBB,                     LPGEN("Supports file transfers via In-Band Bytestreams") },
	{ JABBER_FEAT_OOB,                     JABBER_CAPS_OOB,                     LPGEN("Supports file transfers via Out-of-Band Bytestreams") },
	{ JABBER_FEAT_OOB2,                    JABBER_CAPS_OOB,                     LPGEN("Supports file transfers via Out-of-Band Bytestreams") },
	{ JABBER_FEAT_COMMANDS,                JABBER_CAPS_COMMANDS,                LPGEN("Supports execution of Ad-Hoc commands") },
	{ JABBER_FEAT_REGISTER,                JABBER_CAPS_REGISTER,                LPGEN("Supports in-band registration") },
	{ JABBER_FEAT_MUC,                     JABBER_CAPS_MUC,                     LPGEN("Supports multi-user chat") },
	{ JABBER_FEAT_CHATSTATES,              JABBER_CAPS_CHATSTATES,              LPGEN("Can report chat state in a chat session") },
	{ JABBER_FEAT_LAST_ACTIVITY,           JABBER_CAPS_LAST_ACTIVITY,           LPGEN("Can report information about the last activity of the user") },
	{ JABBER_FEAT_VERSION,                 JABBER_CAPS_VERSION,                 LPGEN("Can report own version information") },
	{ JABBER_FEAT_ENTITY_TIME,             JABBER_CAPS_ENTITY_TIME,             LPGEN("Can report local time of the user") },
	{ JABBER_FEAT_PING,                    JABBER_CAPS_PING,                    LPGEN("Can send and receive ping requests") },
	{ JABBER_FEAT_DATA_FORMS,              JABBER_CAPS_DATA_FORMS,              LPGEN("Supports data forms") },
	{ JABBER_FEAT_VCARD_TEMP,              JABBER_CAPS_VCARD_TEMP,              LPGEN("Supports vCard") },
	{ JABBER_FEAT_XHTML,                   JABBER_CAPS_XHTML,                   LPGEN("Supports XHTML formatting of chat messages") },
	{ JABBER_FEAT_AGENTS,                  JABBER_CAPS_AGENTS,                  LPGEN("Supports Jabber Browsing") },
	{ JABBER_FEAT_BROWSE,                  JABBER_CAPS_BROWSE,                  LPGEN("Supports Jabber Browsing") },
	{ JABBER_FEAT_FEATURE_NEG,             JABBER_CAPS_FEATURE_NEG,             LPGEN("Can negotiate options for specific features") },
	{ JABBER_FEAT_AMP,                     JABBER_CAPS_AMP,                     LPGEN("Can request advanced processing of message stanzas") },
	{ JABBER_FEAT_USER_MOOD,               JABBER_CAPS_USER_MOOD,               LPGEN("Can report information about user moods") },
	{ JABBER_FEAT_USER_MOOD_NOTIFY,        JABBER_CAPS_USER_MOOD_NOTIFY,        LPGEN("Receives information about user moods") },
	{ JABBER_FEAT_PUBSUB,                  JABBER_CAPS_PUBSUB,                  LPGEN("Supports generic publish-subscribe functionality") },
	{ JABBER_FEAT_SECUREIM,                JABBER_CAPS_SECUREIM,                LPGEN("Supports SecureIM plugin for Miranda NG") },
	{ JABBER_FEAT_MIROTR,                  JABBER_CAPS_MIROTR,                  LPGEN("Supports OTR (Off-the-Record Messaging)") },
	{ JABBER_FEAT_NEWGPG,                  JABBER_CAPS_NEWGPG,                  LPGEN("Supports New_GPG plugin for Miranda NG") },
	{ JABBER_FEAT_PRIVACY_LISTS,           JABBER_CAPS_PRIVACY_LISTS,           LPGEN("Blocks packets from other users/group chats using Privacy lists") },
	{ JABBER_FEAT_MESSAGE_RECEIPTS,        JABBER_CAPS_MESSAGE_RECEIPTS,        LPGEN("Supports Message Receipts") },
	{ JABBER_FEAT_USER_TUNE,               JABBER_CAPS_USER_TUNE,               LPGEN("Can report information about the music to which a user is listening") },
	{ JABBER_FEAT_USER_TUNE_NOTIFY,        JABBER_CAPS_USER_TUNE_NOTIFY,        LPGEN("Receives information about the music to which a user is listening") },
	{ JABBER_FEAT_PRIVATE_STORAGE,         JABBER_CAPS_PRIVATE_STORAGE,         LPGEN("Supports private XML Storage (for bookmarks and other)") },
	{ JABBER_FEAT_ATTENTION,               JABBER_CAPS_ATTENTION,               LPGEN("Supports attention requests ('nudge')") },
	{ JABBER_FEAT_MAM,                     JABBER_CAPS_MAM,                     LPGEN("Support Message Archive Management (XEP-0313)") },
	{ JABBER_FEAT_ARCHIVE_AUTO,            JABBER_CAPS_ARCHIVE_AUTO,            LPGEN("Supports chat history retrieving") },
	{ JABBER_FEAT_ARCHIVE_MANAGE,          JABBER_CAPS_ARCHIVE_MANAGE,          LPGEN("Supports chat history management") },
	{ JABBER_FEAT_USER_ACTIVITY,           JABBER_CAPS_USER_ACTIVITY,           LPGEN("Can report information about user activity") },
	{ JABBER_FEAT_USER_ACTIVITY_NOTIFY,    JABBER_CAPS_USER_ACTIVITY_NOTIFY,    LPGEN("Receives information about user activity") },
	{ JABBER_FEAT_MIRANDA_NOTES,           JABBER_CAPS_MIRANDA_NOTES,           LPGEN("Supports Miranda NG notes extension") },
	{ JABBER_FEAT_JINGLE,                  JABBER_CAPS_JINGLE,                  LPGEN("Supports Jingle") },
	{ JABBER_FEAT_ROSTER_EXCHANGE,         JABBER_CAPS_ROSTER_EXCHANGE,         LPGEN("Supports Roster Exchange") },
	{ JABBER_FEAT_DIRECT_MUC_INVITE,       JABBER_CAPS_DIRECT_MUC_INVITE,       LPGEN("Supports direct chat invitations (XEP-0249)") },
	{ JABBER_FEAT_OMEMO_DEVICELIST_NOTIFY, JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY, LPGEN("Receives information about OMEMO devices") },
	{ JABBER_FEAT_CARBONS,				      JABBER_CAPS_CARBONS,                 LPGEN("Supports message carbons (XEP-0280)")},
};

const int g_cJabberFeatCapPairs = _countof(g_JabberFeatCapPairs);

const JabberFeatCapPairExt g_JabberFeatCapPairsExt[] =
{
	{ JABBER_EXT_SECUREIM,          JABBER_CAPS_SECUREIM,             "SecureIM/IsContactSecured" },
	{ JABBER_EXT_MIROTR,            JABBER_CAPS_MIROTR,               "MirOTRMenuCheckService"    },
	{ JABBER_EXT_NEWGPG,            JABBER_CAPS_NEWGPG,               "/ExportGPGKeys"            },
	{ JABBER_EXT_OMEMO,             JABBER_CAPS_OMEMO,                                            },
	{ JABBER_EXT_NUDGE,             JABBER_CAPS_ATTENTION,            "NUDGE/Send"                },
	{ JABBER_EXT_JINGLE,            JABBER_CAPS_JINGLE,               "Jingle/StartSession"       },
	{ JABBER_EXT_COMMANDS,          JABBER_CAPS_COMMANDS                                          },
	{ JABBER_EXT_USER_ACTIVITY,     JABBER_CAPS_USER_ACTIVITY_NOTIFY                              },
	{ JABBER_EXT_USER_MOOD,         JABBER_CAPS_USER_MOOD_NOTIFY                                  },
	{ JABBER_EXT_USER_TUNE,         JABBER_CAPS_USER_TUNE_NOTIFY,     "ListeningTo/Enabled"       },
	{ JABBER_EXT_USER_TUNE,         JABBER_CAPS_USER_TUNE_NOTIFY,     "WATrack/GetFileInfo"       },
	{ JABBER_EXT_MIR_NOTES,         JABBER_CAPS_MIRANDA_NOTES                                     },
	{ szCoreVersion,                JABBER_CAPS_MIRANDA_PARTIAL                                   },
	{ JABBER_EXT_PLATFORMX86,       JABBER_CAPS_PLATFORMX86                                       },
	{ JABBER_EXT_PLATFORMX64,       JABBER_CAPS_PLATFORMX64                                       },
};

const int g_cJabberFeatCapPairsExt = _countof(g_JabberFeatCapPairsExt);

void CJabberProto::OnIqResultCapsDiscoInfo(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->GetFrom()));
	if (r == nullptr)
		return;

	auto *query = pInfo->GetChildNode();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT && query) {
		JabberCapsBits jcbCaps = 0;

		for (auto *feature : TiXmlFilter(query, "feature")) {
			const char *featureName = XmlGetAttr(feature, "var");
			if (!featureName)
				continue;

			for (auto &it : g_JabberFeatCapPairs) {
				if (!mir_strcmp(it.szFeature, featureName)) {
					jcbCaps |= it.jcbCap;
					break;
				}
			}
		}

		// no XEP-0115 support? store info & exit
		CJabberClientPartialCaps *pCaps = r->m_pCaps;
		if (pCaps == nullptr) {
			r->m_jcbCachedCaps = jcbCaps;
			r->m_dwDiscoInfoRequestTime = -1;
			return;
		}

		for (auto *identity : TiXmlFilter(query, "identity")) {
			const char *identityName = XmlGetAttr(identity, "name");
			if (identityName)
				pCaps->SetVer(identityName);
		}

		for (auto *xform : TiXmlFilter(query, "x")) {
			// check that this is a form of required type
			auto *formType = XmlGetChildText(XmlGetChildByTag(xform, "field", "var", "FORM_TYPE"), "value");
			if (!formType || mir_strcmp(formType, JABBER_FEAT_SOFTWARE_INFO))
				continue;

			for (auto *field : TiXmlFilter(xform, "field")) {
				const char *fieldName = XmlGetAttr(field, "var"), *fieldValue = XmlGetChildText(field, "value");
				if (fieldValue == nullptr)
					continue;

				if (!mir_strcmp(fieldName, "os"))
					pCaps->SetOs(fieldValue);
				else if (!mir_strcmp(fieldName, "os_version"))
					pCaps->SetOsVer(fieldValue);
				else if (!mir_strcmp(fieldName, "software"))
					pCaps->SetSoft(fieldValue);
				else if (!mir_strcmp(fieldName, "software_version"))
					pCaps->SetSoftVer(fieldValue);
				else if (!mir_strcmp(fieldName, "x-miranda-core-version"))
					pCaps->SetSoftMir(fieldValue);
			}
		}

		pCaps->SetCaps(jcbCaps, pInfo->GetIqId());

		UpdateMirVer(pInfo->GetHContact(), r);

		JabberUserInfoUpdate(pInfo->GetHContact());
	}
	else {
		if (!r->m_pCaps) { // no XEP-0115 support?
			r->m_jcbCachedCaps = JABBER_RESOURCE_CAPS_NONE;
			r->m_dwDiscoInfoRequestTime = -1;
		}
		else r->m_pCaps->SetCaps(JABBER_RESOURCE_CAPS_ERROR);
	}
}

JabberCapsBits CJabberProto::GetTotalJidCapabilities(const char *jid)
{
	if (jid == nullptr)
		return JABBER_RESOURCE_CAPS_NONE;

	char szBareJid[JABBER_MAX_JID_LEN];
	JabberStripJid(jid, szBareJid, _countof(szBareJid));

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, szBareJid);
	if (item == nullptr)
		item = ListGetItemPtr(LIST_VCARD_TEMP, szBareJid);

	JabberCapsBits jcbToReturn = JABBER_RESOURCE_CAPS_NONE;

	// get bare jid info only if where is no resources
	if (!item || (item && !item->arResources.getCount())) {
		pResourceStatus r(ResourceInfoFromJID(szBareJid));
		jcbToReturn = GetResourceCapabilities(szBareJid, r);
		if (jcbToReturn & JABBER_RESOURCE_CAPS_ERROR)
			jcbToReturn = JABBER_RESOURCE_CAPS_NONE;
	}

	if (item) {
		for (auto &it : item->arResources) {
			pResourceStatus r(it);
			JabberCapsBits jcb = GetResourceCapabilities(MakeJid(szBareJid, it->m_szResourceName), r);
			if (!(jcb & JABBER_RESOURCE_CAPS_ERROR))
				jcbToReturn |= jcb;
		}
	}
	return jcbToReturn;
}

JabberCapsBits CJabberProto::GetResourceCapabilities(const char *jid)
{
	char fullJid[JABBER_MAX_JID_LEN];
	GetClientJID(jid, fullJid, _countof(fullJid));

	pResourceStatus r(ResourceInfoFromJID(fullJid));
	return GetResourceCapabilities(fullJid, r);
}

JabberCapsBits CJabberProto::GetResourceCapabilities(const char *jid, pResourceStatus &r)
{
	if (r == nullptr)
		return JABBER_RESOURCE_CAPS_ERROR;

	// XEP-0115 mode
	if (r->m_pCaps) {
		CJabberClientPartialCaps *pCaps = r->m_pCaps;
		JabberCapsBits jcbCaps = 0;
		bool bRequestSent = false;
		JabberCapsBits jcbMainCaps = pCaps->GetCaps();

		if (jcbMainCaps == JABBER_RESOURCE_CAPS_TIMEOUT && !r->m_dwDiscoInfoRequestTime)
			jcbMainCaps = JABBER_RESOURCE_CAPS_ERROR;

		if (jcbMainCaps == JABBER_RESOURCE_CAPS_UNINIT) {
			// send disco#info query
			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, jid);
			pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			pCaps->SetCaps(JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId());
			r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			char queryNode[512];
			mir_snprintf(queryNode, "%s#%s", pCaps->GetNode(), pCaps->GetHash());
			m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR("node", queryNode));

			bRequestSent = true;
		}
		else if (jcbMainCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS)
			bRequestSent = true;
		else if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT)
			jcbCaps |= jcbMainCaps;

		if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT && r->m_tszCapsExt) {
			char *caps = mir_strdup(r->m_tszCapsExt);

			char *token = strtok(caps, " ");
			while (token) {
				for (auto &it : g_JabberFeatCapPairsExt) {
					if (!mir_strcmp(it.szFeature, token)) {
						jcbCaps |= it.jcbCap;
						break;
					}
				}

				token = strtok(nullptr, " ");
			}

			mir_free(caps);
		}

		if (bRequestSent)
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;

		return jcbCaps | r->m_jcbManualDiscoveredCaps;
	}

	// no XEP-0115: send query each time it's needed
	switch (r->m_dwDiscoInfoRequestTime) {
	case -1:
		return r->m_jcbCachedCaps;

	case 0:
		RequestOldCapsInfo(r, jid);
		break;
	}
	return JABBER_RESOURCE_CAPS_IN_PROGRESS;
}

bool CJabberProto::HandleCapsInfoRequest(const TiXmlElement *, CJabberIqInfo *pInfo, const char *szNode)
{
	XmlNodeIq iq("result", pInfo);

	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
	if (szNode){
		CMStringA szExtCap(FORMAT, "%s#%s", JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc.c_str());
		query << XATTR("node", szExtCap);
	}

	CMStringA szName(getMStringA("Identity", "Miranda")); // hidden setting to be entered from dbeditor++
	if (m_bAllowVersionRequests)
		szName.AppendFormat(" %s", __VERSION_STRING_DOTS);
	query << XCHILD("identity") << XATTR("category", "client") << XATTR("type", "pc") << XATTR("name", szName);

	for (auto &it : GetSortedFeatStrings(GetOwnCaps())) {
		query << XCHILD("feature") << XATTR("var", it);
	}

	if (m_bAllowVersionRequests && szNode) {
		TiXmlElement *form = query << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "result");
		form << XCHILD("field") << XATTR("var", "FORM_TYPE") << XATTR("type", "hidden") << XCHILD("value", JABBER_FEAT_SOFTWARE_INFO);

		if (m_bShowOSVersion) {
			char os[256];
			if (OS_GetDisplayString(os, _countof(os))) {
				form << XCHILD("field") << XATTR("var", "os") << XCHILD("value", "Microsoft Windows");
				form << XCHILD("field") << XATTR("var", "os_version") << XCHILD("value", os);
			}
		}
		form << XCHILD("field") << XATTR("var", "software") << XCHILD("value", "Miranda NG Jabber Protocol");
		form << XCHILD("field") << XATTR("var", "software_version") << XCHILD("value", __VERSION_STRING_DOTS);
		form << XCHILD("field") << XATTR("var", "x-miranda-core-version") << XCHILD("value", szCoreVersion);
	}

	m_ThreadInfo->send(iq);
	return true;
}

JabberCapsBits CJabberProto::GetOwnCaps(bool IncludeDynamic)
{
	JabberCapsBits jcb = JABBER_CAPS_MIRANDA_ALL;

	if (IncludeDynamic)
		for (auto &it : m_lstJabberFeatCapPairsDynamic)
			jcb |= it->jcbCap;

	if (!m_bAllowTimeReplies)
		jcb &= ~JABBER_CAPS_ENTITY_TIME;
	if (!m_bAllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;
	if (m_bUseOMEMO)
		jcb |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;
	if (!m_bMsgAck)
		jcb &= ~(JABBER_CAPS_CHAT_MARKERS | JABBER_CAPS_MESSAGE_RECEIPTS);

	return jcb;
}

LIST<char> CJabberProto::GetSortedFeatStrings(JabberCapsBits jcb)
{
	LIST<char> feats(10, strcmp);
	for (auto &it : g_JabberFeatCapPairs)
		if (jcb & it.jcbCap)
			feats.insert((char*)it.szFeature);

	for (auto &it : m_lstJabberFeatCapPairsDynamic)
		if (jcb & it->jcbCap)
			feats.insert(it->szFeature);

	return feats;
}

void CJabberProto::RequestOldCapsInfo(pResourceStatus &r, const char *fullJid)
{
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid);
	pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
	pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
	r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO));
}

void CJabberProto::UpdateFeatHash()
{
	CMStringA szName(getMStringA("Identity", "Miranda")); // hidden setting to be entered from dbeditor++
	if (m_bAllowVersionRequests)
		szName.AppendFormat(" %s", __VERSION_STRING_DOTS);
	CMStringA feat_buf(FORMAT, "client/pc//%s<", szName.c_str());

	for (auto &it : GetSortedFeatStrings(GetOwnCaps())) {
		feat_buf.Append(it);
		feat_buf.AppendChar('<');
	}

	if (m_bAllowVersionRequests) {
		feat_buf.AppendFormat("%s<", JABBER_FEAT_SOFTWARE_INFO);

		if (m_bShowOSVersion) {
			char os[256];
			if (OS_GetDisplayString(os, _countof(os))) {
				feat_buf.Append("os<Microsoft Windows<");
				feat_buf.AppendFormat("os_version<%s<", os);
			}
		}
		feat_buf.Append("software<Miranda NG Jabber Protocol<");
		feat_buf.AppendFormat("software_version<%s<", __VERSION_STRING_DOTS);
		feat_buf.AppendFormat("x-miranda-core-version<%s<", szCoreVersion);
	}

	uint8_t hash[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((uint8_t *)feat_buf.c_str(), feat_buf.GetLength(), hash);
	ptrA szHash(mir_base64_encode(&hash, sizeof(hash)));
	m_szFeaturesCrc = szHash;
}

/////////////////////////////////////////////////////////////////////////////////////////
//  CJabberClientPartialCaps class

CJabberClientPartialCaps::CJabberClientPartialCaps(CJabberClientCaps *pParent, const char *szHash, const char *szVer)
	: m_parent(pParent),
	m_szHash(mir_strdup(szHash)),
	m_szSoftVer(mir_strdup(szVer))
{
	m_iTime = time(0);
}

CJabberClientPartialCaps::~CJabberClientPartialCaps()
{
	delete m_pNext;
}

CJabberClientPartialCaps* CJabberClientPartialCaps::SetNext(CJabberClientPartialCaps *pCaps)
{
	CJabberClientPartialCaps *pRetVal = m_pNext;
	m_pNext = pCaps;
	return pRetVal;
}

void CJabberClientPartialCaps::SetCaps(JabberCapsBits jcbCaps, int nIqId)
{
	if (jcbCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS)
		m_dwRequestTime = GetTickCount();
	else
		m_dwRequestTime = 0;
	m_jcbCaps = jcbCaps;
	m_nIqId = nIqId;
}

JabberCapsBits CJabberClientPartialCaps::GetCaps()
{
	if (m_jcbCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS && GetTickCount() - m_dwRequestTime > JABBER_RESOURCE_CAPS_QUERY_TIMEOUT) {
		m_jcbCaps = JABBER_RESOURCE_CAPS_TIMEOUT;
		m_dwRequestTime = 0;
	}
	return m_jcbCaps;
}

CJabberClientPartialCaps* CJabberClientCaps::FindByVersion(const char *szHash)
{
	if (m_pCaps == nullptr)
		return nullptr;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while (pCaps) {
		if (!mir_strcmp(szHash, pCaps->GetHash()))
			return pCaps;
		pCaps = pCaps->GetNext();
	}
	return nullptr;
}

CJabberClientPartialCaps* CJabberClientCaps::FindById(int nIqId)
{
	if (!m_pCaps || nIqId == -1)
		return nullptr;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while (pCaps) {
		if (pCaps->GetIqId() == nIqId)
			break;
		pCaps = pCaps->GetNext();
	}
	return pCaps;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberClientCaps class

CJabberClientCaps::CJabberClientCaps(const char *szNode) :
	m_szNode(mir_strdup(szNode))
{
	m_pCaps = nullptr;
}

CJabberClientCaps::~CJabberClientCaps()
{
	mir_free(m_szNode);
	delete m_pCaps;
}

JabberCapsBits CJabberClientCaps::GetPartialCaps(const char *szVer)
{
	CJabberClientPartialCaps *pCaps = FindByVersion(szVer);
	return (pCaps) ? pCaps->GetCaps() : JABBER_RESOURCE_CAPS_UNINIT;
}

CJabberClientPartialCaps* CJabberClientCaps::SetPartialCaps(const char *szHash, const char *szVer, JabberCapsBits jcbCaps, int nIqId)
{
	CJabberClientPartialCaps *pCaps = FindByVersion(szHash);
	if (!pCaps) {
		pCaps = new CJabberClientPartialCaps(this, szHash, szVer);
		pCaps->SetNext(m_pCaps);
		m_pCaps = pCaps;
	}

	pCaps->SetCaps(jcbCaps, nIqId);
	return pCaps;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberClientCapsManager class

static int sttCompareNodes(const CJabberClientCaps *p1, const CJabberClientCaps *p2)
{
	return mir_strcmp(p1->GetNode(), p2->GetNode());
}

CJabberClientCapsManager::CJabberClientCapsManager() :
	m_arCaps(10, sttCompareNodes)
{
}

CJabberClientCapsManager::~CJabberClientCapsManager()
{
}

CJabberClientCaps* CJabberClientCapsManager::FindClient(const char *szNode)
{
	if (szNode == nullptr)
		return nullptr;

	return m_arCaps.find((CJabberClientCaps*)&szNode);
}

CJabberClientPartialCaps* CJabberClientCapsManager::GetPartialCaps(const char *szNode, const char *szHash)
{
	mir_cslock lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	return (pClient == nullptr) ? nullptr : pClient->FindByVersion(szHash);
}

CJabberClientPartialCaps* CJabberClientCapsManager::SetClientCaps(const char *szNode, const char *szHash, const char *szVer, JabberCapsBits jcbCaps, int nIqId)
{
	mir_cslockfull lck(m_cs);
	auto *pClient = FindClient(szNode);
	if (!pClient) {
		pClient = new CJabberClientCaps(szNode);
		m_arCaps.insert(pClient);
	}

	CJabberClientPartialCaps *res = pClient->SetPartialCaps(szHash, szVer, jcbCaps, nIqId);
	lck.unlock();
	Netlib_Logf(0, "CAPS: set caps %I64x for: %s#%s => [%s]", jcbCaps, szHash, szNode, szVer);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const char *str2buf(const std::string &str)
{
	return (str.empty()) ? nullptr : str.c_str();
}

void CJabberClientCapsManager::Load()
{
	int fileId = _wopen(VARSW(L"%miranda_userdata%\\jabberCaps.json"), _O_BINARY | _O_RDONLY);
	if (fileId == -1)
		return;

	size_t dwFileLength = _filelength(fileId), dwReadLen;
	ptrA szBuf((char *)mir_alloc(dwFileLength + 1));
	dwReadLen = _read(fileId, szBuf, (unsigned)dwFileLength);
	_close(fileId);
	if (dwFileLength != dwReadLen)
		return;

	szBuf[dwFileLength] = 0;

	JSONNode root = JSONNode::parse(szBuf);
	if (!root)
		return;

	for (auto &node : root) {
		std::string szNode = node["node"].as_string();
		auto *pClient = FindClient(szNode.c_str());
		if (!pClient) {
			pClient = new CJabberClientCaps(szNode.c_str());
			m_arCaps.insert(pClient);
		}

		for (auto &ver : node["versions"]) {
			std::string szVer = ver["softver"].as_string();
			std::string szHash = ver["hash"].as_string();
			JabberCapsBits jcbCaps = _atoi64(ver["caps"].as_string().c_str());

			auto *res = pClient->SetPartialCaps(szHash.c_str(), szVer.c_str(), jcbCaps);
			res->SetTime(ver["time"].as_int());
			res->SetOs(str2buf(ver["os"].as_string()));
			res->SetOsVer(str2buf(ver["osver"].as_string()));
			res->SetSoft(str2buf(ver["soft"].as_string()));
			res->SetSoftMir(str2buf(ver["softmir"].as_string()));
		}
	}
}

void CJabberClientCapsManager::Save()
{
	JSONNode root(JSON_ARRAY);
	int iFilterTime = time(0) - 30 * 86400; // month ago

	for (auto &it : m_arCaps) {
		JSONNode versions(JSON_ARRAY); versions.set_name("versions");
		for (auto *p = it->GetFirst(); p != nullptr; p = p->GetNext()) {
			if (p->GetTime() < iFilterTime)
				continue;

			JSONNode ver;
			ver << CHAR_PARAM("hash", p->GetHash()) << INT64_PARAM("caps", p->GetCaps()) << INT_PARAM("time", p->GetTime())
				<< CHAR_PARAM("os", p->GetOs()) << CHAR_PARAM("osver", p->GetOsVer())
				<< CHAR_PARAM("soft", p->GetSoft()) << CHAR_PARAM("softver", p->GetSoftVer()) << CHAR_PARAM("softmir", p->GetSoftMir());
			versions.push_back(ver);
		}

		JSONNode node; node << CHAR_PARAM("node", it->GetNode()) << versions;
		root << node;
	}

	std::string szBody = root.write_formatted();

	int fileId = _wopen(VARSW(L"%miranda_userdata%\\jabberCaps.json"), _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE);
	if (fileId != -1) {
		_write(fileId, szBody.c_str(), (unsigned)szBody.length());
		_close(fileId);
	}
}
