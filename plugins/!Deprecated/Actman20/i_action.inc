{Action code}
const
  rtInt  = 1;
  rtWide = 2;

const
  SST_BYTE   = 0;
  SST_WORD   = 1;
  SST_DWORD  = 2;
  SST_QWORD  = 3;
  SST_NATIVE = 4;
  SST_BARR   = 5;
  SST_WARR   = 6;
  SST_BPTR   = 7;
  SST_WPTR   = 8;
  SST_LAST   = 9;
  SST_PARAM  = 10;

const
  protostr = '<proto>';

const
  BufferSize = 8192;

function DBRW(act:pHKAction;hContact:THANDLE;avalue:uint_ptr;
              last,restype:uint_ptr):uint_ptr;
var
  buf ,buf1 :array [0..127] of AnsiChar;
  sbuf:array [0..127] of AnsiChar;
  module,setting:pAnsiChar;
  tmp:pWideChar;
  tmpa,tmpa1:pAnsichar;
begin
  module :=act^.dbmodule;
  setting:=act^.dbsetting;

  with act^ do
  begin
    if restype=rtWide then
      FastWideToAnsiBuf(pWideChar(last),@sbuf)
    else
      IntToStr(sbuf,last);

    if (flags2 and ACF2_RW_MVAR)<>0 then module :=ParseVarString(module ,hContact,sbuf);
    if (flags2 and ACF2_RW_SVAR)<>0 then setting:=ParseVarString(setting,hContact,sbuf);
    StrCopy(buf,module);
    StrReplace(buf,protostr,GetContactProtoAcc(hContact));

    StrReplace(buf,'<last>',sbuf);
    StrCopy(buf1,setting);
    StrReplace(buf1,'<last>',sbuf);

    if (flags2 and ACF2_RW_TVAR)<>0 then
      pWideChar(avalue):=ParseVarString(pWideChar(avalue),hContact,@sbuf);

    if ((flags  and ACF_DBUTEXT)=0) and
       ((flags2 and ACF2_RW_TVAR)<>0) then
    begin
      tmp:=pWideChar(avalue);
      avalue:=StrToInt(tmp);
      mFreeMem(tmp);
    end;

    if (flags and ACF_DBDELETE)<>0 then
    begin
      result:=DBDeleteSetting(hContact,buf,setting);
    end
    else if (flags and ACF_DBWRITE)<>0 then
    begin
      if (flags and ACF_DBANSI)=ACF_DBANSI then
      begin
        WideToAnsi(pWideChar(avalue),tmpa,MirandaCP);
        DBWriteString(hContact,buf,buf1,tmpa);
        mFreeMem(tmpa);
        if (flags2 and ACF2_RW_TVAR)=0 then
          StrDupW(pWideChar(avalue),pWideChar(avalue));
      end
      else if (flags and ACF_DBBYTE )=ACF_DBBYTE then DBWriteByte(hContact,buf,setting,avalue)
      else if (flags and ACF_DBWORD )=ACF_DBWORD then DBWriteWord(hContact,buf,setting,avalue)
      else if (flags and ACF_DBUTEXT)=ACF_DBUTEXT then
      begin
        DBWriteUnicode(hContact,buf,buf1,pWideChar(avalue));
        if (flags2 and ACF2_RW_TVAR)=0 then
          StrDupW(pWideChar(avalue),pWideChar(avalue));
      end
      else DBWriteDWord(hContact,buf,setting,avalue);

      result:=avalue;
    end
    else
    begin
      if (flags and ACF_DBANSI)=ACF_DBANSI then
      begin
        WideToAnsi(pWideChar(avalue),tmpa1,MirandaCP);
        tmpa:=DBReadString(hContact,buf,buf1,tmpa1);
        AnsiToWide(tmpa,PWideChar(result),MirandaCP);
        mFreeMem(tmpa1);
        mFreeMem(tmpa);

        if (flags2 and ACF2_RW_TVAR)<>0 then
          mFreeMem(avalue);
      end
      else if (flags and ACF_DBBYTE )=ACF_DBBYTE then result:=DBReadByte(hContact,buf,setting,avalue)
      else if (flags and ACF_DBWORD )=ACF_DBWORD then result:=DBReadWord(hContact,buf,setting,avalue)
      else if (flags and ACF_DBUTEXT)=ACF_DBUTEXT then
      begin
        result:=uint_ptr(DBReadUnicode(hContact,buf,buf1,pWideChar(avalue)));
        if (flags2 and ACF2_RW_TVAR)<>0 then
          mFreeMem(avalue);
      end
      else result:=DBReadDWord(hContact,buf,setting,avalue);

    end;
    if (flags2 and ACF2_RW_MVAR)<>0 then mFreeMem(module);
    if (flags2 and ACF2_RW_SVAR)<>0 then mFreeMem(setting);
  end;
end;

function OpenContact(hContact:THANDLE):THANDLE;
begin
  ShowContactDialog(hContact);
{
  if CallService(MS_DB_CONTACT_IS,hContact,0)<>0 then
  begin
    if ServiceExists(MS_MSG_CONVERS)<>0 then
    begin
      CallService(MS_MSG_CONVERS,hContact,0)
    end
    else
      CallService(MS_MSG_SENDMESSAGE,hContact,0)
  end;
}
  result:=hContact;
end;

function replany(var str:pWideChar;aparam:LPARAM;alast:pWideChar):boolean;
var
  buf:array [0..31] of WideChar;
  tmp:pWideChar;
begin
  if StrScanW(str,'<')<>nil then
  begin
    result:=true;
    mGetMem(tmp,2048);
    StrCopyW(tmp,str);
    StrReplaceW(tmp,'<param>',IntToStr(buf,aparam));
    StrReplaceW(tmp,'<last>' ,alast);

    str:=tmp;
  end
  else
    result:=false;
end;

function RunProgram(act:pHKAction;aparam:LPARAM;alast:pWideChar):dword;
var
  tmp,tmpp,lpath:PWideChar;
  replPrg ,replArg :PWideChar;
  replPrg1,replArg1:PWideChar;
  pd:LPARAM;
  vars1,vars2,prgs,argss:boolean;
begin
  with act^ do
  begin

    replPrg:=prgname;
    prgs   :=replany(replPrg,aparam,alast);

    replArg:=args;
    argss  :=replany(replArg,aparam,alast);

    if ((flags2 and ACF2_PRG_PRG)<>0) or
       ((flags2 and ACF2_PRG_ARG)<>0) then
    begin
      pd:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
      if (pd=0) and (CallService(MS_DB_CONTACT_IS,aparam,0)<>0) then
        pd:=aparam;
    end;

    if (flags2 and ACF2_PRG_ARG)<>0 then
    begin
      vars2:=true;
      tmp :=ParseVarString(replArg,pd,alast);
    end
    else
    begin
      vars2:=false;
      tmp :=replArg;
    end;

    if (flags2 and ACF2_PRG_PRG)<>0 then
    begin
      vars1:=true;
      tmpp :=ParseVarString(replPrg,pd,alast);
    end
    else
    begin
      vars1:=false;
      tmpp:=replPrg;
    end;

    if StrScanW(tmpp,'%')<>nil then
    begin
      mGetMem(replPrg1,8192*SizeOf(WideChar));
      ExpandEnvironmentStringsW(tmpp,replPrg1,8191);
      if vars1 then mFreeMem(tmpp);
      if prgs  then mFreeMem(replPrg);
      tmpp :=replPrg1;
      prgs :=false;
      vars1:=true;
    end;
    if StrScanW(tmp,'%')<>nil then
    begin
      mGetMem(replArg1,8192*SizeOf(WideChar));
      ExpandEnvironmentStringsW(tmp,replArg1,8191);
      if vars2 then mFreeMem(tmp);
      if argss then mFreeMem(replArg);
      tmp  :=replArg1;
      argss:=false;
      vars2:=true;
    end;

    if (flags and ACF_CURPATH)=0 then
      lpath:=ExtractW(tmpp,false)
    else
      lpath:=nil;

    if (flags and ACF_PRTHREAD)<>0 then
      time:=0
    else if time=0 then
      time:=INFINITE;
    result:=ExecuteWaitW(tmpp,tmp,lpath,show,time,@pd);

    if vars2 then mFreeMem(tmp);
    if vars1 then mFreeMem(tmpp);

    if prgs  then mFreeMem(replPrg);
    if argss then mFreeMem(replArg);
  end;
  mFreeMem(lpath);
end;
{
function MakeStructure(txt:pWideChar;aparam,alast:LPARAM;
         var code,alen:integer;var ofs:int_ptr; restype:integer=rtInt):pointer; forward;
procedure FreeStructure(var struct;descr:pWideChar); forward;
}
function RunService(act:pHKAction;LastResult,Param:LPARAM;var restype:dword):uint_ptr;
var
  res:int_ptr;
  buf:array [0..255] of AnsiChar;
  cc:integer;
  lservice:pAnsiChar;
  lwparam,llparam:LPARAM;
  tmp1,tmp2:pWideChar;
  code,len:integer;
begin
  result:=uint_ptr(-1);

  lservice:=act^.service;
  lwparam :=act^.wparam;
  llparam :=act^.lparam;
  with act^ do
  begin
    if (flags2 and ACF2_SRV_SRVC)<>0 then
      lservice:=ParseVarString(lservice,Param);

    StrCopy(buf,lservice);
    if StrPos(lservice,protostr)<>nil then
      if CallService(MS_DB_CONTACT_IS,Param,0)=0 then
      begin
        if (flags2 and ACF2_SRV_SRVC)<>0 then
          mFreeMem(lservice);
        exit
      end
      else
        StrReplace(buf,protostr,GetContactProtoAcc(Param));

    if ServiceExists(buf)<>0 then
    begin

      cc:=-1;

      tmp1:=nil;
      tmp2:=nil;
      code:=-1;
      if (flags and ACF_WSTRUCT)<>0 then
      begin
        lwparam:=twParam(MakeStructure(pAnsiChar(lwparam),Param,LastResult,restype))
      end
      else if (flags and ACF_WPARAM)<>0 then
      begin
        lwparam:=Param;
      end
      else if (flags and ACF_WRESULT)<>0 then
      begin
        lwparam:=LastResult;
      end
      else if (flags and ACF_WCURRENT)<>0 then
      begin
        cc:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
        lwparam:=cc;
      end
      else if (flags2 and ACF2_SRV_WPAR)<>0 then
      begin
        if (flags and ACF_WPARNUM)=0 then
        begin
          if (flags and ACF_WUNICODE)=0 then
            lwparam:=uint_ptr(ParseVarString(pAnsiChar(lwparam),Param))
          else
            lwparam:=uint_ptr(ParseVarString(pWideChar(lwparam),Param))
        end
        else
        begin
          tmp1:=ParseVarString(pWideChar(lwparam),Param);
          lwparam:=StrToInt(tmp1);
        end;
      end;

      if (flags and ACF_LSTRUCT)<>0 then
      begin
        llparam:=tlParam(MakeStructure(pAnsiChar(llparam),Param,LastResult,restype))
      end
      else if (flags and ACF_LPARAM)<>0 then
      begin
        llparam:=Param;
      end
      else if (flags and ACF_LRESULT)<>0 then
      begin
        llparam:=LastResult;
      end
      else if (flags and ACF_LCURRENT)<>0 then
      begin
        if cc<>-1 then
          llparam:=cc
        else
          llparam:=WndToContact(WaitFocusedWndChild(GetForegroundwindow){GetFocus});
      end
      else if (flags2 and ACF2_SRV_LPAR)<>0 then
      begin
        if (flags  and ACF_LPARNUM)=0 then
        begin
          if (flags and ACF_LUNICODE)=0 then
            llparam:=uint_ptr(ParseVarString(pAnsiChar(llparam),Param))
          else
            llparam:=uint_ptr(ParseVarString(pWideChar(llparam),Param))
        end
        else
        begin
          tmp2:=ParseVarString(pWideChar(llparam),Param);
          llparam:=StrToInt(tmp2);
        end;
      end;

      res:=CallServiceSync(buf,lwparam,llparam);
      result:=res;
      if (flags and ACF_STRING)<>0 then
      begin
//!! delete old or not?
        if (flags and ACF_UNICODE)=0 then
        begin
          AnsiToWide(pAnsiChar(res),pWideChar(result),MirandaCP);
          if (flags2 and ACF2_FREEMEM)<>0 then
            mFreeMem(pAnsiChar(res));
        end
        else if (flags2 and ACF2_FREEMEM)=0 then
          StrDupW(pWideChar(result),pWideChar(res));
        restype:=rtWide;
      end
      else if (flags and ACF_STRUCT)=0 then
        restype:=rtInt;

      if (flags and ACF_WSTRUCT)<>0 then
      begin
        if (flags and ACF_STRUCT)<>0 then
        begin
          result:=GetStructureResult(lwparam,@code,@len);
          case code of
            SST_LAST: begin
              result:=LastResult;
            end;
            SST_PARAM: begin
              result:=Param;
              restype:=rtInt;
            end;
            SST_BYTE,SST_WORD,SST_DWORD,
            SST_QWORD,SST_NATIVE: begin
              restype:=rtInt;
            end;
            SST_BARR: begin
{
              mGetMem(pAnsiChar(res),len+1);
              StrCopy(pAnsiChar(res),pAnsiChar(ofs),len);
}
              StrDup(pAnsiChar(res),pAnsiChar(result),len);
              AnsiToWide(pAnsiChar(res),PWideChar(result),MirandaCP);
              mFreeMem(pAnsiChar(res));
              restype:=rtWide;
            end;
            SST_WARR: begin
{
              mGetMem(pWideChar(result),len+2);
              len:= len div 2;
              StrCopyW(pWideChar(result),pWideChar(ofs),len);
}
              StrDupW(pWideChar(result),pWideChar(result),len);
              restype:=rtWide;
            end;
            SST_BPTR: begin
              AnsiToWide(pAnsiChar(result),pWideChar(result),MirandaCP);
              restype:=rtWide;
            end;
            SST_WPTR: begin
              StrDupW(pWideChar(result),pWideChar(result));
              restype:=rtWide;
            end;
          end;
        end;
        code:=SST_UNKNOWN;
        FreeStructure(lwparam);
        res:=result;
      end
      else if (flags2 and ACF2_SRV_WPAR)<>0 then
      begin
        if (flags  and ACF_LPARNUM)=0 then
          mFreeMem(pAnsiChar(lwparam))
        else
          mFreeMem(tmp1);
      end;

      if (flags and ACF_LSTRUCT)<>0 then
      begin
        if (flags and ACF_STRUCT)<>0 then
        begin
          result:=GetStructureResult(llparam,@code,@len);
          case code of
            SST_LAST: begin
              result:=LastResult;
            end;
            SST_PARAM: begin
              result:=Param;
              restype:=rtInt;
            end;
            SST_BYTE,SST_WORD,SST_DWORD,
            SST_QWORD,SST_NATIVE: begin
              restype:=rtInt;
            end;
            SST_BARR: begin
{
              mGetMem(pAnsiChar(res),len+1);
              StrCopy(pAnsiChar(res),pAnsiChar(ofs),len);
}
              StrDup(pAnsiChar(res),pAnsiChar(result),len);
              AnsiToWide(pAnsiChar(res),PWideChar(result),MirandaCP);
              mFreeMem(pAnsiChar(res));
              restype:=rtWide;
            end;
            SST_WARR: begin
{
              mGetMem(pWideChar(result),len+2);
              len:= len div 2;
              StrCopyW(pWideChar(result),pWideChar(ofs),len);
}
              StrDupW(pWideChar(result),pWideChar(result),len);
              restype:=rtWide;
            end;
            SST_BPTR: begin
              AnsiToWide(pAnsiChar(result),pWideChar(result),MirandaCP);
              restype:=rtWide;
            end;
            SST_WPTR: begin
              StrDupW(pWideChar(result),pWideChar(result));
              restype:=rtWide;
            end;
          end;
        end;
        code:=SST_UNKNOWN;
        FreeStructure(llparam);
        res:=result;
      end
      else if (flags2 and ACF2_SRV_LPAR)<>0 then
      begin
        if (flags  and ACF_LPARNUM)=0 then
          mFreeMem(pAnsiChar(llparam))
        else
          mFreeMem(tmp2);
      end;

      if (flags and (ACF_INSERT or ACF_MESSAGE or ACF_POPUP))<>0 then
      begin
        if restype=rtInt then
        begin
          if (flags and ACF_HEX)<>0 then
            IntToHex(pWideChar(@buf),result)
          else if ((flags and ACF_SIGNED)<>0) and (res<0) then
          begin
            pWideChar(@buf)[0]:='-';
            IntToStr(PWideChar(@buf)+1,-result);
          end
          else
            IntToStr(pWideChar(@buf),result);
          res:=int_ptr(@buf);
        end
        else
          res:=result;
      end;

      if (flags and ACF_INSERT )<>0 then SendMessageW(WaitFocusedWndChild(GetForegroundwindow){GetFocus},EM_REPLACESEL,0,res);
      if (flags and ACF_POPUP  )<>0 then ShowPopupW(PWideChar(res));
      if (flags and ACF_MESSAGE)<>0 then MessageBoxW(0,PWideChar(res),'',0);

    end;
    if (flags2 and ACF2_SRV_SRVC)<>0 then
      mFreeMem(lservice);
  end;
end;

procedure PasteClipboard(dst:pWideChar);
var
  p:pWideChar;
  fh:tHandle;
begin
  if StrPosW(dst,'^v')<>nil then
  begin
{
    p:=PasteFromClipboard(false);
    StrReplaceW(dst,'^v',p);
    mFreeMem(p);
}
    if OpenClipboard(0) then
    begin
      fh:=GetClipboardData(cf_UnicodeText);
      p:=GlobalLock(fh);
      StrReplaceW(dst,'^v',p);
      GlobalUnlock(fh);
      CloseClipboard;
    end
  end
end;

type
  trec = record
    text:PAnsiChar;
    one, two:integer;
  end;

function GetFileString(fname:PAnsiChar;linenum:integer):pWideChar;
var
  pc,FileBuf,CurLine:PAnsiChar;
  f:THANDLE;
  NumLines, j:integer;
begin
  f:=Reset(fname);
  if f<>INVALID_HANDLE_VALUE then
  begin
    j:=FileSize(f);
    mGetMem(FileBuf,j+1);
    BlockRead(f,FileBuf^,j);
    while (FileBuf+j)^<' ' do dec(j);
    (FileBuf+j+1)^:=#0;
    CloseHandle(f);
    pc:=FileBuf;
    CurLine:=pc;
    NumLines:=1;
    while pc^<>#0 do // count number of lines
    begin
      if pc^=#13 then
      begin
        if linenum=NumLines then
          break;
        inc(pc);
        if pc^=#10 then
          inc(pc);
        inc(NumLines);
        CurLine:=pc;
      end
      else
        inc(pc);
    end;
    if (linenum>NumLines) or (linenum=0) then //ls - lastline
    else if linenum<0 then
    begin
      randomize;
      linenum:=random(NumLines)+1;
      pc:=FileBuf;
      NumLines:=1;
      CurLine:=pc;
      repeat
        if (pc^=#13) or (pc^=#0) then
        begin
          if linenum=NumLines then
            break;
          if pc^<>#0 then
          begin
            inc(pc);
            if pc^=#10 then
              inc(pc);
          end;
          inc(NumLines);
          CurLine:=pc;
        end
        else
          inc(pc);
      until false;
    end;
    pc^:=#0;
    StrReplace(CurLine,'\n',#13#10);
    StrReplace(CurLine,'\t',#09);
    AnsiToWide(CurLine,result,CP_ACP);
    mFreeMem(FileBuf);
  end
  else
    result:=nil;
end;

function Split(buf:PWideChar;macro:PWideChar;var r:trec):integer;
type
  tconv = packed record
    case boolean of
      false: (res:int);
      true: (lo,hi:word);
  end;
var
  i:integer;
  p,pp,lp:pWideChar;
  ls:array [0..511] of WideChar;
begin
  result:=0;
  i:=StrIndexW(buf,macro);
  if i>0 then
  begin
    dec(i);
    p:=buf+i+StrLenW(macro);
    pp:=p;
    while (p^<>#0) and (p^<>')') do
      inc(p);
    ls[0]:=#0;
    if p^<>#0 then // correct syntax
    begin
      lp:=ls;
      while (pp<>p) and (pp^<>',') do // filename
      begin
        lp^:=pp^;
        inc(lp);
        inc(pp);
      end;
      lp^:=#0;
      WideToAnsi(ls,r.text,MirandaCP);
      r.one:=-1;
      r.two:=-1;
      if pp^=',' then
      begin
        inc(pp);
        r.one:=StrToInt(pp);
        while (pp<>p) and (pp^<>',') do inc(pp);
        if pp^=',' then
        begin
          inc(pp);
          r.two:=StrToInt(pp);
        end;
      end;
      tconv(result).lo:=p-buf-i+1; // length
      tconv(result).hi:=i;   // position
    end;
  end;
end;

procedure PasteFileString(dst:pWideChar);
var
  i:integer;
  lp:pWideChar;
  buf:array [0..511] of AnsiChar;
  r:trec;
begin
  repeat
    i:=Split(dst,'^f(',r);
    if i>0 then
    begin
      StrDeleteW(dst,i shr 16,loword(i));
      ConvertFileName(r.text,buf);
      lp:=GetFileString(@buf,r.one);
      if lp<>nil then
      begin
        StrInsertW(lp,dst,i shr 16);
        mFreeMem(lp);
      end;
    end
    else
      break;
  until false;
end;

procedure PasteSelectedText(wnd:hwnd;dst:pWideChar);
var
  sel:integer;
  buf:pWideChar;
begin
  if (StrPosW(dst,'^s')<>nil) and (wnd<>0) then
  begin
    sel:=SendMessageW(wnd,EM_GETSEL,0,0);
    if loword(sel)=(sel shr 16) then
      StrReplaceW(dst,'^s',nil)
    else
    begin
      buf:=GetDlgText(wnd,false);
//!!    next line was active. why?
//      SendMessageW(wnd,WM_GETTEXT,4095,dword(@buf));
      buf[sel shr 16]:=#0;
      StrReplaceW(dst,'^s',buf+loword(sel));
      mFreeMem(buf);
    end;
  end;
end;

function CheckAuto(dst:pWideChar):bool;
var
  p:PWideChar;
begin
  result:=false;
  if dst<>nil then
  begin
    p:=StrEndW(dst);
    if (p-dst)>2 then
    begin
      dec(p,2);
      if (p^='^') and ((p+1)^='a') then
      begin
        result:=true;
        p^:=#0;
      end;
    end;
  end;
end;

function InsertText(act:pHKAction;param:LPARAM;last:pWideChar):uint_ptr;
var
  tmp:PWideChar;
  blob,p:PAnsiChar;
  w:PWideChar;
  hContact:THANDLE;
  wnd:HWND;
  fexist,autosend:bool;
  dbei:TDBEVENTINFO;
  i:cardinal;
  cp:integer;
  fh:THANDLE;
  lstr:pWideChar;
  llen:integer;
  buf:array [0..31] of WideChar;
  b,b1:array [0..MAX_PATH] of AnsiChar;
begin
  result:=uint_ptr(last);
  with act^ do
  begin
    if (flags and ACF_CLIPBRD)<>0 then
    begin
      if (flags and ACF_COPYTO)<>0 then
        CopyToClipboard(last,false)
      else
        result:=uint_ptr(PasteFromClipboard(false));
      exit;
    end;

    hContact:=0;
    if (flags and ACF_FILE)=0 then
    begin
      wnd:=WaitFocusedWndChild(GetForegroundWindow){GetFocus};
      if wnd<>0 then
        hContact:=WndToContact(wnd);
    end
    else
      wnd:=0;

    if hContact=0 then
    begin
      if CallService(MS_DB_CONTACT_IS,param,0)<>0 then
        hContact:=param;
    end;

    if (flags and (ACF_FILE or ACF_FAPPEND or ACF_FWRITE))<>ACF_FILE then
    begin
      mGetMem (w ,BufferSize*SizeOf(WideChar));
      FillChar(w^,BufferSize*SizeOf(WideChar),0);
      StrCopyW(w,text);
      PasteClipboard(w);        // ^v
      PasteFileString(w);       // ^f
      PasteSelectedText(wnd,w); // ^s
      autosend:=CheckAuto(w);   // ^a
      StrReplaceW(w,'^l',last); // ^l
      StrReplaceW(w,'^h',IntToHex(buf,StrToInt(last)));  // ^h
      StrReplaceW(w,'^t',#9);   // ^t
      StrReplaceW(w,'^e',nil);  // ^e
    end
    else
      autosend:=false;

    if (flags2 and ACF2_TXT_TEXT)<>0 then
    begin
      tmp:=ParseVarString(w,hContact,last);
      mFreeMem(w);
      w:=tmp;
    end;

    if (flags and ACF_FILE)<>0 then
    begin
      cp:=0;
      if      (flags and ACF_ANSI)=ACF_ANSI then cp:=1
      else if (flags and (ACF_UTF8 or ACF_SIGN))=ACF_UTF8 then cp:=2
      else if (flags and (ACF_UTF8 or ACF_SIGN))=ACF_SIGN then cp:=4
      else if (flags and (ACF_UTF8 or ACF_SIGN))=(ACF_UTF8 or ACF_SIGN) then cp:=3;

      if (flags2 and ACF2_TXT_FILE)<>0 then
        tmp:=ParseVarString(tfile,hContact,last)
      else
        tmp:=tfile;

      if (flags and (ACF_FAPPEND or ACF_FWRITE))<>0 then
      begin
        case cp of
          1: begin
            llen:=StrLen(WideToAnsi(w,pAnsiChar(lstr),MirandaCP));
          end;
          2,3: begin
            llen:=StrLen(WideToUTF8(w,pAnsiChar(lstr)));
          end;
        else
          lstr:=w;
          llen:=StrLenW(lstr)*SizeOf(WideChar);
        end;
      end
      else
        llen:=0;

      fexist:=FileExists(tmp);
      if fexist and ((flags and ACF_FAPPEND)<>0) then
      begin
        fh:=Append(tmp);
        if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        begin
          BlockWrite(fh,lstr^,llen);
        end;
        if (cp<>0) and (cp<>4) then
          mFreeMem(lstr);
      end
      else if ((flags and ACF_FWRITE)<>0) or
              (not fexist and ((flags and ACF_FAPPEND)<>0)) then
      begin
        fh:=ReWrite(tmp);
        if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        begin
          if cp=3 then
          begin
            i:=SIGN_UTF8;
            BlockWrite(fh,i,3); // UTF8 sign
          end
          else if cp=4 then
          begin
            i:=SIGN_UNICODE;
            BlockWrite(fh,i,2); // UTF16 sign
          end;

          BlockWrite(fh,lstr^,llen);
          if (cp<>0) and (cp<>4) then
            mFreeMem(lstr);
        end;
      end
      else
      begin
        if StrPosW(tmp,'://')<>nil then // remote
        begin
          GetTempPathA(MAX_PATH,b);
          GetTempFileNameA(b,'wat',GetCurrentTime,b1);
          GetFile(FastWideToAnsiBuf(tmp,b),b1);
          if tmp<>tfile then
            mFreeMem(tmp);
          FastAnsiToWide(b1,tmp);
        end
        else
          b1[0]:=#0;
        fh:=Reset(tmp);
        if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        begin
          i:=GetFSize(tmp);
          mGetMem (w ,i+SizeOf(WideChar));
          FillChar(w^,i+SizeOf(WideChar),0);
          BlockRead(fh,w^,i);
          if (flags and ACF_ANSI)<>0 then
          begin
            AnsiToWide(pAnsiChar(w),lstr,MirandaCP);
            mFreeMem(w);
            w:=lstr;
          end
          else if (flags and ACF_UTF8)<>0 then
          begin
            if (pdword(w)^ and $FFFFFF)=SIGN_UTF8 then
              p:=pAnsiChar(w)+3
            else
              p:=pAnsiChar(w);
            mFreeMem(w);
            UTF8ToWide(p,w);
          end
          else
            ChangeUnicode(w);
        end;
        if b1[0]<>#0 then
          DeleteFileA(b1);
      end;
      if fh<>THANDLE(INVALID_HANDLE_VALUE) then
        CloseHandle(fh);
      if tmp<>tfile then
        mFreeMem(tmp);
    end;

    result:=uint_ptr(w);

    if (flags and ACF_FILE)=0 then
    begin
      if autosend then
      begin
        if hContact=0 then exit;
        p:=GetContactProtoAcc(hContact);
        cp:=DBReadDWord(hContact,'Tab_SRMsg','ANSIcodepage',MirandaCP);
        if DBReadByte(hContact,p,'ChatRoom',0)<>1 then
        begin
          i:=WideToCombo(w,blob,cp);
  //          if CallContactService(hContact,PSS_MESSAGEW,0,dword(blob))=
  //             ACKRESULT_FAILED then
          CallContactService(hContact,PSS_MESSAGE,PREF_UNICODE,tlparam(blob));
          dbei.cbSize   :=sizeof(dbei);
          dbei.cbBlob   :=i;
          dbei.pBlob    :=pByte(blob);
          dbei.eventType:=EVENTTYPE_MESSAGE;
          dbei.timestamp:=GetCurrentTime;
          dbei.szModule :=p;
          dbei.flags    :=DBEF_SENT;
          db_event_add(hContact, @dbei);
          mFreeMem(blob);
        end
        else
          SendToChat(hContact,w);
      end
      else
      begin
        GetWindowThreadProcessId(GetForegroundWindow,@i);
        if (i=GetCurrentProcessId) and (wnd<>0) then
          SendMessageW(wnd,EM_REPLACESEL,1,tlparam(w))
        else
          SendString(0,w);
      end;
    end;
  end;
end;
