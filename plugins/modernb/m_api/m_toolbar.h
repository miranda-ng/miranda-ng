#ifndef M_TOOLBAR_H
#define M_TOOLBAR_H

#define TOOLBARBUTTON_ICONIDPREFIX "MTB_"
#define TOOLBARBUTTON_ICONIDPRIMARYSUFFIX "_Primary"
#define TOOLBARBUTTON_ICONIDSECONDARYSUFFIX "_Secondary"
#define TOOLBARBUTTON_ICONNAMEPRESSEDSUFFIX "Pressed"

//button flags
#define TBBF_DISABLED			(1<<0)
#define TBBF_VISIBLE			(1<<1) 
#define TBBF_PUSHED				(1<<2)
#define TBBF_SHOWTOOLTIP		(1<<3)
#define TBBF_ISSEPARATOR		(1<<5)
#define TBBF_ISLBUTTON			(1<<6)
#define TBBF_FLEXSIZESEPARATOR  (TBBF_ISSEPARATOR|TBBF_PUSHED) 
typedef struct _tagTBButton
{
	int cbSize;				 // size of structure
	char * pszButtonID;		 // char id of button used to store button info in DB and know about icon 
	char * pszButtonName;    // name of button (not translated)
	char * pszServiceName;	 // service name to be executed
	LPARAM lParam;			 // param of service to be called
	char * pszTooltipUp, *pszTooltipDn;	
	DWORD defPos;			 // default order pos of button (less values are nearer to edge).. please use values greater that 100. the default buttons has pos: 10,20..90
	DWORD tbbFlags;			 // combine of TBBF_ flags above
	void (*ParamDestructor)(void *); //will be called on parameters deletion
	HANDLE hPrimaryIconHandle;
	HANDLE hSecondaryIconHandle;
}TBButton;

//////////////////////////////////////////////////////////////////////////
// Events
// Only after this event module subscribers should register their buttons
// wparam=lparam=0
// don't forget to return 0 to continue processing
#define ME_TB_MODULELOADED	 "ToolBar/ModuleLoaded"

//////////////////////////////////////////////////////////////////////////
// Services
//
//////////////////////////////////////////////////////////////////////////
// Adding a button
// WPARAM  = 0
// LPARAM  = (TBButton *) &description
// LRESULT = (HANDLE) hButton
// in order to correctly process default icons via iconlib it should be
// registered icolib icon with id named:
// 'TBButton_'+pszButtonID+ 'Up' or +'Down' for Push (2-state) buttons
#define MS_TB_ADDBUTTON "ToolBar/AddButton"

//////////////////////////////////////////////////////////////////////////
// Remove button
// WPARAM = (HANDLE) hButton;
// LPARAM = 0;
#define MS_TB_REMOVEBUTTON "ToolBar/RemoveButton"

//////////////////////////////////////////////////////////////////////////
// SetState
// WPARAM = (HANDLE) hButton;
// LPARAM = one of below TBST_ states 
// LRESULT= old state
#define TBST_PUSHED			1
#define TBST_RELEASED		0
#define MS_TB_SETBUTTONSTATE "ToolBar/SetButtonState"

//////////////////////////////////////////////////////////////////////////
// SetStatebyId
// WPARAM = (char *) szButtonID;
// LPARAM = one of below TBST_ states 
// LRESULT= old state
#define MS_TB_SETBUTTONSTATEBYID "ToolBar/SetButtonStateId"
//////////////////////////////////////////////////////////////////////////
// GetState
// WPARAM = (HANLDE) hButton;
// LPARAM = 0
// LRESULT= current state
#define MS_TB_GETBUTTONSTATE "ToolBar/GetButtonState"

//////////////////////////////////////////////////////////////////////////
// GetState
// WPARAM = (char *) szButtonID;;
// LPARAM = 0
// LRESULT= current state
#define MS_TB_GETBUTTONSTATEBYID "ToolBar/GetButtonStateId"


#endif