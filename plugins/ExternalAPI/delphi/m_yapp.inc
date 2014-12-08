{$IFNDEF M_YAPP}
{$DEFINE M_YAPP}

////////////////////////////////////////////////
// YAPP API
///////////////////////////////////////////////

type
  TPopupData = record
    cbSize    :int;
    flags     :int;      // OR of PDF_* flags below
    hContact  :THANDLE;
    hIcon     :HICON;
    szTitle   :TChar;
    szText    :TChar;
    colorBack :COLORREF; // if colorBack and colorText are equal, defaults will be used
    colorText :COLORREF;
    windowProc:TWNDPROC; // optional custom window procedure
    timeout   :int;      // -1 == infinite, 0 == default, otherwise timeout in seconds
    opaque    :pointer;
  end;

const
  PDF_UNICODE = $0001;

// windowProc messages
  PM_INIT      = WM_USER+$0202; // message sent to your windowProc after the window has been initialized
  PM_DIENOTIFY = WM_USER+$0200; // message sent to your windowProc just before the window is destroyed (can be used e.g. to free your opaque data)
  PM_DESTROY   = WM_USER+$0201; // send to the popup hWnd (use PostMessage generally, or SendMessage inside your windowProc) to kill it

// Show a popup
//wParam = &PopupData
//lParam = 0
// returns -1 if popups disabled
  MS_YAPP_SHOWPOPUP:PAnsiChar = 'YAPP/ShowPopup';

// get hContact from PopupData
// wParam = hWnd
// lParam = 0
// return hContact
  MS_YAPP_GETCONTACT:PAnsiChar = 'YAPP/GetContact';

// get opaque from PopupData
// wParam = hWnd
// lParam = 0
// returns opaque
  MS_YAPP_GETOPAQUE:PAnsiChar = 'YAPP/GetOpaque';

//------------- Class API ----------------//

type
  TPopupClass = record
    cbSize       :int;
    flags        :int;
    pszName      :PAnsiChar;
    szDescription:TChar;
    hIcon        :HICON;
    colorBack    :COLORREF;
    colorText    :COLORREF;
    windowProc   :TWNDPROC;
    timeout      :int;
  end;

const
  PCF_UNICODE = $0001;

// wParam = 0
// lParam = (POPUPCLASS *)&pc
  MS_POPUP_REGISTERCLASS:PAnsiChar = 'PopUp/RegisterClass';

type
  TPopupClassInstance = record
    cbSize      :int;
    pszClassName:PAnsiChar;
    szTitle     :TChar;
    szText      :TChar;
    opaque      :pointer;
    hContact    :THANDLE;
  end;

const
// wParam = 0
// lParam = (POPUPDATACLASS *)&pdc
  MS_POPUP_ADDPOPUPCLASS:PAnsiChar = 'PopUp/AddPopupClass';

{$ENDIF}
