#include "common.h"

HINSTANCE hInst;
int hLangpack = 0;

// add icon to srmm status icons
static void SrmmMenu_UpdateIcon(MCONTACT hContact);
static int SrmmMenu_ProcessEvent(WPARAM wParam, LPARAM lParam);
static int SrmmMenu_ProcessIconClick(WPARAM wParam, LPARAM lParam);

HGENMENU hMenuToggle, hMenuClear;
HANDLE   hServiceToggle, hServiceClear;

CRITICAL_SECTION list_cs;

#define MS_NOHISTORY_TOGGLE 		MODULE "/ToggleOnOff"
#define MS_NOHISTORY_CLEAR	 		MODULE "/Clear"

#define DBSETTING_REMOVE			"RemoveHistory"

// a list of db events - we'll check them for the 'read' flag periodically and delete them whwen marked as read
struct EventListNode {
	MCONTACT hContact;
	MEVENT hDBEvent;
	EventListNode *next;
};

EventListNode *event_list = 0;

// plugin stuff
PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {B25E8C7B-292B-495A-9FB8-A4C3D4EEB04B}
	{0xb25e8c7b, 0x292b, 0x495a, {0x9f, 0xb8, 0xa4, 0xc3, 0xd4, 0xee, 0xb0, 0x4b}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

void RemoveReadEvents(MCONTACT hContact = 0)
{
	DBEVENTINFO info = { sizeof(info) };
	bool remove;

	EnterCriticalSection(&list_cs);
	EventListNode *node = event_list, *prev = 0;
	while(node) {
		remove = false;
		if (hContact == 0 || hContact == node->hContact) {
			info.cbBlob = 0;
			if (!db_event_get(node->hDBEvent, &info)) {
				if ((info.flags & DBEF_READ) || (info.flags & DBEF_SENT)) // note: already checked event type when added to list
					remove = true;
			}
			else { 
				// could not get event info - maybe someone else deleted it - so remove list node
				remove = true;
			}
		}
		
		if (remove) {
			if (db_get_b(node->hContact, MODULE, DBSETTING_REMOVE, 0)) // is history disabled for this contact?
				db_event_delete(node->hContact, node->hDBEvent);
			
			// remove list node anyway
			if (event_list == node) event_list = node->next;
			if (prev) prev->next = node->next;

			free(node);
			
			if (prev) node = prev->next;
			else node = event_list;
		}
		else {
			prev = node;
			node = node->next;
		}
	}
	
	LeaveCriticalSection(&list_cs);	
}

void RemoveAllEvents(MCONTACT hContact)
{
	MEVENT hDBEvent = db_event_first(hContact);
	while(hDBEvent) {
		MEVENT hDBEventNext = db_event_next(hContact, hDBEvent);
		db_event_delete(hContact, hDBEvent);
		hDBEvent = hDBEventNext;
	}
}

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	RemoveReadEvents();
}

int OnDatabaseEventAdd(WPARAM hContact, LPARAM hDBEvent)
{
	// history not disabled for this contact
	if (db_get_b(hContact, MODULE, DBSETTING_REMOVE, 0) == 0)
		return 0;
	
	DBEVENTINFO info = { sizeof(info) };
	if (!db_event_get(hDBEvent, &info)) {
		if (info.eventType == EVENTTYPE_MESSAGE) {
			EventListNode *node = (EventListNode *)malloc(sizeof(EventListNode));
			node->hContact = hContact;
			node->hDBEvent = hDBEvent;	

			EnterCriticalSection(&list_cs);
			node->next = event_list;
			event_list = node;		
			LeaveCriticalSection(&list_cs);	
		}
	}

	return 0;
}

INT_PTR ServiceClear(WPARAM hContact, LPARAM lParam)
{
	if (MessageBox(0, TranslateT("This operation will PERMANENTLY REMOVE all history for this contact.\nAre you sure you want to do this?"), TranslateT("Clear History"), MB_YESNO | MB_ICONWARNING) == IDYES)
		RemoveAllEvents(hContact);
	
	return 0;
}

int PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	bool remove = db_get_b(hContact, MODULE, DBSETTING_REMOVE, 0) != 0;
	char *proto = GetContactProto(hContact);
	bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0) != 0);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIF_TCHAR;
	if (chat_room) mi.flags |= CMIF_HIDDEN;
	else {
		mi.flags |= (CMIM_NAME | CMIM_ICON);
		mi.ptszName = (remove ? LPGENT("Enable History") : LPGENT("Disable History"));
		mi.hIcon = (remove ? hIconKeep : hIconRemove);
	}
	Menu_ModifyItem(hMenuToggle, &mi);

	Menu_ShowItem(hMenuClear, !chat_room && db_event_count(hContact) > 0);
	return 0;
}

INT_PTR ServiceToggle(WPARAM hContact, LPARAM lParam)
{
	int remove = db_get_b(hContact, MODULE, DBSETTING_REMOVE, 0) != 0;
	remove = !remove;
	db_set_b(hContact, MODULE, DBSETTING_REMOVE, remove != 0);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULE;

	for (int i = 0; i < 2; ++i) {
		sid.dwId = i;
		sid.flags = (i == remove) ? 0 : MBF_HIDDEN;
		Srmm_ModifyIcon(hContact, &sid);
	}
	return 0;
}

int WindowEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	MCONTACT hContact = mwd->hContact;

	switch(mwd->uType) {
	case MSG_WINDOW_EVT_CLOSE:
		RemoveReadEvents(hContact);
		break;

	case MSG_WINDOW_EVT_OPEN:
		char *proto = GetContactProto(hContact);
		bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0) != 0);
		int remove = db_get_b(hContact, MODULE, DBSETTING_REMOVE, 0) != 0;

		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULE;
		for (int i=0; i < 2; ++i) {
			sid.dwId = i;
			sid.flags = (chat_room ? MBF_HIDDEN : (i == remove) ? 0 : MBF_HIDDEN);
			Srmm_ModifyIcon(hContact, &sid);
		}
	}

	return 0;
}
										  
int IconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (sicd->cbSize < sizeof(StatusIconClickData))
		return 0;

	if (sicd->flags & MBCF_RIGHTBUTTON) return 0; // ignore right-clicks
	if (strcmp(sicd->szModule, MODULE) != 0) return 0; // not our event

	char *proto = GetContactProto(hContact);
	bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0) != 0);
	if (!chat_room)
		ServiceToggle(hContact, 0);

	return 0;
}


// add icon to srmm status icons
void SrmmMenu_Load()
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULE;

	sid.dwId = 0;
	sid.szTooltip = LPGEN("History Enabled");
	sid.hIcon = sid.hIconDisabled = hIconKeep;
	Srmm_AddIcon(&sid);

	sid.dwId = 1;
	sid.szTooltip = LPGEN("History Disabled");
	sid.hIcon = sid.hIconDisabled = hIconRemove;
	Srmm_AddIcon(&sid);
		
	// hook the window events so that we can can change the status of the icon
	HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, IconPressed);
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// create contact menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;

	mi.position = -300010;
	mi.ptszName = LPGENT("Disable History");
	mi.pszService = MS_NOHISTORY_TOGGLE;
	mi.hIcon = hIconRemove;
	hMenuToggle = Menu_AddContactMenuItem(&mi);

	mi.position = -300005;
	mi.ptszName = LPGENT("Clear History");
	mi.pszService = MS_NOHISTORY_CLEAR;
	mi.hIcon = hIconClear;
	hMenuClear = Menu_AddContactMenuItem(&mi);

	// add icon to srmm status icons
	SrmmMenu_Load();
	return 0;
}

extern "C" __declspec (dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	InitializeCriticalSection(&list_cs);

	// Ensure that the common control DLL is loaded (for listview)
	INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_LISTVIEW_CLASSES };
	InitCommonControlsEx(&icex); 	
	
	InitIcons();
	InitOptions();

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdd);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	hServiceToggle = CreateServiceFunction(MS_NOHISTORY_TOGGLE, ServiceToggle);
	hServiceClear = CreateServiceFunction(MS_NOHISTORY_CLEAR, ServiceClear);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void)
{
	DestroyServiceFunction(hServiceToggle);
	DestroyServiceFunction(hServiceClear);

	RemoveReadEvents();
	DeleteCriticalSection(&list_cs);
	return 0;
}
