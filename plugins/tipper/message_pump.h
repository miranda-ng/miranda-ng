#ifndef _MESSAGE_PUMP_INC
#define _MESSAGE_PUMP_INC

void PostMPMessage(UINT msg, WPARAM, LPARAM);

#define MUM_CREATEPOPUP					(WM_USER + 0x011)
#define MUM_DELETEPOPUP					(WM_USER + 0x012)
#define MUM_GOTSTATUS					(WM_USER + 0x013)
#define MUM_GOTAVATAR					(WM_USER + 0x014)
#define MUM_REDRAW                      (WM_USER + 0x015)

extern BOOL (WINAPI *MySetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
extern BOOL (WINAPI *MyAnimateWindow)(HWND hWnd,DWORD dwTime,DWORD dwFlags);
extern HMONITOR (WINAPI *MyMonitorFromPoint)(POINT, DWORD);
extern BOOL (WINAPI *MyGetMonitorInfo)(HMONITOR, LPMONITORINFO);

void InitMessagePump();
void DeinitMessagePump();

INT_PTR ShowTip(WPARAM wParam, LPARAM lParam);
INT_PTR ShowTipW(WPARAM wParam, LPARAM lParam);
INT_PTR HideTip(WPARAM wParam, LPARAM lParam);

int ShowTipHook(WPARAM wParam, LPARAM lParam);
int HideTipHook(WPARAM wParam, LPARAM lParam);

int FramesShowSBTip(WPARAM wParam, LPARAM lParam);
int FramesHideSBTip(WPARAM wParam, LPARAM lParam);

int ProtoAck(WPARAM wParam, LPARAM lParam);

typedef struct {
	int cbSize;
	int isTreeFocused;   //so the plugin can provide an option
	int isGroup;     //0 if it's a contact, 1 if it's a group
	HANDLE hItem;	 //handle to group or contact
	POINT ptCursor;
	RECT rcItem;
	TCHAR *text;		// for tips with specific text
	char *proto;	// for proto tips
} CLCINFOTIPEX;


#endif
