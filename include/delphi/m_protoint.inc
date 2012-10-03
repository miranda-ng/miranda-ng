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

{$IFNDEF M_PROTOINT}
{$DEFINE M_PROTOINT}

type
  TPROTOEVENTTYPE = (
    EV_PROTO_ONLOAD,
    EV_PROTO_ONREADYTOEXIT,
    EV_PROTO_ONEXIT,
    EV_PROTO_ONRENAME,
    EV_PROTO_ONOPTIONS,
 	  EV_PROTO_ONERASE,
    EV_PROTO_ONMENU,
    EV_PROTO_ONCONTACTDELETED,
    EV_PROTO_DBSETTINGSCHANGED);

type
	PROTOCHAR      = TCHAR;
	PROTOFILEEVENT = TPROTORECVFILE;

type
  PPROTO_INTERFACE = ^TPROTO_INTERFACE;
  TPROTO_INTERFACE = record
    iStatus       :int;
    iDesiredStatus:int;
    iXStatus      :int;
    iVersion      :int;  // version 2 or higher designate support of Unicode services
{
    bOldProto     :Bool;
    szPhysName    :PAnsiChar;
    szProtoName   :PAnsiChar;
    tszUserName   :TChar;
}
    tszUserName   :TChar;
    szProtoName   :PAnsiChar;
    szModuleName  :PAnsiChar;

    reserved      :array [0..39] of dword;

    AddToList       :function(intf:PPROTO_INTERFACE;flags:int; var psr:TPROTOSEARCHRESULT):THANDLE; cdecl;
    AddToListByEvent:function(intf:PPROTO_INTERFACE;flags:int; iContact:int; hDbEvent:THANDLE):THANDLE; cdecl;

    Authorize  :function(intf:PPROTO_INTERFACE;hContact:THANDLE):int; cdecl;
    AuthDeny   :function(intf:PPROTO_INTERFACE;hContact:THANDLE;const szReason:TChar):int; cdecl;
    AuthRecv   :function(intf:PPROTO_INTERFACE;hContact:THANDLE;var ptr:TPROTORECVEVENT):int; cdecl;
    AuthRequest:function(intf:PPROTO_INTERFACE;hContact:THANDLE;szMessage:TChar):int; cdecl;

    ChangeInfo:function(intf:PPROTO_INTERFACE;iInfoType:int; var pInfoData:pointer):THANDLE; cdecl;

    FileAllow :function(intf:PPROTO_INTERFACE;hContact:THANDLE;hTransfer:THANDLE; szPath:PROTOCHAR):THANDLE; cdecl;
    FileCancel:function(intf:PPROTO_INTERFACE;hContact:THANDLE;hTransfer:THANDLE):int; cdecl;
    FileDeny  :function(intf:PPROTO_INTERFACE;hContact:THANDLE;hTransfer:THANDLE; var szReason:PROTOCHAR):int; cdecl;
    FileResume:function(intf:PPROTO_INTERFACE;hTransfer:THANDLE;var action:Integer; var szFilename:PROTOCHAR):int; cdecl;

    GetCaps:function(intf:PPROTO_INTERFACE;_type:int):dword_ptr; cdecl;
    GetIcon:function(intf:PPROTO_INTERFACE;iconIndex:int):HICON; cdecl;
    GetInfo:function(intf:PPROTO_INTERFACE;hContact:THANDLE;infoType:int):int; cdecl;

    SearchBasic           :function(intf:PPROTO_INTERFACE;id:PAnsiChar):THANDLE; cdecl;
    SearchByEmail         :function(intf:PPROTO_INTERFACE;email:PAnsiChar):THANDLE; cdecl;
    SearchByName          :function(intf:PPROTO_INTERFACE;nick:PAnsiChar; const firstName:PAnsiChar; const lastName:PAnsiChar):THANDLE; cdecl;
    SearchAdvanced        :function(intf:PPROTO_INTERFACE;owner:HWND):HWND; cdecl;
    CreateExtendedSearchUI:function(intf:PPROTO_INTERFACE;owner:HWND):HWND; cdecl;

    RecvContacts:function(intf:PPROTO_INTERFACE;hContact:THANDLE;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvFile    :function(intf:PPROTO_INTERFACE;hContact:THANDLE;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvMessage :function(intf:PPROTO_INTERFACE;hContact:THANDLE;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvUrl     :function(intf:PPROTO_INTERFACE;hContact:THANDLE;var ptr:TPROTORECVEVENT):int; cdecl;

    SendContacts:function(intf:PPROTO_INTERFACE;hContact:THANDLE;flags:int; nContacts:int; var hContactsList:THANDLE):int; cdecl;
    SendFile    :function(intf:PPROTO_INTERFACE;hContact:THANDLE;szDescription:PROTOCHAR; var ppszFiles:PROTOCHAR):THANDLE; cdecl;
    SendMessage :function(intf:PPROTO_INTERFACE;hContact:THANDLE;flags:int; const msg:PAnsiChar):int; cdecl;
    SendUrl     :function(intf:PPROTO_INTERFACE;hContact:THANDLE;flags:int; const url:PAnsiChar):int; cdecl;

    SetApparentMode:function(intf:PPROTO_INTERFACE;hContact:THANDLE;mode:int):int; cdecl;
    SetStatus      :function(intf:PPROTO_INTERFACE;iNewStatus:int):int; cdecl;

    GetAwayMsg :function(intf:PPROTO_INTERFACE;hContact:THANDLE):THANDLE; cdecl;
    RecvAwayMsg:function(intf:PPROTO_INTERFACE;hContact:THANDLE;mode:int;var evt:TPROTORECVEVENT):int; cdecl;
    SendAwayMsg:function(intf:PPROTO_INTERFACE;hContact:THANDLE;hProcess:THANDLE;const msg:PAnsiChar):int; cdecl;
    SetAwayMsg :function(intf:PPROTO_INTERFACE;iStatus:int; const msg:PAnsiChar):int; cdecl;

    UserIsTyping:function(intf:PPROTO_INTERFACE;hContact:THANDLE; _type:int):int; cdecl;
    OnEvent:function(intf:PPROTO_INTERFACE;iEventType:TPROTOEVENTTYPE;wParam:WPARAM;lParam:LPARAM):int; cdecl;
  end;

 tagPROTO_INTERFACE = TPROTO_INTERFACE;

{$ENDIF}
