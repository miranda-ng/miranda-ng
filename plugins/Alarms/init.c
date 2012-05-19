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
#include "database.h"
#include <m_plugins.h>

struct MM_INTERFACE memoryManagerInterface;
extern char szDbPath[MAX_PATH];

HINSTANCE g_hInst=NULL;
PLUGINLINK *pluginLink;

static int getCapability( int flag )
{
	return 0;
}

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(char * profile, int * error) 
{
	HANDLE hFile=CreateFile(profile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
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

	hFile = CreateFile(profile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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
	// set the memory manager
	memoryManagerInterface.cbSize=sizeof(struct MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI,0,(LPARAM)&memoryManagerInterface);

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
	strncpy(buf,shortName ? "Miranda database" : "Miranda database support",cch);
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

static PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Miranda database - with AutoBackups",
	PLUGIN_MAKE_VERSION(0,5,2,1),
	"Provides Miranda database support: global settings, contacts, history, settings per contact.",
	"Miranda-IM project",
	"mail@scottellis.com.au",
	"Copyright 2000-2005 Miranda-IM project",
	"http://www.scottellis.com.au",
	0,
	DEFMOD_DB
};

__declspec(dllexport) PLUGININFO * MirandaPluginInfo(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION(0,4,0,0) ) return NULL;
	return &pluginInfo;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

int __declspec(dllexport) Unload(void)
{
	return 0;
}
