{
  in dialog: sends CBN_EDITCHANGE for changes
}
unit srvblock;

interface

uses
  windows,
  awkservices;

//----- Service dialog -----

function  CreateServiceBlock(parent:HWND;x,y,width,height:integer;flags:dword=0):HWND;
procedure ClearServiceBlock (Dialog:HWND);
procedure SetServiceListMode(Dialog:HWND;mode:integer);

function SetSrvBlockValue(Dialog:HWND;const value:tServiceValue):boolean;
function GetSrvBlockValue(Dialog:HWND;var   value:tServiceValue):boolean;
// service setting for templates
procedure SetSrvBlockService(Dialog:HWND; service:PAnsiChar);
function  GetSrvBlockService(Dialog:HWND):PAnsiChar;

//----- ServiceValue functions -----

procedure CopyServiceValue (var   dst :tServiceValue; const src:tServiceValue);
procedure ClearServiceValue(var   data:tServiceValue);
procedure SaveServiceValue (const data:tServiceValue; module,setting:PAnsiChar);
procedure LoadServiceValue (var   data:tServiceValue; module,setting:PAnsiChar);

//----- Service execute -----

function ExecuteService(const service:tServiceValue; var data:tSubstData):boolean;


implementation

uses
  messages,
  common, m_api, syswin,
  wrapper,Editwrapper,
  dbsettings,mApiCardM,
  mirutils,mircontacts,
  sparam,strans;

//----- Service dialog -----

const
  IDC_S_SERVICE  = 2040;
  IDC_C_SERVICE  = 2041;
  IDC_CLOSE_WPAR = 2042;
  IDC_CLOSE_LPAR = 2043;
  IDC_CLOSE_RES  = 2044;

function GetApiCard(Dialog:HWND):tmApiCard;
begin
  result:=tmApiCard(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_S_SERVICE),GWLP_USERDATA));
end;

function GetWPar(Dialog:HWND):HWND;
begin
  result:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_CLOSE_WPAR),GWLP_USERDATA);
end;

function GetLPar(Dialog:HWND):HWND;
begin
  result:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_CLOSE_LPAR),GWLP_USERDATA);
end;

function GetRes(Dialog:HWND):HWND;
begin
  result:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_CLOSE_RES),GWLP_USERDATA);
end;

procedure ShowBlock(Dialog:HWND;id:integer);
var
  wpar ,lpar ,res:HWND;
  wparb,lparb,resb:HWND;
  wnd,wndb:HWND;
  rc,rc1:TRECT;
  pt:TPOINT;
begin
  // buttons
  wpar:=GetDlgItem(Dialog,IDC_CLOSE_WPAR);
  lpar:=GetDlgItem(Dialog,IDC_CLOSE_LPAR);
  res :=GetDlgItem(Dialog,IDC_CLOSE_RES);

  // blocks
  wparb:=GetWindowLongPtrW(wpar,GWLP_USERDATA);
  lparb:=GetWindowLongPtrW(lpar,GWLP_USERDATA);
  resb :=GetWindowLongPtrW(res ,GWLP_USERDATA);

  if id=0 then
  begin
    ShowWindow(wparb,SW_HIDE);
    ShowWindow(lparb,SW_HIDE);
    ShowWindow(resb ,SW_HIDE);
    exit;
  end;

  // starting point of coords
  GetWindowRect(wpar,rc);
  pt.x:=rc.left;
  pt.y:=rc.bottom;
  ScreenToClient(Dialog,pt);
  // show/hide blocks
  // enable/disable buttons
  // move buttons to new place
  case id of
    IDC_CLOSE_WPAR: begin
      CheckDlgButton(Dialog,IDC_CLOSE_WPAR,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_LPAR,BST_UNCHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_RES ,BST_UNCHECKED);
      EnableWindow(lpar,true);
      EnableWindow(res ,true);
      ShowWindow(lparb,SW_HIDE);
      ShowWindow(resb ,SW_HIDE);
      wnd :=wpar;
      wndb:=wparb;

      GetClientRect(wparb,rc1);
      SetWindowPos(lpar,HWND_TOP,pt.x,pt.y+rc1.bottom+5,0,0,SWP_NOZORDER or SWP_NOSIZE);
      GetClientRect(lpar,rc);
      SetWindowPos(res,HWND_TOP,pt.x,pt.y+rc1.bottom+rc.bottom+10,0,0,SWP_NOZORDER or SWP_NOSIZE);
    end;

    IDC_CLOSE_LPAR: begin
      CheckDlgButton(Dialog,IDC_CLOSE_WPAR,BST_UNCHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_LPAR,BST_CHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_RES ,BST_UNCHECKED);
      EnableWindow(wpar,true);
      EnableWindow(res ,true);
      ShowWindow(wparb,SW_HIDE);
      ShowWindow(resb ,SW_HIDE);
      wnd :=lpar;
      wndb:=lparb;

      SetWindowPos(lpar,HWND_TOP,pt.x,pt.y+5,0,0,SWP_NOZORDER or SWP_NOSIZE);
      GetClientRect(lpar ,rc);
      GetClientRect(lparb,rc1);
      SetWindowPos(res,HWND_TOP,pt.x,pt.y+rc1.bottom+rc.bottom+10,0,0,SWP_NOZORDER or SWP_NOSIZE);
    end;

    IDC_CLOSE_RES: begin
      CheckDlgButton(Dialog,IDC_CLOSE_WPAR,BST_UNCHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_LPAR,BST_UNCHECKED);
      CheckDlgButton(Dialog,IDC_CLOSE_RES ,BST_CHECKED);
      EnableWindow(wpar,true);
      EnableWindow(lpar,true);
      ShowWindow(wparb,SW_HIDE);
      ShowWindow(lparb,SW_HIDE);
      wnd :=res;
      wndb:=resb;

      SetWindowPos(lpar,HWND_TOP,pt.x,pt.y+5,0,0,SWP_NOZORDER or SWP_NOSIZE);
      GetClientRect(lpar,rc);
      SetWindowPos(res,HWND_TOP,pt.x,pt.y+rc.bottom+10,0,0,SWP_NOZORDER or SWP_NOSIZE);
    end;
  else
    wnd:=0;
    wndb:=0;
  end;
  if wnd<>0 then
  begin
    EnableWindow(wnd ,false);
    ShowWindow  (wndb,SW_SHOW);
  end;
end;

procedure ReloadService(Dialog:HWND;srv:PAnsiChar;setvalue:boolean);
var
  pc:PAnsiChar;
  ApiCard:tmApiCard;
  flag:dword;
begin
  ApiCard:=GetApiCard(Dialog);
  ApiCard.Service:=srv;

  pc:=ApiCard.GetParam(true);
  if pc<>nil then
  begin
    FillParam(GetWPar(Dialog),pc);
    mFreeMem(pc);
  end;

  pc:=ApiCard.GetParam(false);
  if pc<>nil then
  begin
    FillParam(GetLPar(Dialog),pc);
    mFreeMem(pc);
  end;

  pc:=ApiCard.ResultType;
  flag:=ACF_TYPE_NUMBER;
  if pc<>nil then
  begin
    if      lstrcmpia(pc,'struct')=0 then flag:=ACF_TYPE_STRUCT
    else if lstrcmpia(pc,'str'   )=0 then flag:=ACF_TYPE_STRING
    else if lstrcmpia(pc,'wide'  )=0 then flag:=ACF_TYPE_UNICODE;
    mFreeMem(pc);
  end;

  SetResultValue(GetRes(Dialog),flag);
end;

procedure FillTemplate(Dialog:HWND);
var
  wnd:HWND;
  buf:array [0..127] of AnsiChar;
begin
  wnd:=GetDlgItem(Dialog,IDC_C_SERVICE);
  SendMessageA(wnd,CB_GETLBTEXT,SendMessage(wnd,CB_GETCURSEL,0,0),tlparam(@buf));
  ReloadService(Dialog,@buf,true);
end;

function DlgServiceProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  proc:pointer;
  pc:PAnsiChar;
  ApiCard:tmApiCard;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      ApiCard:=GetApiCard(Dialog);
      if ApiCard<>nil then
        ApiCard.Free;
    end;

    WM_HELP: begin
      ApiCard:=GetApiCard(Dialog);
      pc:=ApiCard.NameFromList(GetDlgItem(Dialog,IDC_C_SERVICE));
      ApiCard.Service:=pc;
      mFreeMem(pc);
      ApiCard.Show;

      result:=1;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_EDITCHANGE,
        EN_CHANGE: begin
          SendMessage(GetParent(Dialog),WM_COMMAND,CBN_EDITCHANGE shl 16,Dialog);
        end;

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_CLOSE_WPAR,
            IDC_CLOSE_LPAR,
            IDC_CLOSE_RES: ShowBlock(Dialog,loword(wParam));
          else // from parameter and result
            SendMessage(GetParent(Dialog),WM_COMMAND,BN_CLICKED shl 16,Dialog);
          end;
        end;

        CBN_SELCHANGE:  begin
          case loword(wParam) of
            IDC_C_SERVICE: FillTemplate(Dialog);
          end;
        end;
      end;
    end;
  else
    proc:=pointer(GetWindowLongPtrW(Dialog,GWLP_USERDATA));
    result:=CallWindowProc(proc,Dialog,hMessage,wParam,lParam)
  end;
end;

function CreateServiceBlock(parent:HWND;x,y,width,height:integer;flags:dword=0):HWND;
var
  hf:HFONT;
  ctrl,wnd,srv,srvs:HWND;
  proc:pointer;
  ApiCard:tmApiCard;
  rc,rc1:TRECT;
  dx,dy:integer;
  ux,uy:integer;
  h,bs:integer;
begin
  hf:=SendMessageW(parent,WM_GETFONT,0,0);
  GetUnitSize(parent,ux,uy);

  // let think what x,y and width is dialog-related
  if height=0 then
    h:=100
  else
    h:=height;
  SetRect(rc,x,y,x+width,y+h);
  dx:=rc.right-rc.left;

  result:=CreateWindowExW(WS_EX_CONTROLPARENT,'STATIC',nil,WS_CHILD+WS_VISIBLE,
          x,y,dx,rc.bottom-rc.top, parent,0,hInstance,nil);
  proc:=pointer(SetWindowLongPtrW(result,GWLP_WNDPROC,long_ptr(@DlgServiceProc)));
  SetWindowLongPtrW(result,GWLP_USERDATA,long_ptr(proc));
  SendMessageW(result,WM_SETFONT,hf,0);

  dy:=0;
  // Service label
  rc.bottom:=12*uy div 8;
  srvs:=CreateWindowW('STATIC','Service:',WS_CHILD+WS_VISIBLE+SS_CENTERIMAGE+SS_LEFT,
        0,dy,dx,rc.bottom, result,IDC_S_SERVICE,hInstance,nil);
  SendMessageW(srvs,WM_SETFONT,hf,0);
  inc(dy,rc.bottom+2);

  // Service name combobox
  rc.bottom:=14*uy div 8;
  srv:=CreateWindowW('COMBOBOX',nil,WS_CHILD+WS_VISIBLE+WS_VSCROLL+CBS_DROPDOWN+CBS_AUTOHSCROLL,
        0,dy,dx,76, result,IDC_C_SERVICE,hInstance,nil);
  SendMessageW(srv,WM_SETFONT,hf,0);
  inc(dy,rc.bottom+2);
  MakeEditField(result,IDC_C_SERVICE);

  if (flags and ACF_BLOCK_EXPAND)<>0 then
    bs:=WS_CHILD+BS_AUTOCHECKBOX+BS_PUSHLIKE
  else
    bs:=WS_CHILD+WS_VISIBLE+BS_AUTOCHECKBOX+BS_PUSHLIKE;

  // wParam button+block
  rc.bottom:=11*uy div 8;
  ctrl:=CreateWindowW('BUTTON','wParam',bs,
        0,dy,dx,rc.bottom, result,IDC_CLOSE_WPAR,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  if (flags and ACF_BLOCK_EXPAND)=0 then
    inc(dy,rc.bottom+4);

  wnd:=CreateParamBlock(result,0,dy,dx,flags);
  SetWindowLongPtrW(ctrl,GWLP_USERDATA,wnd);
  SetParamLabel(wnd,'wParam');
  GetClientRect(wnd,rc1);
  if (flags and ACF_BLOCK_EXPAND)<>0 then
    inc(dy,rc1.bottom+8);

  // lParam button+block
  ctrl:=CreateWindowW('BUTTON','lParam',bs,
        0,dy,dx,rc.bottom, result,IDC_CLOSE_LPAR,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  if (flags and ACF_BLOCK_EXPAND)=0 then
    inc(dy,rc.bottom+4);

  wnd:=CreateParamBlock(result,0,dy,dx,flags);
  SetWindowLongPtrW(ctrl,GWLP_USERDATA,wnd);
  SetParamLabel(wnd,'lParam');
  if (flags and ACF_BLOCK_EXPAND)<>0 then
    inc(dy,rc1.bottom+8);

  // result button+block
  ctrl:=CreateWindowW('BUTTON','Result',bs,
        0,dy,dx,rc.bottom, result,IDC_CLOSE_RES,hInstance,nil);
  SendMessageW(ctrl,WM_SETFONT,hf,0);
  if (flags and ACF_BLOCK_EXPAND)=0 then
    inc(dy,rc.bottom+4);

  wnd:=CreateResultBlock(result,0,dy,dx,flags);
  SetWindowLongPtrW(ctrl,GWLP_USERDATA,wnd);
  GetClientRect(wnd,rc);

  // autoresize panel
  if height=0 then
  begin
    if (flags and ACF_BLOCK_EXPAND)=0 then
    begin
      if rc1.bottom>rc.bottom then
        h:=rc1.bottom
      else
        h:=rc.bottom;
    end
    else
    begin
      h:=rc.bottom;
    end;
    MoveWindow(result,x,y,dx,dy+h,false);
  end;

  // additional
  ApiCard:=CreateServiceCard(result);
  ApiCard.FillList(srv);
  SetWindowLongPtrW(srvs,GWLP_USERDATA,long_ptr(ApiCard));

  if (flags and ACF_BLOCK_EXPAND)=0 then
    ShowBlock(result,IDC_CLOSE_WPAR);
end;

procedure ClearServiceBlock(Dialog:HWND);
begin
  if Dialog=0 then
    exit;

  SetDlgItemTextA(Dialog,IDC_C_SERVICE,'');
  SetEditFlags(GetDlgItem(Dialog,IDC_C_SERVICE),EF_SCRIPT,0);

  SetParamValue (GetWPar(Dialog),ACF_TYPE_NUMBER,nil);
  SetParamValue (GetLPar(Dialog),ACF_TYPE_NUMBER,nil);
  SetResultValue(GetRes (Dialog),ACF_TYPE_NUMBER);
end;

procedure SetServiceListMode(Dialog:HWND;mode:integer);
var
  ApiCard:tmApiCard;
begin
  if Dialog=0 then
    exit;

  ApiCard:=GetApiCard(Dialog);
  ApiCard.FillList(GetDlgItem(Dialog,IDC_C_SERVICE),mode);
end;

function SetSrvBlockValue(Dialog:HWND;const value:tServiceValue):boolean;
begin
  if Dialog=0 then
  begin
    result:=false;
    exit;
  end;

  result:=true;

  if CB_SelectData(Dialog,IDC_C_SERVICE,Hash(value.service,StrLen(value.service)))<>CB_ERR then
    ReloadService(Dialog,value.service,false)
  else
    SetDlgItemTextA(Dialog,IDC_C_SERVICE,value.service);

  SetEditFlags(GetDlgItem(Dialog,IDC_C_SERVICE),EF_SCRIPT,
        ord((value.flags and ACF_FLAG_SCRIPT)<>0));

  SetParamValue (GetWPar(Dialog),value.w_flags,value.wparam);
  SetParamValue (GetLPar(Dialog),value.l_flags,value.lparam);
  SetResultValue(GetRes (Dialog),value.flags);
end;

function GetSrvBlockValue(Dialog:HWND;var value:tServiceValue):boolean;
var
  ApiCard:tmApiCard;
begin
  if Dialog=0 then
  begin
    result:=false;
    exit;
  end;

  result:=true;

  ApiCard:=GetApiCard(Dialog);
  value.service:=ApiCard.NameFromList(GetDlgItem(Dialog,IDC_C_SERVICE));

  GetParamValue(GetWPar(Dialog),value.w_flags,value.wparam);
  GetParamValue(GetLPar(Dialog),value.l_flags,value.lparam);
  value.flags:=GetResultValue(GetRes(Dialog));

  if (GetEditFlags(Dialog,IDC_C_SERVICE) and EF_SCRIPT)<>0 then
    value.flags:=value.flags or ACF_FLAG_SCRIPT;
end;

procedure SetSrvBlockService(Dialog:HWND; service:PAnsiChar);
begin
  if Dialog=0 then
    exit;

  ReloadService(Dialog,service,true);
end;

function GetSrvBlockService(Dialog:HWND):PAnsiChar;
begin
  if Dialog=0 then
  begin
    result:=nil;
    exit;
  end;

  result:=GetDlgText(Dialog,IDC_C_SERVICE);
end;

//----- ServiceValue functions -----

procedure CopyServiceValue(var dst:tServiceValue; const src:tServiceValue);
begin
  move(src,dst,SizeOf(tServiceValue));
  StrDup(dst.service,dst.service);

  case dst.w_flags of
    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: StrDupW(pWideChar(dst.wparam),pWideChar(dst.wparam));
    ACF_TYPE_STRUCT : StrDup (pAnsiChar(dst.wparam),pAnsiChar(dst.wparam));
  end;

  case dst.l_flags of
    ACF_TYPE_NUMBER,
    ACF_TYPE_STRING,
    ACF_TYPE_UNICODE: StrDupW(pWideChar(dst.lparam),pWideChar(dst.lparam));
    ACF_TYPE_STRUCT : StrDup (pAnsiChar(dst.lparam),pAnsiChar(dst.lparam));
  end;
end;

const
  iosection:PAnsiChar = '/service/';
  ioflags  :PAnsiChar = 'flags';
  ioservice:PAnsiChar = 'service';
  iowparam :PAnsiChar = 'wparam';
  iolparam :PAnsiChar = 'lparam';

procedure SaveServiceValue(const data:tServiceValue; module,setting:PAnsiChar);
var
  buf:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  p:=StrCopyE(StrCopyE(buf,setting),iosection);
  StrCopy(p,ioflags);   DBWriteDWord  (0,module,buf,data.flags);
  StrCopy(p,ioservice); DBWriteString (0,module,buf,data.service);
  StrCopy(p,iowparam);  SaveParamValue(data.w_flags,data.wparam,module,buf);
  StrCopy(p,iolparam);  SaveParamValue(data.l_flags,data.lparam,module,buf);
end;

procedure LoadServiceValue(var data:tServiceValue; module,setting:PAnsiChar);
var
  buf:array [0..127] of AnsiChar;
  p:PAnsiChar;
begin
  p:=StrCopyE(StrCopyE(buf,setting),iosection);
  StrCopy(p,ioflags);   data.flags  :=DBReadDWord (0,module,buf);
  StrCopy(p,ioservice); data.service:=DBReadString(0,module,buf);
  StrCopy(p,iowparam);  LoadParamValue(data.w_flags,data.wparam,module,buf);
  StrCopy(p,iolparam);  LoadParamValue(data.l_flags,data.lparam,module,buf);
end;

procedure ClearServiceValue(var data:tServiceValue);
begin
  mFreeMem(data.service);

  ClearParam(data.w_flags,data.wparam);
  ClearParam(data.l_flags,data.lparam);

  FillChar(data,SizeOf(tServiceValue),0);
end;

//----- Service execute -----

const
  protostr='<proto>';

function ExecuteService(const service:tServiceValue; var data:tSubstData):boolean;
var
  buf:array [0..255] of AnsiChar;
  lservice:PAnsiChar;
  lwparam,llparam:TLPARAM;
  wp,lp:PWideChar;
  res:int_ptr;
begin
  result:=false;

  // Service name processing
  if (service.flags and ACF_FLAG_SCRIPT)<>0 then
    lservice:=ParseVarString(service.service,data.Parameter)
  else  
    lservice:=service.service;

  StrCopy(buf,lservice);

  if (service.flags and ACF_FLAG_SCRIPT)<>0 then
    mFreeMem(lservice);

  if StrPos(buf,protostr)<>nil then
    if CallService(MS_DB_CONTACT_IS,data.Parameter,0)<>0 then
      StrReplace(buf,protostr,GetContactProtoAcc(data.Parameter))
    else
      Exit;

  if ServiceExists(buf)<>0 then
  begin
    result:=true;

    lwparam:=PrepareParameter(service.w_flags,TLPARAM(service.wparam),data);
    llparam:=PrepareParameter(service.l_flags,TLPARAM(service.lparam),data);

    res:=CallServiceSync(buf,lwparam,llparam);
//    ClearSubstData(data); DO NOT CLEAR!! (consist of input data)

    // result type processing
    case service.flags and ACF_TYPE_MASK of
      ACF_TYPE_STRING,
      ACF_TYPE_UNICODE: begin
        data.ResultType:=ACF_TYPE_UNICODE;

        if (service.flags and ACF_TYPE_MASK)=ACF_TYPE_STRING then
          AnsiToWide(pAnsiChar(res),pWideChar(data.LastResult),MirandaCP)
        else
          StrDupW(pWideChar(data.LastResult),pWideChar(res));

        if (service.flags and ACF_FLAG_FREEMEM)<>0 then // Miranda Memory manager used
          mir_free(pointer(res));
      end;

      ACF_TYPE_NUMBER: begin
        data.LastResult:=res;
        data.ResultType:=ACF_TYPE_NUMBER;
      end;

      ACF_TYPE_STRUCT: begin
        data.ResultType:=ACF_TYPE_UNICODE;

        if (service.w_flags and ACF_TYPE_MASK)=ACF_TYPE_STRUCT then 
        begin
          wp:=GetStructureResult(lwparam);
        end
        else
          wp:=nil;

        lp:=nil;
        if (service.l_flags and ACF_TYPE_MASK)=ACF_TYPE_STRUCT then
        begin
          if wp=nil then
            lp:=GetStructureResult(llparam);
        end;

        if wp<>nil then
          PWideChar(data.LastResult):=wp
        else
          PWideChar(data.LastResult):=lp;
      end;
    end;

    ReleaseParameter(service.w_flags,lwparam);
    ReleaseParameter(service.l_flags,llparam);
  end;
end;

end.
