/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#include "hdr/modern_commonheaders.h"
#include "m_clui.h"
#include "m_clc.h"
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_awaymsg.h"

void InsertContactIntoTree(HANDLE hContact,int status);
static displayNameCacheEntry *displayNameCache;
void CListSettings_FreeCacheItemDataOption( pdisplayNameCacheEntry pDst, DWORD flag );

int PostAutoRebuidMessage(HWND hwnd);
static int displayNameCacheSize;

BOOL CLM_AUTOREBUILD_WAS_POSTED=FALSE;
SortedList *clistCache = NULL;
TCHAR* GetNameForContact(HANDLE hContact,int flag,boolean *isUnknown);
char *GetProtoForContact(HANDLE hContact);
int GetStatusForContact(HANDLE hContact,char *szProto);
TCHAR *UnknownConctactTranslatedName=NULL;

void InvalidateDNCEbyPointer(HANDLE hContact,pdisplayNameCacheEntry pdnce,int SettingType);

static int handleCompare( void* c1, void* c2 )
{

	INT_PTR p1, p2;

	displayNameCacheEntry * dnce1=(displayNameCacheEntry *)c1;
	displayNameCacheEntry * dnce2=(displayNameCacheEntry *)c2;

	p1=(INT_PTR)dnce1->m_cache_hContact;
	p2=(INT_PTR)dnce2->m_cache_hContact;

	if ( p1 == p2 )
		return 0;

	return (int)(p1 - p2);
}

void InitCacheAsync();
void UninitCacheAsync();

void InitDisplayNameCache(void)
{
	int i=0;
    InitCacheAsync();
	InitAwayMsgModule();
	clistCache = li.List_Create( 0, 50 );
	clistCache->sortFunc = handleCompare;
}
void FreeDisplayNameCache()
{
    UninitCacheAsync();
	UninitAwayMsgModule();
	if ( clistCache != NULL ) {
		int i;
		for ( i = 0; i < clistCache->realCount; i++) {
			pcli->pfnFreeCacheItem(( ClcCacheEntryBase* )clistCache->items[i] );
			mir_free_and_nill( clistCache->items[i] );
		}

		li.List_Destroy( clistCache ); 
		mir_free_and_nill(clistCache);
		clistCache = NULL;
	}	
}


ClcCacheEntryBase* cliGetCacheEntry(HANDLE hContact)
{
	ClcCacheEntryBase* p;   
	int idx;
	if (!clistCache) return NULL;
	if ( !li.List_GetIndex( clistCache, &hContact, &idx )) {	
		if (( p = pcli->pfnCreateCacheItem( hContact )) != NULL ) {
			li.List_Insert( clistCache, p, idx );
			pcli->pfnInvalidateDisplayNameCacheEntry( hContact );
		}
	}
	else p = ( ClcCacheEntryBase* )clistCache->items[idx];
	pcli->pfnCheckCacheItem( p );
	return p;
}


void CListSettings_FreeCacheItemData(pdisplayNameCacheEntry pDst)
{
	CListSettings_FreeCacheItemDataOption( pDst, CCI_ALL);
}

void CListSettings_FreeCacheItemDataOption( pdisplayNameCacheEntry pDst, DWORD flag )
{
    if (!pDst) return;
	if ( flag & CCI_NAME)
	{
		if (pDst->m_cache_tcsName) 
		{
	        if (!pDst->isUnknown && pDst->m_cache_tcsName!=UnknownConctactTranslatedName) mir_free_and_nill(pDst->m_cache_tcsName);
	
	    }
	    #if defined( _UNICODE )
			if (pDst->m_cache_szName) mir_free_and_nill(pDst->m_cache_szName);
		#endif
	}

    if ( flag & CCI_GROUP ) 
		if (pDst->m_cache_tcsGroup) mir_free_and_nill(pDst->m_cache_tcsGroup);

	if ( flag & CCI_LINES ) 
	{
		if (pDst->szSecondLineText) mir_free_and_nill(pDst->szSecondLineText);
		if (pDst->szThirdLineText)  mir_free_and_nill(pDst->szThirdLineText);
		pDst->ssSecondLine.DestroySmileyList();
		pDst->ssThirdLine.DestroySmileyList();
	}
}


int CListSettings_GetCopyFromCache(pdisplayNameCacheEntry pDest, DWORD flag);
int CListSettings_SetToCache(pdisplayNameCacheEntry pSrc, DWORD flag);


void CListSettings_CopyCacheItems(pdisplayNameCacheEntry pDst, pdisplayNameCacheEntry pSrc, DWORD flag)
{
    if (!pDst||!pSrc) return;
    CListSettings_FreeCacheItemDataOption(pDst, flag);

	if ( flag & CCI_NAME )
	{
		pDst->isUnknown=pSrc->isUnknown;
        if (pSrc->isUnknown)
		    pDst->m_cache_tcsName = pSrc->m_cache_tcsName;
        else
		    pDst->m_cache_tcsName=mir_tstrdup(pSrc->m_cache_tcsName);
		#if defined( _UNICODE )
	        pDst->m_cache_szName=mir_strdup(pSrc->m_cache_szName);
		#endif
	}
	if ( flag & CCI_GROUP )  pDst->m_cache_tcsGroup=mir_tstrdup(pSrc->m_cache_tcsGroup);
	if ( flag & CCI_PROTO )	pDst->m_cache_cszProto=pSrc->m_cache_cszProto;
	if ( flag & CCI_STATUS ) pDst->m_cache_nStatus=pSrc->m_cache_nStatus;
	if ( flag & CCI_LINES )
	{
		if ( pDst->szThirdLineText )	mir_free( pDst->szThirdLineText );
		pDst->szThirdLineText = NULL;
		if ( pSrc->szThirdLineText )	pDst->szThirdLineText = mir_tstrdup( pSrc->szThirdLineText );
	
		if ( pDst->szSecondLineText )	mir_free( pDst->szSecondLineText );
		pDst->szSecondLineText = NULL;
		if ( pSrc->szSecondLineText )	pDst->szSecondLineText = mir_tstrdup( pSrc->szSecondLineText );

		pDst->ssThirdLine  = pSrc->ssThirdLine;
		pDst->ssSecondLine = pSrc->ssSecondLine;
	}
	if ( flag & CCI_TIME)	
	{
		pDst->hTimeZone=pSrc->hTimeZone;
	}
	if ( flag & CCI_OTHER)
	{
//		if ( flag & CCI_HIDDEN)			
			pDst->m_cache_nHidden=pSrc->m_cache_nHidden;
//		if ( flag & CCI_NOHIDEOFFLINE)	
			pDst->m_cache_nNoHiddenOffline=pSrc->m_cache_nNoHiddenOffline;
//		if ( flag & CCI_NOPROTO)		
			pDst->m_cache_bProtoNotExists=pSrc->m_cache_bProtoNotExists;
	
//		if ( flag & CCI_HIDESUBCONTACT)	
			pDst->m_cache_nHiddenSubcontact=pSrc->m_cache_nHiddenSubcontact;
//		if ( flag & CCI_I)				
			pDst->i=pSrc->i;
//		if ( flag & CCI_APPARENT)		
			pDst->ApparentMode=pSrc->ApparentMode;
//		if ( flag & CCI_NOTONLIST)		
			pDst->NotOnList=pSrc->NotOnList;
//		if ( flag & CCI_IDLETS)			
			pDst->IdleTS=pSrc->IdleTS;
//		if ( flag & CCI_CCONTACT)		
			pDst->ClcContact=pSrc->ClcContact;
//		if ( flag & CCI_EXPAND)			
			pDst->IsExpanded=pSrc->IsExpanded;
//		if ( flag & CCI_UNKNOWN)		
//			pDst->isUnknown=pSrc->isUnknown;
	}
}

int CListSettings_GetCopyFromCache(pdisplayNameCacheEntry pDest, DWORD flag)
{
    pdisplayNameCacheEntry pSource;
    if (!pDest || !pDest->m_cache_hContact) return -1;
    pSource=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(pDest->m_cache_hContact);
    if (!pSource) return -1;
    CListSettings_CopyCacheItems(pDest, pSource, flag);
    return 0;
}

int CListSettings_SetToCache(pdisplayNameCacheEntry pSrc, DWORD flag)
{
    pdisplayNameCacheEntry pDst;
    if (!pSrc || !pSrc->m_cache_hContact) return -1;
    pDst=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(pSrc->m_cache_hContact);
    if (!pDst) return -1;
    CListSettings_CopyCacheItems(pDst, pSrc, flag);
    return 0;
}

void cliFreeCacheItem( pdisplayNameCacheEntry p )
{
	HANDLE hContact=p->m_cache_hContact;
    TRACEVAR("cliFreeCacheItem hContact=%d",hContact);
	if ( !p->isUnknown && p->m_cache_tcsName && p->m_cache_tcsName!=UnknownConctactTranslatedName) mir_free_and_nill(p->m_cache_tcsName);
	p->m_cache_tcsName = NULL; 
	#if defined( _UNICODE )
		if ( p->m_cache_szName) { mir_free_and_nill(p->m_cache_szName); p->m_cache_szName = NULL; }
	#endif
	if ( p->m_cache_tcsGroup) { mir_free_and_nill(p->m_cache_tcsGroup); p->m_cache_tcsGroup = NULL; }
	if ( p->szSecondLineText) mir_free_and_nill(p->szSecondLineText);
	if ( p->szThirdLineText) mir_free_and_nill(p->szThirdLineText);
	p->ssSecondLine.DestroySmileyList();
	p->ssThirdLine.DestroySmileyList();
}


/*
void FreeDisplayNameCache(SortedList *list)
{
	int i;
		for( i=0; i < list->realCount; i++) {
			FreeDisplayNameCacheItem(( pdisplayNameCacheEntry )list->items[i] );
			mir_free_and_nill(list->items[i]);
		}
	li.List_Destroy(list);

}
*/
void cliCheckCacheItem(pdisplayNameCacheEntry pdnce)
{
	if (pdnce!=NULL)
	{
		if (pdnce->m_cache_hContact==NULL) //selfcontact
		{
			if (!pdnce->m_cache_tcsName) pdnce->m_cache_tcsName=GetNameForContact(pdnce->m_cache_hContact,0,&pdnce->isUnknown);
#ifdef _UNICODE
			if (!pdnce->m_cache_szName) pdnce->m_cache_szName=mir_t2a(pdnce->m_cache_tcsName);
#endif
			return;
		}
		if (pdnce->m_cache_cszProto==NULL&&pdnce->m_cache_bProtoNotExists==FALSE)
		{
			pdnce->m_cache_cszProto=GetProtoForContact(pdnce->m_cache_hContact);
			if (pdnce->m_cache_cszProto==NULL) 
			{
				pdnce->m_cache_bProtoNotExists=FALSE;
			}else
			{
				if (CallService(MS_PROTO_ISPROTOCOLLOADED,0,(LPARAM)pdnce->m_cache_cszProto)==(int)NULL &&0)
				{
					pdnce->m_cache_bProtoNotExists=TRUE;
				}else
				{
					if(pdnce->m_cache_cszProto&&pdnce->m_cache_tcsName) 
					{
						if (!pdnce->isUnknown && pdnce->m_cache_tcsName!=UnknownConctactTranslatedName) 
							mir_free_and_nill(pdnce->m_cache_tcsName);
						pdnce->m_cache_tcsName=NULL;
					}
				}
			}
		}

		if (pdnce->m_cache_tcsName==NULL)
		{			
			if (pdnce->m_cache_bProtoNotExists || !pdnce->m_cache_cszProto)
			{
				pdnce->m_cache_tcsName=UnknownConctactTranslatedName;
				pdnce->isUnknown=TRUE;
			}
			else
			{
                if (!pdnce->isUnknown && pdnce->m_cache_tcsName &&pdnce->m_cache_tcsName!=UnknownConctactTranslatedName) mir_free_and_nill (pdnce->m_cache_tcsName);
				if (g_flag_bOnModulesLoadedCalled)
					pdnce->m_cache_tcsName = GetNameForContact(pdnce->m_cache_hContact,0,&pdnce->isUnknown); //TODO UNICODE
				else
                    // what to return here???
					pdnce->m_cache_tcsName = GetNameForContact(pdnce->m_cache_hContact,0,&pdnce->isUnknown); //TODO UNICODE
			}	
#if defined( _UNICODE )
			if (pdnce->m_cache_szName) mir_free_and_nill(pdnce->m_cache_szName);
#endif
		}
		else
		{
			if (pdnce->isUnknown&&pdnce->m_cache_cszProto&&pdnce->m_cache_bProtoNotExists==TRUE&&g_flag_bOnModulesLoadedCalled)
			{
				if (CallService(MS_PROTO_ISPROTOCOLLOADED,0,(LPARAM)pdnce->m_cache_cszProto)==(int)NULL)
				{
					pdnce->m_cache_bProtoNotExists=FALSE;						
					pdnce->m_cache_tcsName= GetNameForContact(pdnce->m_cache_hContact,0,&pdnce->isUnknown); //TODO UNICODE
				}
			}
		}

		if (pdnce___GetStatus( pdnce )==0) //very strange look status sort is broken let always reread status
		{
			pdnce___SetStatus( pdnce , GetStatusForContact(pdnce->m_cache_hContact,pdnce->m_cache_cszProto));
		}
		if (pdnce->m_cache_tcsGroup==NULL)
		{
			DBVARIANT dbv={0};

			if (!ModernGetSettingTString(pdnce->m_cache_hContact,"CList","Group",&dbv))
			{
				pdnce->m_cache_tcsGroup=mir_tstrdup(dbv.ptszVal);
				//mir_free_and_nill(dbv.ptszVal);
				ModernDBFreeVariant(&dbv);
			}else
			{
				pdnce->m_cache_tcsGroup=mir_tstrdup(TEXT(""));
			}

		}
		if (pdnce->m_cache_nHidden==-1)
		{
			pdnce->m_cache_nHidden=ModernGetSettingByte(pdnce->m_cache_hContact,"CList","Hidden",0);
		}
		//if (pdnce->HiddenSubcontact==-1)
		//{
		pdnce->m_cache_nHiddenSubcontact=g_szMetaModuleName && ModernGetSettingByte(pdnce->m_cache_hContact,g_szMetaModuleName,"IsSubcontact",0);
		//pdnce->HiddenSubcontact=ServiceExists(MS_MC_GETMETACONTACT) && CallService(MS_MC_GETMETACONTACT, (WPARAM)pdnce->hContact, 0) != 0;
		//};

		if (pdnce->m_cache_nNoHiddenOffline==-1)
		{
			pdnce->m_cache_nNoHiddenOffline=ModernGetSettingByte(pdnce->m_cache_hContact,"CList","noOffline",0);
		}

		if (pdnce->IdleTS==-1)
		{
			pdnce->IdleTS=ModernGetSettingDword(pdnce->m_cache_hContact,pdnce->m_cache_cszProto,"IdleTS",0);
		};

		if (pdnce->ApparentMode==-1)
		{
			pdnce->ApparentMode=ModernGetSettingWord(pdnce->m_cache_hContact,pdnce->m_cache_cszProto,"ApparentMode",0);
		};				
		if (pdnce->NotOnList==-1)
		{
			pdnce->NotOnList=ModernGetSettingByte(pdnce->m_cache_hContact,"CList","NotOnList",0);
		};		

		if (pdnce->IsExpanded==-1)
		{
			pdnce->IsExpanded=ModernGetSettingByte(pdnce->m_cache_hContact,"CList","Expanded",0);
		}
		if (pdnce->dwLastMsgTime==0)
		{
			pdnce->dwLastMsgTime=ModernGetSettingDword(pdnce->m_cache_hContact, "CList", "mf_lastmsg", 0);
			if (pdnce->dwLastMsgTime==0) pdnce->dwLastMsgTime=CompareContacts2_getLMTime(pdnce->m_cache_hContact);
		}
#ifdef _UNICODE
		if (!pdnce->m_cache_szName) pdnce->m_cache_szName=mir_t2a(pdnce->m_cache_tcsName);
#endif
	}
}

void IvalidateDisplayNameCache(DWORD mode)
{
	if ( clistCache != NULL ) 
	{
		int i;
		for ( i = 0; i < clistCache->realCount; i++) 
		{
			PDNCE pdnce=(PDNCE)clistCache->items[i];
			if (mode&16)
			{
				InvalidateDNCEbyPointer(pdnce->m_cache_hContact,pdnce,16);
			}
		}
	}
}

void InvalidateDNCEbyPointer(HANDLE hContact,pdisplayNameCacheEntry pdnce,int SettingType)
{
	if (hContact==NULL) return;
	if (pdnce==NULL) return;
	
	if (pdnce)
	{
		if (SettingType==16)
		{
			pdnce->ssSecondLine.DestroySmileyList();
			if (pdnce->szSecondLineText) mir_free_and_nill(pdnce->szSecondLineText);
			pdnce->ssThirdLine.DestroySmileyList();
			if (pdnce->szThirdLineText) mir_free_and_nill(pdnce->szThirdLineText);
			pdnce->ssSecondLine.iMaxSmileyHeight=0;
			pdnce->ssThirdLine.iMaxSmileyHeight=0;
			pdnce->hTimeZone=NULL;
			pdnce->dwLastMsgTime=0;//CompareContacts2_getLMTime(pdnce->hContact);
			Cache_GetTimezone(NULL,pdnce->m_cache_hContact);
			SettingType&=~16;
		}

		if (SettingType==-1||SettingType==DBVT_DELETED)
		{	
			if (pdnce->m_cache_tcsName && !pdnce->isUnknown) mir_free_and_nill(pdnce->m_cache_tcsName);
			pdnce->m_cache_tcsName=NULL;
			if (pdnce->m_cache_tcsGroup) mir_free_and_nill(pdnce->m_cache_tcsGroup);
			// if (pdnce->szProto) mir_free_and_nill(pdnce->szProto);   //free proto
			pdnce->m_cache_tcsGroup=NULL;

			pdnce->m_cache_nHidden=-1;
			pdnce->m_cache_nHiddenSubcontact=-1;
			pdnce->m_cache_bProtoNotExists=FALSE;
			pdnce->m_cache_cszProto=NULL;
			pdnce___SetStatus( pdnce , 0 );
			pdnce->IdleTS=-1;
			pdnce->ApparentMode=-1;
			pdnce->NotOnList=-1;
			pdnce->isUnknown=FALSE;
			pdnce->m_cache_nNoHiddenOffline=-1;
			pdnce->IsExpanded=-1;
			return;
		}
		if (SettingType>=DBVT_WCHAR)
		{
			if (pdnce->m_cache_tcsName && !pdnce->isUnknown) mir_free_and_nill(pdnce->m_cache_tcsName);
			if (pdnce->m_cache_tcsGroup) mir_free_and_nill(pdnce->m_cache_tcsGroup);
			//if (pdnce->szProto) mir_free_and_nill(pdnce->szProto);
			pdnce->m_cache_tcsName=NULL;			
			pdnce->m_cache_tcsGroup=NULL;
			pdnce->m_cache_cszProto=NULL;
			return;
		}
		// in other cases clear all binary cache
		pdnce->m_cache_nHidden=-1;
		pdnce->m_cache_nHiddenSubcontact=-1;
		pdnce->m_cache_bProtoNotExists=FALSE;
		pdnce___SetStatus( pdnce , 0 );
		pdnce->IdleTS=-1;
		pdnce->ApparentMode=-1;
		pdnce->NotOnList=-1;
		pdnce->isUnknown=FALSE;
		pdnce->m_cache_nNoHiddenOffline=-1;
		pdnce->IsExpanded=-1;
		pdnce->dwLastMsgTime=0;//CompareContacts2_getLMTime(pdnce->hContact);
	};
};

char *GetContactCachedProtocol(HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry=NULL;
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry&&cacheEntry->m_cache_cszProto) return cacheEntry->m_cache_cszProto;

	return (NULL);
}

char* GetProtoForContact(HANDLE hContact)
{
	return (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT,(WPARAM)hContact,0);
}

int GetStatusForContact(HANDLE hContact,char *szProto)
{
	if (szProto)
		return (int)(ModernGetSettingWord((HANDLE)hContact,szProto,"Status",ID_STATUS_OFFLINE));
	else 
		return (ID_STATUS_OFFLINE);
}

TCHAR* GetNameForContact(HANDLE hContact,int flag,boolean *isUnknown)
{
	TCHAR* result = pcli->pfnGetContactDisplayName(hContact, flag | GCDNF_NOCACHE);
	BOOL itUnknown;
	if (UnknownConctactTranslatedName == NULL)
		UnknownConctactTranslatedName = TranslateT("(Unknown Contact)");
	itUnknown=lstrcmp(result ,UnknownConctactTranslatedName) == 0;
	if (itUnknown) {
		mir_free(result);
		result=UnknownConctactTranslatedName;
	}
	if (isUnknown) {
		*isUnknown = itUnknown;
	}

	return (result);
}

int GetContactInfosForSort(HANDLE hContact,char **Proto,TCHAR **Name,int *Status)
{
	pdisplayNameCacheEntry cacheEntry=NULL;
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if (cacheEntry!=NULL)
	{
		if (Proto!=NULL) *Proto=cacheEntry->m_cache_cszProto;
		if (Name!=NULL) *Name=cacheEntry->m_cache_tcsName;
		if (Status!=NULL) *Status=pdnce___GetStatus( cacheEntry );
	}
	return (0);
};


int GetContactCachedStatus(HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry=NULL;
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	return pdnce___GetStatus( cacheEntry );
}

int ContactAdded(WPARAM wParam,LPARAM lParam)
{
    if (MirandaExiting()) return 0;
	cli_ChangeContactIcon((HANDLE)wParam,ExtIconFromStatusMode((HANDLE)wParam,(char*)GetContactCachedProtocol((HANDLE)wParam),ID_STATUS_OFFLINE),1); ///by FYR
	pcli->pfnSortContacts();
	return 0;
}

int ContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	DBVARIANT dbv={0};
	pdisplayNameCacheEntry pdnce;
    if (MirandaExiting() || !pcli || !clistCache || (HANDLE)wParam == NULL) return 0;

	dbv.pszVal = NULL;
	pdnce=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry((HANDLE)wParam);

	if (pdnce==NULL)
	{
		TRACE("!!! Very bad pdnce not found.");
		//if (dbv.pszVal) mir_free_and_nill(dbv.pszVal);
		return 0;
	}
	if (pdnce->m_cache_bProtoNotExists==FALSE && pdnce->m_cache_cszProto)
	{
			if (!strcmp(cws->szModule,pdnce->m_cache_cszProto))
		{
			InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);

				if (!strcmp(cws->szSetting,"IsSubcontact"))
			{
				PostMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0);
			}
			if (!mir_strcmp(cws->szSetting, "Status") ||
					wildcmp((char*)cws->szSetting, (char*) "Status?",2))
			{
				
				if (g_szMetaModuleName && !mir_strcmp(cws->szModule,g_szMetaModuleName) && mir_strcmp(cws->szSetting, "Status"))
				{
					int res=0;
					//InvalidateDisplayNameCacheEntryByPDNE((HANDLE)wParam,pdnce,cws->value.type);
					if (pcli->hwndContactTree && g_flag_bOnModulesLoadedCalled) 
						res=PostAutoRebuidMessage(pcli->hwndContactTree);
					if ((ModernGetSettingWord(NULL,"CList","SecondLineType",SETTING_SECONDLINE_TYPE_DEFAULT)==TEXT_STATUS_MESSAGE||ModernGetSettingWord(NULL,"CList","ThirdLineType",SETTING_THIRDLINE_TYPE_DEFAULT)==TEXT_STATUS_MESSAGE) &&pdnce->m_cache_hContact && pdnce->m_cache_cszProto)
					{
						//	if (pdnce->status!=ID_STATUS_OFFLINE)  
						amRequestAwayMsg((HANDLE)wParam);  
					}
					ModernDBFreeVariant(&dbv);
					return 0;
				}
				if (!(pdnce->m_cache_nHidden==1)) 
				{		
					pdnce___SetStatus( pdnce , cws->value.wVal ); //dont use direct set
					if (cws->value.wVal == ID_STATUS_OFFLINE) 
					{
						if (g_CluiData.bRemoveAwayMessageForOffline)
						{
							ModernWriteSettingString((HANDLE)wParam,"CList","StatusMsg","");
						}
					}
					if ((ModernGetSettingWord(NULL,"CList","SecondLineType",0)==TEXT_STATUS_MESSAGE||ModernGetSettingWord(NULL,"CList","ThirdLineType",0)==TEXT_STATUS_MESSAGE) &&pdnce->m_cache_hContact && pdnce->m_cache_cszProto)
					{
						//	if (pdnce->status!=ID_STATUS_OFFLINE)  
						amRequestAwayMsg((HANDLE)wParam);  
					}
					pcli->pfnClcBroadcast( INTM_STATUSCHANGED,wParam,0);
					cli_ChangeContactIcon((HANDLE)wParam, ExtIconFromStatusMode((HANDLE)wParam,cws->szModule, cws->value.wVal), 0); //by FYR
					pcli->pfnSortContacts();
				}
				else 
				{
					if (!(!mir_strcmp(cws->szSetting, "LogonTS")
						||!mir_strcmp(cws->szSetting, "TickTS")
						||!mir_strcmp(cws->szSetting, "InfoTS")
						))
					{
							pcli->pfnSortContacts();
					}
					ModernDBFreeVariant(&dbv);
					return 0;
				}
			}
		}

		if(!strcmp(cws->szModule,"CList")) 
		{
			//name is null or (setting is myhandle)
            if (!strcmp(cws->szSetting,"Rate"))
            {
                pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
            }
			else if (pdnce->m_cache_tcsName==NULL || !strcmp(cws->szSetting,"MyHandle"))
			{
				InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);
			}
			else if (!strcmp(cws->szSetting,"Group")) 
			{
				InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);
			}
			else if (!strcmp(cws->szSetting,"Hidden")) 
			{
				InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);		
				if(cws->value.type==DBVT_DELETED || cws->value.bVal==0) 
				{
					char *szProto=(char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
					//				ChangeContactIcon((HANDLE)wParam,IconFromStatusMode(szProto,szProto==NULL?ID_STATUS_OFFLINE:DBGetContactSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE)),1);
					cli_ChangeContactIcon((HANDLE)wParam,ExtIconFromStatusMode((HANDLE)wParam,szProto,szProto==NULL?ID_STATUS_OFFLINE:ModernGetSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE)),1);  //by FYR
				}
				pcli->pfnClcBroadcast(CLM_AUTOREBUILD,0, 0);
			}
			else if(!strcmp(cws->szSetting,"noOffline")) 
			{
				InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);		
                pcli->pfnClcBroadcast(CLM_AUTOREBUILD,0, 0);
			}
		}
		else if(!strcmp(cws->szModule,"Protocol")) 
		{
			if(!strcmp(cws->szSetting,"p")) 
			{
				char *szProto;
				InvalidateDNCEbyPointer((HANDLE)wParam,pdnce,cws->value.type);	
				if(cws->value.type==DBVT_DELETED) szProto=NULL;
				else szProto=cws->value.pszVal;
				cli_ChangeContactIcon((HANDLE)wParam,ExtIconFromStatusMode((HANDLE)wParam,szProto,szProto==NULL?ID_STATUS_OFFLINE:ModernGetSettingWord((HANDLE)wParam,szProto,"Status",ID_STATUS_OFFLINE)),0); //by FYR
			}
		}
		// Clean up
		ModernDBFreeVariant(&dbv);
		//if (dbv.pszVal)
		//	mir_free_and_nill(dbv.pszVal);
	} 
	
	return 0;
}

int PostAutoRebuidMessage(HWND hwnd)
{
	if (!CLM_AUTOREBUILD_WAS_POSTED)
		CLM_AUTOREBUILD_WAS_POSTED=PostMessage(hwnd,CLM_AUTOREBUILD,0,0);
	return CLM_AUTOREBUILD_WAS_POSTED;
}
