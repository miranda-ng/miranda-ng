#pragma once

#ifndef modern_global_structure_h__
#define modern_global_structure_h__

typedef struct tagCLUIDATA
{
	/************************************
	**         Global variables       **
	************************************/

	/*         NotifyArea menu          */
	HMENU  hMenuNotify;
	WORD   wNextMenuID;
	int    iIconNotify;
	BOOL   bEventAreaEnabled;
	BOOL   bNotifyActive;
	DWORD  dwFlags;
	int    hIconNotify;
	MCONTACT hUpdateContact;

	/*         Contact List View Mode          */
	TCHAR  groupFilter[2048];
	char   protoFilter[2048];
	char   varFilter[2048];
	DWORD  lastMsgFilter;
	char   current_viewmode[256], old_viewmode[256];
	BYTE   boldHideOffline;
	BYTE   bOldUseGroups;
	DWORD  statusMaskFilter;
	DWORD  stickyMaskFilter;
	DWORD  filterFlags;
	DWORD  bFilterEffective;
	DWORD  t_now;

	// Modern Global Variables
	BOOL   fDisableSkinEngine;
	BOOL   fOnDesktop;
	BOOL   fSmoothAnimation;
	BOOL   fLayered;
	BOOL   fDocked;
	BOOL   fSortNoOfflineBottom;
	BOOL   fAutoSize;
	BOOL   fAeroGlass;
	HRGN   hAeroGlassRgn;

	BOOL   mutexPreventDockMoving;
	BOOL   mutexOnEdgeSizing;
	BOOL   mutexPaintLock;

	BYTE   bCurrentAlpha;
	BYTE   bSTATE;
	BYTE   bBehindEdgeSettings;
	BYTE   bSortByOrder[3];

	signed char nBehindEdgeState;

	DWORD  dwKeyColor;

	HWND   hwndEventFrame;

	int    LeftClientMargin;
	int    RightClientMargin;
	int    TopClientMargin;
	int    BottomClientMargin;

	BOOL   bInternalAwayMsgDiscovery;
	BOOL   bRemoveAwayMessageForOffline;

	//hEventHandles

	HANDLE hEventBkgrChanged;
	HANDLE hEventPreBuildTrayMenu;
	HANDLE hEventPreBuildGroupMenu;
	HANDLE hEventPreBuildSubGroupMenu;
	HANDLE hEventStatusBarShowToolTip;
	HANDLE hEventStatusBarHideToolTip;
	HANDLE hEventSkinServicesCreated;

	int    nGapBetweenTitlebar;
} CLUIDATA;

EXTERN_C CLUIDATA g_CluiData;

#endif // modern_global_structure_h__
