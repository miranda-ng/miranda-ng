/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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

#include "jabber.h"
#include "jabber_iq.h"
#include "jabber_rc.h"

CJabberAdhocSession::CJabberAdhocSession(CJabberProto* global)
{
	m_pNext = NULL;
	m_pUserData = NULL;
	m_bAutofreeUserData = FALSE;
	m_dwStage = 0;
	ppro = global;
	m_szSessionId.Format(_T("%u%u"), ppro->SerialNext(), GetTickCount());
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

	const TCHAR *szNode = xmlGetAttrValue(pInfo->GetChildNode(), _T("node"));
	if (!szNode)
		return TRUE;

	m_adhocManager.HandleCommandRequest(iqNode, pInfo, (TCHAR*)szNode);
	return TRUE;
}

BOOL CJabberAdhocManager::HandleItemsRequest(HXML, CJabberIqInfo *pInfo, const TCHAR *szNode)
{
	if (!szNode || !m_pProto->m_options.EnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return FALSE;

	if (!_tcscmp(szNode, JABBER_FEAT_COMMANDS)) {
		XmlNodeIq iq(_T("result"), pInfo);
		HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS) << XATTR(_T("node"), JABBER_FEAT_COMMANDS);
		{
			mir_cslock lck(m_cs);

			CJabberAdhocNode* pNode = GetFirstNode();
			while (pNode) {
				TCHAR *szJid = pNode->GetJid();
				if (!szJid)
					szJid = m_pProto->m_ThreadInfo->fullJID;

				resultQuery << XCHILD(_T("item")) << XATTR(_T("jid"), szJid)
					<< XATTR(_T("node"), pNode->GetNode()) << XATTR(_T("name"), pNode->GetName());

				pNode = pNode->GetNext();
			}
		}

		m_pProto->m_ThreadInfo->send(iq);
		return TRUE;
	}
	return FALSE;
}

BOOL CJabberAdhocManager::HandleInfoRequest(HXML, CJabberIqInfo *pInfo, const TCHAR *szNode)
{
	if (!szNode || !m_pProto->m_options.EnableRemoteControl || !m_pProto->IsRcRequestAllowedByACL(pInfo))
		return FALSE;

	// FIXME: same code twice
	if (!_tcscmp(szNode, JABBER_FEAT_COMMANDS)) {
		XmlNodeIq iq(_T("result"), pInfo);
		HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(_T("node"), JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD(_T("identity")) << XATTR(_T("name"), _T("Ad-hoc commands"))
			<< XATTR(_T("category"), _T("automation")) << XATTR(_T("type"), _T("command-node"));

		resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_COMMANDS);
		resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_DATA_FORMS);
		resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_DISCO_INFO);
		resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_DISCO_ITEMS);

		m_pProto->m_ThreadInfo->send(iq);
		return TRUE;
	}

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode *pNode = FindNode(szNode);
	if (pNode == NULL)
		return FALSE;

	XmlNodeIq iq(_T("result"), pInfo);
	HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_INFO) << XATTR(_T("node"), JABBER_FEAT_DISCO_INFO);
	resultQuery << XCHILD(_T("identity")) << XATTR(_T("name"), pNode->GetName())
		<< XATTR(_T("category"), _T("automation")) << XATTR(_T("type"), _T("command-node"));

	resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_COMMANDS);
	resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_DATA_FORMS);
	resultQuery << XCHILD(_T("feature")) << XATTR(_T("var"), JABBER_FEAT_DISCO_INFO);
	lck.unlock();
	m_pProto->m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberAdhocManager::HandleCommandRequest(HXML iqNode, CJabberIqInfo *pInfo, const TCHAR *szNode)
{
	// ATTN: ACL and db settings checked in calling function

	HXML commandNode = pInfo->GetChildNode();

	mir_cslockfull lck(m_cs);
	CJabberAdhocNode* pNode = FindNode(szNode);
	if (!pNode) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(_T("error"), pInfo)
				<< XCHILD(_T("error")) << XATTR(_T("type"), _T("cancel"))
					<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));

		return FALSE;
	}

	const TCHAR *szSessionId = xmlGetAttrValue(commandNode, _T("sessionid"));

	CJabberAdhocSession* pSession = NULL;
	if (szSessionId) {
		pSession = FindSession(szSessionId);
		if (!pSession) {
			lck.unlock();

			XmlNodeIq iq(_T("error"), pInfo);
			HXML errorNode = iq << XCHILD(_T("error")) << XATTR(_T("type"), _T("modify"));
			errorNode << XCHILDNS(_T("bad-request"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"));
			errorNode << XCHILDNS(_T("bad-sessionid"), JABBER_FEAT_COMMANDS);
			m_pProto->m_ThreadInfo->send(iq);
			return FALSE;
		}
	}
	else
		pSession = AddNewSession();

	if (!pSession) {
		lck.unlock();

		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(_T("error"), pInfo)
				<< XCHILD(_T("error")) << XATTR(_T("type"), _T("cancel"))
					<< XCHILDNS(_T("forbidden"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));

		return FALSE;
	}

	// session id and node exits here, call handler

	int nResultCode = pNode->CallHandler(iqNode, pInfo, pSession);

	if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_COMPLETED) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(_T("result"), pInfo)
				<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), szNode)
					<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("completed"))
				<< XCHILD(_T("note"), TranslateT("Command completed successfully")) << XATTR(_T("type"), _T("info")));

		RemoveSession(pSession);
		pSession = NULL;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_CANCEL) {
		m_pProto->m_ThreadInfo->send(
			XmlNodeIq(_T("result"), pInfo)
				<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), szNode)
					<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("canceled"))
				<< XCHILD(_T("note"), TranslateT("Error occurred during processing command")) << XATTR(_T("type"), _T("error")));

		RemoveSession(pSession);
		pSession = NULL;
	}
	else if (nResultCode == JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION) {
		RemoveSession(pSession);
		pSession = NULL;
	}

	return TRUE;
}

BOOL CJabberAdhocManager::FillDefaultNodes()
{
	AddNode(NULL, JABBER_FEAT_RC_SET_STATUS,       TranslateT("Set status"),              &CJabberProto::AdhocSetStatusHandler);
	AddNode(NULL, JABBER_FEAT_RC_SET_OPTIONS,      TranslateT("Set options"),             &CJabberProto::AdhocOptionsHandler);
	AddNode(NULL, JABBER_FEAT_RC_FORWARD,          TranslateT("Forward unread messages"), &CJabberProto::AdhocForwardHandler);
	AddNode(NULL, JABBER_FEAT_RC_LEAVE_GROUPCHATS, TranslateT("Leave group chats"),        &CJabberProto::AdhocLeaveGroupchatsHandler);
	AddNode(NULL, JABBER_FEAT_RC_WS_LOCK,          TranslateT("Lock workstation"),        &CJabberProto::AdhocLockWSHandler);
	AddNode(NULL, JABBER_FEAT_RC_QUIT_MIRANDA,     TranslateT("Quit Miranda NG"),         &CJabberProto::AdhocQuitMirandaHandler);
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
		default: return NULL;
	}
	mir_strcpy(str,prefix); mir_strcat(str,suffix);
	return str;
}

int CJabberProto::AdhocSetStatusHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(_T("result"), pInfo);
		HXML xNode = iq
			<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_SET_STATUS)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("executing"))
			<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"));

		xNode << XCHILD(_T("title"), TranslateT("Change Status"));
		xNode << XCHILD(_T("instructions"), TranslateT("Choose the status and status message"));

		xNode << XCHILD(_T("field")) << XATTR(_T("type"), _T("hidden")) << XATTR(_T("var"), _T("FORM_TYPE"))
			<< XATTR(_T("value"), JABBER_FEAT_RC);

		HXML fieldNode = xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Status"))
			<< XATTR(_T("type"), _T("list-single")) << XATTR(_T("var"), _T("status"));

		fieldNode << XCHILD(_T("required"));

		int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
		switch (status) {
		case ID_STATUS_INVISIBLE:
			fieldNode << XCHILD(_T("value"), _T("invisible"));
			break;
		case ID_STATUS_AWAY:
		case ID_STATUS_ONTHEPHONE:
		case ID_STATUS_OUTTOLUNCH:
			fieldNode << XCHILD(_T("value"), _T("away"));
			break;
		case ID_STATUS_NA:
			fieldNode << XCHILD(_T("value"), _T("xa"));
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			fieldNode << XCHILD(_T("value"), _T("dnd"));
			break;
		case ID_STATUS_FREECHAT:
			fieldNode << XCHILD(_T("value"), _T("chat"));
			break;
		case ID_STATUS_ONLINE:
		default:
			fieldNode << XCHILD(_T("value"), _T("online"));
			break;
		}

		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Free for chat")) << XCHILD(_T("value"), _T("chat"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Online")) << XCHILD(_T("value"), _T("online"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Away")) << XCHILD(_T("value"), _T("away"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Extended Away (N/A)")) << XCHILD(_T("value"), _T("xa"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Do Not Disturb")) << XCHILD(_T("value"), _T("dnd"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Invisible")) << XCHILD(_T("value"), _T("invisible"));
		fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), TranslateT("Offline")) << XCHILD(_T("value"), _T("offline"));

		// priority
		TCHAR szPriority[ 256 ];
		mir_sntprintf(szPriority, SIZEOF(szPriority), _T("%d"), (short)getWord("Priority", 5));
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Priority")) << XATTR(_T("type"), _T("text-single"))
			<< XATTR(_T("var"), _T("status-priority")) << XCHILD(_T("value"), szPriority);

		// status message text
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Status message"))
			<< XATTR(_T("type"), _T("text-multi")) << XATTR(_T("var"), _T("status-message"));

		// global status
		fieldNode = xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Change global status"))
			<< XATTR(_T("type"), _T("boolean")) << XATTR(_T("var"), _T("status-global"));

		ptrT tszStatusMsg((TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGT, status, 0));
		if (tszStatusMsg)
			fieldNode << XCHILD(_T("value"), tszStatusMsg);

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}
	
	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = xmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		HXML fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("status")), valueNode;
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		LPCTSTR ptszValue = xmlGetText( xmlGetChild(fieldNode , "value"));
		if (ptszValue == NULL)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		int status;
		if (!_tcscmp(ptszValue, _T("away"))) status = ID_STATUS_AWAY;
		else if (!_tcscmp(ptszValue, _T("xa"))) status = ID_STATUS_NA;
		else if (!_tcscmp(ptszValue, _T("dnd"))) status = ID_STATUS_DND;
		else if (!_tcscmp(ptszValue, _T("chat"))) status = ID_STATUS_FREECHAT;
		else if (!_tcscmp(ptszValue, _T("online"))) status = ID_STATUS_ONLINE;
		else if (!_tcscmp(ptszValue, _T("invisible"))) status = ID_STATUS_INVISIBLE;
		else if (!_tcscmp(ptszValue, _T("offline"))) status = ID_STATUS_OFFLINE;
		else
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		int priority = -9999;

		fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("status-priority"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (ptszValue = xmlGetText(valueNode))
				priority = _ttoi(ptszValue);

		if (priority >= -128 && priority <= 127)
			setWord("Priority", (WORD)priority);

		const TCHAR *szStatusMessage = NULL;
		fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("status-message"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			szStatusMessage = xmlGetText(valueNode);

		// skip f...ng away dialog
		int nNoDlg = db_get_b(NULL, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 0);
		db_set_b(NULL, "SRAway", StatusModeToDbSetting(status, "NoDlg"), 1);

		db_set_ts(NULL, "SRAway", StatusModeToDbSetting(status, "Msg"), szStatusMessage ? szStatusMessage : _T(""));

		fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("status-global"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value"))) {
			if ((ptszValue = xmlGetText(valueNode)) != NULL && _ttoi(ptszValue))
				CallService(MS_CLIST_SETSTATUSMODE, status, NULL);
			else
				CallProtoService(m_szModuleName, PS_SETSTATUS, status, NULL);
		}
		SetAwayMsg(status, szStatusMessage);

		// return NoDlg setting
		db_set_b(NULL, "SRAway", StatusModeToDbSetting(status, "NoDlg"), (BYTE)nNoDlg);

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocOptionsHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	if (pSession->GetStage() == 0) {
		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(_T("result"), pInfo);
		HXML xNode = iq
			<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_SET_OPTIONS)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("executing"))
			<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"));

		xNode << XCHILD(_T("title"), TranslateT("Set Options"));
		xNode << XCHILD(_T("instructions"), TranslateT("Set the desired options"));

		xNode << XCHILD(_T("field")) << XATTR(_T("type"), _T("hidden")) << XATTR(_T("var"), _T("FORM_TYPE"))
			<< XATTR(_T("value"), JABBER_FEAT_RC);

		// Automatically Accept File Transfers
		TCHAR szTmpBuff[ 1024 ];
		mir_sntprintf(szTmpBuff, SIZEOF(szTmpBuff), _T("%d"), db_get_b(NULL, "SRFile", "AutoAccept", 0));
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Automatically Accept File Transfers"))
			<< XATTR(_T("type"), _T("boolean")) << XATTR(_T("var"), _T("auto-files")) << XCHILD(_T("value"), szTmpBuff);

		// Use sounds
		mir_sntprintf(szTmpBuff, SIZEOF(szTmpBuff), _T("%d"), db_get_b(NULL, "Skin", "UseSound", 0));
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Play sounds"))
			<< XATTR(_T("type"), _T("boolean")) << XATTR(_T("var"), _T("sounds")) << XCHILD(_T("value"), szTmpBuff);

		// Disable remote controlling
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Disable remote controlling (check twice what you are doing)"))
			<< XATTR(_T("type"), _T("boolean")) << XATTR(_T("var"), _T("enable-rc")) << XCHILD(_T("value"), _T("0"));

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = xmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Automatically Accept File Transfers
		HXML fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("auto-files")), valueNode;
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (xmlGetText(valueNode))
				db_set_b(NULL, "SRFile", "AutoAccept", (BYTE)_ttoi(xmlGetText(valueNode)));

		// Use sounds
		fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("sounds"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (xmlGetText(valueNode))
				db_set_b(NULL, "Skin", "UseSound", (BYTE)_ttoi(xmlGetText(valueNode)));

		// Disable remote controlling
		fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("enable-rc"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (xmlGetText(valueNode) && _ttoi(xmlGetText(valueNode)))
				m_options.EnableRemoteControl = 0;

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::RcGetUnreadEventsCount()
{
	int nEventsSent = 0;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrT jid( getTStringA(hContact, "jid"));
		if (jid == NULL) continue;

		for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(hDbEvent);
			if (dbei.cbBlob == -1)
				continue;

			dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob + 1);
			int nGetTextResult = db_event_get(hDbEvent, &dbei);
			if (!nGetTextResult && dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT)) {
				TCHAR *szEventText = DbGetEventTextT(&dbei, CP_ACP);
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
	TCHAR szMsg[ 1024 ];
	if (pSession->GetStage() == 0) {
		int nUnreadEvents = RcGetUnreadEventsCount();
		if (!nUnreadEvents) {
			mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("There is no messages to forward"));

			m_ThreadInfo->send(
				XmlNodeIq(_T("result"), pInfo)
					<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_FORWARD)
						<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("completed"))
					<< XCHILD(_T("note"), szMsg) << XATTR(_T("type"), _T("info")));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		// first form
		pSession->SetStage(1);

		XmlNodeIq iq(_T("result"), pInfo);
		HXML xNode = iq
			<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_FORWARD)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("executing"))
			<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"));

		xNode << XCHILD(_T("title"), TranslateT("Forward options"));

		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%d message(s) to be forwarded"), nUnreadEvents);
		xNode << XCHILD(_T("instructions"), szMsg);

		xNode << XCHILD(_T("field")) << XATTR(_T("type"), _T("hidden")) << XATTR(_T("var"), _T("FORM_TYPE"))
			<< XCHILD(_T("value"), JABBER_FEAT_RC);

		// remove clist events
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), TranslateT("Mark messages as read")) << XATTR(_T("type"), _T("boolean"))
			<< XATTR(_T("var"), _T("remove-clist-events")) << XCHILD(_T("value"),
			m_options.RcMarkMessagesAsRead == 1 ? _T("1") : _T("0"));

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = xmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		BOOL bRemoveCListEvents = TRUE;

		// remove clist events
		HXML fieldNode = xmlGetChildByTag(xNode,"field", "var", _T("remove-clist-events")), valueNode;
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (xmlGetText(valueNode) && !_ttoi(xmlGetText(valueNode)))
				bRemoveCListEvents = FALSE;

		m_options.RcMarkMessagesAsRead = bRemoveCListEvents ? 1 : 0;

		int nEventsSent = 0;
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			ptrT tszJid( getTStringA(hContact, "jid"));
			if (tszJid == NULL)
				continue;
				
			for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.cbBlob = db_event_getBlobSize(hDbEvent);
				if (dbei.cbBlob == -1)
					continue;

				mir_ptr<BYTE> pEventBuf((PBYTE)mir_alloc(dbei.cbBlob + 1));
				dbei.pBlob = pEventBuf;
				if (db_event_get(hDbEvent, &dbei))
					continue;

				if (dbei.eventType != EVENTTYPE_MESSAGE || (dbei.flags & (DBEF_READ | DBEF_SENT)))
					continue;

				ptrT szEventText( DbGetEventTextT(&dbei, CP_ACP));
				if (szEventText == NULL)
					continue;

				XmlNode msg(_T("message"));
				msg << XATTR(_T("to"), pInfo->GetFrom()) << XATTRID(SerialNext())
					<< XCHILD(_T("body"), szEventText);

				HXML addressesNode = msg << XCHILDNS(_T("addresses"), JABBER_FEAT_EXT_ADDRESSING);
				TCHAR szOFrom[JABBER_MAX_JID_LEN];

				size_t cbBlob = strlen((LPSTR)dbei.pBlob)+1;
				if (cbBlob < dbei.cbBlob) { // rest of message contains a sender's resource
					ptrT szOResource( mir_utf8decodeT((LPSTR)dbei.pBlob + cbBlob+1));
					mir_sntprintf(szOFrom, SIZEOF(szOFrom), _T("%s/%s"), tszJid, szOResource);
				} else
					_tcsncpy_s(szOFrom, tszJid, _TRUNCATE);

				addressesNode << XCHILD(_T("address")) << XATTR(_T("type"), _T("ofrom")) << XATTR(_T("jid"), szOFrom);
				addressesNode << XCHILD(_T("address")) << XATTR(_T("type"), _T("oto")) << XATTR(_T("jid"), m_ThreadInfo->fullJID);

				time_t ltime = (time_t)dbei.timestamp;
				struct tm *gmt = gmtime(&ltime);
				TCHAR stime[512];
				mir_sntprintf(stime, SIZEOF(stime), _T("%.4i-%.2i-%.2iT%.2i:%.2i:%.2iZ"), gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
					gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
				msg << XCHILDNS(_T("delay"), _T("urn:xmpp:delay")) << XATTR(_T("stamp"), stime);

				m_ThreadInfo->send(msg);

				nEventsSent++;

				db_event_markRead(hContact, hDbEvent);
				if (bRemoveCListEvents)
					CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)hDbEvent);
			}
		}

		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("%d message(s) forwarded"), nEventsSent);

		m_ThreadInfo->send(
			XmlNodeIq(_T("result"), pInfo)
				<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_FORWARD)
					<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("completed"))
				<< XCHILD(_T("note"), szMsg) << XATTR(_T("type"), _T("info")));

		return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
	}

	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}

int CJabberProto::AdhocLockWSHandler(HXML, CJabberIqInfo *pInfo, CJabberAdhocSession* pSession)
{
	BOOL bOk = LockWorkStation();

	TCHAR szMsg[ 1024 ];
	if (bOk)
		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("Workstation successfully locked"));
	else
		mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("Error %d occurred during workstation lock"), GetLastError());

	m_ThreadInfo->send(
		XmlNodeIq(_T("result"), pInfo)
			<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_WS_LOCK)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("completed"))
			<< XCHILD(_T("note"), szMsg) << XATTR(_T("type"), bOk ? _T("info") : _T("error")));

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

		XmlNodeIq iq(_T("result"), pInfo);
		HXML xNode = iq
			<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_QUIT_MIRANDA)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("executing"))
			<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"));

		xNode << XCHILD(_T("title"), TranslateT("Confirmation needed"));
		xNode << XCHILD(_T("instructions"), TranslateT("Please confirm Miranda NG shutdown"));

		xNode << XCHILD(_T("field")) << XATTR(_T("type"), _T("hidden")) << XATTR(_T("var"), _T("FORM_TYPE"))
			<< XCHILD(_T("value"), JABBER_FEAT_RC);

		// I Agree checkbox
		xNode << XCHILD(_T("field")) << XATTR(_T("label"), _T("I agree")) << XATTR(_T("type"), _T("boolean"))
			<< XATTR(_T("var"), _T("allow-shutdown")) << XCHILD(_T("value"), _T("0"));

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = xmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		HXML fieldNode, valueNode;

		// I Agree checkbox
		fieldNode = xmlGetChildByTag(xNode,"field", "var", _T("allow-shutdown"));
		if (fieldNode && (valueNode = xmlGetChild(fieldNode , "value")))
			if (xmlGetText(valueNode) && _ttoi(xmlGetText(valueNode)))
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
				if (item != NULL)
					nChatsCount++;
			}
		}

		if (!nChatsCount) {
			TCHAR szMsg[ 1024 ];
			mir_sntprintf(szMsg, SIZEOF(szMsg), TranslateT("There is no group chats to leave"));

			m_ThreadInfo->send(
				XmlNodeIq(_T("result"), pInfo)
					<< XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_LEAVE_GROUPCHATS)
						<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("completed"))
					<< XCHILD(_T("note"), szMsg) << XATTR(_T("type"), _T("info")));

			return JABBER_ADHOC_HANDLER_STATUS_REMOVE_SESSION;
		}

		pSession->SetStage(1);

		XmlNodeIq iq(_T("result"), pInfo);
		HXML xNode = iq
			<<	XCHILDNS(_T("command"), JABBER_FEAT_COMMANDS) << XATTR(_T("node"), JABBER_FEAT_RC_LEAVE_GROUPCHATS)
				<< XATTR(_T("sessionid"), pSession->GetSessionId()) << XATTR(_T("status"), _T("executing"))
			<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"));

		xNode << XCHILD(_T("title"), TranslateT("Leave group chats"));
		xNode << XCHILD(_T("instructions"), TranslateT("Choose the group chats you want to leave"));

		xNode << XCHILD(_T("field")) << XATTR(_T("type"), _T("hidden")) << XATTR(_T("var"), _T("FORM_TYPE"))
			<< XATTR(_T("value"), JABBER_FEAT_RC);

		// Groupchats
		HXML fieldNode = xNode << XCHILD(_T("field")) << XATTR(_T("label"), NULL) << XATTR(_T("type"), _T("list-multi")) << XATTR(_T("var"), _T("groupchats"));
		fieldNode << XCHILD(_T("required"));
		{
			mir_cslock lck(m_csLists);
			LISTFOREACH_NODEF(i, this, LIST_CHATROOM)
			{
				JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
				if (item != NULL)
					fieldNode << XCHILD(_T("option")) << XATTR(_T("label"), item->jid) << XCHILD(_T("value"), item->jid);
			}
		}

		m_ThreadInfo->send(iq);
		return JABBER_ADHOC_HANDLER_STATUS_EXECUTING;
	}

	if (pSession->GetStage() == 1) {
		// result form here
		HXML commandNode = pInfo->GetChildNode();
		HXML xNode = xmlGetChildByTag(commandNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
		if (!xNode)
			return JABBER_ADHOC_HANDLER_STATUS_CANCEL;

		// Groupchat list here:
		HXML fieldNode = xmlGetChildByTag(xNode,"field", "var", _T("groupchats"));
		if (fieldNode) {
			for (i=0; i < xmlGetChildCount(fieldNode); i++) {
				HXML valueNode = xmlGetChild(fieldNode, i);
				if (valueNode && xmlGetName(valueNode) && xmlGetText(valueNode) && !_tcscmp(xmlGetName(valueNode), _T("value"))) {
					JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, xmlGetText(valueNode));
					if (item)
						GcQuit(item, 0, NULL);
				}
			}
		}

		return JABBER_ADHOC_HANDLER_STATUS_COMPLETED;
	}
	return JABBER_ADHOC_HANDLER_STATUS_CANCEL;
}