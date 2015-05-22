// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

struct gateway_index
{
	HANDLE hConn;
	DWORD  dwIndex;
};

static mir_cs gatewayMutex;

static gateway_index *gateways = NULL;
static int gatewayCount = 0;

static DWORD *spammerList = NULL;
static int spammerListCount = 0;


void MoveDlgItem(HWND hwndDlg, int iItem, int left, int top, int width, int height)
{
	RECT rc;

	rc.left = left;
	rc.top = top;
	rc.right = left + width;
	rc.bottom = top + height;
	MapDialogRect(hwndDlg, &rc);
	MoveWindow(GetDlgItem(hwndDlg, iItem), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE);
}


void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
	EnableWindow(GetDlgItem(hwndDlg, control), state);
}


void ShowDlgItem(HWND hwndDlg, UINT control, int state)
{
	ShowWindow(GetDlgItem(hwndDlg, control), state);
}


void icq_EnableMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		EnableDlgItem(hwndDlg, controls[i], state);
}


void icq_ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		ShowDlgItem(hwndDlg, controls[i], state);
}


// Maps the ICQ status flag (as seen in the status change SNACS) and returns
// a Miranda style status.
int IcqStatusToMiranda(WORD nIcqStatus)
{
	int nMirandaStatus;

	// :NOTE: The order in which the flags are compared are important!
	// I dont like this method but it works.

	if (nIcqStatus & ICQ_STATUSF_INVISIBLE)
		nMirandaStatus = ID_STATUS_INVISIBLE;
	else
		if (nIcqStatus & ICQ_STATUSF_DND)
			nMirandaStatus = ID_STATUS_DND;
		else
			if (nIcqStatus & ICQ_STATUSF_OCCUPIED)
				nMirandaStatus = ID_STATUS_OCCUPIED;
			else
				if (nIcqStatus & ICQ_STATUSF_NA)
					nMirandaStatus = ID_STATUS_NA;
				else
					if (nIcqStatus & ICQ_STATUSF_AWAY)
						nMirandaStatus = ID_STATUS_AWAY;
					else
						if (nIcqStatus & ICQ_STATUSF_FFC)
							nMirandaStatus = ID_STATUS_FREECHAT;
						else
							// Can be discussed, but I think 'online' is the most generic ICQ status
							nMirandaStatus = ID_STATUS_ONLINE;

	return nMirandaStatus;
}

WORD MirandaStatusToIcq(int nMirandaStatus)
{
	WORD nIcqStatus;

	switch (nMirandaStatus) {
	case ID_STATUS_ONLINE:
		nIcqStatus = ICQ_STATUS_ONLINE;
		break;

	case ID_STATUS_AWAY:
		nIcqStatus = ICQ_STATUS_AWAY;
		break;

	case ID_STATUS_OUTTOLUNCH:
	case ID_STATUS_NA:
		nIcqStatus = ICQ_STATUS_NA;
		break;

	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OCCUPIED:
		nIcqStatus = ICQ_STATUS_OCCUPIED;
		break;

	case ID_STATUS_DND:
		nIcqStatus = ICQ_STATUS_DND;
		break;

	case ID_STATUS_INVISIBLE:
		nIcqStatus = ICQ_STATUS_INVISIBLE;
		break;

	case ID_STATUS_FREECHAT:
		nIcqStatus = ICQ_STATUS_FFC;
		break;

	case ID_STATUS_OFFLINE:
		// Oscar doesnt have anything that maps to this status. This should never happen.
		_ASSERTE(nMirandaStatus != ID_STATUS_OFFLINE);
		nIcqStatus = 0;
		break;

	default:
		// Online seems to be a good default.
		// Since it cant be offline, it must be a new type of online status.
		nIcqStatus = ICQ_STATUS_ONLINE;
		break;
	}

	return nIcqStatus;
}

int MirandaStatusToSupported(int nMirandaStatus)
{
	int nSupportedStatus;

	switch (nMirandaStatus) {

		// These status mode does not need any mapping
	case ID_STATUS_ONLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_NA:
	case ID_STATUS_OCCUPIED:
	case ID_STATUS_DND:
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:
		nSupportedStatus = nMirandaStatus;
		break;

	case ID_STATUS_FREECHAT:
		nSupportedStatus = ID_STATUS_ONLINE;
		break;

		// This mode is not support and must be mapped to something else
	case ID_STATUS_OUTTOLUNCH:
		nSupportedStatus = ID_STATUS_NA;
		break;

		// This mode is not support and must be mapped to something else
	case ID_STATUS_ONTHEPHONE:
		nSupportedStatus = ID_STATUS_OCCUPIED;
		break;

		// This is not supposed to happen.
	default:
		_ASSERTE(0);
		// Online seems to be a good default.
		nSupportedStatus = ID_STATUS_ONLINE;
		break;
	}

	return nSupportedStatus;
}

char* MirandaStatusToStringUtf(int mirandaStatus)
{ // return miranda status description in utf-8, use unicode service is possible
	return tchar_to_utf8(pcli->pfnGetStatusModeDescription(mirandaStatus, 0));
}

char** CIcqProto::MirandaStatusToAwayMsg(int nStatus)
{
	switch (nStatus) {
	case ID_STATUS_ONLINE:
		return &m_modeMsgs.szOnline;

	case ID_STATUS_AWAY:
		return &m_modeMsgs.szAway;

	case ID_STATUS_NA:
		return &m_modeMsgs.szNa;

	case ID_STATUS_OCCUPIED:
		return &m_modeMsgs.szOccupied;

	case ID_STATUS_DND:
		return &m_modeMsgs.szDnd;

	case ID_STATUS_FREECHAT:
		return &m_modeMsgs.szFfc;

	default:
		return NULL;
	}
}

int AwayMsgTypeToStatus(int nMsgType)
{
	switch (nMsgType) {
	case MTYPE_AUTOONLINE:
		return ID_STATUS_ONLINE;

	case MTYPE_AUTOAWAY:
		return ID_STATUS_AWAY;

	case MTYPE_AUTOBUSY:
		return ID_STATUS_OCCUPIED;

	case MTYPE_AUTONA:
		return ID_STATUS_NA;

	case MTYPE_AUTODND:
		return ID_STATUS_DND;

	case MTYPE_AUTOFFC:
		return ID_STATUS_FREECHAT;

	default:
		return ID_STATUS_OFFLINE;
	}
}


void SetGatewayIndex(HANDLE hConn, DWORD dwIndex)
{
	mir_cslock l(gatewayMutex);

	for (int i = 0; i < gatewayCount; i++) {
		if (hConn == gateways[i].hConn) {
			gateways[i].dwIndex = dwIndex;
			return;
		}
	}

	gateways = (gateway_index *)SAFE_REALLOC(gateways, sizeof(gateway_index)* (gatewayCount + 1));
	gateways[gatewayCount].hConn = hConn;
	gateways[gatewayCount].dwIndex = dwIndex;
	gatewayCount++;
}


DWORD GetGatewayIndex(HANDLE hConn)
{
	mir_cslock l(gatewayMutex);

	for (int i = 0; i < gatewayCount; i++)
		if (hConn == gateways[i].hConn)
			return gateways[i].dwIndex;

	return 1; // this is default
}


void FreeGatewayIndex(HANDLE hConn)
{
	mir_cslock l(gatewayMutex);

	for (int i = 0; i < gatewayCount; i++) {
		if (hConn == gateways[i].hConn) {
			gatewayCount--;
			memmove(&gateways[i], &gateways[i + 1], sizeof(gateway_index)* (gatewayCount - i));
			gateways = (gateway_index*)SAFE_REALLOC(gateways, sizeof(gateway_index)* gatewayCount);

			// Gateway found, exit loop
			break;
		}
	}
}


void CIcqProto::AddToSpammerList(DWORD dwUIN)
{
	mir_cslock l(gatewayMutex);

	spammerList = (DWORD *)SAFE_REALLOC(spammerList, sizeof(DWORD)* (spammerListCount + 1));
	spammerList[spammerListCount] = dwUIN;
	spammerListCount++;
}


BOOL CIcqProto::IsOnSpammerList(DWORD dwUIN)
{
	mir_cslock l(gatewayMutex);

	for (int i = 0; i < spammerListCount; i++)
		if (dwUIN == spammerList[i])
			return TRUE;

	return FALSE;
}


// ICQ contacts cache

void CIcqProto::AddToContactsCache(MCONTACT hContact, DWORD dwUin, const char *szUid)
{
	if (!hContact || (!dwUin && !szUid))
		return;

	icq_contacts_cache *cache_item = (icq_contacts_cache*)SAFE_MALLOC(sizeof(icq_contacts_cache));
	cache_item->hContact = hContact;
	cache_item->dwUin = dwUin;
	if (!dwUin)
		cache_item->szUid = null_strdup(szUid);

	mir_cslock l(contactsCacheMutex);
	contactsCache.insert(cache_item);
}


void CIcqProto::InitContactsCache()
{
	// build cache
	mir_cslock l(contactsCacheMutex);

	MCONTACT hContact = db_find_first(m_szModuleName);

	while (hContact) {
		DWORD dwUin;
		uid_str szUid;
		if (!getContactUid(hContact, &dwUin, &szUid))
			AddToContactsCache(hContact, dwUin, szUid);

		hContact = db_find_next(hContact, m_szModuleName);
	}
}


void CIcqProto::UninitContactsCache(void)
{
	{	mir_cslock l(contactsCacheMutex);

		// cleanup the cache
		for (int i = 0; i < contactsCache.getCount(); i++) {
			icq_contacts_cache *cache_item = contactsCache[i];

			SAFE_FREE((void**)&cache_item->szUid);
			SAFE_FREE((void**)&cache_item);
		}

		contactsCache.destroy();
	}
}


void CIcqProto::DeleteFromContactsCache(MCONTACT hContact)
{
	mir_cslock l(contactsCacheMutex);

	for (int i = 0; i < contactsCache.getCount(); i++) {
		icq_contacts_cache *cache_item = contactsCache[i];

		if (cache_item->hContact == hContact) {
			contactsCache.remove(i);
			// Release memory
			SAFE_FREE((void**)&cache_item->szUid);
			SAFE_FREE((void**)&cache_item);
			break;
		}
	}
}


MCONTACT CIcqProto::HandleFromCacheByUid(DWORD dwUin, const char *szUid)
{
	icq_contacts_cache cache_item = { NULL, dwUin, szUid };

	mir_cslock l(contactsCacheMutex);
	// find in list
	int i = contactsCache.getIndex(&cache_item);
	if (i != -1)
		return contactsCache[i]->hContact;

	return NULL;
}


MCONTACT CIcqProto::HContactFromUIN(DWORD dwUin, int *Added)
{
	if (Added) *Added = 0;

	MCONTACT hContact = HandleFromCacheByUid(dwUin, NULL);
	if (hContact)
		return hContact;

	hContact = db_find_first(m_szModuleName);
	while (hContact) {
		DWORD dwContactUin;

		dwContactUin = getContactUin(hContact);
		if (dwContactUin == dwUin) {
			AddToContactsCache(hContact, dwUin, NULL);
			return hContact;
		}

		hContact = db_find_next(hContact, m_szModuleName);
	}

	//not present: add
	if (Added) {
		debugLogA("Attempt to create ICQ contact %u", dwUin);

		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!hContact) {
			debugLogA("Failed to create ICQ contact %u", dwUin);
			return INVALID_CONTACT_ID;
		}

		if (CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName) != 0) {
			// For some reason we failed to register the protocol to this contact
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);
			debugLogA("Failed to register ICQ contact %u", dwUin);
			return INVALID_CONTACT_ID;
		}

		setDword(hContact, UNIQUEIDSETTING, dwUin);

		if (!bIsSyncingCL) {
			db_set_b(hContact, "CList", "NotOnList", 1);
			setContactHidden(hContact, 1);

			setWord(hContact, "Status", ID_STATUS_OFFLINE);

			icq_QueueUser(hContact);

			if (icqOnline())
				icq_sendNewContact(dwUin, NULL);
		}
		AddToContactsCache(hContact, dwUin, NULL);
		*Added = 1;
		debugLogA("ICQ contact %u created ok", dwUin);
		return hContact;
	}

	// not in list, check that uin do not belong to us
	if (getContactUin(NULL) == dwUin)
		return NULL;

	return INVALID_CONTACT_ID;
}

MCONTACT CIcqProto::HContactFromUID(DWORD dwUin, const char *szUid, int *Added)
{
	if (dwUin)
		return HContactFromUIN(dwUin, Added);

	if (Added) *Added = 0;

	if (!m_bAimEnabled)
		return INVALID_CONTACT_ID;

	MCONTACT hContact = HandleFromCacheByUid(dwUin, szUid);
	if (hContact) return hContact;

	hContact = db_find_first(m_szModuleName);
	while (hContact) {
		DWORD dwContactUin;
		uid_str szContactUid;
		if (!getContactUid(hContact, &dwContactUin, &szContactUid)) {
			if (!dwContactUin && !stricmpnull(szContactUid, szUid)) {
				if (strcmpnull(szContactUid, szUid)) // fix case in SN
					setString(hContact, UNIQUEIDSETTING, szUid);

				return hContact;
			}
		}
		hContact = db_find_next(hContact, m_szModuleName);
	}

	//not present: add
	if (Added) {
		debugLogA("Attempt to create ICQ contact by string <%s>", szUid);

		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, UNIQUEIDSETTING, szUid);

		if (!bIsSyncingCL) {
			db_set_b(hContact, "CList", "NotOnList", 1);
			setContactHidden(hContact, 1);

			setWord(hContact, "Status", ID_STATUS_OFFLINE);

			if (icqOnline())
				icq_sendNewContact(0, szUid);
		}
		AddToContactsCache(hContact, 0, szUid);
		*Added = 1;

		return hContact;
	}

	return INVALID_CONTACT_ID;
}

MCONTACT CIcqProto::HContactFromAuthEvent(MEVENT hEvent)
{
	DWORD body[3];

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (strcmpnull(dbei.szModule, m_szModuleName))
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

char* NickFromHandle(MCONTACT hContact)
{
	if (hContact == INVALID_CONTACT_ID)
		return null_strdup(Translate("<invalid>"));

	return null_strdup((char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, 0));
}

char* NickFromHandleUtf(MCONTACT hContact)
{
	if (hContact == INVALID_CONTACT_ID)
		return ICQTranslateUtf(LPGEN("<invalid>"));

	return tchar_to_utf8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
}

char* strUID(DWORD dwUIN, char *pszUID)
{
	if (dwUIN && pszUID)
		_ltoa(dwUIN, pszUID, 10);

	return pszUID;
}


/* a mir_strcmp() that likes NULL */
int __fastcall strcmpnull(const char *str1, const char *str2)
{
	if (str1 && str2)
		return mir_strcmp(str1, str2);

	if (!str1 && !str2)
		return 0;

	return 1;
}

/* a stricmp() that likes NULL */
int __fastcall stricmpnull(const char *str1, const char *str2)
{
	if (str1 && str2)
		return _stricmp(str1, str2);

	if (!str1 && !str2)
		return 0;

	return 1;
}

char* __fastcall strstrnull(const char *str, const char *substr)
{
	if (str)
		return (char*)strstr(str, substr);

	return NULL;
}

char* __fastcall null_strdup(const char *string)
{
	if (string)
		return _strdup(string);

	return NULL;
}


WCHAR* __fastcall null_strdup(const WCHAR *string)
{
	if (string)
		return wcsdup(string);

	return NULL;
}


char* __fastcall null_strcpy(char *dest, const char *src, size_t maxlen)
{
	if (!dest)
		return NULL;

	if (src && src[0]) {
		strncpy(dest, src, maxlen);
		dest[maxlen] = '\0';
	}
	else
		dest[0] = '\0';

	return dest;
}


WCHAR* __fastcall null_strcpy(WCHAR *dest, const WCHAR *src, size_t maxlen)
{
	if (!dest)
		return NULL;

	if (src && src[0]) {
		wcsncpy(dest, src, maxlen);
		dest[maxlen] = '\0';
	}
	else
		dest[0] = '\0';

	return dest;
}


size_t __fastcall null_strcut(char *string, size_t maxlen)
{ // limit the string to max length (null & utf-8 strings ready)
	size_t len = mir_strlen(string);
	if (len < maxlen)
		return len;

	len = maxlen;

	if (UTF8_IsValid(string)) // handle utf-8 string
	{ // find the first byte of possible multi-byte character
		while ((string[len] & 0xc0) == 0x80) len--;
	}

	// simply cut the string
	string[len] = '\0';
	return len;
}


void parseServerAddress(char* szServer, WORD* wPort)
{
	int i = 0;

	while (szServer[i] && szServer[i] != ':') i++;
	if (szServer[i] == ':') { // port included
		*wPort = atoi(&szServer[i + 1]);
	} // otherwise do not change port

	szServer[i] = '\0';
}

char* DemangleXml(const char *string, size_t len)
{
	char *szWork = (char*)SAFE_MALLOC(len + 1), *szChar = szWork;

	for (size_t i = 0; i < len; i++) {
		if (!_strnicmp(string + i, "&gt;", 4)) {
			*szChar = '>';
			szChar++;
			i += 3;
		}
		else if (!_strnicmp(string + i, "&lt;", 4)) {
			*szChar = '<';
			szChar++;
			i += 3;
		}
		else if (!_strnicmp(string + i, "&amp;", 5)) {
			*szChar = '&';
			szChar++;
			i += 4;
		}
		else if (!_strnicmp(string + i, "&quot;", 6)) {
			*szChar = '"';
			szChar++;
			i += 5;
		}
		else {
			*szChar = string[i];
			szChar++;
		}
	}
	*szChar = '\0';

	return szWork;
}

char* MangleXml(const char *string, size_t len)
{
	size_t l = 1;
	char *szWork, *szChar;

	for (size_t i = 0; i < len; i++) {
		if (string[i] == '<' || string[i] == '>')
			l += 4;
		else if (string[i] == '&')
			l += 5;
		else if (string[i] == '"')
			l += 6;
		else l++;
	}
	szChar = szWork = (char*)SAFE_MALLOC(l + 1);
	for (size_t i = 0; i < len; i++) {
		if (string[i] == '<') {
			*(DWORD*)szChar = ';tl&';
			szChar += 4;
		}
		else if (string[i] == '>') {
			*(DWORD*)szChar = ';tg&';
			szChar += 4;
		}
		else if (string[i] == '&') {
			*(DWORD*)szChar = 'pma&';
			szChar += 4;
			*szChar = ';';
			szChar++;
		}
		else if (string[i] == '"') {
			*(DWORD*)szChar = 'ouq&';
			szChar += 4;
			*(WORD*)szChar = ';t';
			szChar += 2;
		}
		else {
			*szChar = string[i];
			szChar++;
		}
	}
	*szChar = '\0';

	return szWork;
}

char* EliminateHtml(const char *string, size_t len)
{
	char *tmp = (char*)SAFE_MALLOC(len + 1);
	BOOL tag = FALSE;
	char *res;

	for (size_t i = 0, j = 0; i < len; i++) {
		if (!tag && string[i] == '<') {
			if ((i + 4 <= len) && (!_strnicmp(string + i, "<br>", 4) || !_strnicmp(string + i, "<br/>", 5))) { // insert newline
				tmp[j] = '\r';
				j++;
				tmp[j] = '\n';
				j++;
			}
			tag = TRUE;
		}
		else if (tag && string[i] == '>') {
			tag = FALSE;
		}
		else if (!tag) {
			tmp[j] = string[i];
			j++;
		}
		tmp[j] = '\0';
	}
	SAFE_FREE((void**)&string);
	res = DemangleXml(tmp, mir_strlen(tmp));
	SAFE_FREE((void**)&tmp);

	return res;
}

char* ApplyEncoding(const char *string, const char *pszEncoding)
{
	// decode encoding to Utf-8
	if (string && pszEncoding) { // we do only encodings known to icq5.1 // TODO: check if this is enough
		if (!_strnicmp(pszEncoding, "utf-8", 5)) // it is utf-8 encoded
			return null_strdup(string);

		if (!_strnicmp(pszEncoding, "unicode-2-0", 11)) { // it is UCS-2 encoded
			size_t wLen = mir_wstrlen((WCHAR*)string) + 1;
			WCHAR *szStr = (WCHAR*)_alloca(wLen*2);
			BYTE *tmp = (BYTE*)string;

			unpackWideString(&tmp, szStr, wLen*2);

			return make_utf8_string(szStr);
		}
		if (!_strnicmp(pszEncoding, "iso-8859-1", 10)) // we use "Latin I" instead - it does the job
			return ansi_to_utf8_codepage(string, 1252);
	}
	if (string) // consider it CP_ACP
		return ansi_to_utf8(string);

	return NULL;
}

void CIcqProto::ResetSettingsOnListReload()
{
	// Reset a bunch of session specific settings
	setWord(DBSETTING_SERVLIST_PRIVACY, 0);
	setWord(DBSETTING_SERVLIST_METAINFO, 0);
	setWord(DBSETTING_SERVLIST_AVATAR, 0);
	setWord(DBSETTING_SERVLIST_PHOTO, 0);
	setWord("SrvRecordCount", 0);
	delSetting(DBSETTING_SERVLIST_UNHANDLED);

	MCONTACT hContact = db_find_first(m_szModuleName);

	while (hContact) {
		// All these values will be restored during the serv-list receive
		setWord(hContact, DBSETTING_SERVLIST_ID, 0);
		setWord(hContact, DBSETTING_SERVLIST_GROUP, 0);
		setWord(hContact, DBSETTING_SERVLIST_PERMIT, 0);
		setWord(hContact, DBSETTING_SERVLIST_DENY, 0);
		delSetting(hContact, DBSETTING_SERVLIST_IGNORE);
		setByte(hContact, "Auth", 0);
		delSetting(hContact, DBSETTING_SERVLIST_DATA);

		hContact = db_find_next(hContact, m_szModuleName);
	}

	FlushSrvGroupsCache();
}

void CIcqProto::ResetSettingsOnConnect()
{
	// Reset a bunch of session specific settings
	setByte("SrvVisibility", 0);
	setDword("IdleTS", 0);

	MCONTACT hContact = db_find_first(m_szModuleName);

	while (hContact) {
		setDword(hContact, "LogonTS", 0);
		setDword(hContact, "IdleTS", 0);
		setDword(hContact, "TickTS", 0);
		setByte(hContact, "TemporaryVisible", 0);

		// All these values will be restored during the login
		if (getContactStatus(hContact) != ID_STATUS_OFFLINE)
			setWord(hContact, "Status", ID_STATUS_OFFLINE);

		hContact = db_find_next(hContact, m_szModuleName);
	}
}

void CIcqProto::ResetSettingsOnLoad()
{
	setDword("IdleTS", 0);
	setDword("LogonTS", 0);

	MCONTACT hContact = db_find_first(m_szModuleName);

	while (hContact) {
		setDword(hContact, "LogonTS", 0);
		setDword(hContact, "IdleTS", 0);
		setDword(hContact, "TickTS", 0);
		if (getContactStatus(hContact) != ID_STATUS_OFFLINE) {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);

			delSetting(hContact, DBSETTING_XSTATUS_ID);
			delSetting(hContact, DBSETTING_XSTATUS_NAME);
			delSetting(hContact, DBSETTING_XSTATUS_MSG);
		}
		setByte(hContact, "DCStatus", 0);

		hContact = db_find_next(hContact, m_szModuleName);
	}
}

int RandRange(int nLow, int nHigh)
{
	return nLow + (int)((nHigh - nLow + 1)*rand() / (RAND_MAX + 1.0));
}


bool IsStringUIN(const char *pszString)
{
	size_t nLen = mir_strlen(pszString);

	if (nLen > 0 && pszString[0] != '0') {
		for (size_t i = 0; i < nLen; i++)
			if ((pszString[i] < '0') || (pszString[i] > '9'))
				return FALSE;

		return TRUE;
	}

	return FALSE;
}


void __cdecl CIcqProto::ProtocolAckThread(icq_ack_args* pArguments)
{
	Sleep(150);

	if (pArguments->nAckResult == ACKRESULT_SUCCESS)
		debugLogA("Sent fake message ack");
	else if (pArguments->nAckResult == ACKRESULT_FAILED)
		debugLogA("Message delivery failed");

	ProtoBroadcastAck(pArguments->hContact, pArguments->nAckType, pArguments->nAckResult, pArguments->hSequence, pArguments->pszMessage);

	SAFE_FREE((void**)(char **)&pArguments->pszMessage);
	SAFE_FREE((void**)&pArguments);
}

void CIcqProto::SendProtoAck(MCONTACT hContact, DWORD dwCookie, int nAckResult, int nAckType, char* pszMessage)
{
	icq_ack_args* pArgs = (icq_ack_args*)SAFE_MALLOC(sizeof(icq_ack_args)); // This will be freed in the new thread
	pArgs->hContact = hContact;
	pArgs->hSequence = (HANDLE)dwCookie;
	pArgs->nAckResult = nAckResult;
	pArgs->nAckType = nAckType;
	pArgs->pszMessage = (LPARAM)null_strdup(pszMessage);

	ForkThread((MyThreadFunc)&CIcqProto::ProtocolAckThread, pArgs);
}

void CIcqProto::SetCurrentStatus(int nStatus)
{
	int nOldStatus = m_iStatus;

	m_iStatus = nStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)nOldStatus, nStatus);
}


int CIcqProto::IsMetaInfoChanged(MCONTACT hContact)
{
	DBVARIANT infoToken = { DBVT_DELETED };
	int res = 0;

	if (!getSetting(hContact, DBSETTING_METAINFO_TOKEN, &infoToken)) { // contact does have info from directory, check if it is not outdated
		double dInfoTime = 0;
		double dInfoSaved = 0;

		if ((dInfoTime = getSettingDouble(hContact, DBSETTING_METAINFO_TIME, 0)) > 0) {
			if ((dInfoSaved = getSettingDouble(hContact, DBSETTING_METAINFO_SAVED, 0)) > 0) {
				if (dInfoSaved < dInfoTime)
					res = 2; // directory info outdated
			}
			else
				res = 1; // directory info not saved at all
		}

		db_free(&infoToken);
	}
	else { // it cannot be detected if user info was not changed, so use a generic threshold
		DBVARIANT infoSaved = { DBVT_DELETED };
		DWORD dwInfoTime = 0;

		if (!getSetting(hContact, DBSETTING_METAINFO_SAVED, &infoSaved)) {
			if (infoSaved.type == DBVT_BLOB && infoSaved.cpbVal == 8) {
				double dwTime = *(double*)infoSaved.pbVal;

				dwInfoTime = (dwTime - 25567) * 86400;
			}
			else if (infoSaved.type == DBVT_DWORD)
				dwInfoTime = infoSaved.dVal;

			db_free(&infoSaved);

			if ((time(NULL) - dwInfoTime) > 14 * 3600 * 24) {
				res = 3; // threshold exceeded
			}
		}
		else
			res = 4; // no timestamp found
	}

	return res;
}


void __cdecl CIcqProto::SetStatusNoteThread(void *pDelay)
{
	if (pDelay)
		SleepEx((DWORD)pDelay, TRUE);

	mir_cslockfull l(cookieMutex);

	if (icqOnline() && (setStatusNoteText || setStatusMoodData)) { // send status note change packets, write status note to database
		if (setStatusNoteText) { // change status note in directory
			mir_cslockfull rlck(m_ratesMutex);
			if (m_rates) { // rate management
				WORD wGroup = m_rates->getGroupFromSNAC(ICQ_EXTENSIONS_FAMILY, ICQ_META_CLI_REQUEST);

				while (m_rates->getNextRateLevel(wGroup) < m_rates->getLimitLevel(wGroup, RML_LIMIT)) { // we are over rate, need to wait before sending
					int nDelay = m_rates->getDelayToLimitLevel(wGroup, RML_IDLE_10);

					rlck.unlock();
					l.unlock();

					debugLogA("Rates: SetStatusNote delayed %dms", nDelay);

					SleepEx(nDelay, TRUE); // do not keep things locked during sleep
					l.lock();
					rlck.lock();
					if (!m_rates) // we lost connection when we slept, go away
						break;
				}
			}
			rlck.unlock();

			BYTE *pBuffer = NULL;
			size_t cbBuffer = 0;

			ppackTLV(&pBuffer, &cbBuffer, 0x226, mir_strlen(setStatusNoteText), (BYTE*)setStatusNoteText);
			icq_changeUserDirectoryInfoServ(pBuffer, cbBuffer, DIRECTORYREQUEST_UPDATENOTE);

			SAFE_FREE((void**)&pBuffer);
		}

		if (setStatusNoteText || setStatusMoodData) { // change status note and mood in session data
			mir_cslockfull rlck(m_ratesMutex);
			if (m_rates) { // rate management
				WORD wGroup = m_rates->getGroupFromSNAC(ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);

				while (m_rates->getNextRateLevel(wGroup) < m_rates->getLimitLevel(wGroup, RML_LIMIT)) { // we are over rate, need to wait before sending
					int nDelay = m_rates->getDelayToLimitLevel(wGroup, RML_IDLE_10);

					rlck.unlock();
					l.unlock();

					debugLogA("Rates: SetStatusNote delayed %dms", nDelay);

					SleepEx(nDelay, TRUE); // do not keep things locked during sleep
					l.lock();
					rlck.lock();
					if (!m_rates) // we lost connection when we slept, go away
						break;
				}
			}
			rlck.unlock();

			// check if the session data were not updated already
			char *szCurrentStatusNote = getSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, NULL);
			char *szCurrentStatusMood = NULL;
			DBVARIANT dbv = { DBVT_DELETED };

			if (m_bMoodsEnabled && !getString(DBSETTING_STATUS_MOOD, &dbv))
				szCurrentStatusMood = dbv.pszVal;

			if (!setStatusNoteText && szCurrentStatusNote)
				setStatusNoteText = null_strdup(szCurrentStatusNote);
			if (m_bMoodsEnabled && !setStatusMoodData && szCurrentStatusMood)
				setStatusMoodData = null_strdup(szCurrentStatusMood);

			if (strcmpnull(szCurrentStatusNote, setStatusNoteText) || (m_bMoodsEnabled && strcmpnull(szCurrentStatusMood, setStatusMoodData))) {
				db_set_utf(NULL, m_szModuleName, DBSETTING_STATUS_NOTE, setStatusNoteText);
				if (m_bMoodsEnabled)
					setString(DBSETTING_STATUS_MOOD, setStatusMoodData);

				size_t wStatusNoteLen = mir_strlen(setStatusNoteText);
				size_t wStatusMoodLen = m_bMoodsEnabled ? mir_strlen(setStatusMoodData) : 0;
				size_t wDataLen = (wStatusNoteLen ? wStatusNoteLen + 4 : 0) + 4 + wStatusMoodLen + 4;

				icq_packet packet;
				serverPacketInit(&packet, wDataLen + 14);
				packFNACHeader(&packet, ICQ_SERVICE_FAMILY, ICQ_CLIENT_SET_STATUS);
				// Change only session data
				packWord(&packet, 0x1D);              // TLV 1D
				packWord(&packet, WORD(wDataLen));    // TLV length
				packWord(&packet, 0x02);              // Item Type
				if (wStatusNoteLen) {
					packWord(&packet, 0x400 | WORD(wStatusNoteLen + 4)); // Flags + Item Length
					packWord(&packet, WORD(wStatusNoteLen));  // Text Length
					packBuffer(&packet, (LPBYTE)setStatusNoteText, wStatusNoteLen);
					packWord(&packet, 0);               // Encoding not specified (utf-8 is default)
				}
				else packWord(&packet, 0);            // Flags + Item Length

				packWord(&packet, 0x0E);              // Item Type
				packWord(&packet, WORD(wStatusMoodLen));    // Flags + Item Length
				if (wStatusMoodLen)
					packBuffer(&packet, (LPBYTE)setStatusMoodData, wStatusMoodLen); // Mood

				sendServPacket(&packet);
			}
			SAFE_FREE(&szCurrentStatusNote);
			db_free(&dbv);
		}
	}
	SAFE_FREE(&setStatusNoteText);
	SAFE_FREE(&setStatusMoodData);
}


int CIcqProto::SetStatusNote(const char *szStatusNote, DWORD dwDelay, int bForce)
{
	int bChanged = FALSE;

	// bForce is intended for login sequence - need to call this earlier than icqOnline()
	// the process is delayed and icqOnline() is ready when needed inside SetStatusNoteThread()
	if (!bForce && !icqOnline()) return bChanged;

	// reuse generic critical section (used for cookies list and object variables locks)
	mir_cslock l(cookieMutex);

	if (!setStatusNoteText && (!m_bMoodsEnabled || !setStatusMoodData)) { // check if the status note was changed and if yes, create thread to change it
		char *szCurrentStatusNote = getSettingStringUtf(NULL, DBSETTING_STATUS_NOTE, NULL);

		if (strcmpnull(szCurrentStatusNote, szStatusNote)) { // status note was changed
			// create thread to change status note on existing server connection
			setStatusNoteText = null_strdup(szStatusNote);

			if (dwDelay)
				ForkThread(&CIcqProto::SetStatusNoteThread, (void*)dwDelay);
			else // we cannot afford any delay, so do not run in separate thread
				SetStatusNoteThread(NULL);

			bChanged = TRUE;
		}
		SAFE_FREE(&szCurrentStatusNote);
	}
	else { // only alter status note object with new status note, keep the thread waiting for execution
		SAFE_FREE(&setStatusNoteText);
		setStatusNoteText = null_strdup(szStatusNote);

		bChanged = TRUE;
	}

	return bChanged;
}


int CIcqProto::SetStatusMood(const char *szMoodData, DWORD dwDelay)
{
	int bChanged = FALSE;

	if (!icqOnline()) return bChanged;

	// reuse generic critical section (used for cookies list and object variables locks)
	mir_cslock l(cookieMutex);

	if (!setStatusNoteText && !setStatusMoodData) { // check if the status mood was changed and if yes, create thread to change it
		char *szCurrentStatusMood = NULL;
		DBVARIANT dbv = { DBVT_DELETED };

		if (!getString(DBSETTING_STATUS_MOOD, &dbv))
			szCurrentStatusMood = dbv.pszVal;

		if (strcmpnull(szCurrentStatusMood, szMoodData)) { // status mood was changed
			// create thread to change status mood on existing server connection
			setStatusMoodData = null_strdup(szMoodData);
			if (dwDelay)
				ForkThread(&CIcqProto::SetStatusNoteThread, (void*)dwDelay);
			else // we cannot afford any delay, so do not run in separate thread
				SetStatusNoteThread(NULL);

			bChanged = TRUE;
		}
		db_free(&dbv);
	}
	else { // only alter status mood object with new status mood, keep the thread waiting for execution
		SAFE_FREE(&setStatusMoodData);
		setStatusMoodData = null_strdup(szMoodData);

		bChanged = TRUE;
	}

	return bChanged;
}


void CIcqProto::writeDbInfoSettingTLVStringUtf(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
	oscar_tlv *pTLV = chain->getTLV(wTlv, 1);

	if (pTLV && pTLV->wLen > 0) {
		char *str = (char*)_alloca(pTLV->wLen + 1);

		memcpy(str, pTLV->pData, pTLV->wLen);
		str[pTLV->wLen] = '\0';
		db_set_utf(hContact, m_szModuleName, szSetting, str);
	}
	else delSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVWord(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
	int num = chain->getNumber(wTlv, 1);
	if (num > 0 && num != 0x7FFF)
		setWord(hContact, szSetting, num);
	else
		delSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVByte(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
	int num = chain->getNumber(wTlv, 1);
	if (num > 0 && num != 0x7F)
		setByte(hContact, szSetting, num);
	else
		delSetting(hContact, szSetting);
}


void CIcqProto::writeDbInfoSettingTLVDouble(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
	double num = chain->getDouble(wTlv, 1);
	if (num > 0)
		setSettingDouble(hContact, szSetting, num);
	else
		delSetting(hContact, szSetting);
}

void CIcqProto::writeDbInfoSettingTLVDate(MCONTACT hContact, const char* szSettingYear, const char* szSettingMonth, const char* szSettingDay, oscar_tlv_chain* chain, WORD wTlv)
{
	double time = chain->getDouble(wTlv, 1);

	if (time > 0) { // date is stored as double with unit equal to a day, incrementing since 1/1/1900 0:00 GMT
		SYSTEMTIME sTime = { 0 };
		if (VariantTimeToSystemTime(time + 2, &sTime)) {
			setWord(hContact, szSettingYear, sTime.wYear);
			setByte(hContact, szSettingMonth, (BYTE)sTime.wMonth);
			setByte(hContact, szSettingDay, (BYTE)sTime.wDay);
		}
		else {
			delSetting(hContact, szSettingYear);
			delSetting(hContact, szSettingMonth);
			delSetting(hContact, szSettingDay);
		}
	}
	else {
		delSetting(hContact, szSettingYear);
		delSetting(hContact, szSettingMonth);
		delSetting(hContact, szSettingDay);
	}
}


void CIcqProto::writeDbInfoSettingTLVBlob(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv)
{
	oscar_tlv *pTLV = chain->getTLV(wTlv, 1);

	if (pTLV && pTLV->wLen > 0)
		setSettingBlob(hContact, szSetting, pTLV->pData, pTLV->wLen);
	else
		delSetting(hContact, szSetting);
}

char* time2text(time_t time)
{
	tm *local = localtime(&time);

	if (local) {
		char *str = asctime(local);
		str[24] = '\0'; // remove new line
		return str;
	}
	
	return "<invalid>";
}


bool CIcqProto::validateStatusMessageRequest(MCONTACT hContact, WORD byMessageType)
{
	// Privacy control
	if (getByte("StatusMsgReplyCList", 0)) {
		// Don't send statusmessage to unknown contacts
		if (hContact == INVALID_CONTACT_ID)
			return false;

		// Don't send statusmessage to temporary contacts or hidden contacts
		if (db_get_b(hContact, "CList", "NotOnList", 0) ||
			db_get_b(hContact, "CList", "Hidden", 0))
			return false;

		// Don't send statusmessage to invisible contacts
		if (getByte("StatusMsgReplyVisible", 0)) {
			WORD wStatus = getContactStatus(hContact);
			if (wStatus == ID_STATUS_OFFLINE)
				return false;
		}
	}

	// Dont send messages to people you are hiding from
	if (hContact != INVALID_CONTACT_ID &&
		getWord(hContact, "ApparentMode", 0) == ID_STATUS_OFFLINE) {
		return false;
	}

	// Dont respond to request for other statuses than your current one
	if ((byMessageType == MTYPE_AUTOONLINE && m_iStatus != ID_STATUS_ONLINE) ||
		(byMessageType == MTYPE_AUTOAWAY && m_iStatus != ID_STATUS_AWAY) ||
		(byMessageType == MTYPE_AUTOBUSY && m_iStatus != ID_STATUS_OCCUPIED) ||
		(byMessageType == MTYPE_AUTONA   && m_iStatus != ID_STATUS_NA) ||
		(byMessageType == MTYPE_AUTODND  && m_iStatus != ID_STATUS_DND) ||
		(byMessageType == MTYPE_AUTOFFC  && m_iStatus != ID_STATUS_FREECHAT)) {
		return false;
	}

	if (hContact != INVALID_CONTACT_ID && m_iStatus == ID_STATUS_INVISIBLE &&
		getWord(hContact, "ApparentMode", 0) != ID_STATUS_ONLINE) {
		if (!getByte(hContact, "TemporaryVisible", 0)) { // Allow request to temporary visible contacts
			return false;
		}
	}

	// All OK!
	return true;
}


void __fastcall SAFE_DELETE(MZeroedObject **p)
{
	if (*p) {
		delete *p;
		*p = NULL;
	}
}


void __fastcall SAFE_FREE(void** p)
{
	if (*p) {
		free(*p);
		*p = NULL;
	}
}


void* __fastcall SAFE_MALLOC(size_t size)
{
	void* p = NULL;

	if (size) {
		p = malloc(size);
		if (p)
			memset(p, 0, size);
	}
	return p;
}


void* __fastcall SAFE_REALLOC(void* p, size_t size)
{
	if (p)
		return realloc(p, size);

	return SAFE_MALLOC(size);
}


DWORD ICQWaitForSingleObject(HANDLE hObject, DWORD dwMilliseconds, int bWaitAlways)
{
	DWORD dwResult;

	do { // will get WAIT_IO_COMPLETION for QueueUserAPC(), ignore it unless terminating
		dwResult = WaitForSingleObjectEx(hObject, dwMilliseconds, TRUE);
	}
	while (dwResult == WAIT_IO_COMPLETION && (bWaitAlways || !Miranda_Terminated()));

	return dwResult;
}


HANDLE NetLib_OpenConnection(HANDLE hUser, const char* szIdent, NETLIBOPENCONNECTION* nloc)
{
	Netlib_Logf(hUser, "%sConnecting to %s:%u", szIdent ? szIdent : "", nloc->szHost, nloc->wPort);

	nloc->cbSize = sizeof(NETLIBOPENCONNECTION);
	nloc->flags |= NLOCF_V2;

	return (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hUser, (LPARAM)nloc);
}


HANDLE CIcqProto::NetLib_BindPort(NETLIBNEWCONNECTIONPROC_V2 pFunc, void* lParam, WORD* pwPort, DWORD* pdwIntIP)
{
	NETLIBBIND nlb = { 0 };
	nlb.cbSize = sizeof(NETLIBBIND);
	nlb.pfnNewConnectionV2 = pFunc;
	nlb.pExtra = lParam;
	SetLastError(ERROR_INVALID_PARAMETER); // this must be here - NetLib does not set any error :((

	HANDLE hBoundPort = (HANDLE)CallService(MS_NETLIB_BINDPORT, (WPARAM)m_hDirectNetlibUser, (LPARAM)&nlb);

	if (pwPort) *pwPort = nlb.wPort;
	if (pdwIntIP) *pdwIntIP = nlb.dwInternalIP;

	return hBoundPort;
}


void NetLib_CloseConnection(HANDLE *hConnection, int bServerConn)
{
	if (*hConnection) {
		NetLib_SafeCloseHandle(hConnection);

		if (bServerConn)
			FreeGatewayIndex(*hConnection);
	}
}


void NetLib_SafeCloseHandle(HANDLE *hConnection)
{
	if (*hConnection) {
		Netlib_CloseHandle(*hConnection);
		*hConnection = NULL;
	}
}


int CIcqProto::NetLog_Direct(const char *fmt, ...)
{
	char szText[1024];

	va_list va;
	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	return CallService(MS_NETLIB_LOG, (WPARAM)m_hDirectNetlibUser, (LPARAM)szText);
}

int CIcqProto::NetLog_Uni(BOOL bDC, const char *fmt, ...)
{
	char szText[1024];
	va_list va;
	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	HANDLE hNetlib = (bDC) ? m_hDirectNetlibUser : m_hNetlibUser;
	return CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)szText);
}

char* __fastcall ICQTranslateUtf(const char *src)
{ // this takes UTF-8 strings only!!!
	char *szRes = NULL;

	if (!mir_strlen(src)) { // for the case of empty strings
		return null_strdup(src);
	}

	{ // we can use unicode translate (0.5+)
		WCHAR* usrc = make_unicode_string(src);

		szRes = make_utf8_string(TranslateW(usrc));

		SAFE_FREE((void**)&usrc);
	}
	return szRes;
}

char* __fastcall ICQTranslateUtfStatic(const char *src, char *buf, size_t bufsize)
{ // this takes UTF-8 strings only!!!
	if (mir_strlen(src)) { // we can use unicode translate (0.5+)
		WCHAR *usrc = make_unicode_string(src);

		make_utf8_string_static(TranslateW(usrc), buf, bufsize);

		SAFE_FREE((void**)&usrc);
	}
	else
		buf[0] = '\0';

	return buf;
}

char* CIcqProto::GetUserStoredPassword(char *szBuffer, size_t cbSize)
{
	if (!getSettingStringStatic(NULL, "Password", szBuffer, cbSize))
		if (mir_strlen(szBuffer))
			return szBuffer;

	return NULL;
}


char* CIcqProto::GetUserPassword(BOOL bAlways)
{
	if (m_szPassword[0] != '\0' && (m_bRememberPwd || bAlways))
		return m_szPassword;

	if (GetUserStoredPassword(m_szPassword, sizeof(m_szPassword))) {
		m_bRememberPwd = true;
		return m_szPassword;
	}

	return NULL;
}


WORD CIcqProto::GetMyStatusFlags()
{
	WORD wFlags = 0;

	// Webaware setting bit flag
	if (getByte("WebAware", 0))
		wFlags |= STATUS_WEBAWARE;

	// DC setting bit flag
	switch (getByte("DCType", 0)) {
	case 0:
		break;

	case 1:
		wFlags |= STATUS_DCCONT;
		break;

	case 2:
		wFlags |= STATUS_DCAUTH;
		break;

	default:
		wFlags |= STATUS_DCDISABLED;
		break;
	}
	return wFlags;
}


int IsValidRelativePath(const char *filename)
{
	if (strstrnull(filename, "..\\") || strstrnull(filename, "../") ||
		strstrnull(filename, ":\\") || strstrnull(filename, ":/") ||
		filename[0] == '\\' || filename[0] == '/')
		return 0; // Contains malicious chars, Failure

	return 1; // Success
}


const char* ExtractFileName(const char *fullname)
{
	const char *szFileName;

	// already is only filename
	if (((szFileName = strrchr(fullname, '\\')) == NULL) && ((szFileName = strrchr(fullname, '/')) == NULL))
		return fullname;

	return szFileName + 1;  // skip backslash
}


char* FileNameToUtf(const TCHAR *filename)
{
	WCHAR *usFileName = NULL;
	int wchars = GetLongPathName(filename, usFileName, 0);
	usFileName = (WCHAR*)_alloca((wchars + 1) * sizeof(WCHAR));
	GetLongPathName(filename, usFileName, wchars);

	return make_utf8_string(usFileName);
}


int FileAccessUtf(const char *path, int mode)
{
	size_t size = mir_strlen(path) + 2;
	TCHAR *szPath = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(path, szPath, size))
		return _taccess(szPath, mode);

	return -1;
}


int FileStatUtf(const char *path, struct _stati64 *buffer)
{
	size_t size = mir_strlen(path) + 2;
	TCHAR *szPath = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(path, szPath, size))
		return _tstati64(szPath, buffer);

	return -1;
}


int MakeDirUtf(const char *dir)
{
	int wRes = -1;
	size_t size = mir_strlen(dir) + 2;
	TCHAR *szDir = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(dir, szDir, size)) { // _tmkdir can created only one dir at once
		wRes = _tmkdir(szDir);
		// check if dir not already existed - return success if yes
		if (wRes == -1 && errno == 17 /* EEXIST */)
			wRes = 0;
		else if (wRes && errno == 2 /* ENOENT */) { // failed, try one directory less first
			char *szLast = (char*)strrchr(dir, '\\');
			if (!szLast) szLast = (char*)strrchr(dir, '/');
			if (szLast) {
				char cOld = *szLast;

				*szLast = '\0';
				if (!MakeDirUtf(dir))
					wRes = _tmkdir(szDir);

				*szLast = cOld;
			}
		}
	}

	return wRes;
}


int OpenFileUtf(const char *filename, int oflag, int pmode)
{
	size_t size = mir_strlen(filename) + 2;
	TCHAR *szFile = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(filename, szFile, size))
		return _topen(szFile, oflag, pmode);

	return -1;
}


WCHAR *GetWindowTextUcs(HWND hWnd)
{
	int nLen = GetWindowTextLengthW(hWnd);
	WCHAR *utext = (WCHAR*)SAFE_MALLOC((nLen + 2)*sizeof(WCHAR));
	GetWindowTextW(hWnd, utext, nLen + 1);
	return utext;
}


void SetWindowTextUcs(HWND hWnd, WCHAR *text)
{
	SetWindowTextW(hWnd, text);
}


char* GetWindowTextUtf(HWND hWnd)
{
	int nLen = GetWindowTextLength(hWnd);
	TCHAR *szText = (TCHAR*)_alloca((nLen + 2) * sizeof(TCHAR));

	GetWindowText(hWnd, szText, nLen + 1);

	return tchar_to_utf8(szText);
}


char* GetDlgItemTextUtf(HWND hwndDlg, int iItem)
{
	return GetWindowTextUtf(GetDlgItem(hwndDlg, iItem));
}


void SetWindowTextUtf(HWND hWnd, const char *szText)
{
	size_t size = mir_strlen(szText) + 2;
	TCHAR *tszText = (TCHAR*)_alloca(size * sizeof(TCHAR));

	if (utf8_to_tchar_static(szText, tszText, size))
		SetWindowText(hWnd, tszText);
}


void SetDlgItemTextUtf(HWND hwndDlg, int iItem, const char *szText)
{
	SetWindowTextUtf(GetDlgItem(hwndDlg, iItem), szText);
}


static int ControlAddStringUtf(HWND ctrl, DWORD msg, const char *szString)
{
	char str[MAX_PATH];
	char *szItem = ICQTranslateUtfStatic(szString, str, MAX_PATH);
	int item = -1;
	WCHAR *wItem = make_unicode_string(szItem);
	item = SendMessage(ctrl, msg, 0, (LPARAM)wItem);
	SAFE_FREE((void**)&wItem);
	return item;
}

int ComboBoxAddStringUtf(HWND hCombo, const char *szString, DWORD data)
{
	int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
}

int ListBoxAddStringUtf(HWND hList, const char *szString)
{
	return ControlAddStringUtf(hList, LB_ADDSTRING, szString);
}

int MessageBoxUtf(HWND hWnd, const char *szText, const char *szCaption, UINT uType)
{
	int res;
	char str[1024];
	char cap[MAX_PATH];
	WCHAR *text = make_unicode_string(ICQTranslateUtfStatic(szText, str, 1024));
	WCHAR *caption = make_unicode_string(ICQTranslateUtfStatic(szCaption, cap, MAX_PATH));
	res = MessageBoxW(hWnd, text, caption, uType);
	SAFE_FREE((void**)&caption);
	SAFE_FREE((void**)&text);
	return res;
}

char* CIcqProto::ConvertMsgToUserSpecificAnsi(MCONTACT hContact, const char* szMsg)
{ // this takes utf-8 encoded message
	WORD wCP = getWord(hContact, "CodePage", m_wAnsiCodepage);
	char* szAnsi = NULL;

	if (wCP != CP_ACP) // convert to proper codepage
		if (!utf8_decode_codepage(szMsg, &szAnsi, wCP))
			return NULL;

	return szAnsi;
}

// just broadcast generic send error with dummy cookie and return that cookie
DWORD CIcqProto::ReportGenericSendError(MCONTACT hContact, int nType, const char* szErrorMsg)
{
	DWORD dwCookie = GenerateCookie(0);
	SendProtoAck(hContact, dwCookie, ACKRESULT_FAILED, nType, Translate(szErrorMsg));
	return dwCookie;
}
