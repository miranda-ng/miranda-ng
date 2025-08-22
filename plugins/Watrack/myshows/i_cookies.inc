{}
const
  cookies:pAnsiChar=nil;

function SendRequestCookies(url:PAnsiChar;useCookies:boolean):pAnsiChar;
var
  nlu:TNETLIBUSER;
  resp:THANDLE;
  hTmpNetLib:THANDLE;
  bufLen:int;
  pBuf:PAnsiChar;
  nlh:array [0..1] of TNETLIBHTTPHEADER;
  pHeader: PNETLIBHTTPHEADER;
begin
  result:=nil;
  pHeader:=nil;

  if useCookies and (cookies<>nil) then
  begin
    nlh[0].szName :='Cookie';
    nlh[0].szValue:=cookies;
    nlh[1].szName :=0;
    nlh[1].szValue:=0;
    pHeader := @nlh;
  end;

  FillChar(nlu,SizeOf(nlu),0);
  nlu.flags := NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
  nlu.szSettingsModule:='dummy';
  hTmpNetLib:=Netlib_RegisterUser(@nlu);

  resp:=Netlib_HttpRequest(hTmpNetLib, url, REQUEST_GET, NLHRF_NODUMP or NLHRF_HTTP11, 0, pHeader);
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
