{popup variables}
const
  ActionList:PPOPUPACTION=nil;
var
  PopTitle,
  PopText:pWideChar;
  PopRequest,
  PopupFile:dword;
  PopupColor:dword;
  PopupFore,
  PopupBack:cardinal;
  PopupPause:cardinal;
  PopupDelay:integer;
  PopupAction:cardinal;
  PopupButtons:cardinal;

  DisablePlugin:integer;
  IsPopup2Present:boolean;
  IsFreeImagePresent:boolean;
var
  hMenuInfo   :THANDLE;
  ssmi,sic,
  plStatusHook:THANDLE;
  PopupPresent:bool;
  onttbhook,
  opthook:THANDLE;
  ttbInfo:THANDLE;