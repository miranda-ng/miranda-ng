/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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
#include "clist.h"

TCHAR *GetNameForContact(MCONTACT hContact,int flag,boolean *isUnknown);
char *GetProtoForContact(MCONTACT hContact);
int GetStatusForContact(MCONTACT hContact,char *szProto);
TCHAR *UnknownConctactTranslatedName;
extern boolean OnModulesLoadedCalled;
void InvalidateDisplayNameCacheEntryByPDNE(MCONTACT hContact,ClcCacheEntry *pdnce,int SettingType);

static int handleCompare( ClcCacheEntry* c1, ClcCacheEntry* c2 )
{
	return (char*)c1->hContact - (char*)c2->hContact;
}

void InitDisplayNameCache(SortedList *list)
{
	int i, idx;

	memset(list,0,sizeof(SortedList));
	list->sortFunc = (FSortFunc)handleCompare;
	list->increment = CallService(MS_DB_CONTACT_GETCOUNT,0,0)+1;

	i = 0;
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		ClcCacheEntry *pdnce = (ClcCacheEntry *)mir_calloc(sizeof(ClcCacheEntry));
		pdnce->hContact = hContact;
		InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,0);
		List_GetIndex(list,pdnce,&idx);
		List_Insert(list,pdnce,idx);
		i++;
}	}

void FreeDisplayNameCacheItem(ClcCacheEntry *p)
{
	if ( p->tszName) { mir_free(p->tszName); p->tszName = NULL; }
	if ( p->szProto) { mir_free(p->szProto); p->szProto = NULL; }
	if ( p->tszGroup) { mir_free(p->tszGroup); p->tszGroup = NULL; }
}

void FreeDisplayNameCache(SortedList *list)
{
	for (int i = 0; i < list->realCount; i++) {
		FreeDisplayNameCacheItem((ClcCacheEntry*)list->items[i] );
		mir_free(list->items[i]);
	}

	List_Destroy(list);
}

void CheckPDNCE(ClcCacheEntry *_pdnce)
{
	ClcCacheEntry *pdnce = (ClcCacheEntry *)_pdnce;
	if (pdnce == NULL)
		return;

	if (pdnce->szProto == NULL && pdnce->protoNotExists == FALSE) {
		pdnce->szProto = GetProtoForContact(pdnce->hContact);
		if (pdnce->szProto == NULL)
			pdnce->protoNotExists = FALSE;
		else {
			if (CallService(MS_PROTO_ISPROTOCOLLOADED,0,(LPARAM)pdnce->szProto) == 0)
				pdnce->protoNotExists = TRUE;
			else {
				if ( pdnce->szProto && pdnce->tszName ) {
					mir_free(pdnce->tszName);
					pdnce->tszName = NULL;
	}	}	}	}

	if (pdnce->tszName == NULL)
	{
		if (pdnce->protoNotExists)
			pdnce->tszName = mir_tstrdup(TranslateT("_NoProtocol_"));
		else {
			if (OnModulesLoadedCalled)
				pdnce->tszName = GetNameForContact(pdnce->hContact,0,&pdnce->isUnknown);
			else
				pdnce->tszName = GetNameForContact(pdnce->hContact,0,NULL);
		}
	}
	else {
		if (pdnce->isUnknown&&pdnce->szProto&&pdnce->protoNotExists == TRUE&&OnModulesLoadedCalled) {
			pdnce->protoNotExists = FALSE;
			mir_free(pdnce->tszName);
			pdnce->tszName = GetNameForContact(pdnce->hContact,0,&pdnce->isUnknown);
	}	}

	if (pdnce->status == 0)
		pdnce->status = GetStatusForContact(pdnce->hContact,pdnce->szProto);

	if (pdnce->tszGroup == NULL)
	{
		DBVARIANT dbv;

		if ( !db_get_ts(pdnce->hContact,"CList","Group",&dbv))
		{
			pdnce->tszGroup = mir_tstrdup(dbv.ptszVal);
			mir_free(dbv.pszVal);
		}
		else pdnce->tszGroup = mir_tstrdup( _T(""));
	}

	if (pdnce->bIsHidden == -1)
		pdnce->bIsHidden = db_get_b(pdnce->hContact,"CList","Hidden",0);

	if (pdnce->noHiddenOffline == -1)
		pdnce->noHiddenOffline = db_get_b(pdnce->hContact,"CList","noOffline",0);

	if ( pdnce->IdleTS == -1 )
		pdnce->IdleTS = db_get_dw(pdnce->hContact,pdnce->szProto,"IdleTS",0);

	if (pdnce->ApparentMode == -1)
		pdnce->ApparentMode = db_get_w(pdnce->hContact,pdnce->szProto,"ApparentMode",0);

	if (pdnce->NotOnList == -1)
		pdnce->NotOnList = db_get_b(pdnce->hContact,"CList","NotOnList",0);

	if (pdnce->IsExpanded == -1)
		pdnce->IsExpanded = db_get_b(pdnce->hContact,"CList","Expanded",0);
}

void InvalidateDisplayNameCacheEntryByPDNE(MCONTACT hContact,ClcCacheEntry *pdnce,int SettingType)
{
	if ( hContact == NULL || pdnce == NULL )
		return;

	if ( SettingType == -1 || SettingType == DBVT_DELETED )
	{
        mir_free(pdnce->tszName);
		pdnce->tszName = NULL;
		mir_free(pdnce->tszGroup);
		pdnce->tszGroup = NULL;

		pdnce->bIsHidden = -1;
		pdnce->protoNotExists = FALSE;
		mir_free(pdnce->szProto);
		pdnce->szProto = NULL;
		pdnce->status = 0;
		pdnce->IdleTS = -1;
		pdnce->ApparentMode = -1;
		pdnce->NotOnList = -1;
		pdnce->isUnknown = FALSE;
		pdnce->noHiddenOffline = -1;
		pdnce->IsExpanded = -1;
		return;
	}

	if (SettingType >= DBVT_WCHAR)
	{
		mir_free(pdnce->tszName);
		pdnce->tszName = NULL;
		mir_free(pdnce->tszGroup);
		pdnce->tszGroup = NULL;
		mir_free(pdnce->szProto);
		pdnce->szProto = NULL;
		return;
	}

	// in other cases clear all binary cache
	pdnce->bIsHidden = -1;
	pdnce->protoNotExists = FALSE;
	pdnce->status = 0;
	pdnce->IdleTS = -1;
	pdnce->ApparentMode = -1;
	pdnce->NotOnList = -1;
	pdnce->isUnknown = FALSE;
	pdnce->noHiddenOffline = -1;
 	pdnce->IsExpanded = -1;
}

char *GetContactCachedProtocol(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry&&cacheEntry->szProto)
		return cacheEntry->szProto;

	return NULL;
}

char *GetProtoForContact(MCONTACT hContact)
{
	return mir_strdup(GetContactProto(hContact));
}

int GetStatusForContact(MCONTACT hContact,char *szProto)
{
	int status = ID_STATUS_OFFLINE;
	if (szProto)
		status = db_get_w((MCONTACT)hContact, szProto, "Status", ID_STATUS_OFFLINE);

	return (status);
}

TCHAR* GetNameForContact(MCONTACT hContact,int flag,boolean *isUnknown)
{
	TCHAR* result = pcli->pfnGetContactDisplayName(hContact, flag | GCDNF_NOCACHE);

	if (isUnknown) {
		if (UnknownConctactTranslatedName == NULL)
			UnknownConctactTranslatedName = TranslateT("(Unknown Contact)");

		*isUnknown = lstrcmp(result ,UnknownConctactTranslatedName) == 0;
	}

	return result;
}

ClcCacheEntry *GetContactFullCacheEntry(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry != NULL)
		return(cacheEntry);

	return NULL;
}

int GetContactInfosForSort(MCONTACT hContact,char **Proto,TCHAR **Name,int *Status)
{
	ClcCacheEntry *cacheEntry = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry != NULL)
	{
		if (Proto != NULL)  *Proto  = cacheEntry->szProto;
		if (Name != NULL)   *Name   = cacheEntry->tszName;
		if (Status != NULL) *Status = cacheEntry->status;
	}
	return 0;
}

int GetContactCachedStatus(MCONTACT hContact)
{
	ClcCacheEntry *cacheEntry = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry&&cacheEntry->status != 0) return cacheEntry->status;
	return 0;
}

int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	// Early exit
	if (hContact == NULL)
		return 0;

	__try
	{
		ClcCacheEntry *pdnce = (ClcCacheEntry *)pcli->pfnGetCacheEntry(hContact);
		if (pdnce == NULL) {
			OutputDebugStringA("!!! Very bad pdnce not found.");
			return 0;
		}

		if (pdnce && pdnce->protoNotExists == FALSE && pdnce->szProto) {
			if ( !strcmp(cws->szModule,pdnce->szProto)) {
				InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,cws->value.type);

				if (cws->value.type == DBVT_WORD && !strcmp(cws->szSetting, "Status")) {
					if ( !(pdnce->bIsHidden == 1)) {

						if ( db_get_b(NULL,"CList","ShowStatusMsg",0)||db_get_b(hContact,"CList","StatusMsgAuto",0))
							db_set_s(hContact, "CList", "StatusMsg", "");

						if ( db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT))	{
							// User's state is changing, and we are hideOffline-ing
							if (cws->value.wVal == ID_STATUS_OFFLINE) {
								ChangeContactIcon(hContact, ExtIconFromStatusMode(hContact,cws->szModule, cws->value.wVal), 0); //by FYR
								return 0;
							}
							ChangeContactIcon(hContact, ExtIconFromStatusMode(hContact,cws->szModule, cws->value.wVal), 0); //by FYR
						}
						else ChangeContactIcon(hContact, ExtIconFromStatusMode(hContact,cws->szModule, cws->value.wVal), 0); //by FYR
					}
				}
				else if (!strcmp(cws->szModule, META_PROTO)) {
					pcli->pfnInitAutoRebuild(pcli->hwndContactTree);
					return 0;
				}
				else return 0;

				SortContacts();
			}
		}

		if ( !strcmp(cws->szModule,"CList")) {
			if (pdnce->tszName == NULL || !strcmp(cws->szSetting,"MyHandle"))
				InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,cws->value.type);

			if ((!strcmp(cws->szSetting,"Group")))
				InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,cws->value.type);

			if ( !strcmp(cws->szSetting,"Hidden")) {
				InvalidateDisplayNameCacheEntryByPDNE(hContact,pdnce,cws->value.type);
				if (cws->value.type == DBVT_DELETED || cws->value.bVal == 0) {
					char *szProto = GetContactProto(hContact);
					ChangeContactIcon(hContact,ExtIconFromStatusMode(hContact, szProto, szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE)), 1);  //by FYR
				}
			}

			if ( !strcmp(cws->szSetting, "noOffline"))
				InvalidateDisplayNameCacheEntryByPDNE(hContact, pdnce, cws->value.type);
		}

		if ( !strcmp(cws->szModule, "Protocol")) {
			if ( !strcmp(cws->szSetting,"p")) {
				OutputDebugStringA("CHANGE: proto\r\n");
				InvalidateDisplayNameCacheEntryByPDNE(hContact, pdnce, cws->value.type);
				char *szProto = (cws->value.type == DBVT_DELETED) ? NULL : cws->value.pszVal;
				ChangeContactIcon(hContact, ExtIconFromStatusMode(hContact, szProto, szProto == NULL?ID_STATUS_OFFLINE:db_get_w(hContact,szProto,"Status",ID_STATUS_OFFLINE)),0); //by FYR
			}
		}
	}
	__except (exceptFunction(GetExceptionInformation()))
	{
	}

	return 0;
}
