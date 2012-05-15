#ifndef _POPWIN_INC
#define _POPWIN_INC

#define POP_WIN_CLASS					_T(MODULE) _T("MimTTClass")

#define PUM_GETHEIGHT					(WM_USER + 0x020)
#define PUM_CALCPOS						(WM_USER + 0x021)
#define PUM_SETSTATUSTEXT				(WM_USER + 0x022)
#define PUM_UPDATERGN					(WM_USER + 0x023)
#define PUM_SETAVATAR					(WM_USER + 0x024)
#define PUM_REFRESH_VALUES				(WM_USER + 0x025)

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int AvatarChanged(WPARAM wParam, LPARAM lParam); // exposed so hook/unhook is in main thread

#endif
