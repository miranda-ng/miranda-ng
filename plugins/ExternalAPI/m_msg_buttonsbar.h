#ifndef M_MSG_BUTTONSBAR_H__
#define M_MSG_BUTTONSBAR_H__

////////////////////////////////////////////////////////////////
//Events
//

///////////////////////////////////////////////////
// ButtonClicked event
// wParam = (HANDLE)hContact;
// lParam = (CustomButtonClickData *)&CustomButtonClickData;
// catch to show a popup menu, etc.
#define ME_MSG_BUTTONPRESSED		"TabSRMM/ButtonsBar/ButtonPressed"


// event flags
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

#endif  //M_MSG_BUTTONSBAR_H__