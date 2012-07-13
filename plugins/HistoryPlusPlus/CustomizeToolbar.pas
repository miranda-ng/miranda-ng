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

unit CustomizeToolbar;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Dialogs, CommCtrl,
  Forms, ComCtrls, StdCtrls, ExtCtrls, CheckLst, Buttons,
  hpp_global, m_api;

type
  TfmCustomizeToolbar = class(TForm)
    bnAdd: TButton;
    bnRemove: TButton;
    lbAdded: TListBox;
    lbAvailable: TListBox;
    laAvailable: TLabel;
    laAdded: TLabel;
    bnUp: TButton;
    bnDown: TButton;
    Bevel1: TBevel;
    bnOK: TButton;
    bnCancel: TButton;
    bnReset: TButton;
    tiScroll: TTimer;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure lbAvailableDrawItem(Control: TWinControl; Index: Integer; Rect: TRect;
      State: TOwnerDrawState);
    procedure lbAddedDragOver(Sender, Source: TObject; X, Y: Integer; State: TDragState;
      var Accept: Boolean);
    procedure lbAddedDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure lbAvailableDragOver(Sender, Source: TObject; X, Y: Integer; State: TDragState;
      var Accept: Boolean);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure lbAvailableDragDrop(Sender, Source: TObject; X, Y: Integer);
    procedure FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
    procedure bnResetClick(Sender: TObject);
    procedure OnWMChar(var Message: TWMChar); message WM_CHAR;
    procedure bnAddClick(Sender: TObject);
    procedure bnOKClick(Sender: TObject);
    procedure bnCancelClick(Sender: TObject);
    procedure tiScrollTimer(Sender: TObject);
    procedure lbAvailableClick(Sender: TObject);
    procedure bnUpClick(Sender: TObject);
    procedure bnDownClick(Sender: TObject);
    procedure bnRemoveClick(Sender: TObject);
  private
    ItemBmp: TBitmap;
    DragOverIndex: Integer;
    TimerScrollDirection: Integer;
    procedure FillButtons;
    procedure UpdateControlButtons;
    procedure TranslateForm;

    procedure AddItem(src: Integer; dst: Integer = -1);
    procedure RemoveItem(src: Integer);

    function GenerateToolbarString: AnsiString;
    procedure SaveToolbar(ToolbarStr: AnsiString);

    procedure HMIcons2Changed(var M: TMessage); message HM_NOTF_ICONS2CHANGED;
  public
    { Public declarations }
  end;

var
  fmCustomizeToolbar: TfmCustomizeToolbar = nil;

implementation

uses HistoryForm, hpp_database, hpp_options, HistoryControls, hpp_forms;

{$R *.dfm}

procedure TfmCustomizeToolbar.lbAddedDragDrop(Sender, Source: TObject; X, Y: Integer);
var
  src, dst: Integer;
begin
  tiScroll.Enabled := False;
  if Source = lbAvailable then
  begin
    src := lbAvailable.ItemIndex;
    dst := lbAdded.ItemAtPos(Point(X, Y), False);
    AddItem(src, dst);
  end
  else
  begin
    src := lbAdded.ItemIndex;
    dst := lbAdded.ItemAtPos(Point(X, Y), True);
    lbAdded.Items.Move(src, dst);
    lbAdded.ItemIndex := dst;
  end;
  lbAdded.SetFocus;

  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.lbAddedDragOver(Sender, Source: TObject; X, Y: Integer;
  State: TDragState; var Accept: Boolean);
var
  idx: Integer;
  r: TRect;
begin
  Accept := True;

  if (lbAdded.ClientHeight - Y) < 10 then
    TimerScrollDirection := 1
  else if Y < 10 then
    TimerScrollDirection := 2
  else
    TimerScrollDirection := 0;

  tiScroll.Enabled := (TimerScrollDirection <> 0);

  idx := DragOverIndex;
  if idx = lbAdded.Count then
    Dec(idx);
  r := lbAdded.ItemRect(idx);
  InvalidateRect(lbAdded.Handle, @r, False);
  DragOverIndex := lbAdded.ItemAtPos(Point(X, Y), False);
  idx := DragOverIndex;
  if idx = lbAdded.Count then
    Dec(idx);
  r := lbAdded.ItemRect(idx);
  InvalidateRect(lbAdded.Handle, @r, False);
  lbAdded.Update;
end;

procedure TfmCustomizeToolbar.lbAvailableClick(Sender: TObject);
begin
  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.lbAvailableDragDrop(Sender, Source: TObject; X, Y: Integer);
begin
  RemoveItem(lbAdded.ItemIndex);
  lbAvailable.SetFocus;
end;

procedure TfmCustomizeToolbar.lbAvailableDragOver(Sender, Source: TObject; X, Y: Integer;
  State: TDragState; var Accept: Boolean);
begin
  Accept := (Source = lbAdded) and (lbAdded.ItemIndex <> -1);
end;

procedure TfmCustomizeToolbar.lbAvailableDrawItem(Control: TWinControl; Index: Integer;
  Rect: TRect; State: TOwnerDrawState);
var
  txtW: String;
  r: TRect;
  r2: TRect;
  but: THppToolButton;
  fm: THistoryFrm;
  src, dst: Integer;
  lb: TListBox;
  can: TCanvas;
  tf: DWord;
  DrawLineTop, DrawLineBottom: Boolean;
begin
  if Control = lbAdded then
    lb := lbAdded
  else
    lb := lbAvailable;

  ItemBmp.Width := Rect.Right - Rect.Left;
  ItemBmp.Height := Rect.Bottom - Rect.Top;
  can := ItemBmp.Canvas;
  can.Font := lb.Font;

  r := can.ClipRect;
  if (odSelected in State) and (odFocused in State) then
  begin
    can.Brush.Color := clHighlight;
    can.Font.Color := clHighlightText;
  end
  else
  begin
    can.Brush.Color := clWindow;
    can.Font.Color := clWindowText;
  end;

  can.FillRect(r);

  tf := DT_SINGLELINE or DT_VCENTER or DT_NOPREFIX;
  txtW := lb.Items[Index];

  if (odSelected in State) and (not(odFocused in State)) then
  begin
    r2 := r;
    InflateRect(r2, -1, -1);
    can.Pen.Color := clHighlight;
    can.Rectangle(r2);
  end;

  if txtW <> '-' then
  begin
    r2 := r;
    r2.Left := r2.Left + 20 + 4;
    DrawText(can.Handle, PChar(txtW), Length(txtW), r2, tf);
    r2 := Classes.Rect(r.Left + 2, r.Top + 2, r.Left + 20 + 2, r.Bottom - 2);
    { can.Brush.Color := clBtnFace;
      can.FillRect(r2); }
    fm := THistoryFrm(Owner);
    if lb.Items.Objects[Index] is THppToolButton then
    begin
      but := THppToolButton(lb.Items.Objects[Index]);
      ImageList_Draw(fm.ilToolbar.Handle, but.ImageIndex, can.Handle, r2.Left + 2, r2.Top + 2,
        ILD_NORMAL);
    end
    else if lb.Items.Objects[Index] = fm.tbEventsFilter then
    begin
      DrawIconEx(can.Handle, r2.Left + 2, r2.Top + 2, hppIcons[HPP_ICON_DROPDOWNARROW].Handle,
        16, 16, 0, 0, DI_NORMAL);
    end
    else if lb.Items.Objects[Index] = fm.tbHistory then
    begin
      DrawIconEx(can.Handle, r2.Left + 2, r2.Top + 2, hppIcons[HPP_ICON_CONTACTHISTORY].Handle,
        16, 16, 0, 0, DI_NORMAL);
    end;
  end
  else
  begin
    r2 := Classes.Rect(r.Left, r.Top + ((r.Bottom - r.Top) div 2), r.Right, r.Bottom);
    r2.Bottom := r2.Top + 1;
    InflateRect(r2, -((r2.Right - r2.Left) div 10), 0);
    can.Pen.Color := can.Font.Color;
    can.MoveTo(r2.Left, r2.Top);
    can.LineTo(r2.Right, r2.Top);
  end;

  if (lbAdded.Dragging) or (lbAvailable.Dragging) and (lb = lbAdded) then
  begin
    DrawLineTop := False;
    DrawLineBottom := False;
    dst := DragOverIndex;
    can.Pen.Color := clHighlight;
    if lbAdded.Dragging then
    begin
      src := lbAdded.ItemIndex;
      if Index = dst then
      begin
        if (dst < src) then
          DrawLineTop := True
        else
          DrawLineBottom := True
      end;
    end
    else
    begin
      if Index = dst then
        DrawLineTop := True;
    end;
    if (dst = lb.Count) and (Index = lb.Count - 1) then
      DrawLineBottom := True;

    if DrawLineTop then
    begin
      can.MoveTo(r.Left, r.Top);
      can.LineTo(r.Right, r.Top);
    end;
    if DrawLineBottom then
    begin
      can.MoveTo(r.Left, r.Bottom - 1);
      can.LineTo(r.Right, r.Bottom - 1);
    end;
  end;

  BitBlt(lb.Canvas.Handle,Rect.Left,Rect.Top,ItemBmp.Width,ItemBmp.Height,can.Handle,0,0,SRCCOPY);
end;

procedure TfmCustomizeToolbar.OnWMChar(var Message: TWMChar);
begin
  if not(csDesigning in ComponentState) then
    with Message do
    begin
      Result := 1;
      if (Perform(WM_GETDLGCODE, 0, 0) and DLGC_WANTCHARS = 0) and
        (GetParentForm(Self).Perform(CM_DIALOGCHAR, CharCode, KeyData) <> 0) then
        Exit;
      Result := 0;
    end;
end;

procedure TfmCustomizeToolbar.RemoveItem(src: Integer);
begin
  if (src = -1) or (src > lbAdded.Count - 1) then
    Exit;

  if (lbAdded.Items.Objects[src] <> nil) then
  begin
    // delete last item -- separator
    lbAvailable.Items.Delete(lbAvailable.Items.Count - 1);
    // add item
    lbAvailable.AddItem(lbAdded.Items[src], lbAdded.Items.Objects[src]);
    // sort
    lbAvailable.Sorted := True;
    lbAvailable.Sorted := False;
    // add separator back
    lbAvailable.AddItem('-', nil);
  end;
  lbAvailable.ItemIndex := lbAvailable.Items.IndexOfObject(lbAdded.Items.Objects[src]);

  lbAdded.Items.Delete(src);
  if src < lbAdded.Count then
    lbAdded.ItemIndex := src
  else if src - 1 < lbAdded.Count then
    lbAdded.ItemIndex := src - 1;

  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.SaveToolbar(ToolbarStr: AnsiString);
begin
  if ToolbarStr = '' then
    ToolbarStr := DEF_HISTORY_TOOLBAR;
  if ToolbarStr = DEF_HISTORY_TOOLBAR then
    DBDeleteContactSetting(0, hppDBName, 'HistoryToolbar')
  else
    WriteDBStr(hppDBName, 'HistoryToolbar', ToolbarStr);
end;

procedure TfmCustomizeToolbar.tiScrollTimer(Sender: TObject);
begin
  case TimerScrollDirection of
    1: lbAdded.Perform(WM_VSCROLL, SB_LINEDOWN, 0);
    2: lbAdded.Perform(WM_VSCROLL, SB_LINEUP, 0)
  else
    tiScroll.Enabled := False;
  end;
end;

procedure TfmCustomizeToolbar.AddItem(src, dst: Integer);
begin
  if (src = -1) or (src > lbAvailable.Count - 1) then
    Exit;

  lbAdded.AddItem(lbAvailable.Items[src], lbAvailable.Items.Objects[src]);
  if lbAvailable.Items[src] <> '-' then
    lbAvailable.Items.Delete(src);
  if (dst <> lbAdded.Count - 1) and (dst <> -1) then
  begin
    lbAdded.Items.Move(lbAdded.Count - 1, dst);
    lbAdded.ItemIndex := dst;
  end
  else
    lbAdded.ItemIndex := lbAdded.Count - 1;
  if src < lbAvailable.Count then
    lbAvailable.ItemIndex := src
  else if src - 1 < lbAvailable.Count then
    lbAvailable.ItemIndex := src - 1;

  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.bnAddClick(Sender: TObject);
begin
  AddItem(lbAvailable.ItemIndex, lbAdded.ItemIndex);
end;

procedure TfmCustomizeToolbar.FillButtons;
var
  i: Integer;
  fm: THistoryFrm;
  but: TControl;
  txt: String;
begin
  lbAdded.Clear;
  lbAvailable.Clear;
  fm := THistoryFrm(Owner);

  for i := 0 to fm.Toolbar.ButtonCount - 1 do
  begin
    but := fm.Toolbar.Buttons[i];
    txt := '';
    if but is THppToolButton then
    begin
      if THppToolButton(but).Style in [tbsSeparator, tbsDivider] then
        txt := '-'
      else
        txt := THppToolButton(but).Hint
    end
    else if but = fm.tbEventsFilter then
      txt := TranslateW('Event Filters')
    else if but is TSpeedButton then
      txt := TSpeedButton(but).Hint;

    if txt <> '' then
    begin
      if but.Visible then
      begin
        if txt = '-' then
          lbAdded.AddItem(txt, nil)
        else
          lbAdded.AddItem(txt, but);
      end
      else
        lbAvailable.AddItem(txt, but);
    end;
  end;
  lbAvailable.Sorted := True;
  lbAvailable.Sorted := False;
  lbAvailable.AddItem('-', nil);

  if lbAdded.Count > 0 then
  begin
    lbAdded.ItemIndex := 0;
    if Visible then
      lbAdded.SetFocus
    else
      ActiveControl := lbAdded;
  end
  else
  begin
    lbAvailable.ItemIndex := 0;
    if Visible then
      lbAvailable.SetFocus
    else
      ActiveControl := lbAvailable;
  end;
  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Action := caFree;
end;

procedure TfmCustomizeToolbar.FormCreate(Sender: TObject);
begin
  fmCustomizeToolbar := Self;

  DesktopFont := True;
  MakeFontsParent(Self);
  TranslateForm;

  ItemBmp := TBitmap.Create;
  FillButtons;
end;

procedure TfmCustomizeToolbar.FormDestroy(Sender: TObject);
begin
  fmCustomizeToolbar := nil;
  ItemBmp.Free;
  try
    THistoryFrm(Owner).CustomizeToolbarForm := nil;
  except
    // "eat" exceptions if any
  end;
end;

procedure TfmCustomizeToolbar.FormKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
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
      VK_RETURN, VK_EXECUTE, VK_ESCAPE, VK_CANCEL:
        Mask := DLGC_WANTALLKEYS;
    end;
    if (Mask <> 0) and (Perform(CM_WANTSPECIALKEY, Key, 0) = 0) and
      (Perform(WM_GETDLGCODE, 0, 0) and Mask = 0) and (Self.Perform(CM_DIALOGKEY, Key, 0) <> 0)
    then
      Exit;
  end;
end;

function TfmCustomizeToolbar.GenerateToolbarString: AnsiString;
var
  i: Integer;
  but: TControl;
  but_str: AnsiString;
  fm: THistoryFrm;
begin
  Result := '';
  fm := THistoryFrm(Owner);
  for i := 0 to lbAdded.Count - 1 do
  begin
    but := TControl(lbAdded.Items.Objects[i]);
    if      but = nil                then but_str := ' '
    else if but = fm.tbSessions      then but_str := '[SESS]'
    else if but = fm.tbBookmarks     then but_str := '[BOOK]'
    else if but = fm.tbSearch        then but_str := '[SEARCH]'
    else if but = fm.tbFilter        then but_str := '[FILTER]'
    else if but = fm.tbCopy          then but_str := '[COPY]'
    else if but = fm.tbDelete        then but_str := '[DELETE]'
    else if but = fm.tbSave          then but_str := '[SAVE]'
    else if but = fm.tbHistory       then but_str := '[HISTORY]'
    else if but = fm.tbHistorySearch then but_str := '[GLOBSEARCH]'
    else if but = fm.tbEventsFilter  then but_str := '[EVENTS]'
    else if but = fm.tbUserMenu      then but_str := '[USERMENU]'
    else if but = fm.tbUserDetails   then but_str := '[USERDETAILS]';
    Result := Result + but_str;
  end;
end;

procedure TfmCustomizeToolbar.HMIcons2Changed(var M: TMessage);
begin
  lbAvailable.Repaint;
  lbAdded.Repaint;
end;

procedure TfmCustomizeToolbar.bnOKClick(Sender: TObject);
begin
  SaveToolbar(GenerateToolbarString);
  NotifyAllForms(HM_NOTF_TOOLBARCHANGED, 0, 0);
  close;
end;

procedure TfmCustomizeToolbar.bnCancelClick(Sender: TObject);
begin
  close;
end;

procedure TfmCustomizeToolbar.bnDownClick(Sender: TObject);
var
  idx: Integer;
begin
  idx := lbAdded.ItemIndex;
  if (idx < 0) or (idx > lbAdded.Count - 1) then
    Exit;
  lbAdded.Items.Move(idx, idx + 1);
  lbAdded.ItemIndex := idx + 1;
  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.bnRemoveClick(Sender: TObject);
begin
  RemoveItem(lbAdded.ItemIndex);
end;

procedure TfmCustomizeToolbar.bnResetClick(Sender: TObject);
begin
  DBDeleteContactSetting(0, hppDBName, 'HistoryToolbar');
  NotifyAllForms(HM_NOTF_TOOLBARCHANGED, 0, 0);
  FillButtons;
  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.bnUpClick(Sender: TObject);
var
  idx: Integer;
begin
  idx := lbAdded.ItemIndex;
  if idx < 1 then
    Exit;
  lbAdded.Items.Move(idx, idx - 1);
  lbAdded.ItemIndex := idx - 1;
  UpdateControlButtons;
end;

procedure TfmCustomizeToolbar.TranslateForm;
begin
  Caption             := TranslateUnicodeString(Caption);
  laAvailable.Caption := TranslateUnicodeString(laAvailable.Caption);
  laAdded.Caption     := TranslateUnicodeString(laAdded.Caption);
  bnOK.Caption        := TranslateUnicodeString(bnOK.Caption);
  bnCancel.Caption    := TranslateUnicodeString(bnCancel.Caption);
  bnReset.Caption     := TranslateUnicodeString(bnReset.Caption);
  bnAdd.Caption       := TranslateUnicodeString(bnAdd.Caption);
  bnRemove.Caption    := TranslateUnicodeString(bnRemove.Caption);
  bnUp.Caption        := TranslateUnicodeString(bnUp.Caption);
  bnDown.Caption      := TranslateUnicodeString(bnDown.Caption);
end;

procedure TfmCustomizeToolbar.UpdateControlButtons;
begin
  bnAdd.Enabled    := (lbAvailable.ItemIndex <> -1);
  bnRemove.Enabled := (lbAdded.ItemIndex <> -1);
  bnUp.Enabled     := (lbAdded.ItemIndex <> -1) and (lbAdded.ItemIndex > 0);
  bnDown.Enabled   := (lbAdded.ItemIndex <> -1) and (lbAdded.ItemIndex < lbAdded.Count - 1);
  bnOK.Enabled     := (lbAdded.Count > 0);
end;

end.

