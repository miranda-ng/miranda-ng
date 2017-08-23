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
#include "jabber_rc.h"
#include "version.h"

BOOL CJabberProto::OnIqRequestVersion(HXML, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetFrom())
		return TRUE;

	if (!m_options.AllowVersionRequests)
		return FALSE;

	XmlNodeIq iq(L"result", pInfo);
	HXML query = iq << XQUERY(JABBER_FEAT_VERSION);
	query << XCHILD(L"name", L"Miranda NG Jabber");
	query << XCHILD(L"version", szCoreVersion);

	if (m_options.ShowOSVersion) {
		wchar_t os[256] = { 0 };
		if (!GetOSDisplayString(os, _countof(os)))
			mir_wstrncpy(os, L"Microsoft Windows", _countof(os));
		query << XCHILD(L"os", os);
	}

	m_ThreadInfo->send(iq);
	return TRUE;
}

// last activity (XEP-0012) support
BOOL CJabberProto::OnIqRequestLastActivity(HXML, CJabberIqInfo *pInfo)
{
	m_ThreadInfo->send(
		XmlNodeIq(L"result", pInfo) << XQUERY(JABBER_FEAT_LAST_ACTIVITY)
		<< XATTRI(L"seconds", m_tmJabberIdleStartTime ? time(0) - m_tmJabberIdleStartTime : 0));
	return TRUE;
}

// XEP-0199: XMPP Ping support
BOOL CJabberProto::OnIqRequestPing(HXML, CJabberIqInfo *pInfo)
{
	m_ThreadInfo->send(XmlNodeIq(L"result", pInfo) << XATTR(L"from", m_ThreadInfo->fullJID));
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
	wchar_t stime[100];
	wchar_t szTZ[10];

	TimeZone_PrintDateTime(UTC_TIME_HANDLE, L"I", stime, _countof(stime), 0);

	int nGmtOffset = GetGMTOffset();
	mir_snwprintf(szTZ, L"%+03d:%02d", nGmtOffset / 60, nGmtOffset % 60);

	XmlNodeIq iq(L"result", pInfo);
	HXML timeNode = iq << XCHILDNS(L"time", JABBER_FEAT_ENTITY_TIME);
	timeNode << XCHILD(L"utc", stime); timeNode << XCHILD(L"tzo", szTZ);
	LPCTSTR szTZName = TimeZone_GetName(nullptr);
	if (szTZName)
		timeNode << XCHILD(L"tz", szTZName);
	m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberProto::OnIqProcessIqOldTime(HXML, CJabberIqInfo *pInfo)
{
	struct tm *gmt;
	time_t ltime;
	wchar_t stime[100], *dtime;

	_tzset();
	time(&ltime);
	gmt = gmtime(&ltime);
	mir_snwprintf(stime, L"%.4i%.2i%.2iT%.2i:%.2i:%.2i",
		gmt->tm_year + 1900, gmt->tm_mon + 1,
		gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	dtime = _wctime(&ltime);
	dtime[24] = 0;

	XmlNodeIq iq(L"result", pInfo);
	HXML queryNode = iq << XQUERY(JABBER_FEAT_ENTITY_TIME_OLD);
	queryNode << XCHILD(L"utc", stime);
	LPCTSTR szTZName = TimeZone_GetName(nullptr);
	if (szTZName)
		queryNode << XCHILD(L"tz", szTZName);
	queryNode << XCHILD(L"display", dtime);
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

	const wchar_t *szMimeType = ProtoGetAvatarMimeType(pictureType);
	if (szMimeType == nullptr)
		return TRUE;

	wchar_t szFileName[MAX_PATH];
	GetAvatarFileName(0, szFileName, _countof(szFileName));

	FILE* in = _wfopen(szFileName, L"rb");
	if (in == nullptr)
		return TRUE;

	long bytes = _filelength(_fileno(in));
	ptrA buffer((char*)mir_alloc(bytes * 4 / 3 + bytes + 1000));
	if (buffer == nullptr) {
		fclose(in);
		return TRUE;
	}

	fread(buffer, bytes, 1, in);
	fclose(in);

	ptrA str(mir_base64_encode((PBYTE)(char*)buffer, bytes));
	m_ThreadInfo->send(XmlNodeIq(L"result", pInfo) << XQUERY(JABBER_FEAT_AVATAR) << XCHILD(L"query", _A2T(str)) << XATTR(L"mimetype", szMimeType));
	return TRUE;
}

BOOL CJabberProto::OnSiRequest(HXML node, CJabberIqInfo *pInfo)
{
	const wchar_t *szProfile = XmlGetAttrValue(pInfo->GetChildNode(), L"profile");

	if (szProfile && !mir_wstrcmp(szProfile, JABBER_FEAT_SI_FT))
		FtHandleSiRequest(node);
	else {
		XmlNodeIq iq(L"error", pInfo);
		HXML error = iq << XCHILD(L"error") << XATTRI(L"code", 400) << XATTR(L"type", L"cancel");
		error << XCHILDNS(L"bad-request", L"urn:ietf:params:xml:ns:xmpp-stanzas");
		error << XCHILD(L"bad-profile");
		m_ThreadInfo->send(iq);
	}
	return TRUE;
}

BOOL CJabberProto::OnRosterPushRequest(HXML, CJabberIqInfo *pInfo)
{
	HXML queryNode = pInfo->GetChildNode();

	// RFC 3921 #7.2 Business Rules
	if (pInfo->GetFrom()) {
		wchar_t *szFrom = JabberPrepareJid(pInfo->GetFrom());
		if (!szFrom)
			return TRUE;

		wchar_t *szTo = JabberPrepareJid(m_ThreadInfo->fullJID);
		if (!szTo) {
			mir_free(szFrom);
			return TRUE;
		}

		wchar_t *pDelimiter = wcschr(szFrom, '/');
		if (pDelimiter) *pDelimiter = 0;

		pDelimiter = wcschr(szTo, '/');
		if (pDelimiter) *pDelimiter = 0;

		BOOL bRetVal = mir_wstrcmp(szFrom, szTo) == 0;

		mir_free(szFrom);
		mir_free(szTo);

		// invalid JID
		if (!bRetVal) {
			debugLogW(L"<iq/> attempt to hack via roster push from %s", pInfo->GetFrom());
			return TRUE;
		}
	}

	const wchar_t *jid, *str;

	debugLogA("<iq/> Got roster push, query has %d children", XmlGetChildCount(queryNode));
	for (int i = 0;; i++) {
		HXML itemNode = XmlGetChild(queryNode, i);
		if (!itemNode)
			break;

		if (mir_wstrcmp(XmlGetName(itemNode), L"item") != 0)
			continue;
		if ((jid = XmlGetAttrValue(itemNode, L"jid")) == nullptr)
			continue;
		if ((str = XmlGetAttrValue(itemNode, L"subscription")) == nullptr)
			continue;

		// we will not add new account when subscription=remove
		if (!mir_wstrcmp(str, L"to") || !mir_wstrcmp(str, L"both") || !mir_wstrcmp(str, L"from") || !mir_wstrcmp(str, L"none")) {
			const wchar_t *name = XmlGetAttrValue(itemNode, L"name");
			ptrW nick((name != nullptr) ? mir_wstrdup(name) : JabberNickFromJID(jid));
			if (nick != nullptr) {
				MCONTACT hContact = HContactFromJID(jid, false);
				if (hContact == 0)
					hContact = DBCreateContact(jid, nick, false, false);
				else
					setWString(hContact, "jid", jid);

				JABBER_LIST_ITEM *item = ListAdd(LIST_ROSTER, jid, hContact);
				replaceStrW(item->nick, nick);
				item->bRealContact = true;

				HXML groupNode = XmlGetChild(itemNode, "group");
				replaceStrW(item->group, XmlGetText(groupNode));

				if (name != nullptr) {
					ptrW tszNick(getWStringA(hContact, "Nick"));
					if (tszNick != nullptr) {
						if (mir_wstrcmp(nick, tszNick) != 0)
							db_set_ws(hContact, "CList", "MyHandle", nick);
						else
							db_unset(hContact, "CList", "MyHandle");
					}
					else db_set_ws(hContact, "CList", "MyHandle", nick);
				}
				else db_unset(hContact, "CList", "MyHandle");

				if (!m_options.IgnoreRosterGroups) {
					if (item->group != nullptr) {
						Clist_GroupCreate(0, item->group);
						db_set_ws(hContact, "CList", "Group", item->group);
					}
					else db_unset(hContact, "CList", "Group");
				}
			}
		}

		if (JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid)) {
			if (!mir_wstrcmp(str, L"both")) item->subscription = SUB_BOTH;
			else if (!mir_wstrcmp(str, L"to")) item->subscription = SUB_TO;
			else if (!mir_wstrcmp(str, L"from")) item->subscription = SUB_FROM;
			else item->subscription = SUB_NONE;
			debugLogW(L"Roster push for jid=%s, set subscription to %s", jid, str);

			MCONTACT hContact = HContactFromJID(jid);

			// subscription = remove is to remove from roster list
			// but we will just set the contact to offline and not actually
			// remove, so that history will be retained.
			if (!mir_wstrcmp(str, L"remove")) {
				if (hContact) {
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
		XmlNodeIq iq(L"error", pInfo);
		HXML e = XmlAddChild(iq, L"error", L"File transfer refused"); XmlAddAttr(e, L"code", 406);
		m_ThreadInfo->send(iq);
		return TRUE;
	}

	filetransfer *ft = new filetransfer(this);
	ft->std.totalFiles = 1;
	ft->jid = mir_wstrdup(pInfo->GetFrom());
	ft->std.hContact = pInfo->GetHContact();
	ft->type = FT_OOB;
	ft->httpHostName = nullptr;
	ft->httpPort = 80;
	ft->httpPath = nullptr;

	// Parse the URL
	wchar_t *str = (wchar_t*)XmlGetText(n);	// URL of the file to get
	if (!wcsnicmp(str, L"http://", 7)) {
		wchar_t *p = str + 7, *q;
		if ((q = wcschr(p, '/')) != nullptr) {
			wchar_t text[1024];
			if (q - p < _countof(text)) {
				wcsncpy_s(text, p, q - p);
				text[q - p] = '\0';
				if ((p = wcschr(text, ':')) != nullptr) {
					ft->httpPort = (WORD)_wtoi(p + 1);
					*p = '\0';
				}
				ft->httpHostName = mir_u2a(text);
			}
		}
	}

	if (pInfo->GetIdStr())
		ft->szId = JabberId2string(pInfo->GetIqId());

	if (ft->httpHostName && ft->httpPath) {
		wchar_t *desc = nullptr;

		debugLogA("Host=%s Port=%d Path=%s", ft->httpHostName, ft->httpPort, ft->httpPath);
		if ((n = XmlGetChild(pInfo->GetChildNode(), "desc")) != nullptr)
			desc = (wchar_t*)XmlGetText(n);

		wchar_t *str2;
		debugLogW(L"description = %s", desc);
		if ((str2 = wcsrchr(ft->httpPath, '/')) != nullptr)
			str2++;
		else
			str2 = ft->httpPath;
		str2 = mir_wstrdup(str2);
		JabberHttpUrlDecode(str2);

		PROTORECVFILET pre;
		pre.dwFlags = PRFF_UNICODE;
		pre.timestamp = time(nullptr);
		pre.descr.w = desc;
		pre.files.w = &str2;
		pre.fileCount = 1;
		pre.lParam = (LPARAM)ft;
		ProtoChainRecvFile(ft->std.hContact, &pre);
		mir_free(str2);
	}
	else {
		// reject
		XmlNodeIq iq(L"error", pInfo);
		HXML e = XmlAddChild(iq, L"error", L"File transfer refused"); XmlAddAttr(e, L"code", 406);
		m_ThreadInfo->send(iq);
		delete ft;
	}
	return TRUE;
}

BOOL CJabberProto::OnHandleDiscoInfoRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return TRUE;

	const wchar_t *szNode = XmlGetAttrValue(pInfo->GetChildNode(), L"node");
	// caps hack
	if (m_clientCapsManager.HandleInfoRequest(iqNode, pInfo, szNode))
		return TRUE;

	// ad-hoc hack:
	if (szNode && m_adhocManager.HandleInfoRequest(iqNode, pInfo, szNode))
		return TRUE;

	// another request, send empty result
	m_ThreadInfo->send(
		XmlNodeIq(L"error", pInfo)
		<< XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
		<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
	return TRUE;
}

BOOL CJabberProto::OnHandleDiscoItemsRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!pInfo->GetChildNode())
		return TRUE;

	// ad-hoc commands check:
	const wchar_t *szNode = XmlGetAttrValue(pInfo->GetChildNode(), L"node");
	if (szNode && m_adhocManager.HandleItemsRequest(iqNode, pInfo, szNode))
		return TRUE;

	// another request, send empty result
	XmlNodeIq iq(L"result", pInfo);
	HXML resultQuery = iq << XQUERY(JABBER_FEAT_DISCO_ITEMS);
	if (szNode)
		XmlAddAttr(resultQuery, L"node", szNode);

	if (!szNode && m_options.EnableRemoteControl)
		resultQuery << XCHILD(L"item") << XATTR(L"jid", m_ThreadInfo->fullJID)
		<< XATTR(L"node", JABBER_FEAT_COMMANDS) << XATTR(L"name", L"Ad-hoc commands");

	m_ThreadInfo->send(iq);
	return TRUE;
}

BOOL CJabberProto::AddClistHttpAuthEvent(CJabberHttpAuthParams *pParams)
{
	char szService[256];
	mir_snprintf(szService, "%s%s", m_szModuleName, JS_HTTP_AUTH);

	CLISTEVENT cle = {};
	cle.hIcon = (HICON)LoadIconEx("openid");
	cle.flags = CLEF_PROTOCOLGLOBAL | CLEF_UNICODE;
	cle.hDbEvent = -99;
	cle.lParam = (LPARAM)pParams;
	cle.pszService = szService;
	cle.szTooltip.w = TranslateT("Http authentication request received");
	pcli->pfnAddEvent(&cle);
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

	const wchar_t *szId = XmlGetAttrValue(pConfirm, L"id");
	const wchar_t *szMethod = XmlGetAttrValue(pConfirm, L"method");
	const wchar_t *szUrl = XmlGetAttrValue(pConfirm, L"url");
	if (!szId || !szMethod || !szUrl)
		return TRUE;

	CJabberHttpAuthParams *pParams = (CJabberHttpAuthParams*)mir_calloc(sizeof(CJabberHttpAuthParams));
	if (pParams) {
		pParams->m_nType = CJabberHttpAuthParams::IQ;
		pParams->m_szFrom = mir_wstrdup(pInfo->GetFrom());
		pParams->m_szId = mir_wstrdup(szId);
		pParams->m_szMethod = mir_wstrdup(szMethod);
		pParams->m_szUrl = mir_wstrdup(szUrl);
		AddClistHttpAuthEvent(pParams);
	}
	return TRUE;
}
