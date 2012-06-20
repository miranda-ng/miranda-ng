/*
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../commonstatus.h"
#include "startupstatus.h"

// {4B733944-5A70-4b52-AB2D-68B1EF38FFE8}
#define MIID_STARTUPSTATUS { 0x4b733944, 0x5a70, 0x4b52, { 0xab, 0x2d, 0x68, 0xb1, 0xef, 0x38, 0xff, 0xe8 } }

static UINT_PTR setStatusTimerId = 0;

int CompareSettings( const TSSSetting* p1, const TSSSetting* p2 )
{	return lstrcmpA( p1->szName, p2->szName );
}

static TSettingsList startupSettings( 10, CompareSettings );

TSSSetting::TSSSetting( PROTOACCOUNT* pa )
{
	cbSize = sizeof(PROTOCOLSETTINGEX);
	szName = pa->szModuleName;
	tszAccName = pa->tszAccountName;
	status = lastStatus = CallProtoService( pa->szModuleName, PS_GETSTATUS, 0, 0 );
	szMsg = NULL;
}

TSSSetting::TSSSetting( int profile, PROTOACCOUNT* pa )
{
	cbSize = sizeof(PROTOCOLSETTINGEX);

	// copy name
	szName = pa->szModuleName;
	tszAccName = pa->tszAccountName;

	// load status
	char setting[80];
	_snprintf(setting, sizeof(setting), "%d_%s", profile, pa->szModuleName);
	int iStatus = DBGetContactSettingWord(NULL, MODULENAME, setting, 0);
	if ( iStatus < MIN_STATUS || iStatus > MAX_STATUS )
		iStatus = DEFAULT_STATUS;
	status = iStatus;

	// load last status
	_snprintf(setting, sizeof(setting), "%s%s", PREFIX_LAST, szName);
	iStatus = DBGetContactSettingWord(NULL, MODULENAME, setting, 0);
	if ( iStatus < MIN_STATUS || iStatus > MAX_STATUS )
		iStatus = DEFAULT_STATUS;
	lastStatus = iStatus;

	szMsg = GetStatusMessage(profile, szName);
	if ( szMsg )
		szMsg = _strdup( szMsg );
}

TSSSetting::~TSSSetting()
{
	if ( szMsg != NULL )
		free( szMsg );
}

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE 
	hOptionsHook,
	hShutdownHook,
	hAccChangeHook,
	hProtoAckHook,
	hCSStatusChangeHook,
	hStatusChangeHook;

static HWND hMessageWindow;

static BYTE showDialogOnStartup = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// command line options

static PROTOCOLSETTINGEX* IsValidProtocol(TSettingsList& protoSettings, char* protoName)
{
	for ( int i=0; i < protoSettings.getCount(); i++ )
		if ( !strncmp( protoSettings[i].szName, protoName, strlen(protoSettings[i].szName )))
			return &protoSettings[i];
	
	return NULL;
}

static int IsValidStatusDesc(char* statusDesc)
{
	if (!strncmp("away", statusDesc, 4))
		return ID_STATUS_AWAY;
	if (!strncmp("na", statusDesc, 2))
		return ID_STATUS_NA;
	if (!strncmp("dnd", statusDesc, 3))
		return ID_STATUS_DND;
	if (!strncmp("occupied", statusDesc, 8))
		return ID_STATUS_OCCUPIED;
	if (!strncmp("freechat", statusDesc, 8))
		return ID_STATUS_FREECHAT;
	if (!strncmp("online", statusDesc, 6))
		return ID_STATUS_ONLINE;
	if (!strncmp("offline", statusDesc, 7))
		return ID_STATUS_OFFLINE;
	if (!strncmp("invisible", statusDesc, 9))
		return ID_STATUS_INVISIBLE;
	if (!strncmp("onthephone", statusDesc, 10))
		return ID_STATUS_ONTHEPHONE;
	if (!strncmp("outtolunch", statusDesc, 10))
		return ID_STATUS_OUTTOLUNCH;
	if (!strncmp("last", statusDesc, 4))
		return ID_STATUS_LAST;
	
	return 0;
}

static void ProcessCommandLineOptions(TSettingsList& protoSettings)
{
	if ( protoSettings.getCount() == 0 ) 
		return;

	char *cmdl = GetCommandLineA();
	while (*cmdl != '\0') {
		while (*cmdl != '/') {
			if (*cmdl == '\0') 
				return;

			cmdl++;
		}
		if (*cmdl == '\0') 			
			return;

		cmdl++;
		if (!strncmp(cmdl, "showdialog", 10)) {
			showDialogOnStartup = TRUE;
			continue;
		}
		char *protoName = cmdl; // first protocol ?
		PROTOCOLSETTINGEX* protoSetting = IsValidProtocol(protoSettings, protoName);
		if (protoSetting != NULL) {
			while (*cmdl != '=') {
				if (*cmdl == '\0') 
					return;

				cmdl++; // skip to status
			}

			if (*cmdl == '\0') 
				return;

			cmdl++;
			char *statusDesc = cmdl;
			int status = IsValidStatusDesc(statusDesc);
			if (status != 0)
				protoSetting->status = status;
}	}	}

static void SetLastStatusMessages(TSettingsList& ps)
{
	for (int i=0; i < ps.getCount(); i++ ) {
		if (ps[i].status != ID_STATUS_LAST)
			continue;

		char dbSetting[128];
		_snprintf(dbSetting, sizeof(dbSetting), "%s%s", PREFIX_LASTMSG, ps[i].szName);

		DBVARIANT dbv;
		if ( ps[i].szMsg == NULL && !DBGetContactSetting(NULL, MODULENAME, dbSetting, &dbv)) {
			ps[i].szMsg = _strdup(dbv.pszVal); // remember this won't be freed
			DBFreeVariant(&dbv);
}	}	}

/////////////////////////////////////////////////////////////////////////////////////////
// Account control event

int OnAccChanged(WPARAM wParam,LPARAM lParam)
{
	PROTOACCOUNT* pa = ( PROTOACCOUNT* )lParam;
	switch( wParam ) {
	case PRAC_ADDED:
		startupSettings.insert( new TSSSetting( -1, pa ));
		break;
		
	case PRAC_REMOVED:
		{
			for ( int i=0; i < startupSettings.getCount(); i++ ) {
				if ( !lstrcmpA( startupSettings[i].szName, pa->szModuleName )) {
					startupSettings.remove( i );
					break;
		}	}	}
		break;
	}

	return 0;
}

// 'allow override'
static int ProcessProtoAck(WPARAM wParam,LPARAM lParam)
{
	// 'something' made a status change
	ACKDATA *ack=(ACKDATA*)lParam;
	if ( ack->type != ACKTYPE_STATUS && ack->result != ACKRESULT_FAILED )
		return 0;

	if ( !DBGetContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0 )
		return 0;

	for (int i=0; i < startupSettings.getCount(); i++) {
		if ( !strcmp( ack->szModule, startupSettings[i].szName )) {
			startupSettings[i].szName = "";
			log_debugA("StartupStatus: %s overridden by ME_PROTO_ACK, status will not be set", ack->szModule);
	}	}

	return 0;
}

static int StatusChange(WPARAM wParam, LPARAM lParam)
{
	// change by menu
	if ( !DBGetContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0 )
		return 0;

	char* szProto = (char *)lParam;
	if (szProto == NULL) { // global status change
		for ( int i=0; i < startupSettings.getCount(); i++ ) {
			startupSettings[i].szName = "";
			log_debugA("StartupStatus: all protos overridden by ME_CLIST_STATUSMODECHANGE, status will not be set");
		}
	}
	else {
		for ( int i=0; i < startupSettings.getCount(); i++) {
			if ( !strcmp( startupSettings[i].szName, szProto )) {
				startupSettings[i].szName = "";
				log_debugA("StartupStatus: %s overridden by ME_CLIST_STATUSMODECHANGE, status will not be set", szProto);
	}	}	}

	return 0;
}

static int CSStatusChangeEx(WPARAM wParam, LPARAM lParam)
{
	// another status plugin made the change
	if ( !DBGetContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, 1) || startupSettings.getCount() == 0 )
		return 0;

	if (wParam != 0) {
		PROTOCOLSETTINGEX** ps = *(PROTOCOLSETTINGEX***)wParam;
		if (ps == NULL)
			return -1;

		for (int i=0; i < startupSettings.getCount(); i++ ) {
			for ( int j=0; j < startupSettings.getCount(); j++ ) {
				if ( ps[i]->szName == NULL || startupSettings[j].szName == NULL )
					continue;

				if ( !strcmp( ps[i]->szName, startupSettings[j].szName )) {
					log_debugA("StartupStatus: %s overridden by MS_CS_SETSTATUSEX, status will not be set", ps[i]->szName);
					// use a hack to disable this proto
					startupSettings[j].szName = "";
	}	}	}	}

	return 0;
}

static VOID CALLBACK SetStatusTimed(HWND hwnd,UINT message, UINT_PTR idEvent,DWORD dwTime)
{
	KillTimer(NULL, setStatusTimerId);
	UnhookEvent(hProtoAckHook);
	UnhookEvent(hCSStatusChangeHook);
	UnhookEvent(hStatusChangeHook);
	CallService(MS_CS_SETSTATUSEX, (WPARAM)&startupSettings, 0);
}

static int Exit(WPARAM wParam, LPARAM lParam)
{
	DeinitProfilesModule();
	UnhookEvent(hOptionsHook);
	UnhookEvent(hShutdownHook);
	UnhookEvent(hAccChangeHook);

	// save last protocolstatus
	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );

	for ( int i=0; i < count; i++ ) {
		if ( !IsSuitableProto( protos[i] ))
			continue;

		char lastName[128], lastMsg[128];
		mir_snprintf(lastName, sizeof(lastName), "%s%s", PREFIX_LAST, protos[i]->szModuleName);
		if (CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protos[i]->szModuleName)) {
			DBWriteContactSettingWord(NULL, MODULENAME, lastName, (WORD)CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0));
			mir_snprintf(lastMsg, sizeof(lastMsg), "%s%s", PREFIX_LASTMSG, protos[i]->szModuleName);
			DBDeleteContactSetting(NULL, MODULENAME, lastMsg);
			
			int status = CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0);
			if ( !CallService(MS_PROTO_ISPROTOCOLLOADED, 0, (LPARAM)protos[i]->szModuleName))
				continue;

			if ( !(CallProtoService(protos[i]->szModuleName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG ))
				continue;

			if ( !(CallProtoService(protos[i]->szModuleName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(status)))
				continue;

			if (ServiceExists(MS_NAS_GETSTATE)) {
				// NewAwaySys
				NAS_PROTOINFO npi = { 0 };
				npi.cbSize = sizeof(NAS_PROTOINFO);
				npi.szProto = protos[i]->szModuleName;
				npi.status = 0;
				CallService(MS_NAS_GETSTATEA, (WPARAM)&npi, (LPARAM)1);
				if (npi.szMsg == NULL) {
					npi.status = 0;
					npi.szProto = NULL;
					CallService(MS_NAS_GETSTATEA, (WPARAM)&npi, (LPARAM)1);
				}
				if (npi.szMsg != NULL) {
					DBWriteContactSettingString(NULL, MODULENAME, lastMsg, npi.szMsg);
					mir_free(npi.szMsg);
	}	}	}	}

	if ( (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, 1)) || (DBGetContactSettingByte(NULL, MODULENAME, SETTING_OFFLINECLOSE, 0))) {
		if (ServiceExists(MS_CLIST_SETSTATUSMODE))
			CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)ID_STATUS_OFFLINE, 0);
		else
			log_debugA("StartupStatus: MS_CLIST_SETSTATUSMODE not available!");
	}

	// set windowstate and docked for next startup
	if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSTATE, 0)) {
		int state = DBGetContactSettingByte(NULL, MODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
		HWND hClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
		BOOL isHidden = !IsWindowVisible(hClist);
		switch (state) {
		case SETTING_STATE_HIDDEN:
			// try to use services where possible
			if (!isHidden)
				CallService(MS_CLIST_SHOWHIDE, 0, 0);
			break;

		case SETTING_STATE_MINIMIZED:
			if (!DBGetContactSettingByte(NULL, MODULE_CLIST, SETTING_TOOLWINDOW, 0))
				ShowWindow(hClist, SW_SHOWMINIMIZED);
			break;
			
		case SETTING_STATE_NORMAL:
			// try to use services where possible (that's what they're for)
			if (isHidden)
				CallService(MS_CLIST_SHOWHIDE, 0, 0);
			break;
	}	}

	// hangup
	if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_AUTOHANGUP, 0))
		InternetAutodialHangup(0);
	
	int state = DBGetContactSettingByte(NULL, MODULENAME, SETTING_WINSTATE, SETTING_STATE_NORMAL);
	// set windowstate and docked for next startup
	if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSTATE, 0))
		DBWriteContactSettingByte(NULL, MODULE_CLIST, SETTING_WINSTATE, (BYTE)state);
			
	if ( hMessageWindow )
		DestroyWindow(hMessageWindow);

	startupSettings.destroy();
	return 0;
}

/* Window proc for poweroff event */
static DWORD CALLBACK MessageWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_ENDSESSION:
		log_debugA("WM_ENDSESSION");
		if (wParam) {
			log_debugA("WM_ENDSESSION: calling exit");
			Exit(0, 0);
			log_debugA("WM_ENDSESSION: exit called");
		}
		break;
	}

	return TRUE;
}

int CSModuleLoaded(WPARAM wParam, LPARAM lParam)
{
	protoList = ( OBJLIST<PROTOCOLSETTINGEX>* )&startupSettings;

	InitProfileModule();

	hAccChangeHook = HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);
	hOptionsHook = HookEvent(ME_OPT_INITIALISE, OptionsInit);

	/* shutdown hook for normal shutdown */
	hShutdownHook = HookEvent(ME_SYSTEM_OKTOEXIT, Exit);
	/* message window for poweroff */
	hMessageWindow = CreateWindowEx(0, _T("STATIC"), NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	SetWindowLongPtr(hMessageWindow, GWLP_WNDPROC, (LONG_PTR)MessageWndProc);

	GetProfile( -1, startupSettings );

	// override with cmdl
	ProcessCommandLineOptions( startupSettings );
	if (startupSettings.getCount() == 0)
		return 0;// no protocols are loaded

	SetLastStatusMessages(startupSettings);
	showDialogOnStartup = (showDialogOnStartup || DBGetContactSettingByte(NULL, MODULENAME, SETTING_SHOWDIALOG, 0));

	// dial
	if ( showDialogOnStartup || DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, 1))
		if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_AUTODIAL, 0))
			InternetAutodial(0, NULL);

	// set the status!
	if ( showDialogOnStartup || DBGetContactSettingByte(NULL, MODULENAME, SETTING_SHOWDIALOG, 0)) {
		CallService(MS_CS_SHOWCONFIRMDLGEX, (WPARAM)&startupSettings, DBGetContactSettingDword(NULL, MODULENAME, SETTING_DLGTIMEOUT, 5));
	}
	else if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, 1)) {
		// set hooks for override
		if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_OVERRIDE, 1))  {
			hProtoAckHook = HookEvent(ME_PROTO_ACK, ProcessProtoAck);
			hCSStatusChangeHook = HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChangeEx);
			hStatusChangeHook = HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChange);
		}
		setStatusTimerId = SetTimer(NULL, 0, DBGetContactSettingDword(NULL,MODULENAME,SETTING_SETPROFILEDELAY,500), SetStatusTimed);
	}

	// win size and location
	if ( DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINLOCATION, 0) ||
		  DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSIZE, 0))
	{
		WINDOWPLACEMENT wndpl = { 0 };
		wndpl.length = sizeof(wndpl);
		
		HWND hClist = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

		// store in db
		if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINLOCATION, 0)) {
			DBWriteContactSettingDword(NULL, MODULE_CLIST, SETTING_XPOS, DBGetContactSettingDword(NULL, MODULENAME, SETTING_XPOS, 0));
			DBWriteContactSettingDword(NULL, MODULE_CLIST, SETTING_YPOS, DBGetContactSettingDword(NULL, MODULENAME, SETTING_YPOS, 0));
		}
		if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSIZE, 0)) {
			DBWriteContactSettingDword(NULL, MODULE_CLIST, SETTING_WIDTH, DBGetContactSettingDword(NULL, MODULENAME, SETTING_WIDTH, 0));
			if (!DBGetContactSettingByte(NULL, MODULE_CLUI, SETTING_AUTOSIZE, 0))
				DBWriteContactSettingDword(NULL, MODULE_CLIST, SETTING_HEIGHT, DBGetContactSettingDword(NULL, MODULENAME, SETTING_HEIGHT, 0));
		}
		if ( GetWindowPlacement( hClist, &wndpl )) {
			if ( wndpl.showCmd == SW_SHOWNORMAL && !CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) {
				RECT rc;
				if (GetWindowRect(hClist, &rc)) {
					int x = rc.left;
					int y = rc.top;
					int width = rc.right - rc.left;
					int height = rc.bottom - rc.top;
					if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINLOCATION, 0)) {
						x = DBGetContactSettingDword(NULL, MODULENAME, SETTING_XPOS, x);
						y = DBGetContactSettingDword(NULL, MODULENAME, SETTING_YPOS, y);
					}
					if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETWINSIZE, 0)) {
						width = DBGetContactSettingDword(NULL, MODULENAME, SETTING_WIDTH, width);
						if (!DBGetContactSettingByte(NULL, MODULE_CLUI, SETTING_AUTOSIZE, 0))
							height = DBGetContactSettingDword(NULL, MODULENAME, SETTING_HEIGHT, height);
					}
					MoveWindow(hClist, x, y, width, height, TRUE);
	}	}	}	}
	
	return 0;
}
