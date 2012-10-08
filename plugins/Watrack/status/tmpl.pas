unit Tmpl;

interface
// ----- main data -----
type
  tTemplateType = (
    tmpl_pm    ,tmpl_chat,
    tmpl_xtitle,tmpl_xtext,
    tmpl_stext ,
    tmpl_tunes);
const
  tmpl_first   = tmpl_pm;
  tmpl_last    = tmpl_tunes;
{
const
  TMPL_EMPTY    = 0;
  TMPL_PARENT   = $4000;
  TMPL_INACTIVE = ;
}
procedure CreateTemplates;
procedure FreeTemplates;
procedure SaveTemplates;

function SetTemplateActive(active:boolean;aType:tTemplateType;proto:cardinal=0;
                          ProtoStatus:integer=0):boolean;
function IsTemplateActive(aType:tTemplateType;proto:cardinal=0;
                          ProtoStatus:integer=0):boolean;
function GetTemplateStr(aType:tTemplateType;proto:cardinal=0;
                        ProtoStatus:integer=0):PWideChar;
function SetTemplateStr(aStr:PWideChar;aType:tTemplateType;proto:cardinal=0;
                        ProtoStatus:integer=0):integer;

function GetMacros(TmplType:tTemplateType;proto:integer):pWideChar;

implementation

uses common, m_api, windows, dbsettings, mirutils, protocols,wat_api,global;

const
  dubtmpl = $4000;
const
  DefaultTemplate = 0;
  NumStatus = 10;

type
  pStrTemplate = ^tStrTemplate;
  tStrTemplate = array [0..NumStatus-1,tTemplateType] of SmallInt;

type
  pMyString = ^tMyString;
  tMyString = record
    count:cardinal; // link count
    text :pWideChar;
  end;
  pMyStrArray = ^tMyStrArray;
  tMyStrArray = array [1..1000] of tMyString;

type
  tTmpl = integer;
  pStrTemplates = ^tStrTemplates;
  tStrTemplates = array [0..100] of tStrTemplate;

const
  NumTemplates:cardinal=0;
  StrTemplates:pStrTemplates=nil;

var
  strings:pMyStrArray;
  NumString:cardinal;

const
  defTemplate    = 'I am listening to %artist% - "%title%"';
  defChannelText = '/me listening to %artist% - "%title%"';
  defStatusTitle = 'Now listening to';
  defStatusText  = '%artist% - %title%';

  defAltTemplate    = 'I am listening to %artist% - %title%?iflonger(%album%,0, (from "%album%"),)';
  defAltChannelText = '/me listening to %artist% - %title%?iflonger(%album%,0, (from "%album%"),)';

// ----- procedures -----
{$include i_opt_tmpl.inc}

function AddString(var newstr:PWideChar):cardinal;
var
  i:cardinal;
  tmp:pMyStrArray;
begin
  for i:=1 to NumString do // search in table
  begin
    if StrCmpW(newstr,strings^[i].text)=0 then
    begin
      result:=i;
      mFreeMem(newstr);
      exit;
    end;
  end;
  Inc(NumString);
  mGetMem(tmp,SizeOf(tMyString)*NumString);
  move(strings^,tmp^,SizeOf(tMyString)*(NumString-1));
  mFreeMem(strings);
  strings:=tmp;
  tmp^[NumString].count:=0;
  tmp^[NumString].text:=newstr;
  result:=NumString;
end;

procedure PackStrings;
var
  i,j:integer;
  OldNumString:cardinal;
  lTmplType:tTemplateType;
  lProtoStatus:cardinal;
  tmp:pMyStrArray;
  NumProto:integer;
  tmpl:pStrTemplate;
begin
  // clear counters
  for i:=1 to NumString do
    strings^[i].count:=0;
  // counts strings
  NumProto:=GetNumProto;
  for i:=0 to NumProto do
  begin
    tmpl:=@StrTemplates^[i];
    for lProtoStatus:=0 to NumStatus-1 do
      for lTmplType:=tmpl_first to tmpl_last do
      begin
        j:=tmpl^[lProtoStatus,lTmplType];
        if j>0 then
          inc(strings^[j].count);
      end;
  end;
  // delete strings
  i:=1;
  OldNumString:=NumString;

  if DisablePlugin=dsEnabled then
    DisablePlugin:=dsTemporary;

  while Cardinal(i)<=NumString do
  begin
    if strings^[i].count=0 then
    begin
      mFreeMem(strings^[i].text);
      if cardinal(i)<NumString then
      begin
        // shift strings
        move(strings^[i+1],strings^[i],SizeOf(tMyString)*(NumString-cardinal(i)));
        // shift protos
        for j:=0 to NumProto do
        begin
          tmpl:=@StrTemplates^[j];
          for lProtoStatus:=0 to NumStatus-1 do
            for lTmplType:=tmpl_first to tmpl_last do
            begin
              if tmpl^[lProtoStatus,lTmplType]>i then
                dec(tmpl^[lProtoStatus,lTmplType]);
            end;
        end;
      end;
      dec(NumString);
      continue;
    end;
    inc(i);
  end;
  if OldNumString<>NumString then
  begin
    mGetMem(tmp,SizeOf(tMyString)*NumString);
    move(strings^,tmp^,SizeOf(tMyString)*NumString);
    mFreeMem(strings);
    strings:=tmp;
  end;

//  if DisablePlugin<0 then
//    SetTitle;
  if DisablePlugin<>dsPermanent then
    DisablePlugin:=dsEnabled;

end;

function SetTemplateActive(active:boolean;aType:tTemplateType;proto:cardinal=0;
                          ProtoStatus:integer=0):boolean;
var
  res:smallint;
begin
  if proto>NumTemplates then
    proto:=0;

  res:=ABS(StrTemplates^[proto][ProtoStatus,aType]);
  if not active then res:=-res;
  StrTemplates^[proto][ProtoStatus,aType]:=res;
  result:=res>0;
end;

function IsTemplateActive(aType:tTemplateType;proto:cardinal=0;
                          ProtoStatus:integer=0):boolean;
begin
  if proto>NumTemplates then
    proto:=0;

  result:=StrTemplates^[proto][ProtoStatus,aType]>0;
end;

function GetTmplString(num:integer):pWideChar;
begin
  if (num>0) and (Cardinal(num)<=NumString) then
    result:=strings^[num].text
  else
    result:=nil;
end;

function GetTemplateStr(aType:tTemplateType;proto:cardinal=0;
                        ProtoStatus:integer=0):PWideChar;
var
  i:smallint;
begin
   if proto>NumTemplates then
     proto:=0;

                                    i:=abs(StrTemplates^[proto          ][ProtoStatus,aType]);
  if i=smallint(dubtmpl) then begin i:=abs(StrTemplates^[proto          ][0          ,aType]);
  if i=smallint(dubtmpl) then begin i:=abs(StrTemplates^[DefaultTemplate][ProtoStatus,aType]);
  if i=smallint(dubtmpl) then       i:=abs(StrTemplates^[DefaultTemplate][0          ,aType]); end; end;
  if i=smallint(dubtmpl) then
    i:=0;

  result:=GetTmplString(ABS(i)); //normalize
end;

function SetTemplateStr(aStr:PWideChar;aType:tTemplateType;proto:cardinal=0;
                        ProtoStatus:integer=0):integer;
var
  tmpl:pStrTemplate;
  tmp,tmp1:smallint;
begin
  tmpl:=@StrTemplates^[proto];

  if (aStr=nil) or (aStr^=#0) then
    result:=0
  else
    result:=AddString(aStr);

  tmp1:=result;
  tmp:=tmpl^[0,aType];
  if tmp1=tmp then
    tmp1:=smallint(dubtmpl)
  else if tmp=smallint(dubtmpl) then
  begin
    if tmp1=tmpl^[0,aType] then
      tmp1:=smallint(dubtmpl);
  end;
  tmpl^[ProtoStatus,aType]:=tmp1;
end;

procedure CreateTemplates;
var
  i:integer;
begin
  NumTemplates:=GetNumProto;
  // Size in bytes
  i:=SizeOf(tStrTemplate)*(NumTemplates+1);
  mGetMem(StrTemplates,i);
  //  size in words
  FillWord(StrTemplates^,i div 2,dubtmpl);
  LoadTemplates;
end;

procedure FreeTemplates;
begin
  mFreeMem(StrTemplates);
  while NumString>0 do
  begin
    mFreeMem(strings^[NumString].text);
    dec(NumString);
  end;
  mFreeMem(strings);
end;

function GetMacros(TmplType:tTemplateType;proto:integer):pWideChar;
var
  r:PWideChar;
  status:integer;
begin
  if proto=0 then
    r:=GetTemplateStr(TmplType,0,0)
  else
  begin
    status:=GetProtoStatusNum(proto);
    if IsTemplateActive(TmplType,proto,status) then
      r:=GetTemplateStr(TmplType,proto,status)
    else
    begin
      result:=pWideChar(-1);
      exit;
    end;
  end;

  if r=nil then
    result:=nil
  else
    result:=pWideChar(CallService(MS_WAT_REPLACETEXT,0,lparam(r)));
end;

end.
