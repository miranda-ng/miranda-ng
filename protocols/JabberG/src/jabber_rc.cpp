/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

CJabberAdhocSession::CJabberAdhocSession(CJabberProto* global)
{
	m_pNext = nullptr;
	m_pUserData = nullptr;
	m_bAutofreeUserData = FALSE;
	m_dwStage = 0;
	ppro = global;
	m_szSessionId.Format(L"%u%u", ppro->SerialNext(), GetTickCount());
	m_dwStartTime = GetTickCount();
}

BOOL CJabberProto::IsRcRequestAllowedByACL(CJabberIqInfo *pInfo)
{
	if (!pInfo || !pInfo->GetFrom())
		return FALSE;

	return IsMyOwnJID(pInfo->GetFrom());
}

BOOL CJabberProto::HandleAdhocCommandRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return TRUE;

	if (!m_options.EnableRemoteControl || !IsRcRequestAllowedByACL(pInfo)) {
		// FIXME: send error and return
		return TRUE;
	}

	const wchar_t *szNode = XmlGetAttrValue(pInfo->GetChildNode(), L"node");
	if (!szNode)
		return TRUE;

	m_adhocManager.HandleCommandRequest(iqNode, pInfo, (wchar_t*)szNode);
	return TRUE;
}

BOOL CJabberAdhocManager::HandleItemsRequest(HXML, CJabberIqInfo *pInfo, const wchar_t *szNode)
{
	if (!szNode || !m_pProto->m_options.EnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return FALSE;

	if (!mir_wstrcmp(szNode, JABBER_FEAT_COMMANDS)) {
		XmlNodeIq iq(L"result", pInfo);
		HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR(L"node", JABBER_FEAT_COMMANDS);
		{
			mir_cslock lck(m_cs);

			CJabberAdhocNode* pNode = GetFirstNode();
			while (pNode) {
				wchar_t *szJid = pNode->GetJid();
				if (!szJid)
					szJid = m_pProto->m_ThreadInfo->fullJID;

				resultQuery << XCHILD(L"item") << XATTR(L"jid", szJid)
					<< XATTR(L"node", pNode->GetNode()) << XATTR(L"name", pNode->GetName());

				pNode = pNode->GetNext();
			}
		}

		m_pProto->m_ThreadInfo->send(iq);
		return TRUE;
	}
	return FALSE;
}

BOOL CJabberAdhocManager::HandleInfoRequest(HXML, CJabberIqInfo *pInfo, const wchar_t *szNode)
{
	if (!szNode || !m_pProto->m_options.EnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return FALSE;

	// FIXME: same code twice
	if (!mir_wstrcmp(szNode, JABBER_FEAT_COMMANDS)) {
		XmlNodeIq iq(L"result", pInfo);
		HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(L"node", JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD(L"identity") << XATTR(L"name", L"Ad-hoc commands")
			<< XATTR(L"category", L"automation") << XATTR(L"type", L"command-node");

		resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_DATA_FORMS);
		resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_DISCO_INFO);
		resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_DISCO_ITEMS);

		m_pProto->m_ThreadInfo->send(iq);
		return TRUE;
	}

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode *pNode = FindNode(szNode);
	if (pNode == nullptr)
		return FALSE;

	XmlNodeIq iq(L"result", pInfo);
	HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(L"node", JABBER_FEAT_DISCO_INFO);
	resultQuery << XCHILD(L"identity") << XATTR(L"name", pNode->GetName())
		<< XATTR(L"category", L"automation") << XATTR(L"type", L"command-node");

	resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_COMMANDS);
	resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_DATA_FORMS);
	resultQuery << XCHILD(L"feature") << XATTR(L"var", JABBER_FEAT_DISCO_INFO);
	lck.unlock();
	m_pProto->m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberAdhocManager::HandleCommandRequest(HXML iqNode, CJabberIqInfo *pInfo, const wchar_t *szNode)
{
	// ATTN: ACL and db settings checked in calling function

	HXML commandNode = pInfo->GetChildNode();

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode* pNode = FindNode(szNode);
	if (!pNode) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(L"error", pInfo)
				<< XCHILD(L"error") << XATTR(L"type", L"cancel")
					<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));

		return FALSE;
	}

	const wchar_t *szSessionId = XmlGetAttrValue(commandNode, L"sessionid");

	CJabberAdhocSession* pSession = nullptr;
	if (szSessionId) {
		pSession = FindSession(szSessionId);
		if (!pSession) {
			lck.unlock();

			XmlNodeIq iq(L"error", pInfo);
			HXML errorNode = iq << XCHILD(L"error") << XATTR(L"type", L"modify");
			errorNode << XCHILDNS(L"bad-request", L"urn:ietf:params:xml:ns:xmpp-stanzas");
			errorNode << XCHILDNS(L"bad-sessionid", JABBER_FEAT_COMMANDS);
			m_pProto->m_ThreadInfo->send(iq);
			return FALSE;
		}
	}
	else
		pSession = AddNewSession();

	if (!pSession) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(L"error", pInfo)
				<< XCHILD(L"error") << XATTR(L"type", L"cancel")
					<< XCHILDNS(L"forbidden", L"urn:ietf:params:xml:ns:xmpp-stanzas"));

		return FALSE;
	}

	// session id and node exits here, call handler

	int nResultCode = pNode->CallHandler(iqNode, pInfo, pSession);

	if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_COMPLETED) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(L"result", pInfo)
				<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", szNode)
					<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"completed")
				<< XCHILD(L"note", TranslateT("Command completed successfully")) << XATTR(L"type", L"info"));

		RemoveSession(pSession);
		pSession = nullptr;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_CANCEL) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(L"result", pInfo)
				<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", szNode)
					<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"canceled")
				<< XCHILD(L"note", TranslateT("Error occurred during processing command")) << XATTR(L"type", L"error"));

		RemoveSession(pSession);
		pSession = nullptr;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION) {
		RemoveSession(pSession);
		pSession = nullptr;
	}

	return TRUE;
}

BOOL CJabberAdhocManager::FillDefaultNodes()
{
	AddNode(nullptr, JABBER_FEAT_RC_SET_STATUS,       TranslateT("Set status"),              &CJabberProto::AdhocSetStatusHandler);
	AddNode(nullptr, JABBER_FEAT_RC_SET_OPTIONS,      TranslateT("Set options"),             &CJabberProto::AdhocOptionsHandler);
	AddNode(nullptr, JABBER_FEAT_RC_FORWARD,          TranslateT("Forward unread messages"), &CJabberProto::AdhocForwardHandler);
	AddNode(nullptr, JABBER_FEAT_RC_LEAVE_GROUPCHATS, TranslateT("Leave group chats"),        &CJabberProto::AdhocLeaveGroupchatsHandler);
	AddNode(nullptr, JABBER_FEAT_RC_WS_LOCK,          TranslateT("Lock workstation"),        &CJabberProto::AdhocLockWSHandler);
	AddNode(nullptr, JABBER_FEAT_RC_QUIT_MIRANDA,     TranslateT("Quit Miranda NG"),         &CJabberProto::AdhocQuitMirandaHandler);
	return TRUE;
}


static char *StatusModeToDbSetting(int status,const char *suffix)
{
	char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY:       prefix="Away";	    break;
		case ID_STATUS_NA:         prefix="Na";	    break;
		case ID_STATUS_DND:        prefix="Dnd";      break;
		case ID_STATUS_OCCUPIED:   prefix="Occupied"; break;
		case ID_STATUS_FREECHAT:   prefix="FreeChat"; break;
		case ID_STATUS_ONLINE:     prefix="On";       break;
		case ID_STATUS_OFFLINE:    prefix="Off";      break;
		case ID_STATUS_INVISIBLE:  prefix="Inv";      break;
		case ID_STATUS_ONTHEPHONE: prefix="Otp";      break;
		case ID_STATUS_OUTTOLUNCH: prefix="Otl";      break;
		case ID_STATUS_IDLE:       prefix="Idl";      break;
		default: return nullptr;
	}
	mir_strcpy(str,prefix); mir_strcat(str,suffix);
	return str;
}

int CJabberProto::AdhocSetStatusHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(L"result", pInfo);
		HXML xNode = iq
			<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_SET_STATUS)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"executing")
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form");

		xNode << XCHILD(L"title", TranslateT("Change Status"));
		xNode << XCHILD(L"instructions", TranslateT("Choose the status and status message"));

		xNode << XCHILD(L"field") << XATTR(L"type", L"hidden") << XATTR(L"var", L"FORM_TYPE")
			<< XATTR(L"value", JABBER_FEAT_RC);

		HXML fieldNode = xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Status"))
			<< XATTR(L"type", L"list-single") << XATTR(L"var", L"status");

		fieldNode << XCHILD(L"required");

		int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
		switch (status) {
		case ID_STATUS_INVISIBLE:
			fieldNode << XCHILD(L"value", L"invisible");
			break;
		case ID_STATUS_AWAY:
		case ID_STATUS_ONTHEPHONE:
		case ID_STATUS_OUTTOLUNCH:
			fieldNode << XCHILD(L"value", L"away");
			break;
		case ID_STATUS_NA:
			fieldNode << XCHILD(L"value", L"xa");
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			fieldNode << XCHILD(L"value", L"dnd");
			break;
		case ID_STATUS_FREECHAT:
			fieldNode << XCHILD(L"value", L"chat");
			break;
		case ID_STATUS_ONLINE:
		default:
			fieldNode << XCHILD(L"value", L"online");
			break;
		}

		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Free for chat")) << XCHILD(L"value", L"chat");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Online")) << XCHILD(L"value", L"online");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Away")) << XCHILD(L"value", L"away");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Extended away (Not available)")) << XCHILD(L"value", L"xa");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Do not disturb")) << XCHILD(L"value", L"dnd");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Invisible")) << XCHILD(L"value", L"invisible");
		fieldNode << XCHILD(L"option") << XATTR(L"label", TranslateT("Offline")) << XCHILD(L"value", L"offline");

		// priority
		wchar_t szPriority[ 256 ];
		mir_snwprintf(szPriority, L"%d", (int)getDword("Priority", 5));
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Priority")) << XATTR(L"type", L"text-single")
			<< XATTR(L"var", L"status-priority") << XCHILD(L"value", szPriority);

		// status message text
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Status message"))
			<< XATTR(L"type", L"text-multi") << XATTR(L"var", L"status-message");

		// global status
		fieldNode = xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Change global status"))
			<< XATTR(L"type", L"boolean") << XATTR(L"var", L"status-global");

		ptrW tszStatusMsg((wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, status, 0));
		if (tszStatusMsg)
			fieldNode << XCHILD(L"value", tszStatusMsg);

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}
	
	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		HXML fieldNode = XmlGetChildByTag(xNode, "field", "var", L"status"), valueNode;
		if (!fieldNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		LPCTSTR ptszValue = XmlGetText( XmlGetChild(fieldNode , "value"));
		if (ptszValue == nullptr)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		int status;
		if (!mir_wstrcmp(ptszValue, L"away")) status = ID_STATUS_AWAY;
		else if (!mir_wstrcmp(ptszValue, L"xa")) status = ID_STATUS_NA;
		else if (!mir_wstrcmp(ptszValue, L"dnd")) status = ID_STATUS_DND;
		else if (!mir_wstrcmp(ptszValue, L"chat")) status = ID_STATUS_FREECHAT;
		else if (!mir_wstrcmp(ptszValue, L"online")) status = ID_STATUS_ONLINE;
		else if (!mir_wstrcmp(ptszValue, L"invisible")) status = ID_STATUS_INVISIBLE;
		else if (!mir_wstrcmp(ptszValue, L"offline")) status = ID_STATUS_OFFLINE;
		else
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		int priority = -9999;

		fieldNode = XmlGetChildByTag(xNode, "field", "var", L"status-priority");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (ptszValue = XmlGetText(valueNode))
				priority = _wtoi(ptszValue);

		if (priority >= -128 && priority <= 127)
			setDword("Priority", priority);

		const wchar_t *szStatusMessage = nullptr;
		fieldNode = XmlGetChildByTag(xNode, "field", "var", L"status-message");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			szStatusMessage = XmlGetText(valueNode);

		// skip f...ng away dialog
		int nNoDlg = db_get_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 0);
		db_set_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 1);

		db_set_ws(0, "SRAway", StatusModeToDbSetting(status, "Msg"), szStatusMessage ? szStatusMessage : L"");

		fieldNode = XmlGetChildByTag(xNode, "field", "var", L"status-global");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value"))) {
			if ((ptszValue = XmlGetText(valueNode)) != nullptr && _wtoi(ptszValue))
				Clist_SetStatusMode(status);
			else
				CallProtoService(m_szModuleName, PS_SETSTATUS, status, 0);
		}
		SetAwayMsg(status, szStatusMessage);

		// return NoDlg setting
		db_set_b(0, "SRAway", StatusModeToDbSetting(status, "NoDlg"), (BYTE)nNoDlg);

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocOptionsHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(L"result", pInfo);
		HXML xNode = iq
			<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_SET_OPTIONS)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"executing")
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form");

		xNode << XCHILD(L"title", TranslateT("Set Options"));
		xNode << XCHILD(L"instructions", TranslateT("Set the desired options"));

		xNode << XCHILD(L"field") << XATTR(L"type", L"hidden") << XATTR(L"var", L"FORM_TYPE")
			<< XATTR(L"value", JABBER_FEAT_RC);

		// Automatically Accept File Transfers
		wchar_t szTmpBuff[ 1024 ];
		mir_snwprintf(szTmpBuff, L"%d", db_get_b(0, "SRFile", "AutoAccept", 0));
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Automatically Accept File Transfers"))
			<< XATTR(L"type", L"boolean") << XATTR(L"var", L"auto-files") << XCHILD(L"value", szTmpBuff);

		// Use sounds
		mir_snwprintf(szTmpBuff, L"%d", db_get_b(0, "Skin", "UseSound", 0));
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Play sounds"))
			<< XATTR(L"type", L"boolean") << XATTR(L"var", L"sounds") << XCHILD(L"value", szTmpBuff);

		// Disable remote controlling
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Disable remote controlling (check twice what you are doing)"))
			<< XATTR(L"type", L"boolean") << XATTR(L"var", L"enable-rc") << XCHILD(L"value", L"0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Automatically Accept File Transfers
		HXML fieldNode = XmlGetChildByTag(xNode, "field", "var", L"auto-files"), valueNode;
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (XmlGetText(valueNode))
				db_set_b(0, "SRFile", "AutoAccept", (BYTE)_wtoi(XmlGetText(valueNode)));

		// Use sounds
		fieldNode = XmlGetChildByTag(xNode, "field", "var", L"sounds");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (XmlGetText(valueNode))
				db_set_b(0, "Skin", "UseSound", (BYTE)_wtoi(XmlGetText(valueNode)));

		// Disable remote controlling
		fieldNode = XmlGetChildByTag(xNode, "field", "var", L"enable-rc");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (XmlGetText(valueNode) && _wtoi(XmlGetText(valueNode)))
				m_options.EnableRemoteControl = 0;

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::RcGetUnreadEventsCount()
{
	int nEventsSent = 0;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrW jid( getWStringA(hContact, "jid"));
		if (jid == nullptr) continue;

		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = {};
			dbei.cbBlob = db_event_getBlobSize(hDbEvent);
			if (dbei.cbBlob == -1)
				continue;

			dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob + 1);
			int nGetTextResult = db_event_get(hDbEvent, &dbei);
			if (!nGetTextResult && dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT)) {
				wchar_t *szEventText = DbEvent_GetTextW(&dbei, CP_ACP);
				if (szEventText) {
					nEventsSent++;
					mir_free(szEventText);
				}
			}
			mir_free(dbei.pBlob);
		}
	}
	return nEventsSent;
}

int CJabberProto::AdhocForwardHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	wchar_t szMsg[ 1024 ];
	if (pSession->GetStage() == 0) {
		int nUnreadEvents = RcGetUnreadEventsCount();
		if (!nUnreadEvents) {
			mir_snwprintf(szMsg, TranslateT("There is no messages to forward"));

			m_ThreadInfo->send(
				XmlNodeIq(L"result", pInfo)
					<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_FORWARD)
						<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"completed")
					<< XCHILD(L"note", szMsg) << XATTR(L"type", L"info"));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(L"result", pInfo);
		HXML xNode = iq
			<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_FORWARD)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"executing")
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form");

		xNode << XCHILD(L"title", TranslateT("Forward options"));

		mir_snwprintf(szMsg, TranslateT("%d message(s) to be forwarded"), nUnreadEvents);
		xNode << XCHILD(L"instructions", szMsg);

		xNode << XCHILD(L"field") << XATTR(L"type", L"hidden") << XATTR(L"var", L"FORM_TYPE")
			<< XCHILD(L"value", JABBER_FEAT_RC);

		// remove clist events
		xNode << XCHILD(L"field") << XATTR(L"label", TranslateT("Mark messages as read")) << XATTR(L"type", L"boolean")
			<< XATTR(L"var", L"remove-clist-events") << XCHILD(L"value",
			m_options.RcMarkMessagesAsRead == 1 ? L"1" : L"0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		BOOL bRemoveCListEvents = TRUE;

		// remove clist events
		HXML fieldNode = XmlGetChildByTag(xNode,"field", "var", L"remove-clist-events"), valueNode;
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (XmlGetText(valueNode) && !_wtoi(XmlGetText(valueNode)))
				bRemoveCListEvents = FALSE;

		m_options.RcMarkMessagesAsRead = bRemoveCListEvents ? 1 : 0;

		int nEventsSent = 0;
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			ptrW tszJid( getWStringA(hContact, "jid"));
			if (tszJid == nullptr)
				continue;
				
			for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
				DBEVENTINFO dbei = {};
				dbei.cbBlob = db_event_getBlobSize(hDbEvent);
				if (dbei.cbBlob == -1)
					continue;

				mir_ptr<BYTE> pEventBuf((PBYTE)mir_alloc(dbei.cbBlob + 1));
				dbei.pBlob = pEventBuf;
				if (db_event_get(hDbEvent, &dbei))
					continue;

				if (dbei.eventType != EVENTTYPE_MESSAGE || (dbei.flags & (DBEF_READ | DBEF_SENT)))
					continue;

				ptrW szEventText( DbEvent_GetTextW(&dbei, CP_ACP));
				if (szEventText == nullptr)
					continue;

				XmlNode msg(L"message");
				msg << XATTR(L"to", pInfo->GetFrom()) << XATTRID(SerialNext())
					<< XCHILD(L"body", szEventText);

				HXML addressesNode = msg << XCHILDNS(L"addresses", JABBER_FEAT_EXT_ADDRESSING);
				wchar_t szOFrom[JABBER_MAX_JID_LEN];

				size_t cbBlob = mir_strlen((LPSTR)dbei.pBlob)+1;
				if (cbBlob < dbei.cbBlob) { // rest of message contains a sender's resource
					ptrW szOResource( mir_utf8decodeW((LPSTR)dbei.pBlob + cbBlob+1));
					mir_snwprintf(szOFrom, L"%s/%s", tszJid, szOResource);
				} else
					wcsncpy_s(szOFrom, tszJid, _TRUNCATE);

				addressesNode << XCHILD(L"address") << XATTR(L"type", L"ofrom") << XATTR(L"jid", szOFrom);
				addressesNode << XCHILD(L"address") << XATTR(L"type", L"oto") << XATTR(L"jid", m_ThreadInfo->fullJID);

				time_t ltime = (time_t)dbei.timestamp;
				struct tm *gmt = gmtime(&ltime);
				wchar_t stime[512];
				mir_snwprintf(stime, L"%.4i-%.2i-%.2iT%.2i:%.2i:%.2iZ", gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
					gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
				msg << XCHILDNS(L"delay", L"urn:xmpp:delay") << XATTR(L"stamp", stime);

				m_ThreadInfo->send(msg);

				nEventsSent++;

				db_event_markRead(hContact, hDbEvent);
				if (bRemoveCListEvents)
					pcli->pfnRemoveEvent(hContact, hDbEvent);
			}
		}

		mir_snwprintf(szMsg, TranslateT("%d message(s) forwarded"), nEventsSent);

		m_ThreadInfo->send(
			XmlNodeIq(L"result", pInfo)
				<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_FORWARD)
					<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"completed")
				<< XCHILD(L"note", szMsg) << XATTR(L"type", L"info"));

		return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
	}

	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocLockWSHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	BOOL bOk = LockWorkStation();

	wchar_t szMsg[ 1024 ];
	if (bOk)
		mir_snwprintf(szMsg, TranslateT("Workstation successfully locked"));
	else
		mir_snwprintf(szMsg, TranslateT("Error %d occurred during workstation lock"), GetLastError());

	m_ThreadInfo->send(
		XmlNodeIq(L"result", pInfo)
			<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_WS_LOCK)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"completed")
			<< XCHILD(L"note", szMsg) << XATTR(L"type", bOk ? L"info" : L"error"));

	return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
}

static void __stdcall JabberQuitMirandaIMThread(void*)
{
	CallService("CloseAction", 0, 0);
}

int CJabberProto::AdhocQuitMirandaHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(L"result", pInfo);
		HXML xNode = iq
			<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_QUIT_MIRANDA)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"executing")
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form");

		xNode << XCHILD(L"title", TranslateT("Confirmation needed"));
		xNode << XCHILD(L"instructions", TranslateT("Please confirm Miranda NG shutdown"));

		xNode << XCHILD(L"field") << XATTR(L"type", L"hidden") << XATTR(L"var", L"FORM_TYPE")
			<< XCHILD(L"value", JABBER_FEAT_RC);

		// I Agree checkbox
		xNode << XCHILD(L"field") << XATTR(L"label", L"I agree") << XATTR(L"type", L"boolean")
			<< XATTR(L"var", L"allow-shutdown") << XCHILD(L"value", L"0");

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		HXML fieldNode, valueNode;

		// I Agree checkbox
		fieldNode = XmlGetChildByTag(xNode,"field", "var", L"allow-shutdown");
		if (fieldNode && (valueNode = XmlGetChild(fieldNode , "value")))
			if (XmlGetText(valueNode) && _wtoi(XmlGetText(valueNode)))
				CallFunctionAsync(JabberQuitMirandaIMThread, 0);

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocLeaveGroupchatsHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
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
			wchar_t szMsg[ 1024 ];
			mir_snwprintf(szMsg, TranslateT("There is no group chats to leave"));

			m_ThreadInfo->send(
				XmlNodeIq(L"result", pInfo)
					<< XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_LEAVE_GROUPCHATS)
						<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"completed")
					<< XCHILD(L"note", szMsg) << XATTR(L"type", L"info"));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		pSession->SetStage(1);

		XmlNodeIq iq(L"result", pInfo);
		HXML xNode = iq
			<<	XCHILDNS(L"command", JABBER_FEAT_COMMANDS) << XATTR(L"node", JABBER_FEAT_RC_LEAVE_GROUPCHATS)
				<< XATTR(L"sessionid", pSession->GetSessionId()) << XATTR(L"status", L"executing")
			<< XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"form");

		xNode << XCHILD(L"title", TranslateT("Leave group chats"));
		xNode << XCHILD(L"instructions", TranslateT("Choose the group chats you want to leave"));

		xNode << XCHILD(L"field") << XATTR(L"type", L"hidden") << XATTR(L"var", L"FORM_TYPE")
			<< XATTR(L"value", JABBER_FEAT_RC);

		// Groupchats
		HXML fieldNode = xNode << XCHILD(L"field") << XATTR(L"label", nullptr) << XATTR(L"type", L"list-multi") << XATTR(L"var", L"groupchats");
		fieldNode << XCHILD(L"required");
		{
			mir_cslock lck(m_csLists);
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != nullptr)
					fieldNode << XCHILD(L"option") << XATTR(L"label", item->jid) << XCHILD(L"value", item->jid);
			}
		}

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = XmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Groupchat list here:
		HXML fieldNode = XmlGetChildByTag(xNode,"field", "var", L"groupchats");
		if (fieldNode) {
			for (i=0; i < XmlGetChildCount(fieldNode); i++) {
				HXML valueNode = XmlGetChild(fieldNode, i);
				if (valueNode && XmlGetName(valueNode) && XmlGetText(valueNode) && !mir_wstrcmp(XmlGetName(valueNode), L"value")) {
					JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, XmlGetText(valueNode));
					if (item)
						GcQuit(item, 0, nullptr);
				}
			}
		}

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}