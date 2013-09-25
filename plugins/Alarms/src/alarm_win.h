#ifndef _ALARM_WIN_INC
#define _ALARM_WIN_INC

#include "options.h"
#include "alarmlist.h"

INT_PTR CALLBACK DlgProcAlarm(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define WMU_SETALARM				(WM_USER + 0x100)
#define WMU_FAKEALARM				(WM_USER + 0x101)

#define WMU_SETOPT					(WM_USER + 60)

extern HANDLE hAlarmWindowList;

void SetAlarmWinOptions();

void InitAlarmWin();
void DeinitAlarmWin();

#endif
