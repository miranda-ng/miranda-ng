#ifndef _CONF_DIALOG_INC
#define _CONF_DIALOG_INC

#include "utils.h"

// return values
#define CD_CANCEL			1
#define CD_NOINSTALL		2
#define CD_CONFALL			3
#define CD_OK				4

INT_PTR CALLBACK DlgProcConfirm(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
