/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org),
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

MWindowList g_hFileWindows;
HANDLE hDlgSucceeded, hDlgCanceled;

CMOption<bool> File::bAutoMin(SRFILEMODULE, "AutoMin", false);
CMOption<bool> File::bAutoClear(SRFILEMODULE, "AutoClear", true);
CMOption<bool> File::bAutoClose(SRFILEMODULE, "AutoClose", false);
CMOption<bool> File::bAutoAccept(SRFILEMODULE, "AutoAccept", false);
CMOption<bool> File::bOfflineAuto(SRFILEMODULE, "OfflineAuto", true);
CMOption<bool> File::bReverseOrder(SRFILEMODULE, "ReverseOrder", false);
CMOption<bool> File::bWarnBeforeOpening(SRFILEMODULE, "WarnBeforeOpening", true);

CMOption<wchar_t*> File::wszSaveDir(SRFILEMODULE, "RecvFilesDirAdv", L"");
CMOption<wchar_t*> File::wszScanCmdLine(SRFILEMODULE, "ScanCmdLine", L"");

CMOption<uint8_t> File::iIfExists(SRFILEMODULE, "IfExists", FILERESUME_ASK);
CMOption<uint8_t> File::iUseScanner(SRFILEMODULE, "UseScanner", VIRUSSCAN_DISABLE);

CMOption<uint32_t> File::iOfflineSize(SRFILEMODULE, "OfflineSize", 2000);

static HGENMENU hSRFileMenuItem;

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
	char *dir = mir_u2a(buf);
	mir_strncpy((char *)lParam, dir, MAX_PATH);
	mir_free(dir);
	return 0;
}

static INT_PTR GetReceivedFilesFolderW(WPARAM wParam, LPARAM lParam)
{
	wchar_t buf[MAX_PATH];
	GetContactReceivedFilesDir(wParam, buf, MAX_PATH, TRUE);
	mir_wstrncpy((wchar_t *)lParam, buf, MAX_PATH);
	return 0;
}

static INT_PTR RecvFileCommand(WPARAM, LPARAM lParam)
{
	LaunchRecvDialog((CLISTEVENT *)lParam);
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

CMStringW FindUniqueFileName(const wchar_t *pszOriginalFile)
{
	const wchar_t *pszExtension, *pszFilename;
	if ((pszFilename = wcsrchr(pszOriginalFile, '\\')) == nullptr)
		pszFilename = pszOriginalFile;
	if ((pszExtension = wcsrchr(pszFilename + 1, '.')) == nullptr)
		pszExtension = pszFilename + mir_wstrlen(pszFilename);

	CMStringW buf;
	for (int i = 1;; i++) {
		buf.Format(L"%.*s (%d)%s", unsigned(pszExtension - pszOriginalFile), pszOriginalFile, i, pszExtension);
		if (_waccess(buf, 0) != 0)
			break;
	}
	return buf;
}

// Triple redirection sucks but is needed to nullify the array pointer
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
	if (src->szCurrentFile.w) dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile);
	if (src->pszFiles.w) {
		dest->pszFiles.w = (wchar_t**)mir_alloc(sizeof(wchar_t*)*src->totalFiles);
		for (int i = 0; i < src->totalFiles; i++) {
			MAllStrings s = { src->pszFiles.a[i] };
			dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, s);
		}
	}
	if (src->szWorkingDir.w)
		dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir);
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
		for (int i = 0; i < src->totalFiles; i++) {
			MAllStrings fname; fname.w = src->pszFiles.w[i];
			if (!dest->pszFiles.w[i] || !fname.w || PFTS_CompareWithTchar(src, fname, dest->pszFiles.w[i])) {
				mir_free(dest->pszFiles.w[i]);
				if (fname.w)
					dest->pszFiles.w[i] = PFTS_StringToTchar(src->flags, fname);
				else
					dest->pszFiles.w[i] = nullptr;
			}
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
	if (src->szWorkingDir.w && (!dest->szWorkingDir.w || PFTS_CompareWithTchar(src, src->szWorkingDir, dest->szWorkingDir.w))) {
		mir_free(dest->szWorkingDir.w);
		if (src->szWorkingDir.w)
			dest->szWorkingDir.w = PFTS_StringToTchar(src->flags, src->szWorkingDir);
		else
			dest->szWorkingDir.w = nullptr;
	}

	if (!dest->szCurrentFile.w || !src->szCurrentFile.w || PFTS_CompareWithTchar(src, src->szCurrentFile, dest->szCurrentFile.w)) {
		mir_free(dest->szCurrentFile.w);
		if (src->szCurrentFile.w)
			dest->szCurrentFile.w = PFTS_StringToTchar(src->flags, src->szCurrentFile);
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
			if (!dbei.markedRead() && dbei.eventType == EVENTTYPE_FILE)
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
		while (auto *cle = Clist_GetEvent(ack->hContact, iEvent++))
			if (cle->lParam == (LPARAM)ack->hProcess)
				Clist_RemoveEvent(ack->hContact, cle->hDbEvent);
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

static int SRFilePreShutdown(WPARAM, LPARAM)
{
	if (g_hFileWindows) {
		WindowList_Broadcast(g_hFileWindows, WM_CLOSE, 0, 1);
		WindowList_Destroy(g_hFileWindows);
		g_hFileWindows = 0;
	}
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

MEVENT Proto_RecvFile(MCONTACT hContact, PROTORECVFILE *pre)
{
	if (!pre || !pre->fileCount)
		return 0;

	bool bSilent = (pre->dwFlags & PRFF_SILENT) != 0;
	bool bSent = (pre->dwFlags & PRFF_SENT) != 0;

	DB::EventInfo dbei;
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.szId = pre->szId;
	dbei.szUserId = pre->szUserId;
	dbei.eventType = EVENTTYPE_FILE;
	dbei.flags = DBEF_UTF;
	if (bSent)
		dbei.flags |= DBEF_SENT;
	if (pre->dwFlags & PRFF_READ)
		dbei.flags |= DBEF_READ;

	CMStringW wszFiles, wszDescr;

	if ((pre->dwFlags & PRFF_UNICODE) == PRFF_UNICODE) {
		for (int i = 0; i < pre->fileCount; i++) {
			if (i != 0)
				wszFiles.AppendChar(',');
			wszFiles.Append(pre->files.w[i]);
		}
		
		wszDescr = pre->descr.w;
	}
	else {
		bool bUtf = (pre->dwFlags & PRFF_UTF) != 0;

		for (int i = 0; i < pre->fileCount; i++) {
			if (i != 0)
				wszFiles.AppendChar(',');

			if (bUtf)
				wszFiles.Append(Utf2T(pre->files.a[i]));
			else
				wszFiles.Append(_A2T(pre->files.a[i]));
		}

		wszDescr = (bUtf) ? Utf2T(pre->descr.a).get() : _A2T(pre->descr.a);
	}

	DB::FILE_BLOB blob(wszFiles, wszDescr);
	CallProtoService(dbei.szModule, PS_PRECREATE_OFFLINEFILE, WPARAM(&blob), pre->lParam);
	blob.write(dbei);

	MEVENT hdbe = db_event_add(hContact, &dbei);

	// yes, we can receive a file that was sent from another device. let's ignore it
	if (!bSent) {
		CLISTEVENT cle = {};
		cle.hContact = hContact;
		cle.hDbEvent = hdbe;
		cle.lParam = pre->lParam;

		if (!bSilent && File::bAutoAccept && Contact::OnList(hContact))
			LaunchRecvDialog(&cle);
		else {
			Skin_PlaySound("RecvFile");

			// load offline files always (if OfflineSize = 0) 
			// or if they are less than a limit (if a transfer has specified file size)
			if (bSilent && File::bOfflineAuto)
				if (File::iOfflineSize == 0 || (blob.getSize() > 0 && blob.getSize() < File::iOfflineSize * 1024))
					Srmm_DownloadOfflineFile(hdbe, false);

			if (!Contact::IsGroupChat(hContact)) {
				wchar_t szTooltip[256];
				mir_snwprintf(szTooltip, TranslateT("File from %s"), Clist_GetContactDisplayName(hContact));

				cle.szTooltip.w = szTooltip;
				cle.flags |= CLEF_UNICODE;
				cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
				cle.pszService = (bSilent) ? MS_MSG_READMESSAGE : "SRFile/RecvFile";
				g_clistApi.pfnAddEvent(&cle);
			}
		}
	}
	
	return hdbe;
}

int LoadSendRecvFileModule(void)
{
	g_hFileWindows = WindowList_Create();

	CreateServiceFunction("FtMgr/Show", FtMgrShowCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x75794ab5, 0x2573, 0x48f4, 0xb4, 0xa0, 0x93, 0xd6, 0xf5, 0xe0, 0xf3, 0x32);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.position = 1900000000;
	mi.name.a = LPGEN("File &transfers...");
	mi.pszService = "FtMgr/Show"; //MS_PROTO_SHOWFTMGR;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_SYSTEM_MODULESLOADED, SRFileModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SRFilePreShutdown);
	HookEvent(ME_OPT_INITIALISE, SRFileOptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRFilePreBuildMenu);
	HookEvent(ME_PROTO_ACK, SRFileProtoAck);

	hDlgSucceeded = CreateHookableEvent(ME_FILEDLG_SUCCEEDED);
	hDlgCanceled = CreateHookableEvent(ME_FILEDLG_CANCELED);

	CreateServiceFunction(MS_FILE_SENDFILE, SendFileCommand);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILES, SendSpecificFiles);
	CreateServiceFunction(MS_FILE_SENDSPECIFICFILEST, SendSpecificFilesT);
	CreateServiceFunction(MS_FILE_GETRECEIVEDFILESFOLDER, GetReceivedFilesFolder);
	CreateServiceFunction(MS_FILE_GETRECEIVEDFILESFOLDERW, GetReceivedFilesFolderW);
	CreateServiceFunction("SRFile/RecvFile", RecvFileCommand);

	CreateServiceFunction("SRFile/OpenContRecDir", openContRecDir);
	CreateServiceFunction("SRFile/OpenRecDir", openRecDir);

	g_plugin.addSound("RecvFile",   LPGENW("File"), LPGENW("Incoming"));
	g_plugin.addSound("FileDone",   LPGENW("File"), LPGENW("Complete"));
	g_plugin.addSound("FileFailed", LPGENW("File"), LPGENW("Error"));
	g_plugin.addSound("FileDenied", LPGENW("File"), LPGENW("Denied"));
	return 0;
}