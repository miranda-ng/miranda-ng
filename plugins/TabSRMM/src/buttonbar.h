#ifndef _BUTTONSBAR_H
#define _BUTTONSBAR_H

#define MIN_CBUTTONID 4000
#define MAX_CBUTTONID 5000

#define BBSF_IMBUTTON		(1<<0)
#define BBSF_CHATBUTTON		(1<<1)
#define BBSF_CANBEHIDDEN	(1<<2)
#define BBSF_NTBSWAPED		(1<<3)
#define BBSF_NTBDESTRUCT	(1<<4)

struct CustomButtonData : public MZeroedObject
{
	~CustomButtonData()
	{
		mir_free(pszModuleName);
		mir_free(ptszTooltip);
	}

	DWORD  dwPosition;    // default order pos of button, counted from window edge (left or right)

	DWORD  dwButtonOrigID; // id of button used while button creation and to store button info in DB
	char  *pszModuleName;  // module name without spaces and underline symbols (e.g. "tabsrmm")

	DWORD  dwButtonCID;
	DWORD  dwArrowCID;    // only use with BBBF_ISARROWBUTTON flag

	TCHAR *ptszTooltip;   // button's tooltip

	int    iButtonWidth;  // must be 22 for regular button and 33 for button with arrow
	HANDLE hIcon;         // Handle to icolib registred icon
	
	bool   bIMButton, bChatButton;
	bool   bCanBeHidden, bHidden, bAutoHidden, bSeparator, bDisabled, bPushButton;
	bool   bLSided, bRSided;
	BYTE   opFlags;
};

static INT_PTR CB_ModifyButton(WPARAM wParam, LPARAM lParam);
static INT_PTR CB_RemoveButton(WPARAM wParam, LPARAM lParam);
static INT_PTR CB_AddButton(WPARAM wParam, LPARAM lParam);
static INT_PTR CB_GetButtonState(WPARAM wParam, LPARAM lParam);
static INT_PTR CB_SetButtonState(WPARAM wParam, LPARAM lParam);
static void CB_GetButtonSettings(MCONTACT hContact,CustomButtonData *cbd);

void CB_WriteButtonSettings(MCONTACT hContact,CustomButtonData *cbd);
int  sstSortButtons(const void * vmtbi1, const void * vmtbi2);

void CB_DeInitCustomButtons();
void CB_InitCustomButtons();
void CB_InitDefaultButtons();
void CB_ReInitCustomButtons();

/* MinGW doesn't like this struct declatations below */
void BB_UpdateIcons(HWND hdlg,TWindowData *dat);
void BB_RefreshTheme(const TWindowData *dat);
void CB_DestroyAllButtons(HWND hwndDlg,TWindowData *dat);
void CB_DestroyButton(HWND hwndDlg,TWindowData *dat,DWORD dwButtonCID,DWORD dwFlags);
void CB_ChangeButton(HWND hwndDlg,TWindowData *dat,CustomButtonData* cbd);

#endif
