{$DEFINE UseCore}
{$INCLUDE compilers.inc}
unit dbsettings;
interface

uses windows,m_api;

function DBReadByte (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:byte =0):byte;
function DBReadWord (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:word =0):word;
function DBReadDWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:dword=0):dword;

function DBReadSetting   (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
function DBReadSettingStr(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;

function DBReadStringLength(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
function DBReadString (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         default:PAnsiChar=nil;enc:integer=DBVT_ASCIIZ):PAnsiChar;
function DBReadUTF8   (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:PAnsiChar=nil):PAnsiChar;
function DBReadUnicode(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:PWideChar=nil):PWideChar;

function DBReadStruct (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):Integer;
function DBWriteStruct(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):Integer;

function DBWriteSetting(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
function DBWriteByte (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:Byte ):int_ptr;
function DBWriteWord (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:Word ):int_ptr;
function DBWriteDWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:dword):int_ptr;

function DBWriteString (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         val:PAnsiChar;enc:integer=DBVT_ASCIIZ):int_ptr;
function DBWriteUTF8   (hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:PAnsiChar):int_ptr;
function DBWriteUnicode(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:PWideChar):int_ptr;

function DBFreeVariant(dbv:PDBVARIANT):int_ptr;
function DBDeleteSetting(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar):int_ptr;

function DBDeleteGroup(hContact:THANDLE;szModule:PAnsiChar;prefix:pAnsiChar=nil):int_ptr;

function DBDeleteModule(szModule:PAnsiChar):integer; // 0.8.0+

function DBGetSettingType(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar):integer;

implementation

uses common;

function DBReadByte(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:byte=0):byte;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_b(hContact, szModule, szSetting, default);
end;
{$ELSE}
var
  dbv:TDBVARIANT;
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  If CallService(MS_DB_CONTACT_GETSETTING,hContact,lParam(@cgs))<>0 then
    Result:=default
  else
    Result:=dbv.bVal;
end;
{$ENDIF}

function DBReadWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:word=0):word;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_w(hContact, szModule, szSetting, default);
end;
{$ELSE}
var
  dbv:TDBVARIANT;
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  If CallService(MS_DB_CONTACT_GETSETTING,hContact,lParam(@cgs))<>0 then
    Result:=default
  else
    Result:=dbv.wVal;
end;
{$ENDIF}

function DBReadDWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:dword=0):dword;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_dw(hContact, szModule, szSetting, default);
end;
{$ELSE}
var
  dbv:TDBVARIANT;
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  If CallService(MS_DB_CONTACT_GETSETTING,hContact,lParam(@cgs))<>0 then
    Result:=default
  else
    Result:=dbv.dVal;
end;
{$ENDIF}

function DBReadSetting(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get(hContact, szModule, szSetting, dbv);
end;
{$ELSE}
var
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=dbv;
  Result:=CallService(MS_DB_CONTACT_GETSETTING,hContact,lParam(@cgs));
end;
{$ENDIF}

function DBReadSettingStr(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
var
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=dbv;
  Result:=CallService(MS_DB_CONTACT_GETSETTING_STR,hContact,lParam(@cgs));
end;

function DBReadStringLength(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
var
  cgs:TDBCONTACTGETSETTING;
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  i:=CallService(MS_DB_CONTACT_GETSETTING_STR,hContact,lParam(@cgs));
  if (i<>0) or (dbv.szVal.a=nil) or (dbv.szVal.a^=#0) then
    result:=0
  else
    result:=lstrlena(dbv.szVal.a);
//!!  if i=0 then
    DBFreeVariant(@dbv);
end;

function DBReadString(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         default:PAnsiChar=nil;enc:integer=DBVT_ASCIIZ):PAnsiChar;
var
  cgs:TDBCONTACTGETSETTING;
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  dbv._type    :=enc;
  i:=CallService(MS_DB_CONTACT_GETSETTING_STR,hContact,lParam(@cgs));
  if i=0 then
    default:=dbv.szVal.a;

  if (default=nil) or (default^=#0) then
    result:=nil
  else
    StrDup(result,default);

//!!  if i=0 then
    DBFreeVariant(@dbv);
end;

function DBReadUTF8(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:PAnsiChar=nil):PAnsiChar;
begin
  result:=DBReadString(hContact,szModule,szSetting,default,DBVT_UTF8);
end;

function DBReadUnicode(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;default:PWideChar=nil):PWideChar;
var
  cgs:TDBCONTACTGETSETTING;
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  dbv._type    :=DBVT_WCHAR;
  i:=CallService(MS_DB_CONTACT_GETSETTING_STR,hContact,lParam(@cgs));
  if i=0 then
    default:=dbv.szVal.w;

  if (default=nil) or (default^=#0) then
    result:=nil
  else
    StrDupW(result,default);

//!!  if i=0 then
    DBFreeVariant(@dbv);
end;

function DBReadStruct(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):Integer;
var
  dbv:TDBVariant;
begin
  FillChar(dbv,SizeOf(dbv),0);
  dbv._type:=DBVT_BLOB;
  dbv.pbVal:=nil;
  if (DBReadSetting(0,szModule,szSetting,@dbv)=0) and
     (dbv.pbVal<>nil) and (dbv.cpbVal=size) then
  begin
    move(dbv.pbVal^,ptr^,size);
    DBFreeVariant(@dbv);
    result:=1;
  end
  else
    result:=0;
end;

function DBWriteStruct(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):Integer;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_blob(hContact, szModule, szSetting, ptr, size);
end;
{$ELSE}
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule    :=szModule;
  cws.szSetting   :=szSetting;
  cws.value._type :=DBVT_BLOB;
  cws.value.pbVal :=ptr;
  cws.value.cpbVal:=size;
  result:=CallService(MS_DB_CONTACT_WRITESETTING,0,lParam(@cws));
end;
{$ENDIF}

function DBWriteSetting(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
var
  cws: TDBCONTACTWRITESETTING;
begin
  cws.szModule  :=szModule;
  cws.szSetting :=szSetting;
  move(dbv^,cws.value,SizeOf(TDBVARIANT));
  Result := CallService(MS_DB_CONTACT_WRITESETTING, hContact, lParam(@cws));
end;

function DBWriteByte(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:Byte):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_b(hContact, szModule, szSetting, val);
end;
{$ELSE}
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule   :=szModule;
  cws.szSetting  :=szSetting;
  cws.value._type:=DBVT_BYTE;
  cws.value.bVal :=val;
  Result:=CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;
{$ENDIF}

function DBWriteWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:Word):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_w(hContact, szModule, szSetting, val);
end;
{$ELSE}
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule   :=szModule;
  cws.szSetting  :=szSetting;
  cws.value._type:=DBVT_WORD;
  cws.value.wVal :=val;
  Result:=CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;
{$ENDIF}

function DBWriteDWord(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:dword):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_dw(hContact, szModule, szSetting, val);
end;
{$ELSE}
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule   :=szModule;
  cws.szSetting  :=szSetting;
  cws.value._type:=DBVT_DWORD;
  cws.value.dVal :=val;
  Result:=CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;
{$ENDIF}

function DBWriteString(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;
         val:PAnsiChar;enc:integer=DBVT_ASCIIZ):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_s(hContact, szModule, szSetting, val);
end;
{$ELSE}
var
  cws:TDBCONTACTWRITESETTING;
  p:dword;
begin
  cws.szModule     :=szModule;
  cws.szSetting    :=szSetting;
  cws.value._type  :=enc;
  if val=nil then
  begin
    p:=0;
    val:=@p;
  end;
  cws.value.szVal.a:=val;
  Result:=CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;
{$ENDIF}

function DBWriteUTF8(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:PAnsiChar):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_utf(hContact, szModule, szSetting, val);
end;
{$ELSE}
begin
  result:=DBWriteString(hContact,szModule,szSetting,val,DBVT_UTF8);
end;
{$ENDIF}

function DBWriteUnicode(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar;val:PWideChar):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_ws(hContact, szModule, szSetting, val);
end;
{$ELSE}
begin
  result:=DBWriteString(hContact,szModule,szSetting,PAnsiChar(val),DBVT_WCHAR);
{
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule     :=szModule;
  cws.szSetting    :=szSetting;
  cws.value._type  :=DBVT_WCHAR;
  cws.value.szVal.w:=Val;
  Result:=CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
}
end;
{$ENDIF}

function DBFreeVariant(dbv:PDBVARIANT):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_free(dbv);
end;
{$ELSE}
begin
  Result:=CallService(MS_DB_CONTACT_FREEVARIANT,0,lParam(dbv));
end;
{$ENDIF}

function DBDeleteSetting(hContact:THandle;szModule:PAnsiChar;szSetting:PAnsiChar):int_ptr;
{$IFDEF UseCore}
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_unset(hContact, szModule, szSetting);
end;
{$ELSE}
var
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  Result:=CallService(MS_DB_CONTACT_DELETESETTING,hContact,lParam(@cgs));
end;
{$ENDIF}

type
  ppchar = ^pAnsiChar;

function EnumSettingsProc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
begin
  lstrcpya(ppchar(lParam)^,szSetting);
  while ppchar(lParam)^^<>#0 do inc(ppchar(lParam)^);
  inc(ppchar(lParam)^);
  result:=0;
end;
function EnumSettingsProcCalc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
begin
  inc(pint_ptr(lParam)^,lstrlena(szSetting)+1);
  result:=0;
end;

function DBDeleteGroup(hContact:THANDLE;szModule:PAnsiChar;prefix:pAnsiChar=nil):int_ptr;
var
  ces:TDBCONTACTENUMSETTINGS;
  cgs:TDBCONTACTGETSETTING;
  p:PAnsiChar;
  num,len:integer;
  ptr:pAnsiChar;
begin
  ces.szModule:=szModule;
  num:=0;

  ces.pfnEnumProc:=@EnumSettingsProcCalc;
  ces.lParam     :=lParam(@num);
  ces.ofsSettings:=0;
  CallService(MS_DB_CONTACT_ENUMSETTINGS,hContact,lparam(@ces));

  GetMem(p,num+1);
  ptr:=p;
  ces.pfnEnumProc:=@EnumSettingsProc;
  ces.lParam     :=lparam(@ptr);
  ces.ofsSettings:=0;
  result:=CallService(MS_DB_CONTACT_ENUMSETTINGS,hContact,lparam(@ces));
  ptr^:=#0;

  cgs.szModule:=szModule;
  ptr:=p;
  if (prefix<>nil) and (prefix^<>#0) then
    len:=StrLen(prefix)
  else
    len:=0;
  while ptr^<>#0 do
  begin
    if (len=0) or (StrCmp(prefix,ptr,len)=0) then
    begin
      cgs.szSetting:=ptr;
      CallService(MS_DB_CONTACT_DELETESETTING,hContact,lParam(@cgs));
    end;
    while ptr^<>#0 do inc(ptr);
    inc(ptr);
  end;
  FreeMem(p);
end;

function DBDeleteModule(szModule:PAnsiChar):integer; // 0.8.0+
begin
  result:=0;
  CallService(MS_DB_MODULE_DELETE,0,lParam(szModule));
end;

function DBGetSettingType(hContact:THANDLE;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
var
  ldbv:TDBVARIANT;
begin
  if DBReadSetting(hContact,szModule,szSetting,@ldbv)=0 then
  begin
    result:=ldbv._type;
    DBFreeVariant(@ldbv);
  end
  else
    result:=DBVT_DELETED;
end;

begin
end.
