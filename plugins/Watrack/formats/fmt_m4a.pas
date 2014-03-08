{M4A code template}
unit fmt_M4A;
{$include compilers.inc}

interface
uses wat_api;

function ReadM4A(var Info:tSongInfo):boolean; cdecl;

implementation

uses windows,common,io,srv_format,
{$IFDEF KOL_MCK}KolZlibBzip{$ELSE}Zlib,zwrapper{$ENDIF};

type
  mp4Atom = record
    len:dword;
    name:dword;
  end;

const
  at_moov = $766F6F6D;
  at_mvhd = $6468766D;
  at_udta = $61746475;
  at_meta = $6174656D;
  at_ilst = $74736C69;
  at_cmov = $766F6D63;
  at_dcom = $6D6F6364;
  at_cmvd = $64766D63; // 4 - unpacked size, data
  at_trak = $6B617274;
  at_tkhd = $64686B74; // not needed
  at_mdia = $6169646D;
  at_minf = $666E696D;
  at_smhd = $64686D73;
  at_vmhd = $64686D76;
  at_stbl = $6C627473;
  at_stsd = $64737473;

const
  atm_nam  = $6D616EA9; // title
  atm_ART  = $545241A9; // artist
  atm_wrt  = $747277A9; // writer
  atm_alb  = $626C61A9; // album
  atm_day  = $796164A9; // date
  atm_cmt  = $746D63A9; // comment
  atm_gen  = $6E6567A9; // alt.genre
  atm_gnre = $65726E67; // genre
  atm_trkn = $6E6B7274; // track number
//  atm_zlib = $62696C7A;

type
  pstsd = ^tstsd;
  tstsd = packed record
    version       :byte;
    flags         :array [0..2] of byte;
    NumEntries    :dword;
    SampleDescSize:dword; // $56
    DataFormat    :dword;
    reserved      :array [0..5] of byte;
    RefIndex      :word;
    Ver           :word;
    Revision      :word;
    Vendor        :dword;
    Temporal      :dword;
    Spacial       :dword;
    Width         :word;
    Height        :word;
    HRes          :dword; //single;
    VRes          :dword;
    DataSize      :dword;
    FrameCount    :word;
    CompNameLen   :byte;
    Compressor    :array [0..18] of AnsiChar;
    ColorDepth    :word;
    ColorTableID  :word;
  end;
  pastsd = ^astsd;
  astsd = packed record
    Version     :byte;
    Flags       :array [0..2] of byte;
    NumEntires  :dword;
    DescSize    :dword;
    CodingName  :array[0..3] of AnsiChar;
    Reserved    :array[0..5] of byte;
    RefIndex    :word;
    Reserved_   :array[0..1] of dword;
    ChannelCount:word;
    SampleSize  :word;
    Pre_defined :word;
    Reserved___ :word;
    Samplerate  :dword;
  end;
  pmvhd = ^mvhd;
  mvhd = packed record
    Version:byte;
    flags:array [0..2] of byte;
    Creation:dword;
    Modification:dword;
    TimeScale:dword;
    Duration:dword;
  end;

procedure ReadAtom(f:THANDLE;var atom:mp4Atom);
begin
  BlockRead(f,atom.len,4);
  if atom.len>0 then
  begin
    BlockRead(f,atom.name,4);
    atom.len:=BSwap(atom.len);
  end
  else
  begin
    atom.name:=0;
    atom.len:=8;
  end;
end;

procedure GetAtom(var p:pbyte;var atom:mp4Atom);
begin
  atom.len:=pdword(p)^;
  inc(p,4);
  if atom.len>0 then
  begin
    atom.name:=pdword(p)^;
    inc(p,4);
    atom.len:=BSwap(atom.len);
  end
  else
  begin
    atom.name:=0;
    atom.len:=8;
  end;
end;

function SetTree(from:mp4Atom;var p:pbyte;path:PAnsiChar;var parent:pbyte):integer;
var
  atom:mp4Atom;
  len:cardinal;
  saved:pbyte;
begin
  saved:=p;
  len:=0;
  dec(from.len,SizeOf(from));
  parent:=p;
  repeat
    GetAtom(p,atom);
    if atom.name=pdword(path)^ then
    begin
      inc(path,4);
      if path^<>#0 then
      begin
        parent:=p;
        inc(path);
        len:=0;
        from.len:=atom.len-SizeOf(atom);
      end
      else
      begin
        result:=atom.len;
        exit;
      end;
    end
    else
    begin
      inc(p,atom.len-SizeOf(atom));
      inc(len,atom.len);
    end;
  until len>=from.len;
  result:=-1;
  p:=saved;
end;

function ReadInt(var p:pbyte):dword;
var
  len:integer;
begin
  len:=pdword(p)^;
  inc(p,4);
  len:=BSwap(len);
  if len>0 then
    inc(p,4); // 'data'
  inc(p,4); // type?
  inc(p,4); // encoding?
  dec(len,8+8);
  if len>4 then len:=4;
  if len=2 then
  begin
    result:=p^*$100;
    inc(p);
    inc(result,p^);
    inc(p);
  end
  else
  begin
    result:=BSwap(pdword(p)^);
    inc(p,4);
  end;
end;

procedure ReadProp(var p:pbyte;var prop:pWideChar);
var
  len:integer;
  ltmp:PAnsiChar;
  c:byte;
begin
  len:=pdword(p)^;
  inc(p,4);
  len:=BSwap(len);
  if len>0 then
    inc(p,4); // 'data'
  inc(p,4); // type?
  inc(p,4); // encoding?
  dec(len,8+8);
  ltmp:=pointer(p);
  inc(p,len);
  c:=p^;
  p^:=0;
  UTF8ToWide(ltmp,prop);
  p^:=c;
end;

function ReadM4A(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  atom:mp4Atom;
  cursize,parentsize:integer;
  par,buf,p,pn,finish:pbyte;
  size:integer;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  cursize:=0;
  parentsize:=FileSize(f);
  repeat
    ReadAtom(f,atom);
    if atom.name=at_moov then
    begin
      mGetMem(buf,atom.len);
      BlockRead(f,buf^,atom.len);
      p:=buf;
      finish:=pByte(PAnsiChar(p)+atom.len-SizeOf(atom));
      repeat
        GetAtom(p,atom);
        pn:=PByte(PAnsiChar(p)+atom.len-SizeOf(atom));

        if atom.name=at_cmov then
        begin
          size:=SetTree(atom,p,'cmvd',par);
          if size>0 then
          begin
            ZDecompressBuf(PAnsiChar(p)+4,size-SizeOf(mp4Atom),
                pointer(pn),size,BSwap(pdword(p)^));
            mFreeMem(buf);
            buf:=pn;
            p:=buf;
            GetAtom(p,atom); //must be 'moov'
            finish:=PByte(PAnsiChar(p)+atom.len-SizeOf(atom));
            continue;
          end;
        end;

        if atom.name=at_mvhd then
        begin
          if pmvhd(p)^.TimeScale<>0 then
            Info.total:=BSwap(pmvhd(p)^.Duration) div BSwap(pmvhd(p)^.TimeScale);
        end;
        if atom.name=at_udta then
        begin
          size:=SetTree(atom,p,'meta.ilst',par);
          if size>0 then
          begin
            cursize:=0;
            repeat
              GetAtom(p,atom);
              if      atom.name=atm_nam  then ReadProp(p,Info.title)
              else if atom.name=atm_ART  then ReadProp(p,Info.artist)
//            else if atom.name=atm_wrt  then ReadProp(p,Info.title)
              else if atom.name=atm_alb  then ReadProp(p,Info.album)
              else if atom.name=atm_day  then ReadProp(p,Info.year)
              else if atom.name=atm_cmt  then ReadProp(p,Info.comment)
//            else if atom.name=atm_gen  then ReadProp(p,Info.genre)
              else if atom.name=atm_gnre then Info.genre:=GenreName(ReadInt(p)-1)
              else if atom.name=atm_trkn then Info.track:=ReadInt(p)
              else
                inc(p,atom.len-SizeOf(mp4Atom));
              inc(cursize,atom.len);
            until cursize>=size;
          end;
        end;
        // video properties
        if atom.name=at_trak then
        begin
          if SetTree(atom,p,'mdia.minf.vmhd',par)>0 then
          begin
            p:=par;
            if SetTree(atom,p,'stbl.stsd',par)>0 then
            begin
              Info.width :=swap(pstsd(p)^.Width);
              Info.height:=swap(pstsd(p)^.Height);
              Info.codec :=pstsd(p)^.DataFormat;
            end;
          end
          // audio props
          else if SetTree(atom,p,'mdia.minf.smhd',par)>0 then
          begin
            p:=par;
            if SetTree(atom,p,'stbl.stsd',par)>0 then
            begin
              Info.khz:=(BSwap(pastsd(p)^.Samplerate) shr 16) div 1000;
              Info.channels:=swap(pastsd(p)^.ChannelCount);
            end;
            p:=par;
            if SetTree(atom,p,'stsz',par)>0 then
            begin
              if pdword(PAnsiChar(p)+4)^=0 then
                Info.vbr:=1;
            end;
          end;
        end;
        p:=pn;
      until PAnsiChar(p)>=PAnsiChar(finish);
      mFreeMem(buf);
      break;
    end
    else
      Skip(f,atom.len-SizeOf(mp4Atom));
    inc(cursize,atom.len);
  until cursize>=parentsize;
  CloseHandle(f);
end;

var
  LocalFormatLinkM4A,
  LocalFormatLinkMP4,
  LocalFormatLinkMOV,
  LocalFormatLink3GP:twFormat;

procedure InitLink;
begin
  LocalFormatLinkM4A.Next:=FormatLink;

  LocalFormatLinkM4A.This.proc :=@ReadM4A;
  LocalFormatLinkM4A.This.ext  :='M4A';
  LocalFormatLinkM4A.This.flags:=0;

  FormatLink:=@LocalFormatLinkM4A;

  LocalFormatLinkMP4.Next:=FormatLink;

  LocalFormatLinkMP4.This.proc :=@ReadM4A;
  LocalFormatLinkMP4.This.ext  :='MP4';
  LocalFormatLinkMP4.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkMP4;

  LocalFormatLinkMOV.Next:=FormatLink;

  LocalFormatLinkMOV.This.proc :=@ReadM4A;
  LocalFormatLinkMOV.This.ext  :='MOV';
  LocalFormatLinkMOV.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkMOV;

  LocalFormatLink3GP.Next:=FormatLink;

  LocalFormatLink3GP.This.proc :=@ReadM4A;
  LocalFormatLink3GP.This.ext  :='3GP';
  LocalFormatLink3GP.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLink3GP;

end;

initialization
  InitLink;
end.
