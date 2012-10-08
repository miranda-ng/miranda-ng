{FLV file format}
unit fmt_FLV;
{$include compilers.inc}

interface
uses wat_api;

function ReadFLV(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,srv_format;

type
  tFLVHeader = packed record
    Signature:array [0..2] of AnsiChar; // FLV
    Version  :byte;
    flags    :byte;
    Offset   :dword; // reversed byte order
  end;
type
  tFLVStream = packed record
    PreviousTagSize  :dword;
    TagType          :byte;
    BodyLength       :array [0..2] of byte;
    Timestamp        :array [0..2] of byte;
    TimestampExtended:byte;
    StreamID         :array [0..2] of byte;
//    Body
  end;
{
  twork = record
    endptr:PAnsiChar;
    Info  :pSongInfo;
    key   :PAnsiChar;
    len   :cardinal;
  end;
}
//  FLVTagTypes
const
  FLV_AUDIO = 8;
  FLV_VIDEO = 9;
  FLV_META  = 18;

const
  BufSize = 128*1024;

type
  pArr = ^tArr;
  tArr = array [0..7] of byte;

  transform=packed record
    case byte of
      0: (txt:array [0..3] of AnsiChar);
      1: (num:dword);
  end;
  trecode=packed record
    case byte of
      0: (i:int64);
      1: (d:double);
  end;

function Reverse(buf:int64;len:integer):int64;
var
  i:integer;
begin
  result:=0;
  for i:=0 to len-1 do
    result:=(result shl 8)+tArr(buf)[i];
end;

function ProcessValue(var ptr:PAnsiChar;key:pAnsiChar;var Info:tSongInfo):integer;
var
  tmp:int64;
  i,len:integer;
  recode:trecode;
  code:integer;
  codec:transform;
  value:array [0..63] of AnsiChar;
begin
  result:=1;
  code:=ord(ptr^); // type of value
  ptr^:=#0;        // finalization for key name
  inc(ptr);        // value data pointer

  case code of // v10.1 specification

    // Numeric, Double (8 bytes)
    0: begin
      move(ptr^,tmp,8);
      recode.i:=Reverse(tmp,8);
      i:=trunc(recode.d);
      if      StrCmp(key,'duration'     )=0 then Info.total :=i
      else if StrCmp(key,'totalduration')=0 then Info.total :=i
      else if StrCmp(key,'width'        )=0 then Info.width :=i
      else if StrCmp(key,'height'       )=0 then Info.height:=i
      else if StrCmp(key,'audiodatarate')=0 then Info.kbps  :=i
      else if StrCmp(key,'framerate'    )=0 then Info.fps   :=trunc(recode.d*100)
      else if StrCmp(key,'audiosize'    )=0 then
      begin
        if Info.kbps=0 then
          Info.kbps:=trunc((recode.d*8)/(Info.total*1000))
      end
      else if StrCmp(key,'videocodecid')=0 then
      begin
        case i of
          2:   codec.txt:='H263';
          3:   codec.txt:='Scrn';
          4,5: codec.txt:='VP6 ';
          6:   codec.txt:='Src2';
          7:   codec.txt:='AVC ';
        end;
        Info.codec:=codec.num;
      end;

      inc(ptr,8);
    end;
  
    // Boolean, UI8
    1: begin
      if StrCmp(key,'stereo')=0 then Info.channels:=ORD(ptr^)+1;

      inc(ptr);
    end;
    
    // String
    2: begin
      i:=Reverse(pWord(ptr)^,2); inc(ptr,2);

      if StrCmp(key,'creationdate')=0 then
      begin
        move(ptr^,value[0],i);
        value[i]:=#0;
        AnsiToWide(value,Info.year);
      end;

      inc(ptr,i);
    end;

    // Object
    3: begin
      repeat
        len:=Reverse(pWord(ptr)^,2); inc(ptr,2); // key name length
        key:=ptr; inc(ptr,len);                  // key name

        result:=ProcessValue(ptr,key,Info);
      until result<=0;

      if result<0 then
        result:=1;
    end;

    // Movie clip, reserved
    4: begin
    end;
  
    // NULL
    5: begin
    end;

    // Undefined
    6: begin
    end;
  
    // reference, UI16
    7: begin
      inc(ptr,2);
    end;
  
    // ECMA array
    8: begin
      i:=pdword(ptr)^; inc(ptr,4);
      i:=Reverse(i,4);
      while i>0 do
      begin

        len:=Reverse(pWord(ptr)^,2); inc(ptr,2); // key name length
        key:=ptr; inc(ptr,len);                  // key name

        result:=ProcessValue(ptr,key,Info);

        if result=0 then break
        else if result<0 then
        begin
          result:=1;
          break;
        end;
        dec(i);
      end;
    end;

    // Object end marker, UI8[3]=0,0,9
    9: begin
      result:=-1;
      inc(ptr,3);
    end;
  
    10: // array, 4 bytes - num of elements, elements
    begin
      i:=pdword(ptr)^; inc(ptr,4);
      i:=Reverse(i,4);
      while i>0 do
      begin
        result:=ProcessValue(ptr,nil,Info);
        if result=0 then exit
        else if result<0 then
        begin
          result:=1;
          break;
        end;
        dec(i);
      end;
    end;

    // Date, Double + UI16 (UTC)
    11: begin
      inc(ptr,8);
      inc(ptr,2);
    end;
  
    // LongString, 4 bytes = len, len - string
    12: begin
      i:=pdword(ptr)^; inc(ptr,4);
      i:=Reverse(i,4);

      inc(ptr,i);
    end;

  end;
end;

function ReadFLV(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  codec:transform;
  FLVHdr:tFLVHeader;
  StrmHdr:tFLVStream;
  i,len:integer;
  buf,pp,p,endbuf:PAnsiChar;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;

  mGetMem(buf,BufSize);
  endbuf:=buf+BlockRead(f,buf^,BufSize);
  p:=buf;
  CloseHandle(f);
  move(p^,FLVHdr,SizeOf(tFLVHeader));
  if (FLVHdr.Signature[0]='F') and (FLVHdr.Signature[1]='L') and
     (FLVHdr.Signature[2]='V') and (FLVHdr.Version=1) then
  begin
    inc(p,SizeOf(tFLVHeader));
    result:=true;
    while (p<endbuf) and ((FLVHdr.flags and 5)<>0) do
    begin
      move(p^,StrmHdr,SizeOf(tFLVStream));
      inc(p,SizeOf(tFLVStream));
      len:=(StrmHdr.BodyLength[0] shl 16)+(StrmHdr.BodyLength[1] shl 8)+
            StrmHdr.BodyLength[2];
      pp:=p;
      case StrmHdr.TagType of

        FLV_AUDIO: begin
          Info.channels:=(ord(p^) and 1)+1;
          // samplesize is (S_Byte and 2) shr 1 = 8 or 16
          case (ord(p^) and $C) shr 2 of
            0: Info.khz:=5;
            1: Info.khz:=11;
            2: Info.khz:=22;
            3: Info.khz:=44;
          end;
          FLVHdr.flags:=FLVHdr.flags and not 4;
        end;

        FLV_VIDEO: begin
          case ord(p^) and $0F of
            2:   codec.txt:='H263';
            3:   codec.txt:='Scrn';
            4,5: codec.txt:='VP6 ';
            6:   codec.txt:='Src2';
            7:   codec.txt:='AVC ';
          end;
          Info.codec:=codec.num;
          FLVHdr.flags:=FLVHdr.flags and not 1;
        end;

        FLV_META: begin
          if (StrmHdr.TagType and $40)=0 then // not encripted
          begin
            if pByte(p)^=2 then // string
            begin
              Inc(p);
              i:=Reverse(pWord(p)^,2); inc(p,2);
              if StrCmp(p,'onMetaData',i)=0 then // Metadata processing start
              begin
                inc(p,i);
                ProcessValue(p,nil,Info); // metadata, no need key name, our info
                // checking for video
                if Info.codec<>0 then
                  FLVHdr.flags:=FLVHdr.flags and not 1;
                // checking for audio
                if (Info.khz<>0) and (Info.channels<>0) then
                  FLVHdr.flags:=FLVHdr.flags and not 4;
                // break; // if metainfo is enough
              end;
            end;
          end;
        end;

      end;
      p:=pp+len;
    end;
  end;
  mFreeMem(buf);
end;

var
  LocalFormatLink:twFormat;

procedure InitLink;
begin
  LocalFormatLink.Next:=FormatLink;

  LocalFormatLink.This.proc :=@ReadFLV;
  LocalFormatLink.This.ext  :='FLV';
  LocalFormatLink.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLink;
end;

initialization
  InitLink;
end.
