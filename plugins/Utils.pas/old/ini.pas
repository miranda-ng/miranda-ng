unit INI;

interface

uses windows;

{+}function SetStorage(name:PAnsiChar;inINI:boolean):cardinal;
{+}procedure FreeStorage(aHandle:cardinal);

{+}procedure SetDefaultSection(aHandle:cardinal;name:PAnsiChar);
{+}procedure SetCurrentSection(aHandle:cardinal;sect:PAnsiChar);

{+}procedure FlushSettings(aHandle:cardinal);
{+}procedure FlushSection(aHandle:cardinal);

{+}procedure WriteNCInt(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;value:integer);
{+}procedure WriteNCStr(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;value:PAnsiChar);

{+}procedure WriteNCStruct(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;ptr:pointer;size:integer);
{*}procedure WriteStruct(aHandle:cardinal;param:PAnsiChar;ptr:pointer;size:integer);
{+}function  ReadStruct (aHandle:cardinal;param:PAnsiChar;ptr:pointer;size:integer):boolean;

{+}procedure WriteFlag(aHandle:cardinal;param:PAnsiChar;value:integer);
{+}procedure WriteInt (aHandle:cardinal;param:PAnsiChar;value:integer);
{+}procedure WriteStr (aHandle:cardinal;param:PAnsiChar;value:PWideChar);
procedure WriteAnsiStr(aHandle:cardinal;param:PAnsiChar;value:PAnsiChar);
{+}function  ReadFlag(aHandle:cardinal;param:PAnsiChar; default:integer):integer;
{+}function  ReadInt (aHandle:cardinal;param:PAnsiChar; default:integer):integer;
procedure ReadStr (aHandle:cardinal;var dst:PWideChar;param:PAnsiChar;default:PWideChar);
procedure ReadAnsiStr(aHandle:cardinal;var dst:PAnsiChar;param:PAnsiChar;default:PAnsiChar);

procedure WriteSect(aHandle:cardinal;src:PAnsiChar);
procedure ReadSect (aHandle:cardinal;var dst:PAnsiChar);

{*}procedure ClearSection(aHandle:cardinal);
{+}procedure DeleteParam(aHandle:cardinal;param:PAnsiChar);

implementation

uses common,io,m_api,dbsettings;

type
  PStorage = ^TStorage;
  TStorage = record
    SName     :PAnsiChar;
    SType     :bool;
    SHandle   :THANDLE;
    DefSection:PAnsiChar;
    Section   :Array [0..127] of AnsiChar;
    ParOffset :integer;
    Buffer    :PAnsiChar;
    INIBuffer :PAnsiChar;
  end;
  PStHeap = ^TStHeap;
  TStHeap = array [0..10] of TStorage;

const
  Storage:PStHeap=nil;
  NumStorage:cardinal=0;

type
  pbrec=^brec;
  brec=record
    ptr:PAnsiChar;
    handle:cardinal;
  end;

const
  DefDefSection:PAnsiChar = 'default';

{+}function SetStorage(name:PAnsiChar;inINI:boolean):cardinal;
var
  i:integer;
  tmp:PStHeap;
begin
  if Storage=nil then
  begin
    mGetMem(Storage,SizeOf(TStorage));
    FillChar(Storage^,SizeOf(TStorage),0);
    NumStorage:=1;
    result:=0;
  end
  else
  begin
    integer(result):=-1;
    for i:=0 to NumStorage-1 do
    begin
      if Storage^[i].SName=nil then // free cell
      begin
        result:=i;
        break;
      end;
    end;
    if integer(result)<0 then
    begin
      mGetMem(tmp,SizeOf(TStorage)*(NumStorage+1));
      move(Storage^,tmp^,SizeOf(TStorage)*NumStorage);
      mFreeMem(Storage);
      Storage:=tmp;
      FillChar(Storage^[NumStorage],SizeOf(TStorage),0);
      result:=NumStorage;
      inc(NumStorage);
    end
  end;
  with Storage^[result] do
  begin
    StrDup(SName,name);
    SType:=inINI;
  end;
end;

{+}procedure FreeStorage(aHandle:cardinal);
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
  begin
    mFreeMem(SName);
    mFreeMem(DefSection);
    mFreeMem(Buffer);
    mFreeMem(INIBuffer);
  end;
end;

{+}procedure WriteNCStruct(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;ptr:pointer;size:integer);
var
  cws:TDBCONTACTWRITESETTING;
  pn:array [0..127] of AnsiChar;
  i:integer;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      if (sect=nil) or (sect^=#0) then
        sect:=DefSection;
      if sect=nil then
        sect:=DefDefSection;
      WritePrivateProfileStructA(sect,param,ptr,size,SName);
    end
    else
    begin
      if (sect<>nil) and (sect^<>#0) then
      begin
        i:=StrLen(sect);
        move(sect^,pn,i);
        pn[i]:='/';
        inc(i);
      end
      else
        i:=0;
      StrCopy(pn+i,param);
      cws.szModule    :=SName;
      cws.szSetting   :=pn;
      cws.value._type :=DBVT_BLOB;
      cws.value.pbVal :=ptr;
      cws.value.cpbVal:=size;
      PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING,0,lParam(@cws));
    end
end;

{*}procedure WriteStruct(aHandle:cardinal;param:PAnsiChar;ptr:pointer;size:integer);
const
  hex:array [0..15] of AnsiChar = '0123456789ABCDEF';
var
  lptr:PAnsiChar;
  buf,buf1:PAnsiChar;
  i:integer;
  crc:integer;
  cws:TDBCONTACTWRITESETTING;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      mGetMem(buf,(size+1)*2);
      crc:=0;
      buf1:=buf;
      for i:=0 to size-1 do
      begin
        inc(crc,PByte(ptr)^);
        buf1^    :=hex[pbyte(ptr)^ shr 4];
        (buf1+1)^:=hex[pbyte(ptr)^ and $0F];
        inc(buf1,2);
        inc(pbyte(ptr));
      end;
      buf1^    :=hex[(crc and $FF) shr 4];
      (buf1+1)^:=hex[(crc and $0F)];

      StrCat(Buffer,param);
      lptr:=StrEnd(Buffer);
      lptr^:='=';
      inc(lptr);
      move(buf^,lptr^,(size+1)*2);
      mFreeMem(buf);
      inc(lptr,(size+1)*2);
      lptr^    :=#13;
      (lptr+1)^:=#10;
      (lptr+2)^:=#0;
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      cws.szModule    :=SName;
      cws.szSetting   :=Section;
      cws.value._type :=DBVT_BLOB;
      cws.value.pbVal :=ptr;
      cws.value.cpbVal:=size;
      PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING,0,lParam(@cws));
    end
end;

{+}function ReadStruct(aHandle:cardinal;param:PAnsiChar;ptr:pointer;size:integer):boolean;
var
  dbv:TDBVariant;
begin
  if aHandle>=NumStorage then
  begin
    result:=false;
    exit;
  end;
  with Storage^[aHandle] do
    if SType then
    begin
      result:=GetPrivateProfileStructA(Section,param,ptr,size,SName);
    end
    else
    begin
      dbv._type:=DBVT_BLOB;
      dbv.pbVal:=nil;
      StrCopy(Section+ParOffset,param);
      if (DBReadSetting(0,SName,Section,@dbv)=0) and
         (dbv.pbVal<>nil) and (dbv.cpbVal=size) then
      begin
        move(dbv.pbVal^,ptr^,size);
        DBFreeVariant(@dbv);
        result:=true;
      end
      else
        result:=false;
    end
end;

{+}procedure WriteNCInt(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;value:integer);
var
  pn:array [0..127] of AnsiChar;
  i:integer;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if Stype then
    begin
      if (sect=nil) or (sect^=#0) then
        sect:=DefSection;
      if sect=nil then
        sect:=DefDefSection;
      WritePrivateProfileStringA(sect,param,IntToStr(pn,value),SName);
    end
    else
    begin
      if (sect<>nil) and (sect^<>#0) then
      begin
        i:=StrLen(sect);
        move(sect^,pn,i);
        pn[i]:='/';
        inc(i);
      end
      else
        i:=0;
      StrCopy(pn+i,param);
      DBWriteDWord(0,SName,pn,value)
    end
end;

{+}procedure WriteNCStr(aHandle:cardinal;sect:PAnsiChar;param:PAnsiChar;value:PAnsiChar);
var
  pn:array [0..127] of AnsiChar;
  i:integer;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      if (sect=nil) or (sect^=#0) then
        sect:=DefSection;
      if sect=nil then
        sect:=DefDefSection;
      WritePrivateProfileStringA(sect,param,value,SName);
    end
    else
    begin
      if (sect<>nil) and (sect^<>#0) then
      begin
        i:=StrLen(sect);
        move(sect^,pn,i);
        pn[i]:='/';
        inc(i);
      end
      else
        i:=0;
      StrCopy(pn+i,param);
      DBWriteString(0,SName,pn,value);
    end
end;

{+}procedure SetDefaultSection(aHandle:cardinal;name:PAnsiChar);
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
  begin
    mFreeMem(DefSection);
    StrDup(DefSection,name);
  end;
end;

{+}procedure SetCurrentSection(aHandle:cardinal;sect:PAnsiChar);
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
  begin
    if sect=nil then
      sect:=DefSection;
    if sect=nil then
      sect:='';
    if SType then
    begin
      if (sect=nil) or (sect^=#0) then
        sect:=DefDefSection;
      StrCopy(Section,sect);
      mGetMem(Buffer,16384);
      Buffer^    :=#13;
      (Buffer+1)^:=#10;
      (Buffer+2)^:=#0;
    end
    else
    begin
      if sect<>nil then
      begin
        StrCopy(Section,sect);
        ParOffset:=StrLen(Section);
        Section[ParOffset]:='/';
        inc(ParOffset);
      end
      else
        ParOffset:=0;
    end
  end;
end;

{+}procedure FlushSettings(aHandle:cardinal);
var
  size:integer;
  ptr:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
  begin
    if SType then
    begin
      if INIBuffer=nil then
        exit;
      ptr:=INIBuffer+1;
      size:=StrLen(ptr);
      seek(SHandle,0);
      BlockWrite(SHandle,ptr^,size);
      SetEndOfFile(SHandle);
      mFreeMem(INIBuffer);
      CloseHandle(SHandle);
    end;
  end;
end;

{+}procedure FlushSection(aHandle:cardinal);
var
  size,i:integer;
  sect:array [0..127] of AnsiChar;
  ptr1,ptr:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      if INIBuffer=nil then
      begin
        mGetMem(INIBuffer,32768);
        INIBuffer[0]:=#10;
        SHandle:=Reset(SName);
        if thandle(SHandle)=INVALID_HANDLE_VALUE then
          INIBuffer[1]:=#0
        else
        begin
          size:=FileSize(SHandle);
          INIBuffer[size+1]:=#0;
          BlockRead(SHandle,(INIBuffer+1)^,size);
          CloseHandle(SHandle);
        end;
        SHandle:=ReWrite(SName);
      end;
      // construct section name
      sect[0]:=#10;
      sect[1]:='[';
      size:=StrLen(Section);
      move(Section,sect[2],size);
      sect[size+2]:=']';
      sect[size+3]:=#0;
      // search section
      ptr:=StrPos(INIBuffer,sect);
      // delete section
      if ptr<>nil then
      begin
        ptr1:=ptr;
//!!        inc(ptr);
        while (ptr^<>#0) and ((ptr^<>#10) or ((ptr+1)^<>'[')) do inc(ptr);
        if ptr^<>#0 then
          StrCopy(ptr1,ptr+1)
        else
          ptr1^:=#0;
      end;
      // append section
      if (Buffer<>nil) and (StrLen(Buffer)>0) then
      begin
        i:=StrLen(INIBuffer);
        if INIBuffer[i-1]<>#10 then
        begin
          INIBuffer[i]  :=#13;
          INIBuffer[i+1]:=#10;
          inc(i,2);
        end;
        StrCopy(INIBuffer+i,sect+1);
        StrCat(INIBuffer,Buffer);
      end;
      mFreeMem(Buffer);
    end;
end;

{+}procedure WriteFlag(aHandle:cardinal;param:PAnsiChar;value:integer);
var
  ptr:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      StrCat(Buffer,param);
      ptr:=StrEnd(Buffer);
      ptr^:='=';
      (ptr+1)^:=CHR((value and 1)+ORD('0'));
      inc(ptr,2);
      ptr^    :=#13;
      (ptr+1)^:=#10;
      (ptr+2)^:=#0;
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      DBWriteByte(0,SName,Section,value)
    end;
end;

{+}procedure WriteInt(aHandle:cardinal;param:PAnsiChar;value:integer);
var
  ptr:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      StrCat(Buffer,param);
      ptr:=StrEnd(Buffer);
      ptr^:='=';
      IntToStr(ptr+1,value);
      ptr:=StrEnd(Buffer);
      ptr^    :=#13;
      (ptr+1)^:=#10;
      (ptr+2)^:=#0;
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      DBWriteDWord(0,SName,Section,value)
    end;
end;

procedure WriteStrInt(aHandle:cardinal;param:PAnsiChar;value:pointer;wide:bool);
var
  buf:array [0..2047] of AnsiChar;
  ptr:PAnsiChar;
  lval:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
    begin
      StrCat(Buffer,param);
      ptr:=StrEnd(Buffer);
      ptr^:='=';
      inc(ptr);
      if (value<>nil) then
      begin
        buf[0]:=#0;
        if wide then
        begin
          if PWideChar(value)^<>#0 then
          begin
            WideToUTF8(value,lval);
            StrCopy(buf,lval,SizeOf(buf)-1);
            mFreeMem(lval);
          end
        end
        else if PAnsiChar(value)^<>#0 then
          StrCopy(buf,value,SizeOf(buf)-1);
        if buf[0]<>#0 then
        begin
          Escape(buf);
          StrCopy(ptr,buf);
          ptr:=StrEnd(Buffer);
        end;
      end;
      ptr^    :=#13;
      (ptr+1)^:=#10;
      (ptr+2)^:=#0;
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      if wide then
        DBWriteUnicode(0,SName,Section,value)
      else
        DBWriteString(0,SName,Section,value)
    end;
end;

{+}procedure WriteStr(aHandle:cardinal;param:PAnsiChar;value:PWideChar);
begin
  WriteStrInt(aHandle,param,value,true);
end;

{+}procedure WriteAnsiStr(aHandle:cardinal;param:PAnsiChar;value:PAnsiChar);
begin
  WriteStrInt(aHandle,param,value,false);
end;

{+}function ReadFlag(aHandle:cardinal; param:PAnsiChar; default:integer):integer;
begin
  if aHandle>=NumStorage then
  begin
    result:=default;
    exit;
  end;
  with Storage^[aHandle] do
    if SType then
    begin
      result:=GetPrivateProfileIntA(Section,param,default,SName)
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      result:=DBReadByte(0,SName,Section,default)
    end;
end;

{+}function ReadInt(aHandle:cardinal; param:PAnsiChar; default:integer):integer;
begin
  if aHandle>=NumStorage then
  begin
    result:=default;
    exit;
  end;
  with Storage^[aHandle] do
    if SType then
    begin
      result:=GetPrivateProfileIntA(Section,param,default,SName)
    end
    else
    begin
      StrCopy(Section+ParOffset,param);
      result:=DBReadDWord(0,SName,Section,default)
    end;
end;

procedure ReadStrInt(aHandle:cardinal;var dst;param:PAnsiChar;default:pointer;wide:bool);

  function EnumSettingsProc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
  var
    p:pbrec;
    i:integer;
  begin
    p:=pbrec(lparam);
    if StrCmp(Storage^[p^.handle].Section,szSetting,Storage^[p^.handle].ParOffset)=0 then
    begin
      i:=StrLen(szSetting)+1;
      move(szSetting^,p^.ptr^,i);
      inc(p^.ptr,i);
    end;
    result:=0;
  end;

var
  buf:array [0..4095] of AnsiChar;
  p:brec;
  ces:TDBCONTACTENUMSETTINGS;
  def:PAnsiChar;
  i:integer;
begin
  if aHandle>=NumStorage then
  begin
    if wide then
      StrDupW(pWideChar(dst),pWideChar(default))
    else
      StrDup(PAnsiChar(dst),PAnsiChar(default));
    exit;
  end;
  with Storage^[aHandle] do
    if SType then
    begin
      if wide then
      begin
        if default=nil then
          StrDup(def,'')
        else
          WideToUTF8(default,def);
      end
      else
      begin
        if default=nil then
          def:=''
        else
          def:=default;
      end;
      i:=GetPrivateProfileStringA(Section,param,def,buf,4095,SName)+1;
      mFreeMem(def);
      if param<>nil then
      begin
        if buf[0]<>#0 then
        begin
          Unescape(buf);
          if wide then
            UTF8ToWide(buf,pWideChar(dst))
          else
            StrDup(PAnsiChar(dst),buf);
        end
        else
          PAnsiChar(dst):=nil;
      end
      else //!! full section
      begin
        mGetMem(dst,i);
        move(buf,PAnsiChar(dst)^,i);
        buf[i-1]:=#0;
      end;
    end
    else
    begin
      if param<>nil then
      begin
        StrCopy(Section+ParOffset,param);
        if wide then
          pWideChar(dst):=DBReadUnicode(0,SName,Section,pWideChar(default))
        else
          PAnsiChar(dst):=DBReadString(0,SName,Section,PAnsiChar(default));
      end
      else
      begin
        p.ptr:=@buf;
        p.handle:=aHandle;
        FillChar(buf,SizeOf(buf),0);
        ces.pfnEnumProc:=@EnumSettingsProc;
        ces.lParam     :=lparam(@p);
        ces.szModule   :=SName;
        ces.ofsSettings:=0;
        PluginLink^.CallService(MS_DB_CONTACT_ENUMSETTINGS,0,lparam(@ces));
        mGetMem(dst,p.ptr-PAnsiChar(@buf)+1);
        move(buf,PAnsiChar(dst)^,p.ptr-PAnsiChar(@buf)+1);
      end;
    end;
end;

procedure ReadStr(aHandle:cardinal;var dst:PWideChar;param:PAnsiChar;default:PWideChar);
begin
  ReadStrInt(aHandle,dst,param,default,true);
end;

procedure ReadAnsiStr(aHandle:cardinal;var dst:PAnsiChar;param:PAnsiChar;default:PAnsiChar);
begin
  ReadStrInt(aHandle,dst,param,default,false);
end;

{*}procedure ClearSection(aHandle:cardinal);

  function EnumSettingsProc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
  var
    db:TDBCONTACTGETSETTING;
  begin
    with Storage^[lParam] do
    begin
      db.szModule:=SName;
      StrCopy(Section+ParOffset,szSetting);
      db.szSetting:=Section;
    end;
    PluginLink^.CallService(MS_DB_CONTACT_DELETESETTING,0,tlparam(@db));
    result:=0;
  end;

var
  ces:TDBCONTACTENUMSETTINGS;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
      WritePrivateProfileStringA(Section,nil,nil,SName)
    else
    begin
      ces.pfnEnumProc:=@EnumSettingsProc;
      ces.lParam     :=aHandle;
      ces.szModule   :=SName;
      ces.ofsSettings:=0;
      PluginLink^.CallService(MS_DB_CONTACT_ENUMSETTINGS,0,lparam(@ces));
    end;
end;

{*}procedure WriteSect(aHandle:cardinal;src:PAnsiChar);
var
  p:PAnsiChar;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
    if SType then
      WritePrivateProfileSectionA(Section,src,SName)
    else
    begin
      ClearSection(aHandle);
      while src^<>#0 do
      begin
        // write as strings
        p:=src;
        while src^<>'=' do inc(src);
        inc(src);
        DBWriteString(0,SName,p,src);
        while src^<>#0 do inc(src);
        inc(src);
      end;
    end;
end;

procedure ReadSect(aHandle:cardinal;var dst:PAnsiChar);

  function EnumSettingsProc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
  var
    dbv:TDBVariant;
    i:integer;
    p:pbrec;
    buf:array [0..31] of AnsiChar;
  begin
    result:=1;

    p:=pbrec(lparam);
    if (DBReadSetting(0,Storage^[p^.handle].Section,szSetting,@dbv)=0) then
    begin
      i:=StrLen(szSetting);
      move(szSetting^,p^.ptr^,i);
      inc(p^.ptr,i);
      p^.ptr^:='=';
      case dbv._type of
        DBVT_ASCIIZ: begin
          if dbv.szVal.a<>nil then
          begin
            i:=StrLen(dbv.szVal.a)+1;
            move(dbv.szVal.a^,(p^.ptr+1)^,i);
            DBFreeVariant(@dbv);
          end
        end;
        DBVT_BYTE,DBVT_WORD,DBVT_DWORD: begin
          case dbv._type of
            DBVT_BYTE : i:=dbv.bVal;
            DBVT_WORD : i:=dbv.wVal;
            DBVT_DWORD: i:=dbv.dVal;
          end;
          i:=StrLen(IntToStr(buf,i))+1;
          move(buf,(p^.ptr+1)^,i);
        end;
      else
        exit;
      end;
      inc(p^.ptr,i{+1});
    end;
  end;

var
  buf:array [0..16383] of AnsiChar;
  p:brec;
  ces:TDBCONTACTENUMSETTINGS;
  i:integer;
begin
  if aHandle>=NumStorage then
  begin
    dst:=nil;
    exit;
  end;
  with Storage^[aHandle] do
  begin
    if SType then
    begin
      i:=GetPrivateProfileSectionA(Section,buf,SizeOf(buf),SName)+1;
    end
    else
    begin
      p.ptr:=@buf;
      p.handle:=aHandle;
      FillChar(buf,SizeOf(buf),0);

      ces.pfnEnumProc:=@EnumSettingsProc;
      ces.lParam     :=lparam(@p);
      ces.szModule   :=SName;
      ces.ofsSettings:=0;
      PluginLink^.CallService(MS_DB_CONTACT_ENUMSETTINGS,0,lparam(@ces));
      i:=p.ptr-PAnsiChar(@buf)+1;
    end;
    mGetMem(dst,i);
    move(buf,dst^,i);
    buf[i-1]:=#0;
  end;
end;

{+}procedure DeleteParam(aHandle:cardinal;param:PAnsiChar);
var
  db:TDBCONTACTGETSETTING;
begin
  if aHandle>=NumStorage then
    exit;
  with Storage^[aHandle] do
  begin
    if SType then
      WritePrivateProfileStringA(Section,param,nil,SName)
    else
    begin
      StrCopy(Section+ParOffset,param);
      db.szModule :=SName;
      db.szSetting:=Section;
      PluginLink^.CallService(MS_DB_CONTACT_DELETESETTING,0,lparam(@db));
    end;
  end;
end;

end.
