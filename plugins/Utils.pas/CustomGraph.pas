unit CustomGraph;

interface

uses windows;

const
  clScrollBar           = TCOLORREF(COLOR_SCROLLBAR           or $80000000);
  clBackground          = TCOLORREF(COLOR_BACKGROUND          or $80000000);
  clActiveCaption       = TCOLORREF(COLOR_ACTIVECAPTION       or $80000000);
  clInactiveCaption     = TCOLORREF(COLOR_INACTIVECAPTION     or $80000000);
  clMenu                = TCOLORREF(COLOR_MENU                or $80000000);
  clWindow              = TCOLORREF(COLOR_WINDOW              or $80000000);
  clWindowFrame         = TCOLORREF(COLOR_WINDOWFRAME         or $80000000);
  clMenuText            = TCOLORREF(COLOR_MENUTEXT            or $80000000);
  clWindowText          = TCOLORREF(COLOR_WINDOWTEXT          or $80000000);
  clCaptionText         = TCOLORREF(COLOR_CAPTIONTEXT         or $80000000);
  clActiveBorder        = TCOLORREF(COLOR_ACTIVEBORDER        or $80000000);
  clInactiveBorder      = TCOLORREF(COLOR_INACTIVEBORDER      or $80000000);
  clAppWorkSpace        = TCOLORREF(COLOR_APPWORKSPACE        or $80000000);
  clHighlight           = TCOLORREF(COLOR_HIGHLIGHT           or $80000000);
  clHighlightText       = TCOLORREF(COLOR_HIGHLIGHTTEXT       or $80000000);
  clBtnFace             = TCOLORREF(COLOR_BTNFACE             or $80000000);
  clBtnShadow           = TCOLORREF(COLOR_BTNSHADOW           or $80000000);
  clGrayText            = TCOLORREF(COLOR_GRAYTEXT            or $80000000);
  clGreyText            = TCOLORREF(COLOR_GRAYTEXT            or $80000000);
  clBtnText             = TCOLORREF(COLOR_BTNTEXT             or $80000000);
  clInactiveCaptionText = TCOLORREF(COLOR_INACTIVECAPTIONTEXT or $80000000);
  clBtnHighlight        = TCOLORREF(COLOR_BTNHIGHLIGHT        or $80000000);
  cl3DDkShadow          = TCOLORREF(COLOR_3DDKSHADOW          or $80000000);
  cl3DLight             = TCOLORREF(COLOR_3DLIGHT             or $80000000);
  clInfoText            = TCOLORREF(COLOR_INFOTEXT            or $80000000);
  clInfoBk              = TCOLORREF(COLOR_INFOBK              or $80000000);

  clBlack   = TCOLORREF( $000000 );
  clMaroon  = TCOLORREF( $000080 );
  clGreen   = TCOLORREF( $008000 );
  clOlive   = TCOLORREF( $008080 );
  clNavy    = TCOLORREF( $800000 );
  clPurple  = TCOLORREF( $800080 );
  clTeal    = TCOLORREF( $808000 );
  clGray    = TCOLORREF( $808080 );
  clGrey    = TCOLORREF( $808080 );
  clSilver  = TCOLORREF( $C0C0C0 );
  clRed     = TCOLORREF( $0000FF );
  clLime    = TCOLORREF( $00FF00 );
  clYellow  = TCOLORREF( $00FFFF );
  clBlue    = TCOLORREF( $FF0000 );
  clFuchsia = TCOLORREF( $FF00FF );
  clAqua    = TCOLORREF( $FFFF00 );
  clLtGray  = TCOLORREF( $C0C0C0 );
  clLtGrey  = TCOLORREF( $C0C0C0 );
  clDkGray  = TCOLORREF( $808080 );
  clDkGrey  = TCOLORREF( $808080 );
  clWhite   = TCOLORREF( $FFFFFF );
  clNone    = TCOLORREF( $1FFFFFFF );
  clDefault = TCOLORREF( $20000000 );

  clMoneyGreen = TCOLORREF( $C0DCC0 );
  clSkyBlue    = TCOLORREF( $F0CAA6 );
  clCream      = TCOLORREF( $F0FBFF );
  clMedGray    = TCOLORREF( $A4A0A0 );
  clMedGrey    = TCOLORREF( $A4A0A0 );
  clOrange     = TCOLORREF( $3399FF );
  clBrown      = TCOLORREF( $505080 );
  clDkBrown    = TCOLORREF( $282840 );

  clGRushHiLight = TCOLORREF( $F3706C );
  clGRushLighten = TCOLORREF( $F1EEDF );
  clGRushLight   = TCOLORREF( $E1CEBF );
  clGRushNormal  = TCOLORREF( $D1BEAF );
  clGRushMedium  = TCOLORREF( $B6BFC6 );
  clGRushDark    = TCOLORREF( $9EACB4 );

function ColorToRGB(Color: TCOLORREF):TCOLORREF;

implementation

function ColorToRGB(Color: TCOLORREF):TCOLORREF;
begin
  if integer(Color) < 0 then
    Result := GetSysColor(Color and $000000FF)
  else
    Result := Color;
end;

end.
