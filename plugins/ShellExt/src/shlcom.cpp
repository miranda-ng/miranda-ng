#include "stdafx.h"
#include "shlcom.h"
#include "shlicons.h"

#pragma comment(lib, "rpcrt4.lib")

int DllFactoryCount, DllObjectCount;

struct TCMInvokeCommandInfo
{
	int   cbSize;
	uint32_t fMask;
	HWND  hwnd;
	char* lpVerb;  // maybe index, type cast as Integer
	char* lpParams;
	char* lpDir;
	int   nShow;
	uint32_t dwHotkey;
	HICON hIcon;
};

/////////////////////////////////////////////////////////////////////////////////////////

int IsCOMRegistered()
{
	HKEY hRegKey;
	int  res = 0;

	// these arent the BEST checks in the world
	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"miranda.shlext", 0, KEY_READ, &hRegKey)) {
		res += COMREG_OK;
		RegCloseKey(hRegKey);
	}

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_READ, &hRegKey)) {
		DWORD lpType = REG_SZ;
		if (!RegQueryValueEx(hRegKey, L"{72013A26-A94C-11d6-8540-A5E62932711D}", nullptr, &lpType, nullptr, nullptr))
			res += COMREG_APPROVED;
		RegCloseKey(hRegKey);
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

char* CreateProcessUID(int pid, char* buf, size_t bufLen)
{
	sprintf_s(buf, bufLen, "mim.shlext.%d$", pid);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////

//
// IPC part
//

struct TAddArgList
{
	LPSTR szFile; // file being processed
	int cch; // it's length (with space for NULL char)
	int count; // number we have so far
	LPSTR* files;
	MCONTACT hContact;
	HANDLE hEvent;
};

BOOL AddToList(TAddArgList& args)
{
	char szBuf[MAX_PATH];
	LPSTR szThis;

	uint32_t attr = GetFileAttributesA(args.szFile);
	if (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_HIDDEN) == 0) {
		if ((args.count % 10) == 5)
			if (Miranda_IsTerminated() != 0)
				return true;

		if (attr & FILE_ATTRIBUTE_DIRECTORY) {
			// add the directory
			lstrcpyA(szBuf, args.szFile);
			args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
			char* p = mir_strdup(szBuf);
			args.files[args.count++] = p;
			// tack on ending search token
			lstrcatA(szBuf, "\\*");

			WIN32_FIND_DATAA fd;
			HANDLE hFind = FindFirstFileA(szBuf, &fd);
			while (true) {
				if (fd.cFileName[0] != '.') {
					mir_snprintf(szBuf, "%s\\%s", args.szFile, fd.cFileName);
					// keep a copy of the current thing being processed
					szThis = args.szFile;
					args.szFile = szBuf;
					int cchThis = args.cch;
					args.cch = (int)strlen(szBuf) + 1;
					// recurse
					BOOL Result = AddToList(args);
					// restore
					args.szFile = szThis;
					args.cch = cchThis;
					if (Result) {
						FindClose(hFind);
						return true;
					}
				}
				if (!FindNextFileA(hFind, &fd))
					break;
			}
			FindClose(hFind);
		}
		else {
			// add the file
			args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
			args.files[args.count++] = mir_strdup(args.szFile);
		}
	}
	return false;
}

void NTAPI MainThreadIssueTransfer(ULONG_PTR param)
{
	TAddArgList* p = (TAddArgList*)param;
	g_plugin.setByte(p->hContact, SHLExt_MRU, 1);
	CallService(MS_FILE_SENDSPECIFICFILES, (WPARAM)p->hContact, LPARAM(p->files));
	SetEvent(p->hEvent);
}

void __cdecl IssueTransferThread(THeaderIPC * pipch)
{
	HANDLE hMainThread = HANDLE(pipch->Param);

	char szBuf[MAX_PATH];
	GetCurrentDirectoryA(sizeof(szBuf), szBuf);

	TAddArgList args;
	args.count = 0;
	args.files = nullptr;
	TSlotIPC* pct = pipch->DataPtr;
	BOOL bQuit = false;
	while (pct != nullptr) {
		if (pct->cbSize != sizeof(TSlotIPC))
			break;
		args.szFile = LPSTR(UINT_PTR(pct) + sizeof(TSlotIPC));
		args.hContact = pct->hContact;
		args.cch = pct->cbStrSection + 1;
		bQuit = AddToList(args);
		if (bQuit)
			break;
		pct = pct->Next;
	} // while

	if (args.files != nullptr) {
		args.files = (LPSTR*)mir_realloc(args.files, (args.count + 1) * sizeof(LPSTR));
		args.files[args.count++] = nullptr;
		if (!bQuit) {
			args.hEvent = CreateEvent(nullptr, true, false, nullptr);
			QueueUserAPC(MainThreadIssueTransfer, hMainThread, UINT_PTR(&args));
			while (true) {
				if (WaitForSingleObjectEx(args.hEvent, INFINITE, true) != WAIT_IO_COMPLETION)
					break;
			}
			CloseHandle(args.hEvent);
		}
		for (int j = 0; j < args.count; j++)
			mir_free(args.files[j]);
		mir_free(args.files);
	}
	SetCurrentDirectoryA(szBuf);
	mir_free(pipch);
	CloseHandle(hMainThread);
}

struct TSlotInfo
{
	MCONTACT hContact;
	int    hProto;
	int    dwStatus; // will be aligned anyway
};

int __cdecl SortContact(const void* Item1, const void* Item2)
{
	return Clist_ContactCompare(((TSlotInfo*)Item1)->hContact, ((TSlotInfo*)Item2)->hContact);
}

void ipcGetSkinIcons(THeaderIPC * ipch)
{
	TSlotProtoIcons spi;
	char szTmp[64];

	int protoCount;
	PROTOACCOUNT** pp;
	Proto_EnumAccounts(&protoCount, &pp);
	if (protoCount != 0) {
		spi.pid = GetCurrentProcessId();
		while (protoCount > 0) {
			PROTOACCOUNT* pa = *pp;
			lstrcpyA(szTmp, pa->szModuleName);
			lstrcatA(szTmp, PS_GETCAPS);
			uint32_t dwCaps = CallService(szTmp, PFLAGNUM_1, 0);
			if (dwCaps & PF1_FILESEND) {
				TSlotIPC* pct = ipcAlloc(ipch, sizeof(TSlotProtoIcons));
				if (pct != nullptr) {
					// capture all the icons!
					spi.hProto = murmur_hash(pa->szModuleName);
					for (int j = 0; j < _countof(spi.hIcons); j++)
						spi.hIcons[j] = Skin_LoadProtoIcon(pa->szModuleName, ID_STATUS_OFFLINE + j);

					pct->fType = REQUEST_NEWICONS;
					memcpy(LPSTR(pct) + sizeof(TSlotIPC), &spi, sizeof(TSlotProtoIcons));
					if (ipch->NewIconsBegin == nullptr)
						ipch->NewIconsBegin = pct;
				}
			}
			pp++;
			protoCount--;
		}
	}

	// add Miranda icon
	TSlotIPC* pct = ipcAlloc(ipch, sizeof(TSlotProtoIcons));
	if (pct != nullptr) {
		memset(&spi.hIcons, 0, sizeof(spi.hIcons));
		spi.hProto = 0; // no protocol
		spi.hIcons[0] = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		pct->fType = REQUEST_NEWICONS;
		memcpy(LPSTR(pct) + sizeof(TSlotIPC), &spi, sizeof(TSlotProtoIcons));
		if (ipch->NewIconsBegin == nullptr)
			ipch->NewIconsBegin = pct;
	}
}

bool ipcGetSortedContacts(THeaderIPC * ipch, int* pSlot, bool bGroupMode)
{
	// hide offliners?
	bool bHideOffline = Clist::HideOffline;
	// do they wanna hide the offline people anyway?
	if (g_plugin.getByte(SHLExt_ShowNoOffline, 0) == 1)
		// hide offline people
		bHideOffline = true;

	// get the number of contacts
	int dwContacts = db_get_contact_count();
	if (dwContacts == 0)
		return false;

	// get the contacts in the array to be sorted by status, trim out anyone
	// who doesn't wanna be seen.
	TSlotInfo * pContacts = (TSlotInfo*)mir_alloc((dwContacts + 2) * sizeof(TSlotInfo));
	int i = 0;
	int dwOnline = 0;
	for (auto& hContact : Contacts()) {
		if (i >= dwContacts)
			break;

		// do they have a running protocol? 
		char* szProto = Proto_GetBaseAccountName(hContact);
		if (szProto != nullptr) {
			// does it support file sends?
			uint32_t dwCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
			if ((dwCaps & PF1_FILESEND) == 0)
				continue;

			int dwStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			if (dwStatus != ID_STATUS_OFFLINE)
				dwOnline++;
			else if (bHideOffline)
				continue;

			// is HIT on?
			if (BST_UNCHECKED == g_plugin.getByte(SHLExt_UseHITContacts, BST_UNCHECKED)) {
				// don't show people who are hidden, "NotOnList" or ignored
				if (Contact::IsHidden(hContact) || !Contact::OnList(hContact) || Ignore_IsIgnored(hContact, IGNOREEVENT_MESSAGE | IGNOREEVENT_FILE) != 0)
					continue;
			}
			// is HIT2 off?
			if (BST_UNCHECKED == g_plugin.getByte(SHLExt_UseHIT2Contacts, BST_UNCHECKED))
				if (db_get_w(hContact, szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
					continue;

			// store
			pContacts[i].hContact = hContact;
			pContacts[i].dwStatus = dwStatus;
			pContacts[i++].hProto = murmur_hash(szProto);
		}
	}

	// if no one is online and the CList isn't showing offliners, quit
	if (dwOnline == 0 && bHideOffline) {
		mir_free(pContacts);
		return false;
	}

	dwContacts = i;
	qsort(pContacts, dwContacts, sizeof(TSlotInfo), SortContact);

	// create an IPC slot for each contact and store display name, etc
	for (i = 0; i < dwContacts; i++) {
		ptrA szContact(mir_u2a(Clist_GetContactDisplayName(pContacts[i].hContact)));
		if (szContact != NULL) {
			ptrA szGroup;
			if (bGroupMode)
				szGroup = db_get_sa(pContacts[i].hContact, "CList", "Group");

			int cch = lstrlenA(szContact) + 1;
			TSlotIPC* pct = ipcAlloc(ipch, cch + 1 + lstrlenA(szGroup) + 1);
			if (pct == nullptr)
				break;

			// lie about the actual size of the TSlotIPC
			pct->cbStrSection = cch;
			LPSTR szSlot = LPSTR(pct) + sizeof(TSlotIPC);
			lstrcpyA(szSlot, szContact);
			pct->fType = REQUEST_CONTACTS;
			pct->hContact = pContacts[i].hContact;
			pct->Status = pContacts[i].dwStatus;
			pct->hProto = pContacts[i].hProto;
			pct->MRU = g_plugin.getByte(pct->hContact, SHLExt_MRU);
			if (ipch->ContactsBegin == nullptr)
				ipch->ContactsBegin = pct;
			szSlot += cch + 1;
			if (szGroup != 0) {
				pct->hGroup = murmur_hash(szGroup);
				lstrcpyA(szSlot, szGroup);
			}
			else {
				pct->hGroup = 0;
				*szSlot = 0;
			}
			pSlot[0]++;
		}
	}
	mir_free(pContacts);
	return true;
}

// worker thread to clear MRU, called by the IPC bridge
void __cdecl ClearMRUThread(void*)
{
	for (auto& hContact : Contacts())
		if (g_plugin.getBool(hContact, SHLExt_MRU))
			g_plugin.setByte(hContact, SHLExt_MRU, false);
}

// this function is called from an APC into the main thread
void __stdcall ipcService(ULONG_PTR)
{
	HANDLE hSignal;
	LPSTR szBuf;
	char szGroupStr[32];
	DBVARIANT dbv;
	LPSTR szMiranda;

	// try to open the file mapping object the caller must make sure no other
	// running instance is using this file
	HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, IPC_PACKET_NAME);
	if (hMap == nullptr)
		return;

	// map the file to this process
	THeaderIPC * pMMT = (THeaderIPC*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	// if it fails the caller should of had some timeout in wait
	if (pMMT != nullptr && pMMT->cbSize == sizeof(THeaderIPC) && pMMT->dwVersion == PLUGIN_MAKE_VERSION(2, 0, 1, 2)) {
		// toggle the right bits
		int* bits = &pMMT->fRequests;
		// jump right to a worker thread for file processing?
		if (*bits & REQUEST_XFRFILES) {
			THeaderIPC* cloned = (THeaderIPC*)mir_alloc(IPC_PACKET_SIZE);
			// translate from client space to cloned heap memory
			pMMT->pServerBaseAddress = pMMT->pClientBaseAddress;
			pMMT->pClientBaseAddress = cloned;
			memcpy(cloned, pMMT, IPC_PACKET_SIZE);
			ipcFixupAddresses(cloned);
			DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &cloned->Param, THREAD_SET_CONTEXT, false, 0);
			mir_forkThread<THeaderIPC>(&IssueTransferThread, cloned);
			goto Reply;
		}
		// the request was to clear the MRU entries, we have no return data
		if (*bits & REQUEST_CLEARMRU) {
			mir_forkthread(&ClearMRUThread);
			goto Reply;
		}
		// the IPC header may have pointers that need to be translated
		// in either case the supplied data area pointers has to be
		// translated to this address space.
		// the server base address is always removed to get an offset
		// to which the client base is added, this is what ipcFixupAddresses() does
		pMMT->pServerBaseAddress = pMMT->pClientBaseAddress;
		pMMT->pClientBaseAddress = pMMT;
		// translate to the server space map
		ipcFixupAddresses(pMMT);
		// store the address map offset so the caller can retranslate
		pMMT->pServerBaseAddress = pMMT;
		// return some options to the client
		if (g_plugin.getByte(SHLExt_ShowNoIcons, 0) != 0)
			pMMT->dwFlags = HIPC_NOICONS;

		// see if we have a custom string for 'Miranda'
		szMiranda = "Miranda";
		lstrcpynA(pMMT->MirandaName, szMiranda, sizeof(pMMT->MirandaName) - 1);

		// for the MRU menu
		szBuf = Translate("Recently");
		lstrcpynA(pMMT->MRUMenuName, szBuf, sizeof(pMMT->MRUMenuName) - 1);

		// and a custom string for "clear entries"
		szBuf = Translate("Clear entries");
		lstrcpynA(pMMT->ClearEntries, szBuf, sizeof(pMMT->ClearEntries) - 1);

		// if the group mode is on, check if they want the CList setting
		bool bGroupMode = (BST_CHECKED == g_plugin.getByte(SHLExt_UseGroups, BST_UNCHECKED));
		if (bGroupMode && BST_CHECKED == g_plugin.getByte(SHLExt_UseCListSetting, BST_UNCHECKED))
			bGroupMode = Clist::UseGroups;

		TSlotIPC * pct = nullptr;
		int iSlot = 0;
		// return profile if set
		if (BST_UNCHECKED == g_plugin.getByte(SHLExt_ShowNoProfile, BST_UNCHECKED)) {
			pct = ipcAlloc(pMMT, 50);
			if (pct != nullptr) {
				// will actually return with .dat if there's space for it, not what the docs say
				pct->Status = STATUS_PROFILENAME;
				Profile_GetNameA(49, (char*)pct + sizeof(TSlotIPC));
			}
		}
		if (*bits & REQUEST_NEWICONS)
			ipcGetSkinIcons(pMMT);

		if (*bits & REQUEST_GROUPS) {
			// return contact's grouping if it's present
			while (bGroupMode) {
				_itoa(iSlot, szGroupStr, 10);
				if (db_get_s(0, "CListGroups", szGroupStr, &dbv) != 0)
					break;
				pct = ipcAlloc(pMMT, lstrlenA(dbv.pszVal + 1) + 1);
				// first byte has flags, need null term
				if (pct != nullptr) {
					if (pMMT->GroupsBegin == nullptr)
						pMMT->GroupsBegin = pct;
					pct->fType = REQUEST_GROUPS;
					pct->hContact = 0;
					szBuf = LPSTR(pct) + sizeof(TSlotIPC); // get the end of the slot
					lstrcpyA(szBuf, dbv.pszVal + 1);
					pct->hGroup = 0;
					db_free(&dbv); // free the string
				}
				else {
					// outta space
					db_free(&dbv);
					break;
				}
				iSlot++;
			}
			// if there was no space left, it'll } on null
			if (pct == nullptr)
				* bits = (*bits | GROUPS_NOTIMPL) & ~REQUEST_GROUPS;
		}
		// SHOULD check slot space.
		if (*bits & REQUEST_CONTACTS) {
			if (!ipcGetSortedContacts(pMMT, &iSlot, bGroupMode))
				// fail if there were no contacts AT ALL
				* bits = (*bits | CONTACTS_NOTIMPL) & ~REQUEST_CONTACTS;
		}
		// store the number of slots allocated
		pMMT->Slots = iSlot;
Reply:
		// get the handle the caller wants to be signalled on 
		hSignal = OpenEventA(EVENT_ALL_ACCESS, false, pMMT->SignalEventName);
		if (hSignal != nullptr) {
			SetEvent(hSignal);
			CloseHandle(hSignal);
		}

		UnmapViewOfFile(pMMT);
	}
	CloseHandle(hMap);
}

void __cdecl ThreadServer(HANDLE hMainThread)
{
	char szBuf[100];
	HANDLE hEvent = CreateEventA(nullptr, false, false, CreateProcessUID(GetCurrentProcessId(), szBuf, sizeof(szBuf)));
	while (true) {
		int retVal = WaitForSingleObjectEx(hEvent, INFINITE, true);
		if (retVal == WAIT_OBJECT_0)
			QueueUserAPC(ipcService, hMainThread, 0);

		if (Miranda_IsTerminated() == 1)
			break;
	}
	CloseHandle(hEvent);
	CloseHandle(hMainThread);
}

void InvokeThreadServer()
{
	HANDLE hMainThread = nullptr;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, false, 0);
	if (hMainThread != nullptr)
		mir_forkthread(&ThreadServer, hMainThread);
}

// helper functions
HRESULT RemoveCOMRegistryEntries()
{
	HKEY hRootKey;
	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"miranda.shlext", 0, KEY_READ, &hRootKey)) {
		// need to delete the subkey before the parent key is deleted under NT/2000/XP
		RegDeleteKey(hRootKey, L"CLSID");
		// close the key
		RegCloseKey(hRootKey);
		// delete it
		if (RegDeleteKey(HKEY_CLASSES_ROOT, L"miranda.shlext") != ERROR_SUCCESS)
			MessageBox(nullptr,
				TranslateT("Unable to delete registry key for 'shlext COM', this key may already be deleted or you may need admin rights."),
				TranslateT("Problem"), MB_ICONERROR);
	}
	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"\\*\\shellex\\ContextMenuHandlers", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if (RegDeleteKey(hRootKey, L"miranda.shlext") != ERROR_SUCCESS)
			MessageBox(nullptr,
				TranslateT("Unable to delete registry key for 'File context menu handlers', this key may already be deleted or you may need admin rights."),
				TranslateT("Problem"), MB_ICONERROR);
		RegCloseKey(hRootKey);
	}
	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"Directory\\shellex\\ContextMenuHandlers", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if (RegDeleteKey(hRootKey, L"miranda.shlext") != ERROR_SUCCESS)
			MessageBox(nullptr,
				TranslateT("Unable to delete registry key for 'Directory context menu handlers', this key may already be deleted or you may need admin rights."),
				TranslateT("Problem"), MB_ICONERROR);
		RegCloseKey(hRootKey);
	}
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_ALL_ACCESS, &hRootKey)) {
		if (RegDeleteValue(hRootKey, L"{72013A26-A94C-11d6-8540-A5E62932711D}") != ERROR_SUCCESS) {
			MessageBox(nullptr,
				TranslateT("Unable to delete registry entry for 'Approved context menu handlers', this key may already be deleted or you may need admin rights."),
				TranslateT("Problem"), MB_ICONERROR);
		}
		RegCloseKey(hRootKey);
	}
	return S_OK;
}

// called by the options code to remove COM entries, and before that, get permission, if required.
void CheckUnregisterServer()
{
	if (bIsVistaPlus) {
		// launches regsvr to remove the dll under admin.
		wchar_t szFileName[MAX_PATH], szBuf[MAX_PATH * 2];
		GetModuleFileName(g_plugin.getInst(), szFileName, _countof(szFileName));
		mir_snwprintf(szBuf, L"/s /u \"%s\"", szFileName);

		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = L"regsvr32";
		sei.lpParameters = szBuf;
		if (ShellExecuteEx(&sei) == TRUE)
			return;

		Sleep(1000);
	}
	RemoveCOMRegistryEntries();
}

// Wow, I can't believe there isn't a direct API for this - 'runas' will invoke the UAC and ask
// for permission before installing the shell extension.  note the filepath arg has to be quoted }
void CheckRegisterServer()
{
	wchar_t szFileName[MAX_PATH], szBuf[MAX_PATH * 2];

	HKEY hRegKey;
	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"miranda.shlext", 0, KEY_READ, &hRegKey))
		RegCloseKey(hRegKey);
	else if (bIsVistaPlus) {
		MessageBox(nullptr,
			TranslateT("Shell context menus requires your permission to register with Windows Explorer (one time only)."),
			TranslateT("Miranda NG - Shell context menus (shellext.dll)"), MB_OK | MB_ICONINFORMATION);
		// /s = silent
		GetModuleFileName(g_plugin.getInst(), szFileName, _countof(szFileName));
		mir_snwprintf(szBuf, L"/s \"%s\"", szFileName);

		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = L"regsvr32";
		sei.lpParameters = szBuf;
		ShellExecuteEx(&sei);
	}
}
