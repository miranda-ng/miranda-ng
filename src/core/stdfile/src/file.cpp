/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#include "file.h"

HANDLE hDlgSucceeded, hDlgCanceled;

wchar_t* PFTS_StringToTchar(int flags, const wchar_t* s);
int PFTS_CompareWithTchar(PROTOFILETRANSFERSTATUS* ft, const wchar_t* s, wchar_t *r);

static HGENMENU hSRFileMenuItem;

wchar_t* GetContactID(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (Contact::IsGroupChat(hContact, szProto))
		if (wchar_t *theValue = db_get_wsa(hContact, szProto, "ChatRoomID"))
			return theValue;

	return Contact::GetInfo(CNF_UNIQUEID, hContact, szProto);
}

static INT_PTR SendFileCommand(WPARAM hContact, LPARAM)
{
	FileSendData fsd;
	fsd.hContact = hContact;
	fsd.ppFiles = nullptr;
	return (INT_PTR)CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
}

static INT_PTR SendSpecificFiles(WPARAM hContact, LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact = hContact;

	char** ppFiles = (char**)lParam;
	int count = 0;
	while (ppFiles[count] != nullptr)
		count++;

	fsd.ppFiles = (const wchar_t**)alloca((count + 1) * sizeof(void*));
	for (int i = 0; i < count; i++)
		fsd.ppFiles[i] = mir_a2u(ppFiles[i]);
	fsd.ppFiles[count] = nullptr;

	HWND hWnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
	for (int j = 0; j < count; j++)
		mir_free((void*)fsd.ppFiles[j]);
	return (INT_PTR)hWnd;
}

static INT_PTR SendSpecificFilesT(WPARAM hContact, LPARAM lParam)
{
	FileSendData fsd;
	fsd.hContact = hContact;
	fsd.ppFiles = (const wchar_t**)lParam;
	return (INT_PTR)CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILESEND), NULL, DlgProcSendFile, (LPARAM)&fsd);
}

static INT_PTR GetReceivedFilesFolder(WPARAM wParam, LPARAM lParam)
{
	wchar_t buf[MAX_PATH];
	GetContactReceivedFilesDir(wParam, buf, MAX_PATH, TRUE);
	char* dir = mir_u2a(buf);
	mir_strncpy((char*)lParam, dir, MAX_PATH);
	mir_free(dir);
	return 0;
}

static INT_PTR RecvFileCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILERECV), NULL, DlgProcRecvFile, lParam);
	return 0;
}

void PushFileEvent(MCONTACT hContact, MEVENT hdbe, LPARAM lParam)
{
	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = hdbe;
	cle.lParam = lParam;
	if (g_plugin.bAutoAccept && Contact::OnList(hContact)) {
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILERECV), NULL, DlgProcRecvFile, (LPARAM)&cle);
	}
	else {
		Skin_PlaySound("RecvFile");

		wchar_t szTooltip[256];
		mir_snwprintf(szTooltip, TranslateT("File from %s"), Clist_GetContactDisplayName(hContact));
		cle.szTooltip.w = szTooltip;

		cle.flags |= CLEF_UNICODE;
		cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		cle.pszService = "SRFile/RecvFile";
		g_clistApi.pfnAddEvent(&cle);
	}
}

static int FileEventAdded(WPARAM wParam, LPARAM lParam)
{
	uint32_t dwSignature;

	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(uint32_t);
	dbei.pBlob = (uint8_t*)&dwSignature;
	db_event_get(lParam, &dbei);
	if (dbei.flags & (DBEF_SENT | DBEF_READ) || dbei.eventType != EVENTTYPE_FILE || dwSignature == 0)
		return 0;

	PushFileEvent(wParam, lParam, 0);
	return 0;
}

int SRFile_GetRegValue(HKEY hKeyBase, const wchar_t *szSubKey, const wchar_t *szValue, wchar_t *szOutput, int cbOutput)
{
	HKEY hKey;
	DWORD cbOut = cbOutput;

	if (RegOpenKeyEx(hKeyBase, szSubKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return 0;

	if (RegQueryValueEx(hKey, szValue, nullptr, nullptr, (uint8_t*)szOutput, &cbOut) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return 0;
	}

	RegCloseKey(hKey);
	return 1;
}

void GetSensiblyFormattedSize(__int64 size, wchar_t *szOut, int cchOut, int unitsOverride, int appendUnits, int *unitsUsed)
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
		case UNITS_BYTES: mir_snwprintf(szOut, cchOut, L"%u%s%s", (int)size, appendUnits ? L" " : L"", appendUnits ? TranslateT("bytes") : L""); break;
		case UNITS_KBPOINT1: mir_snwprintf(szOut, cchOut, L"%.1lf%s", size / 1024.0, appendUnits ? L" KB" : L""); break;
		case UNITS_KBPOINT0: mir_snwprintf(szOut, cchOut, L"%u%s", (int)(size / 1024), appendUnits ? L" KB" : L""); break;
		case UNITS_GBPOINT3: mir_snwprintf(szOut, cchOut, L"%.3f%s", (size >> 20) / 1024.0, appendUnits ? L" GB" : L""); break;
		default: mir_snwprintf(szOut, cchOut, L"%.2lf%s", size / 1048576.0, appendUnits ? L" MB" : L""); break;
	}
}

// Tripple redirection sucks but is needed to nullify the array pointer
void FreeFilesMatrix(wchar_t ***files)
{
	if (*files == nullptr)
		return;

	// Free each filename in the pointer array
	wchar_t **pFile = *files;
	while (*pFile != nullptr) {
		mir_free(*pFile);
		*pFile = nullptr;
		pFile++;
	}

	// Free the array itself
	mir_free(*files);
	*files = nullptr;
}

void FreeProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *fts)
{
	mir_free(fts->szCurrentFile.w);
	if (fts->pszFiles.w) {
		for (int i = 0; i < fts->totalFiles; i++) mir_free(fts->pszFiles.w[i]);
		mir_free(fts->pszFiles.w);
	}
	mir_free(fts->szWorkingDir.w);
}

void CopyProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	*dest = *src;
	if (src->szCurrentFile.w) dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile.w);
	if (src->pszFiles.w) {
		dest->pszFiles.w = (wchar_t**)mir_alloc(sizeof(wchar_t*)*src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++)
			dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, src->pszFiles.w[i]);
	}
	if (src->szWorkingDir.w) dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir.w);
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_UNICODE;
}

void UpdateProtoFileTransferStatus(PROTOFILETRANSFERSTATUS *dest, PROTOFILETRANSFERSTATUS *src)
{
	dest->hContact = src->hContact;
	dest->flags = src->flags;
	if (dest->totalFiles != src->totalFiles) {
		for (int i = 0; i < dest->totalFiles; i++) mir_free(dest->pszFiles.w[i]);
		mir_free(dest->pszFiles.w);
		dest->pszFiles.w = nullptr;
		dest->totalFiles = src->totalFiles;
	}
	if (src->pszFiles.w) {
		if (!dest->pszFiles.w)
			dest->pszFiles.w = (wchar_t**)mir_calloc(sizeof(wchar_t*)*src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++)
			if (!dest->pszFiles.w[i] || !src->pszFiles.w[i] || PFTS_CompareWithTchar(src, src->pszFiles.w[i], dest->pszFiles.w[i])) {
				mir_free(dest->pszFiles.w[i]);
				if (src->pszFiles.w[i])
					dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, src->pszFiles.w[i]);
				else
					dest->pszFiles.w[i] = nullptr;
			}
	}
	else if (dest->pszFiles.w) {
		for (int i = 0; i < dest->totalFiles; i++)
			mir_free(dest->pszFiles.w[i]);
		mir_free(dest->pszFiles.w);
		dest->pszFiles.w = nullptr;
	}

	dest->currentFileNumber = src->currentFileNumber;
	dest->totalBytes = src->totalBytes;
	dest->totalProgress = src->totalProgress;
	if (src->szWorkingDir.w && (!dest->szWorkingDir.w || PFTS_CompareWithTchar(src, src->szWorkingDir.w, dest->szWorkingDir.w))) {
		mir_free(dest->szWorkingDir.w);
		if (src->szWorkingDir.w)
			dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir.w);
		else
			dest->szWorkingDir.w = nullptr;
	}

	if (!dest->szCurrentFile.w || !src->szCurrentFile.w || PFTS_CompareWithTchar(src, src->szCurrentFile.w, dest->szCurrentFile.w)) {
		mir_free(dest->szCurrentFile.w);
		if (src->szCurrentFile.w)
			dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile.w);
		else
			dest->szCurrentFile.w = nullptr;
	}
	dest->currentFileSize = src->currentFileSize;
	dest->currentFileProgress = src->currentFileProgress;
	dest->currentFileTime = src->currentFileTime;
	dest->flags &= ~PFTS_UTF;
	dest->flags |= PFTS_UNICODE;
}

static void RemoveUnreadFileEvents(void)
{
	for (auto &hContact : Contacts()) {
		MEVENT hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			DBEVENTINFO dbei = {};
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
	char *szProto = Proto_GetBaseAccountName(wParam);
	if (szProto != nullptr) {
		bool isChat = Contact::IsGroupChat(wParam, szProto);
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

static int SRFileProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type == ACKTYPE_FILE) {
		int iEvent = 0;
		while (CLISTEVENT *cle = g_clistApi.pfnGetEvent(ack->hContact, iEvent++))
			if (cle->lParam == (LPARAM)ack->hProcess)
				g_clistApi.pfnRemoveEvent(ack->hContact, cle->hDbEvent);
	}
	return 0;
}

static int SRFileModulesLoaded(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x7f8dcf77, 0xe448, 0x4505, 0xb0, 0x56, 0xb, 0xb1, 0xab, 0xac, 0x64, 0x9d);
	mi.position = -2000020000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.name.a = LPGEN("&File");
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
	wchar_t szContRecDir[MAX_PATH];
	GetContactReceivedFilesDir(hContact, szContRecDir, _countof(szContRecDir), TRUE);
	ShellExecute(nullptr, L"open", szContRecDir, nullptr, nullptr, SW_SHOW);
	return 0;
}

INT_PTR openRecDir(WPARAM, LPARAM)
{
	wchar_t szContRecDir[MAX_PATH];
	GetReceivedFilesDir(szContRecDir, _countof(szContRecDir));
	ShellExecute(nullptr, L"open", szContRecDir, nullptr, nullptr, SW_SHOW);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR Proto_RecvFileT(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA*)lParam;
	PROTORECVFILE* pre = (PROTORECVFILE*)ccs->lParam;
	if (pre->fileCount == 0)
		return 0;

	DBEVENTINFO dbei = {};
	dbei.szModule = Proto_GetBaseAccountName(ccs->hContact);
	dbei.timestamp = pre->timestamp;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.flags = DBEF_UTF;
	if (pre->dwFlags & PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;

	bool bUnicode = (pre->dwFlags & PRFF_UNICODE) == PRFF_UNICODE;

	const char *szDescr, **pszFiles;
	if (bUnicode) {
		pszFiles = (const char**)alloca(pre->fileCount * sizeof(char*));
		for (int i = 0; i < pre->fileCount; i++)
			pszFiles[i] = mir_utf8encodeW(pre->files.w[i]);
		
		szDescr = mir_utf8encodeW(pre->descr.w);
	}
	else {
		pszFiles = pre->files.a;
		szDescr = pre->descr.a;
	}

	dbei.cbBlob = sizeof(uint32_t);

	for (int i = 0; i < pre->fileCount; i++)
		dbei.cbBlob += (int)mir_strlen(pszFiles[i]) + 1;

	dbei.cbBlob += (int)mir_strlen(szDescr) + 1;

	if ((dbei.pBlob = (uint8_t*)mir_alloc(dbei.cbBlob)) == nullptr)
		return 0;

	*(uint32_t*)dbei.pBlob = 0;
	uint8_t* p = dbei.pBlob + sizeof(uint32_t);
	for (int i = 0; i < pre->fileCount; i++) {
		mir_strcpy((char*)p, pszFiles[i]);
		p += mir_strlen(pszFiles[i]) + 1;
		if (bUnicode)
			mir_free((void*)pszFiles[i]);
	}

	mir_strcpy((char*)p, (szDescr == nullptr) ? "" : szDescr);
	if (bUnicode)
		mir_free((void*)szDescr);

	MEVENT hdbe = db_event_add(ccs->hContact, &dbei);
	PushFileEvent(ccs->hContact, hdbe, pre->lParam);
	mir_free(dbei.pBlob);
	return 0;
}

int LoadSendRecvFileModule(void)
{
	CreateServiceFunction("FtMgr/Show", FtMgrShowCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x75794ab5, 0x2573, 0x48f4, 0xb4, 0xa0, 0x93, 0xd6, 0xf5, 0xe0, 0xf3, 0x32);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.position = 1900000000;
	mi.name.a = LPGEN("File &transfers...");
	mi.pszService = "FtMgr/Show"; //MS_PROTO_SHOWFTMGR;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, SRFileModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, FileEventAdded);
	HookEvent(ME_OPT_INITIALISE, FileOptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRFilePreBuildMenu);
	HookEvent(ME_PROTO_ACK, SRFileProtoAck);

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

	g_plugin.addSound("RecvFile",   LPGENW("File"), LPGENW("Incoming"));
	g_plugin.addSound("FileDone",   LPGENW("File"), LPGENW("Complete"));
	g_plugin.addSound("FileFailed", LPGENW("File"), LPGENW("Error"));
	g_plugin.addSound("FileDenied", LPGENW("File"), LPGENW("Denied"));
	return 0;
}
