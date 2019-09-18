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

unit hpp_forms;

interface

uses Windows, Messages, Classes, Graphics,
  Controls, Forms, Menus, ComCtrls, StdCtrls,
  Themes;

type
  THppHintWindow = class(THintWindow{THintWindow})
  private
    procedure CMInvalidate(var Message: TMessage); message CM_INVALIDATE;
  protected
    procedure NCPaint(DC: HDC); override;
  public
    constructor Create(AOwner: TComponent); override;
  end;

procedure NotifyAllForms(Msg:UInt; wParam:WPARAM; lParam: LPARAM);
procedure BringFormToFront(Form: TForm);
procedure MakeFontsParent(Control: TControl);
procedure MakeDoubleBufferedParent(Control: TWinControl);

//procedure AddMenu(M: TMenuItem; FromM,ToM: TPopupMenu; Index: integer);
procedure AddMenuArray(Menu: TPopupMenu; List: Array of TMenuItem; Index: integer);

procedure TranslateMenu(mi: TMenuItem);
procedure TranslateToolbar(const tb: TToolBar);

function ShiftStateToKeyData(ShiftState :TShiftState):Longint;
function IsFormShortCut(List: Array of TComponent; Key: Word; ShiftState: TShiftState): Boolean;

function Utils_RestoreFormPosition(Form: TForm; hContact: THandle; const Module,Prefix: AnsiString): Boolean;
function Utils_SaveFormPosition(Form: TForm; hContact: THandle; const Module,Prefix: AnsiString): Boolean;

implementation

uses
  hpp_services, hpp_opt_dialog, hpp_database, hpp_mescatcher,
  HistoryForm, GlobalSearch, m_api,
  {$IFNDEF NO_EXTERNALGRID}hpp_external,{$ENDIF}
  CustomizeFiltersForm, CustomizeToolbar;

{procedure AddMenu(M: TMenuItem; FromM,ToM: TPopupMenu; Index: integer);
//var
//  i: integer;
//  mi: TMenuItem;
begin
  if ToM.FindItem(M.Handle,fkHandle) = nil then begin
    if FromM.FindItem(M.Handle,fkHandle) <> nil then
      FromM.Items.Remove(M);
    if Index = -1 then ToM.Items.Add(M)
                  else ToM.Items.Insert(Index,M);
  end;
end;}

procedure AddMenuArray(Menu: TPopupMenu; List: Array of TMenuItem; Index: integer);
var
  i: integer;
begin
  for i := 0 to High(List) do
  begin
    if List[i].Parent <> nil then
    begin
      if List[i].GetParentMenu = Menu then continue;
      List[i].Parent.Remove(List[i]);
    end;
    if Index = -1 then
      Menu.Items.Add(List[i])
    else
      Menu.Items.Insert(Index+i,List[i]);
  end;
end;

function IsFormShortCut(List: Array of TComponent; Key: Word; ShiftState: TShiftState): Boolean;
var
  i: integer;
  mes: TWMKey;
begin
  Result := False;
  if Key = 0 then exit;
  if Key = VK_INSERT then
  begin
    if ShiftState = [ssCtrl] then
    begin
      Key := Ord('C')
    end
    else if ShiftState = [ssShift] then
    begin
      Key := Ord('V'); ShiftState := [ssCtrl];
    end;
  end;
  mes.CharCode := Key;
  mes.KeyData := ShiftStateToKeyData(ShiftState);
  for i := 0 to High(List) do
  begin
    if List[i] is TMenu then
    begin
      Result := TMenu(List[i]).IsShortCut(mes);
    end
    else if List[i] is TForm then
    begin
      Result := (TForm(List[i]).Menu <> nil) and
                (TForm(List[i]).Menu.WindowHandle <> 0) and
                (TForm(List[i]).Menu.IsShortCut(mes));
    end;
    if Result then break;
  end;
end;

function ShiftStateToKeyData(ShiftState :TShiftState):Longint;
const
  AltMask = $20000000;
begin
  Result := 0;
  if ssShift in ShiftState then Result := Result or VK_SHIFT;
  if ssCtrl in ShiftState  then Result := Result or VK_CONTROL;
  if ssAlt in ShiftState   then Result := Result or AltMask;
end;

function Utils_RestoreFormPosition(Form: TForm; hContact: THandle; const Module,Prefix: AnsiString): Boolean;
var
  w,h,l,t,mon: Integer;
  maximized: Boolean;
  deltaW, deltaH: integer;
begin
  Result := True;
  deltaW := Form.Width - Form.ClientWidth;
  deltaH := Form.Height - Form.ClientHeight;
  mon := GetDBWord(Module,Prefix+'monitor',Form.Monitor.MonitorNum);
  if mon >= Screen.MonitorCount then mon := Form.Monitor.MonitorNum;
  w := GetDBWord(Module,Prefix+'width',Form.ClientWidth) + deltaW;
  h := GetDBWord(Module,Prefix+'height',Form.ClientHeight) + deltaH;
  l := GetDBInt(Module,Prefix+'x',Screen.Monitors[mon].Left+((Screen.Monitors[mon].Width-w) div 2));
  t := GetDBInt(Module,Prefix+'y',Screen.Monitors[mon].Top+((Screen.Monitors[mon].Height-h) div 2));
  maximized := GetDBBool(Module,Prefix+'maximized',False);
  // just to be safe, don't let window jump out of the screen
  // at least 100 px from each side should be visible
  if l+100 > Screen.DesktopWidth then l := Screen.DesktopWidth-100;
  if t+100 > Screen.DesktopHeight then t := Screen.DesktopHeight-100;
  if l+w < 100 then l := 100-w;
  if t+h < 100 then t := 100-h;
  Form.SetBounds(l,t,w,h);
  if maximized then Form.WindowState := wsMaximized;
end;

function Utils_SaveFormPosition(Form: TForm; hContact: THandle; const Module,Prefix: AnsiString): Boolean;
var
  w,h,l,t: Integer;
  wp: TWindowPlacement;
  maximized: Boolean;
begin
  Result := True;
  maximized := (Form.WindowState = wsMaximized);
  if maximized then
  begin
    wp.length := SizeOf(TWindowPlacement);
    GetWindowPlacement(Form.Handle,@wp);
    l := wp.rcNormalPosition.Left;
    t := wp.rcNormalPosition.Top;
    w := wp.rcNormalPosition.Right - wp.rcNormalPosition.Left - (Form.Width - Form.ClientWidth);
    h := wp.rcNormalPosition.Bottom - wp.rcNormalPosition.Top - (Form.Height - Form.ClientHeight);
  end
  else
  begin
    l := Form.Left;
    t := Form.Top;
    w := Form.ClientWidth;
    h := Form.ClientHeight;
  end;
  WriteDBInt(Module,Prefix+'x',l);
  WriteDBInt(Module,Prefix+'y',t);
  WriteDBWord(Module,Prefix+'width',w);
  WriteDBWord(Module,Prefix+'height',h);
  WriteDBWord(Module,Prefix+'monitor',Form.Monitor.MonitorNum);
  WriteDBBool(Module,Prefix+'maximized',maximized);
end;

procedure BringFormToFront(Form: TForm);
begin
  if Form.WindowState = wsMaximized then
    ShowWindow(Form.Handle,SW_SHOWMAXIMIZED)
  else
    ShowWindow(Form.Handle,SW_SHOWNORMAL);
  Form.BringToFront;
end;

procedure NotifyAllForms(Msg:UInt; wParam:WPARAM; lParam: LPARAM);
var
  i: Integer;
begin
  if hDlg <> 0 then
    SendMessage(hDlg,Msg,wParam,lParam);

  // we are going backwards here because history forms way want to
  // close themselves on the message, so we would have AVs if go from 0 to Count

  {$IFNDEF NO_EXTERNALGRID}
  ExternalGrids.Perform(Msg,wParam,lParam);
  {$ENDIF}

  for i := HstWindowList.Count - 1 downto 0 do
  begin
    if Assigned(THistoryFrm(HstWindowList[i]).EventDetailForm) then
      THistoryFrm(HstWindowList[i]).EventDetailForm.Perform(Msg,wParam,lParam);
    THistoryFrm(HstWindowList[i]).Perform(Msg,wParam,lParam);
  end;

  if Assigned(fmGlobalSearch) then
    fmGlobalSearch.Perform(Msg,wParam,lParam);

  if Assigned(fmCustomizeFilters) then
    fmCustomizeFilters.Perform(Msg,wParam,lParam);

  if Assigned(fmCustomizeToolbar) then
    fmCustomizeToolbar.Perform(Msg,wParam,lParam);
end;

// This procedure scans all control children and if they have
// no ParentFont, sets ParentFont to true but reapplies font styles,
// so even having parent font and size, controls remain bold or italic
//
// Of course it can be done cleaner and for all controls supporting fonts
// property through TPropertyEditor and GetPropInfo, but then it would
// need vcl sources to compile, so not a best alternative for open source plugin
procedure MakeFontsParent(Control: TControl);
var
  i: Integer;
  fs: TFontStyles;
begin
  // Set TLabel & TLabel
  if (Control is TLabel) and (not TLabel(Control).ParentFont) then
  begin
    fs := TLabel(Control).Font.Style;
    TLabel(Control).ParentFont := True;
    TLabel(Control).Font.Style := fs;
  end;
  if (Control is TLabel) and (not TLabel(Control).ParentFont) then
  begin
    fs := TLabel(Control).Font.Style;
    TLabel(Control).ParentFont := True;
    TLabel(Control).Font.Style := fs;
  end;
  // Process children
  for i := 0 to Control.ComponentCount - 1 do
  begin
    if Control.Components[i] is TControl then
    begin
      MakeFontsParent(TControl(Control.Components[i]));
    end;
  end;
end;

{ THppHintWindow }

type
  THackHintWindow = class(TCustomControl)
  private
    FActivating: Boolean;
  end;

constructor THppHintWindow.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);
  Color := clInfoBk;
end;

procedure THppHintWindow.CMInvalidate(var Message: TMessage);
begin
  if (THackHintWindow(Self).FActivating) and
     (Application.Handle = 0) and (ParentWindow = 0) then
  begin
    ParentWindow := hppMainWindow;
    ShowWindow(Handle, SW_SHOWNOACTIVATE);
  end;
  inherited;
end;

procedure THppHintWindow.NCPaint(DC: HDC);
var
  R: TRect;
begin
  R := Rect(0, 0, Width, Height);
  DrawEdge(DC, R, EDGE_ETCHED, BF_RECT or BF_MONO);
end;

// This procedure scans all WinControl children and set them the same
// DoubleBuffered property.
procedure MakeDoubleBufferedParent(Control: TWinControl);
var
  i: Integer;
  DoubleBuffered: Boolean;
begin
  DoubleBuffered := Control.DoubleBuffered;
  for i := 0 to Control.ComponentCount - 1 do
  begin
    if not (Control.Components[i] is TCustomRichEdit) and
       (Control.Components[i] is TWinControl) then
    begin
      TWinControl(Control.Components[i]).DoubleBuffered := DoubleBuffered;
      MakeDoubleBufferedParent(TWinControl(Control.Components[i]));
    end;
  end;
end;

procedure TranslateMenu(mi: TMenuItem);
var
  i: integer;
begin
  for i := 0 to mi.Count-1 do
    if mi.Items[i].Caption <> '-' then
    begin
      TMenuItem(mi.Items[i]).Caption := TranslateUnicodeString(mi.Items[i].Caption{TRANSLATE-IGNORE});
      if mi.Items[i].Count > 0 then TranslateMenu(mi.Items[i]);
    end;
end;

procedure TranslateToolbar(const tb: TToolBar);
var
  i: integer;
begin
  for i := 0 to tb.ButtonCount-1 do
    if tb.Buttons[i].Style <> tbsSeparator then
    begin
      TToolBar(tb.Buttons[i]).Hint    := TranslateUnicodeString(tb.Buttons[i].Hint{TRANSLATE-IGNORE});
      TToolBar(tb.Buttons[i]).Caption := TranslateUnicodeString(tb.Buttons[i].Caption{TRANSLATE-IGNORE});
    end;
end;

initialization

  // init ThemeServices before widows open
  Themes.StyleServices;

end.
