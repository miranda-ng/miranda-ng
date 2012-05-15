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
#include "m_clui.h"
#include "m_clc.h"
#include "clist.h"

TCHAR *GetNameForContact(HANDLE hContact,int flag,boolean *isUnknown);
char *GetProtoForContact(HANDLE hContact);
int GetStatusForContact(HANDLE hContact,char *szProto);
TCHAR *UnknownConctactTranslatedName;
extern boolean OnModulesLoadedCalled;
void InvalidateDisplayNameCacheEntryByPDNE(HANDLE hContact,pdisplayNameCacheEntry pdnce,int SettingType);

static int handleCompare( displayNameCacheEntry* c1, displayNameCacheEntry* c2 )
{
	return (char*)c1->hContact - (char*)c2->hContact;
}

void InitDisplayNameCache(SortedList *list)
{
	int i, idx;
	HANDLE hContact;

	memset(list,0,sizeof(SortedList));
	list->sortFunc=handleCompare;
	list->increment=CallService(MS_DB_CONTACT_GETCOUNT,0,0)+1;

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	i=0;
	while (hContact!=0)
	{
		displayNameCacheEntry *pdnce = mir_calloc(sizeof(displayNameCacheEntry));
		pdnce->hContact = hContact;
		InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,0);
		li.List_GetIndex(list,pdnce,&idx);
		li.List_Insert(list,pdnce,idx);
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
		i++;
}	}

void FreeDisplayNameCacheItem(ClcCacheEntryBase *_p)
{
	pdisplayNameCacheEntry p = (pdisplayNameCacheEntry)_p;
	if ( p->name) { mir_free(p->name); p->name = NULL; }
	#if defined( _UNICODE )
		if ( p->szName) { mir_free(p->szName); p->szName = NULL; }
	#endif
	if ( p->szProto) { mir_free(p->szProto); p->szProto = NULL; }
	if ( p->szGroup) { mir_free(p->szGroup); p->szGroup = NULL; }
}

void FreeDisplayNameCache(SortedList *list)
{
	int i;

	for( i=0; i < list->realCount; i++) {
		FreeDisplayNameCacheItem(list->items[i] );
		mir_free(list->items[i]);
	}
	
	li.List_Destroy(list);
}

void CheckPDNCE(ClcCacheEntryBase *_pdnce)
{
	pdisplayNameCacheEntry pdnce = (pdisplayNameCacheEntry)_pdnce;
	if (pdnce == NULL)
		return;

	if (pdnce->szProto == NULL && pdnce->protoNotExists == FALSE) {
		pdnce->szProto=GetProtoForContact(pdnce->hContact);
		if (pdnce->szProto == NULL) 
			pdnce->protoNotExists=FALSE;
		else {
			if (CallService(MS_PROTO_ISPROTOCOLLOADED,0,(LPARAM)pdnce->szProto) == 0)
				pdnce->protoNotExists=TRUE;
			else {
				if ( pdnce->szProto && pdnce->name ) {
					mir_free(pdnce->name);
					pdnce->name=NULL;
	}	}	}	}

	if (pdnce->name == NULL)
	{			
		if (pdnce->protoNotExists)
			pdnce->name = mir_tstrdup(TranslateT("_NoProtocol_"));
		else {
			if (OnModulesLoadedCalled)
				pdnce->name=GetNameForContact(pdnce->hContact,0,&pdnce->isUnknown);
			else
				pdnce->name=GetNameForContact(pdnce->hContact,0,NULL);
		}	
	}
	else {
		if (pdnce->isUnknown&&pdnce->szProto&&pdnce->protoNotExists == TRUE&&OnModulesLoadedCalled) {
			pdnce->protoNotExists=FALSE;
            mir_free(pdnce->name);
			pdnce->name=GetNameForContact(pdnce->hContact,0,&pdnce->isUnknown);
	}	}

	if (pdnce->status == 0)
		pdnce->status = GetStatusForContact(pdnce->hContact,pdnce->szProto);

	if (pdnce->szGroup == NULL)
	{
		DBVARIANT dbv;

		if (!DBGetContactSettingTString(pdnce->hContact,"CList","Group",&dbv))
		{
			pdnce->szGroup = mir_tstrdup(dbv.ptszVal);
			mir_free(dbv.pszVal);
		}
		else pdnce->szGroup = mir_tstrdup( _T(""));
	}

	if (pdnce->Hidden == -1)
		pdnce->Hidden=DBGetContactSettingByte(pdnce->hContact,"CList","Hidden",0);
	
	if (pdnce->noHiddenOffline == -1)
		pdnce->noHiddenOffline=DBGetContactSettingByte(pdnce->hContact,"CList","noOffline",0);

	if ( pdnce->IdleTS == -1 )
		pdnce->IdleTS = DBGetContactSettingDword(pdnce->hContact,pdnce->szProto,"IdleTS",0);

	if (pdnce->ApparentMode == -1)
		pdnce->ApparentMode=DBGetContactSettingWord(pdnce->hContact,pdnce->szProto,"ApparentMode",0);
	
	if (pdnce->NotOnList == -1)
		pdnce->NotOnList=DBGetContactSettingByte(pdnce->hContact,"CList","NotOnList",0);

	if (pdnce->IsExpanded == -1)
		pdnce->IsExpanded=DBGetContactSettingByte(pdnce->hContact,"CList","Expanded",0);
}

void InvalidateDisplayNameCacheEntryByPDNE(HANDLE hContact,pdisplayNameCacheEntry pdnce,int SettingType)
{
	if ( hContact == NULL || pdnce == NULL )
		return;

	if ( SettingType == -1 || SettingType == DBVT_DELETED )
	{		
        mir_free(pdnce->name);
		pdnce->name=NULL;
		mir_free(pdnce->szGroup);
		pdnce->szGroup=NULL;

		pdnce->Hidden=-1;
		pdnce->protoNotExists=FALSE;
		mir_free(pdnce->szProto);
		pdnce->szProto=NULL;
		pdnce->status=0;
		pdnce->IdleTS=-1;
		pdnce->ApparentMode=-1;
		pdnce->NotOnList=-1;
		pdnce->isUnknown=FALSE;
		pdnce->noHiddenOffline=-1;
		pdnce->IsExpanded=-1;
		return;
	}

	if (SettingType >= DBVT_WCHAR)
	{
		mir_free(pdnce->name);
		pdnce->name=NULL;
		mir_free(pdnce->szGroup);
		pdnce->szGroup=NULL;
		mir_free(pdnce->szProto);
		pdnce->szProto=NULL;
		return;
	}

	// in other cases clear all binary cache
	pdnce->Hidden=-1;
	pdnce->protoNotExists=FALSE;
	pdnce->status=0;
	pdnce->IdleTS=-1;
	pdnce->ApparentMode=-1;
	pdnce->NotOnList=-1;
	pdnce->isUnknown=FALSE;
	pdnce->noHiddenOffline=-1;
 	pdnce->IsExpanded=-1;
}

char *GetContactCachedProtocol(HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry&&cacheEntry->szProto) 
		return cacheEntry->szProto;

	return NULL;
}

char *GetProtoForContact(HANDLE hContact)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	char name[32];

	dbv.type=DBVT_ASCIIZ;
	dbv.pszVal=name;
	dbv.cchVal=SIZEOF(name);
	dbcgs.pValue=&dbv;
	dbcgs.szModule="Protocol";
	dbcgs.szSetting="p";
	if(CallService(MS_DB_CONTACT_GETSETTINGSTATIC,(WPARAM)hContact,(LPARAM)&dbcgs)) return NULL;
	return mir_strdup(dbcgs.pValue->pszVal);
}

int GetStatusForContact(HANDLE hContact,char *szProto)
{
	int status = ID_STATUS_OFFLINE;
	if (szProto)
		status = DBGetContactSettingWord((HANDLE)hContact,szProto,"Status",ID_STATUS_OFFLINE);

	return (status);
}

TCHAR* GetNameForContact(HANDLE hContact,int flag,boolean *isUnknown)
{
	TCHAR* result = pcli->pfnGetContactDisplayName(hContact, flag | GCDNF_NOCACHE);

	if (isUnknown) {
		if (UnknownConctactTranslatedName == NULL)
			UnknownConctactTranslatedName = TranslateT("(Unknown Contact)");

		*isUnknown = lstrcmp(result ,UnknownConctactTranslatedName) == 0;
	}

	return result;
}

pdisplayNameCacheEntry GetContactFullCacheEntry(HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry!=NULL)
		return(cacheEntry);

	return (NULL);
}

int GetContactInfosForSort(HANDLE hContact,char **Proto,TCHAR **Name,int *Status)
{
	pdisplayNameCacheEntry cacheEntry = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry!=NULL)
	{
		if (Proto!=NULL)  *Proto  = cacheEntry->szProto;
		if (Name!=NULL)   *Name   = cacheEntry->name;
		if (Status!=NULL) *Status = cacheEntry->status;
	}
	return (0);
}

int GetContactCachedStatus(HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry&&cacheEntry->status!=0) return cacheEntry->status;
	return (0);	
}

int ContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	pdisplayNameCacheEntry pdnce;

	// Early exit
	if ((HANDLE)wParam == NULL)
		return 0;

	__try  
	{
		pdnce = (pdisplayNameCacheEntry)pcli->pfnGetCacheEntry((HANDLE)wParam);
		if (pdnce == NULL)
		{
			OutputDebugStringA("!!! Very bad pdnce not found.");
			return 0;
		}

		if (pdnce&&(pdnce->protoNotExists == FALSE)&&pdnce->szProto)
		{
			if (!strcmp(cws->szModule,pdnce->szProto))
			{
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);

				if (cws->value.type == DBVT_WORD&&!strcmp(cws->szSetting, "Status")) {				
					if (!(pdnce->Hidden == 1)) {

						if(DBGetContactSettingByte((HANDLE)NULL,"CList","ShowStatusMsg",0)||DBGetContactSettingByte((HANDLE)wParam,"CList","StatusMsgAuto",0))
							DBWriteContactSettingString((HANDLE)wParam, "CList", "StatusMsg", "");	

						if (DBGetContactSettingByte(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT))	{
							// User's state is changing, and we are hideOffline-ing

							if (cws->value.wVal == ID_STATUS_OFFLINE) {
								ChangeContactIcon((HANDLE)wParam, ExtIconFromStatusMode((HANDLE)wParam,cws->szModule, cws->value.wVal), 0); //by FYR
								return 0;
							}
							ChangeContactIcon((HANDLE)wParam, ExtIconFromStatusMode((HANDLE)wParam,cws->szModule, cws->value.wVal), 0); //by FYR
						}
						else
						{
							ChangeContactIcon((HANDLE)wParam, ExtIconFromStatusMode((HANDLE)wParam,cws->szModule, cws->value.wVal), 0); //by FYR
						}
					}
				}
				else if (!strcmp(cws->szModule,"MetaContacts"))
				{
					PostMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
					return 0;
				}
				else return 0;

				SortContacts();
		}	}

		if(!strcmp(cws->szModule,"CList")) {
			/*
			sprintf(buf,"CHANGE: module:CList setting:%s %s\r\n",cws->szSetting,cws->value.pszVal);
			OutputDebugStringA(buf);
			*/

			//name is null or (setting is myhandle)
			if(pdnce->name == NULL||(!strcmp(cws->szSetting,"MyHandle")) )
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);

			if((!strcmp(cws->szSetting,"Group")) )
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);

			if(!strcmp(cws->szSetting,"Hidden")) {
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);		
				if(cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
					char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
					ChangeContactIcon((HANDLE)wParam,ExtIconFromStatusMode((HANDLE)wParam,szProto,szProto == NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE)),1);  //by FYR
			}	}

			if(!strcmp(cws->szSetting,"noOffline"))
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);
		}

		if(!strcmp(cws->szModule,"Protocol")) {
			if(!strcmp(cws->szSetting,"p")) {
				char *szProto;

				OutputDebugStringA("CHANGE: proto\r\n");
				InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);	
				if(cws->value.type == DBVT_DELETED) szProto=NULL;
				else szProto=cws->value.pszVal;
				ChangeContactIcon((HANDLE)wParam,ExtIconFromStatusMode((HANDLE)wParam,szProto,szProto == NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE)),0); //by FYR
			}
		}
	} 
	__except (exceptFunction(GetExceptionInformation()) ) 
	{ 
	} 

	return 0;
}
