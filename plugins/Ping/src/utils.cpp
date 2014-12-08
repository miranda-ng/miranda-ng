#include "common.h"

LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_COMMAND: {
			PUDeletePopup( hWnd );
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopup( hWnd );
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK sttMainThreadCallback( ULONG_PTR dwParam )
{
	POPUPDATAT* ppd = ( POPUPDATAT* )dwParam;

	if ( ServiceExists(MS_POPUP_ADDPOPUPT))
		PUAddPopupT(ppd);

	free( ppd );
}

void __stdcall	ShowPopup(TCHAR *line1,TCHAR *line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	 if(ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
		ShowClassPopupT("pingpopups",line1,line2);
	} else if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT *ppd = ( POPUPDATAT* )calloc( sizeof( POPUPDATAT ), 1 );

		ppd->lchContact = NULL;
		ppd->lchIcon = (flags ? hIconResponding : hIconNotResponding);
		_tcsncpy( ppd->lptzContactName, line1, MAX_CONTACTNAME);
		_tcsncpy( ppd->lptzText, line2, MAX_SECONDLINE);

		ppd->colorBack = GetSysColor( COLOR_BTNFACE );
		ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
		ppd->iSeconds = 10;

		ppd->PluginWindowProc = NullWindowProc;
		ppd->PluginData = NULL;

		QueueUserAPC(sttMainThreadCallback, mainThread, ( ULONG_PTR)ppd );
	}
	else{
		MessageBox( NULL, line2, _T(PLUG) _T(" Message"), MB_OK | MB_ICONINFORMATION );
		return;
	}
}

// service functions

// wParam is zero
// lParam is address of PINGADDRESS structure where ping result is placed (i.e. modifies 'responding' 
// and 'round_trip_time')
INT_PTR PluginPing(WPARAM,LPARAM lParam)
{
	PINGADDRESS *pa = (PINGADDRESS *)lParam;

	if(pa->port == -1) {
		// ICMP echo
		if(use_raw_ping) {
			pa->round_trip_time = raw_ping(_T2A(pa->pszName), options.ping_timeout * 1000);
			pa->responding = (pa->round_trip_time != -1);
		} else {
		
			ICMP_ECHO_REPLY result;
			pa->responding = ICMP::get_instance()->ping(_T2A(pa->pszName), result);
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
		conn.szHost = mir_t2a(pa->pszName);
		conn.wPort = pa->port;
		conn.timeout = options.ping_timeout;

		HANDLE s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)hNetlibUser, (LPARAM)&conn);
		mir_free((void*)conn.szHost);

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
	for(pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		i->status = PS_DISABLED;
		i->miss_count = 0;
	}
	CallService(PLUG "/SetPingList", (WPARAM)&pl, 0);
	return 0;
}

INT_PTR PingEnableAll(WPARAM wParam, LPARAM lParam) {
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		if(i->status == PS_DISABLED) {
			i->status = PS_NOTRESPONDING;
		}
	}
	CallService(PLUG "/SetPingList", (WPARAM)&pl, 0);
	return 0;
}


INT_PTR ToggleEnabled(WPARAM wParam, LPARAM lParam) {
	int retval = 0;
	PINGLIST pl;
	CallService(PLUG "/GetPingList", 0, (LPARAM)&pl);
	for(pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		if(i->item_id == (DWORD)wParam) {

			if(i->status == PS_DISABLED)
				i->status = PS_NOTRESPONDING;
			else {
				i->status = PS_DISABLED;
				i->miss_count = 0;
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
	for(pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		if(i->item_id == (DWORD)wParam) {

			add_edit_addr = *i;
	
			if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwndList, DlgProcDestEdit) == IDOK) {
	
				*i = add_edit_addr;
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
	for(pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		if(i->item_id == (DWORD)wParam) {
			if(_tcslen(i->pszCommand)) {
				ShellExecute(0, _T("open"), i->pszCommand, i->pszParams, 0, SW_SHOW);
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
	mir_snprintf(buf, SIZEOF(buf), "Address%d", index);
	if(!db_get_ts(0, "PingPlug", buf, &dbv)) {
		_tcsncpy(pa.pszName, dbv.ptszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
	} else
		_tcsncpy(pa.pszName, TranslateT("Unknown Address"), MAX_PINGADDRESS_STRING_LENGTH);

	mir_snprintf(buf, SIZEOF(buf), "Label%d", index);
	if(!db_get_ts(0, "PingPlug", buf, &dbv)) {
		_tcsncpy(pa.pszLabel, dbv.ptszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
	} else
		_tcsncpy(pa.pszLabel, TranslateT("Unknown"), MAX_PINGADDRESS_STRING_LENGTH);

	mir_snprintf(buf, SIZEOF(buf), "Port%d", index);
	pa.port = (int)db_get_dw(0, "PingPlug", buf, -1);

	mir_snprintf(buf, SIZEOF(buf), "Proto%d", index);
	if(!db_get_s(0, "PingPlug", buf, &dbv)) {
		strncpy(pa.pszProto, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH);
		db_free(&dbv);
		mir_snprintf(buf, SIZEOF(buf), "Status%d", index);
		pa.set_status = db_get_w(0, "PingPlug", buf, ID_STATUS_ONLINE);
		mir_snprintf(buf, SIZEOF(buf), "Status2%d", index);
		pa.get_status = db_get_w(0, "PingPlug", buf, ID_STATUS_OFFLINE);
	} else
		pa.pszProto[0] = '\0';


	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = index;
	pa.pszCommand[0] = '\0';
	pa.pszParams[0] = '\0';

	pa.item_id = 0;
	mir_snprintf(buf, SIZEOF(buf), "Enabled%d", index);
	if(db_get_b(0, "PingPlug", buf, 1) == 1)
		pa.status = PS_NOTRESPONDING;
	else
		pa.status = PS_DISABLED;
}

// read in addresses from old pingplug
void import_ping_addresses()
{
	int count = db_get_dw(0, "PingPlug", "NumEntries", 0);
	PINGADDRESS pa;

	EnterCriticalSection(&list_cs);
	list_items.clear();
	for(int index = 0; index < count; index++)
	{
		import_ping_address(index, pa);
		list_items.push_back(pa);
	}
	write_ping_addresses();
	LeaveCriticalSection(&list_cs);
}

HANDLE hPopupClass;
HICON  hIconResponding, hIconNotResponding, hIconTesting, hIconDisabled;

static int OnShutdown(WPARAM, LPARAM)
{
	Popup_UnregisterClass(hPopupClass);
	return 0;
}

int ReloadIcons(WPARAM, LPARAM)
{
	hIconResponding = Skin_GetIcon("ping_responding");
	hIconNotResponding = Skin_GetIcon("ping_not_responding");
	hIconTesting = Skin_GetIcon("ping_testing");
	hIconDisabled = Skin_GetIcon("ping_disabled");

	RefreshWindow(0, 0);
	return 0;
}

void InitUtils()
{
	hIconResponding = Skin_GetIcon("ping_responding");
	hIconNotResponding = Skin_GetIcon("ping_not_responding");
	hIconTesting = Skin_GetIcon("ping_testing");
	hIconDisabled = Skin_GetIcon("ping_disabled");

	POPUPCLASS test = { sizeof(test) };
	test.flags = PCF_TCHAR;
	test.hIcon = hIconResponding;
	test.iSeconds = -1;
	test.ptszDescription = TranslateT("Ping");
	test.pszName = "pingpopups";
	test.PluginWindowProc = NullWindowProc;
	if (hPopupClass = Popup_RegisterClass(&test))
		HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
}
