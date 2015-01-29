{Save/load options}

const
  opt_numstr:PAnsiChar = 'template/numstr';
const
  ppref = 'proto/';
  spref = 'strings/';

procedure SaveTemplates;
var
  i,lProtoStatus:cardinal;
  lTmplType:tTemplateType;
  p:PAnsiChar;
  buf:PAnsiChar;
  NumProto:cardinal;
  tmpl:pStrTemplate;
  tmp:SmallInt;
  setting:array [0..63] of AnsiChar;
  pset:PAnsiChar;
begin
  DBWriteWord(0,PluginShort,opt_numstr,NumString);
  StrCopy(setting,spref);
  pset:=StrEnd(setting);
  for i:=1 to NumString do
  begin
    IntToStr(pset,i);
    DBWriteUnicode(0,PluginShort,setting,strings^[i].text);
  end;

  NumProto:=GetNumProto;
  mGetMem(buf,16384);
  for i:=0 to NumProto do
  begin
    pset:=StrCopyE(setting,ppref);
    pset:=StrCopyE(pset,GetProtoName(i));

    StrCopy(pset,'/XStatus');
    DBWriteWord(0,PluginShort,setting,GetProtoSetting(i,true));
    inc(pset);
    if i<>0 then
    begin
      StrCopy(pset,'enabled');
      DBWriteWord(0,PluginShort,setting,GetProtoSetting(i));
    end;

    p:=buf;
    tmpl:=@StrTemplates^[i];
    pset[3]:=#0;
    for lProtoStatus:=0 to NumStatus-1 do
      for lTmplType:=tmpl_first to tmpl_last do
      begin
        tmp:=tmpl^[lProtoStatus,lTmplType];
        if tmp=0 then
        begin
          if p<>buf then
          begin
            p^:=',';
            inc(p);
          end;
          p^:='0'; inc(p); // for compatibility
          p^:=AnsiChar(lProtoStatus  +ORD('0')); inc(p);
          p^:=AnsiChar(ORD(lTmplType)+ORD('0')); inc(p);
        end
        else if (tmp<>0) and (tmp<>smallint(dubtmpl)) then
        begin
          pset[0]:='0'; // for compatibility
          pset[1]:=AnsiChar(lProtoStatus  +ORD('0'));
          pset[2]:=AnsiChar(ORD(lTmplType)+ORD('0'));
          DBWriteWord(0,PluginShort,setting,word(tmp));
        end;
      end;
    if p<>buf then
    begin
      p^:=#0;
      StrCopy(pset,'empty');
      DBWriteString(0,PluginShort,setting,buf);
    end;
  end;
  mFreeMem(buf);
end;

procedure InitDefault;
var
  tmpl:pStrTemplate;
begin
  NumString:=8;
  mGetMem (strings ,SizeOf(tMyString)*NumString);
  FillChar(strings^,SizeOf(tMyString)*NumString,0);

  if isVarsInstalled then
  begin
    StrDupW(strings^[1].text,TranslateW(defAltTemplate));
    StrDupW(strings^[4].text,TranslateW(defAltChannelText));
  end
  else
  begin
    StrDupW(strings^[1].text,TranslateW(defTemplate));
    StrDupW(strings^[4].text,TranslateW(defChannelText));
  end;
  StrDupW(strings^[2].text,TranslateW(defStatusTitle));
  StrDupW(strings^[3].text,TranslateW(defStatusText));

  tmpl:=@StrTemplates^[DefaultTemplate];
  if tmpl^[0,tmpl_first]=smallint(dubtmpl) then
  begin
    // music played
    tmpl^[0,tmpl_pm    ]:=1;
    tmpl^[0,tmpl_chat  ]:=4;
    tmpl^[0,tmpl_xtitle]:=2;
    tmpl^[0,tmpl_stext ]:=3;
    tmpl^[0,tmpl_xtext ]:=3;
  end;
end;

function EnumSettingsProc(const szSetting:PAnsiChar;lParam:LPARAM):int; cdecl;
var
  p:^PAnsiChar;
  i:cardinal;
  pp:AnsiChar;
begin
  if StrCmp(ppref,szSetting,Length(ppref))=0 then
  begin
    i:=StrLen(szSetting)+1;
    pp:=szSetting[i-2];
    if (pp>='0') and (pp<='9') then
    begin
      p:=pointer(lParam);
      move(szSetting^,p^^,i);
      inc(p^,i);
    end;
  end;
  result:=0;
end;

function EnumTemplates:PAnsiChar;
var
  ces:TDBCONTACTENUMSETTINGS;
  p:PAnsiChar;
begin
  mGetMem(result,16384);
  result^:=#0;
  p:=result;
  ces.pfnEnumProc:=@EnumSettingsProc;
  ces.lParam     :=lparam(@p);
  ces.szModule   :=PluginShort;
  ces.ofsSettings:=0;
  CallService(MS_DB_CONTACT_ENUMSETTINGS,0,lparam(@ces));
end;

procedure LoadTemplates;
var
  buf:PAnsiChar;
  lProtoStatus,i,j:cardinal;
  lTmplType:tTemplateType;
  p:PAnsiChar;
  pc:PAnsiChar;
  NumProto:cardinal;
  tmpl:pStrTemplate;
  setting:array [0..63] of AnsiChar;
  pset:PAnsiChar;
begin
  NumString:=DBReadWord(0,PluginShort,opt_numstr,0);
  if NumString>0 then
  begin
    mGetMem (strings ,SizeOf(tMyString)*NumString);
    FillChar(strings^,SizeOf(tMyString)*NumString,0);
    StrCopy(setting,spref);
    pset:=StrEnd(setting);
    for i:=1 to NumString do
    begin
      IntToStr(pset,i);
      strings^[i].text:=DBReadUnicode(0,PluginShort,setting,nil);
    end;

    NumProto:=GetNumProto;
    buf:=EnumTemplates;
    for i:=0 to NumProto do
    begin
      pset:=StrCopyE(setting,ppref);
      pset:=StrCopyE(pset,GetProtoName(i));

      StrCopy(pset,'/XStatus');
      j:=DBReadWord(0,PluginShort,setting,$080B);
{!!
      if j=0 then          
        j:=DefaultXStatus;
}
      SetProtoSetting(i,j,true);

      inc(pset);
      if i<>0 then
      begin
        StrCopy(pset,'enabled');
        SetProtoSetting(i,DBReadWord(0,PluginShort,setting,psf_all));
      end;

      tmpl:=@StrTemplates^[i];
      StrCopy(pset,'empty');
      pc:=DBReadString(0,PluginShort,setting,nil);
      if pc<>nil then
      begin
        p:=pc;
        if (p^>='0') and (p^<='9') then
          while p^<>#0 do
          begin
            lProtoStatus :=              ORD(p[1])-ORD('0');
            lTmplType    :=tTemplateType(ORD(p[2])-ORD('0'));
            tmpl^[lProtoStatus,lTmplType]:=0;
            inc(p,3);
            if p^=',' then
              inc(p);
          end;
        mFreeMem(pc);
      end;

      pc:=buf;
      pset^:=#0;
      j:=StrLen(setting);
      pset[3]:=#0;
      while pc^<>#0 do
      begin
        if StrCmp(pc,setting,j)=0 then // only proper proto
        begin
          pc:=StrEnd(pc);
          lProtoStatus :=              ORD((pc-2)^)-ORD('0');
          lTmplType    :=tTemplateType(ORD((pc-1)^)-ORD('0'));

          pset[0]:='0';
          pset[1]:=AnsiChar(lProtoStatus  +ORD('0'));
          pset[2]:=AnsiChar(ORD(lTmplType)+ORD('0'));

          tmpl^[lProtoStatus,lTmplType]:=
              DBReadWord(0,PluginShort,setting,dubtmpl);
        end
        else
          pc:=StrEnd(pc);
        inc(pc);
      end;
    end;
    mFreeMem(buf);
  end
  else
    InitDefault;
end;
