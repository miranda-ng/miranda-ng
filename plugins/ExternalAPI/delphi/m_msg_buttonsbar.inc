{$IFNDEF M_MSG_BUTTONSBAR}
{$DEFINE M_MSG_BUTTONSBAR}

const
  BBSF_HIDDEN   = 1;
  BBSF_DISABLED = 2;
  BBSF_PUSHED   = 4;
  BBSF_RELEASED = 8;

////////////////////////////////////////////////////////////////
//Events
//
///////////////////////////////////////////////////
// ToolBar loaded event
// wParam = 0;
// lParam = 0;
// This event will be send after module loaded and after each toolbar reset
// You should add your buttons on this event
  ME_MSG_TOOLBARLOADED:PAnsiChar = 'SRMM/ButtonsBar/ModuleLoaded';

///////////////////////////////////////////////////
// ButtonClicked event
// wParam = (HANDLE)hContact;
// lParam = (CustomButtonClickData *)&CustomButtonClickData;
// catch to show a popup menu, etc.
  ME_MSG_BUTTONPRESSED:PAnsiChar = 'SRMM/ButtonsBar/ButtonPressed';

//event flags
  BBCF_RIGHTBUTTON    = 1;
  BBCF_SHIFTPRESSED   = 2;
  BBCF_CONTROLPRESSED = 4;
  BBCF_ARROWCLICKED   = 8;

type
  pCustomButtonClickData = ^tCustomButtonClickData;
  tCustomButtonClickData = record
    pszModule :PAnsiChar; // button owners name 
    pt        :TPOINT;    // screen coordinates for menus
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
  BBBF_ISRSIDEBUTTON = $0080;
  BBBF_CANBEHIDDEN   = $0100;
  BBBF_ISDUMMYBUTTON = $0200;

type
  pBBButton = ^tBBButton;
  tBBButton = record
    pszModuleName:PAnsiChar; // module name without spaces and underline symbols (e.g. "tabsrmm")
    dwButtonID   :dword; // your button ID, will be combined with pszModuleName for storing settings, etc...  
    szText       :PWideChar; // button's text
    szTooltip    :PWideChar; // button's tooltip
    dwDefPos     :dword; // default order pos of button, counted from window edge (left or right)
                         // use value >100, because internal buttons using 10,20,30... 80, etc
    bbbFlags     :dword; // combine of BBBF_ flags above
    hIcon        :HICON; // Handle to icolib registered icon, it's better to register with pszSection = "TabSRMM/Toolbar"
  end;	

function Srmm_RemoveButton(bbdi:pBBButton) : integer; stdcall; external AppDll;
function Srmm_SetButtonState(hContact:TMCONTACT;bbdi:pBBButton) : integer; stdcall; external AppDll;
                                                             
{$ENDIF}
