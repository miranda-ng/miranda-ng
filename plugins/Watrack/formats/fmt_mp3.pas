{MP3 file process}
unit fmt_MP3;
{$include compilers.inc}

interface
uses wat_api;

function ReadMP3(var Info:tSongInfo):boolean; cdecl;
function ReadMPG(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

const
  ScanSize = 16*1024; // block size to search header
type
  tMP3FrameHdr = record
    Version   :integer;
    Layer     :cardinal;
    Bitrate   :cardinal;
    Samplerate:cardinal;
    Channel   :cardinal; //Stereo, Joint, Dual, Mono
    Length    :cardinal;
    CRC       :boolean;
    _Private  :boolean;
    Copyright :boolean;
    Original  :boolean;
    isVBR     :boolean;
  end;

// ........ ........ 111..... 11111111 syncword
// ........ ........ ...xx... ........ version (11=1, 10=2, 00=2.5)
// ........ ........ .....xx. ........ layer (01=III, 10=II, 11=I)
// ........ ........ .......x ........ crc (0=yes, 1=no)
// xx...... ........ ........ ........ mode (00=stereo, 10=joint, 01=dual, 11=mono)
// ..xx.... ........ ........ ........ mode ext (only for joint stereo)
// ....x... ........ ........ ........ copyright (0=no, 1=yes)
// .....x.. ........ ........ ........ original (0=orig, 1=copy)
// ......xx ........ ........ ........ emphasis (not 10)
// ........ xxxx.... ........ ........ bitrate (not 0000 nor 1111)
// ........ ....xx.. ........ ........ sampling rate (not 11)
// ........ ......x. ........ ........ padded (0=no, 1=yes)
// ........ .......x ........ ........ private bit

const
  btable:array [0..1,0..2,0..15] of word = (
    (    //MPEG 2 & 2.5
      (0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0), //Layer III
      (0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0), //Layer II
      (0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,0)  //Layer I
    ),(  //MPEG 1
      (0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,0), //Layer III
      (0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,0), //Layer II
      (0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,0)  //Layer I
    )
  );
  stable: array [0..3,0..2] of word = (
    (32000, 16000,  8000), //MPEG 2.5
    (    0,     0,     0), //reserved
    (22050, 24000, 16000), //MPEG 2
    (44100, 48000, 32000)  //MPEG 1
  );

procedure TranslateFrameHdr(const block:array of byte;var MP3FrameHdr:tMP3FrameHdr);
begin
  FillChar(MP3FrameHdr,SizeOf(MP3FrameHdr),0);
  if block[0]=$FF then
  begin
    with MP3FrameHdr do
    begin
      Version   :=(block[1] and $18) shr 3;
      Layer     :=(block[1] and $06) shr 1;
      CRC       :=not Odd(block[1]);
      Bitrate   :=btable[Version and 1][Layer-1][block[2] shr 4];
      Samplerate:=stable[Version][(block[2] and $0C) shr 2];
      _Private  :=odd(block[2]);
      Channel   :=block[3] shr 6;
      Copyright :=((block[3] and $08) shr 3)<>0;
      Original  :=((block[3] and $04) shr 2)<>0;
    end;
  end;
end;

procedure CheckVBR(f:THANDLE; var hdr:tMP3FrameHdr);
var
  pos,apos:cardinal;
  sign:longint;
  frames:longint;
begin
  pos:=FilePos(f);
  hdr.Length:=0;
  if hdr.Version=3 then
  begin
    if hdr.Channel=3 then
      apos:=17
    else
      apos:=32;
  end
  else if hdr.Channel=3 then
    apos:=9
  else
    apos:=17;
  Skip(f,apos);
  BlockRead(f,sign,4);
  hdr.isVBR:=sign=$676E6958; //Xing
//calculate length
  if hdr.isVBR then
  begin
    if hdr.Samplerate<>0 then
    begin
//      Seek(f,pos+36);
      BlockRead(f,sign,4);
      if (sign and $01000000)<>0 then
      begin
        BlockRead(f,frames,4);
        frames:=BSwap(frames);
        hdr.Length:=Round((1152/hdr.Samplerate)*frames/(4-hdr.Version)); //!
      end;
    end;
  end
  else if hdr.Bitrate<>0 then
    hdr.Length:=((8*(FileSize(f)-(pos-4))) div 1000) div hdr.Bitrate;
end;

function SearchStart(f:THANDLE; var l:array of byte):Boolean;
var
  CurPos:longint;
  buf:array [0..ScanSize] of byte;
  i,j:integer;
begin
  CurPos:=FilePos(f)-4;
  Seek(f,CurPos);
  j:=BlockRead(f,buf,ScanSize);
  i:=0;
  while i<j do
  begin
    if (i<(j-2)) and (buf[i]=$FF) and //FF FB E4
       ((buf[i+1] and $E0)=$E0) and
       ((buf[i+2] and $F0)<>$F0) then
    begin
      Seek(f,CurPos+i);
      BlockRead(f,l,4);
      result:=true;
      Exit;
    end;
    inc(i);
  end;
  result:=false;
end;

function ReadMP3(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  l:array [0..3] of byte;
  hdr:tMP3FrameHdr;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  ReadID3v2(f,Info);
  BlockRead(f,l,4);
//  if l[0]<>$FF then
  if not SearchStart(f,l) then
  begin
    CloseHandle(f);
    Exit;
  end;
  TranslateFrameHdr(l,hdr);
  CheckVBR(f,hdr);
  Info.kbps :=hdr.Bitrate;
  Info.khz  :=hdr.Samplerate div 1000;
  Info.total:=hdr.Length;
  if hdr.Channel=3 then
    Info.channels:=1
  else
    Info.channels:=2;
  Info.vbr:=ord(hdr.isVBR);

  ReadAPEv2(f,Info);
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

const
  mpgAudio   = 1;
  mpgVideo   = 2;
  mpgVersion = 4;

type
  l2b=array [0..3] of byte;

function ReadDWord(var p:pAnsiChar;endptr:pAnsiChar):integer;
begin
  if (p+4)<endptr then
  begin
    result:=pDWord(p)^;
    inc(p,4);
  end
  else
    result:=-1;
end;

function ReadWord(var p:pAnsiChar;endptr:pAnsiChar):integer;
begin
  if (p+2)<endptr then
  begin
    result:=pWord(p)^;
    inc(p,2);
  end
  else
    result:=-1;
end;

function ReadByte(var p:pAnsiChar;endptr:pAnsiChar):integer;
begin
  if p<endptr then
  begin
    result:=pByte(p)^;
    inc(p);
  end
  else
    result:=-1;
end;

function ChunkRead(var p:pAnsiChar;endptr:pAnsiChar):integer;
var
  i:integer;
begin
  repeat
    if ReadByte(p,endptr)=0 then
      if ReadByte(p,endptr)=0 then
      begin
        repeat
          i:=ReadByte(p,endptr);
        until i<>0;
        if i=1 then
        begin
          result:=ReadByte(p,endptr) or $100;
          exit;
        end;
      end;
  until p>=endptr;
  result:=0;
end;

const
  BufSize = 256*1024;

function ReadMPG(var Info:tSongInfo):boolean; cdecl;
var
  endptr,buf,p:PAnsiChar;
  f:THANDLE;
  BlockType:integer;
  l:dword;
  w:word;
  b:byte;
  flag:integer;
  version,Layer:integer;
//  vbitrate:integer;
//  FrmCnt:integer;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  flag:=mpgAudio+mpgVideo+mpgVersion;

  mGetMem(buf,BufSize);
  endptr:=buf+BlockRead(f,buf^,BufSize);
  CloseHandle(f);
  p:=buf;
//  FrmCnt:=0;
  while (flag<>0) and (p<endptr) do
  begin
    BlockType:=ChunkRead(p,endptr);
    case BlockType of
      $1BA: begin // pack
        if (flag and mpgVersion)<>0 then
        begin
          flag:=flag and not mpgVersion;
          if (ReadByte(p,endptr) and $C0)=$40 then
            Info.codec:=$3247504D  // MPG2
          else
            Info.codec:=$4745504D; // MPEG
        end;
      end;
      $1B3: begin // Video
        if (flag and mpgVideo)<>0 then
        begin
          l:=ReadDWord(p,endptr);
          flag:=flag and not mpgVideo;
          Info.width :=((l2b(l)[1] and $F0) shr 4)+(l2b(l)[0] shl 4);
          Info.height:=((l2b(l)[1] and $0F) shl 8)+l2b(l)[2];
          case l2b(l)[3] and $F of
            1: Info.fps:=2397;
            2: Info.fps:=2400;
            3: Info.fps:=2500;
            4: Info.fps:=2997;
            5: Info.fps:=3000;
            6: Info.fps:=5000;
            7: Info.fps:=5994;
            8: Info.fps:=6000;
          end;
//          BlockRead(f,l,4);
//          vbitrate:=(l2b(l)[0] shl 10)+(l2b(l)[1] shl 2)+(l2b(l)[2] shr 6);
        end;
      end;
      0,$1B7,$1B9: break;
{
      $1E0: begin
        BlockRead(f,w,2);
        w:=swap(w);
         mGetMem(buf,w);
        BlockRead(f,buf^,w);
        p:=buf;
        for l:=0 to w-4 do
        begin
          if pdword(p)^=$00010000 then
          begin
            inc(FrmCnt);
            inc(p,4);
          end
          else
            inc(p);
        end;
        mFreeMem(buf);
      end;
}
      $1C0: begin // audio
        w:=swap(ReadWord(p,endptr));
        if flag and mpgAudio<>0 then
        begin
          flag:=flag and not mpgAudio;
          b:=ReadByte(p,endptr);
          dec(w);
          if (b and $C0)=$80 then
          begin
            b:=ReadByte(p,endptr);
            l:=ReadByte(p,endptr);
            dec(w,2);
            if (b and $80)<>0 then
            begin
              inc(p,5);
              dec(w,5);
              dec(l,5);
              if (b and $40)<>0 then
              begin
                inc(p,5);
                dec(w,5);
                dec(l,5);
              end;
            end;
            if l>0 then
            begin
              inc(p,l);
              dec(w,l);
            end;
          end
          else
          begin
            while (b and $80)<>0 do
            begin
              dec(w);
              if w=0 then break;
              b:=ReadByte(p,endptr);
            end;
            if (b and $40)<>0 then
            begin
              inc(p);
              b:=ReadByte(p,endptr);
              dec(w,2);
            end;
            if (b and $20)<>0 then
            begin
              inc(p,4);
              dec(w,4);
              if (b and $10)<>0 then
              begin
                inc(p,5);
                dec(w,5);
              end;
            end;
          end;
          l:=ReadDWord(p,endptr);
          version:=(l2b(l)[1] and $18) shr 3;
          Layer  :=(l2b(l)[1] and $06) shr 1;
          Info.kbps    :=btable[version and 1][Layer-1][l2b(l)[2] shr 4];
          Info.khz     :=(stable[version][(l2b(l)[2] and $0C) shr 2]) div 1000;
          Info.channels:=l2b(l)[3] shr 6;
          if Info.channels=3 then
            Info.channels:=1
          else
            Info.channels:=2;
//          if w>0 then inc(p,w);
        end;
//        else
          inc(p,w);
      end;
{
      $1B5: begin
        BlockRead(f,l,4);
        if (l2b(l)[0] and $F0)=$10 then
        begin
          vbitrate:=vbitrate+
              ((((l2b(l)[2] and $1F) shl 7)+(l2b(l)[3] shr 1)) shl 18);
        end;
      end;
}
{
      $1BD: begin
      end;
}
      $1C1..$1DF, // audio
//??      $1E0,
      $1E1..$1EF, // video
      $1BB{,$1BD},$1BE,$1BF: begin // system,private,padding,private
        inc(p,swap(ReadWord(p,endptr)));
      end;
    end;
  end;
//  vbitrate:=(vbitrate*400) div 1000;
//  Info.total:=(FrmCnt*100) div Info.fps;
  mFreeMem(buf);
  result:=true;
end;

var
  LocalFormatLinkMP3,
  LocalFormatLinkMPG,
  LocalFormatLinkMPEG:twFormat;

procedure InitLink;
begin
  LocalFormatLinkMP3.Next:=FormatLink;

  LocalFormatLinkMP3.This.proc :=@ReadMP3;
  LocalFormatLinkMP3.This.ext  :='MP3';
  LocalFormatLinkMP3.This.flags:=0;

  FormatLink:=@LocalFormatLinkMP3;

  LocalFormatLinkMPG.Next:=FormatLink;

  LocalFormatLinkMPG.This.proc :=@ReadMPG;
  LocalFormatLinkMPG.This.ext  :='MPG';
  LocalFormatLinkMPG.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkMPG;

  LocalFormatLinkMPEG.Next:=FormatLink;

  LocalFormatLinkMPEG.This.proc :=@ReadMPG;
  LocalFormatLinkMPEG.This.ext  :='MPEG';
  LocalFormatLinkMPEG.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkMPEG;
end;

initialization
  InitLink;
end.
