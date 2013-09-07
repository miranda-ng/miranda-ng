#include "Mra.h"

#define MRA_PASS_CRYPT_VER	2

struct SetXStatusData
{
	CMraProto *ppro;
	DWORD dwXStatus;
	DWORD dwCountdown;
	HICON hDlgIcon;
};

struct SetBlogStatusData
{
	CMraProto *ppro;
	HANDLE hContact;
};

#define RECURSION_DATA_STACK_ITEMS_MIN 128
struct RECURSION_DATA_STACK_ITEM
{
	HANDLE hFind;
	DWORD dwFileNameLen;
	WIN32_FIND_DATA w32fdFindFileData;
};

LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

CMStringA MraGetSelfVersionString()
{
	WORD v[4];
	DWORD dwMirVer = CallService(MS_SYSTEM_GETFILEVERSION, 0, (LPARAM)v);
	LPSTR	lpszSecIM = ServiceExists("SecureIM/IsContactSecured") ? " + SecureIM" : "";

	CMStringA szSelfVersion;
	szSelfVersion.Format("Miranda NG %lu.%lu.%lu.%lu Unicode (MRA v%lu.%lu.%lu.%lu)%s, version: %lu.%lu",
		v[0], v[1], v[2], v[3], __FILEVERSION_STRING, lpszSecIM, PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR);
	return szSelfVersion;
}

static DWORD GetParamValue(const CMStringA &szData, LPCSTR szParamName, DWORD dwParamNameSize, CMStringA &szParamValue)
{
	if (szData.IsEmpty())
		return ERROR_INVALID_HANDLE;

	char *tmp = NEWSTR_ALLOCA(szData.c_str());
	_strlwr(tmp);

	LPSTR lpszParamDataStart = strstr(tmp, szParamName);
	if (lpszParamDataStart)
	if ((*((WORD*)(lpszParamDataStart+dwParamNameSize))) == (*((WORD*)"=\""))) {
		lpszParamDataStart += dwParamNameSize+2;
		LPSTR lpszParamDataEnd = strchr(lpszParamDataStart, '"');
		if (lpszParamDataEnd) {
			szParamValue = CMStringA(szData.c_str()+(lpszParamDataStart-tmp), lpszParamDataEnd-lpszParamDataStart);
			return NO_ERROR;
		}
	}
	return ERROR_NOT_FOUND;
}

CMStringA MraGetVersionStringFromFormatted(const CMStringA &szUserAgentFormatted)
{
	if (szUserAgentFormatted.IsEmpty())
		return "";

	CMStringA res, tmp;

	if ( !GetParamValue(szUserAgentFormatted, "name", 4, tmp))
		if (tmp == "Miranda IM" || tmp == "Miranda NG") {
			GetParamValue(szUserAgentFormatted, "title", 5, res);
			return res;
		}

	if ( !GetParamValue(szUserAgentFormatted, "client", 6, tmp))
		res += tmp + " ";

	if ( !GetParamValue(szUserAgentFormatted, "name", 4, tmp))
		res += tmp + " ";

	if ( !GetParamValue(szUserAgentFormatted, "title", 5, tmp))
		res += tmp + " ";

	if ( !GetParamValue(szUserAgentFormatted, "version", 7, tmp)) {
		res += tmp + " ";
		if ( !GetParamValue(szUserAgentFormatted, "build", 5, tmp))
			res += tmp;
	}

	// no data extracted, copy raw
	return (res.IsEmpty()) ? szUserAgentFormatted : res;
}

DWORD MraAddrListGetFromBuff(const CMStringA &szAddresses, MRA_ADDR_LIST *pmalAddrList)
{
	if (szAddresses.IsEmpty() || !pmalAddrList)
		return ERROR_INVALID_HANDLE;
	
	DWORD dwAllocatedCount = ALLOCATED_COUNT;
	pmalAddrList->dwAddrCount = 0;
	pmalAddrList->pMailAddress = (MRA_ADDR_LIST_ITEM*)mir_calloc(sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount);

	LPSTR buf = NEWSTR_ALLOCA(szAddresses.c_str()), lpszCurrentItem = buf;

	while (TRUE) {
		LPSTR lpszEndItem = strchr(lpszCurrentItem, ';');
		if (lpszEndItem == NULL) lpszEndItem = buf + szAddresses.GetLength();
		if (!lpszEndItem)
			break;

		LPSTR lpszDelimiter = strchr(lpszCurrentItem, ':');
		if (!lpszDelimiter)
			break;

		if (pmalAddrList->dwAddrCount == dwAllocatedCount) {
			dwAllocatedCount += ALLOCATED_COUNT;
			pmalAddrList->pMailAddress = (MRA_ADDR_LIST_ITEM*)mir_realloc(pmalAddrList->pMailAddress, (sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount));
		}

		*lpszDelimiter = 0;
		pmalAddrList->pMailAddress[pmalAddrList->dwAddrCount].dwAddr = inet_addr(lpszCurrentItem);
		pmalAddrList->pMailAddress[pmalAddrList->dwAddrCount].dwPort = StrToUNum32((lpszDelimiter+1), (lpszEndItem-(lpszDelimiter+1)));
		*lpszDelimiter = ':';
		pmalAddrList->dwAddrCount++;
		lpszCurrentItem = lpszEndItem+1;

		if (lpszEndItem == buf + szAddresses.GetLength())
			break;
	}
	pmalAddrList->pMailAddress = (MRA_ADDR_LIST_ITEM*)mir_realloc(pmalAddrList->pMailAddress, (sizeof(MRA_ADDR_LIST_ITEM)*pmalAddrList->dwAddrCount));
	return NO_ERROR;
}

CMStringA MraAddrListGetToBuff(MRA_ADDR_LIST *pmalAddrList)
{
	if (!pmalAddrList)
		return "";

	CMStringA res;
	for (size_t i = 0; i < pmalAddrList->dwAddrCount; i++) {
		char buf[100];
		mir_snprintf(buf, sizeof(buf), "%s:%lu;", inet_ntoa((*((in_addr*)&pmalAddrList->pMailAddress[i].dwAddr))), pmalAddrList->pMailAddress[i].dwPort);
		res += buf;
	}

	return res;
}

void CMraProto::MraAddrListStoreToContact(HANDLE hContact, MRA_ADDR_LIST *pmalAddrList)
{
	if (!hContact || !pmalAddrList)
		return;

	if (!pmalAddrList->dwAddrCount)
		return;

	setDword(hContact, "OldIP", getDword(hContact, "IP", 0));
	setDword(hContact, "IP", ntohl(pmalAddrList->pMailAddress[0].dwAddr));
	if (pmalAddrList->dwAddrCount > 1) {
		setDword(hContact, "OldRealIP", getDword(hContact, "RealIP", 0));
		setDword(hContact, "RealIP", ntohl(pmalAddrList->pMailAddress[1].dwAddr));
	}
}

void MraAddrListFree(MRA_ADDR_LIST *pmalAddrList)
{
	if (pmalAddrList) {
		mir_free(pmalAddrList->pMailAddress);
		pmalAddrList->dwAddrCount = 0;
	}
}

BOOL DB_GetStaticStringA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{
	BOOL bRet = FALSE;
	size_t dwReadedStringLen, dwRetBuffSizeLocal;
	DBVARIANT dbv = {0};
	if (db_get_ws(hContact, lpszModule, lpszValueName, &dbv) == 0)
	{
		dwReadedStringLen = lstrlenW(dbv.pwszVal);
		if (lpszRetBuff && (dwRetBuffSize>dwReadedStringLen))
		{
			dwRetBuffSizeLocal = WideCharToMultiByte(MRA_CODE_PAGE, 0, dbv.pwszVal, dwReadedStringLen, lpszRetBuff, dwRetBuffSize, NULL, NULL);
			(*((CHAR*)(lpszRetBuff+dwRetBuffSizeLocal))) = 0;
			bRet = TRUE;
		}else {
			dwRetBuffSizeLocal = dwReadedStringLen;
			if (lpszRetBuff && dwRetBuffSize >= sizeof(WORD)) (*((WORD*)lpszRetBuff)) = 0;
		}
		if (pdwRetBuffSize) (*pdwRetBuffSize) = dwRetBuffSizeLocal;

		db_free(&dbv);
	}
	else {
		if (lpszRetBuff && dwRetBuffSize >= sizeof(WORD)) (*((WORD*)lpszRetBuff)) = 0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize) = 0;
	}
	return bRet;
}

// sizes in wchars
BOOL DB_GetStaticStringW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPWSTR lpwszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{
	BOOL bRet = FALSE;
	size_t dwReadedStringLen;
	DBVARIANT dbv = {0};
	if (db_get_ws(hContact, lpszModule, lpszValueName, &dbv) == 0)
	{
		dwReadedStringLen = lstrlenW(dbv.pwszVal);
		if (lpwszRetBuff && (dwRetBuffSize>dwReadedStringLen))
		{
			memmove(lpwszRetBuff, dbv.pszVal, (dwReadedStringLen*sizeof(WCHAR)));//include null terminated
			(*((WCHAR*)(lpwszRetBuff+dwReadedStringLen))) = 0;
			bRet = TRUE;
		}else {
			if (lpwszRetBuff && dwRetBuffSize >= sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff)) = 0;
		}
		if (pdwRetBuffSize) (*pdwRetBuffSize) = dwReadedStringLen;

		db_free(&dbv);
	}
	else {
		if (lpwszRetBuff && dwRetBuffSize >= sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff)) = 0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize) = 0;
	}
	return bRet;
}

BOOL DB_GetStringA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, CMStringA& res)
{
	char *szRes = db_get_sa(hContact, lpszModule, lpszValueName);
	if (szRes) {
		res = szRes;
		mir_free(szRes);
		return TRUE;
	}
	
	res.Empty();
	return FALSE;
}


BOOL DB_GetStringW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, CMStringW& res)
{
	WCHAR *szRes = db_get_wsa(hContact, lpszModule, lpszValueName);
	if (szRes) {
		res = szRes;
		mir_free(szRes);
		return TRUE;
	}
	
	res.Empty();
	return FALSE;
}

BOOL DB_SetStringExA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, const CMStringA &szValue)
{
	if (szValue.IsEmpty()) {
		db_unset(hContact, lpszModule, lpszValueName);
		return true;
	}

	return db_set_s(hContact, lpszModule, lpszValueName, szValue);
}

BOOL DB_SetStringExW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, const CMStringW &szValue)
{
	if (szValue.IsEmpty()) {
		db_unset(hContact, lpszModule, lpszValueName);
		return true;
	}

	return db_set_ws(hContact, lpszModule, lpszValueName, szValue);
}

BOOL DB_GetContactSettingBlob(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPVOID lpRet, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{
	BOOL bRet = FALSE;
	DBVARIANT dbv;
	if (db_get(hContact, lpszModule, lpszValueName, &dbv) == 0)
	{
		if (dbv.type == DBVT_BLOB)
		{
			if (dwRetBuffSize >= dbv.cpbVal)
			{
				memmove(lpRet, dbv.pbVal, dbv.cpbVal);
				bRet = TRUE;
			}
			if (pdwRetBuffSize) (*pdwRetBuffSize) = dbv.cpbVal;
		}
		db_free(&dbv);
	}
	else {
		if (pdwRetBuffSize)	(*pdwRetBuffSize) = 0;
		bRet = FALSE;
	}
	return(bRet);
}

DWORD CMraProto::GetContactFlags(HANDLE hContact)
{
	DWORD dwRet = 0;

	if (IsContactMra(hContact))
	{
		dwRet = getDword(hContact, "ContactFlags", 0);
		dwRet &= ~(CONTACT_FLAG_REMOVED|CONTACT_FLAG_GROUP|CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE|CONTACT_FLAG_IGNORE|CONTACT_FLAG_SHADOW|CONTACT_FLAG_MULTICHAT);
		dwRet |= CONTACT_FLAG_UNICODE_NAME;

		CMStringA szEmail;
		if (mraGetStringA(hContact, "e-mail", szEmail))
			if (IsEMailChatAgent(szEmail))
				dwRet |= CONTACT_FLAG_MULTICHAT;

		if (db_get_b(hContact, "CList", "Hidden", 0))
			dwRet |= CONTACT_FLAG_SHADOW;

		switch (getWord(hContact, "ApparentMode", 0)) {
		case ID_STATUS_OFFLINE:
			dwRet |= CONTACT_FLAG_INVISIBLE;
			break;
		case ID_STATUS_ONLINE:
			dwRet |= CONTACT_FLAG_VISIBLE;
			break;
		}

		if (CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, IGNOREEVENT_MESSAGE)) dwRet |= CONTACT_FLAG_IGNORE;
	}
	return(dwRet);
}

DWORD CMraProto::SetContactFlags(HANDLE hContact, DWORD dwContactFlag)
{
	if ( !IsContactMra(hContact))
		return ERROR_INVALID_HANDLE;

	setDword(hContact, "ContactFlags", dwContactFlag);

	if (dwContactFlag&CONTACT_FLAG_SHADOW)
		db_set_b(hContact, "CList", "Hidden", 1);
	else
		db_unset(hContact, "CList", "Hidden");

	switch (dwContactFlag&(CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE)) {
	case CONTACT_FLAG_INVISIBLE:
		setWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
		break;
	case CONTACT_FLAG_VISIBLE:
		setWord(hContact, "ApparentMode", ID_STATUS_ONLINE);
		break;
	default:
		setWord(hContact, "ApparentMode", 0);
		break;
	}

	if (dwContactFlag&CONTACT_FLAG_IGNORE)
		CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, IGNOREEVENT_MESSAGE);
	else
		CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, IGNOREEVENT_MESSAGE);

	return 0;
}

DWORD CMraProto::GetContactBasicInfoW(HANDLE hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, CMStringA *szEmail, CMStringW *wszNick, CMStringA *szPhones)
{
	if ( !IsContactMra(hContact))
		return ERROR_INVALID_HANDLE;

	if (pdwID)
		*pdwID = getDword(hContact, "ContactID", -1);
	if (pdwGroupID)
		*pdwGroupID = getDword(hContact, "GroupID", -1);
	if (pdwContactSeverFlags)
		*pdwContactSeverFlags = getDword(hContact, "ContactSeverFlags", 0);
	if (pdwStatus)
		*pdwStatus = MraGetContactStatus(hContact);
	if (pdwContactFlag)
		*pdwContactFlag = GetContactFlags(hContact);
	if (szEmail)
		mraGetStringA(hContact, "e-mail", *szEmail);
	if (wszNick)
		DB_GetStringW(hContact, "CList", "MyHandle", *wszNick);

	if (szPhones) {
		CMStringA szPhone;

		for (int i = 0; i < 3; i++) {
			char szValue[50];
			mir_snprintf(szValue, SIZEOF(szValue), "MyPhone%d", i);
			if ( DB_GetStringA(hContact, "UserInfo", szValue, szPhone)) {
				if (szPhones->GetLength())
					szPhones->AppendChar(',');
				szPhones->Append(szPhone);
			}
		}
	}
	return 0;
}

DWORD CMraProto::SetContactBasicInfoW(HANDLE hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, const CMStringA *szEmail, const CMStringW *wszNick, const CMStringA *szPhones)
{
	if ( !IsContactMra(hContact))
		return ERROR_INVALID_HANDLE;

	// LOCK
	if (dwSetInfoFlags & SCBIFSI_LOCK_CHANGES_EVENTS)
		setDword(hContact, "HooksLocked", TRUE);

	// поля которые нужны, и изменения которых не отслеживаются
	if (dwFlags & SCBIF_ID)
		setDword(hContact, "ContactID", dwID);

	if ((dwFlags & SCBIF_EMAIL) && szEmail != NULL && !szEmail->IsEmpty())
		mraSetStringExA(hContact, "e-mail", *szEmail);

	// поля изменения которых отслеживаются
	if (dwFlags & SCBIF_GROUP_ID) {
		setDword(hContact, "GroupID", dwGroupID);

		MraGroupItem *grp = m_groups.find((MraGroupItem*)&dwGroupID);
		if (grp)
			db_set_ts(hContact, "CList", "Group", grp->m_name);
	}

	if ((dwFlags & SCBIF_NICK) && wszNick != NULL && !wszNick->IsEmpty()) {
		if ((dwFlags & SCBIF_FLAG) && ((dwContactFlag & CONTACT_FLAG_UNICODE_NAME) == 0))
			DB_SetStringExA(hContact, "CList", "MyHandle", CMStringA(*wszNick));
		else
			DB_SetStringExW(hContact, "CList", "MyHandle", *wszNick);
	}

	if ((dwFlags & SCBIF_PHONES) && szPhones != NULL && !szPhones->IsEmpty()) {
		int iStart = 0, i = 0;
		while (true) {
			CMStringA szPhone = szPhones->Tokenize(",", iStart);
			if (iStart == -1)
				break;

			char szValue[MAX_PATH];
			mir_snprintf(szValue, SIZEOF(szValue), "MyPhone%d", i++);
			DB_SetStringExA(hContact, "UserInfo", szValue, "+" + szPhone);
		}
	}

	if (dwFlags & SCBIF_FLAG)
		SetContactFlags(hContact, dwContactFlag);

	if (dwFlags & SCBIF_SERVER_FLAG)
		setDword(hContact, "ContactSeverFlags", dwContactSeverFlags);

	if (dwFlags & SCBIF_STATUS)
		MraSetContactStatus(hContact, dwStatus);

	SetExtraIcons(hContact);
	// UNLOCK
	if (dwSetInfoFlags & SCBIFSI_LOCK_CHANGES_EVENTS)
		setDword(hContact, "HooksLocked", FALSE);

	return 0;
}

HANDLE CMraProto::MraHContactFromEmail(const CMStringA& szEmail, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded)
{
	if ( szEmail.IsEmpty())
		return NULL;

	HANDLE hContact = NULL;
	bool bFound = false;

	//check not already on list
	CMStringA szEMailLocal;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (mraGetStringA(hContact, "e-mail", szEMailLocal))
			if (szEMailLocal == szEmail) {
				if (bTemporary == FALSE)
					db_unset(hContact, "CList", "NotOnList");
				bFound = true;
				break;
			}

	if (!bFound && bAddIfNeeded) {
		//not already there: add
		if (IsEMailChatAgent(szEmail)) {
			GCSESSION gcw = {0};
			CMStringW wszEMail = szEmail;

			gcw.cbSize = sizeof(GCSESSION);
			gcw.iType = GCW_CHATROOM;
			gcw.pszModule = m_szModuleName;
			gcw.ptszName = wszEMail;
			gcw.ptszID = (LPWSTR)wszEMail.c_str();
			gcw.dwFlags = GC_UNICODE;

			if (CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw) == 0) {
				BOOL bChatAdded = FALSE;
				for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
					if (mraGetStringA(hContact, "ChatRoomID", szEMailLocal)) {
						if (szEMailLocal == szEmail) {
							bChatAdded = TRUE;
							break;
						}
					}
				}
				if (bChatAdded == FALSE)
					hContact = NULL;
			}
		}
		else {
			hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);
		}

		if (hContact) {
			if ( IsEMailChatAgent(szEmail))
				SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL), -1, -1, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, ID_STATUS_ONLINE, &szEmail, 0, 0);
			else {
				if (bTemporary)
					db_set_b(hContact, "CList", "NotOnList", 1);
				mraSetStringExA(hContact, "MirVer", MIRVER_UNKNOWN);
				SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL), -1, -1, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, ID_STATUS_OFFLINE, &szEmail, 0, 0);
			}
		}
	}

	if (pbAdded)
		*pbAdded = (bFound == FALSE && bAddIfNeeded && hContact);

	return hContact;
}

bool CMraProto::MraUpdateContactInfo(HANDLE hContact)
{
	if (m_bLoggedIn && hContact)
	if (IsContactMra(hContact)) {
		CMStringA szEmail;
		if (mraGetStringA(hContact, "e-mail", szEmail)) {
			MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, GAIF_FORCE, hContact, 0);
			if ( MraWPRequestByEMail(hContact, ACKTYPE_GETINFO, szEmail))
				return true;
		}
	}
	return false;
}

DWORD CMraProto::MraContactCapabilitiesGet(HANDLE hContact)
{
	return getDword(hContact, DBSETTING_CAPABILITIES, 0);
}

void CMraProto::MraContactCapabilitiesSet(HANDLE hContact, DWORD dwFutureFlags)
{
	setDword(hContact, DBSETTING_CAPABILITIES, dwFutureFlags);
}

DWORD CMraProto::MraGetContactStatus(HANDLE hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

DWORD CMraProto::MraSetContactStatus(HANDLE hContact, DWORD dwNewStatus)
{
	DWORD dwOldStatus = MraGetContactStatus(hContact);

	if (dwNewStatus != dwOldStatus) {
		bool bChatAgent = IsContactChatAgent(hContact);
		if (dwNewStatus == ID_STATUS_OFFLINE) {
			if (hContact) {
				setByte(hContact, DBSETTING_XSTATUSID, MRA_MIR_XSTATUS_NONE);
				delSetting(hContact, DBSETTING_XSTATUSNAME);
				delSetting(hContact, DBSETTING_XSTATUSMSG);
				delSetting(hContact, DBSETTING_BLOGSTATUSTIME);
				delSetting(hContact, DBSETTING_BLOGSTATUSID);
				delSetting(hContact, DBSETTING_BLOGSTATUS);
				delSetting(hContact, DBSETTING_BLOGSTATUSMUSIC);
				MraContactCapabilitiesSet(hContact, 0);
				if (bChatAgent)
					MraChatSessionDestroy(hContact);
			}
			setDword(hContact, "LogonTS", 0);
			delSetting(hContact, "IP");
			delSetting(hContact, "RealIP");
		}
		else {
			if (dwOldStatus == ID_STATUS_OFFLINE) {
				DWORD dwTime = (DWORD)_time32(NULL);
				setDword(hContact, "LogonTS", dwTime);
				setDword(hContact, "OldLogonTS", dwTime);

				if (bChatAgent)
					MraChatSessionNew(hContact);
			}
			MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, 0, hContact, 0);
		}

		setWord(hContact, "Status", (WORD)dwNewStatus);
	}
	return dwOldStatus;
}

void CMraProto::MraUpdateEmailStatus(const CMStringA &pszFrom, const CMStringA &pszSubject, DWORD dwDate, DWORD dwUIDL)
{
	BOOL bTrayIconNewMailNotify;
	WCHAR szStatusText[MAX_SECONDLINE];

	bTrayIconNewMailNotify = getByte("TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY);

	if (m_dwEmailMessagesUnread) {
		CMStringA szEmail;
		HANDLE hContact = NULL;
		WCHAR szMailBoxStatus[MAX_SECONDLINE];

		mir_sntprintf(szMailBoxStatus, SIZEOF(szMailBoxStatus), TranslateW(L"Unread mail is available: %lu/%lu messages"), m_dwEmailMessagesUnread, dwEmailMessagesTotal);

		if ( !pszFrom.IsEmpty() || !pszSubject.IsEmpty()) {
			CMStringA szFrom, szSubject;
			if ( GetEMailFromString(szFrom, szEmail))
				hContact = MraHContactFromEmail(szEmail, FALSE, TRUE, NULL);

			mir_sntprintf(szStatusText, SIZEOF(szStatusText), TranslateW(L"From: %S\r\nSubject: %S\r\n%s"), pszFrom.c_str(), szSubject.c_str(), szMailBoxStatus);
		}
		else lstrcpynW(szStatusText, szMailBoxStatus, SIZEOF(szStatusText));

		if (bTrayIconNewMailNotify) {
			char szServiceFunction[MAX_PATH], *pszServiceFunctionName;
			CLISTEVENT cle = {0};

			cle.cbSize = sizeof(cle);
			//cle.hContact;
			//cle.hDbEvent;
			cle.lpszProtocol = m_szModuleName;
			cle.hIcon = IconLibGetIcon( gdiMenuItems[0].hIconHandle );
			cle.flags = (CLEF_UNICODE|CLEF_PROTOCOLGLOBAL);
			cle.pszService = "";
			cle.ptszTooltip = szStatusText;

			if (getByte("TrayIconNewMailClkToInbox", MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX))
			{
				strncpy(szServiceFunction, m_szModuleName, MAX_PATH);
				pszServiceFunctionName = szServiceFunction+strlen(m_szModuleName);
				memmove(pszServiceFunctionName, MRA_GOTO_INBOX, sizeof(MRA_GOTO_INBOX));
				cle.pszService = szServiceFunction;
			}
			CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
		}

		SkinPlaySound(szNewMailSound);
		if (hContact)
		{// update user info
			MraUpdateContactInfo(hContact);
			MraPopupShowFromContactW(hContact, MRA_POPUP_TYPE_EMAIL_STATUS, (MRA_POPUP_ALLOW_ENTER), szStatusText);
		}else {
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_EMAIL_STATUS, (MRA_POPUP_ALLOW_ENTER), szStatusText);
		}
	}else {
		if (getByte("IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY))
		{
			if (bTrayIconNewMailNotify) CallService(MS_CLIST_REMOVEEVENT, 0, (LPARAM)m_szModuleName);
			PUDeletePopup(hWndEMailPopupStatus);
			hWndEMailPopupStatus = NULL;
		}else {
			mir_sntprintf(szStatusText, SIZEOF(szStatusText), TranslateW(L"No unread mail is available\r\nTotal messages: %lu"), dwEmailMessagesTotal);
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_EMAIL_STATUS, (MRA_POPUP_ALLOW_ENTER), szStatusText);
		}
	}
}

bool IsHTTPSProxyUsed(HANDLE m_hNetlibUser)
{
	NETLIBUSERSETTINGS nlus = { sizeof(nlus) };
	if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)m_hNetlibUser, (LPARAM)&nlus))
		if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTPS)
			return true;

	return false;
}

// определяет принадлежность контакта данной копии плагина
bool CMraProto::IsContactMra(HANDLE hContact)
{
	return CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName) != 0;
}

// определяется является ли контакт контактом MRA протокола, не зависимо от того какому плагину он принадлежит
bool IsContactMraProto(HANDLE hContact)
{
	LPSTR lpszProto = GetContactProto(hContact);
	if (lpszProto) {
		CMStringW szBuff;
		if ( DB_GetStringW(hContact, lpszProto, "AvatarLastCheckTime", szBuff))
		if ( DB_GetStringW(hContact, lpszProto, "AvatarLastModifiedTime", szBuff))
			return true;
	}
	return false;
}

bool CMraProto::IsEMailMy(const CMStringA &szEmail)
{
	if (!szEmail.IsEmpty()) {
		CMStringA szEmailMy;
		if (mraGetStringA(NULL, "e-mail", szEmailMy)) {
			if (szEmail.GetLength() == szEmailMy.GetLength())
			if (!_stricmp(szEmail, szEmailMy))
				return true;
		}
	}
	return false;
}


bool CMraProto::IsEMailChatAgent(const CMStringA &szEmail)
{
	if (!szEmail.IsEmpty()) {
		CMStringA domain = szEmail.Right(sizeof(MAILRU_CHAT_CONF_DOMAIN));
		if (domain[0] == '@' && _stricmp(domain.c_str()+1, MAILRU_CHAT_CONF_DOMAIN))
			return true;
	}
	return false;
}

bool CMraProto::IsContactChatAgent(HANDLE hContact)
{
	if (hContact == NULL)
		return false;

	CMStringA szEmail;
	return mraGetStringA(hContact, "e-mail", szEmail) ? IsEMailChatAgent(szEmail) : false;
}

bool IsEMailMR(const CMStringA &szEmail)
{
	if (szEmail) {
		for (int i = 0; lpcszMailRuDomains[i]; i++) {
			int dwDomainLen = lstrlenA(lpcszMailRuDomains[i]);
			if (dwDomainLen < szEmail.GetLength())
			if (!_stricmp(lpcszMailRuDomains[i], szEmail.c_str()+szEmail.GetLength()-dwDomainLen))
			if (szEmail[szEmail.GetLength()-dwDomainLen-1] == '@')
				return true;
		}
	}
	return false;
}

bool GetEMailFromString(const CMStringA& szBuff, CMStringA& szEmail)
{
	if (!szBuff.IsEmpty()) {
		int Start, End;
		if ((Start = szBuff.Find('<')) != -1) {
			Start++;
			if ((End = szBuff.Find('>', Start)) != -1) {
				szEmail = szBuff.Mid(Start, End-Start-1);
				return true;
			}
		}
	}

	szEmail.Empty();
	return false;
}

DWORD GetContactEMailCountParam(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName)
{
	DWORD dwRet = 0;
	CMStringA szEmail;

	if ( DB_GetStringA(hContact, lpszModule, lpszValueName, szEmail))
		if (bMRAOnly == FALSE || IsEMailMR(szEmail))
			dwRet++;

	for (int i=0; TRUE; i++) {
		char szBuff[100];
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if (DB_GetStringA(hContact, lpszModule, szBuff, szEmail)) {
			if (bMRAOnly == FALSE || IsEMailMR(szEmail))
				dwRet++;
		}
		else {
			if (i > EMAILS_MIN_COUNT)
				break;
		}
	}
	return dwRet;
}

DWORD CMraProto::GetContactEMailCount(HANDLE hContact, BOOL bMRAOnly)
{
	LPSTR lpszProto = (hContact) ? GetContactProto(hContact) : m_szModuleName;

	DWORD dwRet = 0;
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, lpszProto, "e-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "e-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "Mye-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "Companye-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "MyCompanye-mail");
	return dwRet;
}

bool GetContactFirstEMailParam(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName, CMStringA &res)
{
	CMStringA szEmail;

	if ( DB_GetStringA(hContact, lpszModule, lpszValueName, szEmail))
		if (bMRAOnly == FALSE || IsEMailMR(szEmail)) {
			res = szEmail;
			return true;
		}

	for (int i = 0; true; i++) {
		char szBuff[100];
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if ( DB_GetStringA(hContact, lpszModule, szBuff, szEmail)) {
			if (bMRAOnly == FALSE || IsEMailMR(szEmail)) {
				res = szEmail;
				return true;
			}
		}
		else if (i>EMAILS_MIN_COUNT)
			break;
	}
	return false;
}

bool CMraProto::GetContactFirstEMail(HANDLE hContact, BOOL bMRAOnly, CMStringA &res)
{
	LPSTR lpszProto = (hContact) ? GetContactProto(hContact) : m_szModuleName;

	bool bRet = GetContactFirstEMailParam(hContact, bMRAOnly, lpszProto, "e-mail", res);
	if (!bRet) bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "e-mail", res);
	if (!bRet) bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "Mye-mail", res);
	if (!bRet) bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "Companye-mail", res);
	if (!bRet) bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "MyCompanye-mail", res);
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMraProto::ShowFormattedErrorMessage(LPWSTR lpwszErrText, DWORD dwErrorCode)
{
	WCHAR szErrorText[2048], szErrDescription[1024];
	size_t dwErrDescriptionSize;

	if (dwErrorCode == NO_ERROR)
		mir_sntprintf(szErrorText, SIZEOF(szErrorText), L"%s", TranslateW(lpwszErrText));
	else {
		dwErrDescriptionSize = (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, szErrDescription, (SIZEOF(szErrDescription)-sizeof(WCHAR)), NULL)-2);
		szErrDescription[dwErrDescriptionSize] = 0;
		mir_sntprintf(szErrorText, SIZEOF(szErrorText), L"%s %lu: %s", TranslateW(lpwszErrText), dwErrorCode, szErrDescription);
	}
	MraPopupShowFromAgentW(MRA_POPUP_TYPE_ERROR, 0, szErrorText);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void FakeThread(void* param)
{
	Sleep(100);
	CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)param);
	mir_free(param);
}

DWORD CMraProto::ProtoBroadcastAckAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)::mir_calloc(sizeof(ACKDATA));
	ack->cbSize = sizeof(ACKDATA);
	ack->szModule = m_szModuleName;
	ack->hContact = hContact;
	ack->type = type;
	ack->result = hResult;
	ack->hProcess = hProcess;
	ack->lParam = lParam;
	mir_forkthread(FakeThread, ack);
	return 0;
}

CMStringA CMraProto::CreateBlobFromContact(HANDLE hContact, const CMStringW &wszRequestReason)
{
	CMStringA res('\0', 8), tmp;
	DWORD *p = (DWORD*)res.c_str();
	p[0] = 0; p[1] = (DWORD)hContact;

	mraGetStringA(hContact, "Nick", tmp);
	res += tmp; res.AppendChar(0);

	mraGetStringA(hContact, "FirstName", tmp);
	res += tmp; res.AppendChar(0);

	mraGetStringA(hContact, "LastName", tmp);
	res += tmp; res.AppendChar(0);

	mraGetStringA(hContact, "e-mail", tmp);
	res += tmp; res.AppendChar(0);

	tmp = wszRequestReason;
	res += tmp; res.AppendChar(0);
	return res;
}

CMStringA CopyNumber(const CMStringA &str)
{
	CMStringA res;

	for (LPCSTR p = str; *p; p++)
		if (*p >= '0' && *p <= '9')
			res.AppendChar(*p);

	return res;
}

void EnableControlsArray(HWND hWndDlg, WORD *pwControlsList, size_t dwControlsListCount, BOOL bEnabled)
{
	for (size_t i = 0; i < dwControlsListCount; i++)
		EnableWindow(GetDlgItem(hWndDlg, pwControlsList[i]), bEnabled);
}

LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CHAR)
	if (GetKeyState(VK_CONTROL) & 0x8000) {
		if (wParam == '\n') {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		if (wParam == 1) { // ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 23) { // ctrl-w
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
	}

	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SetXStatusDlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR iRet = FALSE;
	SetXStatusData *dat = (SetXStatusData*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:// set our xStatus
		TranslateDialogDefault(hWndDlg);

		dat = (SetXStatusData*)lParam;
		if (dat) {
			char szValueName[MAX_PATH];
			CMStringW szBuff;

			dat->hDlgIcon = IconLibGetIcon(hXStatusAdvancedStatusIcons[dat->dwXStatus]);
			dat->dwCountdown = 5;

			mir_subclassWindow( GetDlgItem(hWndDlg, IDC_XTITLE), MessageEditSubclassProc);
			mir_subclassWindow( GetDlgItem(hWndDlg, IDC_XMSG),   MessageEditSubclassProc);

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_XTITLE, EM_LIMITTEXT, STATUS_TITLE_MAX, 0);
			SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_XMSG, EM_LIMITTEXT, STATUS_DESC_MAX, 0);
			SendMessage(hWndDlg, WM_SETICON, ICON_BIG, (LPARAM)dat->hDlgIcon);
			SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)TranslateW(lpcszXStatusNameDef[dat->dwXStatus]));

			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldName", dat->dwXStatus);
			if (dat->ppro->mraGetStringW(NULL, szValueName, szBuff))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, szBuff.c_str()); // custom xstatus name
			else // default xstatus name
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, TranslateW(lpcszXStatusNameDef[dat->dwXStatus]));

			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dat->dwXStatus);
			if (dat->ppro->mraGetStringW(NULL, szValueName, szBuff))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XMSG, szBuff.c_str()); // custom xstatus description
			else // default xstatus description
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XMSG, L"");

			SendMessage(hWndDlg, WM_TIMER, 0, 0);
			SetTimer(hWndDlg, 1, 1000, 0);
			iRet = TRUE;
		}
		break;

	case WM_TIMER:
		if (dat->dwCountdown != -1) {
			WCHAR szBuff[MAX_PATH];
			mir_sntprintf(szBuff, SIZEOF(szBuff), L"%s %ld", TranslateW(L"Closing in"), dat->dwCountdown);
			SET_DLG_ITEM_TEXT(hWndDlg, IDOK, szBuff);
			dat->dwCountdown--;
			break;
		}

	case WM_CLOSE:
		DestroyWindow(hWndDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hWndDlg);
			break;
		case IDC_XTITLE:
		case IDC_XMSG:
			KillTimer(hWndDlg, 1);
			SET_DLG_ITEM_TEXTW(hWndDlg, IDOK, TranslateW(L"OK"));
			break;
		}
		break;

	case WM_DESTROY:
		if (dat) { // set our xStatus
			char szValueName[MAX_PATH];
			HWND hWndEdit;
			WCHAR szBuff[STATUS_TITLE_MAX+STATUS_DESC_MAX];
			WNDPROC OldMessageEditProc;

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)0);

			hWndEdit = GetDlgItem(hWndDlg, IDC_XTITLE);
			OldMessageEditProc = (WNDPROC)GetWindowLongPtr(hWndEdit, GWLP_USERDATA);
			SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)OldMessageEditProc);
			SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)0);

			hWndEdit = GetDlgItem(hWndDlg, IDC_XMSG);
			OldMessageEditProc = (WNDPROC)GetWindowLongPtr(hWndEdit, GWLP_USERDATA);
			SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)OldMessageEditProc);
			SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)0);

			DWORD dwBuffSize = GET_DLG_ITEM_TEXT(hWndDlg, IDC_XMSG, szBuff, (STATUS_DESC_MAX+1));
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dat->dwXStatus);
			dat->ppro->mraSetStringExW(NULL, szValueName, szBuff);
			dat->ppro->mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, szBuff);

			dwBuffSize = GET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, szBuff, (STATUS_TITLE_MAX+1));
			if (dwBuffSize == 0) { // user delete all text
				lstrcpynW(szBuff, TranslateW(lpcszXStatusNameDef[dat->dwXStatus]), (STATUS_TITLE_MAX+1));
				dwBuffSize = lstrlenW(szBuff);
			}
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%dName", dat->dwXStatus);
			dat->ppro->mraSetStringExW(NULL, szValueName, szBuff);
			dat->ppro->mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, szBuff);

			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = (CMIM_NAME|CMIF_UNICODE);
			mi.ptszName = szBuff;
			Menu_ModifyItem(dat->ppro->hXStatusMenuItems[dat->dwXStatus], &mi);

			dat->ppro->MraSetXStatusInternal(dat->dwXStatus);

			mir_free(dat);
		}
		EndDialog(hWndDlg, NO_ERROR);
		break;
	}
	return iRet;
}

bool CMraProto::MraRequestXStatusDetails(DWORD dwXStatus)
{
	if ( IsXStatusValid(dwXStatus)) {
		SetXStatusData *dat = (SetXStatusData*)mir_calloc(sizeof(SetXStatusData));
		dat->dwXStatus = dwXStatus;
		dat->ppro = this;
		return DialogBoxParam(masMraSettings.hInstance, MAKEINTRESOURCE(IDD_SETXSTATUS), NULL, SetXStatusDlgProc, (LPARAM)dat) != -1;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SendReplyBlogStatusDlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR iRet = FALSE;
	SetBlogStatusData *dat = (SetBlogStatusData*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (message) {
    case WM_INITDIALOG:// set our xStatus
		TranslateDialogDefault(hWndDlg);

		dat = (SetBlogStatusData*)lParam;
		if (dat) {
			SYSTEMTIME stBlogStatusTime = {0};

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			HWND hWndEdit = GetDlgItem(hWndDlg, IDC_MSG_TO_SEND);
			WNDPROC OldMessageEditProc = (WNDPROC)SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)MessageEditSubclassProc);
			SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)OldMessageEditProc);
			SendMessage(hWndEdit, EM_LIMITTEXT, MICBLOG_STATUS_MAX, 0);

			SendMessage(hWndDlg, WM_SETICON, ICON_BIG, (LPARAM)IconLibGetIcon(gdiMenuItems[5].hIconHandle));

			// blog status message
			CMStringW szBuff;
			if (dat->ppro->mraGetStringW(dat->hContact, DBSETTING_BLOGSTATUS, szBuff))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_USER_BLOG_STATUS_MSG, szBuff.c_str());

			// reply to some user blog
			if (dat->hContact) {
				szBuff.Format( TranslateW(L"Reply to %s blog status"), GetContactNameW(dat->hContact));
				SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)szBuff.c_str());
			}
			else SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)TranslateW(L"Set my blog status"));

			DWORD dwTime = dat->ppro->getDword(dat->hContact, DBSETTING_BLOGSTATUSTIME, 0);
			if (dwTime && MakeLocalSystemTimeFromTime32(dwTime, &stBlogStatusTime))
				szBuff.Format(L"%s: %04ld.%02ld.%02ld %02ld:%02ld", TranslateW(L"Writed"),
					stBlogStatusTime.wYear, stBlogStatusTime.wMonth, stBlogStatusTime.wDay, stBlogStatusTime.wHour, stBlogStatusTime.wMinute);
			else
				szBuff.Empty();

			SET_DLG_ITEM_TEXT(hWndDlg, IDC_STATIC_WRITED_TIME, szBuff.c_str());
			EnableWindow(GetDlgItem(hWndDlg, IDC_CHK_NOTIFY), (dat->hContact == NULL));
			iRet = TRUE;
		}
		break;

	 case WM_CLOSE:
		 DestroyWindow(hWndDlg);
		 break;

	 case WM_COMMAND:
		 switch (LOWORD(wParam)) {
		 case IDOK:
			 {
				 DWORD dwFlags;
				 WCHAR szBuff[MICBLOG_STATUS_MAX];
				 size_t dwBuffSize;
				 DWORDLONG dwBlogStatusID;

				 dwBuffSize = GET_DLG_ITEM_TEXT(hWndDlg, IDC_MSG_TO_SEND, szBuff, SIZEOF(szBuff));
				 if (dat->hContact) {
					 dwFlags = (MRIM_BLOG_STATUS_REPLY|MRIM_BLOG_STATUS_NOTIFY);
					 
					 DBVARIANT dbv;
					 dbv.type = DBVT_BLOB;
					 dbv.pbVal = (PBYTE)&dwBlogStatusID;
					 dbv.cpbVal = sizeof(DWORDLONG);
					 db_get(dat->hContact, dat->ppro->m_szModuleName, DBSETTING_BLOGSTATUSID, &dbv);
				 }
				 else {
					 dwFlags = MRIM_BLOG_STATUS_UPDATE;
					 if (IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_NOTIFY)) dwFlags |= MRIM_BLOG_STATUS_NOTIFY;
					 dwBlogStatusID = 0;
				 }
				 dat->ppro->MraChangeUserBlogStatus(dwFlags, szBuff, dwBlogStatusID);
			 }
		 case IDCANCEL:
			 DestroyWindow(hWndDlg);
			 break;

		 case IDC_MSG_TO_SEND:
			 if (HIWORD(wParam) == EN_CHANGE) {
				 WCHAR wszBuff[MAX_PATH];
				 size_t dwMessageSize = GET_DLG_ITEM_TEXT_LENGTH(hWndDlg, IDC_MSG_TO_SEND);

				 EnableWindow(GetDlgItem(hWndDlg, IDOK), dwMessageSize);
				 mir_sntprintf(wszBuff, SIZEOF(wszBuff), L"%d/%d", dwMessageSize, MICBLOG_STATUS_MAX);
				 SET_DLG_ITEM_TEXTW(hWndDlg, IDC_STATIC_CHARS_COUNTER, wszBuff);
			 }
			 break;
		 }
		 break;

	 case WM_DESTROY:
		 if (dat) {
			 HWND hWndEdit;
			 WNDPROC OldMessageEditProc;

			 SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)0);

			 hWndEdit = GetDlgItem(hWndDlg, IDC_MSG_TO_SEND);
			 OldMessageEditProc = (WNDPROC)GetWindowLongPtr(hWndEdit, GWLP_USERDATA);
			 SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)OldMessageEditProc);
			 SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)0);
			 mir_free(dat);
		 }
		 EndDialog(hWndDlg, NO_ERROR);
		 break;
	}
	return iRet;
}

bool CMraProto::MraSendReplyBlogStatus(HANDLE hContact)
{
	SetBlogStatusData* dat = (SetBlogStatusData*)mir_calloc( sizeof(SetBlogStatusData));
	dat->ppro = this;
	dat->hContact = hContact;
	return CreateDialogParam(masMraSettings.hInstance, MAKEINTRESOURCE(IDD_MINIBLOG), NULL, SendReplyBlogStatusDlgProc, (LPARAM)dat) != NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD GetYears(CONST PSYSTEMTIME pcstSystemTime)
{
	DWORD dwRet = 0;

	if (pcstSystemTime) {
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		dwRet = stTime.wYear - pcstSystemTime->wYear;
		// день рожденья будет в след месяце
		if (stTime.wMonth<pcstSystemTime->wMonth)
			dwRet--;
		// др ещё будет в этом месяце или уже был...
		else {
			// др в этом месяце
			if (stTime.wMonth == pcstSystemTime->wMonth)
				// ещё только будет, не сегодня
				if (stTime.wDay<pcstSystemTime->wDay)
					dwRet--;
		}
	}
	return dwRet;
}

/////////////////////////////////////////////////////////////////////////////////////////

DWORD FindFile(LPWSTR lpszFolder, DWORD dwFolderLen, LPWSTR lpszFileName, DWORD dwFileNameLen, LPWSTR lpszRetFilePathName, DWORD dwRetFilePathLen, DWORD *pdwRetFilePathLen)
{
	DWORD dwRetErrorCode;

	if (lpszFolder && dwFolderLen && lpszFileName && dwFileNameLen) {
		WCHAR szPath[32768];
		DWORD dwPathLen, dwRecDeepAllocated, dwRecDeepCurPos, dwFilePathLen;
		RECURSION_DATA_STACK_ITEM *prdsiItems;

		if (dwFolderLen == -1) dwFolderLen = lstrlenW(lpszFolder);
		if (dwFileNameLen == -1) dwFileNameLen = lstrlenW(lpszFileName);

		dwRecDeepCurPos = 0;
		dwRecDeepAllocated = RECURSION_DATA_STACK_ITEMS_MIN;
		prdsiItems = (RECURSION_DATA_STACK_ITEM*)mir_calloc(dwRecDeepAllocated*sizeof(RECURSION_DATA_STACK_ITEM));
		if (prdsiItems) {
			dwPathLen = dwFolderLen;
			memmove(szPath, lpszFolder, (dwPathLen*sizeof(WCHAR)));
			if (szPath[(dwPathLen-1)] != '\\') {
				szPath[dwPathLen] = '\\';
				dwPathLen++;
			}
			szPath[dwPathLen] = 0;
			lstrcatW(szPath, L"*.*");

			prdsiItems[dwRecDeepCurPos].dwFileNameLen = 0;
			prdsiItems[dwRecDeepCurPos].hFind = FindFirstFileEx(szPath, FindExInfoStandard, &prdsiItems[dwRecDeepCurPos].w32fdFindFileData, FindExSearchNameMatch, NULL, 0);
			if (prdsiItems[dwRecDeepCurPos].hFind != INVALID_HANDLE_VALUE) {
				dwRetErrorCode = ERROR_FILE_NOT_FOUND;
				do {
					dwPathLen-=prdsiItems[dwRecDeepCurPos].dwFileNameLen;

					while (dwRetErrorCode == ERROR_FILE_NOT_FOUND && FindNextFile(prdsiItems[dwRecDeepCurPos].hFind, &prdsiItems[dwRecDeepCurPos].w32fdFindFileData)) {
						if (prdsiItems[dwRecDeepCurPos].w32fdFindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) {// folder
							if (CompareString( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName, -1, L".", 1) != CSTR_EQUAL)
							if (CompareString( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName, -1, L"..", 2) != CSTR_EQUAL) {
								prdsiItems[dwRecDeepCurPos].dwFileNameLen = (lstrlenW(prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName)+1);
								memmove((szPath+dwPathLen), prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName, (prdsiItems[dwRecDeepCurPos].dwFileNameLen*sizeof(WCHAR)));
								lstrcatW(szPath, L"\\*.*");
								dwPathLen += prdsiItems[dwRecDeepCurPos].dwFileNameLen;

								dwRecDeepCurPos++;
								if (dwRecDeepCurPos == dwRecDeepAllocated) { // need more space
									dwRecDeepAllocated += RECURSION_DATA_STACK_ITEMS_MIN;
									prdsiItems = (RECURSION_DATA_STACK_ITEM*)mir_realloc(prdsiItems, dwRecDeepAllocated*sizeof(RECURSION_DATA_STACK_ITEM));
									if (prdsiItems == NULL) {
										dwRecDeepCurPos = 0;
										dwRetErrorCode = GetLastError();
										break;
									}
								}
								prdsiItems[dwRecDeepCurPos].hFind = FindFirstFileEx(szPath, FindExInfoStandard, &prdsiItems[dwRecDeepCurPos].w32fdFindFileData, FindExSearchNameMatch, NULL, 0);
							}
						}
						else {// file
							if (CompareString( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName, -1, lpszFileName, dwFileNameLen) == CSTR_EQUAL) {
								prdsiItems[dwRecDeepCurPos].dwFileNameLen = lstrlenW(prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName);
								memmove((szPath+dwPathLen), prdsiItems[dwRecDeepCurPos].w32fdFindFileData.cFileName, ((prdsiItems[dwRecDeepCurPos].dwFileNameLen+1)*sizeof(WCHAR)));
								dwFilePathLen = (dwPathLen+prdsiItems[dwRecDeepCurPos].dwFileNameLen);

								if (pdwRetFilePathLen) (*pdwRetFilePathLen) = dwFilePathLen;
								if (lpszRetFilePathName && dwRetFilePathLen) {
									dwFilePathLen = min(dwFilePathLen, dwRetFilePathLen);
									memmove(lpszRetFilePathName, szPath, ((dwFilePathLen+1)*sizeof(WCHAR)));
								}

								dwRetErrorCode = NO_ERROR;
							}
						}
					}

					if (prdsiItems) FindClose(prdsiItems[dwRecDeepCurPos].hFind);
					dwRecDeepCurPos--;
				}
					while (dwRecDeepCurPos != -1);
			}
			mir_free(prdsiItems);
		}
		else dwRetErrorCode = GetLastError();
	}
	else dwRetErrorCode = ERROR_INVALID_HANDLE;

	return dwRetErrorCode;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD MemFillRandom(LPVOID lpBuff, size_t dwBuffSize)
{
	DWORD dwRetErrorCode;

	if (lpBuff && dwBuffSize) {
		HCRYPTPROV hCryptProv = NULL;

		if (CryptAcquireContext(&hCryptProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, 0) == FALSE && GetLastError() == NTE_BAD_KEYSET)
			CryptAcquireContext(&hCryptProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET);

		if (hCryptProv) {
			if (CryptGenRandom(hCryptProv, dwBuffSize, (BYTE*)lpBuff))
				dwRetErrorCode = NO_ERROR;
			else
				dwRetErrorCode = GetLastError();
		}
		else dwRetErrorCode = GetLastError();

		CryptReleaseContext(hCryptProv, 0);
	}
	else dwRetErrorCode = ERROR_INVALID_HANDLE;

	return dwRetErrorCode;
}

bool CMraProto::SetPassDB(const CMStringA& pass)
{
	if (pass.GetLength() >= 128)
		return false;

	CMStringA szEmail;
	if ( !mraGetStringA(NULL, "e-mail", szEmail))
		return false;

	BYTE btRandomData[256], btCryptedPass[256] = {0}, bthmacSHA1[MIR_SHA1_HASH_SIZE] = {0};
	MemFillRandom(btRandomData, sizeof(btRandomData));

	SHA1GetDigest((void*)pass.c_str(), pass.GetLength(), &btCryptedPass[1]);

	//BASE64EncodeUnSafe(lpszBuff, dwBuffSize, &btCryptedPass[(1+MIR_SHA1_HASH_SIZE)], (sizeof(btCryptedPass)-1), &dwBuffSize);
	memmove(&btCryptedPass[(1+MIR_SHA1_HASH_SIZE)], pass.c_str(), pass.GetLength());
	btCryptedPass[0] = (BYTE)pass.GetLength();
	//memmove(&btCryptedPass[1], lpszBuff, dwBuffSize);

	mir_hmac_sha1(bthmacSHA1, (BYTE*)szEmail.c_str(), szEmail.GetLength(), btRandomData, sizeof(btRandomData));

	RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);
	RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, sizeof(btRandomData));
	CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
	RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);

	setDword("pCryptVer", MRA_PASS_CRYPT_VER);
	mraWriteContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData));
	mraWriteContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass));
	return true;
}
																		 
/////////////////////////////////////////////////////////////////////////////////////////


bool CMraProto::GetPassDB_v1(CMStringA &res)
{
	BYTE btRandomData[256] = {0}, btCryptedPass[256] = {0}, bthmacSHA1[MIR_SHA1_HASH_SIZE] = {0};
	size_t dwRandomDataSize, dwCryptedPass, dwPassSize;
	CMStringA szEmail;

	if (mraGetContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData), &dwRandomDataSize))
	if (dwRandomDataSize == sizeof(btRandomData))
	if (mraGetContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass), &dwCryptedPass))
	if (dwCryptedPass == sizeof(btCryptedPass))
	if (mraGetStringA(NULL, "e-mail", szEmail)) {
		mir_hmac_sha1(bthmacSHA1, (BYTE*)szEmail.c_str(), szEmail.GetLength(), btRandomData, sizeof(btRandomData));

		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);
		CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
		RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, dwRandomDataSize);
		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);

		dwPassSize = (*btCryptedPass);
		btCryptedPass[dwPassSize+1+MIR_SHA1_HASH_SIZE] = 0;

		unsigned dwDecodedSize;
		mir_ptr<BYTE> pDecoded((PBYTE)mir_base64_decode((LPCSTR)&btCryptedPass[1+MIR_SHA1_HASH_SIZE], &dwDecodedSize));
		SHA1GetDigest(pDecoded, dwDecodedSize, btRandomData);
		if ( !memcmp(&btCryptedPass[1], btRandomData, MIR_SHA1_HASH_SIZE)) {
			res = CMStringA((LPSTR)(PBYTE)pDecoded, dwDecodedSize);
			return true;
		}
	}

	return false;
}

bool CMraProto::GetPassDB(CMStringA &res)
{
	switch (getDword("pCryptVer", 0)) {
	case 1:
		return GetPassDB_v1(res);
	case 2:
		break;
	default:
		return false;
	}

	CMStringA szEmail;
	BYTE btRandomData[256] = {0}, btCryptedPass[256] = {0}, bthmacSHA1[MIR_SHA1_HASH_SIZE] = {0};
	size_t dwRandomDataSize, dwCryptedPass;

	if (mraGetContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData), &dwRandomDataSize))
	if (dwRandomDataSize == sizeof(btRandomData))
	if (mraGetContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass), &dwCryptedPass))
	if (dwCryptedPass == sizeof(btCryptedPass))
	if (mraGetStringA(NULL, "e-mail", szEmail)) {
		mir_hmac_sha1(bthmacSHA1, (BYTE*)szEmail.GetString(), szEmail.GetLength(), btRandomData, sizeof(btRandomData));

		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);
		CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
		RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, dwRandomDataSize);
		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, MIR_SHA1_HASH_SIZE);

		DWORD dwPassSize = btCryptedPass[0];
		SHA1GetDigest(&btCryptedPass[(1+MIR_SHA1_HASH_SIZE)], dwPassSize, btRandomData);
		if ( !memcmp(&btCryptedPass[1], btRandomData, MIR_SHA1_HASH_SIZE))
			res = CMStringA((char*)&btCryptedPass[(1+MIR_SHA1_HASH_SIZE)], dwPassSize);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD ReplaceInBuff(LPVOID lpInBuff, size_t dwInBuffSize, size_t dwReplaceItemsCount, LPVOID *plpInReplaceItems, size_t *pdwInReplaceItemsCounts, LPVOID *plpOutReplaceItems, size_t *pdwOutReplaceItemsCounts, CMStringW &ret)
{
	DWORD dwRetErrorCode = NO_ERROR;
	
	#ifdef _DEBUG //check tables
		for (size_t i = 0;i<dwReplaceItemsCount;i++)
		{
			if (lstrlen((LPTSTR)plpInReplaceItems[i]) != (pdwInReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
			if (lstrlen((LPTSTR)plpOutReplaceItems[i]) != (pdwOutReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
		}
	#endif

	LPBYTE *plpbtFounded = (LPBYTE*)mir_calloc((sizeof(LPBYTE)*dwReplaceItemsCount));
	if (plpbtFounded) {
		LPBYTE lpbtOutBuffCur, lpbtInBuffCur, lpbtInBuffCurPrev, lpbtOutBuffMax;
		size_t i, dwFirstFoundIndex = 0, dwFoundCount = 0, dwMemPartToCopy;

		lpbtInBuffCurPrev = (LPBYTE)lpInBuff;
		lpbtOutBuffCur = (LPBYTE)ret.GetString();
		lpbtOutBuffMax = LPBYTE(lpbtOutBuffCur) + ret.GetLength();
		for (i = 0; i < dwReplaceItemsCount; i++) {// looking for the first time
			plpbtFounded[i] = (LPBYTE)MemoryFind((lpbtInBuffCurPrev-(LPBYTE)lpInBuff), lpInBuff, dwInBuffSize, plpInReplaceItems[i], pdwInReplaceItemsCounts[i]);
			if (plpbtFounded[i])
				dwFoundCount++;
		}

		while (dwFoundCount) {
			for (i = 0; i < dwReplaceItemsCount; i++)
				if (plpbtFounded[i] && (plpbtFounded[i] < plpbtFounded[dwFirstFoundIndex] || plpbtFounded[dwFirstFoundIndex] == NULL))
					dwFirstFoundIndex = i;

			if (plpbtFounded[dwFirstFoundIndex]) {// in found
				dwMemPartToCopy = (plpbtFounded[dwFirstFoundIndex]-lpbtInBuffCurPrev);
				if (lpbtOutBuffMax>(lpbtOutBuffCur+(dwMemPartToCopy+pdwInReplaceItemsCounts[dwFirstFoundIndex])))
				{
					memmove(lpbtOutBuffCur, lpbtInBuffCurPrev, dwMemPartToCopy);lpbtOutBuffCur += dwMemPartToCopy;
					memmove(lpbtOutBuffCur, plpOutReplaceItems[dwFirstFoundIndex], pdwOutReplaceItemsCounts[dwFirstFoundIndex]);lpbtOutBuffCur += pdwOutReplaceItemsCounts[dwFirstFoundIndex];
					lpbtInBuffCurPrev = (plpbtFounded[dwFirstFoundIndex]+pdwInReplaceItemsCounts[dwFirstFoundIndex]);

					for (i = 0;i<dwReplaceItemsCount;i++) {// looking for in next time
						if (plpbtFounded[i] && plpbtFounded[i] < lpbtInBuffCurPrev) {
							plpbtFounded[i] = (LPBYTE)MemoryFind((lpbtInBuffCurPrev-(LPBYTE)lpInBuff), lpInBuff, dwInBuffSize, plpInReplaceItems[i], pdwInReplaceItemsCounts[i]);
							if (plpbtFounded[i] == NULL) dwFoundCount--;
						}
					}
				}
				else {
					dwRetErrorCode = ERROR_BUFFER_OVERFLOW;
					DebugBreak();
					break;
				}
			}
			else {// сюда по идее никогда не попадём, на всякий случай.
				DebugBreak();
				break;
			}
		}
		lpbtInBuffCur = (((LPBYTE)lpInBuff)+dwInBuffSize);
		memmove(lpbtOutBuffCur, lpbtInBuffCurPrev, (lpbtInBuffCur-lpbtInBuffCurPrev));lpbtOutBuffCur += (lpbtInBuffCur-lpbtInBuffCurPrev);
		(*((WORD*)lpbtOutBuffCur)) = 0;

		mir_free(plpbtFounded);

		ret.Truncate(lpbtOutBuffCur-((LPBYTE)ret.GetString()));
	}
	else dwRetErrorCode = GetLastError();

	return dwRetErrorCode;
}

static const LPTSTR lpszXMLTags[]		 = {TEXT("&apos;"), 		TEXT("&quot;"), 		TEXT("&amp;"), 		TEXT("&lt;"), 		TEXT("&gt;")};
static const size_t dwXMLTagsCount[]	 = {(6*sizeof(TCHAR)), 	(6*sizeof(TCHAR)), 	(5*sizeof(TCHAR)), 	(4*sizeof(TCHAR)), 	(4*sizeof(TCHAR))};
static const LPTSTR lpszXMLSymbols[]	 = {TEXT("\'"), 			TEXT("\""), 			TEXT("&"), 			TEXT("<"), 			TEXT(">")};
static const size_t dwXMLSymbolsCount[]	 = {sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR)};

//Decode XML coded string. The function translate special xml code into standard characters.
CMStringW DecodeXML(const CMStringW &lptszMessage)
{
	CMStringW ret('\0', lptszMessage.GetLength());
	ReplaceInBuff((void*)lptszMessage.GetString(), lptszMessage.GetLength()*sizeof(TCHAR), SIZEOF(lpszXMLTags), (LPVOID*)lpszXMLTags, (size_t*)dwXMLTagsCount, (LPVOID*)lpszXMLSymbols, (size_t*)dwXMLSymbolsCount, ret);
	return ret;
}

//Encode XML coded string. The function translate special saved xml characters into special characters.
CMStringW EncodeXML(const CMStringW &lptszMessage)
{
	CMStringW ret;
	ReplaceInBuff((void*)lptszMessage.GetString(), lptszMessage.GetLength()*sizeof(TCHAR), SIZEOF(lpszXMLTags), (LPVOID*)lpszXMLSymbols, (size_t*)dwXMLSymbolsCount, (LPVOID*)lpszXMLTags, (size_t*)dwXMLTagsCount, ret);
	return ret;
}
