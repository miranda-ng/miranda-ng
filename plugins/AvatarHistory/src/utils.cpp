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

BOOL ProtocolEnabled(const char *proto)
{
	if (proto == NULL)
		return FALSE;

	char setting[256];
	mir_snprintf(setting, SIZEOF(setting), "%sEnabled", proto);
	return (BOOL) db_get_b(NULL, MODULE_NAME, setting, TRUE);
}

BOOL ContactEnabled(MCONTACT hContact, char *setting, int def)
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
	for (int nIndex = 0; nIndex < nSize; nIndex++)
		if (pBuffer[nIndex] > 0x7F)
			return FALSE;

	return TRUE;
}

void ConvertToFilename(TCHAR *str, size_t size)
{
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
				str[i] = '_';
		}
	}
}

int GetUIDFromHContact(MCONTACT contact, TCHAR* uinout, int uinout_len)
{
	bool found = true;

	CONTACTINFO cinfo = { sizeof(cinfo) };
	cinfo.hContact = contact;
	cinfo.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
	if (CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&cinfo) == 0) {
		if (cinfo.type == CNFT_ASCIIZ) {
			mir_tstrncpy(uinout, cinfo.pszVal, uinout_len);
			// It is up to us to free the string
			// The catch? We need to use Miranda's free(), not our CRT's :)
			mir_free(cinfo.pszVal);
		}
		else if (cinfo.type == CNFT_DWORD)
			_itot(cinfo.dVal,uinout,10);
		else if (cinfo.type == CNFT_WORD)
			_itot(cinfo.wVal,uinout,10);
		else
			found = false;
	}
	else found = false;

	if (!found)
		mir_tstrncpy(uinout, TranslateT("Unknown UIN"), uinout_len);

	return 0;
}

TCHAR* GetExtension(TCHAR *file)
{
	if (file == NULL) return _T("");
	TCHAR *ext = _tcsrchr(file, _T('.'));
	if (ext != NULL)
		ext++;
	else
		ext = _T("");

	return ext;
}

TCHAR* GetHistoryFolder(TCHAR *fn)
{
	if (fn == NULL) return NULL;
	FoldersGetCustomPathT(hFolder, fn, MAX_PATH, basedir);
	CreateDirectoryTreeT(fn);
	return fn;
}

TCHAR* GetProtocolFolder(TCHAR *fn, char *proto)
{
	GetHistoryFolder(fn);

	if (proto == NULL)
		proto = Translate("Unknown Protocol");

	mir_sntprintf(fn, MAX_PATH, _T("%s\\%S"), fn, proto);
	CreateDirectoryTreeT(fn);
	return fn;
}

TCHAR* GetContactFolder(TCHAR *fn, MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	GetProtocolFolder(fn, proto);
	
	TCHAR uin[MAX_PATH];
	GetUIDFromHContact(hContact, uin, SIZEOF(uin));
	ConvertToFilename(uin, MAX_PATH); //added so that weather id's like "yw/CI0000" work
	mir_sntprintf(fn, MAX_PATH, _T("%s\\%s"), fn, uin);
	CreateDirectoryTreeT(fn);
	
#ifdef DBGPOPUPS
	TCHAR log[1024];
	mir_sntprintf(log, MAX_REGS(log), _T("Path: %s\nProto: %S\nUIN: %s"), fn, proto, uin);
	ShowPopup(hContact, _T("AVH Debug: GetContactFolder"), log);
#endif

	return fn;
}

TCHAR* GetOldStyleAvatarName(TCHAR *fn, MCONTACT hContact)
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

void CreateOldStyleShortcut(MCONTACT hContact, TCHAR *history_filename)
{
	TCHAR shortcut[MAX_PATH] = _T("");

	GetOldStyleAvatarName(shortcut, hContact);

	mir_sntprintf(shortcut, SIZEOF(shortcut), _T("%s.%s.lnk"), shortcut,
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
	return !CopyFile(old_file, new_file, TRUE);
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

	mir_sntprintf(search, SIZEOF(search), _T("%s\\%s.*"), file, hash);

	WIN32_FIND_DATA finddata;
	HANDLE hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return NULL;

	do
	{
		size_t len = mir_tstrlen(finddata.cFileName);
		if (len > 4 
			&& (!mir_tstrcmpi(&finddata.cFileName[len-4], _T(".png"))
				|| !mir_tstrcmpi(&finddata.cFileName[len-4], _T(".bmp"))
				|| !mir_tstrcmpi(&finddata.cFileName[len-4], _T(".gif"))
				|| !mir_tstrcmpi(&finddata.cFileName[len-4], _T(".jpg"))
				|| !mir_tstrcmpi(&finddata.cFileName[len-5], _T(".jpeg"))))
		{
			mir_sntprintf(file, SIZEOF(file), _T("%s\\%s"), file, finddata.cFileName);
			ret = mir_tstrdup(file);
			break;
		}
	} while(FindNextFile(hFind, &finddata));
	FindClose(hFind);

	return ret;
}

BOOL CreateShortcut(TCHAR *file, TCHAR *shortcut)
{
	IShellLink *psl = NULL;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);
	if (SUCCEEDED(hr)) {
		psl->SetPath(file); 

		IPersistFile *ppf = NULL; 
		hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 
		if (SUCCEEDED(hr)) {
			hr = ppf->Save(shortcut, TRUE); 
			ppf->Release(); 
		}

		psl->Release(); 
	} 

	return SUCCEEDED(hr);
}

BOOL ResolveShortcut(TCHAR *shortcut, TCHAR *file)
{
	IShellLink* psl = NULL;

	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

	if (SUCCEEDED(hr)) {
		IPersistFile* ppf = NULL; 
		hr = psl->QueryInterface(IID_IPersistFile,  (void **) &ppf); 

		if (SUCCEEDED(hr)) {
			hr = ppf->Load(shortcut, STGM_READ); 
			if (SUCCEEDED(hr)) {
				hr = psl->Resolve(NULL, SLR_UPDATE); 
				if (SUCCEEDED(hr)) {
					WIN32_FIND_DATA wfd;
					hr = psl->GetPath(file, MAX_PATH, &wfd, SLGP_RAWPATH); 
				}
			}

			ppf->Release(); 
		}
		psl->Release(); 
	}

	return SUCCEEDED(hr);
}
