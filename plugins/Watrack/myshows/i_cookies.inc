{}
const
  cookies:pAnsiChar=nil;

function SendRequestCookies(url:PAnsiChar;useCookies:boolean):pAnsiChar;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:THANDLE;
  hTmpNetLib:THANDLE;
  bufLen:int;
  pBuf:PAnsiChar;
  nlh:array [0..10] of TNETLIBHTTPHEADER;
begin
  result:=nil;

  FillChar(req,SizeOf(req),0);
  req.requestType:=REQUEST_GET;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP or NLHRF_HTTP11;

  if useCookies and (cookies<>nil) then
  begin
    nlh[0].szName :='Cookie';
    nlh[0].szValue:=cookies;

    req.headers     :=@nlh;
    req.headersCount:=1;
  end;

  FillChar(nlu,SizeOf(nlu),0);
  nlu.flags := NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
  nlu.szSettingsModule:='dummy';
  hTmpNetLib:=Netlib_RegisterUser(@nlu);

  resp:=Netlib_HttpTransaction(hTmpNetLib,@req);
  if resp<>0 then
  begin
    if Netlib_HttpResult(resp)=200 then
    begin
      pBuf := Netlib_HttpBuffer(resp,bufLen);
      if pBuf<>nil then
        StrDup(result,pBuf,bufLen)
      else
        result:=PAnsiChar(200);
      if not useCookies then
        cookies := Netlib_HttpCookies(resp);
    end
    else
    begin
      result:=pAnsiChar(int_ptr(Netlib_HttpResult(resp) and $0FFF));
    end;
    Netlib_FreeHttpRequest(resp);
  end;

  Netlib_CloseHandle(hTmpNetLib);
end;
