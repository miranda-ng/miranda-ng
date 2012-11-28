#ifndef _PING_UTILS
#define _PING_UTILS

#pragma warning( disable : 4786 )

#include "pingthread.h" // for mainthread, for popup
//#include "icmp.h"
#include "rawping.h"
#include "icmp.h"
#include "options.h"

void __stdcall	ShowPopup( const char* line1, const char* line2, int flags );

INT_PTR PluginPing(WPARAM wParam,LPARAM lParam);

void Lock(CRITICAL_SECTION *cs, char *lab);
void Unlock(CRITICAL_SECTION *cs);

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
