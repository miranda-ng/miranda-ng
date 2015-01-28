{}

const
  client_id  = 'wat';//'wat'; 'tst'
  client_ver = '1.0';
  api_key    = '51f5d25159da31b0814609c3a12900e2';

const
  defreq = 'http://post.audioscrobbler.com/?hs=true&p=1.2.1&c=<client-id>&v=<client-ver>&u=<user>&t=<timestamp>&a=<auth>';
  scraddr = 'http://ws.audioscrobbler.com/2.0/';

function GetMD5Str(const digest:TMD5Hash; buf:pAnsiChar):PAnsiChar;
begin
  buf[00]:=HexDigitChrLo[digest[00] shr 4]; buf[01]:=HexDigitChrLo[digest[00] and $0F];
  buf[02]:=HexDigitChrLo[digest[01] shr 4]; buf[03]:=HexDigitChrLo[digest[01] and $0F];
  buf[04]:=HexDigitChrLo[digest[02] shr 4]; buf[05]:=HexDigitChrLo[digest[02] and $0F];
  buf[06]:=HexDigitChrLo[digest[03] shr 4]; buf[07]:=HexDigitChrLo[digest[03] and $0F];
  buf[08]:=HexDigitChrLo[digest[04] shr 4]; buf[09]:=HexDigitChrLo[digest[04] and $0F];
  buf[10]:=HexDigitChrLo[digest[05] shr 4]; buf[11]:=HexDigitChrLo[digest[05] and $0F];
  buf[12]:=HexDigitChrLo[digest[06] shr 4]; buf[13]:=HexDigitChrLo[digest[06] and $0F];
  buf[14]:=HexDigitChrLo[digest[07] shr 4]; buf[15]:=HexDigitChrLo[digest[07] and $0F];
  buf[16]:=HexDigitChrLo[digest[08] shr 4]; buf[17]:=HexDigitChrLo[digest[08] and $0F];
  buf[18]:=HexDigitChrLo[digest[09] shr 4]; buf[19]:=HexDigitChrLo[digest[09] and $0F];
  buf[20]:=HexDigitChrLo[digest[10] shr 4]; buf[21]:=HexDigitChrLo[digest[10] and $0F];
  buf[22]:=HexDigitChrLo[digest[11] shr 4]; buf[23]:=HexDigitChrLo[digest[11] and $0F];
  buf[24]:=HexDigitChrLo[digest[12] shr 4]; buf[25]:=HexDigitChrLo[digest[12] and $0F];
  buf[26]:=HexDigitChrLo[digest[13] shr 4]; buf[27]:=HexDigitChrLo[digest[13] and $0F];
  buf[28]:=HexDigitChrLo[digest[14] shr 4]; buf[29]:=HexDigitChrLo[digest[14] and $0F];
  buf[30]:=HexDigitChrLo[digest[15] shr 4]; buf[31]:=HexDigitChrLo[digest[15] and $0F];
  buf[32]:=#0;
  result:=@buf;
end;

function GetMD5(const data;datalen:integer;var digest:TMD5Hash):TMD5Hash;
begin
  FillChar(digest,16,0);

  mir_md5_hash(pbyte(data),datalen,digest);

  result:=digest;
end;

function HandShake(login, password:PAnsiChar; notify:bool=false):bool;
var
  buf:array [0..32] of AnsiChar;
  digest:TMD5Hash;
  stat:mir_md5_state_t;
  timestamp:array [0..31] of AnsiChar;
  request:array [0..511] of AnsiChar;
  tmp,res:pAnsiChar;
begin
  result:=false;
  GetMD5Str(GetMD5(password,StrLen(password),digest),buf);
  mir_md5_init(@stat);
  mir_md5_append(@stat,@buf,32);
  IntToStr(timestamp,GetCurrentTimestamp);
  mir_md5_append(@stat,@timestamp,StrLen(timestamp));
  mir_md5_finish(@stat,digest);
  GetMD5Str(digest,buf);
  StrCopy(request,defreq);
  StrReplace(request,'<client-id>' ,client_id);
  StrReplace(request,'<client-ver>',client_ver);
  StrReplace(request,'<user>'      ,login);
  StrReplace(request,'<timestamp>' ,timestamp);
  StrReplace(request,'<auth>'      ,buf);

  res:=SendRequest(request,REQUEST_GET);
  if (res<>nil) and (uint_ptr(res)>$0FFF) then
  begin
    if StrCmp(CharReplace(res,#10,#0),'OK')=0 then
    begin
      result:=true;
      tmp:=StrEnd(res)+1; StrDup(session_id,tmp);
      tmp:=StrEnd(tmp)+1; StrDup(np_url    ,tmp);
      tmp:=StrEnd(tmp)+1; StrDup(sub_url   ,tmp);
    end
    else if notify then
    begin
      tmp:=StrCopyE(request,Translate('Last.fm error: '));
      if      StrCmp(res,'BANNED'  )=0 then StrCopy(tmp,Translate('Client is banned'))
      else if StrCmp(res,'BADAUTH' )=0 then StrCopy(tmp,Translate('Bad Auth. Check login and password'))
      else if StrCmp(res,'BADTIME' )=0 then StrCopy(tmp,Translate('Bad TimeStamp'))
      else if StrCmp(res,'FAILED',6)=0 then StrCopy(tmp,res+7);
      CallService(MS_POPUP_SHOWMESSAGE,wparam(@request),SM_ERROR);
    end;
    mFreeMem(res);
  end;
end;

function Encode(dst,src:pAnsiChar):PAnsiChar;
begin
  while src^<>#0 do
  begin
    if not (src^ in [' ','%','+','&','?',#128..#255]) then
      dst^:=src^
    else
    begin
      dst^:='%'; inc(dst);
      dst^:=HexDigitChr[ord(src^) shr 4]; inc(dst);
      dst^:=HexDigitChr[ord(src^) and $0F];
    end;
    inc(src);
    inc(dst);
  end;
  dst^:=#0;
  result:=dst;
end;

function SendNowPlaying:integer;
var
  si:pSongInfoA;
  buf    :array [0..31  ] of AnsiChar;
  args   :array [0..1023] of AnsiChar;
  res,pc:PAnsiChar;
begin
  result:=-1;
  if session_id<>nil then
  begin
    si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UTF8,0));

    pc:=@args;
    pc:=StrCopyE(pc,'s='); pc:=StrCopyE(pc,session_id); //'?s='
    pc:=StrCopyE(pc,'&a=');
    if si^.artist=nil then pc:=StrCopyE(pc,'Unknown')
    else                   pc:=Encode(pc,si^.artist);
    pc:=StrCopyE(pc,'&t=');
    if si^.title =nil then pc:=StrCopyE(pc,'Unknown')
    else                   pc:=Encode(pc,si^.title);
    pc:=StrCopyE(pc,'&l='); if si^.total>0 then pc:=StrCopyE(pc,IntToStr(buf,si^.total));
    pc:=StrCopyE(pc,'&b='); pc:=Encode(pc,si^.album);
    pc:=StrCopyE(pc,'&n=');
    if si^.track<>0 then
      {pc:=}StrCopyE(pc,IntToStr(buf,si^.track));

    res:=SendRequest(np_url,REQUEST_POST,args);
    if (res<>nil) and (uint_ptr(res)>$0FFF) then
    begin
      if StrCmp(CharReplace(res,#10,#0),'OK')=0 then
        result:=1
      else if StrCmp(res,'BADSESSION')=0 then
        result:=-1;
      mFreeMem(res);
    end;
  end;
end;

function Scrobble:integer;
var
  si:pSongInfoA;
  buf,timestamp:array [0..31] of AnsiChar;
  args   :array [0..1023] of AnsiChar;
  res,pc:PAnsiChar;
begin
  result:=-1;
  if session_id<>nil then
  begin
    si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UTF8,0));
    IntToStr(timestamp,GetCurrentTimestamp);

    pc:=@args;
    pc:=StrCopyE(pc,'s='  ); pc:=StrCopyE(pc,session_id);
    pc:=StrCopyE(pc,'&a[0]=');
    if si^.artist=nil then pc:=StrCopyE(pc,'Unknown')
    else                   pc:=Encode(pc,si^.artist);
    pc:=StrCopyE(pc,'&t[0]=');
    if si^.title =nil then pc:=StrCopyE(pc,'Unknown')
    else                   pc:=Encode(pc,si^.title);
    pc:=StrCopyE(pc,'&i[0]='); pc:=StrCopyE(pc,timestamp);
    pc:=StrCopyE(pc,'&r[0]=&m[0]=');
    pc:=StrCopyE(pc,'&l[0]=');
    if si^.total>0 then
    begin
      pc:=StrCopyE(pc,IntToStr(buf,si^.total));
      pc:=StrCopyE(pc,'&o[0]=P');
    end
    else
    begin
      pc:=StrCopyE(pc,'&o[0]=R');
    end;
    pc:=StrCopyE(pc,'&b[0]='); pc:=Encode(pc,si^.album);
    pc:=StrCopyE(pc,'&n[0]=');
    if si^.track<>0 then
      {pc:=}StrCopyE(pc,IntToStr(buf,si^.track));

    res:=SendRequest(sub_url,REQUEST_POST,args);
    if (res<>nil) and (uint_ptr(res)>$0FFF) then
    begin
      if StrCmp(CharReplace(res,#10,#0),'OK')=0 then
        result:=1
      else if StrCmp(res,'BADSESSION')=0 then
      begin
        result:=-1;
      end
      else if StrCmp(res,'FAILED',6)=0 then
      begin
        StrCopy(StrCopyE(args,Translate('Last.fm error: ')),res+7);
        CallService(MS_POPUP_SHOWMESSAGE,wparam(@args),SM_NOTIFY);
        result:=0;
      end;
      mFreeMem(res);
    end;
  end;
end;

//----- Get Info service functions -----

function FullEncode(dst,src:pAnsiChar):PAnsiChar;
begin
  while src^<>#0 do
  begin
    if src^ in ['A'..'Z','a'..'z','0'..'9'] then
      dst^:=src^
    else
    begin
      dst^:='%'; inc(dst);
      dst^:=HexDigitChr[ord(src^) shr 4]; inc(dst);
      dst^:=HexDigitChr[ord(src^) and $0F];
    end;
    inc(src);
    inc(dst);
  end;
  dst^:=#0;
  result:=dst;
end;

var
  xmlparser:TXML_API_W;

function FixInfo(Info:pWideChar):pWideChar;
var
  pc,ppc:pWideChar;
  cnt:cardinal;
  need:boolean;
begin
  pc:=Info;
  cnt:=0;
  need:=false;
  while pc^<>#0 do
  begin
    if pc^=#$0D then
    begin
      inc(cnt);
      inc(pc);
      if pc^<>#$0A then
        need:=true;
    end
    else
      inc(pc);
  end;
  if need then
  begin
    mGetMem(result,(StrLenW(Info)+1+cnt)*SizeOf(WideChar));
    pc:=Info;
    ppc:=result;
    while pc^<>#0 do
    begin
      ppc^:=pc^;
      if pc^=#$0D then
      begin
        inc(ppc);
        ppc^:=#$0A;
      end;
      inc(pc);
      inc(ppc);
    end;
    ppc^:=#0;
  end
  else
    StrDupW(result,Info);
end;

function GetArtistInfo(var data:tLastFMInfo;lang:integer):int;
var
  si:pSongInfo;
  res,pc:pAnsiChar;
  request:array [0..1023] of AnsiChar;
  root,actnode,node,nnode:HXML;
  i:integer;
  pcw,p,pp:PWideChar;
  artist:pAnsiChar;
begin
  result:=0;
  if data.artist=nil then
  begin
    si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
    pWideChar(artist):=si^.artist;
  end
  else
    pWideChar(artist):=data.artist;
  if artist=nil then
    exit;
  WideToUTF8(pWideChar(artist),artist);
  pc:=FullEncode(StrCopyE(request,
      'http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&api_key='+api_key+'&artist='),
      artist);
  mFreeMem(artist);
  if lang<>0 then
    StrCopyE(StrCopyE(pc,'&lang='),pAnsiChar(@lang));
  res:=SendRequest(request,REQUEST_GET);
  if (res<>nil) and (uint_ptr(res)>$0FFF) then
  begin
    UTF8ToWide(res,pcw);
    mFreeMem(res);
    xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
    CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
    with xmlparser do
    begin
      i:=StrLenW(pcw)*SizeOf(WideChar);
      root:=parseString(pcw,@i,nil);

      actnode:=getChild(getChild(root,0),0); // "artist"

      if data.artist=nil then
        StrDupW(data.artist,getText(GetNthChild(actnode,'name',0)));

      i:=0;
      repeat
        node:=GetNthChild(actnode,'image',i);
        if node=0 then break;
        if StrCmpW(GetAttrValue(node,'size'),'medium')=0 then
        begin
          WideToUTF8(GetText(node),data.image);
          break;
        end;
        inc(i);
      until false;

      // bio
      p:=StrPosW(pcw,'<content><![CDATA[');
      if p<>nil then
      begin
        inc(p,18);
        pp:=StrPosW(p,']]');
        if pp<> nil then pp^:=#0;
        data.info:=FixInfo(p);
      end;

      // similar
      i:=0;
      pcw:=pWideChar(@request); pcw^:=#0;
      node:=GetNthChild(actnode,'similar',0);
      repeat
        nnode:=GetNthChild(GetNthChild(node,'artist',i),'name',0);
        if nnode=0 then break;
        if pcw<>@request then
        begin
          pcw^:=','; inc(pcw);
          pcw^:=' '; inc(pcw);
        end;
          pcw:=StrCopyEW(pcw,GetText(nnode));
        inc(i);
      until false;
      pcw:=#0;
      StrDupW(data.similar,pWideChar(@request));

      // tags
      i:=0;
      pcw:=pWideChar(@request); pcw^:=#0;
      node:=GetNthChild(actnode,'tags',0);
      repeat
        nnode:=GetNthChild(GetNthChild(node,'tag',i),'name',0);
        if nnode=0 then break;
        if pcw<>@request then
        begin
          pcw^:=','; inc(pcw);
          pcw^:=' '; inc(pcw);
        end;
          pcw:=StrCopyEW(pcw,GetText(nnode));
        inc(i);
      until false;
      pcw:=#0;
      StrDupW(data.tags,pWideChar(@request));
      DestroyNode(root);
    end;
  end;
end;

function GetAlbumInfo(var data:tLastFMInfo;lang:integer):int;
var
  si:pSongInfo;
  res,pc:pAnsiChar;
  request:array [0..1023] of AnsiChar;
  root,actnode,node,nnode:HXML;
  i:integer;
  p,pp,pcw:PWideChar;
  album,artist:pAnsiChar;
begin
  result:=0;
  si:=nil;
  if data.album=nil then
  begin
    si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
    pWideChar(album):=si^.album;
  end
  else
    pWideChar(album):=data.album;
  if album=nil then
    exit;
  WideToUTF8(pWideChar(album),album);
  pc:=FullEncode(StrCopyE(request,
     'http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key='+api_key+'&album='),
     album);
  mFreeMem(album);
  if data.artist=nil then
  begin
    if si=nil then
      si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
    pWideChar(artist):=si^.artist;
  end
  else
    pWideChar(artist):=data.artist;
  if artist<>nil then
  begin
    WideToUTF8(pWideChar(artist),artist);
    pc:=FullEncode(StrCopyE(pc,'&artist='),artist);
    mFreeMem(artist);
  end;

  if lang<>0 then
    StrCopyE(StrCopyE(pc,'&lang='),pAnsiChar(@lang));

  res:=SendRequest(request,REQUEST_GET);
  if res<>nil then
  begin
    UTF8ToWide(res,pcw);
    mFreeMem(res);
    xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
    CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
    with xmlparser do
    begin
      i:=StrLenW(pcw)*SizeOf(WideChar);
      root:=parseString(pcw,@i,nil);

      actnode:=getChild(getChild(root,0),0); // "album"

      if data.album=nil then
        StrDupW(data.album,getText(GetNthChild(actnode,'name',0)));
      StrDupW(data.release,getText(GetNthChild(actnode,'releasedate',0)));
      if data.artist=nil then
        StrDupW(data.artist,getText(GetNthChild(actnode,'artist',0)));

      i:=0;
      repeat
        node:=GetNthChild(actnode,'image',i);
        if node=0 then break;
        if StrCmpW(GetAttrValue(node,'size'),'medium')=0 then
        begin
          WideToUTF8(GetText(node),data.image);
          break;
        end;
        inc(i);
      until false;

      p:=StrPosW(pcw,'<content><![CDATA[');
      if p<>nil then
      begin
        inc(p,18);
        pp:=StrPosW(p,']]');
        if pp<> nil then pp^:=#0;
        data.info:=FixInfo(p);
      end;

      // tags
      i:=0;
      pcw:=pWideChar(@request); pcw^:=#0;
      node:=GetNthChild(actnode,'toptags',0);
      repeat
        nnode:=GetNthChild(GetNthChild(node,'tag',i),'name',0);
        if nnode=0 then break;
        if pcw<>@request then
        begin
          pcw^:=','; inc(pcw);
          pcw^:=' '; inc(pcw);
        end;
          pcw:=StrCopyEW(pcw,GetText(nnode));
        inc(i);
      until false;
      pcw:=#0;
      StrDupW(data.tags,pWideChar(@request));

      DestroyNode(root);
    end;
  end;
end;

function GetTrackInfo(var data:tLastFMInfo;lang:integer):int;
var
  si:pSongInfo;
  res,pc:pAnsiChar;
  request:array [0..1023] of AnsiChar;
  root,actnode,node,anode:HXML;
  i:integer;
  p,pp,pcw:PWideChar;
  title,artist:pAnsiChar;
begin
  result:=0;
  si:=nil;
  if data.album=nil then
  begin
    si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
    pWideChar(title):=si^.title;
  end
  else
    pWideChar(title):=data.title;
  if title=nil then
    exit;
  WideToUTF8(pWideChar(title),title);
  pc:=FullEncode(StrCopyE(request,
     'http://ws.audioscrobbler.com/2.0/?method=track.getinfo&api_key='+api_key+'&track='),
     title);
  mFreeMem(title);
  if data.artist=nil then
  begin
    if si=nil then
      si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
    pWideChar(artist):=si^.artist;
  end
  else
    pWideChar(artist):=data.artist;
  if artist<>nil then
  begin
    WideToUTF8(pWideChar(artist),artist);
    pc:=FullEncode(StrCopyE(pc,'&artist='),artist);
    mFreeMem(artist);
  end;

  if lang<>0 then
    StrCopyE(StrCopyE(pc,'&lang='),pAnsiChar(@lang));

  res:=SendRequest(request,REQUEST_GET);
  if res<>nil then
  begin
    UTF8ToWide(res,pcw);
    mFreeMem(res);
    xmlparser.cbSize:={XML_API_SIZEOF_V1;//}SizeOf(TXML_API_W);
    CallService(MS_SYSTEM_GET_XI,0,lparam(@xmlparser));
    with xmlparser do
    begin
      i:=StrLenW(pcw)*SizeOf(WideChar);
      root:=parseString(pcw,@i,nil);

      actnode:=getChild(getChild(root,0),0); // "track"
      if data.artist=nil then
        StrDupW(data.artist,getText(GetNthChild(GetNthChild(actnode,'artist',0),'name',0)));

      anode:=GetNthChild(actnode,'album',i);

      if data.album=nil then
        StrDupW(data.album,getText(GetNthChild(anode,'title',0)));

      data.trknum:=StrToInt(getAttrValue(anode,'position'));
      if data.title=nil then
        StrDupW(data.title,getText(GetNthChild(actnode,'name',0)));

      i:=0;
      repeat
        node:=GetNthChild(anode,'image',i);
        if node=0 then break;
        if StrCmpW(GetAttrValue(node,'size'),'medium')=0 then
        begin
          WideToUTF8(GetText(node),data.image);
          break;
        end;
        inc(i);
      until false;

      p:=StrPosW(pcw,'<content><![CDATA[');
      if p<>nil then
      begin
        inc(p,18);
        pp:=StrPosW(p,']]');
        if pp<> nil then pp^:=#0;
        data.info:=FixInfo(p);
      end;

      // tags
      i:=0;
      pcw:=pWideChar(@request); pcw^:=#0;
      node:=GetNthChild(actnode,'toptags',0);
      repeat
        anode:=GetNthChild(GetNthChild(node,'tag',i),'name',0);
        if anode=0 then break;
        if pcw<>@request then
        begin
          pcw^:=','; inc(pcw);
          pcw^:=' '; inc(pcw);
        end;
        pcw:=StrCopyEW(pcw,GetText(anode));
        inc(i);
      until false;
      pcw:=#0;
      StrDupW(data.tags,pWideChar(@request));

      DestroyNode(root);
    end;
  end;
end;
