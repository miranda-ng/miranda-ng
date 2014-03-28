{$IFNDEF M_TOPTOOLBAR}
{$DEFINE M_TOPTOOLBAR}

const
  TTB_OPTDIR = 'TopToolBar';

//button flags
const
  TTBBF_DISABLED     = $0001;
  TTBBF_VISIBLE      = $0002;
  TTBBF_PUSHED       = $0004;
  TTBBF_SHOWTOOLTIP  = $0008;
  TTBBF_ISSEPARATOR  = $0020;
  TTBBF_ISLBUTTON    = $0040;
  TTBBF_ISSBUTTON    = $0100;
  TTBBF_ASPUSHBUTTON = $0200;

type
  PTTBButton = ^TTBButton;
  TTBButton = record
    cbSize       :int;
    pszService   :PAnsiChar; // or program for lButtons
    dwFlags      :dword;
    lParamUp     :LPARAM;
    wParamUp     :WPARAM;
    lParamDown   :LPARAM;
    wParamDown   :WPARAM;
    name         :PAnsiChar;
    hIconUp      :HICON;     // or hIconHandleUp:THANDLE
    hIconDn      :HICON;     // or hIconHandleDn:THANDLE
    pszTooltipUp :PansiChar;
    pszTooltipDn :PansiChar;
  end;

const

//=== EVENTS ===
{
  toptoolbar/moduleloaded event
  wParam = lParam = 0
  Called when the toolbar services are available

  !!!Warning you may work with TTB services only in this event or later.

  Im use settimer() - so notify appear in miranda message loop
  after all onmodulesload calls.
}
  ME_TTB_MODULELOADED:PAnsiChar = 'TopToolBar/ModuleLoaded';


{
//=== SERVICES ===

  toptoolbar/addbutton service
  wparam = (TTBButton*)lpTTBButton
  lparam = (hLangpack)
  returns: hTTBButton - handle of added button on success, -1 on failure.
}
  MS_TTB_ADDBUTTON:PAnsiChar = 'TopToolBar/AddButton';

{
  toptoolbar/removebutton service
  wparam = (HANDLE)hTTButton
  lparam = 0
  returns: 0 on success, -1 on failure.
}
  MS_TTB_REMOVEBUTTON:PAnsiChar = 'TopToolBar/RemoveButton';

{
  toptoolbar/setstate service
  wparam = (HANDLE)hTTButton
  lparam = (LPARAM) state
  returns: 0 on success, -1 on failure.
}
  TTBST_PUSHED   = 1;
  TTBST_RELEASED = 2;

  MS_TTB_SETBUTTONSTATE:PAnsiChar = 'TopToolBar/SetState';

{
  toptoolbar/getstate service
  wparam = (HANDLE)hTTButton
  lparam = 0
  returns: state on success, -1 on failure.
}
  MS_TTB_GETBUTTONSTATE:PAnsiChar = 'TopToolBar/GetState';

{
  toptoolbar/getoptions service
  (HIWORD)wparam = (HANDLE)hTTButton
  (LOWORD)wparam = TTBO_FLAG
  lparam = 0,or lparam=lpTTBButton if flag=TTBO_ALLDATA
  returns: value on success, -1 on failure.
}
  TTBO_FLAGS   = 0; // get/set all flags
  TTBO_POS     = 1; // position
  TTBO_WIDTH   = 2; // not impemented
  TTBO_HEIGHT  = 3; // not impemented
  TTBO_TIPNAME = 4; // tool tip name
  TTBO_ALLDATA = 5; // change all data via lparam=lpTTBButton

  MS_TTB_GETBUTTONOPTIONS:PAnsiChar = 'TopToolBar/GetOptions';

{
  toptoolbar/setoptions service
  (HIWORD)wparam = (HANDLE)hTTButton
  (LOWORD)wparam = TTBO_FLAG
  lparam = value
  returns: 1 on success, -1 on failure.
}
  MS_TTB_SETBUTTONOPTIONS:PAnsiChar = 'TopToolBar/SetOptions';

{
  toptoolbar/setcustomproc service
  wparam = pfnCustomproc
  lparam = procedure parameter
  returns: always returns 0.
}
  MS_TTB_SETCUSTOMPROC:PAnsiChar = 'TopToolBar/SetCustomProc';

const
  TTB_WINDOW_HANDLE = THANDLE(-1);

{
  this procedure is executed on each button on toolbar
  and on a toolbar itself with hTTButton == TTB_WINDOW_HANDLE
  hTTButton = button handle
  hwndBtn   = button window handle
  userInfo  = lParam passed into TopToolbar_SetCustomProc
}
type
  pfnCustomProc = procedure(hTTButton:THANDLE; hwndBtn:HWND; userInfo:LPARAM); cdecl;


///////////////////////////////////////////////////////////////////////////////
// Toolbar internal structures

type
  TTBCtrlButton = record
    hWindow:HWND;
  end;

type
  pfnTTBPainterFunc = procedure(btnctrl:pointer{PTTBCtrl}; hdc:HDC); cdecl;

type
  PTTBCtrl = ^TTBCtrl;
  TTBCtrl = record
    hWnd         :HWND;
    hFrame       :THANDLE;
    nButtonWidth :int;
    nButtonHeight:int;
    nButtonSpace :int;
    nLastHeight  :int;     // fix against recursion in WM_SIZE
    bOrderChanged:bool;    // set by buttons' arrange procedure if the buttons' order was changed
    bFlatButtons,
    bAutoSize,
    bSingleLine  :bool;
    bHardUpdate  :bool;    // clist modern requires to delete buttons to update them

    pButtonList  :PSortedList;
  end;

{
  Sets the custom painting procedure for a toolbar
  wParam = not used
  lParam = lParam = size_t: size of new data block (published from TTBCtrl)
  Usage: SendMessage(hwndToolbar, TTB_SETCUSTOM, 0, (LPARAM)&CustomData);
  Only works on TopToolbars
}
const
  TTB_SETCUSTOMDATASIZE = WM_USER+1;

{$ENDIF}
