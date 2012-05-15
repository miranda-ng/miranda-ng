/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "clc.h"

struct CListEvent
{
	int imlIconIndex;
	int flashesDone;
	CLISTEVENT cle;
};

struct CListImlIcon
{
	int index;
	HICON hIcon;
};
static struct CListImlIcon *imlIcon;
static int imlIconCount;

extern HIMAGELIST hCListImages;

static UINT_PTR flashTimerId;
static int iconsOn;
static int disableTrayFlash;
static int disableIconFlash;

int fnGetImlIconIndex(HICON hIcon)
{
	int i;

	for (i = 0; i < imlIconCount; i++) {
		if (imlIcon[i].hIcon == hIcon)
			return imlIcon[i].index;
	}
	imlIcon = (struct CListImlIcon *) mir_realloc(imlIcon, sizeof(struct CListImlIcon) * (imlIconCount + 1));
	imlIconCount++;
	imlIcon[i].hIcon = hIcon;
	imlIcon[i].index = ImageList_AddIcon(hCListImages, hIcon);
	return imlIcon[i].index;
}

static char * GetEventProtocol(int idx)
{
	if (cli.events.count && idx>=0 && idx<cli.events.count)
	{
		char *szProto;
		if (cli.events.items[idx]->cle.hContact == NULL)
		{
			if (cli.events.items[idx]->cle.flags&CLEF_PROTOCOLGLOBAL)
				szProto = cli.events.items[idx]->cle.lpszProtocol;
			else
				szProto = NULL;
		}
		else
			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) cli.events.items[idx]->cle.hContact, 0);
		return szProto;
	}
	return NULL;
}

static void ShowOneEventInTray(int idx)
{
	cli.pfnTrayIconUpdateWithImageList((iconsOn || disableTrayFlash) ? cli.events.items[idx]->imlIconIndex : 0, cli.events.items[idx]->cle.ptszTooltip, GetEventProtocol(idx));
}

static void ShowEventsInTray()
{
	int i;
	char ** pTrayProtos;
	char nTrayProtoCnt;
	int nTrayCnt=cli.trayIconCount;
	if (!cli.events.count || !nTrayCnt)  return; 
	if (cli.events.count ==1 || nTrayCnt == 1) 
	{ 
		ShowOneEventInTray(0); //for only one icon in tray show topmost event
		return;
	}
		
	// in case if we have several icons in tray and several events with different protocols 
	// lets use several icon to show events from protocols in different icons
	cli.pfnLockTray();
	pTrayProtos = (char**)_alloca(sizeof(char*)*cli.trayIconCount);
	nTrayProtoCnt=0;
	for (i=0; i<cli.trayIconCount; i++)
	{
	   if (cli.trayIcon[i].id == 0 || !cli.trayIcon[i].szProto) continue;
	   pTrayProtos[nTrayProtoCnt++]=cli.trayIcon[i].szProto;
	}
	for (i=0; i<cli.events.count; i++)
	{
		char * iEventProto=GetEventProtocol(i);
		{
			int j;
			for (j=0; j<nTrayProtoCnt; j++)
				if ( iEventProto && pTrayProtos[j] && !lstrcmpA(pTrayProtos[j],iEventProto))
					break;
			if ( j>=nTrayProtoCnt )  j=0;	//event was not found so assume first icon
			if ( pTrayProtos[j] )		//if not already set
				   ShowOneEventInTray(i);		//show it
			pTrayProtos[j]=NULL;		//and clear slot
		}
	}
	cli.pfnUnlockTray();	
}

static VOID CALLBACK IconFlashTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	int i, j;
	ShowEventsInTray();
	for (i = 0; i < cli.events.count; i++) {
		for (j = 0; j < i; j++)
			if (cli.events.items[j]->cle.hContact == cli.events.items[i]->cle.hContact)
				break;
		if (j >= i)
			cli.pfnChangeContactIcon(cli.events.items[i]->cle.hContact, iconsOn || disableIconFlash ? cli.events.items[i]->imlIconIndex : 0, 0);
		//decrease eflashes in any case - no need to collect all events
		if (cli.events.items[i]->cle.flags & CLEF_ONLYAFEW) {
			if (0 >= --cli.events.items[i]->flashesDone)
				cli.pfnRemoveEvent( cli.events.items[i]->cle.hContact, cli.events.items[i]->cle.hDbEvent);
	}	}

	if (cli.events.count == 0) {
		KillTimer(NULL, idEvent);
		cli.pfnTrayIconSetToBase( NULL );
	}

	iconsOn = !iconsOn;
}

struct CListEvent* fnAddEvent( CLISTEVENT *cle )
{
	int i;
	struct CListEvent* p;

	if (cle==NULL || cle->cbSize != sizeof(CLISTEVENT))
		return NULL;

	if (cle->flags & CLEF_URGENT) {
		for (i = 0; i < cli.events.count; i++)
			if (!(cli.events.items[i]->cle.flags & CLEF_URGENT))
				break;
	}
	else i = cli.events.count;

	if (( p = ( struct CListEvent* )cli.pfnCreateEvent()) == NULL )
		return NULL;

	List_Insert(( SortedList* )&cli.events, p, i );
	p->cle = *cle;
	p->imlIconIndex = fnGetImlIconIndex(cli.events.items[i]->cle.hIcon);
	p->flashesDone = 12;
	p->cle.pszService = mir_strdup(cli.events.items[i]->cle.pszService);
	#if defined( _UNICODE )
		if (p->cle.flags & CLEF_UNICODE)
			p->cle.ptszTooltip = mir_tstrdup((TCHAR*)p->cle.ptszTooltip);
		else
			p->cle.ptszTooltip = mir_a2u((char*)p->cle.pszTooltip); //if no flag defined it handled as unicode
	#else
		p->cle.ptszTooltip = mir_tstrdup(p->cle.ptszTooltip); 
	#endif	
	if (cli.events.count == 1) {
		char *szProto;
		if (cle->hContact == NULL)
		{
			if (cle->flags&CLEF_PROTOCOLGLOBAL)
				szProto = cle->lpszProtocol;
			else
				szProto=NULL;
		}
		else
			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)cle->hContact, 0);
		iconsOn = 1;
		flashTimerId = SetTimer(NULL, 0, DBGetContactSettingWord(NULL, "CList", "IconFlashTime", 550), IconFlashTimer);
		cli.pfnTrayIconUpdateWithImageList( p->imlIconIndex, p->cle.ptszTooltip, szProto);
	}
	cli.pfnChangeContactIcon(cle->hContact, p->imlIconIndex, 1);
	cli.pfnSortContacts();
	return p;
}

// Removes an event from the contact list's queue
// Returns 0 if the event was successfully removed, or nonzero if the event was not found
int fnRemoveEvent( HANDLE hContact, HANDLE dbEvent )
{
	int i;
	char *szProto;
	int nSameProto=0;

	// Find the event that should be removed
	for (i = 0; i < cli.events.count; i++)
		if ((cli.events.items[i]->cle.hContact == hContact) && (cli.events.items[i]->cle.hDbEvent == dbEvent))
			break;

	// Event was not found
	if (i == cli.events.count)
		return 1;

	// Update contact's icon
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	cli.pfnChangeContactIcon(cli.events.items[i]->cle.hContact,
		CallService(MS_CLIST_GETCONTACTICON, (WPARAM)cli.events.items[i]->cle.hContact, 1),
		0);

	// Free any memory allocated to the event
	cli.pfnFreeEvent( cli.events.items[i] );
	List_Remove(( SortedList* )&cli.events, i );
	{
		//count same protocoled events
		char * szEventProto;
		for (i = 0; i < cli.events.count; i++)
		{
			if (cli.events.items[i]->cle.hContact)
				szEventProto=(char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)(cli.events.items[i]->cle.hContact), 0);
			else if (cli.events.items[i]->cle.flags&CLEF_PROTOCOLGLOBAL) 
				szEventProto=(char *) cli.events.items[i]->cle.lpszProtocol;
			else 
				szEventProto = NULL;
			if (szEventProto && szProto && !lstrcmpA(szEventProto,szProto))
				nSameProto++;

		}

	}
	if (cli.events.count == 0 || nSameProto == 0) {
		if (cli.events.count == 0) 
			KillTimer(NULL, flashTimerId);
		cli.pfnTrayIconSetToBase( hContact == NULL ? NULL : szProto);
	}
	else {
		if (cli.events.items[0]->cle.hContact == NULL)
			szProto = NULL;
		else
			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) cli.events.items[0]->cle.hContact, 0);
		cli.pfnTrayIconUpdateWithImageList(iconsOn ? cli.events.items[0]->imlIconIndex : 0, cli.events.items[0]->cle.ptszTooltip, szProto);
	}

	return 0;
}

CLISTEVENT* fnGetEvent( HANDLE hContact, int idx )
{
	if ( hContact == INVALID_HANDLE_VALUE) {
		if (idx >= cli.events.count)
			return NULL;
		return &cli.events.items[idx]->cle;
	}

	for (int i = 0; i < cli.events.count; i++)
		if (cli.events.items[i]->cle.hContact == hContact)
			if (idx-- == 0)
				return &cli.events.items[i]->cle;
	return NULL;
}

int fnEventsProcessContactDoubleClick(HANDLE hContact)
{
	for (int i = 0; i < cli.events.count; i++) {
		if (cli.events.items[i]->cle.hContact == hContact) {
			HANDLE hDbEvent = cli.events.items[i]->cle.hDbEvent;
			CallService(cli.events.items[i]->cle.pszService, (WPARAM) (HWND) NULL, (LPARAM) & cli.events.items[i]->cle);
			cli.pfnRemoveEvent(hContact, hDbEvent);
			return 0;
	}	}

	return 1;
}

int fnEventsProcessTrayDoubleClick(int index)
{
	BOOL click_in_first_icon=FALSE;
	if (cli.events.count) {
		HANDLE hContact, hDbEvent;
		int eventIndex=0;
		cli.pfnLockTray();
		if (cli.trayIconCount>1 && index>0)	{
			int i;
			char * szProto=NULL;
			for (i=0; i<cli.trayIconCount; i++)
				if (cli.trayIcon[i].id==index)	{
					szProto=cli.trayIcon[i].szProto;
					if (i==0) click_in_first_icon=TRUE;
					break;
				}
			if (szProto) {
				for(i=0; i<cli.events.count; i++) {
					char * eventProto=NULL;				
					if (cli.events.items[i]->cle.hContact) 
						eventProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)cli.events.items[i]->cle.hContact, 0);
					if (!eventProto)
						eventProto=cli.events.items[i]->cle.lpszProtocol;
		
					if (!eventProto || !_strcmpi(eventProto, szProto))	{
						eventIndex=i;
						break;
				}	}
				
				if (i==cli.events.count) { //EventNotFound
					//lets  process backward try to find first event without desired proto in tray
					int j;
					if (click_in_first_icon)
						for(i=0; i<cli.events.count; i++) {
							char * eventProto=NULL;				
							if (cli.events.items[i]->cle.hContact) 
								eventProto=(char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)cli.events.items[i]->cle.hContact, 0);
							if (!eventProto)
								eventProto=cli.events.items[i]->cle.lpszProtocol;
							if (eventProto) {
								for (j=0; j<cli.trayIconCount; j++)
									if (cli.trayIcon[j].szProto && !_strcmpi(eventProto, cli.trayIcon[j].szProto))
										break;
								
								if (j==cli.trayIconCount) {
									eventIndex=i;
									break;
						}	}	}
					if (i==cli.events.count) { //not found 
						cli.pfnUnlockTray();
						return 1;	//continue processing to show contact list
		}	}	}	}

		cli.pfnUnlockTray();		
		hContact = cli.events.items[eventIndex]->cle.hContact;
		hDbEvent = cli.events.items[eventIndex]->cle.hDbEvent;
		//if (!ServiceExists(cli.events.items[eventIndex]->cle.pszService))
		//	; may be better to show send msg?
		CallService(cli.events.items[eventIndex]->cle.pszService, (WPARAM) NULL, (LPARAM) & cli.events.items[eventIndex]->cle);
		cli.pfnRemoveEvent(hContact, hDbEvent);
		return 0;
	}
	return 1;
}

static int RemoveEventsForContact(WPARAM wParam, LPARAM)
{
	int j, hit;

	/*
	the for(;;) loop is used here since the cli.events.count can not be relied upon to take us
	thru the cli.events.items[] array without suffering from shortsightedness about how many unseen
	events remain, e.g. three events, we remove the first, we're left with 2, the event
	loop exits at 2 and we never see the real new 2.
	*/

	for (; cli.events.count > 0;) {
		for (hit = 0, j = 0; j < cli.events.count; j++) {
			if (cli.events.items[j]->cle.hContact == (HANDLE) wParam) {
				cli.pfnRemoveEvent((HANDLE)wParam, cli.events.items[j]->cle.hDbEvent);
				hit = 1;
			}
		}
		if (j == cli.events.count && hit == 0)
			return 0;           /* got to the end of the array and didnt remove anything */
	}

	return 0;
}

static int CListEventSettingsChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	if (hContact == NULL && cws && cws->szModule && cws->szSetting && strcmp(cws->szModule, "CList") == 0) {
		if (strcmp(cws->szSetting, "DisableTrayFlash") == 0)
			disableTrayFlash = (int) cws->value.bVal;
		else if (strcmp(cws->szSetting, "NoIconBlink") == 0)
			disableIconFlash = (int) cws->value.bVal;
	}
	return 0;
}

/***************************************************************************************/

INT_PTR AddEventSyncStub(WPARAM wParam, LPARAM lParam) { return CallServiceSync(MS_CLIST_ADDEVENT"_SYNC",wParam, lParam); }
INT_PTR AddEventStub(WPARAM, LPARAM lParam) { return cli.pfnAddEvent((CLISTEVENT*)lParam ) == NULL; }
INT_PTR RemoveEventStub(WPARAM wParam, LPARAM lParam) { return cli.pfnRemoveEvent((HANDLE)wParam,(HANDLE)lParam ); }
INT_PTR GetEventStub(WPARAM wParam, LPARAM lParam) { return (INT_PTR)cli.pfnGetEvent((HANDLE)wParam,(int)lParam); }

int InitCListEvents(void)
{
	memset( &cli.events, 0, sizeof(cli.events));	
	cli.events.increment = 10;
	
	disableTrayFlash = DBGetContactSettingByte(NULL, "CList", "DisableTrayFlash", 0);
	disableIconFlash = DBGetContactSettingByte(NULL, "CList", "NoIconBlink", 0);
	CreateServiceFunction(MS_CLIST_ADDEVENT, AddEventSyncStub); //need to be called through sync to keep flash timer workable
	CreateServiceFunction(MS_CLIST_ADDEVENT"_SYNC", AddEventStub);
	CreateServiceFunction(MS_CLIST_REMOVEEVENT, RemoveEventStub);
	CreateServiceFunction(MS_CLIST_GETEVENT, GetEventStub);
	HookEvent(ME_DB_CONTACT_DELETED, RemoveEventsForContact);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, CListEventSettingsChanged);
	return 0;
}

struct CListEvent* fnCreateEvent( void )
{
	return (struct CListEvent*)mir_calloc( sizeof(struct CListEvent));
}

void fnFreeEvent( struct CListEvent* p )
{
   if ( p->cle.pszService )
      mir_free( p->cle.pszService );
   if ( p->cle.pszTooltip )
      mir_free( p->cle.pszTooltip );
	mir_free( p );
}

void UninitCListEvents(void)
{
	int i;

	if (cli.events.count) KillTimer(NULL, flashTimerId);

	for (i = 0; i < cli.events.count; i++)
		cli.pfnFreeEvent(( struct CListEvent* )cli.events.items[i] );
	List_Destroy(( SortedList* )&cli.events );

	if ( imlIcon != NULL )
		mir_free( imlIcon );
}
