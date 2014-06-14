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
  POPUP_VERSION = $02010003;

const
  MAX_CONTACTNAME = 2048;
  MAX_SECONDLINE  = 2048;
  MAX_ACTIONTITLE = 64;

//  POPUP_USE_SKINNED_BG = $FFFFFFFF;

// Popup Action flags
  PAF_ENABLED = $01; // Actions is enabled. You may store one global
                     // action set and toggle some items depending on
                     // popup you are requesting

type
  PPOPUPACTION = ^TPOPUPACTION;
  TPOPUPACTION = record
    cbSize  :int;    // sizeof(POPUPACTION)
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
    iSeconds        : int;          // Custom delay time in seconds.
                                    // -1 means "forever", 0 means = 'default time".
{ Data prior $02010003 version
    lpzClass        : PAnsiChar;    // Popup class. Used with skinning. See Popup/AddClass for details
    skinBack        : COLORREF; // Background color for colorizable skins
    cZero: array [0..15-SizeOf(PAnsiChar)-SizeOf(COLORREF)] of byte;
}
  // you *MUST* pass APT_NEWDATA flag for services to take care of this data
    hReserved       : THANDLE;      // Reserved. Must be NULL
    actionCount     : int;          // Amount of passed actions
    lpActions       : PPOPUPACTION; // Popup Actions
    icbSize         : int;          // struct size for future
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
{ Data prior $02010003 version
    cZero: array [0..15] of AnsiChar; //16 unused bytes which may come useful in the future.
}
  // you *MUST* pass APT_NEWDATA flag for services to take care of this data
    hReserved       : THANDLE;      // Reserved. Must be NULL
    actionCount     : int;          // Amount of passed actions
    lpActions       : PPOPUPACTION; // Popup Actions
    icbSize         : int;          // struct size for future
  end;

{
  When you call MS_POPUP_ADDPOPUP, my plugin will check if the given POPUPDATA
structure is filled with acceptable values. If not, the data will be rejected
and no popup will be shown.

- lpzText should be given, because it's really bad if a user chooses to have the
second line displayed and it's empty :-) Just write it and let the user choose
if it will be displayed or not.

- PluginWindowProc is a WNDPROC address you have to give me. Why? What? Where?
Calm down 8) My plugin will take care of the creation of the popup, of the
destruction of the popup, of the come into view and the hiding of the popup.
Transparency, animations... all this stuff.
  My plugin will not (as example) open the MessageWindow when you left click on
a popup. Why? Because I don't know if your popup desires to open the
MessageWindow :)))) This means that you need to make a WNDPROC which takes care
of the WM_messages you need.
  For example, WM_COMMAND or WM_CONTEXTMENU or WM_LMOUSEUP or whatever.
  At the end of your WNDPROC remember to "return DefWindowProc(hwnd, msg,
wParam, lParam);" When you process a message that needs a return value (an
example could be WM_CTLCOLORSTATIC, but you don't need to catch it 'cause it's
my plugin's job), simply return the nedeed value. :)
The default WNDPROC does nothing.

- PluginData is a pointer to a void, which means a pointer to anything. You can
make your own structure to store the data you need (example: a status
information, a date, your name, whatever) and give me a pointer to that struct.
You will need to destroy that structure and free the memory when the Popup is
going to be destroyed. You'll know this when you receive a UM_FREEPLUGINDATA.
The name tells it all: free your own plugin data.

Appendix A: Messages my plugin will handle and your WNDPROC will never see.
WM_CREATE, WM_DESTROY, WM_TIMER, WM_ERASEBKGND
WM_CTLCOLOR* [whatever it may be: WM_CTLCOLORDLG, WM_CTLCOLORSTATIC...]
WM_PAINT, WM_PRINT, WM_PRINTCLIENT
}

const
{
  Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.
  wParam = (WPARAM)(*POPUPDATA)PopupDataAddress
  lParam = 0
  Returns: > 0 on success, 0 if creation went bad, -1 if the PopupData contained unacceptable values.
  NOTE: it returns -1 if the PopupData was not valid, if there were already too many popups, if the module was disabled.
  Otherwise, it can return anything else...

Popup Plus 2.0.4.0+
You may pass additional creation flags via lParam:
  APF_RETURN_HWND ....... function returns handle to newly created popup window (however this calls are a bit slower)
  APF_CUSTOM_POPUP ...... new popup is created in hidden state and doesn't obey to popup queue rules.
              you may control it via UM_* messages and custom window procedure
  APF_NO_HISTORY ........ do not log this popup in popup history (useful for previews)
  APF_NO_POPUP .......... do not show popup. this is useful if you want popup yo be stored in history only
  APF_NEWDATA ........... use new version of POPUPDATAEX/POPUPDATAW structs
}
  APF_RETURN_HWND  = 1;
  APF_CUSTOM_POPUP = 2;
  APF_NO_HISTORY   = 4;
  APF_NO_POPUP     = 8;
  APF_NEWDATA      = $10;

  MS_POPUP_ADDPOPUPW:PAnsiChar = 'Popup/AddPopupW';
  MS_POPUP_ADDPOPUP :PAnsiChar = 'Popup/AddPopupEx';

{
  Returns the handle to the contact associated to the specified PopupWindow.
  You will probably need to know this handle inside your WNDPROC. Exampole: you want to open the MessageWindow. :-)
  Call MS_POPUP_GETCONTACT on the hWnd you were given in the WNDPROC.
  wParam = (WPARAM)(HWND)hPopupWindow
  lParam = 0;
  Returns: the HANDLE of the contact. Can return NULL, meaning it's the main contact. -1 means failure.
}
  MS_POPUP_GETCONTACT:PAnsiChar = 'Popup/GetContact';

{
  wParam = hPopupWindow
  lParam = PluginDataAddress;
  Returns: the address of the PLUGINDATA structure. Can return NULL, meaning nothing was given. -1 means failure.
  IMPORTANT NOTE: it doesn't seem to work if you do:
  CallService(..., (LPARAM)aPointerToAStruct);
  and then use that struct.
  Do this, instead:
  aPointerToStruct = CallService(..., (LPARAM)aPointerToAStruct);
  and it will work. Just look at the example I've written above (PopupDlgProc).
}
  MS_POPUP_GETPLUGINDATA:PAnsiChar = 'Popup/GetPluginData';

{
  Requests an action or an answer from Popup module.
  wParam = (WPARAM)wpQuery
  returns 0 on success, -1 on error, 1 on stupid calls ;-)
}
  PUQS_ENABLEPOPUPS  = 1; // returns 0 if state was changed, 1 if state wasn't changed
  PUQS_DISABLEPOPUPS = 2; // " "
  PUQS_GETSTATUS     = 3; // Returns 1 if popups are enabled, 0 if popups are disabled.

  MS_POPUP_QUERY:PAnsiChar = 'Popup/Query';

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

{
  wParam = hPopupWindow
  lParam = lpzNewText
  returns: > 0 for success, -1 for failure, 0 if the failure is due to second line not being shown.
  Changes the text displayed in the second line of the popup.
}
  MS_POPUP_CHANGETEXTW:PAnsiChar = 'Popup/ChangetextW';

{
  wParam = (WPARAM)(HWND)hPopupWindow
  lParam = (LPARAM)(POPUPDATAEX*)newData
  Changes the entire popup
}
  MS_POPUP_CHANGEW:PAnsiChar = 'Popup/ChangeW';

{
 UM_CHANGEPOPUP
  This message is triggered by Change/ChangeText services. You also may post it directly :)

  wParam = Modification type
  lParam = value of type defined by wParam
}

//  CPT_TEXT   = 1; // lParam = (AnsiChar *)text
  CPT_TEXTW  = 2; // lParam = (WCHAR *)text
//  CPT_TITLE  = 3; // lParam = (AnsiChar *)title
  CPT_TITLEW = 4; // lParam = (WCHAR *)title
  CPT_DATAW  = 7; // lParam = (POPUPDATAW *)data
  CPT_DATA2  = 8; // lParam = (POPUPDATA2 *)data

  UM_CHANGEPOPUP = WM_USER + $0203;

{ UM_POPUPACTION
  Popup Action notification

  wParam and lParam are specified bu plugin.
  wParam = 0 is used buy popup plus internally!
}

  UM_POPUPACTION = WM_USER + $0204;

{ UM_POPUPMODIFYACTIONICON
  Modify Popup Action Icon

  wParam = (WPARAM)(LPPOPUPACTIONID)&actionId
  lParam = (LPARAM)(HICON)hIcon
}

type
  PPOPUPACTIONID = ^TPOPUPACTIONID;
  TPOPUPACTIONID = record
    wParam:WPARAM;
    lParam:LPARAM;
  end;

const
  UM_POPUPMODIFYACTIONICON = WM_USER + $0205;

const
  SM_WARNING      = $01; //Triangle icon.
  SM_NOTIFY       = $02; //Exclamation mark icon.
  SM_ERROR        = $03; //Cross icon.
{
  This is mainly for developers.
  Shows a warning message in a Popup. It's useful if you need a = 'MessageBox" like function,
  but you don't want a modal window (which will interfere with a DialogProcedure. MessageBox
  steals focus and control, this one not.
  wParam = lpzMessage
  lParam = SM_* flag
  Returns: 0 if the popup was shown, -1 in case of failure.
}
  MS_POPUP_SHOWMESSAGE :PAnsiChar = 'Popup/ShowMessage';
  MS_POPUP_SHOWMESSAGEW:PAnsiChar = 'Popup/ShowMessageW';

{ Popup/Filter
Filters Popups out

wParam = (HANDLE)hContact
lParam = (void*)pWindowProc;

returns: 0 = Popup allowed, 1 = Popup filtered out
}

  ME_POPUP_FILTER:PAnsiChar = 'Popup/Filter';

{ Popup/RegisterActions
Registers your action in popup action list

wParam = (WPARAM)(LPPOPUPACTION)actions
lParam = (LPARAM)actionCount

Returns: 0 if the popup was shown, -1 in case of failure.
}
  MS_POPUP_REGISTERACTIONS:PAnsiChar = 'Popup/RegisterActions';

//------------- Class API ----------------//

type
  TPOPUPCLASS = record
    cbSize          :int;
    flags           :int;
    pszName         :PAnsiChar;
    szDescription   :TChar;
    hIcon           :HICON;
    colorBack       :TCOLORREF;
    colorText       :TCOLORREF;
    PluginWindowProc:pointer;
    iSeconds        :int;
    lParam          :LPARAM; //APF_RETURN_HWND, APF_CUSTOM_POPUP  ... as above
  end;

const
  PCF_UNICODE = $0001;

// wParam = 0
// lParam = (POPUPCLASS *)&pc
  MS_POPUP_REGISTERCLASS   = 'Popup/RegisterClass';
  MS_POPUP_UNREGISTERCLASS = 'Popup/UnregisterClass';

type
  TPOPUPDATACLASS = record
	  cbSize      :int;
  	pszClassName:PAnsiChar;
		szTitle     :TChar;
		szText      :TChar;
	  PluginData  :pointer;
	  hContact    :TMCONTACT;
  end;

const
// wParam = 0
// lParam = (POPUPDATACLASS *)&pdc
  MS_POPUP_ADDPOPUPCLASS = 'Popup/AddPopupClass';

(* OLD
{
  Each skinned popup (e.g. with colorBack == POPUP_USE_SKINNED_BG) should have
  class set. Then you can choose separate skin for each class (for example, you
  can create separate class for your plugin and use it for all ypu popups. User
  would became able to choose skin for your popups independently from others)

  You have to register popup class before using it. To do so call
  "Popup/AddClass" with lParam = (LPARAM)(const AnsiChar * )popUpClassName.

  All class names are translated (via Translate()) before being added to list.
  You should use english names for them.

  There are three predefined classes and one for backward compatability.

  Note that you can add clases after popup wal loaded, e.g. you shoul intercept
  ME_SYSTEM_MODULESLOADED event
}
  MS_POPUP_ADDCLASS = 'Popup/AddClass';

  POPUP_CLASS_DEFAULT = 'Default';
  POPUP_CLASS_WARNING = 'Warning';
  POPUP_CLASS_NOTIFY  = 'Notify';
  POPUP_CLASS_OLDAPI  = 'Popup 1.0.1.x compatability'; // for internal purposes

const
  NFOPT_POPUP2_BACKCOLOR    = 'Popup2/BackColor';
  NFOPT_POPUP2_TEXTCOLOR    = 'Popup2/TextColor';
  NFOPT_POPUP2_TIMEOUT      = 'Popup2/Timeout';
  NFOPT_POPUP2_LCLICKSVC    = 'Popup2/LClickSvc';
  NFOPT_POPUP2_LCLICKCOOKIE = 'Popup2/LClickCookie';
  NFOPT_POPUP2_RCLICKSVC    = 'Popup2/RClickSvc';
  NFOPT_POPUP2_RCLICKCOOKIE = 'Popup2/RClickCookie';
  NFOPT_POPUP2_STATUSMODE   = 'Popup2/StatusMode';
  NFOPT_POPUP2_PLUGINDATA   = 'Popup2/PluginData';
  NFOPT_POPUP2_WNDPROC      = 'Popup2/WndProc';

  NFOPT_POPUP2_BACKCOLOR_S  = 'Popup2/BackColor/Save';
  NFOPT_POPUP2_TEXTCOLOR_S  = 'Popup2/TextColor/Save';
  NFOPT_POPUP2_TIMEOUT_S    = 'Popup2/Timeout/Save';

  MS_POPUP2_SHOW   = 'Popup2/Show';
  MS_POPUP2_UPDATE = 'Popup2/Update';
  MS_POPUP2_REMOVE = 'Popup2/Remove';
*)
///////////////////////////////////////////////////////////////
// Few notes about new popup api
// ------------------------------
// When you call any ADD service, Popup Plus creates local
// copy of POPUPDATA2 to store the data. Each time you call
// CHANGE service this data is updated. You can use the
// MS_POPUP_GETDATA2 service to retrieve Popups's copy of
// this data, however you MUST NOT chahge that.

// unicode or ansi mode
const
  PU2_ANSI         = 00;
  PU2_UNICODE      = 01;
  PU2_CUSTOM_POPUP = 02;

type
  PPOPUPDATA2 = ^TPOPUPDATA2;
  TPOPUPDATA2 = record
  // general
    cbSize:int;
    flags:dword;

  // miranda bindings
    lchContact:TMCONTACT;
    lchEvent  :THANDLE;

  // style
    colorBack:TCOLORREF;
    colorText:TCOLORREF;
    lchIcon:HICON;
    hbmAvatar:HBITMAP;
    pzTitle:TCHAR;
    pzText:TCHAR;
    lpzSkin:PAnsiChar;

  // time and timeout
    iSeconds:int;
    dwTimestamp:dword;

  // plugin bindings
    PluginWindowProc:TWNDPROC;
    PluginData:pointer;

  // popup actions
    actionCount:int;
    lpActions:PPOPUPACTION; // Ansi or unicode
    lchNotification:THANDLE;
  end;

const
// Creates new popup
//    wParam = (WPARAM)(LPPOPUPDATA2)&ppd2
//    lParam = (LPARAM)(combination of APF_* flags)
//    returns: window handle (if requested) of NULL on success, -1 on failure.
  MS_POPUP_ADDPOPUP2:PAnsiChar = 'Popup/AddPopup2';

// Update an popup
//    wParam = (WPARAM)(HWND)hwndPopup
//    lParam = (LPARAM)(LPPOPUPDATA2)&ppd2
//    returns: zero on success, -1 on failure.
  MS_POPUP_CHANGEPOPUP2:PAnsiChar = 'Popup/ChangePopup2';

{$ENDIF}
