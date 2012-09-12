/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "private.h"
#include "resource.h"
#include "icons.h"
#include "options.h"

#include <time.h>

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;

struct MM_INTERFACE   mmi  = {sizeof(struct MM_INTERFACE  ),0};


// add icon to srmm status icons
static void SrmmMenu_UpdateIcon(HANDLE hContact);
static int SrmmMenu_ProcessEvent(WPARAM wParam, LPARAM lParam);
static int SrmmMenu_ProcessIconClick(WPARAM wParam, LPARAM lParam);


HANDLE hEventDbEventAdded, hEventMenuPrebuild, hMenuToggle, hMenuClear, hServiceToggle, hServiceClear, hEventWindow, hEventIconPressed;

CRITICAL_SECTION list_cs;

#define MS_NOHISTORY_TOGGLE 		MODULE "/ToggleOnOff"
#define MS_NOHISTORY_CLEAR	 		MODULE "/Clear"

#define DBSETTING_REMOVE			"RemoveHistory"

// a list of db events - we'll check them for the 'read' flag periodically and delete them whwen marked as read
struct EventListNode {
	HANDLE hContact, hDBEvent;
	EventListNode *next;
};

EventListNode *event_list = 0;

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	MODULE,
	PLUGIN_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD),
	DESC_STRING,
	"Scott Ellis, NightFox",
	"mail@scottellis.com.au; nightfox@myied.org",
	"© 2005 Scott Ellis; NightFox 2010",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
#ifdef _UNICODE
{ 0xb25e8c7b, 0x292b, 0x495a, { 0x9f, 0xb8, 0xa4, 0xc3, 0xd4, 0xee, 0xb0, 0x4b } } // {B25E8C7B-292B-495a-9FB8-A4C3D4EEB04B}
#else
{ 0x12e5a39, 0x78a6, 0x4ccb, { 0xa6, 0x3f, 0x60, 0xfa, 0x7c, 0xb4, 0xd, 0xee } } // {012E5A39-78A6-4ccb-A63F-60FA7CB40DEE}
#endif
};

extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}

extern "C" __declspec (dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_NOHISTORY, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}



void RemoveReadEvents(HANDLE hContact = 0) {
	DBEVENTINFO info = {0};
	info.cbSize = sizeof(info);
	bool remove;

	EnterCriticalSection(&list_cs);
	EventListNode *node = event_list, *prev = 0;
	while(node) {
		remove = false;
		if(hContact == 0 || hContact == node->hContact) {
			info.cbBlob = 0;
			if(!CallService(MS_DB_EVENT_GET, (WPARAM)node->hDBEvent, (LPARAM)&info)) {
				if((info.flags & DBEF_READ) || (info.flags & DBEF_SENT)) // note: already checked event type when added to list
					remove = true;
			} else { 
				// could not get event info - maybe someone else deleted it - so remove list node
				remove = true;
			}
		}
		
		if(remove) {
			if(DBGetContactSettingByte(node->hContact, MODULE, DBSETTING_REMOVE, 0)) // is history disabled for this contact?
				CallService(MS_DB_EVENT_DELETE, (WPARAM)node->hContact, (LPARAM)node->hDBEvent);
			
			// remove list node anyway
			if(event_list == node) event_list = node->next;
			if(prev) prev->next = node->next;

			free(node);
			
			if(prev) node = prev->next;
			else node = event_list;
		} else {
			prev = node;
			node = node->next;
		}
	}
	
	LeaveCriticalSection(&list_cs);	
}

void RemoveAllEvents(HANDLE hContact) {
	HANDLE hDBEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0), hDBEventNext;
	while(hDBEvent) {
		hDBEventNext = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDBEvent, 0);
		CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hDBEvent);
		hDBEvent = hDBEventNext;
	}

}

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime) {
	RemoveReadEvents();
}


int OnDatabaseEventAdd(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam, hDBEvent = (HANDLE)lParam;
	
	// history not disabled for this contact
	if(DBGetContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 0) == 0)
		return 0;
	
	DBEVENTINFO info = {0};
	info.cbSize = sizeof(info);
	if(!CallService(MS_DB_EVENT_GET, (WPARAM)hDBEvent, (LPARAM)&info)) {
		if(info.eventType == EVENTTYPE_MESSAGE) {
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


INT_PTR ServiceClear(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	if(MessageBox(0, TranslateT("This operation will PERMANENTLY REMOVE all history for this contact.\nAre you sure you want to do this?"), TranslateT("Clear History"), MB_YESNO | MB_ICONWARNING) == IDYES) {
		RemoveAllEvents(hContact);
	}
	
	return 0;
}

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	bool remove = (DBGetContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 0) != 0);
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) != 0);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);

	mi.flags = CMIM_FLAGS;
	if(chat_room) mi.flags |= CMIF_HIDDEN;
	else {
		mi.flags |= (CMIM_NAME | CMIM_ICON);
		mi.pszName = (remove ? Translate("Enable History") : Translate("Disable History"));
		mi.hIcon = (remove ? hIconKeep : hIconRemove);
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuToggle, (LPARAM)&mi);

	mi.flags = CMIM_FLAGS;
	if(chat_room) mi.flags |= CMIF_HIDDEN;
	else {
		int event_count = (int)CallService(MS_DB_EVENT_GETCOUNT, (WPARAM)hContact, 0);
		if(event_count <= 0) mi.flags |= CMIF_HIDDEN;
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuClear, (LPARAM)&mi);
	
	return 0;
}

INT_PTR ServiceToggle(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	int remove = (DBGetContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 0) != 0);
	remove = !remove;
	DBWriteContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, (remove ? 1 : 0));

	StatusIconData sid = {0};
	sid.cbSize = sizeof(sid);
	sid.szModule = MODULE;


	for (int i = 0; i < 2; ++i)
	{
		sid.dwId = i;
		sid.flags = (i == remove) ? 0 : MBF_HIDDEN;
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
	}
/*
	sid.dwId = 0;
	sid.flags = (remove ? MBF_DISABLED : 0);
	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
*/
	return 0;
}

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	HANDLE hContact = mwd->hContact;
	int remove = (DBGetContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 0) != 0);

	if(mwd->uType == MSG_WINDOW_EVT_CLOSE) {
		RemoveReadEvents(hContact);
		return 0;
	}

	if(mwd->uType != MSG_WINDOW_EVT_OPEN) return 0;

	if(!ServiceExists(MS_MSG_MODIFYICON)) return 0;
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) != 0);



	StatusIconData sid = {0};
	sid.cbSize = sizeof(sid);
	sid.szModule = MODULE;
/*
	sid.dwId = 0;
	sid.flags = (chat_room ? MBF_HIDDEN : (remove ? MBF_DISABLED : 1));
	CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
*/

	for (int i = 0; i < 2; ++i)
	{
		sid.dwId = i;
		sid.flags = (chat_room ? MBF_HIDDEN : (i == remove) ? 0 : MBF_HIDDEN);
		CallService(MS_MSG_MODIFYICON, (WPARAM)hContact, (LPARAM)&sid);
	}



	return 0;
}


int IconPressed(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if(sicd->cbSize < (int)sizeof(StatusIconClickData))
		return 0;

	if(sicd->flags & MBCF_RIGHTBUTTON) return 0; // ignore right-clicks
	if(strcmp(sicd->szModule, MODULE) != 0) return 0; // not our event

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	bool chat_room = (proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) != 0);

	if(!chat_room) ServiceToggle((WPARAM)hContact, 0);

	return 0;
}


// add icon to srmm status icons
void SrmmMenu_Load()
{
	if(ServiceExists(MS_MSG_ADDICON)) {
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.flags = 0;
		sid.dwId = 0;
//		sid.hIcon = hIconKeep;
//		sid.hIconDisabled = hIconRemove;

		sid.szTooltip = Translate("History Enabled");
		sid.hIcon = sid.hIconDisabled = hIconKeep;

		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		sid.dwId = 1;
		sid.szTooltip = Translate("History Disabled");
		sid.hIcon = sid.hIconDisabled = hIconRemove;
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		
		// hook the window events so that we can can change the status of the icon
		hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
		hEventIconPressed = HookEvent(ME_MSG_ICONPRESSED, IconPressed);
	}	


}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/no_history.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_no_history.html";
		update.pbBetaVersionPrefix = (BYTE *)"NoHistory plugin, version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	hServiceToggle = (HANDLE)CreateServiceFunction(MS_NOHISTORY_TOGGLE, ServiceToggle);
	hServiceClear = (HANDLE)CreateServiceFunction(MS_NOHISTORY_CLEAR, ServiceClear);
	
	InitIcons();
	
	// create contact menu item
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_ALL;

	mi.position = -300010;
	mi.pszName = Translate("Disable History");
	mi.pszService = MS_NOHISTORY_TOGGLE;
	mi.hIcon = hIconRemove;
	hMenuToggle = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.position = -300005;
	mi.pszName = Translate("Clear History");
	mi.pszService = MS_NOHISTORY_CLEAR;
	mi.hIcon = hIconClear;
	hMenuClear = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	hEventMenuPrebuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	// hooked so we can track events added to the database
	hEventDbEventAdded = HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdd);
	
	// add icon to srmm status icons
	SrmmMenu_Load();

	return 0;
}


HANDLE hModulesLoaded;
extern "C" __declspec (dllexport) int Load(PLUGINLINK *link) {
	pluginLink=link;

	CallService( MS_SYSTEM_GET_MMI,	 0, (LPARAM)&mmi  );
		
	InitializeCriticalSection(&list_cs);

	// fix for wrong module name - ugh
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		if(!DBGetContactSetting(hContact, "NoHostory", DBSETTING_REMOVE, &dbv)) {
			if(dbv.type == DBVT_BYTE && dbv.bVal != 0)
				DBWriteContactSettingByte(hContact, MODULE, DBSETTING_REMOVE, 1);
			DBFreeVariant(&dbv);
			DBDeleteContactSetting(hContact, "NoHostory", DBSETTING_REMOVE);
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
	
	INITCOMMONCONTROLSEX icex;

	// Ensure that the common control DLL is loaded (for listview)
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	
	
	InitOptions();
	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void) {
	UnhookEvent(hEventWindow);
	UnhookEvent(hEventIconPressed);

	UnhookEvent(hModulesLoaded);
	DeinitOptions();
	
	UnhookEvent(hEventDbEventAdded);
	UnhookEvent(hEventMenuPrebuild);
	DestroyServiceFunction(hServiceToggle);
	DestroyServiceFunction(hServiceClear);

	RemoveReadEvents();
	DeleteCriticalSection(&list_cs);
	
	DeinitIcons();
	return 0;
}
