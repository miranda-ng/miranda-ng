{WAV processing}
unit fmt_WAV;
{$include compilers.inc}

interface
uses wat_api;

function ReadWAV(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

const
  wavRIFF = $46464952;
  wavWAVE = $45564157;
  wavfmt_ = $20746D66;
  wavfact = $74636166;
  wavdata = $61746164;
type
  tWAVChunk = packed record
    id  :dword;
    size:dword;
  end;
type
  tWAVFormatChunk = packed record
    Codec        :word;
    Channels     :word;
    SampleRate   :dword;
    AvgBPS       :dword;
    BlockAlign   :word;
    BitsPerSample:word;
  end;

const
  WavPackID = $6B707677;
type
//    ckID         :dword; // "wvpk"
//    ckSize       :dword; // size of entire frame (minus 8, of course)
  tWavPackHeader = packed record
    version      :word;  // 0x403 for now
    track_no     :byte;  // track number (0 if not used, like now)
    index_no     :byte;  // track sub-index (0 if not used, like now)
    total_samples:dword; // for entire file (-1 if unknown)
    block_index  :dword; // index of first sample in block (to file begin)
    block_samples:dword; // # samples in This block
    flags        :dword; // various flags for id and decoding
    crc          :dword; // crc for actual decoded data
  end;

function ReadWAV(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  chunk:tWAVChunk;
  fmtchunk:tWAVFormatChunk;
  tmp:dword;
  WPH:tWavPackHeader;
  fsize:dword;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  BlockRead(f,chunk,SizeOf(chunk));
  if chunk.id=WavPackID then
  begin
    BlockRead(f,WPH,SizeOf(tWavPackHeader));
    BlockRead(f,tmp,2); //!! $1621 33,22
    BlockRead(f,chunk,SizeOf(chunk));
  end
  else
  begin
    WPH.version:=0;
    integer(WPH.total_samples):=-1;
  end;
  if chunk.id<>wavRIFF then
    exit;
  BlockRead(f,chunk,SizeOf(dword));
  if chunk.id<>wavWAVE then
    exit;
  BlockRead(f,chunk,SizeOf(chunk));
  if chunk.id<>wavfmt_ then
    exit;
  BlockRead(f,fmtchunk,SizeOf(tWAVFormatChunk));
  Info.channels:=fmtchunk.Channels;
  Info.khz     :=fmtchunk.SampleRate div 1000;
  if chunk.size>SizeOf(tWAVFormatChunk) then
    Skip(f,chunk.size-SizeOf(tWAVFormatChunk));
  fsize:=FileSize(f);
  while FilePos(f)<fsize do
  begin
    BlockRead(f,chunk,SizeOf(chunk));
    if chunk.id=wavfact then
    begin
      BlockRead(f,tmp,4);
      break;
    end;
    if chunk.id=wavdata then
    begin
      tmp:=chunk.size;
      break;
    end;
    Skip(f,chunk.size);
  end;
  if WPH.version<>0 then
  begin
    ReadAPEv2(f,Info);
    ReadID3v1(f,Info);
  end;
  if integer(WPH.total_samples)=-1 then
    if (fmtchunk.BitsPerSample<>0) and (fmtchunk.Channels<>0) then
      WPH.total_samples:=(tmp*8) div (fmtchunk.Channels*fmtchunk.BitsPerSample);
  if fmtchunk.SampleRate<>0 then
    Info.total:= WPH.total_samples div fmtchunk.SampleRate;
  if Info.total<>0 then
    Info.kbps:=tmp*8 div Info.total div 1000;

  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLinkWAV,
  LocalFormatLinkWV:twFormat;

procedure InitLink;
begin
  LocalFormatLinkWAV.Next:=FormatLink;

  LocalFormatLinkWAV.This.proc :=@ReadWAV;
  LocalFormatLinkWAV.This.ext  :='WAV';
  LocalFormatLinkWAV.This.flags:=0;

  FormatLink:=@LocalFormatLinkWAV;

  LocalFormatLinkWV.Next:=FormatLink;

  LocalFormatLinkWV.This.proc :=@ReadWAV;
  LocalFormatLinkWV.This.ext  :='WV';
  LocalFormatLinkWV.This.flags:=0;

  FormatLink:=@LocalFormatLinkWV;
end;

initialization
  InitLink;
end.
