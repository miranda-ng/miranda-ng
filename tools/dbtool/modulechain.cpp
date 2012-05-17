/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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
#include "dbtool.h"

struct ModChainEntry {
	DWORD ofsOld,ofsNew;
	int size;
	char name[257];
} static *modChain=NULL;
static int modChainCount;
static DWORD ofsCurrent;
static int phase,iCurrentModName;
static DWORD ofsLast;
static int last_mod = 0;

int WorkModuleChain(int firstTime)
{
	DBModuleName moduleName,*newModName;

	if(firstTime) {
		AddToStatus(STATUS_MESSAGE,TranslateT("Processing module name chain"));
		modChainCount=0;
		last_mod = 0;
		if(modChain!=NULL) free(modChain);
		modChain = (ModChainEntry*)malloc(sizeof(ModChainEntry));
		phase=0;
		ofsCurrent=dbhdr.ofsFirstModuleName;
	}
	switch(phase) {
		case 0:
			if(ofsCurrent==0) {
				phase++;
				return ERROR_SUCCESS;
			}
			if(!SignatureValid(ofsCurrent,DBMODULENAME_SIGNATURE)) {
				AddToStatus(STATUS_ERROR,TranslateT("Module chain corrupted, further entries ignored"));
				phase++;
				return ERROR_SUCCESS;
			}
			if(PeekSegment(ofsCurrent,&moduleName,offsetof(DBModuleName,name))!=ERROR_SUCCESS) {
				phase++;
				return ERROR_SUCCESS;
			}
			if(moduleName.cbName>256)
				AddToStatus(STATUS_WARNING,TranslateT("Unreasonably long module name, skipping"));
			else {
				modChain=(ModChainEntry*)realloc(modChain,sizeof(ModChainEntry)*++modChainCount);

				modChain[modChainCount-1].ofsOld=ofsCurrent;
				modChain[modChainCount-1].size=offsetof(DBModuleName,name)+moduleName.cbName;
				modChain[modChainCount-1].ofsNew=0;

				if (moduleName.cbName)
					PeekSegment(ofsCurrent+offsetof(DBModuleName,name),&modChain[modChainCount-1].name,moduleName.cbName);
				modChain[modChainCount-1].name[moduleName.cbName]=0;
			}
			ofsCurrent=moduleName.ofsNext;
			break;
		case 1:
			ofsLast = 0;
			iCurrentModName=0;
			dbhdr.ofsFirstModuleName=0;
			phase++;
		case 2:
			if(iCurrentModName>=modChainCount) {
				DWORD dw = 0;
				if(ofsLast)	WriteSegment(ofsLast+offsetof(DBModuleName,ofsNext),&dw,sizeof(DWORD));
				return ERROR_NO_MORE_ITEMS;
			}
			if(modChain[iCurrentModName].ofsNew==0) {
				newModName=(DBModuleName*)_alloca(modChain[iCurrentModName].size);
				if(ReadSegment(modChain[iCurrentModName].ofsOld,newModName,modChain[iCurrentModName].size)!=ERROR_SUCCESS)
					return ERROR_NO_MORE_ITEMS;
				if((modChain[iCurrentModName].ofsNew=WriteSegment(WSOFS_END,newModName,modChain[iCurrentModName].size))==WS_ERROR)
					return ERROR_HANDLE_DISK_FULL;
				{ // check duplicated modulenames
					int i, n=0;
					for(i=iCurrentModName+1;i<modChainCount;i++)
						if(!strcmp(modChain[i].name, modChain[iCurrentModName].name)) {
							modChain[i].ofsNew = modChain[iCurrentModName].ofsNew;
							n++;
						}
					if (n) {
						TCHAR *pszModuleName;
#ifdef UNICODE
						TCHAR szModuleName[257];
						MultiByteToWideChar(CP_ACP, 0, modChain[iCurrentModName].name, -1, szModuleName, sizeof(szModuleName) / sizeof(TCHAR));
						pszModuleName = szModuleName;
#else
						pszModuleName = modChain[iCurrentModName].name;
#endif
						AddToStatus(STATUS_WARNING,TranslateT("Module name '%s' is not unique: %d duplicates found)"), pszModuleName, n);
					}
				}
				if(iCurrentModName==0)
					dbhdr.ofsFirstModuleName=modChain[iCurrentModName].ofsNew;
				else
					if(WriteSegment(ofsLast+offsetof(DBModuleName,ofsNext),&modChain[iCurrentModName].ofsNew,sizeof(DWORD))==WS_ERROR)
						return ERROR_HANDLE_DISK_FULL;
				ofsLast = modChain[iCurrentModName].ofsNew;
			}
			iCurrentModName++;
			break;
	}
	return ERROR_SUCCESS;
}

DWORD ConvertModuleNameOfs(DWORD ofsOld)
{
	int i;

	if ( modChain[last_mod].ofsOld==ofsOld )
		return modChain[last_mod].ofsNew;

	for(i=0;i<modChainCount;i++)
		if(modChain[i].ofsOld==ofsOld) {
			last_mod = i;
			return modChain[last_mod].ofsNew;
		}

	AddToStatus(STATUS_ERROR,TranslateT("Invalid module name offset, skipping data"));
	return 0;
}

void FreeModuleChain()
{
	if(modChain!=NULL) {
		free(modChain);
		modChain = NULL;
		last_mod = 0;
	}
}
