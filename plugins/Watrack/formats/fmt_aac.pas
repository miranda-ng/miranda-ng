{AAC file process}
unit fmt_AAC;
{$include compilers.inc}

interface
uses wat_api;

function ReadAAC(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

const
  SampleRates:array [0..15] of dword = (
    96000,88200,64000,48000,44100,32000,24000,22050,
    16000,12000,11025,8000,0,0,0,0);

procedure ReadADIFheader(f:THANDLE;var Info:tSongInfo);
var
  buf:array [0..29] of byte;
  bs,sf_idx,skip:dword;
begin
  BlockRead(f,buf,30);
  if (buf[0] and $80)<>0 then
    skip:=9
  else
    skip:=0;
  Info.kbps:=(((buf[0+skip] and $0F) shl 19)+(buf[1+skip] shl 11)+
        (buf[2+skip] shl 3)+{or}((buf[3+skip] and $E0){shr 5})) div 1000;
  bs:=buf[0+skip] and $10;
  if bs=0 then
    sf_idx:=(buf[7+skip] and $78) shr 3
  else
    sf_idx:=((buf[4+skip] and $07) shl 1)+((buf[5+skip] and $80) shr 7);
  Info.khz:=SampleRates[sf_idx];
end;

procedure ReadADTSheader(var Info:tSongInfo;sign:dword);
type
  l2b=record
    b:array [0..3] of byte;
  end;
var
  sr_idx:integer;
begin
  Info.channels:=((l2b(sign).b[2] and $01) shl 2)+
                 ((l2b(sign).b[3] and $C0) shr 6);
  sr_idx:=(l2b(sign).b[2] and $3C) shr 2;
  Info.khz:=SampleRates[sr_idx] div 1000;
end;

function ReadAAC(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  sign:dword;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);
  BlockRead(f,sign,4);
  Info.khz:=44;
  Info.kbps:=128;
  Info.channels:=2;
  if (lobyte(sign)=$FF) and ((hibyte(sign) and $F6)=$F0) then
    ReadADTSheader(Info,sign)
  else if sign=$46494441 then // 'ADIF'
    ReadADIFheader(f,Info);

  ReadAPEv2(f,Info);
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLink:twFormat;

procedure InitLink;
begin
  LocalFormatLink.Next:=FormatLink;

  LocalFormatLink.This.proc :=@ReadAAC;
  LocalFormatLink.This.ext  :='AAC';
  LocalFormatLink.This.flags:=0;

  FormatLink:=@LocalFormatLink;
end;

initialization
  InitLink;
end.
