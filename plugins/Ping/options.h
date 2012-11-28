#ifndef _PING_OPTIONS
#define _PING_OPTIONS

#include "pinglist.h"
#include "utils.h"
#include "icmp.h"
#include "pingthread.h"

#include "resource.h"

// wake event for ping thread
extern HANDLE hWakeEvent;

extern PingOptions options;
extern PINGADDRESS add_edit_addr;

INT_PTR CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int PingOptInit(WPARAM wParam,LPARAM lParam);

BOOL Edit(HWND hwnd, PINGADDRESS &addr);

void LoadOptions();
void SaveOptions();

#endif
