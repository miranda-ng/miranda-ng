{MPC file format}
unit fmt_MPC;
{$include compilers.inc}

interface
uses wat_api;

function ReadMPC(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

const
  DefID = $002B504D;// 'MP+'

function ReadMPC(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  tmp:array [0..5] of dword;
  samples,TotalFrames:dword;
  lastframe:dword;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);

  BlockRead(f,tmp,SizeOf(tmp));
  if ((tmp[0] and $FFFFFF)=DefID) and
     (((tmp[0] shr 24) and $0F)>=7) then // sv7-sv8
  begin
    Info.kbps:=0;
    if (tmp[2] and 2)<>0 then
      Info.channels:=2
    else
      Info.channels:=1;
    case (tmp[2] and $3000) shr 12 of //C000-14?
      00: Info.khz:=44100;
      01: Info.khz:=48000;
      02: Info.khz:=37800;
      03: Info.khz:=32000;
    end;
    lastframe:=(tmp[5] and $FFF) shr 1;
    samples:=tmp[1]*1152+lastframe;
  end
  else
  begin //4-6
    if not ((tmp[0] and $1FFF) and $3FF) in [4..6] then
      exit;
    Info.khz:=44100;
    Info.kbps:=tmp[1] and $1F;
    if ((tmp[0] and $1FFF) and $3FF)=4 then
      TotalFrames:=loword(tmp[2])
    else
      TotalFrames:=tmp[2];
    samples:=TotalFrames*1152;
  end;

  if Info.khz<>0 then
    Info.total:=samples div Info.khz;
  Info.khz:=Info.khz div 1000;
  if (Info.kbps=0) and (samples<>0) then
// if fs=samples*channels*deep/8 then kbps=khz*deep*channels/1152
// Info.kbps:=(Info.khz*8)*taginfo.FileSize/1152/samples;

    Info.kbps:=(Info.khz div 8)*FileSize(f) div samples;  //!!
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

  LocalFormatLink.This.proc :=@ReadMPC;
  LocalFormatLink.This.ext  :='MPC';
  LocalFormatLink.This.flags:=0;

  FormatLink:=@LocalFormatLink;
end;

initialization
  InitLink;
end.
