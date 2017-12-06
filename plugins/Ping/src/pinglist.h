#ifndef _PINGLIST_H
#define _PINGLIST_H

INT_PTR LoadPingList(WPARAM wParam, LPARAM lParam);
INT_PTR GetPingList(WPARAM wParam, LPARAM lParam);
INT_PTR SavePingList(WPARAM wParam, LPARAM lParam);
INT_PTR SetPingList(WPARAM wParam, LPARAM lParam); // use when you modified db yourself
INT_PTR SetAndSavePingList(WPARAM wParam, LPARAM lParam);
INT_PTR ClearPingList(WPARAM wParam, LPARAM lParam);
INT_PTR GetListSize(WPARAM wParam, LPARAM lParam);

// only call with list_cs locked!
void write_ping_addresses();


BOOL changing_clist_handle();
void set_changing_clist_handle(BOOL flag);
#endif
