#include "common.h"
#include "message_pump.h"
#include "popwin.h"
#include "services.h"
#include "options.h"

unsigned message_pump_thread_id = 0;
int num_popups = 0;

HANDLE hMPEvent;

#define MUM_FINDWINDOW		(WM_USER + 0x050)

#define MAX_POPUPS	100

// from popups, popup2 implementation, slightly modified
// return true if there is a full-screen application (e.g. game) running
bool is_full_screen() {
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);
	// use ClientRect instead of WindowRect so that it works normally for maximized applications - thx Nikto
	RECT ClientRect;
	HWND hWnd;

	HWND hWndDesktop = GetDesktopWindow();
	HWND hWndShell = GetShellWindow();

	// check foregroundwindow
	hWnd = GetForegroundWindow();
	if(hWnd && hWnd != hWndDesktop && hWnd != hWndShell) {
		GetClientRect(hWnd, &ClientRect);
		if ((ClientRect.right - ClientRect.left) >= w && (ClientRect.bottom - ClientRect.top) >= h)
			return true;
	}
	
	// check other top level windows
	while ((hWnd = FindWindowEx(NULL, hWnd, NULL, NULL))) {
		if(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) || hWnd == hWndDesktop || hWnd == hWndShell)
			continue;
			
//		if(DBGetContactSettingByte(0, MODULE, "ShowForNonTopmostFullscreenWindows", 0) == 1) {
			if (!(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
				continue;
//		}

		// not sure if this could be done more simply using 'IsZoomed'?
		GetClientRect(hWnd, &ClientRect);
		if ((ClientRect.right - ClientRect.left) < w || (ClientRect.bottom - ClientRect.top) < h)
			continue;

		return true;
	}

	return false;
}

bool is_workstation_locked()
{
	bool rc = false;
	HDESK hDesk = OpenDesktop((TCHAR*)_T("default"), 0, FALSE, DESKTOP_SWITCHDESKTOP);
	if(hDesk != 0) {
		HDESK hDeskInput = OpenInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);
		if(hDeskInput == 0) {
			rc = true;
		} else
			CloseDesktop(hDeskInput);

		CloseDesktop(hDesk);
	} 

	return rc;
}


unsigned __stdcall MessagePumpThread(void* param) {
	InitWindowStack();

	if(param) SetEvent((HANDLE)param);

	MSG hwndMsg = {0};
	while(GetMessage(&hwndMsg, 0, 0, 0) > 0 && !Miranda_Terminated()) {
		if(!IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) {
			switch(hwndMsg.message) {
				case MUM_CREATEPOPUP:
					{
						bool enabled = true;
						int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
						if(status >= ID_STATUS_OFFLINE && status <= ID_STATUS_OUTTOLUNCH && options.disable_status[status - ID_STATUS_OFFLINE])
							enabled = false;
						if((options.disable_full_screen && is_full_screen()) || is_workstation_locked())
							enabled = false;

						PopupData *pd = (PopupData *)hwndMsg.lParam;
						if(enabled && num_popups < MAX_POPUPS) {
							//HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, _T("Popup"), WS_POPUP, 0, 0, 0, 0, GetDesktopWindow(), 0, hInst, (LPVOID)hwndMsg.lParam);
							HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, _T("Popup"), WS_POPUP, 0, 0, 0, 0, 0, 0, hInst, (LPVOID)hwndMsg.lParam);
							num_popups++;
							if(hwndMsg.wParam) // set notifyer handle
								SendMessage(hwnd, PUM_SETNOTIFYH, hwndMsg.wParam, 0);
						} else {
							if (pd) {
								mir_free(pd->pwzTitle);
								mir_free(pd->pwzText);
								mir_free(pd);
							}
						}
					}
					break;

				case MUM_DELETEPOPUP:
					{
						HWND hwnd = (HWND)hwndMsg.lParam;
						if(IsWindow(hwnd)) {
							DestroyWindow(hwnd);
							num_popups--;
						}
					}
					break;
				case MUM_NMUPDATE:
					{
						HANDLE hNotify = (HANDLE)hwndMsg.wParam;
						BroadcastMessage(PUM_UPDATENOTIFY, (WPARAM)hNotify, 0);
					}
					break;
				case MUM_NMREMOVE:
					{
						HANDLE hNotify = (HANDLE)hwndMsg.wParam;
						BroadcastMessage(PUM_KILLNOTIFY, (WPARAM)hNotify, 0);
					}
					break;
				case MUM_NMAVATAR:
					RepositionWindows();
					break;
				default:
					{
						TranslateMessage(&hwndMsg);
						DispatchMessage(&hwndMsg);
						// do this here in case the window has gone
						if(hwndMsg.message == PUM_CHANGE || hwndMsg.message == PUM_SETTEXT)
							mir_free((void *)hwndMsg.lParam);
					}
					break;
			}
		}
	}

	DeinitWindowStack();
	num_popups = 0;

	//if(param) SetEvent((HANDLE)param);

    DeinitOptions();
	DeinitServices();

	return 0;
}

void PostMPMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	PostThreadMessage(message_pump_thread_id, msg, wParam, lParam);
}

// given a popup data pointer, and a handle to an event, this function
// will post a message to the message queue which will set the hwnd value
// and then set the event...so create an event, call this function and then wait on the event
// when the event is signalled, the hwnd will be valid
void FindWindow(PopupData *pd, HANDLE hEvent, HWND *hwnd);

void InitMessagePump() {
	WNDCLASS popup_win_class = {0};
	popup_win_class.lpfnWndProc = PopupWindowProc;
	popup_win_class.hInstance = hInst;
	popup_win_class.lpszClassName = POP_WIN_CLASS;
	popup_win_class.hCursor = LoadCursor(NULL, IDC_ARROW); 
	RegisterClass(&popup_win_class);

	InitServices();

	hMPEvent = CreateEvent(0, TRUE, 0, 0);
	CloseHandle(mir_forkthreadex(MessagePumpThread, hMPEvent, 0, &message_pump_thread_id));
	WaitForSingleObject(hMPEvent, INFINITE);
	CloseHandle(hMPEvent);
}

void DeinitMessagePump() {
	PostMPMessage(WM_QUIT, 0, 0);
}
