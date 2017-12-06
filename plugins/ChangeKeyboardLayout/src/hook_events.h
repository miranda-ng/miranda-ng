#ifndef M_CKL_HOOK_EVENTS_H
#define M_CKL_HOOK_EVENTS_H

#include "stdafx.h"

void ReadMainOptions();
void WriteMainOptions();
void ReadPopupOptions();
void WritePopupOptions();
extern int OnOptionsInitialise(WPARAM wParam, LPARAM lParam);
extern int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int CALLBACK CKLPopupDlgProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);

#endif