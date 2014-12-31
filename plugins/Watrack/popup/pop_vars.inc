{popup variables}
const
  ActionList:PPOPUPACTION=nil;
var
  PopTitle,
  PopText:pWideChar;
  PopRequest,
  PopUpFile:dword;
  PopUpColor:dword;
  PopUpFore,
  PopUpBack:cardinal;
  PopUpPause:cardinal;
  PopUpDelay:integer;
  PopUpAction:cardinal;
  PopUpButtons:cardinal;

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