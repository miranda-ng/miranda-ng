/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

#include "jabber_caps.h"

int CJabberProto::SerialNext(void)
{
	return ::InterlockedIncrement(&m_nSerial);
}

///////////////////////////////////////////////////////////////////////////////
// JabberChatRoomHContactFromJID - looks for the char room MCONTACT with required JID

MCONTACT CJabberProto::ChatRoomHContactFromJID(const char *jid)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, jid);
	if (item != nullptr && item->hContact)
		return item->hContact;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// JabberHContactFromJID - looks for the MCONTACT with required JID

MCONTACT CJabberProto::HContactFromJID(const char *jid, bool bStripResource)
{
	if (jid == nullptr)
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item != nullptr && item->hContact)
		return item->hContact;

	if (bStripResource) {
		char szJid[JABBER_MAX_JID_LEN];
		JabberStripJid(jid, szJid, _countof(szJid));
		return HContactFromJID(szJid, false);
	}

	return 0;
}

CMStringA MakeJid(const char *jid, const char *resource)
{
	CMStringA ret(jid);
	if (resource != nullptr) {
		ret.AppendChar('/');
		ret.Append(resource);
	}
	return ret;
}

void CJabberProto::UpdateItem(JABBER_LIST_ITEM *pItem, const char *name)
{
	if (!m_bIgnoreRoster || db_get_wsm(pItem->hContact, "CList", "MyHandle").IsEmpty()) {
		if (name != nullptr) {
			ptrA tszNick(getUStringA(pItem->hContact, "Nick"));
			if (tszNick != nullptr) {
				if (mir_strcmp(pItem->nick, tszNick) != 0)
					db_set_utf(pItem->hContact, "CList", "MyHandle", pItem->nick);
				else
					db_unset(pItem->hContact, "CList", "MyHandle");
			}
			else db_set_utf(pItem->hContact, "CList", "MyHandle", pItem->nick);
		}
		else db_unset(pItem->hContact, "CList", "MyHandle");
	}

	// check group delimiters
	if (pItem->group && m_szGroupDelimiter) {
		CMStringA szNewGroup(pItem->group);
		szNewGroup.Replace(m_szGroupDelimiter, "\\");
		replaceStr(pItem->group, szNewGroup.Detach());
	}

	if (!m_bIgnoreRoster) {
		if (pItem->group != nullptr) {
			Clist_GroupCreate(0, Utf2T(pItem->group));
			db_set_utf(pItem->hContact, "CList", "Group", pItem->group);
		}
		else db_unset(pItem->hContact, "CList", "Group");
	}
}

char* JabberNickFromJID(const char *jid)
{
	if (jid == nullptr)
		return mir_strdup("");

	const char *p = strchr(jid, '@');
	if (p == nullptr)
		p = strchr(jid, '/');

	return (p != nullptr) ? mir_strndup(jid, p - jid) : mir_strdup(jid);
}

pResourceStatus CJabberProto::ResourceInfoFromJID(const char *jid)
{
	if (jid == nullptr)
		return nullptr;

	const char *p = strchr(jid, '/');

	JABBER_LIST_ITEM *item = nullptr;
	if (p) {
		char szJid[JABBER_MAX_JID_LEN];
		JabberStripJid(jid, szJid, _countof(szJid));
		item = ListGetItemPtr(LIST_CHATROOM, szJid);
	}
	if (item == nullptr)
		item = ListGetItemPtr(LIST_VCARD_TEMP, jid);
	if (item == nullptr)
		item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return nullptr;

	if (p == nullptr)
		return item->getTemp();

	return item->findResource(p + 1);
}

char* JabberPrepareJid(const char *jid)
{
	if (jid == nullptr)
		return nullptr;

	char *szNewJid = mir_strdup(jid);
	if (!szNewJid)
		return nullptr;

	char *pDelimiter = strchr(szNewJid, '/');
	if (pDelimiter)
		*pDelimiter = 0;
	CharLowerA(szNewJid);
	if (pDelimiter)
		*pDelimiter = '/';
	return szNewJid;
}

char* JabberSha1(const char *str, JabberShaStrBuf buf)
{
	uint8_t digest[MIR_SHA1_HASH_SIZE];
	mir_sha1_ctx sha;
	mir_sha1_init(&sha);
	mir_sha1_append(&sha, (uint8_t*)str, (int)mir_strlen(str));
	mir_sha1_finish(&sha, digest);

	bin2hex(digest, sizeof(digest), buf);
	return buf;
}

wchar_t* JabberStrFixLines(const wchar_t *str)
{
	if (str == nullptr)
		return nullptr;

	const wchar_t *p;
	int add = 0;
	bool prev_r = false;
	bool prev_n = false;

	for (p = str; p && *p; ++p)
		if (*p == '\r' || *p == '\n')
			++add;

	wchar_t *buf = (wchar_t *)mir_alloc((mir_wstrlen(str) + add + 1) * sizeof(wchar_t));
	wchar_t *res = buf;

	for (p = str; p && *p; ++p) {
		if (*p == '\n' && !prev_r)
			*res++ = '\r';
		if (*p != '\r' && *p != '\n' && prev_r)
			*res++ = '\n';
		*res++ = *p;
		prev_r = *p == '\r';
		prev_n = *p == '\n';
	}
	*res = 0;

	return buf;
}

void JabberHttpUrlDecode(wchar_t *str)
{
	wchar_t *p, *q;
	unsigned int code;

	if (str == nullptr) return;
	for (p = q = (wchar_t*)str; *p != '\0'; p++, q++) {
		if (*p == '%' && *(p + 1) != '\0' && isxdigit(*(p + 1)) && *(p + 2) != '\0' && isxdigit(*(p + 2))) {
			swscanf((wchar_t*)p + 1, L"%2x", &code);
			*q = (unsigned char)code;
			p += 2;
		}
		else *q = *p;
	}

	*q = '\0';
}

int JabberCombineStatus(int status1, int status2)
{
	// Combine according to the following priority (high to low)
	// ID_STATUS_FREECHAT
	// ID_STATUS_ONLINE
	// ID_STATUS_DND
	// ID_STATUS_AWAY
	// ID_STATUS_NA
	// ID_STATUS_INVISIBLE (valid only for TLEN_PLUGIN)
	// ID_STATUS_OFFLINE
	// other ID_STATUS in random order (actually return status1)
	if (status1 == ID_STATUS_FREECHAT || status2 == ID_STATUS_FREECHAT)
		return ID_STATUS_FREECHAT;
	if (status1 == ID_STATUS_ONLINE || status2 == ID_STATUS_ONLINE)
		return ID_STATUS_ONLINE;
	if (status1 == ID_STATUS_DND || status2 == ID_STATUS_DND)
		return ID_STATUS_DND;
	if (status1 == ID_STATUS_AWAY || status2 == ID_STATUS_AWAY)
		return ID_STATUS_AWAY;
	if (status1 == ID_STATUS_NA || status2 == ID_STATUS_NA)
		return ID_STATUS_NA;
	if (status1 == ID_STATUS_INVISIBLE || status2 == ID_STATUS_INVISIBLE)
		return ID_STATUS_INVISIBLE;
	if (status1 == ID_STATUS_OFFLINE || status2 == ID_STATUS_OFFLINE)
		return ID_STATUS_OFFLINE;
	return status1;
}

struct tagErrorCodeToStr
{
	int code;
	wchar_t *str;
}
static JabberErrorCodeToStrMapping[] = {
		{ JABBER_ERROR_REDIRECT, LPGENW("Redirect") },
		{ JABBER_ERROR_BAD_REQUEST, LPGENW("Bad request") },
		{ JABBER_ERROR_UNAUTHORIZED, LPGENW("Unauthorized") },
		{ JABBER_ERROR_PAYMENT_REQUIRED, LPGENW("Payment required") },
		{ JABBER_ERROR_FORBIDDEN, LPGENW("Forbidden") },
		{ JABBER_ERROR_NOT_FOUND, LPGENW("Not found") },
		{ JABBER_ERROR_NOT_ALLOWED, LPGENW("Not allowed") },
		{ JABBER_ERROR_NOT_ACCEPTABLE, LPGENW("Not acceptable") },
		{ JABBER_ERROR_REGISTRATION_REQUIRED, LPGENW("Registration required") },
		{ JABBER_ERROR_REQUEST_TIMEOUT, LPGENW("Request timeout") },
		{ JABBER_ERROR_CONFLICT, LPGENW("Conflict") },
		{ JABBER_ERROR_INTERNAL_SERVER_ERROR, LPGENW("Internal server error") },
		{ JABBER_ERROR_NOT_IMPLEMENTED, LPGENW("Not implemented") },
		{ JABBER_ERROR_REMOTE_SERVER_ERROR, LPGENW("Remote server error") },
		{ JABBER_ERROR_SERVICE_UNAVAILABLE, LPGENW("Service unavailable") },
		{ JABBER_ERROR_REMOTE_SERVER_TIMEOUT, LPGENW("Remote server timeout") },
};

wchar_t* JabberErrorStr(int errorCode)
{
	for (auto &it : JabberErrorCodeToStrMapping)
		if (it.code == errorCode)
			return it.str;

	return LPGENW("Unknown error");
}

CMStringW JabberErrorMsg(const TiXmlElement *errorNode, int *pErrorCode)
{
	CMStringW ret;
	if (errorNode == nullptr) {
		if (pErrorCode)
			*pErrorCode = -1;
		ret.Format(L"%s -1: %s", TranslateT("Error"), TranslateT("Unknown error message"));
		return ret;
	}

	if (auto *pChild = XmlFirstChild(errorNode, "error"))
		errorNode = pChild;

	int errorCode = errorNode->IntAttribute("code");

	const char *str = errorNode->GetText();
	if (str == nullptr)
		str = XmlGetChildText(errorNode, "text");
	
	if (str == nullptr) {
		for (auto *c : TiXmlEnum(errorNode)) {
			const char *attr = XmlGetAttr(c, "xmlns");
			if (attr && !mir_strcmp(attr, "urn:ietf:params:xml:ns:xmpp-stanzas")) {
				str = c->Name();
				break;
			}
		}
	}

	ret.Format(L"%s %d: %s", TranslateT("Error"), errorCode, TranslateW(JabberErrorStr(errorCode)));
	if (str != nullptr)
		ret.AppendFormat(L"\r\n%s", Utf2T(str).get());

	if (pErrorCode)
		*pErrorCode = errorCode;
	return ret;
}

void CJabberProto::SendVisibleInvisiblePresence(bool invisible)
{
	if (!m_bJabberOnline) return;

	LISTFOREACH(i, this, LIST_ROSTER)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item == nullptr)
			continue;

		MCONTACT hContact = HContactFromJID(item->jid);
		if (hContact == 0)
			continue;

		uint16_t apparentMode = getWord(hContact, "ApparentMode", 0);
		if (invisible && apparentMode == ID_STATUS_OFFLINE)
			m_ThreadInfo->send(XmlNode("presence") << XATTR("to", item->jid) << XATTR("type", "invisible"));
		else if (!invisible && apparentMode == ID_STATUS_ONLINE)
			SendPresenceTo(m_iStatus, item->jid);
	}
}

time_t JabberIsoToUnixTime(const char *stamp)
{
	wchar_t date[9];
	int i, y;

	if (stamp == nullptr)
		return 0;

	auto *p = stamp;

	// Get the date part
	for (i = 0; *p != '\0' && i < 8 && isdigit(*p); p++, i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year (1970-2069)
		y = (date[0] - '0') * 10 + (date[1] - '0');
		if (y < 70) y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
		y -= 1900;
	}
	else return 0;

	struct tm timestamp;
	timestamp.tm_year = y;

	// Parse month
	timestamp.tm_mon = (date[i - 4] - '0') * 10 + date[i - 3] - '0' - 1;

	// Parse date
	timestamp.tm_mday = (date[i - 2] - '0') * 10 + date[i - 1] - '0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (sscanf(p, "%d:%d:%d", &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return (time_t)0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	time_t t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

void CJabberProto::SendPresenceTo(int status, const char *to, const TiXmlElement *extra, const char *msg)
{
	if (!m_bJabberOnline) return;

	// Send <presence/> update for status (we won't handle ID_STATUS_OFFLINE here)
	int iPriority = getDword("Priority", 0);
	UpdatePriorityMenu(iPriority);

	char szPriority[40];
	itoa(iPriority, szPriority, 10);

	XmlNode p("presence"); p << XCHILD("priority", szPriority);
	if (to != nullptr)
		p << XATTR("to", to);

	if (extra)
		p += extra;

	// XEP-0115:Entity Capabilities
	TiXmlElement *c = p << XCHILDNS("c", JABBER_FEAT_ENTITY_CAPS) << XATTR("hash", "sha-1")
		<< XATTR("node", JABBER_CAPS_MIRANDA_NODE) << XATTR("ver", m_szFeaturesCrc);
	
	if (m_bAllowVersionRequests) {
		LIST<char> arrExtCaps(5);
		if (g_plugin.bSecureIM)
			arrExtCaps.insert(JABBER_EXT_SECUREIM);

		if (g_plugin.bMirOTR)
			arrExtCaps.insert(JABBER_EXT_MIROTR);

		if (g_plugin.bNewGPG)
			arrExtCaps.insert(JABBER_EXT_NEWGPG);

		if(m_bUseOMEMO)
			arrExtCaps.insert(JABBER_EXT_OMEMO);

		if (g_plugin.bPlatform)
			arrExtCaps.insert(JABBER_EXT_PLATFORMX64);
		else
			arrExtCaps.insert(JABBER_EXT_PLATFORMX86);

		if (m_bEnableRemoteControl)
			arrExtCaps.insert(JABBER_EXT_COMMANDS);

		if (m_bEnableUserMood)
			arrExtCaps.insert(JABBER_EXT_USER_MOOD);

		if (m_bEnableUserTune)
			arrExtCaps.insert(JABBER_EXT_USER_TUNE);

		if (m_bEnableUserActivity)
			arrExtCaps.insert(JABBER_EXT_USER_ACTIVITY);

		if (m_bAcceptNotes)
			arrExtCaps.insert(JABBER_EXT_MIR_NOTES);

		NotifyFastHook(hExtListInit, (WPARAM)&arrExtCaps, (LPARAM)(IJabberInterface*)this);

		// add features enabled through IJabberNetInterface::AddFeatures()
		for (auto &it : m_lstJabberFeatCapPairsDynamic)
			if (m_uEnabledFeatCapsDynamic & it->jcbCap)
				arrExtCaps.insert(it->szExt);

		if (arrExtCaps.getCount()) {
			CMStringW szExtCaps = arrExtCaps[0];
			for (int i = 1; i < arrExtCaps.getCount(); i++) {
				szExtCaps.AppendChar(' ');
				szExtCaps += arrExtCaps[i];
			}
			c->SetAttribute("ext", szExtCaps);
		}
	}

	if (m_tmJabberIdleStartTime) {
		// XEP-0319 support
		char szSince[100];
		time2str(m_tmJabberIdleStartTime, szSince, _countof(szSince));
		p << XCHILDNS("idle", JABBER_FEAT_IDLE) << XATTR("since", szSince);
	}

	if (m_bEnableAvatars) {
		TiXmlElement *x = p << XCHILDNS("x", "vcard-temp:x:update");

		ptrA hashValue(getUStringA("AvatarHash"));
		if (hashValue != nullptr) // XEP-0153: vCard-Based Avatars
			x << XCHILD("photo", hashValue);
		else
			x << XCHILD("photo");

		ptrA vcardHash(getUStringA("VCardHash"));
		if (vcardHash != nullptr) {
			x = p << XCHILDNS("x", "miranda:x:vcard");
			x->SetText(vcardHash.get());
		}
	}
	{
		mir_cslock lck(m_csModeMsgMutex);
		switch (status) {
		case ID_STATUS_ONLINE:
			if (!msg) msg = m_modeMsgs.szOnline;
			break;
		case ID_STATUS_INVISIBLE:
			p << XATTR("type", "invisible");
			break;
		case ID_STATUS_AWAY:
			p << XCHILD("show", "away");
			if (!msg) msg = m_modeMsgs.szAway;
			break;
		case ID_STATUS_NA:
			p << XCHILD("show", "xa");
			if (!msg) msg = m_modeMsgs.szNa;
			break;
		case ID_STATUS_DND:
		case ID_STATUS_OCCUPIED:
			p << XCHILD("show", "dnd");
			if (!msg) msg = m_modeMsgs.szDnd;
			break;
		case ID_STATUS_FREECHAT:
			p << XCHILD("show", "chat");
			if (!msg) msg = m_modeMsgs.szFreechat;
			break;
		default: // Should not reach here
			break;
		}

		if (msg)
			p << XCHILD("status", msg);
	}

	m_ThreadInfo->send(p);
}

void CJabberProto::SendPresence(int status, bool bSendToAll)
{
	SendPresenceTo(status, nullptr);
	SendVisibleInvisiblePresence(status == ID_STATUS_INVISIBLE);

	// Also update status in all chatrooms
	if (bSendToAll) {
		LISTFOREACH(i, this, LIST_CHATROOM)
		{
			JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
			if (item != nullptr && item->nick != nullptr)
				SendPresenceTo(status == ID_STATUS_INVISIBLE ? ID_STATUS_ONLINE : status, MakeJid(item->jid, item->nick));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetPacketID - converts the xml id attribute into an integer

int JabberGetPacketID(const char *str)
{
	if (mir_strlen(str) >= 20)
		if (!memcmp(str, "mir", 3) && !memcmp(g_plugin.szRandom, str+3, 16) && str[19] == '_')
			return atoi(str + 20);

	return -1;
}

char* JabberId2string(int id)
{
	char text[100];
	mir_snprintf(text, "mir%s_%d", g_plugin.szRandom, id);
	return mir_strdup(text);
}

///////////////////////////////////////////////////////////////////////////////
// JabberGetClientJID - adds a resource postfix to a JID

char* CJabberProto::GetClientJID(MCONTACT hContact, char *dest, size_t destLen)
{
	if (hContact == 0)
		return nullptr;

	ptrA jid(getUStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : "jid"));
	return GetClientJID(jid, dest, destLen);
}

char* CJabberProto::GetClientJID(const char *jid, char *dest, size_t destLen)
{
	if (jid == nullptr)
		return nullptr;

	strncpy_s(dest, destLen, jid, _TRUNCATE);

	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, jid);
	if (LI != nullptr) {
		if (strchr(dest, '/') == nullptr) {
			pResourceStatus r(LI->getBestResource());
			if (r != nullptr)
				strncpy_s(dest, destLen, MakeJid(jid, r->m_szResourceName), _TRUNCATE);
		}
	}

	return dest;
}

///////////////////////////////////////////////////////////////////////////////
// JabberStripJid - strips a resource postfix from a JID

char* JabberStripJid(const char *jid, char *dest, size_t destLen)
{
	if (jid == nullptr)
		*dest = 0;
	else {
		strncpy_s(dest, destLen, jid, _TRUNCATE);

		char *p = strchr(dest, '/');
		if (p != nullptr)
			*p = 0;
	}

	return dest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// TStringPairs class members

TStringPairs::TStringPairs(char* buffer) :
	elems(nullptr)
{
	TStringPairsElem tempElem[100];

	char* token = strtok(buffer, ",");

	for (numElems = 0; token != nullptr; numElems++) {
		char* p = strchr(token, '='), *p1;
		if (p == nullptr)
			break;

		while (isspace(*token))
			token++;

		tempElem[numElems].name = rtrim(token);
		*p++ = 0;
		if ((p1 = strchr(p, '\"')) != nullptr) {
			*p1 = 0;
			p = p1 + 1;
		}

		if ((p1 = strrchr(p, '\"')) != nullptr)
			*p1 = 0;

		tempElem[numElems].value = rtrim(p);
		token = strtok(nullptr, ",");
	}

	if (numElems) {
		elems = new TStringPairsElem[numElems];
		memcpy(elems, tempElem, sizeof(tempElem[0]) * numElems);
	}
}

TStringPairs::~TStringPairs()
{
	delete[] elems;
}

const char* TStringPairs::operator[](const char* key) const
{
	for (int i = 0; i < numElems; i++)
		if (!mir_strcmp(elems[i].name, key))
			return elems[i].value;

	return "";
}

////////////////////////////////////////////////////////////////////////
// Manage combo boxes with recent item list

void CJabberProto::ComboLoadRecentStrings(HWND hwndDlg, UINT idcCombo, char *param, int recentCount)
{
	for (int i = 0; i < recentCount; i++) {
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, "%s%d", param, i);
		ptrW tszRecent(getWStringA(setting));
		if (tszRecent != nullptr)
			SendDlgItemMessage(hwndDlg, idcCombo, CB_ADDSTRING, 0, tszRecent);
	}

	if (!SendDlgItemMessage(hwndDlg, idcCombo, CB_GETCOUNT, 0, 0))
		SendDlgItemMessage(hwndDlg, idcCombo, CB_ADDSTRING, 0, (LPARAM)L"");
}

void CJabberProto::ComboAddRecentString(HWND hwndDlg, UINT idcCombo, char *param, const wchar_t *string, int recentCount)
{
	if (!string || !*string)
		return;
	if (SendDlgItemMessage(hwndDlg, idcCombo, CB_FINDSTRING, (WPARAM)-1, (LPARAM)string) != CB_ERR)
		return;

	int id;
	SendDlgItemMessage(hwndDlg, idcCombo, CB_ADDSTRING, 0, (LPARAM)string);
	if ((id = SendDlgItemMessage(hwndDlg, idcCombo, CB_FINDSTRING, (WPARAM)-1, (LPARAM)L"")) != CB_ERR)
		SendDlgItemMessage(hwndDlg, idcCombo, CB_DELETESTRING, id, 0);

	id = getByte(param, 0);
	char setting[MAXMODULELABELLENGTH];
	mir_snprintf(setting, "%s%d", param, id);
	setWString(setting, string);
	setByte(param, (id + 1) % recentCount);
}

////////////////////////////////////////////////////////////////////////
// time2str & str2time

char* time2str(time_t _time, char *buf, size_t bufLen)
{
	struct tm *T = gmtime(&_time);
	mir_snprintf(buf, bufLen, "%04d-%02d-%02dT%02d:%02d:%02dZ",
					  T->tm_year + 1900, T->tm_mon + 1, T->tm_mday, T->tm_hour, T->tm_min, T->tm_sec);
	return buf;
}

time_t str2time(const char *buf)
{
	struct tm T = { 0 };
	if (sscanf(buf, "%04d-%02d-%02dT%02d:%02d:%02dZ", &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec) != 6) {
		int boo;
		if (sscanf(buf, "%04d-%02d-%02dT%02d:%02d:%02d.%dZ", &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, &boo) != 7)
			return 0;
	}

	T.tm_year -= 1900;
	T.tm_mon--;
	return _mkgmtime(&T);
}

////////////////////////////////////////////////////////////////////////
// case-insensitive wcsstr
const wchar_t *JabberStrIStr(const wchar_t *str, const wchar_t *substr)
{
	wchar_t *str_up = NEWWSTR_ALLOCA(str);
	wchar_t *substr_up = NEWWSTR_ALLOCA(substr);

	CharUpperBuff(str_up, (uint32_t)mir_wstrlen(str_up));
	CharUpperBuff(substr_up, (uint32_t)mir_wstrlen(substr_up));

	wchar_t *p = wcsstr(str_up, substr_up);
	return p ? (str + (p - str_up)) : nullptr;
}

////////////////////////////////////////////////////////////////////////

BOOL CJabberProto::EnterString(CMStringW &result, const wchar_t *caption, int type, char *windowName, int recentCount, int timeout)
{
	if (caption == nullptr) {
		caption = NEWWSTR_ALLOCA(result.GetString());
		result.Empty();
	}

	ENTER_STRING param = {};
	param.type = type;
	param.caption = caption;
	param.szModuleName = m_szModuleName;
	param.szDataPrefix = windowName;
	param.recentCount = recentCount;
	param.timeout = timeout;
	param.ptszInitVal = result;
	INT_PTR res = ::EnterString(&param);
	if (res) {
		result = param.ptszResult;
		mir_free(param.ptszResult);
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// XEP-0203 delay support

bool JabberReadXep203delay(const TiXmlElement *node, time_t &msgTime)
{
	auto *n = XmlGetChildByTag(node, "delay", "xmlns", "urn:xmpp:delay");
	if (n == nullptr)
		return false;

	const char *ptszTimeStamp = XmlGetAttr(n, "stamp");
	if (ptszTimeStamp == nullptr)
		return false;

	// skip '-' chars
	char *szStamp = NEWSTR_ALLOCA(ptszTimeStamp);
	int si = 0, sj = 0;
	while (true) {
		if (szStamp[si] == '-')
			si++;
		else if (!(szStamp[sj++] = szStamp[si++]))
			break;
	};
	msgTime = JabberIsoToUnixTime(szStamp);
	return msgTime != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CJabberProto::IsMyOwnJID(const char *szJID)
{
	if (m_ThreadInfo == nullptr)
		return false;

	ptrA szFrom(JabberPrepareJid(szJID));
	if (szFrom == nullptr)
		return false;

	ptrA szTo(JabberPrepareJid(m_ThreadInfo->fullJID));
	if (szTo == nullptr)
		return false;

	char *pDelimiter = strchr(szFrom, '/');
	if (pDelimiter)
		*pDelimiter = 0;

	pDelimiter = strchr(szTo, '/');
	if (pDelimiter)
		*pDelimiter = 0;

	return mir_strcmp(szFrom, szTo) == 0;
}

bool CJabberProto::IsSendAck(MCONTACT hContact)
{
	if (getBool(hContact, "MsgAck"))
		return true;

	if (getBool(hContact, "MsgNoAck"))
		return false;

	return m_bMsgAck;
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CJabberProto::LoadHttpAvatars(void* param)
{
	Thread_SetName("Jabber: LoadHttpAvatars");

	OBJLIST<JABBER_HTTP_AVATARS> &avs = *(OBJLIST<JABBER_HTTP_AVATARS>*)param;
	HNETLIBCONN hHttpCon = nullptr;
	for (auto &it : avs) {
		NETLIBHTTPREQUEST nlhr = { 0 };
		nlhr.cbSize = sizeof(nlhr);
		nlhr.requestType = REQUEST_GET;
		nlhr.flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_PERSISTENT;
		nlhr.szUrl = it->Url;
		nlhr.nlc = hHttpCon;

		NLHR_PTR res(Netlib_HttpTransaction(m_hNetlibUser, &nlhr));
		if (res) {
			hHttpCon = res->nlc;
			if (res->resultCode == 200 && res->dataLength) {
				int pictureType = ProtoGetBufferFormat(res->pData);
				if (pictureType != PA_FORMAT_UNKNOWN) {
					PROTO_AVATAR_INFORMATION ai;
					ai.format = pictureType;
					ai.hContact = it->hContact;

					if (getByte(ai.hContact, "AvatarType", PA_FORMAT_UNKNOWN) != (unsigned char)pictureType) {
						wchar_t tszFileName[MAX_PATH];
						GetAvatarFileName(ai.hContact, tszFileName, _countof(tszFileName));
						DeleteFile(tszFileName);
					}

					setByte(ai.hContact, "AvatarType", pictureType);

					char buffer[2 * MIR_SHA1_HASH_SIZE + 1];
					uint8_t digest[MIR_SHA1_HASH_SIZE];
					mir_sha1_ctx sha;
					mir_sha1_init(&sha);
					mir_sha1_append(&sha, (uint8_t*)res->pData, res->dataLength);
					mir_sha1_finish(&sha, digest);
					bin2hex(digest, sizeof(digest), buffer);

					ptrA cmpsha(getStringA(ai.hContact, "AvatarHash"));
					if (cmpsha == nullptr || strnicmp(cmpsha, buffer, sizeof(buffer))) {
						wchar_t tszFileName[MAX_PATH];
						GetAvatarFileName(ai.hContact, tszFileName, _countof(tszFileName));
						wcsncpy_s(ai.filename, tszFileName, _TRUNCATE);
						FILE* out = _wfopen(tszFileName, L"wb");
						if (out != nullptr) {
							fwrite(res->pData, res->dataLength, 1, out);
							fclose(out);
							setString(ai.hContact, "AvatarHash", buffer);
							ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
							debugLogW(L"Broadcast new avatar: %s", ai.filename);
						}
						else ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
					}
				}
			}
		}
		else hHttpCon = nullptr;
	}
	delete &avs;
	if (hHttpCon)
		Netlib_CloseHandle(hHttpCon);
}

/////////////////////////////////////////////////////////////////////////////////////////
// UI utilities

void SetDlgItemTextUtf(HWND hwndDlg, int ctrlId, const char *szValue)
{
	if (szValue)
		SetDlgItemTextW(hwndDlg, ctrlId, Utf2T(szValue));
}

void SetWindowTextUtf(HWND hwndDlg, const char *szValue)
{
	if (szValue)
		SetWindowTextW(hwndDlg, Utf2T(szValue));
}

int UIEmulateBtnClick(HWND hwndDlg, UINT idcButton)
{
	if (IsWindowEnabled(GetDlgItem(hwndDlg, idcButton)))
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(idcButton, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, idcButton));
	return 0;
}

void UIShowControls(HWND hwndDlg, int *idList, int nCmdShow)
{
	for (; *idList; ++idList)
		ShowWindow(GetDlgItem(hwndDlg, *idList), nCmdShow);
}
