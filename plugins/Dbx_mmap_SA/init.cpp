/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

int hLangpack;

extern TCHAR szDbPath[MAX_PATH];

HINSTANCE g_hInst = NULL;

CDdxMmap* g_Db = NULL;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	"Provides Miranda database support: global settings, contacts, history, settings per contact. Enhanced modification with Encryption support.",
	"Miranda-IM project, modification by FYR and chaos.persei, nullbie, Billy_Bons",
	"chaos.persei@gmail.com; ashpynov@gmail.com; bio@msx.ru; ghazan@miranda-im.org",
	"Copyright 2000-2011 Miranda IM project, FYR, chaos.persei, induction, nullbie",
	"http://dbmmapmod.googlecode.com/",
	UNICODE_AWARE,
	// {28FF9B91-3E4D-4f1c-B47C-C641B037FF40}
	{ 0x28ff9b91, 0x3e4d, 0x4f1c, { 0xb4, 0x7c, 0xc6, 0x41, 0xb0, 0x37, 0xff, 0x40 } }
};

static int getCapability( int flag )
{
	return 0;
}

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(TCHAR *profile, int *error)
{
	HANDLE hFile = CreateFile(profile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		CreateDbHeaders(hFile);
		CloseHandle(hFile);
		return 0;
	}
	if ( error != NULL ) *error = EMKPRF_CREATEFAILED;
	return 1;
}

// returns 0 if the given profile has a valid header
static int grokHeader(TCHAR *profile, int *error )
{
	int rc = 1;
	int chk = 0;
	struct DBHeader hdr;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dummy = 0;

	hFile = CreateFile(profile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) {
		if ( error != NULL ) *error = EGROKPRF_CANTREAD;
		return 1;
	}
	// read the header, which can fail (for various reasons)
	if ( !ReadFile(hFile, &hdr, sizeof(struct DBHeader), &dummy, NULL)) {
		if ( error != NULL) *error = EGROKPRF_CANTREAD;
		CloseHandle(hFile);
		return 1;
	}
	chk = CheckDbHeaders(&hdr);
	if ( chk == 0 ) {
		// all the internal tests passed, hurrah
		rc = 0;
		if ( error != NULL ) *error = 0;
	} else {
		// didn't pass at all, or some did.
		switch ( chk ) {
			case 1:
			{
				// "Miranda ICQ DB" wasn't present
				if ( error != NULL ) *error = EGROKPRF_UNKHEADER;
				break;
			}
			case 2:
			{
				// header was present, but version information newer
				if ( error != NULL ) *error =  EGROKPRF_VERNEWER;
				break;
			}
			case 3:
			{
				// header/version OK, internal data missing
				if ( error != NULL ) *error = EGROKPRF_DAMAGED;
				break;
			}
		} // switch
	} //if
	CloseHandle(hFile);
	return rc;
}

// returns 0 if all the APIs are injected otherwise, 1
static MIDatabase* LoadDatabase(TCHAR *profile)
{
	if (g_Db) delete g_Db;
	g_Db = new CDdxMmap(profile);

	// don't need thread notifications
	_tcsncpy(szDbPath, profile, SIZEOF(szDbPath));

	// set the memory, lists & UTF8 manager
	mir_getLP( &pluginInfo );

	{ // Are we running under unicode Miranda core ?
		char szVer[MAX_PATH];
		CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
		_strlwr(szVer); // make sure it is lowercase

		/* WARNING!!! You CANNOT remove this code */
		if (strstr(szVer, "coffee") != NULL)
		{
			// We are running under damn violators
			void (*f)();

			MessageBox(0, TranslateT("Running mmap_sa is forbidden under license violating products, sorry"), TranslateT("Warning!"), MB_OK);

			f = NULL;
			f();
		}
		/* end of protected code */
	}

	// inject all APIs and hooks into the core
	LoadDatabaseModule();

	return g_Db;
}

static int UnloadDatabase(int wasLoaded)
{
	if ( !wasLoaded) return 0;
	UnloadDatabaseModule();
	return 0;
}

static int getFriendlyName( TCHAR* buf, size_t cch, int shortName )
{
	_tcsncpy(buf,shortName ? _T("db3x secured_mmap driver") : _T("db3x mmap database support"), cch);
	return 0;
}

static DATABASELINK dblink = {
	sizeof(DATABASELINK),
	getCapability,
	getFriendlyName,
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase,
};

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) DATABASELINK* DatabasePluginInfo(void * reserved)
{
	return &dblink;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_DATABASE, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	return 1;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}