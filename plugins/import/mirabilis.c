/*

Import plugin for Miranda IM

Copyright (C) 2001-2005 Martin Öberg, Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

// ==============
// == INCLUDES ==
// ==============

#include "import.h"
#include "mirabilis.h"
#include "resource.h"

BOOL IsDuplicateEvent(HANDLE hContact, DBEVENTINFO dbei);
BOOL IsProtocolLoaded(char* pszProtocolName);
HANDLE HContactFromNumericID(char* pszProtoName, char* pszSetting, DWORD dwID);
HANDLE AddContact(HWND hdlgProgress, char* pszProtoName, char* pszUniqueSetting, DBVARIANT* id, DBVARIANT* nick, DBVARIANT* group);

// ====================
// ====================
// == IMPLEMENTATION ==
// ====================
// ====================

static void SearchForDatabases(HWND hdlg, const TCHAR *dbPath, const TCHAR *type)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szRootName[MAX_PATH],*str2;

	int i;

	wsprintf(szSearchPath, _T("%s\\*.idx"), dbPath);
	hFind=FindFirstFile(szSearchPath,&fd);
	if(hFind!=INVALID_HANDLE_VALUE) {
		do {
			lstrcpy(szRootName,fd.cFileName);
			str2=_tcsrchr(szRootName,'.');
			if(str2!=NULL) *str2=0;
			if(lstrlen(szRootName)>3 && !lstrcmpi(szRootName+lstrlen(szRootName)-3,_T("tmp")))
				continue;
			lstrcat(szRootName,type);
			i=SendDlgItemMessage(hdlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)szRootName);
			str2 = (TCHAR*)mir_alloc((lstrlen(dbPath) + 2+lstrlen(fd.cFileName))*sizeof(TCHAR));
			wsprintf(str2, _T("%s\\%s"), dbPath, fd.cFileName);
			SendDlgItemMessage(hdlg,IDC_LIST,LB_SETITEMDATA,i,(LPARAM)str2);
		}
			while( FindNextFile( hFind, &fd ));

		FindClose(hFind);
	}
}

INT_PTR CALLBACK MirabilisPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		{
			HKEY hKey;
			LONG lResult;
			int i;
			TranslateDialogDefault(hdlg);
			if (ERROR_SUCCESS != (lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Mirabilis\\ICQ\\DefaultPrefs"), 0, KEY_QUERY_VALUE, &hKey)))
				lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Mirabilis\\ICQ\\DefaultPrefs"), 0, KEY_QUERY_VALUE, &hKey);

			if (lResult == ERROR_SUCCESS) {
				TCHAR dbPath[MAX_PATH];
				DWORD cch;
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("New Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (99a)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("99b Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (99b)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2000a Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2000a)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2000b Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2000b)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2001a Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2001a)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2001b Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2001b)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2002a Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2002a)"));
				cch=sizeof(dbPath);
				if(ERROR_SUCCESS==RegQueryValueEx(hKey,_T("2003a Database"),NULL,NULL,(LPBYTE)dbPath,&cch))
					SearchForDatabases(hdlg,dbPath,_T(" (2003a)"));
			}

			for (i = 0; i < cICQAccounts; i++)
			{
				SendDlgItemMessage(hdlg, IDC_MIRABILISACCOUNT, CB_ADDSTRING, 0, (LPARAM)tszICQAccountName[i]);
			}
			SendDlgItemMessage(hdlg, IDC_MIRABILISACCOUNT, CB_SETCURSEL, 0, 0);
			
			SetTimer(hdlg,1,2000,NULL);
			SendMessage(hdlg,WM_TIMER,0,0);
			return TRUE;
		}
	case WM_TIMER:
		{	HANDLE hMirabilisMutex;
			hMirabilisMutex=OpenMutexA(MUTEX_ALL_ACCESS,FALSE,"Mirabilis ICQ Mutex");
			if(hMirabilisMutex!=NULL) {
				CloseHandle(hMirabilisMutex);
				ShowWindow(GetDlgItem(hdlg,IDC_MIRABILISRUNNING),SW_SHOW);
			}
			else ShowWindow(GetDlgItem(hdlg,IDC_MIRABILISRUNNING),SW_HIDE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_IMPORTTYPE,(LPARAM)ImportTypePageProc);
			break;
		case IDOK:
			{	TCHAR filename[MAX_PATH];
				GetDlgItemText(hdlg,IDC_FILENAME,filename,SIZEOF(filename));
				if(_taccess(filename,4)) {
					MessageBox(hdlg,TranslateT("The given file does not exist. Please check that you have entered the name correctly."),TranslateT("Mirabilis Import"),MB_OK);
					break;
				}
				lstrcpy(importFile,filename);
				iICQAccount = SendDlgItemMessage(hdlg, IDC_MIRABILISACCOUNT, CB_GETCURSEL, 0, 0);
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_OPTIONS,(LPARAM)MirabilisOptionsPageProc);
				break;
			}
		case IDCANCEL:
			PostMessage(GetParent(hdlg),WM_CLOSE,0,0);
			break;
		case IDC_LIST:
			if(HIWORD(wParam)==LBN_SELCHANGE) {
				int sel=SendDlgItemMessage(hdlg,IDC_LIST,LB_GETCURSEL,0,0);
				if(sel==LB_ERR) break;
				SetDlgItemText(hdlg,IDC_FILENAME,(TCHAR*)SendDlgItemMessage(hdlg,IDC_LIST,LB_GETITEMDATA,sel,0));
			}
			break;
		case IDC_OTHER:
			{	OPENFILENAME ofn;
				TCHAR str[MAX_PATH], text[256];
				int index;

				// TranslateTS doesnt translate \0 separated strings
				index = mir_sntprintf(text, 64, _T("%s (*.idx)"), TranslateT("Mirabilis ICQ database indexes")) + 1;
				_tcscpy(text + index, _T("*.idx")); index += 6;
				index += mir_sntprintf(text + index, 64, _T("%s (*.*)"), TranslateT("All Files")) + 1;
				_tcscpy(text + index, _T("*.*")); index += 4;
				text[index] = 0;

				GetDlgItemText(hdlg,IDC_FILENAME,str,SIZEOF(str));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hdlg;
				ofn.lpstrFilter = text;
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
				ofn.nMaxFile = SIZEOF(str);
				ofn.lpstrDefExt = _T("idx");
				if(GetOpenFileName(&ofn))
					SetDlgItemText(hdlg,IDC_FILENAME,str);
				break;
			}
		}
		break;

	case WM_DESTROY:
		{	int i;
			for(i=SendDlgItemMessage(hdlg,IDC_LIST,LB_GETCOUNT,0,0)-1;i>=0;i--)
				mir_free((char*)SendDlgItemMessage(hdlg,IDC_LIST,LB_GETITEMDATA,i,0));
			break;
		}
	}
	return FALSE;
}


INT_PTR CALLBACK MirabilisOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		EnableWindow(GetDlgItem(hdlg, IDC_RADIO_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_STATIC_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_RADIO_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_STATIC_CONTACTS), TRUE);
		CheckDlgButton(hdlg, IDC_RADIO_ALL, BST_CHECKED);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_MIRABILISDB, (LPARAM)MirabilisPageProc);
			break;
		case IDOK:
			if (IsDlgButtonChecked(hdlg, IDC_RADIO_ALL)) {
				DoImport = MirabilisImport;
				nImportOption = IMPORT_ALL;
				nCustomOptions = IOPT_MSGSENT|IOPT_MSGRECV|IOPT_URLSENT|IOPT_URLRECV;
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}
			if (IsDlgButtonChecked(hdlg, IDC_RADIO_CONTACTS)) {
				DoImport = MirabilisImport;
				nImportOption = IMPORT_CONTACTS;
				nCustomOptions = 0;
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}
			break;
		case IDCANCEL:
			PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
			break;
		}
		break;
	}

	return FALSE;
}

static int GetHighestIndexEntry(void)
{
	struct TIdxIndexEntry *entry;
	DWORD ofs;

	ofs=*(PDWORD)(pIdx+12);
	for(;;) {
		entry=(struct TIdxIndexEntry*)(pIdx+ofs);
		if(entry->entryIdLow==(DWORD)-2) return ((struct TIdxDatEntry*)entry)->entryId;
		if(entry->ofsHigher>=0xF0000000) ofs=entry->ofsInHere;
		else ofs=entry->ofsHigher;
	}
}

static int GetIdDatOfs(DWORD id)
{
	struct TIdxIndexEntry *entry;
	DWORD ofs = *(PDWORD)(pIdx+12);
	for(;;) {
		entry=(struct TIdxIndexEntry*)(pIdx+ofs);
		if(entry->entryIdLow==(DWORD)-2) {
			if(entry->entryIdHigh==id) return ((struct TIdxDatEntry*)entry)->datOfs;
			return 0;
		}
		if(id<entry->entryIdLow) ofs=entry->ofsLower;
		else if(entry->ofsHigher<0xF0000000 && id>=entry->entryIdHigh) ofs=entry->ofsHigher;
		else ofs=entry->ofsInHere;
	}
	return 0;
}

static int GetDatEntryType(DWORD ofs)
{
	return *(int*)(pDat+ofs+4);
}

DWORD GetDBVersion()
{
	dwDBVersion = *(PDWORD)(pIdx+16);

	switch (dwDBVersion) {
	case DBV99A:
		AddMessage( LPGEN("This looks like a ICQ 99a database."));
		break;
	case DBV99B:
		AddMessage( LPGEN("This looks like a ICQ 99b database."));
		break;
	case DBV2000A:
		AddMessage( LPGEN("This looks like a ICQ 2000a database."));
		break;
	case DBV2000B:
		AddMessage( LPGEN("This looks like a ICQ 2000b database."));
		break;
	case DBV2001A:
		AddMessage( LPGEN("This looks like a ICQ 2001, 2002 or 2003a database."));
		break;
	default:
		AddMessage( LPGEN("This database is an unknown version."));
		return 0;
	}

	return dwDBVersion;
}

int GetEntryVersion(WORD wSeparatorValue)
{
	int nVersion;

	if (wSeparatorValue < ENTRYV99A)
		nVersion = 0; // Cannot handle ICQ98 contacts
	else if ((wSeparatorValue >= ENTRYV99A) && (wSeparatorValue < ENTRYV99B))
		nVersion = ENTRYV99A;
	else if ((wSeparatorValue >= ENTRYV99B) && (wSeparatorValue < ENTRYV2000A))
		nVersion = ENTRYV99B;
	else if ((wSeparatorValue >= ENTRYV2000A) && (wSeparatorValue < ENTRYV2000B))
		nVersion = ENTRYV2000A;
	else if ((wSeparatorValue >= ENTRYV2000B) && (wSeparatorValue < ENTRYV2001A))
		nVersion = ENTRYV2000B;
	else if ((wSeparatorValue >= ENTRYV2001A) && (wSeparatorValue < ENTRYV2001B))
		nVersion = ENTRYV2001A;
	else if ((wSeparatorValue >= ENTRYV2001B) && (wSeparatorValue < ENTRYV2002A))
		nVersion = ENTRYV2001B;
	else if (wSeparatorValue >= ENTRYV2002A)
		nVersion = ENTRYV2002A;
	else
		nVersion = ENTRYVUNKNOWN; // Just in case... Skip undocumented contact versions

	return nVersion;
}

DWORD ReadSubList(DWORD dwOffset)
{
	DWORD dwSubType, dwProperties, n;

	#ifdef _LOGGING
		AddMessage( LPGEN("Attempting to parse sub list at offset %u."), dwOffset);
	#endif

	// Check number of properties in sub list
	dwProperties = *(PDWORD)(pDat+dwOffset);
	dwOffset+=4;

	// Check sub list type
	dwSubType = *(PBYTE)(pDat+dwOffset);
	dwOffset+=1;

	switch (dwSubType){
	case 0x6B:
		for(n=0;n<dwProperties;n++) dwOffset+=*(PWORD)(pDat+dwOffset)+2;
		break;
	case 0x6E:
		for(n=0;n<dwProperties;n++) {
			if (!(dwOffset = ReadPropertyBlock(dwOffset, NULL, NULL))) return 0;
		}
		break;
	default:
		// Unknown sub list type
		AddMessage( LPGEN("Error: Unknown sub list type (%u) at offset %u."), dwSubType, dwOffset);
		return 0;
	}

	return dwOffset;
}

DWORD ReadPropertyBlock(DWORD dwOffset, char* SearchWord, int* nSearchResult)
{
	DWORD n, dwProperties, nameOfs;
	WORD nameLen;

	#ifdef _LOGGING
		AddMessage( LPGEN("Attempting to parse property block at offset %u."), dwOffset );
	#endif

	// Reset search result
	if (SearchWord) *nSearchResult = 0;

	// Check number of properties in block
	dwOffset+=2;
	dwProperties = *(PDWORD)(pDat+dwOffset);

	// Scan all properties and search for
	// 'SearchWord' (if it has been specified).
	dwOffset+=4;
	for(n=0;n<dwProperties;n++) {
		nameLen=*(PWORD)(pDat+dwOffset); // Length of current property name
		dwOffset+=2;
		nameOfs=dwOffset;  // Save pointer to start of name
		dwOffset+=nameLen; // dwOffset now points to property value type

		if ( SearchWord ) {
			// Is this the property we are searching for?
			if(!lstrcmpA((char*)(pDat+nameOfs),SearchWord)){
				*nSearchResult = 1;
				return dwOffset;
		}	}

		// Increase 'dwOffset' to point to length of next property
		switch(*(pDat+dwOffset)) {
		case 0x64:
		case 0x65: dwOffset+=2; break;
		case 0x66:
		case 0x67: dwOffset+=3; break;
		case 0x68:
		case 0x69: dwOffset+=5; break;
		case 0x6b: dwOffset+=*(PWORD)(pDat+dwOffset+1)+3; break;
		case 0x6d:
			dwOffset = ReadSubList(dwOffset+1);
			if (!dwOffset) return 0;
			break;
		case 0x6f: dwOffset+=*(PDWORD)(pDat+dwOffset+1)+5; break;
		default:
			// Unknown property value type
			AddMessage( LPGEN("Error: Unknown datatype (%u) at offset %u."), *(pDat+dwOffset), dwOffset);
			return 0;
	}	}

	// We have reached the end without finding
	// the property we searched for
	if (SearchWord) *nSearchResult = 0;

	// Return offset to the byte right after the
	// property list
	return dwOffset;
}

DWORD ReadPropertyBlockList(DWORD dwOffset, char* SearchWord, int* nSearchResult)
{
	DWORD dwBlocks, n;

	#ifdef _LOGGING
		AddMessage( LPGEN("Attempting to parse property block list at offset %u."), dwOffset );
	#endif

	// Check number of blocks in list
	dwBlocks = *(PDWORD)(pDat+dwOffset);

	// Scan all blocks and search for 'SearchWord' (if
	// it has been specified).
	dwOffset += 4;
	for(n = 0;n<dwBlocks;n++) {
		if (!(dwOffset = ReadPropertyBlock(dwOffset, SearchWord, nSearchResult))) {
			AddMessage( LPGEN("Failed to read Property block."));
			return 0;
		}
		if (SearchWord)
			// Was the property found in the block?
			if (*nSearchResult) return dwOffset;
	}

	// We have reached the end without finding
	// the property we searched for
	if (SearchWord) *nSearchResult = 0;

	// Return offset to the byte right after the
	// property list
	return dwOffset;
}

DWORD ReadWavList(DWORD dwOffset)
{
	DWORD dwWavEntries, n;
	WORD wNameLen;

	#ifdef _LOGGING
		AddMessage( LPGEN("Attempting to parse wav file list at offset %u."), dwOffset);
	#endif

	// Check number of wav entries
	dwWavEntries = *(PDWORD)(pDat+dwOffset);

	// Read entire list
	dwOffset+=4;
	for(n = 0;n<dwWavEntries;n++) {
		wNameLen = *(PWORD)(pDat+dwOffset+0x0A);
		dwOffset += wNameLen + 0x0C;
	}

	// Return the offset to the byte right
	// after the list
	return dwOffset;
}

DWORD FindMyDetails(void)
{
	DWORD dwOffset = GetIdDatOfs(1005);

	if (!dwOffset) return 0;
	if(*(PDWORD)(pDat+dwOffset+0x08) != 1005) return 0;
	if(*(PBYTE)(pDat+dwOffset+0x0C) != 0xE4) return 0;
	if(*(int*)(pDat+dwOffset+0x1e) != 'USER') return 0;
	if(*(PDWORD)(pDat+dwOffset+0x22) != 6) return 0;

	return dwOffset;
}

// dwOffset must point to MyDetails
DWORD FindGroupList(DWORD dwOffset)
{
	DWORD n, dwPhoneEntries;
	WORD wSeparatorValue;
	int nFormat;

	wSeparatorValue = *(PWORD)(pDat+dwOffset+0x1c);
	nFormat = GetEntryVersion(wSeparatorValue);

	#ifdef _LOGGING
		AddMessage( LPGEN("Attempting to parse group list, type %d."), nFormat );
	#endif

	switch (nFormat) {
	case ENTRYV99A:
		if (!(dwOffset = ReadWavList(dwOffset+0x54))) return 0;
		if (!(dwOffset = ReadPropertyBlock(dwOffset+38, NULL, NULL))) return 0;
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // User name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Nick name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // First name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Last name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Primary e-mail
		dwOffset += 0x13;                      // Various fixed length data
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home city
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home state
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Additional details
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // User homepage
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home phone number
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Notes
		dwOffset += 0x08;                      // Various fixed length data
		dwOffset += 0x04;                      // ++ UNKNOWN ++
		dwPhoneEntries = *(PDWORD)(pDat+dwOffset); // Phonebook starts here
		dwOffset += 0x04;
		for(n = 0;n<dwPhoneEntries;n++){
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += 2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
		}
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Picture file name
		dwOffset += 0x06;                      // ++ UNKNOWN ++ (8 enligt spec)
		dwOffset += 0x06;                      // Various fixed length data
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Secondary e-mail
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Old e-mail
		dwOffset += 0x04;                      // ++ UNKNOWN ++
		dwOffset += 0x03;                      // Various fixed length data
		dwOffset += 0x08;                      // ++ UNKNOWN ++
		dwOffset += 0x03;                      // Various fixed length data
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home street address
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home fax number
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Home cell phone number
		dwOffset += 0x04;                      // ++ UNKNOWN ++
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Company Div/Dept
		dwOffset += 0x01;                      // Occupation
		dwOffset += 0x04;                      // ++ UNKNOWN ++
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Company position
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Company name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work street address
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work state
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work city
		dwOffset += 0x08;                      // Various fixed length data
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work phone number
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work fax number
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Work homepage
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Past background #1
		dwOffset += 0x02;                      // Past background #1 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Past background #2
		dwOffset += 0x02;                      // Past background #2 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Past background #3
		dwOffset += 0x02;                      // Past background #3 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Affiliation #1
		dwOffset += 0x02;                      // Affiliation #1 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Affiliation #2
		dwOffset += 0x02;                      // Affiliation #2 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Affiliation #3
		dwOffset += 0x02;                      // Affiliation #3 category
		dwOffset += 0x14;                      // ++ UNKNOWN ++
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Interest #1
		dwOffset += 0x02;                      // Interest #1 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Interest #2
		dwOffset += 0x02;                      // Interest #2 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Interest #3
		dwOffset += 0x02;                      // Interest #3 category
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Interest #4
		dwOffset += 0x02;                      // Interest #4 category
		dwOffset += 0x28;                      // ++ UNKNOWN ++
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // Password
		dwOffset += 0x04;                      // ++ UNKNOWN ++
		dwOffset += 0x0E;                      // ++ UNKNOWN ++
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // POP3 account name
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // POP3 account password
		dwOffset += *(PWORD)(pDat+dwOffset)+2; // POP server name
		dwOffset += 0x15;                      // ++ UNKNOWN ++
		return dwOffset;

	case ENTRYV99B:
		if (!(dwOffset = ReadWavList(dwOffset+0x2C))) return 0;
		if (!(dwOffset = ReadPropertyBlockList(dwOffset+0x02, NULL, NULL))) return 0;
		dwOffset += 0x08;
		dwPhoneEntries = *(PDWORD)(pDat+dwOffset); // Phonebook
		dwOffset += 0x04;
		for(n = 0;n<dwPhoneEntries;n++){
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
			dwOffset += 2;
			dwOffset += *(PWORD)(pDat+dwOffset)+2;
		}
		return dwOffset + 0x06;

	case ENTRYV2000A:
	case ENTRYV2000B:
	case ENTRYV2002A:
		if (!(dwOffset = ReadWavList(dwOffset+0x2C))) return 0;
		if (!(dwOffset = ReadPropertyBlockList(dwOffset+0x02, NULL, NULL))) return 0;
		return dwOffset + 0x06;

	case ENTRYV2001A:
	case ENTRYV2001B:
		if (!(dwOffset = ReadPropertyBlockList(dwOffset+0x2C, NULL, NULL))) return 0;
		return dwOffset + 0x06;

	default:
		AddMessage( LPGEN("default"));
		return 0;
	}
}

// ------------------------------------------------
// Finds the name of a group with a specific GroupID.
// ------------------------------------------------
// dwGroupID is the GroupID of the group.
// Returns a pointer to the name string or NULL if
// it was not found.
char* GetGroupName(DWORD dwGroupID)
{
	DWORD dwGroups, n, tmpOfs, dwOffset;
	char* strGroupName = 0;
	int nSearchResult;

	// Check for the existence of any group
	if (!dwGroupListOfs) return 0;
	dwOffset = dwGroupListOfs;
	dwGroups = *(PDWORD)(pDat + dwOffset);
	if (dwGroups == 0) return 0;

	dwOffset += 4;

	// Examine all groups
	switch (dwDBVersion) {
	case DBV99A:
	case DBV99B:
		for (n = 0; n < dwGroups; n++){
			if (dwGroupID == *(PDWORD)(pDat + dwOffset)){
				if (*(PWORD)(pDat + dwOffset + 4) > 1)
					return 6 + (char*)(pDat + dwOffset);

				break;
			}
			else
				// Skip to next group
				dwOffset += *(PWORD)(pDat + dwOffset + 4) + 12;
		}
		break;

	case DBV2000A:
	case DBV2000B:
	case DBV2001A:
		for (n = 0; n < dwGroups; n++){
			if (tmpOfs = ReadPropertyBlock(dwOffset, "GroupID", &nSearchResult)){
				if (nSearchResult) {
					if (dwGroupID == *(PDWORD)(pDat + tmpOfs + 1)){
						strGroupName = 3 + (char*)(pDat + ReadPropertyBlock(dwOffset, "GroupName", &nSearchResult));
						if (nSearchResult) {
							if ((DWORD)*(strGroupName - 2) > 1)
								return strGroupName;
							break;
			}	}	}	}

			// Skip to next group
			if ( dwOffset != ReadPropertyBlock(dwOffset, NULL, NULL))
				break;
		}
		break;
	}

	// The GroupID was not found, or it was found
	// but the group did not have a name, or there
	// was an error during parsing.
	return 0;
}

// ------------------------------------------------
// Scans a group list and adds all found groups to
// the Miranda contact list
// ------------------------------------------------
// dwOffset must point to the number of entries in
// the following group list.
// Returns the number of added groups, or -1 if an error
// occurred

int ImportGroups()
{
	DWORD dwGroups, n, tmpOfs, dwOffset;
	int nImported = 0;
	int nSearchResult, nFormat;
	WORD wSeparatorValue;

	if (!(dwOffset = FindMyDetails())) {
		AddMessage( LPGEN("ERROR: Failed to find owner information."));
		return -1;
	}

	wSeparatorValue = *(PWORD)(pDat + dwOffset + 0x1c);
	nFormat = GetEntryVersion(wSeparatorValue);

	dwGroupListOfs  = dwOffset = FindGroupList(dwOffset);
	if (!dwOffset) {
		AddMessage( LPGEN("ERROR: Failed to find contact list groups."));
		#ifdef _LOGGING
		{ // If this is a debug build, dump MyDetails block to disk
			FILE *stream;
			DWORD dwSize;
			dwOffset = FindMyDetails();
			dwSize = *(PDWORD)(pDat + dwOffset);
			stream = fopen("import_grouplist_dump.bin", "w");
			fwrite(pDat + dwOffset, 1, dwSize, stream);
			fclose(stream);
		}
		#endif
		return -1;
	}

	// Check number of groups
	dwGroups = *(PDWORD)(pDat + dwOffset);
	if (dwGroups > 0)
		AddMessage( LPGEN("Importing groups."));
	else {
		AddMessage( LPGEN("This database does not contain any contact groups."));
		return 0;
	}

	dwOffset += 4;

	// Import all groups with a name
	switch (nFormat) {
	case ENTRYV99A:
	case ENTRYV99B:
		for (n = 0; n < dwGroups; n++){
			if (*(PWORD)(pDat+dwOffset+4) > 1) {
				if ( CreateGroup(DBVT_ASCIIZ, (char*)(pDat + dwOffset) + 6, NULL ))
					nImported++;
				dwOffset += *(PWORD)(pDat + dwOffset + 4) + 12;
		}	}
		break;

	case ENTRYV2000A:
	case ENTRYV2000B:
	case ENTRYV2001A:
	case ENTRYV2001B:
	case ENTRYV2002A:
		for (n = 0; n < dwGroups; n++){
			if (tmpOfs = ReadPropertyBlock(dwOffset, "GroupName", &nSearchResult)){
				if (nSearchResult) {
					if (CreateGroup( DBVT_ASCIIZ, (char*)(pDat + tmpOfs + 3), NULL ))
						nImported++;
			}	}

			dwOffset = ReadPropertyBlock(dwOffset, NULL, NULL);
			if (!dwOffset) {
				AddMessage( LPGEN("ERROR: An error occurred while importing groups."));
				AddMessage( LPGEN("All groups may not have not been imported."));
				#ifdef _LOGGING
								{ // If this is a debug build, dump MyDetails block to disk
									FILE *stream;
									DWORD dwSize;
									dwOffset = FindMyDetails();
									dwSize = *(PDWORD)(pDat + dwOffset);
									stream = fopen("import_grouplist_dump.bin", "w");
									fwrite(pDat + dwOffset, 1, dwSize, stream);
									fclose(stream);
								}
				#endif
				return -1;
		}	}
		break;

	default:
		return -1;
	}

	return nImported;
}

// Imports the contact at offset dwOffset
// Returns the HANDLE of the Miranda contact
// or INVALID_HANDLE_VALUE on failure

HANDLE ImportContact(DWORD dwOffset)
{
	int nContactVersion, nSearchResult;
	BYTE Status;
	WORD wSeparatorValue;
	DWORD dwGroup, dwUIN = 0, tmpOfs = 0;
	char *strNickname = 0, *strGroupName = 0;

	if (*(int*)(pDat + dwOffset + 4) != DATENTRY_CONTACT)
		return INVALID_HANDLE_VALUE;

	if (*(int*)(pDat + dwOffset + 0x1e) != 'USER')
		return INVALID_HANDLE_VALUE;

	#ifdef _LOGGING
		{ // If this is a debug build, dump contact to disk
			FILE *stream;
			DWORD dwSize;
			dwSize = *(PDWORD)(pDat + dwOffset);
			stream = fopen("import_last_contact.bin", "w");
			fwrite(pDat + dwOffset, 1, dwSize, stream);
			fclose(stream);
		}
	#endif

	Status = *(pDat + dwOffset + 0x22);
	wSeparatorValue = *(PWORD)(pDat + dwOffset + 0x1c);
	nContactVersion = GetEntryVersion(wSeparatorValue);

	dwGroup = *(PDWORD)(pDat + dwOffset + 0x26);
	if (dwGroup >= 1000)
		strGroupName = GetGroupName(dwGroup);

	if (Status == 5)
		return INVALID_HANDLE_VALUE;	// Skip deleted contacts

	if ((Status != 2) && (Status != 3)) {
		AddMessage( LPGEN("Skipping inactive contact."));
		return INVALID_HANDLE_VALUE;
	}

	if ((nContactVersion < ENTRYV99A) || (nContactVersion == 0)) {
		AddMessage( LPGEN("Skipping contact with unsupported version."));
		return INVALID_HANDLE_VALUE;
	}

	switch(nContactVersion){
	case ENTRYV99A:
		if (!(dwOffset = ReadWavList(dwOffset + 0x54))) return INVALID_HANDLE_VALUE;
		if (!(dwOffset = ReadPropertyBlock(dwOffset + 0x26, NULL, NULL))) return INVALID_HANDLE_VALUE;
		// Check for custom nickname
		if (*(PWORD)(pDat + dwOffset) > 1) strNickname = (char*)(dwOffset + pDat + 2);
		// Find UIN
		dwOffset += *(PWORD)(pDat + dwOffset) + 2;	// Custom nick name
		dwOffset += *(PWORD)(pDat + dwOffset) + 2;	// Nick name
		dwOffset += *(PWORD)(pDat + dwOffset) + 2;	// First name
		dwOffset += *(PWORD)(pDat + dwOffset) + 2;	// Last name
		dwOffset += *(PWORD)(pDat + dwOffset) + 2;	// E-mail
		dwUIN = *(PDWORD)(pDat + dwOffset);         // UIN
		break;

	case ENTRYV99B:
	case ENTRYV2000A:
	case ENTRYV2000B:
		if (!(dwOffset = ReadWavList(dwOffset + 0x2C))) return INVALID_HANDLE_VALUE;
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x02, "UIN", &nSearchResult);
		if (nSearchResult) dwUIN = *(PDWORD)(pDat + tmpOfs + 1);
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x02, "MyDefinedHandle", &nSearchResult);
		if (nSearchResult) strNickname = (char*)(tmpOfs + pDat + 3);
		break;

	case ENTRYV2001A:
	case ENTRYV2001B:
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x2C, "MyDefinedHandle", &nSearchResult);
		if (nSearchResult) strNickname = (char*)(tmpOfs + pDat + 3);
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x2C, "UIN", &nSearchResult);
		if (nSearchResult) dwUIN = *(PDWORD)(pDat + tmpOfs + 1);
		break;

	case ENTRYV2002A:
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x32, "MyDefinedHandle", &nSearchResult);
		if (nSearchResult) strNickname = (char*)(tmpOfs + pDat + 3);
		tmpOfs = ReadPropertyBlockList(dwOffset + 0x32, "UIN", &nSearchResult);
		if (nSearchResult) dwUIN = *(PDWORD)(pDat + tmpOfs + 1);
		break;
	}

	if (!dwUIN) {
		AddMessage( LPGEN("Skipping unrecognizable contact."));
		return INVALID_HANDLE_VALUE;
	}

	if (dwUIN < 10000) {
		AddMessage( LPGEN("Skipping non-ICQ contact %u."), dwUIN );
		return INVALID_HANDLE_VALUE;
	}

	if (HContactFromNumericID( szICQModuleName[ iICQAccount ], "UIN", dwUIN) == INVALID_HANDLE_VALUE) {
		DBVARIANT id, nick, group;
		id.type = DBVT_DWORD; id.dVal = dwUIN;
		if ( strNickname != NULL && strlen(strNickname) > 0 )
			nick.type = DBVT_ASCIIZ, nick.pszVal = strNickname;
		else
			nick.type = DBVT_DELETED;
		group.type = DBVT_ASCIIZ, group.pszVal = strGroupName;
		return AddContact(hdlgProgress, szICQModuleName[ iICQAccount ], "UIN", &id, &nick, &group);
	}
	else {
		if ((strNickname != NULL) && (strlen(strNickname) > 0))
			AddMessage( LPGEN("Skipping duplicate ICQ contact %u, %s"), dwUIN, strNickname);
		else
			AddMessage( LPGEN("Skipping duplicate ICQ contact %u"), dwUIN);
	}

	// Failure
	return INVALID_HANDLE_VALUE;
}

BOOL ImportMessage(DWORD dwOffset)
{
	struct TDatMessage *msg = (struct TDatMessage*)(pDat + dwOffset);
	struct TDatEntryFooter *footer;
	DBEVENTINFO dbei;
	HANDLE hContact;
	int nUCTOffset;
	TIME_ZONE_INFORMATION TimeZoneInformation;
	int nHistoryCount = 0;

	// Get timestamp offset. In ICQ, event timestamps are stored
	// as UTC + (0-TZ offset). YES! That's the negation of the
	// timezone offset, only God and Mirabilis knows why.
	GetTimeZoneInformation(&TimeZoneInformation);
	nUCTOffset = -TimeZoneInformation.Bias * 60;

	// Ignore messages in 'Deleted' folder
	if (msg->filingStatus&FILING_DELETED)
		return FALSE;

	// Skip messages from non-icq contacts
	if (msg->uin < 10000) {
		AddMessage( LPGEN("Ignoring msg from user %d at ofs %d."), msg->uin, dwOffset );
		return FALSE;
	}

	// Ignore received messages?
	if (( msg->filingStatus & FILING_RECEIVED ) && !( nCustomOptions & IOPT_MSGRECV ))
		return FALSE;

	// Ignores sent messages?
	if ( !(msg->filingStatus & FILING_RECEIVED) && !( nCustomOptions & IOPT_MSGSENT ))
		return FALSE;

	// Check if contact exists in Miranda database
	hContact = HistoryImportFindContact(hdlgProgress, szICQModuleName[ iICQAccount ], msg->uin, nCustomOptions&IOPT_ADDUNKNOWN);
	if (hContact == INVALID_HANDLE_VALUE)
		return FALSE; // Contact couldn't be found/added

	// Convert the event to a Miranda dbevent
	footer = (struct TDatEntryFooter*)(pDat + dwOffset + msg->textLen + offsetof(struct TDatMessage, text));
	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = footer->sent == 1 ? DBEF_SENT : DBEF_READ;
	dbei.szModule = szICQModuleName[ iICQAccount ];
	// Convert timestamp
	dbei.timestamp = footer->timestamp + nUCTOffset;
	dbei.cbBlob = msg->textLen;
	dbei.pBlob = (PBYTE)alloca(msg->textLen);
	CopyMemory(dbei.pBlob, msg->text, dbei.cbBlob);
	dbei.pBlob[dbei.cbBlob - 1] = 0;

	// Check for duplicate entries
	if (IsDuplicateEvent(hContact, dbei)) {
		nDupes++;
	}
	else {
		if (CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei))
			nMessagesCount++;
	}

	return TRUE;
}

BOOL ImportExtendedMessage(DWORD dwOffset)
{
	struct TDatMessage *msg = (struct TDatMessage*)(pDat + dwOffset);
	struct TDatEntryFooter *footer;
	DBEVENTINFO dbei;
	HANDLE hContact;
	int nUCTOffset;
	TIME_ZONE_INFORMATION TimeZoneInformation;
	int nHistoryCount = 0;
	char* pszText = 0;
	DWORD dwRichTextOffset = 0;
	DWORD wRichTextLength = 0;
	DWORD wLength = 0;
	BOOL bFreeMe = FALSE;

	// Get timestamp offset. In ICQ, event timestamps are stored
	// as UTC + (0-TZ offset). YES! That's the negation of the
	// timezone offset, only God and Mirabilis knows why.
	GetTimeZoneInformation(&TimeZoneInformation);
	nUCTOffset = -TimeZoneInformation.Bias * 60;

	// Ignore messages in 'Deleted' folder
	if (msg->filingStatus&FILING_DELETED)
		return FALSE;

	// Skip messages from non-icq contacts
	if (msg->uin < 10000) {
		AddMessage( LPGEN("Ignoring msg from user %d at ofs %d."), msg->uin, dwOffset );
		return FALSE;
	}

	// Ignore received messages?
	if (( msg->filingStatus & FILING_RECEIVED) && !( nCustomOptions & IOPT_MSGRECV ))
		return FALSE;

	// Ignore sent messages?
	if ( !( msg->filingStatus & FILING_RECEIVED ) && !( nCustomOptions & IOPT_MSGSENT ))
		return FALSE;

	// Check if contact exists in Miranda database
	hContact = HistoryImportFindContact(hdlgProgress, szICQModuleName[ iICQAccount ], msg->uin, nCustomOptions&IOPT_ADDUNKNOWN);
	if (hContact == INVALID_HANDLE_VALUE)
		return FALSE; // Contact couldn't be found/added

	// Find a piece of usable text content
	if (msg->textLen <= 1) {
		// Skip past the RTF segment
		wRichTextLength = *(PWORD)(pDat + dwOffset + 0x2A + msg->textLen + 0x21);
		dwRichTextOffset = dwOffset + 0x2A + msg->textLen + 0x23;

		// Use the UTF-8 text segment
		wLength = *(PWORD)(pDat + dwRichTextOffset + wRichTextLength);
		if (wLength <= 1) {
			AddMessage( LPGEN("Ignoring msg with no text from %d ofs %d."),  msg->uin, dwOffset );
			return FALSE;
		}
		pszText = _strdup(pDat + dwRichTextOffset + wRichTextLength + 2);
		bFreeMe = TRUE;
		mir_utf8decode(pszText, NULL);
		wLength = (DWORD)strlen(pszText)+1;
	}
	else {
		// Use the ANSI text segment
		wLength = msg->textLen;
		pszText = pDat + dwOffset + 0x2A;
	}

	// Convert the event to a Miranda dbevent
	footer = (struct TDatEntryFooter*)(pDat + dwOffset + msg->textLen + offsetof(struct TDatMessage, text));
	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = footer->sent == 1 ? DBEF_SENT : DBEF_READ;
	dbei.szModule = szICQModuleName[ iICQAccount ];
	// Convert timestamp
	dbei.timestamp = footer->timestamp + nUCTOffset;
	dbei.cbBlob = wLength;
	dbei.pBlob = (PBYTE)calloc(wLength,1);
	CopyMemory(dbei.pBlob, pszText, dbei.cbBlob);
	dbei.pBlob[dbei.cbBlob - 1] = 0;

	// Check for duplicate entries
	if (IsDuplicateEvent(hContact, dbei)) {
		nDupes++;
	}
	else {
		if (CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei))
			nMessagesCount++;
	}

	free(dbei.pBlob);
	if (bFreeMe)
		free(pszText);

	return TRUE;
}

BOOL ImportURLMessage(DWORD dwOffset)
{
	struct TDatMessage *msg = (struct TDatMessage*)(pDat + dwOffset);
	struct TDatEntryFooter *footer;
	DBEVENTINFO dbei;
	HANDLE hContact;
	int nUCTOffset;
	TIME_ZONE_INFORMATION TimeZoneInformation;
	int nHistoryCount = 0;
	char *pSeparator;

	// Get timestamp offset. In ICQ, event timestamps are stored
	// as UTC + (0-TZ offset). YES! That's the negation of the
	// timezone offset, only God and Mirabilis knows why.
	GetTimeZoneInformation(&TimeZoneInformation);
	nUCTOffset = -TimeZoneInformation.Bias * 60;

	// Ignore URLs in 'Deleted' folder
	if (msg->filingStatus&FILING_DELETED)
		return FALSE;

	// Skip URLs from non-icq contacts
	if (msg->uin < 10000) {
		AddMessage( LPGEN("Ignoring msg from user %d at ofs %d."), msg->uin, dwOffset );
		return FALSE;
	}

	// Ignore received URLs?
	if (( msg->filingStatus & FILING_RECEIVED ) && !( nCustomOptions & IOPT_URLRECV ))
		return FALSE;

	// Ignores sent URLs?
	if ( !( msg->filingStatus & FILING_RECEIVED ) && !( nCustomOptions & IOPT_URLSENT ))
		return FALSE;

	// Check if contact exists in Miranda database
	hContact = HistoryImportFindContact(hdlgProgress, szICQModuleName[ iICQAccount ], msg->uin, nCustomOptions&IOPT_ADDUNKNOWN);
	if (hContact == INVALID_HANDLE_VALUE)
		return FALSE; // Contact couldn't be found/added

	// Convert the event to a Miranda dbevent
	footer = (struct TDatEntryFooter*)(pDat + dwOffset + msg->textLen + offsetof(struct TDatMessage, text));
	ZeroMemory(&dbei, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_URL;
	dbei.flags = footer->sent == 1 ? DBEF_SENT : DBEF_READ;
	dbei.szModule = szICQModuleName[ iICQAccount ];
	// Convert timestamp
	dbei.timestamp = footer->timestamp + nUCTOffset;
	dbei.cbBlob = msg->textLen;
	dbei.pBlob = (PBYTE)alloca(msg->textLen);
	CopyMemory(dbei.pBlob, msg->text, dbei.cbBlob);
	dbei.pBlob[dbei.cbBlob - 1] = 0;
	// Separate URL and description
	pSeparator = strchr((char*)dbei.pBlob, 0xFE);
	if (pSeparator != NULL)
		*pSeparator = 0;

	// Check for duplicate entries
	if (IsDuplicateEvent(hContact, dbei))
		nDupes++;
	else if (CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei))
		nMessagesCount++;

	return TRUE;
}

BOOL ImportEvent(DWORD dwOffset)
{
	struct TDatMessage *msg = (struct TDatMessage*)(pDat + dwOffset);

	// Events have IDs > 2000
	if (msg->hdr.entryId < 2001) {
		AddMessage( LPGEN("Skipping event with ID < 2001."));
		return FALSE;
	}

	// Separate code paths based on the event signature
	switch (msg->hdr.subType) {

	case SUBTYPE_MESSAGE: // All kinds of messages
		switch (msg->type) {
		case 1: // Normal message
			if ((nCustomOptions&IOPT_MSGRECV) || (nCustomOptions&IOPT_MSGSENT)) {
				return ImportMessage(dwOffset);
			}
			break;

		case 4: // URL
			if ((nCustomOptions&IOPT_URLSENT) || (nCustomOptions&IOPT_URLRECV)) {
				return ImportURLMessage(dwOffset);
			}
			break;

		case 6: // Request for authorization
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Request for auth.' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 7: // Authorization request denied
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Auth. denied' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 8: // Authorization request accepted
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Auth. accepted' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 9: // System message
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'System message', ofs %d."), dwOffset );
			#endif
			break;

		case 12: // You were added
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'You were added' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 13: // WWWPager ?
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'WWW Pager' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 14: // Email Express ?
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Email Express' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 19: // Contact list
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Contact' msg, ofs %d."), dwOffset );
			#endif
			break;

		case 21: // Phonecall request?
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Phonecall' msg (?), ofs %d."), dwOffset );
			#endif
			break;

		case 26: // SMS request?
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'SMS' msg (?), ofs %d."), dwOffset );
			#endif
			break;

		case 29: // Active list invitation ??
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 29 msg, ofs %d."), dwOffset );
			#endif
			break;

		case 30: // Birthday reminder
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 'Birthday' msg (?), ofs %d."), dwOffset );
			#endif
			break;

		case 32: // Unknown (Tomer)
			#ifdef _LOGGING
				AddMessage( LPGEN("Skipping 32 msg, ofs %d."), dwOffset );
			#endif
			break;

		default:
			AddMessage( LPGEN("Skipping unknown 0xE0 subtype (%d), ofs %d."), msg->type, dwOffset );

			#ifdef _LOGGING
			{ // If this is a debug build, dump entry to disk
				FILE *stream;
				DWORD dwSize = *(PDWORD)(pDat + dwOffset);
				wsprintfA(str, "import_unknown_E0subtype_%u-%u.bin", msg->type, dwOffset);
				stream = fopen(str, "w");
				fwrite(pDat + dwOffset, 1, dwSize, stream);
				fclose(stream);
			}
			#endif

			return FALSE;
		}
		break;

	case SUBTYPE_CHATREQUEST: // 0xE1
		#ifdef _LOGGING
			if (nImportOption != IMPORT_CONTACTS)
				AddMessage( LPGEN("Skipping 'Chat request' msg, ofs %d."), dwOffset );
		#endif
		break;

	case SUBTYPE_FILEREQUEST: // 0xE2
		#ifdef _LOGGING
			if (nImportOption != IMPORT_CONTACTS)
				AddMessage( LPGEN("Skipping file message offset %d."), dwOffset );
		#endif
		break;

	case 0xE3: // External (IPhone, Battlecom) Maybe general voice calls?
		#ifdef _LOGGING
			if (nImportOption != IMPORT_CONTACTS)
				AddMessage( LPGEN("Skipping message type 0xE3 at offset %d."), dwOffset );
		#endif
		break;

	case 0xE4: // My details
		break;
	case 0xE5: // Contact
		break;
	case 0xE6: // Reminder
		break;
	case 0xE7: // Addressbook
		break;
	case 0xEC: // Voice message
		break;
	case 0xED: // Unknown, something to do with chatting and .CHT files
		//		if (importHistory) {
		//			wsprintf(str, "Skipping message type 0xED at offset %d.", dwOffset);
		//			AddMessage( LPGEN(str);
		//		}
		break;
	case 0xEE: // Note
		break;
	case 0xEF: // Event folder
		break;
		//	case 0xF0: // Unknown
		//		if (importHistory) {
		//			wsprintf(str, "Skipping message type 0xF0 at offset %d.", dwOffset);
		//			AddMessage( LPGEN(str);
		//		}
		//		break;
	case 0xF1: // Server list
		break;
		//	case 0xF6: // Unknown
		//		if (importHistory) {
		//			wsprintf(str, "Skipping message type 0xF6 at offset %d.", dwOffset);
		//			AddMessage( LPGEN(str);
		//		}
		//		break;
	case 0x50: // Extended message, ICQ 2000a+?
		if (nImportOption != IMPORT_CONTACTS) {
			return ImportExtendedMessage(dwOffset);
		}
		break;

	case 0xA0: // URL message type 2
		if (nImportOption != IMPORT_CONTACTS) {
			if ((msg->filingStatus&FILING_RECEIVED) || (nCustomOptions&IOPT_URLRECV)) {
				return ImportURLMessage(dwOffset);
			}
		}
		break;

	default:
		if (nImportOption != IMPORT_CONTACTS) {
			AddMessage( LPGEN("Skipping unknown event type %d at offset %d."), msg->hdr.subType, dwOffset );

#ifdef _LOGGING
			{ // If this is a debug build, dump entry to disk
				FILE *stream;
				DWORD dwSize;
				dwSize = *(PDWORD)(pDat + dwOffset);
				wsprintfA(str, "import_unknown_eventtype_%u-%u.bin", msg->hdr.subType, dwOffset);
				stream = fopen(str, "w");
				fwrite(pDat + dwOffset, 1, dwSize, stream);
				fclose(stream);
			}
#endif

		}
		break;
	}

	return FALSE;
}


static void MirabilisImport(HWND hdlgProgressWnd)
{
	HANDLE hIdx, hDat, hIdxMapping, hDatMapping;
	DWORD i, ofs, highestIndexEntry;
	TCHAR datFilename[MAX_PATH];
	MSG msg;
	DWORD dwTimer;


	int status = 0;
	hdlgProgress = hdlgProgressWnd;
	nDupes = nContactsCount = nMessagesCount = 0;

	SetProgress(0);
	lstrcpy(datFilename, importFile);
	{
		TCHAR* str2;
		str2 = _tcsrchr(datFilename,'.');
		if ( str2 != NULL )
			lstrcpy(str2, _T(".dat"));
	}

	hIdx = CreateFile(importFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hIdx == INVALID_HANDLE_VALUE) {
		AddMessage( LPGEN("Failed to open index file"));
		AddMessage( LPGEN("Import aborted"));
		SetProgress(100);
		return;
	}

	hDat = CreateFile(datFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDat == INVALID_HANDLE_VALUE) {
		AddMessage( LPGEN("Failed to open database file"));
		AddMessage( LPGEN("Import aborted"));
		SetProgress(100);
		return;
	}

	// Creating file mappings
	hIdxMapping = CreateFileMapping(hIdx, NULL, PAGE_READONLY, 0, 0, NULL);
	hDatMapping = CreateFileMapping(hDat, NULL, PAGE_READONLY, 0, 0, NULL);

	// Mapping views of files
	pIdx = (PBYTE)MapViewOfFile(hIdxMapping, FILE_MAP_READ, 0, 0, 0);
	pDat = (PBYTE)MapViewOfFile(hDatMapping, FILE_MAP_READ, 0, 0, 0);

	// Is this a supported format?
	if (GetDBVersion()) {
		AddMessage( "" );

		highestIndexEntry = GetHighestIndexEntry();

		// Import groups
		nGroupsCount = ImportGroups();
		if (nGroupsCount < 0) {
			AddMessage( LPGEN("Group import was not completed."));
			nGroupsCount = 0;
		}
		AddMessage( "" );

		// Start benchmark timer
		dwTimer = time(NULL);

		if ( !IsProtocolLoaded( szICQModuleName[iICQAccount] )) {
			AddMessage( LPGEN("ICQ account is not installed."));
			AddMessage( LPGEN("No ICQ contacts or history will be imported."));
			AddMessage( "" );
		}
		else {
			// Configure database for fast writing
			CallService(MS_DB_SETSAFETYMODE, FALSE, 0);

			// Import contacts
			AddMessage( LPGEN("Importing contacts"));
			for (i = 2001; i <= highestIndexEntry; i++) {     //event ids start at 2001
				if (!(i%10)) {
					if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				if (!(i%100))
					SetProgress(100 * (i - 2001) / (highestIndexEntry - 2001));

				ofs = GetIdDatOfs(i);
				if (ofs != 0) {
					if (ImportContact(ofs) != INVALID_HANDLE_VALUE)
						nContactsCount++;
				}
			}
			AddMessage( "" );

			// Import history
			if (nImportOption != IMPORT_CONTACTS) {
				AddMessage( LPGEN("Importing history (this may take a while)"));
				for (i = 2001; i <= highestIndexEntry; i++) {     //event ids start at 2001
					if (!(i%10)) {
						if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}

					if (!(i%100))
						SetProgress(100 * (i - 2001) / (highestIndexEntry - 2001));

					ofs = GetIdDatOfs(i);
					if (ofs != 0) ImportEvent(ofs);
				}
				AddMessage( "" );
			}

			// Restore database writing mode
			CallService(MS_DB_SETSAFETYMODE, TRUE, 0);
		}

		dwTimer = time(NULL) - dwTimer;

		AddMessage( LPGEN("Import completed in %d seconds."), dwTimer );
		SetProgress(100);
		AddMessage( LPGEN("Added %d contacts and %d groups."), nContactsCount, nGroupsCount );
		if ( nImportOption != IMPORT_CONTACTS )
			AddMessage( LPGEN("Added %d events and skipped %d duplicates."), nMessagesCount, nDupes );
	}

	UnmapViewOfFile(pDat);
	UnmapViewOfFile(pIdx);
	CloseHandle(hDatMapping);
	CloseHandle(hIdxMapping);
	CloseHandle(hDat);
	CloseHandle(hIdx);
}
