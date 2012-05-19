#ifndef _POPUPS_INC
#define _POPUPS_INC

#include <m_popup.h>

#include "options.h"
#include "icons.h"

void InitPopups();
void DeinitPopups();

bool ArePopupsEnabled();

void ShowPopupA(HANDLE hContact, const char* line1, const char* line2, int flags = 0, int timeout = 0);
void ShowPopupW(HANDLE hContact, const wchar_t* line1, const wchar_t* line2, int flags = 0, int timeout = 0);

void ShowPopup(HANDLE hContact, const TCHAR *line1, const TCHAR *line2, int flags = 0, int timeout = 0);

void ShowWarning(TCHAR *msg);
void ShowError(TCHAR *msg);

void ChangePopupText(HWND hwnd, TCHAR *msg);

extern HWND hwndPop;
extern HANDLE hEventPop;
extern bool pop_cancelled;

#define WMU_CLOSEPOP	(WM_USER + 0x191)

#define POPFLAG_SAVEHWND		0x01

#endif
