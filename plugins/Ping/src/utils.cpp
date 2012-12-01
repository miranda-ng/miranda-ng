#include "common.h"
#include "utils.h"
#include "icmp.h"

LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_COMMAND: {
			PUDeletePopUp( hWnd );
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp( hWnd );
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK sttMainThreadCallback( ULONG_PTR dwParam )
{
	POPUPDATAEX* ppd = ( POPUPDATAEX* )dwParam;

	if ( ServiceExists(MS_POPUP_ADDPOPUPEX) )
		PUAddPopUpEx(ppd);

	free( ppd );
}

void __stdcall	ShowPopup( const char* line1, const char* line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( ServiceExists( MS_POPUP_ADDPOPUP )) {
		POPUPDATAEX* ppd = ( POPUPDATAEX* )calloc( sizeof( POPUPDATAEX ), 1 );

		ppd->lchContact = NULL;
		ppd->lchIcon = (flags ? hIconResponding : hIconNotResponding);
		strncpy( ppd->lpzContactName, line1,MAX_CONTACTNAME);
		strncpy( ppd->lpzText, line2, MAX_SECONDLINE);

		ppd->colorBack = GetSysColor( COLOR_BTNFACE );
		ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
		ppd->iSeconds = 10;

		ppd->PluginWindowProc = NullWindowProc;
		ppd->PluginData = NULL;

		QueueUserAPC( sttMainThreadCallback , mainThread, ( ULONG )ppd );
	}
	else if(ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		POPUPDATACLASS d = {sizeof(d), "pingpopups"};
		d.pwszTitle = (wchar_t *)line1;
		d.pwszText = (wchar_t *)line2;
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
	} else {
		MessageBox( NULL, line2, PLUG " Message", MB_OK | MB_ICONINFORMATION );
		return;
	}
}

// service functions

// wParam is zero
// lParam is address of PINGADDRESS structure where ping result is placed (i.e. modifies 'responding' 
// and 'round_trip_time')
INT_PTR PluginPing(WPARAM wParam,LPARAM lParam)
{
	PINGADDRESS *pa = (PINGADDRESS *)lParam;

	if(pa->port == -1) {
		// ICMP echo
		if(use_raw_ping) {
			pa->round_trip_time = raw_ping(pa->pszName, options.ping_timeout * 1000);
			pa->responding = (pa->round_trip_time != -1);
		} else {
		
			ICMP_ECHO_REPLY result;
			pa->responding = ICMP::get_instance()->ping(pa->pszName, result);
			if(pa->responding)
				pa->round_trip_time = (short)result.RoundTripTime;
			else
				pa->round_trip_time = -1;
		}
	} else if(hNetlibUser) {
		// TCP connect

		clock_t start_tcp = clock();
		
		//GetLocalTime(&systime);
		NETLIBOPENCONNECTION conn = {0};
		conn.cbSize = sizeof(NETLIBOPENCONNECTION);
		conn.szHost = pa->pszName;
		conn.wPort = pa->port;
		conn.timeout = options.ping_timeout;

		HANDLE s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&conn);

		clock_t end_tcp = clock();
		
		if(s) {
			LINGER l;
			char buf[1024];
			SOCKET socket = (SOCKET)CallService(MS_NETLIB_GETSOCKET, (WPARAM)s, (LPARAM)NLOCF_HTTP);
			l.l_onoff = 1;
			l.l_linger = 0;
			setsockopt(socket, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));

			Netlib_Send(s, "OUT\r\n\r\n", 7, 0); //MSG_RAW);

			//Sleep(ICMP::get_instance()->get_timeout());
			Sleep(options.ping_timeout * 1000);
			unsigned long bytes_remaining;
			ioctlsocket(socket, FIONBIO, &bytes_remaining);

			if(bytes_remaining > 0) {
				int retval, rx = 0;
				while((retval = Netlib_Recv(s, buf, 1024, 0)) != SOCKET_ERROR && (retval > 0) && rx < 2048) {
					rx += retval; // recv at most 2kb before closing connection
				}
			}
			closesocket(socket);
			pa->responding = true;
			pa->round_trip_time = (int)(((end_tcp - start_tcp) / (double)CLOCKS_PER_SEC) * 1000);

			Netlib_CloseHandle(s);
		} else {
			pa->responding = false;
			pa->round_trip_time = -1;
		}

	}
	return 0;
}

void Lock(CRITICAL_SECTION *cs, char *lab) {
//	if(logging) {
//		std::ostringstream oss1;
//		oss1 << "Locking cs: " << cs << ", " << lab;
//		CallService(PROTO "/Log", (WPARAM)oss1.str().c_str(), 0);
//	}
	EnterCriticalSection(cs);
//	if(logging) {
//		std::ostringstream oss2;
//		oss2 << "Locked cs: " << cs;
//		CallService(PROTO "/Log", (WPARAM)oss2.str().c_str(), 0);
//	}
}

void Unlock(CRITICAL_SECTION *cs) {
//	if(logging) {
//		std::ostringstream oss1;
//		oss1 << "Unlocking cs: " << cs;
//		CallService(PROTO "/Log", (WPARAM)oss1.str().c_str(), 0);
//	}
	LeaveCriticalSection(cs);
}

INT_PTR PingDisableAll(WPARAM wParam, LPARAM lParam) {
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(PINGLIST::Iterator i = pl.start(); i.has_val(); i.next()) {
		i.val().status = PS_DISABLED;
		i.val().miss_count = 0;
	}
	CallService(PLUG "/SetPingList", (WPARAM)&pl, 0);
	return 0;
}

INT_PTR PingEnableAll(WPARAM wParam, LPARAM lParam) {
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(PINGLIST::Iterator i = pl.start(); i.has_val(); i.next()) {
		if(i.val().status == PS_DISABLED) {
			i.val().status = PS_NOTRESPONDING;
		}
	}
	CallService(PLUG "/SetPingList", (WPARAM)&pl, 0);
	return 0;
}


INT_PTR ToggleEnabled(WPARAM wParam, LPARAM lParam) {
	int retval = 0;
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(PINGLIST::Iterator i = pl.start(); i.has_val(); i.next()) {
		if(i.val().item_id == (DWORD)wParam) {

			if(i.val().status == PS_DISABLED)
				i.val().status = PS_NOTRESPONDING;
			else {
				i.val().status = PS_DISABLED;
				i.val().miss_count = 0;
				retval = 1;
			}
		}
	}
	CallService(PLUG "/SetPingList", (WPARAM)&pl, 0);
	return 0;
}

INT_PTR EditContact(WPARAM wParam, LPARAM lParam) {
	PINGLIST pl;
	HWND hwndList = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);

	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(PINGLIST::Iterator i = pl.start(); i.has_val(); i.next()) {
		if(i.val().item_id == (DWORD)wParam) {

			add_edit_addr = i.val();
	
			if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwndList, DlgProcDestEdit) == IDOK) {
	
				i.val() = add_edit_addr;
				CallService(PLUG "/SetAndSavePingList", (WPARAM)&pl, 0);
				return 0;
			}
		}
	}
	return 1;
}

INT_PTR DblClick(WPARAM wParam, LPARAM lParam) {
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(PINGLIST::Iterator i = pl.start(); i.has_val(); i.next()) {
		if(i.val().item_id == (DWORD)wParam) {
			if(strlen(i.val().pszCommand)) {
				ShellExecute(0, "open", i.val().pszCommand, i.val().pszParams, 0, SW_SHOW);
			} else {
				return CallService(PLUG "/ToggleEnabled", wParam, 0);
			}
		}
	}
	return 0;
}


void import_ping_address(int index, PINGADDRESS &pa) {
	DBVARIANT dbv;
	char buf[256];
	mir_snprintf(buf, 256, "Address%d", index);
	if(!DBGetContactSetting(0, "PingPlug", buf, &dbv)) {
		strncpy(pa.pszName, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else
		strcpy(pa.pszName, Translate("Unknown Address"));

	mir_snprintf(buf, 256, "Label%d", index);
	if(!DBGetContactSetting(0, "PingPlug", buf, &dbv)) {
		strncpy(pa.pszLabel, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
	} else
		strcpy(pa.pszLabel, Translate("Unknown"));

	mir_snprintf(buf, 256, "Port%d", index);
	pa.port = (int)DBGetContactSettingDword(0, "PingPlug", buf, -1);

	mir_snprintf(buf, 256, "Proto%d", index);
	if(!DBGetContactSetting(0, "PingPlug", buf, &dbv)) {
		strncpy(pa.pszProto, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		DBFreeVariant(&dbv);
		mir_snprintf(buf, 256, "Status%d", index);
		pa.set_status = DBGetContactSettingWord(0, "PingPlug", buf, ID_STATUS_ONLINE);
		mir_snprintf(buf, 256, "Status2%d", index);
		pa.get_status = DBGetContactSettingWord(0, "PingPlug", buf, ID_STATUS_OFFLINE);
	} else
		pa.pszProto[0] = '\0';


	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = index;
	pa.pszCommand[0] = '\0';
	pa.pszParams[0] = '\0';

	pa.item_id = 0;
	mir_snprintf(buf, 256, "Enabled%d", index);
	if(DBGetContactSettingByte(0, "PingPlug", buf, 1) == 1)
		pa.status = PS_NOTRESPONDING;
	else
		pa.status = PS_DISABLED;
}

// read in addresses from old pingplug
void import_ping_addresses() {
	int count = DBGetContactSettingDword(0, "PingPlug", "NumEntries", 0);
	PINGADDRESS pa;

	EnterCriticalSection(&list_cs);
	list_items.clear();
	for(int index = 0; index < count; index++) {
		import_ping_address(index, pa);
		list_items.add(pa);
	}
	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
}

HANDLE hIcoLibIconsChanged;

HICON hIconResponding, hIconNotResponding, hIconTesting, hIconDisabled;

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hIconResponding = Skin_GetIcon("ping_responding");
	hIconNotResponding = Skin_GetIcon("ping_not_responding");
	hIconTesting = Skin_GetIcon("ping_testing");
	hIconDisabled = Skin_GetIcon("ping_disabled");

	RefreshWindow(0, 0);
	return 0;
}

void InitUtils()
{
	TCHAR file[MAX_PATH];
	GetModuleFileName(hInst,file,MAX_PATH);
	{
		SKINICONDESC sid = {0};

		sid.cbSize = sizeof(SKINICONDESC);
		sid.ptszSection = LPGENT("Ping");
		sid.flags = SIDF_PATH_TCHAR;

		sid.pszDescription = LPGEN("Responding");
		sid.pszName = "ping_responding";
		sid.ptszDefaultFile = file;
		sid.iDefaultIndex = -IDI_ICON_RESPONDING;
		Skin_AddIcon(&sid);

		sid.pszDescription = LPGEN("Not Responding");
		sid.pszName = "ping_not_responding";
		sid.ptszDefaultFile = file;
		sid.iDefaultIndex = -IDI_ICON_NOTRESPONDING;
		Skin_AddIcon(&sid);

		sid.pszDescription = LPGEN("Testing");
		sid.pszName = "ping_testing";
		sid.ptszDefaultFile = file;
		sid.iDefaultIndex = -IDI_ICON_TESTING;
		Skin_AddIcon(&sid);

		sid.pszDescription = LPGEN("Disabled");
		sid.pszName = "ping_disabled";
		sid.ptszDefaultFile = file;
		sid.iDefaultIndex = -IDI_ICON_DISABLED;
		Skin_AddIcon(&sid);

		hIconResponding = Skin_GetIcon("ping_responding");
		hIconNotResponding = Skin_GetIcon("ping_not_responding");
		hIconTesting = Skin_GetIcon("ping_testing");
		hIconDisabled = Skin_GetIcon("ping_disabled");

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	}

	if(ServiceExists(MS_POPUP_REGISTERCLASS)) {
		POPUPCLASS test = {0};
		test.cbSize = sizeof(test);
		test.flags = PCF_TCHAR;
		test.hIcon = hIconResponding;
		test.iSeconds = -1;
		test.ptszDescription = TranslateT("Ping");
		test.pszName = "pingpopups";
		test.PluginWindowProc = NullWindowProc;
		CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&test);
	}
}