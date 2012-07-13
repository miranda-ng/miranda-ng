(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

unit CustomizeFiltersForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,Dialogs,
  HistoryControls,m_api,
  StdCtrls, CheckLst,
  hpp_eventfilters, ExtCtrls;

type
  TfmCustomizeFilters = class(TForm)
    bnOK: TButton;
    bnCancel: TButton;
    gbFilter: THppGroupBox;
    edFilterName: THppEdit;
    clEvents: TCheckListBox;
    bnReset: TButton;
    rbExclude: TRadioButton;
    rbInclude: TRadioButton;
    gbFilters: THppGroupBox;
    lbFilters: TListBox;
    bnDown: TButton;
    bnUp: TButton;
    bnDelete: TButton;
    bnAdd: TButton;
    laFilterName: TLabel;
    edCustomEvent: THppEdit;
    cbCustomEvent: TCheckBox;
    paClient: TPanel;
    procedure FormCreate(Sender: TObject);
    procedure bnOKClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormDestroy(Sender: TObject);
    procedure lbFiltersClick(Sender: TObject);
    procedure edFilterNameChange(Sender: TObject);
    procedure bnAddClick(Sender: TObject);
    procedure bnCancelClick(Sender: TObject);
    procedure bnUpClick(Sender: TObject);
    procedure bnDownClick(Sender: TObject);
    procedure bnDeleteClick(Sender: TObject);
    procedure clEventsClickCheck(Sender: TObject);
    procedure bnResetClick(Sender: TObject);
    procedure rbPropertyClick(Sender: TObject);
    procedure lbFiltersDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure lbFiltersDragOver(Sender, Source: TObject; X, Y: Integer;
      State: TDragState; var Accept: Boolean);
    procedure clEventsDrawItem(Control: TWinControl; Index: Integer;
      Rect: TRect; State: TOwnerDrawState);
    procedure lbFiltersDrawItem(Control: TWinControl; Index: Integer;
      Rect: TRect; State: TOwnerDrawState);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure cbCustomEventClick(Sender: TObject);
    procedure edCustomEventChange(Sender: TObject);
    procedure edEditKeyPress(Sender: TObject; var Key: Char);
  private
    LocalFilters: ThppEventFilterArray;

    IncOutWrong: Boolean;
    EventsWrong: Boolean;
    EventsHeaderIndex: Integer;

    DragOverIndex: Integer;

    procedure LoadLocalFilters;
    procedure SaveLocalFilters;
    procedure FillFiltersList;
    procedure FillEventsCheckListBox;

    procedure MoveItem(Src,Dst: Integer);
    procedure UpdateEventsState;
    procedure UpdateUpDownButtons;

    procedure TranslateForm;
  public
    { Public declarations }
  end;

var
  fmCustomizeFilters: TfmCustomizeFilters = nil;

implementation

uses
  hpp_forms, hpp_global, hpp_events, HistoryForm, {hpp_options,}
  TypInfo, Math, GlobalSearch;

const
  // commented to use all events in system history
  //IgnoreEvents: TMessageTypes = [mtSystem, mtWebPager, mtEmailExpress];
  IgnoreEvents: TMessageTypes = [];

{$R *.dfm}

procedure TfmCustomizeFilters.bnAddClick(Sender: TObject);
var
  NewNameFmt,NewName: String;
  NameExists: Boolean;
  num,i: Integer;
begin
  NewNameFmt := TranslateW('New Filter #%d');
  num := 1;
  while True do begin
    NewName := Format(NewNameFmt,[num]);
    NameExists := False;
    for i := 0 to Length(LocalFilters) - 1 do
      if NewName = LocalFilters[i].Name then
      begin
        NameExists := true;
        break;
      end;
    if not NameExists then break;
    Inc(num);
  end;

  i := Length(LocalFilters);
  SetLength(LocalFilters,i+1);
  LocalFilters[i].Name := NewName;
  LocalFilters[i].filMode := FM_INCLUDE;
  LocalFilters[i].filEvents := [mtIncoming,mtOutgoing,mtMessage,mtUrl,mtFile];
  LocalFilters[i].Events := GenerateEvents(LocalFilters[i].filMode,LocalFilters[i].filEvents);

  lbFilters.Items.Add(NewName);
  lbFilters.ItemIndex := i;
  lbFiltersClick(Self);
  if edFilterName.CanFocus then edFilterName.SetFocus;
end;

procedure TfmCustomizeFilters.bnCancelClick(Sender: TObject);
begin
  Close;
end;

procedure TfmCustomizeFilters.bnDeleteClick(Sender: TObject);
var
  n,i: Integer;
begin
  if lbFilters.ItemIndex = -1 then exit;
  n := lbFilters.ItemIndex;
  if (LocalFilters[n].filMode = FM_EXCLUDE) and
     (LocalFilters[n].filEvents = []) then
    exit; // don't delete Show All Events

  for i := n to Length(LocalFilters) - 2 do
    LocalFilters[i] := LocalFilters[i+1];
  SetLength(LocalFilters,Length(LocalFilters)-1);
  lbFilters.DeleteSelected;
  if n >= lbFilters.Count then
    Dec(n);
  lbFilters.ItemIndex := n;
  lbFiltersClick(Self);
end;

procedure TfmCustomizeFilters.bnDownClick(Sender: TObject);
var
  i: Integer;
begin
  if lbFilters.ItemIndex = -1 then exit;
  if lbFilters.ItemIndex = lbFilters.Count-1 then exit;
  i := lbFilters.ItemIndex;
  MoveItem(i,i+1);
end;

procedure TfmCustomizeFilters.bnOKClick(Sender: TObject);
begin
  SaveLocalFilters;
  Close;
end;

procedure TfmCustomizeFilters.bnResetClick(Sender: TObject);
begin
  CopyEventFilters(hppDefEventFilters,LocalFilters);

  FillFiltersList;
  FillEventsCheckListBox;

  SaveLocalFilters;

  if lbFilters.Items.Count > 0 then lbFilters.ItemIndex := 0;
  lbFiltersClick(Self);
end;

procedure TfmCustomizeFilters.bnUpClick(Sender: TObject);
var
  i: Integer;
begin
  if lbFilters.ItemIndex = -1 then exit;
  if lbFilters.ItemIndex = 0 then exit;
  i := lbFilters.ItemIndex;
  MoveItem(i,i-1);
end;

procedure TfmCustomizeFilters.clEventsClickCheck(Sender: TObject);
var
  n,i: Integer;
begin
  UpdateEventsState;
  if EventsWrong or IncOutWrong then exit;
  n := lbFilters.ItemIndex;
  if rbInclude.Checked then
    LocalFilters[n].filMode := FM_INCLUDE
  else
    LocalFilters[n].filMode := FM_EXCLUDE;
  LocalFilters[n].filEvents := [];
  for i := 0 to clEvents.Count - 1 do
  begin
    if clEvents.Header[i] then continue;
    if clEvents.Checked[i] then
      Include(LocalFilters[n].filEvents,TMessageType(Integer(clEvents.Items.Objects[i])));
  end;
  if cbCustomEvent.Checked then
    LocalFilters[n].filEvents := LocalFilters[n].filEvents + EventsCustom;
  LocalFilters[n].Events := GenerateEvents(LocalFilters[n].filMode,LocalFilters[n].filEvents);
end;

procedure TfmCustomizeFilters.clEventsDrawItem(Control: TWinControl;
  Index: Integer; Rect: TRect; State: TOwnerDrawState);
var
  txtW: String;
  r: TRect;
  tf: DWord;
  BrushColor: TColor;
begin
  BrushColor := clEvents.Canvas.Brush.Color;
  txtW := clEvents.Items[Index];
  r := Rect;
  tf := DT_SINGLELINE or DT_VCENTER or DT_NOPREFIX;
  InflateRect(r,-2,0);

  if clEvents.Header[Index] then
  begin
    if (EventsWrong) and (Index = EventsHeaderIndex) then
      if BrushColor = clEvents.HeaderBackgroundColor then clEvents.Canvas.Brush.Color := $008080FF;
    if (IncOutWrong) and (Index <> EventsHeaderIndex) then
      if BrushColor = clEvents.HeaderBackgroundColor then clEvents.Canvas.Brush.Color := $008080FF;
    clEvents.Canvas.FillRect(Rect);
    DrawText(clEvents.Canvas.Handle,PChar(txtW),Length(txtW),r,tf);
    clEvents.Canvas.Brush.Color := BrushColor;
    exit;
  end;

  if (EventsWrong) and (Index > EventsHeaderIndex) then
    if BrushColor = clEvents.Color then clEvents.Canvas.Brush.Color := $008080FF;
  if (IncOutWrong) and (Index < EventsHeaderIndex) then
    if BrushColor = clEvents.Color then clEvents.Canvas.Brush.Color := $008080FF;
  clEvents.Canvas.FillRect(Rect);
  DrawText(clEvents.Canvas.Handle,PChar(txtW),Length(txtW),r,tf);
  clEvents.Canvas.Brush.Color := BrushColor;
end;

procedure TfmCustomizeFilters.edFilterNameChange(Sender: TObject);
begin
  if lbFilters.ItemIndex = -1 then exit;
  if edFilterName.Text = '' then
    edFilterName.Color := $008080FF
  else
    edFilterName.Color := clWindow;
  if edFilterName.Text <> '' then
    LocalFilters[lbFilters.ItemIndex].Name := edFilterName.Text;
  lbFilters.Items.BeginUpdate;
  lbFilters.Items[lbFilters.ItemIndex] := LocalFilters[lbFilters.ItemIndex].Name;
  lbFilters.Items.EndUpdate;
end;

procedure TfmCustomizeFilters.FillEventsCheckListBox;
var
  mt: TMessageType;
  mt_name, pretty_name: String;
  i: Integer;
begin
  clEvents.Items.BeginUpdate;
  clEvents.Items.Clear;

  // add all types except mtOther (we'll add it at the end) and
  // message types in AlwaysExclude and AlwaysInclude
  for mt := Low(TMessageType) to High(TMessageType) do begin
    if (mt in EventsExclude) or (mt in EventsInclude) or (mt in IgnoreEvents) then continue;
    if mt = mtOther then continue; // we'll add mtOther at the end
    if mt in [mtIncoming,mtMessage] then begin // insert header before incoming and message
      if mt = mtIncoming then
        mt_name := TranslateW('Incoming & Outgoing')
      else
        mt_name := TranslateW('Events');
      i := clEvents.Items.Add(mt_name);
      EventsHeaderIndex := i;
      clEvents.Header[i] := True;
    end;

    //pretty_name := GetEnumName(TypeInfo(TMessageType),Ord(mt));
    //Delete(pretty_name,1,2);
    // find filter names if we have substitute
    //for i := 0 to Length(FilterNames) - 1 do
    //  if FilterNames[i].mt = mt then begin
    //    pretty_name := FilterNames[i].Name;
    //    break;
    //  end;
    pretty_name := TranslateUnicodeString(EventRecords[mt].Name{TRANSLATE-IGNORE});
    clEvents.Items.AddObject(pretty_name,Pointer(Ord(mt)));
  end;

  // add mtOther at the end
  mt := mtOther;
  //pretty_name := GetEnumName(TypeInfo(TMessageType),Ord(mt));
  //Delete(pretty_name,1,2);
  // find filter names if we have substitute
  //for i := 0 to Length(FilterNames) - 1 do
  //  if FilterNames[i].mt = mt then begin
  //    pretty_name := FilterNames[i].Name;
  //    break;
  //  end;
  pretty_name := TranslateUnicodeString(EventRecords[mt].Name{TRANSLATE-IGNORE});
  clEvents.Items.AddObject(pretty_name,Pointer(Ord(mt)));
  clEvents.Items.EndUpdate;
end;

procedure TfmCustomizeFilters.FillFiltersList;
var
  i: Integer;
begin
  lbFilters.Items.BeginUpdate;
  lbFilters.Items.Clear;
  for i := 0 to Length(LocalFilters) - 1 do
  begin
    lbFilters.Items.Add(LocalFilters[i].Name);
  end;
  //meEvents.Lines.Clear;
  lbFilters.Items.EndUpdate;
end;

procedure TfmCustomizeFilters.FormCreate(Sender: TObject);
begin
  fmCustomizeFilters := Self;

  DesktopFont := True;
  MakeFontsParent(Self);
  DoubleBuffered := True;
  MakeDoubleBufferedParent(Self);

  TranslateForm;

  LoadLocalFilters;
  FillFiltersList;
  FillEventsCheckListBox;

  if lbFilters.Items.Count > 0 then lbFilters.ItemIndex := 0;
  lbFiltersClick(Self);
  edFilterName.MaxLength := MAX_FILTER_NAME_LENGTH;
end;

procedure TfmCustomizeFilters.lbFiltersClick(Sender: TObject);
var
  i: Integer;
  Lock: Boolean;
begin
  if lbFilters.ItemIndex = -1 then exit;
  Lock := false;
  if Visible then Lock := LockWindowUpdate(Handle);
  try
    rbInclude.Checked := (LocalFilters[lbFilters.ItemIndex].filMode = FM_INCLUDE);
    rbExclude.Checked := (LocalFilters[lbFilters.ItemIndex].filMode = FM_EXCLUDE);
    for i := 0 to clEvents.Items.Count - 1 do begin
      if clEvents.Header[i] then continue;
      clEvents.Checked[i] := TMessageType(Pointer(clEvents.Items.Objects[i])) in LocalFilters[lbFilters.ItemIndex].filEvents;
    end;

    cbCustomEvent.Checked := (LocalFilters[lbFilters.ItemIndex].filEvents*EventsCustom = EventsCustom);
    edCustomEvent.Text := IntToStr(LocalFilters[lbFilters.ItemIndex].filCustom);

    edFilterName.Text := lbFilters.Items[lbFilters.ItemIndex];

    edFilterName.Enabled := (lbFilters.ItemIndex <> GetShowAllEventsIndex(LocalFilters));
    laFilterName.Enabled  := edFilterName.Enabled;
    rbInclude.Enabled     := edFilterName.Enabled;
    rbExclude.Enabled     := edFilterName.Enabled;
    clEvents.Enabled      := edFilterName.Enabled;
    cbCustomEvent.Enabled := edFilterName.Enabled;
    edCustomEvent.Enabled := edFilterName.Enabled and cbCustomEvent.Checked;
    bnDelete.Enabled := edFilterName.Enabled;
  finally
    UpdateUpDownButtons;
    UpdateEventsState;
    if Visible and Lock then LockWindowUpdate(0);
  end;
end;

procedure TfmCustomizeFilters.lbFiltersDragDrop(Sender, Source: TObject; X, Y: Integer);
var
  src,dst: Integer;
begin
  // we insert always *before* droped item, unless we drop on the empty area
  // in this case be insert dragged item at the end
  dst := lbFilters.ItemAtPos(Point(x,y),False);
  src := lbFilters.ItemIndex;
  if src = dst then exit;
  if src < dst then Dec(dst);
  if src = dst then exit;
  MoveItem(src,dst);
end;

procedure TfmCustomizeFilters.lbFiltersDragOver(Sender, Source: TObject; X,
  Y: Integer; State: TDragState; var Accept: Boolean);
var
  r: TRect;
  idx: Integer;
begin
  Accept := True;
  idx := DragOverIndex;
  if idx = lbFilters.Count then Dec(idx);
  r := lbFilters.ItemRect(idx);
  DragOverIndex := lbFilters.ItemAtPos(Point(x,y),False);
  InvalidateRect(lbFilters.Handle,@r,False);
  idx := DragOverIndex;
  if idx = lbFilters.Count then Dec(idx);
  r := lbFilters.ItemRect(idx);
  InvalidateRect(lbFilters.Handle,@r,False);
  lbFilters.Update;
end;

procedure TfmCustomizeFilters.lbFiltersDrawItem(Control: TWinControl;
  Index: Integer; Rect: TRect; State: TOwnerDrawState);
var
  BrushColor: TColor;
  txtW: String;
  r: TRect;
  tf: DWord;
  {src,}dst: Integer;
begin
  BrushColor := lbFilters.Canvas.Brush.Color;
  txtW := lbFilters.Items[Index];
  r := Rect;
  InflateRect(r,-2,0);
  lbFilters.Canvas.FillRect(Rect);
  tf := DT_SINGLELINE or DT_VCENTER or DT_NOPREFIX;
  DrawText(lbFilters.Canvas.Handle,PChar(txtW),Length(txtW),r,tf);
  if lbFilters.Dragging then begin
//    src := lbFilters.ItemIndex;
    dst := DragOverIndex;
    if (dst = lbFilters.Count) and (Index = lbFilters.Count-1) then
    begin
      lbFilters.Canvas.Brush.Color := clHighlight;
      r := Classes.Rect(Rect.Left,Rect.Bottom-1,Rect.Right,Rect.Bottom);
      lbFilters.Canvas.FillRect(r);
    end;
    if (dst = Index) then
    begin
      lbFilters.Canvas.Brush.Color := clHighlight;
      r := Classes.Rect(Rect.Left,Rect.Top,Rect.Right,Rect.Top+1);
      lbFilters.Canvas.FillRect(r);
    end;
  end;
  lbFilters.Canvas.Brush.Color := BrushColor;
end;

procedure TfmCustomizeFilters.LoadLocalFilters;
begin
  CopyEventFilters(hppEventFilters,LocalFilters);
end;

procedure TfmCustomizeFilters.MoveItem(Src, Dst: Integer);
var
  ef: ThppEventFilter;
  i: Integer;
begin
  if Src = Dst then exit;

  lbFilters.Items.Move(Src,Dst);

  ef := LocalFilters[Src];
  if Dst > Src then
    for i := Src to Dst-1 do
      LocalFilters[i] := LocalFilters[i+1]
  else
    for i := Src downto Dst+1 do
      LocalFilters[i] := LocalFilters[i-1];
  LocalFilters[Dst] := ef;

  lbFilters.ItemIndex := Dst;
  UpdateUpDownButtons;
end;

procedure TfmCustomizeFilters.rbPropertyClick(Sender: TObject);
var
  n: Integer;
begin
  n := lbFilters.ItemIndex;
  UpdateEventsState;
  if IncOutWrong or EventsWrong then exit;
  if rbInclude.Checked then
    LocalFilters[n].filMode := FM_INCLUDE
  else
    LocalFilters[n].filMode := FM_EXCLUDE;
  LocalFilters[n].Events := GenerateEvents(LocalFilters[n].filMode,LocalFilters[n].filEvents);
end;

procedure TfmCustomizeFilters.SaveLocalFilters;
begin
  CopyEventFilters(LocalFilters,hppEventFilters);
  WriteEventFilters;
end;

procedure TfmCustomizeFilters.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure TfmCustomizeFilters.FormDestroy(Sender: TObject);
begin
  fmCustomizeFilters := nil;
  try
    if Owner is THistoryFrm then
      THistoryFrm(Owner).CustomizeFiltersForm := nil
    else if Owner is TfmGlobalSearch then
      TfmGlobalSearch(Owner).CustomizeFiltersForm := nil;
  except
    // "eat" exceptions if any
  end;
end;

procedure TfmCustomizeFilters.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  Mask: Integer;
begin
  with Sender as TWinControl do
    begin
      if Perform(CM_CHILDKEY, Key, LPARAM(Sender)) <> 0 then
        Exit;
      Mask := 0;
      case Key of
        VK_TAB:
          Mask := DLGC_WANTTAB;
        VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN:
          // added to change radio buttons from keyboard
          if (Self.ActiveControl is TRadioButton) then Mask := DLGC_WANTARROWS;
        VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
          Mask := DLGC_WANTALLKEYS;
      end;
      if (Mask <> 0)
        and (Perform(CM_WANTSPECIALKEY, Key, 0) = 0)
        and (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0)
        and (Self.Perform(CM_DIALOGKEY, Key, 0) <> 0)
        then Exit;
    end;
end;

procedure TfmCustomizeFilters.TranslateForm;
begin
  Caption               := TranslateUnicodeString(Caption);
  gbFilters.Caption     := TranslateUnicodeString(gbFilters.Caption);
  bnAdd.Caption         := TranslateUnicodeString(bnAdd.Caption);
  bnDelete.Caption      := TranslateUnicodeString(bnDelete.Caption);
  bnUp.Caption          := TranslateUnicodeString(bnUp.Caption);
  bnDown.Caption        := TranslateUnicodeString(bnDown.Caption);
  gbFilter.Caption      := TranslateUnicodeString(gbFilter.Caption);
  laFilterName.Caption  := TranslateUnicodeString(laFilterName.Caption);
  rbInclude.Caption     := TranslateUnicodeString(rbInclude.Caption);
  rbExclude.Caption     := TranslateUnicodeString(rbExclude.Caption);
  cbCustomEvent.Caption := TranslateUnicodeString(cbCustomEvent.Caption);
  bnOK.Caption          := TranslateUnicodeString(bnOK.Caption);
  bnCancel.Caption      := TranslateUnicodeString(bnCancel.Caption);
  bnReset.Caption       := TranslateUnicodeString(bnReset.Caption);
end;

procedure TfmCustomizeFilters.UpdateEventsState;
var
  IncOutChecked,IncOutUnchecked,
  EventsChecked,EventsUnchecked: Boolean;
  InsideEvents: Boolean;
  InsideIncOut: Boolean;
//  HeadEvents: Integer;
  i: Integer;
begin
  if not clEvents.Enabled then begin
    IncOutWrong := False;
    EventsWrong := False;
    bnOK.Enabled := True;
    exit;
  end;
  IncOutChecked := True;
  IncOutUnchecked := True;
  EventsChecked := True;
  EventsUnchecked := True;
  InsideEvents := False;
  InsideIncOut := False;
//  HeadEvents := 0;
  for i := 0 to clEvents.Count - 1 do
  begin

    if clEvents.Header[i] then
    begin
      if InsideIncOut then
      begin
//        HeadEvents := i;
        InsideEvents := True;
      end else
        InsideIncOut := True;
      continue;
    end;

    if InsideEvents then
    begin
      if EventsChecked and (not clEvents.Checked[i]) then
        EventsChecked := False;
      if EventsUnchecked and clEvents.Checked[i] then
        EventsUnchecked := False;
      if (not EventsChecked) and (not EventsUnchecked) then break;
    end
    else
    begin
      if IncOutChecked and (not clEvents.Checked[i]) then
        IncOutChecked := False;
      if IncOutUnchecked and clEvents.Checked[i] then
        IncOutUnchecked := False;
    end;

  end;

  if EventsChecked and not cbCustomEvent.Checked then
    EventsChecked := False;
  if EventsUnchecked and cbCustomEvent.Checked then
    EventsUnchecked := False;

  if rbInclude.Checked then
  begin
    EventsWrong := EventsUnchecked;
    IncOutWrong := IncOutUnchecked;
  end
  else
  begin
    EventsWrong := EventsChecked;
    IncOutWrong := IncOutChecked;
  end;

  // we probably need some help text to show why the filter selection is wrong
  // explanation is given in comments below
  if (rbExclude.Checked) and (EventsUnchecked) and (IncOutUnchecked) then
  begin
    EventsWrong := True;
    IncOutWrong := True;
    // not allowed to duplicate Show All Events filter
  end
  else if (rbInclude.Checked) and (EventsChecked) and (IncOutChecked) then
  begin
    EventsWrong := True;
    IncOutWrong := True;
    // not allowed to quasi-duplicate Show All Events filter
  end
  else
  begin
    if (EventsWrong) or (IncOutWrong) then
      ;// no events will be shown
  end;

  clEvents.Repaint;
  bnOK.Enabled := not (EventsWrong or IncOutWrong);
end;

procedure TfmCustomizeFilters.UpdateUpDownButtons;
begin
  bnUp.Enabled := (lbFilters.ItemIndex <> 0);
  bnDown.Enabled := (lbFilters.ItemIndex <> lbFilters.Count-1);
end;

procedure TfmCustomizeFilters.cbCustomEventClick(Sender: TObject);
begin
  edCustomEvent.Enabled := cbCustomEvent.Checked;
  if lbFilters.ItemIndex = -1 then exit;
  edCustomEvent.Text := IntToStr(LocalFilters[lbFilters.ItemIndex].filCustom);
  clEvents.OnClickCheck(Self);
end;

procedure TfmCustomizeFilters.edCustomEventChange(Sender: TObject);
var
  CustomType: Integer;
begin
  if lbFilters.ItemIndex = -1 then exit;
  if not ((edCustomEvent.Text <> '') and
     TryStrToInt(edCustomEvent.Text,CustomType)) then CustomType := -1;
  if CustomType > $FFFF then
    CustomType := -1;
  if CustomType >= 0 then
  begin
    LocalFilters[lbFilters.ItemIndex].filCustom := Word(CustomType);
    edCustomEvent.Color := clWindow;
  end
  else
    edCustomEvent.Color := $008080FF;
  bnOK.Enabled := (CustomType >= 0);
end;

procedure TfmCustomizeFilters.edEditKeyPress(Sender: TObject; var Key: Char);
begin
  // to prevent ** BLING ** when press Enter
  // to prevent ** BLING ** when press Tab
  // to prevent ** BLING ** when press Esc
  if Ord(Key) in [VK_RETURN,VK_TAB,VK_ESCAPE] then Key := #0;
end;

end.
