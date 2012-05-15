#ifndef _PROGRESS_DIALOG_INC
#define _PROGRESS_DIALOG_INC

#include "icons.h"

#define WMU_SETMESSAGE			(WM_USER + 0x140)		// wParam = char *msg
#define WMU_SETPROGRESS			(WM_USER + 0x141)		// wParam = [1 to 100]

extern HWND hwndProgress;
void CreateProgressWindow();
void ProgressWindowDone();

#endif
