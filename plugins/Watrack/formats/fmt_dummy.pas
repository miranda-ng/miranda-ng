{OFR file}
unit fmt_Dummy;
{$include compilers.inc}

interface
uses wat_api;

function ReadDummy(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

function ReadDummy(var Info:tSongInfo):boolean; cdecl;
{
var
  f:THANDLE;
}
begin
{
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;

  CloseHandle(f);
}
  result:=true;
end;

var
  LocalFormatLinkCUE:twFormat;

procedure InitLink;
begin
  LocalFormatLinkCUE.Next:=FormatLink;

  LocalFormatLinkCUE.This.proc :=@ReadDummy;
  LocalFormatLinkCUE.This.ext  :='CUE';
  LocalFormatLinkCUE.This.flags:=WAT_OPT_CONTAINER;

  FormatLink:=@LocalFormatLinkCUE;
end;

initialization
  InitLink;
end.
