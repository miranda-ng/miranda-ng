/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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

#include <m_db_int.h>

#include "..\..\Db3x_mmap\src\database.h"
#include "..\..\Db3x_mmap\src\dbintf.h"

#define CACHESECTIONSIZE   4096
#define CACHESECTIONCOUNT  32

#define SETTINGSGROUPOFSCOUNT    32

struct CDb3x : public CDb3Base
{
	CDb3x(const TCHAR* tszFileName);
	~CDb3x();

	__inline HANDLE getFile() const { return m_hDbFile; }

protected:
	virtual	DWORD GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc,DWORD ofsContact,DWORD ofsModuleName);
	virtual	void InvalidateSettingsGroupOfsCacheEntry(DWORD ofsSettingsGroup);

	virtual	void  DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes);
	virtual	PBYTE DBRead(DWORD ofs, int bytesRequired, int *bytesAvail);
	virtual	void  DBWrite(DWORD ofs, PVOID pData, int bytes);
	virtual	void  DBFill(DWORD ofs, int bytes);
	virtual	void  DBFlush(int setting);
	virtual	int   InitCache(void);

	PBYTE m_pDbCache;
	DWORD m_lastUseCounter;

	struct DBCacheSectionInfo
	{
		DWORD ofsBase;
		DWORD lastUsed;
	} 
	cacheSectionInfo[CACHESECTIONCOUNT];

	int nextSGOCacheEntry;
	struct SettingsGroupOfsCacheEntry {
		DWORD ofsContact;
		DWORD ofsModuleName;
		DWORD ofsSettingsGroup;
	}
	settingsGroupOfsCache[SETTINGSGROUPOFSCOUNT];

	int  FindSectionForOffset(const DWORD ofs);
	int  FindLRUSection(void);
	void LoadSection(const int i,DWORD ofs);
	void MoveSection(int *sectId,int dest);
};
