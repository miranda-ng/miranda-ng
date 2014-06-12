{
  DataAsMessage plugin for Miranda IM
  Copyright (c) 2006 Chervov Dmitry

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

{$IFNDEF M_DATAASMESSAGE}
{$DEFINE M_DATAASMESSAGE}

const
// DAM_SENDRESULTINFO::iResult values
  DAM_SR_SUCCESS         = 0;
  DAM_SR_TIMEOUT         = 1; // timeout period expired; this value is returned
                              // also if the contact went offline for a time
                              // longer than a timeout period
  DAM_SR_NOTSUPPORTED    = 2; // means this szDataType is not supported by the
                              // remote side
  DAM_SR_NODAM           = 3; // means there is no DataAsMessage plugin on the
                              // remote side; keep in mind that this error may
                              // also appear accidentally because of a bad
                              // connectivity during the handshake (if there
                              // was a timeout when waiting for a response)
  DAM_SR_CANCELLEDLOCAL  = 4; // cancelled from the local(sending) side
  DAM_SR_CANCELLEDREMOTE = 5; // cancelled from the remote(receiving) side
  DAM_SR_BADCRC          = 6; // bad CRC; we can't do anything with this error. presumably, it will happen rarely, and the most probable cause is the protocol that filters some of characters in our messages OR it may be a bug in DataAsMessage plugin (hopefully not ;) ).
  DAM_SR_UNKNOWN         = 7; // unknown error

// Return values for DAM_SENDRESULTPROC
  DAM_SRA_RETRY = 1;

type
// hContact, szDataType and SessionID fields correspond to the fields of the
// DAM_SENDDATAINFO structure
  PDAM_SENDRESULTINFO = ^TDAM_SENDRESULTINFO;
  TDAM_SENDRESULTINFO = record
    cbSize    :int; // sizeof(DAM_SENDRESULTINFO)
    hContact  :THANDLE;
    szDataType:PAnsiChar;
    SessionID :dword;
    iResult   :int; // transmission result code
  end; 

type
  TDAM_SENDRESULTPROC = function(sri:PDAM_SENDRESULTINFO):int; cdecl;
// this procedure receives the result of the transmission. it's called when the
// session closes (either the data was sent successfully or there was an error)
// you can return DAM_SRA_RETRY when iResult is DAM_SR_TIMEOUT if you want to
// retry sending

const
// DAM_SENDDATAINFO::Flags constants
  DAM_SDF_DONTPACK  = 1; // don't pack the data (by default all the data is packed)
  DAM_SDF_NOTIMEOUT = 2; // don't generate a timeout error ever, keep trying to
                         // send the data. If the contact is offline, the data
                         // is saved in the memory until the contact goes online.
                         // Loss of the data occurs only if the sender's miranda
                         // closes (this may change in future to allow fully
                         // functional offline sending that will guarantee the
                         // data to be sent in any case, but of course the
                         // sending starts only when the both contacts are
                         // online). other errors than the timeout error can be
                         // still generated though.

type
  TDAM_SENDDATAINFO = record
    cbSize    :int;     // sizeof(DAM_SENDDATAINFO)
    hContact  :THANDLE;
    szDataType:PAnsiChar;   // zero-terminated string, containing data type,
                        // preferably in format "YourPluginName" or
                        // "YourPluginName/Something" (make sure this string
                        // won't coincide by an accident with someone else's
                        // string!). you can identify your data by this ID later
    nDataLen  :int;     // keep in mind that if the length is too big (more than
                        // about 8 KB), it's more preferable to split your data
                        // into several chunks, as you won't be able to "pick
                        // up" your data at the other end until all the data is
                        // transferred
    cData     :PAnsiChar;
    Flags     :int;     // combination of the DAM_SDF_ constants
    SendAfterSessionID:dword; // may be NULL; otherwise it's guaranteed that the
                              // sending starts only after successful completion
                              // of SendAfterSessionID session
    SendResultProc:TDAM_SENDRESULTPROC; // pointer to a procedure that receives
                                        // the result; can be NULL
    SessionID :dword; // OUT; receives the session ID
  end;

const
// MS_DAM_SENDDATA return values
  DAM_SDA_NOERROR         = 0;
  DAM_SDA_NOTSUPPORTED    = -1; // contact's protocol doesn't support sending/
                                // receiving messages
  DAM_SDA_TOOMANYSESSIONS = -2; // too many sessions

// MS_DAM_SENDDATA
// sends the data
// wParam = (WPARAM)(DAM_SENDDATAINFO*)sdi;
// lParam = 0
// Returns 0 (DAM_SDA_NOERROR) and fills SessionID if the session was queued for sending successfully; returns one of the DAM_SDA_ values on failure
  MS_DAM_SENDDATA = 'DataAsMessage/SendData';

function DAMSendData(hContact:THANDLE; szDataType:PAnsiChar; nDataLen:int;
         cData:PAnsiChar; Flags:int; SendAfterSessionID:dword;
         SendResultProc:TDAM_SENDRESULTPROC;pSessionID:pdword):int;
var
  sdi:TDAM_SENDDATAINFO;
begin
  FillChar(sdi,SizeOf(sdi),0);
  sdi.cbSize    :=SizeOf(sdi);
  sdi.hContact  :=hContact;
  sdi.szDataType:=szDataType;
  sdi.nDataLen  :=nDataLen;
  sdi.cData     :=cData;
  sdi.Flags     :=Flags;
  sdi.SendAfterSessionID:=SendAfterSessionID;
  sdi.SendResultProc    :=SendResultProc;
  Result:=CallService(MS_DAM_SENDDATA,dword(@sdi),0);
  if pSessionID<>nil then
  	pSessionID^:=sdi.SessionID;
end;

type
  TDAM_RECVDATAINFO = record
    cbSize    :int; // sizeof(DAM_RECVDATAINFO)
    hContact  :THANDLE;
    szDataType:PAnsiChar;
    nDataLen  :int;
    cData     :PAnsiChar;
  end;

const
// ME_DAM_RECVDATA
// hook up to this event to check for incoming data
// make sure rdi->szDataType is yours before doing anything!
// The important thing here is that your plugin will receive TWO ME_DAM_RECVDATA notifications on every single MS_DAM_SENDDATA call from a remote side:
//   The first notification arrives when the remote side starts to transmit the data. In this case DAM_RECVDATAINFO::cData = NULL (and DAM_RECVDATAINFO::nDataLen = -1) as we didn't receive any data yet. Return 1 to indicate that your plugin recognized the DAM_RECVDATAINFO::szDataType, otherwise return 0. If there are no any plugin that recognized the data, DAM cancels the transfer and there won't be any second notification for it.
//   The second notification is when the data is transmitted successfully. nDataLen contains the usual data size and cData points to the data buffer. cData is guaranteed to be valid only during the ME_DAM_RECVDATA call. You must copy the data to your own plugin's memory if you need it later. again, return 1 to indicate that your plugin recognized the data, otherwise return 0
// wParam = (WPARAM)(DAM_RECVDATAINFO*)rdi;
// lParam = 0
  ME_DAM_RECVDATA = 'DataAsMessage/RecvData';

{$ENDIF}
