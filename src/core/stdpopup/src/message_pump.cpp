#include "stdafx.h"

static int num_popups = 0;

static HANDLE hMPThread;
unsigned message_pump_thread_id = 0;

#define MUM_FINDWINDOW		(WM_USER + 0x050)

#define MAX_POPUPS	100

unsigned __stdcall MessagePumpThread(void *)
{
	MSG hwndMsg = {};
	while (GetMessage(&hwndMsg, nullptr, 0, 0) > 0 && !bShutdown) {
		if (hwndMsg.hwnd != nullptr && IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) /* Wine fix. */
			continue;
		switch (hwndMsg.message) {
		case MUM_CREATEPOPUP:
			{
				bool enabled = true;
				int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
				if (status >= ID_STATUS_OFFLINE && status <= ID_STATUS_MAX && options.disable_status[status - ID_STATUS_OFFLINE])
					enabled = false;
				if ((options.disable_full_screen && IsFullScreen()) || IsWorkstationLocked())
					enabled = false;

				PopupData *pd = (PopupData *)hwndMsg.lParam;
				if (enabled && num_popups < MAX_POPUPS) {
					CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, L"Popup", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), (LPVOID)hwndMsg.lParam);
					num_popups++;
				}
				else if (pd) {
					mir_free(pd->pwszTitle);
					mir_free(pd->pwszText);
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
	popup_win_class.hInstance = g_plugin.getInst();
	popup_win_class.lpszClassName = POP_WIN_CLASS;
	popup_win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&popup_win_class);

	InitServices();

	hMPThread = mir_forkthreadex(MessagePumpThread, 0, &message_pump_thread_id);
}

void DeinitMessagePump()
{
	PostMPMessage(WM_QUIT, 0, 0);

	WaitForSingleObject(hMPThread, INFINITE);
	CloseHandle(hMPThread);
}
