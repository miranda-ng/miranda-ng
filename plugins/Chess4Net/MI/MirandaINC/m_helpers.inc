(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

*)

{$ifdef M_API_UNIT}

    function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
    function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;

{$else}

    function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := (a shl 24) or (b shl 16) or (c shl 8) or d;
    end;

    function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := 0;
        { could be used to compare for severity of age for positive values, if a<b
        results are minus values, 0 for equal, positive if a is newer }
        Inc(Result, (verA and $FF) - (verB and $FF));
        Inc(Result, (verA and $FF00) - (verB and $FF00));
        Inc(Result, (verA and $FF0000) - (verB and $FF0000));
        Inc(Result, (verA and $FF000000) - (verB and $FF000000));
    end;

{$endif}

{$ifdef M_SYSTEM}
    {$ifdef M_API_UNIT}

    function CallService(const szService: PChar; wParam: WPARAM; lParam: LPARAM): int;

    function HookEvent(const szHook: PChar; hook_proc: TMIRANDAHOOK): int;

    function UnhookEvent(const hHook: THandle): int;

    function CreateServiceFunction(const szName: PChar; const MirandaService: TMIRANDASERVICE): int;

    {$else}

    function CallService(const szService: PChar; wParam: WPARAM; lParam: LPARAM): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(szService, wParam, lParam);
    end;

    function HookEvent(const szHook: PChar; hook_proc: TMIRANDAHOOK): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.HookEvent(szHook, @hook_proc);
    end;

    function UnhookEvent(const hHook: THandle): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.UnhookEvent(hHook);
    end;

    function CreateServiceFunction(const szName: PChar; const MirandaService: TMIRANDASERVICE): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CreateServiceFunction(szName, @MirandaService);
    end;

    {$endif}

{$endif}

{$ifdef M_DATABASE}

    {$ifdef M_API_UNIT}

    function DBGetContactSettingByte(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;

    function DBGetContactSettingWord(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;

    function DBGetContactSettingDword(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;

    function DBGetContactSetting(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; dbv: PDBVARIANT): Integer;

    function DBFreeVariant(dbv: PDBVARIANT): Integer;

    function DBDeleteContactSetting(hContact: THandle; const szModule: PChar; const szSetting: PChar): Integer;

    function DBWriteContactSettingByte(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: Byte): Integer;

    function DBWriteContactSettingWord(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: Word): Integer;

    function DBWriteContactSettingDWord(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: LongInt): Integer;

    function DBWriteContactSettingString(hContact: THandle; const szModule: PChar; const szSetting: PChar; const val: PChar): Integer;

    {$else}

    function DBGetContactSettingByte(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        dbv: TDBVARIANT;
        cgs: TDBCONTACTGETSETTING;
    begin

        cgs.szModule := szModule;
        cgs.szSetting := szSetting;
        cgs.pValue := @dbv;

        If PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then
            Result := ErrorValue
        else
            Result := dbv.bVal;
    end;

    function DBGetContactSettingWord(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        dbv: TDBVARIANT;
        cgs: TDBCONTACTGETSETTING;
    begin
        cgs.szModule := szModule;
        cgs.szSetting := szSetting;
        cgs.pValue := @dbv;
        If PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then
            Result := ErrorValue
        else
            Result := dbv.wVal;
    end;

    function DBGetContactSettingDword(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; errorValue: Integer): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        dbv: TDBVARIANT;
        cgs: TDBCONTACTGETSETTING;
    begin
        cgs.szModule := szModule;
        cgs.szSetting := szSetting;
        cgs.pValue := @dbv;
        If PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs)) <> 0 then
            Result := ErrorValue
        else
            Result := dbv.dVal;
    end;

    function DBGetContactSetting(hContact: THandle;
        const szModule: PChar; const szSetting: PChar; dbv: PDBVARIANT): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cgs: TDBCONTACTGETSETTING;
    begin
        cgs.szModule := szModule;
        cgs.szSetting := szSetting;
        cgs.pValue := dbv;
        Result := PluginLink^.CallService(MS_DB_CONTACT_GETSETTING, hContact, lParam(@cgs));
    end;

    function DBFreeVariant(dbv: PDBVARIANT): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(MS_DB_CONTACT_FREEVARIANT, 0, lParam(dbv));
    end;

    function DBDeleteContactSetting(hContact: THandle; const szModule: PChar; const szSetting: PChar): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cgs: TDBCONTACTGETSETTING;
    begin
        cgs.szModule := szModule;
        cgs.szSetting := szSetting;
        Result := PluginLink^.CallService(MS_DB_CONTACT_DELETESETTING, hContact, lParam(@cgs));
    end;

    function DBWriteContactSettingByte(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: Byte): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cws: TDBCONTACTWRITESETTING;
    begin
        cws.szModule := szModule;
        cws.szSetting := szSetting;
        cws.value.type_ := DBVT_BYTE;
        cws.value.bVal := Val;
        Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
    end;

    function DBWriteContactSettingWord(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: Word): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cws: TDBCONTACTWRITESETTING;
    begin
        cws.szModule := szModule;
        cws.szSetting := szSetting;
        cws.value.type_ := DBVT_WORD;
        cws.value.wVal := Val;
        Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
    end;

    function DBWriteContactSettingDWord(hContact: THandle; const szModule: PChar; const szSetting: PChar; val: LongInt): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cws: TDBCONTACTWRITESETTING;
    begin
        cws.szModule := szModule;
        cws.szSetting := szSetting;
        cws.value.type_ := DBVT_DWORD;
        cws.value.dVal := Val;
        Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
    end;

    function DBWriteContactSettingString(hContact: THandle; const szModule: PChar; const szSetting: PChar; const val: PChar): Integer;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        cws: TDBCONTACTWRITESETTING;
    begin
        cws.szModule := szModule;
        cws.szSetting := szSetting;
        cws.value.type_ := DBVT_ASCIIZ;
        cws.value.pszVal := Val;
        Result := PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
    end;

    {$endif}

{$endif}

{$ifdef M_NETLIB}

    {$ifdef M_API_UNIT}

    function Netlib_CloseHandle(Handle: THandle): int;

    function Netlib_GetBase64DecodedBufferSize(const cchEncoded: int): int;

    function Netlib_GetBase64EncodedBufferSize(const cbDecoded: int): int;

    function Netlib_Send(hConn: THandle; const buf: PChar; len: int; flags: int): int;

    function Netlib_Recv(hConn: THandle; const buf: PChar; len: int; flags: int): int;

    procedure Netlib_Log(hNetLib: THandle; const sz: PChar);

    {$else}

    function Netlib_CloseHandle(Handle: THandle): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(MS_NETLIB_CLOSEHANDLE, Handle, 0);
    end;

    function Netlib_GetBase64DecodedBufferSize(const cchEncoded: int): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := (cchEncoded shr 2) * 3;
    end;

    function Netlib_GetBase64EncodedBufferSize(const cbDecoded: int): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := (cbDecoded  * 4+11) div 12*4+1;
    end;

    function Netlib_Send(hConn: THandle; const buf: PChar; len: int; flags: int): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        nlb: TNETLIBBUFFER;
    begin
        nlb.buf := buf;
        nlb.len := len;
        nlb.flags := flags;
        Result := PluginLink^.CallService(MS_NETLIB_SEND, wParam(hConn), lParam(@nlb));
    end;

    function Netlib_Recv(hConn: THandle; const buf: PChar; len: int; flags: int): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        nlb: TNETLIBBUFFER;
    begin
        nlb.buf := buf;
        nlb.len := len;
        nlb.flags := flags;
        Result := PluginLink^.CallService(MS_NETLIB_RECV, wParam(hConn), lParam(@nlb));
    end;

    procedure Netlib_Log(hNetLib: THandle; const sz: PChar);
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        PluginLink^.CallService(MS_NETLIB_LOG, hNetLib, lParam(sz));
    end;

    {$endif}

{$endif}

{$ifdef M_UTILS}

    {$ifdef M_API_UNIT}

	function WindowList_Add(hList: THandle; hWnd: HWND; hContact: THandle): int;

 	function WindowList_Remove(hList: THandle; hWnd: THandle): int;

	function WindowList_Find(hList: THandle; hContact: THandle): int;

    function WindowList_Broadcast(hList: THandle; message: int; wParam: WPARAM; lParam: LPARAM): int;

    function Utils_SaveWindowPosition(hWnd: THandle; hContact: THandle; const szModule, szNamePrefix: PChar): int;

	function Utils_RestoreWindowPosition(hWnd: THandle; hContact: THandle; Flags: int; const szModule, szNamePrefix: PChar): int;

    {$else}

    function WindowList_Add(hList: THandle; hWnd: hWnd; hContact: THandle): int;
    var
        wle: TWINDOWLISTENTRY;
    begin
        wle.hList    := hList;
        wle.hWnd     := hWnd;
        wle.hContact := hContact;
        Result := PluginLink^.CallService(MS_UTILS_ADDTOWINDOWLIST, 0, lParam(@wle));
    end;

    function WindowList_Remove(hList: THandle; hWnd: THandle): int;
    begin
        Result := PluginLink^.CallService(MS_UTILS_REMOVEFROMWINDOWLIST, hList, hWnd);
    end;

    function WindowList_Find(hList: THandle; hContact: THandle): int;
    begin
        Result := PluginLink^.CallService(MS_UTILS_FINDWINDOWINLIST, hList, hContact);
    end;

    function WindowList_Broadcast(hList: THandle; message: int; wParam: WPARAM; lParam: LPARAM): int;
    var
        msg: TMSG;
    begin
        msg.message := message;
        msg.wParam  := wParam;
        msg.lParam  := lParam;
        Result := PluginLink^.CallService(MS_UTILS_BROADCASTTOWINDOWLIST, hList, Integer(@Msg));
    end;

    function Utils_SaveWindowPosition(hWnd: THandle; hContact: THandle; const szModule, szNamePrefix: PChar): int;
    var
        swp: TSAVEWINDOWPOS;
    begin
        swp.hWnd := hWnd;
        swp.hContact := hContact;
        swp.szModule := szModule;
        swp.szNamePrefix := szNamePrefix;
        Result := PluginLink^.CallService(MS_UTILS_SAVEWINDOWPOSITION, 0, lParam(@swp));
    end;

    function Utils_RestoreWindowPosition(hWnd: THandle; hContact: THandle; Flags: int; const szModule, szNamePrefix: PChar): int;
    var
        swp: TSAVEWINDOWPOS;
    begin
        swp.hWnd         := hWnd;
        swp.hContact     := hContact;
        swp.szModule     := szModule;
        swp.szNamePrefix := szNamePrefix;
        Result := PluginLink^.CallService(MS_UTILS_RESTOREWINDOWPOSITION, Flags, lParam(@swp));
    end;

    {$endif}

{$endif}

{$ifdef M_LANGPACK}

    {$ifdef M_API_UNIT}

	function Translate(sz: PChar): PChar;

	function TranslateString(sz: string): string;

	function TranslateDialogDefault(hwndDlg: THandle): int;

    {$else}

    function Translate(sz: PChar): PChar;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        { the return value maybe NULL(0) -- it's upto the caller to know if the allocated
        string has to be removed from the DLL heap, this has little to do with Miranda,
        but if a dynamic string is passed and a return string is used -- the dynamic
        string is lost -- be careful, lazy? use TranslateString (note it's slower) }
        Result := PChar(PluginLink^.CallService(MS_LANGPACK_TRANSLATESTRING, 0, lParam(sz)));
    end;

    function TranslateString(sz: string): string;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
         Result := string(PChar( PluginLink^.CallService(MS_LANGPACK_TRANSLATESTRING, 0, lParam(sz))));
    end;

    function TranslateDialogDefault(hwndDlg: THandle): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        lptd: TLANGPACKTRANSLATEDIALOG;
    begin
        lptd.cbSize := sizeof(lptd);
        lptd.flags := 0;
        lptd.hwndDlg := hwndDlg;
        lptd.ignoreControls := nil;
        Result := PluginLink^.CallService(MS_LANGPACK_TRANSLATEDIALOG, 0, lParam(@lptd));
    end;

    {$endif}

{$endif}

{$ifdef M_PROTOCOLS}
    {$ifdef M_API_UNIT}

        function CallContactService(hContact: THandle; const szProtoService: PChar; wParam: WPARAM; lParam: LPARAM): int;

        function CallProtoService(const szModule, szService: PChar; wParam: WPARAM; lParam: LPARAM): int;

    {$else}

        function CallContactService(hContact: THandle; const szProtoService: PChar; wParam: WPARAM; lParam: LPARAM): int;
        {$ifdef FPC}
        inline;
        {$endif}
        var
            css: TCCSDATA;
        begin
            css.hContact := hContact;
            css.szProtoService := szProtoService;
            css.wParam := wParam;
            css.lParam := lParam;
            Result := PluginLink^.CallService(MS_PROTO_CALLCONTACTSERVICE, 0, Integer(@css));
        end;

        function CallProtoService(const szModule, szService: PChar; wParam: WPARAM; lParam: LPARAM): int;
        {$ifdef FPC}
        inline;
        {$endif}
        var
            szStr: array[0..MAXMODULELABELLENGTH] of Char;
        begin
            strcpy(szStr, szModule);
            strcat(szStr, szService);
            Result := PluginLink^.CallService(szStr, wParam, lParam);
        end;

    {$endif}
{$endif}

{$ifdef M_PROTOMOD}
    {$ifdef M_API_UNIT}

    function ProtoBroadcastAck(const szModule: PChar; hContact: THandle; type_: int; result_: int; hProcess: THandle; lParam: LPARAM): int;

    function CreateProtoServiceFunction(const szModule, szService: PChar; serviceProc: TMIRANDASERVICE): int;

    {$else}

    function ProtoBroadcastAck(const szModule: PChar; hContact: THandle; type_: int; result_: int; hProcess: THandle; lParam: LPARAM): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        ack: TACKDATA;
    begin
        ack.cbSize   := sizeof(TACKDATA);
        ack.szModule := szModule;
        ack.hContact := hContact;
        ack.type_    := type_;
        ack.result_  := result_;
        ack.hProcess := hProcess;
        ack.lParam   := lParam;
        Result := PluginLink^.CallService(MS_PROTO_BROADCASTACK, 0, Integer(@ack));
    end;

    function CreateProtoServiceFunction(const szModule, szService: PChar; serviceProc: TMIRANDASERVICE): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        szStr: array[0..MAXMODULELABELLENGTH] of Char;
    begin
        strcpy(szStr, szModule);
        strcat(szStr, szService);
        Result := PluginLink^.CreateServiceFunction(szStr, @serviceProc);
    end;

    {$endif}

{$endif}

{$ifdef M_SKIN}

    {$ifdef M_API_UNIT}

    function LoadSkinnedIcon(id: int): THandle;

    function LoadSkinnedProtoIcon(const szProto: PChar; status: int): THandle;

    function SkinAddNewSound(const name, description, defaultFile: PChar): int;

    function SkinPlaySound (const name: PChar): int;

    {$else}

    function LoadSkinnedIcon(id: int): THandle;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(MS_SKIN_LOADICON, id, 0);
    end;

    function LoadSkinnedProtoIcon(const szProto: PChar; status: int): THandle;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(MS_SKIN_LOADPROTOICON, wParam(szProto), status);
    end;

    function SkinAddNewSound(const name, description, defaultFile: PChar): int;
    {$ifdef FPC}
    inline;
    {$endif}
    var
        ssd: TSKINSOUNDDESC;
    begin
        ssd.cbSize := sizeof(ssd);
        ssd.pszName := name;
        ssd.pszDescription := description;
        ssd.pszDefaultFile := defaultFile;
        Result := PluginLink^.CallService(MS_SKIN_ADDNEWSOUND, 0, lParam(@ssd));
    end;

    function SkinPlaySound (const name: PChar): int;
    {$ifdef FPC}
    inline;
    {$endif}
    begin
        Result := PluginLink^.CallService(MS_SKIN_PLAYSOUND, 0, lParam(name));
    end;

    {$endif}

{$endif}
