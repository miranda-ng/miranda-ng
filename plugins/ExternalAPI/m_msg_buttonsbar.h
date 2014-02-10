#ifndef M_MSG_BUTTONSBAR_H__
#define M_MSG_BUTTONSBAR_H__

//////////////////////////////////////////////////////////////////////////
// Services
//
//////////////////////////////////////////////////////////////////////////
// Adding a button
//
// wParam  = 0
// lParam  = (BBButton *) &description
// Returns 0 on success, 1 otherwise
#define MS_BB_ADDBUTTON "TabSRMM/ButtonsBar/AddButton"

//////////////////////////////////////////////////////////////////////////
// Remove button
//
// wParam = 0
// lParam  = (BBButton *) &description, only button ID and ModuleName used
// Returns 0 on success, 1 otherwise
#define MS_BB_REMOVEBUTTON "TabSRMM/ButtonsBar/RemoveButton"

//////////////////////////////////////////////////////////////////////////
// ModifyButton(global)
//
// wParam = 0
// lParam = (BBButton *) &description
// Returns 0 on success, 1 otherwise
#define MS_BB_MODIFYBUTTON		"TabSRMM/ButtonsBar/ModifyButton"


#define BBSF_HIDDEN		(1<<0)
#define BBSF_DISABLED	(1<<1)
#define BBSF_PUSHED		(1<<2)
#define BBSF_RELEASED	(1<<3)

//////////////////////////////////////////////////////////////////////////
// GetButtonState(local)
//
// wParam =  hContact
// lParam = (BBButton *) &description , only ModuleName and ID used
// Returns BBButton struct with BBSF_ bbbFlags:
// Returns 0 on success, 1 otherwise
#define MS_BB_GETBUTTONSTATE	"TabSRMM/ButtonsBar/GetButtonState"

//////////////////////////////////////////////////////////////////////////
// SetButtonState (local)
//
// wParam =  hContact
// lParam = (BBButton *) &description , ModuleName, ID,hIcon,Tooltip, and BBSF_ bbbFlags are used
// Returns 0 on success, 1 otherwise
#define MS_BB_SETBUTTONSTATE	 "TabSRMM/ButtonsBar/SetButtonState"


////////////////////////////////////////////////////////////////
//Events
//
///////////////////////////////////////////////////
// ToolBar loaded event
// wParam = 0;
// lParam = 0;
// This event will be send after module loaded and after each toolbar reset
// You should add your buttons on this event
#define ME_MSG_TOOLBARLOADED		"TabSRMM/ButtonsBar/ModuleLoaded"

///////////////////////////////////////////////////
// ButtonClicked event
// wParam = (HANDLE)hContact;
// lParam = (CustomButtonClickData *)&CustomButtonClickData;
// catch to show a popup menu, etc.
#define ME_MSG_BUTTONPRESSED		"TabSRMM/ButtonsBar/ButtonPressed"


//event flags
#define BBCF_RIGHTBUTTON	(1<<0)
#define BBCF_SHIFTPRESSED	(1<<1)
#define BBCF_CONTROLPRESSED	(1<<2)
#define BBCF_ARROWCLICKED	(1<<3)

typedef struct {
	int cbSize;
	POINT pt;          // screen coordinates for menus
	char* pszModule;   // button owners name
	DWORD dwButtonId;  // registered button ID
	HWND hwndFrom;     // button parents HWND
	MCONTACT hContact;
	DWORD flags;       // BBCF_ flags
} CustomButtonClickData;


//button flags
#define BBBF_DISABLED			(1<<0)
#define BBBF_HIDDEN				(1<<1)
#define BBBF_ISPUSHBUTTON		(1<<2)
#define BBBF_ISARROWBUTTON		(1<<3)
#define BBBF_ISCHATBUTTON		(1<<4)
#define BBBF_ISIMBUTTON			(1<<5)
#define BBBF_ISLSIDEBUTTON		(1<<6)
#define BBBF_ISRSIDEBUTTON		(1<<7)
#define BBBF_CANBEHIDDEN		(1<<8)
#define BBBF_ISDUMMYBUTTON		(1<<9)
#define BBBF_ANSITOOLTIP		(1<<10)

#define BBBF_CREATEBYID			(1<<11)  //only for tabsrmm internal use

typedef struct _tagBBButton
{
	int cbSize;           // size of structure

	DWORD dwButtonID;     // your button ID, will be combined with pszModuleName for storing settings, etc...

	char* pszModuleName;  //module name without spaces and underline symbols (e.g. "tabsrmm")
	union{
		char* pszTooltip;  //button's tooltip
		TCHAR* ptszTooltip;
	};
	DWORD dwDefPos;       // default order pos of button, counted from window edge (left or right)
	                      // use value >100, because internal buttons using 10,20,30... 80, etc
	int iButtonWidth;		 // must be 0
	DWORD bbbFlags;       // combine of BBBF_ flags above
	HANDLE hIcon;         // Handle to icolib registered icon, it's better to register with pszSection = "TabSRMM/Toolbar"
}
	BBButton;

#endif  //M_MSG_BUTTONSBAR_H__