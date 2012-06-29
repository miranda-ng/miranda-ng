{$IFNDEF M_TABSRMM}
{$DEFINE M_TABSRMM}

const
{
 show one of the tray menus
 wParam = 0 -> session list
 wParam = 1 -> tray menu
 lParam must be 0
}
  MS_TABMSG_TRAYSUPPORT:PAnsiChar = 'SRMsg_MOD/Show_TrayMenu';

{
  the service which processes globally registered hotkeys
}
  MS_TABMSG_HOTKEYPROCESS:PAnsiChar = 'SRMsg_MOD/ProcessHotkey';

{
 Checks if there is a message window opened
  wParam=(LPARAM)(HANDLE)hContact  - handle of the contact for which the window is searched.
         ignored if lParam is not zero.
  lParam=(LPARAM)(HWND)hwnd - a window handle - SET THIS TO 0 if you want to obtain the window
          handle from the hContact.
}
  MS_MSG_MOD_MESSAGEDIALOGOPENED:PAnsiChar = 'SRMsg_MOD/MessageDialogOpened';

{
  obtain the message window flags
  wParam = hContact - ignored if lParam is given.
  lParam = hwnd
  returns struct MessageWindowData *dat, 0 if no window is found
}
  MS_MSG_MOD_GETWINDOWFLAGS:PAnsiChar = 'SRMsg_MOD/GetWindowFlags';

{$ENDIF}
