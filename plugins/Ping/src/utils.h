#ifndef _PING_UTILS
#define _PING_UTILS

void __stdcall	ShowPopup(TCHAR *line1, TCHAR *line2, int flags );

INT_PTR PluginPing(WPARAM wParam,LPARAM lParam);

INT_PTR PingDisableAll(WPARAM wParam, LPARAM lParam);
INT_PTR PingEnableAll(WPARAM wParam, LPARAM lParam);

INT_PTR ToggleEnabled(WPARAM wParam, LPARAM lParam);

INT_PTR DblClick(WPARAM wParam, LPARAM lParam);

INT_PTR EditContact(WPARAM wParam, LPARAM lParam);
// read in addresses from old pingplug
void import_ping_addresses();

// use icolib if possible
void InitUtils();
extern HICON hIconResponding, hIconNotResponding, hIconTesting, hIconDisabled;


#endif
