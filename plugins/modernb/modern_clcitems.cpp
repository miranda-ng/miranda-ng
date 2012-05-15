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
#include "m_clc.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_clist.h"
#include "m_api/m_metacontacts.h"
#include "hdr/modern_commonprototypes.h"

void AddSubcontacts(struct ClcData *dat, struct ClcContact * cont, BOOL showOfflineHereGroup)
{
	int subcount,i,j;
	HANDLE hsub;
	pdisplayNameCacheEntry cacheEntry;
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(cont->hContact);
	cont->SubExpanded=(ModernGetSettingByte(cont->hContact,"CList","Expanded",0) && (ModernGetSettingByte(NULL,"CLC","MetaExpanding",SETTING_METAEXPANDING_DEFAULT)));
	subcount=(int)CallService(MS_MC_GETNUMCONTACTS,(WPARAM)cont->hContact,0);

	if (subcount <= 0) {
		cont->isSubcontact=0;
		cont->subcontacts=NULL;
		cont->SubAllocated=0;
		return;
	}

	cont->isSubcontact=0;
    mir_free(cont->subcontacts);
	cont->subcontacts=(struct ClcContact *) mir_calloc(sizeof(struct ClcContact)*subcount);
	cont->SubAllocated=subcount;
	i=0;
	for (j=0; j<subcount; j++) {
		WORD wStatus;
		
		hsub=(HANDLE)CallService(MS_MC_GETSUBCONTACT,(WPARAM)cont->hContact,j);
		cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hsub);
		wStatus=pdnce___GetStatus( cacheEntry );
		if (showOfflineHereGroup||(!(ModernGetSettingByte(NULL,"CLC","MetaHideOfflineSub",SETTING_METAHIDEOFFLINESUB_DEFAULT) && ModernGetSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT) ) ||
			wStatus!=ID_STATUS_OFFLINE )
			//&&
			//(!cacheEntry->Hidden || style&CLS_SHOWHIDDEN)
			)

		{
			cont->subcontacts[i].hContact=cacheEntry->m_cache_hContact;

			cont->subcontacts[i].avatar_pos = AVATAR_POS_DONT_HAVE;
			Cache_GetAvatar(dat, &cont->subcontacts[i]);

			cont->subcontacts[i].iImage=CallService(MS_CLIST_GETCONTACTICON,(WPARAM)cacheEntry->m_cache_hContact,1);
			memset(cont->subcontacts[i].iExtraImage,0xFF,sizeof(cont->subcontacts[i].iExtraImage));
			memset((void*)cont->subcontacts[i].iWideExtraImage,0xFF,sizeof(cont->subcontacts[i].iWideExtraImage));
			cont->subcontacts[i].proto=cacheEntry->m_cache_cszProto;		
			cont->subcontacts[i].type=CLCIT_CONTACT;
			cont->subcontacts[i].flags=0;//CONTACTF_ONLINE;
			cont->subcontacts[i].isSubcontact=i+1;
            cont->subcontacts[i].lastPaintCounter=0;
			cont->subcontacts[i].subcontacts=cont;
			cont->subcontacts[i].image_is_special=FALSE;
			//cont->subcontacts[i].status=cacheEntry->status;
			Cache_GetTimezone(dat, (&cont->subcontacts[i])->hContact);
			Cache_GetText(dat, &cont->subcontacts[i],1);

			{
				int apparentMode;
				char *szProto;  
				int idleMode;
				szProto=cacheEntry->m_cache_cszProto;
				if(szProto!=NULL && !pcli->pfnIsHiddenMode(dat,wStatus))
					cont->subcontacts[i].flags|=CONTACTF_ONLINE;
				apparentMode=szProto!=NULL?cacheEntry->ApparentMode:0;
				apparentMode=szProto!=NULL?cacheEntry->ApparentMode:0;
				if(apparentMode==ID_STATUS_OFFLINE)	cont->subcontacts[i].flags|=CONTACTF_INVISTO;
				else if(apparentMode==ID_STATUS_ONLINE) cont->subcontacts[i].flags|=CONTACTF_VISTO;
				else if(apparentMode) cont->subcontacts[i].flags|=CONTACTF_VISTO|CONTACTF_INVISTO;
				if(cacheEntry->NotOnList) cont->subcontacts[i].flags|=CONTACTF_NOTONLIST;
				idleMode=szProto!=NULL?cacheEntry->IdleTS:0;
				if (idleMode) cont->subcontacts[i].flags|=CONTACTF_IDLE;
            }
			i++;
		}	}

	cont->SubAllocated=i;
	if (!i && cont->subcontacts != NULL) mir_free_and_nill(cont->subcontacts);
}

int cli_AddItemToGroup(struct ClcGroup *group,int iAboveItem)
{
	if ( group == NULL ) return 0;

	iAboveItem = corecli.pfnAddItemToGroup( group, iAboveItem );
	ClearRowByIndexCache();
	return iAboveItem;
}

struct ClcGroup *cli_AddGroup(HWND hwnd,struct ClcData *dat,const TCHAR *szName,DWORD flags,int groupId,int calcTotalMembers)
{
	struct ClcGroup* result;
	ClearRowByIndexCache();	
	if (!dat->force_in_dialog && !(GetWindowLong(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN))
		if (!lstrcmp(_T("-@-HIDDEN-GROUP-@-"),szName))        //group is hidden
		{   	
			ClearRowByIndexCache();
			return NULL;
		}
		result = corecli.pfnAddGroup( hwnd, dat, szName, flags, groupId, calcTotalMembers);
		/* ToDo: fix some times valid contact with valid group are placed to root
		if ( result == NULL )
		{
			result = &dat->list;
		}
		*/
		ClearRowByIndexCache();
		return result;
}

void cli_FreeContact(struct ClcContact *p)
{
	if ( p->SubAllocated) {
		if ( p->subcontacts && !p->isSubcontact) {
			int i;
			for ( i = 0 ; i < p->SubAllocated ; i++ ) {
				p->subcontacts[i].ssText.DestroySmileyList();
				if ( p->subcontacts[i].avatar_pos==AVATAR_POS_ANIMATED )
					AniAva_RemoveAvatar( p->subcontacts[i].hContact );
					p->subcontacts[i].avatar_pos=AVATAR_POS_DONT_HAVE;
			}
			mir_free_and_nill(p->subcontacts);
	}	}

	p->ssText.DestroySmileyList();
	if ( p->avatar_pos==AVATAR_POS_ANIMATED )
		AniAva_RemoveAvatar( p->hContact );
	p->avatar_pos=AVATAR_POS_DONT_HAVE;
	corecli.pfnFreeContact( p );
}

void cli_FreeGroup( struct ClcGroup* group )
{
	corecli.pfnFreeGroup( group );
	ClearRowByIndexCache();
}

int cli_AddInfoItemToGroup(struct ClcGroup *group,int flags,const TCHAR *pszText)
{
	int i = corecli.pfnAddInfoItemToGroup( group, flags, pszText );
	ClearRowByIndexCache();
	return i;
}

static void _LoadDataToContact(struct ClcContact * cont, struct ClcGroup *group, struct ClcData *dat, HANDLE hContact)
{
	pdisplayNameCacheEntry cacheEntry=NULL;
	WORD apparentMode;
	DWORD idleMode;
	char * szProto;

	if (!cont) return;
	cont->type=CLCIT_CONTACT;
	cont->SubAllocated=0;
	cont->isSubcontact=0;
	cont->subcontacts=NULL;
	cont->szText[0]=0;
	cont->lastPaintCounter=0;
	cont->image_is_special=FALSE;
	cont->hContact=hContact;

	pcli->pfnInvalidateDisplayNameCacheEntry(hContact);	
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);	
	
	szProto=cacheEntry->m_cache_cszProto;
	cont->proto=szProto;

	if(szProto!=NULL&&!pcli->pfnIsHiddenMode(dat,pdnce___GetStatus( cacheEntry )))
		cont->flags |= CONTACTF_ONLINE;
	
	apparentMode=szProto!=NULL?cacheEntry->ApparentMode:0;
	
	if (apparentMode)
		switch (apparentMode)
		{
			case ID_STATUS_OFFLINE:
				cont->flags|=CONTACTF_INVISTO;
				break;
			case ID_STATUS_ONLINE:
				cont->flags|=CONTACTF_VISTO;
				break;
			default:
				cont->flags|=CONTACTF_VISTO|CONTACTF_INVISTO;
		}
	
	if(cacheEntry->NotOnList) 
		cont->flags|=CONTACTF_NOTONLIST;
	idleMode=szProto!=NULL?cacheEntry->IdleTS:0;
	
	if (idleMode) 
		cont->flags|=CONTACTF_IDLE;
	

	//Add subcontacts
	if (szProto)
	{	
		if ( g_szMetaModuleName && dat->IsMetaContactsEnabled && mir_strcmp(cont->proto,g_szMetaModuleName)==0) 
			AddSubcontacts(dat,cont,CLCItems_IsShowOfflineGroup(group));
	}
	cont->lastPaintCounter=0;
	cont->avatar_pos=AVATAR_POS_DONT_HAVE;
	Cache_GetAvatar(dat,cont);
	Cache_GetText(dat,cont,1);
	Cache_GetTimezone(dat,cont->hContact);
	cont->iImage=CallService(MS_CLIST_GETCONTACTICON,(WPARAM)hContact,1);
	cont->bContactRate=ModernGetSettingByte(hContact, "CList", "Rate",0);
}

static struct ClcContact * AddContactToGroup(struct ClcData *dat,struct ClcGroup *group, pdisplayNameCacheEntry cacheEntry)
{
	HANDLE hContact;
	int i;
	if (cacheEntry==NULL) return NULL;
	if (group==NULL) return NULL;
	if (dat==NULL) return NULL;
	hContact=cacheEntry->m_cache_hContact;
	dat->NeedResort=1;
	for(i=group->cl.count-1;i>=0;i--)
		if(group->cl.items[i]->type!=CLCIT_INFO || !(group->cl.items[i]->flags&CLCIIF_BELOWCONTACTS)) break;
	i=cli_AddItemToGroup(group,i+1);

	_LoadDataToContact(group->cl.items[i], group, dat, hContact);
	cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	ClearRowByIndexCache();
	return group->cl.items[i];
}

void * AddTempGroup(HWND hwnd,struct ClcData *dat,const TCHAR *szName,DWORD flags,int groupId,int calcTotalMembers)
{
	int i=0;
	int f=0;
	TCHAR * szGroupName;
	DWORD groupFlags;
#ifdef UNICODE
	char *mbuf=mir_u2a((TCHAR *)szName);
#else
	char *mbuf=mir_strdup((char *)szName);
#endif
	if (wildcmp(mbuf,"-@-HIDDEN-GROUP-@-",0))
	{
		mir_free_and_nill(mbuf);
		return NULL;
	} 
	mir_free_and_nill(mbuf);
	for(i=1;;i++) 
	{
		szGroupName = pcli->pfnGetGroupName(i,&groupFlags);
		if(szGroupName==NULL) break;
		if (!mir_tstrcmpi(szGroupName,szName)) f=1;
	}
	if (!f)
	{
		char buf[20];
		TCHAR b2[255];
		void * res=NULL;
		mir_snprintf(buf,SIZEOF(buf),"%d",(i-1));
		mir_sntprintf(b2,SIZEOF(b2),_T("#%s"),szName);
		b2[0]=1|GROUPF_EXPANDED;
		ModernWriteSettingTString(NULL,"CListGroups",buf,b2);
		pcli->pfnGetGroupName(i,&groupFlags);      
		res=cli_AddGroup(hwnd,dat,szName,groupFlags,i,0);
		return res;
	}
	return NULL;
}

void cli_AddContactToTree(HWND hwnd,struct ClcData *dat,HANDLE hContact,int updateTotalCount,int checkHideOffline)
{
	struct ClcGroup *group;
	struct ClcContact * cont;
	pdisplayNameCacheEntry cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);
	if(dat->IsMetaContactsEnabled && cacheEntry && cacheEntry->m_cache_nHiddenSubcontact) return;		//contact should not be added
	if(!dat->IsMetaContactsEnabled && cacheEntry && g_szMetaModuleName && !mir_strcmp(cacheEntry->m_cache_cszProto,g_szMetaModuleName)) return;
	corecli.pfnAddContactToTree(hwnd,dat,hContact,updateTotalCount,checkHideOffline);
	if (FindItem(hwnd,dat,hContact,&cont,&group,NULL,FALSE))
		_LoadDataToContact(cont, group, dat, hContact);
	return;
}

void cli_DeleteItemFromTree(HWND hwnd,HANDLE hItem)
{
	struct ClcData *dat = (struct ClcData *) GetWindowLongPtr(hwnd, 0);
	ClearRowByIndexCache();
	corecli.pfnDeleteItemFromTree(hwnd, hItem);

	//check here contacts are not resorting
	if (hwnd==pcli->hwndContactTree)
		pcli->pfnFreeCacheItem(pcli->pfnGetCacheEntry(hItem)); 
	dat->NeedResort=1;
	ClearRowByIndexCache();
}


__inline BOOL CLCItems_IsShowOfflineGroup(struct ClcGroup* group)
{
	DWORD groupFlags=0;
	if (!group) return FALSE;
	if (group->hideOffline) return FALSE;
	pcli->pfnGetGroupName(group->groupId,&groupFlags);
	return (groupFlags&GROUPF_SHOWOFFLINE)!=0;
}

HANDLE SaveSelection( struct ClcData *dat )
{
	ClcContact * selcontact = NULL;

	if ( pcli->pfnGetRowByIndex( dat, dat->selection, &selcontact, NULL ) == -1 )
		return NULL;
	else
		return pcli->pfnContactToHItem( selcontact );
}

int RestoreSelection( struct ClcData *dat, HANDLE hSelected )
{
	ClcContact * selcontact = NULL;
	ClcGroup   * selgroup   = NULL;

	if ( !hSelected || !pcli->pfnFindItem( dat->hWnd, dat, hSelected, &selcontact, &selgroup, NULL) )
	{
		dat->selection = -1;
		return dat->selection;
	}

	if ( !selcontact->isSubcontact )
	{
		dat->selection = pcli->pfnGetRowsPriorTo( &dat->list, selgroup, li.List_IndexOf((SortedList*)&selgroup->cl, selcontact ) );
	}
	else
	{ 
		dat->selection = pcli->pfnGetRowsPriorTo(&dat->list, selgroup, li.List_IndexOf((SortedList*)&selgroup->cl, selcontact->subcontacts ) );
	
		if (dat->selection != -1 ) 
			dat->selection += selcontact->isSubcontact;
	}
	return dat->selection;

}

void cliRebuildEntireList(HWND hwnd,struct ClcData *dat)
{
	DWORD style=GetWindowLong(hwnd,GWL_STYLE);
	HANDLE hContact;
	struct ClcContact * cont;
	struct ClcGroup *group;
    static int rebuildCounter=0;

    BOOL PlaceOfflineToRoot=ModernGetSettingByte(NULL,"CList","PlaceOfflineToRoot",SETTING_PLACEOFFLINETOROOT_DEFAULT);
	KillTimer(hwnd,TIMERID_REBUILDAFTER);
	
	ClearRowByIndexCache();
	ImageArray_Clear(&dat->avatar_cache);
	RowHeights_Clear(dat);
	RowHeights_GetMaxRowHeight(dat, hwnd);
    TRACEVAR("Rebuild Entire List %d times\n",++rebuildCounter);
  
	dat->list.expanded=1;
	dat->list.hideOffline=ModernGetSettingByte(NULL,"CLC","HideOfflineRoot",SETTING_HIDEOFFLINEATROOT_DEFAULT) && style&CLS_USEGROUPS;
	dat->list.cl.count = dat->list.cl.limit = 0;
	dat->list.cl.increment = 50;
	dat->NeedResort=1;

	HANDLE hSelected = SaveSelection( dat );
	dat->selection=-1;
	dat->HiLightMode=ModernGetSettingByte(NULL,"CLC","HiLightMode",SETTING_HILIGHTMODE_DEFAULT);
	{
		int i;
		TCHAR *szGroupName;
		DWORD groupFlags;

		for(i=1;;i++) {
			szGroupName=pcli->pfnGetGroupName(i,&groupFlags); //UNICODE
			if(szGroupName==NULL) break;
			cli_AddGroup(hwnd,dat,szGroupName,groupFlags,i,0);
		}
	}

	hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hContact) 
    {
		pdisplayNameCacheEntry cacheEntry=NULL;
        int nHiddenStatus;
		cont=NULL;
		cacheEntry=(pdisplayNameCacheEntry)pcli->pfnGetCacheEntry(hContact);

		nHiddenStatus=CLVM_GetContactHiddenStatus(hContact, NULL, dat);
		if ( (style&CLS_SHOWHIDDEN && nHiddenStatus!=-1) || !nHiddenStatus)
		{

			if(lstrlen(cacheEntry->m_cache_tcsGroup)==0)
				group=&dat->list;
			else {
				group=cli_AddGroup(hwnd,dat,cacheEntry->m_cache_tcsGroup,(DWORD)-1,0,0);
			}
			if(group!=NULL) 
			{
				WORD wStatus=pdnce___GetStatus( cacheEntry );
				if (wStatus==ID_STATUS_OFFLINE)
					if (PlaceOfflineToRoot)
						group=&dat->list;
				group->totalMembers++;

				if(!(style&CLS_NOHIDEOFFLINE) && (style&CLS_HIDEOFFLINE || group->hideOffline)) 
				{
					if(cacheEntry->m_cache_cszProto==NULL) {
						if(!pcli->pfnIsHiddenMode(dat,ID_STATUS_OFFLINE)||cacheEntry->m_cache_nNoHiddenOffline || CLCItems_IsShowOfflineGroup(group))
							cont=AddContactToGroup(dat,group,cacheEntry);
					}
					else
						if(!pcli->pfnIsHiddenMode(dat,wStatus)||cacheEntry->m_cache_nNoHiddenOffline || CLCItems_IsShowOfflineGroup(group))
							cont=AddContactToGroup(dat,group,cacheEntry);
				}
				else cont=AddContactToGroup(dat,group,cacheEntry);
			}
		}
		if (cont)	
		{	
			cont->SubAllocated=0;
			if (cont->proto && g_szMetaModuleName && dat->IsMetaContactsEnabled  && strcmp(cont->proto,g_szMetaModuleName)==0)
				AddSubcontacts(dat,cont,CLCItems_IsShowOfflineGroup(group));
		}
		hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}

	if(style&CLS_HIDEEMPTYGROUPS) {
		group=&dat->list;
		group->scanIndex=0;
		for(;;) {
			if(group->scanIndex==group->cl.count) {
				group=group->parent;
				if(group==NULL) break;
			}
			else if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP) {
				if(group->cl.items[group->scanIndex]->group->cl.count==0) {
					group=pcli->pfnRemoveItemFromGroup(hwnd,group,group->cl.items[group->scanIndex],0);
				}
				else {
					group=group->cl.items[group->scanIndex]->group;
					group->scanIndex=0;
				}
				continue;
			}
			group->scanIndex++;
		}
	}

	pcli->pfnSortCLC(hwnd,dat,0);

	RestoreSelection( dat, hSelected );

}

void cli_SortCLC( HWND hwnd, struct ClcData *dat, int useInsertionSort )
{
	HANDLE hSelected = SaveSelection( dat );

	corecli.pfnSortCLC(hwnd,dat,useInsertionSort);

	RestoreSelection( dat, hSelected );
}

int GetNewSelection(struct ClcGroup *group, int selection, int direction)
{
	int lastcount=0, count=0;//group->cl.count;
	if (selection<0) {
		return 0;
	}
	group->scanIndex=0;
	for(;;) {
		if(group->scanIndex==group->cl.count) {
			group=group->parent;
			if(group==NULL) break;
			group->scanIndex++;
			continue;
		}
		if (count>=selection) return count;
		lastcount = count;
		count++;
		if (!direction) {
			if (count>selection) return lastcount;
		}
		if(group->cl.items[group->scanIndex]->type==CLCIT_GROUP && (group->cl.items[group->scanIndex]->group->expanded)) {
			group=group->cl.items[group->scanIndex]->group;
			group->scanIndex=0;
			continue;
		}
		group->scanIndex++;
	}
	return lastcount;
}

struct SavedContactState_t {
	HANDLE hContact;
	BYTE iExtraImage[MAXEXTRACOLUMNS];
	WORD iWideExtraImage[MAXEXTRACOLUMNS];
	int checked;
};

struct SavedGroupState_t {
	int groupId,expanded;
};

struct SavedInfoState_t {
	int parentId;
	struct ClcContact contact;
};

BOOL LOCK_RECALC_SCROLLBAR=FALSE;
void cli_SaveStateAndRebuildList(HWND hwnd, struct ClcData *dat)
{
	LOCK_RECALC_SCROLLBAR=TRUE;

	NMCLISTCONTROL nm;
	int i, j;
	OBJLIST<SavedGroupState_t> savedGroup( 4 );
	OBJLIST<SavedContactState_t> savedContact( 4 );
	OBJLIST<SavedInfoState_t> savedInfo( 4 );

	struct ClcGroup *group;
	struct ClcContact *contact;
	
	pcli->pfnHideInfoTip(hwnd, dat);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);
	pcli->pfnEndRename(hwnd, dat, 1);

	dat->NeedResort = 1;
	group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			
			SavedGroupState_t* p = new SavedGroupState_t;
			p->groupId = group->groupId;
			p->expanded = group->expanded;
			savedGroup.insert( p );
			continue;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) {
			SavedContactState_t* p = new SavedContactState_t;
			p->hContact = group->cl.items[group->scanIndex]->hContact;
			CopyMemory(p->iExtraImage, group->cl.items[group->scanIndex]->iExtraImage,
				sizeof(group->cl.items[group->scanIndex]->iExtraImage));
			
			CopyMemory((void*)p->iWideExtraImage, (void*)group->cl.items[group->scanIndex]->iWideExtraImage,
				sizeof(group->cl.items[group->scanIndex]->iWideExtraImage));
			
			p->checked = group->cl.items[group->scanIndex]->flags & CONTACTF_CHECKED;
			savedContact.insert( p );
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_INFO) {
			SavedInfoState_t* p = new SavedInfoState_t;
			memset( p, 0, sizeof( SavedInfoState_t ));
			if (group->parent == NULL)
				p->parentId = -1;
			else
				p->parentId = group->groupId;
			p->contact = *group->cl.items[group->scanIndex];
			savedInfo.insert( p );
		}
		group->scanIndex++;
	}

	pcli->pfnFreeGroup(&dat->list);
	pcli->pfnRebuildEntireList(hwnd, dat);

	group = &dat->list;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL)
				break;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			for (i = 0; i < savedGroup.getCount(); i++)
				if (savedGroup[i].groupId == group->groupId) {
					group->expanded = savedGroup[i].expanded;
					break;
				}
				continue;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) {
			for (i = 0; i < savedContact.getCount(); i++)
				if (savedContact[i].hContact == group->cl.items[group->scanIndex]->hContact) {
					CopyMemory(group->cl.items[group->scanIndex]->iExtraImage, savedContact[i].iExtraImage,
						sizeof(group->cl.items[group->scanIndex]->iExtraImage));
					
					CopyMemory((void*)group->cl.items[group->scanIndex]->iWideExtraImage, (void*)savedContact[i].iWideExtraImage,
						sizeof(group->cl.items[group->scanIndex]->iWideExtraImage));
					
					if (savedContact[i].checked)
						group->cl.items[group->scanIndex]->flags |= CONTACTF_CHECKED;
					break;
				}
		}
		group->scanIndex++;
	}
	savedGroup.destroy();
	savedContact.destroy();

	for (i = 0; i < savedInfo.getCount(); i++) {
		if (savedInfo[i].parentId == -1)
			group = &dat->list;
		else {
			if (!pcli->pfnFindItem(hwnd, dat, (HANDLE) (savedInfo[i].parentId | HCONTACT_ISGROUP), &contact, NULL, NULL))
				continue;
			group = contact->group;
		}
		j = pcli->pfnAddInfoItemToGroup(group, savedInfo[i].contact.flags, _T(""));
		*group->cl.items[j] = savedInfo[i].contact;
	}
	savedInfo.destroy();

	LOCK_RECALC_SCROLLBAR=FALSE;
	pcli->pfnRecalculateGroupCheckboxes(hwnd, dat);

	pcli->pfnRecalcScrollBar(hwnd, dat);
	nm.hdr.code = CLN_LISTREBUILT;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
}


WORD pdnce___GetStatus(pdisplayNameCacheEntry pdnce)
{
	if (!pdnce) 
		return ID_STATUS_OFFLINE;
	else
		return pdnce->m_cache_nStatus;
	/*
	// this stub will replace direct usage of m_cache_nStatus and will be substituted by getting info from DB directrly
	if (!pdnce) return ID_STATUS_OFFLINE;
	if (!pdnce->m_cache_cszProto) return ID_STATUS_OFFLINE;
	if (!pdnce->m_cache_hContact) return ID_STATUS_OFFLINE;
	return DBGetContactSettingWord( pdnce->m_cache_hContact, pdnce->m_cache_cszProto, "Status" , ID_STATUS_OFFLINE );
	*/
}


void pdnce___SetStatus( pdisplayNameCacheEntry pdnce, WORD wStatus )
{
	if (pdnce) pdnce->m_cache_nStatus=wStatus;
}
struct ClcContact* cliCreateClcContact( void )
{
	 struct ClcContact* contact=(struct ClcContact*)mir_calloc(sizeof( struct ClcContact ) );
	 memset((void*)contact->iWideExtraImage,0xFF,sizeof(contact->iWideExtraImage));
	 return contact;
}

ClcCacheEntryBase* cliCreateCacheItem( HANDLE hContact )
{
	pdisplayNameCacheEntry p = (pdisplayNameCacheEntry)mir_calloc(sizeof( displayNameCacheEntry ));
	if ( p )
	{
		memset(p,0,sizeof( displayNameCacheEntry ));
		p->m_cache_hContact = hContact;
		InvalidateDNCEbyPointer(hContact,p,0);
		p->szSecondLineText=NULL;
		p->szThirdLineText=NULL;
		p->ssSecondLine.plText=NULL;
		p->ssThirdLine.plText=NULL;
	}
	return (ClcCacheEntryBase*)p;
}



void cliInvalidateDisplayNameCacheEntry(HANDLE hContact)
{	
	pdisplayNameCacheEntry p;
	p = (pdisplayNameCacheEntry) pcli->pfnGetCacheEntry(hContact);
	if (p) InvalidateDNCEbyPointer(hContact,p,0);
	return;
}

char* cli_GetGroupCountsText(struct ClcData *dat, struct ClcContact *contact)
{
	char * res;
	
	res=corecli.pfnGetGroupCountsText(dat, contact);
	
	return res;
}

int cliGetGroupContentsCount(struct ClcGroup *group, int visibleOnly)
{
	int count = group->cl.count;
	struct ClcGroup *topgroup = group;

	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.count) {
			if (group == topgroup)
				break;
			group = group->parent;
		}
		else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (!(visibleOnly&0x01) || group->cl.items[group->scanIndex]->group->expanded)) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			count += group->cl.count;
			continue;
		}
        else if ((group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) && 
                 (group->cl.items[group->scanIndex]->subcontacts !=NULL)  && 
                 ((group->cl.items[group->scanIndex]->SubExpanded || (!visibleOnly))))
        {
            count+=group->cl.items[group->scanIndex]->SubAllocated;
        }
		group->scanIndex++;
	}
	return count;
}

/*
* checks the currently active view mode filter and returns true, if the contact should be hidden
* if no view mode is active, it returns the CList/Hidden setting
* also cares about sub contacts (if meta is active)
*/

int __fastcall CLVM_GetContactHiddenStatus(HANDLE hContact, char *szProto, struct ClcData *dat)
{
	int dbHidden = ModernGetSettingByte(hContact, "CList", "Hidden", 0);		// default hidden state, always respect it.
	int filterResult = 1;
	DBVARIANT dbv = {0};
	char szTemp[64];
	TCHAR szGroupMask[256];
	DWORD dwLocalMask;
    PDNCE pdnce=(PDNCE)pcli->pfnGetCacheEntry(hContact);
	BOOL fEmbedded=dat->force_in_dialog;
	// always hide subcontacts (but show them on embedded contact lists)
	
	if(g_CluiData.bMetaAvail && dat != NULL && dat->IsMetaContactsEnabled && g_szMetaModuleName && ModernGetSettingByte(hContact, g_szMetaModuleName, "IsSubcontact", 0))
		return -1; //subcontact
    if (pdnce && pdnce->isUnknown && !fEmbedded)    
        return 1; //'Unknown Contact'
	if(pdnce && g_CluiData.bFilterEffective && !fEmbedded) 
	{
		if(szProto == NULL)
			szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		// check stickies first (priority), only if we really have stickies defined (CLVM_STICKY_CONTACTS is set).
		if(g_CluiData.bFilterEffective & CLVM_STICKY_CONTACTS) 
        {
			if((dwLocalMask = ModernGetSettingDword(hContact, CLVM_MODULE, g_CluiData.current_viewmode, 0)) != 0) {
				if(g_CluiData.bFilterEffective & CLVM_FILTER_STICKYSTATUS) 
                {
					WORD wStatus = ModernGetSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
					return !((1 << (wStatus - ID_STATUS_OFFLINE)) & HIWORD(dwLocalMask));
				}
				return 0;
			}
		}
		// check the proto, use it as a base filter result for all further checks
		if(g_CluiData.bFilterEffective & CLVM_FILTER_PROTOS) {
			mir_snprintf(szTemp, SIZEOF(szTemp), "%s|", szProto);
			filterResult = strstr(g_CluiData.protoFilter, szTemp) ? 1 : 0;
		}
		if(g_CluiData.bFilterEffective & CLVM_FILTER_GROUPS) {
			if(!ModernGetSettingTString(hContact, "CList", "Group", &dbv)) {
				mir_sntprintf(szGroupMask, SIZEOF(szGroupMask), _T("%s|"), &dbv.ptszVal[0]);
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? (filterResult | (_tcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0)) : (filterResult & (_tcsstr(g_CluiData.groupFilter, szGroupMask) ? 1 : 0));
				mir_free(dbv.ptszVal);
			}
			else if(g_CluiData.filterFlags & CLVM_INCLUDED_UNGROUPED)
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 1;
			else
				filterResult = (g_CluiData.filterFlags & CLVM_PROTOGROUP_OP) ? filterResult : filterResult & 0;
		}
		if(g_CluiData.bFilterEffective & CLVM_FILTER_STATUS) {
			WORD wStatus = ModernGetSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
			filterResult = (g_CluiData.filterFlags & CLVM_GROUPSTATUS_OP) ? ((filterResult | ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0))) : (filterResult & ((1 << (wStatus - ID_STATUS_OFFLINE)) & g_CluiData.statusMaskFilter ? 1 : 0));
		}
		if(g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG) 
		{
			DWORD now;
			PDNCE pdnce=(PDNCE)pcli->pfnGetCacheEntry(hContact);
			if (pdnce)
			{
				now = g_CluiData.t_now;
				now -= g_CluiData.lastMsgFilter;
				if(g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG_OLDERTHAN)
					filterResult = filterResult & (pdnce->dwLastMsgTime < now);
				else if(g_CluiData.bFilterEffective & CLVM_FILTER_LASTMSG_NEWERTHAN)
					filterResult = filterResult & (pdnce->dwLastMsgTime > now);
			}
		}
		return (dbHidden | !filterResult);
	}
	else
		return dbHidden;
}
