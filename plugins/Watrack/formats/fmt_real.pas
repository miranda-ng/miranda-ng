{Real file}
unit fmt_Real;
{$include compilers.inc}

interface
uses wat_api;

function ReadReal(var Info:tSongInfo):boolean; cdecl;

implementation
uses windows,common,io,tags,srv_format;

const
  blk_RMF = $464D522E; // '.RMF'
  blkPROP = $504F5250; // 'PROP'
  blkCONT = $544E4F43; // 'CONT' - content
  blkMDPR = $5250444D; // 'MDPR'
  blkDATA = $41544144; // 'DATA'
  blkINDX = $58444E49; // 'INDX'
  blkRMMD = $444D4D52; // 'RMMD' - comment block
  blkRMJD = $444A4D52; // 'RMJD'
  blkRMJE = $454A4D52; // 'RMJE'
type
  tChunk = packed record
    ID:dword;
    Len:dword; //with Chunk;
  end;

type
  pPROP = ^tPROP;
  tPROP = packed record
    w1          :word;
    l1,l2       :dword;
    l3,l4       :dword;
    un1         :dword; // or 2+2
    filetotal   :dword; // msec
    l5          :dword;
    InfoDataSize:dword;
    Infosize    :dword;
    w2          :word;  // always 2 ?
    w           :word;  // chunks+1?
  end;

procedure SkipStr(var p:PAnsiChar;alen:integer);
var
  len:integer;
begin
  if alen=2 then
    len:=(ord(p[0]) shl 8)+ord(p[1])
  else
    len:=ord(p[0]);
  inc(p,alen);
//  if len>0 then
    inc(p,len);
end;

function ReadStr(var p:PAnsiChar;alen:integer):PAnsiChar;
var
  len:integer;
begin
  if alen=2 then
    len:=(ord(p[0]) shl 8)+ord(p[1])
  else
    len:=ord(p[0]);
  inc(p,alen);
  if len>0 then
  begin
    mGetMem(result,len+1);
    move(p^,result^,len);
    result[len]:=#0;
    inc(p,len);
  end
  else
    result:=nil;
end;

function GetWord(var p:PAnsiChar):word;
begin
  result:=(ord(p[0]) shl 8)+ord(p[1]);
  inc(p,2);
end;

function GetLong(var p:PAnsiChar):dword;
begin
  result:=(ord(p[0]) shl 24)+(ord(p[1]) shl 16)+(ord(p[2]) shl 8)+ord(p[3]);
  inc(p,4);
end;

function ReadReal(var Info:tSongInfo):boolean; cdecl;
var
  f:THANDLE;
  chunk:tChunk;
  p,buf:PAnsiChar;
  ls:PAnsiChar;
  ver:integer;
  fsize:cardinal;
begin
  result:=false;
  f:=Reset(Info.mfile);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    exit;
  fsize:=FileSize(f);
  while FilePos(f)<fsize do
  begin
    BlockRead(f,chunk,SizeOf(chunk));
    chunk.Len:=BSwap(chunk.Len);
    if (not (AnsiChar(chunk.ID and $FF) in ['A'..'Z','a'..'z','.'])) or
      (chunk.Len<SizeOf(chunk)) then
      break;
    if (chunk.ID=blkPROP) or (chunk.ID=blkCONT) or (chunk.ID=blkMDPR) then
    begin
      mGetMem(buf,chunk.Len-SizeOf(chunk));
      p:=buf;
      BlockRead(f,buf^,chunk.Len-SizeOf(chunk));
      if chunk.ID=blkPROP then
      begin
        inc(p,22);
{
        GetWord(p); // 0
        GetLong(p); // min total bps?
        GetLong(p); // max total bps?
        GetLong(p); // a samples?
        GetLong(p); // b samples?
        GetLong(p); // c (samplesize?)
}
        Info.total:=GetLong(p) div 1000;
{
        GetLong(p); // X
        GetLong(p); // used data size (w/o INDX and tags)
        GetLong(p); // offset to DATA chunk
        GetWord(p); // number of MDPR chunks
        GetWord(p); // 2-9, 3-11
}
      end
      else if chunk.ID=blkCONT then
      begin
        SkipStr(p,2); // rating?
        ls:=ReadStr(p,2); // title
        AnsiToWide(ls,Info.title);
        mFreeMem(ls);
        ls:=ReadStr(p,2); // author
        AnsiToWide(ls,Info.artist);
        mFreeMem(ls);
{
        SkipStr(p,2); // copyright
        SkipStr(p,2); // description
}
      end
      else if chunk.ID=blkMDPR then
      begin //stream or logical info
        GetLong(p); // MDPR block number (from 0)
        if Info.kbps=0 then
          Info.kbps:=GetLong(p) div 1000 // a stream bps
        else
          GetLong(p); // a stream bps
        inc(p,24);
{
        GetLong(p); // a stream bps
        GetLong(p); // b smp
        GetLong(p); // b smp
        GetLong(p); // 0
        GetLong(p); // X
        GetLong(p); //StreamLen
}
        SkipStr(p,1);     //BlockName - usually 'Audio Stream'
        ls:=ReadStr(p,1); //BlockMime
        if StrCmp(ls,'audio/x-pn-realaudio')=0 then
        begin
          inc(p,20);
{
          GetLong(p); // stream dataLen;
          GetLong(p); // type = $2E$72$61$FD
          GetWord(p); // binary version? [4/5]
          GetWord(p); // 0
          GetLong(p); // last byte = ASC version $2E$72$61$($30+ver.)
          GetWord(p);
          GetWord(p);
}
          ver:=GetWord(p); // =version?
          inc(p,30);
{
          GetLong(p); // datalen incl +2 (ver?)
          GetWord(p); // ? 18,19,1,7
          GetWord(p); // 0
          GetWord(p); // un1
          GetLong(p); //
          GetLong(p); //
          GetLong(p); //
          GetWord(p); //
          GetWord(p); // un2=un1
          GetWord(p); //
          GetWord(p); // 60 [93] (0 for ra4)
}
          if ver=5 then
          begin
            Info.khz:=GetLong(p) div 1000;
            inc(p,8);
{
            GetLong(p); // equ KHZ
            GetLong(p); // bits/channel?
}
            Info.channels:=GetWord(p);
{
            GetLong(p); // 'genr'
            GetLong(p); // codec name
            GetWord(p); // $01 $07
            GetLong(p); // 0
            GetWord(p); // channel data len (16-stereo,8-mono)
            GetWord(p); // $01 $00
            GetWord(p); // $00 $03[mono-2]
            GetWord(p); // $04 [mono-2] $00
            GetWord(p); //
            if Info.channels=2 then
            begin
              GetLong(p); // 0
              GetWord(p); // 01
              GetWord(p); // 03
            end
}
          end
          else
          begin
            Info.khz:=GetWord(p) div 1000;
            GetLong(p); // bits/channel?
            Info.channels:=GetWord(p);
{
            SkipStr(p,1); // codec
            SkipStr(p,1); // codec
            GetWord(p); // $01 $07
            inc(p,5);
}
          end
        end;
{
        if StrCmp(tmpstr,'logical-fileinfo')=0 then
        begin
          GetLong(p); // a block len w/o
          GetLong(p); // a block len with
          GetLong(p); // 0
          GetLong(p); // number of nodes
          for i:=0 to Nodes-1 do
          begin
            GetLong(p);     // node len with len dword
            GetWord(p);
            SkipStr(p,1);     // node name
            GetLong(p);     // value type? 2 - asciiz
            SkipStr(p,2); //node value
          end;
        end;
}

//        if StrCmp(tmpstr,'Video Stream')=0 then
        if StrCmp(ls,'video/x-pn-realvideo')=0 then
        begin
          GetLong(p); //stream dataLen;
          Info.kbps:=GetLong(p); //override kbps
          GetLong(p); //VIDO=vidtype
          Info.codec:=ord(p[0])+(ord(p[1]) shl 8)+
              (ord(p[2]) shl 16)+(ord(p[3]) shl 24); //codec ex.'RV30'
          inc(p,4);
          Info.width:=GetWord(p); //width
          Info.height:=GetWord(p); //height
          GetWord(p); //fps or colordeep
          GetWord(p); //alt.width ?
          GetWord(p); //alt. height ?
          Info.fps:=GetWord(p)*100; //fps
          {}
        end;

        mFreeMem(ls);
      end;
      mFreeMem(buf);
    end
    else if chunk.ID=blkRMMD then //comment
    begin
      Skip(f,chunk.Len-SizeOf(chunk));
{
    BlockRead(f,chunk,SizeOf(chunk)); //RJMD
    chunk.len:=BSwap(chunk.len);
    BlockRead(f,tmplong,4);


    BlockRead(f,chunk,SizeOf(chunk)); //RMJE
    chunk.len:=BSwap(chunk.len);
}
    end
    else
    begin
      if chunk.ID=blk_RMF then
        if FilePos(f)<>SizeOf(chunk) then // channels-1: ofs=$0A
          break;
      Skip(f,chunk.Len-SizeOf(chunk));
    end;
  end;
  ReadID3v1(f,Info);
  CloseHandle(f);
  result:=true;
end;

var
  LocalFormatLinkRM,
  LocalFormatLinkRA,
  LocalFormatLinkRAM:twFormat;

procedure InitLink;
begin
  LocalFormatLinkRM.Next:=FormatLink;

  LocalFormatLinkRM.This.proc :=@ReadReal;
  LocalFormatLinkRM.This.ext  :='RM';
  LocalFormatLinkRM.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkRM;

  LocalFormatLinkRA.Next:=FormatLink;

  LocalFormatLinkRA.This.proc :=@ReadReal;
  LocalFormatLinkRA.This.ext  :='RA';
  LocalFormatLinkRA.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkRA;

  LocalFormatLinkRAM.Next:=FormatLink;

  LocalFormatLinkRAM.This.proc :=@ReadReal;
  LocalFormatLinkRAM.This.ext  :='RAM';
  LocalFormatLinkRAM.This.flags:=WAT_OPT_VIDEO;

  FormatLink:=@LocalFormatLinkRAM;
end;

initialization
  InitLink;

end.
