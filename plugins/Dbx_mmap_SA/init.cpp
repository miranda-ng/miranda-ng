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

void InitSecurity(void);
void UnloadSecurity(void);

int hLangpack;

HINSTANCE g_hInst = NULL;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	"Provides Miranda database support: global settings, contacts, history, settings per contact. Enhanced modification with Encryption support.",
	"Miranda-IM project, modification by FYR and chaos.persei, nullbie, Billy_Bons",
	"chaos.persei@gmail.com; ashpynov@gmail.com; bio@msx.ru; ghazan@miranda.im",
	"Copyright 2000-2011 Miranda IM project, FYR, chaos.persei, induction, nullbie",
	"http://dbmmapmod.googlecode.com/",
	UNICODE_AWARE,
	// {28FF9B91-3E4D-4f1c-B47C-C641B037FF40}
	{ 0x28ff9b91, 0x3e4d, 0x4f1c, { 0xb4, 0x7c, 0xc6, 0x41, 0xb0, 0x37, 0xff, 0x40 } }
};

LIST<CDdxMmapSA> g_Dbs(1, (LIST<CDdxMmapSA>::FTSortFunc)HandleKeySort);

/////////////////////////////////////////////////////////////////////////////////////////

// returns 0 if the profile is created, EMKPRF*
static int makeDatabase(const TCHAR *profile, int *error)
{
	CDdxMmapSA *tmp = new CDdxMmapSA(profile);
	if (tmp->Create() == ERROR_SUCCESS) {
		tmp->CreateDbHeaders();
		delete tmp;
		return 0;
	}
	delete tmp;
	if (error != NULL) *error = EMKPRF_CREATEFAILED;
	return 1;
}

// returns 0 if the given profile has a valid header
static int grokHeader(const TCHAR *profile, int *error)
{
	CDdxMmapSA *tmp = new CDdxMmapSA(profile);
	if (tmp->Load(true) != ERROR_SUCCESS) {
		delete tmp;
		if (error != NULL) *error = EGROKPRF_CANTREAD;
		return 1;
	}

	int chk = tmp->CheckDbHeaders();
	delete tmp;
	if ( chk == 0 ) {
		// all the internal tests passed, hurrah
		if (error != NULL) *error = 0;
		return 0;
	}
	
	// didn't pass at all, or some did.
	switch ( chk ) {
	case 1:
		// "Miranda ICQ DB" wasn't present
		if (error != NULL) *error = EGROKPRF_UNKHEADER;
		break;

	case 2:
		// header was present, but version information newer
		if (error != NULL) *error =  EGROKPRF_VERNEWER;
		break;

	case 3:
		// header/version OK, internal data missing
		if (error != NULL) *error = EGROKPRF_DAMAGED;
		break;
	}

	return 1;
}

// returns 0 if all the APIs are injected otherwise, 1
static MIDatabase* LoadDatabase(const TCHAR *profile)
{
	// set the memory, lists & UTF8 manager
	mir_getLP( &pluginInfo );

	CDdxMmapSA* db = new CDdxMmapSA(profile);
	if (db->Load(false) != ERROR_SUCCESS) {
		delete db;
		return NULL;
	}

	g_Dbs.insert(db);
	return db;
}

static int UnloadDatabase(MIDatabase* db)
{
	g_Dbs.remove((CDdxMmapSA*)db);
	delete (CDdxMmapSA*)db;
	return 0;
}

static DATABASELINK dblink =
{
	sizeof(DATABASELINK),
	"db3x secure mmap driver",
	_T("db3x secure mmap database support"),
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase
};

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_DATABASE, MIID_LAST};

extern "C" __declspec(dllexport) int Load(void)
{
	InitSecurity();
	InitPreset();

	RegisterDatabasePlugin(&dblink);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	g_Dbs.destroy();
	UnloadSecurity();
	UninitPreset();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	return TRUE;
}
