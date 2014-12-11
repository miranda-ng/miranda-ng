unit iac_call;

interface

implementation

uses
  windows, messages, commctrl,
  sparam,
  common, wrapper, memini,
  iac_global, global,
  m_api, dbsettings, mirutils;

{$include i_cnst_call.inc}
{$resource iac_call.res}

const
  MaxArgCount   = 8;
  MaxArgNameLen = 32;

const
  API_FUNCTION_FILE:pAnsiChar = 'plugins\services.ini';
  namespace = 'Function';

const
  opt_dllname  = 'dllname';
  opt_funcname = 'funcname';
  opt_argcount = 'argcount';
  opt_argf     = 'argtype';
  opt_argn     = 'argname';
  opt_argv     = 'argvalue';

type
  tCallAction = class(tBaseAction)
  private
    dllname:pAnsiChar;
    funcname:pAnsiChar;

    argcount:integer;
    argf:array [0..MaxArgCount-1] of dword;
    argv:array [0..MaxArgCount-1] of uint_ptr;
    argn:array [0..MaxArgCount-1,0..MaxArgNameLen-1] of AnsiChar;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

var
  storage:pointer;

const
  ACF_CDECL       = $00000001;
  ACF_CACHE_NOW   = $00000002;
  ACF_CACHE_MACRO = $00000004;

type
  sp0 = function:int_ptr; stdcall;
  sp1 = function(arg1:int_ptr):int_ptr; stdcall;
  sp2 = function(arg1,arg2:uint_ptr):int_ptr; stdcall;
  sp3 = function(arg1,arg2,arg3:uint_ptr):int_ptr; stdcall;
  sp4 = function(arg1,arg2,arg3,arg4:uint_ptr):int_ptr; stdcall;
  sp5 = function(arg1,arg2,arg3,arg4,arg5:uint_ptr):int_ptr; stdcall;
  sp6 = function(arg1,arg2,arg3,arg4,arg5,arg6:uint_ptr):int_ptr; stdcall;
  sp7 = function(arg1,arg2,arg3,arg4,arg5,arg6,arg7:uint_ptr):int_ptr; stdcall;
  sp8 = function(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8:uint_ptr):int_ptr; stdcall;

  cp0 = function:int_ptr; cdecl;
  cp1 = function(arg1:uint_ptr):int_ptr; cdecl;
  cp2 = function(arg1,arg2:uint_ptr):int_ptr; cdecl;
  cp3 = function(arg1,arg2,arg3:uint_ptr):int_ptr; cdecl;
  cp4 = function(arg1,arg2,arg3,arg4:uint_ptr):int_ptr; cdecl;
  cp5 = function(arg1,arg2,arg3,arg4,arg5:uint_ptr):int_ptr; cdecl;
  cp6 = function(arg1,arg2,arg3,arg4,arg5,arg6:uint_ptr):int_ptr; cdecl;
  cp7 = function(arg1,arg2,arg3,arg4,arg5,arg6,arg7:uint_ptr):int_ptr; cdecl;
  cp8 = function(arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8:uint_ptr):int_ptr; cdecl;

//----- Support functions -----

//----- Object realization -----

constructor tCallAction.Create(uid:dword);
begin
  inherited Create(uid);

  argcount:=0;
  dllname :=nil;
  funcname:=nil;
end;

destructor tCallAction.Destroy;
begin
  mFreeMem(dllname);
  mFreeMem(funcname);

  inherited Destroy;
end;
{
function tCallAction.Clone:tBaseAction;
begin
  result:=.Create(0);
  Duplicate(result);

end;
}
function tCallAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  hDLL:THANDLE;
  pp:pointer;
  res:LRESULT;
  largv:array [0..MaxArgCount-1] of uint_ptr;
  i:integer;
  loaded:bool;
begin
  result:=0;
  if (dllname =nil) or (dllname^ =#0) or
     (funcname=nil) or (funcname^=#0) then
  begin
    // clear last result?
    exit;
  end;

  loaded:=false;
  hDLL:=GetModuleHandleA(dllname);
  if hDLL=0 then
  begin
    loaded:=true;
    hDLL:=LoadLibraryA(dllname);
  end;
//  hDLL:=GetDllHandle(dllname);
  if hDLL<>0 then
  begin
    pp:=GetProcAddress(hDLL,funcname);
    if pp<>nil then
    begin
      // prepare arguments?
      for i:=0 to argcount-1 do
      begin
        largv[i]:=argv[i];
//!!        PreProcess(argf[i],LPARAM(largv[i]),WorkData);
      end;
      // call function
      case argcount of
        0: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp0(pp)
          else
            res:=sp0(pp);
        end;
        1: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp1(pp)(largv[0])
          else
            res:=sp1(pp)(largv[0]);
        end;
        2: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp2(pp)(largv[0],largv[1])
          else
            res:=sp2(pp)(largv[0],largv[1]);
        end;
        3: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp3(pp)(largv[0],largv[1],largv[2])
          else
            res:=sp3(pp)(largv[0],largv[1],largv[2]);
        end;
        4: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp4(pp)(largv[0],largv[1],largv[2],largv[3])
          else
            res:=sp4(pp)(largv[0],largv[1],largv[2],largv[3]);
        end;
        5: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp5(pp)(largv[0],largv[1],largv[2],largv[3],largv[4])
          else
            res:=sp5(pp)(largv[0],largv[1],largv[2],largv[3],largv[4]);
        end;
        6: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp6(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5])
          else
            res:=sp6(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5]);
        end;
        7: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp7(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5],largv[6])
          else
            res:=sp7(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5],largv[6]);
        end;
        8: begin
          if (flags and ACF_CDECL)<>0 then
            res:=cp8(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5],largv[6],largv[7])
          else
            res:=sp8(pp)(largv[0],largv[1],largv[2],largv[3],largv[4],largv[5],largv[6],largv[7]);
        end;
      end;

      ClearResult(WorkData);
    // result type processing
      if (flags and (ACF_RSTRING or ACF_UNICODE))<>0 then
      begin
        if (flags and ACF_RUNICODE)=0 then
          AnsiToWide(pAnsiChar(res),pWideChar(WorkData.LastResult),MirandaCP)
        else
          StrDupW(pWideChar(WorkData.LastResult),pWideChar(res));
        WorkData.ResultType:=rtWide;

        if (flags and ACF_RFREEMEM)<>0 then
          mFreeMem(pAnsiChar(res)); //?? Miranda MM??
      end
      else if (flags and ACF_RSTRUCT)=0 then
      begin
        WorkData.ResultType:=rtInt
{!!!!
      end
      else if (flags and ACF_RSTRUCT)<>0 then
      begin
        PostProcess(flags ,lwparam,WorkData);
        PostProcess(flags2,llparam,WorkData);
}
      end;
      
    end; 
//    FreeDllHandle(hDLL);
    if loaded then
      FreeLibrary(hDLL);
  end;

end;

procedure LoadParam(section:PAnsiChar;flags:dword; var param:pointer);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if (flags and ACF_STRUCT)<>0 then
      param:=DBReadUTF8(0,DBBranch,section,nil)
    else
      param:=DBReadUnicode(0,DBBranch,section,nil);
  end;
end;

procedure tCallAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc,p,pd,ppd:pAnsiChar;
  i:integer;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_dllname ); dllname :=DBReadString(0,DBBranch,section);
      StrCopy(pc,opt_funcname); funcname:=DBReadString(0,DBBranch,section);
      StrCopy(pc,opt_argcount); argcount:=DBReadByte  (0,DBBranch,section);
      if argcount>0 then
      begin
        pd:=nil;
        for i:=0 to argcount-1 do
        begin
          IntToStr(StrCopyE(pc,opt_argf),i); argf[i]:=DBReadDWord (0,DBBranch,section);
          IntToStr(StrCopyE(pc,opt_argn),i); p      :=DBReadString(0,DBBranch,section);
          if (p=nil) or (p^=#0) then
          begin
            if pd=nil then
              pd:=Translate('Argument');
            ppd:=StrCopyE(argn[i],pd); ppd^:=' '; inc(ppd);
            IntToStr(ppd,i+1);
          end
          else
            StrCopy(argn[i],p);
          mFreeMem(p);
          IntToStr(StrCopyE(pc,opt_argv),i); LoadParam(section,argf[i],pointer(argv[i]));
        end;
      end;
    end;
{
    1: begin
    end;
}
  end;
end;

procedure SaveParam(section:PAnsiChar;flags:dword; param:pointer);
begin
  if (flags and (ACF_CURRENT or ACF_RESULT or ACF_PARAM))=0 then
  begin
    if pointer(param)<>nil then
    begin
      if (flags and ACF_STRUCT)<>0 then
        DBWriteUTF8(0,DBBranch,section,param)
      else
        DBWriteUnicode(0,DBBranch,section,param);
    end;
  end;
end;

procedure tCallAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
  i:integer;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,opt_dllname ); DBWriteString(0,DBBranch,section,dllname);
      StrCopy(pc,opt_funcname); DBWriteString(0,DBBranch,section,funcname);
      StrCopy(pc,opt_argcount); DBWriteByte  (0,DBBranch,section,argcount);
      if argcount>0 then
      begin
        for i:=0 to argcount-1 do
        begin
          IntToStr(StrCopyE(pc,opt_argf),i); DBWriteDWord (0,DBBranch,section,argf[i]);
          IntToStr(StrCopyE(pc,opt_argn),i); DBWriteString(0,DBBranch,section,argn[i]);
          IntToStr(StrCopyE(pc,opt_argv),i); SaveParam(section,argf[i],pointer(argv[i]));
        end;
      end;
    end;
{
    1: begin
    end;
}
  end;
end;

//----- Dialog realization -----

{
  IDC_ARGNUM   userdata = current item (argument number)
  IDC_ARGCOUNT userdata = arguments buffer
  Dialog       userdata = parameter control
}

type
  peditvalues = ^teditvalues;
  teditvalues = record
    argf:array [0..MaxArgCount-1] of dword;
    argv:array [0..MaxArgCount-1] of uint_ptr;
    argn:array [0..MaxArgCount-1,0..MaxArgNameLen-1] of AnsiChar;
  end;

function GetArgumentWindow(Dialog:HWND):HWND;
begin
  result:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_CLOSE_ARG),GWLP_USERDATA);
end;

function GetResultWindow(Dialog:HWND):HWND;
begin
  result:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_CLOSE_RES),GWLP_USERDATA);
end;

function GetEditValues(Dialog:HWND):pointer;
begin
  result:=pointer(GetWindowLongPtrW(GetDlgItem(Dialog,IDC_ARGCOUNT),GWLP_USERDATA));
end;

function ChangeArgNumber(Dialog:HWND;cnt:integer=-1):integer;
var
  wnd,wnd1:HWND;
  tr:bool;
  pe:peditvalues;
  i:integer;
begin
  if cnt<0 then
    cnt:=GetDlgItemInt(Dialog,IDC_ARGCOUNT,tr,false)
//    cnt:=CB_GetData(GetDlgItem(Dialog,IDC_ARGCOUNT))
  else
    SendMessage(GetDlgItem(Dialog,IDC_ARGCOUNT),CB_SETCURSEL,cnt,0);

  wnd :=GetArgumentWindow(Dialog);
  wnd1:=GetDlgItem(Dialog,IDC_ARGNUM);
  SendMessage(wnd1,CB_RESETCONTENT,0,0);
  if cnt=0 then
  begin
    ShowWindow(wnd ,SW_HIDE);
    ShowWindow(wnd1,SW_HIDE);
  end
  else
  begin
    if IsDlgButtonChecked(Dialog,IDC_CLOSE_ARG)<>BST_UNCHECKED then
    begin
      ShowWindow(wnd ,SW_SHOW);
      ShowWindow(wnd1,SW_SHOW);
    end;
    pe:=GetEditValues(Dialog);

    for i:=0 to cnt-1 do
    begin
      InsertString(wnd1,i,@pe^.argn[i]);
    end;
    SendMessage(wnd1,CB_SETCURSEL,0,0);
  end;
  result:=cnt;
end;

procedure FillTemplates(fwnd:HWND);
var
  p,pp,pz:pAnsiChar;
  lptr:pointer;
  i:integer;
begin
  SendMessage(fwnd,CB_RESETCONTENT,0,0);

  p:=GetSectionList(storage,namespace);
  pp:=p;
  i:=0;
  while p^<>#0 do
  begin
    lptr:=SearchSection(storage,p,namespace);
    pz:=StrScan(p,'$');
    if pz=nil then
      pz:=p-1;
    // add function if not found
    if SendMessageA(fwnd,CB_FINDSTRINGEXACT,-1,LParam(pz+1))=CB_ERR then
      CB_AddStrData(fwnd,pz+1,int_ptr(lptr){not so required}, i);

    while p^<>#0 do inc(p); inc(p);
    inc(i);
  end;
  FreeSectionList(pp);
  if i>0 then
  begin
    SendMessage(fwnd,CB_SETCURSEL,0,0);
  end;
end;

procedure SearchDllByFName(flist,dlist:HWND;idx:integer=-1);
var
  pc,p,pp,pz:pAnsiChar;
  lptr:pointer;
  buf:array [0..31] of AnsiChar;
  buf1:array [0..63] of AnsiChar;
  i:integer;
begin
  if idx<0 then
    pc:=GetDlgText(flist,true)
  else
  begin
    pc:=@buf1;
    SendMessageA(flist,CB_GETLBTEXT,idx,LParam(@buf1));
  end;

  SendMessage(dlist,CB_RESETCONTENT,0,0);

  p:=GetSectionList(storage,namespace);
  pp:=p;
  i:=0;
  while p^<>#0 do
  begin
    lptr:=SearchSection(storage,p,namespace);
    pz:=StrScan(p,'$');
    if (pz<>nil) and (StrCmp(pz+1,pc)=0) then
    begin
      // add dll name
      StrCopy(buf,p,pz-p);
      CB_AddStrData(dlist,buf,int_ptr(lptr), i);
      inc(i);
    end;

    while p^<>#0 do inc(p); inc(p);
  end;
  FreeSectionList(pp);
  if idx<0 then
    mFreeMem(pc);

  if i>0 then
    SendMessage(dlist,CB_SETCURSEL,0,0);
end;

procedure FillByTemplate(Dialog:HWND);
var
  buf:array [0..15] of AnsiChar;
  fwnd,dwnd:HWND;
  sect:pointer;
  pe:peditvalues;
  pc,pd,ppd:pAnsiChar;
  sel,cnt,i:integer;
begin
  fwnd:=GetDlgItem(Dialog,IDC_LIST_FUNC);
  dwnd:=GetDlgItem(Dialog,IDC_LIST_DLL);
  // right now, without manual typing, by selecting only
  sect:=nil;
  sel:=SendMessage(dwnd,CB_GETCURSEL,0,0);
  if sel=CB_ERR then
  begin
    sel:=SendMessage(fwnd,CB_GETCURSEL,0,0);
    if sel<>CB_ERR then
      sect:=pointer(CB_GetData(fwnd,sel));
  end
  else
    sect:=pointer(CB_GetData(dwnd,sel));
  if sect=nil then
    exit;

// ClearFields if templated found
  if StrCmp(GetParamSectionStr(sect,'calltype',nil),'cdecl')=0 then
  begin
    CheckDlgButton(Dialog,IDC_CDECL,BST_CHECKED);
    CheckDlgButton(Dialog,IDC_STDCALL,BST_UNCHECKED);
  end
  else
  begin
    CheckDlgButton(Dialog,IDC_CDECL,BST_UNCHECKED);
    CheckDlgButton(Dialog,IDC_STDCALL,BST_CHECKED);
  end;

  cnt:=GetParamSectionInt(sect,'argcount');
  pe:=GetEditValues(Dialog);
  pd:=nil;
  for i:=0 to cnt-1 do
  begin
    IntToStr(StrCopyE(buf,opt_argn),i+1); pc:=GetParamSectionStr(sect,buf);
    if (pc=nil) or (pc^=#0) then
    begin
      if pd=nil then
        pd:=Translate('Argument');
      ppd:=StrCopyE(pe^.argn[i],pd); ppd^:=' '; inc(ppd);
      IntToStr(ppd,i+1);
    end
    else
      StrCopy(pe^.argn[i],pc);

    IntToStr(StrCopyE(buf,opt_argv),i+1);

    mFreeMem(pe^.argv[i]);
    StrDup(pAnsiChar(pe^.argv[i]),GetParamSectionStr(sect,buf,''));
    pe^.argf[i]:=ACF_TEMPLATE;
  end;
  ChangeArgNumber(Dialog,cnt);
  FillParam(GetArgumentWindow(Dialog),pointer(pe^.argv[0]));

end;

function FillDllName(Dialog:HWND;idc:integer):boolean;
var
  pw:array [0..1023] of WideChar;
  ppw:pWideChar;
begin
//  mGetMem(pw,1024*SizeOf(WideChar));
  ppw:=GetDlgText(Dialog,idc);
  result:=ShowDlgW(pw,ppw);
  if result then
  begin
    SetDlgItemTextW(Dialog,idc,pw);
  end;
  mFreeMem(ppw);
//  mFreeMem(pw);
end;

procedure ShowHideBlock(Dialog:HWND;showarg:boolean);
var
  wnd,wnd1:HWND;
  rc:TRECT;
  pt:TPOINT;
  tr:bool;
  arg,res:integer;
begin
  wnd1:=GetArgumentWindow(Dialog);
  if showarg then
  begin
    arg:=SW_SHOW;
    res:=SW_HIDE;
    wnd:=wnd1;
  end
  else
  begin
    arg:=SW_HIDE;
    res:=SW_SHOW;
    wnd:=GetDlgItem(Dialog,IDC_CLOSE_ARG);
  end;

  GetWindowRect(wnd,rc);
  pt.x:=rc.left;
  pt.y:=rc.bottom;
  ScreenToClient(Dialog,pt);
  SetWindowPos(GetDlgItem(Dialog,IDC_CLOSE_RES),
      HWND_TOP,1,pt.y+2,0,0,SWP_NOZORDER or SWP_NOSIZE);

  ShowWindow(GetResultWindow(Dialog),res);


  ShowWindow(GetDlgItem(Dialog,IDC_ARGSTAT ),arg);
  ShowWindow(GetDlgItem(Dialog,IDC_ARGCOUNT),arg);

  if GetDlgItemInt(Dialog,IDC_ARGCOUNT,tr,false)=0 then
//  if CB_GetData(GetDlgItem(Dialog,IDC_ARGCOUNT))=0 then
    arg:=SW_HIDE;

  ShowWindow(GetDlgItem(Dialog,IDC_ARGNUM),arg);
  ShowWindow(wnd1,arg);
end;

procedure MakeDLLCacheList(wnd:HWND);
begin
  SendMessage(wnd,CB_RESETCONTENT,0,0);
  InsertString(wnd,0,'Keep to end');
  InsertString(wnd,1,'Close immediately');
  InsertString(wnd,2,'Close for macro');
  SendMessage(wnd,CB_SETCURSEL,0,0);
end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_CDECL  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_STDCALL,BST_UNCHECKED);
{!!
  CheckDlgButton(Dialog,IDC_DLL_IMM  ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_DLL_MACRO,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_DLL_END  ,BST_UNCHECKED);
}
  ClearParamFields (GetArgumentWindow(Dialog));
  ClearResultFields(GetResultWindow  (Dialog));

  ShowHideBlock(Dialog,true);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  wnd,wnd1:HWND;
  buf:array [0..63] of AnsiChar;
  pc:pAnsiChar;
  pe:peditvalues;
  rc:TRECT;
  pt:TPOINT;
  tr:bool;
  i:integer;
begin
  result:=0;

  case hMessage of
    WM_DESTROY: begin
      pe:=GetEditValues(Dialog);
      for i:=0 to MaxArgCount-1 do
      begin
        //?? depends of copying from edit to action
        ClearParam(pe.argf[i],pe.argv[i]);
      end;
      mFreeMem(pe);

      CloseStorage(storage);
    end;

    WM_INITDIALOG: begin
      //insert additional dialogs
      CheckDlgButton(Dialog,IDC_CLOSE_ARG,BST_CHECKED);
      wnd1:=GetDlgItem(Dialog,IDC_CLOSE_ARG);
      EnableWindow(wnd1,false);
      wnd:=GetDlgItem(Dialog,IDC_ARGNUM); // count offset from
      GetWindowRect(wnd,rc);
      pt.x:=rc.left;
      pt.y:=rc.bottom;
      ScreenToClient(Dialog,pt);
      GetClientRect(wnd1,rc);
      SetWindowLongPtrW(wnd1,GWLP_USERDATA,CreateParamBlock(Dialog,0,pt.y+2,rc.right));

      CheckDlgButton(Dialog,IDC_CLOSE_RES,BST_UNCHECKED);
      wnd:=GetDlgItem(Dialog,IDC_CLOSE_RES); // count offset from initial pos
      GetWindowRect(wnd,rc);
      EnableWindow(wnd,true);
      pt.x:=rc.left;
      pt.y:=rc.bottom;
      ScreenToClient(Dialog,pt);
      wnd1:=CreateResultBlock(Dialog,0,pt.y+2,rc.right-rc.left,ACF_NOVISUAL);
      SetWindowLongPtrW(wnd,GWLP_USERDATA,wnd1);
      ShowWindow(wnd1,SW_HIDE);

      mGetMem(pe,SizeOf(teditvalues));
      FillChar(pe^,SizeOf(teditvalues),0);
      SetWindowLongPtrW(GetDlgItem(Dialog,IDC_ARGCOUNT),GWLP_USERDATA,uint_ptr(pe));

      // function argument amount list
      wnd1:=GetDlgItem(Dialog,IDC_ARGCOUNT);
      SendMessage(wnd1,CB_RESETCONTENT,0,0);
      for i:=0 to MaxArgCount do
      begin
        InsertString(wnd1,i,IntToStr(buf,i));
      end;
      SendMessage(wnd1,CB_SETCURSEL,0,0);

      // function templates
      storage:=OpenStorage(API_FUNCTION_FILE);
      if storage<>nil then
      begin
        wnd :=GetDlgItem(Dialog,IDC_LIST_FUNC);
        FillTemplates(wnd)
      end;

      MakeDLLCacheList(GetDlgItem(Dialog,IDC_DLL_CACHE));

      TranslateDialogDefault(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);

      with tCallAction(lParam) do
      begin

        // set function
        SetDlgItemTextA(Dialog,IDC_LIST_FUNC,funcname);
        // sset dll name (semi-auto)
        SetDlgItemTextA(Dialog,IDC_LIST_DLL ,dllname);
        // set call type  (auto)
        if (flags and ACF_CDECL)<>0 then
          CheckDlgButton(Dialog,IDC_CDECL  ,BST_CHECKED)
        else
          CheckDlgButton(Dialog,IDC_STDCALL,BST_CHECKED);
{!! 
        // set dll cache option
        if (flags and ACF_CACHE_NOW)<>0 then
          CheckDlgButton(Dialog,IDC_DLL_IMM,BST_CHECKED)
        else if (flags and ACF_CACHE_MACRO)<>0 then
          CheckDlgButton(Dialog,IDC_DLL_MACRO,BST_CHECKED)
        else
          CheckDlgButton(Dialog,IDC_DLL_END,BST_CHECKED);
}
        if (flags and ACF_CACHE_NOW)<>0 then
          i:=1
        else if (flags and ACF_CACHE_MACRO)<>0 then
          i:=2
        else
          i:=0;
        SendMessage(GetDlgItem(Dialog,IDC_DLL_CACHE),CB_SETCURSEL,i,0);

        // set argument amount
        // (auto)
        ChangeArgNumber(Dialog,argcount);
        if argcount>0 then
        begin
          // copy action values to edit values
          pe:=GetEditValues(Dialog);
          // need to clear ALL params before? or keep as is?
          // depends of enabled or disabled arg amount combo
          for i:=0 to argcount-1 do
          begin
            StrCopy(pe^.argn[i],argn[i]);
            pe^.argf[i]:=DuplicateParam(argf[i],argv[i],pe^.argv[i]);
          end;

          wnd1:=GetDlgItem(Dialog,IDC_ARGNUM);
          SendMessage(wnd1,CB_RESETCONTENT,0,0);

          for i:=0 to argcount-1 do
          begin
            InsertString(wnd1,i,@pe^.argn[i]);
          end;
          SendMessage(wnd1,CB_SETCURSEL,0,0);

          SetParamValue (GetArgumentWindow(Dialog),argf[0],pointer(argv[0]));
          SetResultValue(GetResultWindow  (Dialog),flags);
        end;

      end;
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
      SetDlgItemTextW(Dialog,IDC_LIST_FUNC,nil);
      SetDlgItemTextW(Dialog,IDC_LIST_DLL ,nil);
      SendMessage(GetDlgItem(Dialog,IDC_LIST_DLL),CB_RESETCONTENT,0,0);
      CheckDlgButton(Dialog,IDC_STDCALL,BST_CHECKED);
//!!      CheckDlgButton(Dialog,IDC_DLL_END,BST_CHECKED);
      SendMessage(GetDlgItem(Dialog,IDC_DLL_CACHE),CB_SETCURSEL,0,0);

      ChangeArgNumber(Dialog,0);

      pe:=GetEditValues(Dialog);

      pc:=StrCopyE(buf,Translate('Argument')); pc^:=' '; inc(pc);
      for i:=0 to MaxArgCount-1 do
      begin
        IntToStr(pc,i+1);
        StrCopy(pe^.argn[i],buf);
        ClearParam(pe.argf[i],pe.argv[i]);
      end;

    end;

    WM_ACT_SAVE: begin
      with tCallAction(lParam) do
      begin
        argcount:=GetDlgItemInt(Dialog,IDC_ARGCOUNT,tr,false);
        if argcount>0 then
        begin
          pe:=GetEditValues(Dialog);
          // save current argument
          i:=GetWindowLongPtrW(GetDlgItem(Dialog,IDC_ARGNUM),GWLP_USERDATA); // current saved
//          i:=CB_GetData(GetDlgItem(Dialog,IDC_ARGNUM)); // current from data
          GetParamValue(GetArgumentWindow(Dialog),pe^.argf[i],pointer(pe^.argv[i]));

          // copy edit values to action values
          for i:=0 to argcount-1 do
          begin
            StrCopy(argn[i],pe^.argn[i]);
            argf[i]:=DuplicateParam(pe^.argf[i],pe^.argv[i],argv[i]);
          end;
        end;

        flags:=flags or GetResultValue(GetResultWindow(Dialog));
        if IsDlgButtonChecked(Dialog,IDC_CDECL)<>BST_UNCHECKED then
          flags:=flags or ACF_CDECL;
{!!
        if IsDlgButtonChecked(Dialog,IDC_DLL_IMM)<>BST_UNCHECKED then
          flags:=flags or ACF_CACHE_NOW
        else if IsDlgButtonChecked(Dialog,IDC_DLL_MACRO)<>BST_UNCHECKED then
          flags:=flags or ACF_CACHE_MACRO;
}
        case SendMessage(GetDlgItem(Dialog,IDC_DLL_CACHE),CB_GETCURSEL,0,0) of
          1: flags:=flags or ACF_CACHE_NOW;
          2: flags:=flags or ACF_CACHE_MACRO;
        end;

        funcname:=GetDlgText(Dialog,IDC_LIST_FUNC,true);
        dllname :=GetDlgText(Dialog,IDC_LIST_DLL ,true);
      
      end;
    end;

    WM_COMMAND: begin
      case wParam shr 16 of

        BN_CLICKED: begin
          case loword(wParam) of
            IDC_DLL: begin
              if not FillDllName(Dialog,IDC_LIST_DLL) then
                exit;
            end;

            IDC_CLOSE_RES: begin
              // close buttons
              EnableWindow(lParam,false);
              EnableWindow(GetDlgItem(Dialog,IDC_CLOSE_ARG),true);
              CheckDlgButton(Dialog,IDC_CLOSE_ARG,BST_UNCHECKED);

              ShowHideBlock(Dialog,false);
              exit;
            end;

            IDC_CLOSE_ARG: begin
              // close buttons
              EnableWindow(lParam,false);
              EnableWindow(GetDlgItem(Dialog,IDC_CLOSE_RES),true);
              CheckDlgButton(Dialog,IDC_CLOSE_RES,BST_UNCHECKED);

              ShowHideBlock(Dialog,true);
              exit;
            end;
          end;
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;

        CBN_EDITCHANGE,
        EN_CHANGE: begin
          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

          if loword(wParam)=IDC_LIST_FUNC then
          begin
            wnd :=GetDlgItem(Dialog,IDC_LIST_FUNC);
            wnd1:=GetDlgItem(Dialog,IDC_LIST_DLL);
            SearchDllByFName(wnd,wnd1);
          end;
        end;

        CBN_SELCHANGE: begin
          case loword(wParam) of
            IDC_LIST_FUNC: begin
            // search in templates
              SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);

              wnd :=GetDlgItem(Dialog,IDC_LIST_FUNC);
              wnd1:=GetDlgItem(Dialog,IDC_LIST_DLL);
              // search this name as part in all ini templates,
              // fill IDC_LIST_DLL combobox
              SearchDllByFName(wnd,wnd1,SendMessage(wnd,CB_GETCURSEL,0,0));
              FillByTemplate(Dialog);
            end;

            IDC_LIST_DLL: begin
              SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
              FillByTemplate(Dialog);
            end;

            IDC_ARGNUM: begin
              pe:=GetEditValues(Dialog);

              wnd:=GetArgumentWindow(Dialog);
              // 1 - save old argument value (save as for dialogs?)
              i:=GetWindowLongPtrW(lParam,GWLP_USERDATA);
              GetParamValue(wnd,pe^.argf[i],pointer(pe^.argv[i]));
              // 2 - set new argument value
              i:=CB_GetData(lParam);
              SetParamValue(wnd,pe^.argf[i],pointer(pe^.argv[i]));

              SetWindowLongPtrW(lParam,GWLP_USERDATA,i);
            end;

            IDC_ARGCOUNT: begin
              ChangeArgNumber(Dialog);
              SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
            end;
          end;
        end;
      end;
    end;

    WM_HELP: begin
      result:=1;
    end;

  end;
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tCallAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTCALL',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Function call';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_CALL';

  ModuleLink :=@vc;
end;

begin
  Init;
end.
