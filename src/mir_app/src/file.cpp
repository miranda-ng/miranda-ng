/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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
CMOption<bool> File::bOfflineDelete(SRFILEMODULE, "OfflineDelete", true);
CMOption<bool> File::bReverseOrder(SRFILEMODULE, "ReverseOrder", false);
CMOption<bool> File::bWarnBeforeOpening(SRFILEMODULE, "WarnBeforeOpening", true);

CMOption<wchar_t*> File::wszSaveDir(SRFILEMODULE, "RecvFilesDirAdv", L"");
CMOption<wchar_t*> File::wszScanCmdLine(SRFILEMODULE, "ScanCmdLine", L"");

CMOption<uint8_t> File::iIfExists(SRFILEMODULE, "IfExists", FILERESUME_ASK);
CMOption<uint8_t> File::iUseScanner(SRFILEMODULE, "UseScanner", VIRUSSCAN_DISABLE);

CMOption<uint32_t> File::iOfflineSize(SRFILEMODULE, "OfflineSize", 2000);

static HGENMENU hSRFileMenuItem;

/////////////////////////////////////////////////////////////////////////////////////////
// remove this piece of shit when we get rid of h++

EXTERN_C MIR_APP_DLL(void) GetFileReceivedFolder(MCONTACT hContact, wchar_t *buf)
{
	File::GetReceivedFolder(hContact, buf, MAX_PATH);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Services

static INT_PTR SendFileCommand(WPARAM hContact, LPARAM)
{
	File::Send(hContact);
	return 0;
}

static INT_PTR RecvFileCommand(WPARAM, LPARAM lParam)
{
	LaunchRecvDialog((CLISTEVENT *)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Events

static void RemoveUnreadFileEvents(void)
{
	for (auto &hContact : Contacts()) {
		MEVENT hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			DB::EventInfo dbei(hDbEvent, false);
			if (dbei.eventType == EVENTTYPE_FILE)
				dbei.wipeNotify();

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
		if (CallProtoService(szProto, PS_GETCAPS, isChat ? PFLAGNUM_4 : PFLAGNUM_1) & (isChat ? PF4_GROUPCHATFILES : PF1_FILESEND)) {
			if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4) & PF4_OFFLINEFILES)
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

static int OnToolbarButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (mir_strcmp(cbcd->pszModule, SRFILEMODULE))
		return 0;

	if (cbcd->dwButtonId == 1) {
		File::Send(cbcd->hContact);
		return 0;
	}
	return 1;
}

static int SRFileModulesLoaded(WPARAM, LPARAM)
{
	// Send File contact menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x7f8dcf77, 0xe448, 0x4505, 0xb0, 0x56, 0xb, 0xb1, 0xab, 0xac, 0x64, 0x9d);
	mi.position = -2000020000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
	mi.name.a = LPGEN("&File");
	mi.pszService = "SRFile/SendCommand";
	hSRFileMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, SendFileCommand);

	// SRMM toolbar button
	BBButton bbd = {};
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_NOREADONLY;
	bbd.dwButtonID = 1;
	bbd.dwDefPos = 50;
	bbd.hIcon = g_plugin.getIconHandle(IDI_ATTACH);
	bbd.pszModuleName = SRFILEMODULE;
	bbd.pwszTooltip = LPGENW("Send file");
	g_plugin.addButton(&bbd);

	HookEvent(ME_MSG_BUTTONPRESSED, OnToolbarButtonPressed);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SRFilePreBuildMenu);
	HookEvent(ME_PROTO_ACK, SRFileProtoAck);

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

static int SRFileEventDeleted(WPARAM hContact, LPARAM hDbEvent)
{
	if (File::bOfflineDelete) {
		DB::EventInfo dbei(hDbEvent);
		if (dbei && dbei.eventType == EVENTTYPE_FILE) {
			DB::FILE_BLOB blob(dbei);
			if (auto *pwszName = blob.getLocalName()) {
				wchar_t wszReceiveFolder[MAX_PATH];
				GetContactSentFilesDir(hContact, wszReceiveFolder, _countof(wszReceiveFolder));

				// we don't remove sent files, located outside Miranda's folder for sent offline files
				if ((dbei.flags & DBEF_SENT) == 0 || !wcsnicmp(pwszName, wszReceiveFolder, wcslen(wszReceiveFolder)))
					DeleteFileW(pwszName);
			}
		}
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
	File::GetReceivedFolder(hContact, szContRecDir, _countof(szContRecDir));
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

MEVENT Proto_RecvFile(MCONTACT hContact, DB::FILE_BLOB &blob, DB::EventInfo &dbei)
{
	bool bSilent = (dbei.flags & DBEF_TEMPORARY) != 0;
	bool bSent = (dbei.flags & DBEF_SENT) != 0;
	bool bRead = (dbei.flags & DBEF_READ) != 0;

	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.eventType = EVENTTYPE_FILE;
	dbei.flags = (dbei.flags & ~DBEF_TEMPORARY) | DBEF_UTF;

	CMStringW wszFiles, wszDescr;

	if (auto *ppro = Proto_GetContactInstance(hContact))
		ppro->OnReceiveOfflineFile(blob, blob.getUserInfo());
	blob.write(dbei);

	MEVENT hdbe = db_event_add(hContact, &dbei);

	// yes, we can receive a file that was sent from another device. let's ignore it
	// also do not notify about events been already read
	if (!bSent && !bRead) {
		CLISTEVENT cle = {};
		cle.hContact = hContact;
		cle.hDbEvent = hdbe;
		cle.lParam = LPARAM(blob.getUserInfo());

		if (!bSilent && File::bAutoAccept && Contact::OnList(hContact))
			LaunchRecvDialog(&cle);
		else {
			// load cloud files always (if OfflineSize = 0) 
			// or if they are less than a limit (if a transfer has specified file size)
			if (bSilent && File::bOfflineAuto)
				if (File::iOfflineSize == 0 || (blob.getSize() > 0 && blob.getSize() < File::iOfflineSize * 1024))
					DownloadOfflineFile(hContact, hdbe, dbei, false, new OFD_Download());

			bool bShow = !Contact::IsGroupChat(hContact);
			if (bShow && blob.isOffline()) {
				auto *pDlg = Srmm_FindDialog(hContact);
				if (!pDlg && db_mc_isSub(hContact))
					pDlg = Srmm_FindDialog(db_mc_getMeta(hContact));
				if (pDlg)
					bShow = false;
			}
			
			if (bShow) {
				Skin_PlaySound("RecvFile");

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
	HookEvent(ME_DB_EVENT_DELETED, SRFileEventDeleted);

	hDlgSucceeded = CreateHookableEvent(ME_FILEDLG_SUCCEEDED);
	hDlgCanceled = CreateHookableEvent(ME_FILEDLG_CANCELED);

	CreateServiceFunction("SRFile/RecvFile", RecvFileCommand);

	CreateServiceFunction("SRFile/OpenContRecDir", openContRecDir);
	CreateServiceFunction("SRFile/OpenRecDir", openRecDir);

	g_plugin.addSound("RecvFile",   LPGENW("File"), LPGENW("Incoming"));
	g_plugin.addSound("FileDone",   LPGENW("File"), LPGENW("Complete"));
	g_plugin.addSound("FileFailed", LPGENW("File"), LPGENW("Error"));
	g_plugin.addSound("FileDenied", LPGENW("File"), LPGENW("Denied"));
	return 0;
}
