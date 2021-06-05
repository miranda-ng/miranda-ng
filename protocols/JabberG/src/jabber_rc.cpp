/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-21 Miranda NG team

XEP-0146 support for Miranda IM

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
#include "jabber_rc.h"

CJabberAdhocSession::CJabberAdhocSession(CJabberProto* _ppro) :
	ppro(_ppro)
{
	m_szSessionId.Format("%u%u", ppro->SerialNext(), GetTickCount());
	m_dwStartTime = GetTickCount();
}

bool CJabberProto::IsRcRequestAllowedByACL(CJabberIqInfo *pInfo)
{
	if (!pInfo || !pInfo->GetFrom())
		return false;

	return IsMyOwnJID(pInfo->GetFrom());
}

bool CJabberProto::HandleAdhocCommandRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return true;

	if (!m_bEnableRemoteControl || !IsRcRequestAllowedByACL(pInfo)) {
		// FIXME: send error and return
		return true;
	}

	const char *szNode = XmlGetAttr(pInfo->GetChildNode(), "node");
	if (szNode)
		m_adhocManager.HandleCommandRequest(iqNode, pInfo, szNode);
	return true;
}

bool CJabberAdhocManager::HandleItemsRequest(const TiXmlElement*, CJabberIqInfo *pInfo, const char *szNode)
{
	if (!szNode || !m_pProto->m_bEnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return false;

	if (mir_strcmp(szNode, JABBER_FEAT_COMMANDS))
		return false;

	XmlNodeIq iq("result", pInfo);
	TiXmlElement *resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR("node", JABBER_FEAT_COMMANDS);
	{
		mir_cslock lck(m_cs);

		CJabberAdhocNode* pNode = GetFirstNode();
		while (pNode) {
			const char *szJid = pNode->GetJid();
			if (!szJid)
				szJid = m_pProto->m_ThreadInfo->fullJID;

			resultQuery << XCHILD("item") << XATTR("jid", szJid)
				<< XATTR("node", pNode->GetNode()) << XATTR("name", pNode->GetName());

			pNode = pNode->GetNext();
		}
	}

	m_pProto->m_ThreadInfo->send(iq);
	return true;
}

bool CJabberAdhocManager::HandleInfoRequest(const TiXmlElement*, CJabberIqInfo *pInfo, const char *szNode)
{
	if (!szNode || !m_pProto->m_bEnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return false;

	// FIXME: same code twice
	if (!mir_strcmp(szNode, JABBER_FEAT_COMMANDS)) {
		XmlNodeIq iq("result", pInfo);
		TiXmlElement *resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR("node", JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD("identity") << XATTR("name", "Ad-hoc commands")
			<< XATTR("category", "automation") << XATTR("type", "command-node");

		resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_DATA_FORMS);
		resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_DISCO_INFO);
		resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_DISCO_ITEMS);

		m_pProto->m_ThreadInfo->send(iq);
		return true;
	}

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode *pNode = FindNode(szNode);
	if (pNode == nullptr)
		return false;

	XmlNodeIq iq("result", pInfo);
	TiXmlElement *resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR("node", JABBER_FEAT_DISCO_INFO);
	resultQuery << XCHILD("identity") << XATTR("name", pNode->GetName())
		<< XATTR("category", "automation") << XATTR("type", "command-node");

	resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_COMMANDS);
	resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_DATA_FORMS);
	resultQuery << XCHILD("feature") << XATTR("var", JABBER_FEAT_DISCO_INFO);
	lck.unlock();
	m_pProto->m_ThreadInfo->send(iq);
	return true;
}

bool CJabberAdhocManager::HandleCommandRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo, const char *szNode)
{
	// ATTN: ACL and db settings checked in calling function

	auto *commandNode = pInfo->GetChildNode();

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode* pNode = FindNode(szNode);
	if (!pNode) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq("error", pInfo)
			<< XCHILD("error") << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));

		return false;
	}

	const char *szSessionId = XmlGetAttr(commandNode, "sessionid");

	CJabberAdhocSession *pSession = nullptr;
	if (szSessionId) {
		pSession = FindSession(szSessionId);
		if (!pSession) {
			lck.unlock();

			XmlNodeIq iq("error", pInfo);
			TiXmlElement *errorNode = iq << XCHILD("error") << XATTR("type", "modify");
			errorNode << XCHILDNS("bad-request", "urn:ietf:params:xml:ns:xmpp-stanzas");
			errorNode << XCHILDNS("bad-sessionid", JABBER_FEAT_COMMANDS);
			m_pProto->m_ThreadInfo->send(iq);
			return false;
		}
	}
	else pSession = AddNewSession();

	if (!pSession) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq("error", pInfo)
			<< XCHILD("error") << XATTR("type", "cancel")
			<< XCHILDNS("forbidden", "urn:ietf:params:xml:ns:xmpp-stanzas"));

		return false;
	}

	// session id and node exits here, call handler

	int nResultCode = pNode->CallHandler(iqNode, pInfo, pSession);

	if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_COMPLETED) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq("result", pInfo)
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", szNode)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "completed")
			<< XCHILD("note", TranslateU("Command completed successfully")) << XATTR("type", "info"));

		RemoveSession(pSession);
		pSession = nullptr;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_CANCEL) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq("result", pInfo)
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", szNode)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "canceled")
			<< XCHILD("note", TranslateU("Error occurred during processing command")) << XATTR("type", "error"));

		RemoveSession(pSession);
		pSession = nullptr;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION) {
		RemoveSession(pSession);
		pSession = nullptr;
	}

	return true;
}

void CJabberAdhocManager::FillDefaultNodes()
{
	AddNode(nullptr, JABBER_FEAT_RC_SET_STATUS, TranslateU("Set status"), &CJabberProto::AdhocSetStatusHandler);
	AddNode(nullptr, JABBER_FEAT_RC_SET_OPTIONS, TranslateU("Set options"), &CJabberProto::AdhocOptionsHandler);
	AddNode(nullptr, JABBER_FEAT_RC_FORWARD, TranslateU("Forward unread messages"), &CJabberProto::AdhocForwardHandler);
	AddNode(nullptr, JABBER_FEAT_RC_LEAVE_GROUPCHATS, TranslateU("Leave group chats"), &CJabberProto::AdhocLeaveGroupchatsHandler);
	AddNode(nullptr, JABBER_FEAT_RC_WS_LOCK, TranslateU("Lock workstation"), &CJabberProto::AdhocLockWSHandler);
	AddNode(nullptr, JABBER_FEAT_RC_QUIT_MIRANDA, TranslateU("Quit Miranda NG"), &CJabberProto::AdhocQuitMirandaHandler);
}

static char* StatusModeToDbSetting(int status, const char *suffix)
{
	char *prefix;
	static char str[64];

	switch (status) {
	case ID_STATUS_AWAY:       prefix = "Away";	    break;
	case ID_STATUS_NA:         prefix = "Na";	    break;
	case ID_STATUS_DND:        prefix = "Dnd";      break;
	case ID_STATUS_OCCUPIED:   prefix = "Occupied"; break;
	case ID_STATUS_FREECHAT:   prefix = "FreeChat"; break;
	case ID_STATUS_ONLINE:     prefix = "On";       break;
	case ID_STATUS_OFFLINE:    prefix = "Off";      break;
	case ID_STATUS_INVISIBLE:  prefix = "Inv";      break;
	case ID_STATUS_IDLE:       prefix = "Idl";      break;
	default: return nullptr;
	}
	mir_strcpy(str, prefix); mir_strcat(str, suffix);
	return str;
}

int CJabberProto::AdhocSetStatusHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq("result", pInfo);
		TiXmlElement *xNode = iq
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_SET_STATUS)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "executing")
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form");

		xNode << XCHILD("title", TranslateU("Change Status"));
		xNode << XCHILD("instructions", TranslateU("Choose the status and status message"));

		xNode << XCHILD("field") << XATTR("type", "hidden") << XATTR("var", "FORM_TYPE")
			<< XATTR("value", JABBER_FEAT_RC);

		TiXmlElement *fieldNode = xNode << XCHILD("field") << XATTR("label", TranslateU("Status"))
			<< XATTR("type", "list-single") << XATTR("var", "status");

		fieldNode << XCHILD("required");

		int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
		switch (status) {
		case ID_STATUS_INVISIBLE:
			fieldNode << XCHILD("value", "invisible");
			break;
		case ID_STATUS_AWAY:
			fieldNode << XCHILD("value", "away");
			break;
		case ID_STATUS_NA:
			fieldNode << XCHILD("value", "xa");
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			fieldNode << XCHILD("value", "dnd");
			break;
		case ID_STATUS_FREECHAT:
			fieldNode << XCHILD("value", "chat");
			break;
		case ID_STATUS_ONLINE:
		default:
			fieldNode << XCHILD("value", "online");
			break;
		}

		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Free for chat")) << XCHILD("value", "chat");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Online")) << XCHILD("value", "online");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Away")) << XCHILD("value", "away");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Extended away (Not available)")) << XCHILD("value", "xa");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Do not disturb")) << XCHILD("value", "dnd");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Invisible")) << XCHILD("value", "invisible");
		fieldNode << XCHILD("option") << XATTR("label", TranslateU("Offline")) << XCHILD("value", "offline");

		// priority
		char szPriority[20];
		itoa(getDword("Priority", 5), szPriority, 10);
		xNode << XCHILD("field") << XATTR("label", TranslateU("Priority")) << XATTR("type", "text-single")
			<< XATTR("var", "status-priority") << XCHILD("value", szPriority);

		// status message text
		xNode << XCHILD("field") << XATTR("label", TranslateU("Status message"))
			<< XATTR("type", "text-multi") << XATTR("var", "status-message");

		// global status
		fieldNode = xNode << XCHILD("field") << XATTR("label", TranslateU("Change global status"))
			<< XATTR("type", "boolean") << XATTR("var", "status-global");

		ptrW tszStatusMsg((wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, status, 0));
		if (tszStatusMsg)
			fieldNode << XCHILD("value", T2Utf(tszStatusMsg));

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		auto *commandNode = pInfo->GetChildNode();
		auto *xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "status");
		if (!fieldNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		auto *nodeValue = XmlFirstChild(fieldNode, "value");
		if (nodeValue == nullptr)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		const char *pszValue = nodeValue->GetText();
		int status;
		if (!mir_strcmp(pszValue, "away")) status = ID_STATUS_AWAY;
		else if (!mir_strcmp(pszValue, "xa")) status = ID_STATUS_NA;
		else if (!mir_strcmp(pszValue, "dnd")) status = ID_STATUS_DND;
		else if (!mir_strcmp(pszValue, "chat")) status = ID_STATUS_FREECHAT;
		else if (!mir_strcmp(pszValue, "online")) status = ID_STATUS_ONLINE;
		else if (!mir_strcmp(pszValue, "invisible")) status = ID_STATUS_INVISIBLE;
		else if (!mir_strcmp(pszValue, "offline")) status = ID_STATUS_OFFLINE;
		else
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		int priority = -9999;

		if (fieldNode = XmlGetChildByTag(xNode, "field", "var", "status-priority"))
			priority = XmlGetChildInt(fieldNode, "value");

		if (priority >= -128 && priority <= 127)
			setDword("Priority", priority);

		const char *szStatusMessage = nullptr;
		if (fieldNode = XmlGetChildByTag(xNode, "field", "var", "status-message"))
			if (auto *valueNode = XmlFirstChild(fieldNode, "value"))
				szStatusMessage = valueNode->GetText();

		// skip f...ng away dialog
		int nNoDlg = db_get_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 0);
		db_set_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 1);

		db_set_utf(0, "SRAway", StatusModeToDbSetting(status, "Msg"), szStatusMessage ? szStatusMessage : "");

		if (fieldNode = XmlGetChildByTag(xNode, "field", "var", "status-global")) {
			if (XmlGetChildInt(fieldNode, "value") > 0)
				Clist_SetStatusMode(status);
			else
				CallProtoService(m_szModuleName, PS_SETSTATUS, status, 0);
		}

		SetAwayMsg(status, Utf2T(szStatusMessage));

		// return NoDlg setting
		db_set_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), (BYTE)nNoDlg);

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocOptionsHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq("result", pInfo);
		TiXmlElement *xNode = iq
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_SET_OPTIONS)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "executing")
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form");

		xNode << XCHILD("title", TranslateU("Set Options"));
		xNode << XCHILD("instructions", TranslateU("Set the desired options"));

		xNode << XCHILD("field") << XATTR("type", "hidden") << XATTR("var", "FORM_TYPE")
			<< XATTR("value", JABBER_FEAT_RC);

		// Automatically Accept File Transfers
		char szTmpBuff[20];
		_itoa_s(db_get_b(0, "SRFile", "AutoAccept", 0), szTmpBuff, 10);
		xNode << XCHILD("field") << XATTR("label", TranslateU("Automatically Accept File Transfers"))
			<< XATTR("type", "boolean") << XATTR("var", "auto-files") << XCHILD("value", szTmpBuff);

		// Use sounds
		_itoa_s(db_get_b(0, "Skin", "UseSound", 0), szTmpBuff, 10);
		xNode << XCHILD("field") << XATTR("label", TranslateU("Play sounds"))
			<< XATTR("type", "boolean") << XATTR("var", "sounds") << XCHILD("value", szTmpBuff);

		// Disable remote controlling
		xNode << XCHILD("field") << XATTR("label", TranslateU("Disable remote controlling (check twice what you are doing)"))
			<< XATTR("type", "boolean") << XATTR("var", "enable-rc") << XCHILD("value", "0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		auto *commandNode = pInfo->GetChildNode();
		auto *xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Automatically Accept File Transfers
		if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "auto-files"))
			db_set_b(0, "SRFile", "AutoAccept", XmlGetChildInt(fieldNode, "value"));

		// Use sounds
		if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "sounds"))
			db_set_b(0, "Skin", "UseSound", XmlGetChildInt(fieldNode, "value"));

		// Disable remote controlling
		if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "enable-rc"))
			m_bEnableRemoteControl = XmlGetChildInt(fieldNode, "value");

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::RcGetUnreadEventsCount()
{
	int nEventsSent = 0;
	for (auto &hContact : AccContacts()) {
		ptrW jid(getWStringA(hContact, "jid"));
		if (jid == nullptr) continue;

		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DB::EventInfo dbei;
			dbei.cbBlob = -1;

			int nGetTextResult = db_event_get(hDbEvent, &dbei);
			if (!nGetTextResult && dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT)) {
				ptrW szEventText(DbEvent_GetTextW(&dbei, CP_ACP));
				if (szEventText)
					nEventsSent++;
			}
		}
	}
	return nEventsSent;
}

int CJabberProto::AdhocForwardHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	if (pSession->GetStage() == 0) {
		int nUnreadEvents = RcGetUnreadEventsCount();
		if (!nUnreadEvents) {
			m_ThreadInfo->send(
				XmlNodeIq("result", pInfo)
				<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_FORWARD)
				<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "completed")
				<< XCHILD("note", TranslateU("There is no messages to forward")) << XATTR("type", "info"));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		// first form
		pSession->SetStage(1);

		XmlNodeIq iq("result", pInfo);
		TiXmlElement *xNode = iq
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_FORWARD)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "executing")
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form");

		xNode << XCHILD("title", TranslateU("Forward options"));

		char szMsg[1024];
		mir_snprintf(szMsg, TranslateU("%d message(s) to be forwarded"), nUnreadEvents);
		xNode << XCHILD("instructions", szMsg);

		xNode << XCHILD("field") << XATTR("type", "hidden") << XATTR("var", "FORM_TYPE")
			<< XCHILD("value", JABBER_FEAT_RC);

		// remove clist events
		xNode << XCHILD("field") << XATTR("label", TranslateU("Mark messages as read")) << XATTR("type", "boolean")
			<< XATTR("var", "remove-clist-events") << XCHILD("value", m_bRcMarkMessagesAsRead == 1 ? "1" : "0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		auto *commandNode = pInfo->GetChildNode();
		auto *xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		bool bRemoveCListEvents = true;

		// remove clist events
		if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "remove-clist-events"))
			bRemoveCListEvents = XmlGetChildInt(fieldNode, "value");

		m_bRcMarkMessagesAsRead = bRemoveCListEvents ? 1 : 0;

		int nEventsSent = 0;
		for (auto &hContact : AccContacts()) {
			ptrA szJid(getUStringA(hContact, "jid"));
			if (szJid == nullptr)
				continue;

			for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
				DB::EventInfo dbei;
				dbei.cbBlob = -1;
				if (db_event_get(hDbEvent, &dbei))
					continue;

				if (dbei.eventType != EVENTTYPE_MESSAGE || (dbei.flags & (DBEF_READ | DBEF_SENT)))
					continue;

				ptrW szEventText(DbEvent_GetTextW(&dbei, CP_ACP));
				if (szEventText == nullptr)
					continue;

				XmlNode msg("message");
				msg << XATTR("to", pInfo->GetFrom()) << XATTRID(SerialNext())
					<< XCHILD("body", T2Utf(szEventText));

				TiXmlElement *addressesNode = msg << XCHILDNS("addresses", JABBER_FEAT_EXT_ADDRESSING);
				
				CMStringA szOFrom;
				size_t cbBlob = mir_strlen((LPSTR)dbei.pBlob) + 1;
				if (cbBlob < dbei.cbBlob) // rest of message contains a sender's resource
					szOFrom = MakeJid(szJid, (LPSTR)dbei.pBlob + cbBlob + 1);
				else
					szOFrom = szJid;

				addressesNode << XCHILD("address") << XATTR("type", "ofrom") << XATTR("jid", szOFrom);
				addressesNode << XCHILD("address") << XATTR("type", "oto") << XATTR("jid", m_ThreadInfo->fullJID);

				time_t ltime = (time_t)dbei.timestamp;
				struct tm *gmt = gmtime(&ltime);
				char stime[512];
				mir_snprintf(stime, "%.4i-%.2i-%.2iT%.2i:%.2i:%.2iZ", gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
					gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
				msg << XCHILDNS("delay", "urn:xmpp:delay") << XATTR("stamp", stime);

				m_ThreadInfo->send(msg);

				nEventsSent++;

				db_event_markRead(hContact, hDbEvent);
				if (bRemoveCListEvents)
					g_clistApi.pfnRemoveEvent(hContact, hDbEvent);
			}
		}

		m_ThreadInfo->send(
			XmlNodeIq("result", pInfo)
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_FORWARD)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "completed")
			<< XCHILD("note", CMStringA(FORMAT, TranslateU("%d message(s) forwarded"), nEventsSent)) << XATTR("type", "info"));

		return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
	}

	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocLockWSHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	BOOL bOk = LockWorkStation();

	char szMsg[1024];
	if (bOk)
		mir_snprintf(szMsg, TranslateU("Workstation successfully locked"));
	else
		mir_snprintf(szMsg, TranslateU("Error %d occurred during workstation lock"), GetLastError());

	m_ThreadInfo->send(
		XmlNodeIq("result", pInfo)
		<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_WS_LOCK)
		<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "completed")
		<< XCHILD("note", szMsg) << XATTR("type", bOk ? "info" : "error"));

	return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
}

static void __stdcall JabberQuitMirandaIMThread(void*)
{
	CallService("CloseAction", 0, 0);
}

int CJabberProto::AdhocQuitMirandaHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq("result", pInfo);
		TiXmlElement *xNode = iq
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_QUIT_MIRANDA)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "executing")
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form");

		xNode << XCHILD("title", TranslateU("Confirmation needed"));
		xNode << XCHILD("instructions", TranslateU("Please confirm Miranda NG shutdown"));

		xNode << XCHILD("field") << XATTR("type", "hidden") << XATTR("var", "FORM_TYPE")
			<< XCHILD("value", JABBER_FEAT_RC);

		// I Agree checkbox
		xNode << XCHILD("field") << XATTR("label", "I agree") << XATTR("type", "boolean")
			<< XATTR("var", "allow-shutdown") << XCHILD("value", "0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		auto *commandNode = pInfo->GetChildNode();
		auto *xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// I Agree checkbox
		if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "allow-shutdown"))
			if (XmlGetChildInt(fieldNode, "value"))
				CallFunctionAsync(JabberQuitMirandaIMThread, nullptr);

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocLeaveGroupchatsHandler(const TiXmlElement*, CJabberIqInfo *pInfo, CJabberAdhocSession *pSession)
{
	int i = 0;
	if (pSession->GetStage() == 0) {
		// first form
		int nChatsCount = 0;
		{
			mir_cslock lck(m_csLists);
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != nullptr)
					nChatsCount++;
			}
		}

		if (!nChatsCount) {
			m_ThreadInfo->send(
				XmlNodeIq("result", pInfo)
				<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_LEAVE_GROUPCHATS)
				<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "completed")
				<< XCHILD("note", TranslateU("There is no group chats to leave")) << XATTR("type", "info"));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		pSession->SetStage(1);

		XmlNodeIq iq("result", pInfo);
		TiXmlElement *xNode = iq
			<< XCHILDNS("command", JABBER_FEAT_COMMANDS) << XATTR("node", JABBER_FEAT_RC_LEAVE_GROUPCHATS)
			<< XATTR("sessionid", pSession->GetSessionId()) << XATTR("status", "executing")
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form");

		xNode << XCHILD("title", TranslateU("Leave group chats"));
		xNode << XCHILD("instructions", TranslateU("Choose the group chats you want to leave"));

		xNode << XCHILD("field") << XATTR("type", "hidden") << XATTR("var", "FORM_TYPE")
			<< XATTR("value", JABBER_FEAT_RC);

		// Groupchats
		TiXmlElement *fieldNode = xNode << XCHILD("field") << XATTR("label", nullptr) << XATTR("type", "list-multi") << XATTR("var", "groupchats");
		fieldNode << XCHILD("required");
		{
			mir_cslock lck(m_csLists);
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != nullptr)
					fieldNode << XCHILD("option") << XATTR("label", item->jid) << XCHILD("value", item->jid);
			}
		}

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		auto *commandNode = pInfo->GetChildNode();
		auto *xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Groupchat list here:
		auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "groupchats");
		if (fieldNode) {
			for (auto *valueNode : TiXmlFilter(fieldNode, "value")) {
				JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, valueNode->GetText());
				if (item)
					GcQuit(item, 0, nullptr);
			}
		}

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}