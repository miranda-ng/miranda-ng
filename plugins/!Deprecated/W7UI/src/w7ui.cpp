#include "headers.h"

ITaskbarList3 *g_pTaskbarList = NULL;
UINT g_wm_TaskbarButtonCreated = 0;
HANDLE hSrmmWindows = NULL;

int g_eventSlotTyping = 0;
int g_eventSlotMessage = 0;

void InitJumpList();

static LRESULT CALLBACK W7UIHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK W7SrmmProxyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int OnModulesLoaded(WPARAM, LPARAM);
static int OnProcessSrmmEvent(WPARAM, LPARAM lParam);
static int OnStatusModeChanged(WPARAM wParam, LPARAM);

int LoadW7UI()
{
	CoInitialize(NULL);

	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&g_pTaskbarList);
	g_pTaskbarList->HrInit();

	g_wm_TaskbarButtonCreated = RegisterWindowMessage(_T("TaskbarButtonCreated"));

	CJumpListBuilder::Load();
	LoadJumpList();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_CLIST_STATUSMODECHANGE, OnStatusModeChanged);

	hSrmmWindows = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	new CClistProxyWindow();

	return 0;
}

int UnloadW7UI()
{
	CJumpListBuilder::Unload();
	return 0;
}

static int OnContactIsTyping(WPARAM wParam, LPARAM lParam)
{	
	if (!wParam) return 0;
	
	if (lParam) g_clistProxyWnd->AddOverlayEvent(g_eventSlotTyping);
	else g_clistProxyWnd->RemoveOverlayEvent(g_eventSlotTyping);

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{	
	InitJumpList();
	g_eventSlotMessage = g_clistProxyWnd->AllocateOverlayEvent(LoadSkinnedIconHandle(SKINICON_EVENT_MESSAGE));
	g_eventSlotTyping = g_clistProxyWnd->AllocateOverlayEvent(LoadSkinnedIconHandle(SKINICON_OTHER_TYPING));
	OnStatusModeChanged(CallService(MS_CLIST_GETSTATUSMODE, 0, 0), 0);
	HookEvent(ME_MSG_WINDOWEVENT, OnProcessSrmmEvent);
	HookEvent(ME_PROTO_CONTACTISTYPING, OnContactIsTyping);
	return 0;
}

static LRESULT SrmmSubclassProc(MSG *msg, TSubclassData *data)
{
	HWND hwndProxy = WindowList_Find(hSrmmWindows, (HANDLE)data->lParam);
	CSrmmProxyWindow *wnd = CDwmWindow::GetWindow<CSrmmProxyWindow>(hwndProxy);

	if (wnd)
	{
		switch (msg->message)
		{
		case WM_SETFOCUS:
			wnd->OnTabActive();
			break;
		case WM_ACTIVATE:
			switch (msg->wParam)
			{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				wnd->OnTabActive();
				break;
			case WA_INACTIVE:
				wnd->OnTabInactive();
			}
			break;
		case WM_NCACTIVATE:
			if (msg->wParam)
				wnd->OnTabActive();
			else
				wnd->OnTabInactive();
			break;
		}
	}

	return CallWindowProc(data->oldWndProc, msg->hwnd, msg->message, msg->wParam, msg->lParam);
}

static HWND FindParent(HWND hwnd)
{
	while (1)
	{
		HWND hwndParent = GetParent(hwnd);
		if (hwndParent == NULL)
			return hwnd;
		hwnd = hwndParent;
	}
}

int OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *evt = (MessageWindowEventData *)lParam;
	
	switch (evt->uType)
	{
		case MSG_WINDOW_EVT_OPENING:
		{
			CSubclassMgr::Subclass(evt->hwndWindow, SrmmSubclassProc, (LPARAM)evt->hContact);
			break;
		}

		case MSG_WINDOW_EVT_OPEN:
		{
			HWND hwndParent = FindParent(evt->hwndWindow);
			if (hwndParent != evt->hwndWindow)
			{
				SetWindowLongPtr(hwndParent, GWL_EXSTYLE, GetWindowLongPtr(hwndParent, GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
				CSrmmProxyWindow *wnd = new CSrmmProxyWindow(evt->hContact, evt->hwndWindow, hwndParent);
				HWND hwndProxy = wnd->hwnd();
				g_pTaskbarList->UnregisterTab(hwndParent);
				g_pTaskbarList->RegisterTab(hwndProxy, hwndParent);
				g_pTaskbarList->SetTabOrder(hwndProxy, 0);
				g_pTaskbarList->SetTabActive(hwndProxy, hwndParent, TBATF_USEMDITHUMBNAIL);
				WindowList_Add(hSrmmWindows, hwndProxy, evt->hContact);
			}
			break;
		}

		case MSG_WINDOW_EVT_CLOSING:
		{
			HWND hwndProxy = WindowList_Find(hSrmmWindows, evt->hContact);
			if (hwndProxy)
			{
				WindowList_Remove(hSrmmWindows, hwndProxy);
				g_pTaskbarList->UnregisterTab(hwndProxy);
				DestroyWindow(hwndProxy);
			}
			break;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// status processing

int OnStatusModeChanged(WPARAM wParam, LPARAM)
{
	static int icons[] =
	{
		SKINICON_STATUS_OFFLINE,
		SKINICON_STATUS_ONLINE,
		SKINICON_STATUS_AWAY,
		SKINICON_STATUS_DND,
		SKINICON_STATUS_NA,
		SKINICON_STATUS_OCCUPIED,
		SKINICON_STATUS_FREE4CHAT,
		SKINICON_STATUS_INVISIBLE,
		SKINICON_STATUS_ONTHEPHONE,
		SKINICON_STATUS_OUTTOLUNCH,
	};

	g_clistProxyWnd->SetOverlayIcon(LoadSkinnedIcon(icons[wParam - ID_STATUS_OFFLINE]));

	return 0;
}
