// Name:   KOL Addon - Visual XP Styles
// Rev.:   1.99 + KOL 3.00.A
// Date:   02 oct 2010
// Author: MTsv DN
// Thanks: mdw, Vladimir Kladov

{$IFDEF _FPC}
const
  clGrey = TColor($808080);
  clLtGrey = TColor($C0C0C0);
  clDkGrey = TColor($808080);
{$ENDIF}

procedure ConvertBitmap2Grayscale(var Bmp: PBitmap);
type
 TRGBArray = array[0..32767] of TRGBTriple;
 PRGBArray = ^TRGBArray;
var
 x, y, Gray: Integer;
 Row: PRGBArray;
 R, G, B : Byte;
 TrColor : Integer;
begin
 Bmp.PixelFormat := pf24bit;
 TrColor := Bmp.Pixels[Bmp.Width - 1, 0];
 for y := 0 to Bmp.Height - 1 do
  begin
   Row := Bmp.ScanLine[y];
   for x := 0 to Bmp.Width - 1 do
    begin
     R := LoByte(LoWord(TrColor));
     G := HiByte(LoWord(TrColor));
     B := LoByte(HiWord(TrColor));
     if (Row[x].rgbtRed = R) and
        (Row[x].rgbtGreen = G) and
        (Row[x].rgbtBlue = B) then continue;
     Gray             := (Row[x].rgbtRed + Row[x].rgbtGreen + Row[x].rgbtBlue) div 3;
     Row[x].rgbtRed   := Gray;
     Row[x].rgbtGreen := Gray;
     Row[x].rgbtBlue  := Gray;
    end;
  end;
end;
//********************* Creating font on Sender font base ********************//
function CreateNewFont(Sender : PControl): HFont;
const
 CLEARTYPE_QUALITY  = 5;
var
 fnWeight : Integer;
 fnItalic, fnUnderline, fnStrikeOut,
 fnQuality, fnPitch : DWORD;
begin
 // Font style
 if Sender.Font.FontStyle = [fsBold] then fnWeight := 700 else fnWeight := 0;
 if Sender.Font.FontStyle = [fsItalic] then fnItalic := DWORD(TRUE) else fnItalic := DWORD(FALSE);
 if Sender.Font.FontStyle = [fsUnderline] then fnUnderline := DWORD(TRUE) else fnUnderline := DWORD(FALSE);
 if Sender.Font.FontStyle = [fsStrikeOut] then fnStrikeOut := DWORD(TRUE) else fnStrikeOut := DWORD(FALSE);

 // Font quality
 case Sender.Font.FontQuality of
  fqAntialiased:    fnQuality := DWORD(ANTIALIASED_QUALITY);
  {$IFDEF AUTO_REPLACE_CLEARTYPE}
  fqClearType:      fnQuality := DWORD(CLEARTYPE_QUALITY);
  {$ELSE}
  fqClearType:      fnQuality := DWORD(ANTIALIASED_QUALITY);
  {$ENDIF}
  fqDraft:          fnQuality := DWORD(DRAFT_QUALITY);
  fqNonAntialiased: fnQuality := DWORD(NONANTIALIASED_QUALITY);
  fqProof:          fnQuality := DWORD(PROOF_QUALITY);
  {fqDefault:} else fnQuality := DWORD(DEFAULT_QUALITY);
 end;

 // Font pitch
 case Sender.Font.FontPitch of
  fpFixed:     fnPitch := DWORD(FIXED_PITCH);
  fpVariable:  fnPitch := DWORD(VARIABLE_PITCH);
  {fpDefault:} else fnPitch := DWORD(DEFAULT_PITCH);
 end;

 Result := CreateFont(Sender.Font.FontHeight,
                      Sender.Font.FontWidth,
                      0,
                      Sender.Font.FontOrientation,
                      fnWeight,
                      fnItalic,
                      fnUnderline,
                      fnStrikeOut,
                      Sender.Font.FontCharset,
                      OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS,
                      fnQuality,
                      fnPitch,
                      PKOLChar(Sender.Font.FontName));
end;
//***************************** Initializing themes **************************//
function InitThemes : boolean;
begin
 Result := false;
 ThemeLibrary := LoadLibrary(themelib);
 if ThemeLibrary > 0 then
  begin
   OpenThemeData := GetProcAddress(ThemeLibrary, 'OpenThemeData');
   DrawThemeBackground := GetProcAddress(ThemeLibrary, 'DrawThemeBackground');
   IsThemeBackgroundPartiallyTransparent := GetProcAddress(ThemeLibrary, 'IsThemeBackgroundPartiallyTransparent');
   DrawThemeParentBackground := GetProcAddress(ThemeLibrary, 'DrawThemeParentBackground');
   DrawThemeText := GetProcAddress(ThemeLibrary, 'DrawThemeText');
   CloseThemeData := GetProcAddress(ThemeLibrary, 'CloseThemeData');
   IsThemeActive := GetProcAddress(ThemeLibrary, 'IsThemeActive');
   IsAppThemed := GetProcAddress(ThemeLibrary, 'IsAppThemed');
   GetThemeColor := GetProcAddress(ThemeLibrary, 'GetThemeColor');
   Result := true;
  end;
end;
//***************************** Deinitializing themes ************************//
procedure DeinitThemes;
begin
 if ThemeLibrary > 0 then
  begin
   FreeLibrary(ThemeLibrary);
   ThemeLibrary := 0;
   OpenThemeData := nil;
   DrawThemeBackground := nil;
   IsThemeBackgroundPartiallyTransparent := nil;
   DrawThemeParentBackground := nil;
   CloseThemeData := nil;
   IsAppThemed := nil;
   IsThemeActive := nil;
   GetThemeColor := nil;
  end;
end;
//****************************** Checking themes *****************************//
procedure CheckThemes;
// Check Manifest file or resource
 function IsManifestFilePresent : boolean;
 begin
  Result := false;
  if FileExists(ParamStr(0) + '.manifest') then //dufa. в случае с DLL ExePath вернет путь до нее, а не до EXE
   begin
    Result := true;
    exit;
   end;
  if FindResource(hInstance, MAKEINTRESOURCE(1), MakeIntResource(24)) <> 0 then
   Result := true;
 end;
// Check activity themes
 function UseThemes: Boolean;
 begin
  if (ThemeLibrary > 0) then Result := IsThemeActive
   else Result := False;
 end;
begin
 AppTheming := false;
 if IsManifestFilePresent then
  if InitThemes then
   begin
    if UseThemes then
     AppTheming := true;
    DeinitThemes;
   end;
end;
//****************************** Drawing Splitter ****************************//
procedure WndSplitterXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
const
 Bit  : Word = $FF;
var
 B, Brush : HBRUSH;
 fDC : HDC;
 Bmp : HBITMAP;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndSplitterXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;
  
 // Draw back layer
 Brush := CreateSolidBrush(Color2RGB(Sender.fParent.Color));
 fDC := SelectObject(DC, Brush);
 FillRect(DC, Sender.ClientRect, Brush);
 SelectObject(DC, fDC);
 DeleteObject(Brush);

 // Creating brush and pen
 if  {$IFDEF USE_FLAGS} G4_Pressed in Sender.fFlagsG4
     {$ELSE} Sender.fPressed {$ENDIF} then
 begin
     Bmp := CreateBitmap(2, 2, 1, 1, @Bit);
     B := CreatePatternBrush(Bmp);
     fDC := SelectObject(DC, B);
     // Drawing splitter
     PatBlt   (DC, 0, 0, Sender.Width, Sender.Height, PATINVERT);
     // Destroying  brush and pen
     SelectObject(DC, fDC);
     DeleteObject(B);
     DeleteObject(Bmp);
 end;
end;
//*************************** Drawing TabControl Page ************************//
procedure WndTabXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 hThemes : THandle;
 Color : COLORREF;
 Brush : HBRUSH;
 fDC : HDC;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndTabXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;
   hThemes := OpenThemeData(Sender.fHandle, 'TAB');
   if hThemes <> 0 then
    begin
     GetThemeColor(hThemes, 10, 0, 3805, Color);
     Sender.Color := Color2RGB(Color);
     Brush := CreateSolidBrush(Color2RGB(Color));
     fDC := SelectObject(DC, Brush);
     FillRect(DC, Sender.ClientRect, Brush);
     SelectObject(DC, fDC);
     DeleteObject(Brush);
     CloseThemeData(hThemes);
    end;
end;
//*************************** Drawing Panel control **************************//
procedure WndPanelXPResize( Dummy : Pointer; Sender: PObj );
var
 R : TRect;
begin
 R := PControl(Sender).ClientRect;
 InvalidateRect(PControl(Sender).fHandle, @R, False);
end;

procedure WndPanelXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 RClient, RText : TRect;
 LPos : DWORD;
 S : KOLString;
 F : HFONT;
 fDC1, fDC2 : HDC;
 hThemes : THandle;
 TxtColor, Color : COLORREF;
 Brush : HBRUSH;
 Pen : HPEN;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndPanelXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;

 // Getting rects
 RClient := Sender.ClientRect;
 // Getting text and text flags
 S := Sender.fCaption;
 LPos := 0;
 if S <> '' then
  begin
   case Sender.fVerticalAlign of
    vaTop:    LPos := DT_TOP;
    vaCenter: LPos := DT_VCENTER;
    vaBottom: LPos := DT_BOTTOM;
   end;
   case Sender.fTextAlign of
    taLeft:    LPos := LPos or DT_LEFT;
    taCenter:  LPos := LPos or DT_CENTER;
    taRight:   LPos := LPos or DT_RIGHT;
   end;
  end;

 // Draw back layer
 if (Sender.EdgeStyle = esTransparent) or
    ({$IFDEF USE_FLAGS} (G2_Transparent in Sender.fFlagsG2)
     {$ELSE} Sender.fTransparent {$ENDIF}) then else
  begin
   Brush := CreateSolidBrush(Color2RGB(Sender.fParent.Color));
   fDC1 := SelectObject(DC, Brush);
   FillRect(DC, RClient, Brush);

   case Sender.EdgeStyle of
   esRaised, esLowered:
    begin
     Sender.fStyle.Value := Sender.fStyle.Value and (not SS_SUNKEN) and (not WS_DLGFRAME);
     Sender.fExStyle := Sender.fExStyle and (not WS_EX_STATICEDGE) or WS_EX_WINDOWEDGE;

     Pen := CreatePen(PS_SOLID, 1, Color2RGB(clLtGrey));
     fDC2 := SelectObject(DC, Pen);
     RoundRect(DC, RClient.Left, RClient.Top,
                   RClient.Right, RClient.Bottom, 5, 5);
     SelectObject(DC, fDC2);
     DeleteObject(Pen);
    end;
   end;

   SelectObject(DC, fDC1);
   DeleteObject(Brush);
  end;

 if S <> '' then
  begin
   hThemes := OpenThemeData(Sender.fHandle, 'button');
   Color := Sender.Font.Color;
   if  hThemes <> 0 then
   begin
       {$IFDEF USE_FLAGS}
       if  (F3_Disabled in Sender.fStyle.f3_Style) then
       {$ELSE}
       if  not Sender.fEnabled then
       {$ENDIF}
           GetThemeColor(hThemes, 1, 4, 3803, Color);
       CloseThemeData(hThemes);
   end;
   RText := MakeRect(2, 2, Sender.Width-2, Sender.Height-2);

   // Create font
   F := CreateNewFont(Sender);
   fDC1 := SelectObject(DC, F);
   // Draw text
   SetBkMode(DC, TRANSPARENT);
   TxtColor := SetTextColor(DC, Color2RGB(Color));
   DrawText(DC, PKOLChar(S), Length(S), RText, LPos or DT_SINGLELINE);
   // Backup color
   SetTextColor(DC, Color2RGB(TxtColor));
   SetBkMode(DC, OPAQUE);
   // Destroying font
   SelectObject(DC, fDC1);
   DeleteObject(F);
  end;
end;
//************************** Drawing GroupBox control ************************//
procedure WndGroupBoxXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 hThemes : THandle;
 RClient, RText, RClipMain, RClipLeft, RClipRight : TRect;
 LPos, fState : DWORD;
 S : KOLWideString;
 F : HFONT;
 fDC : HDC;
 TxtColor, Color : COLORREF;
 TextWidth, TextHeight : Integer;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndGroupBoxXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;
  
 // Getting text and text flags
 LPos := 0;
 case Sender.fVerticalAlign of
  vaTop:    LPos := DT_TOP;
  vaCenter: LPos := DT_VCENTER;
  vaBottom: LPos := DT_BOTTOM;
 end;
 case Sender.fTextAlign of
  taLeft:    LPos := LPos or DT_LEFT;
  taCenter:  LPos := LPos or DT_CENTER;
  taRight:   LPos := LPos or DT_RIGHT;
 end;
 S := KOLWideString( Sender.fCaption );

 // Getting rects
 TextWidth := Sender.Canvas.WTextWidth(S);
 TextHeight := Sender.Canvas.WTextHeight(S);

 RClient := Sender.ClientRect;
 RClient.Left := RClient.Left - Sender.MarginLeft;
 RClient.Top := RClient.Top - Sender.MarginTop + (TextHeight div 2);
 RClient.Right := RClient.Right + Sender.MarginRight;
 RClient.Bottom := RClient.Bottom + Sender.MarginBottom;

 case Sender.fTextAlign of
  taCenter:
   begin
    RText := MakeRect(((RClient.Right div 2) - (TextWidth div 2)) - 2,
                      RClient.Top-6,
                      ((RClient.Right div 2) + (TextWidth div 2)) + 2,
                      TextHeight + (RClient.Top-6));
    RClipLeft := MakeRect(RClient.Left,
                          RClient.Top,
                          ((RClient.Right div 2) - (TextWidth div 2)) - 2,
                          TextHeight + (RClient.Top-6));
    RClipRight := MakeRect(((RClient.Right div 2) + (TextWidth div 2)) + 2,
                           RClient.Top-6,
                           RClient.Right,
                           TextHeight + (RClient.Top-6));
   end;
  taRight:
   begin
    RText := MakeRect((RClient.Right-4) - TextWidth,
                      RClient.Top-6,
                      RClient.Right-4,
                      TextHeight + (RClient.Top-6));
    RClipLeft := MakeRect(RClient.Left,
                          RClient.Top,
                          (RClient.Right-4) - TextWidth,
                          TextHeight + (RClient.Top-6));
    RClipRight := MakeRect(RClient.Right-4,
                           RClient.Top-6,
                           RClient.Right,
                           TextHeight + (RClient.Top-6));
   end;
 else
  RText := MakeRect(RClient.Left+4,
                    RClient.Top-6,
                    TextWidth + RClient.Left+4,
                    TextHeight + RClient.Top-6);
  RClipLeft := MakeRect(RClient.Left,
                        RClient.Top,
                        RClient.Left+4,
                        TextHeight + RClient.Top-6);
  RClipRight := MakeRect(TextWidth + RClient.Left+4,
                         RClient.Top-6,
                         RClient.Right,
                         TextHeight + RClient.Top-6);
 end;
 RClipMain := MakeRect(RClient.Left,
                       TextHeight + RClient.Top-6,
                       RClient.Right,
                       RClient.Bottom);
 // Open themes
 hThemes := OpenThemeData(Sender.fHandle, 'button');
 if hThemes <> 0 then
  begin
   Sender.Color := Sender.fParent.fColor;
   {$IFDEF USE_FLAGS}
   if  not (F3_Disabled in Sender.fStyle.f3_Style) then
   {$ELSE}
   if  Sender.fEnabled then
   {$ENDIF}
       fState := 1 else fState := 2;
   // Drawing GroupBox rect "step by step"
   DrawThemeBackground(hThemes, DC, 4{BP_GROUPBOX}, fState{GBS_XXXXXX},  RClient, @RClipMain);
   DrawThemeBackground(hThemes, DC, 4{BP_GROUPBOX}, fState{GBS_XXXXXX},  RClient, @RClipLeft);
   DrawThemeBackground(hThemes, DC, 4{BP_GROUPBOX}, fState{GBS_XXXXXX},  RClient, @RClipRight);
   // Drawing GroupBox text
   {$IFDEF USE_FLAGS}
   if  F3_Disabled in Sender.fStyle.f3_Style then
   {$ELSE}
   if  not Sender.fEnabled then
   {$ENDIF}
       GetThemeColor(hThemes, {WP_CAPTION} 1, {CS_DISABLED} 3, 3803, Color)
    else GetThemeColor(hThemes, {WP_CAPTION} 1, {CS_ACTIVE} 1, 3803, Color);
   // Close themes
   CloseThemeData(hThemes);

   // Create font
   F := CreateNewFont(Sender);
   fDC := SelectObject(DC, F);
   // Draw text
   SetBkMode(DC, TRANSPARENT);
   TxtColor := SetTextColor(DC, Color2RGB(Color));
   DrawTextW(DC, PWideChar(S), Length(S), RText, LPos or DT_SINGLELINE);
   // Backup color
   SetTextColor(DC, Color2RGB(TxtColor));
   SetBkMode(DC, OPAQUE);
   // Destroying font
   SelectObject(DC, fDC);
   DeleteObject(F);
  end;
end;
//************************* Drawing CheckBox control *************************//
procedure WndCheckBoxXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 hThemes : THandle;
 RClient, RCheck, RText : TRect;
 fState : DWORD;
 W, H : Integer;
 S : KOLString;
 F : HFONT;
 fDC : HDC;
 Color : COLORREF;
 TxtColor : COLORREF;
 Brush : HBRUSH;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndCheckBoxXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;
  
 // Getting metrics
 W := GetSystemMetrics( SM_CXMENUCHECK );
 H := GetSystemMetrics( SM_CYMENUCHECK );
 // Getting caption
 S := Sender.fCaption;
 // Getting rects
 RClient := Sender.ClientRect;
 RCheck := RClient;
 RCheck.Right := RCheck.Left + W;
 if  {$IFDEF USE_FLAGS} G1_WordWrap in Sender.fFlagsG1
     {$ELSE}            Sender.fWordWrap {$ENDIF} then
     RCheck.Top := RCheck.Top + Sender.Border
 else
     RCheck.Top := RCheck.Top + (RCheck.Bottom - RCheck.Top - H) div 2;
 RCheck.Bottom := RCheck.Top + H;
 RText := MakeRect(RCheck.Right + Sender.fMargin, RCheck.Top,
                   RClient.Right, RCheck.Bottom);
 // Getting state
 fState := 1; {CBS_UNCHECKEDNORMAL}
 {$IFDEF USE_FLAGS}
 if  F3_Disabled in Sender.fStyle.f3_Style then
 {$ELSE}
 if  not Sender.fEnabled then
 {$ENDIF}
     fState := 4 {CBS_UNCHECKEDDISABLED}
 else
 if  {$IFDEF USE_FLAGS} G4_Hot in Sender.fFlagsG4
     {$ELSE} Sender.fHot {$ENDIF} then
     fState := 2; {CBS_UNCHECKEDHOT}
 if  {$IFDEF USE_FLAGS} G4_Pressed in Sender.fFlagsG4
     {$ELSE} Sender.fPressed {$ENDIF} then
     fState := 3{CBS_UNCHECKEDPRESSED};
 case Sender.Check3 of
 tsChecked : Inc( fState, 4 );
 tsIndeterminate : Inc( fState, 8 );
 end;

 // Draw back layer
 if  {$IFDEF USE_FLAGS} not( G2_Transparent in Sender.fFlagsG2 )
     {$ELSE} not Sender.fTransparent {$ENDIF} then
 begin
     Brush := CreateSolidBrush(Color2RGB(Sender.fParent.Color));
     fDC := SelectObject(DC, Brush);
     FillRect(DC, RClient, Brush);
     SelectObject(DC, fDC);
     DeleteObject(Brush);
 end;

 // Draw theme
 Color := Sender.Font.Color;
 hThemes := OpenThemeData(Sender.fHandle, 'button');
 if hThemes <> 0 then
 begin
     {$IFDEF USE_FLAGS}
     if  F3_Disabled in Sender.fStyle.f3_Style then
     {$ELSE}
     if  not Sender.fEnabled then
     {$ENDIF}
         GetThemeColor(hThemes, 1, 4, 3803, Color);
     DrawThemeBackground(hThemes, DC, 3 {BP_CHECKBOX}, fState, RCheck, @RCheck);
     CloseThemeData(hThemes);
 end;
  
 // Create font
 F := CreateNewFont(Sender);
 fDC := SelectObject(DC, F);
 // Draw text
 SetBkMode(DC, TRANSPARENT);
 TxtColor := SetTextColor(DC, Color2RGB(Color));
 DrawText(DC, PKOLChar(S), Length(S), RText, DT_LEFT or DT_VCENTER or DT_SINGLELINE);
 // Destroying font
 SetTextColor(DC, Color2RGB(TxtColor));
 SetBkMode(DC, OPAQUE);
 // Destroying object
 SelectObject(DC, fDC);
 DeleteObject(F);

 // Draw focusrect
 if  GetFocus = Sender.fHandle then
 begin
     dec( RText.Left );
     DrawFocusRect(DC, RText);
 end;
end;
//************************* Drawing RadioBox control *************************//
procedure WndRadioBoxXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 hThemes : THandle;
 RClient, RDot, RText : TRect;
 fState : DWORD;
 W, H : Integer;
 S : KOLString;
 F : HFONT;
 fDC : HDC;
 Color, TxtColor : COLORREF;
 Brush : HBRUSH;
begin
 // Checking user owner-draw
 if Assigned(Sender.EV.fOnPaint) and (@Sender.EV.fOnPaint <> @WndRadioBoxXPDraw) then
  begin
   Sender.EV.fOnPaint(Sender, DC);
   exit;
  end;
  
 // Getting metrics
 W := GetSystemMetrics( SM_CXMENUCHECK );
 H := GetSystemMetrics( SM_CYMENUCHECK );
 // Getting caption
 S := Sender.fCaption;
 // Getting rects
 RClient := Sender.ClientRect;
 RDot := RClient;
 RDot.Right := RDot.Left + W;
 if  {$IFDEF USE_FLAGS} G1_WordWrap in Sender.fFlagsG1
     {$ELSE}            Sender.fWordWrap {$ENDIF} then
     RDot.Top := RDot.Top + Sender.Border
 else
     RDot.Top := RDot.Top + (RDot.Bottom - RDot.Top - H) div 2;
 RDot.Bottom := RDot.Top + H;
 RText := MakeRect(RDot.Right + Sender.Border, RDot.Top,
                   RClient.Right, RDot.Bottom);
 // Getting state
 fState := 1; {CBS_UNCHECKEDNORMAL}
 {$IFDEF USE_FLAGS}
 if  F3_Disabled in Sender.fStyle.f3_Style then
 {$ELSE}
 if  not Sender.fEnabled then
 {$ENDIF}
     fState := 4 {CBS_UNCHECKEDDISABLED}
 else
 if  {$IFDEF USE_FLAGS} G4_Hot in Sender.fFlagsG4
     {$ELSE} Sender.fHot {$ENDIF} then
     fState := 2; {CBS_UNCHECKEDHOT}
 if  {$IFDEF USE_FLAGS} G4_Pressed in Sender.fFlagsG4
     {$ELSE} Sender.fPressed {$ENDIF} then
     fState := 3{CBS_UNCHECKEDPRESSED};
 if Sender.Checked then
  Inc( fState, 4 );

 // Draw back layer
 if  {$IFDEF USE_FLAGS} not( G2_Transparent in Sender.fFlagsG2 )
     {$ELSE} not Sender.fTransparent {$ENDIF} then
 begin
     Brush := CreateSolidBrush(Color2RGB(Sender.fParent.Color));
     fDC := SelectObject(DC, Brush);
     FillRect(DC, RClient, Brush);
     SelectObject(DC, fDC);
     DeleteObject(Brush);
 end;

 // Draw theme
 Color := Sender.Font.Color;
 hThemes := OpenThemeData(Sender.fHandle, 'button');
 if hThemes <> 0 then
 begin
     {$IFDEF USE_FLAGS}
     if  F3_Disabled in Sender.fStyle.f3_Style then
     {$ELSE}
     if  not Sender.fEnabled then
     {$ENDIF}
         GetThemeColor(hThemes, 1, 4, 3803, Color);
     DrawThemeBackground(hThemes, DC, 2 {BP_RADIOBOX}, fState, RDot, @RDot);
     CloseThemeData(hThemes);
 end;
  
 // Create font
 F := CreateNewFont(Sender);
 fDC := SelectObject(DC, F);
 // Draw text
 SetBkMode(DC, TRANSPARENT);
 TxtColor := SetTextColor(DC, Color2RGB(Color));
 DrawText(DC, PKOLChar(S), Length(S), RText, DT_LEFT or DT_VCENTER or DT_SINGLELINE);
 // Destroying font
 SetTextColor(DC, Color2RGB(TxtColor));
 SetBkMode(DC, OPAQUE);
 // Destroying object
 SelectObject(DC, fDC);
 DeleteObject(F);
 
 // Draw focusrect
 if  GetFocus = Sender.fHandle then
 begin
     dec( RText.Left );
     DrawFocusRect(DC, RText);
 end;
end;

//******************** Drawing Button and BitButton control ******************//
procedure WndButtonXPDraw( Dummy : Pointer; Sender: PControl; DC: HDC );
var
 hThemes : THandle;
 F : HFONT;
 fDC1, fDC2 : HDC;
 RClient : TRect;
 RText, R1 : TRect;
 RIcon : TRect;
 S : WideString;
 fState, bStyle : DWORD;
 Bmp : HBITMAP;
 W, H : Integer;
 HPos, VPos : DWORD;
 Brush : HBRUSH;
 Pen : HPEN;
 SenderWidth, SenderHeight : integer;
 Flags: DWORD;
 _DC : HDC;
 OldBmp: HBitmap;
 ic : PIcon;
 b : PBitmap;
 i : integer;
 il : PImageList;
begin
 // Checking user owner-draw
 if  Assigned(Sender.EV.fOnPaint)
 and (TMethod(Sender.EV.fOnPaint).Code <> @WndButtonXPDraw) then
 begin
     Sender.EV.fOnPaint(Sender, DC);
     exit;
 end;
 if  Assigned(Sender.EV.fOnBitBtnDraw)
 and (TMethod(Sender.EV.fOnBitBtnDraw).Code <> @DummyProc123_0) then
 begin
   fState := 0{PBS_NORMAL};
   {$IFDEF USE_FLAGS}
   if  F3_Disabled in Sender.fStyle.f3_Style then
   {$ELSE}
   if  not Sender.fEnabled then
   {$ENDIF}
       fState := 2{PBS_DISABLED}
   else
   if  GetFocus = Sender.fHandle then
       fState := 3{PBS_PRESSED}
   else
   if  {$IFDEF USE_FLAGS} G4_Hot in Sender.fFlagsG4
       {$ELSE} Sender.fHot {$ENDIF} then
       fState := 4{PBS_HOT};
   if  {$IFDEF USE_FLAGS} G4_Pressed in Sender.fFlagsG4
       {$ELSE} Sender.fPressed {$ENDIF} then
       fState := 1{PBS_PRESSED};
   Sender.EV.fOnBitBtnDraw(Sender, fState);
   exit;
 end;

 // Getting rects
 RClient := Sender.ClientRect;
 RText := RClient;
 // Calc bitmap rect
 Bmp := Sender.DF.fGlyphBitmap;
 HPos := 0; VPos := 0;
 if  Bmp <> 0 then
 begin
     SenderWidth := Sender.Width;
     SenderHeight := Sender.Height;
     W := Sender.DF.fGlyphWidth;
     H := Sender.DF.fGlyphHeight;
     if  Sender.DF.fGlyphLayout in [ glyphLeft ] then
     begin
         RIcon := MakeRect((SenderWidth div 2) - (W + (W div 4)),
                           (SenderHeight div 2) - (H div 2),
                            W, SenderHeight);
         RText.Left := (SenderWidth div 2) + (W div 4);
         HPos := DT_LEFT;
         VPos := DT_VCENTER;
     end;
     if  Sender.DF.fGlyphLayout in [ glyphRight ] then
     begin
         RIcon := MakeRect((SenderWidth div 2) + (W div 4),
                           (SenderHeight div 2) - (H div 2),
                            W, SenderHeight);
         RText.Right := (SenderWidth div 2) - (W div 4);
         HPos := DT_RIGHT;
         VPos := DT_VCENTER;
     end;
     if  Sender.DF.fGlyphLayout in [ glyphOver ] then
     begin
         RIcon := MakeRect((SenderWidth div 2) - (W div 2),
                           (SenderHeight div 2) - (H div 2),
                            W, SenderHeight);
         HPos := DT_CENTER;
         VPos := DT_VCENTER;
     end;
     if  Sender.DF.fGlyphLayout in [ glyphTop ] then
     begin
         RIcon := MakeRect((SenderWidth div 2) - (W div 2),
                           (SenderHeight div 2) - (H + (H div 4)),
                           W, SenderHeight);
         RText.Top := (SenderHeight div 2) + (H div 4);
         HPos := DT_CENTER;
         VPos := DT_TOP;
     end;
     if  Sender.DF.fGlyphLayout in [ glyphBottom ] then
     begin
         RIcon := MakeRect((SenderWidth div 2) - (W div 2),
                           (SenderHeight div 2) + (H div 4),
                            W, SenderHeight);
         RText.Bottom := (SenderHeight div 2) - (H div 4);
         HPos := DT_CENTER;
         VPos := DT_BOTTOM;
     end;
 end else
 begin
     HPos := DT_CENTER;
     VPos := DT_VCENTER;
     RIcon := MakeRect(0, 0, 0, 0);
 end;

 // Getting caption
 S := KOLWideString( Sender.fCaption );
 // Getting state
 fState := 1{PBS_NORMAL};
 {$IFDEF USE_FLAGS}
 if  F3_Disabled in Sender.fStyle.f3_Style then
 {$ELSE}
 if  not Sender.fEnabled then
 {$ENDIF}
     fState := 4{PBS_DISABLED}
 else
 if  {$IFDEF USE_FLAGS} G4_Hot in Sender.fFlagsG4
     {$ELSE} Sender.fHot {$ENDIF} then
     fState := 2{PBS_HOT};
 if  {$IFDEF USE_FLAGS} G4_Pressed in Sender.fFlagsG4
     {$ELSE} Sender.fPressed {$ENDIF} then
     fState := 3{PBS_PRESSED};
 // Opening themes
 hThemes := OpenThemeData(Sender.fHandle, 'button');
 if hThemes <> 0 then
  begin
   Brush := CreateSolidBrush(Color2RGB(Sender.fParent.Color));
   fDC1 := SelectObject(DC, Brush);
   FillRect(DC, RClient, Brush);
   if (Sender.Flat) and (fState = 1{PBS_NORMAL}) then
    begin
     Pen := CreatePen(PS_SOLID, 1, clLtGrey);
     fDC2 := SelectObject(DC, Pen);
     RoundRect(DC, RClient.Left+2, RClient.Top+2, RClient.Right-2, RClient.Bottom-2, 3, 3);
     SelectObject(DC, fDC2);
     DeleteObject(Pen);
    end
     else
      DrawThemeBackground(hThemes, DC, 1{BP_PUSHBUTTON}, fState, RClient, @RClient);
   SelectObject(DC, fDC1);
   DeleteObject(Brush);

   if Bmp <> 0 then
    begin
     if bboImageList in Sender.DF.fBitBtnOptions then
      begin
       bStyle := ILD_TRANSPARENT;
       {$IFDEF USE_FLAGS}
       if  not (F3_Disabled in Sender.fStyle.f3_Style) then
       {$ELSE}
       if  Sender.fEnabled then
       {$ENDIF}
	    i := Sender.BitBtnImgIdx
        else
         begin
          ic := NewIcon;
          ic.fSize := Sender.DF.fGlyphWidth;
          ic.fHandle := ImageList_GetIcon(Bmp, Sender.BitBtnImgIdx, bStyle);
          b := NewBitmap(ic.fSize, ic.fSize);
          b.fHandle := ic.Convert2Bitmap(clBtnFace);
          ConvertBitmap2Grayscale(b);
          i := ImageList_Add(Bmp, b.fHandle, 0);
          Free_And_Nil(b);
          Free_And_Nil(ic);
         end;
       ImageList_Draw(Bmp, i, DC, RIcon.Left, RIcon.Top, bStyle);
      end
       else
        begin
         _DC := CreateCompatibleDC( 0 );
         {$IFDEF USE_FLAGS}
         if  not (F3_Disabled in Sender.fStyle.f3_Style) then
         {$ELSE}
         if  Sender.fEnabled then
         {$ENDIF}
		  OldBmp := SelectObject( _DC, Bmp)
          else
           begin
            bStyle := ILD_TRANSPARENT;
            il := NewImageList(Sender.fParent);
            il.HandleNeeded;
            i := ImageList_Add(il.fHandle, Bmp, 0);
            ic := NewIcon;
            ic.fSize := Sender.DF.fGlyphWidth;
            ic.fHandle := ImageList_GetIcon(il.fHandle, i, bStyle);
            b := NewBitmap(ic.fSize, ic.fSize);
            b.fHandle := ic.Convert2Bitmap(clBtnFace);
            ConvertBitmap2Grayscale(b);
            OldBmp := SelectObject( _DC, b.fHandle);
            Free_And_Nil(b);
            Free_And_Nil(ic);
            Free_And_Nil(il);
           end;
         StretchBlt( DC, RIcon.Left, RIcon.Top, Sender.DF.fGlyphWidth, Sender.DF.fGlyphHeight,
                     _DC, 0, 0, Sender.DF.fGlyphWidth, Sender.DF.fGlyphHeight,
                     SRCCOPY);
         SelectObject( _DC, OldBmp );
         DeleteDC( _DC );
        end;
    end;
   // Create font
   F := CreateNewFont(Sender);
   fDC1 := SelectObject(DC, F);
   // Draw text
   Flags := HPos or VPos;
   R1 := RText;
   if Sender.Style and BS_MULTILINE = 0 then
     Flags := Flags or DT_SINGLELINE
   else
   begin
     Flags := Flags and not DT_VCENTER or DT_WORDBREAK;
     if VPos and DT_VCENTER <> 0 then
     begin
       DrawTextW(DC, PWideChar( S ), Length(S), R1, Flags or DT_CALCRECT);
       OffsetRect( R1, 0,
         ( (RText.Bottom - RText.Top) - (R1.Bottom - R1.Top) ) div 2 );
       if HPos and DT_CENTER <> 0 then
       OffsetRect( R1,
         ( (RText.Right - RText.Left) - (R1.Right - R1.Left) ) div 2, 0 );
     end;
   end;
   DrawThemeText(hThemes, DC, 1{BP_PUSHBUTTON}, fState, PWideChar(S), Length(S),
                 Flags, 0, R1);
   // Destroying font
   SelectObject(DC, fDC1);
   DeleteObject(F);

   CloseThemeData(hThemes);
  end;      

 if (GetFocus = Sender.fHandle) and (bboFocusRect in Sender.DF.fBitBtnOptions) then
  DrawFocusRect(DC, MakeRect(RClient.Left+4, RClient.Top+4, RClient.Right-4, RClient.Bottom-4));
end;
//************************* Control MouseEnter event *************************//
{$IFDEF ASM_VERSION}
procedure WndXPMouseEnter( Dummy : Pointer; Sender: PObj );
asm
    {$IFDEF  USE_FLAGS}
    OR       [EDX].TControl.fFlagsG4, 1 shl G4_Hot
    {$ELSE}
    MOV      [EDX].TControl.fHot, 1
    {$ENDIF}
    {$IFDEF  EVENTS_DYNAMIC}
    MOV      EAX, [EDX].TControl.EV
    MOV      ECX, [EAX].TEvents.fOnMouseEnter.TMethod.Code
    {$ELSE}
    MOV      ECX, [EDX].TControl.EV.fOnMouseEnter.TMethod.Code
    {$ENDIF}
    JECXZ    @@fin
    CMP      ECX, offset[WndXPMouseEnter]
    JZ       @@fin
    {$IFDEF  EVENTS_DYNAMIC}
    MOV      EAX, [EAX].TEvents.fOnMouseEnter.TMethod.Data
    {$ELSE}
    MOV      EAX, [EDX].TControl.EV.fOnMouseEnter.TMethod.Data
    {$ENDIF}
    CALL     ECX
@@fin:
end;
{$ELSE}
procedure WndXPMouseEnter( Dummy : Pointer; Sender: PObj );
begin
     with PControl(Sender)^ do
     begin
        {$IFDEF USE_FLAGS}
                fFlagsG4 := fFlagsG4 + [G4_Hot];
        {$ELSE} fHot := true; {$ENDIF}
        if  Assigned(EV.fOnMouseEnter) and
            (@EV.fOnMouseEnter <> @WndXPMouseEnter) then
            EV.fOnMouseEnter(Sender);
     end;
end;
{$ENDIF}
//************************* Control MouseLeave event *************************//
{$IFDEF ASM_VERSION}
procedure WndXPMouseLeave( Dummy : Pointer; Sender: PObj );
asm
    {$IFDEF  USE_FLAGS}
    AND      [EDX].TControl.fFlagsG4, not(1 shl G4_Hot)
    {$ELSE}
    MOV      [EDX].TControl.fHot, 0
    {$ENDIF}
    {$IFDEF  EVENTS_DYNAMIC}
    MOV      EAX, [EDX].TControl.EV
    MOV      ECX, [EAX].TEvents.fOnMouseLeave.TMethod.Code
    {$ELSE}
    MOV      ECX, [EDX].TControl.EV.fOnMouseLeave.TMethod.Code
    {$ENDIF}
    JECXZ    @@fin
    CMP      ECX, offset[WndXPMouseLeave]
    JZ       @@fin
    {$IFDEF  EVENTS_DYNAMIC}
    MOV      EAX, [EAX].TEvents.fOnMouseLeave.TMethod.Data
    {$ELSE}
    MOV      EAX, [EDX].TControl.EV.fOnMouseLeave.TMethod.Data
    {$ENDIF}
    CALL     ECX
@@fin:
end;
{$ELSE}
procedure WndXPMouseLeave( Dummy : Pointer; Sender: PObj );
begin
    {$IFDEF USE_FLAGS}
            PControl(Sender).fFlagsG4 :=
            PControl(Sender).fFlagsG4 - [G4_Hot];
    {$ELSE} PControl(Sender).fHot := false; {$ENDIF}
    if  Assigned(PControl(Sender).EV.fOnMouseLeave) and
        (@PControl(Sender).EV.fOnMouseLeave <> @WndXPMouseLeave) then
        PControl(Sender).EV.fOnMouseLeave(Sender);
end;
{$ENDIF}
//*************************** Control Message event **************************//
function WndXPMessage( Sender: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
var
 pt : TPoint;
 Mouse: TMouseEventData;
 dDC : HDC;
begin
 Result := false;

 case Msg.message of
 WM_LBUTTONDBLCLK:
  begin
   if Assigned(Sender.EV.fOnMouseDblClk) then
    begin
     Mouse.Button := mbLeft;
     Mouse.StopHandling := false;
     Mouse.R1 := 0;
     Mouse.R2 := 0;
     Mouse.Shift := 120;
     Mouse.X := 0;
     Mouse.Y := 0;
     GetCursorPos(pt);
     if ScreenToClient(Sender.fHandle, pt) then
      begin
       Mouse.X := pt.X;
       Mouse.Y := pt.Y;
      end;
     Sender.EV.fOnMouseDblClk(Sender, Mouse);
    end;
    if  {$IFDEF USE_FLAGS} not(G5_IsSplitter in Sender.fFlagsG5)
        {$ELSE} not Sender.fIsSplitter {$ENDIF} then
        Sender.Perform( WM_LBUTTONDOWN, Msg.wParam, Msg.lParam );
  end;
  
 WM_LBUTTONDOWN:
  begin
   if  Assigned(Sender.EV.fOnMouseDown) then
   begin
       Mouse.Button := mbLeft;
       Mouse.StopHandling := false;
       Mouse.R1 := 0;
       Mouse.R2 := 0;
       Mouse.Shift := 120;
       Mouse.X := 0;
       Mouse.Y := 0;
       GetCursorPos(pt);
       if  ScreenToClient(Sender.fHandle, pt) then
       begin
           Mouse.X := pt.X;
           Mouse.Y := pt.Y;
       end;
       Sender.EV.fOnMouseDown(Sender, Mouse);
   end;
   {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 + [G4_Pressed];
   {$ELSE} Sender.fPressed := true; {$ENDIF}
   dDC := GetWindowDC(Msg.hWnd);
   Sender.EV.fOnPaint(Sender, dDC);
   ReleaseDC( Msg.hWnd, dDC );  // vampir_infernal 15.10.2008
  end;

 WM_LBUTTONUP:
  begin
   if  Assigned(Sender.EV.fOnMouseUp) then
   begin
       Mouse.Button := mbLeft;
       Mouse.StopHandling := false;
       Mouse.R1 := 0;
       Mouse.R2 := 0;
       Mouse.Shift := 120;
       Mouse.X := 0;
       Mouse.Y := 0;
       GetCursorPos(pt);
       if  ScreenToClient(Sender.fHandle, pt) then
       begin
           Mouse.X := pt.X;
           Mouse.Y := pt.Y;
       end;
       Sender.EV.fOnMouseUp(Sender, Mouse);
   end;
   {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 - [G4_Pressed];
   {$ELSE} Sender.fPressed := false; {$ENDIF}
   dDC := GetWindowDC(Msg.hWnd);
   Sender.EV.fOnPaint(Sender, dDC);
   ReleaseDC( Msg.hWnd, dDC );
  end;

 WM_KEYDOWN:
  begin
     if  Msg.wParam = VK_SPACE then
     begin
         if  Assigned(Sender.EV.fOnKeyDown) then
             Sender.EV.fOnKeyDown(Sender, Msg.wParam, GetShiftState);
         {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 + [G4_Pressed];
         {$ELSE} Sender.fPressed := true; {$ENDIF}
         dDC := GetWindowDC(Msg.hWnd);
         Sender.EV.fOnPaint(Sender, dDC);
         ReleaseDC( Msg.hWnd, dDC );
     end;
  end;

 WM_KEYUP:
  begin
     if Msg.wParam = VK_SPACE then
     begin
         if  Assigned(Sender.EV.fOnKeyUp) then
             Sender.EV.fOnKeyUp(Sender, Msg.wParam, GetShiftState);
         {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 - [G4_Pressed];
         {$ELSE} Sender.fPressed := false; {$ENDIF}
         dDC := GetWindowDC(Msg.hWnd);
         Sender.EV.fOnPaint(Sender, dDC);
         ReleaseDC( Msg.hWnd, dDC );
     end;
  end;

  WM_KILLFOCUS:
   begin
      {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 - [G4_Hot];
      {$ELSE} Sender.fHot := false; {$ENDIF}
      dDC := GetWindowDC(Msg.hWnd);
      Sender.EV.fOnPaint(Sender, dDC);
      ReleaseDC( Msg.hWnd, dDC );
   end;

  WM_SETFOCUS:
   begin
      {$IFDEF USE_FLAGS} Sender.fFlagsG4 := Sender.fFlagsG4 + [G4_Hot];
      {$ELSE} Sender.fHot := TRUE; {$ENDIF}
      dDC := GetWindowDC(Msg.hWnd);
      Sender.EV.fOnPaint(Sender, dDC);
      ReleaseDC( Msg.hWnd, dDC );
      Result := true;
   end;
  end;
end;
//*************************** Events for CheckBox ****************************//
procedure XP_Themes_For_CheckBox(Sender : PControl);
begin
 if  AppTheming then
     Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndCheckBoxXPDraw ) );
end;
//*************************** Events for RadioBox ****************************//
procedure XP_Themes_For_RadioBox(Sender : PControl);
begin
 if  AppTheming then
     Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndRadioBoxXPDraw ) );
end;
//**************************** Events for Panel ******************************//
procedure XP_Themes_For_Panel(Sender : PControl);
begin
 if AppTheming then
  begin
   if Sender.EdgeStyle = esTransparent then Sender.SetTransparent(True) else
    begin
     Sender.OnResize := TOnEvent( MakeMethod( nil, @WndPanelXPResize ) );
     Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndPanelXPDraw ) );
    end;
  end;
end;
//*************************** Events for Splitter ****************************//
procedure XP_Themes_For_Splitter(Sender : PControl);
begin
 if AppTheming then
  begin
   Sender.AttachProc(WndXPMessage);
   Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndSplitterXPDraw ) );
  end;
end;
//**************************** Events for Label ******************************//
procedure XP_Themes_For_Label(Sender : PControl);
begin
 if AppTheming then Sender.SetTransparent(True);
end;
//************************** Events for GroupBox *****************************//
procedure XP_Themes_For_GroupBox(Sender : PControl);
begin
 if  AppTheming then
     Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndGroupBoxXPDraw ) );
end;
//************************** Events for TabPanel *****************************//
procedure XP_Themes_For_TabPanel(Sender : PControl);
begin
 if  AppTheming then
     Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndTabXPDraw ) );
end;
//********************* Events for Button and BitButton **********************//
procedure XP_Themes_For_BitBtn(Sender : PControl);
begin
 if AppTheming then
  begin
   Sender.AttachProc(WndXPMessage);
   Sender.OnMouseEnter := TOnEvent( MakeMethod( nil, @WndXPMouseEnter ) );
   Sender.OnMouseLeave := TOnEvent( MakeMethod( nil, @WndXPMouseLeave ) );
   Sender.OnPaint := TOnPaint( MakeMethod( nil, @WndButtonXPDraw ) );
  end;
end;
//*********************** Deattach ownerdraw function ************************//
procedure Deattach(Sender : PControl; PaintProc : Pointer);
begin
 if  Sender.IsProcAttached(WndXPMessage) then
     Sender.DetachProc(WndXPMessage);
 if  {$IFDEF NIL_EVENTS} Assigned(Sender.EV.fOnMouseEnter) and {$ENDIF}
     (@Sender.EV.fOnMouseEnter = @WndXPMouseEnter)
 and ({$IFDEF USE_FLAGS} not(G3_Flat in Sender.fFlagsG3)
      {$ELSE} not Sender.fFlat {$ENDIF}) then
     {$IFDEF NIL_EVENTS} Sender.EV.fOnMouseEnter := nil;
     {$ELSE} TMethod( Sender.EV.fOnMouseEnter ).Code := @DummyObjProc;
     {$ENDIF}
 if  {$IFDEF NIL_EVENTS} Assigned(Sender.EV.fOnMouseLeave) and {$ENDIF}
     (@Sender.EV.fOnMouseLeave = @WndXPMouseLeave)
 and ({$IFDEF USE_FLAGS} not(G3_Flat in Sender.fFlagsG3)
      {$ELSE} not Sender.fFlat {$ENDIF}) then
     {$IFDEF NIL_EVENTS} Sender.EV.fOnMouseLeave := nil;
     {$ELSE} TMethod( Sender.EV.fOnMouseLeave ).Code := @DummyObjProc;
     {$ENDIF} 
 if  {$IFDEF NIL_EVENTS} Assigned(Sender.EV.fOnPaint) and {$ENDIF}
     (@Sender.EV.fOnPaint = PaintProc) then
     {$IFDEF NIL_EVENTS} Sender.EV.fOnPaint := nil;
     {$ELSE}             TMethod( Sender.EV.fOnPaint ).Code := @DummyObjProc;
     {$ENDIF}
end;
//********************* Handling of message WM_THEMECHANGED ******************//
function WndXP_WM_THEMECHANGED( Sender: PControl; var Msg: TMsg; var Rslt: Integer ): Boolean;
begin
 Result := false;

 if Msg.message = $31A {WM_THEMECHANGED} then
 begin
   if AppTheming then DeinitThemes;
   CheckThemes;
   if AppTheming then
   begin
       InitThemes;
       if ((Sender.fStyle.Value and BS_AUTOCHECKBOX) = BS_AUTOCHECKBOX) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           XP_Themes_For_CheckBox(Sender);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_AUTO3STATE) = BS_AUTO3STATE) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           XP_Themes_For_CheckBox(Sender);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_RADIOBUTTON) = BS_RADIOBUTTON) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           XP_Themes_For_RadioBox(Sender);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_GROUPBOX) = BS_GROUPBOX) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 =
               [G5_IsGroupbox])
          {$ELSE}
          (Sender.fIsGroupBox = true) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           XP_Themes_For_GroupBox(Sender);
           exit;
       end;
       if (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) {$ENDIF} then
       begin
           XP_Themes_For_BitBtn(Sender);
           exit;
       end;
       if (Sender.SubClassName = 'obj_STATIC') then
       begin
         if  {$IFDEF USE_FLAGS} G1_IsStaticControl in Sender.fFlagsG1
             {$ELSE} Sender.fIsStaticControl > 0 {$ENDIF} then
             XP_Themes_For_Label(Sender)
         else
         begin
           if  {$IFDEF USE_FLAGS} G5_IsSplitter in Sender.fFlagsG5
               {$ELSE} Sender.fIsSplitter {$ENDIF} then
               XP_Themes_For_Splitter(Sender)
           else
           begin
               if  Sender.fParent.SubClassName = 'obj_SysTabControl32' then
                   XP_Themes_For_TabPanel(Sender)
               else
                   XP_Themes_For_Panel(Sender);
            end;
         end;
         exit;
       end;
   end else
   begin
       if ((Sender.fStyle.Value and BS_AUTOCHECKBOX) = BS_AUTOCHECKBOX) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           Deattach(Sender, @WndCheckBoxXPDraw);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_AUTO3STATE) = BS_AUTO3STATE) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           Deattach(Sender, @WndCheckBoxXPDraw);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_RADIOBUTTON) = BS_RADIOBUTTON) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           Deattach(Sender, @WndRadioBoxXPDraw);
           exit;
       end;
       if ((Sender.fStyle.Value and BS_GROUPBOX) = BS_GROUPBOX) and
          (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter, G5_IsBitBtn] * Sender.fFlagsG5 =
               [G5_IsGroupbox])
          {$ELSE}
          (Sender.fIsGroupBox = true) and
          (Sender.fIsSplitter = false) and
          (Sender.fIsBitBtn = false) {$ENDIF} then
       begin
           Deattach(Sender, @WndGroupBoxXPDraw);
           exit;
       end;
       if (Sender.SubClassName = 'obj_BUTTON') and
          {$IFDEF USE_FLAGS}
              ([G5_IsGroupbox, G5_IsSplitter] * Sender.fFlagsG5 = [])
          {$ELSE}
          (Sender.fIsGroupBox = false) and
          (Sender.fIsSplitter = false) {$ENDIF} then
       begin
           Deattach(Sender, @WndButtonXPDraw);
           exit;
       end;
       if (Sender.SubClassName = 'obj_STATIC') then
       begin
           if  {$IFDEF USE_FLAGS} G1_IsStaticControl in Sender.fFlagsG1
               {$ELSE} Sender.fIsStaticControl > 0 {$ENDIF} then
           else
           begin
              if  {$IFDEF USE_FLAGS} G5_IsSplitter in Sender.fFlagsG5
                  {$ELSE} Sender.fIsSplitter {$ENDIF} then
                  Deattach(Sender, @WndSplitterXPDraw)
              else if Sender.fParent.SubClassName = 'obj_SysTabControl32' then
                  Deattach(Sender, @WndTabXPDraw)
              else
              begin
                  Deattach(Sender, @WndPanelXPDraw);
                  case Sender.EdgeStyle of
                  esRaised:
                      begin
                         Sender.fStyle.Value := Sender.fStyle.Value and (not SS_SUNKEN);
                         Sender.fExStyle := Sender.fExStyle and (not WS_EX_STATICEDGE);
                         Sender.fExStyle := Sender.fExStyle or WS_EX_WINDOWEDGE;
                         Sender.fStyle.Value := Sender.fStyle.Value or WS_DLGFRAME;
                      end;
                  esLowered:
                      begin
                         Sender.fStyle.Value := Sender.fStyle.Value and (not WS_DLGFRAME);
                         Sender.fExStyle := Sender.fExStyle or WS_EX_WINDOWEDGE;
                         Sender.fExStyle := Sender.fExStyle or WS_EX_STATICEDGE;
                         Sender.fStyle.Value := Sender.fStyle.Value or SS_SUNKEN;
                      end;
                  else
                      Sender.fStyle.Value := Sender.fStyle.Value and (not SS_SUNKEN) and (not WS_DLGFRAME);
                      Sender.fExStyle := Sender.fExStyle and (not WS_EX_STATICEDGE) or WS_EX_WINDOWEDGE;
                  end;
              end;
           end;
           Sender.SetTransparent(
               {$IFDEF USE_FLAGS} G2_ClassicTransparent in Sender.fFlagsG2
               {$ELSE} Sender.fClassicTransparent {$ENDIF} );
           exit;
       end;
   end;
 end;
end;
//********************* Attaching to message WM_THEMECHANGED *****************//
type TSenderProc = procedure(Sender: PControl);
{$IFDEF ASM_VERSION}
procedure Attach_WM_THEMECHANGED(Sender : PControl; const XP_Themes_for: TSenderProc);
asm
    {$IFDEF USE_FLAGS}
    MOV  CX, word ptr [EAX].TControl.fFlagsG2
    AND  CX, not(1 shl G3_ClassicTransparent)shl 8 or (1 shl G2_Transparent)
    OR   CL, CH
    MOV  [EAX].TControl.fFlagsG3, CL
    {$ELSE}
    MOV  CL, [EAX].TControl.fTransparent
    MOV  [EAX].TControl.fClassicTransparent, CL
    {$ENDIF}
    PUSH EDX
    PUSH EAX
    MOV  EDX, offset[WndXP_WM_THEMECHANGED]
    CALL TControl.AttachProc
    POP  EAX
    POP  EDX
    CALL EDX
end;
{$ELSE PASCAL}
procedure Attach_WM_THEMECHANGED(Sender : PControl; const XP_Themes_for: TSenderProc);
begin
    {$IFDEF USE_FLAGS}
            if  G2_Transparent in Sender.fFlagsG2 then
                Sender.fFlagsG3 := Sender.fFlagsG3 + [G3_ClassicTransparent]
            else
                Sender.fFlagsG3 := Sender.fFlagsG3 - [G3_ClassicTransparent];
    {$ELSE} Sender.fClassicTransparent := Sender.fTransparent; {$ENDIF}
    Sender.AttachProc(WndXP_WM_THEMECHANGED);
    XP_Themes_for(Sender);
end;
{$ENDIF ASM_VERSION}
//********************************* End File *********************************//
