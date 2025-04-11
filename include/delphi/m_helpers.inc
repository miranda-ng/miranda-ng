{$IFDEF M_API_UNIT}

//----- CListint -----
function IsHContactGroup  (h:TMCONTACT):bool;
function IsHContactInfo   (h:TMCONTACT):bool;
function IsHContactContact(h:TMCONTACT):bool;

function CLCDEFAULT_BKCOLOUR:dword;
function CLCDEFAULT_TEXTCOLOUR:dword;
function CLCDEFAULT_SELBKCOLOUR:dword;
function CLCDEFAULT_SELTEXTCOLOUR:dword;
function CLCDEFAULT_HOTTEXTCOLOUR:dword;

function CreateVersionString(version:dword;buf:PAnsiChar):PAnsiChar;
function CreateVersionStringPlugin(pluginInfo:PPluginInfoEx;buf:PAnsiChar):PAnsiChar;
function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;

function mir_hashstr (const key:PAnsiChar):uint; {inline;}
function mir_hashstrW(const key:PWideChar):uint; {inline;}
function lrtrim (str:PAnsiChar):PAnsiChar; {inline}
function lrtrimp(str:PAnsiChar):PAnsiChar; {inline}

function Translate (sz: PAnsiChar): PAnsiChar;
function Translatew(sz: PWideChar): PWideChar;
function TranslateString    (const sz:AnsiString):AnsiString;
function TranslateWideString(const sz:WideString):WideString;
function TranslateUnicodeString(sz: UnicodeString): UnicodeString;
procedure TranslateDialogDefault(hwndDlg: THANDLE);
procedure TranslateMenu(hMenu: HMENU);
function Langpack_Register:Pointer;

function CreateProtoServiceFunction(const szModule, szService: PAnsiChar; serviceProc: TMIRANDASERVICE): int_ptr;

function Menu_AddContextFrameMenuItem(mi:PMO_MenuItem):HGENMENU;
function Menu_AddMainMenuItem        (mi:PMO_MenuItem):HGENMENU;
function Menu_AddContactMenuItem     (mi:PMO_MenuItem):HGENMENU;
function Menu_AddStatusMenuItem      (mi:PMO_MenuItem):HGENMENU;
function Menu_AddProtoMenuItem       (mi:PMO_MenuItem):HGENMENU;
function Menu_AddTrayMenuItem        (mi:PMO_MenuItem):HGENMENU;

procedure Srmm_AddButton(bbdi:pBBButton);

procedure SET_UID(pmi:PMO_MenuItem; strguid:PAnsiChar);

function Options_AddPage (wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
function Hotkey_Register(hk:PHOTKEYDESC):int_ptr;

function Skin_AddIcon(si:PSKINICONDESC):THANDLE;
procedure Icon_Register (hInst:HINST; const szSection:PAnsiChar; pIcons:pIconItem ; iCount:size_t; prefix:PAnsiChar);
procedure Icon_RegisterW(hInst:HINST; const szSection:PWideChar; pIcons:pIconItemW; iCount:size_t; prefix:PAnsiChar);

procedure FontRegister(pFontID:pFontID);
procedure ColourRegister(pColorID:pColourID);

function TopToolbar_AddButton(pButton:PTTBButton):THANDLE;

function Options_Open(group,page,tab:PWideChar):int_ptr;

{$ELSE}

function _Srmm_AddButton(bbdi:pBBButton; pPlugin:Pointer) : integer; stdcall; 
             external AppDll name 'Srmm_AddButton';

function _AddMainMenuItem(mi:PMO_MenuItem; pszProto:PAnsiChar) : HGENMENU; stdcall;
             external AppDll name 'Menu_AddMainMenuItem';

function _AddContactMenuItem(mi:PMO_MenuItem; pszProto:PAnsiChar) : HGENMENU; stdcall;
             external AppDll name 'Menu_AddContactMenuItem';

function _AddProtoMenuItem(mi:PMO_MenuItem; pszProto:PAnsiChar) : HGENMENU; stdcall;
             external AppDll name 'Menu_AddProtoMenuItem';

function _AddStatusMenuItem(mi:PMO_MenuItem; pszProto:PAnsiChar) : HGENMENU; stdcall;
             external AppDll name 'Menu_AddStatusMenuItem';

function _AddTrayMenuItem(mi:PMO_MenuItem) : HGENMENU; stdcall;
             external AppDll name 'Menu_AddTrayMenuItem';

//----- CListint -----

function IsHContactGroup(h:TMCONTACT):bool;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=(h xor HCONTACT_ISGROUP)<(HCONTACT_ISGROUP xor HCONTACT_ISINFO);
end;
function IsHContactInfo(h:TMCONTACT):bool;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=(h and HCONTACT_ISINFO)=HCONTACT_ISINFO;
end;
function IsHContactContact(h:TMCONTACT):bool;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
 result:=(h and HCONTACT_ISGROUP)=0;
end;

function CLCDEFAULT_BKCOLOUR:dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=GetSysColor(COLOR_3DFACE);
end;
function CLCDEFAULT_TEXTCOLOUR:dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=GetSysColor(COLOR_WINDOWTEXT);
end;
function CLCDEFAULT_SELBKCOLOUR:dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=GetSysColor(COLOR_HIGHLIGHT);
end;
function CLCDEFAULT_SELTEXTCOLOUR:dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=GetSysColor(COLOR_HIGHLIGHTTEXT);
end;
function CLCDEFAULT_HOTTEXTCOLOUR:dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=RGB(0,0,255);
end;


function CreateVersionString(version:dword;buf:PAnsiChar):PAnsiChar;
var
  vers:array [0..3] of integer;
begin
  vers[0]:=(version shr 24) and $FF;
  vers[1]:=(version shr 16) and $FF;
  vers[2]:=(version shr  8) and $FF;
  vers[3]:=version and $FF;
  wvsprintfa(buf,'%d.%d.%d.%d',@vers);
  result:=buf;
end;

function CreateVersionStringPlugin(pluginInfo:PPluginInfoEx;buf:PAnsiChar):PAnsiChar;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=CreateVersionString(pluginInfo^.version,buf);
end;

function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  Result := (a shl 24) or (b shl 16) or (c shl 8) or d;
end;

function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;
begin
  Result := 0;
  { could be used to compare for severity of age for positive values, if a<b
  results are minus values, 0 for equal, positive if a is newer }
  Inc(Result, (verA and $FF      ) - (verB and $FF));
  Inc(Result, (verA and $FF00    ) - (verB and $FF00));
  Inc(Result, (verA and $FF0000  ) - (verB and $FF0000));
  Inc(Result, (verA and $FF000000) - (verB and $FF000000));
end;

function mir_hashstr(const key:PAnsiChar):uint; {inline;}
var
  len:int;
begin
  if key=NIL then
    result:=0
  else
  begin
    len:=lstrlena(key);
    result:=mir_hash(key, len);
  end;
end;

function mir_hashstrW(const key:PWideChar):uint; {inline;}
var
  len:int;
begin
  if key=NIL then
    result:=0
  else
  begin
    len:=lstrlenw(key);
    result:=mir_hash(key, len*SizeOF(WideChar));
  end;
end;

function lrtrim(str:PAnsiChar):PAnsiChar;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=ltrim(rtrim(str));
end;
function lrtrimp(str:PAnsiChar):PAnsiChar;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=ltrimp(rtrim(str));
end;

function TranslateW(sz: PWideChar): PWideChar;
begin
  result:=TranslateW_LP(sz, @g_plugin);
end;

function Translate(sz: PAnsiChar): PAnsiChar;
begin
  result:=TranslateA_LP(sz, @g_plugin);
end;

function TranslateString(const sz:AnsiString):AnsiString;
begin
  Result:=AnsiString(TranslateA_LP(PAnsiChar(sz),@g_plugin));
end;

function TranslateWideString(const sz:WideString):WideString;
begin
  Result:=WideString(TranslateW_LP(PWideChar(sz),@g_plugin));
end;

function TranslateUnicodeString(sz: UnicodeString): UnicodeString;
begin
  Result:=UnicodeString(TranslateW_LP(PWideChar(sz),@g_plugin));
end;

procedure TranslateDialogDefault(hwndDlg: THANDLE);
begin
  TranslateDialog_LP(hwndDlg,@g_plugin);
end;

procedure TranslateMenu(hMenu: HMENU);
begin
  TranslateMenu_LP(hMenu,@g_plugin);
end;

function Langpack_Register:Pointer;
begin
  cli := Clist_GetInterface();
  result := @g_plugin;
end;


function CreateProtoServiceFunction(const szModule, szService: PAnsiChar; serviceProc: TMIRANDASERVICE): int_ptr;
var
  szStr: array [0..MAXMODULELABELLENGTH*2] of AnsiChar;
begin
  if (szModule=nil) or (szService=nil) then
  begin
    result:=0;
    exit;
  end;
  lstrcpya(@szStr, szModule);
  lstrcata(@szStr, szService);
  Result := CreateServiceFunction(szStr, @serviceProc);
end;


function Menu_AddContextFrameMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=CallService(MS_CLIST_ADDCONTEXTFRAMEMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddMainMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=_AddMainMenuItem(mi, nil);
end;

function Menu_AddContactMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=_AddContactMenuItem(mi, nil);
end;

function Menu_AddStatusMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=_AddStatusMenuItem(mi, nil);
end;

function Menu_AddProtoMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=_AddProtoMenuItem(mi, nil);
end;

function Menu_AddTrayMenuItem(mi:PMO_MenuItem):HGENMENU;
begin
  mi^.pPlugin:=@g_plugin;
  result:=_AddTrayMenuItem(mi);
end;

function Hotkey_Register(hk:PHOTKEYDESC):int_ptr;
begin
	result:=_Hotkey_Register(hk, @g_plugin);
end;

function Skin_AddIcon(si:PSKINICONDESC):THANDLE;
begin
  result:=IcoLib_AddIcon(si, @g_plugin);
end;

procedure Icon_Register(hInst:HINST; const szSection:PAnsiChar; pIcons:pIconItem; iCount:size_t; prefix:PAnsiChar);
begin
	mir_Icon_Register(hInst, szSection, pIcons, iCount, prefix, @g_plugin);
end;

procedure Icon_RegisterW(hInst:HINST; const szSection:PWideChar; pIcons:pIconItemW; iCount:size_t; prefix:PAnsiChar);
begin
	mir_Icon_RegisterW(hInst, szSection, pIcons, iCount, prefix, @g_plugin);
end;

procedure FontRegister(pFontID:pFontID);
begin
   Font_Register(pFontID, @g_plugin);
end;

procedure ColourRegister(pColorID:pColourID);
begin
	Colour_Register(pColorID, @g_plugin);
end;

function TopToolbar_AddButton(pButton:PTTBButton):THANDLE;
begin
  result:=CallService(MS_TTB_ADDBUTTON, WPARAM(pButton), LPARAM(@g_plugin));
end;

function Options_AddPage(wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
begin
  result:=_Options_AddPage(wParam, odp, @g_plugin);
end;

function Options_Open(group,page,tab:PWideChar):int_ptr;
begin
	result:=_Options_Open(group,page,tab,@g_plugin);
end;

procedure Srmm_AddButton(bbdi:pBBButton);
begin
   _Srmm_AddButton(bbdi,@g_plugin);
end;

function UuidFromStringA(StringUuid:PAnsiChar; var Uuid:TGUID) : integer; stdcall; external 'Rpcrt4.dll';

procedure SET_UID(pmi:PMO_MenuItem; strguid:PAnsiChar);
begin
   UuidFromStringA(strguid, pmi.uid);
end;

{$ENDIF}
