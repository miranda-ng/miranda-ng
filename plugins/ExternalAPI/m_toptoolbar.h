#ifndef M_TOPTOOLBAR_H
#define M_TOPTOOLBAR_H

//button flags
#define TTBBF_DISABLED       0x0001
#define TTBBF_VISIBLE        0x0002
#define TTBBF_PUSHED         0x0004
#define TTBBF_SHOWTOOLTIP    0x0008
#define TTBBF_ISSEPARATOR    0x0020
#define TTBBF_ISLBUTTON      0x0040
#define TTBBF_ICONBYHANDLE   0x0080
#define TTBBF_ISSBUTTON      0x0100
#define TTBBF_ASPUSHBUTTON   0x0200

typedef struct {
	int cbSize;
	union {
		char *pszService;
		TCHAR *program;
	};
	DWORD dwFlags;
	LPARAM lParamUp;
	WPARAM wParamUp;
	LPARAM lParamDown;
	WPARAM wParamDown;
	char *name;
	union {
		HICON hIconUp;
		HANDLE hIconHandleUp;
	};
	union {
		HICON hIconDn;
		HANDLE hIconHandleDn;
	};
}
	TTBButton, * lpTTBButton;

//=== EVENTS ===
/*
toptoolbar/moduleloaded event
wParam = lParam = 0
Called when the toolbar services are available

!!!Warning you may work with TTB services only in this event or later.

*/
#define ME_TTB_MODULELOADED				"TopToolBar/ModuleLoaded"



//=== SERVICES ===
/*
toptoolbar/addbutton service
wparam = (TTBButton*)lpTTBButton
lparam = 0
returns: hTTBButton - handle of added button on success, -1 on failure.
*/
#define MS_TTB_ADDBUTTON						"TopToolBar/AddButton"

/*
toptoolbar/removebutton service
wparam = (HANDLE)hTTButton
lparam = 0
returns: 0 on success, -1 on failure.
*/
#define MS_TTB_REMOVEBUTTON					"TopToolBar/RemoveButton"

/*
toptoolbar/setstate service
wparam = (HANDLE)hTTButton
lparam = (LPARAM) state
returns: 0 on success, -1 on failure.
*/
#define TTBST_PUSHED			1
#define TTBST_RELEASED		2

#define MS_TTB_SETBUTTONSTATE					"TopToolBar/SetState"

/*
toptoolbar/getstate service
wparam = (HANDLE)hTTButton
lparam = 0
returns: state on success, -1 on failure.
*/
#define MS_TTB_GETBUTTONSTATE					"TopToolBar/GetState"

/*
toptoolbar/getoptions service
(HIWORD)wparam = (HANDLE)hTTButton
(LOWORD)wparam = TTBO_FLAG
lparam = 0,or lparam=lpTTBButton if flag=TTBO_ALLDATA
returns: value on success, -1 on failure.
*/
#define TTBO_FLAGS            0     //get/set all flags
#define TTBO_WIDTH            2     //not impemented
#define TTBO_HEIGHT           3     //not impemented
#define TTBO_TIPNAME          4     //tool tip name
#define TTBO_ALLDATA          5     //change all data via lparam=lpTTBButton


#define MS_TTB_GETBUTTONOPTIONS					"TopToolBar/GetOptions"

/*
toptoolbar/setoptions service
(HIWORD)wparam = (HANDLE)hTTButton
(LOWORD)wparam = TTBO_FLAG
lparam = value
returns: 1 on success, -1 on failure.
*/
#define MS_TTB_SETBUTTONOPTIONS					"TopToolBar/SetOptions"


#endif