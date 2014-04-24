{RTF related code}
unit MyRTF;
{$include compilers.inc}

interface
uses windows;

procedure SendRTF(wnd:HWND;txt:PWideChar;isUnicode:Boolean;CP:integer=CP_ACP);

implementation
uses richedit,common,messages{,m_api};

const
  RTFBufferSize = 16384;
const
  CTableHdr = '{\colortbl';
const
(*
  ColorTable = '{\colortbl;'+
  '\red255\green255\blue255;'+
  '\red0\green0\blue0;'+
  '\red0\green0\blue127;'+
  '\red0\green147\blue0;'+
  '\red255\green0\blue0;'+
  '\red127\green0\blue0;'+
  '\red156\green0\blue156;'+
  '\red252\green127\blue0;'+
  '\red255\green255\blue0;'+
  '\red0\green252\blue0;'+
  '\red0\green147\blue147;'+
  '\red0\green255\blue255;'+
  '\red0\green0\blue252;'+
  '\red255\green0\blue255;'+
  '\red127\green127\blue127;'+
  '\red210\green210\blue210;}';
*)
  ColorTableD =
  '\red255\green255\blue255;'+
  '\red0\green0\blue0;'+
  '\red0\green0\blue127;'+
  '\red0\green147\blue0;'+
  '\red255\green0\blue0;'+
  '\red127\green0\blue0;'+
  '\red156\green0\blue156;'+
  '\red252\green127\blue0;'+
  '\red255\green255\blue0;'+
  '\red0\green252\blue0;'+
  '\red0\green147\blue147;'+
  '\red0\green255\blue255;'+
  '\red0\green0\blue252;'+
  '\red255\green0\blue255;'+
  '\red127\green127\blue127;'+
  '\red210\green210\blue210;';

function StreamWriteCallback(dwCookie:dword_ptr;pbBuff:PAnsiChar;cb:long;var pcb:long):dword;stdcall;
begin
  pcb:=StrLen(PAnsiChar(dwCookie));
  if cb<pcb then pcb:=cb;
  move(PAnsiChar(dwCookie)^,pbBuff^,pcb);
  result:=0;
end;

procedure WriteRTF(wnd:HWND;const pszText:PAnsiChar);
var
  stream:TEDITSTREAM;
begin
  FillChar(stream,SizeOf(stream),0);
  stream.pfnCallback:=@StreamWriteCallback;
  stream.dwCookie   :=dword_ptr(pszText);
  SendMessage(wnd,EM_STREAMIN,SF_RTF or SFF_PLAINRTF or SFF_SELECTION,lparam(@stream));
end;

function StreamReadCallback(dwCookie:dword_ptr;pbBuff:PAnsiChar;cb:long;var pcb:long):dword;stdcall;
type
  pdword_ptr=^dword_ptr;
begin
  pcb:=cb;
  move(pbBuff^,PAnsiChar(pdword_ptr(dwCookie)^)^,pcb);
//  PAnsiChar(pdword(dwCookie)^)[pcb]:=#0;
  result:=0;
end;

procedure ReadRTF(wnd:HWND;var dst:PAnsiChar);
var
  stream:TEDITSTREAM;
begin
  FillChar(stream,SizeOf(stream),0);
  stream.pfnCallback:=@StreamReadCallback;
  stream.dwCookie:=dword_ptr(@dst);
  SendMessage(wnd,EM_STREAMOUT,SF_RTF+SFF_SELECTION,lparam(@stream));
end;

procedure ReplaceTag(src:PAnsiChar;what,new:PAnsiChar;recurse:boolean);
var
  i:integer;
  block:boolean;
  p:pAnsiChar;
begin
  block:=what^='{';
  repeat
    p:=StrPos(src,what);
    if p<>nil then
    begin
      src:=p;
      if src[StrLen(what)] in ['A'..'Z','a'..'z'] then
      begin
        inc(src);
        continue;
      end;
      i:=1;
      if block then
      begin
        while src[i]<>'}' do inc(i); inc(i);
      end
      else
      begin
        while not (src[i] in ['}',' ','\',';',#13]) do
         inc(i);
      end;
      StrCopy(src,src+i);
      if new<>nil then
        StrInsert(new,src,0);
    end
    else
      break;
    if not recurse then break;
  until false;
end;

procedure ReplaceTags(var src:PAnsiChar);
var
  i:integer;
begin
  ReplaceTag(src,'\b'        ,nil,false);
  ReplaceTag(src,'\i'        ,nil,false);
  ReplaceTag(src,'\ul'       ,nil,false);
  if (StrPos(src,'\{cf')<>nil) or (StrPos(src,'\{bg')<>nil) then
  begin
    ReplaceTag(src,'\cf'       ,nil,false);
    ReplaceTag(src,'\highlight',nil,false);
    StrReplace(src,'\{/cf\}','\cf17 ');
    StrReplace(src,'\{/bg\}','\highlight0 ');
    i:=StrIndex(src,CTableHdr);
    StrInsert(ColorTableD,src,i+integer(StrLen(CTableHdr))+1);
    ReplaceTag(src,'\pard','\pard\cf17',false);
  end;

  StrReplace(src,'\{b\}'  ,'\b1 ');
  StrReplace(src,'\{/b\}' ,'\b0 ');
  StrReplace(src,'\{i\}'  ,'\i1 ');
  StrReplace(src,'\{/i\}' ,'\i0 ');
  StrReplace(src,'\{u\}'  ,'\ul ');
  StrReplace(src,'\{/u\}' ,'\ul0 ');

  repeat
    i:=StrIndex(src,'\{cf');
    if i>0 then
    begin
      StrCopy(src+i,src+i+1);
      i:=StrIndex(src,'\}');
      if i>0 then
      begin
        StrCopy(src+i,src+i+1);
        src[i-1]:=' ';
      end;
    end;
  until i=0;
  repeat
    i:=StrIndex(src,'\{bg');
    if i>0 then
    begin
      StrCopy(src+i,src+i+3);
      StrInsert('highlight',src,i);
      i:=StrIndex(src,'\}');
      if i>0 then
      begin
        StrCopy(src+i,src+i+1);
        src[i-1]:=' ';
      end;
    end;
  until i=0;
end;

function CharCount(p:PWideChar):integer;
begin
  result:=0;
  while p^<>#0 do
  begin
    if p^=#10 then inc(result);
    inc(p);
  end;
end;

procedure SendRTF(wnd:HWND;txt:PWideChar;isUnicode:Boolean;CP:integer=CP_ACP);
var
  tmp:PAnsiChar;
  sstart:integer;
  ls:PAnsiChar;
begin
  SendMessage(wnd,EM_GETSEL,wparam(@sstart),0);
  if isUnicode then
    SendMessagew(wnd,EM_REPLACESEL,0,lparam(txt))
  else
  begin
    SendMessageA(wnd,EM_REPLACESEL,0,lparam(WideToAnsi(txt,ls,CP)));
    mFreeMem(ls);
  end;

  SendMessage(wnd,EM_SETSEL,sstart,sstart+integer(StrLenW(txt))-CharCount(txt));
  mGetMem (tmp ,RTFBufferSize);
  FillChar(tmp^,RTFBufferSize,0);
  ReadRTF(wnd,tmp);
  ReplaceTags(tmp);
  WriteRTF(wnd,tmp);
  mFreeMem(tmp);
  SendMessage(wnd,EM_SETSEL,-1,0);
end;

end.
