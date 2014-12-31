{}
//type  tDigest = array [0..15] of byte;
(*
const
  client_id  = 'wat';//'wat'; 'tst'
  client_ver = '1.0';
  api_key    = '51f5d25159da31b0814609c3a12900e2';
*)
{$include i_cookies.inc}

const API_URL = 'http://api.myshows.me/';

const
  defreq = API_URL+'profile/login?login=<login>&password=<password>';

procedure ShowError(code:integer);
var
  buf:array [0..511] of WideChar;
  ppc:pWideChar;
begin
  case code of
    401: begin // “ребуетс€ авторизаци€
      ppc:='Authorization required'; 
    end;
    403: begin // »м€ пользовател€ или пароль не подошли
      ppc:='User name of password wrong';
    end;
    404: begin // Ќе найдено, неправильные параметры
      ppc:='Not found / wrong parameters';
    end;
    500: begin // параметр запроса отсутствует
      ppc:='Wrong query parameters';
    end;
  else
    ppc:='something wrong!';
  end;
  StrCopyW(StrCopyEW(buf,'MyShows: '),TranslateW(ppc));
  
  if ServiceExists(MS_POPUP_SHOWMESSAGEW)<>0 then
    CallService(MS_POPUP_SHOWMESSAGEW,TWPARAM(@buf),SM_WARNING)
  else
    MessageBoxW(0,@buf,'ERROR',MB_ICONERROR)
end;

function GetMD5Str(digest:TMD5Hash; buf:pAnsiChar):PAnsiChar;
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

function Handshake(login, password:PAnsiChar):boolean;
var
  buf:array [0..32] of AnsiChar;
  digest:TMD5Hash;
  request:array [0..511] of AnsiChar;
  res:pAnsiChar;
  stat:mir_md5_state_t;
begin
  result:=false;
  GetMD5Str(GetMD5(password,StrLen(password),digest),buf);
  mir_md5_init(@stat);
  mir_md5_append(@stat,@buf,32);
  mir_md5_finish(@stat,digest);
  StrCopy(request,defreq);
  StrReplace(request,'<login>'   ,login);
  StrReplace(request,'<password>',buf);

  res:=SendRequestCookies(request,false);
//  res:=SendRequest(request,REQUEST_GET);
  if res<>nil then
  begin
    if uint_ptr(res)<$0FFF then
    begin
      ShowError(int_ptr(res));
    end
    else
    begin
      result:=true;
      mFreeMem(res);
    end;
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

function SendMSRequest(request:pAnsiChar;doShowError:boolean):boolean;
var
  res:pAnsiChar;
begin
  result:=true;
  res:=SendRequestCookies(request,true);
  if (uint_ptr(res)<>200) and (uint_ptr(res)<$0FFF) then
  begin
//!!    if int_ptr(res)=401 then
    begin
      Handshake(msh_login,msh_password);

      res:=SendRequestCookies(request,true);
    end;
    if (uint_ptr(res)<$0FFF) then
      if (uint_ptr(res)<>200) and doShowError then
      begin
        ShowError(int_ptr(res));
        result:=false;
      end;
  end;
end;

function Scrobble(show:boolean):boolean;
var
  si:pSongInfoA;
  buf:array [0..511] of AnsiChar;
//  bufw:array [0..511] of WideChar;
  res,pc:PAnsiChar;
  {img,}epId:pAnsiChar;
  shId:pWideChar;
//  imgw:pWideChar;
  jn,jroot:PJSONNODE;
begin
  result:=false;

  si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UTF8,0));
  Encode(buf,si.mfile);
  pc:=Extract(buf,true);
  
  // Episode search by filename
  StrCopy(StrCopyE(buf,API_URL+'shows/search/file/?q='),pc);
  mFreeMem(pc);
  res:=SendRequest(buf,REQUEST_GET);
  if uint_ptr(res)>$0FFF then
  begin
    jroot:=json_parse(res);

    jn:=json_get(jroot,'show');
    shId:=json_as_string(json_get(jn,'id'));

    jn:=json_get(jn,'episodes');
    epId:=json_name(json_at(jn,0));
  end
  else
  begin
    if show and (res<>nil) then
      ShowError(int_ptr(res));
    exit;
  end;

  // Show mark as "watching"
  pc:=StrCopyE(buf,API_URL+'profile/shows/');
  FastWideToAnsiBuf(shId,pc);
  StrCat(pc,'/watching');
  if SendMSRequest(buf,show) then
  begin
    // Episode check
    StrCopy(StrCopyE(buf,API_URL+'profile/episodes/check/'),epId);
  //  StrCopy(request,API_URL+'profile/shows/');
    if SendMSRequest(buf,show) then
    begin
{
      if si.cover=nil then
      begin
        jn:=json_get(jroot,'show');
        img:=json_as_string(json_get(jn,'image'));
        si:=pointer(CallService(MS_WAT_RETURNGLOBAL,WAT_INF_UNICODE,0));
        FastAnsiToWide(img,pSongInfoW(si)^.cover);
        json_free(img);
      end;
}        
      //!! add option to show it??
      if ServiceExists(MS_POPUP_SHOWMESSAGEW)<>0 then
      begin
        json_free(shId);
        json_free(epId);

        jn:=json_get(jroot,'show');
        shId:=json_as_string(json_get(jn,'title'));

        jn:=json_get(jn,'episodes');
        pWideChar(epId):=json_as_string(json_get(jn,'title'));

        mGetMem(pc,1024);
        StrCopyW(
          StrCopyEW(
            StrCopyEW(
              StrCopyEW(
                StrCopyEW(pWideChar(pc),'Show "'),
              shId),
            '"'#13#10'episode "'),
          pWideChar(epId)),
        '" checked');
        CallService(MS_POPUP_SHOWMESSAGEW,TWPARAM(pc),SM_NOTIFY);
        mFreeMem(pc);
      end;
      result:=true;
    end;
  end;
  json_free(shId);
  json_free(epId);

  json_delete(jroot);
end;

