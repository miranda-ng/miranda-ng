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

{-----------------------------------------------------------------------------
 hpp_mescatcher (historypp project)

 Version:   1.0
 Created:   09.12.2006
 Author:    theMIROn

 [ Description ]

 Hidden window, used for catching WM messages and hotkeys

 [ History ]

 1.0 (09.12.2006)
   First version

 [ Modifications ]
 none

 [ Known Issues ]
 none

 Contributors: theMIROn
-----------------------------------------------------------------------------}

unit hpp_mescatcher;

interface

{.$DEFINE USE_CUSTOMIDLEHOOK}

uses
  Windows, Messages, Classes, Controls;

const
  hppWindowClassName = 'History++ MainWindow';

var
  hppMainWindow: HWND = 0;

procedure hppWakeMainThread(Sender: TObject);
function hppRegisterMainWindow: Boolean;
function hppUnregisterMainWindow: Boolean;

implementation

uses Forms, Themes;

{$IFDEF USE_CUSTOMIDLEHOOK}
type
  THackApplication = class(TComponent)
  protected
    FxxxxxxxxxHandle: HWnd;
    FxxxxxxxxxBiDiMode: TBiDiMode;
    FxxxxxxxxxBiDiKeyboard: AnsiString;
    FxxxxxxxxxNonBiDiKeyboard: AnsiString;
    FxxxxxxxxxObjectInstance: Pointer;
    FxxxxxxxxxMainForm: TForm;
    FMouseControl: TControl;
  end;
{$ENDIF}

var
  SavedWakeMainThread: TNotifyEvent = nil;
  SavedCheckIniChange: function (var Message: TMessage): Boolean of object = nil;
  {$IFDEF USE_CUSTOMIDLEHOOK}
  ForegroundIdleHook: HHOOK;
  {$ENDIF}

function MainWindowWndProc(hwndDlg: HWND; uMsg: uint; wParam: WPARAM; lParam: LPARAM): lresult; stdcall;
var
  Message: TMessage;
begin
  Result := 0;
  if Assigned(SavedCheckIniChange) then
  begin
    Message.Msg := uMsg;
    SavedCheckIniChange(Message);
  end;
  case uMsg of
    //WM_HOTKEY:
    // place for global hotkeys :)
    //if wParam = Hotkey then
    //  CallService(MS_HPP_SHOWGLOBALSEARCH,0,0);
    CM_WINDOWHOOK: begin
      if (wParam = 0) and not Assigned(SavedCheckIniChange) then
        SavedCheckIniChange := TWindowHook(Pointer(LParam)^);
    end;
    WM_SETTINGCHANGE: begin
      // workaround to force vcl notice mouse setting changed
      if wParam = SPI_SETWHEELSCROLLLINES then
        Mouse.SettingChanged(SPI_GETWHEELSCROLLLINES)
      else
        Mouse.SettingChanged(wParam);
      Result := DefWindowProc(hwndDlg, uMsg, wParam, lParam);
    end;
    WM_FONTCHANGE: begin
      Screen.ResetFonts;
      Result := DefWindowProc(hwndDlg, uMsg, wParam, lParam);
    end;
    WM_THEMECHANGED:
      StyleServices.ApplyThemeChange;
    WM_NULL:
      CheckSynchronize;
    else
      Result := DefWindowProc(hwndDlg, uMsg, wParam, lParam);
  end;
end;

{$IFDEF USE_CUSTOMIDLEHOOK}
function IdleHookProc(code: Integer; wParam: WPARAM; lParam: LPARAM): Integer; stdcall;
var
  Control: TControl;
  MouseControl: TControl;
  CaptureControl: TControl;
  P: TPoint;
begin
  if code < 0 then
  begin
    Result := CallNextHookEx(ForegroundIdleHook,code,wParam,lParam);
    exit;
  end;
  GetCursorPos(P);
  Control := FindDragTarget(P, True);
  MouseControl := THackApplication(Application).FMouseControl;
  CaptureControl := GetCaptureControl;
  if MouseControl <> Control then
  begin
    if ((MouseControl <> nil) and (CaptureControl = nil)) or
       ((CaptureControl <> nil) and (MouseControl = CaptureControl)) then
      MouseControl.Perform(CM_MOUSELEAVE, 0, 0);
    MouseControl := Control;
    if ((MouseControl <> nil) and (CaptureControl = nil)) or
       ((CaptureControl <> nil) and (MouseControl = CaptureControl)) then
      MouseControl.Perform(CM_MOUSEENTER, 0, 0);
    THackApplication(Application).FMouseControl := MouseControl;
  end;
  if Application.ShowHint and (MouseControl = nil) then
    Application.CancelHint;
  Result := 1;
end;
{$ENDIF}

function hppRegisterMainWindow: Boolean;
var
  WndClass: TWNDCLASS;
begin
  Result := False;
  ZeroMemory(@WndClass,SizeOf(WndClass));
  WndClass.lpfnWndProc   := @MainWindowWndProc;
  WndClass.hInstance     := GetModuleHandle(nil);
  WndClass.lpszClassName := hppWindowClassName;
  if Windows.RegisterClass(WndClass) = 0 then exit;
  hppMainWindow := CreateWindow(hppWindowClassName,hppWindowClassName,WS_DISABLED,
                             0,0,0,0,0,0,WndClass.hInstance,nil);
  Result := (hppMainWindow <> 0);
  if Result then
  begin
    // assign Application.CheckIniChange function
    Application.Handle := hppMainWindow;
    Application.Handle := 0;
    SavedWakeMainThread := Classes.WakeMainThread;
    @Classes.WakeMainThread := @hppWakeMainThread;
  end;
  {$IFDEF USE_CUSTOMIDLEHOOK}
  ForegroundIdleHook := SetWindowsHookEx(WH_FOREGROUNDIDLE,
                             @IdleHookProc,0,GetCurrentThreadID);
  {$ENDIF}
end;

function hppUnregisterMainWindow: Boolean;
begin
  if hppMainWindow <> 0 then
  begin
    DestroyWindow(hppMainWindow);
    hppMainWindow := 0;
  end;
  Result := Boolean(Windows.UnregisterClass(hppWindowClassName,GetModuleHandle(nil)));
  Classes.WakeMainThread := SavedWakeMainThread;
  {$IFDEF USE_CUSTOMIDLEHOOK}
  if ForegroundIdleHook <> 0 then UnhookWindowsHookEx(ForegroundIdleHook);
  {$ENDIF}
end;

procedure hppWakeMainThread(Sender: TObject);
begin
  PostMessage(hppMainWindow, WM_NULL, 0, 0);
  if Assigned(SavedWakeMainThread) then
    SavedWakeMainThread(Sender);
end;

end.
