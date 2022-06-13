#include "stdafx.h"

LRESULT CALLBACK NullWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		PUDeletePopup(hWnd);
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void __stdcall	ShowPopup(wchar_t *line1, wchar_t *line2, int)
{
	if (Miranda_IsTerminated())
		return;

	ShowClassPopupW("pingpopups", line1, line2);
}

// service functions

// wParam is zero
// lParam is address of PINGADDRESS structure where ping result is placed (i.e. modifies 'responding' 
// and 'round_trip_time')
INT_PTR PluginPing(WPARAM, LPARAM lParam)
{
	PINGADDRESS *pa = (PINGADDRESS *)lParam;
	if (pa->port == -1) {
		// ICMP echo
		if (use_raw_ping) {
			pa->round_trip_time = raw_ping(_T2A(pa->pszName), options.ping_timeout * 1000);
			pa->responding = (pa->round_trip_time != -1);
		}
		else {

			ICMP_ECHO_REPLY result;
			pa->responding = ICMP::get_instance()->ping(_T2A(pa->pszName), result);
			if (pa->responding)
				pa->round_trip_time = (short)result.RoundTripTime;
			else
				pa->round_trip_time = -1;
		}
	}
	else if (hNetlibUser) {
		// TCP connect

		clock_t start_tcp = clock();

		HNETLIBCONN s = Netlib_OpenConnection(hNetlibUser, _T2A(pa->pszName), pa->port, options.ping_timeout);

		clock_t end_tcp = clock();

		if (s) {
			LINGER l;
			char buf[1024];
			SOCKET socket = Netlib_GetSocket(s);
			l.l_onoff = 1;
			l.l_linger = 0;
			setsockopt(socket, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l));

			Netlib_Send(s, "OUT\r\n\r\n", 7, 0); //MSG_RAW);

			//Sleep(ICMP::get_instance()->get_timeout());
			Sleep(options.ping_timeout * 1000);
			unsigned long bytes_remaining;
			ioctlsocket(socket, FIONBIO, &bytes_remaining);

			if (bytes_remaining > 0) {
				int retval, rx = 0;
				while ((retval = Netlib_Recv(s, buf, 1024, 0)) != SOCKET_ERROR && (retval > 0) && rx < 2048) {
					rx += retval; // recv at most 2kb before closing connection
				}
			}
			closesocket(socket);
			pa->responding = true;
			pa->round_trip_time = (int)(((end_tcp - start_tcp) / (double)CLOCKS_PER_SEC) * 1000);

			Netlib_CloseHandle(s);
		}
		else {
			pa->responding = false;
			pa->round_trip_time = -1;
		}

	}
	return 0;
}

INT_PTR PingDisableAll(WPARAM, LPARAM)
{
	PINGLIST pl;
	GetPingList(pl);
	
	for (auto &it: pl) {
		it.status = PS_DISABLED;
		it.miss_count = 0;
	}
	SetPingList(pl);
	return 0;
}

INT_PTR PingEnableAll(WPARAM, LPARAM)
{
	PINGLIST pl;
	GetPingList(pl);

	for (auto &it : pl)
		if (it.status == PS_DISABLED)
			it.status = PS_NOTRESPONDING;

	SetPingList(pl);
	return 0;
}


INT_PTR ToggleEnabled(WPARAM wParam, LPARAM)
{
	PINGLIST pl;
	GetPingList(pl);

	for (auto &it : pl) {
		if (it.item_id == (uint32_t)wParam) {

			if (it.status == PS_DISABLED)
				it.status = PS_NOTRESPONDING;
			else {
				it.status = PS_DISABLED;
				it.miss_count = 0;
			}
		}
	}
	SetPingList(pl);
	return 0;
}

INT_PTR EditContact(WPARAM wParam, LPARAM)
{
	PINGLIST pl;
	GetPingList(pl);

	HWND hwndList = g_clistApi.hwndContactList;
	for (auto &it : pl) {
		if (it.item_id == (uint32_t)wParam) {
			add_edit_addr = it;

			if (DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DIALOG3), hwndList, DlgProcDestEdit) == IDOK) {
				it = add_edit_addr;
				SetAndSavePingList(pl);
				return 0;
			}
		}
	}
	return 1;
}

INT_PTR DblClick(WPARAM wParam, LPARAM)
{
	PINGLIST pl;
	GetPingList(pl);

	for (auto &it : pl) {
		if (it.item_id == (uint32_t)wParam) {
			if (mir_wstrlen(it.pszCommand)) {
				ShellExecute(nullptr, L"open", it.pszCommand, it.pszParams, nullptr, SW_SHOW);
			}
			else {
				return CallService(MODULENAME "/ToggleEnabled", wParam, 0);
			}
		}
	}
	return 0;
}

void import_ping_address(int index, PINGADDRESS &pa)
{
	DBVARIANT dbv;
	char buf[256];
	mir_snprintf(buf, "Address%d", index);
	if (!db_get_ws(0, "PingPlug", buf, &dbv)) {
		mir_wstrncpy(pa.pszName, dbv.pwszVal, _countof(pa.pszName));
		db_free(&dbv);
	}
	else mir_wstrncpy(pa.pszName, TranslateT("Unknown Address"), _countof(pa.pszName));

	mir_snprintf(buf, "Label%d", index);
	if (!db_get_ws(0, "PingPlug", buf, &dbv)) {
		mir_wstrncpy(pa.pszLabel, dbv.pwszVal, _countof(pa.pszLabel));
		db_free(&dbv);
	}
	else mir_wstrncpy(pa.pszLabel, TranslateT("Unknown"), _countof(pa.pszLabel));

	mir_snprintf(buf, "Port%d", index);
	pa.port = (int)db_get_dw(0, "PingPlug", buf, -1);

	mir_snprintf(buf, "Proto%d", index);
	if (!db_get_s(0, "PingPlug", buf, &dbv)) {
		mir_strncpy(pa.pszProto, dbv.pszVal, _countof(pa.pszProto));
		db_free(&dbv);
		mir_snprintf(buf, "Status%d", index);
		pa.set_status = db_get_w(0, "PingPlug", buf, ID_STATUS_ONLINE);
		mir_snprintf(buf, "Status2%d", index);
		pa.get_status = db_get_w(0, "PingPlug", buf, ID_STATUS_OFFLINE);
	}
	else pa.pszProto[0] = '\0';

	pa.responding = false;
	pa.round_trip_time = 0;
	pa.miss_count = 0;
	pa.index = index;
	pa.pszCommand[0] = '\0';
	pa.pszParams[0] = '\0';

	pa.item_id = 0;
	mir_snprintf(buf, "Enabled%d", index);
	if (db_get_b(0, "PingPlug", buf, 1) == 1)
		pa.status = PS_NOTRESPONDING;
	else
		pa.status = PS_DISABLED;
}

// read in addresses from old pingplug
void import_ping_addresses()
{
	int count = db_get_dw(0, "PingPlug", "NumEntries", 0);
	PINGADDRESS pa;

	mir_cslock lck(list_cs);
	list_items.clear();
	for (int index = 0; index < count; index++) {
		import_ping_address(index, pa);
		list_items.push_back(pa);
	}
	write_ping_addresses();
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
	hIconResponding = IcoLib_GetIcon("ping_responding");
	hIconNotResponding = IcoLib_GetIcon("ping_not_responding");
	hIconTesting = IcoLib_GetIcon("ping_testing");
	hIconDisabled = IcoLib_GetIcon("ping_disabled");

	RefreshWindow();
	return 0;
}

void InitUtils()
{
	hIconResponding = IcoLib_GetIcon("ping_responding");
	hIconNotResponding = IcoLib_GetIcon("ping_not_responding");
	hIconTesting = IcoLib_GetIcon("ping_testing");
	hIconDisabled = IcoLib_GetIcon("ping_disabled");

	POPUPCLASS test = {};
	test.flags = PCF_UNICODE;
	test.hIcon = hIconResponding;
	test.iSeconds = -1;
	test.pszDescription.w = TranslateT("Ping");
	test.pszName = "pingpopups";
	test.PluginWindowProc = NullWindowProc;
	if (hPopupClass = Popup_RegisterClass(&test))
		HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
}
