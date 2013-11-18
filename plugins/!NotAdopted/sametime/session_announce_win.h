#ifndef _SESSION_ANNOUNCE_WIN_INC
#define _SESSION_ANNOUNCE_WIN_INC

#include "common.h"

typedef struct {
	TCHAR msg[MAX_MESSAGE_SIZE];
	GList *recipients;
} AnnouncementData;

typedef void (*SendAnnounceFunc)(AnnouncementData *ad);

INT_PTR CALLBACK SessionAnnounceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif
