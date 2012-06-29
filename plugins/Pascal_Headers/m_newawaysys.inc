{
  New Away System plugin for Miranda IM
  Copyright (C) 2005 Chervov Dmitry

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
}
{$IFNDEF M_NEWAWAYSYS}
{$DEFINE M_NEWAWAYSYS}

const
// NAS_PROTOINFO::Flags constants
  PIF_NO_CLIST_SETSTATUSMODE = 1; // NAS won't call MS_CLIST_SETSTATUSMODE service on a global status change, if this flag is set. it's useful if you want to change the global message and status in NAS without changing current "real" protocol statuses. NAS ignores this flag if szProto != NULL
  PIF_NOTTEMPORARY           = 2; // usually you should NOT set this flag
// MS_NAS_SETSTATE: NAS will overwrite current user-defined message for szProto if this flag is specified; otherwise (if the flag isn't specified), your szMsg will be stored only until the next szProto status change, and won't overwrite any messages specified by user
// MS_NAS_GETSTATE: NAS ignores any temporary messages and returns only non-temporary ones. this flag affects something only when status == 0

type
  PNAS_PROTOINFO = ^TNAS_PROTOINFO;
  TNAS_PROTOINFO = record
    cbSize : int;
    szProto: PAnsiChar; // pointer to protocol modulename (NULL means global)
    Msg    : TChar; // pointer to the status message (may be NULL -
                    // means that there's no specific message for this protocol)
{
  Be aware that MS_NAS_GETSTATE allocates memory for szMsg through Miranda's
  memory management interface (MS_SYSTEM_GET_MMI). And MS_NAS_SETSTATE expects
  szMsg to be allocated through the same service. MS_NAS_SETSTATE deallocates szMsg.
}
    status : WORD; // status mode. 0 means current (NAS will overwrite 0 with
                   // the current status mode)
{
  for MS_NAS_GETSTATE if the specified status is not 0, MS_NAS_GETSTATE will
  return the default/last status message (depends on settings) - i.e. the same
  message that will be shown by default when user changes status to the specified
  one. please note that, for example, if current status mode is ID_STATUS_AWAY,
  then status messages returned by MS_NAS_GETSTATE for status=0 and
  status=ID_STATUS_AWAY may be different! for status=ID_STATUS_AWAY it always
  returns the default/last status message, and for status=0 it returns
  _current_ status message.
}
    Flags  : int;
  end;

const
// MS_NAS_GETSTATE
// Fills specified array of NAS_PROTOINFO items with protocol data.
// You must construct the array and specify cbSize and szProto fields of
// all items in the array before calling this service.
// Remember to free szMsg fields through Miranda's MMI if you don't pass them back to NAS through MS_NAS_SETSTATE later.
// wParam = (WPARAM)(NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items to be filled.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
  MS_NAS_GETSTATEA:PAnsiChar = 'NewAwaySystem/GetStateA';
  MS_NAS_GETSTATEW:PAnsiChar = 'NewAwaySystem/GetStateW';

// MS_NAS_SETSTATE
// Changes status mode and message of specified protocols.
// (Note that this service deallocates szMsg field of the specified items through
// Miranda's MMI, so the array is not valid anymore after MS_NAS_SETSTATE returns!)
// wParam = (NAS_PROTOINFO*)pi - pointer to an array of NAS_PROTOINFO items.
// lParam = (LPARAM)(int)protoCount - number of items in pi.
// returns 0 on success
  MS_NAS_SETSTATEA:PAnsiChar = 'NewAwaySystem/SetStateA';
  MS_NAS_SETSTATEW:PAnsiChar = 'NewAwaySystem/SetStateW';
var
  MS_NAS_GETSTATE:PAnsiChar absolute MS_NAS_GETSTATEW;
  MS_NAS_SETSTATE:PAnsiChar absolute MS_NAS_SETSTATEW;

const
// NAS_ISWINFO::Flags constants
  ISWF_NOCOUNTDOWN = 1; // don't start the countdown to close the window
  ISWF_UNICODE     = 2; // specifies that NAS_ISWINFO::szMsg is a WCHAR*
type
  PNAS_ISWINFO = ^TNAS_ISWINFO;
  TNAS_ISWINFO = record
    cbSize  :int;
    szProto :PAnsiChar;   // pointer to initial protocol modulename (NULL means
                      // global); ignored when hContact is not NULL.
    hContact:THANDLE; // NAS will select this contact in the window initially,
                      // if it's not NULL.
    Msg     :TChar;   // pointer to an initial status message (may be NULL,
                      // NAS will use the default message then)
    status  :word;    // status mode. 0 means current.
    iFlags  :int;     // a combination of ISWF_ constants
  end;

const
// MS_NAS_INVOKESTATUSWINDOW
// Invokes the status message change window.
// Though if the window is open already, this service just activates an existing window and changes protocol status (i.e. it ignores szMsg and hContact). This behavior may change in future.
// wParam = (WPARAM)(NAS_ISWINFO*)iswi - pointer to a NAS_ISWINFO structure.
// lParam = 0
// returns HWND of the window on success, or NULL on failure.
  MS_NAS_INVOKESTATUSWINDOW:PAnsiChar = 'NewAwaySystem/InvokeStatusWindow';

{$ENDIF}
