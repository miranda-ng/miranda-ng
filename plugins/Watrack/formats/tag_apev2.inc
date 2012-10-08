{APE tag}
{$IFDEF Interface}
function ReadAPEv2(buf:PAnsiChar;var Info:tSongInfo;count:integer=0):longint; overload;
function ReadAPEv2(f:THANDLE;var Info:tSongInfo):longint; overload;
{$ELSE}
const
  APESign  = 'APETAGEX';
type
  pAPEHeader = ^tAPEHeader;
  tAPEHeader = packed record
    ID:array [0..7] of AnsiChar;
    Version:dword;
    TagSize:dword; //footer + all items
    ItemCount:dword;
    TagFlags:dword;
    Reserved:array [0..7] of byte;
  end;

procedure ReadAPEValue(const buf:PAnsiChar;var dst:pWideChar;ver:dword);
begin
  if dst=nil then
    if ver>1000 then
      UTF8ToWide(buf,dst)
    else
      AnsiToWide(buf,dst);
end;

function ReadAPEv2(buf:PAnsiChar;var Info:tSongInfo;count:integer=0):longint;
var
  APE:pAPEHeader;
  len:integer;
  ptr,key:PAnsiChar;
  flag:dword;
  cf:THANDLE;
  buf0,buf1:array [0..MAX_PATH-1] of AnsiChar;
  b:AnsiChar;
//  extw:array [0..7] of WideChar;
begin
  result:=0;
  APE:=pointer(buf);
  if APE.ID=APESign then
  begin
    inc(buf,SizeOf(tAPEHeader));
    count:=APE.ItemCount;
  end;
  while count>0 do
  begin
    len :=pdword(buf)^; inc(buf,4);
    flag:=pdword(buf)^; inc(buf,4);
    key:=buf;
    while buf^<>#0 do inc(buf); inc(buf);

    ptr:=buf+len;
    b:=ptr^;
    ptr^:=#0;
    if      lstrcmpia(key,'TITLE'  )=0 then ReadAPEValue(buf,Info.title  ,APE.Version)
    else if lstrcmpia(key,'ARTIST' )=0 then ReadAPEValue(buf,Info.artist ,APE.Version)
    else if lstrcmpia(key,'ALBUM'  )=0 then ReadAPEValue(buf,Info.album  ,APE.Version)
    else if lstrcmpia(key,'COMMENT')=0 then ReadAPEValue(buf,Info.comment,APE.Version)
    else if lstrcmpia(key,'GENRE'  )=0 then ReadAPEValue(buf,Info.genre  ,APE.Version)
    else if lstrcmpia(key,'YEAR'   )=0 then ReadAPEValue(buf,Info.year   ,APE.Version)
    else if lstrcmpia(key,'TRACK'  )=0 then if Info.track=0 then Info.track:=StrToInt(buf)
    else if lstrcmpia(key,'LYRICS' )=0 then ReadAPEValue(buf,Info.lyric  ,APE.Version)
    //!! must preserve multipart lyric
    else if (lstrcmpia(key,'Cover Art (Front)')=0) or
            (lstrcmpia(key,'Cover Art (Back)' )=0) or
            (lstrcmpia(key,'APIC'             )=0) then
    begin
      if Info.cover=nil then
      begin
        while buf^<>#0 do inc(buf); inc(buf); // point to data now
        flag:=GetImageType(pByte(buf));
        if flag<>0 then
        begin
{
    FastAnsiToWideBuf(PAnsiChar(@flag),pWideChar(@extw));
    Info.Cover:=SaveTemporaryW(buf,ptr-buf,PWideChar(@extw));
}
          GetTempPathA(SizeOf(buf0),buf0);
          GetTempFileNameA(buf0,'wat',GetCurrentTime,buf1);
          ChangeExt(buf1,PAnsiChar(@flag));

          cf:=ReWrite(PAnsiChar(@buf1));
          BlockWrite(cf,buf^,ptr-buf);
          CloseHandle(cf);
          AnsiToWide(PAnsiChar(@buf1),Info.cover);
        end;
      end;
    end;
    ptr^:=b;
    buf:=ptr;
    dec(count);
  end;
end;

function ReadAPEv2(f:THANDLE;var Info:tSongInfo):longint;
var
  APE:tAPEHeader;
  buf:PAnsiChar;
  fpos:dword;
  TagID:array [1..3] of AnsiChar;
begin
  result:=0;
  fpos:=FileSize(f);
  Seek(f,fpos-SizeOf(TID3v1Tag));
  BlockRead(f,TagID,3);
  if TagID=TAG1Sign then
    dec(fpos,SizeOf(TID3v1Tag));
  Seek(f,fpos-SizeOf(APE));
  BlockRead(f,APE,SizeOf(APE));
  // footer must be  copied as header
  if APE.ID=APESign then
  begin
    if (APE.TagFlags and $20000000)=0 then //Footer
    begin
      Seek(f,fpos-APE.TagSize{-SizeOf(APE)});// without header but with footer
      GetMem(buf,APE.TagSize);
      BlockRead(f,buf^,APE.TagSize);
      result:=ReadAPEv2(buf,Info,APE.ItemCount);
      FreeMem(buf);
    end;
  end;
end;
{$ENDIF}
