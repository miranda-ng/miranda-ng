/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

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

#include "DatabaseLink.h"
#ifndef _MSC_VER
#include "savestrings_gcc.h"
#endif

static int getCapability(int);
static int getFriendlyName(char*, size_t, int);
static int makeDatabase(char*, int*);
static int grokHeader(char*, int*);
static int Load(char*, void*);
static int Unload(int);

DATABASELINK gDBLink = {
	sizeof(DATABASELINK),
	getCapability,
	getFriendlyName,
	makeDatabase,
	grokHeader,
	Load,
	Unload,
};

PLUGINLINK *pluginLink = NULL;
MM_INTERFACE mmi = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
UTF8_INTERFACE utfi = {0,0,0,0,0,0,0};
HANDLE hSystemModulesLoaded = 0;


static int SystemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	Update upd = {0,0,0,0,0,0,0,0,0,0,0,0,0};

	upd.cbSize = sizeof(upd);
	upd.szComponentName = gInternalName;
	upd.szBetaVersionURL = "http://www-user.tu-chemnitz.de/~kunmi/?dbx_tree";
#ifdef _M_X64
	upd.pbBetaVersionPrefix = (BYTE *)"<!-- Updater Beta x64: ";
	upd.cpbBetaVersionPrefix = 23;
	upd.szBetaUpdateURL = "http://www-user.tu-chemnitz.de/~kunmi/Downloads/dbx_tree64.zip";
#else
	upd.pbBetaVersionPrefix = (BYTE *)"<!-- Updater Beta: ";
	upd.cpbBetaVersionPrefix = 19;
	upd.szBetaUpdateURL = "http://www-user.tu-chemnitz.de/~kunmi/Downloads/dbx_tree.zip";
#endif
	upd.pbVersion = (BYTE*)gResVersionString;
	upd.cpbVersion = sizeof(gResVersionString) - 1;
	upd.szBetaChangelogURL = "http://www-user.tu-chemnitz.de/~kunmi/?dbx_tree=BetaLog&lang=en";

	CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);

	UnhookEvent(hSystemModulesLoaded);
	hSystemModulesLoaded = 0;

	return 0;
}

/*
returns what the driver can do given the flag
*/
static int getCapability(int flag)
{
	return 0;
}

/*
	buf: pointer to a string buffer
	cch: length of buffer
	shortName: if true, the driver should return a short but descriptive name, e.g. "3.xx profile"
	Affect: The database plugin must return a "friendly name" into buf and not exceed cch bytes,
	e.g. "Database driver for 3.xx profiles"
	Returns: 0 on success, non zero on failure
*/

static int getFriendlyName(char* buf, size_t cch, int shortName)
{
	if (shortName)
		strncpy_s(buf, cch, gInternalName, strlen(gInternalName));
	else
		strncpy_s(buf, cch, gInternalNameLong, strlen(gInternalNameLong));
	return 0;
}

/*
	profile: pointer to a string which contains full path + name
	Affect: The database plugin should create the profile, the filepath will not exist at
		the time of this call, profile will be C:\..\<name>.dat
	Note: Do not prompt the user in anyway about this operation.
	Note: Do not initialise internal data structures at this point!
	Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_*
*/
static int makeDatabase(char* profile, int* error)
{
	if (gDataBase) delete gDataBase;
  gDataBase = new CDataBase(profile);

	*error = gDataBase->CreateDB();
	return *error;
}

/*
	profile: [in] a null terminated string to file path of selected profile
	error: [in/out] pointer to an int to set with error if any
	Affect: Ask the database plugin if it supports the given profile, if it does it will
		return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_* can be valid error
		condition, most common error would be [EGROKPRF_UNKHEADER]
	Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
		etc.
	Returns: 0 on success, non zero on failure
*/
static int grokHeader(char* profile, int* error)
{
	if (gDataBase) delete gDataBase;
	gDataBase = new CDataBase(profile);

	*error = gDataBase->CheckDB();
	return *error;
}

/*
Affect: Tell the database to create all services/hooks that a 3.xx legecy database might support into link,
	which is a PLUGINLINK structure
Returns: 0 on success, nonzero on failure
*/
static int Load(char* profile, void* link)
{
	if (gDataBase) delete gDataBase;
	gDataBase = new CDataBase(profile);

	pluginLink = (PLUGINLINK*)link;

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);

	RegisterServices();
	CompatibilityRegister();

	hSystemModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, SystemModulesLoaded);

	return gDataBase->OpenDB();
}

/*
Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
Returns: 0 on success, nonzero on failure
Note: Unload() might be called even if Load() was never called, wasLoaded is set to 1 if Load() was ever called.
*/
static int Unload(int wasLoaded)
{
	if (gDataBase)
		delete gDataBase;

	gDataBase = NULL;
	return 0;
}


