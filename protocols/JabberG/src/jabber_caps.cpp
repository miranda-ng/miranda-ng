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
#include "version.h"

const JabberFeatCapPair g_JabberFeatCapPairs[] = {
	{ JABBER_FEAT_DISCO_INFO,           JABBER_CAPS_DISCO_INFO,           LPGENW("Supports Service Discovery info") },
	{ JABBER_FEAT_DISCO_ITEMS,          JABBER_CAPS_DISCO_ITEMS,          LPGENW("Supports Service Discovery items list") },
	{ JABBER_FEAT_ENTITY_CAPS,          JABBER_CAPS_ENTITY_CAPS,          LPGENW("Can inform about its Jabber capabilities") },
	{ JABBER_FEAT_SI,                   JABBER_CAPS_SI,                   LPGENW("Supports stream initiation (e.g., for filetransfers)") },
	{ JABBER_FEAT_SI_FT,                JABBER_CAPS_SI_FT,                LPGENW("Supports stream initiation for file transfers") },
	{ JABBER_FEAT_BYTESTREAMS,          JABBER_CAPS_BYTESTREAMS,          LPGENW("Supports file transfers via SOCKS5 Bytestreams") },
	{ JABBER_FEAT_IBB,                  JABBER_CAPS_IBB,                  LPGENW("Supports file transfers via In-Band Bytestreams") },
	{ JABBER_FEAT_OOB,                  JABBER_CAPS_OOB,                  LPGENW("Supports file transfers via Out-of-Band Bytestreams") },
	{ JABBER_FEAT_OOB2,                 JABBER_CAPS_OOB,                  LPGENW("Supports file transfers via Out-of-Band Bytestreams") },
	{ JABBER_FEAT_COMMANDS,             JABBER_CAPS_COMMANDS,             LPGENW("Supports execution of Ad-Hoc commands") },
	{ JABBER_FEAT_REGISTER,             JABBER_CAPS_REGISTER,             LPGENW("Supports in-band registration") },
	{ JABBER_FEAT_MUC,                  JABBER_CAPS_MUC,                  LPGENW("Supports multi-user chat") },
	{ JABBER_FEAT_CHATSTATES,           JABBER_CAPS_CHATSTATES,           LPGENW("Can report chat state in a chat session") },
	{ JABBER_FEAT_LAST_ACTIVITY,        JABBER_CAPS_LAST_ACTIVITY,        LPGENW("Can report information about the last activity of the user") },
	{ JABBER_FEAT_VERSION,              JABBER_CAPS_VERSION,              LPGENW("Can report own version information") },
	{ JABBER_FEAT_ENTITY_TIME,          JABBER_CAPS_ENTITY_TIME,          LPGENW("Can report local time of the user") },
	{ JABBER_FEAT_PING,                 JABBER_CAPS_PING,                 LPGENW("Can send and receive ping requests") },
	{ JABBER_FEAT_DATA_FORMS,           JABBER_CAPS_DATA_FORMS,           LPGENW("Supports data forms") },
	{ JABBER_FEAT_MESSAGE_EVENTS,       JABBER_CAPS_MESSAGE_EVENTS,       LPGENW("Can request and respond to events relating to the delivery, display, and composition of messages") },
	{ JABBER_FEAT_VCARD_TEMP,           JABBER_CAPS_VCARD_TEMP,           LPGENW("Supports vCard") },
	{ JABBER_FEAT_AVATAR,               JABBER_CAPS_AVATAR,               LPGENW("Supports iq-based avatars") },
	{ JABBER_FEAT_XHTML,                JABBER_CAPS_XHTML,                LPGENW("Supports XHTML formatting of chat messages") },
	{ JABBER_FEAT_AGENTS,               JABBER_CAPS_AGENTS,               LPGENW("Supports Jabber Browsing") },
	{ JABBER_FEAT_BROWSE,               JABBER_CAPS_BROWSE,               LPGENW("Supports Jabber Browsing") },
	{ JABBER_FEAT_FEATURE_NEG,          JABBER_CAPS_FEATURE_NEG,          LPGENW("Can negotiate options for specific features") },
	{ JABBER_FEAT_AMP,                  JABBER_CAPS_AMP,                  LPGENW("Can request advanced processing of message stanzas") },
	{ JABBER_FEAT_USER_MOOD,            JABBER_CAPS_USER_MOOD,            LPGENW("Can report information about user moods") },
	{ JABBER_FEAT_USER_MOOD_NOTIFY,     JABBER_CAPS_USER_MOOD_NOTIFY,     LPGENW("Receives information about user moods") },
	{ JABBER_FEAT_PUBSUB,               JABBER_CAPS_PUBSUB,               LPGENW("Supports generic publish-subscribe functionality") },
	{ JABBER_FEAT_SECUREIM,             JABBER_CAPS_SECUREIM,             LPGENW("Supports SecureIM plugin for Miranda NG") },
	{ JABBER_FEAT_MIROTR,               JABBER_CAPS_MIROTR,               LPGENW("Supports OTR (Off-the-Record Messaging)") },
	{ JABBER_FEAT_NEWGPG,               JABBER_CAPS_NEWGPG,               LPGENW("Supports New_GPG plugin for Miranda NG") },
	{ JABBER_FEAT_PRIVACY_LISTS,        JABBER_CAPS_PRIVACY_LISTS,        LPGENW("Blocks packets from other users/group chats using Privacy lists") },
	{ JABBER_FEAT_MESSAGE_RECEIPTS,     JABBER_CAPS_MESSAGE_RECEIPTS,     LPGENW("Supports Message Receipts") },
	{ JABBER_FEAT_USER_TUNE,            JABBER_CAPS_USER_TUNE,            LPGENW("Can report information about the music to which a user is listening") },
	{ JABBER_FEAT_USER_TUNE_NOTIFY,     JABBER_CAPS_USER_TUNE_NOTIFY,     LPGENW("Receives information about the music to which a user is listening") },
	{ JABBER_FEAT_PRIVATE_STORAGE,      JABBER_CAPS_PRIVATE_STORAGE,      LPGENW("Supports private XML Storage (for bookmarks and other)") },
	{ JABBER_FEAT_ATTENTION,            JABBER_CAPS_ATTENTION,            LPGENW("Supports attention requests ('nudge')") },
	{ JABBER_FEAT_ARCHIVE_AUTO,         JABBER_CAPS_ARCHIVE_AUTO,         LPGENW("Supports chat history retrieving") },
	{ JABBER_FEAT_ARCHIVE_MANAGE,       JABBER_CAPS_ARCHIVE_MANAGE,       LPGENW("Supports chat history management") },
	{ JABBER_FEAT_USER_ACTIVITY,        JABBER_CAPS_USER_ACTIVITY,        LPGENW("Can report information about user activity") },
	{ JABBER_FEAT_USER_ACTIVITY_NOTIFY, JABBER_CAPS_USER_ACTIVITY_NOTIFY, LPGENW("Receives information about user activity") },
	{ JABBER_FEAT_MIRANDA_NOTES,        JABBER_CAPS_MIRANDA_NOTES,        LPGENW("Supports Miranda NG notes extension") },
	{ JABBER_FEAT_JINGLE,               JABBER_CAPS_JINGLE,               LPGENW("Supports Jingle") },
	{ JABBER_FEAT_ROSTER_EXCHANGE,      JABBER_CAPS_ROSTER_EXCHANGE,      LPGENW("Supports Roster Exchange") },
	{ JABBER_FEAT_DIRECT_MUC_INVITE,    JABBER_CAPS_DIRECT_MUC_INVITE,    LPGENW("Supports direct chat invitations (XEP-0249)") },
	{ JABBER_FEAT_OMEMO_DEVICELIST_NOTIFY,			JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY,		  LPGENW("Receives information about OMEMO devices") },
	{ nullptr }
};

const JabberFeatCapPairExt g_JabberFeatCapPairsExt[] = {
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
	{ nullptr }
};

void CJabberProto::AddDefaultCaps()
{
	JabberCapsBits myCaps = JABBER_CAPS_MIRANDA_ALL;
	if (m_options.UseOMEMO)
		myCaps |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;
	for (int i = 0; g_JabberFeatCapPairsExt[i].szFeature; i++)
		if (g_JabberFeatCapPairsExt[i].Valid())
			myCaps |= g_JabberFeatCapPairsExt[i].jcbCap;

	wchar_t szOsBuffer[256]; szOsBuffer[0] = 0;
	GetOSDisplayString(szOsBuffer, _countof(szOsBuffer));

	CJabberClientPartialCaps *pCaps = m_clientCapsManager.SetOwnCaps(JABBER_CAPS_MIRANDA_NODE, _T(__VERSION_STRING_DOTS), myCaps);
	pCaps->m_szOs = mir_wstrdup(L"Microsoft Windows");
	pCaps->m_szOsVer = mir_wstrdup(szOsBuffer);
	pCaps->m_szSoft = mir_wstrdup(L"Miranda NG Jabber Protocol");
	pCaps->m_szSoftMir = mir_wstrdup(szCoreVersion);  
}

void CJabberProto::OnIqResultCapsDiscoInfo(HXML, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->GetFrom()));
	if (r == nullptr)
		return;

	HXML query = pInfo->GetChildNode();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT && query) {
		JabberCapsBits jcbCaps = 0;

		HXML feature;
		for (int i = 1; (feature = XmlGetNthChild(query, L"feature", i)) != nullptr; i++) {
			const wchar_t *featureName = XmlGetAttrValue(feature, L"var");
			if (!featureName)
				continue;

			for (int j = 0; g_JabberFeatCapPairs[j].szFeature; j++) {
				if (!mir_wstrcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
					jcbCaps |= g_JabberFeatCapPairs[j].jcbCap;
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

		HXML identity;
		for (int i = 1; (identity = XmlGetNthChild(query, L"identity", i)) != nullptr; i++) {
			const wchar_t *identityName = XmlGetAttrValue(identity, L"name");
			if (identityName)
				pCaps->SetVer(identityName);
		}

		HXML xform;
		for (int i = 1; (xform = XmlGetNthChild(query, L"x", i)) != nullptr; i++) {
			wchar_t *szFormTypeValue = XPath(xform, L"field[@var='FORM_TYPE']/value");
			if (!mir_wstrcmp(szFormTypeValue, L"urn:xmpp:dataforms:softwareinfo")) {
				JSONNode root;
				if (pCaps->m_szOs = mir_wstrdup(XPath(xform, L"field[@var='os']/value")))
					root.push_back(JSONNode("o", _T2A(pCaps->m_szOs).get()));
				if (pCaps->m_szOsVer = mir_wstrdup(XPath(xform, L"field[@var='os_version']/value")))
					root.push_back(JSONNode("ov", _T2A(pCaps->m_szOsVer).get()));
				if (pCaps->m_szSoft = mir_wstrdup(XPath(xform, L"field[@var='software']/value")))
					root.push_back(JSONNode("s", _T2A(pCaps->m_szSoft).get()));
				if (pCaps->m_szSoftVer = mir_wstrdup(XPath(xform, L"field[@var='software_version']/value")))
					root.push_back(JSONNode("sv", _T2A(pCaps->m_szSoftVer).get()));
				if (pCaps->m_szSoftMir = mir_wstrdup(XPath(xform, L"field[@var='x-miranda-core-version']/value")))
					root.push_back(JSONNode("sm", _T2A(pCaps->m_szSoftMir).get()));
				root.push_back(JSONNode("c",  CMStringA(FORMAT, "%lld", jcbCaps)));

				CMStringA szName(FORMAT, "%S#%S", pCaps->GetNode(), pCaps->GetHash());
				json_string szValue = root.write();
				db_set_s(0, "JabberCaps", szName, szValue.c_str());
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

JabberCapsBits CJabberProto::GetTotalJidCapabilities(const wchar_t *jid)
{
	if (jid == nullptr)
		return JABBER_RESOURCE_CAPS_NONE;

	wchar_t szBareJid[JABBER_MAX_JID_LEN];
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
		for (int i = 0; i < item->arResources.getCount(); i++) {
			wchar_t szFullJid[JABBER_MAX_JID_LEN];
			mir_snwprintf(szFullJid, L"%s/%s", szBareJid, item->arResources[i]->m_tszResourceName);
			pResourceStatus r(item->arResources[i]);
			JabberCapsBits jcb = GetResourceCapabilities(szFullJid, r);
			if (!(jcb & JABBER_RESOURCE_CAPS_ERROR))
				jcbToReturn |= jcb;
		}
	}
	return jcbToReturn;
}

JabberCapsBits CJabberProto::GetResourceCapabilities(const wchar_t *jid)
{
	wchar_t fullJid[JABBER_MAX_JID_LEN];
	GetClientJID(jid, fullJid, _countof(fullJid));

	pResourceStatus r(ResourceInfoFromJID(fullJid));
	return GetResourceCapabilities(fullJid, r);
}

JabberCapsBits CJabberProto::GetResourceCapabilities(const wchar_t *jid, pResourceStatus &r)
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
			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, jid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			pCaps->SetCaps(JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId());
			r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			wchar_t queryNode[512];
			mir_snwprintf(queryNode, L"%s#%s", pCaps->GetNode(), pCaps->GetHash());
			m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(L"node", queryNode));

			bRequestSent = true;
		}
		else if (jcbMainCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS)
			bRequestSent = true;
		else if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT)
			jcbCaps |= jcbMainCaps;

		if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT && r->m_tszCapsExt) {
			wchar_t *caps = mir_wstrdup(r->m_tszCapsExt);

			wchar_t *token = wcstok(caps, L" ");
			while (token) {
				for (int i = 0; i < _countof(g_JabberFeatCapPairsExt); i++) {
					if (!mir_wstrcmp(g_JabberFeatCapPairsExt[i].szFeature, token)) {
						jcbCaps |= g_JabberFeatCapPairsExt[i].jcbCap;
						break;
					}
				}

				token = wcstok(nullptr, L" ");
			}

			mir_free(caps);
		}

		if (bRequestSent)
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;

		return jcbCaps | r->m_jcbManualDiscoveredCaps;
	}

	// no XEP-0115: send query each time it's needed
	switch(r->m_dwDiscoInfoRequestTime) {
	case -1:
		return r->m_jcbCachedCaps;

	case 0:
		RequestOldCapsInfo(r, jid);
		break;
	}
	return JABBER_RESOURCE_CAPS_IN_PROGRESS;
}

void CJabberProto::RequestOldCapsInfo(pResourceStatus &r, const wchar_t *fullJid)
{
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
	pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
	r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

	m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO));
}

void CJabberProto::GetCachedCaps(const wchar_t *szNode, const wchar_t *szVer, pResourceStatus &r)
{
	CMStringA szName(FORMAT, "%S#%S", szNode, szVer);
	ptrA szValue(db_get_sa(0, "JabberCaps", szName));
	if (szValue != 0) {
		JSONNode root = JSONNode::parse(szValue);
		if (root) {
			CMStringW wszCaps = root["c"].as_mstring();
			r->m_pCaps = m_clientCapsManager.SetClientCaps(szNode, szVer, nullptr, _wtoi64(wszCaps));
			r->m_pCaps->m_szOs = mir_wstrdup(root["o"].as_mstring());
			r->m_pCaps->m_szOsVer = mir_wstrdup(root["ov"].as_mstring());
			r->m_pCaps->m_szSoft = mir_wstrdup(root["s"].as_mstring());
			r->m_pCaps->m_szSoftVer = mir_wstrdup(root["sv"].as_mstring());
			r->m_pCaps->m_szSoftMir = mir_wstrdup(root["sm"].as_mstring());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//  CJabberClientPartialCaps class

CJabberClientPartialCaps::CJabberClientPartialCaps(CJabberClientCaps *pParent, const wchar_t *szHash, const wchar_t *szVer)
	: m_parent(pParent),
	m_szHash(mir_wstrdup(szHash)),
	m_szSoftVer(mir_wstrdup(szVer)),
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

CJabberClientPartialCaps* CJabberClientCaps::FindByVersion(const wchar_t *szHash)
{
	if (m_pCaps == nullptr)
		return nullptr;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while (pCaps) {
		if (!mir_wstrcmp(szHash, pCaps->GetHash()))
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

CJabberClientCaps::CJabberClientCaps(const wchar_t *szNode) :
	m_szNode(mir_wstrdup(szNode))
{
	m_pCaps = nullptr;
}

CJabberClientCaps::~CJabberClientCaps()
{
	mir_free(m_szNode);
	delete m_pCaps;
}

JabberCapsBits CJabberClientCaps::GetPartialCaps(const wchar_t *szVer)
{
	CJabberClientPartialCaps *pCaps = FindByVersion(szVer);
	return (pCaps) ? pCaps->GetCaps() : JABBER_RESOURCE_CAPS_UNINIT;
}

CJabberClientPartialCaps* CJabberClientCaps::SetPartialCaps(const wchar_t *szHash, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId)
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
	return mir_wstrcmp(p1->GetNode(), p2->GetNode());
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
	for (int i = 0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		jcb |= ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap;
	if (!ppro->m_options.AllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;

	if (ppro->m_options.UseOMEMO)
		jcb |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;

	CMStringA feat_buf(FORMAT, "client/pc//Miranda %d.%d.%d.%d<", __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
	for (int i = 0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
			feat_buf.Append(_T2A(g_JabberFeatCapPairs[i].szFeature));
			feat_buf.AppendChar('<');
		}

	for (int i = 0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (jcb & ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap) {
			feat_buf.Append(_T2A(ppro->m_lstJabberFeatCapPairsDynamic[i]->szFeature));
			feat_buf.AppendChar('<');
		}

	wchar_t szOsBuffer[256]; szOsBuffer[0] = 0;
	GetOSDisplayString(szOsBuffer, _countof(szOsBuffer));

	feat_buf.Append("software_version"); feat_buf.AppendChar('<');
	feat_buf.Append(__VERSION_STRING_DOTS); feat_buf.AppendChar('<');
	
	feat_buf.Append("x-miranda-core-version"); feat_buf.AppendChar('<'); 
	feat_buf.Append(_T2A(szCoreVersion)); feat_buf.AppendChar('<');

	BYTE hash[MIR_SHA1_HASH_SIZE];
	mir_sha1_hash((BYTE*)feat_buf.c_str(), feat_buf.GetLength(), hash);
	ptrA szHash(mir_base64_encode((BYTE*)&hash, sizeof(hash)));
	m_szFeaturesCrc = szHash;
}

const wchar_t* CJabberClientCapsManager::GetFeaturesCrc()
{
	return m_szFeaturesCrc.c_str();
}

CJabberClientCaps* CJabberClientCapsManager::FindClient(const wchar_t *szNode)
{
	if (szNode == nullptr)
		return nullptr;

	return m_arCaps.find((CJabberClientCaps*)&szNode);
}

JabberCapsBits CJabberClientCapsManager::GetClientCaps(const wchar_t *szNode, const wchar_t *szVer)
{
	mir_cslockfull lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		lck.unlock();
		ppro->debugLogW(L"CAPS: get no caps for: %s, %s", szNode, szVer);
		return JABBER_RESOURCE_CAPS_UNINIT;
	}
	JabberCapsBits jcbCaps = pClient->GetPartialCaps(szVer);
	lck.unlock();
	ppro->debugLogW(L"CAPS: get caps %I64x for: %s, %s", jcbCaps, szNode, szVer);
	return jcbCaps;
}

CJabberClientPartialCaps* CJabberClientCapsManager::GetPartialCaps(const wchar_t *szNode, const wchar_t *szHash)
{
	mir_cslock lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	return (pClient == nullptr) ? nullptr : pClient->FindByVersion(szHash);
}

CJabberClientPartialCaps* CJabberClientCapsManager::SetClientCaps(const wchar_t *szNode, const wchar_t *szHash, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId)
{
	mir_cslockfull lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		pClient = new CJabberClientCaps(szNode);
		m_arCaps.insert(pClient);
	}
	
	CJabberClientPartialCaps *res = pClient->SetPartialCaps(szHash, szVer, jcbCaps, nIqId);
	lck.unlock();
	ppro->debugLogW(L"CAPS: set caps %I64x for: %s#%s => [%s]", jcbCaps, szHash, szNode, szVer);
	return res;
}

bool CJabberClientCapsManager::HandleInfoRequest(HXML, CJabberIqInfo *pInfo, const wchar_t *szNode)
{
	int i;
	JabberCapsBits jcb = 0;

	if (szNode) {
		wchar_t szExtCap[512], szExtCapWHash[560];
		mir_snwprintf(szExtCap, L"%s#%s", JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc.c_str());
		if (!mir_wstrcmp(szExtCap, szNode)) {
			szNode = nullptr;
			goto LBL_All;
		}

		for (i = 0; g_JabberFeatCapPairsExt[i].szFeature; i++) {
			if (!g_JabberFeatCapPairsExt[i].Valid())
				continue;
			// TODO: something better here
			mir_snwprintf(szExtCap, L"%s#%s", JABBER_CAPS_MIRANDA_NODE, g_JabberFeatCapPairsExt[i].szFeature);
			mir_snwprintf(szExtCapWHash, L"%s %s", szExtCap, m_szFeaturesCrc.c_str());
			if (!mir_wstrcmp(szNode, szExtCap) || !mir_wstrcmp(szNode, szExtCapWHash)) {
				jcb = g_JabberFeatCapPairsExt[i].jcbCap;
				break;
			}
		}

		// check features registered through IJabberNetInterface::RegisterFeature() and IJabberNetInterface::AddFeatures()
		for (i = 0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++) {
			// TODO: something better here
			mir_snwprintf(szExtCap, L"%s#%s", JABBER_CAPS_MIRANDA_NODE, ppro->m_lstJabberFeatCapPairsDynamic[i]->szExt);
			mir_snwprintf(szExtCapWHash, L"%s %s", szExtCap, m_szFeaturesCrc.c_str());
			if (!mir_wstrcmp(szNode, szExtCap) || !mir_wstrcmp(szNode, szExtCapWHash)) {
				jcb = ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap;
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
		for (i=0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
			jcb |= ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap;
	}

	if (ppro->m_options.UseOMEMO)
		jcb |= JABBER_CAPS_OMEMO_DEVICELIST_NOTIFY;

	if (!ppro->m_options.AllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;

	XmlNodeIq iq(L"result", pInfo);

	HXML query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
	if (szNode)
		query << XATTR(L"node", szNode);

	CMStringW szName(FORMAT, L"Miranda %d.%d.%d.%d", __MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
	query << XCHILD(L"identity") << XATTR(L"category", L"client") << XATTR(L"type", L"pc") << XATTR(L"name", szName);

	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap)
			query << XCHILD(L"feature") << XATTR(L"var", g_JabberFeatCapPairs[i].szFeature);

	for (i=0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (jcb & ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap)
			query << XCHILD(L"feature") << XATTR(L"var", ppro->m_lstJabberFeatCapPairsDynamic[i]->szFeature);

	if (ppro->m_options.AllowVersionRequests && !szNode) {
		HXML form = query << XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"result");
		form << XCHILD(L"field") << XATTR(L"var", L"FORM_TYPE") << XATTR(L"type", L"hidden")
			<< XCHILD(L"value", L"urn:xmpp:dataforms:softwareinfo");

		CJabberClientPartialCaps *pCaps = GetPartialCaps(JABBER_CAPS_MIRANDA_NODE, m_szFeaturesCrc);
		if (ppro->m_options.ShowOSVersion) {
			form << XCHILD(L"field") << XATTR(L"var", L"os") << XCHILD(L"value", pCaps->GetOs());
			form << XCHILD(L"field") << XATTR(L"var", L"os_version") << XCHILD(L"value", pCaps->GetOsVer());
		}
		form << XCHILD(L"field") << XATTR(L"var", L"software") << XCHILD(L"value", pCaps->GetSoft());
		form << XCHILD(L"field") << XATTR(L"var", L"software_version") << XCHILD(L"value", pCaps->GetSoftVer());
		form << XCHILD(L"field") << XATTR(L"var", L"x-miranda-core-version") << XCHILD(L"value", pCaps->GetSoftMir());
	}

	ppro->m_ThreadInfo->send(iq);
	return true;
}
