{$include default.tmpl}
function ReadTemplate(fname:PAnsiChar;var buf:PAnsiChar):integer;
var
  f:THANDLE;
  size:integer;
begin
  if (fname=nil) or (fname^=#0) then
    f:=INVALID_HANDLE_VALUE
  else
    f:=Reset(fname);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
    result:=0
  else
  begin
    size:=FileSize(f);
    mGetMem(buf,size+1);
    buf[size+1]:=#0;
    BlockRead(f,buf^,size);
    CloseHandle(f);
    result:=size;
  end;
end;

function StatOut(report,log,template:PAnsiChar):boolean;

const
  bufsize = 16384;
var
  fout:THANDLE;
  tt,tf:array [0..15] of AnsiChar;
  timebuf:array [0..17] of AnsiChar; // for current date / time
  outbuf:PAnsiChar;
  outpos:PAnsiChar;

  procedure OutChar(var pc:PAnsiChar);
  begin
    outpos^:=pc^;
    inc(pc);
    inc(outpos);
    if (outpos-outbuf)=bufsize then
    begin
      BlockWrite(fout,outbuf^,bufsize);
      outpos:=outbuf;
    end;
  end;

  procedure OutStr(pc:PAnsiChar);
  begin
    while pc^<>#0 do
      OutChar(pc);
  end;

  procedure OutputBlock(var start:PAnsiChar;var Root:pCells;asortmode:integer);
  const
    blocksize = 8192;
  var
    i,max,cnt,len:integer;
    items:cardinal;
    Cell:pStatCell;
    ls,ls1:array [0..511] of AnsiChar;
    block:array [0..blocksize-1] of AnsiChar;
  begin
    len:=StrIndex(start,'%end%');
    if len=0 then
      len:=StrLen(start)
    else
      dec(len);
    if len>6143 then
      err('Template block too large');

    Resort(Root,asortmode);

    case asortmode of
      stArtist,stAlbum,stPath: begin

        Cell:=Root^.Cells[0];
        max:=Cell^.Count;
        if asortmode=stPath then OnlyPath(ls,Cell^.MFile); // speed optimization

        for i:=0 to Root^.Count-1 do
        begin
          with Root^.Cells[i]^ do
          begin
            AltCount:=0;
            if      asortmode=stArtist then cnt:=lstrcmpia(Cell^.Artist,Artist)
            else if asortmode=stAlbum  then cnt:=lstrcmpia(Cell^.Album,Album)
            else                            cnt:=lstrcmpia(ls,OnlyPath(ls1,MFile));
            if cnt=0 then
              inc(max,Count)
            else
            begin
              Cell^.AltCount:=max;
              Cell:=Root^.Cells[i];
              if asortmode=stPath then OnlyPath(ls,Cell^.MFile); // speed optimization
              max:=Count;
            end;
          end;
        end;
        Cell^.AltCount:=max;

        Resort(Root,stAltCount);
        if (asortmode=stAlbum) and (Root^.Cells[0]^.Album^=#0) then
        begin
          if Root^.Count>1 then
            max:=Root^.Cells[1]^.AltCount
          else
            max:=0;
        end
        else
          max:=Root^.Cells[0]^.AltCount;
      end;
      stCount: begin
        max:=Root^.Cells[0]^.Count;
      end;
      stLength: begin
        max:=Root^.Cells[0]^.Length;
      end;
    else
      max:=1;
    end;
    
    items:=1;
    if ReportItems>0 then
      for i:=0 to Root^.Count-1 do
      begin
        with Root^.Cells[i]^ do
        begin
          if (asortmode=stAlbum) and (Album^=#0) then continue;
          case asortmode of
            stArtist,
            stAlbum,
            stPath  : cnt:=AltCount;
            stCount : cnt:=Count;
            stLength: cnt:=Length;
          else
            cnt:=1;
          end;
          if cnt=0 then break;
          move(start^,block,len);
          block[len]:=#0;
          StrReplace(block,'%date%'       ,ShowTime(ls,LastTime));
          StrReplace(block,'%length%'     ,IntToTime(ls,Length));
          StrReplace(block,'%artist%'     ,Artist);
          StrReplace(block,'%title%'      ,Title);
          StrReplace(block,'%album%'      ,Album);
          StrReplace(block,'%file%'       ,MFile);
          StrReplace(block,'%path%'       ,OnlyPath(ls,MFile));
          StrReplace(block,'%num%'        ,IntToStr(ls,items));
          StrReplace(block,'%currenttime%',timebuf);
          StrReplace(block,'%totaltime%'  ,tt);
          StrReplace(block,'%totalfiles%' ,tf);
          StrReplace(block,'%percent%'    ,IntToStr(ls,round(cnt*100/max)));
          StrReplace(block,'%count%'      ,IntToStr(ls,cnt));
          OutStr(block);
        end;
        if items=ReportItems then break;
        inc(items);
      end;
    inc(start,len+5);
  end;

var
  TmplBuf:PAnsiChar;
  ptr:PAnsiChar;
  i,j,k:integer;
  size:integer;
  lsortmode:integer;
  MyTime:TSYSTEMTIME;
  Root:pCells;
  b1,tmp:PAnsiChar;
begin
  result:=false;
  GetLocalTime(MyTime);
  ShowTime(timebuf,PackTime(MyTime));

  Lock:=true;
  Root:=BuildTree(log,b1);
  if Root<>nil then
  begin
    Resort(Root,stArtist);
    Lock:=false;
    size:=ReadTemplate(template,TmplBuf);
    if size=0 then
    begin
      StrDup(TmplBuf,IntTmpl);
      size:=StrLen(IntTmpl);
    end;
    ptr:=TmplBuf;
    fout:=Rewrite(report);
    if fout=THANDLE(INVALID_HANDLE_VALUE) then
      exit;
    mGetMem(outbuf,bufsize);
    outpos:=outbuf;

    i:=0;
    k:=0;
    for j:=0 to Root^.Count-1 do
    begin
      inc(k);
      with Root^.Cells[j]^ do
        inc(i,Length*Count);
    end;
    IntToTime(tt,i); // total time
    IntToStr(tf,k);  // total files
    
    lsortmode:=stDate;
    while (ptr-TmplBuf)<size do
    begin
      while (ptr^<>'%') and (ptr^<>#0) do
        OutChar(ptr);
      if ptr^=#0 then break;
      if StrCmp(ptr,'%block_',7)=0 then
      begin
        if ptr>@TmplBuf then
        begin
          if (ptr-1)^<' ' then
            k:=-1;
        end;
        inc(ptr,7);
        if StrCmp(ptr,'end%',4)=0 then
        begin
          i:=4;
        end
        else if StrCmp(ptr,'freqartist%',11)=0 then
        begin
          lsortmode:=stArtist;
          i:=11;
        end
        else if StrCmp(ptr,'freqsongs%',10)=0 then
        begin
          lsortmode:=stCount;
          i:=10;
        end
        else if StrCmp(ptr,'freqalbum%',10)=0 then
        begin
          lsortmode:=stAlbum;
          i:=10;
        end
        else if StrCmp(ptr,'lastsongs%',10)=0 then
        begin
          lsortmode:=stDate;
          i:=10;
        end
        else if StrCmp(ptr,'songtime%',9)=0 then
        begin
          lsortmode:=stLength;
          i:=9;
        end
        else if StrCmp(ptr,'freqpath%',9)=0 then
        begin
          lsortmode:=stPath;
          i:=9;
        end
        else
        begin
          OutChar(ptr);
          continue;
        end;
        inc(ptr,i);
        if k<0 then
        begin
          while (ptr^<' ') and (ptr^<>#0) do inc(ptr);
          k:=0;
        end;
        if (ReportMask and lsortmode)=0 then
        begin
          tmp:=StrPos(ptr,'%block_end%');
          if tmp<>nil then
            ptr:=tmp+11
          else
            break;
        end;
      end
      else if StrCmp(ptr,'%start%',7)=0 then
      begin
        if ptr>@TmplBuf then
        begin
          if (ptr-1)^<' ' then
            k:=-1;
        end;
        inc(ptr,7);
        if k<0 then
        begin
          while (ptr^<' ') and (ptr^<>#0) do inc(ptr);
          k:=0;
        end;
        OutputBlock(ptr,Root,lsortmode);
      end
      else if StrCmp(ptr,'%currenttime%',13)=0 then
      begin
        inc(ptr,13);
        OutStr(timebuf);
      end
      else if StrCmp(ptr,'%totalfiles%',12)=0 then
      begin
        inc(ptr,12);
        OutStr(tf);
      end
      else if StrCmp(ptr,'%totaltime%',11)=0 then
      begin
        inc(ptr,11);
        OutStr(tt);
      end
      else
        OutChar(ptr);
    end;
    BlockWrite(fout,outbuf^,outpos-outbuf);
    CloseHandle(fout);
    mFreeMem(outbuf);
    mFreeMem(TmplBuf);
    ClearStatCells(Root);
    result:=true;
  end;
  mFreeMem(b1);
end;
