/*

'Language Pack Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

int hLangpack;
HINSTANCE hInst;

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), 
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE, 
	// {D4BDD1EB-56F1-4A87-A187-67246EE919A2}
	{0xd4bdd1eb, 0x56f1, 0x4a87, {0xa1, 0x87, 0x67, 0x24, 0x6e, 0xe9, 0x19, 0xa2}}, 
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, void*)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP( &pluginInfo );

	//if (!IsRunAsAdmin())
	MovePacks(_T("langpack_*.txt"));

	/*INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_TREEVIEW_CLASSES|ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);*/

	/* menu item */
	CLISTMENUITEM mi = { sizeof(mi) }; 
	mi.position = 2000089999; 
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_RELOAD)); 
	mi.pszName = LPGEN("Reload langpack"); 
	mi.pszService = MS_LANGPACK_RELOAD; 
	Menu_AddMainMenuItem(&mi); 

	/* reset langpack */
	mir_ptr<TCHAR> langpack(db_get_tsa(NULL, "LangMan", "Langpack"));
	if (langpack)
	{
		TCHAR szPath[MAX_PATH];
		GetPackPath(szPath, SIZEOF(szPath), FALSE, langpack);

		DWORD dwAttrib = GetFileAttributes(szPath);
		if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
			CallService(MS_LANGPACK_RELOAD, 0, (LPARAM)szPath);
	}

	InitOptions();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitOptions();

	/* move default langpack to root */
	mir_ptr<TCHAR> langpack(db_get_tsa(NULL, "LangMan", "Langpack"));
	if (langpack)
	{
		TCHAR szFrom[MAX_PATH], szDest[MAX_PATH];
		GetPackPath(szFrom, SIZEOF(szFrom), FALSE, langpack);

		DWORD dwAttrib = GetFileAttributes(szFrom);
		if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			GetPackPath(szDest, SIZEOF(szDest), TRUE, langpack);
			if (!MoveFile(szFrom, szDest) && GetLastError() == ERROR_ALREADY_EXISTS) {
				DeleteFile(szDest);
				MoveFile(szFrom, szDest);
			}
		}
	}

	return 0;
}