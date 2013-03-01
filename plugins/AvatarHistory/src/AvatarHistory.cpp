/*
Avatar History Plugin
---------

 This plugin uses the event provided by Avatar Service to 
 automatically back up contacts' avatars when they change.
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "AvatarHistory.h"

HINSTANCE hInst;

DWORD mirVer;

HANDLE hFolder = NULL;

char *metacontacts_proto = NULL;

TCHAR profilePath[MAX_PATH];		// database profile path (read at startup only)
TCHAR basedir[MAX_PATH];
int hLangpack = 0;
HANDLE hAvatarWindowsList = NULL;

static int ModulesLoaded(WPARAM wParam, LPARAM lParam);
static int PreShutdown(WPARAM wParam, LPARAM lParam);
static int AvatarChanged(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam,LPARAM lParam);

TCHAR * GetHistoryFolder(TCHAR *fn);
TCHAR * GetProtocolFolder(TCHAR *fn, char *proto);
TCHAR * GetOldStyleAvatarName(TCHAR *fn, HANDLE hContact);

void InitMenuItem();

void * GetHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format);

// Services
static INT_PTR IsEnabled(WPARAM wParam, LPARAM lParam);
static INT_PTR GetCachedAvatar(WPARAM wParam, LPARAM lParam);
TCHAR * GetCachedAvatar(char *proto, TCHAR *hash);
BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut);

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DBE8C990-7AA0-458D-BAB7-33EB07238E71}
	{0xdbe8c990, 0x7aa0, 0x458d, {0xba, 0xb7, 0x33, 0xeb, 0x7, 0x23, 0x8e, 0x71}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static INT_PTR CALLBACK FirstRunDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM) createDefaultOverlayedIcon(TRUE));
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) createDefaultOverlayedIcon(FALSE));
			TranslateDialogDefault(hwnd);

			CheckDlgButton(hwnd, IDC_MIR_PROTO, BST_CHECKED);
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					int ret = 0;

					if (IsDlgButtonChecked(hwnd, IDC_MIR_SAME))
						ret = IDC_MIR_SAME;
					else if (IsDlgButtonChecked(hwnd, IDC_MIR_PROTO))
						ret = IDC_MIR_PROTO;
					else if (IsDlgButtonChecked(hwnd, IDC_MIR_SHORT))
						ret = IDC_MIR_SHORT;
					else if (IsDlgButtonChecked(hwnd, IDC_SHORT))
						ret = IDC_SHORT;
					else if (IsDlgButtonChecked(hwnd, IDC_DUP))
						ret = IDC_DUP;

					EndDialog(hwnd, ret);
					return TRUE;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			EndDialog(hwnd, 0);
			return TRUE;
		}
	}

	return FALSE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	// Is first run?
	if (DBGetContactSettingByte(NULL, MODULE_NAME, "FirstRun", 1))
	{
		// Show dialog
		int ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FIRST_RUN), NULL, FirstRunDlgProc, 0);
		if (ret == 0)
			return -1;

		// Write settings

		DBWriteContactSettingByte(NULL, MODULE_NAME, "LogToDisk", 1);

		if (ret == IDC_MIR_SAME)
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogKeepSameFolder", 1);
		else
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogKeepSameFolder", 0);

		if (ret == IDC_MIR_SHORT || ret == IDC_SHORT || ret == IDC_DUP)
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogPerContactFolders", 1);
		else
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogPerContactFolders", 0);

		if (ret == IDC_DUP)
			DBWriteContactSettingByte(NULL, MODULE_NAME, "StoreAsHash", 0);
		else
			DBWriteContactSettingByte(NULL, MODULE_NAME, "StoreAsHash", 1);

		if (ret == IDC_MIR_SAME || ret == IDC_MIR_PROTO || ret == IDC_MIR_SHORT)
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogToHistory", 1);
		else
			DBWriteContactSettingByte(NULL, MODULE_NAME, "LogToHistory", 0);

		DBWriteContactSettingByte(NULL, MODULE_NAME, "FirstRun", 0);
	}

	LoadOptions();

	HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	CreateServiceFunction(MS_AVATARHISTORY_ENABLED, IsEnabled);
	CreateServiceFunction(MS_AVATARHISTORY_GET_CACHED_AVATAR, GetCachedAvatar);

	if(CallService(MS_DB_GETPROFILEPATHT, MAX_PATH, (LPARAM)profilePath) != 0)
		_tcscpy(profilePath, _T(".")); // Failed, use current dir

	SkinAddNewSoundExT("avatar_changed",LPGENT("Avatar History"),LPGENT("Contact changed avatar"));
	SkinAddNewSoundExT("avatar_removed",LPGENT("Avatar History"),LPGENT("Contact removed avatar"));

	hAvatarWindowsList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	SetupIcoLib();
	InitMenuItem();

	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	mir_sntprintf(basedir, MAX_REGS(basedir), _T("%s\\Avatars History"), profilePath);

	hFolder = FoldersRegisterCustomPathT( LPGEN("Avatars"), LPGEN("Avatar History"),
		PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\Avatars History"));

	InitPopups();

	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);

	if (DBGetContactSettingByte(NULL, MODULE_NAME, "LogToHistory", AVH_DEF_LOGTOHISTORY)) {
		char *templates[] = { "Avatar change\nchanged his/her avatar", "Avatar removal\nremoved his/her avatar" };
		HICON hIcon = createDefaultOverlayedIcon(FALSE);
		HistoryEvents_RegisterWithTemplates(MODULE_NAME, "avatarchange", "Avatar change", EVENTTYPE_AVATAR_CHANGE, hIcon, 
			HISTORYEVENTS_FORMAT_CHAR | HISTORYEVENTS_FORMAT_WCHAR | HISTORYEVENTS_FORMAT_RICH_TEXT,
			HISTORYEVENTS_FLAG_SHOW_IM_SRMM | HISTORYEVENTS_FLAG_EXPECT_CONTACT_NAME_BEFORE, 
			GetHistoryEventText, templates, MAX_REGS(templates));
		DestroyIcon(hIcon);
	}
	
	HookEvent(ME_AV_CONTACTAVATARCHANGED, AvatarChanged);

	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hAvatarWindowsList,WM_CLOSE,0,0);
	return 0;
}

BOOL ProtocolEnabled(const char *proto)
{
	if (proto == NULL)
		return FALSE;
		
	char setting[256];
	mir_snprintf(setting, sizeof(setting), "%sEnabled", proto);
	return (BOOL) DBGetContactSettingByte(NULL, MODULE_NAME, setting, TRUE);
}

BOOL ContactEnabled(HANDLE hContact, char *setting, int def) 
{
	if (hContact == NULL)
		return FALSE;

	char *proto = GetContactProto(hContact);
	if (!ProtocolEnabled(proto))
		return FALSE;

	BYTE globpref = db_get_b(NULL, MODULE_NAME, setting, def);
	BYTE userpref = db_get_b(hContact, MODULE_NAME, setting, BST_INDETERMINATE);

	return (globpref && userpref == BST_INDETERMINATE) || userpref == BST_CHECKED;
}

// Returns true if the unicode buffer only contains 7-bit characters.
BOOL IsUnicodeAscii(const WCHAR * pBuffer, int nSize)
{
	BOOL bResult = TRUE;
	int nIndex;

	for (nIndex = 0; nIndex < nSize; nIndex++) {
		if (pBuffer[nIndex] > 0x7F) {
			bResult = FALSE;
			break;
		}
	}
	return bResult;
}

void ConvertToFilename(TCHAR *str, size_t size) {
	for(size_t i = 0; i < size && str[i] != '\0'; i++) {
		switch(str[i]) {
			case '/':
			case '\\':
			case ':':
			case '*':
			case '?':
			case '"':
			case '<':
			case '>':
			case '|':
			//case '.':
				str[i] = '_';
		}
	}
}

void ErrorExit(HANDLE hContact,LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    ShowDebugPopup(hContact,TEXT("Error"),  (LPCTSTR)lpDisplayBuf); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

#ifdef _UNICODE

#define CS "%S"

#else

#define CS "%s"

#endif


TCHAR * GetExtension(TCHAR *file)
{
	if(file == NULL) return _T("");
	TCHAR *ext = _tcsrchr(file, _T('.'));
	if (ext != NULL)
		ext++;
	else
		ext = _T("");

	return ext;
}


void CreateOldStyleShortcut(HANDLE hContact, TCHAR *history_filename)
{
	TCHAR shortcut[MAX_PATH] = _T("");

	GetOldStyleAvatarName(shortcut, hContact);

	mir_sntprintf(shortcut, MAX_REGS(shortcut), _T("%s.%s.lnk"), shortcut, 
		GetExtension(history_filename));

	if (!CreateShortcut(history_filename, shortcut))
	{
		ShowPopup(hContact, TranslateT("Avatar History: Unable to create shortcut"), shortcut);
	}
	else
	{
		ShowDebugPopup(hContact, TranslateT("AVH Debug: Shortcut created successfully"), shortcut);
	}
}


BOOL CopyImageFile(TCHAR *old_file, TCHAR *new_file)
{
	TCHAR *ext = GetExtension(old_file);
	mir_sntprintf(new_file, MAX_PATH, _T("%s.%s"), new_file, ext);

	BOOL ret = CopyFile(old_file, new_file, TRUE);
	if(!ret)
		ErrorExit(NULL,_T("CopyImageFile"));
	return !ret;
}

// fired when the contacts avatar changes
// wParam = hContact
// lParam = struct avatarCacheEntry *cacheEntry
// the event CAN pass a NULL pointer in lParam which means that the avatar has changed,
// but is no longer valid (happens, when a contact removes his avatar, for example).
// DONT DESTROY the bitmap handle passed in the struct avatarCacheEntry *
// 
// It is also possible that this event passes 0 as wParam (hContact), in which case,
// a protocol picture (pseudo - avatar) has been changed. 
static int AvatarChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	CONTACTAVATARCHANGEDNOTIFICATION* avatar = (CONTACTAVATARCHANGEDNOTIFICATION*)lParam;

	if (hContact == NULL)
	{
		ShowDebugPopup(NULL, TranslateT("AVH Debug"), TranslateT("Invalid contact/avatar... skipping"));
		return 0;
	}

	char *proto = GetContactProto((HANDLE)wParam);
	if (proto == NULL)
	{
		ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Invalid protocol... skipping"));
		return 0;
	}
	else if (metacontacts_proto != NULL && strcmp(metacontacts_proto, proto) == 0)
	{
		ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Ignoring metacontacts notification"));
		return 0;
	}

	DBVARIANT dbvOldHash = {0};
	bool ret = (DBGetContactSettingTString(hContact,MODULE_NAME,"AvatarHash",&dbvOldHash) == 0);

	if (avatar == NULL)
	{
		if (!ret || !_tcscmp(dbvOldHash.ptszVal, _T("-")))
		{
			//avoid duplicate "removed avatar" notifications
			//do not notify on an empty profile
			ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Removed avatar, no avatar before...skipping"));
			DBFreeVariant(&dbvOldHash);
			return 0;
		}
		SkinPlaySound("avatar_removed");

		// Is a flash avatar or avs could not load it
		DBWriteContactSettingTString(hContact, MODULE_NAME, "AvatarHash", _T("-"));

		if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS) && opts.popup_show_removed)
			ShowPopup(hContact, NULL, opts.popup_removed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
			HistoryEvents_AddToHistorySimple(hContact, EVENTTYPE_AVATAR_CHANGE, 1, DBEF_READ);
	}
	else
	{
		if(ret && !_tcscmp(dbvOldHash.ptszVal, avatar->hash)) 
		{
			// same avatar hash, skipping
			ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Hashes are the same... skipping"));
			DBFreeVariant(&dbvOldHash);
			return 0;
		}
		SkinPlaySound("avatar_changed");
		DBWriteContactSettingTString(hContact, "AvatarHistory", "AvatarHash", avatar->hash);

		TCHAR history_filename[MAX_PATH] = _T("");

		if (ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK))
		{
			if (!opts.log_store_as_hash)
			{
				if (opts.log_per_contact_folders)
				{
					GetOldStyleAvatarName(history_filename, hContact);
					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), history_filename);

					if (ServiceExists(MS_MC_GETMETACONTACT)) 
					{
						HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, wParam, 0);

						if (hMetaContact != NULL && ContactEnabled(hMetaContact, "LogToDisk", AVH_DEF_LOGTOHISTORY))
						{
							TCHAR filename[MAX_PATH] = _T("");

							GetOldStyleAvatarName(filename, hMetaContact);
							if (CopyImageFile(avatar->filename, filename))
								ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), filename);
							else
								ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), filename);
						}
					}
				}
			}
			else
			{
				// See if we already have the avatar
				TCHAR hash[128];
				lstrcpyn(hash, avatar->hash, sizeof(hash));
				ConvertToFilename(hash, sizeof(hash));

				TCHAR *file = GetCachedAvatar(proto, hash);

				if (file != NULL)
				{
					lstrcpyn(history_filename, file, MAX_REGS(history_filename));
					mir_free(file);
				}
				else
				{
					if (opts.log_keep_same_folder)
						GetHistoryFolder(history_filename);
					else
						GetProtocolFolder(history_filename, proto);

					mir_sntprintf(history_filename, MAX_REGS(history_filename), 
							_T("%s\\%s"), history_filename, hash);

					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), history_filename);
				}

				if (opts.log_per_contact_folders)
				{
					CreateOldStyleShortcut(hContact, history_filename);

					if (ServiceExists(MS_MC_GETMETACONTACT)) 
					{
						HANDLE hMetaContact = (HANDLE) CallService(MS_MC_GETMETACONTACT, wParam, 0);

						if (hMetaContact != NULL && ContactEnabled(hMetaContact, "LogToDisk", AVH_DEF_LOGTOHISTORY))
							CreateOldStyleShortcut(hMetaContact, history_filename);
					}
				}
			}
		}


		if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS) && opts.popup_show_changed)
			ShowPopup(hContact, NULL, opts.popup_changed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY))
		{
			TCHAR rel_path[MAX_PATH] = _T("");
			CallService(MS_UTILS_PATHTORELATIVET,(WPARAM)history_filename,(LPARAM)rel_path);
			char *blob = mir_utf8encodeT(rel_path);
			int flags = DBEF_READ | DBEF_UTF;
			if(ServiceExists(MS_HISTORYEVENTS_ADD_TO_HISTORY))
			{
				HistoryEvents_AddToHistoryEx(hContact, EVENTTYPE_AVATAR_CHANGE, 0, NULL, 0, (PBYTE) blob, (int) strlen(blob) + 1, flags);
			}
			else
			{
				DBEVENTINFO AvatarEvent = { 0 };
				AvatarEvent.cbSize = sizeof(AvatarEvent);
				AvatarEvent.szModule = GetContactProto(hContact);
				AvatarEvent.flags = flags;
				AvatarEvent.timestamp = (DWORD) time(NULL);
				AvatarEvent.eventType = EVENTTYPE_AVATAR_CHANGE;
				AvatarEvent.cbBlob = (DWORD) strlen(blob) + 1;
				AvatarEvent.pBlob = (PBYTE) blob;
				CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&AvatarEvent);
			}
			mir_free(blob);
		}
	}

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}


static INT_PTR IsEnabled(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	return ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK) 
		|| ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS)
		|| ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY);
}


/*
Get cached avatar

wParam: (char *) protocol name
lParam: (TCHAR *) hash 
return: (TCHAR *) NULL if none is found or the path to the avatar. You need to free this string 
        with mir_free.
*/
static INT_PTR GetCachedAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR hash[128];
	lstrcpyn(hash, (TCHAR *) lParam, sizeof(hash));
	ConvertToFilename(hash, sizeof(hash));
	return (INT_PTR) GetCachedAvatar((char *) wParam, hash);
}

TCHAR * GetCachedAvatar(char *proto, TCHAR *hash)
{
	TCHAR *ret = NULL;
	TCHAR file[1024] = _T("");
	TCHAR search[1024] = _T("");
	if (opts.log_keep_same_folder)
		GetHistoryFolder(file);
	else
		GetProtocolFolder(file, proto);

	mir_sntprintf(search, MAX_REGS(search), _T("%s\\%s.*"), file, hash);

	WIN32_FIND_DATA finddata;
	HANDLE hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return NULL;

	do
	{
		size_t len = lstrlen(finddata.cFileName);
		if (len > 4 
			&& (!lstrcmpi(&finddata.cFileName[len-4], _T(".png"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".bmp"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".gif"))
				|| !lstrcmpi(&finddata.cFileName[len-4], _T(".jpg"))
				|| !lstrcmpi(&finddata.cFileName[len-5], _T(".jpeg"))))
		{
			mir_sntprintf(file, MAX_REGS(file), _T("%s\\%s"), file, finddata.cFileName);
			ret = mir_tstrdup(file);
			break;
		}
	} while(FindNextFile(hFind, &finddata));
	FindClose(hFind);

	return ret;
}


int GetUIDFromHContact(HANDLE contact, TCHAR* uinout, size_t uinout_len)
{
	CONTACTINFO cinfo;

	ZeroMemory(&cinfo,sizeof(CONTACTINFO));
	cinfo.cbSize = sizeof(CONTACTINFO);
	cinfo.hContact = contact;
	cinfo.dwFlag = CNF_UNIQUEID | CNF_TCHAR;

	bool found = true;
	if(CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&cinfo)==0)
	{
		if(cinfo.type == CNFT_ASCIIZ)
		{
			lstrcpyn(uinout, cinfo.pszVal, uinout_len);
			// It is up to us to free the string
			// The catch? We need to use Miranda's free(), not our CRT's :)
			mir_free(cinfo.pszVal);
		}
		else if(cinfo.type == CNFT_DWORD)
		{
			_itot(cinfo.dVal,uinout,10);
		}
		else if(cinfo.type == CNFT_WORD)
		{
			_itot(cinfo.wVal,uinout,10);
		}
		else found = false;
	}
	else found = false;

	if (!found)
	{
		lstrcpyn(uinout, TranslateT("Unknown UIN"),uinout_len);
	}
	return 0;
}


TCHAR * GetHistoryFolder(TCHAR *fn)
{
	if (fn == NULL) return NULL;
	FoldersGetCustomPathT(hFolder, fn, MAX_PATH, basedir);
	if(!CreateDirectory(fn, NULL))
		ErrorExit(NULL,_T("GetHistoryFolder"));

	return fn;
}


TCHAR * GetProtocolFolder(TCHAR *fn, char *proto)
{
	GetHistoryFolder(fn);

	if (proto == NULL)
		proto = Translate("Unknown Protocol");

	mir_sntprintf(fn, MAX_PATH, _T("%s\\") _T(CS), fn, proto);
	if(!CreateDirectory(fn, NULL))
		ErrorExit(NULL,_T("CreateDirectory"));
	
	return fn;
}


TCHAR * GetContactFolder(TCHAR *fn, HANDLE hContact)
{
	TCHAR uin[MAX_PATH];

	char *proto = GetContactProto(hContact);
	GetProtocolFolder(fn, proto);
	
	GetUIDFromHContact(hContact, uin, MAX_REGS(uin));
	mir_sntprintf(fn, MAX_PATH, _T("%s\\%s"), fn, uin);
	if(!CreateDirectory(fn, NULL))
		ErrorExit(hContact,_T("CreateDirectory"));
	ConvertToFilename(uin, sizeof(uin)); //added so that weather id's like "yw/CI0000" work
	
#ifdef DBGPOPUPS
	TCHAR log[1024];
	mir_sntprintf(log, MAX_REGS(log), _T("Path: %s\nProto: ") _T(CS) _T("\nUIN: %s"), fn, proto, uin);
	ShowPopup(hContact, _T("AVH Debug: GetContactFolder"), log);
#endif

	return fn;
}

TCHAR * GetOldStyleAvatarName(TCHAR *fn, HANDLE hContact)
{
	GetContactFolder(fn, hContact);

	SYSTEMTIME curtime;
	GetLocalTime(&curtime);
	mir_sntprintf(fn, MAX_PATH, 
		_T("%s\\%04d-%02d-%02d %02dh%02dm%02ds"), fn, 
		curtime.wYear, curtime.wMonth, curtime.wDay, 
		curtime.wHour, curtime.wMinute, curtime.wSecond);
	ShowDebugPopup(hContact,TranslateT("AVH Debug: GetOldStyleAvatarName"),fn);
	return fn;
}

BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut)
{
	CoInitialize(NULL);

    IShellLink* psl = NULL;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

    if (SUCCEEDED(hr)) 
    {
        psl->SetPath(file); 

        IPersistFile* ppf = NULL; 
        hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 

        if (SUCCEEDED(hr))
        {
#ifdef _UNICODE
			hr = ppf->Save(shortcut, TRUE); 
#else
			WCHAR tmp[MAX_PATH]; 
            MultiByteToWideChar(CP_ACP, 0, shortcut, -1, tmp, MAX_PATH); 
            hr = ppf->Save(tmp, TRUE); 
#endif
            ppf->Release(); 
        }

        psl->Release(); 
    } 

	if(FAILED(hr))
		ErrorExit(NULL,_T("CreateShortcut"));
	return SUCCEEDED(hr);
}


BOOL ResolveShortcut(TCHAR *shortcut, TCHAR *file)
{
	CoInitialize(NULL);

    IShellLink* psl = NULL;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

    if (SUCCEEDED(hr)) 
    {
        IPersistFile* ppf = NULL; 
		hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 

        if (SUCCEEDED(hr))
		{
#ifdef _UNICODE
			hr = ppf->Load(shortcut, STGM_READ); 
#else
			WCHAR tmp[MAX_PATH]; 
			MultiByteToWideChar(CP_ACP, 0, shortcut, -1, tmp, MAX_PATH); 
			hr = ppf->Load(tmp, STGM_READ); 
#endif

			if (SUCCEEDED(hr))
			{
				hr = psl->Resolve(NULL, SLR_UPDATE); 

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = psl->GetPath(file, MAX_PATH, &wfd, SLGP_RAWPATH); 
				}
			}

            ppf->Release(); 
		}
        psl->Release(); 
    }

	if(FAILED(hr))
		ErrorExit(NULL,_T("CreateShortcut"));
	return SUCCEEDED(hr);
}


template<class T>
void ConvertToRTF(Buffer<char> *buffer, T *line)
{
	buffer->append("{\\uc1 ", 6);
	
	for (; *line; line++) 
	{
		if (*line == (T)'\r' && line[1] == (T)'\n') {
			buffer->append("\\line ", 6);
			line++;
		}
		else if (*line == (T)'\n') {
			buffer->append("\\line ", 6);
		}
		else if (*line == (T)'\t') {
			buffer->append("\\tab ", 5);
		}
		else if (*line == (T)'\\' || *line == (T)'{' || *line == (T)'}') {
			buffer->append('\\');
			buffer->append((char) *line);
		}
		else if (*line < 128) {
			buffer->append((char) *line);
		}
		else 
			buffer->appendPrintf("\\u%d ?", *line);
	}

	buffer->append('}');
}


void GetRTFFor(Buffer<char> *buffer, HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	BYTE *p = (BYTE *) malloc(dwLen);
	if (p == NULL)
		return;

	dwLen = GetBitmapBits(hBitmap, dwLen, p);

	buffer->appendPrintf("{\\pict\\wbitmap0\\wbmbitspixel%u\\wbmplanes%u\\wbmwidthbytes%u\\picw%u\\pich%u ", 
				bmp.bmBitsPixel, bmp.bmPlanes, bmp.bmWidthBytes, bmp.bmWidth, bmp.bmHeight);

	for (DWORD i = 0; i < dwLen; i++)
		buffer->appendPrintf("%02X", p[i]);

	buffer->append('}');


/*	
	BITMAPINFOHEADER bih = { 0 };
	HDC hdc = GetDC(NULL);
	GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, p, (BITMAPINFO *) & bih, DIB_RGB_COLORS);

	buffer->appendPrintf("{\\pict\\wbitmap0\\wbmbitspixel%u\\wbmplanes%u\\wbmwidthbytes%u\\picw%u\\pich%u ", 
				bmp.bmBitsPixel, bmp.bmPlanes, bmp.bmWidthBytes, bmp.bmWidth, bmp.bmHeight);

	DWORD i;
	for (i = 0; i < sizeof(BITMAPINFOHEADER); i++)
		buffer->appendPrintf("%02X", ((PBYTE) & bih)[i]);

	for (i = 0; i < dwLen; i++)
		buffer->appendPrintf("%02X", p[i]);

	buffer->append('}');
*/

	free(p);
}


void * GetHistoryEventText(HANDLE hContact, HANDLE hDbEvent, DBEVENTINFO *dbe, int format)
{
	void *ret;

	if (format & HISTORYEVENTS_FORMAT_CHAR)
	{
		ret = DbGetEventTextA(dbe, CP_ACP);
	}
	else if (format & HISTORYEVENTS_FORMAT_WCHAR)
	{
		ret = DbGetEventTextW(dbe, CP_ACP);
	}
	else if (format & HISTORYEVENTS_FORMAT_RICH_TEXT)
	{
		Buffer<char> buffer;

		TCHAR *tmp = DbGetEventTextT(dbe, CP_ACP);
		ConvertToRTF(&buffer, tmp);
		mir_free(tmp);

		// Load the image
		size_t i;
		for(i = dbe->cbBlob-2; i > 0 && dbe->pBlob[i] != 0; i--) ;
		i++;

		if (dbe->pBlob[i] != 0)
		{
			TCHAR absFile[MAX_PATH] = _T("");
			CallService(MS_UTILS_PATHTOABSOLUTET,(WPARAM) &dbe->pBlob[i], (LPARAM)absFile);
			if(absFile != NULL)
			{
				HBITMAP hBmp = (HBITMAP) CallService(MS_IMG_LOAD, (WPARAM) absFile, IMGL_TCHAR);

				if (hBmp != NULL)
				{
					buffer.append("\\line  ", 7);
					GetRTFFor(&buffer, hBmp);
					DeleteObject(hBmp);
				}
			}
		}

		buffer.append(' ');
		buffer.pack();
		ret = buffer.detach();
	}

	return ret;
}

