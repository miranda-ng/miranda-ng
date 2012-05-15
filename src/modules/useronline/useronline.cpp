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

static int uniqueEventId=0;

static int UserOnlineSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	int newStatus,oldStatus;

	if((HANDLE)wParam==NULL || strcmp(cws->szSetting,"Status")) return 0;
	newStatus=cws->value.wVal;
	oldStatus=DBGetContactSettingWord((HANDLE)wParam,"UserOnline","OldStatus",ID_STATUS_OFFLINE);
	DBWriteContactSettingWord((HANDLE)wParam,"UserOnline","OldStatus",(WORD)newStatus);
	if(CallService(MS_IGNORE_ISIGNORED,wParam,IGNOREEVENT_USERONLINE)) return 0;
	if(DBGetContactSettingByte((HANDLE)wParam,"CList","Hidden",0)) return 0;
    if(newStatus==ID_STATUS_OFFLINE&&oldStatus!=ID_STATUS_OFFLINE) {
       // Remove the event from the queue if it exists since they are now offline     
       int lastEvent = (int)DBGetContactSettingDword((HANDLE)wParam,"UserOnline","LastEvent",0);
       
       if (lastEvent) {
           CallService(MS_CLIST_REMOVEEVENT,wParam,(LPARAM)lastEvent);
           DBWriteContactSettingDword((HANDLE)wParam,"UserOnline", "LastEvent", 0);
       }
    }
	if((newStatus==ID_STATUS_ONLINE || newStatus==ID_STATUS_FREECHAT) &&
	   oldStatus!=ID_STATUS_ONLINE && oldStatus!=ID_STATUS_FREECHAT) {
		{
			DWORD ticked = db_dword_get(NULL, "UserOnline", cws->szModule, GetTickCount());
			// only play the sound (or show event) if this event happens at least 10 secs after the proto went from offline
			if ( GetTickCount() - ticked > (1000*10) ) { 
				CLISTEVENT cle;
				TCHAR tooltip[256];

				ZeroMemory(&cle,sizeof(cle));
				cle.cbSize=sizeof(cle);
				cle.flags=CLEF_ONLYAFEW | CLEF_TCHAR;
				cle.hContact=(HANDLE)wParam;
				cle.hDbEvent=(HANDLE)(uniqueEventId++);
				cle.hIcon = LoadSkinIcon( SKINICON_OTHER_USERONLINE, false );
				cle.pszService="UserOnline/Description";
				mir_sntprintf(tooltip,SIZEOF(tooltip),TranslateT("%s is Online"), cli.pfnGetContactDisplayName(( HANDLE )wParam, 0 ));
				cle.ptszTooltip=tooltip;
				CallService(MS_CLIST_ADDEVENT,0,(LPARAM)&cle);
				IconLib_ReleaseIcon( cle.hIcon, 0 );
                DBWriteContactSettingDword(cle.hContact,"UserOnline", "LastEvent", (DWORD)cle.hDbEvent);
				SkinPlaySound("UserOnline");
			}
		}
	}
	return 0;
}

static int UserOnlineAck(WPARAM, LPARAM lParam)
{
	ACKDATA * ack = (ACKDATA*) lParam;
	if ( ack != 0 && ack->szModule && ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS && ack->hProcess == (HANDLE)ID_STATUS_OFFLINE) {
		// if going from offline to any other mode, remember when it happened.
		db_dword_set(NULL, "UserOnline", ack->szModule, GetTickCount());
	}
	return 0;
}

static int UserOnlineModulesLoaded(WPARAM, LPARAM)
{
	// reset the counter
	for ( int j = 0; j < accounts.getCount(); j++ )
		if ( Proto_IsAccountEnabled( accounts[j] )) db_dword_set( NULL, "UserOnline", accounts[j]->szModuleName, GetTickCount());

	return 0;
}

static int UserOnlineAccountsChanged( WPARAM eventCode, LPARAM lParam )
{
	PROTOACCOUNT* pa = (PROTOACCOUNT*)lParam;

	switch( eventCode ) {
	case PRAC_ADDED:
	case PRAC_CHECKED:
		// reset the counter
		if ( Proto_IsAccountEnabled( pa ))
			db_dword_set( NULL, "UserOnline", pa->szModuleName, GetTickCount());
		break;
	}
	return 0;
}

int LoadUserOnlineModule(void)
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED,UserOnlineSettingChanged);
	HookEvent(ME_PROTO_ACK, UserOnlineAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, UserOnlineModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, UserOnlineAccountsChanged);
	SkinAddNewSoundEx("UserOnline","Alerts","Online");
	return 0;
}
