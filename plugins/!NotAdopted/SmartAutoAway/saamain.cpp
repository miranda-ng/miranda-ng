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

HANDLE hService[5];
HANDLE hHooks[6];

#ifndef SAA_PLUGIN
  int LoadIdleModule(void){return 0;}
  int UnloadIdleModule(void){return 0;}
#endif

int LoadAutoAwayModule(void)
{
	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED,InitVariables);
	hIdleEvent = CreateHookableEvent(ME_IDLE_CHANGED);
	hHooks[1] = HookEvent(ME_OPT_INITIALISE,AutoAwayOptInitialise);
	MyGetLastInputInfo=(BOOL (WINAPI *)(PLASTINPUTINFO)) GetProcAddress( GetModuleHandleA("user32"),"GetLastInputInfo" );
	// load settings into live ones
	idleOpts = idleOptsPerm=db_get_dw(NULL, AA_MODULE,AA_IDLEOPTIONS,idleOptsDefault);
//	if ((idleOptsPerm&IdleSuppressIdleMenu)==0) AddIdleMenu();
//	idleCheckPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLECHECK, 0);
//	idleMethodPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLEMETHOD, 0);
//	idleGLIPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLEGLI, 1);
//	idleTimeFirstPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLETIME1ST, 10);
//	idleTimeSecondPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLETIME2ND, 30);
//	idleTimeFirstOnPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLETIME1STON, 0);
//	idleTimeSecondOnPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLETIME2NDON, 0);
//	idleOnSaverPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLEONSAVER, 0);
//	idleOnLockPerm = db_get_b(NULL, IDLEMODULE, IDL_IDLEONLOCK, 0);
//	idlePrivatePerm = db_get_b(NULL, IDLEMODULE, IDL_IDLEPRIVATE, 0);
	hService[0] = CreateServiceFunction(MS_IDLE_GETIDLEINFO, IdleGetInfo);

	hHooks[2] = HookEvent(ME_SYSTEM_SHUTDOWN,AutoAwayShutdown);
	hHooks[3] = HookEvent(ME_IDLE_CHANGED, AutoAwayEvent);
	hHooks[4] = HookEvent(ME_CLIST_STATUSMODECHANGE,StatusModeChangeEvent);
	hHooks[5] = HookEvent(ME_PROTO_ACK, ProtoAckEvent);

	hService[1] = CreateServiceFunction(AA_IDLE_BENOTIDLESERVICE,(MIRANDASERVICE)idleServiceNotIdle);
	hService[2] = CreateServiceFunction(AA_IDLE_BESHORTIDLESERVICE,(MIRANDASERVICE)idleServiceShortIdle);
	hService[3] = CreateServiceFunction(AA_IDLE_BELONGIDLESERVICE,(MIRANDASERVICE)idleServiceLongIdle);
	hService[4] = CreateServiceFunction(AA_IDLE_RECONNECTSERVICE,(MIRANDASERVICE)reconnectService);
	return 0;
}

int InitVariables( WPARAM wParam, LPARAM lParam )
{
	int j=0;
	int i=0;
	localeID = CallService(MS_LANGPACK_GETLOCALE,0,0);
	if (localeID==CALLSERVICE_NOTFOUND) localeID=LOCALE_USER_DEFAULT;
	codePage = CallService(MS_LANGPACK_GETCODEPAGE,0,0);
	if (codePage==CALLSERVICE_NOTFOUND) codePage=CP_ACP;
#ifdef UNICODE
	HasAwayMessageW = (ServiceExists(MS_AWAYMSG_GETSTATUSMSGW)!=0);
#endif
	for (i=0;i<4;i++) {
		Hotkey_Register(&hotkeydescs[i]);
	}


#ifdef AALOG
	{
	    NETLIBUSER nlu = { 0 };
	    nlu.cbSize = sizeof(nlu);
		nlu.szSettingsModule = "SAA";
		nlu.flags=NUF_NOOPTIONS | NUF_NOHTTPSOPTION;
	    nlu.szDescriptiveName = Translate(SECTIONNAME " Module");
		hNetlib = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);
	}
	{
		SYSTEMTIME st={0};
		int drift;
		char str[32];
		char log[1024];
		char target[1024];
		drift = GetTimeZone(str);
		GetLastActiveLocalTime(&st,0);
		sprintf(log,"Now is %02d/%02d/%02d %02d:%02d:%02d %s (%02d:%02d)",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond,str,
		div(drift,60).quot,drift<0?-div(drift,60).rem:div(drift,60).rem);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		GetLastActiveLocalTime(&st,24*60*60*1000);
		sprintf(log,"1 day ago was %02d/%02d/%02d %02d:%02d:%02d %s (%02d:%02d)",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond,str,
		div(drift,60).quot,drift<0?-div(drift,60).rem:div(drift,60).rem);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		ParseString("%Y-%m-%d %H:%M:%S",target,1023);
		sprintf(log,"Testing ParseString: %s",target);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		ParseString("%E/%d/%y %W %h:%M:%S %p",target,1023);
		sprintf(log,"Testing ParseString: %s",target);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		ParseString("%w %% %Z (%z)%n%L%b%l%b%K%b%k",target,1023);
		sprintf(log,"Testing ParseString: %s",target);
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
	}
#endif

	CallService(MS_PROTO_ENUMACCOUNTS,(WPARAM)&protoCount,(LPARAM)&accounts);
	reqStatus = (int *)mir_alloc((protoCount)*sizeof(reqStatus[0]));
	courStatus = (int *)mir_alloc((protoCount)*sizeof(courStatus[0]));
	courAwayStatus = (int *)mir_alloc((protoCount)*sizeof(courStatus[0]));
	protoModes = (int *)mir_alloc((protoCount)*sizeof(protoModes[0]));
	messCaps = (int *)mir_alloc((protoCount)*sizeof(messCaps[0]));

	awayStatuses=(short int *)mir_alloc((protoCount)*sizeof(awayStatuses[0]));
	onlyIfBits=(long int *)mir_alloc((protoCount)*sizeof(onlyIfBits[0]));
	awayStatusesPerm=(short int *)mir_alloc((protoCount)*sizeof(awayStatuses[0]));
	onlyIfBitsPerm=(long int *)mir_alloc((protoCount)*sizeof(onlyIfBits[0]));

	reconnectOpts=(long int *)mir_alloc((protoCount)*sizeof(reconnectOpts[0]));
	reconnectOptsPerm=(long int *)mir_alloc((protoCount)*sizeof(reconnectOptsPerm[0]));
	protoOfflineTimes=(unsigned int *)mir_alloc((protoCount)*sizeof(protoOfflineTimes[0]));
	protoStatus=(int *)mir_alloc((protoCount)*sizeof(protoStatus[0]));

	idleMessOpts=(long int *)mir_alloc((protoCount)*sizeof(idleMessOpts[0]));
	idleMessOptsPerm=(long int *)mir_alloc((protoCount)*sizeof(idleMessOptsPerm[0]));

	mesgHere = (TCHAR**)mir_alloc(protoCount * sizeof(mesgHere[0]));
	mesgHerePerm = (TCHAR**)mir_alloc(protoCount * sizeof(mesgHerePerm[0]));
	mesgShort = (TCHAR**)mir_alloc(protoCount * sizeof(mesgShort[0]));
	mesgShortPerm = (TCHAR**)mir_alloc(protoCount * sizeof(mesgShortPerm[0]));
	mesgLong = (TCHAR**)mir_alloc(protoCount * sizeof(mesgLong[0]));
	mesgLongPerm = (TCHAR**)mir_alloc(protoCount * sizeof(mesgLongPerm[0]));

	#ifdef UNICODE
		protoHasAwayMessageW=(bool *)mir_alloc(protoCount * sizeof(protoHasAwayMessageW[0]));
	#endif

	isWaitingForRestoreStatusMessage = (int *)mir_alloc((protoCount)*sizeof(isWaitingForRestoreStatusMessage[0]));

	for (j = 0 ; j < protoCount ; j++) {
		int caps=0;
#ifdef AALOG
			{
				char log[1024];
				sprintf(log,"Checking protocol index %d out of %d...",j+1,protoCount);
				CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
			}
#endif
		reqStatus[j] = 0;
		courStatus[j] = ID_STATUS_OFFLINE;
		protoModes[j] =0;
		isWaitingForRestoreStatusMessage[j] = 0;
		caps = CallProtoService(accounts[j]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
		messCaps[j] = (CallProtoService(accounts[j]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND)?
		CallProtoService(accounts[j]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0):0;
#ifdef AALOG
		{
			char log[1024];
			if (messCaps[j]){
				int i;
				sprintf(log,"StatusMessages Caps for %s:",
					accounts[j]->szModuleName
				);
				for (i=0;i<10;i++){
					int statusFlag = Proto_Status2Flag(aa_Status[i]);
					if (statusFlag & messCaps[j]){
						sprintf(log,"%s %s",log,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, aa_Status[i], 0));
					}
				}
			} else {
				sprintf(log,"%s cannot set StatusMessage",
					accounts[j]->szModuleName);
			}
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)log);
		}
#endif
		for (i = 0; i<numStatuses; i++){
			protoModes[j] <<= 1;
			protoModes[j] |= (Proto_Status2Flag(aa_Status[i]) & caps)?1:0;
			protoModes[j] |= (aa_Status[i] == ID_STATUS_OFFLINE);
		}
		if ( isInterestingProto(j) ) {
			char str[256];
			sprintf(str,AA_BITSONLYIF,accounts[j]->szModuleName);
//			MessageBox(	0,str,accounts[j]->szModuleName,0);
			onlyIfBits[j]= db_get_dw(NULL,AA_MODULE,str,-1);
			if (onlyIfBits[j]==-1){
//				onlyIfBits[j] = (((protoModes[j]<<16)|(protoModes[j])) & aa_OnlyIfBitsDefault);
				onlyIfBits[j] =  aa_OnlyIfBitsDefault;
				db_set_dw(NULL,AA_MODULE,str,onlyIfBits[j]);
			}
			onlyIfBitsPerm[j]=onlyIfBits[j];
#ifdef UNICODE
			sprintf(str,"%s%s",accounts[j]->szModuleName,PS_SETAWAYMSGW);
			protoHasAwayMessageW[j]=(ServiceExists(str)!=0);
#endif
//			sprintf(str,AA_BITSLONGONLYIF,accounts[j]->szModuleName);
////			MessageBox(	0,str,accounts[j]->szModuleName,0);
//			if (!db_get_w(NULL,AA_MODULE,str,0))
//				db_set_w(NULL,AA_MODULE,str,(protoModes[j] & aa_OnlyIfBitsDefault)|known);
			sprintf(str,AA_AWAYSTATUSES,accounts[j]->szModuleName);
//			MessageBox(	0,str,accounts[j]->szModuleName,0);
			awayStatuses[j] = db_get_w(NULL,AA_MODULE,str,-1);
			if (awayStatuses[j] ==-1){
				int old = db_get_w(NULL,AA_OLDMODULE,AA_SHORTSTATUS,-1);
				int s = StatusToProtoIndex(((old==-1)?ID_STATUS_AWAY:OldIndexToStatus(old)),protoModes[j]);
				int l = 0;
				int comb = 0;
				old = db_get_w(NULL,AA_OLDMODULE,AA_LONGSTATUS,-1);
				l = StatusToProtoIndex(((old==-1)?ID_STATUS_NA:OldIndexToStatus(old)),protoModes[j]);
				comb = (l<<4)|s;
				old = db_get_b(NULL,AA_OLDMODULE,AA_USESHORT,-1);
				if (s) comb |= ((old==-1)?1<<8:(old==1)<<8);
				old = db_get_b(NULL,AA_OLDMODULE,AA_USELONG,-1);
				if (l) comb |= ((old==-1)?1<<9:(old==1)<<9);
//				comb |= (1<<15);
				awayStatuses[j]=comb;
				db_set_w(NULL,AA_MODULE,str,(WORD)awayStatuses[j]);
			}
			awayStatusesPerm[j] = awayStatuses[j];

			sprintf(str,AA_LASTREQUESTEDSTATUS,accounts[j]->szModuleName); //get last requested status
			reqStatus[j] = db_get_w(NULL,AA_MODULE,str,ID_STATUS_ONLINE); //default: online

			sprintf(str,AA_RECONNECTOPTS,accounts[j]->szModuleName); //get reconnect options status
			reconnectOpts[j] = db_get_dw(NULL,AA_MODULE,str,aa_ReconnectOptsDefault);
			reconnectOptsPerm[j] = reconnectOpts[j];
			protoStatus[j] = 0;
			protoOfflineTimes[j] = 0;
			mesgHere[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));
			mesgHerePerm[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));
			mesgShort[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));
			mesgShortPerm[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));
			mesgLong[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));
			mesgLongPerm[j]=(TCHAR *)mir_alloc(sizeof(TCHAR)*(maxMessageLength+1));

			if (messCaps[j]){
				DBVARIANT dbv;
				sprintf(str,idleMsgOptionsName,accounts[j]->szModuleName);
				idleMessOptsPerm[j] = idleMessOpts[j] = db_get_dw(NULL,AA_MODULE,str,idleMsgOptionsDefault);

				sprintf(str,idleMsgOptionsTextHere,accounts[j]->szModuleName);
				if(db_get_ts(NULL,AA_MODULE,str,&dbv)==0) {
					_tcsncpy(mesgHere[j],dbv.ptszVal,maxMessageLength);
					mesgHere[j][maxMessageLength]=0;
					db_free(&dbv);
				} else _tcscpy(mesgHere[j],messHereDefault);
				if(!_tcscmp(mesgHere[j],messHereDefaultOld)) _tcscpy(mesgHere[j],messHereDefault);
				_tcscpy(mesgHerePerm[j],mesgHere[j]);

				sprintf(str,idleMsgOptionsTextShort,accounts[j]->szModuleName);
				if(db_get_ts(NULL,AA_MODULE,str,&dbv)==0) {
					_tcsncpy(mesgShort[j],dbv.ptszVal,maxMessageLength);
					mesgShort[j][maxMessageLength]=0;
					db_free(&dbv);
				} else _tcscpy(mesgShort[j],messShortDefault);
				if(!_tcscmp(mesgShort[j],messShortDefaultOld)) _tcscpy(mesgShort[j],messShortDefault);
				_tcscpy(mesgShortPerm[j],mesgShort[j]);

				sprintf(str,idleMsgOptionsTextLong,accounts[j]->szModuleName);
				if(db_get_ts(NULL,AA_MODULE,str,&dbv)==0) {
					_tcsncpy(mesgLong[j],dbv.ptszVal,maxMessageLength);
					mesgLong[j][maxMessageLength]=0;
					db_free(&dbv);
				} else _tcscpy(mesgLong[j],messLongDefault);
				if(!_tcscmp(mesgLong[j],messLongDefaultOld)) _tcsncpy(mesgLong[j],messLongDefault,maxMessageLength);
				_tcsncpy(mesgLongPerm[j],mesgLong[j],maxMessageLength);
//				sprintf(mesgHere[j],"%d: %s: %s",j,accounts[j]->szModuleName,"Test text");
			} else {
				idleMessOptsPerm[j] = idleMessOpts[j] = 0;
				mesgHerePerm[j] = mesgHere[j] = mesgShortPerm[j] = mesgShort[j] = mesgLongPerm[j] = mesgLong[j] = NULL;
			}
		}
	}
#ifdef AALOG
	{
		char str[1024]="";
		for (j = 0 ; j < protoCount ; j++) {
			int status;
			sprintf(str,"%s\n%s Type:%d\nLastReqStatus: %s\n",str,accounts[j]->szModuleName,accounts[j]->szProtoName,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)reqStatus[j], 0));
			i=1;
			while(status=StatusByProtoIndex(protoModes[j],i)){
				sprintf(str,"%s  %d. %s\n",str,i,CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)status, 0));
				i++;
			}
//					IsStatusBitSet(aa_Status[i],protoModes[j])?"":"not",
//					CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)aa_Status[i], 0),
//					IsStatusBitSet(aa_Status[i],aa_OnlyIfBitsDefault)?"n":"ff"
//				);

		}
		CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
	}
#endif
//something is hapaning here on startup, if main status is not offline
//will move Notify hIdleEvent for later in the IdleTimer
//	NotifyEventHooks( hIdleEvent, 0, idleOpts&IdleBitsPrivate?IDF_PRIVACY:0 ); //Tell everyone we are here
	FirstTick = 1; //we will wait 1 ticks of the timer before Forcing Awake
//	FirstTick = 10; //we will wait 20 seconds before Forcing Awake
	hIdleTimer=SetTimer(NULL, 0, 2000, IdleTimer);
//	{
//		char str[20000]="";
//		for (i=0;i<=101;i++){
//			char str1[128];
//			GetStringFromDelay(str1,GetReconnectDelayFromSlider(i));
//			sprintf(str,"%d\t%d\t%s",GetReconnectDelayFromSlider(i),i,str1);
//			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)str);
//		}
//	}
	if ((idleOptsPerm&IdleSuppressIdleMenu)==0) AddIdleMenu();
	if ((idleOptsPerm&IdleSuppressAutoHere)!=0) isCurrentlyIdle=db_get_b(NULL,AA_MODULE,AA_LASTIDLESTATUS,0);;
	return 0;
}

//#ifndef SAA_PLUGIN
int AutoAwayShutdown(WPARAM wParam,LPARAM lParam)
{
	int i;

#ifdef AALOG
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Killing Timer");
#endif
	KillTimer(NULL, hIdleTimer);
	if (hHookIconsChanged){
		#ifdef AALOG
			CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"UnHooking hHookIconsChanged");
		#endif
		UnhookEvent(hHookIconsChanged);
	}
#ifdef AALOG
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"DestroyHookableEvent(hIdleEvent)");
#endif
	for (i=0; i<sizeof(hHooks)/sizeof(HANDLE); ++i)
		UnhookEvent(hHooks[i]);
	for (i=0; i<sizeof(hService)/sizeof(HANDLE); ++i)
		DestroyServiceFunction(hService[i]);
	DestroyHookableEvent(hIdleEvent);
#ifdef AALOG
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"UnHooking Event "ME_SYSTEM_SHUTDOWN"");
#endif
//	UnhookEvent(ME_SYSTEM_SHUTDOWN);
#ifdef AALOG
	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Done");

	if (hNetlib) {
		Netlib_CloseHandle(hNetlib);
		hNetlib = NULL;
	}

#endif

	mir_free(reqStatus);
	mir_free(courStatus);
	mir_free(courAwayStatus);
	mir_free(protoModes);
	mir_free(awayStatusesPerm);
	mir_free(awayStatuses);
	mir_free(onlyIfBitsPerm);
	mir_free(onlyIfBits);
	mir_free(reconnectOptsPerm);
	mir_free(reconnectOpts);
	mir_free(protoOfflineTimes);
	mir_free(protoStatus);
	mir_free(idleMessOpts);
	mir_free(idleMessOptsPerm);
	mir_free(isWaitingForRestoreStatusMessage);

	for (i=0; i<protoCount; ++i) if (messCaps[i]){
		mir_free(mesgHere[i]);
		mir_free(mesgHerePerm[i]);
		mir_free(mesgShort[i]);
		mir_free(mesgShortPerm[i]);
		mir_free(mesgLong[i]);
		mir_free(mesgLongPerm[i]);
	}

	mir_free(messCaps);
	mir_free(mesgHere);
	mir_free(mesgHerePerm);
	mir_free(mesgShort);
	mir_free(mesgShortPerm);
	mir_free(mesgLong);
	mir_free(mesgLongPerm);

	return 0;
}
//#endif
