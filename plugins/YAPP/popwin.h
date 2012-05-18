#ifndef _POPWIN_INC
#define _POPWIN_INC

#define POP_WIN_CLASS					_T(MODULE) _T("YAPPWindowClass")

#define PUM_SETTEXT						(WM_USER + 0x020)
#define PUM_GETCONTACT					(WM_USER + 0x021)
#define PUM_GETOPAQUE					(WM_USER + 0x022)
#define PUM_CHANGE						(WM_USER + 0x023)
#define PUM_MOVE						(WM_USER + 0x024)
#define PUM_GETHEIGHT					(WM_USER + 0x025)
#define PUM_UPDATERGN					(WM_USER + 0x026)

#define PUM_SETNOTIFYH					(WM_USER + 0x030)
#define PUM_KILLNOTIFY					(WM_USER + 0x031)
#define PUM_UPDATENOTIFY				(WM_USER + 0x032)

void InitWindowStack();
void DeinitWindowStack();

void BroadcastMessage(UINT msg, WPARAM wParam, LPARAM lParam);

void RepositionWindows();

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


#endif
