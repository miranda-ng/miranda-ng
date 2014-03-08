{
 Tipper API
 note: Tipper is internally unicode and requires unicows.dll to function
 correctly on 95/98/ME so you'll find a lot of wchar_t stuff in here

 translation function type
 use hContact, module and setting to read your db value(s) and put the resulting
 string into buff return buff if the translation was successful, or return 0 for failure
}
{$IFNDEF M_TIPPER}
{$DEFINE M_TIPPER}
type
  TranslateFunc = function (hContact:TMCONTACT;module:PAnsiChar;setting_or_prefix:PAnsiChar;
       buff:PWideChar;bufflen:int):PWideChar; cdecl;

type
  PDBVTranslation = ^TDBVTranslation;
  TDBVTranslation = record
    tfunc:TranslateFunc; // address of your translation function (see typedef above)
    name :PWideChar;     // make sure this is unique, and DO NOT translate it
    id   :dword;         // will be overwritten by Tipper - do not use
  end;

{
  add a translation to tipper
  wParam not used
  lParam = pointer to PDBVTranslation
}
const
  MS_TIPPER_ADDTRANSLATION:PAnsiChar = 'Tipper/AddTranslation';

{
  Show tooltip
  wParam - optional (PAnsiChar) text for text-only tips
  lParam - (PCLCINFOTIP) infoTip
}
  MS_TIPPER_SHOWTIP:PAnsiChar = 'mToolTip/ShowTip';

{
  unicode extension to the basic functionality
  wParam - optional (PWideChar) text for text-only tips
  lParam - (PCLCINFOTIP) infoTip
}
  MS_TIPPER_SHOWTIPW:PAnsiChar = 'mToolTip/ShowTipW';

{
  wParam=lParam=0;
}
  MS_TIPPER_HIDETIP:PAnsiChar = 'mToolTip/HideTip';

{$ENDIF}
