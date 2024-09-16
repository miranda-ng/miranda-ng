#ifndef __history_h__
#define __history_h__

#define EVENTTYPE_OTHER 12345
#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_JABBER_PRESENCE 2001

enum
{
	UM_LOAD_CONTACT = WM_USER + 1,

	UM_REBUILD_LIST,
	UM_FILTER_LIST,
	UM_REDRAW_LIST,

	UM_ADD_EVENT,
	UM_ADD_EVENT_FILTER,
	UM_REMOVE_EVENT,
	UM_EDIT_EVENT,

	UM_SELECTED,

	UM_GET_EVENT,
	UM_GET_EVENT_TEXT,
	UM_GET_EVENT_COUNT,
	UM_GET_EVENT_HANDLE,
	UM_GET_EVENT_CONTACT,

	UM_BOOKMARKS = WM_USER + 0x661,
	UM_LOCATETIME = WM_USER + 0x662,
};

extern MWindowList g_hNewstoryWindows, g_hNewstoryLogs;

void InitMenus();
void InitHotkeys();

INT_PTR svcShowNewstory(WPARAM, LPARAM);
INT_PTR svcGlobalSearch(WPARAM, LPARAM);

CSrmmLogWindow* __cdecl NewStory_Stub(CMsgDialog &pDlg);

#endif // __history_h__