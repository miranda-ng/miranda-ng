#pragma once
INT_PTR SVC_StartOTR(WPARAM wParam, LPARAM lParam);
INT_PTR SVC_RefreshOTR(WPARAM wParam, LPARAM lParam);
INT_PTR SVC_StopOTR(WPARAM wParam, LPARAM lParam);
INT_PTR SVC_VerifyOTR(WPARAM wParam, LPARAM lParam);
int SVC_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
void InitMenu();

#define MS_OTR_MENUSTART		MODULENAME"/Start"
#define MS_OTR_MENUSTOP			MODULENAME"/Stop"
#define MS_OTR_MENUREFRESH		MODULENAME"/Refresh"
#define MS_OTR_MENUVERIFY		MODULENAME"/Verify"
#define MS_OTR_MENUTOGGLEHTML	MODULENAME"/ToggleHTML"