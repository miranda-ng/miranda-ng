(*
===============================================================================
                                Popup plugin
Plugin Name: Popup
Plugin author: hrk, Luca Santarelli, hrk@users.sourceforge.net
This file has been created by egodust, Sam, egodust@users.sourceforge.net
===============================================================================

The purpose of this plugin is to give developers a common "platform/interface" to show Popups. It is born from the source code of NewStatusNotify, another plugin I've made.

Remember that users *must* have this plugin enabled, or they won't get any popup. Write this in the requirements, do whatever you wish ;-)... but tell them!
===============================================================================

-- To use this file you need Windows.pas, m_globaldefs.pas (get it from the CVS under the 'inc' module)
-- To include this in the source, use {$include m_popup.h}

*)

{$ifndef M_POPUP_H}
{$define M_POPUP_H}

{$ifdef FPC}
    {$PACKRECORDS C}
    {$MODE Delphi}
{$endif}

const

    MAX_CONTACTNAME = 2048;
    MAX_SECONDLINE  = 2048;

    SM_WARNING      = $01;      //Triangle icon.
    SM_NOTIFY       = $02;      //Exclamation mark icon.

type

    // for info on what this stuff is, see m_popup.h

    PPOPUPDATA = ^TPOPUPDATA;
    TPOPUPDATA = record
        lchContact: MCONTACT;
        lchIcon: THandle;
        lpszContactName: array[0..MAX_CONTACTNAME-1] of Char;
        lpszText: array[0..MAX_SECONDLINE-1] of Char;
        colorBack: COLORREF;
        colorForeText: COLORREF;
        PluginWindowProc: Pointer;      // must be a window procedure using stdcall
        PluginData: Pointer;
    end;

type

    // for info on what this stuff is, see m_popup.h

    PPOPUPDATA = ^TPOPUPDATA;
    TPOPUPDATA = record
        lchContact: MCONTACT;
        lchIcon: THandle;
        lpszContactName: array[0..MAX_CONTACTNAME-1] of Char;
        lpszText: array[0..MAX_SECONDLINE-1] of Char;
        colorBack: COLORREF;
        colorForeText: COLORREF;
        PluginWindowProc: Pointer;      // must be a window procedure using stdcall
        PluginData: Pointer;
        iSeconds: int;      //Custom delay time in seconds. -1 means "forever", 0 means "default time".
        cZero: array[0..15] of Char;      //16 unused bytes which may come useful in the future.
    end;

const

(*
    Creates, adds and shows a popup, given a (valid) POPUPDATA structure pointer.
    wParam = (WPARAM)(*POPUPDATA)PopupDataAddress
    lParam = 0
    Returns: > 0 on success, 0 if creation went bad, -1 if the PopupData contained unacceptable values.
    NOTE: it returns -1 if the PopupData was not valid, if there were already too many popups, if the module was disabled.
    Otherwise, it can return anything else...
*)

    MS_POPUP_ADDPOPUP = 'Popup/AddPopup';

(*
    The same, but with a POPUPDATAEX structure pointer.
    wParam = (WPARAM)(*POPUPDATAEX)PopupDataExAddress
    lParam = 0
*)

    MS_POPUP_ADDPOPUP = 'Popup/AddPopupEx';

(*
    Returns the handle to the contact associated to the specified PopupWindow.
    You will probably need to know this handle inside your WNDPROC. Exampole: you want to open the MessageWindow. :-)
    Call MS_POPUP_GETCONTACT on the hWnd you were given in the WNDPROC.
    wParam = (WPARAM)(HWND)hPopupWindow
    lParam = 0;
    Returns: the HANDLE of the contact. Can return NULL, meaning it's the main contact. -1 means failure.
*)  

    MS_POPUP_GETCONTACT = 'Popup/GetContact';
    
(*
    wParam = hPopupWindow
    lParam = PluginDataAddress;
    Returns: the address of the PLUGINDATA structure. Can return NULL, meaning nothing was given. -1 means failure.
    IMPORTANT NOTE: it doesn't seem to work if you do:
    CallService(..., (LPARAM)aPointerToAStruct);
    and then use that struct.
    Do this, instead:
    aPointerToStruct = CallService(..., (LPARAM)aPointerToAStruct);
    and it will work. Just look at the example I've written above (PopupDlgProc).
*)
    MS_POPUP_GETPLUGINDATA = 'Popup/GetPluginData';
    
(*
    wParam = 0
    lParam = 0
    Returns: 0 if the user has chosen not to have the second line, 1 if he choose to have the second line.
*)
    MS_POPUP_ISSECONDLINESHOWN = 'Popup/IsSecondLineShown';
    
(*
    UM_FREEPLUGINDATA
    wParam = lParam = 0. Process this message if you have allocated your own memory. (i.e.: POPUPDATA.PluginData != NULL)
*)
    UM_FREEPLUGINDATA        = ((*WM_USER*)$400 + $200);
    
(*
    UM_DESTROYPOPUP
    wParam = lParam = 0. Send this message when you want to destroy the popup, or use the function below.
*)
    UM_DESTROYPOPUP          = ((*WM_USER*)$400 + $201);

(*
    UM_INITPOPUP
    wParam = (WPARAM)(HWND)hPopupWindow (but this is useless, since I'll directly send it to your hPopupWindow
    lParam = 0.
    This message is sent to the Popup when its creation has been finished, so POPUPDATA (and thus your PluginData) is reachable.
    Catch it if you needed to catch WM_CREATE or WM_INITDIALOG, which you'll never ever get in your entire popup-life.
    Return value: if you process this message, return 0. If you don't process it, return 0. Do whatever you like ;-)
*)
    UM_INITPOPUP            = ($400(*WM_USER*) + $202);

(*
    wParam = hPopupWindow
    lParam = lpzNewText
    returns: > 0 for success, -1 for failure, 0 if the failure is due to second line not being shown. (but you could call PUIsSecondLineShown() before changing the text...)
    Changes the text displayed in the second line of the popup.
*)
    MS_POPUP_CHANGETEXT         = 'Popup/Changetext';

(*
    This is mainly for developers.
    Shows a warning message in a Popup. It's useful if you need a "MessageBox" like function, but you don't want a modal window (which will interfere with a DialogProcedure. MessageBox steals focus and control, this one not.
    wParam = lpzMessage
    lParam = 0;  Returns: 0 if the popup was shown, -1 in case of failure.
*)
    MS_POPUP_SHOWMESSAGE        = 'Popup/ShowMessage';
    

    (* helper functions, will be inlined on FPC if you have the swithces enabled *)
    
    function PUAddPopup(ppdp: PPOPUPDATA): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := CallService(MS_POPUP_ADDPOPUP, WPARAM(ppdp), 0);
    end;
    
    function PUGetContact(hPopupWindow: THandle): THandle;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := CallService(MS_POPUP_GETCONTACT, WPARAM(hPopupWindow), 0);
    end;

    function PUGetPluginData(hPopupWindow: THandle): Pointer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        dummy: pointer;
    begin
        dummy := nil;
        Int(Result) := CallService(MS_POPUP_GETPLUGINDATA, WPARAM(hPopupWindow), LPARAM(dummy));
    end;

    function PUIsSecondLineShown: BOOL;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Int(Result) := CallService(MS_POPUP_ISSECONDLINESHOWN, 0, 0);
    end;

    function PUDeletePopup(hWndPopup: THandle): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := SendMessage(hWndPopup, UM_DESTROYPOPUP, 0, 0);
    end;
    
    function PUChangeText(hWndPopup: THandle; lpzNewText: PChar): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := CallService(MS_POPUP_CHANGETEXT, WPARAM(hWndPopup), LPARAM(lpzNewText));
    end;
    
    function PUShowMessage(lpzText: PChar; kind: Byte): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := CallService(MS_POPUP_SHOWMESSAGE, WPARAM(lpzText), LPARAM(kind));
    end;

{$endif}

