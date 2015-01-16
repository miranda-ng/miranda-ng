/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"
#include "file.h"

HANDLE hDlgSucceeded, hDlgCanceled;

TCHAR* PFTS_StringToTchar(int flags, const PROTOCHAR* s);
int PFTS_CompareWithTchar(PROTOFILETRANSFERSTATUS* ft, const PROTOCHAR* s, TCHAR *r);

static HGENMENU hSRFileMenuItem;

TCHAR* GetContactID(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1) {
		if (TCHAR *theValue = db_get_tsa(hContact, szProto, "ChatRoomID"))
			return theValue;
	}
	else {
		CONTACTINFO ci = { sizeof(ci) };
		ci.hContact = hContact;
		ci.szProto = szProto;
		ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
		if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci)) {
			switch (ci.type) {
			case CNFT_ASCIIZ:
				return (TCHAR*)ci.pszVal;
			case CNFT_DWORD:
				return _itot(ci.dVal, (TCHAR*)mir_alloc(sizeof(TCHAR)*32), 10);
			}
		}
	}
	return NULL;
}

static INT_PTR SendFileCommand(WPARAM hContact, LPARAM)
{
	struct FileSendData fsd;
	fsd.hContact = hContact;
	fsd.ppFiles = NULL;
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
}

static INT_PTR SendSpecificFiles(WPARAM hContact, LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact = hContact;

	char** ppFiles = (char**)lParam;
	int count = 0;
	while (ppFiles[count] != NULL)
		count++;

	fsd.ppFiles = (const TCHAR**)alloca((count + 1) * sizeof(void*));
	for (int i = 0; i < count; i++)
		fsd.ppFiles[i] = mir_a2t(ppFiles[i]);
	fsd.ppFiles[count] = NULL;

	HWND hWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
	for (int j = 0; j < count; j++)
		mir_free((void*)fsd.ppFiles[j]);
	return (INT_PTR)hWnd;
}

static INT_PTR SendSpecificFilesT(WPARAM hContact, LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact = hContact;
	fsd.ppFiles = (const TCHAR**)lParam;
	return (INT_PTR)CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
}

static INT_PTR GetReceivedFilesFolder(WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[MAX_PATH];
	GetContactReceivedFilesDir(wParam, buf, MAX_PATH, TRUE);
	char* dir = mir_t2a(buf);
	mir_strncpy((char*)lParam, dir, MAX_PATH);
	mir_free(dir);
	return 0;
}

static INT_PTR RecvFileCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILERECV), NULL, DlgProcRecvFile, lParam);
	return 0;
}

void PushFileEvent(MCONTACT hContact, MEVENT hdbe, LPARAM lParam)
{
	CLISTEVENT cle = { 0 };
	cle.cbSize = sizeof(cle);
	cle.hContact = hContact;
	cle.hDbEvent = hdbe;
	cle.lParam = lParam;
	if (db_get_b(NULL, "SRFile", "AutoAccept", 0) && !db_get_b(hContact, "CList", "NotOnList", 0)) {
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_FILERECV), NULL, DlgProcRecvFile, (LPARAM)&cle);
	}
	else {
		SkinPlaySound("RecvFile");

		TCHAR szTooltip[256];
		mir_sntprintf(szTooltip, SIZEOF(szTooltip), TranslateT("File from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
		cle.ptszTooltip = szTooltip;

		cle.flags |= CLEF_TCHAR;
		cle.hIcon = LoadSkinIcon(SKINICON_EVENT_FILE);
		cle.pszService = "SRFile/RecvFile";
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	}
}

static int FileEventAdded(WPARAM wParam, LPARAM lParam)
{
	DWORD dwSignature;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD);
	dbei.pBlob = (PBYTE)&dwSignature;
	db_event_get(lParam, &dbei);
	if (dbei.flags & (DBEF_SENT | DBEF_READ) || dbei.eventType != EVENTTYPE_FILE || dwSignature == 0)
		return 0;

	PushFileEvent(wParam, lParam, 0);
	return 0;
}

int SRFile_GetRegValue(HKEY hKeyBase, const TCHAR *szSubKey, const TCHAR *szValue, TCHAR *szOutput, int cbOutput)
{
	HKEY hKey;
	DWORD cbOut = cbOutput;

	if (RegOpenKeyEx(hKeyBase, szSubKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;

	if (RegQueryValueEx(hKey, szValue, NULL, NULL, (PBYTE)szOutput, &cbOut) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return 0;
	}

	RegCloseKey(hKey);
	return 1;
}

void GetSensiblyFormattedSize(__int64 size, TCHAR *szOut, int cchOut, int unitsOverride, int appendUnits, int *unitsUsed)
{
	if (!unitsOverride) {
		if (size < 1000) unitsOverride = UNITS_BYTES;
		else if (size < 100 * 1024) unitsOverride = UNITS_KBPOINT1;
		else if (size < 1024 * 1024) unitsOverride = UNITS_KBPOINT0;
		else if (size < 1024 * 1024 * 1024) unitsOverride = UNITS_MBPOINT2;
		else unitsOverride = UNITS_GBPOINT3;
	}

	if (unitsUsed)
		*unitsUsed = unitsOverride;
	
	switch (unitsOverride) {
		case UNITS_BYTES: mir_sntprintf(szOut, cchOut, _T("%u%s%s"), (int)size, appendUnits ? _T(" ") : _T(""), appendUnits ? TranslateT("bytes") : _T("")); break;
		case UNITS_KBPOINT1: mir_sntprintf(szOut, cchOut, _T("%.1lf%s"), size / 1024.0, appendUnits ? _T(" KB") : _T("")); break;
		case UNITS_KBPOINT0: mir_sntprintf(szOut, cchOut, _T("%u%s"), (int)(size / 1024), appendUnits ? _T(" KB") : _T("")); break;
		case UNITS_GBPOINT3: mir_sntprintf(szOut, cchOut, _T("%.3f%s"), (size >> 20) / 1024.0, appendUnits ? _T(" GB") : _T("")); break;
		default: mir_sntprintf(szOut, cchOut, _T("%.2lf%s"), size / 1048576.0, appendUnits ? _T(" MB") : _T("")); break;
	}
}

// Tripple redirection sucks but is needed to nullify the array pointer
void FreeFilesMatrix(TCHAR ***files)
{
	if (*files == NULL)
		return;

	// Free each filename in the pointer array
	TCHAR **pFile = *files;
	while (*pFile != NULL) {
		mir_free(*pFile);
		*pFile = NULL;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = NULL;
}

void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts)
{
	mir_free(fts->tszCurrentFile);
	if (fts->ptszFiles) {
		for (int i = 0; i < fts->totalFiles; i++) mir_free(fts->ptszFiles[i]);
		mir_free(fts->ptszFiles);
	}
	mir_free(fts->tszWorkingDir);
}

void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	*dest = *src;
	if (src->tszCurrentFile) dest->tszCurrentFile = PFTS_StringToTchar(src->flags, src->tszCurrentFile);
	if (src->ptszFiles) {
		dest->ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)*src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++)
			dest->ptszFiles[i] = PFTS_StringToTchar(src->flags, src->ptszFiles[i]);
	}
	if (src->tszWorkingDir) dest->tszWorkingDir = PFTS_StringToTchar(src->flags, src->tszWorkingDir);
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_TCHAR;
}

void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	dest->hContact = src->hContact;
	dest->flags = src->flags;
	if (dest->totalFiles != src->totalFiles) {
		for (int i = 0; i < dest->totalFiles; i++) mir_free(dest->ptszFiles[i]);
		mir_free(dest->ptszFiles);
		dest->ptszFiles = NULL;
		dest->totalFiles = src->totalFiles;
	}
	if (src->ptszFiles) {
		if (!dest->ptszFiles)
			dest->ptszFiles = (TCHAR**)mir_calloc(sizeof(TCHAR*)*src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++)
			if (!dest->ptszFiles[i] || !src->ptszFiles[i] || PFTS_CompareWithTchar(src, src->ptszFiles[i], dest->ptszFiles[i])) {
				mir_free(dest->ptszFiles[i]);
				if (src->ptszFiles[i])
					dest->ptszFiles[i] = PFTS_StringToTchar(src->flags, src->ptszFiles[i]);
				else
					dest->ptszFiles[i] = NULL;
			}
	}
	else if (dest->ptszFiles) {
		for (int i = 0; i < dest->totalFiles; i++)
			mir_free(dest->ptszFiles[i]);
		mir_free(dest->ptszFiles);
		dest->ptszFiles = NULL;
	}

	dest->currentFileNumber = src->currentFileNumber;
	dest->totalBytes = src->totalBytes;
	dest->totalProgress = src->totalProgress;
	if (src->tszWorkingDir && (!dest->tszWorkingDir || PFTS_CompareWithTchar(src, src->tszWorkingDir, dest->tszWorkingDir))) {
		mir_free(dest->tszWorkingDir);
		if (src->tszWorkingDir)
			dest->tszWorkingDir = PFTS_StringToTchar(src->flags, src->tszWorkingDir);
		else
			dest->tszWorkingDir = NULL;
	}

	if (!dest->tszCurrentFile || !src->tszCurrentFile || PFTS_CompareWithTchar(src, src->tszCurrentFile, dest->tszCurrentFile)) {
		mir_free(dest->tszCurrentFile);
		if (src->tszCurrentFile)
			dest->tszCurrentFile = PFTS_StringToTchar(src->flags, src->tszCurrentFile);
		else
			dest->tszCurrentFile = NULL;
	}
	dest->currentFileSize = src->currentFileSize;
	dest->currentFileProgress = src->currentFileProgress;
	dest->currentFileTime = src->currentFileTime;
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_TCHAR;
}

static void RemoveUnreadFileEvents(void)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		MEVENT hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && dbei.eventType == EVENTTYPE_FILE)
				db_event_markRead(hContact, hDbEvent);
			hDbEvent = db_event_next(hContact, hDbEvent);
		}
	}
}

static int SRFilePreBuildMenu(WPARAM wParam, LPARAM)
{
	bool bEnabled = false;
	char *szProto = GetContactProto(wParam);
	if (szProto != NULL) {
		bool isChat = db_get_b(wParam, szProto, "ChatRoom", false) != 0;
		if (CallProtoService(szProto, PS_GETCAPS, isChat ? PFLAGNUM_4 : PFLAGNUM_1, 0) & (isChat ? PF4_GROUPCHATFILES : PF1_FILESEND)) {
			if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_OFFLINEFILES)
				bEnabled = true;
			else if (db_get_w(wParam, szProto, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
				bEnabled = true;
		}
	}

	Menu_ShowItem(hSRFileMenuItem, bEnabled);
	return 0;
}

static int SRFileModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -2000020000;
	mi.icolibItem = GetSkinIconHandle(SKINICON_EVENT_FILE);
	mi.pszName = LPGEN("&File");
	mi.pszService = MS_FILE_SENDFILE;
	hSRFileMenuItem = Menu_AddContactMenuItem(&mi);

	RemoveUnreadFileEvents();
	return 0;
}

INT_PTR FtMgrShowCommand(WPARAM, LPARAM)
{
	FtMgr_Show(true, true);
	return 0;
}

INT_PTR openContRecDir(WPARAM hContact, LPARAM)
{
	TCHAR szContRecDir[MAX_PATH];
	GetContactReceivedFilesDir(hContact, szContRecDir, SIZEOF(szContRecDir), TRUE);
	ShellExecute(0, _T("open"), szContRecDir, 0, 0, SW_SHOW);
	return 0;
}

INT_PTR openRecDir(WPARAM, LPARAM)
{
	TCHAR szContRecDir[MAX_PATH];
	GetReceivedFilesDir(szContRecDir, SIZEOF(szContRecDir));
	ShellExecute(0, _T("open"), szContRecDir, 0, 0, SW_SHOW);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void sttRecvCreateBlob(DBEVENTINFO &dbei, int fileCount, char **pszFiles, char *szDescr)
{
	dbei.cbBlob = sizeof(DWORD);

	for (int i = 0; i < fileCount; i++)
		dbei.cbBlob += (int)mir_strlen(pszFiles[i]) + 1;

	dbei.cbBlob += (int)mir_strlen(szDescr) + 1;

	if ((dbei.pBlob = (BYTE*)mir_alloc(dbei.cbBlob)) == 0)
		return;

	*(DWORD*)dbei.pBlob = 0;
	BYTE* p = dbei.pBlob + sizeof(DWORD);
	for (int i = 0; i < fileCount; i++) {
		strcpy((char*)p, pszFiles[i]);
		p += mir_strlen(pszFiles[i]) + 1;
	}
	strcpy((char*)p, (szDescr == NULL) ? "" : szDescr);
}

static INT_PTR Proto_RecvFileT(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVFILET* pre = (PROTORECVFILET*)ccs->lParam;
	if (pre->fileCount == 0)
		return 0;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(ccs->hContact);
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ : 0;
	dbei.eventType = EVENTTYPE_FILE;

	char **pszFiles = (char**)alloca(pre->fileCount * sizeof(char*));
	for (int i = 0; i < pre->fileCount; i++)
		pszFiles[i] = Utf8EncodeT(pre->ptszFiles[i]);

	char *szDescr = Utf8EncodeT(pre->tszDescription);
	dbei.flags |= DBEF_UTF;
	sttRecvCreateBlob(dbei, pre->fileCount, pszFiles, szDescr);

	for (int i = 0; i < pre->fileCount; i++)
		mir_free(pszFiles[i]);
	mir_free(szDescr);

	MEVENT hdbe = db_event_add(ccs->hContact, &dbei);
	PushFileEvent(ccs->hContact, hdbe, pre->lParam);
	mir_free(dbei.pBlob);
	return 0;
}

int LoadSendRecvFileModule(void)
{
	CreateServiceFunction("FtMgr/Show", FtMgrShowCommand);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = GetSkinIconHandle(SKINICON_EVENT_FILE);
	mi.position = 1900000000;
	mi.pszName = LPGEN("File &transfers...");
	mi.pszService = "FtMgr/Show"; //MS_PROTO_SHOWFTMGR;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, SRFileModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, FileEventAdded);
	HookEvent(ME_OPT_INITIALISE, FileOptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRFilePreBuildMenu);

	hDlgSucceeded = CreateHookableEvent(ME_FILEDLG_SUCCEEDED);
	hDlgCanceled = CreateHookableEvent(ME_FILEDLG_CANCELED);

	CreateServiceFunction(MS_PROTO_RECVFILET, Proto_RecvFileT);

	CreateServiceFunction(MS_FILE_SENDFILE, SendFileCommand);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILES, SendSpecificFiles);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILEST, SendSpecificFilesT);
	CreateServiceFunction(MS_FILE_GETRECEIVEDFILESFOLDER, GetReceivedFilesFolder);
	CreateServiceFunction("SRFile/RecvFile", RecvFileCommand);

	CreateServiceFunction("SRFile/OpenContRecDir", openContRecDir);
	CreateServiceFunction("SRFile/OpenRecDir", openRecDir);

	SkinAddNewSoundEx("RecvFile", LPGEN("File"), LPGEN("Incoming"));
	SkinAddNewSoundEx("FileDone", LPGEN("File"), LPGEN("Complete"));
	SkinAddNewSoundEx("FileFailed", LPGEN("File"), LPGEN("Error"));
	SkinAddNewSoundEx("FileDenied", LPGEN("File"), LPGEN("Denied"));
	return 0;
}
