#include "stdafx.h"

CMPlugin	g_plugin;

static HWND hTimerWnd = (HWND)nullptr;
static UINT TID = (UINT)12021;
HANDLE hCheckEvent = nullptr;
HANDLE hCheckHook = nullptr;
HANDLE hConnectionCheckThread = nullptr;
HANDLE hFilterOptionsThread = nullptr;
HANDLE killCheckThreadEvent = nullptr;
HANDLE hExceptionsMutex = nullptr;

uint32_t FilterOptionsThreadId;
uint32_t ConnectionCheckThreadId;

CONNECTION *first = nullptr;
CONNECTION *connExceptions = nullptr;
CONNECTION *connCurrentEdit;
int currentStatus = ID_STATUS_OFFLINE, diffstat = 0;
wchar_t *tcpStates[] = { L"CLOSED", L"LISTEN", L"SYN_SENT", L"SYN_RCVD", L"ESTAB", L"FIN_WAIT1", L"FIN_WAIT2", L"CLOSE_WAIT", L"CLOSING", L"LAST_ACK", L"TIME_WAIT", L"DELETE_TCB" };

int ConnectionNotifyOptInit(WPARAM wParam, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	PLUGINNAME,
	__VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,		//not transient
	//	4BB5B4AA-C364-4F23-9746-D5B708A286A5
	{ 0x4bb5b4aa, 0xc364, 0x4f23, { 0x97, 0x46, 0xd5, 0xb7, 0x8, 0xa2, 0x86, 0xa5 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(PLUGINNAME, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_PROTOCOL);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// authentication callback futnction from extension manager

BOOL strrep(wchar_t *src, wchar_t *needle, wchar_t *newstring)
{
	wchar_t *found, begining[MAX_SETTING_STR], tail[MAX_SETTING_STR];
	size_t pos = 0;

	//strset(begining, ' ');
	//strset(tail, ' ');
	if (!(found = wcsstr(src, needle)))
		return FALSE;

	pos = (found - src);
	wcsncpy_s(begining, src, pos);
	begining[pos] = 0;

	pos = pos + mir_wstrlen(needle);
	wcsncpy_s(tail, src + pos, _TRUNCATE);
	begining[pos] = 0;

	pos = mir_snwprintf(src, mir_wstrlen(src), L"%s%s%s", begining, newstring, tail);
	return TRUE;
}

void saveSettingsConnections(CONNECTION *connHead)
{
	char buff[128];
	int i = 0;
	CONNECTION *tmp = connHead;
	while (tmp != nullptr) {

		mir_snprintf(buff, "%dFilterIntIp", i);
		g_plugin.setWString(buff, tmp->strIntIp);
		mir_snprintf(buff, "%dFilterExtIp", i);
		g_plugin.setWString(buff, tmp->strExtIp);
		mir_snprintf(buff, "%dFilterPName", i);
		g_plugin.setWString(buff, tmp->PName);
		mir_snprintf(buff, "%dFilterIntPort", i);
		g_plugin.setDword(buff, tmp->intIntPort);
		mir_snprintf(buff, "%dFilterExtPort", i);
		g_plugin.setDword(buff, tmp->intExtPort);
		mir_snprintf(buff, "%dFilterAction", i);
		g_plugin.setDword(buff, tmp->Pid);
		i++;
		tmp = tmp->next;
	}
	g_plugin.iFiltersCount = i;
	g_plugin.setDword("FiltersCount", g_plugin.iFiltersCount);

}

//load filters from db
CONNECTION* LoadSettingsConnections()
{
	CONNECTION *connHead = nullptr;
	DBVARIANT dbv;
	char buff[128];
	int i = 0;
	for (i = g_plugin.iFiltersCount - 1; i >= 0; i--) {
		CONNECTION *conn = (CONNECTION*)mir_alloc(sizeof(CONNECTION));
		mir_snprintf(buff, "%dFilterIntIp", i);
		if (!g_plugin.getWString(buff, &dbv))
			wcsncpy(conn->strIntIp, dbv.pwszVal, _countof(conn->strIntIp));
		db_free(&dbv);
		mir_snprintf(buff, "%dFilterExtIp", i);
		if (!g_plugin.getWString(buff, &dbv))
			wcsncpy(conn->strExtIp, dbv.pwszVal, _countof(conn->strExtIp));
		db_free(&dbv);
		mir_snprintf(buff, "%dFilterPName", i);
		if (!g_plugin.getWString(buff, &dbv))
			wcsncpy(conn->PName, dbv.pwszVal, _countof(conn->PName));
		db_free(&dbv);

		mir_snprintf(buff, "%dFilterIntPort", i);
		conn->intIntPort = g_plugin.getDword(buff, -1);

		mir_snprintf(buff, "%dFilterExtPort", i);
		conn->intExtPort = g_plugin.getDword(buff, -1);

		mir_snprintf(buff, "%dFilterAction", i);
		conn->Pid = g_plugin.getDword(buff, 0);

		conn->next = connHead;
		connHead = conn;
	}
	return connHead;
}
//called to load settings from database
void LoadSettings()
{
	g_plugin.iInterval = g_plugin.getDword("Interval", 500);
	g_plugin.iInterval1 = g_plugin.getDword("PopupInterval");
	g_plugin.bResolveIp = g_plugin.getBool("ResolveIp", true);
	g_plugin.iDefaultAction = g_plugin.getByte("FilterDefaultAction", TRUE);

	g_plugin.bSetColours = g_plugin.getBool("PopupSetColours");
	g_plugin.BgColor = g_plugin.getDword("PopupBgColor", (uint32_t)0xFFFFFF);
	g_plugin.FgColor = g_plugin.getDword("PopupFgColor", (uint32_t)0x000000);
	g_plugin.iFiltersCount = g_plugin.getDword("FiltersCount");
	g_plugin.iStatusMask = g_plugin.getWord("StatusMask", 16);
	for (int i = 0; i < MAX_STATUS_COUNT; i++) {
		char buff[128];
		mir_snprintf(buff, "Status%d", i);
		g_plugin.iStatus[i] = (g_plugin.getByte(buff, 0) == 1);
	}
}

// gives protocol avainable statuses
INT_PTR GetCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_1)
		return 0;
	if (wParam == PFLAGNUM_2)
		return PF2_ONLINE; // add the possible statuses here.
	if (wParam == PFLAGNUM_3)
		return 0;
	return 0;
}

// gives  name to protocol module
INT_PTR GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, PLUGINNAME, wParam);
	return 0;
}

// gives icon for proto module
INT_PTR TMLoadIcon(WPARAM wParam, LPARAM)
{
	UINT id;

	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_ICON1;
		break; // IDI_TM is the main icon for the protocol
	default:
		return 0;
	}
	return (INT_PTR)LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam&PLIF_SMALL ? SM_CXSMICON : SM_CXICON), GetSystemMetrics(wParam&PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}
//=======================================================
//SetStatus
//=======================================================
INT_PTR SetStatus(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_STATUS_OFFLINE) {
		diffstat = 0;
		//PostThreadMessage(ConnectionCheckThreadId,WM_QUIT ,0, 0);
		SetEvent(killCheckThreadEvent);

	}
	else if (wParam == ID_STATUS_ONLINE) {
		diffstat = 0;
		ResetEvent(killCheckThreadEvent);
		if (!hConnectionCheckThread)
			hConnectionCheckThread = (HANDLE)mir_forkthreadex(checkthread, nullptr, (unsigned int*)&ConnectionCheckThreadId);
	}
	else {
		int retv = 0;

		if (g_plugin.iStatus[wParam - ID_STATUS_ONLINE])
			retv = SetStatus(ID_STATUS_OFFLINE, lParam);
		else
			retv = SetStatus(ID_STATUS_ONLINE, lParam);
		//LNEnableMenuItem(hMenuHandle ,TRUE);
		diffstat = wParam;
		return retv;

		// the status has been changed to unknown  (maybe run some more code)
	}
	//broadcast the message

	//oldStatus = currentStatus;
	if (currentStatus != (int)wParam)
		ProtoBroadcastAck(PLUGINNAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)currentStatus, wParam);
	currentStatus = wParam;
	return 0;

}
//=======================================================
//GetStatus
//=======================================================
INT_PTR GetStatus(WPARAM, LPARAM)
{
	return currentStatus;
}

//thread function with connections check loop
static unsigned __stdcall checkthread(void *)
{

	#ifdef _DEBUG
	_OutputDebugString(L"check thread started");
	#endif
	while (1) {
		CONNECTION* conn = nullptr, *connOld = first, *cur = nullptr;
		#ifdef _DEBUG
		_OutputDebugString(L"checking connections table...");
		#endif
		if (WAIT_OBJECT_0 == WaitForSingleObject(killCheckThreadEvent, 100)) {
			hConnectionCheckThread = nullptr;
			return 0;
		}

		conn = GetConnectionsTable();
		cur = conn;
		while (cur != nullptr) {
			if (searchConnection(first, cur->strIntIp, cur->strExtIp, cur->intIntPort, cur->intExtPort, cur->state) == nullptr && (g_plugin.iStatusMask & (1 << (cur->state - 1)))) {

				#ifdef _DEBUG
				wchar_t msg[1024];
				mir_snwprintf(msg, L"%s:%d\n%s:%d", cur->strIntIp, cur->intIntPort, cur->strExtIp, cur->intExtPort);
				_OutputDebugString(L"New connection: %s", msg);
				#endif
				pid2name(cur->Pid, cur->PName, _countof(cur->PName));
				if (WAIT_OBJECT_0 == WaitForSingleObject(hExceptionsMutex, 100)) {
					if (checkFilter(connExceptions, cur)) {
						showMsg(cur->PName, cur->Pid, cur->strIntIp, cur->strExtIp, cur->intIntPort, cur->intExtPort, cur->state);
						Skin_PlaySound(PLUGINNAME_NEWSOUND);
					}
					ReleaseMutex(hExceptionsMutex);
				}
			}
			cur = cur->next;
		}

		first = conn;
		deleteConnectionsTable(connOld);
		Sleep(g_plugin.iInterval);
	}
	hConnectionCheckThread = nullptr;
	return 1;
}

//popup reactions
static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED)//client clicked on popup with left mouse button
		{
			CONNECTION *conn = (CONNECTION*)mir_alloc(sizeof(CONNECTION));
			CONNECTION *mpd = (CONNECTION*) PUGetPluginData(hWnd);

			memcpy(conn, mpd, sizeof(CONNECTION));
			PUDeletePopup(hWnd);
			PostThreadMessage(FilterOptionsThreadId, WM_ADD_FILTER, 0, (LPARAM)conn);
		}
		break;

	case WM_RBUTTONUP:
		PUDeletePopup(hWnd);
		break;

	case UM_INITPOPUP:
		//struct CONNECTON *conn=NULL;
		//conn = (CONNECTION*)PUGetPluginData(hWnd,(LPARAM)conn);
		//MessageBox(NULL,conn->extIp);
		//PUDeletePopUp(hWnd);
		break;

	case UM_FREEPLUGINDATA:
		CONNECTION *mpd = (CONNECTION*)PUGetPluginData(hWnd);
		if (mpd > 0) mir_free(mpd);
		return TRUE; //TRUE or FALSE is the same, it gets ignored.
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//show popup
void showMsg(wchar_t *pName, uint32_t pid, wchar_t *intIp, wchar_t *extIp, int intPort, int extPort, int state)
{
	CONNECTION *mpd = (CONNECTION*)mir_alloc(sizeof(CONNECTION));

	POPUPDATAW ppd;
	ppd.lchContact = NULL;//(HANDLE)hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON1));
	if (g_plugin.bResolveIp) {
		wchar_t hostName[128];
		getDnsName(extIp, hostName, _countof(hostName));
		mir_snwprintf(ppd.lpwzText, L"%s:%d\n%s:%d", hostName, extPort, intIp, intPort);
	}
	else mir_snwprintf(ppd.lpwzText, L"%s:%d\n%s:%d", extIp, extPort, intIp, intPort);

	mir_snwprintf(ppd.lpwzContactName, L"%s (%s)", pName, tcpStates[state - 1]);

	if (g_plugin.bSetColours) {
		ppd.colorBack = g_plugin.BgColor;
		ppd.colorText = g_plugin.FgColor;
	}
	ppd.PluginWindowProc = PopupDlgProc;

	ppd.iSeconds = g_plugin.iInterval1;
	//Now the "additional" data.
	wcsncpy_s(mpd->strIntIp, intIp, _TRUNCATE);
	wcsncpy_s(mpd->strExtIp, extIp, _TRUNCATE);
	wcsncpy_s(mpd->PName, pName, _TRUNCATE);
	mpd->intIntPort = intPort;
	mpd->intExtPort = extPort;
	mpd->Pid = pid;

	//Now that the plugin data has been filled, we add it to the PopUpData.
	ppd.PluginData = mpd;

	//Now that every field has been filled, we want to see the popup.
	PUAddPopupW(&ppd);
}

//called after all plugins loaded.
//all Connection staff will be called, that will not hang miranda on startup
static int modulesloaded(WPARAM, LPARAM)
{

	#ifdef _DEBUG
	_OutputDebugString(L"Modules loaded, lets start TN...");
	#endif
	//	hConnectionCheckThread = (HANDLE)mir_forkthreadex(checkthread, 0, 0, ConnectionCheckThreadId);

	//#ifdef _DEBUG
	//	_OutputDebugString("started check thread %d",hConnectionCheckThread);
	//#endif
	killCheckThreadEvent = CreateEvent(nullptr, FALSE, FALSE, L"killCheckThreadEvent");
	hFilterOptionsThread = startFilterThread();
	//updaterRegister();

	return 0;
}
//function hooks before unload
static int preshutdown(WPARAM, LPARAM)
{
	deleteConnectionsTable(first);
	deleteConnectionsTable(connExceptions);

	PostThreadMessage(ConnectionCheckThreadId, WM_QUIT, 0, 0);
	PostThreadMessage(FilterOptionsThreadId, WM_QUIT, 0, 0);

	return 0;
}

int CMPlugin::Load()
{
	#ifdef _DEBUG
	_OutputDebugString(L"Entering Load dll");
	#endif

	hExceptionsMutex = CreateMutex(nullptr, FALSE, L"ExceptionsMutex");

	LoadSettings();
	connExceptions = LoadSettingsConnections();

	// set all contacts to offline
	for (auto &hContact : Contacts(PLUGINNAME))
		g_plugin.setWord(hContact, "status", ID_STATUS_OFFLINE);

	CreateProtoServiceFunction(PLUGINNAME, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(PLUGINNAME, PS_GETNAME, GetName);
	CreateProtoServiceFunction(PLUGINNAME, PS_LOADICON, TMLoadIcon);
	CreateProtoServiceFunction(PLUGINNAME, PS_SETSTATUS, SetStatus);
	CreateProtoServiceFunction(PLUGINNAME, PS_GETSTATUS, GetStatus);

	g_plugin.addSound(PLUGINNAME_NEWSOUND, _A2W(PLUGINNAME), LPGENW("New Connection Notification"));
	
	HookEvent(ME_OPT_INITIALISE, ConnectionNotifyOptInit); // register service to hook option call
	HookEvent(ME_SYSTEM_MODULESLOADED, modulesloaded); // hook event that all plugins are loaded
	HookEvent(ME_SYSTEM_PRESHUTDOWN, preshutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WaitForSingleObjectEx(hConnectionCheckThread, INFINITE, FALSE);

	if (hConnectionCheckThread) CloseHandle(hConnectionCheckThread);
	if (hCheckEvent) DestroyHookableEvent(hCheckEvent);
	if (hCheckHook) UnhookEvent(hCheckHook);
	if (killCheckThreadEvent) CloseHandle(killCheckThreadEvent);
	if (hExceptionsMutex) CloseHandle(hExceptionsMutex);

	#ifdef _DEBUG
	_OutputDebugString(L"Unloaded");
	#endif
	return 0;
}
