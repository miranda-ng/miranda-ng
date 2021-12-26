#pragma once

#ifndef modern_global_structure_h__
#define modern_global_structure_h__

struct CLUIDATA
{
	/************************************
	**         Global variables       **
	************************************/

	// NotifyArea menu
	HMENU    hMenuNotify;
	WORD     wNextMenuID;
	int      iIconNotify;
	DWORD    dwFlags;
	int      hIconNotify;
	MCONTACT hUpdateContact;

	// Contact List View Mode
	wchar_t  groupFilter[2048];
	char     protoFilter[2048];
	char     varFilter[2048];
	DWORD    lastMsgFilter;
	char     current_viewmode[256], old_viewmode[256];
	uint8_t     boldHideOffline;
	uint8_t     bOldUseGroups;
	DWORD    statusMaskFilter;
	DWORD    stickyMaskFilter;
	DWORD    filterFlags;
	DWORD    bFilterEffective;
	DWORD    t_now;

	// Modern Global Variables
	int      fDocked;
	HRGN     hAeroGlassRgn;

	BOOL     mutexPreventDockMoving;
	BOOL     mutexOnEdgeSizing;
	BOOL     mutexPaintLock;

	bool     bEventAreaEnabled;
	bool     bNotifyActive;
	bool     fDisableSkinEngine;
	bool     fOnDesktop;
	bool     fSmoothAnimation;
	bool     fLayered;
	bool     fSortNoOfflineBottom;
	bool     fAutoSize;
	bool     fAeroGlass;
	uint8_t     bCurrentAlpha;
	uint8_t     bSTATE;
	uint8_t     bBehindEdgeSettings;
	uint8_t     bSortByOrder[3];

	signed char nBehindEdgeState;

	DWORD   dwKeyColor;

	HWND    hwndEventFrame;

	int     LeftClientMargin;
	int     RightClientMargin;
	int     TopClientMargin;
	int     BottomClientMargin;

	BOOL    bInternalAwayMsgDiscovery;
	BOOL    bRemoveAwayMessageForOffline;
			  
	//hEventHandles

	HANDLE  hEventBkgrChanged;
	HANDLE  hEventStatusBarShowToolTip;
	HANDLE  hEventStatusBarHideToolTip;
	HANDLE  hEventSkinServicesCreated;

	int     nGapBetweenTitlebar;

	__inline bool hasSort(int order) const
	{
		for (auto &it : bSortByOrder)
			if (it == order)
				return true;

		return false;
	}
};

EXTERN_C CLUIDATA g_CluiData;

#endif // modern_global_structure_h__
