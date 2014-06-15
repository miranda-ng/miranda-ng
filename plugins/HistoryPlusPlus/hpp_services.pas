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

{ -----------------------------------------------------------------------------
  hpp_services (historypp project)

  Version:   1.5
  Created:   05.08.2004
  Author:    Oxygen

  [ Description ]

  Module with history's own services

  [ History ]

  1.5 (05.08.2004)
  First version

  [ Modifications ]
  none

  [ Known Issues ]
  none

  Contributors: theMIROn, Art Fedorov
  ----------------------------------------------------------------------------- }

unit hpp_services;

interface

uses
  Classes, Windows, Controls,
  m_api,
  HistoryForm;

var
  hHppRichEditItemProcess: THandle;
  HstWindowList: TList;

procedure hppRegisterServices;
procedure hppUnregisterServices;

procedure CloseGlobalSearchWindow;
procedure CloseHistoryWindows;
function FindContactWindow(hContact: THandle): THistoryFrm;
function OpenContactHistory(hContact: THandle; Index: Integer = -1): THistoryFrm;

function AllHistoryRichEditProcess(wParam: WPARAM; lParam: LPARAM): Int; cdecl;

implementation

uses
  SysUtils, GlobalSearch, EmptyHistoryForm,
  hpp_global, {hpp_database,} hpp_itemprocess, hpp_forms,
  hpp_options{, hpp_mescatcher, hpp_bookmarks};

// our own processing of RichEdit for all history windows
function AllHistoryRichEditProcess(wParam { hRichEdit } : WPARAM; lParam { PItemRenderDetails } : LPARAM): Int; cdecl;
begin
  Result := 0;
  if GridOptions.SmileysEnabled        then Result := Result or DoSupportSmileys(wParam, lParam);
  if GridOptions.AvatarsHistoryEnabled then Result := Result or DoSupportAvatarHistory(wParam, lParam);
end;

procedure CloseHistoryWindows;
var
  i: Integer;
begin
  try
    for i := HstWindowList.Count - 1 downto 0 do
      THistoryFrm(HstWindowList[i]).Free;
  except
  end;
end;

procedure CloseGlobalSearchWindow;
begin
  try
    if Assigned(fmGlobalSearch) then
      fmGlobalSearch.Free;
  except
  end;
end;

function FindContactWindow(hContact: THandle): THistoryFrm;
var
  i: Integer;
begin
  Result := nil;
  for i := 0 to HstWindowList.Count - 1 do
  begin
    if THistoryFrm(HstWindowList[i]).hContact = hContact then
    begin
      Result := THistoryFrm(HstWindowList[i]);
      break;
    end;
  end;
end;

function OpenContactHistory(hContact: THandle; Index: Integer = -1): THistoryFrm;
var
  wHistory: THistoryFrm;
  NewWindow: Boolean;
begin
  // check if window exists, otherwise create one
  wHistory := FindContactWindow(hContact);
  NewWindow := not Assigned(wHistory);
  if NewWindow then
  begin
    wHistory := THistoryFrm.Create(nil);
    HstWindowList.Add(wHistory);
    wHistory.WindowList := HstWindowList;
    wHistory.hg.Options := GridOptions;
    wHistory.hContact   := hContact;
    wHistory.Load;
  end;
  if Index <> -1 then
  begin
    wHistory.ShowAllEvents;
    wHistory.ShowItem(index);
  end;
  if NewWindow then
    wHistory.Show
  else
    BringFormToFront(wHistory); // restore even if minimized
  Result := wHistory;
end;

// MS_HISTORY_SHOWCONTACTHISTORY service
// show history called by miranda
function HppShowHistory(wParam { hContact } : WPARAM; lParam { 0 } : LPARAM): uint_ptr; cdecl;
begin
  OpenContactHistory(wParam);
  Result := 0;
end;

// MS_HPP_GETVERSION service
// See m_historypp.inc for details
function HppGetVersion(wParam { 0 } : WPARAM; lParam { 0 } : LPARAM): uint_ptr; cdecl;
begin
  Result := hppVersion;
end;

// MS_HPP_SHOWGLOBALSEARCH service
// See m_historypp.inc for details
function HppShowGlobalSearch(wParam { 0 } : WPARAM; lParam { 0 } : LPARAM): uint_ptr; cdecl;
begin
  if not Assigned(fmGlobalSearch) then
  begin
    fmGlobalSearch := TfmGlobalSearch.Create(nil);
    fmGlobalSearch.hg.Options := GridOptions;
    fmGlobalSearch.Show;
  end
  else
    BringFormToFront(fmGlobalSearch);
  Result := 0;
end;

// MS_HPP_OPENHISTORYEVENT service
// See m_historypp.inc for details
function HppOpenHistoryEvent(wParam { POpenEventParams } : WPARAM; lParam: LPARAM): uint_ptr; cdecl;
var
  hDbEvent: THandle;
  item, sel: Integer;
  oep: TOpenEventParams;
begin
  if Assigned(POpenEventParams(wParam)) then
  begin
    oep := POpenEventParams(wParam)^;
    hDbEvent := db_event_last(oep.hContact);
    item := 0;
    sel := -1;
    while (hDbEvent <> oep.hDbEvent) and (hDbEvent <> 0) do
    begin
      hDbEvent := db_event_prev(oep.hContact,hDbEvent);
      Inc(item);
    end;
    if hDbEvent = oep.hDbEvent then
      sel := item;
    OpenContactHistory(oep.hContact, sel);
  end;
  Result := 0;
end;

// MS_HPP_EMPTYHISTORY service
// See m_historypp.inc for details
function HppEmptyHistory(wParam { hContact } : WPARAM; lParam { 0 } : LPARAM): uint_ptr; cdecl;
var
  wHistory: THistoryFrm;
begin
  wHistory := FindContactWindow(wParam);
  with TEmptyHistoryFrm.Create(wHistory) do
  begin
    Contact := wParam;
    Result := int_ptr(ShowModal = mrYes);
    Free;
  end;
end;

procedure hppRegisterServices;
begin
  HstWindowList := TList.Create;

  CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY,@HppShowHistory);
  CreateServiceFunction(MS_HPP_EMPTYHISTORY, @HppEmptyHistory);
  CreateServiceFunction(MS_HPP_GETVERSION, @HppGetVersion);
  CreateServiceFunction(MS_HPP_SHOWGLOBALSEARCH,@HppShowGlobalSearch);
  CreateServiceFunction(MS_HPP_OPENHISTORYEVENT,@HppOpenHistoryEvent);

  hHppRichEditItemProcess := CreateHookableEvent(ME_HPP_RICHEDIT_ITEMPROCESS);
  HookEvent(ME_HPP_RICHEDIT_ITEMPROCESS,AllHistoryRichEditProcess);
end;

procedure hppUnregisterServices;
begin
  CloseHistoryWindows;
  CloseGlobalSearchWindow;
  DestroyHookableEvent(hHppRichEditItemProcess);
  HstWindowList.Free;
end;

end.
