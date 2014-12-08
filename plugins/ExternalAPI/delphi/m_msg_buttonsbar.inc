{$IFNDEF M_MSG_BUTTONSBAR}
{$DEFINE M_MSG_BUTTONSBAR}

//////////////////////////////////////////////////////////////////////////
// Services
//
//////////////////////////////////////////////////////////////////////////
// Adding a button
//
// wParam  = 0
// lParam  = (BBButton *) &description
const
  MS_BB_ADDBUTTON:PAnsiChar = 'TabSRMM/ButtonsBar/AddButton';

//////////////////////////////////////////////////////////////////////////
// Remove button
//
// wParam = 0 
// lParam  = (BBButton *) &description, only button ID and ModuleName used
  MS_BB_REMOVEBUTTON:PAnsiChar = 'TabSRMM/ButtonsBar/RemoveButton';

//////////////////////////////////////////////////////////////////////////
// ModifyButton(global)
//
// wParam = 0
// lParam = (BBButton *) &description
  MS_BB_MODIFYBUTTON:PAnsiChar = 'TabSRMM/ButtonsBar/ModifyButton';


  BBSF_HIDDEN   = 1;
  BBSF_DISABLED = 2;
  BBSF_PUSHED   = 4;
  BBSF_RELEASED = 8;

//////////////////////////////////////////////////////////////////////////
// GetButtonState(local)
//
// wParam =  hContact
// lParam = (BBButton *) &description , only ModuleName and ID used
// Returns BBButton struct with BBSF_ bbbFlags: 
  MS_BB_GETBUTTONSTATE:PansiChar = 'TabSRMM/ButtonsBar/GetButtonState';

//////////////////////////////////////////////////////////////////////////
// SetButtonState (local)
//
// wParam =  hContact
// lParam = (BBButton *) &description , ModuleName, ID,hIcon,Tooltip, and BBSF_ bbbFlags are used
  MS_BB_SETBUTTONSTATE:PAnsiChar = 'TabSRMM/ButtonsBar/SetButtonState';


////////////////////////////////////////////////////////////////
//Events
//
///////////////////////////////////////////////////
// ToolBar loaded event
// wParam = 0;
// lParam = 0;
// This event will be send after module loaded and after each toolbar reset
// You should add your buttons on this event
  ME_MSG_TOOLBARLOADED:PAnsiChar = 'TabSRMM/ButtonsBar/ModuleLoaded';

///////////////////////////////////////////////////
// ButtonClicked event
// wParam = (HANDLE)hContact;
// lParam = (CustomButtonClickData *)&CustomButtonClickData;
// catch to show a popup menu, etc.
  ME_MSG_BUTTONPRESSED:PAnsiChar = 'TabSRMM/ButtonsBar/ButtonPressed';

//event flags
  BBCF_RIGHTBUTTON    = 1;
  BBCF_SHIFTPRESSED   = 2;
  BBCF_CONTROLPRESSED = 4;
  BBCF_ARROWCLICKED   = 8;

type
  pCustomButtonClickData = ^tCustomButtonClickData;
  tCustomButtonClickData = record
    cbSize    :int;
    pt        :TPOINT;    // screen coordinates for menus
    pszModule :PAnsiChar; // button owners name 
    dwButtonId:dword;     // registered button ID
    hwndFrom  :HWND;      // button parents HWND
    hContact  :TMCONTACT;
    flags     :dword;     // BBCF_ flags
  end;

const
//button flags
  BBBF_DISABLED      = $0001;
  BBBF_HIDDEN        = $0002;
  BBBF_ISPUSHBUTTON  = $0004;
  BBBF_ISARROWBUTTON = $0008;
  BBBF_ISCHATBUTTON  = $0010;
  BBBF_ISIMBUTTON    = $0020;
  BBBF_ISLSIDEBUTTON = $0040;
  BBBF_ISRSIDEBUTTON = $0080;
  BBBF_CANBEHIDDEN   = $0100;
  BBBF_ISDUMMYBUTTON = $0200;
  BBBF_ANSITOOLTIP   = $0400;
  BBBF_CREATEBYID    = $0800; //only for tabsrmm internal use

type
  tagBBButton = record
    cbSize       :int;   // size of structure
    dwButtonID   :dword; // your button ID, will be combined with pszModuleName for storing settings, etc...  
    pszModuleName:PAnsiChar; // module name without spaces and underline symbols (e.g. "tabsrmm")
    szTooltip    :TChar; // button's tooltip
    dwDefPos     :dword; // default order pos of button, counted from window edge (left or right)
                         // use value >100, because internal buttons using 10,20,30... 80, etc
    iButtonWidth :int;   // must be 0
    bbbFlags     :dword; // combine of BBBF_ flags above
    hIcon        :HICON; // Handle to icolib registered icon, it's better to register with pszSection = "TabSRMM/Toolbar"
  end;	
  tBBButton = tagBBButton;


{$ENDIF}
