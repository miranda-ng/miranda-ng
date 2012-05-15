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
// Miranda Memory-Mapped Secured DataBase
// (C) Artem Shpynov aka FYR, Igonin Vitaliy aka chaos.persei, Victor Pavlychko aka nullbie, 2007 - 2008

#include "commonheaders.h"

/* Public API */
int InitPreset();
void UninitPreset();

int	DBPreset_QuerySetting	(const char *szModule, const char *szSetting, DBVARIANT *dbv, BOOL isStatic);
int	DBPreset_CompareSetting	(const char *szModule, const char *szSetting, DBVARIANT *dbv);

/* Preset cache item */
typedef struct
{
	DWORD dwHash;
	char *szModule;
	char *szSetting;
	DBVARIANT dbv;
} DBPresetItem;

static DBPresetItem *	DBPresetItem_Create		(char *szModule, char *szSetting, BYTE bType);
static void				DBPresetItem_Destroy	(DBPresetItem *item);
static void				DBPresetItem_Hash		(DBPresetItem *item);
static int				DBPresetItem_Cmp		(DBPresetItem *item1, DBPresetItem *item2);

SortedList *lstPresets = NULL;

int InitPreset()
{
	char szIniPath[MAX_PATH];
	char szLine[2048];
	int lineLength;
	char szSection[128];
	FILE *fp;

	GetModuleFileNameA(GetModuleHandle(NULL), szIniPath, SIZEOF(szIniPath));
	strcpy(strrchr(szIniPath, '\\')+1, "dbpreset.ini");

	fp=fopen(szIniPath,"rt");

	// no preset
	if (!fp) return 0;

	lstPresets = li.List_Create(0, 50);
	lstPresets->sortFunc = DBPresetItem_Cmp;

	while(!feof(fp))
	{
		if(fgets(szLine,sizeof(szLine),fp)==NULL) break;
		lineLength=lstrlenA(szLine);
		while(lineLength && (BYTE)(szLine[lineLength-1])<=' ') szLine[--lineLength]='\0';
		if(szLine[0]==';' || szLine[0]<=' ') continue;
		if(szLine[0]=='[')
		{
			char *szEnd=strchr(szLine+1,']');
			if(szEnd==NULL) continue;
			if(szLine[1]=='!')
				szSection[0]='\0';
			else
				lstrcpynA(szSection, szLine+1, (int)min(sizeof(szSection), szEnd-szLine));
		} else
		{
			char *szValue;
			char szName[128];
			DBPresetItem *item;

			if(szSection[0]=='\0') continue;
			szValue=strchr(szLine,'=');
			if(szValue==NULL) continue;
			lstrcpynA(szName, szLine, (int)min(sizeof(szName), szValue-szLine+1));
			szValue++;

			switch(szValue[0])
			{
				case 'b':
				case 'B':
					item = DBPresetItem_Create(szSection, szName, DBVT_BYTE);
					item->dbv.bVal = (BYTE)strtol(szValue+1,NULL,0);
					li.List_InsertPtr(lstPresets, item);
					break;
				case 'w':
				case 'W':
					item = DBPresetItem_Create(szSection, szName, DBVT_WORD);
					item->dbv.wVal = (WORD)strtol(szValue+1,NULL,0);
					li.List_InsertPtr(lstPresets, item);
					break;
				case 'd':
				case 'D':
					item = DBPresetItem_Create(szSection, szName, DBVT_DWORD);
					item->dbv.dVal = (DWORD)strtoul(szValue+1,NULL,0);
					li.List_InsertPtr(lstPresets, item);
					break;
				case 's':
				case 'S':
					item = DBPresetItem_Create(szSection, szName, DBVT_ASCIIZ);
					item->dbv.pszVal = mir_strdup(szValue+1);
					li.List_InsertPtr(lstPresets, item);
					break;
				case 'u':
				case 'U':
					item = DBPresetItem_Create(szSection, szName, DBVT_UTF8);
					item->dbv.pszVal = mir_strdup(szValue+1);
					li.List_InsertPtr(lstPresets, item);
					break;
				case 'n':
				case 'N':
				{
					PBYTE buf;
					int len;
					char *pszValue,*pszEnd;

					buf=(PBYTE)mir_alloc(lstrlenA(szValue+1));
					for(len=0,pszValue=szValue+1;;len++) {
						buf[len]=(BYTE)strtol(pszValue,&pszEnd,0x10);
						if(pszValue==pszEnd) break;
						pszValue=pszEnd;
					}

					item = DBPresetItem_Create(szSection, szName, DBVT_BLOB);
					item->dbv.pbVal = buf;
					item->dbv.cpbVal = len;
					li.List_InsertPtr(lstPresets, item);
					break;
				}
			}
		}
	}
	fclose(fp);

	return 0;
}

void UninitPreset()
{
	int i;
	if (!lstPresets) return;
	for (i = 0; i < lstPresets->realCount; ++i)
		DBPresetItem_Destroy(lstPresets->items[i]);
	li.List_Destroy(lstPresets);
}

int DBPreset_QuerySetting(const char *szModule, const char *szSetting, DBVARIANT *dbv, BOOL isStatic)
{
	DBPresetItem *item;
	DBPresetItem search = {0};

	if (!lstPresets) return FALSE;

	search.szModule = (char *)szModule;
	search.szSetting = (char *)szSetting;
	DBPresetItem_Hash(&search);
	item = li.List_Find(lstPresets, &search);

	if (!item) return FALSE;

	dbv->type = item->dbv.type;
	switch (item->dbv.type)
	{
		case DBVT_BYTE:   dbv->bVal = item->dbv.bVal; return TRUE;
		case DBVT_WORD:   dbv->wVal = item->dbv.wVal; return TRUE;
		case DBVT_DWORD:  dbv->dVal = item->dbv.dVal; return TRUE;

		case DBVT_UTF8:
		case DBVT_ASCIIZ:
			if (isStatic && dbv->pszVal)
				lstrcpynA(dbv->pszVal, item->dbv.pszVal, dbv->cchVal);
			else if (!isStatic)
				dbv->pszVal = mir_strdup(item->dbv.pszVal);
			return TRUE;

		default:
			return FALSE;
	}

	return FALSE;
}

int DBPreset_CompareSetting(const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	DBPresetItem *item;
	DBPresetItem search = {0};

	if (!lstPresets) return FALSE;

	search.szModule = (char *)szModule;
	search.szSetting = (char *)szSetting;
	DBPresetItem_Hash(&search);
	item = li.List_Find(lstPresets, &search);

	if (!item) return FALSE;
	if (item->dbv.type != item->dbv.type) return FALSE;
	switch (item->dbv.type)
	{
		case DBVT_BYTE:   return dbv->bVal == item->dbv.bVal ? TRUE : FALSE;
		case DBVT_WORD:   return dbv->wVal == item->dbv.wVal ? TRUE : FALSE;
		case DBVT_DWORD:  return dbv->dVal == item->dbv.dVal ? TRUE : FALSE;
		case DBVT_UTF8:
		case DBVT_ASCIIZ: return strcmp(dbv->pszVal, item->dbv.pszVal) ? FALSE : TRUE;
	}

	return FALSE;
}

static DBPresetItem *DBPresetItem_Create(char *szModule, char *szSetting, BYTE bType)
{
	DBPresetItem *item = (DBPresetItem *)mir_alloc(sizeof(DBPresetItem));
	item->szModule = mir_strdup(szModule);
	item->szSetting = mir_strdup(szSetting);
	DBPresetItem_Hash(item);
	item->dbv.type = bType;
	return item;
}

static void DBPresetItem_Destroy(DBPresetItem *item)
{
	if (!item) return;
	if (item->szModule)
	{
		mir_free(item->szModule);
		item->szModule = NULL;
	}
	if (item->szSetting)
	{
		mir_free(item->szSetting);
		item->szSetting = NULL;
	}

	switch (item->dbv.type)
	{
		case DBVT_ASCIIZ:
		case DBVT_UTF8:
		case DBVT_WCHAR:
		{
			if (item->dbv.pszVal)
				mir_free(item->dbv.pszVal);
			item->dbv.pszVal=0;
			break;
		}
		case DBVT_BLOB:
		{
			if (item->dbv.pbVal)
				mir_free(item->dbv.pbVal);
			item->dbv.pbVal=0;
			break;
		}
	}
	item->dbv.type = 0;
}

static void DBPresetItem_Hash(DBPresetItem *item)
{
	int i;
	int shift=0;
	item->dwHash=0;
	for(i=0;item->szModule[i];i++)
	{
		item->dwHash^=item->szModule[i]<<shift;
		if (shift>24) item->dwHash^=(item->szModule[i]>>(32-shift))&0x7F;
		shift=(shift+5)&0x1F;
	}
	for(i=0;item->szSetting[i];i++)
	{
		item->dwHash^=item->szSetting[i]<<shift;
		if (shift>24) item->dwHash^=(item->szSetting[i]>>(32-shift))&0x7F;
		shift=(shift+5)&0x1F;
	}
}

static int DBPresetItem_Cmp(DBPresetItem *item1, DBPresetItem *item2)
{
	int cmp;
	if (item1->dwHash < item2->dwHash) return -1;
	if (item1->dwHash > item2->dwHash) return 1;
	if (cmp = strcmp(item1->szModule, item2->szModule)) return cmp;
	return strcmp(item1->szSetting, item2->szSetting);
}
