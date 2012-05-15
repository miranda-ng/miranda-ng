unit general;

interface

uses Windows,Messages,SysUtils,IniFiles,
     m_api,
     ImportT,
     ImportTU;

var MirVers:DWORD;
    IsMirandaUnicode:boolean;
    cp:cardinal;

var
   AppPath:array[0..MAX_PATH] of char;
   TxtPatterns: array of RTxtPattern;
   PatternNames: array of PChar;
   PatternsCount:integer;
   Protocols:array of TDestProto;
   ProtoCount: integer;
   CheckForDuplicates:boolean;
   ShowDuplicates:boolean;

const
  BIN_PROCEDURE_COUNT=6; //количество реализованых процедур бинарного импорта 

const
  {$EXTERNALSYM PBM_SETRANGE}
  PBM_SETRANGE            = WM_USER+1;
  {$EXTERNALSYM PBM_SETPOS}
  PBM_SETPOS              = WM_USER+2;

const
  IMPORT_TXT_MODULE=  'ImportTXT';
  IMPORT_TXT_SERVICE= IMPORT_TXT_MODULE+'/Import';
  IMPORT_WIZ_SERVICE= IMPORT_TXT_MODULE+'/Wizard';

  //keys
  IMPORT_TXT_AS=      'AutoStart';
  IMPORT_TXT_LP=      'LastPattern';

type TOnAccountListChange = procedure ();

var
  OnAccountListChange:TOnAccountListChange;

function ReadPattern(FileName: string):boolean;
procedure ReadPatterns;
procedure ExtractFilePath(fName:PChar);
function fIsMirandaUnicode:boolean;
function GetContactProto(hContact: THandle): String;
function GetContactByUID(proto:string;id:string):THandle;
function GetContactByNick(Proto:string;Nick:string):THandle;
procedure EnumProtocols;
function GetContactID(hContact: THandle; Proto: String = ''; Contact: boolean = false): String;
function GetContactNick(hContact: THandle; Proto: String = ''; Contact: boolean = false): String;
function DBReadByte (hContact:THANDLE;szModule:PChar;szSetting:PChar;default:byte =0):byte;
function DBWriteByte (hContact:THANDLE;szModule:PChar;szSetting:PChar;val:Byte ):Integer;
procedure SetLastPattern(lp:byte);
function GetLastPattern:byte;
function TimeStampToWStr(ts:dword):WideString;
function StrToTimeStamp(STime:PChar;len:integer):dword;

function RLWord(adr:integer):word;
function RLInteger(adr:integer):integer;

implementation


procedure ExtractFilePath(fName:PChar);
var p:PChar;
begin
  p:=fName;
  if p<>nil then
   begin
    while p^<>#0 do inc(p);
    while p^<>'\' do dec(p);
    inc(p);
    p^:=#0;
   end;
end;

function ReadPattern(FileName: string):boolean;
var TI: TIniFile;
    h:integer;
    Err:boolean;
    tempstr:string;
begin
 err:=false;
 TI:=TIniFile.Create(FileName);
 try
  h:=High(TxtPatterns);
  if not TI.SectionExists('General') then begin result:=true; exit; end;
    //if "General" exists
    if TI.ValueExists('General','Name') then TxtPatterns[h].Name:=TI.ReadString('General','Name','')
                                        else err:=true;
    if TI.ValueExists('General','Type') then TxtPatterns[h].IType:=TI.ReadInteger('General','Type',1)
                                        else err:=true;
    case TxtPatterns[h].IType of
    1:
     begin
      if TI.ValueExists('General','Charset') then
       begin
        tempstr:=TI.ReadString('General','Charset','UTF8');
         if tempstr='ANSI' then TxtPatterns[h].Charset:=inANSI
                           else
         if tempstr='UTF8' then TxtPatterns[h].Charset:=inUTF8
                           else
         if tempstr='UCS2' then TxtPatterns[h].Charset:=inUCS2
                           else err:=true;
       end
                                             else err:=true;
      if TxtPatterns[h].Charset=inANSI then
       begin
        TxtPatterns[h].Codepage:= TI.ReadInteger('General','Codepage',0);
        if not IsValidCodePage(TxtPatterns[h].Codepage) then TxtPatterns[h].Codepage:=0;
       end;
      TxtPatterns[h].UseHeader:=TI.ReadInteger('General','UseHeader',0);
      TxtPatterns[h].UsePreMsg:=TI.ReadBool('General','UsePreMsg',false);
      // Read message section
      if TI.SectionExists('Message') then
       begin
        if TI.ValueExists('Message','Pattern') then TxtPatterns[h].Msg.Pattern:=TI.ReadString('Message','Pattern','')
                                               else err:=true;
        TxtPatterns[h].Msg.Incoming:=TI.ReadString('Message','In','');
        TxtPatterns[h].Msg.Outgoing:=TI.ReadString('Message','Out','');
        if TI.ValueExists('Message','Direction') then TxtPatterns[h].Msg.Direction:=TI.ReadInteger('Message','Direction',0)
                                                 else err:=true;
        if TI.ValueExists('Message','Day') then TxtPatterns[h].Msg.Day:=TI.ReadInteger('Message','Day',0)
                                           else err:=true;
        if TI.ValueExists('Message','Month') then TxtPatterns[h].Msg.Month:=TI.ReadInteger('Message','Month',0)
                                             else err:=true;
        if TI.ValueExists('Message','Year') then TxtPatterns[h].Msg.Year:=TI.ReadInteger('Message','Year',0)
                                            else err:=true;
        if TI.ValueExists('Message','Hours') then TxtPatterns[h].Msg.Hours:=TI.ReadInteger('Message','Hours',0)
                                             else err:=true;
        if TI.ValueExists('Message','Minutes') then TxtPatterns[h].Msg.Minutes:=TI.ReadInteger('Message','Minutes',0)
                                               else err:=true;
        TxtPatterns[h].Msg.Seconds:=TI.ReadInteger('Message','Seconds',0)
       end
                                      else err:=true;
      // if need read header section
      if (TxtPatterns[h].UseHeader>0) then
       if TI.SectionExists('Header') then
        begin
         if TI.ValueExists('Header','Pattern') then TxtPatterns[h].Header.Pattern:=TI.ReadString('Header','Pattern','')
                                               else err:=true;
         if (not TI.ValueExists('Header','In')) and ((TxtPatterns[h].UseHeader and 1)=1) then err:=true;
         TxtPatterns[h].Header.Incoming:=TI.ReadInteger('Header','In',0);
         TxtPatterns[h].Header.Outgoing:=TI.ReadInteger('Header','Out',0);
         TxtPatterns[h].Header.InNick:=TI.ReadInteger('Header','InNick',0);
         TxtPatterns[h].Header.OutNick:=TI.ReadInteger('Header','OutNick',0);
         TxtPatterns[h].Header.InUID:=TI.ReadInteger('Header','InUID',0);
         TxtPatterns[h].Header.OutUID:=TI.ReadInteger('Header','OutUID',0);
         if ((TxtPatterns[h].UseHeader and 2)=2) then
           if (TxtPatterns[h].Header.InNick=0) and (TxtPatterns[h].Header.InUID=0) then err:=true;
        end
                                     else err:=true;
      // if nead read PreMessage section
      if TxtPatterns[h].UsePreMsg then
      if TI.SectionExists('PreMessage') then
       begin
        TxtPatterns[h].PreMsg.PreRN:=TI.ReadInteger('PreMessage','PreRN',-1);
        TxtPatterns[h].PreMsg.AfterRN:=TI.ReadInteger('PreMessage','AfterRN',-1);
        TxtPatterns[h].PreMsg.PreSP:=TI.ReadInteger('PreMessage','PreSP',0);
        TxtPatterns[h].PreMsg.AfterSP:=TI.ReadInteger('PreMessage','AfterSP',0);
       end
                                        else err:=true;
     end; //1
    2:
     begin
      TxtPatterns[h].BinProc:= TI.ReadInteger('General','BinProcedure',0);
      if (TxtPatterns[h].BinProc>BIN_PROCEDURE_COUNT) then err:=true;
     end;
    end; //case
  TxtPatterns[h].DefExtension:=TI.ReadString('General','DefaultExtension','txt');
  TxtPatterns[h].UseFileName:=TI.ReadBool('General','UseFileName',false);
  // if nead read FileName section
  if TxtPatterns[h].UseFileName then
   if TI.SectionExists('FileName') then
    begin
     if TI.ValueExists('FileName','Pattern') then TxtPatterns[h].FName.Pattern:=TI.ReadString('FileName','Pattern','')
                                             else err:=true;
     TxtPatterns[h].FName.InNick:=TI.ReadInteger('FileName','InNick',0);
     TxtPatterns[h].FName.InUID:=TI.ReadInteger('FileName','InUID',0);
     if (TxtPatterns[h].FName.InNick=0) and (TxtPatterns[h].FName.InUID=0) then err:=true;
     TxtPatterns[h].FName.OutNick:=TI.ReadInteger('FileName','OutNick',0);
     TxtPatterns[h].FName.OutUID:=TI.ReadInteger('FileName','OutUID',0);
    end
                                 else err:=true;
 finally
  TI.Free;
 end;
 Result:=err;
end;

procedure ReadPatterns;
var
  SR: TSearchRec;
  FileAttrs: Integer;
  i:integer;
begin
 FileAttrs:=faAnyFile;
 i:=0;
 if FindFirst(AppPath+'\importtxt\*.ini',FileAttrs,SR)=0 then
    begin
     repeat
      SetLength(TxtPatterns,i+1);
      SetLength(PatternNames,i+1);
      if not ReadPattern(AppPath+'\importtxt\'+SR.Name) then
             begin
            PatternNames[i]:= PChar(TxtPatterns[i].Name);
            inc(i);
             end;
     until FindNext(SR) <> 0 ;
     FindClose(SR);
    end;
 PatternsCount:=i;
end;


function GetContactByUID(proto:string;id:string):THandle;
var
  contact:THandle;
  otherproto:string;
  ci:TCONTACTINFO;
  idnum:integer;
  tempwstr:PWideChar;
  ws:WideString;
begin
  if not TryStrToInt(id,idnum) then idnum:=0;
  if IsMirandaUnicode then
    begin
     tempwstr:=UTF8ToWide(PChar(id),tempwstr);
     ws :=tempwstr;
     FreeMem(tempwstr);
    end;
  result:=INVALID_HANDLE_VALUE;
  contact:=pluginlink^.CallService(MS_DB_CONTACT_FINDFIRST, 0, 0 );
  while (contact<>0) do
  begin
    otherproto:=PChar(pluginlink^.CallService(MS_PROTO_GETCONTACTBASEPROTO,contact,0));
    if otherproto=proto then
    begin
      ci.cbSize:=SizeOf(ci);
      ci.dwFlag:=CNF_UNIQUEID;
      if IsMirandaUnicode then ci.dwFlag:=ci.dwFlag or CNF_UNICODE;
      ci.hContact:=contact;
      ci.szProto:=PChar(otherproto);
      if pluginlink^.CallService(MS_CONTACT_GETCONTACTINFO,0,integer(@ci))=0 then
      begin
        case ( ci._type ) of
        CNFT_BYTE:   if ci.retval.bVal = idnum then
         begin
          result:=contact;
          break;
         end;
        CNFT_WORD:   if ci.retval.wVal = idnum then
         begin
          result:=contact;
          break;
         end;
        CNFT_DWORD:  if ci.retval.dVal = DWORD(idnum) then
         begin
          result:=contact;
          break;
         end;
        CNFT_ASCIIZ:
        if IsMirandaUnicode then
        if ws=ci.retval.szVal.w then
         begin
          result:=contact;
          break;
         end else
                            else
        if id=ci.retval.szVal.a then
         begin
          result:=contact;
          break;
         end;
        end; //case
      end; //if
    end; //if
    contact:= Pluginlink^.CallService(MS_DB_CONTACT_FINDNEXT,contact,0);
  end; //while
end;

function GetContactByNick(Proto:string;Nick:string):THandle;
var
  contact:THandle;
  otherproto:string;
  ci:TCONTACTINFO;
  tempwstr:PWideChar;
  ws:WideString;
begin
  if IsMirandaUnicode then
    begin
     tempwstr:=UTF8ToWide(PChar(Nick),tempwstr);
     ws :=tempwstr;
     FreeMem(tempwstr);
    end;
  result:=INVALID_HANDLE_VALUE;
  contact:=pluginlink^.CallService(MS_DB_CONTACT_FINDFIRST, 0, 0 );
  while (contact<>0) do
  begin
    otherproto:=PChar(pluginlink^.CallService(MS_PROTO_GETCONTACTBASEPROTO,contact,0));
    if otherproto=proto then
    begin
      ci.cbSize:=SizeOf(ci);
      ci.dwFlag:=CNF_NICK;
      if IsMirandaUnicode then ci.dwFlag:=ci.dwFlag or CNF_UNICODE;
      ci.hContact:=contact;
      ci.szProto:=PChar(otherproto);
      if pluginlink^.CallService(MS_CONTACT_GETCONTACTINFO,0,integer(@ci))=0 then
      begin
        if IsMirandaUnicode then
         begin
          if ws=ci.retval.szVal.w then
           begin
            result:=contact;
            break;
           end;
         end
                            else
         begin
          if Nick=ci.retval.szVal.a then
           begin
            result:=contact;
            break;
           end;
         end;
      end; //if
    end; //if
    contact:= Pluginlink^.CallService(MS_DB_CONTACT_FINDNEXT,contact,0);
  end; //while
end;

procedure EnumProtocols;
var i,iProtoCount:integer;
    ppAccounts:^PPROTOACCOUNT;
    temps:string;
begin
 ProtoCount:=0;
 SetLength(Protocols,30);
 if MirVers> $080000 then pluginLink^.CallService(MS_PROTO_ENUMACCOUNTS,int(@iProtoCount),int(@ppAccounts))
                     else pluginLink^.CallService(MS_PROTO_ENUMPROTOCOLS,int(@iProtoCount),int(@ppAccounts));
 for i:=1 to iProtoCount do
   begin
    if (ppAccounts^^._type=PROTOTYPE_PROTOCOL) then
     begin
      temps:=GetContactID(0,ppAccounts^^.szModuleName,false);
      if temps<>'' then
       begin
        protocols[ProtoCount].ProtoName:=ppAccounts^^.szModuleName;
        protocols[ProtoCount].ProtoUID:=temps;
        protocols[ProtoCount].ProtoNick:=GetContactNick(0,ppAccounts^^.szModuleName,false);
        inc(ProtoCount);
       end;
     end;
    inc(ppAccounts);
   end;
 SetLength(Protocols,ProtoCount);
end;

function fIsMirandaUnicode:boolean;
var ver:ShortString;
begin
 Result:=true;
 SetLength(ver,255);
 if PluginLink.CallService(MS_SYSTEM_GETVERSIONTEXT,wParam(255),lParam(@ver[1]))=0 then
      Result:=Pos('Unicode',ver)>0;
end;

function GetContactProto(hContact: THandle): String;
begin
  Result := PChar(PluginLink.CallService(MS_PROTO_GETCONTACTBASEPROTO,hContact,0));
end;

function DBFreeVariant(dbv:PDBVARIANT):integer;
begin
  Result:=PluginLink^.CallService(MS_DB_CONTACT_FREEVARIANT,0,lParam(dbv));
end;

function GetContactID(hContact: THandle; Proto: String = ''; Contact: boolean = false): String;
var
  uid: PChar;
  dbv: TDBVARIANT;
  cgs: TDBCONTACTGETSETTING;
  tempstr:PChar;
begin
  Result := '';
  if not ((hContact = 0) and Contact) then begin
    if Proto = '' then Proto := GetContactProto(hContact);
    uid := PChar(CallProtoService(PChar(Proto),PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0));
    if (Cardinal(uid) <> CALLSERVICE_NOTFOUND) and (uid <> nil) then begin
      cgs.szModule := PChar(Proto);
      cgs.szSetting := uid;
      cgs.pValue := @dbv;
      if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING,hContact,LPARAM(@cgs)) = 0 then begin
        case dbv._type of
          DBVT_BYTE:
            Result := intToStr(dbv.bVal);
          DBVT_WORD:
            Result := intToStr(dbv.wVal);
          DBVT_DWORD:
            Result := intToStr(dbv.dVal);
          DBVT_ASCIIZ:
            if IsMirandaUnicode then
              begin
               tempstr:=ANSIToUTF8(dbv.szVal.a,tempstr,cp);
               Result :=tempstr;
               FreeMem(tempstr);
              end else Result := dbv.szVal.a;
          DBVT_UTF8:
            if IsMirandaUnicode then Result :=dbv.szVal.a
                                else
             begin
              tempstr:=UTF8ToANSI(dbv.szVal.a,tempstr,cp);
              Result :=tempstr;
              FreeMem(tempstr);
             end;
          DBVT_WCHAR:
            begin
             if IsMirandaUnicode then tempstr:=WideToUTF8(dbv.szVal.w,tempstr)
                                 else tempstr:=WideToAnsi(dbv.szVal.w,tempstr,cp);
             Result:=tempstr;
             FreeMem(tempstr);
            end;
        end;
        // free variant
        DBFreeVariant(@dbv);
      end;
    end;
  end;
end;

function GetContactNick(hContact: THandle; Proto: String = ''; Contact: boolean = false): String;
var
  dbv: TDBVARIANT;
  cgs: TDBCONTACTGETSETTING;
  tempstr:PChar;
begin
  Result := '';
  if not ((hContact = 0) and Contact) then begin
    if Proto = '' then Proto := GetContactProto(hContact);
      cgs.szModule := PChar(Proto);
      cgs.szSetting := 'Nick';
      cgs.pValue := @dbv;
      if PluginLink^.CallService(MS_DB_CONTACT_GETSETTING,hContact,LPARAM(@cgs)) = 0 then begin
        case dbv._type of
          DBVT_BYTE:
            Result := intToStr(dbv.bVal);
          DBVT_WORD:
            Result := intToStr(dbv.wVal);
          DBVT_DWORD:
            Result := intToStr(dbv.dVal);
          DBVT_ASCIIZ:
            if IsMirandaUnicode then
              begin
               tempstr:=ANSIToUTF8(dbv.szVal.a,tempstr,cp);
               Result :=tempstr;
               FreeMem(tempstr);
              end else Result := dbv.szVal.a;
          DBVT_UTF8:
            if IsMirandaUnicode then Result :=dbv.szVal.a
                                else
             begin
              tempstr:=UTF8ToANSI(dbv.szVal.a,tempstr,cp);
              Result :=tempstr;
              FreeMem(tempstr);
             end;
          DBVT_WCHAR:
            begin
             if IsMirandaUnicode then tempstr:=WideToUTF8(dbv.szVal.w,tempstr)
                                 else tempstr:=WideToAnsi(dbv.szVal.w,tempstr,cp);
             Result:=tempstr;
             FreeMem(tempstr);
            end;
        end;
        // free variant
        DBFreeVariant(@dbv);
      end;
  end;
end;

function DBReadByte(hContact:THANDLE;szModule:PChar;szSetting:PChar;default:byte=0):byte;
var
  dbv:TDBVARIANT;
  cgs:TDBCONTACTGETSETTING;
begin
  cgs.szModule :=szModule;
  cgs.szSetting:=szSetting;
  cgs.pValue   :=@dbv;
  If PluginLink^.CallService(MS_DB_CONTACT_GETSETTING,hContact,lParam(@cgs))<>0 then
    Result:=default
  else
    Result:=dbv.bVal;
end;

function DBWriteByte(hContact:THANDLE;szModule:PChar;szSetting:PChar;val:Byte):Integer;
var
  cws:TDBCONTACTWRITESETTING;
begin
  cws.szModule   :=szModule;
  cws.szSetting  :=szSetting;
  cws.value._type:=DBVT_BYTE;
  cws.value.bVal :=Val;
  Result:=PluginLink^.CallService(MS_DB_CONTACT_WRITESETTING,hContact,lParam(@cws));
end;

procedure SetLastPattern(lp:byte);
begin
 DBWriteByte(0,IMPORT_TXT_MODULE,IMPORT_TXT_LP,lp);
end;

function GetLastPattern:byte;
begin
 result:=DBReadByte(0,IMPORT_TXT_MODULE,IMPORT_TXT_LP,0);
 if result>=PatternsCount then result:=0;
end;

function TimeStampToWStr(ts:dword):WideString;
var dbtts:TDBTIMETOSTRING;
    s:WideString;
begin
 SetLength(s,20);
 dbtts.szFormat.w:='d s';
 dbtts.szDest.w:=PWideChar(s);
 dbtts.cbDest:=20;
 pluginlink^.CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT,ts,Int(@dbtts));
 result:=s;
end;

function StrToTimeStamp(STime:PChar;len:integer):dword;
var hour, min, sec, day, month, year: integer;
begin
 sec:=0;
 day:=(ord(stime[0])-$30)*10+(ord(stime[1])-$30);
 month:=(ord(stime[3])-$30)*10+(ord(stime[4])-$30);
 year:=(ord(stime[6])-$30)*1000+(ord(stime[7])-$30)*100+(ord(stime[8])-$30)*10+(ord(stime[9])-$30);
 hour:=(ord(stime[11])-$30)*10+(ord(stime[12])-$30);
 min:=(ord(stime[14])-$30)*10+(ord(stime[15])-$30);
 if len>15 then sec:=(ord(stime[17])-$30)*10+(ord(stime[18])-$30);
  result:=Timestamp(year,month,day,hour,min,sec);
end;

function RLWord(adr:integer):word;
begin
 Result:=PByte(adr+1)^ + (PByte(adr)^ *$100);
end;

function RLInteger(adr:integer):integer;
begin
  Result:=PByte(adr+3)^ + (PByte(adr+2)^*$100) +
 (PByte(adr+1)^ * $10000) + (PByte(adr)^ *$1000000);
end;

begin
  GetModuleFileName(hInstance,@AppPath[0],MAX_PATH);
  ExtractFilePath(AppPath);
  ReadPatterns;
end.
