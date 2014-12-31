/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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
#include "globals.h"


// globals
//static HANDLE AutoAwayEventHandle = NULL;
//static HANDLE StatusModeChangeEventHandle = NULL;
//static HANDLE ProtoAckEventHandle = NULL;
//static HANDLE AutoAwayShutdownHandle = NULL;
//static HANDLE AutoAwayOptInitialiseHandle = NULL;


//#define AA_LASTSTATUS "LastStatus_"
//#define AA_ONLYIF "OnlyIf"
//#define AA_ONLYIF2 "OnlyIf2"

//#define ME_IDLE_CHANGED "Miranda/SmartIdle/Changed"
//static HANDLE hIdleEvent = NULL;


#define allprotomodes			0x03FF         //0000 0011 1111 1111;
#define aa_OnlyIfBitsDefault	0x01BB01B8 //2 x 0000 0001 1011 1011;
									   //1111 0000 0000 0000;
//#define known 0x8000				   //1000 0000 0000 0000;




//static int aa_OnlyIfDefaults[] = {2,2,2,2,1,2,2,2,2,1};

extern  unsigned long lastIdleEventTime;


int AutoAwayOptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=1000000000;
#ifdef SAA_PLUGIN
	odp.hInstance=g_hInst;
#else
	odp.hInstance=GetModuleHandle(NULL);
#endif
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_SMARTAUTOAWAY);
	odp.pszTitle=SECTIONNAME;
	odp.pszGroup="Status";
	odp.pfnDlgProc=DlgProcAutoAwayOpts;
	odp.flags=ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

static short int curIdleStatus = -1; //0: NotIdle, 1: Short Idle, 2: Long Idle, -1 Not init yet: Will be inited on AutoAwayEvent;
int AutoAwayEvent(WPARAM wParam, LPARAM lParam)
{
//	char str[MAXMODULELABELLENGTH];
//#define IDF_ISIDLE		0x1 // idle has become active (if not set, inactive)
//#define IDF_SHORT		0x2 // short idle mode
//#define IDF_LONG		0x4 // long idle mode
//#define IDF_PRIVACY		0x8 // if set, the information provided shouldn't be given to third parties.
//#define IDF_ONFORCE	   0x10 // screensaver/onstationlocked,etc cause instant idle

	int j;

#ifdef AALOG
	{
		char log[1024]="";
		sprintf(log,"IdleEvent came: %sdle;%s%s%s%sorced.",
			lParam & IDF_ISIDLE?"I":"Not i",
			lParam & IDF_SHORT?" Short;":"",
			lParam & IDF_LONG?" Long;":"",
			lParam & IDF_PRIVACY?" Private;":"",
			lParam & IDF_ONFORCE?" F":"Not f");
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		{
			MIRANDA_IDLE_INFO mii;
			ZeroMemory(&mii, sizeof(mii));
			mii.cbSize = sizeof(mii);
			if (CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii)==0){
				sprintf(log,"IdleInfo valid: IdleTime:%d; %srivate; Status: %s(%d); aaLock: %s.",
					mii.idleTime,
					mii.privacy?"P":"Not p",
					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)mii.aaStatus, 0),
					mii.aaStatus,
					mii.aaLock?"Yes":"No");
			} else {
				sprintf(log,"IdleInfo Failed: Size: %d; IdleTime: %d.",mii.cbSize,mii.idleTime);
			}
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		}
	}
#endif
	if ( lParam & IDF_ISIDLE ) { //going to sleep...
		if ( lParam & IDF_LONG ) { //... deeply
			if (curIdleStatus > 1) return 0; // we are already deeply sleeping...
			curIdleStatus = 2;
		} else { //... lite if IDF_LONG is not set
			if (curIdleStatus > 0) return 0; // we are already sleeping...
			curIdleStatus = 1;
		}
	} else { // we are going to awake
		if (curIdleStatus == 0) return 0; // we are already awake...
		curIdleStatus = 0;
	}
	if (!starting){
		lastIdleEventTime = time(NULL)-secondsAfterLastActivity;
		if ((idleOptsPerm&IdleSuppressAutoHere)!=0) {
			db_set_b(NULL,AA_MODULE,AA_LASTIDLESTATUS,(BYTE)curIdleStatus);
			db_set_dw(NULL,AA_MODULE,AA_LASTIDLETIME,(DWORD)lastIdleEventTime);
	}	}
	switch (curIdleStatus) {
	case 0:
		xModifyMenu(0,CMIF_CHECKED);
		xModifyMenu(1,0);
		xModifyMenu(2,0);
		break;
	case 1:
		xModifyMenu(0,0);
		xModifyMenu(1,CMIF_CHECKED);
		xModifyMenu(2,0);
		break;
	case 2:
		xModifyMenu(0,0);
		xModifyMenu(1,CMIF_GRAYED);
		xModifyMenu(2,CMIF_CHECKED);
		break;
	}

	for (j = 0 ; j < protoCount ; j++) {
		///if ( accounts[j]->type != PROTOTYPE_PROTOCOL ) continue;
		if (!accounts[j]->bIsEnabled) continue;
		{ //status mode change
			int newStatus = 0;
			int reqStatusInd = numStatuses-StatusToProtoIndex(reqStatus[j],allprotomodes);
			bool isStatusLocked = db_get_b(NULL, accounts[j]->szModuleName, "LockMainStatus", 0) != 0;
			if (isStatusLocked) newStatus = reqStatus[j];
			else switch (curIdleStatus) {
				case 1 : { //sleeping lite
					if ( GetBits(awayStatusesPerm[j],maskIsShortAwayEnabled)){
						int bits = 1<<(reqStatusInd);
						BOOL permit = GetBits(onlyIfBitsPerm[j], bits);
						if ( permit ) {
						// get the index of the status we should use
							newStatus = aa_Status[GetBits(awayStatusesPerm[j],maskShortAwayStatus)-1];
						}  else {
#ifdef AALOG
							{
								char log[1024] ;
								sprintf(log,"%s: Status Change to short idle was not permitted",accounts[j]->szModuleName,0);
								CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
							}
#endif
							continue;
						}

					}  else continue;
					break;
				}
				case 2 : { //sleeping deeply
					if ( GetBits(awayStatusesPerm[j],maskIsLongAwayEnabled) ) {
						int bits = 1<<(reqStatusInd+16);
						BOOL permit = GetBits(onlyIfBitsPerm[j], bits);
						if ( permit ) {
						// get the index of the status we should use
							newStatus = aa_Status[GetBits(awayStatusesPerm[j],maskLongAwayStatus)-1];
						}  else {
#ifdef AALOG
							{
								char log[1024] ;
								sprintf(log,"%s: Status Change to long idle was not permitted",accounts[j]->szModuleName,0);
								CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
							}
#endif
							continue;
						}
					}   else continue;
					break;
				}
				case 0: { //wakening up
						newStatus = reqStatus[j];
#ifdef AALOG
						{
							char str[256];
							sprintf(str,"%s WakeningUp: %s",accounts[j]->szModuleName,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0));
							CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
						}
#endif
					break;
				}
			} // switch
			if (newStatus){
				courAwayStatus[j] = newStatus;
				//FatalError? and "dontReconnectOnFatalError"? and we are not just awaken
#ifdef AALOG
				{
					char str[1000];
					sprintf(str,"%s: Reconnect conditions before change status to %s(%d) are: FatalError did%s happen; DontReconnectOnFatalError is %sabled and we are going to %s.",
						accounts[j]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0),newStatus,
						(protoStatus[j]&2)?"":"n't",
						(reconnectOptsPerm[j]&aa_ReconnectNotFatalError)?"En":"Dis",
						(curIdleStatus==1)?"take a short nap":((curIdleStatus==2)?"sleep":"awake"));
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
				}
#endif
				if ((protoStatus[j]&2)&&
					(reconnectOptsPerm[j]&aa_ReconnectNotFatalError)&&
					(curIdleStatus>0)) {
					// do nothing :) but only the log, eventualy
#ifdef AALOG
					char str[1000];
					sprintf(str,"%s: we will not change the status to %s(%d).",
						accounts[j]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0),newStatus);
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
				} else {
#ifdef AALOG
					char str[1000];
					sprintf(str,"%s: we will change the status to %s(%d).",
						accounts[j]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0),newStatus);
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
					if ( CallProtoService(accounts[j]->szModuleName, PS_GETSTATUS, 0, 0) != newStatus ) {
						//if (db_get_b( NULL, accounts[j]->szModuleName, , "LockMainStatus", 0 ))
						//	continue; //szName

						CallProtoService(accounts[j]->szModuleName, PS_SETSTATUS, newStatus, 0);
#ifdef AALOG
					}  else {
						char str[256];
						sprintf(str,"%s: %s already set",accounts[j]->szModuleName,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0));
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
					}
				}
				if (isStatusLocked) {
					// do nothing - log only
#ifdef AALOG
					char str[1000];
					sprintf(str,"%s: Protocol Locked to %s(%d). Not touching StatusMessage",
						accounts[j]->szModuleName,
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0),newStatus);
					CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
				} else if (messCaps[j] && !(lParam&IDF_PRIVACY)){//setting the statusMessage if protocol supports AND privacy is disabled
					TCHAR * awayMsg=0;
					int specificMsg = messCaps[j] & Proto_Status2Flag(newStatus);

					switch (curIdleStatus) {
						case 0 : specificMsg = (specificMsg > 0) && (GetBits(idleMessOpts[j],IdleMessOptsMaskUseHere)>0); break;
						case 1 : specificMsg = (specificMsg > 0) && (GetBits(idleMessOpts[j],IdleMessOptsMaskUseShort)>0); break;
						case 2 : specificMsg = (specificMsg > 0) && (GetBits(idleMessOpts[j],IdleMessOptsMaskUseLong)>0); break;
					}
					if (/*!starting &&*/ specificMsg){
						awayMsg = (TCHAR *)mir_alloc((maxMessageLength+1)*sizeof(TCHAR));
						//TCHAR temp[maxMessageLength+1];
						switch (curIdleStatus) {
							case 0 : ParseString(mesgHerePerm[j],awayMsg,maxMessageLength); break;
							case 1 : ParseString(mesgShortPerm[j],awayMsg,maxMessageLength); break;
							case 2 : ParseString(mesgLongPerm[j],awayMsg,maxMessageLength); break;
							default: awayMsg[0]=0;
						}
						//my_u2a(temp, awayMsg, maxMessageLength);
					} else {
#ifdef UNICODE
						if (HasAwayMessageW){
							awayMsg = (TCHAR *) CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM) newStatus, 0);
						} else {
							char *temp=(char *)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) newStatus, 0);
							if ((int)temp == CALLSERVICE_NOTFOUND) awayMsg = 0;
							else{
								awayMsg = (TCHAR *)mir_alloc((maxMessageLength+1)*sizeof(TCHAR));
								my_a2u(temp,awayMsg,maxMessageLength);
								mir_free(temp);
							}
						}
#else
						char *temp=(char *)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) newStatus, 0);
						awayMsg=(char *)CallService(MS_AWAYMSG_GETSTATUSMSG, (WPARAM) newStatus, 0);
						if ((int)temp == CALLSERVICE_NOTFOUND) awayMsg = 0;
#endif
					}
#ifdef AALOG
					{
						char str[1000];
						sprintf(str,"%s: Specific Status Message is %s",accounts[j]->szModuleName,
							specificMsg?"Enabled":"Disabled");
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
						sprintf(str,"%s: Going to %s (%d); StatusMessage is: %s", accounts[j]->szModuleName,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, newStatus, 0),newStatus,
							awayMsg ? awayMsg : "none or private");
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
					}
#endif
#ifdef UNICODE
					if (protoHasAwayMessageW[j]) CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSGW, newStatus, (LPARAM) awayMsg);
					else {
						char temp[maxMessageLength+1];
						my_u2a(awayMsg,temp,maxMessageLength);
						CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSG, newStatus, (LPARAM) temp);
					}
#else
					CallProtoService(accounts[j]->szModuleName, PS_SETAWAYMSG, newStatus, (LPARAM) awayMsg);
#endif
					if (awayMsg)  {
#ifdef AALOG
						{char str[1000];
						sprintf(str,"%s: Cleaning after MS_AWAYMSG_GETSTATUSMSG: %s",accounts[j]->szModuleName,awayMsg);
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);}
#endif
						mir_free(awayMsg);
					}
					if ((curIdleStatus==0) && (GetBits(idleMessOpts[j],IdleMessOptsMaskRstHere)>0) && /*!starting &&*/ specificMsg){
#ifdef AALOG
						char str[1000];
						sprintf(str,"%s: Scheduling \"ResetDefaultStatusMessage\" after %d seconds",accounts[j]->szModuleName,GetBits(idleMessOpts[j],IdleMessOptsMaskRstHereDelay));
						CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
#endif
						isWaitingForRestoreStatusMessage[j] = GetBits(idleMessOpts[j],IdleMessOptsMaskRstHereDelay)/2+1;
					}
				}
			}//if
		} //StatusChange Was done
	} //for
	if (starting) {
#ifdef AALOG
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Clearing the \"starting\" flag...");
#endif
		starting = 0;
	}
	return 0;
}



extern int StatusModeChangeEvent(WPARAM wParam,LPARAM lParam)
{
	int j=0;
	char str[256];

#ifdef AALOG
	{
		char log[1024];
		sprintf(log,"Status Mode change event: %s: %s\n\n",lParam,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, wParam, 0));
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
	}
#endif
	if (lParam){
		for (j = 0 ; j < protoCount ; j++) {
			//if ( accounts[j]->type != PROTOTYPE_PROTOCOL ) continue;
			if (!accounts[j]->bIsEnabled) continue;
			if (strcmp((char *)lParam,accounts[j]->szModuleName)==0) {
				sprintf(str,AA_LASTREQUESTEDSTATUS,accounts[j]->szModuleName);
				reqStatus[j] = wParam;
				db_set_w(NULL,AA_MODULE,str,(WORD)reqStatus[j]);
			}
		}
	} else {
		if (idleOptsPerm&KeepMainStatusOffline) {
#ifdef AALOG
			{
				char log[1024];
				sprintf(log,"Global Status Mode change event: %s. Keep it offline!",CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, wParam, 0));
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
			}
#endif
			db_set_w(NULL,"CList","Status",(WORD)ID_STATUS_OFFLINE);
		}
		if (starting && (wParam == ID_STATUS_OFFLINE)){
#ifdef AALOG
			{
				char log[1024];
				sprintf(log,"Do not set last request status to %s. We are still starting.",CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, wParam, 0));
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
			}
#endif
		} else for (j = 0 ; j < protoCount ; j++) {
			//if ( accounts[j]->type != PROTOTYPE_PROTOCOL ) continue;
			if (!accounts[j]->bIsEnabled) continue;
			sprintf(str,AA_LASTREQUESTEDSTATUS,accounts[j]->szModuleName);
			reqStatus[j] = wParam;
			db_set_w(NULL,AA_MODULE,str,(WORD)reqStatus[j]);
		}
	}
#ifdef AALOG
	{
		char log[1024] = {0};
		for (j = 0 ; j < protoCount ; j++) {
			sprintf(log,"%s%d:%s: %s\n",log,j,accounts[j]->szModuleName,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, reqStatus[j], 0));
		}
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Status Mode change event: done");
	}
#endif
	if (idleOptsPerm&IdleHereOnStatusChange){
		if (curIdleStatus){
			if (wParam!=ID_STATUS_OFFLINE){
#ifdef AALOG
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"New requested status. Forcing ""Not Idle"".");
#endif
				SimulateIdle(0);
			}
		}
	}

//	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Keeping Last Requested statuses");
	if (theDialog) SetDlgItems(theDialog, courProtocolSelection);
	if (theDialogIdleMessages) SetDlgItemsIdleMessages(theDialogIdleMessages, courProtocolSelection);
	return 0;
}



int ProtoAckEvent(WPARAM wParam,LPARAM lParam)
{
#ifdef AALOG
	char log[2048];
#endif
	char str[1024];
	int j = 0;
	ACKDATA * ack = (ACKDATA*) lParam;
	for (j = 0 ; j < protoCount ; j++) /*if (accounts[j]->type==PROTOTYPE_PROTOCOL)*/if (accounts[j]->bIsEnabled) if (strcmp(ack->szModule,accounts[j]->szModuleName)==0){
		if (ack->type==ACKTYPE_STATUS) {
//			MessageBox(	0,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->lParam, 0),accounts[j]->szModuleName,0);
			if (!(ack->hContact)) ProtocolsProc(j,ack);
			if (ack->result==ACKRESULT_SUCCESS){
				courStatus[j] = ack->lParam;
				if (theDialog){
					int coursel = SendDlgItemMessage(theDialog, IDC_AAPROTOCOL, CB_GETCURSEL, 0, 0);
					SendDlgItemMessageA(theDialog,IDC_AAPROTOCOL,CB_GETLBTEXT,coursel,(LPARAM)str);
					if (strcmp(ack->szModule,str)==0){
						SetDlgItems(theDialog,GetCourSelProtocol(coursel));
//						SendDlgItemMessage(theDialog, IDC_COURSTATUSLABEL, WM_SETTEXT, 0,
//							(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)courStatus[j], 0));
					}
				}
			}
		} else if (ack->type==ACKTYPE_LOGIN) {
 			if (!ack->hContact) LoginAck(j,ack);
		}
	}

#ifdef AALOG
//	if (theDialog) SendDlgItemMessage(theDialog, 40201, WM_SETTEXT, 0,(LPARAM)ack->szModule);
	sprintf(log,"ProtoAck: szModule: %s;",ack->szModule);
	sprintf(str,"%s",ack->hContact?(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)ack->hContact,0):"null");
	sprintf(log,"%s hContact: %s;",log,str);
//	if (theDialog) SendDlgItemMessage(theDialog, 40202, WM_SETTEXT, 0,(LPARAM)str);
	sprintf(str,"%d",ack->type);
	sprintf(log,"%stype: %s;",log,str);
//	if (theDialog) SendDlgItemMessage(theDialog, 40203, WM_SETTEXT, 0,str);
	sprintf(str,"%d",ack->result);
	sprintf(log,"%s result: %s;",log,str);
//	if (theDialog) SendDlgItemMessage(theDialog, 40204, WM_SETTEXT, 0,str);
	sprintf(str,"%d(%s)",ack->hProcess,
		CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->hProcess, 0));
	sprintf(log,"%s hProcess: %s;",log,str);
//	if (theDialog) SendDlgItemMessage(theDialog, 40205, WM_SETTEXT, 0,str);
	sprintf(str,"%d(%s)",ack->lParam,
			CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)ack->lParam, 0));
	sprintf(log,"%s lParam: %s;",log,str);
//	if (theDialog) SendDlgItemMessage(theDialog, 40206, WM_SETTEXT, 0,str);
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
#endif
	return 0;
}


int SimulateIdle(int idlstatus)
{
//						MessageBox(theDialog,"ButtonPressed","Button",0);
	int flags = IDF_ISIDLE;
	switch (idlstatus)
	{
	case 0: {flags = 0;/*MessageBox(theDialog,"BeHere","Button Pressed",0);*/ break;}
	case 1: {flags |= IDF_SHORT; /*MessageBox(theDialog,"BeAway","Button Pressed",0);*/ break;}
	case 2: {flags |= IDF_LONG; /*MessageBox(theDialog,"BeNA","Button Pressed",0);*/ break;}
	}
	if (idleOptsPerm&IdleBitsPrivate) flags |= IDF_PRIVACY;
	isCurrentlyIdle = idlstatus;
	NotifyEventHooks( hIdleEvent, 0, flags );
	return 0;
}

LRESULT SetDlgItemsAA(HWND hwndDlg, int coursel){
		int status = awayStatuses[coursel];
		long onlyif = onlyIfBits[coursel];
		BOOL enabledShort = (status & maskIsShortAwayEnabled)>0;
		BOOL enabledLong = (status & maskIsLongAwayEnabled)>0;
		int i =0;
		EnableWindow(GetDlgItem(hwndDlg, IDC_AASTATUS), enabledShort);
		EnableWindow(GetDlgItem(hwndDlg, IDC_AALONGSTATUS), enabledLong);
		CheckDlgButton(hwndDlg, IDC_AASHORTIDLE, enabledShort?BST_CHECKED:BST_UNCHECKED );
		CheckDlgButton(hwndDlg, IDC_AALONGIDLE, enabledLong?BST_CHECKED:BST_UNCHECKED );

		SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_AALONGSTATUS, CB_RESETCONTENT, 0, 0);
		for (i=0;i<numStatuses;i++){
			if (StatusToProtoIndex(aa_Status[i],protoModes[coursel])){
				// short idle flags
				SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_ADDSTRING, 0, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)aa_Status[i], GSMDF_TCHAR) );
				// long idle flags
				SendDlgItemMessage(hwndDlg, IDC_AALONGSTATUS, CB_ADDSTRING, 0, (LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)aa_Status[i], GSMDF_TCHAR) );
			}
			{
				BOOL bits = IsStatusBitSet(aa_Status[i],onlyif)>0;
				BOOL bitl = IsStatusBitSet(aa_Status[i],onlyif>>16)>0;
				BOOL bita = IsStatusBitSet(aa_Status[i],protoModes[coursel])==0;
				EnableWindow(GetDlgItem(hwndDlg, aa_Status[i]), enabledShort);
				StrikeOut(hwndDlg,aa_Status[i],bita);
				CheckDlgButton(hwndDlg, aa_Status[i],      bits?BST_CHECKED:BST_UNCHECKED );
				EnableWindow(GetDlgItem(hwndDlg, aa_Status[i]+0x10), enabledLong);
				StrikeOut(hwndDlg,aa_Status[i]+0x10,bita);
				CheckDlgButton(hwndDlg, aa_Status[i]+0x10, bitl?BST_CHECKED:BST_UNCHECKED );
//				ShowWindow(GetDlgItem(hwndDlg, aa_Status[i]), bita);
//				ShowWindow(GetDlgItem(hwndDlg, aa_Status[i]+0x10), bita);
			}
		}
		{
			int s = status & 0x000F;
			int l = status & 0x00F0;
			l = l>>4;
			s = StatusToProtoIndex(aa_Status[s-1],protoModes[coursel]);
			l = StatusToProtoIndex(aa_Status[l-1],protoModes[coursel]);
			SendDlgItemMessage(hwndDlg, IDC_AASTATUS, CB_SETCURSEL, s-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_AALONGSTATUS, CB_SETCURSEL, l-1, 0);
		}
	return TRUE;
}

INT_PTR CALLBACK DlgProcOptsAA(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
        case WM_INITDIALOG:
            TranslateDialogDefault(hwndDlg);
			SetDlgItemsAA(hwndDlg, courProtocolSelection);
            return TRUE;
		case WM_USER+1:
		{
			awayStatuses[courProtocolSelection] = (short)SetBits(awayStatuses[courProtocolSelection],maskIsShortAwayEnabled,IsDlgButtonChecked(hwndDlg, IDC_AASHORTIDLE)==BST_CHECKED);
			awayStatuses[courProtocolSelection] = (short)SetBits(awayStatuses[courProtocolSelection],maskIsLongAwayEnabled,IsDlgButtonChecked(hwndDlg, IDC_AALONGIDLE)==BST_CHECKED);
			SetDlgItemsAA(hwndDlg, courProtocolSelection);
			break;
		}
		case WM_USER+2:
		{
			int t = HIWORD(lParam)?16:0;
			int thebit = numStatuses-StatusToProtoIndex(LOWORD(lParam),allprotomodes);
			int value = IsDlgButtonChecked(hwndDlg, LOWORD(lParam)+t)==BST_CHECKED;
			thebit = 1<<(thebit+t);
			onlyIfBits[courProtocolSelection] =
				SetBits(onlyIfBits[courProtocolSelection],
					thebit,
					value);
			break;
		}
		case WM_USER+3:
		{
			int awStInd = SendDlgItemMessage(hwndDlg, lParam, CB_GETCURSEL, 0, 0);
			awStInd = StatusByProtoIndex(protoModes[courProtocolSelection],awStInd+1);
			awStInd = StatusToProtoIndex(awStInd,allprotomodes);
			if (lParam == IDC_AASTATUS) {
				awayStatuses[courProtocolSelection] = (short)SetBits(awayStatuses[courProtocolSelection],maskShortAwayStatus,awStInd);
			} else if (lParam == IDC_AALONGSTATUS) {
				awayStatuses[courProtocolSelection] = (short)SetBits(awayStatuses[courProtocolSelection],maskLongAwayStatus,awStInd);
			}
#ifdef _DEBUG
				else MessageBoxA(hwndDlg,"Came From Long Place","WM_USER+3",0)
#endif //_DEBUG
				;
			SetDlgItemsIdleMessages(theDialogIdleMessages, courProtocolSelection);
			break;
		}
        case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_AASHORTIDLE:
				case IDC_AALONGIDLE:
					{
						SendMessage(hwndDlg, WM_USER+1,0,0);
						break;
					}
				case 	ID_STATUS_AWAY:
				case 	ID_STATUS_DND:
				case 	ID_STATUS_NA:
				case 	ID_STATUS_OCCUPIED:
				case 	ID_STATUS_INVISIBLE:
				case 	ID_STATUS_ONTHEPHONE:
				case 	ID_STATUS_OUTTOLUNCH:
				case 	ID_STATUS_ONLINE:
				case 	ID_STATUS_FREECHAT:
				case 	ID_STATUS_OFFLINE:
					{
						SendMessage(hwndDlg, WM_USER+2,0,wParam);
						break;
					}
				case 	ID_STATUS_AWAY+16:
				case 	ID_STATUS_DND+16:
				case 	ID_STATUS_NA+16:
				case 	ID_STATUS_OCCUPIED+16:
				case 	ID_STATUS_INVISIBLE+16:
				case 	ID_STATUS_ONTHEPHONE+16:
				case 	ID_STATUS_OUTTOLUNCH+16:
				case 	ID_STATUS_ONLINE+16:
				case 	ID_STATUS_FREECHAT+16:
				case 	ID_STATUS_OFFLINE+16:
					{
						SendMessage(hwndDlg, WM_USER+2,0,(wParam-16)|0x00010000);
						break;
					}
				case IDC_AASTATUS:
				case IDC_AALONGSTATUS:
					{
						if ( HIWORD(wParam) != CBN_SELCHANGE ) return TRUE;
						SendMessage(hwndDlg, WM_USER+3,0,LOWORD(wParam));
						break;
					} //case
            }
            if ((HIWORD(wParam)==BN_CLICKED)||(HIWORD(wParam)==CBN_SELCHANGE))
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->idFrom) {
                case 0:
                    switch (((LPNMHDR) lParam)->code) {
                        case PSN_APPLY: {
                            return TRUE;
                        }
                    }
                    break;
            }
            break;
        case WM_DESTROY:
            break;
    }
    return FALSE;
}
