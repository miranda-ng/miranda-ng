//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "StdAfx.h"
#include "handlers.h"
#include "db.h"
#include "inbox.h"
#include "notifications.h"
#include "options.h"

#define JABBER_IQID  _T("mir_")
#define JABBER_IQID_FORMAT  _T("mir_%d")

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <tchar.h>

GoogleTalkAcc* isGoogle(LPARAM lParam)
{
	return g_accs.find((GoogleTalkAcc*)&lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void FormatMessageUrl(LPCTSTR format, LPTSTR buf, LPCTSTR mailbox, LPCTSTR tid)
{
	ULARGE_INTEGER iTid; iTid.QuadPart = _tstoi64(tid);
	size_t l = mir_tstrlen(buf);
	mir_sntprintf(buf, l, format, mailbox, iTid.HighPart, iTid.LowPart);
	assert(l >= mir_tstrlen(buf));
}

void MakeUrlHex(LPTSTR url, LPCTSTR tid)
{
	ULARGE_INTEGER iTid; iTid.QuadPart = _tstoi64(tid);
	LPTSTR tidInUrl = _tcsstr(url, tid);
	LPTSTR trail = tidInUrl + mir_tstrlen(tid);
	wsprintf(tidInUrl, _T("%x%08x"), iTid.HighPart, iTid.LowPart); //!!!!!!!!!!!!
	wmemmove(tidInUrl + mir_tstrlen(tidInUrl), trail, mir_tstrlen(trail) + 1);
}

LPTSTR ExtractJid(LPCTSTR jidWithRes)
{
	LPCTSTR p = _tcsrchr(jidWithRes, '/');
	if (p == NULL)
		return mir_tstrdup(jidWithRes);

	size_t l = size_t(p - jidWithRes);
	LPTSTR result = (LPTSTR)mir_alloc((l + 1) * sizeof(TCHAR));
	_tcsncpy(result, jidWithRes, l);
	result[l] = 0;
	return result;
}

BOOL TimerHandler(IJabberInterface *ji, HXML node, void *pUserData);

BOOL InternalListHandler(HXML node, LPCTSTR jid, LPCTSTR mailboxUrl)
{
	ULONGLONG maxTid = 0;
	LPCTSTR sMaxTid = NULL;
	int unreadCount = 0;
	for (int i = 0; i < xmlGetChildCount(node); i++) {
		LPCTSTR sTid = xmlGetAttrValue(xmlGetChild(node, i), ATTRNAME_TID);
		ULONGLONG tid = _tcstoui64(sTid, NULL, 10);
		if (tid > maxTid) {
			maxTid = tid;
			sMaxTid = sTid;
		}

		HXML senders = xmlGetChildByPath(xmlGetChild(node, i), NODENAME_SENDERS, FALSE);
		for (int j = 0; j < xmlGetChildCount(senders); j++)
			if (xmlGetAttrValue(xmlGetChild(senders, j), ATTRNAME_UNREAD)) {
				unreadCount++;
				break;
			}
	}

	LPCSTR acc = GetJidAcc(jid);
	if (!acc)
		return FALSE;

	if (!unreadCount) {
		SetupPseudocontact(jid, xmlGetAttrValue(node, ATTRNAME_TOTAL_MATCHED), acc);
		return TRUE;
	}

	DWORD settings = ReadNotificationSettings(acc);

	if (unreadCount > 5) {
		CloseNotifications(acc, mailboxUrl, jid, FALSE);
		UnreadMailNotification(acc, jid, mailboxUrl, xmlGetAttrValue(node, ATTRNAME_TOTAL_MATCHED));
	}
	else
		for (int i = 0; i < xmlGetChildCount(node); i++) {
			MAIL_THREAD_NOTIFICATION mtn = { 0 };
			HXML thread = xmlGetChild(node, i);

			mtn.subj = xmlGetText(xmlGetChildByPath(thread, NODENAME_SUBJECT, FALSE));
			mtn.snip = xmlGetText(xmlGetChildByPath(thread, NODENAME_SNIPPET, FALSE));

			int threadUnreadCount = 0;
			HXML senders = xmlGetChildByPath(thread, NODENAME_SENDERS, FALSE);
			for (int j = 0; threadUnreadCount < SENDER_COUNT && j < xmlGetChildCount(senders); j++) {
				HXML sender = xmlGetChild(senders, j);
				if (xmlGetAttrValue(sender, ATTRNAME_UNREAD)) {
					mtn.senders[threadUnreadCount].name = xmlGetAttrValue(sender, ATTRNAME_NAME);
					mtn.senders[threadUnreadCount].addr = xmlGetAttrValue(sender, ATTRNAME_ADDRESS);
					threadUnreadCount++;
				}
			}

			LPCTSTR url = xmlGetAttrValue(thread, ATTRNAME_URL);
			LPCTSTR tid = xmlGetAttrValue(thread, ATTRNAME_TID);

			if (ReadCheckbox(0, IDC_STANDARDVIEW, settings))
				FormatMessageUrl(MESSAGE_URL_FORMAT_STANDARD, (LPTSTR)url, mailboxUrl, tid);
			else if (ReadCheckbox(0, IDC_HTMLVIEW, settings))
				FormatMessageUrl(MESSAGE_URL_FORMAT_HTML, (LPTSTR)url, mailboxUrl, tid);
			else
				MakeUrlHex((LPTSTR)url, tid);

			CloseNotifications(acc, url, jid, i);
			UnreadThreadNotification(acc, jid, url, xmlGetAttrValue(node, ATTRNAME_TOTAL_MATCHED), &mtn);
		}

	LPCTSTR time = xmlGetAttrValue(node, ATTRNAME_RESULT_TIME);
	WriteJidSetting(LAST_MAIL_TIME_FROM_JID, jid, time);
	WriteJidSetting(LAST_THREAD_ID_FROM_JID, jid, sMaxTid);
	return TRUE;
}

BOOL MailListHandler(IJabberInterface *ji, HXML node, void *)
{
	LPCTSTR jidWithRes = xmlGetAttrValue(node, ATTRNAME_TO);
	__try {
		if (!node || mir_tstrcmp(xmlGetAttrValue(node, ATTRNAME_TYPE), IQTYPE_RESULT)) return TRUE;

		LPCTSTR jid = xmlGetAttrValue(node, ATTRNAME_FROM);
		assert(jid);

		node = xmlGetChildByPath(node, NODENAME_MAILBOX, FALSE);
		if (!node) return TRUE; // empty list

		LPCTSTR url = xmlGetAttrValue(node, ATTRNAME_URL);

		return InternalListHandler(node, jid, url);
	}
	__finally {
		if (jidWithRes)
			ji->AddTemporaryIqHandler(TimerHandler, JABBER_IQ_TYPE_RESULT, 0,
			(PVOID)_tcsdup(jidWithRes), TIMER_INTERVAL);
		// Never get a real result stanza. Results elapsed request after WAIT_TIMER_INTERVAL ms
	}
}

void RequestMail(LPCTSTR jidWithRes, IJabberInterface *ji)
{
	HXML child = NULL;
	HXML node = xmlCreateNode(NODENAME_IQ, NULL, FALSE);
	xmlAddAttr(node, ATTRNAME_TYPE, IQTYPE_GET);
	xmlAddAttr(node, ATTRNAME_FROM, jidWithRes);

	UINT uID = ji->SerialNext();
	ptrT jid(ExtractJid(jidWithRes));
	xmlAddAttr(node, ATTRNAME_TO, jid);

	ptrT
		lastMailTime(ReadJidSetting(LAST_MAIL_TIME_FROM_JID, jid)),
		lastThreadId(ReadJidSetting(LAST_THREAD_ID_FROM_JID, jid));

	TCHAR id[30];
	mir_sntprintf(id, JABBER_IQID_FORMAT, uID);
	xmlAddAttr(node, ATTRNAME_ID, id);

	child = xmlAddChild(node, NODENAME_QUERY, NULL);
	xmlAddAttr(child, ATTRNAME_XMLNS, NOTIFY_FEATURE_XMLNS);
	xmlAddAttr(child, ATTRNAME_NEWER_THAN_TIME, lastMailTime);
	xmlAddAttr(child, ATTRNAME_NEWER_THAN_TID, lastThreadId);

	if (ji->SendXmlNode(node))
		ji->AddTemporaryIqHandler(MailListHandler, JABBER_IQ_TYPE_RESULT, (int)uID, NULL, RESPONSE_TIMEOUT);

	if (child) xmlDestroyNode(child);
	if (node) xmlDestroyNode(node);
}

BOOL TimerHandler(IJabberInterface *ji, HXML, void *pUserData)
{
	RequestMail((LPCTSTR)pUserData, ji);
	free(pUserData);
	return FALSE;
}

BOOL NewMailHandler(IJabberInterface *ji, HXML node, void *)
{
	HXML response = xmlCreateNode(NODENAME_IQ, NULL, FALSE);
	__try {
		xmlAddAttr(response, ATTRNAME_TYPE, IQTYPE_RESULT);

		LPCTSTR attr = xmlGetAttrValue(node, ATTRNAME_ID);
		if (!attr) return FALSE;
		xmlAddAttr(response, ATTRNAME_ID, attr);

		attr = xmlGetAttrValue(node, ATTRNAME_FROM);
		if (attr) xmlAddAttr(response, ATTRNAME_TO, attr);

		attr = xmlGetAttrValue(node, ATTRNAME_TO);
		if (!attr) return FALSE;
		xmlAddAttr(response, ATTRNAME_FROM, attr);

		int bytesSent = ji->SendXmlNode(response);
		RequestMail(attr, ji);
		return bytesSent > 0;
	}
	__finally {
		xmlDestroyNode(response);
	}
}

void SetNotificationSetting(LPCTSTR jidWithResource, IJabberInterface *ji)
{
	HXML child = NULL;
	HXML node = xmlCreateNode(NODENAME_IQ, NULL, FALSE);

	xmlAddAttr(node, ATTRNAME_TYPE, IQTYPE_SET);
	xmlAddAttr(node, ATTRNAME_FROM, jidWithResource);

	ptrT jid(ExtractJid(jidWithResource));
	xmlAddAttr(node, ATTRNAME_TO, jid);

	TCHAR id[30];
	mir_sntprintf(id, JABBER_IQID_FORMAT, ji->SerialNext());
	xmlAddAttr(node, ATTRNAME_ID, id);

	child = xmlAddChild(node, NODENAME_USERSETTING, NULL);
	xmlAddAttr(child, ATTRNAME_XMLNS, SETTING_FEATURE_XMLNS);

	child = xmlAddChild(child, NODENAME_MAILNOTIFICATIONS, NULL);
	xmlAddAttr(child, ATTRNAME_VALUE, SETTING_TRUE);

	ji->SendXmlNode(node);

	if (child) xmlDestroyNode(child);
	if (node) xmlDestroyNode(node);
}

BOOL DiscoverHandler(IJabberInterface *ji, HXML node, void *)
{
	if (!node) return FALSE;

	LPCTSTR jid = xmlGetAttrValue(node, ATTRNAME_TO);
	assert(jid);
	node = xmlGetChildByAttrValue(node, NODENAME_QUERY, ATTRNAME_XMLNS, DISCOVERY_XMLNS);

	HXML child = xmlGetChildByAttrValue(node, NODENAME_FEATURE, ATTRNAME_VAR, SETTING_FEATURE_XMLNS);
	if (child)
		SetNotificationSetting(jid, ji);

	child = xmlGetChildByAttrValue(node, NODENAME_FEATURE, ATTRNAME_VAR, NOTIFY_FEATURE_XMLNS);
	if (child) {
		ji->AddIqHandler(NewMailHandler, JABBER_IQ_TYPE_SET, NOTIFY_FEATURE_XMLNS, NODENAME_NEW_MAIL);
		RequestMail(jid, ji);
	}

	return FALSE;
}

BOOL SendHandler(IJabberInterface *ji, HXML node, void *)
{
	GoogleTalkAcc *gta = isGoogle(LPARAM(ji));
	if (gta == NULL)
		return FALSE;

	HXML queryNode = xmlGetChildByAttrValue(node, NODENAME_QUERY, ATTRNAME_XMLNS, DISCOVERY_XMLNS);
	if (queryNode) {
		LPCTSTR ptszId = xmlGetAttrValue(node, ATTRNAME_ID);
		if (ptszId)
			ji->AddTemporaryIqHandler(DiscoverHandler, JABBER_IQ_TYPE_RESULT, _ttoi(ptszId + 4), NULL, RESPONSE_TIMEOUT, 500);
	}

	if (!mir_tstrcmp(xmlGetName(node), _T("presence")) && xmlGetAttrValue(node, ATTRNAME_TO) == 0) {
		if (!gta->m_bGoogleSharedStatus)
			return FALSE;

		HXML statNode = xmlGetChildByPath(node, _T("status"), 0);
		HXML showNode = xmlGetChildByPath(node, _T("show"), 0);
		if (statNode) {
			LPCTSTR status = xmlGetText(showNode);
			LPCTSTR msg = xmlGetText(statNode);
			gta->SendIqGoogleSharedStatus(status, msg);
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Google shared status

BOOL OnIqResultGoogleSharedStatus(IJabberInterface *ji, HXML node, void *)
{
	GoogleTalkAcc *gta = isGoogle(LPARAM(ji));
	if (gta != NULL) {
		gta->m_bGoogleSharedStatus = mir_tstrcmp(xmlGetAttrValue(node, ATTRNAME_TYPE), IQTYPE_RESULT) == 0;
		gta->m_bGoogleSharedStatusLock = FALSE;
	}
	return FALSE;
}

BOOL OnIqSetGoogleSharedStatus(IJabberInterface *ji, HXML iqNode, void *)
{
	GoogleTalkAcc *gta = isGoogle(LPARAM(ji));
	if (gta == NULL)
		return FALSE;
	if (mir_tstrcmp(xmlGetAttrValue(iqNode, ATTRNAME_TYPE), IQTYPE_SET))
		return FALSE;
	if (gta->m_bGoogleSharedStatusLock)
		return TRUE;

	int status;
	HXML query = xmlGetChildByPath(iqNode, NODENAME_QUERY, 0);
	HXML node = xmlGetChildByPath(query, _T("invisible"), 0);
	if (0 == mir_tstrcmpi(_T("true"), xmlGetAttrValue(node, _T("value"))))
		status = ID_STATUS_INVISIBLE;
	else {
		LPCTSTR txt = xmlGetText(xmlGetChildByPath(query, _T("show"), 0));
		if (txt && 0 == mir_tstrcmpi(_T("dnd"), txt))
			status = ID_STATUS_DND;
		else if (gta->m_pa->ppro->m_iStatus == ID_STATUS_DND || gta->m_pa->ppro->m_iStatus == ID_STATUS_INVISIBLE)
			status = ID_STATUS_ONLINE;
		else
			status = gta->m_pa->ppro->m_iStatus;
	}

	if (status != gta->m_pa->ppro->m_iStatus)
		CallProtoService(gta->m_pa->szModuleName, PS_SETSTATUS, status, 0);

	return TRUE;
}

void GoogleTalkAcc::SendIqGoogleSharedStatus(LPCTSTR status, LPCTSTR msg)
{
	HXML iq = xmlCreateNode(NODENAME_IQ, NULL, FALSE);
	xmlAddAttr(iq, ATTRNAME_TYPE, IQTYPE_GET);

	HXML query = xmlAddChild(iq, NODENAME_QUERY, NULL);
	xmlAddChild(query, ATTRNAME_XMLNS, JABBER_FEAT_GTALK_SHARED_STATUS);
	xmlAddAttrInt(query, _T("version"), 2);

	xmlAddChild(query, _T("status"), msg);
	if (!mir_tstrcmp(status, _T("invisible"))) {
		xmlAddChild(query, _T("show"), _T("default"));
		xmlAddAttr(xmlAddChild(query, _T("invisible"), 0), _T("value"), _T("true"));
	}
	else {
		if (!mir_tstrcmp(status, _T("dnd")))
			xmlAddChild(query, _T("show"), _T("dnd"));
		else
			xmlAddChild(query, _T("show"), _T("default"));

		xmlAddAttr(xmlAddChild(query, _T("invisible"), 0), _T("value"), _T("false"));
	}
	m_bGoogleSharedStatusLock = TRUE;
	m_japi->SendXmlNode(iq);
	xmlDestroyNode(iq);
}

int OnServerDiscoInfo(WPARAM wParam, LPARAM lParam)
{
	GoogleTalkAcc *gta = isGoogle(lParam);
	if (gta == NULL)
		return FALSE;

	// m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PING;

	JABBER_DISCO_FIELD *fld = (JABBER_DISCO_FIELD*)wParam;
	if (!mir_tstrcmp(fld->category, _T("server")) && !mir_tstrcmp(fld->type, _T("im")) && !mir_tstrcmp(fld->name, _T("Google Talk"))) {
		HXML iq = xmlCreateNode(NODENAME_IQ, NULL, FALSE);
		xmlAddAttr(iq, ATTRNAME_TYPE, IQTYPE_GET);

		HXML query = xmlAddChild(iq, NODENAME_QUERY, NULL);
		xmlAddChild(query, ATTRNAME_XMLNS, JABBER_FEAT_GTALK_SHARED_STATUS);
		xmlAddAttrInt(query, _T("version"), 2);
		gta->m_japi->SendXmlNode(iq);
		xmlDestroyNode(iq);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// adds Google extensions into the caps list

int OnExtListInit(WPARAM wParam, LPARAM lParam)
{
	GoogleTalkAcc *gta = isGoogle(lParam);
	if (gta != NULL) {
		LIST<TCHAR> *pList = (LIST<TCHAR>*)wParam;
		pList->insert(JABBER_EXT_GTALK_PMUC);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// for our pseudo contact only our own popups are allowed
// 0 = allowed, 1 = disallowed

int OnFilterPopup(WPARAM hContact, LPARAM lParam)
{
	if (!db_get_b(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0))
		return 0;

	return (lParam != (LPARAM)&PopupProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void sttCreateInstance(LPCSTR szModuleName)
{
	IJabberInterface *japi = getJabberApi(szModuleName);
	if (japi == NULL)
		return;

	ptrA host(db_get_sa(NULL, szModuleName, "ManualHost"));
	if (host == NULL || mir_strcmp(host, "talk.google.com"))
		return;

	GoogleTalkAcc *gta = new GoogleTalkAcc();
	gta->m_japi = japi;
	gta->m_pa = Proto_GetAccount(szModuleName);
	g_accs.insert(gta);

	// Google Shared Status (http://code.google.com/apis/talk/jep_extensions/shared_status.html)
	japi->AddIqHandler(::OnIqResultGoogleSharedStatus, JABBER_IQ_TYPE_SET, JABBER_FEAT_GTALK_SHARED_STATUS, NULL);
	japi->AddIqHandler(::OnIqResultGoogleSharedStatus, JABBER_IQ_TYPE_GET, JABBER_FEAT_GTALK_SHARED_STATUS, NODENAME_QUERY);

	japi->AddSendHandler(SendHandler);
}

int AccListChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == PRAC_ADDED)
		sttCreateInstance(((PROTOACCOUNT*)lParam)->szModuleName);
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	int count;
	PROTOACCOUNT **protos;
	Proto_EnumAccounts(&count, &protos);
	for (int i = 0; i < count; i++)
		sttCreateInstance(protos[i]->szModuleName);

	HookEvent(ME_JABBER_MENUINIT, InitMenus);
	HookEvent(ME_POPUP_FILTER, OnFilterPopup);
	HookEvent(ME_JABBER_EXTLISTINIT, OnExtListInit);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialization);
	return 0;
}
