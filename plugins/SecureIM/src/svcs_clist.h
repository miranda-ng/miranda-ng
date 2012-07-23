#ifndef __SVCS_CLIST_H__
#define __SVCS_CLIST_H__

#include <windows.h>

int __cdecl onContactSettingChanged(WPARAM,LPARAM);
int __cdecl onContactAdded(WPARAM,LPARAM);
int __cdecl onContactDeleted(WPARAM,LPARAM);
int __cdecl onExtraImageListRebuilding(WPARAM,LPARAM);
int __cdecl onExtraImageApplying(WPARAM,LPARAM);
int __cdecl onRebuildContactMenu(WPARAM,LPARAM);

#endif
