{MKV file process}
unit fmt_MKV;
{$include compilers.inc}

interface
uses wat_api;

function ReadMKV(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,srv_format;

const
  idEBML              = $A45DFA3;
  idSegment           = $8538067;
  idInfo              = $549A966;
  idTimecodeScale     = $AD7B1;
  idDuration          = $489;
  idTracks            = $654AE6B;
  idTrackEntry        = $2E;
  idTrackType         = $3;
  idCodecPrivate      = $23A2;
  idName              = $136E;
  idVideo             = $60;
  idAudio             = $61;
  idPixelWidth        = $30;
  idPixelHeight       = $3A;
  idDefaultDuration   = $3E383;
  idSamplingFrequency = $35;
  idChannels          = $1F;
  idCluster           = $F43B675;

function GetNumber(var ptr:pbyte):int64;
begin
  if (ptr^ and $80)<>0 then
    result:=ptr^ and $7F
  else if (ptr^ and $40)<>0 then
  begin
    result:=(ptr^ and $3F) shl 8; inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $20)<>0 then
  begin
    result:=(ptr^ and $1F) shl 16; inc(ptr);
    result:=result+(ptr^ shl 8);   inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $10)<>0 then
  begin
    result:=(ptr^ and $0F) shl 24; inc(ptr);
    result:=result+(ptr^ shl 16);  inc(ptr);
    result:=result+(ptr^ shl 8);   inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $08)<>0 then
  begin
    result:=int64(ptr^ and $07) shl 32; inc(ptr);
    result:=result+(ptr^ shl 24);       inc(ptr);
    result:=result+(ptr^ shl 16);       inc(ptr);
    result:=result+(ptr^ shl 8);        inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $04)<>0 then
  begin
    result:=int64(ptr^ and $03) shl 40;  inc(ptr);
    result:=result+(int64(ptr^) shl 32); inc(ptr);
    result:=result+(ptr^ shl 24);        inc(ptr);
    result:=result+(ptr^ shl 16);        inc(ptr);
    result:=result+(ptr^ shl 8);         inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $02)<>0 then
  begin
    result:=int64(ptr^ and $01) shl 48;  inc(ptr);
    result:=result+(int64(ptr^) shl 40); inc(ptr);
    result:=result+(int64(ptr^) shl 32); inc(ptr);
    result:=result+(ptr^ shl 24);        inc(ptr);
    result:=result+(ptr^ shl 16);        inc(ptr);
    result:=result+(ptr^ shl 8);         inc(ptr);
    result:=result+ptr^;
  end
  else if (ptr^ and $01)<>0 then
  begin
    inc(ptr);
    result:=       (int64(ptr^) shl 48);  inc(ptr);
    result:=result+(int64(ptr^) shl 40);  inc(ptr);
    result:=result+(int64(ptr^) shl 32);  inc(ptr);
    result:=result+(ptr^ shl 24);         inc(ptr);
    result:=result+(ptr^ shl 16);         inc(ptr);
    result:=result+(ptr^ shl 8);          inc(ptr);
    result:=result+ptr^;
  end
  else
    result:=0;
  inc(ptr);
end;

function GetInt(var ptr:pbyte;len:integer):int64;
var
  i:integer;
begin
  result:=0;
  for i:=0 to len-1 do
  begin
    result:=(result shl 8)+ptr^;
    inc(ptr);
  end;
end;

function GetFloat(var ptr:pbyte):single;
var
  i:dword;
  f:single absolute i;
begin
  i:=(  ptr^ shl 24); inc(ptr);
  inc(i,ptr^ shl 16); inc(ptr);
  inc(i,ptr^ shl 8);  inc(ptr);
  inc(i,ptr^);        inc(ptr);
  result:=f;
end;

function ReadMKV(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  id,len:integer;
  ptr:pByte;
  buf:array [0..16383] of byte;
  trktype,scale:integer;
  ls:PAnsiChar;
  tmp:integer;
  lTotal:real;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  BlockRead(f,buf,SizeOf(buf));
  ptr:=@buf;
  trktype:=0;
  lTotal:=0;
  scale:=1;
  repeat
    id :=GetNumber(ptr);
    len:=GetNumber(ptr);
    if id=idEBML then // just check
    begin
      result:=true;
      inc(ptr,len);
    end
    else if id=idCluster then
      break
    else if id=idSegment       then // do nothing
    else if id=idInfo          then // do nothing
    else if id=idTracks        then // do nothing
    else if id=idTrackEntry    then // do nothing
    else if id=idVideo         then // do nothing
    else if id=idAudio         then // do nothing
    else if id=idTimecodeScale then
      scale:=GetInt(ptr,len)
    else if id=idDuration then
      lTotal:=GetFloat(ptr)
    else if id=idTrackType then
    begin
      tmp:=trktype;
      trktype:=GetInt(ptr,len); // 1-video,2-audio
      if (tmp=2) and (trktype=2) then
        break;
    end
    else if (id=idCodecPrivate) and (trktype=1) then
    begin
      inc(ptr,16);
      // 4 - ? (40=size included?)
      // width,height
      // 2 - ?
      // 2 - bitperpixel?
      Info.codec:=ptr^; inc(ptr);
      Info.codec:=Info.codec+(ptr^ shl 8 ); inc(ptr);
      Info.codec:=Info.codec+(ptr^ shl 16); inc(ptr);
      Info.codec:=Info.codec+(ptr^ shl 24);
      inc(ptr,len-19);
    end
    else if (id=idName) and (Info.title=NIL) then
    begin
      mGetMem(ls,len+1);
      move(ptr^,ls^,len);
      ls[len]:=#0;
      AnsiToWide(ls,Info.title);
      mFreeMem(ls);
      inc(ptr,len);
    end
    else if id=idPixelWidth then
      Info.width:=GetInt(ptr,len)
    else if id=idPixelHeight then
      Info.height:=GetInt(ptr,len)
    else if id=idDefaultDuration then
    begin
      if trktype=1 then
      begin
        Info.fps:=(GetInt(ptr,len) div 1000);
        if Info.fps<>0 then
          Info.fps:=100000000 div Info.fps;
      end
      else
      begin
        GetInt(ptr,len);
      end;
    end
    else if id=idSamplingFrequency then
      Info.khz:=round(GetFloat(ptr)) div 1000
    else if id=idChannels then
      Info.channels:=GetInt(ptr,len)
    else
      inc(ptr,len);
  until pAnsiChar(ptr)>=(PAnsiChar(@buf)+SizeOf(buf));
  Info.total:=trunc(lTotal/(1000000000/scale));
  CloseHandle(f);
end;

var
  LocalFormatLink:twFormat;

procedure InitLink;
begin
  LocalFormatLink.Next:=FormatLink;

  LocalFormatLink.This.proc :=@ReadMKV;
  LocalFormatLink.This.ext  :='MKV';
  LocalFormatLink.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLink;
end;

initialization
  InitLink;
end.
