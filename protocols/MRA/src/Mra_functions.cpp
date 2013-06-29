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

DWORD MraGetSelfVersionString(LPSTR lpszSelfVersion, size_t dwSelfVersionSize, size_t *pdwSelfVersionSizeRet)
{
	if (!lpszSelfVersion || !dwSelfVersionSize)
		return ERROR_INVALID_HANDLE;

	WORD v[4];
	DWORD dwMirVer = CallService(MS_SYSTEM_GETFILEVERSION, 0, (LPARAM)v);
	LPSTR	lpszUnicode = (IsUnicodeEnv()? " Unicode":""),
			lpszSecIM = ( ServiceExists("SecureIM/IsContactSecured")? " + SecureIM":"");
	size_t dwSelfVersionSizeRet;

	dwSelfVersionSizeRet = mir_snprintf(lpszSelfVersion, dwSelfVersionSize, "Miranda NG %lu.%lu.%lu.%lu%s (MRA v%lu.%lu.%lu.%lu)%s, version: %lu.%lu",
		v[0], v[1], v[2], v[3], lpszUnicode,
		__FILEVERSION_STRING, lpszSecIM, PROTO_VERSION_MAJOR, PROTO_VERSION_MINOR);

	if (pdwSelfVersionSizeRet)
		*pdwSelfVersionSizeRet = dwSelfVersionSizeRet;
	return 0;
}

DWORD GetParamValue(LPSTR lpszData, size_t dwDataSize, LPSTR lpszParamName, size_t dwParamNameSize, LPSTR lpszParamValue, size_t dwParamValueSize, size_t *pParamValueSizeRet)
{
	if (!lpszData || !dwDataSize || !lpszParamName || !dwParamNameSize || !lpszParamValue || !dwParamValueSize)
		return ERROR_INVALID_HANDLE;

	char szData[USER_AGENT_MAX+4096];
	LPSTR lpszParamDataStart, lpszParamDataEnd;

	dwDataSize = ((dwDataSize<SIZEOF(szData))? dwDataSize:SIZEOF(szData));
	BuffToLowerCase(szData, lpszData, dwDataSize);

	lpszParamDataStart = (LPSTR)MemoryFind(0, szData, dwDataSize, lpszParamName, dwParamNameSize);
	if (lpszParamDataStart)
	if ((*((WORD*)(lpszParamDataStart+dwParamNameSize))) == (*((WORD*)"=\""))) {
		lpszParamDataStart += dwParamNameSize+2;
		lpszParamDataEnd = (LPSTR)MemoryFindByte((lpszParamDataStart-szData), szData, dwDataSize, '"');
		if (lpszParamDataEnd) {
			memmove(lpszParamValue, (lpszData+(lpszParamDataStart-szData)), (lpszParamDataEnd-lpszParamDataStart));
			if (pParamValueSizeRet) (*pParamValueSizeRet) = (lpszParamDataEnd-lpszParamDataStart);
			return NO_ERROR;
		}
	}
	return ERROR_NOT_FOUND;
}

DWORD MraGetVersionStringFromFormatted(LPSTR dwUserAgentFormatted, size_t dwUserAgentFormattedSize, LPSTR lpszVersion, size_t dwVersionSize, size_t *pdwVersionSizeRet)
{
	if (!dwUserAgentFormatted || !dwUserAgentFormattedSize || !lpszVersion || !dwVersionSize)
		return ERROR_INVALID_HANDLE;

	char szBuff[4096];
	size_t dwBuffSize, dwVersionSizeRet;

	if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "name", 4, szBuff, SIZEOF(szBuff), &dwBuffSize))
		if ( !_strnicmp(szBuff, "Miranda IM", dwBuffSize) || !_strnicmp(szBuff, "Miranda NG", dwBuffSize)) {
			GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "title", 5, lpszVersion, dwVersionSize, pdwVersionSizeRet);
			return 0;
		}

	dwVersionSizeRet = 0;
	if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "client", 6, lpszVersion, dwVersionSize, &dwBuffSize)) {
		dwVersionSizeRet += dwBuffSize;
		*((BYTE*)(lpszVersion+dwVersionSizeRet)) = ' ';
	}

	if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "name", 4, lpszVersion, dwVersionSize, &dwBuffSize)) {
		dwVersionSizeRet += dwBuffSize;
		*((BYTE*)(lpszVersion+dwVersionSizeRet)) = ' ';
	}

	if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "title", 5, lpszVersion, dwVersionSize, &dwBuffSize)) {
		dwVersionSizeRet += dwBuffSize;
		*((BYTE*)(lpszVersion+dwVersionSizeRet)) = ' ';
	}

	if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "version", 7, (lpszVersion+dwVersionSizeRet+1), (dwVersionSize-dwVersionSizeRet), &dwBuffSize)) {
		dwVersionSizeRet += (dwBuffSize+1);
		*((BYTE*)(lpszVersion+dwVersionSizeRet)) = '.';
		if ( !GetParamValue(dwUserAgentFormatted, dwUserAgentFormattedSize, "build", 5, (lpszVersion+dwVersionSizeRet+1), (dwVersionSize-dwVersionSizeRet), &dwBuffSize))
			dwVersionSizeRet += (dwBuffSize+1);
	}

	// no data extracted, copy raw
	if (dwVersionSizeRet == 0) {
		dwVersionSizeRet = (dwUserAgentFormattedSize < dwVersionSize) ? dwUserAgentFormattedSize : dwVersionSize;
		memmove(lpszVersion, dwUserAgentFormatted, dwVersionSizeRet);
	}

	if (pdwVersionSizeRet)
		*pdwVersionSizeRet = dwVersionSizeRet;

	return 0;
}

DWORD MraAddrListGetFromBuff(LPSTR lpszAddreses, size_t dwAddresesSize, MRA_ADDR_LIST *pmalAddrList)
{
	if (!lpszAddreses || !dwAddresesSize || !pmalAddrList)
		return ERROR_INVALID_HANDLE;

	LPSTR lpszCurrentItem, lpszDelimiter, lpszEndItem;
	DWORD dwAllocatedCount;

	dwAllocatedCount = ALLOCATED_COUNT;
	pmalAddrList->dwAddrCount = 0;
	pmalAddrList->pmaliAddress = (MRA_ADDR_LIST_ITEM*)mir_calloc(sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount);
	lpszCurrentItem = lpszAddreses;

	while (TRUE) {
		lpszEndItem = (LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses), lpszAddreses, dwAddresesSize, ';');
		if (lpszEndItem == NULL) lpszEndItem = (lpszAddreses+dwAddresesSize);
		if (!lpszEndItem)
			break;

		lpszDelimiter = (LPSTR)MemoryFindByte((lpszCurrentItem-lpszAddreses), lpszAddreses, dwAddresesSize, ':');
		if (!lpszDelimiter)
			break;

		if (pmalAddrList->dwAddrCount == dwAllocatedCount) {
			dwAllocatedCount += ALLOCATED_COUNT;
			pmalAddrList->pmaliAddress = (MRA_ADDR_LIST_ITEM*)mir_realloc(pmalAddrList->pmaliAddress, (sizeof(MRA_ADDR_LIST_ITEM)*dwAllocatedCount));
		}

		(*lpszDelimiter) = 0;
		pmalAddrList->pmaliAddress[pmalAddrList->dwAddrCount].dwAddr = inet_addr(lpszCurrentItem);
		pmalAddrList->pmaliAddress[pmalAddrList->dwAddrCount].dwPort = StrToUNum32((lpszDelimiter+1), (lpszEndItem-(lpszDelimiter+1)));
		(*lpszDelimiter) = ':';
		pmalAddrList->dwAddrCount++;
		lpszCurrentItem = (lpszEndItem+1);

		if (lpszEndItem == lpszAddreses + dwAddresesSize)
			break;
	}
	pmalAddrList->pmaliAddress = (MRA_ADDR_LIST_ITEM*)mir_realloc(pmalAddrList->pmaliAddress, (sizeof(MRA_ADDR_LIST_ITEM)*pmalAddrList->dwAddrCount));
	return NO_ERROR;
}

DWORD MraAddrListGetToBuff(MRA_ADDR_LIST *pmalAddrList, LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSizeRet)
{
	if (!pmalAddrList || !lpszBuff || !dwBuffSize)
		return ERROR_INVALID_HANDLE;

	LPSTR lpszCurPos = lpszBuff;

	for (size_t i = 0;i<pmalAddrList->dwAddrCount;i++)
		lpszCurPos += mir_snprintf(lpszCurPos, (dwBuffSize-((size_t)lpszCurPos-(size_t)lpszBuff)), "%s:%lu;",
			inet_ntoa((*((in_addr*)&pmalAddrList->pmaliAddress[i].dwAddr))), pmalAddrList->pmaliAddress[i].dwPort);

	if (pdwBuffSizeRet)
		*pdwBuffSizeRet = lpszCurPos - lpszBuff;
	return NO_ERROR;
}

void CMraProto::MraAddrListStoreToContact(HANDLE hContact, MRA_ADDR_LIST *pmalAddrList)
{
	if (!hContact || !pmalAddrList)
		return;

	if (!pmalAddrList->dwAddrCount)
		return;

	mraSetDword(hContact, "OldIP", mraGetDword(hContact, "IP", 0));
	mraSetDword(hContact, "IP", HTONL(pmalAddrList->pmaliAddress[0].dwAddr));
	if (pmalAddrList->dwAddrCount > 1) {
		mraSetDword(hContact, "OldRealIP", mraGetDword(hContact, "RealIP", 0));
		mraSetDword(hContact, "RealIP", HTONL(pmalAddrList->pmaliAddress[1].dwAddr));
	}
}

void MraAddrListFree(MRA_ADDR_LIST *pmalAddrList)
{
	if (pmalAddrList) {
		mir_free(pmalAddrList->pmaliAddress);
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
	}else {
		if (lpszRetBuff && dwRetBuffSize >= sizeof(WORD)) (*((WORD*)lpszRetBuff)) = 0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize) = 0;
	}
	return bRet;
}


BOOL DB_GetStaticStringW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPWSTR lpwszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{// sizes in wchars
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
	}else {
		if (lpwszRetBuff && dwRetBuffSize >= sizeof(WCHAR)) (*((WCHAR*)lpwszRetBuff)) = 0;
		if (pdwRetBuffSize)	(*pdwRetBuffSize) = 0;
	}
	return bRet;
}

BOOL DB_SetStringExA(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPCSTR lpszValue, size_t dwValueSize)
{
	BOOL bRet = FALSE;

	if (lpszValue && dwValueSize) {
		LPWSTR lpwszValueLocal;
		size_t dwValueSizeLocal;

		dwValueSizeLocal = (dwValueSize+MAX_PATH);
		lpwszValueLocal = (LPWSTR)mir_calloc((dwValueSizeLocal*sizeof(WCHAR)));

		if (lpwszValueLocal) {
			DBCONTACTWRITESETTING cws = {0};

			cws.szModule = lpszModule;
			cws.szSetting = lpszValueName;
			cws.value.type = DBVT_WCHAR;
			cws.value.pwszVal = (WCHAR*)lpwszValueLocal;

			dwValueSizeLocal = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpszValue, dwValueSize, lpwszValueLocal, dwValueSizeLocal);
			lpwszValueLocal[dwValueSizeLocal] = 0;
			bRet = (CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws) == 0);

			mir_free(lpwszValueLocal);
		}
	}
	else {
		bRet = TRUE;
		db_unset(hContact, lpszModule, lpszValueName);
	}
	return bRet;
}

BOOL DB_SetStringExW(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPCWSTR lpwszValue, size_t dwValueSize)
{
	BOOL bRet = FALSE;

	if (lpwszValue && dwValueSize) {
		LPWSTR lpwszValueLocal = (LPWSTR)mir_calloc(((dwValueSize+MAX_PATH)*sizeof(WCHAR)));

		if (lpwszValueLocal) {
			DBCONTACTWRITESETTING cws = {0};

			cws.szModule = lpszModule;
			cws.szSetting = lpszValueName;
			cws.value.type = DBVT_WCHAR;
			cws.value.pwszVal = (WCHAR*)lpwszValueLocal;
			memmove(lpwszValueLocal, lpwszValue, (dwValueSize*sizeof(WCHAR)));
			bRet = (CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws) == 0);

			mir_free(lpwszValueLocal);
		}
	}
	else {
		bRet = TRUE;
		db_unset(hContact, lpszModule, lpszValueName);
	}
	return bRet;
}

int DB_WriteContactSettingBlob(HANDLE hContact, LPCSTR lpszModule, LPCSTR lpszValueName, LPVOID lpValue, size_t dwValueSize)
{
	DBCONTACTWRITESETTING cws = {0};

	cws.szModule = lpszModule;
	cws.szSetting = lpszValueName;
	cws.value.type = DBVT_BLOB;
	cws.value.pbVal = (LPBYTE)lpValue;
	cws.value.cpbVal = (WORD)dwValueSize;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
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
		CHAR szEMail[MAX_EMAIL_LEN];
		size_t dwEMailSize;

		dwRet = mraGetDword(hContact, "ContactFlags", 0);
		dwRet &= ~(CONTACT_FLAG_REMOVED|CONTACT_FLAG_GROUP|CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE|CONTACT_FLAG_IGNORE|CONTACT_FLAG_SHADOW|CONTACT_FLAG_MULTICHAT);
		dwRet |= CONTACT_FLAG_UNICODE_NAME;

		if (mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
		{
			if (IsEMailChatAgent(szEMail, dwEMailSize))
			{
				dwRet |= CONTACT_FLAG_MULTICHAT;
			}
		}

		if (db_get_b(hContact, "CList", "Hidden", 0)) dwRet |= CONTACT_FLAG_SHADOW;

		switch (mraGetWord(hContact, "ApparentMode", 0)) {
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

	mraSetDword(hContact, "ContactFlags", dwContactFlag);

	if (dwContactFlag&CONTACT_FLAG_SHADOW)
		db_set_b(hContact, "CList", "Hidden", 1);
	else
		db_unset(hContact, "CList", "Hidden");

	switch (dwContactFlag&(CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE)) {
	case CONTACT_FLAG_INVISIBLE:
		mraSetWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
		break;
	case CONTACT_FLAG_VISIBLE:
		mraSetWord(hContact, "ApparentMode", ID_STATUS_ONLINE);
		break;
	default:
		mraSetWord(hContact, "ApparentMode", 0);
		break;
	}

	if (dwContactFlag&CONTACT_FLAG_IGNORE)
		CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, IGNOREEVENT_MESSAGE);
	else
		CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, IGNOREEVENT_MESSAGE);

	return 0;
}

DWORD CMraProto::GetContactBasicInfoW(HANDLE hContact, DWORD *pdwID, DWORD *pdwGroupID, DWORD *pdwContactFlag, DWORD *pdwContactSeverFlags, DWORD *pdwStatus, LPSTR lpszEMail, size_t dwEMailSize, size_t *pdwEMailSize, LPWSTR lpwszNick, size_t dwNickSize, size_t *pdwNickSize, LPSTR lpszPhones, size_t dwPhonesSize, size_t *pdwPhonesSize)
{
	if ( !IsContactMra(hContact))
		return ERROR_INVALID_HANDLE;

	if (pdwID)
		*pdwID = mraGetDword(hContact, "ContactID", -1);
	if (pdwGroupID)
		*pdwGroupID = mraGetDword(hContact, "GroupID", -1);
	if (pdwContactSeverFlags)
		*pdwContactSeverFlags = mraGetDword(hContact, "ContactSeverFlags", 0);
	if (pdwStatus)
		*pdwStatus = MraGetContactStatus(hContact);
	if (pdwContactFlag)
		*pdwContactFlag = GetContactFlags(hContact);
	if (lpszEMail && pdwEMailSize)
		mraGetStaticStringA(hContact, "e-mail", lpszEMail, dwEMailSize, pdwEMailSize);
	if (lpwszNick && pdwNickSize)
		DB_GetStaticStringW(hContact, "CList", "MyHandle", lpwszNick, dwNickSize, pdwNickSize);

	if (lpszPhones && pdwPhonesSize) {
		char szPhone[MAX_PATH], szValue[MAX_PATH];
		size_t dwPhoneSize, dwCopied = 0;

		for (size_t i = 0; i < 3; i++) {
			mir_snprintf(szValue, SIZEOF(szValue), "MyPhone%lu", i);
			if ( DB_GetStaticStringA(hContact, "UserInfo", szValue, szPhone, SIZEOF(szPhone), &dwPhoneSize)) {
				if (dwCopied)
					*((LPBYTE)(lpszPhones+dwCopied++)) = ',';
				dwCopied += CopyNumber((lpszPhones+dwCopied), szPhone, dwPhoneSize);
			}
		}
		*pdwPhonesSize = dwCopied;
	}
	return 0;
}

DWORD CMraProto::SetContactBasicInfoW(HANDLE hContact, DWORD dwSetInfoFlags, DWORD dwFlags, DWORD dwID, DWORD dwGroupID, DWORD dwContactFlag, DWORD dwContactSeverFlags, DWORD dwStatus, LPSTR lpszEMail, size_t dwEMailSize, LPWSTR lpwszNick, size_t dwNickSize, LPSTR lpszPhones, size_t dwPhonesSize)
{
	if ( !IsContactMra(hContact))
		return ERROR_INVALID_HANDLE;

	// LOCK
	if (dwSetInfoFlags & SCBIFSI_LOCK_CHANGES_EVENTS)
		mraSetDword(hContact, "HooksLocked", TRUE);

	// поля которые нужны, и изменения которых не отслеживаются
	if (dwFlags & SCBIF_ID)
		mraSetDword(hContact, "ContactID", dwID);

	if (dwFlags & SCBIF_EMAIL)
		if (lpszEMail && dwEMailSize)
			mraSetStringExA(hContact, "e-mail", lpszEMail, dwEMailSize);

	// поля изменения которых отслеживаются
	if (dwFlags & SCBIF_GROUP_ID)
		mraSetDword(hContact, "GroupID", dwGroupID);

	if (dwFlags & SCBIF_NICK) {
		if ((dwFlags & SCBIF_FLAG) && ((dwContactFlag&CONTACT_FLAG_UNICODE_NAME) == 0))
		{
			if (lpwszNick && dwNickSize)
				DB_SetStringExA(hContact, "CList", "MyHandle", (LPSTR)lpwszNick, dwNickSize);
		}
		else {
			if (lpwszNick && dwNickSize)
				DB_SetStringExW(hContact, "CList", "MyHandle", lpwszNick, dwNickSize);
		}
	}

	if (dwFlags & SCBIF_PHONES) {
		if (lpszPhones && dwPhonesSize) {
			char szPhone[MAX_PATH], szValue[MAX_PATH];
			LPSTR lpszCurPhone, lpszPhoneNext;
			size_t i, dwCurPhoneSize;

			i = 0;
			lpszCurPhone = lpszPhones;
			lpszPhoneNext = lpszPhones;
			while (lpszPhoneNext) {
				lpszPhoneNext = (LPSTR)MemoryFindByte((lpszCurPhone-lpszPhones), lpszPhones, dwPhonesSize, ',');
				if (lpszPhoneNext)
					dwCurPhoneSize = lpszPhoneNext - lpszCurPhone;
				else
					dwCurPhoneSize = (lpszPhones + dwPhonesSize) - lpszCurPhone;

				szPhone[0] = '+';
				memmove((szPhone+1), lpszCurPhone, min(dwCurPhoneSize, (SIZEOF(szPhone)-1)));
				mir_snprintf(szValue, SIZEOF(szValue), "MyPhone%lu", i);
				DB_SetStringExA(hContact, "UserInfo", szValue, szPhone, (1+dwCurPhoneSize));

				i++;
				lpszCurPhone = (lpszPhoneNext+1);
			}
		}
	}

	if (dwFlags & SCBIF_FLAG)
		SetContactFlags(hContact, dwContactFlag);

	if (dwFlags & SCBIF_SERVER_FLAG)
		mraSetDword(hContact, "ContactSeverFlags", dwContactSeverFlags);

	if (dwFlags & SCBIF_STATUS)
		MraSetContactStatus(hContact, dwStatus);

	SetExtraIcons(hContact);
	// UNLOCK
	if (dwSetInfoFlags & SCBIFSI_LOCK_CHANGES_EVENTS)
		mraSetDword(hContact, "HooksLocked", FALSE);

	return 0;
}

HANDLE CMraProto::MraHContactFromEmail(LPSTR lpszEMail, size_t dwEMailSize, BOOL bAddIfNeeded, BOOL bTemporary, BOOL *pbAdded)
{
	HANDLE hContact = NULL;

	if (lpszEMail && dwEMailSize)
	{
		BOOL bFound = FALSE;
		CHAR szEMailLocal[MAX_EMAIL_LEN];
		size_t dwEMailLocalSize;

		if (dwEMailSize == -1) dwEMailSize = lstrlenA(lpszEMail);
		//check not already on list
		for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			if (mraGetStaticStringA(hContact, "e-mail", szEMailLocal, SIZEOF(szEMailLocal), &dwEMailLocalSize))
			if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, szEMailLocal, dwEMailLocalSize, lpszEMail, dwEMailSize) == CSTR_EQUAL) {
				if (bTemporary == FALSE) db_unset(hContact, "CList", "NotOnList");
				bFound = TRUE;
				break;
			}
		}

		if (bFound == FALSE && bAddIfNeeded)
		{//not already there: add
			if (IsEMailChatAgent(lpszEMail, dwEMailSize))
			{
				GCSESSION gcw = {0};
				WCHAR wszEMail[MAX_EMAIL_LEN] = {0};

				gcw.cbSize = sizeof(GCSESSION);
				gcw.iType = GCW_CHATROOM;
				gcw.pszModule = m_szModuleName;
				gcw.ptszName = wszEMail;
				gcw.ptszID = (LPWSTR)wszEMail;
				gcw.dwFlags = GC_UNICODE;
				MultiByteToWideChar(MRA_CODE_PAGE, 0, lpszEMail, dwEMailSize, wszEMail, SIZEOF(wszEMail));

				if (CallServiceSync(MS_GC_NEWSESSION, NULL, (LPARAM)&gcw) == 0) {
					BOOL bChatAdded = FALSE;
					for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
						if (mraGetStaticStringA(hContact, "ChatRoomID", szEMailLocal, SIZEOF(szEMailLocal), &dwEMailLocalSize))
						if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, szEMailLocal, dwEMailLocalSize, lpszEMail, dwEMailSize) == CSTR_EQUAL) {
							bChatAdded = TRUE;
							break;
						}
					}
					if (bChatAdded == FALSE) hContact = NULL;
				}
			}else {
				hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName);
			}

			if (hContact) {
				if (IsEMailChatAgent(lpszEMail, dwEMailSize))
					SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL), -1, -1, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, ID_STATUS_ONLINE, lpszEMail, dwEMailSize, NULL, 0, NULL, 0);
				else {
					if (bTemporary)
						db_set_b(hContact, "CList", "NotOnList", 1);
					mraSetStringExA(hContact, "MirVer", MIRVER_UNKNOWN, (sizeof(MIRVER_UNKNOWN)-1));
					SetContactBasicInfoW(hContact, SCBIFSI_LOCK_CHANGES_EVENTS, (SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS|SCBIF_EMAIL), -1, -1, 0, CONTACT_INTFLAG_NOT_AUTHORIZED, ID_STATUS_OFFLINE, lpszEMail, dwEMailSize, NULL, 0, NULL, 0);
				}
			}
		}

		if (pbAdded) (*pbAdded) = (bFound == FALSE && bAddIfNeeded && hContact);
	}
	return hContact;
}

BOOL CMraProto::MraUpdateContactInfo(HANDLE hContact)
{
	BOOL bRet = FALSE;

	if (m_bLoggedIn && hContact)
	if (IsContactMra(hContact)) {
		CHAR szEMail[MAX_EMAIL_LEN];
		size_t dwEMailSize;

		if (mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
			MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, GAIF_FORCE, hContact, 0);

			if (MraWPRequestByEMail(hContact, ACKTYPE_GETINFO, szEMail, dwEMailSize))
				bRet = TRUE;
		}
	}
	return bRet;
}

DWORD CMraProto::MraContactCapabilitiesGet(HANDLE hContact)
{
	return mraGetDword(hContact, DBSETTING_CAPABILITIES, 0);
}

void CMraProto::MraContactCapabilitiesSet(HANDLE hContact, DWORD dwFutureFlags)
{
	mraSetDword(hContact, DBSETTING_CAPABILITIES, dwFutureFlags);
}

DWORD CMraProto::MraGetContactStatus(HANDLE hContact)
{
	return mraGetWord(hContact, "Status", ID_STATUS_OFFLINE);
}

DWORD CMraProto::MraSetContactStatus(HANDLE hContact, DWORD dwNewStatus)
{
	DWORD dwOldStatus = MraGetContactStatus(hContact);

	if (dwNewStatus != dwOldStatus)
	{
		BOOL bChatAgent;

		bChatAgent = IsContactChatAgent(hContact);

		if (dwNewStatus == ID_STATUS_OFFLINE)
		{
			if (hContact)
			{
				mraSetByte(hContact, DBSETTING_XSTATUSID, MRA_MIR_XSTATUS_NONE);
				mraDelValue(hContact, DBSETTING_XSTATUSNAME);
				mraDelValue(hContact, DBSETTING_XSTATUSMSG);
				mraDelValue(hContact, DBSETTING_BLOGSTATUSTIME);
				mraDelValue(hContact, DBSETTING_BLOGSTATUSID);
				mraDelValue(hContact, DBSETTING_BLOGSTATUS);
				mraDelValue(hContact, DBSETTING_BLOGSTATUSMUSIC);
				MraContactCapabilitiesSet(hContact, 0);
				if (bChatAgent) MraChatSessionDestroy(hContact);
			}
			mraSetDword(hContact, "LogonTS", 0);
			mraDelValue(hContact, "IP");
			mraDelValue(hContact, "RealIP");
		}else {
			if (dwOldStatus == ID_STATUS_OFFLINE)
			{
				DWORD dwTime = (DWORD)_time32(NULL);

				mraSetDword(hContact, "LogonTS", dwTime);
				mraSetDword(hContact, "OldLogonTS", dwTime);

				if (bChatAgent) MraChatSessionNew(hContact);
			}
			MraAvatarsQueueGetAvatarSimple(hAvatarsQueueHandle, 0, hContact, 0);
		}

		mraSetWord(hContact, "Status", (WORD)dwNewStatus);
	}
	return(dwOldStatus);
}

void CMraProto::MraUpdateEmailStatus(LPSTR lpszFrom, size_t dwFromSize, LPSTR lpszSubject, size_t dwSubjectSize, DWORD dwDate, DWORD dwUIDL)
{
	BOOL bTrayIconNewMailNotify;
	WCHAR szStatusText[MAX_SECONDLINE];

	bTrayIconNewMailNotify = mraGetByte(NULL, "TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY);

	if (dwEmailMessagesUnread)
	{
		LPSTR lpszEMail;
		size_t dwEMailSize;
		HANDLE hContact = NULL;
		WCHAR szMailBoxStatus[MAX_SECONDLINE];

		mir_sntprintf(szMailBoxStatus, SIZEOF(szMailBoxStatus), TranslateW(L"Unread mail is available: %lu/%lu messages"), dwEmailMessagesUnread, dwEmailMessagesTotal);

		if ((lpszFrom && dwFromSize) || (lpszSubject && dwSubjectSize))
		{
			WCHAR szFrom[MAX_PATH] = {0}, szSubject[MAX_PATH] = {0};

			if (GetEMailFromString(lpszFrom, dwFromSize, &lpszEMail, &dwEMailSize))
			{
				hContact = MraHContactFromEmail(lpszEMail, dwEMailSize, FALSE, TRUE, NULL);
			}

			dwFromSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpszFrom, dwFromSize, szFrom, SIZEOF(szFrom));
			szFrom[dwFromSize] = 0;

			dwSubjectSize = MultiByteToWideChar(MRA_CODE_PAGE, 0, lpszSubject, dwSubjectSize, szSubject, SIZEOF(szSubject));
			szSubject[dwSubjectSize] = 0;

			mir_sntprintf(szStatusText, SIZEOF(szStatusText), TranslateW(L"From: %s\r\nSubject: %s\r\n%s"), szFrom, szSubject, szMailBoxStatus);
		}else {
			lstrcpynW(szStatusText, szMailBoxStatus, SIZEOF(szStatusText));
		}

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

			if (mraGetByte(NULL, "TrayIconNewMailClkToInbox", MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX))
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
		if (mraGetByte(NULL, "IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY))
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


BOOL IsUnicodeEnv()
{// Are we running under unicode Miranda core ?
	return TRUE;
}


BOOL IsHTTPSProxyUsed(HANDLE hNetlibUser)
{
	BOOL bRet = FALSE;
	NETLIBUSERSETTINGS nlus = {0};

	nlus.cbSize = sizeof(nlus);
	if (CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)hNetlibUser, (LPARAM)&nlus))
	{
		if (nlus.useProxy && nlus.proxyType == PROXYTYPE_HTTPS) bRet = TRUE;
	}
	return(bRet);
}

// определяет принадлежность контакта данной копии плагина
BOOL CMraProto::IsContactMra(HANDLE hContact)
{
	return(CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)m_szModuleName));
}

// определяется является ли контакт контактом MRA протокола, не зависимо от того какому плагину он принадлежит
BOOL IsContactMraProto(HANDLE hContact)
{
	BOOL bRet = FALSE;
	LPSTR lpszProto = GetContactProto(hContact);

	if (lpszProto)
	{
		WCHAR szBuff[MAX_PATH];

		if (DB_GetStaticStringW(hContact, lpszProto, "AvatarLastCheckTime", szBuff, SIZEOF(szBuff), NULL))
		if (DB_GetStaticStringW(hContact, lpszProto, "AvatarLastModifiedTime", szBuff, SIZEOF(szBuff), NULL))
		{
			bRet = TRUE;
		}
	}
	return(bRet);
}

BOOL CMraProto::IsEMailMy(LPSTR lpszEMail, size_t dwEMailSize)
{
	BOOL bRet = FALSE;

	if (lpszEMail)
	{
		CHAR szEMailMy[MAX_EMAIL_LEN];
		size_t dwEMailMySize;

		if (mraGetStaticStringA(NULL, "e-mail", szEMailMy, SIZEOF(szEMailMy), &dwEMailMySize))
		{
			if (dwEMailSize == 0) dwEMailSize = lstrlenA(lpszEMail);

			if (dwEMailMySize == dwEMailSize)
			if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, lpszEMail, dwEMailSize, szEMailMy, dwEMailMySize) == CSTR_EQUAL)
			{
				bRet = TRUE;
			}
		}
	}
return(bRet);
}


BOOL CMraProto::IsEMailChatAgent(LPSTR lpszEMail, size_t dwEMailSize)
{
	BOOL bRet = FALSE;

	if (lpszEMail)
	{
		if (dwEMailSize == 0) dwEMailSize = lstrlenA(lpszEMail);
		if ((sizeof(MAILRU_CHAT_CONF_DOMAIN)-1)<dwEMailSize)
		if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, MAILRU_CHAT_CONF_DOMAIN, (sizeof(MAILRU_CHAT_CONF_DOMAIN)-1), (lpszEMail+(dwEMailSize-(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1))), (sizeof(MAILRU_CHAT_CONF_DOMAIN)-1)) == CSTR_EQUAL)
		{
			if ( (*(BYTE*)(lpszEMail+((dwEMailSize-(sizeof(MAILRU_CHAT_CONF_DOMAIN)-1))-1))) == '@' )
			{
				bRet = TRUE;
			}
		}
	}
return(bRet);
}


BOOL CMraProto::IsContactChatAgent(HANDLE hContact)
{
	BOOL bRet = FALSE;

	if (hContact)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		size_t dwEMailSize;

		if (mraGetStaticStringA(hContact, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize))
		{
			bRet = IsEMailChatAgent(szEMail, dwEMailSize);
		}
		//bRet = ((GetContactFlags(hContact)&CONTACT_FLAG_MULTICHAT) != 0);
	}
return(bRet);
}



BOOL IsEMailMR(LPSTR lpszEMail, size_t dwEMailSize)
{
	BOOL bRet = FALSE;

	if (lpszEMail)
	{
		size_t i, dwDomainLen;

		if (dwEMailSize == 0) dwEMailSize = lstrlenA(lpszEMail);
		for (i = 0;lpcszMailRuDomains[i];i++)
		{
			dwDomainLen = lstrlenA(lpcszMailRuDomains[i]);
			if (dwDomainLen<dwEMailSize)
			if (CompareStringA( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, lpcszMailRuDomains[i], dwDomainLen, (lpszEMail+(dwEMailSize-dwDomainLen)), dwDomainLen) == CSTR_EQUAL)
			{
				if ( (*(BYTE*)(lpszEMail+((dwEMailSize-dwDomainLen)-1))) == '@' )
				{
					bRet = TRUE;
					break;
				}
			}
		}
	}
return(bRet);
}


BOOL GetEMailFromString(LPSTR lpszBuff, size_t dwBuffSize, LPSTR *plpszEMail, size_t *pdwEMailSize)
{
	BOOL bRet = FALSE;

	if (lpszBuff && dwBuffSize)
	{
		LPSTR lpszEMailStart, lpszEMailEnd;

		if ((lpszEMailStart = (LPSTR)MemoryFindByte(0, lpszBuff, dwBuffSize, '<')))
		{
			lpszEMailStart++;
			if ((lpszEMailEnd = (LPSTR)MemoryFindByte((lpszEMailStart-lpszBuff), lpszBuff, dwBuffSize, '>')))
			{
				if (plpszEMail)		(*plpszEMail) = lpszEMailStart;
				if (pdwEMailSize)	(*pdwEMailSize) = (lpszEMailEnd-lpszEMailStart);

				bRet = TRUE;
			}
		}
	}

	if (bRet == FALSE)
	{
		if (plpszEMail)		(*plpszEMail) = NULL;
		if (pdwEMailSize)	(*pdwEMailSize) = 0;
	}

return(bRet);
}


DWORD GetContactEMailCountParam(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName)
{
	DWORD dwRet = 0;
	CHAR szBuff[MAX_PATH], szEMail[MAX_EMAIL_LEN];
	size_t i, dwEMailSize;

	if (DB_GetStaticStringA(hContact, lpszModule, lpszValueName, szEMail, SIZEOF(szEMail), &dwEMailSize))
	{
		if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize)) dwRet++;
	}

	for (i = 0;TRUE;i++)
	{
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if (DB_GetStaticStringA(hContact, lpszModule, szBuff, szEMail, SIZEOF(szEMail), &dwEMailSize))
		{
			if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize)) dwRet++;
		}else {
			if (i>EMAILS_MIN_COUNT) break;
		}
	}
return(dwRet);
}


DWORD CMraProto::GetContactEMailCount(HANDLE hContact, BOOL bMRAOnly)
{
	DWORD dwRet = 0;
	LPSTR lpszProto;

	if (hContact)
		lpszProto = GetContactProto(hContact);
	else
		lpszProto = m_szModuleName;

	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, lpszProto, "e-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "e-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "Mye-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "Companye-mail");
	dwRet += GetContactEMailCountParam(hContact, bMRAOnly, "UserInfo", "MyCompanye-mail");

return(dwRet);
}



BOOL GetContactFirstEMailParam(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszModule, LPSTR lpszValueName, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{
	BOOL bRet = FALSE;
	CHAR szBuff[MAX_PATH], szEMail[MAX_EMAIL_LEN];
	size_t i, dwEMailSize;

	if (DB_GetStaticStringA(hContact, lpszModule, lpszValueName, szEMail, SIZEOF(szEMail), &dwEMailSize))
	{
		if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize))
		{
			lstrcpynA(lpszRetBuff, szEMail, dwRetBuffSize);
			if (pdwRetBuffSize) (*pdwRetBuffSize) = dwEMailSize;
			bRet = TRUE;
		}
	}

	for (i = 0;bRet == FALSE;i++)
	{
		mir_snprintf(szBuff, SIZEOF(szBuff), "%s%lu", lpszValueName, i);
		if (DB_GetStaticStringA(hContact, lpszModule, szBuff, szEMail, SIZEOF(szEMail), &dwEMailSize))
		{
			if (bMRAOnly == FALSE || IsEMailMR(szEMail, dwEMailSize))
			{
				lstrcpynA(lpszRetBuff, szEMail, dwRetBuffSize);
				if (pdwRetBuffSize) (*pdwRetBuffSize) = dwEMailSize;
				bRet = TRUE;
				break;
			}
		}else {
			if (i>EMAILS_MIN_COUNT) break;
		}
	}
return(bRet);
}


BOOL CMraProto::GetContactFirstEMail(HANDLE hContact, BOOL bMRAOnly, LPSTR lpszRetBuff, size_t dwRetBuffSize, size_t *pdwRetBuffSize)
{
	BOOL bRet = FALSE;
	LPSTR lpszProto;

	if (hContact)
		lpszProto = GetContactProto(hContact);
	else
		lpszProto = m_szModuleName;

	bRet = GetContactFirstEMailParam(hContact, bMRAOnly, lpszProto, "e-mail", lpszRetBuff, dwRetBuffSize, pdwRetBuffSize);
	if (bRet == FALSE)	bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "e-mail", lpszRetBuff, dwRetBuffSize, pdwRetBuffSize);
	if (bRet == FALSE)	bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "Mye-mail", lpszRetBuff, dwRetBuffSize, pdwRetBuffSize);
	if (bRet == FALSE)	bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "Companye-mail", lpszRetBuff, dwRetBuffSize, pdwRetBuffSize);
	if (bRet == FALSE)	bRet = GetContactFirstEMailParam(hContact, bMRAOnly, "UserInfo", "MyCompanye-mail", lpszRetBuff, dwRetBuffSize, pdwRetBuffSize);

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

DWORD CMraProto::CreateBlobFromContact(HANDLE hContact, LPWSTR lpwszRequestReason, size_t dwRequestReasonSize, LPBYTE lpbBuff, size_t dwBuffSize, size_t *pdwBuffSizeRet)
{
	DWORD dwRetErrorCode;
	size_t dwBuffSizeRet = ((sizeof(DWORD)*2)+dwRequestReasonSize+5), dwSize;

	if (dwBuffSize >= dwBuffSizeRet) {
		PBYTE pCurBlob = lpbBuff;

		*(DWORD*)pCurBlob = 0; pCurBlob += sizeof(DWORD);
		*(DWORD*)pCurBlob = (DWORD)hContact; pCurBlob += sizeof(DWORD);

		mraGetStaticStringA(hContact, "Nick", (LPSTR)pCurBlob, (dwBuffSize-(pCurBlob-lpbBuff)), &dwSize);
		*(pCurBlob+dwSize) = 0;
		pCurBlob += (dwSize+1);

		mraGetStaticStringA(hContact, "FirstName", (LPSTR)pCurBlob, (dwBuffSize-(pCurBlob-lpbBuff)), &dwSize);
		*(pCurBlob+dwSize) = 0;
		pCurBlob += (dwSize+1);

		mraGetStaticStringA(hContact, "LastName", (LPSTR)pCurBlob, (dwBuffSize-(pCurBlob-lpbBuff)), &dwSize);
		*(pCurBlob+dwSize) = 0;
		pCurBlob += (dwSize+1);

		mraGetStaticStringA(hContact, "e-mail", (LPSTR)pCurBlob, (dwBuffSize-(pCurBlob-lpbBuff)), &dwSize);
		*(pCurBlob+dwSize) = 0;
		pCurBlob += (dwSize+1);

		dwSize = WideCharToMultiByte(MRA_CODE_PAGE, 0, lpwszRequestReason, dwRequestReasonSize, (LPSTR)pCurBlob, (dwBuffSize-(pCurBlob-lpbBuff)), NULL, NULL);
		*(pCurBlob+dwSize) = 0;
		pCurBlob += (dwSize+1);

		dwBuffSizeRet = (pCurBlob-lpbBuff);
		dwRetErrorCode = NO_ERROR;
	}
	else dwRetErrorCode = ERROR_INSUFFICIENT_BUFFER;

	if (pdwBuffSizeRet)
		*pdwBuffSizeRet = dwBuffSizeRet;
	return dwRetErrorCode;
}

size_t CopyNumber(LPCVOID lpcOutBuff, LPCVOID lpcBuff, size_t dwLen)
{
	BYTE btChar;
	LPBYTE lpbOutBuff = (LPBYTE)lpcOutBuff, lpbInBuff = (LPBYTE)lpcBuff;

	for (size_t i = 0; i < dwLen; i++) {
		btChar = (*lpbInBuff++);
		if (btChar >= '0' && btChar <= '9') (*lpbOutBuff++) = btChar;
	}
	*lpbOutBuff = 0;

	return lpbOutBuff-(LPBYTE)lpcOutBuff;
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
			WCHAR szBuff[STATUS_TITLE_MAX+STATUS_DESC_MAX];

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
			if (dat->ppro->mraGetStaticStringW(NULL, szValueName, szBuff, (STATUS_TITLE_MAX+1), NULL))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, szBuff); // custom xstatus name
			else // default xstatus name
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, TranslateW(lpcszXStatusNameDef[dat->dwXStatus]));

			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dat->dwXStatus);
			if (dat->ppro->mraGetStaticStringW(NULL, szValueName, szBuff, (STATUS_DESC_MAX+1), NULL))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_XMSG, szBuff); // custom xstatus description
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
			size_t dwBuffSize;
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

			dwBuffSize = GET_DLG_ITEM_TEXT(hWndDlg, IDC_XMSG, szBuff, (STATUS_DESC_MAX+1));
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%ldMsg", dat->dwXStatus);
			dat->ppro->mraSetStringExW(NULL, szValueName, szBuff, dwBuffSize);
			dat->ppro->mraSetStringExW(NULL, DBSETTING_XSTATUSMSG, szBuff, dwBuffSize);

			dwBuffSize = GET_DLG_ITEM_TEXT(hWndDlg, IDC_XTITLE, szBuff, (STATUS_TITLE_MAX+1));
			if (dwBuffSize == 0) { // user delete all text
				lstrcpynW(szBuff, TranslateW(lpcszXStatusNameDef[dat->dwXStatus]), (STATUS_TITLE_MAX+1));
				dwBuffSize = lstrlenW(szBuff);
			}
			mir_snprintf(szValueName, SIZEOF(szValueName), "XStatus%dName", dat->dwXStatus);
			dat->ppro->mraSetStringExW(NULL, szValueName, szBuff, dwBuffSize);
			dat->ppro->mraSetStringExW(NULL, DBSETTING_XSTATUSNAME, szBuff, dwBuffSize);

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

BOOL CMraProto::MraRequestXStatusDetails(DWORD dwXStatus)
{
	if ( IsXStatusValid(dwXStatus)) {
		SetXStatusData *dat = (SetXStatusData*)mir_calloc(sizeof(SetXStatusData));
		dat->dwXStatus = dwXStatus;
		dat->ppro = this;
		return DialogBoxParam(masMraSettings.hInstance, MAKEINTRESOURCE(IDD_SETXSTATUS), NULL, SetXStatusDlgProc, (LPARAM)dat) != -1;
	}

	return FALSE;
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
			WCHAR szBuff[MICBLOG_STATUS_MAX];
			SYSTEMTIME stBlogStatusTime = {0};

			SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			HWND hWndEdit = GetDlgItem(hWndDlg, IDC_MSG_TO_SEND);
			WNDPROC OldMessageEditProc = (WNDPROC)SetWindowLongPtr(hWndEdit, GWLP_WNDPROC, (LONG_PTR)MessageEditSubclassProc);
			SetWindowLongPtr(hWndEdit, GWLP_USERDATA, (LONG_PTR)OldMessageEditProc);
			SendMessage(hWndEdit, EM_LIMITTEXT, MICBLOG_STATUS_MAX, 0);

			SendMessage(hWndDlg, WM_SETICON, ICON_BIG, (LPARAM)IconLibGetIcon(gdiMenuItems[5].hIconHandle));

			// blog status message
			if (dat->ppro->mraGetStaticStringW(dat->hContact, DBSETTING_BLOGSTATUS, szBuff, SIZEOF(szBuff), NULL))
				SET_DLG_ITEM_TEXT(hWndDlg, IDC_USER_BLOG_STATUS_MSG, szBuff);

			// reply to some user blog
			if (dat->hContact) {
				mir_sntprintf(szBuff, SIZEOF(szBuff), TranslateW(L"Reply to %s blog status"), GetContactNameW(dat->hContact));
				SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)szBuff);
			}
			else SendMessage(hWndDlg, WM_SETTEXT, 0, (LPARAM)TranslateW(L"Set my blog status"));

			DWORD dwTime = dat->ppro->mraGetDword(dat->hContact, DBSETTING_BLOGSTATUSTIME, 0);
			if (dwTime && MakeLocalSystemTimeFromTime32(dwTime, &stBlogStatusTime))
				mir_sntprintf(szBuff, SIZEOF(szBuff), L"%s: %04ld.%02ld.%02ld %02ld:%02ld", TranslateW(L"Writed"), stBlogStatusTime.wYear, stBlogStatusTime.wMonth, stBlogStatusTime.wDay, stBlogStatusTime.wHour, stBlogStatusTime.wMinute);
			else
				szBuff[0] = 0;

			SET_DLG_ITEM_TEXT(hWndDlg, IDC_STATIC_WRITED_TIME, szBuff);
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
					 dat->ppro->mraGetContactSettingBlob(dat->hContact, DBSETTING_BLOGSTATUSID, &dwBlogStatusID, sizeof(DWORDLONG), NULL);
				 }
				 else {
					 dwFlags = MRIM_BLOG_STATUS_UPDATE;
					 if (IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_CHK_NOTIFY)) dwFlags |= MRIM_BLOG_STATUS_NOTIFY;
					 dwBlogStatusID = 0;
				 }
				 dat->ppro->MraChangeUserBlogStatus(dwFlags, szBuff, dwBuffSize, dwBlogStatusID);
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

BOOL CMraProto::MraSendReplyBlogStatus(HANDLE hContact)
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


DWORD MemFillRandom(LPVOID lpBuff, size_t dwBuffSize)
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

BOOL CMraProto::SetPassDB(LPSTR lpszBuff, size_t dwBuffSize)
{
	BOOL bRet = FALSE;
	BYTE btRandomData[256], btCryptedPass[256] = {0}, bthmacSHA1[SHA1HashSize] = {0};
	char szEMail[MAX_EMAIL_LEN] = {0};
	size_t dwEMailSize;

#if /*defined (_DEBUG) ||*/ defined (REL_DEB)

	mraSetStringExA(NULL, "Pass", lpszBuff, dwBuffSize);
	bRet = TRUE;
#else
	if (dwBuffSize < 128)
	if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
		MemFillRandom(btRandomData, sizeof(btRandomData));

		SHA1GetDigest(lpszBuff, dwBuffSize, &btCryptedPass[1]);

		//BASE64EncodeUnSafe(lpszBuff, dwBuffSize, &btCryptedPass[(1+SHA1HashSize)], (sizeof(btCryptedPass)-1), &dwBuffSize);
		memmove(&btCryptedPass[(1+SHA1HashSize)], lpszBuff, dwBuffSize);
		btCryptedPass[0] = (BYTE)dwBuffSize;
		//memmove(&btCryptedPass[1], lpszBuff, dwBuffSize);

		hmac_sha1(btRandomData, sizeof(btRandomData), (BYTE*)szEMail, dwEMailSize, bthmacSHA1);

		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);
		RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, sizeof(btRandomData));
		CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);


		mraSetDword(NULL, "pCryptVer", MRA_PASS_CRYPT_VER);
		mraWriteContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData));
		mraWriteContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass));

		bRet = TRUE;
	}
#endif
return(bRet);
}


BOOL CMraProto::GetPassDB_v1(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize)
{
	BYTE btRandomData[256] = {0}, btCryptedPass[256] = {0}, bthmacSHA1[SHA1HashSize] = {0};
	char szEMail[MAX_EMAIL_LEN] = {0};
	size_t dwRandomDataSize, dwCryptedPass, dwEMailSize, dwPassSize;

	if (mraGetDword(NULL, "pCryptVer", 0) == 1)
	if (mraGetContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData), &dwRandomDataSize))
	if (dwRandomDataSize == sizeof(btRandomData))
	if (mraGetContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass), &dwCryptedPass))
	if (dwCryptedPass == sizeof(btCryptedPass))
	if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
		hmac_sha1(btRandomData, sizeof(btRandomData), (BYTE*)szEMail, dwEMailSize, bthmacSHA1);

		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);
		CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
		RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, dwRandomDataSize);
		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);

		dwPassSize = (*btCryptedPass);
		BASE64DecodeUnSafe(&btCryptedPass[(1+SHA1HashSize)], dwPassSize, &btCryptedPass[(1+SHA1HashSize)], (sizeof(btCryptedPass)-1), &dwPassSize);
		SHA1GetDigest(&btCryptedPass[(1+SHA1HashSize)], dwPassSize, btRandomData);
		if (MemoryCompare(&btCryptedPass[1], SHA1HashSize, btRandomData, SHA1HashSize) == CMEM_EQUAL)
		if (dwBuffSize >= dwPassSize) {
			memmove(lpszBuff, &btCryptedPass[(1+SHA1HashSize)], dwPassSize);
			*(lpszBuff + dwPassSize) = 0;

			if (pdwBuffSize)
				*pdwBuffSize = dwPassSize;
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CMraProto::GetPassDB_v2(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize)
{
	BYTE btRandomData[256] = {0}, btCryptedPass[256] = {0}, bthmacSHA1[SHA1HashSize] = {0};
	char szEMail[MAX_EMAIL_LEN] = {0};
	size_t dwRandomDataSize, dwCryptedPass, dwEMailSize, dwPassSize;

	if (mraGetDword(NULL, "pCryptVer", 0) == 2)
	if (mraGetContactSettingBlob(NULL, "pCryptData", btRandomData, sizeof(btRandomData), &dwRandomDataSize))
	if (dwRandomDataSize == sizeof(btRandomData))
	if (mraGetContactSettingBlob(NULL, "pCryptPass", btCryptedPass, sizeof(btCryptedPass), &dwCryptedPass))
	if (dwCryptedPass == sizeof(btCryptedPass))
	if (mraGetStaticStringA(NULL, "e-mail", szEMail, SIZEOF(szEMail), &dwEMailSize)) {
		hmac_sha1(btRandomData, sizeof(btRandomData), (BYTE*)szEMail, dwEMailSize, bthmacSHA1);

		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);
		CopyMemoryReverseDWORD(btCryptedPass, btCryptedPass, sizeof(btCryptedPass));
		RC4(btCryptedPass, sizeof(btCryptedPass), btRandomData, dwRandomDataSize);
		RC4(btCryptedPass, sizeof(btCryptedPass), bthmacSHA1, SHA1HashSize);

		dwPassSize = ((*btCryptedPass)&0xff);
		SHA1GetDigest(&btCryptedPass[(1+SHA1HashSize)], dwPassSize, btRandomData);
		if (MemoryCompare(&btCryptedPass[1], SHA1HashSize, btRandomData, SHA1HashSize) == CMEM_EQUAL)
		if (dwBuffSize >= dwPassSize)
		{
			memmove(lpszBuff, &btCryptedPass[(1+SHA1HashSize)], dwPassSize);
			(*(lpszBuff+dwPassSize)) = 0;

			if (pdwBuffSize) (*pdwBuffSize) = dwPassSize;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMraProto::GetPassDB(LPSTR lpszBuff, size_t dwBuffSize, size_t *pdwBuffSize)
{
	#if /*defined (_DEBUG) ||*/ defined (REL_DEB)
		mraGetStaticStringA(NULL, "Pass", lpszBuff, dwBuffSize, pdwBuffSize);
		return TRUE;
	#else
		switch (mraGetDword(NULL, "pCryptVer", 0)) {
		case 1:
			return GetPassDB_v1(lpszBuff, dwBuffSize, pdwBuffSize);
		case 2:
			return GetPassDB_v2(lpszBuff, dwBuffSize, pdwBuffSize);
		default:
			return FALSE;
		}
	#endif
}

DWORD ReplaceInBuff(LPVOID lpInBuff, size_t dwInBuffSize, size_t dwReplaceItemsCount, LPVOID *plpInReplaceItems, size_t *pdwInReplaceItemsCounts, LPVOID *plpOutReplaceItems, size_t *pdwOutReplaceItemsCounts, LPVOID lpOutBuff, size_t dwOutBuffSize, size_t *pdwOutBuffSize)
{
	DWORD dwRetErrorCode = NO_ERROR;
	LPBYTE *plpbtFounded;
	#ifdef _DEBUG //check tables
		for (size_t i = 0;i<dwReplaceItemsCount;i++)
		{
			if (lstrlen((LPTSTR)plpInReplaceItems[i]) != (pdwInReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
			if (lstrlen((LPTSTR)plpOutReplaceItems[i]) != (pdwOutReplaceItemsCounts[i]/sizeof(TCHAR))) DebugBreak();
		}
	#endif

	plpbtFounded = (LPBYTE*)mir_calloc((sizeof(LPBYTE)*dwReplaceItemsCount));
	if (plpbtFounded) {
		LPBYTE lpbtOutBuffCur, lpbtInBuffCur, lpbtInBuffCurPrev, lpbtOutBuffMax;
		size_t i, dwFirstFoundIndex = 0, dwFoundCount = 0, dwMemPartToCopy;

		lpbtInBuffCurPrev = (LPBYTE)lpInBuff;
		lpbtOutBuffCur = (LPBYTE)lpOutBuff;
		lpbtOutBuffMax = (((LPBYTE)lpOutBuff)+dwOutBuffSize);
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

		if (pdwOutBuffSize) (*pdwOutBuffSize) = (lpbtOutBuffCur-((LPBYTE)lpOutBuff));
	}
	else dwRetErrorCode = GetLastError();

	return dwRetErrorCode;
}

static const LPTSTR lpszXMLTags[]		 = {TEXT("&apos;"), 		TEXT("&quot;"), 		TEXT("&amp;"), 		TEXT("&lt;"), 		TEXT("&gt;")};
static const size_t dwXMLTagsCount[]	 = {(6*sizeof(TCHAR)), 	(6*sizeof(TCHAR)), 	(5*sizeof(TCHAR)), 	(4*sizeof(TCHAR)), 	(4*sizeof(TCHAR))};
static const LPTSTR lpszXMLSymbols[]	 = {TEXT("\'"), 			TEXT("\""), 			TEXT("&"), 			TEXT("<"), 			TEXT(">")};
static const size_t dwXMLSymbolsCount[]	 = {sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR), 		sizeof(TCHAR)};

//Decode XML coded string. The function translate special xml code into standard characters.
DWORD DecodeXML(LPTSTR lptszMessage, size_t dwMessageSize, LPTSTR lptszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize)
{
	DWORD dwRet = ReplaceInBuff(lptszMessage, (dwMessageSize*sizeof(TCHAR)), SIZEOF(lpszXMLTags), (LPVOID*)lpszXMLTags, (size_t*)dwXMLTagsCount, (LPVOID*)lpszXMLSymbols, (size_t*)dwXMLSymbolsCount, lptszMessageConverted, (dwMessageConvertedBuffSize*sizeof(TCHAR)), pdwMessageConvertedSize);
	if (pdwMessageConvertedSize)
		*pdwMessageConvertedSize /= sizeof(TCHAR);
	return dwRet;
}

//Encode XML coded string. The function translate special saved xml characters into special characters.
DWORD EncodeXML(LPTSTR lptszMessage, size_t dwMessageSize, LPTSTR lptszMessageConverted, size_t dwMessageConvertedBuffSize, size_t *pdwMessageConvertedSize)
{
	DWORD dwRet = ReplaceInBuff(lptszMessage, (dwMessageSize*sizeof(TCHAR)), SIZEOF(lpszXMLTags), (LPVOID*)lpszXMLSymbols, (size_t*)dwXMLSymbolsCount, (LPVOID*)lpszXMLTags, (size_t*)dwXMLTagsCount, lptszMessageConverted, (dwMessageConvertedBuffSize*sizeof(TCHAR)), pdwMessageConvertedSize);
	if (pdwMessageConvertedSize)
		*pdwMessageConvertedSize /= sizeof(TCHAR);
	return dwRet;
}
