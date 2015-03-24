#include "common.h"

unsigned message_pump_thread_id = 0;
int num_popups = 0;

HANDLE hMPEvent;

#define MUM_FINDWINDOW		(WM_USER + 0x050)

#define MAX_POPUPS	100

unsigned __stdcall MessagePumpThread(void* param)
{
	if (param)
		SetEvent((HANDLE)param);

	MSG hwndMsg = { 0 };
	while (GetMessage(&hwndMsg, 0, 0, 0) > 0 && !bShutdown) {
		if (hwndMsg.hwnd != NULL && IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) /* Wine fix. */
			continue;
		switch(hwndMsg.message) {
		case MUM_CREATEPOPUP:
			{
				bool enabled = true;
				int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
				if (status >= ID_STATUS_OFFLINE && status <= ID_STATUS_OUTTOLUNCH && options.disable_status[status - ID_STATUS_OFFLINE])
					enabled = false;
				if ((options.disable_full_screen && IsFullScreen()) || IsWorkstationLocked())
					enabled = false;

				PopupData *pd = (PopupData*)hwndMsg.lParam;
				if (enabled && num_popups < MAX_POPUPS) {
					//HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, _T("Popup"), WS_POPUP, 0, 0, 0, 0, GetDesktopWindow(), 0, hInst, (LPVOID)hwndMsg.lParam);
					HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, _T("Popup"), WS_POPUP, 0, 0, 0, 0, 0, 0, hInst, (LPVOID)hwndMsg.lParam);
					num_popups++;
					if (hwndMsg.wParam) // set notifyer handle
						SendMessage(hwnd, PUM_SETNOTIFYH, hwndMsg.wParam, 0);
				}
				else if (pd) {
					mir_free(pd->pwzTitle);
					mir_free(pd->pwzText);
					mir_free(pd);
				}
			}
			break;

		case MUM_DELETEPOPUP:
			{
				HWND hwnd = (HWND)hwndMsg.lParam;
				if (IsWindow(hwnd)) {
					DestroyWindow(hwnd);
					num_popups--;
				}
			}
			break;

		case MUM_NMUPDATE:
			BroadcastMessage(PUM_UPDATENOTIFY, hwndMsg.wParam, 0);
			break;

		case MUM_NMREMOVE:
			BroadcastMessage(PUM_KILLNOTIFY, hwndMsg.wParam, 0);
			break;

		case MUM_NMAVATAR:
			RepositionWindows();
			break;

		default:
			TranslateMessage(&hwndMsg);
			DispatchMessage(&hwndMsg);
			break;
		}
	}

	DeinitWindowStack();
	num_popups = 0;
	return 0;
}

void PostMPMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	PostThreadMessage(message_pump_thread_id, msg, wParam, lParam);
}

// given a popup data pointer, and a handle to an event, this function
// will post a message to the message queue which will set the hwnd value
// and then set the event...so create an event, call this function and then wait on the event
// when the event is signalled, the hwnd will be valid

void InitMessagePump()
{
	WNDCLASS popup_win_class = { 0 };
	popup_win_class.lpfnWndProc = PopupWindowProc;
	popup_win_class.hInstance = hInst;
	popup_win_class.lpszClassName = POP_WIN_CLASS;
	popup_win_class.hCursor = LoadCursor(NULL, IDC_ARROW); 
	RegisterClass(&popup_win_class);

	InitServices();

	hMPEvent = CreateEvent(0, TRUE, 0, 0);
	CloseHandle(mir_forkthreadex(MessagePumpThread, hMPEvent, &message_pump_thread_id));
	WaitForSingleObject(hMPEvent, INFINITE);
	CloseHandle(hMPEvent);
}

void DeinitMessagePump()
{
	PostMPMessage(WM_QUIT, 0, 0);
}
