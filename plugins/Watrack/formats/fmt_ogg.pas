{OGG, SPX and FLAC file formats}
unit fmt_OGG;
{$include compilers.inc}

interface
uses wat_api;

function ReadOGG(var Info:tSongInfo):boolean; cdecl;
function ReadSPX(var Info:tSongInfo):boolean; cdecl;
function ReadfLaC(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format,utils;

const
  OGGSign = $5367674F; //OggS
const
  SpeexID = 'Speex   ';
type
  tSPEXHeader = packed record
    speex_string    :array [0..7] of AnsiChar;
    speex_version   :array [0..19] of AnsiChar;
    speex_version_id:dword;
    header_size     :dword; //sizeof(tSPEXHeader)
    rate            :dword;
    mode            :dword;
    bitstrm_version :dword;
    nb_channels     :dword;
    bitrate         :dword;
    frame_size      :dword;
    vbr             :dword;
    fpp             :dword; //frames_per_packet
    extra_headers   :dword;
    reserved1       :dword;
    reserved2       :dword;
  end;
type
  pOGGHdr = ^tOGGHdr;
  tOGGHdr = packed record
    ID       :dword;
    Version  :byte;
    HdrType  :byte;
    Granule  :Int64; // absolute position
    BitStrmSN:dword;
    PageSeqN :dword;
    CRC      :dword;
    PageSegs :byte;
  end;
const
  strmOGG = 1;
  strmOGM = 2;
const
  VideoD  = $65646976;
  VideoW  = $006F;
  VorbisD = $62726F76;
  VorbisW = $7369;
type
  tOGMInfo = packed record
    padding         :word;  // 0
    codec           :dword;
    size            :dword;
    time_unit       :int64; // 1/10000000 sec
    samples_per_unit:int64; // fps = 10000000*spu/time_unit
    default_len     :dword; // 1
    buffersize      :dword;
    bit_per_sample  :dword;
    width           :dword;
    height          :dword;
    dummy           :dword; // 0
  end;

//const VorbisStream:array [0..5] of byte = ($76,$6F,$72,$62,$69,$73); // 'vorbis'

type
  tOGGInfo = packed record
    version   :dword;
    Channels  :byte;
    samplerate:dword;
    maxkbps   :dword;
    nominal   :dword;
    minkbps   :dword;
    BlockSizes:byte;
    dummy     :byte;
  end;

//--------------- fLaC section ---------------
const
  fLaCSign = $43614C66; //fLaC
{
0 : STREAMINFO
1 : PADDING
2 : APPLICATION
3 : SEEKTABLE
4 : VORBIS_COMMENT
5 : CUESHEET
}
type
  tMetaHdr = packed record
    blocktype:byte;
    blocklen:array [0..2] of byte;
  end;
type
  tStreamInfo = packed record
    MinBlockSize:word;
    MaxBlocksize:word;
    MinFrameSize:array [0..2] of byte;
    MaxFrameSize:array [0..2] of byte;
    heap:array [0..7] of byte;
    MD5:array [0..15] of byte;
  end;

procedure OGGGetComment(ptr:PAnsiChar;size:integer;var Info:tSongInfo);
var
  alen,len,values:dword;
  clen:int;
  ls:PAnsiChar;
  value:PAnsiChar;
  cover:pByte;
  ext:dword;
  extw:int64;
  c:AnsiChar;
begin
  inc(ptr,pdword(ptr)^+4); //vendor
  values:=pdword(ptr)^; inc(ptr,4);
  ext:=0;
  cover:=nil;
  clen:=0;
  while values>0 do
  begin
    len:=pdword(ptr)^;
    if len>cardinal(size) then
      break;
    dec(size,len);
    inc(ptr,4);
    ls:=ptr;
    c:=ls[len];
    ls[len]:=#0;
    alen:=StrScan(ls,'=')-ls+1;
    if alen>0 then
    begin
      ls[alen-1]:=#0;
      value:=ls+alen;

      if      (Info.title  =nil) and (lstrcmpia(ls,'TITLE'  )=0) then UTF8ToWide(value,Info.title)
      else if (Info.artist =nil) and (lstrcmpia(ls,'ARTIST' )=0) then UTF8ToWide(value,Info.artist)
      else if (Info.album  =nil) and (lstrcmpia(ls,'ALBUM'  )=0) then UTF8ToWide(value,Info.album)
      else if (Info.genre  =nil) and (lstrcmpia(ls,'GENRE'  )=0) then UTF8ToWide(value,Info.genre)
      else if (Info.year   =nil) and (lstrcmpia(ls,'DATE'   )=0) then UTF8ToWide(value,Info.year)
      else if (Info.comment=nil) and (lstrcmpia(ls,'COMMENT')=0) then UTF8ToWide(value,Info.comment)
      else if (Info.lyric  =nil) and (lstrcmpia(ls,'LYRICS' )=0) then UTF8ToWide(value,Info.lyric)

      else if (Info.track=0) and (lstrcmpia(ls,'TRACKNUMBER')=0) then Info.track:=StrToInt(value)

      else if (cover=nil) and (lstrcmpia(ls,'COVERART')=0) then cover:=mir_base64_decode(value,clen)
      else if  lstrcmpia(ls,'COVERARTMIME')=0 then ext:=GetImageType(nil,value);
    end;
    dec(values);
    inc(ptr,len);
    ptr^:=c;
  end;

  if cover<>nil then
  begin
    if ext=0 then
      ext:=GetImageType(cover);
    if ext<>0 then
    begin
      FastAnsiToWideBuf(PAnsiChar(@ext),pWideChar(@extw));
      Info.cover:=SaveTemporaryW(cover,clen,PWideChar(@extw));
    end;
    mFreeMem(cover);
  end;

end;

function CalcSize(num:integer;var arr:array of byte):integer;
var
  i:integer;
begin
  result:=0;
  for i:=0 to num-1 do
  begin
    inc(result,arr[i]);
    if arr[i]<$FF then break;
  end;
end;

function ReadSPX(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  OGGHdr:tOGGHdr;
  SPXHdr:tSPEXHeader;
  buf:array [0..255] of byte;
  ptr:PAnsiChar;
  size:integer;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  BlockRead(f,OGGHdr,SizeOf(tOGGHdr));
  Skip(f,OGGHdr.PageSegs);
  if OGGHdr.ID=OGGSign then
  begin
    BlockRead(f,SPXHdr,SizeOf(SPXHdr));
    if SPXHdr.speex_string<>SpeexID then
    begin
      CloseHandle(f);
      exit;
    end;
    Info.khz:=SPXHdr.rate div 1000;
    Info.vbr:=SPXHdr.vbr;
    if integer(SPXHdr.bitrate)<>-1 then
      Info.kbps:=SPXHdr.bitrate div 1000;

    BlockRead(f,OGGHdr,SizeOf(tOGGHdr));
    BlockRead(f,buf,OGGHdr.PageSegs);
    size:=CalcSize(OGGHdr.PageSegs,buf);
    GetMem(ptr,size+1);
    BlockRead(f,ptr^,size);
    OGGGetComment(ptr,size,Info);
    FreeMem(ptr);

    result:=true;
  end;
  CloseHandle(f);
end;

function Compare(const sign:array of byte):integer;
type
  conv=packed record
    d:dword;w:word;
  end;
var
  p:^conv;
begin
  p:=@sign;
  if (p^.d=VideoD) and (p^.w=VideoW) then
    result:=strmOGM
  else if (p^.d=VorbisD) and (p^.w=VorbisW) then
    result:=strmOGG
  else
    result:=0;
end;

function ReadOGG(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  OGGHdr:tOGGHdr;
  tmp:packed record
    paktype:byte;
    sign:array [0..5] of byte;
  end;
  OGGInfo:tOGGInfo;
  OGMInfo:tOGMInfo;
  fpos:dword;
  SPXHdr:tSPEXHeader;
  i,j:integer;
  DataIndex:integer;
  buf:array [0..255] of byte;
  fsize:dword;
  done:integer;
  ptr:PAnsiChar;
  size:integer;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  tmp.paktype:=0;
  fsize:=FileSize(f);
  done:=0;
  while (done<>3) and (tmp.paktype<>5) and (FilePos(f)<fsize) do
  begin
    BlockRead(f,OGGHdr,SizeOf(tOGGHdr));
    if OGGHdr.ID=OGGSign then
    begin
      BlockRead(f,buf,OGGHdr.PageSegs);
      size:=CalcSize(OGGHdr.PageSegs,buf);
// sum pages to size obtain and number of groups
//      for i:=0 to OGGHdr.PageSegs-1 do
// only first fragment
      begin
        fpos:=FilePos(f);
        BlockRead(f,tmp,SizeOf(tmp));
        if tmp.paktype=5 then
          break;
        if tmp.paktype=1 then
        begin
          case Compare(tmp.sign) of
            strmOGG: begin
              BlockRead(f,OGGInfo,SizeOf(OGGInfo));
              if integer(OGGInfo.nominal)>0 then
                Info.kbps    :=OGGInfo.nominal div 1000;
              Info.khz     :=OGGInfo.samplerate;
              Info.channels:=OGGInfo.Channels;
              done:=done or 1;
            end;
            strmOGM: begin
              BlockRead(f,OGMInfo,SizeOf(OGMInfo));
              Info.codec :=OGMInfo.codec;
              Info.fps   :=round(((10000000*OGMInfo.samples_per_unit) / OGMInfo.time_unit)*100);
              Info.width :=OGMInfo.width;
              Info.height:=OGMInfo.height;
              done:=done or 1;
            end;
          end;
        end
        else if tmp.paktype=ORD('S') then //maybe SPX
        begin
          Seek(f,fpos);
          BlockRead(f,SPXHdr,SizeOf(SPXHdr));
          if SPXHdr.speex_string<>SpeexID then
          begin
            CloseHandle(f);
            exit;
          end;
          Info.khz:=SPXHdr.rate div 1000;
          if integer(SPXHdr.bitrate)<>-1 then
            Info.kbps:=SPXHdr.bitrate div 1000;
          done:=done or 1;
        end
        else if tmp.paktype=3 then
        begin
          GetMem(ptr,size+1);
          BlockRead(f,ptr^,size);
          OGGGetComment(ptr,size,Info);
          FreeMem(ptr);
          done:=done or 2;
        end
        else
          continue;
        result:=true;
      end;
    end;
  end;
  // try to get length
  DataIndex:=FileSize(f)-10;
  for i:=1 to 50 do
  begin
    dec(DataIndex,SizeOf(buf)-10);
    Seek(f,DataIndex);
    BlockRead(f,buf,SizeOf(buf));
    { Get number of PCM samples from last Ogg packet header }
    j:=SizeOf(buf)-10;
    repeat
      if pOGGHdr(@buf[j])^.ID=OGGSign then
      begin
        if j>(SizeOf(buf)-SizeOf(tOGGHdr)) then
        begin
          Seek(f,DataIndex+j);
          BlockRead(f,buf,SizeOf(tOGGHdr));
          j:=0;
        end;
        if Info.fps>0 then
        begin
          Info.total:=(pOGGHdr(@buf[j])^.Granule*100) div Info.fps;
        end
        else if Info.khz<>0 then
          Info.total:=pOGGHdr(@buf[j])^.Granule div Info.khz;
        break;
      end;
      dec(j);
    until j=0;
    if Info.total>0 then break;
  end;
  Info.khz:=Info.khz div 1000;
  CloseHandle(f);
end;

function ReadfLaC(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  data64:int64;
  hdr:tMetaHdr;
  frm:tStreamInfo;
  id:dword;
  flag:integer;
  size:dword;
  buf,ptr:PAnsiChar;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);
  BlockRead(f,id,SizeOf(id));
  if id=fLaCSign then
  begin
    flag:=0;
    repeat
      BlockRead(f,hdr,SizeOf(hdr));
      size:=hdr.blocklen[2]+(hdr.blocklen[1] shl 8)+(hdr.blocklen[0] shl 16);
      case (hdr.blocktype and $7F) of
        0: begin
          if flag=0 then
          begin
            BlockRead(f,frm,SizeOf(frm));
          //samplerate eg.44100
            Info.khz:=((frm.heap[0] shl 12)+(frm.heap[1] shl 4)+(frm.heap[2] shr 4));
            Info.channels:=((frm.heap[2] and $F) shr 1)+1;
          //bits per SAMPLE now
            Info.kbps:=(frm.heap[2] and 1) shl 4+(frm.heap[3] shr 4)+1;
            data64:=((frm.heap[3] and $F) shl 32)+(frm.heap[4] shl 24)+
                     (frm.heap[5] shl 16)+(frm.heap[6] shl 8)+frm.heap[7];

            if (data64<>0) and (Info.khz<>0) then
              Info.total:=data64 div Info.khz;
            Info.kbps:=Info.kbps*8;
Info.kbps:=trunc(FileSize(f)*8/1000);
            Info.khz:=Info.khz div 1000;
            flag:=1;
          end;
        end;
        4: begin
          GetMem(buf,size);
          BlockRead(f,buf^,size);
          OGGGetComment(buf,size,Info);
          FreeMem(buf);
        end;
        6: begin
          if Info.cover=nil then
          begin
            GetMem(buf,size);
            BlockRead(f,buf^,size);
            ptr:=buf;
            id:=BSwap(pdword(ptr)^);
            case id of
              0,3,4,6: begin
                inc(ptr,4);
                id:=BSwap(pdword(ptr)^); // mime size
                inc(ptr,4);
                flag:=GetImageType(nil,ptr);
                inc(ptr,id+4*5); // width, height, depth etc.
                id:=BSwap(pdword(ptr)^); // image size
                inc(ptr,4);
                if flag=0 then
                  flag:=GetImageType(pByte(ptr));
                FastAnsiToWideBuf(PAnsiChar(@flag),pWideChar(@data64));
                Info.cover:=SaveTemporaryW(ptr,id,PWideChar(@data64));
              end;
            end;
            FreeMem(buf);
          end
          else
            Skip(f,size);
        end
        else
        begin
          if (hdr.blocktype and $80)<>0 then
            break;
          Skip(f,size);
        end;
      end;
    until (hdr.blocktype and $80)<>0;
  end;
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLinkOGG,
  LocalFormatLinkOGA,
  LocalFormatLinkOGM,
  LocalFormatLinkSPX,
  LocalFormatLinkFLA,
  LocalFormatLinkFLAC:twFormat;

procedure InitLink;
begin
  LocalFormatLinkOGG.Next:=FormatLink;

  LocalFormatLinkOGG.This.proc :=@ReadOGG;
  LocalFormatLinkOGG.This.ext  :='OGG';
  LocalFormatLinkOGG.This.flags:=0;

  FormatLink:=@LocalFormatLinkOGG;

  LocalFormatLinkOGA.Next:=FormatLink;

  LocalFormatLinkOGA.This.proc :=@ReadOGG;
  LocalFormatLinkOGA.This.ext  :='OGA';
  LocalFormatLinkOGA.This.flags:=0;

  FormatLink:=@LocalFormatLinkOGA;

  LocalFormatLinkOGM.Next:=FormatLink;

  LocalFormatLinkOGM.This.proc :=@ReadOGG;
  LocalFormatLinkOGM.This.ext  :='OGM';
  LocalFormatLinkOGM.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkOGM;

  LocalFormatLinkSPX.Next:=FormatLink;

  LocalFormatLinkSPX.This.proc :=@ReadSPX;
  LocalFormatLinkSPX.This.ext  :='SPX';
  LocalFormatLinkSPX.This.flags:=0;

  FormatLink:=@LocalFormatLinkSPX;

  LocalFormatLinkFLA.Next:=FormatLink;

  LocalFormatLinkFLA.This.proc :=@ReadfLaC;
  LocalFormatLinkFLA.This.ext  :='FLA';
  LocalFormatLinkFLA.This.flags:=0;

  FormatLink:=@LocalFormatLinkFLA;

  LocalFormatLinkFLAC.Next:=FormatLink;

  LocalFormatLinkFLAC.This.proc :=@ReadfLaC;
  LocalFormatLinkFLAC.This.ext  :='FLAC';
  LocalFormatLinkFLAC.This.flags:=0;

  FormatLink:=@LocalFormatLinkFLAC;
end;

initialization
  InitLink;
end.
