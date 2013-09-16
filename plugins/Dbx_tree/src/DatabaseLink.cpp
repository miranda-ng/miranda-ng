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

#include "Interface.h"
#include "DatabaseLink.h"

/*
	profile: pointer to a string which contains full path + name
	Affect: The database plugin should create the profile, the filepath will not exist at
		the time of this call, profile will be C:\..\<name>.dat
	Note: Do not prompt the user in anyway about this operation.
	Note: Do not initialise internal data structures at this point!
	Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_*
*/

static int makeDatabase(const TCHAR *profile)
{
	std::auto_ptr<CDataBase> db( new CDataBase(profile));
	return db->CreateDB();
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
static int grokHeader(const TCHAR *profile)
{
	std::auto_ptr<CDataBase> db( new CDataBase(profile));
	return db->CheckDB();
}

/*
Affect: Tell the database to create all services/hooks that a 3.xx legecy database might support into link,
	which is a PLUGINLINK structure
Returns: 0 on success, nonzero on failure
*/

static MIDatabase* LoadDatabase(const TCHAR *profile)
{
	CDataBase* pDb = new CDataBase(profile);
	pDb->OpenDB();
	return pDb;
}

/*
Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
Returns: 0 on success, nonzero on failure
Note: Unload() might be called even if Load(void) was never called, wasLoaded is set to 1 if Load(void) was ever called.
*/

static int UnloadDatabase(MIDatabase* db)
{
	delete (CDataBase*)db;
	return 0;
}

DATABASELINK gDBLink = {
	sizeof(DATABASELINK),
	__INTERNAL_NAME,
	_T("dbx tree driver"),
	makeDatabase,
	grokHeader,
	LoadDatabase,
	UnloadDatabase,
	NULL  // does not support file checking
};
