#ifndef _PING_OPTIONS
#define _PING_OPTIONS

INT_PTR CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int PingOptInit(WPARAM wParam, LPARAM lParam);

bool Edit(HWND hwnd, PINGADDRESS &addr);

void LoadOptions();
void SaveOptions();

#endif
