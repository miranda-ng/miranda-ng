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

{$ENDIF}
