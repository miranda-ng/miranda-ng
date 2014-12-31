{WMA file format}
unit fmt_WMA;
{$include compilers.inc}

interface
uses wat_api;

function ReadWMA(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,srv_format,utils;

const
  ASF_Header_Object                      :tGUID='{75B22630-668E-11CF-A6D9-00AA0062CE6C}';

  ASF_Header_Extension_Object            :tGUID='{5FBF03B5-A92E-11CF-8EE3-00C00C205365}';
  ASF_Content_Description_Object         :tGUID='{75B22633-668E-11CF-A6D9-00AA0062CE6C}';
  ASF_Extended_Content_Description_Object:tGUID='{D2D0A440-E307-11D2-97F0-00A0C95EA850}';
  ASF_File_Properties_Object             :tGUID='{8CABDCA1-A947-11CF-8EE4-00C00C205365}';
  ASF_Stream_Properties_Object           :tGUID='{B7DC0791-A9B7-11CF-8EE6-00C00C205365}';

  ASF_Metadata_Library_Object            :tGUID='{44231C94-9498-49D1-A141-1D134E457054}';
  ASF_Audio_Media                        :tGUID='{F8699E40-5B4D-11CF-A8FD-00805F5C442B}';
  ASF_Video_Media                        :tGUID='{BC19EFC0-5B4D-11CF-A8FD-00805F5C442B}';

type
  tSize=Int64;

function CompareGUID(const guid1,guid2:tGUID):boolean;
var
  i:integer;
  p1,p2:PAnsiChar;
begin
  p1:=PAnsiChar(@guid1);
  p2:=PAnsiChar(@guid2);
  for i:=0 to 15 do
  begin
    if p1^<>p2^ then
    begin
      result:=false;
      exit;
    end;
    inc(p1);
    inc(p2);
  end;
  result:=true;
end;

function ReadGUID(var buf:PAnsiChar; var guid:pGUID):dword;
var
  size:tSize;
begin
  guid:=pointer(buf);
  inc(buf,SizeOf(tGUID));
  move(buf^,size,SizeOf(size));
  inc(buf,SizeOf(size));
  result:=size-SizeOf(tGUID)-SizeOf(size);
end;

procedure ReadWMATagStr(var dst:pWideChar;ptr:PAnsiChar;alen:word);
begin
  if pword(ptr)^<>0 then
  begin
    mGetMem(dst,alen);
    move(pWideChar(ptr{+2})^,dst^,alen);
  end;
end;

function ReadWMATagStr1(var dst:pWideChar;var ptr:PAnsiChar;value:boolean=true):integer;
var
  len,typ:word;
begin
  if value then
  begin
    typ:=pword(ptr)^;
    inc(ptr,2); //value type
  end
  else
    typ:=0;
  len:=pword(ptr)^;
  result:=-1;
  dst:=nil;
  if len<>0 then
  begin
    if typ=0 then
    begin
      mGetMem(dst,len);
      move(PAnsiChar(ptr+2)^,PAnsiChar(dst)^,len);
    end
    else
    begin
      result:=pword(ptr+2)^;
      if typ<5 then
        result:=pword(ptr+4)^*$10000+result;
    end;
  end;
  inc(ptr,len+2);
end;

procedure ProcessPicture(ptr:PAnsiChar;var Info:tSongInfo);
var
  extw:int64;
  aSize:dword;
begin
  if Info.cover<>nil then exit;
  case ptr^ of
    #0,#3,#4,#6: ;
  else
    exit;
  end;
  inc(ptr);
  aSize:=pdword(ptr)^; inc(ptr,4);
  extw:=GetImageTypeW(nil,pWideChar(ptr));
  while pWideChar(ptr)^<>#0 do inc(ptr,2); inc(ptr,2); // mime
  while pWideChar(ptr)^<>#0 do inc(ptr,2); inc(ptr,2); // descr

  if extw=0 then
    extw:=GetImageTypeW(pByte(ptr));
  Info.cover:=SaveTemporaryW(ptr,aSize,pWideChar(@extw));
end;

procedure ReadHdrExtended(ptr:PAnsiChar;size:dword;var Info:tSongInfo);
var
  buf:PAnsiChar;
  ls:pWideChar;
  cnt,tmp:integer;
  tmpguid:pGUID;
  lsize:dword;
begin
  inc(ptr,SizeOf(tGUID)+2);
  size:=pdword(ptr)^; inc(ptr,4);
  while size>0 do
  begin
    if Info.cover<>nil then break;
    lsize:=ReadGUID(ptr,tmpguid);
    dec(size,lsize+SizeOf(tGUID)+SizeOf(tSize));
    if CompareGUID(tmpguid^,ASF_Metadata_Library_Object) then
    begin
      buf:=ptr;
      cnt:=pdword(buf)^; inc(buf,2);
      while cnt>0 do
      begin
        inc(buf,4); // lang & stream
        {tmp:=pword (buf)^;} inc(buf,2); // namelen
        {tmp:=pword (buf)^;} inc(buf,2); // datatype
        tmp:=pdword(buf)^; inc(buf,4); // datalen
        ls:=PWideChar(buf);
        while pWideChar(buf)^<>#0 do inc(buf,2); inc(buf,2);
        if lstrcmpiw(ls,'WM/Picture')=0 then
        begin
          ProcessPicture(buf,Info);
          inc(buf,tmp);
        end;
        dec(cnt);
      end;
    end;
    inc(ptr,lsize);
  end;
end;

procedure ReadExtended(ptr:PAnsiChar;size:dword;var Info:tSongInfo);
var
  ls,ls1,ls2:pWideChar;
  cnt,tmp:integer;
begin
  cnt:=pword(ptr)^; inc(ptr,2);
  while cnt>0 do
  begin
    dec(cnt);
    ReadWMATagStr1(ls,ptr,false);
    if lstrcmpiw(ls,'WM/AlbumTitle')=0 then
      ReadWMATagStr1(Info.album,ptr)
    else if (Info.lyric=nil) and (lstrcmpiw(ls,'WM/Lyrics')=0) then
      ReadWMATagStr1(Info.lyric,ptr)
    else if (Info.lyric=nil) and (lstrcmpiw(ls,'WM/Lyrics_Synchronised')=0) then
    begin
      inc(ptr,2+2);
      inc(ptr); // timestamp type
      if ptr^=#1 then // lyric
      begin
        inc(ptr);
        tmp:=pdword(ptr)^; inc(ptr,4);
        mGetMem(ls2,tmp);
        Info.lyric:=ls2;
        ls1:=pWideChar(ptr);
        inc(ptr,tmp);
        while ls1^<>#0 do // description
        begin
          inc(ls1);
          dec(tmp,SizeOf(WideChar));
        end;
        inc(ls1);
        dec(tmp,SizeOf(WideChar));
        while tmp>0 do
        begin
          if PAnsiChar(ls1)^=#$0A then
          begin
            inc(PAnsiChar(ls1));
            ls2^:=#$0A;
            dec(tmp);
            inc(ls2);
          end;
          while ls1^<>#0 do
          begin
            ls2^:=ls1^; inc(ls2); inc(ls1);
            dec(tmp,SizeOf(WideChar));
          end;
          inc(ls1,1+2); // terminator + timestamp
          dec(tmp,SizeOf(WideChar)+4);
        end;
        ls2^:=#0;
//        ptr:=PAnsiChar(ls1);
      end
    end
    else if lstrcmpiw(ls,'WM/Genre')=0 then
      ReadWMATagStr1(Info.genre,ptr)
    else if lstrcmpiw(ls,'WM/Year')=0 then
    begin
      tmp:=ReadWMATagStr1(Info.year,ptr);
      if tmp<>-1 then
        IntToStr(Info.year,tmp);
    end
    else if lstrcmpiw(ls,'WM/Track')=0 then
    begin
      tmp:=ReadWMATagStr1(ls1,ptr);
      if tmp=-1 then
      begin
        Info.track:=StrToInt(ls1)+1;
        mFreeMem(ls1);
      end
      else
        Info.track:=tmp;
    end
    else if lstrcmpiw(ls,'WM/TrackNumber')=0 then
    begin
      tmp:=ReadWMATagStr1(ls1,ptr);
      if tmp=-1 then
      begin
        Info.track:=StrToInt(ls1);
        mFreeMem(ls1);
      end
      else
        Info.track:=tmp;
    end
    else if lstrcmpiw(ls,'WM/Picture')=0 then
    begin
      inc(ptr,2); // data type
      tmp:=pword(ptr)^; inc(ptr,2);
      ProcessPicture(ptr,Info);
      inc(ptr,tmp);
    end
    else
      inc(ptr,4+pword(ptr+2)^);
    mFreeMem(ls);
  end;
end;

procedure ReadFileProp(ptr:PAnsiChar;var Info:tSongInfo);
type
  pFileProp = ^tFileProp;
  tFileProp = packed record
    FileGUID  :tGUID;
    FileSize  :tSize;
    Creation  :tSize;
    Packets   :tSize;
    Play      :tSize;
    Send      :tSize;
    PreRoll   :tSize;
    Flags     :dword;
    minpacket :dword;
    maxpacket :dword;
    maxbitrate:dword;
  end;
begin
  Info.total:=pFileProp(ptr)^.Play div 10000000;
end;

procedure ReadStreamProp(ptr:PAnsiChar;size:dword;var Info:tSongInfo);
type
  pAudio = ^tAudio;
  tAudio=packed record // WAVEFORMATEX
    Codec        :word;
    Channels     :word;
    Samples      :dword;
    AvgBPS       :dword;
    BlockAlign   :word;
    BitsPerSample:word;
    size         :word;
  end;
  pVideo = ^tVideo;
  tVideo = packed record
    width   :dword;
    height  :dword;
    reserved:byte;
    size    :word;
    bitmap  :BITMAPINFOHEADER;
  end;
  tPrefix = packed record
    StreamType  :tGUID;
    ECGUID      :tGUID; // Error Correction
    TimeOffset  :int64;
    DataLength  :dword;
    ECDataLength:dword;
    Flags       :word;
    Reserved    :dword;
  end;

var
  tmpguid:pGUID;
begin
  tmpguid:=pointer(ptr);
  inc(ptr,SizeOf(tPrefix)); //ofset to Type-Specific Data
  if CompareGUID(tmpguid^,ASF_Audio_Media) then
  begin
    Info.channels:=pAudio(ptr)^.Channels;
    Info.khz     :=pAudio(ptr)^.Samples div 1000;
    Info.kbps    :=(pAudio(ptr)^.AvgBPS*8) div 1000;
  end
  else if CompareGUID(tmpguid^,ASF_Video_Media) then
  begin
    Info.width :=pVideo(ptr)^.bitmap.biWidth;  // pVideo(ptr)^.width
    Info.height:=pVideo(ptr)^.bitmap.biHeight; // pVideo(ptr)^.height
    Info.codec :=pVideo(ptr)^.bitmap.biCompression;
  end
end;

procedure ReadContent(ptr:PAnsiChar;var Info:tSongInfo);
type
  pContent = ^tContent;
  tContent = packed record
    TitleLength      :word;
    AuthorLength     :word;
    CopyrightLength  :word;
    DescriptionLength:word;
    RatingLength     :word;
  end;
var
  cont:pContent;
begin
  cont:=pointer(ptr);
  inc(ptr,SizeOf(tContent));
  if cont^.TitleLength>0 then //title
  begin
    ReadWMATagStr(Info.title,ptr,cont^.TitleLength);
    inc(ptr,cont^.TitleLength);
  end;
  if cont^.AuthorLength>0 then //artist
  begin
    ReadWMATagStr(Info.artist,ptr,cont^.AuthorLength);
    inc(ptr,cont^.AuthorLength);
  end;
  inc(ptr,cont^.CopyrightLength); //copyright
  if cont^.DescriptionLength>0 then //comment
    ReadWMATagStr(Info.comment,ptr,cont^.DescriptionLength);
end;

function ReadWMA(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  tmpguid:pGUID;
  size:int64;
  buf1,buf2:PAnsiChar;
  HdrObjects:dword;
  base:tGUID;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;

  BlockRead(f,base,SizeOf(tGUID));
  if CompareGUID(base,ASF_Header_Object) then
  begin
    BlockRead(f,size,SizeOf(size));
    dec(size,SizeOf(tGUID)+SizeOf(size));

    GetMem(buf1,size);
    buf2:=buf1;
    BlockRead(f,buf1^,size);
    HdrObjects:=pdword(buf2)^; inc(buf2,6);
    while HdrObjects>0 do
    begin
      size:=ReadGUID(buf2,tmpguid);
      if CompareGUID(tmpguid^,ASF_Content_Description_Object) then
        ReadContent(buf2,Info)
      else if CompareGUID(tmpguid^,ASF_Extended_Content_Description_Object) then
        ReadExtended(buf2,size,Info)
      else if CompareGUID(tmpguid^,ASF_Header_Extension_Object) then
        ReadHdrExtended(buf2,size,Info)
      else if CompareGUID(tmpguid^,ASF_File_Properties_Object) then
        ReadFileProp(buf2,Info)
      else if CompareGUID(tmpguid^,ASF_Stream_Properties_Object) then
        ReadStreamProp(buf2,size,Info);
      inc(buf2,size);
      dec(HdrObjects);
    end;
    FreeMem(buf1);

    result:=true;
  end;
  CloseHandle(f);
end;

var
  LocalFormatLinkWMA,
  LocalFormatLinkWMV,
  LocalFormatLinkASF:twFormat;

procedure InitLink;
begin
  LocalFormatLinkWMA.Next:=FormatLink;

  LocalFormatLinkWMA.This.proc :=@ReadWMA;
  LocalFormatLinkWMA.This.ext  :='WMA';
  LocalFormatLinkWMA.This.flags:=0;

  FormatLink:=@LocalFormatLinkWMA;

  LocalFormatLinkWMV.Next:=FormatLink;

  LocalFormatLinkWMV.This.proc :=@ReadWMA;
  LocalFormatLinkWMV.This.ext  :='WMV';
  LocalFormatLinkWMV.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkWMV;

  LocalFormatLinkASF.Next:=FormatLink;

  LocalFormatLinkASF.This.proc :=@ReadWMA;
  LocalFormatLinkASF.This.ext  :='ASF';
  LocalFormatLinkASF.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkASF;
end;

initialization
  InitLink;
end.
