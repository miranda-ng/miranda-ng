/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static int OnModulesLoaded(WPARAM, LPARAM)
{
	for (auto &it : Accounts())
		if (auto *pApi = getJabberApi(it->szModuleName)) {
			auto *pAccount = new CJabberAccount(pApi);
			g_arJabber.insert(pAccount);
			pAccount->Init();
		}

	return 0;
}

static int OnAccountCreated(WPARAM reason, LPARAM param)
{
	if (reason == PRAC_ADDED) {
		auto *pa = (PROTOACCOUNT *)param;
		if (auto *pApi = getJabberApi(pa->szModuleName)) {
			auto *pAccount = new CJabberAccount(pApi);
			g_arJabber.insert(pAccount);
			pAccount->Init();
		}
	}
	return 0;
}

static int OnSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (!hContact) {
		auto *pcws = (DBCONTACTWRITESETTING *)lParam;
		if (!mir_strcmp(pcws->szSetting, "EnableVOIP")) {
			for (auto &it : g_arJabber) {
				if (!mir_strcmp(it->m_szModuleName, pcws->szModule)) {
					it->InitVoip(pcws->value.bVal != 0);
					break;
				}
			}
		}
	}

	return 0;
}

void CJabberAccount::InitHooks()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, &OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, &OnAccountCreated);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, &OnSettingChanged);
}

///////////////////////////////////////////////////////////////////////////////
// Permanent IQ handler

static BOOL OnProcessJingle(struct IJabberInterface *api, const TiXmlElement *node, void *pUserData)
{
	auto *pThis = (CJabberAccount *)pUserData;

	auto *child = XmlGetChildByTag(node, "jingle", "xmlns", JABBER_FEAT_JINGLE);
	if (!child)
		return false;

	const char *type = XmlGetAttr(node, "type");
	if (type == nullptr)
		return false;

	const char *szAction = XmlGetAttr(child, "action");
	const char *szSid = XmlGetAttr(child, "sid");

	if (!mir_strcmp(type, "get") || !mir_strcmp(type, "set")) {
		const char *idStr = XmlGetAttr(node, "id");
		const char *from = XmlGetAttr(node, "from");
		const char *szInitiator = XmlGetAttr(child, "initiator");
		auto *content = XmlGetChildByTag(child, "content", "creator", "initiator");

		if (szAction && szSid) {
			if (!mir_strcmp(szAction, "session-initiate")) {
				// if this is a Jingle 'session-initiate' and noone processed it yet, reply with "unsupported-applications"
				api->SendXml(XmlNodeIq("result", idStr, from));

				const TiXmlElement *descr = XmlGetChildByTag(content, "description", "xmlns", JABBER_FEAT_JINGLE_RTP);
				const char *reason = NULL;
				if (pThis->m_bEnableVOIP && descr) {
					if (pThis->m_voipSession.IsEmpty()) {
						pThis->m_voipSession = szSid;
						pThis->m_voipPeerJid = from;
						pThis->m_isOutgoing = false;
						pThis->m_offerNode = child->DeepClone(&pThis->m_offerDoc)->ToElement();

						//Make call GUI
						VOICE_CALL vc = {};
						vc.cbSize = sizeof(VOICE_CALL);
						vc.moduleName = pThis->m_szModuleName;
						vc.id = szSid;                         // Protocol specific ID for this call
						vc.hContact = api->ContactFromJID(from);   // Contact associated with the call (can be NULL)
						vc.state = VOICE_STATE_RINGING;
						vc.szNumber.a = pThis->m_voipPeerJid;
						NotifyEventHooks(pThis->m_hVoiceEvent, WPARAM(&vc), 0);

						// ringing message
						XmlNodeIq iq("set", api->GetSerialNext(), from);
						TiXmlElement *rjNode = iq << XCHILDNS("jingle", JABBER_FEAT_JINGLE);
						rjNode << XATTR("action", "session-info") << XATTR("sid", szSid);
						if (szInitiator)
							rjNode << XATTR("initiator", szInitiator);
						rjNode << XCHILDNS("ringing", "urn:xmpp:jingle:apps:rtp:info:1");

						api->SendXml(iq);
						return true;
					}

					// Save this event to history
					PROTORECVEVENT recv = {};
					recv.timestamp = (uint32_t)time(0);
					recv.szMessage = "** A call while we were busy **";
					ProtoChainRecvMsg(api->ContactFromJID(from), &recv);
					reason = "busy";
				}

				XmlNodeIq iq("set", api->GetSerialNext(), from);
				TiXmlElement *jingleNode = iq << XCHILDNS("jingle", JABBER_FEAT_JINGLE);
				jingleNode << XATTR("action", "session-terminate") << XATTR("sid", szSid);
				if (szInitiator)
					jingleNode << XATTR("initiator", szInitiator);
				jingleNode << XCHILD("reason") << XCHILD(reason ? reason : "unsupported-applications");

				api->SendXml(iq);
				return true;
			}
			else if (!mir_strcmp(szAction, "session-accept")) {
				if (pThis->m_bEnableVOIP && pThis->m_voipSession == szSid) {
					api->SendXml(XmlNodeIq("result", idStr, from));
					if (pThis->OnRTPDescription(child)) {
						//Make call GUI
						VOICE_CALL vc = {};
						vc.cbSize = sizeof(VOICE_CALL);
						vc.moduleName = pThis->m_szModuleName;
						vc.id = szSid;
						vc.hContact = api->ContactFromJID(from);
						vc.state = VOICE_STATE_TALKING;
						NotifyEventHooks(pThis->m_hVoiceEvent, WPARAM(&vc), 0);
					}
					return true;
				}
			}
			else if (!mir_strcmp(szAction, "session-terminate")) {
				if (pThis->m_bEnableVOIP && pThis->m_voipSession == szSid) {
					// EndCall()
					api->SendXml(XmlNodeIq("result", idStr, from));

					VOICE_CALL vc = {};
					vc.cbSize = sizeof(VOICE_CALL);
					vc.moduleName = pThis->m_szModuleName;
					vc.id = szSid;
					vc.hContact = api->ContactFromJID(from);
					vc.state = VOICE_STATE_ENDED;
					NotifyEventHooks(pThis->m_hVoiceEvent, WPARAM(&vc), 0);

					pThis->VOIPTerminateSession(nullptr);
					return true;
				}
			}
			else if (!mir_strcmp(szAction, "transport-info")) {
				auto *transport = XmlGetChildByTag(content, "transport", "xmlns", JABBER_FEAT_JINGLE_ICEUDP);
				if (pThis->m_bEnableVOIP && pThis->m_voipSession == szSid && transport) {
					api->SendXml(XmlNodeIq("result", idStr, from));
					if (const TiXmlElement *candidate = XmlFirstChild(transport, "candidate")) {
						pThis->OnICECandidate(candidate);
						return true;
					}
				}
			}
		}

		// if it's something else than 'session-initiate' and noone processed it yet, reply with "unknown-session"
		XmlNodeIq iq("error", idStr, from);
		TiXmlElement *errNode = iq << XCHILD("error");
		errNode << XATTR("type", "cancel");
		errNode << XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas");
		errNode << XCHILDNS("unknown-session", "urn:xmpp:jingle:errors:1");
		api->SendXml(iq);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Services

static INT_PTR __cdecl JabberVOIP_call(void *pThis, WPARAM hContact, LPARAM)
{
	auto *pAcc = (CJabberAccount *)pThis;
	if (pAcc->VOIPCallIinitiate(hContact)) {
		VOICE_CALL vc = {};
		vc.cbSize = sizeof(VOICE_CALL);
		vc.moduleName = pAcc->m_szModuleName;
		vc.id = pAcc->m_voipSession;                // Protocol especific ID for this call
		vc.hContact = hContact;       // Contact associated with the call (can be NULL)
		vc.state = VOICE_STATE_READY;
		vc.szNumber.a = pAcc->m_voipPeerJid;
		NotifyEventHooks(pAcc->m_hVoiceEvent, WPARAM(&vc), 0);
	}

	return 0;
}

static INT_PTR __cdecl JabberVOIP_answercall(void *pThis, WPARAM id, LPARAM)
{
	auto *pAcc = (CJabberAccount *)pThis;
	if (strcmp((const char *)id, pAcc->m_voipSession))
		return 0;

	VOICE_CALL vc = {};
	vc.cbSize = sizeof(VOICE_CALL);
	vc.moduleName = pAcc->m_szModuleName;
	vc.hContact = pAcc->m_api->ContactFromJID(pAcc->m_voipPeerJid);// Contact associated with the call (can be NULL)
	vc.szNumber.a = pAcc->m_voipPeerJid;
	vc.id = pAcc->m_voipSession;
	vc.state = VOICE_STATE_ENDED;

	if (pAcc->VOIPCreatePipeline()) {
		if (pAcc->m_isOutgoing)
			vc.state = VOICE_STATE_CALLING;
		else if (pAcc->OnRTPDescription(pAcc->m_offerNode))
			vc.state = VOICE_STATE_TALKING;
		else
			pAcc->VOIPTerminateSession();
	}

	NotifyEventHooks(pAcc->m_hVoiceEvent, WPARAM(&vc), 0);
	return 0;
}

static INT_PTR __cdecl JabberVOIP_dropcall(void *pThis, WPARAM id, LPARAM)
{
	auto *pAcc = (CJabberAccount *)pThis;

	VOICE_CALL vc = {};
	vc.cbSize = sizeof(VOICE_CALL);
	vc.moduleName = pAcc->m_szModuleName;
	vc.id = (char *)id;
	vc.state = VOICE_STATE_ENDED;
	NotifyEventHooks(pAcc->m_hVoiceEvent, WPARAM(&vc), 0);

	pAcc->VOIPTerminateSession();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// CJabberAccount members

CJabberAccount::CJabberAccount(IJabberInterface *_1) :
	m_api(_1),
	m_szModuleName(m_api->GetModuleName()),
	m_bEnableVOIP(m_api->GetModuleName(), "EnableVOIP", false)
{
	CMStringA tmp(m_szModuleName);
	m_hVoiceEvent = CreateHookableEvent(tmp + PE_VOICE_CALL_STATE);
	CreateServiceFunctionObj(tmp + PS_VOICE_CALL, &JabberVOIP_call, this);
	CreateServiceFunctionObj(tmp + PS_VOICE_ANSWERCALL, &JabberVOIP_answercall, this);
	CreateServiceFunctionObj(tmp + PS_VOICE_DROPCALL, &JabberVOIP_dropcall, this);
}

CJabberAccount::~CJabberAccount()
{
	DestroyHookableEvent(m_hVoiceEvent);

	if (m_bEnableVOIP)
		InitVoip(false);
}

void CJabberAccount::Init()
{
	m_api->RegisterFeature(JABBER_FEAT_JINGLE, LPGEN("Supports Jingle"), "jingle");
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_ICEUDP, LPGEN("Jingle ICE-UDP Transport"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_RTP, LPGEN("Jingle RTP"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_DTLS, LPGEN("Jingle DTLS"));
	m_api->RegisterFeature(JABBER_FEAT_JINGLE_RTPAUDIO, LPGEN("Jingle RTP Audio"));

	m_api->AddFeatures(JABBER_FEAT_JINGLE "\0" JABBER_FEAT_JINGLE_ICEUDP "\0" JABBER_FEAT_JINGLE_RTP "\0" JABBER_FEAT_JINGLE_DTLS "\0" JABBER_FEAT_JINGLE_RTPAUDIO "\0\0");

	m_api->AddIqHandler(&OnProcessJingle, JABBER_IQ_TYPE_ANY, JABBER_FEAT_JINGLE, 0, this);

	if (m_bEnableVOIP)
		InitVoip(true);
}
