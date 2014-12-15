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

XML_API xi = { 0 };

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
	for (int i = 0; i < xi.getChildCount(node); i++) {
		LPCTSTR sTid = xi.getAttrValue(xi.getChild(node, i), ATTRNAME_TID);
		ULONGLONG tid = _tcstoui64(sTid, NULL, 10);
		if (tid > maxTid) {
			maxTid = tid;
			sMaxTid = sTid;
		}

		HXML senders = xi.getChildByPath(xi.getChild(node, i), NODENAME_SENDERS, FALSE);
		for (int j = 0; j < xi.getChildCount(senders); j++)
			if (xi.getAttrValue(xi.getChild(senders, j), ATTRNAME_UNREAD)) {
				unreadCount++;
				break;
			}
	}

	LPCSTR acc = GetJidAcc(jid);
	if (!acc)
		return FALSE;

	if (!unreadCount) {
		SetupPseudocontact(jid, xi.getAttrValue(node, ATTRNAME_TOTAL_MATCHED), acc);
		return TRUE;
	}

	DWORD settings = ReadNotificationSettings(acc);

	if (unreadCount > 5) {
		CloseNotifications(acc, mailboxUrl, jid, FALSE);
		UnreadMailNotification(acc, jid, mailboxUrl, xi.getAttrValue(node, ATTRNAME_TOTAL_MATCHED));
	}
	else
		for (int i = 0; i < xi.getChildCount(node); i++) {
			MAIL_THREAD_NOTIFICATION mtn = { 0 };
			HXML thread = xi.getChild(node, i);

			mtn.subj = xi.getText(xi.getChildByPath(thread, NODENAME_SUBJECT, FALSE));
			mtn.snip = xi.getText(xi.getChildByPath(thread, NODENAME_SNIPPET, FALSE));

			int threadUnreadCount = 0;
			HXML senders = xi.getChildByPath(thread, NODENAME_SENDERS, FALSE);
			for (int j = 0; threadUnreadCount < SENDER_COUNT && j < xi.getChildCount(senders); j++) {
				HXML sender = xi.getChild(senders, j);
				if (xi.getAttrValue(sender, ATTRNAME_UNREAD)) {
					mtn.senders[threadUnreadCount].name = xi.getAttrValue(sender, ATTRNAME_NAME);
					mtn.senders[threadUnreadCount].addr = xi.getAttrValue(sender, ATTRNAME_ADDRESS);
					threadUnreadCount++;
				}
			}

			LPCTSTR url = xi.getAttrValue(thread, ATTRNAME_URL);
			LPCTSTR tid = xi.getAttrValue(thread, ATTRNAME_TID);

			if (ReadCheckbox(0, IDC_STANDARDVIEW, settings))
				FormatMessageUrl(MESSAGE_URL_FORMAT_STANDARD, (LPTSTR)url, mailboxUrl, tid);
			else if (ReadCheckbox(0, IDC_HTMLVIEW, settings))
				FormatMessageUrl(MESSAGE_URL_FORMAT_HTML, (LPTSTR)url, mailboxUrl, tid);
			else
				MakeUrlHex((LPTSTR)url, tid);

			CloseNotifications(acc, url, jid, i);
			UnreadThreadNotification(acc, jid, url, xi.getAttrValue(node, ATTRNAME_TOTAL_MATCHED), &mtn);
		}

	LPCTSTR time = xi.getAttrValue(node, ATTRNAME_RESULT_TIME);
	WriteJidSetting(LAST_MAIL_TIME_FROM_JID, jid, time);
	WriteJidSetting(LAST_THREAD_ID_FROM_JID, jid, sMaxTid);
	return TRUE;
}

BOOL MailListHandler(IJabberInterface *ji, HXML node, void *)
{
	LPCTSTR jidWithRes = xi.getAttrValue(node, ATTRNAME_TO);
	__try {
		if (!node || mir_tstrcmp(xi.getAttrValue(node, ATTRNAME_TYPE), IQTYPE_RESULT)) return TRUE;

		LPCTSTR jid = xi.getAttrValue(node, ATTRNAME_FROM);
		assert(jid);

		node = xi.getChildByPath(node, NODENAME_MAILBOX, FALSE);
		if (!node) return TRUE; // empty list

		LPCTSTR url = xi.getAttrValue(node, ATTRNAME_URL);

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
	HXML node = xi.createNode(NODENAME_IQ, NULL, FALSE);
	xi.addAttr(node, ATTRNAME_TYPE, IQTYPE_GET);
	xi.addAttr(node, ATTRNAME_FROM, jidWithRes);

	UINT uID = ji->SerialNext();
	ptrT jid(ExtractJid(jidWithRes));
	xi.addAttr(node, ATTRNAME_TO, jid);

	ptrT
		lastMailTime(ReadJidSetting(LAST_MAIL_TIME_FROM_JID, jid)),
		lastThreadId(ReadJidSetting(LAST_THREAD_ID_FROM_JID, jid));

	TCHAR id[30];
	mir_sntprintf(id, SIZEOF(id), JABBER_IQID_FORMAT, uID);
	xi.addAttr(node, ATTRNAME_ID, id);

	child = xi.addChild(node, NODENAME_QUERY, NULL);
	xi.addAttr(child, ATTRNAME_XMLNS, NOTIFY_FEATURE_XMLNS);
	xi.addAttr(child, ATTRNAME_NEWER_THAN_TIME, lastMailTime);
	xi.addAttr(child, ATTRNAME_NEWER_THAN_TID, lastThreadId);

	if (ji->SendXmlNode(node))
		ji->AddTemporaryIqHandler(MailListHandler, JABBER_IQ_TYPE_RESULT, (int)uID, NULL, RESPONSE_TIMEOUT);

	if (child) xi.destroyNode(child);
	if (node) xi.destroyNode(node);
}

BOOL TimerHandler(IJabberInterface *ji, HXML, void *pUserData)
{
	RequestMail((LPCTSTR)pUserData, ji);
	free(pUserData);
	return FALSE;
}

BOOL NewMailHandler(IJabberInterface *ji, HXML node, void *)
{
	HXML response = xi.createNode(NODENAME_IQ, NULL, FALSE);
	__try {
		xi.addAttr(response, ATTRNAME_TYPE, IQTYPE_RESULT);

		LPCTSTR attr = xi.getAttrValue(node, ATTRNAME_ID);
		if (!attr) return FALSE;
		xi.addAttr(response, ATTRNAME_ID, attr);

		attr = xi.getAttrValue(node, ATTRNAME_FROM);
		if (attr) xi.addAttr(response, ATTRNAME_TO, attr);

		attr = xi.getAttrValue(node, ATTRNAME_TO);
		if (!attr) return FALSE;
		xi.addAttr(response, ATTRNAME_FROM, attr);

		int bytesSent = ji->SendXmlNode(response);
		RequestMail(attr, ji);
		return bytesSent > 0;
	}
	__finally {
		xi.destroyNode(response);
	}
}

void SetNotificationSetting(LPCTSTR jidWithResource, IJabberInterface *ji)
{
	HXML child = NULL;
	HXML node = xi.createNode(NODENAME_IQ, NULL, FALSE);

	xi.addAttr(node, ATTRNAME_TYPE, IQTYPE_SET);
	xi.addAttr(node, ATTRNAME_FROM, jidWithResource);

	ptrT jid(ExtractJid(jidWithResource));
	xi.addAttr(node, ATTRNAME_TO, jid);

	TCHAR id[30];
	mir_sntprintf(id, SIZEOF(id), JABBER_IQID_FORMAT, ji->SerialNext());
	xi.addAttr(node, ATTRNAME_ID, id);

	child = xi.addChild(node, NODENAME_USERSETTING, NULL);
	xi.addAttr(child, ATTRNAME_XMLNS, SETTING_FEATURE_XMLNS);

	child = xi.addChild(child, NODENAME_MAILNOTIFICATIONS, NULL);
	xi.addAttr(child, ATTRNAME_VALUE, SETTING_TRUE);

	ji->SendXmlNode(node);

	if (child) xi.destroyNode(child);
	if (node) xi.destroyNode(node);
}

BOOL DiscoverHandler(IJabberInterface *ji, HXML node, void *)
{
	if (!node) return FALSE;

	LPCTSTR jid = xi.getAttrValue(node, ATTRNAME_TO);
	assert(jid);
	node = xi.getChildByAttrValue(node, NODENAME_QUERY, ATTRNAME_XMLNS, DISCOVERY_XMLNS);

	HXML child = xi.getChildByAttrValue(node, NODENAME_FEATURE, ATTRNAME_VAR, SETTING_FEATURE_XMLNS);
	if (child)
		SetNotificationSetting(jid, ji);

	child = xi.getChildByAttrValue(node, NODENAME_FEATURE, ATTRNAME_VAR, NOTIFY_FEATURE_XMLNS);
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

	HXML queryNode = xi.getChildByAttrValue(node, NODENAME_QUERY, ATTRNAME_XMLNS, DISCOVERY_XMLNS);
	if (queryNode) {
		LPCTSTR ptszId = xi.getAttrValue(node, ATTRNAME_ID);
		if (ptszId)
			ji->AddTemporaryIqHandler(DiscoverHandler, JABBER_IQ_TYPE_RESULT, _ttoi(ptszId + 4), NULL, RESPONSE_TIMEOUT, 500);
	}

	if (!mir_tstrcmp(xi.getName(node), _T("presence")) && xi.getAttrValue(node, ATTRNAME_TO) == 0) {
		if (!gta->m_bGoogleSharedStatus)
			return FALSE;

		HXML statNode = xi.getChildByPath(node, _T("status"), 0);
		HXML showNode = xi.getChildByPath(node, _T("show"), 0);
		if (statNode) {
			LPCTSTR status = xi.getText(showNode);
			LPCTSTR msg = xi.getText(statNode);
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
		gta->m_bGoogleSharedStatus = mir_tstrcmp(xi.getAttrValue(node, ATTRNAME_TYPE), IQTYPE_RESULT) == 0;
		gta->m_bGoogleSharedStatusLock = FALSE;
	}
	return FALSE;
}

BOOL OnIqSetGoogleSharedStatus(IJabberInterface *ji, HXML iqNode, void *)
{
	GoogleTalkAcc *gta = isGoogle(LPARAM(ji));
	if (gta == NULL)
		return FALSE;
	if (mir_tstrcmp(xi.getAttrValue(iqNode, ATTRNAME_TYPE), IQTYPE_SET))
		return FALSE;
	if (gta->m_bGoogleSharedStatusLock)
		return TRUE;

	int status;
	HXML query = xi.getChildByPath(iqNode, NODENAME_QUERY, 0);
	HXML node = xi.getChildByPath(query, _T("invisible"), 0);
	if (0 == _tcsicmp(_T("true"), xi.getAttrValue(node, _T("value"))))
		status = ID_STATUS_INVISIBLE;
	else {
		LPCTSTR txt = xi.getText(xi.getChildByPath(query, _T("show"), 0));
		if (txt && 0 == _tcsicmp(_T("dnd"), txt))
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
	HXML iq = xi.createNode(NODENAME_IQ, NULL, FALSE);
	xi.addAttr(iq, ATTRNAME_TYPE, IQTYPE_GET);

	HXML query = xi.addChild(iq, NODENAME_QUERY, NULL);
	xi.addChild(query, ATTRNAME_XMLNS, JABBER_FEAT_GTALK_SHARED_STATUS);
	xi.addAttrInt(query, _T("version"), 2);

	xi.addChild(query, _T("status"), msg);
	if (!mir_tstrcmp(status, _T("invisible"))) {
		xi.addChild(query, _T("show"), _T("default"));
		xi.addAttr(xi.addChild(query, _T("invisible"), 0), _T("value"), _T("true"));
	}
	else {
		if (!mir_tstrcmp(status, _T("dnd")))
			xi.addChild(query, _T("show"), _T("dnd"));
		else
			xi.addChild(query, _T("show"), _T("default"));

		xi.addAttr(xi.addChild(query, _T("invisible"), 0), _T("value"), _T("false"));
	}
	m_bGoogleSharedStatusLock = TRUE;
	m_japi->SendXmlNode(iq);
	xi.destroyNode(iq);
}

int OnServerDiscoInfo(WPARAM wParam, LPARAM lParam)
{
	GoogleTalkAcc *gta = isGoogle(lParam);
	if (gta == NULL)
		return FALSE;

	// m_ThreadInfo->jabberServerCaps |= JABBER_CAPS_PING;

	JABBER_DISCO_FIELD *fld = (JABBER_DISCO_FIELD*)wParam;
	if (!mir_tstrcmp(fld->category, _T("server")) && !mir_tstrcmp(fld->type, _T("im")) && !mir_tstrcmp(fld->name, _T("Google Talk"))) {
		HXML iq = xi.createNode(NODENAME_IQ, NULL, FALSE);
		xi.addAttr(iq, ATTRNAME_TYPE, IQTYPE_GET);

		HXML query = xi.addChild(iq, NODENAME_QUERY, NULL);
		xi.addChild(query, ATTRNAME_XMLNS, JABBER_FEAT_GTALK_SHARED_STATUS);
		xi.addAttrInt(query, _T("version"), 2);
		gta->m_japi->SendXmlNode(iq);
		xi.destroyNode(iq);
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
	if (host == NULL || strcmp(host, "talk.google.com"))
		return;

	GoogleTalkAcc *gta = new GoogleTalkAcc();
	gta->m_japi = japi;
	gta->m_pa = ProtoGetAccount(szModuleName);
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
	ProtoEnumAccounts(&count, &protos);
	for (int i = 0; i < count; i++)
		sttCreateInstance(protos[i]->szModuleName);

	HookEvent(ME_JABBER_MENUINIT, InitMenus);
	HookEvent(ME_POPUP_FILTER, OnFilterPopup);
	HookEvent(ME_JABBER_EXTLISTINIT, OnExtListInit);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialization);
	return 0;
}
