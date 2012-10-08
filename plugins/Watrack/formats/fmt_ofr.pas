{OFR file}
unit fmt_OFR;
{$include compilers.inc}

interface
uses wat_api;

function ReadOFR(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

type
  tMain = packed record
    ID         :dword; // 'OFR '
    Size       :dword; //15
    SamplesLo  :dword;
    SamplesHi  :word;
    SampleType :byte;
    ChannelsMap:byte;
    Samplerate :dword;
    Encoder    :word;
    Compression:byte;
  end;

function ReadOFR(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  Hdr:tMain;
  Samples:int64;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);
  BlockRead(f,Hdr,SizeOf(Hdr));
  Samples:=Hdr.SamplesLo+Hdr.SamplesHi*$10000;
  Info.channels:=Hdr.ChannelsMap+1;
  Info.khz     :=Hdr.Samplerate div 1000;
  Info.total   :=(Samples div Info.channels)*Info.khz;

  ReadAPEv2(f,Info);
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLinkOFR,
  LocalFormatLinkOFS:twFormat;

procedure InitLink;
begin
  LocalFormatLinkOFR.Next:=FormatLink;

  LocalFormatLinkOFR.This.proc :=@ReadOFR;
  LocalFormatLinkOFR.This.ext  :='OFR';
  LocalFormatLinkOFR.This.flags:=0;

  FormatLink:=@LocalFormatLinkOFR;

  LocalFormatLinkOFS.Next:=FormatLink;

  LocalFormatLinkOFS.This.proc :=@ReadOFR;
  LocalFormatLinkOFS.This.ext  :='OFS';
  LocalFormatLinkOFS.This.flags:=0;

  FormatLink:=@LocalFormatLinkOFS;
end;

initialization
  InitLink;
end.
