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

struct MM_INTERFACE   mmi;
struct LIST_INTERFACE li;
struct UTF8_INTERFACE utfi;
int hLangpack;

extern char szDbPath[MAX_PATH];

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Miranda mmap database driver",
	__VERSION_DWORD,
	"Provides Miranda database support: global settings, contacts, history, settings per contact.",
	"Miranda-IM project",
	"bio@msx.ru; ghazan@miranda-im.org",
	"Copyright 2000-2011 Miranda IM project",
	"",
	UNICODE_AWARE,
	DEFMOD_DB,
    {0xf7a6b27c, 0x9d9c, 0x4a42, { 0xbe, 0x86, 0xa4, 0x48, 0xae, 0x10, 0x91, 0x61 }} //{F7A6B27C-9D9C-4a42-BE86-A448AE109161}
};

HINSTANCE g_hInst=NULL;
PLUGINLINK *pluginLink;

static int getCapability( int flag )
{
	return 0;
}

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(char * profile, int * error)
{
	HANDLE hFile = CreateFileA(profile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		CreateDbHeaders(hFile);
		CloseHandle(hFile);
		return 0;
	}
	if ( error != NULL ) *error=EMKPRF_CREATEFAILED;
	return 1;
}

// returns 0 if the given profile has a valid header
static int grokHeader( char * profile, int * error )
{
	int rc=1;
	int chk=0;
	struct DBHeader hdr;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dummy=0;

	hFile = CreateFileA(profile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( hFile == INVALID_HANDLE_VALUE ) {
		if ( error != NULL ) *error=EGROKPRF_CANTREAD;
		return 1;
	}
	// read the header, which can fail (for various reasons)
	if ( !ReadFile(hFile, &hdr, sizeof(struct DBHeader), &dummy, NULL) ) {
		if ( error != NULL) *error=EGROKPRF_CANTREAD;
		CloseHandle(hFile);
		return 1;
	}
	chk=CheckDbHeaders(&hdr);
	if ( chk == 0 ) {
		// all the internal tests passed, hurrah
		rc=0;
		if ( error != NULL ) *error=0;
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
				if ( error != NULL ) *error= EGROKPRF_VERNEWER;
				break;
			}
			case 3:
			{
				// header/version OK, internal data missing
				if ( error != NULL ) *error=EGROKPRF_DAMAGED;
				break;
			}
		} // switch
	} //if
	CloseHandle(hFile);
	return rc;
}

// returns 0 if all the APIs are injected otherwise, 1
static int LoadDatabase( char * profile, void * plink )
{
	PLUGINLINK *link = plink;
#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// don't need thread notifications
	strncpy(szDbPath, profile, sizeof(szDbPath));

	// this is like Load()'s pluginLink
	pluginLink=link;

	// set the memory, lists & UTF8 manager
	mir_getLI( &li );
	mir_getMMI( &mmi );
	mir_getUTFI( &utfi );
	mir_getLP( &pluginInfo );

	// inject all APIs and hooks into the core
	return LoadDatabaseModule();
}

static int UnloadDatabase(int wasLoaded)
{
	if ( !wasLoaded) return 0;
	UnloadDatabaseModule();
	return 0;
}

static int getFriendlyName( char * buf, size_t cch, int shortName )
{
	strncpy(buf,shortName ? "db3x mmap driver" : "db3x mmap database support",cch);
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
	g_hInst=hInstDLL;
	return TRUE;
}

__declspec(dllexport) DATABASELINK* DatabasePluginInfo(void * reserved)
{
	return &dblink;
}

__declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if ( mirandaVersion < MIRANDA_VERSION_CORE ) {
		MessageBox( NULL, _T("The db3x_mmap plugin cannot be loaded. It requires Miranda IM 0.7.0.0 or later."), _T("db3x_mmap Plugin"), MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
		return NULL;
	}
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_DATABASE, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}
