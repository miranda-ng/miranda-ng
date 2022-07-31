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

#include "stdafx.h"

bool ProtocolEnabled(const char *proto)
{
	return Proto_GetAccount(proto)->IsEnabled();
}

bool ContactEnabled(MCONTACT hContact, char *setting, int def)
{
	if (hContact == NULL)
		return false;

	char *proto = Proto_GetBaseAccountName(hContact);
	if (!ProtocolEnabled(proto))
		return false;

	uint8_t globpref = g_plugin.getByte(setting, def);
	uint8_t userpref = g_plugin.getByte(hContact, setting, BST_INDETERMINATE);

	return (globpref && userpref == BST_INDETERMINATE) || userpref == BST_CHECKED;
}

// Returns true if the unicode buffer only contains 7-bit characters.
BOOL IsUnicodeAscii(const wchar_t * pBuffer, int nSize)
{
	for (int nIndex = 0; nIndex < nSize; nIndex++)
		if (pBuffer[nIndex] > 0x7F)
			return FALSE;

	return TRUE;
}

void ConvertToFilename(wchar_t *str, size_t size)
{
	for (size_t i = 0; i < size && str[i] != '\0'; i++) {
		switch (str[i]) {
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

wchar_t* GetExtension(wchar_t *file)
{
	if (file == nullptr) return L"";
	wchar_t *ext = wcsrchr(file, '.');
	if (ext != nullptr)
		ext++;
	else
		ext = L"";

	return ext;
}

wchar_t* GetHistoryFolder(wchar_t *fn)
{
	if (fn == nullptr) return nullptr;
	FoldersGetCustomPathW(hFolder, fn, MAX_PATH, basedir);
	CreateDirectoryTreeW(fn);
	return fn;
}

wchar_t* GetProtocolFolder(wchar_t *fn, char *proto)
{
	GetHistoryFolder(fn);

	if (proto == nullptr)
		proto = Translate("Unknown protocol");

	mir_snwprintf(fn, MAX_PATH, L"%s\\%S", fn, proto);
	CreateDirectoryTreeW(fn);
	return fn;
}

wchar_t* GetContactFolder(wchar_t *fn, MCONTACT hContact)
{
	char *proto = Proto_GetBaseAccountName(hContact);
	GetProtocolFolder(fn, proto);

	wchar_t uin[MAX_PATH];
	ptrW id(Contact::GetInfo(CNF_UNIQUEID, hContact, proto));
	wcsncpy_s(uin, (id == NULL) ? TranslateT("Unknown UIN") : id, _TRUNCATE);
	ConvertToFilename(uin, MAX_PATH); //added so that weather id's like "yw/CI0000" work
	mir_snwprintf(fn, MAX_PATH, L"%s\\%s", fn, uin);
	CreateDirectoryTreeW(fn);

#ifdef DBGPOPUPS
	wchar_t log[1024];
	mir_snwprintf(log, L"Path: %s\nProto: %S\nUIN: %s", fn, proto, uin);
	ShowPopup(hContact, L"AVH Debug: GetContactFolder", log);
#endif

	return fn;
}

wchar_t* GetOldStyleAvatarName(wchar_t *fn, MCONTACT hContact)
{
	GetContactFolder(fn, hContact);

	SYSTEMTIME curtime;
	GetLocalTime(&curtime);
	mir_snwprintf(fn, MAX_PATH,
		L"%s\\%04d-%02d-%02d %02dh%02dm%02ds", fn,
		curtime.wYear, curtime.wMonth, curtime.wDay,
		curtime.wHour, curtime.wMinute, curtime.wSecond);
	ShowDebugPopup(hContact, L"AVH Debug: GetOldStyleAvatarName", fn);
	return fn;
}

void CreateOldStyleShortcut(MCONTACT hContact, wchar_t *history_filename)
{
	wchar_t shortcut[MAX_PATH] = L"";

	GetOldStyleAvatarName(shortcut, hContact);

	mir_snwprintf(shortcut, L"%s.%s.lnk", shortcut,
		GetExtension(history_filename));

	if (!CreateShortcut(history_filename, shortcut))
		ShowPopup(hContact, TranslateT("Avatar history: Unable to create shortcut"), shortcut);
	else
		ShowDebugPopup(hContact, L"AVH Debug: Shortcut created successfully", shortcut);
}

BOOL CopyImageFile(wchar_t *old_file, wchar_t *new_file)
{
	wchar_t *ext = GetExtension(old_file);
	mir_snwprintf(new_file, MAX_PATH, L"%s.%s", new_file, ext);
	return !CopyFile(old_file, new_file, TRUE);
}

wchar_t* GetCachedAvatar(char *proto, wchar_t *hash)
{
	wchar_t *ret = nullptr;
	wchar_t file[1024] = L"";
	wchar_t search[1024] = L"";
	if (opts.log_keep_same_folder)
		GetHistoryFolder(file);
	else
		GetProtocolFolder(file, proto);

	mir_snwprintf(search, L"%s\\%s.*", file, hash);

	WIN32_FIND_DATA finddata;
	HANDLE hFind = FindFirstFile(search, &finddata);
	if (hFind == INVALID_HANDLE_VALUE)
		return nullptr;

	do {
		size_t len = mir_wstrlen(finddata.cFileName);
		if (len > 4
			&& (!mir_wstrcmpi(&finddata.cFileName[len - 4], L".png")
				|| !mir_wstrcmpi(&finddata.cFileName[len - 4], L".bmp")
				|| !mir_wstrcmpi(&finddata.cFileName[len - 4], L".gif")
				|| !mir_wstrcmpi(&finddata.cFileName[len - 4], L".jpg")
				|| !mir_wstrcmpi(&finddata.cFileName[len - 5], L".jpeg"))) {
			mir_snwprintf(file, L"%s\\%s", file, finddata.cFileName);
			ret = mir_wstrdup(file);
			break;
		}
	} while (FindNextFile(hFind, &finddata));
	FindClose(hFind);

	return ret;
}

BOOL CreateShortcut(wchar_t *file, wchar_t *shortcut)
{
	IShellLink *psl = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if (SUCCEEDED(hr)) {
		psl->SetPath(file);

		IPersistFile *ppf = nullptr;
		hr = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
		if (SUCCEEDED(hr)) {
			hr = ppf->Save(shortcut, TRUE);
			ppf->Release();
		}

		psl->Release();
	}

	return SUCCEEDED(hr);
}

BOOL ResolveShortcut(wchar_t *shortcut, wchar_t *file)
{
	IShellLink* psl = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);

	if (SUCCEEDED(hr)) {
		IPersistFile* ppf = nullptr;
		hr = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);

		if (SUCCEEDED(hr)) {
			hr = ppf->Load(shortcut, STGM_READ);
			if (SUCCEEDED(hr)) {
				hr = psl->Resolve(nullptr, SLR_UPDATE);
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
