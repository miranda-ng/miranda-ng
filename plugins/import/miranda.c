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

#include "resource.h"
#include "mirandadb0700.h"

// ======================
// == GLOBAL FUNCTIONS ==
// ======================

HANDLE HContactFromNumericID(char* pszProtoName, char* pszSetting, DWORD dwID);
HANDLE HContactFromID(char* pszProtoName, char* pszSetting, char* pszID);

HANDLE AddContact(HWND hdlgProgress, char* pszProtoName, char* pszUniqueSetting, DBVARIANT* id, DBVARIANT* nick, DBVARIANT* group);

BOOL IsProtocolLoaded(char* pszProtocolName);
BOOL IsDuplicateEvent(HANDLE hContact, DBEVENTINFO dbei);

INT_PTR CALLBACK ImportTypePageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK FinishedPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ProgressPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK MirandaOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK MirandaAdvOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam);

// =====================
// == LOCAL FUNCTIONS ==
// =====================

void MirandaImport(HWND hdlgProgress);
int CheckFileFormat(HANDLE hFile);
static HANDLE ImportContact(HANDLE hDbFile, struct DBContact Contact);
static void ImportHistory(HANDLE hDbFile, struct DBContact Contact, PROTOCOLDESCRIPTOR **protocol, int protoCount);
static int ImportGroups(HANDLE hDbFile, struct DBHeader *pdbHeader);

#define NEWSTR_ALLOCA(A) (A==NULL)?NULL:strcpy((char*)alloca(strlen(A)+1),A)

// Comment: The Find* functions only return a file offset.
//          The Get* functions actually reads the requested
//          data from the file and gives you a pointer to a structure
//          containing the data.

DWORD FindFirstContact(struct DBHeader* pDbHeader);
DWORD FindNextContact(struct DBContact* pDbContact);
DWORD FindNextEvent(HANDLE hDbFile, DWORD dwOffset);
DWORD FindOwnerContact(struct DBHeader* pDbHeader);

int GetContactCount(struct DBHeader* pDbHeader);
BOOL GetContact(HANDLE hDbFile, DWORD dwOffset, struct DBContact* pDbContact);
BOOL GetSetting(HANDLE hDbFile, struct DBContact* pDbContact, char* pszModuleName, char* pszSettingName, DBVARIANT* pValue);
char* GetNextSetting(char* pDbSetting);
BOOL GetSettings(HANDLE hDbFile, DWORD dwOffset, struct DBContactSettings** pDbSettings);
struct DBContactSettings* GetSettingsGroupByModuleName(HANDLE hdbFile, struct DBContact* pDbContact, char* pszName);
DWORD GetBlobSize(struct DBContactSettings* pDbSettings);
int GetSettingByName(struct DBContactSettings* pDbSettings, char* pszSettingName, DBVARIANT* pValue);
int GetSettingValue(char* pBlob,DBVARIANT* pValue);

BOOL GetEvent(HANDLE hDbFile, DWORD dwOffset, DBEVENTINFO* pDBEI);
char* GetName(HANDLE hDbFile, DWORD dwOffset);


// ======================
// == GLOBAL VARIABLES ==
// ======================

extern void (*DoImport)(HWND);
extern int nImportOption;
extern int nCustomOptions;


// =====================
// == LOCAL VARIABLES ==
// =====================

TCHAR importFile[MAX_PATH];
HWND hdlgProgress;
DWORD dwFileSize;

DWORD nDupes;
DWORD nContactsCount;
DWORD nMessagesCount;
DWORD nGroupsCount;
DWORD nSkippedEvents;
DWORD nSkippedContacts;

time_t dwSinceDate = 0;

// =============
// == DEFINES ==
// =============

#define EVENTTYPE_MESSAGE   0
#define EVENTTYPE_URL       1
#define EVENTTYPE_FILE      1002


// Supported database versions
#define DB_INVALID 0x00000000  // Unknown or corrupted DAT
#define DB_000700  0x00000700  // Miranda 0.1.0.0 - 0.1.2.2+

// DAT file signature
struct DBSignature {
  char name[15];
  BYTE eof;
};

static struct DBSignature dbSignature={"Miranda ICQ DB",0x1A};

// ====================
// ====================
// == IMPLEMENTATION ==
// ====================
// ====================

static void SearchForLists(HWND hdlg, const TCHAR *mirandaPath, const TCHAR *mirandaProf, const TCHAR *pattern, const TCHAR *type)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szRootName[MAX_PATH];
	TCHAR* str2;
	int i;

	mir_sntprintf(szSearchPath, SIZEOF(szSearchPath), _T("%s\\%s"), mirandaPath, pattern);
	hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			_tcscpy(szRootName, fd.cFileName);
			str2 = _tcsrchr(szRootName, '.');
			if (str2 != NULL) *str2 = 0;
			if (mirandaProf == NULL || _tcsicmp(mirandaProf, szRootName))
			{
				_tcscat(szRootName, type);
				i = SendDlgItemMessage(hdlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)szRootName);
				str2 = (TCHAR*)mir_alloc((_tcslen(mirandaPath) + 2 + _tcslen(fd.cFileName)) * sizeof(TCHAR));
				wsprintf(str2, _T("%s\\%s"), mirandaPath, fd.cFileName);
				SendDlgItemMessage(hdlg, IDC_LIST, LB_SETITEMDATA, i, (LPARAM)str2);
			}
		}
		while( FindNextFile( hFind, &fd ));

		FindClose( hFind );
	}
}

INT_PTR CALLBACK MirandaPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			TCHAR *pfd, *pfd1, *pfd2, *pfn;

			REPLACEVARSDATA dat = {0};
			dat.cbSize = sizeof(dat);
			dat.dwFlags = RVF_TCHAR;

			pfd  = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_path%\\Profiles"), (LPARAM)&dat);
			pfd1 = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_path%"), (LPARAM)&dat);
			pfd2 = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_profile%"), (LPARAM)&dat);
			pfn  = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)_T("%miranda_profilename%"), (LPARAM)&dat);

			SearchForLists(hdlg, pfd2, pfn, _T("*.dat"), _T(" (Miranda IM v0.x)"));
			SearchForLists(hdlg, pfd1, NULL, _T("*.dat"), _T(" (Miranda IM v0.x)"));
			if (lstrcmpi(pfd, pfd2))
				SearchForLists(hdlg, pfd, NULL, _T("*.dat"), _T(" (Miranda IM v0.x)"));

			mir_free(pfn);
			mir_free(pfd2);
			mir_free(pfd1);
			mir_free(pfd);
			return TRUE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_IMPORTTYPE,(LPARAM)ImportTypePageProc);
			break;

		case IDOK:
			{
				TCHAR filename[MAX_PATH];

				GetDlgItemText(hdlg, IDC_FILENAME, filename, SIZEOF(filename));
				if (_taccess(filename, 4)) {
					MessageBox(hdlg, TranslateT("The given file does not exist. Please check that you have entered the name correctly."), TranslateT("Miranda Import"), MB_OK);
					break;
				}
				lstrcpy(importFile, filename);
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_OPTIONS,(LPARAM)MirandaOptionsPageProc);
			}
			break;

		case IDCANCEL:
			PostMessage(GetParent(hdlg),WM_CLOSE,0,0);
			break;

		case IDC_LIST:
			if(HIWORD(wParam)==LBN_SELCHANGE) {
				int sel = SendDlgItemMessage(hdlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) break;
				SetDlgItemText(hdlg, IDC_FILENAME, (TCHAR*)SendDlgItemMessage(hdlg, IDC_LIST, LB_GETITEMDATA, sel, 0));
			}
			break;

		case IDC_OTHER:
			{
				OPENFILENAME ofn;
				TCHAR str[MAX_PATH], text[256];
				TCHAR *pfd;
				int index;

				pfd = Utils_ReplaceVarsT(_T("%miranda_profile%"));

				// TranslateTS doesnt translate \0 separated strings
				index = mir_sntprintf(text, 64, _T("%s (*.dat)"), TranslateT("Miranda IM database")) + 1;
				_tcscpy(text + index, _T("*.dat")); index += 6;
				index += mir_sntprintf(text + index, 64, _T("%s (*.*)"), TranslateT("All Files")) + 1;
				_tcscpy(text + index, _T("*.*")); index += 4;
				text[index] = 0;

				GetDlgItemText(hdlg, IDC_FILENAME, str, SIZEOF(str));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hdlg;
				ofn.lpstrFilter = text;
				ofn.lpstrDefExt = _T("dat");
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
				ofn.nMaxFile = SIZEOF(str);
				ofn.lpstrInitialDir = pfd;
				if (GetOpenFileName(&ofn))
					SetDlgItemText(hdlg,IDC_FILENAME,str);

				mir_free(pfd);
				break;
			}
		}
		break;
	case WM_DESTROY:
		{
			int i;

			for(i=SendDlgItemMessage(hdlg,IDC_LIST,LB_GETCOUNT,0,0)-1;i>=0;i--)
				mir_free((char*)SendDlgItemMessage(hdlg,IDC_LIST,LB_GETITEMDATA,i,0));
			break;
	}	}

	return FALSE;
}


INT_PTR CALLBACK MirandaOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		EnableWindow(GetDlgItem(hdlg,IDC_RADIO_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg,IDC_STATIC_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg,IDC_RADIO_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hdlg,IDC_STATIC_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hdlg,IDC_RADIO_CUSTOM), TRUE);
		EnableWindow(GetDlgItem(hdlg,IDC_STATIC_CUSTOM), TRUE);
		CheckDlgButton(hdlg,IDC_RADIO_ALL,BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_MIRANDADB,(LPARAM)MirandaPageProc);
			break;

		case IDOK:
			if(IsDlgButtonChecked(hdlg,IDC_RADIO_ALL)) {
				nImportOption = IMPORT_ALL;
				nCustomOptions = 0;//IOPT_MSGSENT|IOPT_MSGRECV|IOPT_URLSENT|IOPT_URLRECV;
				DoImport = MirandaImport;
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_PROGRESS,(LPARAM)ProgressPageProc);
				break;
			}

			if(IsDlgButtonChecked(hdlg,IDC_RADIO_CONTACTS)) {
				nImportOption = IMPORT_CONTACTS;
				nCustomOptions = 0;
				DoImport = MirandaImport;
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_PROGRESS,(LPARAM)ProgressPageProc);
				break;
			}

			if(IsDlgButtonChecked(hdlg,IDC_RADIO_CUSTOM)) {
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_ADVOPTIONS,(LPARAM)MirandaAdvOptionsPageProc);
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

static const UINT InControls[]={IDC_IN_MSG,IDC_IN_URL,IDC_IN_FT,IDC_IN_OTHER};
static const UINT OutControls[]={IDC_OUT_MSG,IDC_OUT_URL,IDC_OUT_FT,IDC_OUT_OTHER};
static const UINT SysControls[]={IDC_CONTACTS, IDC_SYSTEM};

INT_PTR CALLBACK MirandaAdvOptionsPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			struct tm *TM = NULL;
			struct _SYSTEMTIME ST = {0};

			dwSinceDate = DBGetContactSettingDword(NULL,IMPORT_MODULE,"ImportSinceTS",time(NULL));

			TM = localtime(&dwSinceDate);

			ST.wYear = TM->tm_year + 1900;
			ST.wMonth = TM->tm_mon + 1;
			ST.wDay = TM->tm_mday;

    		DateTime_SetSystemtime(GetDlgItem(hdlg,IDC_DATETIMEPICKER),GDT_VALID,&ST);
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_OPTIONS,(LPARAM)MirandaOptionsPageProc);
			break;

		case IDOK:
			DoImport = MirandaImport;
			nImportOption = IMPORT_CUSTOM;
			nCustomOptions = 0;

			if (IsDlgButtonChecked(hdlg,IDC_CONTACTS))
				nCustomOptions |= IOPT_CONTACTS | IOPT_GROUPS;
			if (IsDlgButtonChecked(hdlg,IDC_SYSTEM))
				nCustomOptions |= IOPT_SYSTEM;

			// incoming
			if (IsDlgButtonChecked(hdlg,IDC_IN_MSG))
				nCustomOptions |= IOPT_MSGRECV;
			if (IsDlgButtonChecked(hdlg,IDC_IN_URL))
				nCustomOptions |= IOPT_URLRECV;
			if (IsDlgButtonChecked(hdlg,IDC_IN_FT))
				nCustomOptions |= IOPT_FILERECV;
			if (IsDlgButtonChecked(hdlg,IDC_IN_OTHER))
				nCustomOptions |= IOPT_OTHERRECV;

			// outgoing
			if (IsDlgButtonChecked(hdlg,IDC_OUT_MSG))
				nCustomOptions |= IOPT_MSGSENT;
			if (IsDlgButtonChecked(hdlg,IDC_OUT_URL))
				nCustomOptions |= IOPT_URLSENT;
			if (IsDlgButtonChecked(hdlg,IDC_OUT_FT))
				nCustomOptions |= IOPT_FILESENT;
			if (IsDlgButtonChecked(hdlg,IDC_OUT_OTHER))
				nCustomOptions |= IOPT_OTHERSENT;

			// since date
			dwSinceDate = 0;

			if ( IsDlgButtonChecked( hdlg, IDC_SINCE )) {
				struct _SYSTEMTIME ST = {0};

				if (DateTime_GetSystemtime(GetDlgItem(hdlg,IDC_DATETIMEPICKER), &ST) == GDT_VALID) {
					struct tm TM = {0};

					TM.tm_mday = ST.wDay;
					TM.tm_mon = ST.wMonth - 1;
					TM.tm_year = ST.wYear - 1900;

					dwSinceDate = mktime(&TM);

					DBWriteContactSettingDword(NULL,IMPORT_MODULE,"ImportSinceTS",dwSinceDate);
  			}	}

			if (nCustomOptions)
				PostMessage(GetParent(hdlg),WIZM_GOTOPAGE,IDD_PROGRESS,(LPARAM)ProgressPageProc);
			break;

		case IDCANCEL:
			PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
			break;

		case IDC_SINCE:
			EnableWindow(GetDlgItem(hdlg, IDC_DATETIMEPICKER), IsDlgButtonChecked(hdlg, IDC_SINCE));
			break;

		if (HIWORD(wParam) != STN_CLICKED)
			break;

		case IDC_ALL:
		case IDC_INCOMING:
		case IDC_OUTGOING:
			{
				int i;

				if (LOWORD(wParam) == IDC_ALL)
					for (i = 0; i < sizeof(SysControls)/sizeof(SysControls[0]); i++)
						CheckDlgButton(hdlg,SysControls[i], !IsDlgButtonChecked(hdlg,SysControls[i]));

				if (LOWORD(wParam) != IDC_OUTGOING)
					for (i = 0; i < sizeof(InControls)/sizeof(InControls[0]); i++)
						CheckDlgButton(hdlg,InControls[i], !IsDlgButtonChecked(hdlg,InControls[i]));

				if (LOWORD(wParam) != IDC_INCOMING)
					for (i = 0; i < sizeof(OutControls)/sizeof(OutControls[0]); i++)
						CheckDlgButton(hdlg,OutControls[i], !IsDlgButtonChecked(hdlg,OutControls[i]));
			}
			break;

		case IDC_MSG:
			CheckDlgButton(hdlg,IDC_IN_MSG, !IsDlgButtonChecked(hdlg,IDC_IN_MSG));
			CheckDlgButton(hdlg,IDC_OUT_MSG, !IsDlgButtonChecked(hdlg,IDC_OUT_MSG));
			break;

		case IDC_URL:
			CheckDlgButton(hdlg,IDC_IN_URL, !IsDlgButtonChecked(hdlg,IDC_IN_URL));
			CheckDlgButton(hdlg,IDC_OUT_URL, !IsDlgButtonChecked(hdlg,IDC_OUT_URL));
			break;

		case IDC_FT:
			CheckDlgButton(hdlg,IDC_IN_FT, !IsDlgButtonChecked(hdlg,IDC_IN_FT));
			CheckDlgButton(hdlg,IDC_OUT_FT, !IsDlgButtonChecked(hdlg,IDC_OUT_FT));
			break;

		case IDC_OTHER:
			CheckDlgButton(hdlg,IDC_IN_OTHER, !IsDlgButtonChecked(hdlg,IDC_IN_OTHER));
			CheckDlgButton(hdlg,IDC_OUT_OTHER, !IsDlgButtonChecked(hdlg,IDC_OUT_OTHER));
			break;
		}
		break;
	}
	return FALSE;
}

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

// Read header from file, returns null on failure
struct DBHeader* GetHeader(HANDLE hDbFile)
{
	struct DBHeader* pdbHeader;
	DWORD dwBytesRead;

	if (( pdbHeader = calloc(1, sizeof(struct DBHeader))) == NULL )
		return NULL;

	// Goto start of file
	if (SetFilePointer(hDbFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;

	// Read header
	if ( !ReadFile(hDbFile, pdbHeader, sizeof(struct DBHeader), &dwBytesRead, NULL ) ||
		  dwBytesRead != sizeof(struct DBHeader))
		return NULL;

	// Return pointer to header
	return pdbHeader;
}

int CheckFileFormat(HANDLE hDbFile)
{
	struct DBHeader* pdbHeader;

	// Read header
	if (( pdbHeader = GetHeader(hDbFile)) == NULL )
		return DB_INVALID;

	// Check header signature
	if (memcmp(pdbHeader->signature, &dbSignature, sizeof(pdbHeader->signature))) {
		AddMessage( LPGEN("Signature mismatch" ));
		return DB_INVALID;
	}

	// Determine Miranda version
	switch (pdbHeader->version) {
	case DB_000700:
		AddMessage( LPGEN("This looks like a Miranda database, version 0.1.0.0 or above." ));
		free(pdbHeader);
		return DB_000700;

	default:
		AddMessage( LPGEN("Version mismatch" ));
		free(pdbHeader);
		return DB_INVALID;
}	}

// High level Miranda DB access functions
// Returns true if pValue points to the requested value

BOOL GetSetting(HANDLE hDbFile, struct DBContact* pDbContact, char* pszModuleName, char* pszSettingName, DBVARIANT* pValue)
{
	struct DBContactSettings* pDbSettings;
	if ( pDbSettings = GetSettingsGroupByModuleName(hDbFile, pDbContact, pszModuleName)) {
		if ( GetSettingByName( pDbSettings, pszSettingName, pValue )) {
			free(pDbSettings);
			return TRUE;
		}
		#ifdef _LOGGING
			AddMessage( LPGEN("Failed to find setting %s" ), pszSettingName );
		#endif
		free(pDbSettings);
	}
#ifdef _LOGGING
	else AddMessage( LPGEN("Failed to find module %s" ), pszModuleName );
#endif

	// Search failed
	pValue->type = 0;
	return FALSE;
}

// **
// ** CONTACT CHAIN
// **

// Return offset to first contact
DWORD FindFirstContact(struct DBHeader* pDbHeader)
{
	if (!pDbHeader)
		return 0;

	return pDbHeader->ofsFirstContact;
}

DWORD FindOwnerContact(struct DBHeader* pDbHeader)
{
	if (!pDbHeader)
		return 0;

	return pDbHeader->ofsUser;
}

// Return offset to next contact
DWORD FindNextContact(struct DBContact* pDbContact)
{
	if (!pDbContact)
		return 0;

	if (pDbContact->signature != DBCONTACT_SIGNATURE)
		return 0;

	return pDbContact->ofsNext;
}


// Read the contact at offset 'dwOffset'
// Returns true if successful and pDbContact points to the contact struct
// pDbContact must point to allocated struct
BOOL GetContact(HANDLE hDbFile, DWORD dwOffset, struct DBContact* pDbContact)
{
	DWORD dwBytesRead;

	// Early reject
	if (dwOffset == 0 || dwOffset >= dwFileSize)
		return FALSE;

	// ** Read and verify the struct

	if (SetFilePointer(hDbFile, (LONG)dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;

	if ((!ReadFile(hDbFile, pDbContact, sizeof(struct DBContact), &dwBytesRead, NULL)) ||
		(dwBytesRead != sizeof(struct DBContact)))
		return FALSE;

	if ((pDbContact->signature != DBCONTACT_SIGNATURE) ||
		(pDbContact->ofsNext >= dwFileSize))
		return FALSE; // Contact corrupted

	return TRUE;
}

// Return ptr to next setting in settings struct
char* GetNextSetting(char* pDbSetting)
{
	// Get next setting
	pDbSetting = pDbSetting + *pDbSetting+1; // Skip name
	switch( *(BYTE*)pDbSetting ) {
	case DBVT_BYTE:
		pDbSetting = pDbSetting+1+1;
		break;

	case DBVT_WORD:
		pDbSetting = pDbSetting+1+2;
		break;

	case DBVT_DWORD:
		pDbSetting = pDbSetting+1+4;
		break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
	case DBVT_BLOB:
	case DBVTF_VARIABLELENGTH:
		pDbSetting = pDbSetting + 3 + *(WORD*)(pDbSetting+1);
		break;

	case DBVT_DELETED:
		AddMessage( LPGEN("DEBUG: Deleted setting treated as 0-length setting"));
		pDbSetting = pDbSetting+1;
		break;

	default:
		// Unknown datatype assert
		AddMessage( LPGEN("ERROR: Faulty settings chain"));
		return NULL;
	}

	return pDbSetting;
}


// **
// ** SETTINGS CHAIN
// **

// Return the settings at offset 'dwOffset'
BOOL GetSettingsGroup(HANDLE hDbFile, DWORD dwOffset, struct DBContactSettings** pDbSettings)
{
	DWORD dwBytesRead, dwBlobSize, dwHead;
	struct DBContactSettings pSettings;

	// Early reject
	if (dwOffset == 0 || dwOffset >= dwFileSize)
		return FALSE;

	// ** Read and verify the struct
	if (SetFilePointer(hDbFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;

	dwHead = offsetof(struct DBContactSettings, blob);
	if ((!ReadFile(hDbFile, &pSettings, dwHead, &dwBytesRead, NULL)) ||
		(dwBytesRead != dwHead))
		return FALSE;

	if (pSettings.signature != DBCONTACTSETTINGS_SIGNATURE)
		return FALSE; // Setttings corrupted

	// ** Read the struct and the following blob
	dwBlobSize = pSettings.cbBlob;
	if (!(*pDbSettings = calloc(1, sizeof(struct DBContactSettings) + dwBlobSize)))
		return FALSE;

	memcpy(*pDbSettings, &pSettings, dwHead );

	if ((!ReadFile(hDbFile, (*pDbSettings)->blob, sizeof(struct DBContactSettings) - dwHead + dwBlobSize, &dwBytesRead, NULL)) ||
		(dwBytesRead != sizeof(struct DBContactSettings) - dwHead + dwBlobSize))
	{
		free(*pDbSettings);
		return FALSE;
	}

	return TRUE;
}

// pDbContact is a ptr to a struct DBContact
// Returns pointer to a struct DBContactSettings or NULL
struct DBContactSettings* GetSettingsGroupByModuleName(HANDLE hDbFile, struct DBContact* pDbContact, char* pszName)
{
	char* pszGroupName;
	struct DBContactSettings* pSettingsGroup;
	DWORD dwGroupOfs;

	// Get ptr to first settings group
	if (!(dwGroupOfs = pDbContact->ofsFirstSettings))
		return NULL; // No settings exists in this contact

	// Loop over all settings groups
	while (dwGroupOfs && dwGroupOfs < dwFileSize) {
		pSettingsGroup = NULL;

		// Read and verify the struct
		if (!GetSettingsGroup(hDbFile, dwGroupOfs, &pSettingsGroup))
			return NULL; // Bad struct

		// Struct OK, now get the name
		if ((pszGroupName = GetName(hDbFile, pSettingsGroup->ofsModuleName))) {

			// Is it the right one?
			if (strcmp(pszGroupName, pszName) == 0) {
				#ifdef _LOGGING
					AddMessage( LPGEN("Found module: %s"), pszGroupName );
				#endif
				return pSettingsGroup;
			}
			#ifdef _LOGGING
			else AddMessage( LPGEN("Ignoring module: %s"), pszGroupName );
			#endif
		}
		else AddMessage( LPGEN("Warning: Found module with no name"));

		dwGroupOfs = pSettingsGroup->ofsNext;

		if (pSettingsGroup)
			free(pSettingsGroup);
	}

	// Search failed
	return NULL;
}

// pDbSettings must point to a complete DBContactSettings struct in memory
int GetSettingByName(struct DBContactSettings* pDbSettings, char* pszSettingName, DBVARIANT* dbv)
{
	char pszName[256];
	// We need at least one setting to start with
	char* pDbSetting = pDbSettings->blob;
	if ( !pDbSetting )
		return FALSE;

	// ** pDbSettings now points to the first setting in this module

	// Loop over all settings
	while (pDbSetting && *pDbSetting) {
		memcpy(pszName, pDbSetting+1, *pDbSetting);
		pszName[*pDbSetting] = 0;

		// Is this the right one?
		if (strcmp(pszSettingName, pszName) == 0) {
			return GetSettingValue(pDbSetting, dbv);
		}

		#ifdef _LOGGING
			AddMessage( LPGEN("Ignoring setting: %s"), pszName );
		#endif
		pDbSetting = GetNextSetting(pDbSetting);
	}

	// Search failed
	return FALSE;
}

// dwSettingpointer points to a valid DBSettings struct
int GetSettingValue(char* pBlob, DBVARIANT* dbv)
{
	#ifdef _LOGGING
	{
		char* pszName = calloc((*pBlob)+1, 1);
		memcpy(pszName, pBlob+1, *pBlob);
		AddMessage( LPGEN("Getting type %u value for setting: %s"), (BYTE)*(pBlob+(*pBlob)+1), pszName );
		free(pszName);
	}
	#endif

	// Skip name
	pBlob = pBlob + (*pBlob)+1;
	dbv->type = ( BYTE )*pBlob++;

	// Check what type it is
	switch( dbv->type ) {
	case DBVT_BYTE:
		dbv->bVal = *pBlob;
		return TRUE;

	case DBVT_WORD:
		dbv->wVal = *(WORD*)pBlob;
		return TRUE;

	case DBVT_DWORD:
		dbv->dVal = *(DWORD*)pBlob;
		return TRUE;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		dbv->cchVal = *(WORD*)pBlob;
		dbv->pszVal = calloc( dbv->cchVal+1, sizeof( char ));
		memcpy( dbv->pszVal, pBlob+2, dbv->cchVal );
		dbv->pszVal[ dbv->cchVal ] = 0;
		return TRUE;

	case DBVTF_VARIABLELENGTH:
	case DBVT_BLOB:
		dbv->cpbVal = *(WORD*)pBlob;
		dbv->pbVal  = calloc( dbv->cpbVal+1, sizeof( char ));
		memcpy( dbv->pbVal, pBlob+2, dbv->cpbVal );
		dbv->pbVal[ dbv->cpbVal ] = 0;
		return TRUE;

	case DBVT_DELETED:
		AddMessage( LPGEN("DEBUG: Deleted setting treated as 0-length setting"));

	default:
		dbv->type = DBVT_DELETED;
	}

	return FALSE;
}

void FreeVariant( DBVARIANT* dbv )
{
	switch( dbv->type ) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		if ( dbv->pszVal )
			free( dbv->pszVal );
		break;

	case DBVTF_VARIABLELENGTH:
	case DBVT_BLOB:
		if ( dbv->pbVal )
			free( dbv->pbVal );
		break;
	}

	dbv->type = 0;
}

void WriteVariant( HANDLE hContact, const char* module, const char* var, DBVARIANT* dbv )
{
	DBCONTACTWRITESETTING dbw;
	dbw.szModule = module;
	dbw.szSetting = var;
	dbw.value = *dbv;
	CallService( MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&dbw );
}

// Returns true if pDBEI has been filled in with nice values
// Don't forget to free those pointers!
BOOL GetEvent(HANDLE hDbFile, DWORD dwOffset, DBEVENTINFO* pDBEI)
{
	DWORD dwBytesRead;
	struct DBEvent pEvent;
	static char pBlob[65536];

	// Early reject
	if (dwOffset == 0 || dwOffset >= dwFileSize)
		return FALSE;

	// ** Read and verify the struct
	if (SetFilePointer(hDbFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;

	if (!ReadFile(hDbFile, &pEvent, offsetof(struct DBEvent, blob), &dwBytesRead, NULL) ||
		(dwBytesRead != offsetof(struct DBEvent, blob)))
		return FALSE;

	if (pEvent.signature != DBEVENT_SIGNATURE)
		return FALSE; // Event corrupted

	// ** Read the blob
	if ((!ReadFile(hDbFile, pBlob, pEvent.cbBlob, &dwBytesRead, NULL)) ||
		(dwBytesRead != pEvent.cbBlob))
	{
		return FALSE;
	}

	// ** Copy the static part to the event info struct
	pDBEI->timestamp = pEvent.timestamp;
	pDBEI->eventType = pEvent.eventType;
	pDBEI->cbSize = sizeof(DBEVENTINFO);
	pDBEI->cbBlob = pEvent.cbBlob;
	pDBEI->pBlob = pBlob;
	pDBEI->flags = (pEvent.flags & ~(DBEF_SENT+DBEF_READ)) +
		((pEvent.flags & DBEF_SENT) ? DBEF_SENT : DBEF_READ ); // Imported events are always marked READ

	if (!(pDBEI->szModule = GetName(hDbFile, pEvent.ofsModuleName))) {
		return FALSE;
	}

	return TRUE;
}

// Returns a pointer to a string with the name
// from a DBModuleName struct if given a file offset
// Returns NULL on failure
char* GetName(HANDLE hDbFile, DWORD dwOffset)
{
    static DWORD dwLastOffset = 0;
    static HANDLE hLastDbFile = NULL;
    static char szName[256] = {0};

	DWORD dwBytesRead;
	struct DBModuleName pModule;

	// Early reject
	if (dwOffset == 0 || dwOffset >= dwFileSize)
		return FALSE;

	// Quick lookup
	if (dwOffset == dwLastOffset && hDbFile == hLastDbFile)
		return szName;

	// ** Read and verify the name struct
	if (SetFilePointer(hDbFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return NULL;

	if ((!ReadFile(hDbFile, &pModule, offsetof(struct DBModuleName, name), &dwBytesRead, NULL)) ||
		(dwBytesRead != offsetof(struct DBModuleName, name)))
		return NULL;

	if (pModule.signature != DBMODULENAME_SIGNATURE) {
		AddMessage( LPGEN("Modulename corrupted"));
		return NULL; // ModuleName corrupted
	}

	// ** Name struct OK, now read name into string buffer
	if ((!ReadFile(hDbFile, szName, pModule.cbName, &dwBytesRead, NULL)) || (dwBytesRead != pModule.cbName)) {
		return NULL;
	}

	// terminate string
	szName[pModule.cbName] = 0;

	// update last offset
	dwLastOffset = dwOffset;
	hLastDbFile = hDbFile;

	return szName;
}

DWORD FindNextEvent(HANDLE hDbFile, DWORD dwOffset)
{
	DWORD dwBytesRead;
	struct DBEvent pEvent;

	// Early reject
	if (dwOffset == 0 || dwOffset >= dwFileSize)
		return FALSE;

	// ** Read and verify the struct
	if (SetFilePointer(hDbFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return FALSE;

	if ((!ReadFile(hDbFile, &pEvent, sizeof(struct DBEvent), &dwBytesRead, NULL)) ||
		(dwBytesRead != sizeof(struct DBEvent)))
		return FALSE;

	if ( pEvent.signature != DBEVENT_SIGNATURE || pEvent.ofsNext > dwFileSize )
		return FALSE; // Event corrupted

	return pEvent.ofsNext;
}

int ImportGroups(HANDLE hDbFile, struct DBHeader* pdbHeader)
{
	struct DBContactSettings* pDbSettings;
	struct DBContact DbContact;
	char* pSetting;
	DWORD dwOffset;
	int nGroups = 0;

	// Find owner data
	dwOffset = pdbHeader->ofsUser;
	if (!GetContact(hDbFile, dwOffset, &DbContact)) {
		AddMessage( LPGEN("No owner found."));
		return -1;
	}

	// Find the module with the groups, and import them all
	if ( pDbSettings = GetSettingsGroupByModuleName( hDbFile, &DbContact, "CListGroups" )) {
		pSetting = pDbSettings->blob;
		while ( pSetting && *pSetting ) {
			DBVARIANT dbv;
			if ( GetSettingValue( pSetting, &dbv )) {
				if ( CreateGroup( dbv.type, dbv.pszVal+1, NULL ))
					nGroups++;
				FreeVariant( &dbv );
			}
			pSetting = GetNextSetting(pSetting);
		}
		free(pDbSettings);
	}

	return nGroups;
}

HANDLE ImportContact(HANDLE hDbFile, struct DBContact Contact)
{
	HANDLE hContact;
	DBVARIANT group, nick, dbv;
	char* pszProtoName;
	char* pszUniqueSetting;
	char* pszUserName;
	char id[ 40 ];

	// Check what protocol this contact belongs to
	if ( !GetSetting( hDbFile, &Contact, "Protocol", "p", &dbv )) {
		AddMessage( LPGEN("Skipping contact with no protocol"));
		return INVALID_HANDLE_VALUE;
	}

	pszProtoName = NEWSTR_ALLOCA( dbv.pszVal );
	FreeVariant( &dbv );

	if ( !IsProtocolLoaded( pszProtoName )) {
		AddMessage( LPGEN("Skipping contact, %s not installed."), pszProtoName );
		return INVALID_HANDLE_VALUE;
	}

	// Skip protocols with no unique id setting (some non IM protocols return NULL)
	pszUniqueSetting = (char*)CallProtoService(pszProtoName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	if ( !pszUniqueSetting || (INT_PTR)pszUniqueSetting == CALLSERVICE_NOTFOUND ) {
		AddMessage( LPGEN("Skipping non-IM contact (%s)"), pszProtoName );
		return INVALID_HANDLE_VALUE;
	}

	if ( !GetSetting(hDbFile, &Contact, pszProtoName, pszUniqueSetting, &dbv )) {
		AddMessage( LPGEN("Skipping %s contact, ID not found"), pszProtoName );
		return INVALID_HANDLE_VALUE;
	}

	// Does the contact already exist?
	if ( dbv.type == DBVT_DWORD ) {
		pszUserName = _ltoa( dbv.dVal, id, 10 );
		hContact = HContactFromNumericID( pszProtoName, pszUniqueSetting, dbv.dVal );
	}
	else {
		pszUserName = NEWSTR_ALLOCA( dbv.pszVal );
		hContact = HContactFromID( pszProtoName, pszUniqueSetting, dbv.pszVal );
	}

	if ( hContact != INVALID_HANDLE_VALUE ) {
		AddMessage( LPGEN("Skipping duplicate %s contact %s"), pszProtoName, pszUserName );
		FreeVariant( &dbv );
		return INVALID_HANDLE_VALUE;
	}
	// No, add contact and copy some important settings
	GetSetting(hDbFile, &Contact, "CList", "Group", &group);

	if ( !GetSetting( hDbFile, &Contact, "CList", "MyHandle", &nick ))
		GetSetting(hDbFile, &Contact, pszProtoName, "Nick", &nick );

	hContact = AddContact( hdlgProgress, pszProtoName, pszUniqueSetting, &dbv, &nick, &group );

	if ( hContact != INVALID_HANDLE_VALUE) {

		// Hidden?
		if ( GetSetting( hDbFile, &Contact, "CList", "Hidden", &dbv )) {
			WriteVariant( hContact, "CList", "Hidden", &dbv );
			FreeVariant( &dbv );
		}
		// Ignore settings
		if ( GetSetting( hDbFile, &Contact, "Ignore", "Mask1", &dbv )) {
			WriteVariant( hContact, "Ignore", "Mask1", &dbv );
			FreeVariant( &dbv );
		}

		// Apparent mode
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "ApparentMode", &dbv )) {
			WriteVariant( hContact, pszProtoName, "ApparentMode", &dbv );
			FreeVariant( &dbv );
		}

		// Nick
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "Nick", &dbv )) {
			WriteVariant( hContact, pszProtoName, "Nick", &dbv );
			FreeVariant( &dbv );
		}

		// Myhandle
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "MyHandle", &dbv )) {
			WriteVariant( hContact, pszProtoName, "MyHandle", &dbv );
			FreeVariant( &dbv );
		}

		// First name
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "FirstName", &dbv )) {
			WriteVariant( hContact, pszProtoName, "FirstName", &dbv );
			FreeVariant( &dbv );
		}

		// Last name
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "LastName", &dbv )) {
			WriteVariant( hContact, pszProtoName, "LastName", &dbv );
			FreeVariant( &dbv );
		}

		// About
		if ( GetSetting( hDbFile, &Contact, pszProtoName, "About", &dbv )) {
			WriteVariant( hContact, pszProtoName, "About", &dbv );
			FreeVariant( &dbv );
		}
	}
	else AddMessage( LPGEN("Unknown error while adding %s contact %s"), pszProtoName, pszUserName );

	return hContact;
}

// This function should always be called after contact import. That is
// why there are no messages for errors related to contacts. Those
// would only be a repetition of the messages printed during contact
// import.

static void ImportHistory(HANDLE hDbFile, struct DBContact Contact, PROTOCOLDESCRIPTOR **protocol, int protoCount)
{
	HANDLE hContact = INVALID_HANDLE_VALUE;
	DWORD dwOffset;
	MSG msg;
	DBVARIANT proto;
	int i, skipAll, bIsVoidContact;

	// Is it contats history import?
	if ( protoCount == 0 ) {

		// Check what protocol this contact belongs to
		if ( GetSetting( hDbFile, &Contact, "Protocol", "p", &proto )) {

			// Protocol installed?
			if ( IsProtocolLoaded( proto.pszVal )) {
				// Is contact in database?
				char* pszUniqueSetting = (char*)CallProtoService( proto.pszVal, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

				// Skip protocols with no unique id setting (some non IM protocols return NULL)
				if ( pszUniqueSetting && ( INT_PTR )pszUniqueSetting != CALLSERVICE_NOTFOUND ) {
					DBVARIANT dbv;
					if ( GetSetting( hDbFile, &Contact, proto.pszVal, pszUniqueSetting, &dbv )) {
						if ( dbv.type == DBVT_DWORD )
							hContact = HContactFromNumericID( proto.pszVal, pszUniqueSetting, dbv.dVal );
						else
							hContact = HContactFromID( proto.pszVal, pszUniqueSetting, dbv.pszVal );
						FreeVariant( &dbv );
			}	}	}
			FreeVariant( &proto );
		}
	}
	else hContact = NULL; //system history import

	// OK to import this chain?
	if (hContact == INVALID_HANDLE_VALUE) {
		nSkippedContacts++;
		return;
	}

	i = skipAll = 0;
	bIsVoidContact = CallService( MS_DB_EVENT_GETCOUNT, ( WPARAM )hContact, 0 ) == 0;

	// Get the start of the event chain
	dwOffset = Contact.ofsFirstEvent;
	while (dwOffset) {
		int skip = 0;

		// Copy the event and import it
		DBEVENTINFO dbei = { 0 };
		if (GetEvent(hDbFile, dwOffset, &dbei)) {
			// check protocols during system history import
			if (hContact == NULL) {
				int i;
				skipAll = 1;

				for(i = 0; i < protoCount; i++)
					if (!strcmp(dbei.szModule, protocol[i]->szName)) { //&& protocol[i]->type == PROTOTYPE_PROTOCOL)
						skipAll = 0;
						break;
					}

				skip = skipAll;
			}

			// custom filtering
			if (!skip && nImportOption == IMPORT_CUSTOM) {
				BOOL sent = (dbei.flags&DBEF_SENT);

				if (dbei.timestamp < (DWORD)dwSinceDate)
					skip = 1;

				if (!skip) {
					if (hContact) {
						skip = 1;
						switch(dbei.eventType) {
						case EVENTTYPE_MESSAGE:
							if ((sent?IOPT_MSGSENT:IOPT_MSGRECV)&nCustomOptions)
								skip = 0;
							break;
						case EVENTTYPE_FILE:
							if ((sent?IOPT_FILESENT:IOPT_FILERECV)&nCustomOptions)
								skip = 0;
							break;
						case EVENTTYPE_URL:
							if ((sent?IOPT_URLSENT:IOPT_URLRECV)&nCustomOptions)
								skip = 0;
							break;
						default:
							if ((sent?IOPT_OTHERSENT:IOPT_OTHERRECV)&nCustomOptions)
								skip = 0;
							break;
						}
					}
					else if ( !( nCustomOptions & IOPT_SYSTEM ))
						skip = 1;
				}

				if (skip)
					nSkippedEvents++;
			}

			if (!skip) {
				// Check for duplicate entries
				if ( !IsDuplicateEvent( hContact, dbei )) {
					// Add dbevent
					if (!bIsVoidContact)
						dbei.flags &= ~DBEF_FIRST;
					if (CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei))
						nMessagesCount++;
					else
						AddMessage( LPGEN("Failed to add message"));
				}
				else
					nDupes++;
			}
		}

		if ( !( i%10 )) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}	}

		// skip this chain if needed
		if ( skipAll )
			break;

		// Get next event
		dwOffset = FindNextEvent(hDbFile, dwOffset);
		i++;
	}
}

static void MirandaImport(HWND hdlg)
{
	int nDBVersion;
	int i;
	int nNumberOfContacts = 0;
	MSG msg;
	DWORD dwTimer;
	DWORD dwOffset;
	HANDLE hFile;
	char* pszModuleName = NULL;
	struct DBHeader* pdbHeader = NULL;
	struct DBContact Contact;

	// Just to keep the macros happy
	hdlgProgress = hdlg;

	// Reset statistics
	nSkippedEvents = 0;
	nDupes = 0;
	nContactsCount = 0;
	nMessagesCount = 0;
	nGroupsCount = 0;
	nSkippedContacts = 0;
	SetProgress(0);

	// Open database
	hFile = CreateFile(importFile,
		GENERIC_READ,                 // open for reading
		0,                            // do not share
		NULL,                         // no security
		OPEN_EXISTING,                // existing file only
		FILE_ATTRIBUTE_NORMAL,        // normal file
		NULL);                        // no attr. template

	// Read error
	if (hFile == INVALID_HANDLE_VALUE) {
		AddMessage( LPGEN("Could not open file."));
		SetProgress(100);
		return;
	}

	// Check filesize
	dwFileSize = GetFileSize(hFile, NULL) ;
	if ((dwFileSize == INVALID_FILE_SIZE) || (dwFileSize < sizeof(struct DBHeader))) {
		AddMessage( LPGEN("This is not a valid Miranda IM database."));
		SetProgress(100);
		CloseHandle(hFile);
		return;
	}

	// Check header and database version
	nDBVersion = CheckFileFormat(hFile);
	if (nDBVersion == DB_INVALID) {
		AddMessage( LPGEN("This is not a valid Miranda IM database."));
		SetProgress(100);
		CloseHandle(hFile);
		return;
	}

	// Load database header
	if (!(pdbHeader = GetHeader(hFile))) {
		AddMessage( LPGEN("Read failure."));
		SetProgress(100);
		CloseHandle(hFile);
		return;
	}

	// Get number of contacts
	nNumberOfContacts = pdbHeader->contactCount;
	AddMessage( LPGEN("Number of contacts in database: %d"), nNumberOfContacts );
	AddMessage( "" );

	// Configure database for fast writing
	CallService(MS_DB_SETSAFETYMODE, FALSE, 0);

	// Start benchmark timer
	dwTimer = time(NULL);

	// Import Groups
	if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_GROUPS)) {
		AddMessage( LPGEN("Importing groups."));
		nGroupsCount = ImportGroups(hFile, pdbHeader);
		if (nGroupsCount == -1)
			AddMessage( LPGEN("Group import failed."));

		AddMessage( "" );
	}
	// End of Import Groups

	// Import Contacts
	if (nImportOption != IMPORT_CUSTOM || (nCustomOptions & IOPT_CONTACTS)) {
		AddMessage( LPGEN("Importing contacts."));
		i = 1;
		dwOffset = FindFirstContact(pdbHeader);
		while (dwOffset && (dwOffset < dwFileSize)) {
			if (!GetContact(hFile, dwOffset, &Contact)) {
				AddMessage( LPGEN("ERROR: Chain broken, no valid contact at %d"), dwOffset );
				SetProgress(100);
				break;
			}

			if (ImportContact(hFile, Contact) != INVALID_HANDLE_VALUE)
				nContactsCount++;

			// Update progress bar
			SetProgress(100 * i / nNumberOfContacts);
			i++;

			// Process queued messages
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// Get next contact in chain
			dwOffset = FindNextContact(&Contact);
		}
	}
	else AddMessage( LPGEN("Skipping new contacts import."));
	AddMessage( "" );
	// End of Import Contacts

	// Import history
	if (nImportOption != IMPORT_CONTACTS) {
		// Import NULL contact message chain
		if (nImportOption == IMPORT_ALL || (nCustomOptions & IOPT_SYSTEM)) {
			AddMessage( LPGEN("Importing system history."));
			dwOffset = FindOwnerContact(pdbHeader);
			if (!GetContact(hFile, dwOffset, &Contact)) {
				AddMessage( LPGEN("ERROR: Chain broken, no valid contact at %d"), dwOffset );
				SetProgress(100);
			}
			else {
				PROTOCOLDESCRIPTOR **protocol;
				int protoCount;

				CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&protoCount, (LPARAM)&protocol);

				if (protoCount > 0)
					ImportHistory(hFile, Contact, protocol, protoCount);
			}
		}
		else AddMessage( LPGEN("Skipping system history import."));

		AddMessage( "" );

		// Import other contact messages
		if (nImportOption == IMPORT_ALL || (nCustomOptions & 2046)) { // 2 - 1024 types
			AddMessage( LPGEN("Importing history."));
			dwOffset = FindFirstContact(pdbHeader);
			for(i=1; i <= nNumberOfContacts; i++) {
				if (!GetContact(hFile, dwOffset, &Contact)) {
					AddMessage( LPGEN("ERROR: Chain broken, no valid contact at %d"), dwOffset );
					SetProgress(100);
					break;
				}

				ImportHistory(hFile, Contact, NULL, 0);

				SetProgress(100 * i / nNumberOfContacts);
				dwOffset = FindNextContact(&Contact);
			}
		}
		else AddMessage( LPGEN("Skipping history import."));

		AddMessage( "" );
	}
	// End of Import History

	// Restore database writing mode
	CallService(MS_DB_SETSAFETYMODE, TRUE, 0);

	// Clean up before exit
	CloseHandle(hFile);
	free(pdbHeader);

	// Stop timer
	dwTimer = time(NULL) - dwTimer;

	// Print statistics
	AddMessage( LPGEN("Import completed in %d seconds."), dwTimer );
	SetProgress(100);
	AddMessage((nImportOption == IMPORT_CONTACTS) ?
		LPGEN("Added %d contacts and %d groups.") : LPGEN("Added %d contacts, %d groups and %d events."),
		nContactsCount, nGroupsCount, nMessagesCount);

	if ( nImportOption != IMPORT_CONTACTS ) {
		if (nSkippedContacts)
			AddMessage( LPGEN("Skipped %d contacts."), nSkippedContacts );

		AddMessage((nImportOption == IMPORT_CUSTOM) ?
			LPGEN("Skipped %d duplicates and %d filtered events.") : LPGEN("Skipped %d duplicates."),
			nDupes, nSkippedEvents);
}	}
