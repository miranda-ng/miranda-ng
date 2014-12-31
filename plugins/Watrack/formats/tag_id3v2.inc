{ID3v2 tag}

{$IFDEF Interface}
function ReadID3v2(f:THANDLE; var Info:tSongInfo):longint;
{$ELSE}
const
  frmTRK = $4B5254;
  frmTT2 = $325454;
  frmTP1 = $315054;
  frmTAL = $4C4154;
  frmTYE = $455954;
  frmCOM = $4D4F43;
  frmTCO = $4F4354;
//  frmTCM = $;'; New: 'TCOM'),
//  frmTEN = $;'; New: 'TENC'),
//  frmTCR = $;'; New: 'TCOP'),
//  frmWXX = $;'; New: 'WXXX'),
  frmTT1 = $315454;
//  frmTLA = $;'; New: 'TLAN'),
  frmTOA = $414F54;
  frmULT = $544C55;
  frmSLT = $544C53;
  frmTXX = $585854;
  frmPIC = $434950;

  frmTIT1 = $31544954; // Content group description
  frmTIT2 = $32544954; // Title/songname/content description
  frmTIT3 = $33544954; // Subtitle/Description refinement
  frmTALB = $424C4154; // Album/Movie/Show title
  frmTOAL = $4C414F54; // Original album/movie/show title
  frmTRCK = $4B435254; // Track number/Position in set
  frmTYER = $52455954; // Year
  frmTDRC = $43524454; // Year
  frmTORY = $59524F54; // Original release year
  frmTPE1 = $31455054; // Lead performer(s)/Soloist(s)
  frmTPE2 = $32455054; // Band/orchestra/accompaniment
  frmTPE3 = $33455054; // Conductor/performer refinement
  frmTPE4 = $34455054; // Interpreted, remixed, or otherwise modified by
  frmTOPE = $45504F54; // Original artist(s)/performer(s)
  frmTCON = $4E4F4354; // Content type
  frmCOMM = $4D4D4F43; // Comments
  frmUSLT = $544C5355; // Unsynchronised lyrics
  frmSYLT = $544C5953; // Synchronised lyrics
  frmTXXX = $58585854; // User defined text
  frmAPIC = $43495041; // Attached picture
const
  TAG2Sign = 'ID3';
const
  ExtIDHdrMask=$40;
  FooterPresent=$10;
type
  TID3v2TagHdr = packed record
    ID     :array [0..2] of AnsiChar;
    Version:word;
    Flags  :byte;
    TagSize:dword;
  end;
  PID3v2TagHdr = ^TID3v2TagHdr;
type
  tID3v2FrameHdr = packed record
    ID:dword;
    Size:dword;
    Flags:word;
  end;
  pID3v2FrameHdr = ^tID3v2FrameHdr;
  tID3v2FrameHdrOld = packed record
    ID  : array [0..2] of byte; { Frame ID }
    Size: array [0..2] of byte; { Size excluding header }
  end;
  pID3v2FrameHdrOld = ^tID3v2FrameHdrOld;

var
  Unsync:boolean;

function ID3v2_Correct(data:dword):dword;
type
  l2b=packed record
    b:array [0..3] of byte;
  end;
begin
  result:=l2b(data).b[3];
  inc(result,dword(l2b(data).b[0]) shl 21);
  inc(result,dword(l2b(data).b[1]) shl 14);
  inc(result,dword(l2b(data).b[2]) shl 7);
end;

procedure ID3v2_ReadTagStr1(var dst:PWideChar;ptr:PAnsiChar;alen:integer;enc:integer);
var
  buf:PAnsiChar;
begin
  if (enc=0) or (enc=3) then // ANSI or UTF8
  begin
    if ptr^=#0 then
      alen:=0
    else
      while (alen>0) and (ptr[alen-1]=#0) do dec(alen);

    if alen>0 then
    begin
{
      if enc=0 then
      begin
        StrDup(buf,ptr,alen);
        AnsiToWide(buf,dst)
        mFreeMem(buf);
      end
      else
        UTF8ToWide(buf,dst,alen);
}
      StrDup(buf,ptr,alen);
      if enc=0 then
        AnsiToWide(buf,dst)
      else
        UTF8ToWide(buf,dst);
      mFreeMem(buf);
    end
  end
  else {if enc<3 then} //Unicode
  begin
    if pword(ptr)^>0 then
    begin
      alen:=alen div SizeOf(WideChar);

      StrDupW(dst,pWideChar(ptr),alen);
      ChangeUnicode(dst);
    end;
  end;
end;

procedure ID3v2_ReadTagStr(var dst:PWideChar;ptr:PAnsiChar;alen:integer);
var
  enc:byte;
begin
  enc:=ORD(ptr^);
  inc(ptr);
  dec(alen);
  if alen>0 then
    ID3v2_ReadTagStr1(dst,ptr,alen,enc)
  else
    dst:=nil;
end;

procedure ID3v2_CheckLyric(tag:integer; var dst:PWideChar;ptr:PAnsiChar;len:integer);
var
  org,org1:PAnsiChar;
  orgw,ptrw:pWideChar;
  buf:array [0..127] of AnsiChar;
  enc:byte;
begin
  if dst<>NIL then exit;
  enc:=ord(ptr^);
  inc(ptr);
  if tag=frmUSLT then
  begin
    org:=ptr;
    inc(ptr,3); // language
    if (enc=0) or (enc=3) then
    begin
      while ptr^<>#0 do inc(ptr);
      inc(ptr);
    end
    else
    begin
      while pWord(ptr)^<>0 do inc(ptr,2);
      inc(ptr,2);
    end;
    dec(len,ptr-org);
    ID3v2_ReadTagStr1(dst,ptr,len,enc);
  end
  else if tag=frmSYLT then
  begin
    inc(ptr,4);
    if ptr^<>#1 then exit; // 1 - lyric
    inc(ptr);
    mGetMem(dst,len-6);
    FillChar(dst^,len-6,0);

    if (enc=0) or (enc=3) then
    begin
      while ptr^<>#0 do
      begin
        inc(ptr);
        dec(len);
      end;
      inc(ptr);
      dec(len);
      org:=PAnsiChar(dst);
      while len>0 do
      begin
        while ptr^<>#0 do
        begin
          org^:=ptr^; inc(org); inc(ptr);
          dec(len);
        end;
        inc(ptr,1+4); // terminator+timestamp
        dec(len,1+4);
      end;
      org:=PAnsiChar(dst);
      if enc=0 then
        AnsiToWide(org,dst)
      else
        UTF8ToWide(org,dst);
      mFreeMem(org);
    end
    else
    begin
      orgw:=dst;
      ptrw:=pWideChar(ptr);
      while ptrw^<>#0 do
      begin
        inc(ptrw);
        dec(len,SizeOf(WideChar));
      end;
      inc(ptrw);
      dec(len,SizeOf(WideChar));
      while len>0 do
      begin
        while ptrw^<>#0 do
        begin
          orgw^:=ptrw^; inc(orgw); inc(ptrw);
          dec(len,SizeOf(WideChar));
        end;
        inc(ptrw,1+2); // terminator + timestamp
        dec(len,SizeOf(WideChar)+4);
      end;
    end;
  end
  else if tag=frmTXXX then
  begin
    FillChar(buf,SizeOf(buf),0);
    org1:=ptr;
    if (enc=0) or (enc=3) then
    begin
      org:=@buf;
      while ptr^<>#0 do
      begin
        org^:=ptr^;
        inc(org);
        inc(ptr);
      end;
      inc(ptr);
      if StrCmp(buf,'LYRICS')<>0 then
        exit;
    end
    else
    begin
      orgw:=@buf;
      ptrw:=pWideChar(ptr);
      while ptrw^<>#0 do
      begin
        orgw^:=ptrw^;
        inc(orgw);
        inc(ptrw);
      end;
      inc(ptrw);
      if StrCmpW(pWideChar(@buf),'LYRICS')<>0 then
        exit;
      ptr:=PAnsiChar(ptrw);
    end;
    dec(len,ptr-org1);
    ID3v2_ReadTagStr1(dst,ptr,len,enc);
  end;
end;

procedure ID3v2_CheckCover(tag:integer; var dst:pWideChar;ptr:PAnsiChar;len:integer);
var
  org:PAnsiChar;
  ext:dword;
  extw:int64;
  enc:byte;
begin
  if dst<>nil then exit;
  org:=ptr;
  enc:=ord(ptr^); inc(ptr);
  if (pdword(ptr)^ and $FFFFFF)=$3E2D2D then exit; // as '-->'
  if tag=frmAPIC then
  begin
    ext:=GetImageType(nil,ptr);
    repeat inc(ptr) until ptr^=#0; inc(ptr);
  end
  else
  begin
    ext:=pdword(ptr)^ and $FFFFFF;
    inc(ptr,3);
  end;

  if not ord(ptr^) in [0,3,4,6] then exit;
  inc(ptr);
  if (enc=0) or (enc=3) then
  begin
    while ptr^<>#0 do inc(ptr);
    inc(ptr);
  end
  else
  begin
    while pWord(ptr)^<>0 do inc(ptr,2);
    inc(ptr,2);
  end;
  dec(len,ptr-org);

  if ext=0 then
    ext:=GetImageType(pByte(ptr));
  if ext<>0 then
  begin
    FastAnsiToWideBuf(PAnsiChar(@ext),pWideChar(@extw));
    dst:=SaveTemporaryW(ptr,len,PWideChar(@extw));
  end;
end;

function ID3v2_PreReadTag(var Frm:tID3v2FrameHdr;var src:PAnsiChar;ver:integer):PAnsiChar;
var
  i:cardinal;
  dst:PAnsiChar;
begin
  mGetMem(result,Frm.Size);
  if Unsync or ((Frm.Flags and $0200)<>0) then
  begin
    dst:=result;
    i:=0;
    while i<Frm.Size do
    begin
      dst^:=src^;
      inc(src);
      if (dst^=#$FF) and (src^=#0) then
      begin
        inc(src);
        if ver=4 then inc(i);
      end;
      inc(dst);
      inc(i);
    end
  end
  else
  begin
    move(src^,result^,Frm.Size);
    inc(src,Frm.Size);
  end;
end;

procedure ID3v2_ReadTag2(ver:integer;tag:PAnsiChar;Size:integer;var Info:tSongInfo);
type
  a=array [0..3] of byte;
var
  Frm:tID3v2FrameHdr;
  FrmOld:tID3v2FrameHdrOld;
  tmp:integer;
  ls:pWideChar;
  lp:PAnsiChar;
  ptr,buf:PAnsiChar;
  fArtist,fTitle,fAlbum:integer;
  enc:byte;
begin
  lp:=tag+Size;
  fArtist:=0;
  fTitle :=0;
  fAlbum :=0;
  while tag<lp do
  begin
    case ver of
      1,2: begin
        move(tag^,FrmOld,SizeOf(FrmOld));
        Frm.Flags:=0;
        Frm.ID:=FrmOld.ID[0]+(FrmOld.ID[1] shl 8)+(FrmOld.ID[2] shl 16);
        Frm.Size:=(FrmOld.Size[0] shl 16)+(FrmOld.Size[1] shl 8)+FrmOld.Size[2];
        inc(tag,SizeOf(tID3v2FrameHdrOld));
      end;
      3: begin
        move(tag^,Frm,SizeOf(Frm));
        Frm.Size:=BSwap(Frm.Size);
        inc(tag,SizeOf(tID3v2FrameHdr));
      end;
      4: begin
        move(tag^,Frm,SizeOf(Frm));
        Frm.Size:=ID3v2_Correct(Frm.Size);
        inc(tag,SizeOf(tID3v2FrameHdr));
        if (Frm.Flags and $0100)<>0 then
        begin
          Frm.Size:=ID3v2_Correct(pdword(tag)^);
          inc(tag,4);
        end;
      end;
    end;

    if Frm.ID=0 then
      break;
    if Frm.Size=0 then
      continue;
    if (tag+Frm.Size)>lp then
      break;
    buf:=ID3v2_PreReadTag(Frm,tag,ver);

    enc:=ord(buf^);
    case enc of // set priority
      0:   enc:=1;
      1,2: enc:=3;
      3:   enc:=3; // or 2 if you want
    end;
    case Frm.ID of
      frmUSLT,frmULT: ID3v2_CheckLyric(frmUSLT,Info.lyric,buf,Frm.Size);
      frmSYLT,frmSLT: ID3v2_CheckLyric(frmSYLT,Info.lyric,buf,Frm.Size);
      frmTXX,frmTXXX: ID3v2_CheckLyric(frmTXXX,Info.lyric,buf,Frm.Size);
      frmAPIC,frmPIC: ID3v2_CheckCover(Frm.ID ,Info.cover,buf,Frm.Size);

      frmTPE1,frmTP1: begin
        if fArtist<(enc+10) then
        begin
          fArtist:=enc+10;
          mFreeMem(Info.artist);
          ID3v2_ReadTagStr(Info.artist,buf,Frm.Size);
        end
      end;
      frmTIT2,frmTT2: begin
        if fTitle<(enc+10) then
        begin
          fTitle:=enc+10;
          mFreeMem(Info.title);
          ID3v2_ReadTagStr(Info.title,buf,Frm.Size);
        end
      end;
      frmTALB,frmTAL: begin
        if fAlbum<(enc+10) then
        begin
          fAlbum:=enc+10;
          mFreeMem(Info.album);
          ID3v2_ReadTagStr(Info.album,buf,Frm.Size);
        end
      end;
      frmTYER,frmTDRC,frmTYE: begin
        if Info.year<>nil then
          mFreeMem(Info.year);
        ID3v2_ReadTagStr(Info.year,buf,Frm.Size);
      end;

      frmTOPE,frmTPE2,frmTOA,frmTPE4: begin
        if fArtist<enc then
        begin
          fArtist:=enc;
          mFreeMem(Info.artist);
          ID3v2_ReadTagStr(Info.artist,buf,Frm.Size);
        end;
      end;
      frmTIT1,frmTIT3,frmTT1: begin
        if fTitle<enc then
        begin
          fTitle:=enc;
          mFreeMem(Info.title);
          ID3v2_ReadTagStr(Info.title,buf,Frm.Size);
        end;
      end;
      frmTOAL: begin
        if fAlbum<enc then
        begin
          fAlbum:=enc;
          mFreeMem(Info.album);
          ID3v2_ReadTagStr(Info.album,buf,Frm.Size);
        end;
      end;
      frmTORY: begin
        if Info.year=nil then ID3v2_ReadTagStr(Info.year,buf,Frm.Size);
      end;

      frmTCON,frmTCO: begin
        if Info.genre=nil then
        begin
          ID3v2_ReadTagStr(Info.genre,buf,Frm.Size);

          if Info.genre<>nil then
            if Info.genre[0]='(' then
            begin
              tmp:=StrScanW(Info.genre,')')-Info.genre+1;
              if tmp=integer(StrLenW(Info.genre)) then
              begin
                ls:=GenreName(StrToInt(Info.genre+1));
                mFreeMem(Info.genre);
                Info.genre:=ls;
              end
              else if tmp>0 then
                StrCopyW(Info.genre,Info.genre+tmp);
            end;
        end;
      end;
      frmCOMM,frmCOM: begin //!!
        if Info.comment=nil then
        begin
          ptr:=buf;
          inc(ptr,3+1); // language
          if (buf^=#0) or (buf^=#3) then
          begin
            while ptr^<>#0 do inc(ptr);
            inc(ptr);
          end
          else
          begin
            while pWord(ptr)^<>0 do inc(ptr,2);
            inc(ptr,2);
          end;
          dec(Frm.Size,ptr-buf);
          ID3v2_ReadTagStr1(Info.comment,ptr,Frm.Size,ord(buf^));
        end;
      end;
      frmTRCK,frmTRK: begin
        if Info.track=0 then
        begin
          ID3v2_ReadTagStr(ls,buf,Frm.Size);
          Info.track:=StrToInt(ls);
          mFreeMem(ls);
        end;
      end;
    end;
    mFreeMem(buf);
  end;
end;

function ReadID3v2(f:THANDLE; var Info:tSongInfo):longint;
var
  TagHdr:TID3v2TagHdr;
  Tag2:PAnsiChar;
  ExtTagSize:dword;
begin
  BlockRead(f,TagHdr,SizeOf(TagHdr));
  if TagHdr.ID=TAG2Sign then
  begin
    TagHdr.TagSize:=ID3v2_Correct(TagHdr.TagSize);
    Unsync:=(TagHdr.Flags and $80)<>0;
    result:=TagHdr.TagSize;
//    if TagHdr.Version>2 then
    begin
      GetMem(Tag2,TagHdr.TagSize);
      BlockRead(f,Tag2^,TagHdr.TagSize);
      ID3v2_ReadTag2(TagHdr.Version,Tag2,TagHdr.TagSize,Info);
      FreeMem(Tag2);
    end;
    if (TagHdr.Flags and ExtIDHdrMask)<>0 then
    begin
      BlockRead(f,ExtTagSize,SizeOf(ExtTagSize));
      inc(result,4+ExtTagSize);
    end;
    if (TagHdr.Flags and FooterPresent)<>0 then
      inc(result,10);
  end
  else
    result:=0;
  Seek(f,result);
end;
{$ENDIF}
