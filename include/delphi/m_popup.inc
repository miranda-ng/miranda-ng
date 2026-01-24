{
===============================================================================
                                Popup plugin
Plugin Name: Popup
Plugin authors: Luca Santarelli aka hrk (hrk@users.sourceforge.net)
                Victor Pavlychko (nullbie@gmail.com)
===============================================================================
The purpose of this plugin is to give developers a common "platform/interface"
to show Popups. It is born from the source code of NewStatusNotify, another
plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any
popup. Write this in the requirements, do whatever you wish ;-)... but tell
them!
===============================================================================
}

{$IFNDEF M_POPUP}
{$DEFINE M_POPUP}

{
NOTE! Since Popup 1.0.1.2 there is a main meun group called "Popups" where I
have put a "Enable/Disable" item. You can add your own "enable/disable" items
by adding these lines before you call MS_CLIST_ADDMAINMENUITEM:
mi.pszPopupName = Translate("Popups");
mi.position = 0; //You don't need it and it's better if you put it to zero.
}

const
  MAX_CONTACTNAME = 2048;
  MAX_SECONDLINE  = 2048;
  MAX_ACTIONTITLE = 64;

const
  APF_RETURN_HWND  = 1;
  APF_CUSTOM_POPUP = 2;
  APF_NO_HISTORY   = 4;
  APF_NO_POPUP     = 8;

  // Popup Action flags
  PAF_ENABLED = $01; // Actions is enabled. You may store one global
                     // action set and toggle some items depending on
                     // popup you are requesting

type
  PPOPUPACTION = ^TPOPUPACTION;
  TPOPUPACTION = record
    lchIcon :HICON;  // Action Icon
    // Action title text. Please use module name as prefix
    // (e.g. "Popup Plus/Dismiss Popup") and don't translate
    lpzTitle:array [0..MAX_ACTIONTITLE-1] of AnsiChar;
    flags   :dword;  // set of PAF_* flags
    wParam  :WPARAM; // wParam for UM_POPUPACTION message
    lParam  :LPARAM; // lParam for UM_POPUPACTION message
  end;

type
  PPOPUPDATA = ^TPOPUPDATA;
  TPOPUPDATA = record
    lchContact      : TMCONTACT;
    lchIcon         : HICON;
    lpszContactName : array [0..MAX_CONTACTNAME-1] of AnsiChar;
    lpszText        : array [0..MAX_SECONDLINE -1] of AnsiChar;
    colorBack       : COLORREF;
    colorText       : COLORREF;
    PluginWindowProc: pointer;
    PluginData      : pointer;
    iSeconds        : int;
  end;

type
  PPOPUPDATAW = ^TPOPUPDATAW;
  TPOPUPDATAW = record
    lchContact      : TMCONTACT;
    lchIcon         : HICON;
    lpwzContactName : array [0..MAX_CONTACTNAME-1] of WideChar;
    lpwzText        : array [0..MAX_SECONDLINE -1] of WideChar;
    colorBack       : COLORREF;
    colorText       : COLORREF;
    PluginWindowProc: pointer;      // must be a window procedure using stdcall
    PluginData      : pointer;
    iSeconds        : int;          // Custom delay time in seconds.
                                    //  -1 means = 'forever", 0 means = 'default time".
    hReserved       : THANDLE;      // Reserved. Must be NULL
    actionCount     : int;          // Amount of passed actions
    lpActions       : PPOPUPACTION; // Popup Actions
  end;

function PUAddPopupW(ppd:PPOPUPDATAW; flags:int):THANDLE; stdcall; external AppDll;

/////////////////////////////////////////////////////////////////////////////////////////

const
{
  UM_FREEPLUGINDATA
  wParam = lParam = 0. Process this message if you have allocated your own memory. (i.e.: POPUPDATA.PluginData != NULL)
}
  UM_FREEPLUGINDATA  = (WM_USER + $200);

{
  UM_DESTROYPOPUP
  wParam = lParam = 0. Send this message when you want to destroy the popup, or use the function below.
}
  UM_DESTROYPOPUP = (WM_USER + $201);

{
  UM_INITPOPUP
  wParam = (WPARAM)(HWND)hPopupWindow (but this is useless, since I'll directly send it to your hPopupWindow
  lParam = 0.
  This message is sent to the Popup when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
  Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
  Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)
}
  UM_INITPOPUP = (WM_USER + $202);

{ UM_POPUPACTION
  Popup Action notification

  wParam and lParam are specified bu plugin.
  wParam = 0 is used buy popup plus internally!
}

  UM_POPUPACTION = WM_USER + $0204;

const
  SM_WARNING      = $01; //Triangle icon.
  SM_NOTIFY       = $02; //Exclamation mark icon.
  SM_ERROR        = $03; //Cross icon.

/////////////////////////////////////////////////////////////////////////////////////////

function PUGetPluginData(popup:HWND):LPARAM; stdcall; external AppDll;

function PURegisterActions(actions:PPOPUPACTION; count:int):int; stdcall; external AppDll;

function PUShowMessage(szMessage:PAnsiChar;options:int):int; stdcall; external AppDll;
function PUShowMessageW(szMessage:PWideChar;options:int):int; stdcall; external AppDll;

{$ENDIF}
