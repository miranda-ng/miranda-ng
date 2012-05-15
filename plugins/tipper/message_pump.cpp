#include "common.h"
#include "message_pump.h"
#include "popwin.h"
#include "options.h"
#include "str_utils.h"

BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);
HMONITOR (WINAPI *MyMonitorFromPoint)(POINT, DWORD);
BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR, LPMONITORINFO);

unsigned int message_pump_thread_id;

unsigned int CALLBACK MessagePumpThread(void *param) 
{
	HWND hwndTip = 0;

	MSG hwndMsg = {0};
	while (GetMessage(&hwndMsg, 0, 0, 0) > 0 && !Miranda_Terminated()) 
	{
		if (!IsDialogMessage(hwndMsg.hwnd, &hwndMsg)) 
		{
			switch(hwndMsg.message) 
			{
				case MUM_CREATEPOPUP:
					if(hwndTip) DestroyWindow(hwndTip);
					// if child of clist, zorder is right, but it steals the first click on a contact :(

					// copy topmost exstyle from clist, since we'll put clist under tip in WM_CREATE message
					//LONG clist_exstyle = GetWindowLong((HWND)CallService(MS_CLUI_GETHWND, 0, 0), GWL_EXSTYLE);
					//hwndTip = CreateWindowEx((clist_exstyle & WS_EX_TOPMOST), POP_WIN_CLASS, _T("TipperPopup"), WS_POPUP, 0, 0, 0, 0, 0/*(HWND)CallService(MS_CLUI_GETHWND, 0, 0)*/, 0, hInst, (LPVOID)hwndMsg.lParam);

					hwndTip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, POP_WIN_CLASS, _T("TipperPopup"), WS_POPUP, 0, 0, 0, 0, 0/*(HWND)CallService(MS_CLUI_GETHWND, 0, 0)*/, 0, hInst, (LPVOID)hwndMsg.lParam);
					if(hwndMsg.lParam) free((LPVOID)hwndMsg.lParam);
					break;

				case MUM_DELETEPOPUP:
					if(hwndTip) {
						DestroyWindow(hwndTip);
						hwndTip = NULL;
					}
					break;

				case MUM_GOTSTATUS:
					if(hwndTip) SendMessage(hwndTip, PUM_SETSTATUSTEXT, hwndMsg.wParam, hwndMsg.lParam);
					else if(hwndMsg.lParam) free((void *)hwndMsg.lParam);
					break;

				case MUM_REDRAW:
					if(hwndTip) SendMessage(hwndTip, PUM_REFRESH_VALUES, hwndMsg.wParam, hwndMsg.lParam);
					break;

				case MUM_GOTAVATAR:
					if(hwndTip) SendMessage(hwndTip, PUM_SETAVATAR, hwndMsg.wParam, hwndMsg.lParam);
					break;

				default:
					TranslateMessage(&hwndMsg);
					DispatchMessage(&hwndMsg);
					break;
			}
		}
	}

	return 0;
}

void PostMPMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
	PostThreadMessage(message_pump_thread_id, msg, wParam, lParam);
}

void InitMessagePump() {
	WNDCLASS popup_win_class = {0};
	popup_win_class.lpfnWndProc = PopupWindowProc;
	popup_win_class.hInstance = hInst;
	popup_win_class.lpszClassName = POP_WIN_CLASS;
	popup_win_class.hCursor = LoadCursor(NULL, IDC_ARROW); 
	RegisterClass(&popup_win_class);

	HMODULE hUserDll = GetModuleHandle(_T("user32.dll"));
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyAnimateWindow=(BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
		MyMonitorFromPoint = (HMONITOR (WINAPI*)(POINT, DWORD))GetProcAddress(hUserDll, "MonitorFromPoint");
#ifdef _UNICODE
		MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR, LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoW");
#else
		MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR, LPMONITORINFO))GetProcAddress(hUserDll, "GetMonitorInfoA");
#endif
	}



	CloseHandle(mir_forkthreadex(MessagePumpThread, NULL, 0, &message_pump_thread_id));
}

void DeinitMessagePump() {
	PostMPMessage(WM_QUIT, 0, 0);

	UnregisterClass(POP_WIN_CLASS, hInst);
}

INT_PTR ShowTip(WPARAM wParam, LPARAM lParam) {
	CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	if(clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if(clcit->isTreeFocused == 0 && options.show_no_focus == false) return 0;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)malloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->proto = 0;
	clcit2->text = 0;

	if(wParam) { // wParam is char pointer containing text - e.g. status bar tooltip
		clcit2->text = a2t((char *)wParam);
		GetCursorPos(&clcit2->ptCursor);

		//MessageBox(0, clcit2->text, _T("ShowTip"), MB_OK);
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
	return 1;
}

int ShowTipHook(WPARAM wParam, LPARAM lParam) {
    ShowTip(wParam, lParam);
    return 0;
}

#ifdef _UNICODE
INT_PTR ShowTipW(WPARAM wParam, LPARAM lParam) {
	CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	if(clcit->isGroup) return 0; // no group tips (since they're pretty useless)
	if(clcit->isTreeFocused == 0 && options.show_no_focus == false) return 0;

	CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)malloc(sizeof(CLCINFOTIPEX));
	memcpy(clcit2, clcit, sizeof(CLCINFOTIP));
	clcit2->cbSize = sizeof(CLCINFOTIPEX);
	clcit2->proto = 0;
	clcit2->text = 0;

	if(wParam) { // wParam is char pointer containing text - e.g. status bar tooltip
		clcit2->text = _tcsdup((TCHAR *)wParam);
		GetCursorPos(&clcit2->ptCursor);
	}

	PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);
	return 1;
}
#endif

INT_PTR HideTip(WPARAM wParam, LPARAM lParam) {
	//CLCINFOTIP *clcit = (CLCINFOTIP *)lParam;
	PostMPMessage(MUM_DELETEPOPUP, 0, 0);
	return 1;
}

int HideTipHook(WPARAM wParam, LPARAM lParam) {
    HideTip(wParam, lParam);
    return 0;
}

int ProtoAck(WPARAM wParam, LPARAM lParam) {
	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->type == ACKTYPE_AWAYMSG && ack->result == ACKRESULT_SUCCESS)
	{
#ifdef _UNICODE
		DBVARIANT dbv;
		if (!DBGetContactSetting(ack->hContact, "CList", "StatusMsg", &dbv))
		{
			bool unicode = (dbv.type == DBVT_UTF8 || dbv.type == DBVT_WCHAR);
			DBFreeVariant(&dbv);
			if (unicode) {
				DBGetContactSettingTString(ack->hContact, "CList", "StatusMsg", &dbv);
				PostMPMessage(MUM_GOTSTATUS, (WPARAM)ack->hContact, (LPARAM)_tcsdup(dbv.ptszVal));
				DBFreeVariant(&dbv);
				return 0;
			}
		}
#endif	
		char *szMsg = (char *)ack->lParam;
		if (szMsg && szMsg[0])
			PostMPMessage(MUM_GOTSTATUS, (WPARAM)ack->hContact, (LPARAM)a2t(szMsg));
	}
	else if (ack->type == ACKTYPE_GETINFO && ack->result == ACKRESULT_SUCCESS)
	{
		PostMPMessage(MUM_REDRAW, (WPARAM)ack->hContact, 0);
	}
		
	return 0;
}

int FramesShowSBTip(WPARAM wParam, LPARAM lParam) {
	if(options.status_bar_tips) {
		char *proto = (char *)wParam;

		CLCINFOTIPEX *clcit2 = (CLCINFOTIPEX *)malloc(sizeof(CLCINFOTIPEX));
		memset(clcit2, 0, sizeof(CLCINFOTIPEX));
		clcit2->cbSize = sizeof(CLCINFOTIPEX);
		clcit2->proto = proto; // assume static string
		GetCursorPos(&clcit2->ptCursor);

		PostMPMessage(MUM_CREATEPOPUP, 0, (LPARAM)clcit2);

		return 1;
	}
	return 0;
}

int FramesHideSBTip(WPARAM wParam, LPARAM lParam) {
	if(options.status_bar_tips) {
		PostMPMessage(MUM_DELETEPOPUP, 0, 0);
		return 1;
	}
	return 0;
}

