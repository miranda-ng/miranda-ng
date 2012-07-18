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

typedef struct {
	char *name;
	DWORD ofs;
}  ModuleName;

HANDLE hModHeap = NULL;
static SortedList lMods, lOfs;

static int ModCompare( ModuleName *mn1, ModuleName *mn2 )
{
	return strcmp( mn1->name, mn2->name );
}

static int OfsCompare( ModuleName *mn1, ModuleName *mn2 )
{
	return ( mn1->ofs - mn2->ofs );
}

void AddToList(char *name, DWORD len, DWORD ofs)
{
	int index;
	ModuleName *mn = (ModuleName*)HeapAlloc(hModHeap,0,sizeof(ModuleName));
	mn->name = name;
	mn->ofs = ofs;

	if (List_GetIndex(&lMods,mn,&index))
		DatabaseCorruption( _T("%s (Module Name not unique)"));

	List_Insert(&lMods,mn,index);

	if (List_GetIndex(&lOfs,mn,&index))
		DatabaseCorruption( _T("%s (Module Offset not unique)"));

	List_Insert(&lOfs,mn,index);
}


int InitModuleNames(void)
{
	struct DBModuleName *dbmn;
	DWORD ofsThis;
	int nameLen;
	char *mod;

	hModHeap = HeapCreate(0, 0, 0);
	lMods.sortFunc = (FSortFunc)ModCompare;
	lMods.increment = 50;
	lOfs.sortFunc = (FSortFunc)OfsCompare;
	lOfs.increment = 50;

	ofsThis = dbHeader.ofsFirstModuleName;
	dbmn = (struct DBModuleName*)DBRead(ofsThis,sizeof(struct DBModuleName),NULL);
	while(ofsThis) {
		if (dbmn->signature!=DBMODULENAME_SIGNATURE) DatabaseCorruption(NULL);

		nameLen = dbmn->cbName;

		mod = (char*)HeapAlloc(hModHeap,0,nameLen+1);
		CopyMemory(mod,DBRead(ofsThis+offsetof(struct DBModuleName,name),nameLen,NULL),nameLen);
		mod[nameLen] = 0;

		AddToList(mod, nameLen, ofsThis);

		ofsThis = dbmn->ofsNext;
		dbmn = (struct DBModuleName*)DBRead(ofsThis,sizeof(struct DBModuleName),NULL);
	}
	return 0;
}

void UninitModuleNames(void)
{
	HeapDestroy(hModHeap);
	List_Destroy(&lMods);
	List_Destroy(&lOfs);
}

static DWORD FindExistingModuleNameOfs(const char *szName)
{
	static ModuleName *lastmn = NULL;
	ModuleName mn, *pmn;
	int index;

	mn.name = (char*)szName;
	mn.ofs = 0;

	if (lastmn && ModCompare(&mn,lastmn) == 0)
		return lastmn->ofs;

	if (List_GetIndex(&lMods,&mn,&index)) {
		pmn = (ModuleName*)lMods.items[index];
		lastmn = pmn;
		return pmn->ofs;
	}

	return 0;
}

//will create the offset if it needs to
DWORD GetModuleNameOfs(const char *szName)
{
	struct DBModuleName dbmn;
	int nameLen;
	DWORD ofsNew,ofsExisting;
	char *mod;

	ofsExisting = FindExistingModuleNameOfs(szName);
	if (ofsExisting) return ofsExisting;

	nameLen = (int)strlen(szName);

	//need to create the module name
	ofsNew = CreateNewSpace(nameLen+offsetof(struct DBModuleName,name));
	dbmn.signature = DBMODULENAME_SIGNATURE;
	dbmn.cbName = nameLen;
	dbmn.ofsNext = dbHeader.ofsFirstModuleName;
	dbHeader.ofsFirstModuleName = ofsNew;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBWrite(ofsNew,&dbmn,offsetof(struct DBModuleName,name));
	DBWrite(ofsNew+offsetof(struct DBModuleName,name),(PVOID)szName,nameLen);
	DBFlush(0);

	//add to cache
	mod = (char*)HeapAlloc(hModHeap,0,nameLen+1);
	strcpy(mod,szName);
	AddToList(mod, nameLen, ofsNew);

	//quit
	return ofsNew;
}

char *GetModuleNameByOfs(DWORD ofs)
{
	static ModuleName *lastmn = NULL;
	ModuleName mn, *pmn;
	int index;

	if (lastmn && lastmn->ofs == ofs)
		return lastmn->name;

	mn.name = NULL;
	mn.ofs = ofs;

	if (List_GetIndex(&lOfs,&mn,&index)) {
		pmn = (ModuleName*)lOfs.items[index];
		lastmn = pmn;
		return pmn->name;
	}

	DatabaseCorruption(NULL);
	return NULL;
}

STDMETHODIMP_(BOOL) CDdxMmap::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	int ret;
	ModuleName *pmn;
	for(int i = 0; i < lMods.realCount; i++) {
		pmn = (ModuleName *)lMods.items[i];
		ret = pFunc(pmn->name, pmn->ofs, (LPARAM)pParam);
		if (ret)
			return ret;
	}
	return 0;
}
