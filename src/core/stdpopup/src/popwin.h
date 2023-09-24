#ifndef _POPWIN_INC
#define _POPWIN_INC

#define POP_WIN_CLASS        L"PopupWnd2"

#define PUM_SETTEXT          (WM_USER + 0x020)
#define PUM_GETCONTACT       (WM_USER + 0x021)
#define PUM_GETOPAQUE        (WM_USER + 0x022)
#define PUM_CHANGE           (WM_USER + 0x023)
#define PUM_MOVE             (WM_USER + 0x024)
#define PUM_GETHEIGHT        (WM_USER + 0x025)
#define PUM_UPDATERGN        (WM_USER + 0x026)

#define ANIMATE_NO      0
#define ANIMATE_HORZ    1
#define ANIMATE_VERT    2

void DeinitWindowStack();

void BroadcastMessage(UINT msg, WPARAM wParam, LPARAM lParam);

void RepositionWindows();

LRESULT CALLBACK PopupWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void UpdateMenu();

#endif
