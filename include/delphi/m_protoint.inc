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

    tszUserName   :TChar;
    szModuleName  :PAnsiChar;
    hProtoIcon    :THandle;
    hNetlibUser   :THandle;
	//////////////////////////////////////////////////////////////////////////////////////
	// Helpers
(*!!
	__forceinline void debugLogA(LPCSTR szFormat, ...)
	{
		va_list args;
		va_start(args, szFormat);
		ProtoLogA(this, szFormat, args);
	}
	__forceinline void debugLogW(LPCWSTR wszFormat, ...)
	{
		va_list args;
		va_start(args, wszFormat);
		ProtoLogW(this, wszFormat, args);
	}

	__forceinline INT_PTR ProtoBroadcastAck(HCONTACT hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam) {
		return ::ProtoBroadcastAck(m_szModuleName, hContact, type, hResult, hProcess, lParam); }

	__forceinline INT_PTR delSetting(const char *name) { return db_unset(NULL, m_szModuleName, name); }
	__forceinline INT_PTR delSetting(HCONTACT hContact, const char *name) { return db_unset(hContact, m_szModuleName, name); }

	__forceinline bool getBool(const char *name, bool defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue) != 0; }
	__forceinline bool getBool(HCONTACT hContact, const char *name, bool defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue) != 0; }

	__forceinline bool isChatRoom(HCONTACT hContact) { return getBool(hContact, "ChatRoom", false); }

	__forceinline int getByte(const char *name, BYTE defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getByte(HCONTACT hContact, const char *name, BYTE defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue); }

	__forceinline int getWord(const char *name, WORD defaultValue) {
		return db_get_w(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getWord(HCONTACT hContact, const char *name, WORD defaultValue) {
		return db_get_w(hContact, m_szModuleName, name, defaultValue); }

	__forceinline DWORD getDword(const char *name, DWORD defaultValue)  {
		return db_get_dw(NULL, m_szModuleName, name, defaultValue); }
	__forceinline DWORD getDword(HCONTACT hContact, const char *name, DWORD defaultValue) {
		return db_get_dw(hContact, m_szModuleName, name, defaultValue); }

	__forceinline INT_PTR getString(const char *name, DBVARIANT *result) {
		return db_get_s(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getString(HCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_s(hContact, m_szModuleName, name, result); }

	__forceinline INT_PTR getWString(const char *name, DBVARIANT *result) {
		return db_get_ws(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getWString(HCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_ws(hContact, m_szModuleName, name, result); }

	__forceinline char* getStringA(const char *name) {
		return db_get_sa(NULL, m_szModuleName, name); }
	__forceinline char* getStringA(HCONTACT hContact, const char *name) {
		return db_get_sa(hContact, m_szModuleName, name); }

	__forceinline WCHAR* getWStringA(const char *name) {
		return db_get_wsa(NULL, m_szModuleName, name); }
	__forceinline WCHAR* getWStringA(HCONTACT hContact, const char *name) {
		return db_get_wsa(hContact, m_szModuleName, name); }

	__forceinline void setByte(const char *name, BYTE value) { db_set_b(NULL, m_szModuleName, name, value); }
	__forceinline void setByte(HCONTACT hContact, const char *name, BYTE value) { db_set_b(hContact, m_szModuleName, name, value); }

	__forceinline void setWord(const char *name, WORD value) { db_set_w(NULL, m_szModuleName, name, value); }
	__forceinline void setWord(HCONTACT hContact, const char *name, WORD value) { db_set_w(hContact, m_szModuleName, name, value); }

	__forceinline void setDword(const char *name, DWORD value) { db_set_dw(NULL, m_szModuleName, name, value); }
	__forceinline void setDword(HCONTACT hContact, const char *name, DWORD value) { db_set_dw(hContact, m_szModuleName, name, value); }

	__forceinline void setString(const char *name, const char* value) { db_set_s(NULL, m_szModuleName, name, value); }
	__forceinline void setString(HCONTACT hContact, const char *name, const char* value) { db_set_s(hContact, m_szModuleName, name, value); }

	__forceinline void setWString(const char *name, const WCHAR* value) { db_set_ws(NULL, m_szModuleName, name, value); }
	__forceinline void setWString(HCONTACT hContact, const char *name, const WCHAR* value) { db_set_ws(hContact, m_szModuleName, name, value); }

*)
	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

    AddToList       :function(intf:PPROTO_INTERFACE;flags:int; var psr:TPROTOSEARCHRESULT):HCONTACT; cdecl;
    AddToListByEvent:function(intf:PPROTO_INTERFACE;flags:int; iContact:int; hDbEvent:THANDLE):HCONTACT; cdecl;

    Authorize  :function(intf:PPROTO_INTERFACE;hDbEvent:THANDLE):int; cdecl;
    AuthDeny   :function(intf:PPROTO_INTERFACE;hDbEvent:THANDLE;const szReason:TChar):int; cdecl;
    AuthRecv   :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;var ptr:TPROTORECVEVENT):int; cdecl;
    AuthRequest:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;szMessage:TChar):int; cdecl;

    ChangeInfo:function(intf:PPROTO_INTERFACE;iInfoType:int; var pInfoData:pointer):THANDLE; cdecl;

    FileAllow :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;hTransfer:THANDLE; szPath:PROTOCHAR):THANDLE; cdecl;
    FileCancel:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;hTransfer:THANDLE):int; cdecl;
    FileDeny  :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;hTransfer:THANDLE; var szReason:PROTOCHAR):int; cdecl;
    FileResume:function(intf:PPROTO_INTERFACE;hTransfer:THANDLE;var action:Integer; var szFilename:PROTOCHAR):int; cdecl;

    GetCaps:function(intf:PPROTO_INTERFACE;_type:int;hContact:HCONTACT=0):dword_ptr; cdecl;
    GetInfo:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;infoType:int):int; cdecl;

    SearchBasic           :function(intf:PPROTO_INTERFACE;id:PAnsiChar):THANDLE; cdecl;
    SearchByEmail         :function(intf:PPROTO_INTERFACE;email:PAnsiChar):THANDLE; cdecl;
    SearchByName          :function(intf:PPROTO_INTERFACE;nick:PAnsiChar; const firstName:PAnsiChar; const lastName:PAnsiChar):THANDLE; cdecl;
    SearchAdvanced        :function(intf:PPROTO_INTERFACE;owner:HWND):HWND; cdecl;
    CreateExtendedSearchUI:function(intf:PPROTO_INTERFACE;owner:HWND):HWND; cdecl;

    RecvContacts:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvFile    :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvMessage :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;var ptr:TPROTORECVEVENT):int; cdecl;
    RecvUrl     :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;var ptr:TPROTORECVEVENT):int; cdecl;

    SendContacts:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;flags:int; nContacts:int; var hContactsList:THANDLE):int; cdecl;
    SendFile    :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;szDescription:PROTOCHAR; var ppszFiles:PROTOCHAR):THANDLE; cdecl;
    SendMessage :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;flags:int; const msg:PAnsiChar):int; cdecl;
    SendUrl     :function(intf:PPROTO_INTERFACE;hContact:HCONTACT;flags:int; const url:PAnsiChar):int; cdecl;

    SetApparentMode:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;mode:int):int; cdecl;
    SetStatus      :function(intf:PPROTO_INTERFACE;iNewStatus:int):int; cdecl;

    GetAwayMsg :function(intf:PPROTO_INTERFACE;hContact:HCONTACT):THANDLE; cdecl;
    RecvAwayMsg:function(intf:PPROTO_INTERFACE;hContact:HCONTACT;mode:int;var evt:TPROTORECVEVENT):int; cdecl;
    SetAwayMsg :function(intf:PPROTO_INTERFACE;iStatus:int; const msg:PAnsiChar):int; cdecl;

    UserIsTyping:function(intf:PPROTO_INTERFACE;hContact:HCONTACT; _type:int):int; cdecl;
    OnEvent:function(intf:PPROTO_INTERFACE;iEventType:TPROTOEVENTTYPE;wParam:WPARAM;lParam:LPARAM):int; cdecl;
  end;

 tagPROTO_INTERFACE = TPROTO_INTERFACE;

{$ENDIF}
