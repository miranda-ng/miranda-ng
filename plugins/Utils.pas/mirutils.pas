{$Include compilers.inc}
unit mirutils;

interface

uses windows,m_api;

// icons
function SetButtonIcon(btn:HWND;name:PAnsiChar):HICON;
function RegisterSingleIcon(resname,ilname,descr,group:PAnsiChar):int;

// others

function ConvertFileName(src:PAnsiChar;hContact:TMCONTACT=0):PAnsiChar; overload;
function ConvertFileName(src:PWideChar;hContact:TMCONTACT=0):PWideChar; overload;
function ConvertFileName(src:PAnsiChar;dst:PAnsiChar;hContact:TMCONTACT=0):PAnsiChar; overload;
function ConvertFileName(src:PWideChar;dst:PWideChar;hContact:TMCONTACT=0):PWideChar; overload;

procedure ShowPopupW(text:PWideChar;title:PWideChar=nil);
function GetAddonFileName(prefix,altname,path:PAnsiChar;ext:PAnsiChar):PAnsiChar;
function TranslateA2W(sz:PAnsiChar):PWideChar;
function MirandaCP:integer;

function isVarsInstalled:bool;
function ParseVarString(astr:PAnsiChar;aContact:TMCONTACT=0;extra:PAnsiChar=nil):PAnsiChar; overload;
function ParseVarString(astr:PWideChar;aContact:TMCONTACT=0;extra:PWideChar=nil):PWideChar; overload;
function ShowVarHelp(dlg:HWND;id:integer=0):integer;

function CreateGroupW(name:PWideChar;hContact:TMCONTACT):integer;

function MakeGroupMenu(idxfrom:integer=100):HMENU;
function GetNewGroupName(parent:HWND):PWideChar;

const
  MAX_REDIRECT_RECURSE = 4;

function SendRequest(url:PAnsiChar;rtype:int;args:PAnsiChar=nil;hNetLib:THANDLE=0):PAnsiChar;

function GetFile(url:PAnsiChar;save_file:PAnsiChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool; overload;
// next is just wrapper
function GetFile(url:PWideChar;save_file:PWideChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool; overload;

function GetProxy(hNetLib:THANDLE):PAnsiChar;
function LoadImageURL(url:PAnsiChar;size:integer=0):HBITMAP;

implementation

uses
  Messages,
  dbsettings,//freeimage,
  common,io;

const
  clGroup = 'Group';
// Save / Load contact
const
  opt_cproto   = 'cproto';
  opt_cuid     = 'cuid';
  opt_ischat   = 'ischat';

function SetButtonIcon(btn:HWND;name:PAnsiChar):HICON;
begin
  result:=IcoLib_GetIcon(name,0);
  SendMessage(btn,BM_SETIMAGE,IMAGE_ICON,result);
end;

function ConvertFileName(src:PWideChar;dst:PWideChar;hContact:TMCONTACT=0):PWideChar; overload;
var
  pc:PWideChar;
begin
  result:=dst;
  dst^:=#0;
  if (src<>nil) and (src^<>#0) then
  begin
    if isVarsInstalled then
    begin
      pc:=ParseVarString(src,hContact);
      src:=pc;
    end
    else
      pc:=nil;
    PathToAbsoluteW(src,dst);
    mFreeMem(pc);
  end;
end;

function ConvertFileName(src:PWideChar;hContact:TMCONTACT=0):PWideChar; overload;
var
  buf1:array [0..511] of WideChar;
begin
  if (src<>nil) and (src^<>#0) then
    StrDupW(result,ConvertFileName(src,buf1,hContact))
  else
    result:=nil;
end;

function ConvertFileName(src:PAnsiChar;dst:PAnsiChar;hContact:TMCONTACT=0):PAnsiChar; overload;
var
  pc:PAnsiChar;
begin
  result:=dst;
  dst^:=#0;
  if (src<>nil) and (src^<>#0) then
  begin
    if isVarsInstalled then
    begin
      pc:=ParseVarString(src,hContact);
      src:=pc;
    end
    else
      pc:=nil;
    PathToAbsolute(src,dst);
    mFreeMem(pc);
  end;
end;

function ConvertFileName(src:PAnsiChar;hContact:TMCONTACT=0):PAnsiChar; overload;
var
  buf1:array [0..511] of AnsiChar;
begin
  if (src<>nil) and (src^<>#0) then
    StrDup(result,ConvertFileName(src,buf1,hContact))
  else
    result:=nil;
end;

const
  MirCP:integer=-1;

function MirandaCP:integer;
begin
  if MirCP<0 then
    MirCP:=Langpack_GetDefaultCodePage;
  result:=MirCP;
end;

function isVarsInstalled:bool;
{$IFDEF AllowInline}inline;{$ENDIF}
begin
  result:=ServiceExists(MS_VARS_FORMATSTRING);
end;

function ParseVarString(astr:PAnsiChar;aContact:TMCONTACT=0;extra:PAnsiChar=nil):PAnsiChar;
var
  tfi:TFORMATINFO;
  tmp,pc:PAnsiChar;
begin
  if (astr=nil) or (astr^=#0) then exit;

  pc:=Utils_ReplaceVars(astr);
  astr:=pc;

  if isVarsInstalled then
  begin
    FillChar(tfi,SizeOf(tfi),0);
    with tfi do
    begin
      cbSize       :=SizeOf(TFORMATINFO);
      szFormat.a   :=astr;
      szExtraText.a:=extra;
      hContact     :=aContact;
    end;
    tmp:=pointer(CallService(MS_VARS_FORMATSTRING,wparam(@tfi),0));
    StrDup(result,tmp);
    mir_free(tmp);
  end
  else
  begin
    StrDup(result,astr);
  end;
  mir_free(pc);
end;

function ParseVarString(astr:PWideChar;aContact:TMCONTACT=0;extra:PWideChar=nil):PWideChar;
var
  tfi:TFORMATINFO;
  tmp,pc:PWideChar;
begin
  if (astr=nil) or (astr^=#0) then exit;

  pc:=Utils_ReplaceVarsW(astr);
  astr:=pc;

  if isVarsInstalled then
  begin
    FillChar(tfi,SizeOf(tfi),0);
    with tfi do
    begin
      cbSize       :=SizeOf(TFORMATINFO);
      flags        :=FIF_UNICODE;
      szFormat.w   :=astr;
      szExtraText.w:=extra;
      hContact     :=aContact;
    end;
    tmp:=pointer(CallService(MS_VARS_FORMATSTRING,wparam(@tfi),0));
    StrDupW(result,tmp);
    mir_free(tmp);
  end
  else
  begin
    StrDupW(result,astr);
  end;
  mir_free(pc); // forced!
end;

function ShowVarHelp(dlg:HWND;id:integer=0):integer;
var
  vhi:TVARHELPINFO;
begin
  FillChar(vhi,SizeOf(vhi),0);
  with vhi do
  begin
    cbSize:=SizeOf(vhi);
    if id=0 then
      flags:=VHF_NOINPUTDLG
    else
    begin
      flags   :=VHF_FULLDLG or VHF_SETLASTSUBJECT;
      hwndCtrl:=GetDlgItem(dlg,id);
    end;
  end;
  result:=CallService(MS_VARS_SHOWHELPEX,dlg,lparam(@vhi));
end;

procedure ShowPopupW(text:PWideChar;title:PWideChar=nil);
var
  ppdu:TPOPUPDATAW;
begin
  FillChar(ppdu,SizeOf(TPOPUPDATAW),0);
  StrCopyW(ppdu.lpwzText,text,MAX_SECONDLINE-1);
  if title<>nil then
    StrCopyW(ppdu.lpwzContactName,title,MAX_CONTACTNAME-1)
  else
    ppdu.lpwzContactName[0]:=' ';
  PUAddPopupW(@ppdu,APF_NO_HISTORY);
end;

function TranslateA2W(sz:PAnsiChar):PWideChar;
var
  tmp:PWideChar;
begin
  mGetMem(tmp,(StrLen(sz)+1)*SizeOf(WideChar));
  Result:=TranslateW(FastAnsiToWideBuf(sz,tmp));
  if Result<>tmp then
  begin
    StrDupW(Result,Result);
    mFreeMem(tmp);
  end;
end;

function CheckPath(filename,profilepath,path:PAnsiChar):PAnsiChar;
var
  buf:array [0..511] of AnsiChar;
  f:THANDLE;
  p:PAnsiChar;
begin
  result:=nil;
  if profilepath<>nil then
    StrCopy(buf,profilepath)
  else
    buf[0]:=#0;
  StrCat(buf,filename);
  f:=Reset(buf);
  if f=THANDLE(INVALID_HANDLE_VALUE) then
  begin
    if path<>nil then
    begin
      PathToAbsolute(path,buf);
      p:=StrEnd(buf);
      if p^<>'\' then
      begin
        p^:='\';
        inc(p);
        p^:=#0;
      end;
    end
    else if profilepath=nil then
      exit
    else
      buf[0]:=#0;
    StrCat(buf,filename); //path\prefix+name
    f:=Reset(buf);
  end;
  if f<>THANDLE(INVALID_HANDLE_VALUE) then
  begin
    CloseHandle(f);
    StrDup(result,buf);
  end;
end;

function GetAddonFileName(prefix,altname,path:PAnsiChar;ext:PAnsiChar):PAnsiChar;
var
  profilepath:array [0..511] of AnsiChar;
  altfilename,filename:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  Profile_GetPathA(300,@profilepath);
  p:=StrEnd(profilepath);
  p^:='\'; inc(p);
  p^:=#0;
  filename[0]:=#0;
  altfilename[0]:=#0;
  if prefix<>nil then
  begin
    StrCopy(filename,prefix);
    p:=StrEnd(filename);
    Profile_GetNameA(Sizeof(filename)-integer(p-PAnsiChar(@filename)),p);
    ChangeExt(filename,ext);
    result:=CheckPath(filename,profilepath,path);
  end
  else
    result:=nil;

  if (result=nil) and (altname<>nil) then
  begin
    StrCopy(altfilename,altname);
    ChangeExt(altfilename,ext);
    result:=CheckPath(altfilename,profilepath,path);
  end;
  if result=nil then
  begin
    if filename[0]<>#0 then
      StrCat(profilepath,filename)
    else
      StrCat(profilepath,altfilename);
    StrDup(result,profilepath);
  end;
end;

// Import plugin function adaptation
function CreateGroupW(name:PWideChar;hContact:TMCONTACT):integer;
begin
  if (name=nil) or (name^=#0) then
  begin
    result:=0;
    exit;
  end;

  Clist_GroupCreate(0,name);

  if hContact<>0 then
    DBWriteUnicode(hContact,strCList,clGroup,name);

  result:=1;
end;

function MyStrSort(para1:pointer; para2:pointer):int; cdecl;
begin
  result:=StrCmpW(PWideChar(para1),PWideChar(para2));
end;

function MakeGroupMenu(idxfrom:integer=100):HMENU;
var
  sl:TSortedList;
  i:integer;
  b:array [0..15] of AnsiChar;
  p:PWideChar;
begin
  result:=CreatePopupMenu;
  i:=0;
  AppendMenuW(result,MF_STRING,idxfrom,TranslateW('<Root Group>'));
  AppendMenuW(result,MF_SEPARATOR,0,nil);
  FillChar(sl,SizeOf(sl),0);
  sl.increment:=16;
  sl.sortFunc:=@MyStrSort;
  repeat
    p:=DBReadUnicode(0,'CListGroups',IntToStr(b,i),nil);
    if p=nil then break;
    List_InsertPtr(@sl,p+1);
    inc(i);
  until false;
  inc(idxfrom);
  for i:=0 to sl.realCount-1 do
  begin
    AppendMenuW(result,MF_STRING,idxfrom+i,PWideChar(sl.Items[i]));
    p:=PWideChar(sl.Items[i])-1;
    mFreeMem(p);
  end;
  List_Destroy(@sl);
end;

function GetNewGroupName(parent:HWND):PWideChar;
var
  mmenu:HMENU;
  i:integer;
  buf:array [0..63] of WideChar;
  pt:TPoint;
begin
  result:=nil;
  mmenu:=MakeGroupMenu(100);
  GetCursorPos(pt);
  i:=integer(TrackPopupMenu(mmenu,TPM_RETURNCMD+TPM_NONOTIFY,pt.x,pt.y,0,parent,nil));
  if i>100 then // no root or cancel
  begin
    GetMenuStringW(mmenu,i,buf,HIGH(buf)+1,MF_BYCOMMAND);
    StrDupW(result,buf);
  end;
  DestroyMenu(mmenu);
end;

function SendRequest(url:PAnsiChar;rtype:int;args:PAnsiChar=nil;hNetLib:THANDLE=0):PAnsiChar;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:THANDLE;
  bufLen:int;
  pBuf:PAnsiChar;
  hTmpNetLib:THANDLE;
  nlh:array [0..1] of TNETLIBHTTPHEADER;
  buf:array [0..31] of AnsiChar;
begin
  result:=nil;

  FillChar(req,SizeOf(req),0);
  req.requestType:=rtype;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP or NLHRF_HTTP11;
  if args<>nil then
  begin
    nlh[0].szName :='Content-Type';
    nlh[0].szValue:='application/x-www-form-urlencoded';
    nlh[1].szName :='Content-Length';
    nlh[1].szValue:=IntToStr(buf,StrLen(args));
    req.headers     :=@nlh;
    req.headersCount:=2;
    req.pData       :=args;
    req.dataLength  :=StrLen(args);
  end;

  if hNetLib=0 then
  begin
    FillChar(nlu,SizeOf(nlu),0);
    nlu.flags :=NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
    nlu.szSettingsModule:='dummy';
    hTmpNetLib:=Netlib_RegisterUser(@nlu);
  end
  else
    hTmpNetLib:=hNetLib;

  resp:=Netlib_HttpTransaction(hTmpNetLib,@req);
  if resp<>0 then
  begin
    if Netlib_HttpResult(resp)=200 then
    begin
      pBuf := Netlib_HttpBuffer(resp,bufLen);
      StrDup(result,pBuf,bufLen);
    end
    else
    begin
      result:=PAnsiChar(int_ptr(Netlib_HttpResult(resp) and $0FFF));
    end;
    Netlib_FreeHttpRequest(resp);
  end;

  if (hNetLib=0) and (nlu.flags<>0) then
    Netlib_CloseHandle(hTmpNetLib);
end;

(*
static int __inline NLog(AnsiChar *msg) {
  return CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)msg);
}
*)
function GetFile(url:PAnsiChar; save_file:PAnsiChar; hNetLib:THANDLE=0; recurse_count:integer=0):bool;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp:THANDLE;
  hSaveFile:THANDLE;
  retCode, bufLen:integer;
  pBuf:PAnsiChar;
begin
  result:=false;
  if recurse_count>MAX_REDIRECT_RECURSE then
    exit;
  if (url=nil) or (url^=#0) or (save_file=nil) or (save_file^=#0) then
    exit;

  FillChar(req,SizeOf(req),0);
  req.requestType:=REQUEST_GET;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP or NLHRF_REDIRECT;

  FillChar(nlu,SizeOf(nlu),0);
  if hNetLib=0 then
  begin
    nlu.flags := NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
    nlu.szSettingsModule:='dummy';
    hNetLib:=Netlib_RegisterUser(@nlu);
  end;

  resp:=Netlib_HttpTransaction(hNetLib,@req);
  if resp<>0 then
  begin
    retCode:=Netlib_HttpResult(resp);
    if retCode=200 then
    begin
      hSaveFile:=Rewrite(save_file);
      if hSaveFile<>THANDLE(INVALID_HANDLE_VALUE) then
      begin
        pBuf := Netlib_HttpBuffer(resp,bufLen);
        BlockWrite(hSaveFile,pBuf^,bufLen);
        CloseHandle(hSaveFile);
        result:=true;
      end
    end;
    Netlib_FreeHttpRequest(resp);

    if nlu.flags<>0 then
      Netlib_CloseHandle(hNetLib);
  end;
end;

function GetFile(url:PWideChar;save_file:PWideChar;
                 hNetLib:THANDLE=0;recurse_count:integer=0):bool;
var
  aurl,asave:array [0..MAX_PATH-1] of AnsiChar;
begin
  FastWideToAnsiBuf(url,aurl);
  FastWideToAnsiBuf(save_file,asave);
  result:=GetFile(aurl,asave,hNetLib,0);
end;

function GetProxy(hNetLib:THANDLE):PAnsiChar;
var
  nlus:TNETLIBUSERSETTINGS;
  pc:PAnsiChar;
  proxy:array [0..127] of AnsiChar;
begin
  result:=nil;
  nlus.cbSize:=SizeOf(nlus);
  if Netlib_GetUserSettings(hNetLib,@nlus)<>0 then
  begin
    if nlus.useProxy<>0 then
    begin
      if nlus.proxyType<>PROXYTYPE_IE then
      begin
        pc:=@proxy;
        if nlus.szProxyServer<>nil then
        begin
          if nlus.useProxyAuth<>0 then
          begin
            if nlus.szProxyAuthUser<>nil then
            begin
              pc:=StrCopyE(proxy,nlus.szProxyAuthUser);
              if nlus.szProxyAuthPassword<>nil then
              begin
                pc^:=':'; inc(pc);
                pc:=StrCopyE(pc,nlus.szProxyAuthPassword);
              end;
              pc^:='@';
              inc(pc);
            end;
          end;
          pc:=StrCopyE(pc,nlus.szProxyServer);
          if nlus.wProxyPort<>0 then
          begin
            pc^:=':'; inc(pc);
            IntToStr(pc,nlus.wProxyPort);
          end;
        end;
        StrDup(result,proxy);
      end
      else // use IE proxy
      begin
        mGetMem(result,1);
        result^:=#0;
      end;
    end;
  end;
end;

function LoadImageURL(url:PAnsiChar;size:integer=0):HBITMAP;
var
  nlu:TNETLIBUSER;
  req :TNETLIBHTTPREQUEST;
  resp,hNetLib:THANDLE;
  bufLen:integer;
  pBuf:PAnsiChar;
begin
  result:=0;
  if (url=nil) or (url^=#0) then
    exit;

  FillChar(req,SizeOf(req),0);
  req.requestType:=REQUEST_GET;
  req.szUrl      :=url;
  req.flags      :=NLHRF_NODUMP;

  FillChar(nlu,SizeOf(nlu),0);
  nlu.flags := NUF_HTTPCONNS or NUF_NOHTTPSOPTION or NUF_OUTGOING or NUF_NOOPTIONS;
  nlu.szSettingsModule:='dummy';
  hNetLib:=Netlib_RegisterUser(@nlu);

  resp:=Netlib_HttpTransaction(hNetLib,@req);
  if resp<>0 then
  begin
    if Netlib_HttpResult(resp)=200 then
    begin
      pBuf := Netlib_HttpBuffer(resp,bufLen);
      result := Image_LoadFromMem(pBuf, bufLen, FIF_JPEG);
    end;

    Netlib_FreeHttpRequest(resp);
  end;
  Netlib_CloseHandle(hNetLib);
end;

function RegisterSingleIcon(resname,ilname,descr,group:PAnsiChar):int;
var
  sid:TSKINICONDESC;
begin
  FillChar(sid,SizeOf(TSKINICONDESC),0);
  sid.cx         :=16;
  sid.cy         :=16;
  sid.flags      :=0;
  sid.szSection.a:=group;

  sid.hDefaultIcon   :=LoadImageA(hInstance,resname,IMAGE_ICON,16,16,0);
  sid.pszName        :=ilname;
  sid.szDescription.a:=descr;
  result:=Skin_AddIcon(@sid);
  DestroyIcon(sid.hDefaultIcon);
end;

end.
