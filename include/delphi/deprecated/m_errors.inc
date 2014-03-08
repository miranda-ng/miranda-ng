{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}

{$IFNDEF M_ERRORS}
{$DEFINE M_ERRORS}

const
  MERR_UNICODE         = $01;
  MERR_DEFAULT_INFO    = $02;
  MERR_DEFAULT_WARNING = $04;
  MERR_DEFAULT_ERROR   = $08;
  MERR_DEFAULT_ALL     = $0E;

  MERR_TCHAR           = MERR_UNICODE;

// Error notifications are sorted according to this level
  MERR_LEVEL_INFO    = 1;
  MERR_LEVEL_WARNING = 2;
  MERR_LEVEL_ERROR   = 3;

// Predefined error types (no need to call MS_ERROR_REGISTER)
  MERR_TYPE_INFO       :pAnsiChar = 'Core/Info';
  MERR_TYPE_SRV_INFO   :pAnsiChar = 'Core/SrvInfo';
  MERR_TYPE_WARNING    :pAnsiChar = 'Core/Warning';
  MERR_TYPE_SRV_WARNING:pAnsiChar = 'Core/SrvWarning';
  MERR_TYPE_SRV_ERROR  :pAnsiChar = 'Core/SrvError';
  MERR_TYPE_NETWORK    :pAnsiChar = 'Core/Network';
  MERR_TYPE_LOGIN      :pAnsiChar = 'Core/Login';

// Specify set of buttons to make query box
  MERR_BTN_NONE             = 0;
  MERR_BTN_YESNO            = 1;
  MERR_BTN_YESNOCANCEL      = 2;
  MERR_BTN_ABORTRETRYIGNORE = 4;

// General-purpose error definition
type
  PMIRANDAERROR_TYPE = ^MIRANDAERROR_TYPE;
  MIRANDAERROR_TYPE = record
    cbSize :int;
    flags  :dword;
    level  :int;
    name   :PAnsiChar;
    szTitle:TChar;
  end;

// General-purpose error definition
type
  MIRANDAERROR = record
    cbSize        :int;
    flags         :dword;
    _type         :pAnsiChar;
    szModuleTitle :TCHAR;

    buttons       :int;
    btnDefault    :int;
    pszQueryName  :PAnsiChar; // to save answer in DB
    pszSvcCallback:PAnsiChar;
    lParam        :LPARAM;

  // information itself
    hContact      :THANDLE;
    szTitle       :TCHAR;
    szText        :TCHAR;

  // filled by core and may be used in handlers.
    typeInfo      :PMIRANDAERROR_TYPE;
    dwTimestamp   :dword;
  end;

// Information about particular error handler for options UI
type
  MIRANDAERROR_HANDLER = record
    cbSize     :int;
    flags      :dword;
    pszDbModule:PAnsiChar;

    hIcolibIcon:THANDLE;
    szTitle    :TCHAR;
  end;

const
  MS_ERROR_REGISTER  :PAnsiChar = 'Errors/Register';
  MS_ERROR_ADDHANDLER:PAnsiChar = 'Errors/AddHandler';
  MS_ERROR_PROCESS   :PAnsiChar = 'Errors/Process';
  ME_ERROR_ONPROCESS :PAnsiChar = 'Errors/OnProcess';

(*
  static __forceinline void mir_ReportError(HANDLE hContact, TCHAR *ptszModuleTitle,
         char *pszType, TCHAR *ptszText, TCHAR *ptszTitle DEFVAL(NULL))
  {
    MIRANDAERROR err = {0};
    err.cbSize = sizeof(err);
    err.flags = MERR_TCHAR;
    err.type = pszType;
    err.ptszModuleTitle = ptszModuleTitle;
    err.hContact = hContact;
    err.ptszText = ptszText;
    err.ptszTitle = ptszTitle;
    CallServiceSync(MS_ERROR_PROCESS, 0, (LPARAM)&err);
  }
*)

{$ENDIF}
