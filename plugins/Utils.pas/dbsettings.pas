{$INCLUDE compilers.inc}
unit dbsettings;
interface

uses windows,m_api;

function DBReadByte (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:byte =0):byte;
function DBReadWord (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:word =0):word;
function DBReadDWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:dword=0):dword;

function DBReadSetting   (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
function DBReadSettingStr(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;

function DBReadStringLength(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
function DBReadString (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         default:PAnsiChar=nil;enc:integer=DBVT_ASCIIZ):PAnsiChar;
function DBReadUTF8   (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:PAnsiChar=nil):PAnsiChar;
function DBReadUnicode(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:PWideChar=nil):PWideChar;

function DBReadStruct (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):uint_ptr;
function DBWriteStruct(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):integer;

function DBWriteSetting(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
function DBWriteByte (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:byte ):int_ptr;
function DBWriteWord (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:word ):int_ptr;
function DBWriteDWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:dword):int_ptr;

function DBWriteString (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         val:PAnsiChar;enc:integer=DBVT_ASCIIZ):int_ptr;
function DBWriteUTF8   (hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:PAnsiChar):int_ptr;
function DBWriteUnicode(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:PWideChar):int_ptr;

//function DBFreeVariant(dbv:PDBVARIANT):int_ptr;
function DBDeleteSetting(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):int_ptr;

function DBDeleteGroup(hContact:TMCONTACT;szModule:PAnsiChar;prefix:pAnsiChar=nil):int_ptr;

function DBDeleteModule(hContact:TMCONTACT;szModule:PAnsiChar):integer;

function DBGetSettingType(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):integer;

implementation

uses common;

function DBReadByte(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:byte=0):byte;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_b(hContact, szModule, szSetting, default);
end;

function DBReadWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:word=0):word;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_w(hContact, szModule, szSetting, default);
end;

function DBReadDWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:dword=0):dword;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_dw(hContact, szModule, szSetting, default);
end;

function DBReadSetting(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get(hContact, szModule, szSetting, dbv);
end;

function DBReadSettingStr(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_get_s(hContact, szModule, szSetting, dbv, DBVT_ASCIIZ);
end;

function DBReadStringLength(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
var
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  i:=db_get_s(hContact,szModule,szSetting,@dbv,DBVT_ASCIIZ);
  if (i<>0) or (dbv.szVal.a=nil) or (dbv.szVal.a^=#0) then
    result:=0
  else
    result:=StrLen(dbv.szVal.a);

  DBFreeVariant(@dbv);
end;

function DBReadString(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         default:PAnsiChar=nil;enc:integer=DBVT_ASCIIZ):PAnsiChar;
var
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  i:=db_get_s(hContact,szModule,szSetting,@dbv,enc);
  if i=0 then
    default:=dbv.szVal.a;

  if (default=nil) or (default^=#0) then
    result:=nil
  else
    StrDup(result,default);

  DBFreeVariant(@dbv);
end;

function DBReadUTF8(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:PAnsiChar=nil):PAnsiChar;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=DBReadString(hContact,szModule,szSetting,default,DBVT_UTF8);
end;

function DBReadUnicode(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;default:PWideChar=nil):PWideChar;
var
  dbv:TDBVARIANT;
  i:int_ptr;
begin
  FillChar(dbv,SizeOf(dbv),0);
  i:=db_get_s(hContact,szModule,szSetting,@dbv,DBVT_WCHAR);
  if i=0 then
    default:=dbv.szVal.w;

  if (default=nil) or (default^=#0) then
    result:=nil
  else
    StrDupW(result,default);

  DBFreeVariant(@dbv);
end;

function DBReadStruct(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):uint_ptr;
var
  dbv:TDBVariant;
begin
  FillChar(dbv,SizeOf(dbv),0);
  dbv._type:=DBVT_BLOB;
  dbv.pbVal:=nil;
  if (DBReadSetting(0,szModule,szSetting,@dbv)=0) and
     (dbv.pbVal<>nil) and (dbv.cpbVal=size) then
  begin
    if ptr=nil then
      mGetMem(ptr,size);
    move(dbv.pbVal^,ptr^,size);
    DBFreeVariant(@dbv);
    result:=uint_ptr(ptr)
  end
  else
    result:=0;
end;

function DBWriteStruct(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         ptr:pointer;size:dword):integer;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_blob(hContact, szModule, szSetting, ptr, size);
end;

function DBWriteSetting(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;dbv:PDBVARIANT):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  Result:=db_set(hContact, szModule, szSetting, dbv);
end;

function DBWriteByte(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:byte):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_b(hContact, szModule, szSetting, val);
end;

function DBWriteWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:word):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_w(hContact, szModule, szSetting, val);
end;

function DBWriteDWord(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:dword):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_dw(hContact, szModule, szSetting, val);
end;

function DBWriteString(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;
         val:PAnsiChar;enc:integer=DBVT_ASCIIZ):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_s(hContact, szModule, szSetting, val);
end;

function DBWriteUTF8(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:PAnsiChar):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_utf(hContact, szModule, szSetting, val);
end;

function DBWriteUnicode(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar;val:PWideChar):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_set_ws(hContact, szModule, szSetting, val);
end;

function DBDeleteSetting(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):int_ptr;
  {$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=db_unset(hContact, szModule, szSetting);
end;

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

function DBDeleteGroup(hContact:TMCONTACT;szModule:PAnsiChar;prefix:pAnsiChar=nil):int_ptr;
var
  ces:TDBCONTACTENUMSETTINGS;
  p:PAnsiChar;
  code,num:integer;
  ptr:pAnsiChar;
  res:boolean;
  len:cardinal;
  mask:array [0..31] of AnsiChar;
begin
  if (prefix=nil) or (prefix^=#0) then
  begin
    DBDeleteModule(hContact,szModule);
    result:=0;
    exit;
  end;

  ces.szModule:=szModule;
  num:=0;
  //calculate size for setting names buffer
  ces.pfnEnumProc:=@EnumSettingsProcCalc;
  ces.lParam     :=lParam(@num);
  ces.ofsSettings:=0;
  CallService(MS_DB_CONTACT_ENUMSETTINGS,hContact,lparam(@ces));

  //get setting names list
  GetMem(p,num+1);
  ptr:=p;
  ces.pfnEnumProc:=@EnumSettingsProc;
  ces.lParam     :=lparam(@ptr);
  ces.ofsSettings:=0;
  result:=CallService(MS_DB_CONTACT_ENUMSETTINGS,hContact,lparam(@ces));
  ptr^:=#0;

  ptr:=p;
  code:=0;
  if (prefix<>nil) and (prefix^<>#0) then
  begin
    len:=StrLen(prefix);

    if prefix[len-1]='*' then // bla*
    begin
      code:=1;
      dec(len);
    end;
    if prefix^='*' then // *bla
    begin
      code:=code or 2;
      dec(len);
      inc(prefix);
    end;
  end
  else
    len:=0;
  StrCopy(mask,prefix,len);

  while ptr^<>#0 do
  begin
    if len<>0 then
    begin
      res:=false;
      case code of
        // postfix (right side)
        2: begin
          num:=StrLen(ptr)-len;
          if num>=0 then
            res:=StrCmp(mask,ptr+num,len)=0;
        end;
        // content (left, middle or right, no matter)
        3: begin
          res:=StrPos(ptr,mask)<>nil;
        end;
      else // 0 or 1, prefix (left side)
        res:=StrCmp(mask,ptr,len)=0;
      end;
    end
    else
      res:=true;

    if res then
    begin
      DBDeleteSetting(hContact,szModule,ptr);
    end;
    while ptr^<>#0 do inc(ptr);
    inc(ptr);
  end;
  FreeMem(p);
end;

function DBDeleteModule(hContact:TMCONTACT;szModule:PAnsiChar):integer;
begin
  result:=0;
  CallService(MS_DB_MODULE_DELETE,hContact,lParam(szModule));
end;

function DBGetSettingType(hContact:TMCONTACT;szModule:PAnsiChar;szSetting:PAnsiChar):integer;
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
