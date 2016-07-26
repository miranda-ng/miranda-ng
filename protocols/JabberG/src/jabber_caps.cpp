/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-16 Miranda NG project

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
	{ NULL }
};

const JabberFeatCapPairExt g_JabberFeatCapPairsExt[] = {
	{ JABBER_EXT_SECUREIM,          JABBER_CAPS_SECUREIM,             "SecureIM/IsContactSecured" },
	{ JABBER_EXT_MIROTR,            JABBER_CAPS_MIROTR,               "MirOTRMenuCheckService"    },
	{ JABBER_EXT_NEWGPG,            JABBER_CAPS_NEWGPG,               "/ExportGPGKeys"            },
	{ JABBER_EXT_NUDGE,             JABBER_CAPS_ATTENTION,            "NUDGE/Send"                },
	{ JABBER_EXT_JINGLE,            JABBER_CAPS_JINGLE,               "Jingle/StartSession"       },
	{ JABBER_EXT_COMMANDS,          JABBER_CAPS_COMMANDS                                          },
	{ JABBER_EXT_USER_ACTIVITY,     JABBER_CAPS_USER_ACTIVITY_NOTIFY                              },
	{ JABBER_EXT_USER_MOOD,         JABBER_CAPS_USER_MOOD_NOTIFY                                  },
	{ JABBER_EXT_USER_TUNE,         JABBER_CAPS_USER_TUNE_NOTIFY,     "ListeningTo/Enabled"       },
	{ JABBER_EXT_USER_TUNE,         JABBER_CAPS_USER_TUNE_NOTIFY,     "WATrack/GetFileInfo"       },
	{ JABBER_EXT_MIR_NOTES,         JABBER_CAPS_MIRANDA_NOTES                                     },
	{ szCoreVersion,                    JABBER_CAPS_MIRANDA_PARTIAL                                   },
	{ JABBER_EXT_PLATFORMX86,       JABBER_CAPS_PLATFORMX86                                       },
	{ JABBER_EXT_PLATFORMX64,       JABBER_CAPS_PLATFORMX64                                       },
	{ NULL }
};

void CJabberProto::OnIqResultCapsDiscoInfoSI(HXML, CJabberIqInfo *pInfo)
{
	pResourceStatus r( ResourceInfoFromJID(pInfo->GetFrom()));
	if (r == NULL)
		return;

	if (r->m_tszCapsNode == NULL)
		OnIqResultCapsDiscoInfo(NULL, pInfo);

	HXML query = pInfo->GetChildNode();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT && query) {
		// XEP-0232 support
		HXML xform;
		for (int i = 1; (xform = XmlGetNthChild(query, L"x", i)) != NULL; i++) {
			wchar_t *szFormTypeValue = XPath(xform, L"field[@var='FORM_TYPE']/value");
			if (szFormTypeValue && !mir_tstrcmp(szFormTypeValue, L"urn:xmpp:dataforms:softwareinfo")) {
				wchar_t *szTmp = XPath(xform, L"field[@var='os']/value");
				if (szTmp)
					r->m_tszOs = mir_tstrdup(szTmp);
				szTmp = XPath(xform, L"field[@var='os_version']/value");
				if (szTmp)
					r->m_tszOsVersion = mir_tstrdup(szTmp);
				szTmp = XPath(xform, L"field[@var='software']/value");
				if (szTmp)
					r->m_tszSoftware = mir_tstrdup(szTmp);
				szTmp = XPath(xform, L"field[@var='software_version']/value");
				if (szTmp)
					r->m_tszSoftwareVersion = mir_tstrdup(szTmp);
				szTmp = XPath(xform, L"field[@var='x-miranda-core-version']/value");
				if (szTmp)
					r->m_tszXMirandaCoreVersion = mir_tstrdup(szTmp);

				JabberUserInfoUpdate(pInfo->GetHContact());
			}
		}
	}
}

void CJabberProto::OnIqResultCapsDiscoInfo(HXML, CJabberIqInfo *pInfo)
{
	pResourceStatus r(ResourceInfoFromJID(pInfo->GetFrom()));

	HXML query = pInfo->GetChildNode();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT && query) {
		JabberCapsBits jcbCaps = 0;
		HXML feature;
		for (int i = 1; (feature = XmlGetNthChild(query, L"feature", i)) != NULL; i++) {
			const wchar_t *featureName = XmlGetAttrValue(feature, L"var");
			if (!featureName)
				continue;

			for (int j = 0; g_JabberFeatCapPairs[j].szFeature; j++) {
				if (!mir_tstrcmp(g_JabberFeatCapPairs[j].szFeature, featureName)) {
					jcbCaps |= g_JabberFeatCapPairs[j].jcbCap;
					break;
				}
			}
		}

		// no version info support and no XEP-0115 support?
		if (r && r->m_dwVersionRequestTime == -1 && !r->m_tszSoftwareVersion && !r->m_tszSoftware && !r->m_tszCapsNode) {
			r->m_jcbCachedCaps = jcbCaps;
			r->m_dwDiscoInfoRequestTime = -1;
			return;
		}

		if (r && !m_clientCapsManager.SetClientCaps(pInfo->GetIqId(), jcbCaps))
			r->m_jcbCachedCaps = jcbCaps;

		JabberUserInfoUpdate(pInfo->GetHContact());
	}
	else {
		// no version info support and no XEP-0115 support?
		if (r && r->m_dwVersionRequestTime == -1 && !r->m_tszSoftwareVersion && !r->m_tszSoftware && !r->m_tszCapsNode) {
			r->m_jcbCachedCaps = JABBER_RESOURCE_CAPS_NONE;
			r->m_dwDiscoInfoRequestTime = -1;
		}
		else m_clientCapsManager.SetClientCaps(pInfo->GetIqId(), JABBER_RESOURCE_CAPS_ERROR);
	}
}

JabberCapsBits CJabberProto::GetTotalJidCapabilites(const wchar_t *jid)
{
	if (jid == NULL)
		return JABBER_RESOURCE_CAPS_NONE;

	wchar_t szBareJid[JABBER_MAX_JID_LEN];
	JabberStripJid(jid, szBareJid, _countof(szBareJid));

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, szBareJid);
	if (item == NULL)
		item = ListGetItemPtr(LIST_VCARD_TEMP, szBareJid);

	JabberCapsBits jcbToReturn = JABBER_RESOURCE_CAPS_NONE;

	// get bare jid info only if where is no resources
	if (!item || (item && !item->arResources.getCount())) {
		jcbToReturn = GetResourceCapabilites(szBareJid, FALSE);
		if (jcbToReturn & JABBER_RESOURCE_CAPS_ERROR)
			jcbToReturn = JABBER_RESOURCE_CAPS_NONE;
	}

	if (item) {
		for (int i = 0; i < item->arResources.getCount(); i++) {
			wchar_t szFullJid[JABBER_MAX_JID_LEN];
			mir_sntprintf(szFullJid, L"%s/%s", szBareJid, item->arResources[i]->m_tszResourceName);
			JabberCapsBits jcb = GetResourceCapabilites(szFullJid, FALSE);
			if (!(jcb & JABBER_RESOURCE_CAPS_ERROR))
				jcbToReturn |= jcb;
		}
	}
	return jcbToReturn;
}

JabberCapsBits CJabberProto::GetResourceCapabilites(const wchar_t *jid, BOOL appendBestResource)
{
	wchar_t fullJid[JABBER_MAX_JID_LEN];
	if (appendBestResource)
		GetClientJID(jid, fullJid, _countof(fullJid));
	else
		wcsncpy_s(fullJid, jid, _TRUNCATE);

	pResourceStatus r(ResourceInfoFromJID(fullJid));
	if (r == NULL)
		return JABBER_RESOURCE_CAPS_ERROR;

	// XEP-0115 mode
	if (r->m_tszCapsNode && r->m_tszCapsVer) {
		JabberCapsBits jcbCaps = 0, jcbExtCaps = 0;
		BOOL bRequestSent = FALSE;
		JabberCapsBits jcbMainCaps = m_clientCapsManager.GetClientCaps(r->m_tszCapsNode, r->m_tszCapsVer);

		if (jcbMainCaps == JABBER_RESOURCE_CAPS_TIMEOUT && !r->m_dwDiscoInfoRequestTime)
			jcbMainCaps = JABBER_RESOURCE_CAPS_ERROR;

		if (jcbMainCaps == JABBER_RESOURCE_CAPS_UNINIT) {
			// send disco#info query

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			m_clientCapsManager.SetClientCaps(r->m_tszCapsNode, r->m_tszCapsVer, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId());
			r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			wchar_t queryNode[512];
			mir_sntprintf(queryNode, L"%s#%s", r->m_tszCapsNode, r->m_tszCapsVer);
			m_ThreadInfo->send(XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(L"node", queryNode));

			bRequestSent = TRUE;
		}
		else if (jcbMainCaps == JABBER_RESOURCE_CAPS_IN_PROGRESS)
			bRequestSent = TRUE;
		else if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT)
			jcbCaps |= jcbMainCaps;

		if (jcbMainCaps != JABBER_RESOURCE_CAPS_TIMEOUT && r->m_tszCapsExt) {
			wchar_t *caps = mir_tstrdup(r->m_tszCapsExt);

			wchar_t *token = wcstok(caps, L" ");
			while (token) {
				switch (jcbExtCaps = m_clientCapsManager.GetClientCaps(r->m_tszCapsNode, token)) {
				case JABBER_RESOURCE_CAPS_ERROR:
					break;

				case JABBER_RESOURCE_CAPS_UNINIT:
					{
						// send disco#info query

						CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
						pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
						m_clientCapsManager.SetClientCaps(r->m_tszCapsNode, token, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId());

						m_ThreadInfo->send(
							XmlNodeIq(pInfo) << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(L"node", CMString(FORMAT, L"%s#%s", r->m_tszCapsNode, token)));

						bRequestSent = TRUE;
					}
					break;

				case JABBER_RESOURCE_CAPS_IN_PROGRESS:
					bRequestSent = TRUE;
					break;

				default:
					jcbCaps |= jcbExtCaps;
				}

				token = wcstok(NULL, L" ");
			}

			mir_free(caps);
		}

		if (bRequestSent)
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;

		return jcbCaps | r->m_jcbManualDiscoveredCaps;
	}

	// capability mode (version request + service discovery)

	// no version info:
	if (!r->m_tszSoftwareVersion && !r->m_tszSoftware) {
		// version request not sent:
		if (!r->m_dwVersionRequestTime) {
			// send version query

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultVersion, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_HCONTACT | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			r->m_dwVersionRequestTime = pInfo->GetRequestTime();

			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_VERSION);
			m_ThreadInfo->send(iq);
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		// version not received:
		else if (r->m_dwVersionRequestTime != -1) {
			// no timeout?
			if (GetTickCount() - r->m_dwVersionRequestTime < JABBER_RESOURCE_CAPS_QUERY_TIMEOUT)
				return JABBER_RESOURCE_CAPS_IN_PROGRESS;

			// timeout
			r->m_dwVersionRequestTime = -1;
		}
		// no version information, try direct service discovery
		if (!r->m_dwDiscoInfoRequestTime) {
			// send disco#info query

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_DISCO_INFO);
			m_ThreadInfo->send(iq);

			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		else if (r->m_dwDiscoInfoRequestTime == -1)
			return r->m_jcbCachedCaps | r->m_jcbManualDiscoveredCaps;
		else if (GetTickCount() - r->m_dwDiscoInfoRequestTime < JABBER_RESOURCE_CAPS_QUERY_TIMEOUT)
			return JABBER_RESOURCE_CAPS_IN_PROGRESS;
		else
			r->m_dwDiscoInfoRequestTime = -1;
		// version request timeout:
		return JABBER_RESOURCE_CAPS_NONE;
	}

	// version info available:
	if (r->m_tszSoftware && r->m_tszSoftwareVersion) {
		JabberCapsBits jcbMainCaps = m_clientCapsManager.GetClientCaps(r->m_tszSoftware, r->m_tszSoftwareVersion);
		if (jcbMainCaps == JABBER_RESOURCE_CAPS_ERROR) {
			// Bombus hack:
			if (!mir_tstrcmp(r->m_tszSoftware, L"Bombus") || !mir_tstrcmp(r->m_tszSoftware, L"BombusMod")) {
				jcbMainCaps = JABBER_CAPS_SI | JABBER_CAPS_SI_FT | JABBER_CAPS_IBB | JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY | JABBER_CAPS_DATA_FORMS | JABBER_CAPS_LAST_ACTIVITY | JABBER_CAPS_VERSION | JABBER_CAPS_COMMANDS | JABBER_CAPS_VCARD_TEMP;
				m_clientCapsManager.SetClientCaps(r->m_tszSoftware, r->m_tszSoftwareVersion, jcbMainCaps);
			}
			// Neos hack:
			else if (!mir_tstrcmp(r->m_tszSoftware, L"neos")) {
				jcbMainCaps = JABBER_CAPS_OOB | JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY | JABBER_CAPS_LAST_ACTIVITY | JABBER_CAPS_VERSION;
				m_clientCapsManager.SetClientCaps(r->m_tszSoftware, r->m_tszSoftwareVersion, jcbMainCaps);
			}
			// sim hack:
			else if (!mir_tstrcmp(r->m_tszSoftware, L"sim")) {
				jcbMainCaps = JABBER_CAPS_OOB | JABBER_CAPS_VERSION | JABBER_CAPS_MESSAGE_EVENTS | JABBER_CAPS_MESSAGE_EVENTS_NO_DELIVERY;
				m_clientCapsManager.SetClientCaps(r->m_tszSoftware, r->m_tszSoftwareVersion, jcbMainCaps);
			}
		}

		else if (jcbMainCaps == JABBER_RESOURCE_CAPS_UNINIT) {
			// send disco#info query

			CJabberIqInfo *pInfo = AddIQ(&CJabberProto::OnIqResultCapsDiscoInfo, JABBER_IQ_TYPE_GET, fullJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_CHILD_TAG_NODE);
			pInfo->SetTimeout(JABBER_RESOURCE_CAPS_QUERY_TIMEOUT);
			m_clientCapsManager.SetClientCaps(r->m_tszSoftware, r->m_tszSoftwareVersion, JABBER_RESOURCE_CAPS_IN_PROGRESS, pInfo->GetIqId());
			r->m_dwDiscoInfoRequestTime = pInfo->GetRequestTime();

			XmlNodeIq iq(pInfo);
			iq << XQUERY(JABBER_FEAT_DISCO_INFO);
			m_ThreadInfo->send(iq);

			jcbMainCaps = JABBER_RESOURCE_CAPS_IN_PROGRESS;
		}
		return jcbMainCaps | r->m_jcbManualDiscoveredCaps;
	}

	return JABBER_RESOURCE_CAPS_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////
//  CJabberClientPartialCaps class

CJabberClientPartialCaps::CJabberClientPartialCaps(const wchar_t *szVer)
{
	m_szVer = mir_tstrdup(szVer);
	m_jcbCaps = JABBER_RESOURCE_CAPS_UNINIT;
	m_pNext = NULL;
	m_nIqId = -1;
	m_dwRequestTime = 0;
}

CJabberClientPartialCaps::~CJabberClientPartialCaps()
{
	mir_free(m_szVer);
	if (m_pNext)
		delete m_pNext;
}

CJabberClientPartialCaps* CJabberClientPartialCaps::SetNext(CJabberClientPartialCaps *pCaps)
{
	CJabberClientPartialCaps *pRetVal = m_pNext;
	m_pNext = pCaps;
	return pRetVal;
}

void CJabberClientPartialCaps::SetCaps(JabberCapsBits jcbCaps, int nIqId /*= -1*/)
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

CJabberClientPartialCaps* CJabberClientCaps::FindByVersion(const wchar_t *szVer)
{
	if (!m_pCaps || !szVer)
		return NULL;

	CJabberClientPartialCaps *pCaps = m_pCaps;
	while (pCaps) {
		if (!mir_tstrcmp(szVer, pCaps->GetVersion()))
			break;
		pCaps = pCaps->GetNext();
	}
	return pCaps;
}

CJabberClientPartialCaps* CJabberClientCaps::FindById(int nIqId)
{
	if (!m_pCaps || nIqId == -1)
		return NULL;

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

CJabberClientCaps::CJabberClientCaps(const wchar_t *szNode)
{
	m_szNode = mir_tstrdup(szNode);
	m_pCaps = NULL;
	m_pNext= NULL;
}

CJabberClientCaps::~CJabberClientCaps() {
	mir_free(m_szNode);
	if (m_pCaps)
		delete m_pCaps;
	if (m_pNext)
		delete m_pNext;
}

CJabberClientCaps* CJabberClientCaps::SetNext(CJabberClientCaps *pClient)
{
	CJabberClientCaps *pRetVal = m_pNext;
	m_pNext = pClient;
	return pRetVal;
}

JabberCapsBits CJabberClientCaps::GetPartialCaps(wchar_t *szVer)
{
	CJabberClientPartialCaps *pCaps = FindByVersion(szVer);
	return (pCaps) ? pCaps->GetCaps() : JABBER_RESOURCE_CAPS_UNINIT;
}

BOOL CJabberClientCaps::SetPartialCaps(const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId /*= -1*/)
{
	CJabberClientPartialCaps *pCaps = FindByVersion(szVer);
	if (!pCaps) {
		pCaps = new CJabberClientPartialCaps(szVer);
		if (!pCaps)
			return FALSE;
		pCaps->SetNext(m_pCaps);
		m_pCaps = pCaps;
	}

	pCaps->SetCaps(jcbCaps, nIqId);
	return TRUE;
}

BOOL CJabberClientCaps::SetPartialCaps(int nIqId, JabberCapsBits jcbCaps)
{
	CJabberClientPartialCaps *pCaps = FindById(nIqId);
	if (!pCaps)
		return FALSE;

	pCaps->SetCaps(jcbCaps, -1);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CJabberClientCapsManager class

CJabberClientCapsManager::CJabberClientCapsManager(CJabberProto *proto)
{
	ppro = proto;
	m_pClients = NULL;
}

CJabberClientCapsManager::~CJabberClientCapsManager()
{
	if (m_pClients)
		delete m_pClients;
}

CJabberClientCaps * CJabberClientCapsManager::FindClient(const wchar_t *szNode)
{
	if (!m_pClients || !szNode)
		return NULL;

	CJabberClientCaps *pClient = m_pClients;
	while (pClient) {
		if (!mir_tstrcmp(szNode, pClient->GetNode()))
			break;
		pClient = pClient->GetNext();
	}
	return pClient;
}

void CJabberClientCapsManager::AddDefaultCaps()
{
	SetClientCaps(JABBER_CAPS_MIRANDA_NODE, szCoreVersion, JABBER_CAPS_MIRANDA_ALL);

	for (int i=0; g_JabberFeatCapPairsExt[i].szFeature; i++)
		if (g_JabberFeatCapPairsExt[i].Valid())
			SetClientCaps(JABBER_CAPS_MIRANDA_NODE, g_JabberFeatCapPairsExt[i].szFeature, g_JabberFeatCapPairsExt[i].jcbCap);
}

JabberCapsBits CJabberClientCapsManager::GetClientCaps(wchar_t *szNode, wchar_t *szVer)
{
	mir_cslockfull lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		lck.unlock();
		ppro->debugLog(L"CAPS: get no caps for: %s, %s", szNode, szVer);
		return JABBER_RESOURCE_CAPS_UNINIT;
	}
	JabberCapsBits jcbCaps = pClient->GetPartialCaps(szVer);
	lck.unlock();
	ppro->debugLog(L"CAPS: get caps %I64x for: %s, %s", jcbCaps, szNode, szVer);
	return jcbCaps;
}

BOOL CJabberClientCapsManager::SetClientCaps(const wchar_t *szNode, const wchar_t *szVer, JabberCapsBits jcbCaps, int nIqId /*= -1*/)
{
	mir_cslockfull lck(m_cs);
	CJabberClientCaps *pClient = FindClient(szNode);
	if (!pClient) {
		pClient = new CJabberClientCaps(szNode);
		if (!pClient)
			return FALSE;

		pClient->SetNext(m_pClients);
		m_pClients = pClient;
	}
	BOOL bOk = pClient->SetPartialCaps(szVer, jcbCaps, nIqId);
	lck.unlock();
	ppro->debugLog(L"CAPS: set caps %I64x for: %s, %s", jcbCaps, szNode, szVer);
	return bOk;
}

BOOL CJabberClientCapsManager::SetClientCaps(int nIqId, JabberCapsBits jcbCaps)
{
	mir_cslock lck(m_cs);
	if (!m_pClients)
		return FALSE;

	BOOL bOk = FALSE;
	CJabberClientCaps *pClient = m_pClients;
	while (pClient) {
		if (pClient->SetPartialCaps(nIqId, jcbCaps)) {
			ppro->debugLogA("CAPS: set caps %I64x for iq %d", jcbCaps, nIqId);
			bOk = TRUE;
			break;
		}
		pClient = pClient->GetNext();
	}
	return bOk;
}

BOOL CJabberClientCapsManager::HandleInfoRequest(HXML, CJabberIqInfo *pInfo, const wchar_t *szNode)
{
	int i;
	JabberCapsBits jcb = 0;

	if (szNode) {
		for (i=0; g_JabberFeatCapPairsExt[i].szFeature; i++) {
			if (!g_JabberFeatCapPairsExt[i].Valid())
				continue;

			wchar_t szExtCap[ 512 ];
			mir_sntprintf(szExtCap, L"%s#%s", JABBER_CAPS_MIRANDA_NODE, g_JabberFeatCapPairsExt[i].szFeature);
			if (!mir_tstrcmp(szNode, szExtCap)) {
				jcb = g_JabberFeatCapPairsExt[i].jcbCap;
				break;
			}
		}

		// check features registered through IJabberNetInterface::RegisterFeature() and IJabberNetInterface::AddFeatures()
		for (i=0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++) {
			wchar_t szExtCap[ 512 ];
			mir_sntprintf(szExtCap, L"%s#%s", JABBER_CAPS_MIRANDA_NODE, ppro->m_lstJabberFeatCapPairsDynamic[i]->szExt);
			if (!mir_tstrcmp(szNode, szExtCap)) {
				jcb = ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap;
				break;
			}
		}

		// unknown node, not XEP-0115 request
		if (!jcb)
			return FALSE;
	}
	else {
		jcb = JABBER_CAPS_MIRANDA_ALL;
		for (i=0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
			jcb |= ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap;
	}

	if (!ppro->m_options.AllowVersionRequests)
		jcb &= ~JABBER_CAPS_VERSION;

	XmlNodeIq iq(L"result", pInfo);

	HXML query = iq << XQUERY(JABBER_FEAT_DISCO_INFO);
	if (szNode)
		query << XATTR(L"node", szNode);

	query << XCHILD(L"identity") << XATTR(L"category", L"client")
			<< XATTR(L"type", L"pc") << XATTR(L"name", L"Miranda");

	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap)
			query << XCHILD(L"feature") << XATTR(L"var", g_JabberFeatCapPairs[i].szFeature);

	for (i=0; i < ppro->m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (jcb & ppro->m_lstJabberFeatCapPairsDynamic[i]->jcbCap)
			query << XCHILD(L"feature") << XATTR(L"var", ppro->m_lstJabberFeatCapPairsDynamic[i]->szFeature);

	if (ppro->m_options.AllowVersionRequests && !szNode) {
		wchar_t szOsBuffer[256] = {0};
		wchar_t *os = szOsBuffer;

		if (ppro->m_options.ShowOSVersion) {
			if (!GetOSDisplayString(szOsBuffer, _countof(szOsBuffer)))
				mir_tstrncpy(szOsBuffer, L"", _countof(szOsBuffer));
			else {
				wchar_t *szOsWindows = L"Microsoft Windows";
				size_t nOsWindowsLength = mir_tstrlen(szOsWindows);
				if (!wcsnicmp(szOsBuffer, szOsWindows, nOsWindowsLength))
					os += nOsWindowsLength + 1;
			}
		}

		HXML form = query << XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"result");
		form << XCHILD(L"field") << XATTR(L"var", L"FORM_TYPE") << XATTR(L"type", L"hidden")
			<< XCHILD(L"value", L"urn:xmpp:dataforms:softwareinfo");

		if (ppro->m_options.ShowOSVersion) {
			form << XCHILD(L"field") << XATTR(L"var", L"os") << XCHILD(L"value", L"Microsoft Windows");
			form << XCHILD(L"field") << XATTR(L"var", L"os_version") << XCHILD(L"value", os);
		}
		form << XCHILD(L"field") << XATTR(L"var", L"software") << XCHILD(L"value", L"Miranda NG Jabber Protocol");
		form << XCHILD(L"field") << XATTR(L"var", L"software_version") << XCHILD(L"value", _T(__VERSION_STRING_DOTS));
		form << XCHILD(L"field") << XATTR(L"var", L"x-miranda-core-version") << XCHILD(L"value", szCoreVersion);
	}

	ppro->m_ThreadInfo->send(iq);

	return TRUE;
}
