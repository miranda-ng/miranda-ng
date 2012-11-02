/*
	AutoReplacer plugin
	by Angelo Luiz Tartari
*/

#ifndef M_AUTOREPLACER_H
#define M_AUTOREPLACER_H

/*
  Adds a window handle to AutoReplacer.
  This handle must belong to any window based on a editbox (editbox, richtext, TMemo, TEdit, TMaskEdit, etc.).
  After adding a handle, AutoReplacer will automatically work on this window.
  wParam = 0
  lParam = (LPARAM)(HWND)hwnd
  Returns: 0 on success, -1 if hwnd is invalid, 1 on error.
  Example:
    if(ServiceExists(MS_AUTOREPLACER_ADDWINHANDLE))
		autoreplacer_AddWinHandle(GetDlgItem(hwndDlg, IDC_EDIT1));
*/
#define MS_AUTOREPLACER_ADDWINHANDLE "AutoReplacer/AddWinHandle"
static int __inline autoreplacer_AddWinHandle(HWND hwnd) {
	return (int)CallService(MS_AUTOREPLACER_ADDWINHANDLE, 0, (LPARAM)hwnd);
}

/*
  Removes a window handle from AutoReplacer's list.
  wParam = 0
  lParam = (LPARAM)(HWND)hwnd
  Returns: 0 on success, -1 if hwnd is invalid, 1 if hwnd wasn't found.
  Example:
    if(ServiceExists(MS_AUTOREPLACER_REMWINHANDLE))
		autoreplacer_RemWinHandle(GetDlgItem(hwndDlg, IDC_EDIT1));
*/
#define MS_AUTOREPLACER_REMWINHANDLE "AutoReplacer/RemWinHandle"
static int __inline autoreplacer_RemWinHandle(HWND hwnd) {
	return (int)CallService(MS_AUTOREPLACER_REMWINHANDLE, 0, (LPARAM)hwnd);
}

#endif