{-----------------------------------------------------------------------------
 VertSB (historypp project)

 Version:   1.0
 Created:   25.03.2003
 Author:    Oxygen

 [ Description ]

 Reimplementation of TControlScrollBar for use with
 THistoryGrid to make scrolling much better. Sets Page
 for scrollbar to different value, instead of using
 Control's ClientHeight.

 [ History ]

 1.0 () First Release.

 [ Modifications ]

 * (25.03.2003) Scrolling doesn't calls now Control.ScrollBy so slight
   flicker is removed
 * (31.03.2003) Setting pagesize now works!

 [ Known Issues ]
 None

 Based on Borland's Forms.pas source.
 Copyright (c) 1995,99 Inprise Corporation
-----------------------------------------------------------------------------}


unit VertSB;

interface

uses
  Classes, Forms, Graphics, Messages, Controls, Math, Windows;

type

{ TVertScrollBar }

  TScrollBarKind = (sbHorizontal, sbVertical);
  TScrollBarInc = 1..32767;
  TScrollBarStyle = (ssRegular, ssFlat, ssHotTrack);

  TVertScrollBar = class(TPersistent)
  private
    FControl: TScrollingWinControl;
    FIncrement: TScrollBarInc;
    FPageIncrement: TScrollbarInc;
    FPosition: Integer;
    FRange: Integer;
    FCalcRange: Integer;
    FKind: TScrollBarKind;
    FMargin: Word;
    FVisible: Boolean;
    FTracking: Boolean;
    FPageSize: Integer;
    FScaled: Boolean;
    FSmooth: Boolean;
    FDelay: Integer;
    FButtonSize: Integer;
    FColor: TColor;
    FParentColor: Boolean;
    FSize: Integer;
    FStyle: TScrollBarStyle;
    FThumbSize: Integer;
    FPageDiv: Integer;
    FLineDiv: Integer;
    FUpdatingScrollBars: Boolean;
    FUpdateNeeded: Boolean;
    FHidden: Boolean;
//    procedure CalcAutoRange;
    function ControlSize(ControlSB, AssumeSB: Boolean): Integer;
    procedure DoSetRange(Value: Integer);
    function GetScrollPos: Integer;
    function NeedsScrollBarVisible: Boolean;
    function IsIncrementStored: Boolean;
    procedure SetButtonSize(Value: Integer);
    procedure SetColor(Value: TColor);
    procedure SetParentColor(Value: Boolean);
    procedure SetPosition(Value: Integer);
    procedure SetRange(Value: Integer);
    procedure SetSize(Value: Integer);
    procedure SetStyle(Value: TScrollBarStyle);
    procedure SetThumbSize(Value: Integer);
    procedure SetVisible(Value: Boolean);
    function IsRangeStored: Boolean;
    procedure Update(ControlSB, AssumeSB: Boolean);
    procedure WINUpdateScrollBars;
    procedure SetHidden(Value: Boolean);
  public
    constructor Create(AControl: TScrollingWinControl; AKind: TScrollBarKind);
    procedure Assign(Source: TPersistent); override;
    procedure ChangeBiDiPosition;
    property Kind: TScrollBarKind read FKind;
    function IsScrollBarVisible: Boolean;
    property ScrollPos: Integer read GetScrollPos;
    procedure ScrollMessage(var Msg: TWMScroll);
  published
    property ButtonSize: Integer read FButtonSize write SetButtonSize default 0;
    property Color: TColor read FColor write SetColor default clBtnHighlight;
    property Increment: TScrollBarInc read FIncrement write FIncrement stored IsIncrementStored default 8;
    property Margin: Word read FMargin write FMargin default 0;
    property ParentColor: Boolean read FParentColor write SetParentColor default True;
    property Position: Integer read FPosition write SetPosition default 0;
    property Range: Integer read FRange write SetRange stored IsRangeStored default 0;
    property Smooth: Boolean read FSmooth write FSmooth default False;
    property Size: Integer read FSize write SetSize default 0;
    property Style: TScrollBarStyle read FStyle write SetStyle default ssRegular;
    property ThumbSize: Integer read FThumbSize write SetThumbSize default 0;
    property Tracking: Boolean read FTracking write FTracking default True;
    property Visible: Boolean read FVisible write SetVisible default True;
    property PageSize: Integer read FPageSize write FPageSize default 20;
    property Hidden: Boolean read FHidden write SetHidden default False;
  end;

implementation

uses FlatSB, CommCtrl;

{ TVertScrollBar }

procedure TVertScrollBar.WINUpdateScrollBars;
begin
  if not FUpdatingScrollBars and FControl.HandleAllocated then
    try
      FUpdatingScrollBars := True;
      if NeedsScrollBarVisible{OXY: FControl.VertScrollBar.NeedsScrollBarVisible} then
      begin
        //FHorzScrollBar.Update(False, True);
        Update(True, False);
      end
      else if False {OXY: FHorzScrollBar.NeedsScrollBarVisible} then
      begin
        Update(False, True);
        //FHorzScrollBar.Update(True, False);
      end
      else
      begin
        Update(False, False);
        //FHorzScrollBar.Update(True, False);
      end;
    finally
      FUpdatingScrollBars := False;
    end;
end;

constructor TVertScrollBar.Create(AControl: TScrollingWinControl; AKind: TScrollBarKind);
begin
  inherited Create;
  FPageSize := 20;
  FControl := AControl;
  FKind := AKind;
  FTracking := True;
  FPageIncrement := 80;
  FUpdatingScrollBars := False;
  FIncrement := FPageIncrement div 10;
  FVisible := True;
  FDelay := 10;
  FLineDiv := 1;
  FPageDiv := 1;
  FColor := clBtnHighlight;
  FParentColor := True;
  FUpdateNeeded := True;
  FHidden := False;
end;

function TVertScrollBar.IsIncrementStored: Boolean;
begin
  Result := not Smooth;
end;

procedure TVertScrollBar.Assign(Source: TPersistent);
begin
  if Source is TVertScrollBar then
  begin
    Visible   := TVertScrollBar(Source).Visible;
    Range     := TVertScrollBar(Source).Range;
    Position  := TVertScrollBar(Source).Position;
    Increment := TVertScrollBar(Source).Increment;
    Exit;
  end;
  inherited Assign(Source);
end;

procedure TVertScrollBar.ChangeBiDiPosition;
begin
  if Kind = sbHorizontal then
    if IsScrollBarVisible then
      if FControl.UseRightToLeftScrollBar then
        Position := 0
      else
        Position := Range;
end;
(*
procedure TVertScrollBar.CalcAutoRange;
var
  NewRange, AlignMargin: Integer;

  procedure ProcessHorz(Control: TControl);
  begin
    if Control.Visible then
      case Control.Align of
        alLeft, alNone:
          if (Control.Align = alLeft) or (Control.Anchors * [akLeft, akRight] = [akLeft]) then
            NewRange := Max(NewRange, Position + Control.Left + Control.Width);
        alRight: Inc(AlignMargin, Control.Width);
      end;
  end;

  procedure ProcessVert(Control: TControl);
  begin
    if Control.Visible then
      case Control.Align of
        alTop, alNone:
          if (Control.Align = alTop) or (Control.Anchors * [akTop, akBottom] = [akTop]) then
            NewRange := Max(NewRange, Position + Control.Top + Control.Height);
        alBottom: Inc(AlignMargin, Control.Height);
      end;
  end;

var
  i: Integer;
begin
  if False {OXY: FControl.AutoScroll } then
  begin
    if False {OXY: FControl.AutoScrollEnabled } then
    begin
      NewRange := 0;
      AlignMargin := 0;
      for i := 0 to FControl.ControlCount - 1 do
        if Kind = sbHorizontal then
          ProcessHorz(FControl.Controls[I]) else
          ProcessVert(FControl.Controls[I]);
      DoSetRange(NewRange + AlignMargin + Margin);
    end
    else DoSetRange(0);
  end;
end;
*)

function TVertScrollBar.IsScrollBarVisible: Boolean;
var
  Style: Longint;
begin
  Style := WS_HSCROLL;
  if Kind = sbVertical then Style := WS_VSCROLL;
  Result := (Visible) and
            (GetWindowLongPtr(FControl.Handle, GWL_STYLE) and Style <> 0);
end;

function TVertScrollBar.ControlSize(ControlSB, AssumeSB: Boolean): Integer;
var
  BorderAdjust: Integer;

  function ScrollBarVisible(Code: Word): Boolean;
  var
    Style: Longint;
  begin
    Style := WS_HSCROLL;
    if Code = SB_VERT then Style := WS_VSCROLL;
    Result := GetWindowLongPtr(FControl.Handle, GWL_STYLE) and Style <> 0;
  end;

  function Adjustment(Code, Metric: Word): Integer;
  begin
    Result := 0;
    if not ControlSB then
      if AssumeSB and not ScrollBarVisible(Code) then
        Result := -(GetSystemMetrics(Metric) - BorderAdjust)
      else if not AssumeSB and ScrollBarVisible(Code) then
        Result := GetSystemMetrics(Metric) - BorderAdjust;
  end;

begin
  BorderAdjust := Integer(GetWindowLongPtr(FControl.Handle, GWL_STYLE) and
    (WS_BORDER or WS_THICKFRAME) <> 0);
  if Kind = sbVertical then
    Result := FControl.ClientHeight + Adjustment(SB_HORZ, SM_CXHSCROLL)
  else
    Result := FControl.ClientWidth + Adjustment(SB_VERT, SM_CYVSCROLL);
end;

function TVertScrollBar.GetScrollPos: Integer;
begin
  Result := 0;
  if Visible then Result := Position;
end;

function TVertScrollBar.NeedsScrollBarVisible: Boolean;
begin
  Result := FRange > ControlSize(False, False);
end;

procedure TVertScrollBar.ScrollMessage(var Msg: TWMScroll);
var
  Incr, FinalIncr, Count: Integer;
  CurrentTime, StartTime, ElapsedTime: Longint;

  function GetRealScrollPosition: Integer;
  var
    SI: TScrollInfo;
    Code: Integer;
  begin
    SI.cbSize := SizeOf(TScrollInfo);
    SI.fMask := SIF_TRACKPOS;
    Code := SB_HORZ;
    if FKind = sbVertical then Code := SB_VERT;
    Result := Msg.Pos;
    if FlatSB_GetScrollInfo(FControl.Handle, Code, SI) then
      Result := SI.nTrackPos;
  end;

begin
  with Msg do
  begin
    if FSmooth and (ScrollCode in [SB_LINEUP, SB_LINEDOWN, SB_PAGEUP, SB_PAGEDOWN]) then
    begin
      case ScrollCode of
        SB_LINEUP, SB_LINEDOWN:
          begin
            Incr := FIncrement div FLineDiv;
            FinalIncr := FIncrement mod FLineDiv;
            Count := FLineDiv;
          end;
        SB_PAGEUP, SB_PAGEDOWN:
          begin
            Incr := FPageIncrement;
            FinalIncr := Incr mod FPageDiv;
            Incr := Incr div FPageDiv;
            Count := FPageDiv;
          end;
      else
        Count := 0;
        Incr := 0;
        FinalIncr := 0;
      end;
      CurrentTime := 0;
      while Count > 0 do
      begin
        StartTime := GetCurrentTime;
        ElapsedTime := StartTime - CurrentTime;
        if ElapsedTime < FDelay then Sleep(FDelay - ElapsedTime);
        CurrentTime := StartTime;
        case ScrollCode of
          SB_LINEUP  : SetPosition(FPosition - Incr);
          SB_LINEDOWN: SetPosition(FPosition + Incr);
          SB_PAGEUP  : SetPosition(FPosition - Incr);
          SB_PAGEDOWN: SetPosition(FPosition + Incr);
        end;
        FControl.Update;
        Dec(Count);
      end;
      if FinalIncr > 0 then
      begin
        case ScrollCode of
          SB_LINEUP  : SetPosition(FPosition - FinalIncr);
          SB_LINEDOWN: SetPosition(FPosition + FinalIncr);
          SB_PAGEUP  : SetPosition(FPosition - FinalIncr);
          SB_PAGEDOWN: SetPosition(FPosition + FinalIncr);
        end;
      end;
    end
    else
      case ScrollCode of
        SB_LINEUP    : SetPosition(FPosition - FIncrement);
        SB_LINEDOWN  : SetPosition(FPosition + FIncrement);
        SB_PAGEUP  : SetPosition(FPosition - FPageSize);
        SB_PAGEDOWN: SetPosition(FPosition + FPageSize);
        SB_THUMBPOSITION:
          if FCalcRange > 32767 then
            SetPosition(GetRealScrollPosition)
          else
            SetPosition(Pos);
        SB_THUMBTRACK:
          if Tracking then
            if FCalcRange > 32767 then
              SetPosition(GetRealScrollPosition)
            else
              SetPosition(Pos);
        SB_TOP: SetPosition(0);
        SB_BOTTOM: SetPosition(FCalcRange);
        SB_ENDSCROLL: begin end;
      end;
  end;
end;

procedure TVertScrollBar.SetButtonSize(Value: Integer);
const
  SysConsts: array[TScrollBarKind] of Integer = (SM_CXHSCROLL, SM_CXVSCROLL);
var
  NewValue: Integer;
begin
  if Value <> ButtonSize then
  begin
    NewValue := Value;
    if NewValue = 0 then
      Value := GetSystemMetrics(SysConsts[Kind]);
    FButtonSize := Value;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
    if NewValue = 0 then
      FButtonSize := 0;
  end;
end;

procedure TVertScrollBar.SetColor(Value: TColor);
begin
  if Value <> Color then
  begin
    FColor := Value;
    FParentColor := False;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
  end;
end;

procedure TVertScrollBar.SetParentColor(Value: Boolean);
begin
  if ParentColor <> Value then
  begin
    FParentColor := Value;
    if Value then Color := clBtnHighlight;
  end;
end;

procedure TVertScrollBar.SetPosition(Value: Integer);
var
  Code: Word;
  Form: TCustomForm;
//  OldPos: Integer;
begin
  if csReading in FControl.ComponentState then
    FPosition := Value
  else
  begin
    if Value > FCalcRange then Value := FCalcRange
    else if Value < 0 then Value := 0;
    if Kind = sbHorizontal then
      Code := SB_HORZ
    else
      Code := SB_VERT;
    if Value <> FPosition then
    begin
//      OldPos := FPosition;
      FPosition := Value;
      {OXY:
      if Kind = sbHorizontal then
        FControl.ScrollBy(OldPos - Value, 0) else
        FControl.ScrollBy(0, OldPos - Value);
      }
      if csDesigning in FControl.ComponentState then
      begin
        Form := GetParentForm(FControl);
        if (Form <> nil) and (Form.Designer <> nil) then
          Form.Designer.Modified;
      end;
    end;
    if FlatSB_GetScrollPos(FControl.Handle, Code) <> FPosition then
      FlatSB_SetScrollPos(FControl.Handle, Code, FPosition, True);
  end;
end;

procedure TVertScrollBar.SetSize(Value: Integer);
const
  SysConsts: array[TScrollBarKind] of Integer = (SM_CYHSCROLL, SM_CYVSCROLL);
var
  NewValue: Integer;
begin
  if Value <> Size then
  begin
    NewValue := Value;
    if NewValue = 0 then
      Value := GetSystemMetrics(SysConsts[Kind]);
    FSize := Value;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
    if NewValue = 0 then
      FSize := 0;
  end;
end;

procedure TVertScrollBar.SetStyle(Value: TScrollBarStyle);
begin
  if Style <> Value then
  begin
    FStyle := Value;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
  end;
end;

procedure TVertScrollBar.SetThumbSize(Value: Integer);
begin
  if Value <> ThumbSize then
  begin
    FThumbSize := Value;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
  end;
end;

procedure TVertScrollBar.DoSetRange(Value: Integer);
begin
  FRange := Value;
  if FRange < 0 then FRange := 0;
  WINUpdateScrollBars;
end;

procedure TVertScrollBar.SetRange(Value: Integer);
begin
  //OXY: FControl.FAutoScroll := False;
  FScaled := True;
  DoSetRange(Value);
end;

function TVertScrollBar.IsRangeStored: Boolean;
begin
  Result := not False;// OXY: FControl.AutoScroll;
end;

procedure TVertScrollBar.SetVisible(Value: Boolean);
begin
  FVisible := Value;
  WINUpdateScrollBars;
end;

procedure TVertScrollBar.Update(ControlSB, AssumeSB: Boolean);
type
  TPropKind = (pkStyle, pkButtonSize, pkThumbSize, pkSize, pkBkColor);
const
  Props: array[TScrollBarKind, TPropKind] of Integer = (
    { Horizontal }
    (WSB_PROP_HSTYLE, WSB_PROP_CXHSCROLL, WSB_PROP_CXHTHUMB, WSB_PROP_CYHSCROLL,
     WSB_PROP_HBKGCOLOR),
    { Vertical }
    (WSB_PROP_VSTYLE, WSB_PROP_CYVSCROLL, WSB_PROP_CYVTHUMB, WSB_PROP_CXVSCROLL,
     WSB_PROP_VBKGCOLOR));
  Kinds: array[TScrollBarKind] of Integer = (WSB_PROP_HSTYLE, WSB_PROP_VSTYLE);
  Styles: array[TScrollBarStyle] of Integer = (FSB_REGULAR_MODE,
    FSB_ENCARTA_MODE, FSB_FLAT_MODE);
var
  Code: Word;
  ScrollInfo: TScrollInfo;

  procedure UpdateScrollProperties(Redraw: Boolean);
  begin
    FlatSB_SetScrollProp(FControl.Handle, Props[Kind, pkStyle], Styles[Style], Redraw);
    if ButtonSize > 0 then
      FlatSB_SetScrollProp(FControl.Handle, Props[Kind, pkButtonSize], ButtonSize, False);
    if ThumbSize > 0 then
      FlatSB_SetScrollProp(FControl.Handle, Props[Kind, pkThumbSize], ThumbSize, False);
    if Size > 0 then
      FlatSB_SetScrollProp(FControl.Handle, Props[Kind, pkSize], Size, False);
    FlatSB_SetScrollProp(FControl.Handle, Props[Kind, pkBkColor],
      ColorToRGB(Color), False);
  end;

begin
  FCalcRange := 0;
  Code := SB_HORZ;
  if Kind = sbVertical then Code := SB_VERT;
  if Visible then
  begin
    FCalcRange := Range - FPageSize + 1;
    if FCalcRange < 0 then FCalcRange := 0;
  end;
  ScrollInfo.cbSize := SizeOf(ScrollInfo);
  ScrollInfo.fMask := SIF_ALL;
  ScrollInfo.nMin := 0;
  if not Hidden and (FCalcRange > 0) then
    ScrollInfo.nMax := Range else
    ScrollInfo.nMax := 0;
  //if Hidden then
  //  ScrollInfo.nPage := ScrollInfo.nMax+1
  //else
  ScrollInfo.nPage := FPageSize;
  ScrollInfo.nPos := FPosition;
  ScrollInfo.nTrackPos := FPosition;
  UpdateScrollProperties(FUpdateNeeded);
  FUpdateNeeded := False;
  FlatSB_SetScrollInfo(FControl.Handle, Code, ScrollInfo, True);
  SetPosition(FPosition);
  FPageIncrement := (FPageSize+1 * 9) div 10;
  if Smooth then FIncrement := FPageIncrement div 10;
end;

procedure TVertScrollBar.SetHidden(Value: Boolean);
begin
  if Hidden <> Value then
  begin
    FHidden := Value;
    FUpdateNeeded := True;
    WINUpdateScrollBars;
  end;
end;

end.
