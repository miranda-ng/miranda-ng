/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-19 Miranda NG team

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
	{ JABBER_FEAT_MESSAGE_EVENTS,          JABBER_CAPS_MESSAGE_EVENTS,          LPGEN("Can request and respond to events relating to the delivery, display, and composition of messages") },
	{ JABBER_FEAT_VCARD_TEMP,              JABBER_CAPS_VCARD_TEMP,              LPGEN("Supports vCard") },
	{ JABBER_FEAT_AVATAR,                  JABBER_CAPS_AVATAR,                  LPGEN("Supports iq-based avatars") },
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

void CJabberProto::AddDefaultCaps()
{
	JabberCapsBits myCaps = JABBER_CAPS_MIRANDA_ALL;
	if (m_bUseOMEMO)
		myCaps |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;
	
	for (auto &it : g_JabberFeatCapPairsExt)
		if (it.Valid())
			myCaps |= it.jcbCap;

	wchar_t szOsBuffer[256]; szOsBuffer[0] = 0;
	GetOSDisplayString(szOsBuffer, _countof(szOsBuffer));

	CJabberClientPartialCaps *pCaps = m_clientCapsManager.SetOwnCaps(JABBER_CAPS_MIRANDA_NODE, __VERSION_STRING_DOTS, myCaps);
	pCaps->m_szOs = mir_strdup("Microsoft Windows");
	pCaps->m_szOsVer = mir_utf8encodeW(szOsBuffer);
	pCaps->m_szSoft = mir_strdup("Miranda NG Jabber Protocol");
	pCaps->m_szSoftMir = mir_strdup(szCoreVersion);
}

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
			if (!formType || mir_strcmp(formType, "urn:xmpp:dataforms:softwareinfo"))
				continue;

			JSONNode root;
			for (auto *field : TiXmlFilter(xform, "field")) {
				const char *fieldName = XmlGetAttr(field, "var"), *fieldValue = XmlGetChildText(field, "value");
				if (fieldValue == nullptr)
					continue;

				if (!mir_strcmp(fieldName, "os"))
					root.push_back(JSONNode("o", pCaps->m_szOs = mir_strdup(fieldValue)));
				else if (!mir_strcmp(fieldName, "os_version"))
					root.push_back(JSONNode("ov", pCaps->m_szOsVer = mir_strdup(fieldValue)));
				else if (!mir_strcmp(fieldName, "software"))
					root.push_back(JSONNode("s", pCaps->m_szSoft = mir_strdup(fieldValue)));
				else if (!mir_strcmp(fieldName, "software_version"))
					root.push_back(JSONNode("sv", pCaps->m_szSoftVer = mir_strdup(fieldValue)));
				else if (!mir_strcmp(fieldName, "x-miranda-core-version"))
					root.push_back(JSONNode("sm", pCaps->m_szSoftMir = mir_strdup(fieldValue)));
			}
			root.push_back(JSONNode("c", CMStringA(FORMAT, "%lld", jcbCaps)));

			CMStringA szName(FORMAT, "%s#%s", pCaps->GetNode(), pCaps->GetHash());
			json_string szValue = root.write();
			db_set_s(0, "JabberCaps", szName, szValue.c_str());
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

void CJabberProto::RequestOldCapsInfo(pResourceStatus &r, const char *fullJid)
{
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid);
	pInfo->SetParamsToParse(JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
	pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
	r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO));
}

void CJabberProto::GetCachedCaps(const char *szNode, const char *szVer, pResourceStatus &r)
{
	CMStringA szName(FORMAT, "%s#%s", szNode, szVer);
	ptrA szValue(db_get_sa(0, "JabberCaps", szName));
	if (szValue != 0) {
		JSONNode root = JSONNode::parse(szValue);
		if (root) {
			CMStringW wszCaps = root["c"].as_mstring();
			r->m_pCaps = m_clientCapsManager.SetClientCaps(szNode, szVer, nullptr, _wtoi64(wszCaps));
			r->m_pCaps->m_szOs = mir_utf8encodeW(root["o"].as_mstring());
			r->m_pCaps->m_szOsVer = mir_utf8encodeW(root["ov"].as_mstring());
			r->m_pCaps->m_szSoft = mir_utf8encodeW(root["s"].as_mstring());
			r->m_pCaps->m_szSoftVer = mir_utf8encodeW(root["sv"].as_mstring());
			r->m_pCaps->m_szSoftMir = mir_utf8encodeW(root["sm"].as_mstring());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//  CJabberClientPartialCaps class

CJabberClientPartialCaps::CJabberClientPartialCaps(CJabberClientCaps *pParent, const char *szHash, const char *szVer)
	: m_parent(pParent),
	m_szHash(mir_strdup(szHash)),
	m_szSoftVer(mir_strdup(szVer)),
	m_jcbCaps(JABBER_RESOURCE_CAPS_UNINIT),
	m_pNext(nullptr),
	m_nIqId(-1),
	m_dwRequestTime(0)
{
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

CJabberClientCapsManager::CJabberClientCapsManager(CJabberProto *proto) :
	m_arCaps(10, sttCompareNodes)
{
	ppro = proto;

	UpdateFeatHash();
}

CJabberClientCapsManager::~CJabberClientCapsManager()
{
}

void CJabberClientCapsManager::UpdateFeatHash()
{
	m_szFeaturesCrc.Empty();

	JabberCapsBits jcb = JABBER_CAPS_MIRANDA_ALL;
	for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic)
		jcb |= it->jcbCap;
	if (!ppro->m_bAllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;

	if (ppro->m_bUseOMEMO)
		jcb |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;

	CMStringA feat_buf(FORMAT, "client/pc//Miranda %d.%d.%d.%d<", __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
	for (auto &it : g_JabberFeatCapPairs)
		if (jcb & it.jcbCap) {
			feat_buf.Append(it.szFeature);
			feat_buf.AppendChar('<');
		}

	for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic)
		if (jcb & it->jcbCap) {
			feat_buf.Append(it->szFeature);
			feat_buf.AppendChar('<');
		}

	wchar_t szOsBuffer[256]; szOsBuffer[0] = 0;
	GetOSDisplayString(szOsBuffer, _countof(szOsBuffer));

	feat_buf.Append("software_version"); feat_buf.AppendChar('<');
	feat_buf.Append(__VERSION_STRING_DOTS); feat_buf.AppendChar('<');

	feat_buf.Append("x-miranda-core-version"); feat_buf.AppendChar('<');
	feat_buf.Append(szCoreVersion); feat_buf.AppendChar('<');

	BYTE hash[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((BYTE*)feat_buf.c_str(), feat_buf.GetLength(), hash);
	ptrA szHash(mir_base64_encode(&hash, sizeof(hash)));
	m_szFeaturesCrc = szHash;
}

const char* CJabberClientCapsManager::GetFeaturesCrc()
{
	return m_szFeaturesCrc.c_str();
}

CJabberClientCaps* CJabberClientCapsManager::FindClient(const char *szNode)
{
	if (szNode == nullptr)
		return nullptr;

	return m_arCaps.find((CJabberClientCaps*)&szNode);
}

JabberCapsBits CJabberClientCapsManager::GetClientCaps(const char *szNode, const char *szVer)
{
	mir_cslockfull lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		lck.unlock();
		ppro->debugLogA("CAPS: get no caps for: %s, %s", szNode, szVer);
		return JABBER_RESOURCE_CAPS_UNINIT;
	}
	JabberCapsBits jcbCaps = pClient->GetPartialCaps(szVer);
	lck.unlock();
	ppro->debugLogA("CAPS: get caps %I64x for: %s, %s", jcbCaps, szNode, szVer);
	return jcbCaps;
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
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		pClient = new CJabberClientCaps(szNode);
		m_arCaps.insert(pClient);
	}

	CJabberClientPartialCaps *res = pClient->SetPartialCaps(szHash, szVer, jcbCaps, nIqId);
	lck.unlock();
	ppro->debugLogA("CAPS: set caps %I64x for: %s#%s => [%s]", jcbCaps, szHash, szNode, szVer);
	return res;
}

bool CJabberClientCapsManager::HandleInfoRequest(const TiXmlElement*, CJabberIqInfo *pInfo, const char *szNode)
{
	JabberCapsBits jcb = 0;

	if (szNode) {
		char szExtCap[512], szExtCapWHash[560];
		mir_snprintf(szExtCap, "%s#%s", JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc.c_str());
		if (!mir_strcmp(szExtCap, szNode)) {
			szNode = nullptr;
			goto LBL_All;
		}

		for (auto &it : g_JabberFeatCapPairsExt) {
			if (!it.Valid())
				continue;

			// TODO: something better here
			mir_snprintf(szExtCap, "%s#%s", JABBER_CAPS_MIRANDA_NODE, it.szFeature);
			mir_snprintf(szExtCapWHash, "%s %s", szExtCap, m_szFeaturesCrc.c_str());
			if (!mir_strcmp(szNode, szExtCap) || !mir_strcmp(szNode, szExtCapWHash)) {
				jcb = it.jcbCap;
				break;
			}
		}

		// check features registered through IJabberNetInterface::RegisterFeature() and IJabberNetInterface::AddFeatures()
		for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic) {
			// TODO: something better here
			mir_snprintf(szExtCap, "%s#%s", JABBER_CAPS_MIRANDA_NODE, it->szExt);
			mir_snprintf(szExtCapWHash, "%s %s", szExtCap, m_szFeaturesCrc.c_str());
			if (!mir_strcmp(szNode, szExtCap) || !mir_strcmp(szNode, szExtCapWHash)) {
				jcb = it->jcbCap;
				break;
			}
		}

		// unknown node, not XEP-0115 request
		if (!jcb)
			return false;
	}
	else {
LBL_All:
		jcb = JABBER_CAPS_MIRANDA_ALL;
		for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic)
			jcb |= it->jcbCap;
	}

	if (ppro->m_bUseOMEMO)
		jcb |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;

	if (!ppro->m_bAllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;

	XmlNodeIq iq("result", pInfo);

	TiXmlElement *query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
	if (szNode)
		query << XATTR("node", szNode);

	CMStringA szName(FORMAT, "Miranda %d.%d.%d.%d", __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
	query << XCHILD("identity") << XATTR("category", "client") << XATTR("type", "pc") << XATTR("name", szName);

	for (auto &it : g_JabberFeatCapPairs)
		if (jcb & it.jcbCap)
			query << XCHILD("feature") << XATTR("var", it.szFeature);

	for (auto &it : ppro->m_lstJabberFeatCapPairsDynamic)
		if (jcb & it->jcbCap)
			query << XCHILD("feature") << XATTR("var", it->szFeature);

	if (ppro->m_bAllowVersionRequests && !szNode) {
		TiXmlElement *form = query << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "result");
		form << XCHILD("field") << XATTR("var", "FORM_TYPE") << XATTR("type", "hidden")
			<< XCHILD("value", "urn:xmpp:dataforms:softwareinfo");

		CJabberClientPartialCaps *pCaps = GetPartialCaps(JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc);
		if (pCaps) {
			if (ppro->m_bShowOSVersion) {
				form << XCHILD("field") << XATTR("var", "os") << XCHILD("value", pCaps->GetOs());
				form << XCHILD("field") << XATTR("var", "os_version") << XCHILD("value", pCaps->GetOsVer());
			}
			form << XCHILD("field") << XATTR("var", "software") << XCHILD("value", pCaps->GetSoft());
			form << XCHILD("field") << XATTR("var", "software_version") << XCHILD("value", pCaps->GetSoftVer());
			form << XCHILD("field") << XATTR("var", "x-miranda-core-version") << XCHILD("value", pCaps->GetSoftMir());
		}
	}

	ppro->m_ThreadInfo->send(iq);
	return true;
}
