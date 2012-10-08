{AVI file format}
unit fmt_AVI;
{$include compilers.inc}

interface
uses wat_api;

function ReadAVI(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,srv_format;

type
  FOURCC = array [0..3] of AnsiChar;
type
  tChunkHeader = packed record
    case byte of
      0: (Lo,Hi:dword);  {Common}
      1: (ID:FOURCC;     {RIFF}
          Length:dword);
  end;

const
  sRIFF = $46464952;
  sLIST = $5453494C;
  savih = $68697661; { avi header }
  sstrf = $66727473; { stream format }
  sstrh = $68727473; { stream header }
const
  smovi = $69766F6D; {movi list type}
const
  svids = $73646976; {video}
  sauds = $73647561; {audio}
const
  sIART = $54524149; {director}
  sICMT = $544D4349; {comment}
  sICRD = $44524349; {creation date}
  sIGNR = $524E4749; {genre}
  sINAM = $4D414E49; {title}
  sIPRT = $54525049; {part}
  sIPRO = $4F525049; {produced by}
  sISBJ = $4A425349; {subject description}

type
  tWaveFormatEx = packed record
    wFormatTag     :word;
    nChannels      :word;
    nSamplesPerSec :dword;
    nAvgBytesPerSec:dword;
    nBlockAlign    :word;
    wBitsPerSample :word;
    cbSize         :word;

    Reserved1      :word;
    wID            :word;
    fwFlags        :word;
    nBlockSize     :word;
    nFramesPerBlock:word;
    nCodecDelay    :word; {ms}
  end;

type
  tMainAVIHeader = packed record {avih}
    dwMicroSecPerFrame   :dword;
    dwMaxBytesPerSec     :dword;
    dwPaddingGranularity :dword;
    dwFlags              :dword;
    dwTotalFrames        :dword;  { # frames in first movi list}
    dwInitialFrames      :dword;
    dwStreams            :dword;
    dwSuggestedBufferSize:dword;
    dwWidth              :dword;
    dwHeight             :dword;
    dwScale              :dword;
    dwRate               :dword;
    dwStart              :dword;
    dwLength             :dword;
  end;

type
  TAVIExtHeader = packed record {dmlh}
    dwGrandFrames:dword;        {total number of frames in the file}
    dwFuture:array[0..60] of dword;
  end;

type
  tAVIStreamHeader = packed record {strh}
    fccType              :FOURCC; {vids|auds}
    fccHandler           :FOURCC;
    dwFlags              :dword;
    wPriority            :word;
    wLanguage            :word;
    dwInitialFrames      :dword;
    dwScale              :dword;
    dwRate               :dword;
    dwStart              :dword;
    dwLength             :dword;
    dwSuggestedBufferSize:dword;
    dwQuality            :dword;
    dwSampleSize         :dword;
    rcFrame: packed record
      left  :word;
      top   :word;
      right :word;
      bottom:word;
    end;
  end;

var
  vora:dword;

procedure Skip(f:THANDLE;bytes:dword);
var
  i:dword;
begin
  i:=FilePos(f);
  if bytes=0 then
  begin
    if odd(i) then
      Seek(f,i+1);
  end
  else
    Seek(f,i+bytes+(bytes mod 2));
end;

procedure ProcessVideoFormat(f:THANDLE;Size:dword;var Info:tSongInfo);
var
  bih:BitmapInfoHeader;
begin
  BlockRead(f,bih,SizeOf(bih));
  Info.codec :=bih.biCompression;
  Info.width :=bih.biWidth;
  Info.height:=bih.biHeight;
  Skip(f,Size-SizeOf(bih));
end;

procedure ProcessAudioFormat(f:THANDLE;Size:dword;var Info:tSongInfo);
{WAVEFORMATEX or PCMWAVEFORMAT}
var
  AF:tWaveFormatEx;
begin
  BlockRead(f,AF,SizeOf(AF));
  Info.channels:=AF.nChannels;
  Info.khz     :=AF.nSamplesPerSec div 1000;
  Info.kbps    :=(AF.nAvgBytesPerSec*8) div 1000;
  Skip(f,Size-SizeOf(AF));
end;

function ProcessASH(f:THANDLE;var Info:tSongInfo):dword;
var
  ASH:tAVIStreamHeader;
begin
  BlockRead(f,ASH,SizeOf(ASH));
  with ASH do
  begin
    if dword(fccType)=svids then
    begin
      if ASH.dwScale<>0 then
        Info.fps:=(ASH.dwRate*100) div ASH.dwScale;
      if Info.fps<>0 then
        Info.total:=(ASH.dwLength*100) div Info.fps;
      ProcessASH:=1
    end
    else if dword(fccType)=sauds then ProcessASH:=2
    else ProcessASH:=0;
  end;
end;

procedure ProcessMAH(f:THANDLE;var Info:tSongInfo);
var
  MAH:tMainAVIHeader;
begin
  BlockRead(f,MAH,SizeOf(MAH));
//  Info.width:=MAH.dwWidth;
//  Info.height:=MAH.dwHeight;
//  Info.fps:=100000000 div MAH.dwMicroSecPerFrame;
end;

function ProcessChunk(f:THANDLE;var Info:tSongInfo):dword;
var
  lTotal:dword;
  Chunk:tChunkHeader;
  cType:FOURCC;
  ls:PAnsiChar;
begin
  Skip(f,0);
  if (BlockRead(f,Chunk,SizeOF(Chunk))=0) or (Chunk.Lo=0) then
  begin
    result:=FileSize(f);
    Seek(f,FileSize(f));
    exit;
  end;
  result:=Chunk.Length+SizeOf(Chunk);
  case Chunk.Lo of
    sRIFF,sLIST: begin
      BlockRead(f,cType,SizeOf(cType));
      if dword(cType)=smovi then
        Skip(f,Chunk.Length-SizeOf(cType)) // result:=FileSize(f)
      else
      begin
        lTotal:=SizeOf(FOURCC);
        while lTotal<Chunk.Length do
          inc(lTotal,ProcessChunk(f,Info));
      end;
    end;
    sIART,sICMT,sICRD,sIGNR,sINAM,sIPRT,sIPRO,sISBJ: begin
      mGetMem(ls,Chunk.Length);
      BlockRead(f,ls^,Chunk.Length);
      case Chunk.Lo of
        sIART: begin
          AnsiToWide(ls,Info.artist);
        end;
        sICMT: begin
          if Info.comment=NIL then
            AnsiToWide(ls,Info.comment);
        end;
        sICRD: begin
          AnsiToWide(ls,Info.year);
        end;
        sIGNR: begin
          AnsiToWide(ls,Info.genre);
        end;
        sINAM: begin
          AnsiToWide(ls,Info.title);
        end;
        sIPRT: begin
          Info.track:=StrToInt(ls);
        end;
        sIPRO: begin
          if Info.artist=NIL then
            AnsiToWide(ls,Info.artist);
        end;
        sISBJ: begin
          AnsiToWide(ls,Info.comment);
        end;
      end;
      mFreeMem(ls);
    end;
    savih: begin
      ProcessMAH(f,Info);
    end;
    sstrh: begin
      vora:=ProcessASH(f,Info);
    end;
    sstrf: begin
      case vora of
        1: ProcessVideoFormat(f,Chunk.Hi,Info);
        2: ProcessAudioFormat(f,Chunk.Hi,Info);
      else
      end;
    end;
    else
      Skip(f,Chunk.Length);
  end;
end;

function ReadAVI(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ProcessChunk(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLinkAVI,
  LocalFormatLinkDIVX:twFormat;

procedure InitLink;
begin
  LocalFormatLinkAVI.Next:=FormatLink;

  LocalFormatLinkAVI.This.proc :=@ReadAVI;
  LocalFormatLinkAVI.This.ext  :='AVI';
  LocalFormatLinkAVI.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkAVI;

  LocalFormatLinkDIVX.Next:=FormatLink;

  LocalFormatLinkDIVX.This.proc :=@ReadAVI;
  LocalFormatLinkDIVX.This.ext  :='DIVX';
  LocalFormatLinkDIVX.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkDIVX;
end;

initialization
  InitLink;
end.
