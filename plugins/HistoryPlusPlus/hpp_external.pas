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

unit hpp_external;

interface

uses
  Classes, Windows,
  m_api,
  hpp_externalgrid;

type
  TExternalGrids = class(TObject)
  private
    FGrids: TList;
    procedure SetGroupLinked(Value: Boolean);
  public
    constructor Create;
    destructor Destroy; override;
    procedure Add(const ExtGrid: TExternalGrid);
    function Find(Handle: HWND): TExternalGrid;
    function Delete(Handle: HWND): Boolean;
    function Clear(): Boolean;
    procedure Perform(Msg: Cardinal; wParam: WPARAM; lParam: LPARAM);
    property GroupLinked: Boolean write SetGroupLinked;
  end;


const
  MS_HPP_EG_WINDOW         = 'History++/ExtGrid/NewWindow';
  MS_HPP_EG_EVENT	         = 'History++/ExtGrid/Event';
  MS_HPP_EG_NAVIGATE       = 'History++/ExtGrid/Navigate';
  ME_HPP_EG_OPTIONSCHANGED = 'History++/ExtGrid/OptionsChanged';

var
  ExternalGrids: TExternalGrids;

procedure RegisterExtGridServices;
procedure UnregisterExtGridServices;

implementation

uses
  hpp_global, hpp_database;

{$include m_ieview.inc}

var
  hExtOptChanged: THandle;

function _ExtWindow(wParam:WPARAM; lParam: LPARAM): uint_ptr;
var
  par: PIEVIEWWINDOW;
  ExtGrid: TExternalGrid;
  ControlID: Cardinal;
begin
  Result := 0;
  //try
    par := PIEVIEWWINDOW(lParam);
    Assert(par <> nil, 'Empty IEVIEWWINDOW structure');
    case par.iType of
      IEW_CREATE: begin
        {$IFDEF DEBUG}
        OutputDebugString('IEW_CREATE');
        {$ENDIF}
        case par.dwMode of
          IEWM_TABSRMM: ControlID := 1006;  // IDC_LOG from tabSRMM
          IEWM_SCRIVER: ControlID := 1001;  // IDC_LOG from Scriver
          IEWM_MUCC:    ControlID := 0;
          IEWM_CHAT:    ControlID := 0;
          IEWM_HISTORY: ControlID := 0;
        else            ControlID := 0;
        end;
        ExtGrid := TExternalGrid.Create(par.Parent,ControlID);
        case par.dwMode of
          IEWM_MUCC,IEWM_CHAT: begin
            ExtGrid.ShowHeaders   := False;
            ExtGrid.GroupLinked   := False;
            ExtGrid.ShowBookmarks := False;
          end;
          IEWM_HISTORY:
            ExtGrid.GroupLinked := False;
        end;
        ExtGrid.SetPosition(par.x,par.y,par.cx,par.cy);
        ExternalGrids.Add(ExtGrid);
        par.Hwnd := ExtGrid.GridHandle;
      end;
      IEW_DESTROY: begin
        {$IFDEF DEBUG}
        OutputDebugString('IEW_DESTROY');
        {$ENDIF}
        ExternalGrids.Delete(par.Hwnd);
      end;
      IEW_SETPOS: begin
        {$IFDEF DEBUG}
        OutputDebugString('IEW_SETPOS');
        {$ENDIF}
        ExtGrid := ExternalGrids.Find(par.Hwnd);
        if ExtGrid <> nil then
          ExtGrid.SetPosition(par.x,par.y,par.cx,par.cy);
      end;
      IEW_SCROLLBOTTOM: begin
        {$IFDEF DEBUG}
        OutputDebugString('IEW_SCROLLBOTTOM');
        {$ENDIF}
        ExtGrid := ExternalGrids.Find(par.Hwnd);
        if ExtGrid <> nil then
          ExtGrid.ScrollToBottom;
      end;
    end;
  //except
  //end;
end;

function ExtWindowNative(wParam:WPARAM; lParam: LPARAM): uint_ptr; cdecl;
begin
  Result := _ExtWindow(wParam,lParam);
end;

function _ExtEvent(wParam:WPARAM; lParam: LPARAM): uint_ptr; cdecl;
var
  event: PIEVIEWEVENT;
  customEvent: PIEVIEWEVENTDATA;
  UsedCodepage: Cardinal;
  hDBNext: THandle;
  eventCount: Integer;
  ExtGrid: TExternalGrid;
  CustomItem: TExtCustomItem;
begin
  Result := 0;
  //try
    {$IFDEF DEBUG}
    OutputDebugString('MS_IEVIEW_EVENT');
    {$ENDIF}
    event := PIEVIEWEVENT(lParam);
    Assert(event <> nil, 'Empty IEVIEWEVENT structure');
    ExtGrid := ExternalGrids.Find(event.Hwnd);
    if ExtGrid = nil then exit;
    case event.iType of
      IEE_LOG_DB_EVENTS: begin
        UsedCodepage := event.Codepage;
        eventCount := event.Count;
        hDBNext := event.Event.hDBEventFirst;
        ExtGrid.BeginUpdate;
        while (eventCount <> 0) and (hDBNext <> 0) do
        begin
          ExtGrid.AddEvent(event.hContact, hDBNext, UsedCodepage,
                           boolean(event.dwFlags and IEEF_RTL),
                           not boolean(event.dwFlags and IEEF_NO_SCROLLING));
          if eventCount > 0 then Dec(eventCount);
          if eventCount <> 0 then
            hDBNext := db_event_next(event.hContact,hDBNext);
        end;
        ExtGrid.EndUpdate;
      end;
      IEE_LOG_MEM_EVENTS: begin
        UsedCodepage := event.Codepage;
        eventCount := event.Count;
        customEvent := event.Event.eventData;
        ExtGrid.BeginUpdate;
        while (eventCount <> 0) and (customEvent <> nil) do
        begin
          if boolean(customEvent.dwFlags and IEEDF_UNICODE_TEXT) then
            SetString(CustomItem.Text,customEvent.Text.w,lstrlenW(customEvent.Text.w))
          else
            CustomItem.Text := AnsiToWideString(AnsiString(customEvent.Text.a),UsedCodepage);
          if boolean(customEvent.dwFlags and IEEDF_UNICODE_NICK) then
            SetString(CustomItem.Nick,customEvent.Nick.w,lstrlenW(customEvent.Nick.w))
          else
            CustomItem.Nick := AnsiToWideString(AnsiString(customEvent.Nick.a),UsedCodepage);
          CustomItem.Sent := boolean(customEvent.bIsMe);
          CustomItem.Time := customEvent.time;
          CustomItem.hEvent := customEvent.hEvent;
          ExtGrid.AddCustomEvent(event.hContact, CustomItem, UsedCodepage,
                             boolean(event.dwFlags and IEEF_RTL),
                             not boolean(event.dwFlags and IEEF_NO_SCROLLING));
          if eventCount > 0 then Dec(eventCount);
          customEvent := customEvent.next;
        end;
        ExtGrid.EndUpdate;
      end;
      IEE_CLEAR_LOG: begin
        ExtGrid.BeginUpdate;
        ExtGrid.Clear;
        ExtGrid.EndUpdate;
      end;
      IEE_GET_SELECTION: begin
        Result := uint_ptr(ExtGrid.GetSelection());
      end;
      IEE_SAVE_DOCUMENT: begin
        ExtGrid.SaveSelected;
      end;
    end;
  //except
  //end;
end;

function ExtEventNative(wParam:WPARAM; lParam: LPARAM): uint_ptr; cdecl;
begin
  Result := _ExtEvent(wParam,lParam);
end;


function ExtNavigate(wParam:WPARAM; lParam: LPARAM): uint_ptr; cdecl;
begin
  Result := 0;
  //try
    {$IFDEF DEBUG}
    OutputDebugString('MS_IEVIEW_NAVIGATE');
    {$ENDIF}
  //except
  //end;
end;

procedure RegisterExtGridServices;
begin
  ExternalGrids := TExternalGrids.Create;
  CreateServiceFunction(MS_HPP_EG_WINDOW,@ExtWindowNative);
  CreateServiceFunction(MS_HPP_EG_EVENT,@ExtEventNative);
  CreateServiceFunction(MS_HPP_EG_NAVIGATE,@ExtNavigate);
  hExtOptChanged := CreateHookableEvent(ME_HPP_EG_OPTIONSCHANGED);
end;

procedure UnregisterExtGridServices;
begin
  DestroyHookableEvent(hExtOptChanged);
  ExternalGrids.Destroy;
end;

constructor TExternalGrids.Create;
begin
  FGrids := TList.Create;
end;

destructor TExternalGrids.Destroy;
begin
  Clear;
  FGrids.Free;
  inherited;
end;

procedure TExternalGrids.Add(const ExtGrid: TExternalGrid);
begin
  FGrids.Add(ExtGrid);
end;

function TExternalGrids.Find(Handle: HWND): TExternalGrid;
var
  i: Integer;
  ExtGrid: TExternalGrid;
begin
  Result := nil;
  for i := 0 to FGrids.Count-1 do
  begin
    ExtGrid := TExternalGrid(FGrids.Items[i]);
    if ExtGrid.GridHandle = Handle then
    begin
      Result := ExtGrid;
      break;
    end;
  end;
end;

function TExternalGrids.Delete(Handle: HWND): Boolean;
var
  i: Integer;
  ExtGrid: TExternalGrid;
begin
  Result := True;
  for i := 0 to FGrids.Count-1 do
  begin
    ExtGrid := TExternalGrid(FGrids.Items[i]);
    if ExtGrid.GridHandle = Handle then
    begin
      try
        ExtGrid.Free;
      except
        Result := False;
      end;
      FGrids.Delete(i);
      break;
    end;
  end;
end;

function TExternalGrids.Clear: Boolean;
var
  i: Integer;
  ExtGrid: TExternalGrid;
begin
  Result := True;
  for i := 0 to FGrids.Count-1 do
  begin
    ExtGrid := TExternalGrid(FGrids.Items[i]);
    try
      ExtGrid.Free;
    except
      Result := False;
    end;
  end;
  FGrids.Clear;
end;

procedure TExternalGrids.Perform(Msg: Cardinal; wParam: WPARAM; lParam: LPARAM);
var
  i: Integer;
begin
  for i := FGrids.Count-1 downto 0 do
    TExternalGrid(FGrids.Items[i]).Perform(Msg,wParam,lParam);
end;

procedure TExternalGrids.SetGroupLinked(Value: Boolean);
var
  i: Integer;
begin
  for i := FGrids.Count-1 downto 0 do
    TExternalGrid(FGrids.Items[i]).GroupLinked := Value;
end;

end.
