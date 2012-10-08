{}
const
  cookies:pAnsiChar=nil;

function ExtractCookies(resp:PNETLIBHTTPREQUEST):integer;
var
  cnt,len:integer;
  p,pc:pAnsiChar;
begin
  result:=0;

  mFreeMem(cookies);
  mGetMem(cookies,1024);

  pc:=cookies;
  for cnt:=0 to resp^.headersCount-1 do
  begin
    with resp^.headers[cnt] do
      if StrCmp(szName,'Set-Cookie')=0 then
      begin
        len:=0;
        p:=szValue;
        while (p^<>#0) and (p^<>';') do
        begin
          inc(p);
          inc(len);
        end;
        if pc<>cookies then
        begin
          pc^:=';'; inc(pc);
          pc^:=' '; inc(pc);
        end;
        pc:=StrCopyE(pc,szValue,len);
        inc(result);
      end;
  end;
end;

function SendRequestCookies(url:PAnsiChar;useCookies:boolean):pAnsiChar;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:PNETLIBHTTPREQUEST;
  hTmpNetLib:THANDLE;
  nlh:array [0..10] of TNETLIBHTTPHEADER;
begin
  result:=nil;

  FillChar(req,SizeOf(req),0);
  req.cbSize     :=NETLIBHTTPREQUEST_V1_SIZE;//SizeOf(req);
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
  nlu.cbSize          :=SizeOf(nlu);
  nlu.flags           :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
  nlu.szSettingsModule:='dummy';
  hTmpNetLib:=CallService(MS_NETLIB_REGISTERUSER,0,lparam(@nlu));

  resp:=pointer(CallService(MS_NETLIB_HTTPTRANSACTION,hTmpNetLib,lparam(@req)));

  if resp<>nil then
  begin
    if resp^.resultCode=200 then
    begin
      if resp.pData<>nil then
        StrDup(result,resp.pData,resp.dataLength)
      else
        result:=PAnsiChar(200);
      if not useCookies then
        ExtractCookies(resp);
    end
    else
    begin
      result:=pAnsiChar(int_ptr(resp^.resultCode and $0FFF));
    end;
    CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,lparam(resp));
  end;

  CallService(MS_NETLIB_CLOSEHANDLE,hTmpNetLib,0);
end;
