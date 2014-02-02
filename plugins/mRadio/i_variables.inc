{Variables support}
const
  numvars = 8;
type
  tvar = packed record
    name :PWideChar;
    help :PAnsiChar;
  end;
const
  vars:array [0..numvars-1] of tvar = (
   (name:'radio_name'   ;help:'Station Name'),
   (name:'radio_url'    ;help:'Station/playlist URL'),
   (name:'radio_active' ;help:'Currently played URL'),
   (name:'radio_genre'  ;help:'Genre'),
   (name:'radio_bitrate';help:'Bitrate'),
   (name:'radio_title'  ;help:'Current stream title'),
   (name:'radio_codec'  ;help:'Currently used decoder'),
   (name:'radio_status' ;help:'Current status'));
// contact,protocol,host,port,file/path

function GetField(ai:PARGUMENTSINFO):pWideChar; cdecl;
var
  i:integer;
  res:PWideChar;
begin
  res:=nil;
  if ActiveContact<>0 then
  begin
    i:=0;
    repeat
      if lstrcmpiw(PWideChar(ai^.argv^),vars[i].name)=0 then
        break;
      inc(i);
    until i=numvars;
    case i of
      0: res:=DBReadUnicode(ActiveContact,strCList  ,optMyHandle);
      1: res:=DBReadUnicode(ActiveContact,PluginName,optStationURL);
      2: StrDupW(res,ActiveURL);
      3: res:=DBReadUnicode(ActiveContact,PluginName,optGenre);
      4: res:=DBReadUnicode(ActiveContact,PluginName,optBitrate);
      5: res:=MakeMessage;
      6: res:=DBReadUnicode(0,PluginName,optActiveCodec);
      7: StrDupW(res,TranslateW(GetStatusText(CallService(MS_RADIO_COMMAND,MRC_STATUS,RD_STATUS_GET))));
    end;
  end;
  if res=nil then
  begin
    mGetMem(res,2);
    res^:=#0;
  end;
  result:=res;
end;

function FreeField(szReturn:PWideChar):int; cdecl;
begin
  mFreeMem(szReturn);
  result:=1;
end;

procedure RegisterVariables;
const
  Prefix:PAnsiChar = 'Radio'#9;
var
  rt:TTOKENREGISTER;
  i:integer;
  s:array [0..127] of AnsiChar;
  p:pAnsiChar;
begin
  rt.cbSize          :=SizeOf(rt);
  rt.memType         :=TR_MEM_OWNER;
  rt.szService       :=@GetField;
  rt.szCleanupService:=@FreeField;
  rt.flags:=TRF_FIELD or TRF_CLEANUP or TRF_PARSEFUNC or TRF_CLEANUPFUNC or TRF_UNICODE;
  p:=StrCopyE(s,Prefix);
  rt.szHelpText:=@s;
  for i:=0 to numvars-1 do
  begin
    rt.szTokenString.w:=vars[i].name;
    StrCopy(p,vars[i].help);
    CallService(MS_VARS_REGISTERTOKEN,0,lparam(@rt));
  end;
end;
