unit iac_chain;

interface

implementation

uses
  windows, messages, commctrl,
  global, iac_global, mirutils, m_api, inouttext,
  dlgshare,lowlevelc,common,dbsettings, wrapper;

{$include i_cnst_chain.inc}
{$resource iac_chain.res}

const
  ACF_BYNAME     = $00000001; // Address action link by name, not Id
  ACF_NOWAIT     = $00000002; // Don't wait execution result, continue
  ACF_KEEPOLD    = $00000004; // Don't change LastResult value
  ACF_SAMETHREAD = $00000008; // Execute in same thread with waiting
const
  ioNoWait     = 'nowait';
  ioKeepOld    = 'keepold';
  ioSameThread = 'samethread';
const
  opt_chain   = 'chain';
  opt_actname = 'actname';
const
  NoChainText:PWideChar = 'not defined';

type
  tChainAction = class(tBaseAction)
  private
    id     :dword;
    actname:pWideChar;
  public
    constructor Create(uid:dword);
    destructor Destroy; override;
//    function  Clone:tBaseAction; override;
    function  DoAction(var WorkData:tWorkData):LRESULT; override;
    procedure Save(node:pointer;fmt:integer); override;
    procedure Load(node:pointer;fmt:integer); override;
  end;

//----- Support functions -----

//----- Object realization -----

constructor tChainAction.Create(uid:dword);
begin
  inherited Create(uid);

  id     :=0;
  actname:=nil;
end;

destructor tChainAction.Destroy;
begin
  if (flags and ACF_BYNAME)<>0 then
    mFreeMem(actname);

  inherited Destroy;
end;
{
function tChainAction.Clone:tBaseAction;
begin
  result:=tChainAction.Create(0);
  Duplicate(result);

  tChainAction(result).id:=id;
  StrDupW(tChainAction(result).actname,actname);
end;
}
function tChainAction.DoAction(var WorkData:tWorkData):LRESULT;
var
  params:tAct_Param;
  res:int_ptr;
begin
  result:=0;

  if (flags and ACF_BYNAME)<>0 then
  begin
    params.flags:=ACTP_BYNAME;
    params.Id   :=uint_ptr(actname);
  end
  else
  begin
    params.flags:=0;
    params.Id   :=id;
  end;
  if (flags and ACF_SAMETHREAD)<>0 then
    params.flags:=params.flags or ACTP_SAMETHREAD
  else if (flags and ACF_NOWAIT)=0 then
    params.flags:=params.flags or ACTP_WAIT;

  if (flags and ACF_KEEPOLD)=0 then
    params.flags:=params.flags or ACTP_KEEPRESULT;

  params.wParam:=WorkData.Parameter;
  params.lParam:=WorkData.LastResult;
  res:=CallService(MS_ACT_RUNPARAMS,0,tlparam(@params));

  if (flags and ACF_KEEPOLD)=0 then
  begin
    ClearResult(WorkData);
    if (flags and ACF_SAMETHREAD)=0 then
    begin
      WorkData.LastResult:=res;
      WorkData.ResultType:=rtInt;
    end
    else
    begin
      WorkData.LastResult:=params.lParam;
      WorkData.ResultType:=params.lPType;
    end;
  end;
end;

procedure tChainAction.Load(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Load(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_BYNAME)=0 then
      begin
        StrCopy(pc,opt_chain); id:=DBReadDWord(0,DBBranch,section);
      end
      else
      begin
        StrCopy(pc,opt_actname); actname:=DBReadUnicode(0,DBBranch,section);
      end;
    end;

    100: begin
      flags:=flags and not ACF_BYNAME;
      pc:=StrCopyE(section,pAnsiChar(node));
      StrCopy(pc,'text'); id:=DBReadDWord(0,DBBranch,section);
    end;

    1: begin
      with xmlparser do
      begin
        StrDupW(actname,getText(HXML(node)));
        flags:=flags or ACF_BYNAME;

        if StrToInt(getAttrValue(HXML(node),ioNoWait))=1 then
          flags:=flags or ACF_NOWAIT;

        if StrToInt(getAttrValue(HXML(node),ioKeepOld))=1 then
          flags:=flags or ACF_KEEPOLD;

        if StrToInt(getAttrValue(HXML(node),ioSameThread))=1 then
          flags:=flags or ACF_SAMETHREAD;
      end;
    end;
{
    2: begin

      UF8ToWide(GetParamSectionStr(node,'name',nil),actname);
      flags:=flags or ACF_BYNAME;

      if GetParamSectionInt(node,ioNoWait)=1 then
        flags:=flags or ACF_NOWAIT;

      if GetParamSectionInt(node,ioKeepOld)=1 then
        flags:=flags or ACF_KEEPOLD;

      if GetParamSectionInt(node,ioSameThread)=1 then
        flags:=flags or ACF_SAMETHREAD;
    end;
}
  end;
end;

procedure tChainAction.Save(node:pointer;fmt:integer);
var
  section: array [0..127] of AnsiChar;
  pc:pAnsiChar;
begin
  inherited Save(node,fmt);
  case fmt of
    0: begin
      pc:=StrCopyE(section,pAnsiChar(node));
      if (flags and ACF_BYNAME)=0 then
      begin
        StrCopy(pc,opt_chain); DBWriteDWord(0,DBBranch,section,id);
      end
      else
      begin
        StrCopy(pc,opt_actname); DBWriteUnicode(0,DBBranch,section,actname);
      end;
    end;
{
    1: begin
    end;
}
{
    2: begin
    end;
}
{
    3: begin
      Out(node,['CallAction',actname,
                IFF(flags or ACF_SAMETHREAD,'samethread',''),
                IFF(flags or ACF_NOWAIT    ,'nowait',''),
                IFF(flags or ACF_KEEPOLD   ,'keepold','')
               ]);
    end;
}
    13: begin
      tTextExport(node).AddTextW('actionname',actname);
      tTextExport(node).AddFlag('samethread',(flags or ACF_SAMETHREAD)<>0);
      tTextExport(node).AddFlag('nowait'    ,(flags or ACF_NOWAIT    )<>0);
      tTextExport(node).AddFlag('keepold'   ,(flags or ACF_KEEPOLD   )<>0);
      tTextExport(node).AddNewLine();
    end;
  end;
end;

//----- Dialog realization -----

procedure FillChainList(Dialog:HWND);
var
  wnd,list:HWND;
  i:integer;

  li:LV_ITEMW;
  Macro:pMacroRecord;
begin
  wnd:=GetDlgItem(Dialog,IDC_MACRO_LIST);

  SendMessage(wnd,CB_RESETCONTENT,0,0);
  SendMessage(wnd,CB_SETITEMDATA,
    SendMessageW(wnd,CB_ADDSTRING,0,lparam(TranslateW(NoChainText))),0);

  list:=MacroListWindow;
  li.mask      :=LVIF_PARAM;
  li.iSubItem  :=0;
  for i:=0 to SendMessage(list,LVM_GETITEMCOUNT,0,0)-1 do
  begin
    li.iItem:=i;
    SendMessageW(list,LVM_GETITEMW,0,tlparam(@li));
    Macro:=EditMacroList[loword(li.lParam)];
    SendMessage(wnd,CB_SETITEMDATA,
        SendMessageW(wnd,CB_ADDSTRING,0,lparam(@(Macro.descr))),Macro.id);
  end;

end;

procedure ClearFields(Dialog:HWND);
begin
  CheckDlgButton(Dialog,IDC_MACRO_NOWAIT    ,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MACRO_SAMETHREAD,BST_UNCHECKED);
  CheckDlgButton(Dialog,IDC_MACRO_KEEPOLD   ,BST_UNCHECKED);
end;

function DlgProc(Dialog:HWND;hMessage:uint;wParam:WPARAM;lParam:LPARAM):LRESULT; stdcall;
var
  tmp:dword;
  wnd:HWND;
  bb:boolean;
begin
  result:=0;

  case hMessage of
    WM_INITDIALOG: begin
      TranslateDialogDefault(Dialog);

      FillChainList(Dialog);
    end;

    WM_ACT_SETVALUE: begin
      ClearFields(Dialog);
      with tChainAction(lParam) do
      begin
        if (flags and ACF_BYNAME)<>0 then
          SendDlgItemMessageW(Dialog,IDC_MACRO_LIST,CB_SELECTSTRING,twparam(-1),tlparam(actname))
        else
          CB_SelectData(Dialog,IDC_MACRO_LIST,id);
        if (flags and ACF_KEEPOLD)<>0 then
          CheckDlgButton(Dialog,IDC_MACRO_KEEPOLD,BST_CHECKED);

        if (flags and ACF_SAMETHREAD)<>0 then
        begin
          bb:=false;
          CheckDlgButton(Dialog,IDC_MACRO_SAMETHREAD,BST_CHECKED);
        end
        else
        begin
          bb:=true;
          if (flags and ACF_NOWAIT)<>0 then
            CheckDlgButton(Dialog,IDC_MACRO_NOWAIT,BST_CHECKED);
        end;
        EnableWindow(GetDlgItem(Dialog,IDC_MACRO_NOWAIT),bb);
      end;
    end;

    WM_ACT_RESET: begin
      ClearFields(Dialog);
      CheckDlgButton(Dialog,IDC_MACRO_SAMETHREAD,BST_CHECKED);
      SendDlgItemMessage(Dialog,IDC_MACRO_LIST,CB_SETCURSEL,0,0);
    end;

    WM_ACT_SAVE: begin
      with tChainAction(lParam) do
      begin
        id:=CB_GetData(GetDlgItem(Dialog,IDC_MACRO_LIST));

        if IsDlgButtonChecked(Dialog,IDC_MACRO_SAMETHREAD)<>BST_UNCHECKED then
          flags:=flags or ACF_SAMETHREAD
        else if IsDlgButtonChecked(Dialog,IDC_MACRO_NOWAIT)<>BST_UNCHECKED then
          flags:=flags or ACF_NOWAIT;

        if IsDlgButtonChecked(Dialog,IDC_MACRO_KEEPOLD)<>BST_UNCHECKED then
          flags:=flags or ACF_KEEPOLD;
      end;
    end;

    WM_ACT_LISTCHANGE: begin
      if wParam=1 then
      begin
        wnd:=GetDlgItem(Dialog,IDC_MACRO_LIST);
        tmp:=CB_GetData(wnd);
        FillChainList(Dialog);
        CB_SelectData(wnd,tmp);
      end;
    end;
	
    WM_COMMAND: begin
      case wParam shr 16 of
        CBN_SELCHANGE: SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        BN_CLICKED: begin
          if loword(wParam)=IDC_MACRO_SAMETHREAD then
          begin
            EnableWindow(GetDlgItem(Dialog,IDC_MACRO_NOWAIT),
                IsDlgButtonChecked(Dialog,IDC_MACRO_SAMETHREAD)=BST_UNCHECKED);
          end;

          SendMessage(GetParent(GetParent(Dialog)),PSM_CHANGED,0,0);
        end;
      end;
    end;

    WM_HELP: begin
      result:=1;
    end;
  end;
//  result:=DefWindowProc(Dialog,hMessage,wParam,lParam);
end;

//----- Export/interface functions -----

var
  vc:tActModule;

function CreateAction:tBaseAction;
begin
  result:=tChainAction.Create(vc.Hash);
end;

function CreateDialog(parent:HWND):HWND;
begin
  result:=CreateDialogW(hInstance,'IDD_ACTCHAIN',parent,@DlgProc);
end;

procedure Init;
begin
  vc.Next    :=ModuleLink;

  vc.Name    :='Chain';
  vc.Dialog  :=@CreateDialog;
  vc.Create  :=@CreateAction;
  vc.Icon    :='IDI_CHAIN';
  vc.Hash    :=0;

  ModuleLink :=@vc;
end;

begin
  Init;
end.
