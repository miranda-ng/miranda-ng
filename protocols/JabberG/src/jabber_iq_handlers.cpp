/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-15 Miranda NG project

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
#include "version.h"

BOOL CJabberProto::OnIqRequestVersion(HXML, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetFrom())
		return TRUE;

	if (!m_options.AllowVersionRequests)
		return FALSE;

	XmlNodeIq iq(_T("result"), pInfo);
	HXML query = iq << XQUERY(JABBER_FEAT_VERSION);
	query << XCHILD(_T("name"), _T("Miranda NG Jabber"));
	query << XCHILD(_T("version"), szCoreVersion);

	if (m_options.ShowOSVersion) {
		TCHAR os[256] = { 0 };
		if (!GetOSDisplayString(os, _countof(os)))
			mir_tstrncpy(os, _T("Microsoft Windows"), _countof(os));
		query << XCHILD(_T("os"), os);
	}

	m_ThreadInfo->send(iq);
	return TRUE;
}

// last activity (XEP-0012) support
BOOL CJabberProto::OnIqRequestLastActivity(HXML, CJabberIqInfo *pInfo)
{
	m_ThreadInfo->send(
		XmlNodeIq(_T("result"), pInfo) << XQUERY(JABBER_FEAT_LAST_ACTIVITY)
		<< XATTRI(_T("seconds"), m_tmJabberIdleStartTime ? time(0) - m_tmJabberIdleStartTime : 0));
	return TRUE;
}

// XEP-0199: XMPP Ping support
BOOL CJabberProto::OnIqRequestPing(HXML, CJabberIqInfo *pInfo)
{
	m_ThreadInfo->send(XmlNodeIq(_T("result"), pInfo) << XATTR(_T("from"), m_ThreadInfo->fullJID));
	return TRUE;
}

// Returns the current GMT offset in seconds
int GetGMTOffset(void)
{
	TIME_ZONE_INFORMATION tzinfo;
	int nOffset = 0;

	DWORD dwResult = GetTimeZoneInformation(&tzinfo);

	switch (dwResult) {
	case TIME_ZONE_ID_STANDARD:
		nOffset = tzinfo.Bias + tzinfo.StandardBias;
		break;
	case TIME_ZONE_ID_DAYLIGHT:
		nOffset = tzinfo.Bias + tzinfo.DaylightBias;
		break;
	case TIME_ZONE_ID_UNKNOWN:
		nOffset = tzinfo.Bias;
		break;
	case TIME_ZONE_ID_INVALID:
	default:
		nOffset = 0;
		break;
	}

	return -nOffset;
}

// entity time (XEP-0202) support
BOOL CJabberProto::OnIqRequestTime(HXML, CJabberIqInfo *pInfo)
{
	TCHAR stime[100];
	TCHAR szTZ[10];

	TimeZone_PrintDateTime(UTC_TIME_HANDLE, _T("I"), stime, _countof(stime), 0);

	int nGmtOffset = GetGMTOffset();
	mir_sntprintf(szTZ, _countof(szTZ), _T("%+03d:%02d"), nGmtOffset / 60, nGmtOffset % 60);

	XmlNodeIq iq(_T("result"), pInfo);
	HXML timeNode = iq << XCHILDNS(_T("time"), JABBER_FEAT_ENTITY_TIME);
	timeNode << XCHILD(_T("utc"), stime); timeNode << XCHILD(_T("tzo"), szTZ);
	LPCTSTR szTZName = TimeZone_GetName(NULL);
	if (szTZName)
		timeNode << XCHILD(_T("tz"), szTZName);
	m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberProto::OnIqProcessIqOldTime(HXML, CJabberIqInfo *pInfo)
{
	struct tm *gmt;
	time_t ltime;
	TCHAR stime[100], *dtime;

	_tzset();
	time(&ltime);
	gmt = gmtime(&ltime);
	mir_sntprintf(stime, _countof(stime), _T("%.4i%.2i%.2iT%.2i:%.2i:%.2i"),
		gmt->tm_year + 1900, gmt->tm_mon + 1,
		gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	dtime = _tctime(&ltime);
	dtime[24] = 0;

	XmlNodeIq iq(_T("result"), pInfo);
	HXML queryNode = iq << XQUERY(JABBER_FEAT_ENTITY_TIME_OLD);
	queryNode << XCHILD(_T("utc"), stime);
	LPCTSTR szTZName = TimeZone_GetName(NULL);
	if (szTZName)
		queryNode << XCHILD(_T("tz"), szTZName);
	queryNode << XCHILD(_T("display"), dtime);
	m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberProto::OnIqRequestAvatar(HXML, CJabberIqInfo *pInfo)
{
	if (!m_options.EnableAvatars)
		return TRUE;

	int pictureType = m_options.AvatarType;
	if (pictureType == PA_FORMAT_UNKNOWN)
		return TRUE;

	TCHAR *szMimeType;
	switch (pictureType) {
	case PA_FORMAT_JPEG:	 szMimeType = _T("image/jpeg");   break;
	case PA_FORMAT_GIF:	 szMimeType = _T("image/gif");    break;
	case PA_FORMAT_PNG:	 szMimeType = _T("image/png");    break;
	case PA_FORMAT_BMP:	 szMimeType = _T("image/bmp");    break;
	default:	return TRUE;
	}

	TCHAR szFileName[MAX_PATH];
	GetAvatarFileName(NULL, szFileName, _countof(szFileName));

	FILE* in = _tfopen(szFileName, _T("rb"));
	if (in == NULL)
		return TRUE;

	long bytes = _filelength(_fileno(in));
	ptrA buffer((char*)mir_alloc(bytes * 4 / 3 + bytes + 1000));
	if (buffer == NULL) {
		fclose(in);
		return TRUE;
	}

	fread(buffer, bytes, 1, in);
	fclose(in);

	ptrA str(mir_base64_encode((PBYTE)(char*)buffer, bytes));
	m_ThreadInfo->send(XmlNodeIq(_T("result"), pInfo) << XQUERY(JABBER_FEAT_AVATAR) << XCHILD(_T("query"), _A2T(str)) << XATTR(_T("mimetype"), szMimeType));
	return TRUE;
}

BOOL CJabberProto::OnSiRequest(HXML node, CJabberIqInfo *pInfo)
{
	const TCHAR *szProfile = XmlGetAttrValue(pInfo->GetChildNode(), _T("profile"));

	if (szProfile && !mir_tstrcmp(szProfile, JABBER_FEAT_SI_FT))
		FtHandleSiRequest(node);
	else {
		XmlNodeIq iq(_T("error"), pInfo);
		HXML error = iq << XCHILD(_T("error")) << XATTRI(_T("code"), 400) << XATTR(_T("type"), _T("cancel"));
		error << XCHILDNS(_T("bad-request"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"));
		error << XCHILD(_T("bad-profile"));
		m_ThreadInfo->send(iq);
	}
	return TRUE;
}

BOOL CJabberProto::OnRosterPushRequest(HXML, CJabberIqInfo *pInfo)
{
	HXML queryNode = pInfo->GetChildNode();

	// RFC 3921 #7.2 Business Rules
	if (pInfo->GetFrom()) {
		TCHAR *szFrom = JabberPrepareJid(pInfo->GetFrom());
		if (!szFrom)
			return TRUE;

		TCHAR *szTo = JabberPrepareJid(m_ThreadInfo->fullJID);
		if (!szTo) {
			mir_free(szFrom);
			return TRUE;
		}

		TCHAR *pDelimiter = _tcschr(szFrom, _T('/'));
		if (pDelimiter) *pDelimiter = 0;

		pDelimiter = _tcschr(szTo, _T('/'));
		if (pDelimiter) *pDelimiter = 0;

		BOOL bRetVal = mir_tstrcmp(szFrom, szTo) == 0;

		mir_free(szFrom);
		mir_free(szTo);

		// invalid JID
		if (!bRetVal) {
			debugLog(_T("<iq/> attempt to hack via roster push from %s"), pInfo->GetFrom());
			return TRUE;
		}
	}

	JABBER_LIST_ITEM *item;
	MCONTACT hContact = NULL;
	const TCHAR *jid, *str;

	debugLogA("<iq/> Got roster push, query has %d children", XmlGetChildCount(queryNode));
	for (int i = 0;; i++) {
		HXML itemNode = XmlGetChild(queryNode, i);
		if (!itemNode)
			break;

		if (mir_tstrcmp(XmlGetName(itemNode), _T("item")) != 0)
			continue;
		if ((jid = XmlGetAttrValue(itemNode, _T("jid"))) == NULL)
			continue;
		if ((str = XmlGetAttrValue(itemNode, _T("subscription"))) == NULL)
			continue;

		// we will not add new account when subscription=remove
		if (!mir_tstrcmp(str, _T("to")) || !mir_tstrcmp(str, _T("both")) || !mir_tstrcmp(str, _T("from")) || !mir_tstrcmp(str, _T("none"))) {
			const TCHAR *name = XmlGetAttrValue(itemNode, _T("name"));
			ptrT nick((name != NULL) ? mir_tstrdup(name) : JabberNickFromJID(jid));
			if (nick != NULL) {
				if ((item = ListAdd(LIST_ROSTER, jid)) != NULL) {
					replaceStrT(item->nick, nick);

					HXML groupNode = XmlGetChild(itemNode, "group");
					replaceStrT(item->group, XmlGetText(groupNode));

					if ((hContact = HContactFromJID(jid, 0)) == NULL) {
						// Received roster has a new JID.
						// Add the jid (with empty resource) to Miranda contact list.
						hContact = DBCreateContact(jid, nick, FALSE, FALSE);
					}
					else setTString(hContact, "jid", jid);

					if (name != NULL) {
						ptrT tszNick(getTStringA(hContact, "Nick"));
						if (tszNick != NULL) {
							if (mir_tstrcmp(nick, tszNick) != 0)
								db_set_ts(hContact, "CList", "MyHandle", nick);
							else
								db_unset(hContact, "CList", "MyHandle");
						}
						else db_set_ts(hContact, "CList", "MyHandle", nick);
					}
					else db_unset(hContact, "CList", "MyHandle");

					if (!m_options.IgnoreRosterGroups) {
						if (item->group != NULL) {
							Clist_CreateGroup(0, item->group);
							db_set_ts(hContact, "CList", "Group", item->group);
						}
						else db_unset(hContact, "CList", "Group");
					}
				}
			}
		}

		if ((item = ListGetItemPtr(LIST_ROSTER, jid)) != NULL) {
			if (!mir_tstrcmp(str, _T("both"))) item->subscription = SUB_BOTH;
			else if (!mir_tstrcmp(str, _T("to"))) item->subscription = SUB_TO;
			else if (!mir_tstrcmp(str, _T("from"))) item->subscription = SUB_FROM;
			else item->subscription = SUB_NONE;
			debugLog(_T("Roster push for jid=%s, set subscription to %s"), jid, str);
			// subscription = remove is to remove from roster list
			// but we will just set the contact to offline and not actually
			// remove, so that history will be retained.
			if (!mir_tstrcmp(str, _T("remove"))) {
				if ((hContact = HContactFromJID(jid)) != NULL) {
					SetContactOfflineStatus(hContact);
					ListRemove(LIST_ROSTER, jid);
				}
			}
			else if (isChatRoom(hContact))
				db_unset(hContact, "CList", "Hidden");
			else
				UpdateSubscriptionInfo(hContact, item);
		}
	}

	UI_SAFE_NOTIFY(m_pDlgServiceDiscovery, WM_JABBER_TRANSPORT_REFRESH);
	RebuildInfoFrame();
	return TRUE;
}

BOOL CJabberProto::OnIqRequestOOB(HXML, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetFrom() || !pInfo->GetHContact())
		return TRUE;

	HXML n = XmlGetChild(pInfo->GetChildNode(), "url");
	if (!n || !XmlGetText(n))
		return TRUE;

	if (m_options.BsOnlyIBB) {
		// reject
		XmlNodeIq iq(_T("error"), pInfo);
		HXML e = XmlAddChild(iq, _T("error"), _T("File transfer refused")); XmlAddAttr(e, _T("code"), 406);
		m_ThreadInfo->send(iq);
		return TRUE;
	}

	filetransfer *ft = new filetransfer(this);
	ft->std.totalFiles = 1;
	ft->jid = mir_tstrdup(pInfo->GetFrom());
	ft->std.hContact = pInfo->GetHContact();
	ft->type = FT_OOB;
	ft->httpHostName = NULL;
	ft->httpPort = 80;
	ft->httpPath = NULL;

	// Parse the URL
	TCHAR *str = (TCHAR*)XmlGetText(n);	// URL of the file to get
	if (!_tcsnicmp(str, _T("http://"), 7)) {
		TCHAR *p = str + 7, *q;
		if ((q = _tcschr(p, '/')) != NULL) {
			TCHAR text[1024];
			if (q - p < _countof(text)) {
				_tcsncpy_s(text, p, q - p);
				text[q - p] = '\0';
				if ((p = _tcschr(text, ':')) != NULL) {
					ft->httpPort = (WORD)_ttoi(p + 1);
					*p = '\0';
				}
				ft->httpHostName = mir_t2a(text);
			}
		}
	}

	if (pInfo->GetIdStr())
		ft->szId = JabberId2string(pInfo->GetIqId());

	if (ft->httpHostName && ft->httpPath) {
		TCHAR *desc = NULL;

		debugLogA("Host=%s Port=%d Path=%s", ft->httpHostName, ft->httpPort, ft->httpPath);
		if ((n = XmlGetChild(pInfo->GetChildNode(), "desc")) != NULL)
			desc = (TCHAR*)XmlGetText(n);

		TCHAR *str2;
		debugLog(_T("description = %s"), desc);
		if ((str2 = _tcsrchr(ft->httpPath, '/')) != NULL)
			str2++;
		else
			str2 = ft->httpPath;
		str2 = mir_tstrdup(str2);
		JabberHttpUrlDecode(str2);

		PROTORECVFILET pre;
		pre.dwFlags = PRFF_TCHAR;
		pre.timestamp = time(NULL);
		pre.descr.t = desc;
		pre.files.t = &str2;
		pre.fileCount = 1;
		pre.lParam = (LPARAM)ft;
		ProtoChainRecvFile(ft->std.hContact, &pre);
		mir_free(str2);
	}
	else {
		// reject
		XmlNodeIq iq(_T("error"), pInfo);
		HXML e = XmlAddChild(iq, _T("error"), _T("File transfer refused")); XmlAddAttr(e, _T("code"), 406);
		m_ThreadInfo->send(iq);
		delete ft;
	}
	return TRUE;
}

BOOL CJabberProto::OnHandleDiscoInfoRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return TRUE;

	const TCHAR *szNode = XmlGetAttrValue(pInfo->GetChildNode(), _T("node"));
	// caps hack
	if (m_clientCapsManager.HandleInfoRequest(iqNode, pInfo, szNode))
		return TRUE;

	// ad-hoc hack:
	if (szNode && m_adhocManager.HandleInfoRequest(iqNode, pInfo, szNode))
		return TRUE;

	// another request, send empty result
	m_ThreadInfo->send(
		XmlNodeIq(_T("error"), pInfo)
		<< XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
		<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
	return TRUE;
}

BOOL CJabberProto::OnHandleDiscoItemsRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return TRUE;

	// ad-hoc commands check:
	const TCHAR *szNode = XmlGetAttrValue(pInfo->GetChildNode(), _T("node"));
	if (szNode && m_adhocManager.HandleItemsRequest(iqNode, pInfo, szNode))
		return TRUE;

	// another request, send empty result
	XmlNodeIq iq(_T("result"), pInfo);
	HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
	if (szNode)
		XmlAddAttr(resultQuery, _T("node"), szNode);

	if (!szNode && m_options.EnableRemoteControl)
		resultQuery << XCHILD(_T("item")) << XATTR(_T("jid"), m_ThreadInfo->fullJID)
		<< XATTR(_T("node"), JABBER_FEAT_COMMANDS) << XATTR(_T("name"), _T("Ad-hoc commands"));

	m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberProto::AddClistHttpAuthEvent(CJabberHttpAuthParams *pParams)
{
	char szService[256];
	mir_snprintf(szService, "%s%s", m_szModuleName, JS_HTTP_AUTH);

	CLISTEVENT cle = { 0 };
	cle.cbSize = sizeof(CLISTEVENT);
	cle.hIcon = (HICON)LoadIconEx("openid");
	cle.flags = CLEF_PROTOCOLGLOBAL | CLEF_TCHAR;
	cle.hDbEvent = -99;
	cle.lParam = (LPARAM)pParams;
	cle.pszService = szService;
	cle.ptszTooltip = TranslateT("Http authentication request received");
	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	return TRUE;
}

BOOL CJabberProto::OnIqHttpAuth(HXML node, CJabberIqInfo *pInfo)
{
	if (!m_options.AcceptHttpAuth)
		return TRUE;

	if (!node || !pInfo->GetChildNode() || !pInfo->GetFrom() || !pInfo->GetIdStr())
		return TRUE;

	HXML pConfirm = XmlGetChild(node, "confirm");
	if (!pConfirm)
		return TRUE;

	const TCHAR *szId = XmlGetAttrValue(pConfirm, _T("id"));
	const TCHAR *szMethod = XmlGetAttrValue(pConfirm, _T("method"));
	const TCHAR *szUrl = XmlGetAttrValue(pConfirm, _T("url"));
	if (!szId || !szMethod || !szUrl)
		return TRUE;

	CJabberHttpAuthParams *pParams = (CJabberHttpAuthParams*)mir_calloc(sizeof(CJabberHttpAuthParams));
	if (pParams) {
		pParams->m_nType = CJabberHttpAuthParams::IQ;
		pParams->m_szFrom = mir_tstrdup(pInfo->GetFrom());
		pParams->m_szId = mir_tstrdup(szId);
		pParams->m_szMethod = mir_tstrdup(szMethod);
		pParams->m_szUrl = mir_tstrdup(szUrl);
		AddClistHttpAuthEvent(pParams);
	}
	return TRUE;
}
