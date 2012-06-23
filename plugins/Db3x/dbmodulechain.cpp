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

extern struct DBHeader dbHeader;

static INT_PTR EnumModuleNames(WPARAM wParam,LPARAM lParam);

struct ModuleName {
	char *name;
	DWORD hash;
	DWORD ofs;
};
static struct ModuleName *moduleName;
static int moduleNameCount;

int InitModuleNames(void)
{
	struct DBModuleName *dbmn;
	DWORD ofsThis,ofsNext;
	int nameLen;

	moduleNameCount=0;
	moduleName=NULL;
	ofsThis=dbHeader.ofsFirstModuleName;
	dbmn=(struct DBModuleName*)DBRead(ofsThis,sizeof(struct DBModuleName),NULL);
	while(ofsThis) {
		if(dbmn->signature!=DBMODULENAME_SIGNATURE) DatabaseCorruption();
		moduleName=(struct ModuleName*)mir_realloc(moduleName,sizeof(struct ModuleName)*(moduleNameCount+1));
		moduleName[moduleNameCount].ofs=ofsThis;
		moduleName[moduleNameCount].hash=dbmn->cbName;    //very very simple hash so far
		moduleName[moduleNameCount].name=(char*)mir_alloc(dbmn->cbName+1);
		ofsNext=dbmn->ofsNext;
		nameLen=dbmn->cbName;
		CopyMemory(moduleName[moduleNameCount].name,DBRead(ofsThis+offsetof(struct DBModuleName,name),nameLen,NULL),nameLen);
		moduleName[moduleNameCount].name[nameLen]=0;
		moduleNameCount++;
		ofsThis=ofsNext;
		dbmn=(struct DBModuleName*)DBRead(ofsThis,sizeof(struct DBModuleName),NULL);
	}
	CreateServiceFunction(MS_DB_MODULES_ENUM,EnumModuleNames);
	return 0;
}

void UninitModuleNames(void)
{
	int i;
	for(i=0;i<moduleNameCount;i++) mir_free(moduleName[i].name);
	if(moduleNameCount) mir_free(moduleName);
}

static DWORD FindExistingModuleNameOfs(const char *szName,int nameLen)
{
	int i;
	for(i=0;i<moduleNameCount;i++)
		if(moduleName[i].hash==(DWORD)nameLen && !strcmp(moduleName[i].name,szName)) return moduleName[i].ofs;
	return 0;
}

//will create the offset if it needs to
DWORD GetModuleNameOfs(const char *szName)
{
	struct DBModuleName dbmn;
	int nameLen=(int)strlen(szName);
	DWORD ofsNew,ofsExisting;

	ofsExisting=FindExistingModuleNameOfs(szName,nameLen);
	if(ofsExisting) return ofsExisting;
	//need to create the module name
	ofsNew=CreateNewSpace(nameLen+offsetof(struct DBModuleName,name));
	dbmn.signature=DBMODULENAME_SIGNATURE;
	dbmn.cbName=nameLen;
	dbmn.ofsNext=dbHeader.ofsFirstModuleName;
	dbHeader.ofsFirstModuleName=ofsNew;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBWrite(ofsNew,&dbmn,offsetof(struct DBModuleName,name));
	DBWrite(ofsNew+offsetof(struct DBModuleName,name),(PVOID)szName,nameLen);
	DBFlush(0);
	//add to cache
	moduleName=(struct ModuleName*)mir_realloc(moduleName,sizeof(struct ModuleName)*(moduleNameCount+1));
	moduleName[moduleNameCount].ofs=ofsNew;
	moduleName[moduleNameCount].hash=nameLen;    //very very simple hash so far
	moduleName[moduleNameCount].name=(char*)mir_alloc(nameLen+1);
	strcpy(moduleName[moduleNameCount].name,szName);
	moduleNameCount++;
	//quit
	return ofsNew;
}

//it's OK that this is a bit slow - it's rarely used
char *GetModuleNameByOfs(DWORD ofs)
{
	int i;

	for(i=0;i<moduleNameCount;i++)
		if(moduleName[i].ofs==ofs) return moduleName[i].name;
	DatabaseCorruption();
	return NULL;
} 

static INT_PTR EnumModuleNames(WPARAM wParam,LPARAM lParam)
{
	int i;
	INT_PTR ret;
	for(i=0;i<moduleNameCount;i++) {
		ret=((DBMODULEENUMPROC)lParam)(moduleName[i].name,moduleName[i].ofs,wParam);
		if (ret) return ret;
	}
	return 0;
}
