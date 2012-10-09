// part of KOLMHToolTip -- interface_part.
// Moved to separate inc-file still Delphi20XX does not allow compile 
// in DEBUG mode.

const
  Dummy1 = 1;

  TTDT_AUTOMATIC = 0;
  TTDT_RESHOW = 1;
  TTDT_AUTOPOP = 2;
  TTDT_INITIAL = 3;

function NewMHToolTip(AParent: PControl): PMHToolTip;
const
  CS_DROPSHADOW = $00020000;
begin
  DoInitCommonControls(ICC_BAR_CLASSES);
  New(Result, Create);

  Result.fHandle := CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, '', 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, AParent.GetWindowHandle, 0, HInstance, nil);
end;

function TMHToolTip.GetDelay(const Index: Integer): Integer;
begin
  Result := SendMessage(fHandle, TTM_GETDELAYTIME, Index, 0);
end;


procedure TMHToolTip.SetDelay(const Index, Value: Integer);
begin
  SendMessage(handle, TTM_SETDELAYTIME, Index, MAKELONG(Value, 0));
end;


function TMHToolTip.GetColor(const Index: Integer): TColor;
begin
  Result := SendMessage(handle, TTM_GETTIPBKCOLOR + Index, 0, 0);
end;

procedure TMHToolTip.SetColor(const Index: Integer; const Value: TColor);
begin
  SendMessage(handle, TTM_SETTIPBKCOLOR + Index, Value, 0);
end;

function TMHToolTip.GetMaxWidth: Integer;
begin
  Result := SendMessage(fHandle, TTM_GETMAXTIPWIDTH, 0, 0);
end;

procedure TMHToolTip.SetMaxWidth(const Value: Integer);
begin
  SendMessage(fHandle, TTM_SETMAXTIPWIDTH, 0, Value);
end;

function TMHToolTip.GetMargin: TRect;
begin
  SendMessage(fHandle, TTM_GETMARGIN, 0, DWord(@Result));
end;

procedure TMHToolTip.SetMargin(const Value: TRect);
begin
  SendMessage(fHandle, TTM_SETMARGIN, 0, DWord(@Value));
end;

function TMHToolTip.GetActivate: Boolean;
begin
  // ??????
  Result := False;
end;

procedure TMHToolTip.SetActivate(const Value: Boolean);
begin
  SendMessage(fHandle, TTM_ACTIVATE, DWord(Value), 0);
end;

procedure TMHToolTip.Pop;
begin
  SendMessage(fHandle, TTM_POP, 0, 0);
end;

procedure TMHToolTip.Popup;
begin
  SendMessage(fHandle, $0422 {TTM_POPUP}, 0, 0);
end;

procedure TMHToolTip.Update;
begin
  inherited; // ???
  SendMessage(fHandle, TTM_UPDATE, 0, 0);
end;

function NewHint(A: PControl): PMHHint;
begin
  New(Result, Create);

  with Result^ do
  begin
    Parent := A;
    ToolTip := nil; // ???
    HasTool := False; // ???
  end;
  A.Add2AutoFree(Result);
end;

function NewManager: PMHToolTipManager;
begin
  New(Result, Create);
end;

{ TMHHint }

function TMHHint.GetDelay(const Index: Integer): Integer;
begin
//  CreateToolTip;
  Result := 0;
  if Assigned(ToolTip) then
    Result := ToolTip.GetDelay(Index);
end;

function TMHHint.GetFI: TFI;
begin
  /// !!! DANGER-WITH !!!
  with Result, ToolTip^ do
  begin
    FE := FE + [eTextColor];
    Colors[1] := TextColor;

    FE := FE + [eBkColor];
    Colors[0] := BkColor;

    FE := FE + [eAPDelay];
    Delays[TTDT_AUTOPOP] := AutoPopDelay;

    FE := FE + [eRDelay];
    Delays[TTDT_RESHOW] := ReshowDelay;

    FE := FE + [eIDelay];
    Delays[TTDT_INITIAL] := InitialDelay;
  end;
end;

procedure TMHHint.ReConnect(FI: TFI);
var
  TMP: PMHToolTip;
begin
  with GetManager^ do
  begin
    TMP := FindNeed(FI);
    if not Assigned(TMP) then
      TMP := CreateNeed(FI);
    if Assigned(ToolTip) and HasTool then
      MoveTool(TMP);
    ToolTip := TMP;
  end;
end;

procedure TMHHint.MoveTool(T1: PMHToolTip);
var
  TI: TToolInfo;
  TextL: array[0..255] of KOLChar;
begin
  if T1 = ToolTip then
    Exit;
  with TI do
  begin
    cbSize := SizeOf(TI);
    hWnd := Parent.GetWindowHandle;
    uId := Parent.GetWindowHandle;
    lpszText := @TextL[0];
  end;

  SendMessage(ToolTip.handle, TTM_GETTOOLINFO, 0, DWord(@TI));
  SendMessage(ToolTip.handle, TTM_DELTOOL, 0, DWORD(@TI));
  ToolTip.Count := ToolTip.Count - 1;
  SendMessage(T1.handle, TTM_ADDTOOL, 0, DWORD(@TI));
  T1.Count := T1.Count - 1;

  HasTool := True;

end;

procedure TMHHint.SetColor(const Index: Integer; const Value: TColor);
var
  FI: TFI;
begin
  if Assigned(ToolTip) then
  begin
    if ToolTip.Count + Byte(not HasTool) = 1 then
    begin
      ToolTip.SetColor(Index, Value);
      Exit;
    end;
    FI := GetFI;
  end;

  case Index of
    0: FI.FE := FI.FE + [eBkColor];
    1: FI.FE := FI.FE + [eTextColor];
  end;
  FI.Colors[Index] := Value;

  ReConnect(FI);
end;

function TMHHint.GetColor(const Index: Integer): TColor;
begin
  Result := 0;
  if Assigned(ToolTip) then
    Result := ToolTip.GetColor(Index);
end;

procedure TMHHint.SetDelay(const Index, Value: Integer);
var
  FI: TFI;
begin
  if Assigned(ToolTip) then
  begin
    if ToolTip.Count + Byte(not HasTool) = 1 then
    begin
      ToolTip.SetDelay(Index, Value);
      Exit;
    end;
    FI := GetFI;
  end;

  case Index of
    TTDT_AUTOPOP: FI.FE := FI.FE + [eAPDelay]; // Spec
    TTDT_INITIAL: FI.FE := FI.FE + [eIDelay]; // Spec
    TTDT_RESHOW: FI.FE := FI.FE + [eRDelay]; // Spec
  end; //case

  FI.Delays[Index] := Value; //Spec

  ReConnect(FI);
end;

procedure TMHHint.SetText(Value: KOLString);
var
  TI: TToolInfo;
begin
  ProcBegin(TI);

  with TI do
  begin
    uFlags := TTF_SUBCLASS or TTF_IDISHWND; // Spec
    lpszText := PKOLChar(Value); // Spec
  end;

  procEnd(TI);

  if HasTool then
  begin
    TI.lpszText := PKOLChar(Value);
    SendMessage(ToolTip.handle, TTM_SETTOOLINFO, 0, DWord(@TI));
  end;

end;

{ TMHToolTipManager }

function TMHToolTipManager.AddTip: Integer;
begin
  SetLength(TTT, Length(TTT) + 1);
  TTT[Length(TTT) - 1] := NewMHToolTip(Applet);
  Result := Length(TTT) - 1;
end;

function TMHToolTipManager.FindNeed(FI: TFI): PMHToolTip;
var
  i: Integer;
begin
  Result := nil;
  for i := 0 to length(TTT) - 1 do
  begin
    if ((eTextColor in FI.FE) and (not (FI.Colors[1] = TTT[i].TextColor))) or
      ((eBkColor in FI.FE) and (not (FI.Colors[0] = TTT[i].BkColor))) or
      ((eAPDelay in FI.FE) and (not (FI.Delays[TTDT_AUTOPOP] = TTT[i].AutoPopDelay))) or
      ((eIDelay in FI.FE) and (not (FI.Delays[TTDT_INITIAL] = TTT[i].InitialDelay))) or
      ((eRDelay in FI.FE) and (not (FI.Delays[TTDT_RESHOW] = TTT[i].ReshowDelay))) then
      Continue;
    Result := TTT[i];
    Break;
  end;
end;

function TMHToolTipManager.CreateNeed(FI: TFI): PMHToolTip;

begin
  Setlength(TTT, length(TTT) + 1);
  TTT[length(TTT) - 1] := NewMHToolTip(Applet);
  with TTT[length(TTT) - 1]^ do
  begin
    if (eTextColor in FI.FE) then
      TextColor := FI.Colors[1];
    if (eBkColor in FI.FE) then
      BkColor := FI.Colors[0];
    if (eAPDelay in FI.FE) then
      AutoPopDelay := FI.Delays[TTDT_AUTOPOP];
    if (eIDelay in FI.FE) then
      InitialDelay := FI.Delays[TTDT_INITIAL];
    if (eRDelay in FI.FE) then
      ReshowDelay := FI.Delays[TTDT_RESHOW];
  end;
  Result := TTT[length(TTT) - 1];
end;

procedure TMHHint.ProcBegin(var TI: TToolInfo);
begin
  CreateToolTip;

  with TI do
  begin
    cbSize := SizeOf(TI);
    hWnd := Parent.GetWindowHandle;
    uId := Parent.GetWindowHandle;
    hInst := 0;
  end;
end;

procedure TMHHint.ProcEnd(var TI: TToolInfo);
var
  TextLine: array[0..255] of KOLChar;
begin
  if not HasTool then
  begin
    SendMessage(ToolTip.handle, TTM_ADDTOOL, 0, DWORD(@TI));
    HasTool := True;
    ToolTip.Count := ToolTip.Count + 1;
  end
  else
  begin
    with TI do
    begin
      lpszText := @TextLine[0];
    end;
    SendMessage(ToolTip.handle, TTM_SETTOOLINFO, 0, DWord(@TI));
  end;
end;

destructor TMHToolTipManager.Destroy;
var
  i: Integer;
begin
  for i := 0 to Length(TTT) - 1 do
    TTT[i].Free;
  SetLength(TTT, 0);
  inherited;
end;

procedure TMHHint.Pop;
begin
  if Assigned(ToolTip) and (HasTool) then
  begin // ^^^^^^^^^^^^ ???
//  CreateToolTip;
    ToolTip.Pop;
  end;
end;

procedure TMHHint.Popup;
begin
  if Assigned(ToolTip) and (HasTool) then
  begin // ^^^^^^^^^^^^ ???
//  CreateToolTip;
    ToolTip.Popup;
  end;
end;

destructor TMHHint.Destroy;
var
  TI: TToolInfo;
  i: integer;
begin
  with TI do
  begin
    cbSize := SizeOf(TI);
    hWnd := Parent.GetWindowHandle;
    uId := Parent.GetWindowHandle;
  end;

  SendMessage(ToolTip.handle, TTM_DELTOOL, 0, DWORD(@TI));
  ToolTip.Count := ToolTip.Count - 1;
  if ToolTip.Count <= 0 then begin
    i:=Length(Manager.TTT);
    if i > 1 then begin
      Manager.TTT[i - 1].Free;
      SetLength(Manager.TTT, i - 1);
    end
    else
      Free_And_Nil(Manager);
  end;
  inherited;
end;

destructor TMHToolTip.Destroy;
begin
  inherited;
end;

procedure TMHHint.CreateToolTip;
begin
  if not Assigned(ToolTip) then
  begin
    if Length(GetManager.TTT) = 0 then
      GetManager.AddTip;
    ToolTip := GetManager.TTT[0];
  end;
end;

function TMHHint.GetText: KOLString;
var
  TI: TToolInfo;
  TextL: array[0..255] of KOLChar;
begin
  if Assigned(ToolTip) and (HasTool) then
  begin
    // !!!
    with TI do
    begin
    // ????
//      FillChar(TI, SizeOf(TI), 0);
      cbSize := SizeOf(TI);
      hWnd := Parent.GetWindowHandle;
      uId := Parent.GetWindowHandle;
      lpszText := @TextL[0];
    end;
    SendMessage(ToolTip.handle, TTM_GETTOOLINFO, 0, DWord(@TI));
    Result := TextL; //TI.lpszText;// := PChar(Value);
  end;
end;

function TMHHint.GetManager: PMHToolTipManager;
begin
  if Manager=nil then
    Manager:=NewManager;
  Result:=Manager;
end;

